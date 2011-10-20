#include "stdafx.h"

#include "upload_queue.h"
#include "vk_api.h"

namespace vk_uploader
{
    namespace upload_queue
    {
        namespace cfg
        {
            struct upload_task
            {
                upload_task (metadb_handle_ptr p_item, const upload_presets::preset &p_preset) : m_location (p_item->get_location ()), m_preset (p_preset) {}
                upload_task () {}

                playable_location_impl m_location;
                upload_presets::preset m_preset;

                bool operator== (const upload_task &other) { return m_location == other.m_location; }
                bool operator!= (const upload_task &other) { return m_location != other.m_location; }
            };

            FB2K_STREAM_READER_OVERLOAD(playable_location_impl) 
            {
                pfc::string8_fast path;
                t_uint32 subsong;
                stream >> path >> subsong;
                value.set_path (path); value.set_subsong (subsong);
                return stream;
            }
            FB2K_STREAM_WRITER_OVERLOAD(playable_location_impl) { return stream << pfc::string8_fast (value.get_path ()) << value.get_subsong (); }

            FB2K_STREAM_READER_OVERLOAD(upload_presets::preset) { return stream >> value.m_album >> value.m_post_on_wall; }
            FB2K_STREAM_WRITER_OVERLOAD(upload_presets::preset) { return stream << value.m_album << value.m_post_on_wall; }
        
            FB2K_STREAM_READER_OVERLOAD(upload_task) { return stream >> value.m_location >> value.m_preset; }
            FB2K_STREAM_WRITER_OVERLOAD(upload_task) { return stream << value.m_location << value.m_preset; }
        
            class upload_queue_mt : private cfg_objList<upload_task>
            {
                mutable critical_section m_section;
            public:
                upload_queue_mt () : cfg_objList<upload_task> (guid_inline<0x1052cea1, 0x728f, 0x4d79, 0xa2, 0xdf, 0xc2, 0xda, 0xb4, 0x7a, 0x66, 0xeb>::guid) {}

                t_size get_task_count () const { insync (m_section); return get_count (); }

                bool get_top_task (upload_task &p_out) const
                {
                    insync (m_section);
                    bool result = get_count () != 0;
                    if (result) p_out = get_item (0);
                    return result;
                }

                void delete_task (const upload_task &p_task) { insync (m_section); remove_item (p_task); }

                void push_back (metadb_handle_list_cref p_items, const upload_presets::preset &p_preset)
                {
                    insync (m_section);
                    p_items.enumerate ([&] (metadb_handle_ptr p_item) { add_item (upload_task (p_item, p_preset)); });
                }
            };
        }
        cfg::upload_queue_mt tasks_queue;

        
        class que_manage_thread_t : public pfc::thread
        {
            bool filter_bad_file (const metadb_handle_ptr &p_item)
            {
                const t_size max_file_size = 20 * (1 << 20); // 20Mb

                // first test: file size
                t_filesize size = p_item->get_filesize ();
                if (!(size > 0 && size < max_file_size)) return true;

                // second test: file type (mp3, lossy)
                metadb_handle_lock lock (p_item);
                const file_info *p_info;
                if (p_item->get_info_locked (p_info)) {
                    const char *codec = p_info->info_get ("codec");
                    if (codec) {
                        if (pfc::stricmp_ascii ("MP3", codec) != 0) {
                            debug_log () << "Skipping location " << p_item->get_location () << " (File is not MP3. Codec:" << codec << ")";
                            return true;
                        }   
                    }

                    const char *encoding = p_info->info_get ("encoding");
                    if (encoding) {
                        if (pfc::stricmp_ascii ("lossy", encoding) != 0) {
                            debug_log () << "Skipping location " << p_item->get_location () << " (File is not lossy. Encoding:" << encoding << ")";
                            return true;
                        }
                    }
                }
                return false;
            }

            typedef boost::shared_ptr<pfc::array_t<t_uint8>> membuf_ptr;

