#include "stdafx.h"

#include "vk_api.h"

// information about calling vk api can be found here:
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%92%D0%B7%D0%B0%D0%B8%D0%BC%D0%BE%D0%B4%D0%B5%D0%B9%D1%81%D1%82%D0%B2%D0%B8%D0%B5_%D0%BF%D1%80%D0%B8%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD%D0%B8%D1%8F_%D1%81_API

// information about api methods is located there:
// http://vkontakte.ru/developers.php?o=-1&p=%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5_%D0%BC%D0%B5%D1%82%D0%BE%D0%B4%D0%BE%D0%B2_API&s=0

namespace vk_uploader
{
    namespace vk_api
    {
        class api_invoker_imp : public api_invoker
        {
            ULONGLONG m_first_call_time; // time then was made first call
            ULONGLONG m_last_call_time; // time of the most resent call
            t_size m_call_count; // number of calls made since m_first_call_time
            critical_section m_section;

            win32_event m_invoker_avaliable;

            value_t make_request (const char *p_api_name, params_cref p_params)
            {
                try {
                    request_url_builder url (p_api_name, p_params);
                    static_api_ptr_t<http_client> client;
                    http_request::ptr request;

                    client->create_request ("GET")->service_query_t (request);

                    file::ptr response = request->run_ex (url, abort_callback_dummy ());
                    pfc::string8_fast answer;
                    response->read_string_raw (answer, abort_callback_dummy ());
                    popup_message::g_show (answer, "");
                    return from_string (answer);
                } catch (const std::exception &e) {
                    return make_error (e.what ());
                }
            }

            value_t invoke (const char *p_api_name, params_cref p_params) override
            {
                m_invoker_avaliable.wait_for (-1);

                {
                    auto init_timers = [&] () { m_first_call_time = m_last_call_time = GetTickCount64 (); };

                    insync (m_section);

                    if (m_call_count == pfc_infinite) {
                        init_timers ();
                        m_call_count = 0;
                    }

                    m_call_count = (m_call_count + 1) % profider::max_api_calls_per_second;

                    if (m_call_count == 0) {
                        if ((m_last_call_time - m_first_call_time) < 1000) {
                            m_invoker_avaliable.set_state (false);
                            Sleep ((DWORD)(1000 - (m_last_call_time - m_first_call_time)) + 100);
                            m_invoker_avaliable.set_state (true);
                        }
                        init_timers ();
                    }
                }

                value_t result = make_request (p_api_name, p_params);
                ULONGLONG current_time = GetTickCount64 ();
                {
                    insync (m_section);
                    if (m_last_call_time < current_time) m_last_call_time = current_time;
                }

                return result;
            }
        public:
            api_invoker_imp () : m_call_count (pfc_infinite)
            { m_invoker_avaliable.create (true, true); }
        };

        static service_factory_single_t<api_invoker_imp> g_api_invoker_factory;
    }
}