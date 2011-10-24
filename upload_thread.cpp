#include "stdafx.h"

#include "vk_api.h"
#include "upload_thread.h"

namespace vk_uploader
{
    namespace upload_queue
    {
        void upload_thread::run (threaded_process_status &p_status, abort_callback &p_abort)
        {
            try {
                static_api_ptr_t<vk_api::profider> api;

                response_json_ptr result = api->call_api ("audio.getUploadServer");
                if (result.is_valid ()) {
                    membuf_ptr data;
                    get_file_contents (m_file, data);

                    http_request_post::ptr request;
                    static_api_ptr_t<http_client>()->create_request ("POST")->service_query_t (request);
                    request->add_post_data ("file", data.get_ptr (), data.get_size (), "", "");

                    file_ptr response = request->run_ex (result["upload_url"].asCString (), p_abort);

                    pfc::string8_fast answer;
                    response->read_string_raw (answer, p_abort);

                    popup_message::g_show (answer, "");

                    response_json_ptr result (answer);
                    if (result.is_valid ()) {
                        url_params params;

                        params["server"] = result["server"].asCString ();
                        params["audio"] = result["audio"].asCString ();
                        params["hash"] = result["hash"].asCString ();

                        response_json_ptr result = get_api_provider ()->call_api ("audio.save", params);
                        if (result.is_valid ()) {
                            popup_message::g_show (result->toStyledString ().c_str (), "");
                            //debug_log () << "Finished upload of " << p_task.m_location;
                        }
                        else
                            throw pfc::exception (result.get_error_code ());
                    }
                    else
                        throw pfc::exception (result.get_error_code ());
                }
                else
                    throw pfc::exception (result.get_error_code ());
            } catch (const std::exception &e) {
                m_error = e.what ();
            }
        }

        void upload_thread::on_done (HWND p_wnd, bool p_was_aborted)
        { 
            if (p_was_aborted)
                m_error = "upload was aborted by user.";
            const_cast<win32_event&>(m_event_upload_finished).set_state (true);
        }

        void upload_thread::get_file_contents (const char *p_location, membuf_ptr &p_out)
        {
            file_ptr p_file;
            abort_callback_impl p_abort;

            filesystem::g_open_read (p_file, p_location, p_abort);
            if (p_file.is_valid ()) {
                t_size file_size = (t_size)p_file->get_size (p_abort);
                p_out.set_size (file_size);
                t_size read = p_file->read (p_out.get_ptr (), file_size, p_abort);
                if (read != file_size)
                    throw exception_io ();
            }
            else
                throw exception_io_not_found ();
        }
    }
}