            membuf_ptr get_file_contents (const playable_location &p_location) const
            {
                file_ptr p_file;
                abort_callback_impl p_abort;

                filesystem::g_open_read (p_file, p_location.get_path (), p_abort);
                if (p_file.is_valid ()) {
                    t_size file_size = (t_size)p_file->get_size (p_abort);
                    membuf_ptr data (new pfc::array_t<t_uint8>);
                    data->set_size (file_size);
                    t_size read = p_file->read (data->get_ptr (), file_size, p_abort);
                    if (read == file_size)
                        return data;
                    else
                        throw exception_io ();
                }
                else
                    throw exception_io_not_found ();

            }

            void run_task (const cfg::upload_task &p_task)
            {
                metadb_handle_ptr item;
                static_api_ptr_t<metadb>()->handle_create (item, p_task.m_location);
                if (item.is_valid () && filter_bad_file (item) == false) {
                    static_api_ptr_t<vk_api::profider> api;
                    
                    response_json_ptr result = get_api_provider ()->call_api ("audio.getUploadServer");
                    if (result.is_valid ()) {
                        try {
                            membuf_ptr file_contents = get_file_contents (p_task.m_location);
                            http_request_post::ptr request;
                            static_api_ptr_t<http_client>()->create_request ("POST")->service_query_t (request);
                            request->add_post_data ("file", file_contents->get_ptr (), file_contents->get_size (), pfc::string_filename_ext (p_task.m_location.get_path ()), "");

                            abort_callback_dummy p_abort;
                            file_ptr response = request->run_ex (result["upload_url"].asCString (), p_abort);

                            pfc::string8_fast answer;
                            response->read_string_raw (answer, p_abort);

                            response_json_ptr result (answer);
                            if (result.is_valid ()) {
                                url_params params;

                                params["server"] = result["server"].asCString ();
                                params["audio"] = result["audio"].asCString ();
                                params["hash"] = result["hash"].asCString ();

                                response_json_ptr result = get_api_provider ()->call_api ("audio.save", params);
                                if (result.is_valid ()) {
                                    popup_message::g_show (result->toStyledString ().c_str (), "");
                                    debug_log () << "Finished upload of " << p_task.m_location;
                                }
                                else {
                                    static char buf[1024];

                                    strcpy_s (buf, result.get_error ());
                                    throw pfc::exception (buf);
                                }
                            }
                            else {
                                pfc::string8 error_mgs = result.get_error ();
                                throw pfc::exception (error_mgs);
                            }
                        } catch (const std::exception &e) {
                            debug_log () << "Uploading file " << p_task.m_location << " resulted in error: " << e.what ();
                        }
                    }
                    else
                        debug_log () << "getUploadServer returned: " << result.get_error ();
                }
            }

            void threadProc () override
            {
                while (1) {
                    m_new_data_avalible.wait_for (-1);

                    while (tasks_queue.get_task_count () > 0) {
                        cfg::upload_task top_task;
                        if (tasks_queue.get_top_task (top_task)) {
                            run_task (top_task);
                            tasks_queue.delete_task (top_task);
                        }
                    }
                    m_new_data_avalible.set_state (false);
                }
            }

            win32_event m_new_data_avalible; // user added tracks for upload
        public:
            que_manage_thread_t () { m_new_data_avalible.create (true, false); }
            ~que_manage_thread_t () { waitTillDone (); }

            void signalize_new_data () { m_new_data_avalible.set_state (true); }
        } que_manage_thread;

        namespace
        {
            class myinitquit : public initquit
            {
                void on_init () override
                {
                    que_manage_thread.startWithPriority (THREAD_PRIORITY_LOWEST);
                    if (tasks_queue.get_task_count ()) que_manage_thread.signalize_new_data ();
                }

                void on_quit () override {}
            };
            static initquit_factory_t<myinitquit> g_initquit;
        }

        class NOVTABLE manager_imp : public manager
        {
            void push_back (metadb_handle_list_cref p_items, const upload_presets::preset &p_preset) override
            {
                tasks_queue.push_back (p_items, p_preset);
                que_manage_thread.signalize_new_data ();
            }
        };
        static service_factory_single_t<manager_imp> g_upload_que_manager_factory;
    }
}