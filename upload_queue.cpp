#include "stdafx.h"

#include "vk_api.h"
#include "upload_preset.h"
#include "upload_thread.h"

namespace vk_uploader
{
    namespace upload_queue
    {
        struct upload_job
        {
            upload_job () {}
            upload_job (metadb_handle_list_cref p_items, const upload_params &p_preset) : m_preset (p_preset)
            { m_items.add_items (p_items); }
                
            metadb_handle_list m_items;
            upload_params m_preset;

            bool operator== (const upload_job &other) { return m_preset == other.m_preset && (m_items == other.m_items); }
            bool operator!= (const upload_job &other) { return !operator== (other); }
        };


        class job_queue_mt : private pfc::list_t<upload_job>
        {
            mutable critical_section m_section;
        public:
            t_size get_count () const { insync (m_section); return pfc::list_t<upload_job>::get_count (); }

            bool get_top (upload_job &p_out) const
            {
                insync (m_section);
                bool result = get_count () != 0;
                if (result) p_out = get_item (0);
                return result;
            }

            void remove (const upload_job &p_job) { insync (m_section); remove_item (p_job); }

            void push_back (metadb_handle_list_cref p_items, const upload_params &p_params)
            {
                insync (m_section);
                add_item (upload_job (p_items, p_params));
            }
        };

        
        class queue_manager_thread_t : pfc::thread
        {
            class my_main_thread_callback : public main_thread_callback
            {
                service_ptr_t<upload_thread> m_thread;

                void callback_run() override { m_thread->start (); }
            public:
                my_main_thread_callback (const service_ptr_t<upload_thread> &p_thread) : m_thread (p_thread) {}
            };

            void threadProc () override
            {
                while (1) {
                    m_new_data_avalible.wait_for (-1);

                    while (m_queue.get_count () > 0) {
                        upload_job j;
                        if (m_queue.get_top (j)) {
                            m_queue.remove (j);
                            
                            if (j.m_items.get_count ()) {
                                m_item_upload_done.set_state (false);
                                service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (j.m_items, j.m_preset, m_item_upload_done);
                                main_thread_callback_spawn<my_main_thread_callback> (thread);
                                m_item_upload_done.wait_for (-1);
                            }
                        }
                    }
                    m_new_data_avalible.set_state (false);
                }
            }

            win32_event m_new_data_avalible, m_item_upload_done;
            job_queue_mt m_queue;
        public:
            queue_manager_thread_t () { m_new_data_avalible.create (true, false); m_item_upload_done.create (true, false); }
            ~queue_manager_thread_t () { waitTillDone (); }

            void start () { if (!isActive ()) startWithPriority (THREAD_PRIORITY_IDLE); }

            void push_back (metadb_handle_list_cref p_items, const upload_params &p_params)
            {
                m_queue.push_back (p_items, p_params);
                m_new_data_avalible.set_state (true);
            }
        } queue_manager_thread;
        
        namespace
        {
            class myinitquit : public initquit
            {
                void on_init () override { queue_manager_thread.start (); }
                void on_quit () override {}
            };
            static initquit_factory_t<myinitquit> g_initquit;
        }


        void push_back (metadb_handle_list_cref p_items, const upload_params &p_params)
        {
            queue_manager_thread.push_back (p_items, p_params);
        }
    }
}