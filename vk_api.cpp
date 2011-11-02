#include "stdafx.h"

#include "vk_api.h"
#include "vk_api_invoker.h"

// calling api methods :
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%92%D0%B7%D0%B0%D0%B8%D0%BC%D0%BE%D0%B4%D0%B5%D0%B9%D1%81%D1%82%D0%B2%D0%B8%D0%B5_%D0%BF%D1%80%D0%B8%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD%D0%B8%D1%8F_%D1%81_API

// list of api methods :
// http://vkontakte.ru/developers.php?o=-1&p=%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5_%D0%BC%D0%B5%D1%82%D0%BE%D0%B4%D0%BE%D0%B2_API&s=0

namespace vk_uploader
{
    namespace vk_api
    {
        class api_profider_imp : public api_profider
        {
            DWORD m_first_call_time; // time then was made first call
            DWORD m_last_call_time; // time of the most resent call
            t_size m_call_count; // number of calls made since m_first_call_time
            critical_section m_section;

            win32_event m_invoker_avaliable;

            response_json_ptr make_request (const char *p_api_name, params_cref p_params)
            {
                try {
                    http_request::ptr request;
                    static_api_ptr_t<http_client>()->create_request ("GET")->service_query_t (request);

                    abort_callback_dummy p_abort;
                    pfc::string8_fast answer;
                    request->run_ex (request_url_builder (p_api_name, p_params), p_abort)->read_string_raw (answer, p_abort);

                    return response_json_ptr (answer);
                } catch (const std::exception &e) {
                    return make_error_response (e.what ());
                }
            }

            response_json_ptr call_api (const char *p_api_name, params_cref p_params) override
            {
                m_invoker_avaliable.wait_for (-1);

                {
                    auto init_timers = [&] () { m_first_call_time = m_last_call_time = GetTickCount (); };

                    insync (m_section);

                    if (m_call_count == pfc_infinite) {
                        init_timers ();
                        m_call_count = 0;
                    }

                    m_call_count = (m_call_count + 1) % api_profider::max_api_calls_per_second;

                    if (m_call_count == 0) {
                        if ((m_last_call_time - m_first_call_time) < 1000) {
                            m_invoker_avaliable.set_state (false);
                            Sleep (1100 - (m_last_call_time - m_first_call_time));
                            m_invoker_avaliable.set_state (true);
                        }
                        init_timers ();
                    }
                }

                response_json_ptr result = make_request (p_api_name, p_params);
                DWORD current_time = GetTickCount ();
                {
                    insync (m_section);
                    if (m_last_call_time < current_time) m_last_call_time = current_time;
                }

                return result;
            }

            pfc::string8_fast file_upload (const char *p_url, const char *p_file, abort_callback &p_abort) override
            {
                membuf_ptr data;
                get_file_contents (p_file, data);

                http_request_post::ptr request;
                static_api_ptr_t<http_client>()->create_request ("POST")->service_query_t (request);

                request->add_post_data ("file", data.get_ptr (), data.get_size (), pfc::string_filename_ext (p_file), "");

                pfc::string8_fast answer;
                request->run_ex (p_url, p_abort)->read_string_raw (answer, p_abort);

                //popup_message::g_show (answer, "");
                return answer;
            }

        public:
           api_profider_imp () : m_call_count (pfc_infinite) { m_invoker_avaliable.create (true, true); }
        };
        static service_factory_single_t<api_profider_imp> g_api_profider_factory;
    }
}