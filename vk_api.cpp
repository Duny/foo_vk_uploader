#include "stdafx.h"

#include "boost/scope_exit.hpp"

namespace vk_uploader
{
    // {415971BA-5773-4843-9D18-09F28074F5F7}
    __declspec(selectany) const GUID vk_api_profider::class_guid = 
    { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };


    class api_profider_imp : public vk_api_profider
    {
        typedef pfc::array_t<t_uint8> membuf_ptr;
        void get_file_contents (const char *p_path, membuf_ptr &p_out)
        {
            file_ptr p_file;
            abort_callback_impl p_abort;

            filesystem::g_open_read (p_file, p_path, p_abort);
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

        response_json_ptr make_request (const char *p_api_name, params_cref p_params, abort_callback &p_abort)
        {
            http_request::ptr request;
            static_api_ptr_t<http_client>()->create_request ("GET")->service_query_t (request);

            pfc::string8_fast answer;
            request->run_ex (request_url_builder (p_api_name, p_params, p_abort), p_abort)->read_string_raw (answer, p_abort);
            return response_json_ptr (answer);
        }

        response_json_ptr invoke (const char *p_api_name, params_cref p_params, abort_callback &p_abort) override
        {
            m_invoker_avaliable.wait_for (-1);

            auto init_timers = [&] () { m_first_call_time = m_last_call_time = GetTickCount (); };
            {
                insync (m_section);

                if (m_call_count == pfc_infinite) { // initialize timers on the very first call
                    init_timers ();
                    m_call_count = 0;
                }

                m_call_count = (m_call_count + 1) % vk_api_profider::max_api_calls_per_second;

                if (m_call_count == 0) {
                    if ((m_last_call_time - m_first_call_time) < 1000) {
                        console::formatter () << "sleeping " << t_uint32(1100 - (m_last_call_time - m_first_call_time)) << " ms";
                        m_invoker_avaliable.set_state (false);
                        Sleep (1100 - (m_last_call_time - m_first_call_time));
                        m_invoker_avaliable.set_state (true);
                    }
                    init_timers ();
                }
            }

            BOOST_SCOPE_EXIT ((&m_section)(&m_last_call_time))
            {
                insync (m_section);
                m_last_call_time = GetTickCount ();
            } BOOST_SCOPE_EXIT_END

            return make_request (p_api_name, p_params, p_abort);
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

        DWORD m_first_call_time; // time then was made first call
        DWORD m_last_call_time; // time of the most resent call
        t_size m_call_count; // number of calls made since m_first_call_time
        critical_section m_section;

        win32_event m_invoker_avaliable;
    public:
        api_profider_imp () : m_call_count (pfc_infinite) { m_invoker_avaliable.create (true, true); }
    };
    static service_factory_single_t<api_profider_imp> g_api_profider_factory;
}