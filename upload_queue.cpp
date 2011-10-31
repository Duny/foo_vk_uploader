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

            pfc::list_t<t_audio_id> m_ids; // ids of uploaded items

            bool operator== (const upload_job &other) { return m_preset == other.m_preset; }
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

        
        class queue_manager_thread_t : private pfc::thread
        {
            class my_main_thread_callback : public main_thread_callback
            {
                service_ptr_t<upload_thread> m_thread;

                void callback_run() override { m_thread->start (); }
            public:
                my_main_thread_callback (service_ptr_t<upload_thread> p_thread) : m_thread (p_thread) {}
            };

            pfc::string8_fast upload_items (upload_job &p_job)
            {
                pfc::string8_fast errors;

                p_job.m_items.for_each ([&] (metadb_handle_ptr p_item)
                {
                    pfc::string8_fast reason;

                    if (filter_bad_file (p_item, reason))
                        errors << "Skipping " << p_item->get_path () << ": " << reason << "\n";
                    else {
                        m_item_upload_done.set_state (false);

                        service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (p_item, m_item_upload_done);
                        main_thread_callback_spawn<my_main_thread_callback> (thread);

                        m_item_upload_done.wait_for (-1);

                        if (thread->successed ())
                            p_job.m_ids.add_item (thread->get_audio_id ());
                        else
                            errors << p_item->get_path () << " failed: " << thread->get_error () << "\n";
                    }
                });
                return errors;
            }

            pfc::string8_fast post_process (const upload_job &p_job)
            {
                pfc::string8_fast errors;

                if (p_job.m_preset.m_post_on_wall) {
                    vk_api::api_wall_post post (p_job.m_preset.m_post_mgs, p_job.m_ids);
                    if (!post.is_valid ())
                        errors << "Error while posting message on the wall: " << post.get_error ();
                }
                
                return errors;
            }

            void threadProc () override
            {
                while (1) {
                    m_new_data_avalible.wait_for (-1);

                    while (m_queue.get_count () > 0) {
                        upload_job j;
                        if (m_queue.get_top (j)) {
                            m_queue.remove (j);
                            
                            if (j.m_items.get_count ()) {
                                pfc::string8_fast errors = upload_items (j);
                                errors += post_process (j);

                                if (!errors.is_empty ())
                                    popup_message::g_show (errors, "Some items failed to upload", popup_message::icon_error);
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