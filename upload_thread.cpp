#include "stdafx.h"

#include "vk_api.h"
#include "upload_thread.h"

namespace vk_uploader
{
    namespace upload_queue
    {
        void upload_thread::run (threaded_process_status & p_status,abort_callback & p_abort)
        {
            try {
                http_request_post::ptr request;
                static_api_ptr_t<http_client>()->create_request ("POST")->service_query_t (request);
                request->add_post_data ("file", m_data->get_ptr (), m_data->get_size (), "", "");

                abort_callback_dummy p_abort;
                file_ptr response = request->run_ex (m_url, p_abort);

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
            } catch (const std::exception &e) {
                m_error = e.what ();
            }
        }
    }
}