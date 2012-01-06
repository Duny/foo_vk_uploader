#include "stdafx.h"

#include "boost/scope_exit.hpp"
#include "vk_auth.h"
#include "vk_api.h"


namespace vk_uploader
{
    class api_url_builder
    {
        pfc::string8 m_url;
    public:
        api_url_builder (const char *p_method_name, const url_parameters & p_params)
        {
            m_url << "https://api.vk.com/method/" << p_method_name << "?";

            BOOST_FOREACH (const name_value_pair & p, p_params)
                m_url << p.first << "=" << string_url_encoded (p.second) << "&";

            m_url << "access_token=" << get_auth_manager ()->get_access_token ();
        }

        operator const char * () const { return m_url.get_ptr (); }
    };


    class api_provider_impl : public vk_api_provider
    {
        // Helpers
        void get_file_contents (const char *p_path, pfc::array_t<t_uint8> & p_out)
        {
            file_ptr p_file;
            abort_callback_impl p_abort;

            filesystem::g_open_read (p_file, p_path, p_abort);
            if (p_file.is_valid ()) {
                auto file_size = static_cast<t_size>(p_file->get_size (p_abort)); // TODO: Potential x64 error
                p_out.set_size (file_size);
                t_size read = p_file->read (p_out.get_ptr (), file_size, p_abort);
                if (read != file_size)
                    throw exception_io ();
            }
            else
                throw exception_io_not_found ();
        }

        response_json_ptr make_request (const char *p_api_name, const url_parameters & p_params, abort_callback & p_abort)
        {
            pfc::string8_fast answer;
            static_api_ptr_t<http_client>()->create_request ("GET")->run_ex (api_url_builder (p_api_name, p_params), p_abort)->read_string_raw (answer, p_abort);
            return response_json_ptr (answer);
        }

        response_json_ptr invoke (const char *p_api_name, const url_parameters & p_params, abort_callback &p_abort) override
        {
            m_invoker_avaliable.wait_for (-1);

            auto init_timers = [&] () { m_first_call_time = m_last_call_time = GetTickCount (); };
            {
                insync (m_section);

                if (m_call_count == pfc_infinite) { // initialize timers on the very first call
                    init_timers ();
                    m_call_count = 0;
                }

                m_call_count = (m_call_count + 1) % vk_api_provider::max_api_calls_per_second;

                // Not more than max_api_calls_per_second per one second (1000 ms)
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

        membuf_ptr upload_audio_file (const char *p_url, const char *p_file_path, abort_callback &p_abort) override
        {
            membuf_ptr data; data.new_t ();
            pfc::array_t<t_uint8> & byte_array = *data;

            // Read whole file into memory
            get_file_contents (p_file_path, byte_array);

            http_request_post::ptr request;
            static_api_ptr_t<http_client>()->create_request ("POST")->service_query_t (request);

            request->add_post_data ("file", byte_array.get_ptr (), byte_array.get_size (), pfc::string_filename_ext (p_file_path), "");
            //request->run_ex (p_url, p_abort)->read_string_raw (m_answer, p_abort);
            file_ptr answer = request->run_ex (p_url, p_abort);
            t_size answer_size = static_cast<t_size>(answer->get_size (p_abort));
            byte_array.set_size (answer_size);
            answer->read (byte_array.get_ptr (), answer_size, p_abort);
            return data;
        }

        DWORD m_first_call_time; // time then was made first call
        DWORD m_last_call_time; // time of the most resent call
        t_size m_call_count; // number of calls made since m_first_call_time
        critical_section m_section;

        win32_event m_invoker_avaliable;
    public:
        api_provider_impl () : m_call_count (pfc_infinite) { m_invoker_avaliable.create (true, true); }
    };

    namespace { service_factory_single_t<api_provider_impl> g_factory; }
}