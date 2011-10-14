#include "stdafx.h"

#include "vk_api.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class profider_imp : public profider
        {            
            // must be instanced with operator new
            /*class call_api_thread : public pfc::thread
            {
                const char *m_api_name;
                params_t m_api_params;
                api_callback &m_api_callback;

                void threadProc () override
                {
                    response result = static_api_ptr_t<api_invoker>()->invoke (m_api_name, m_api_params);
                    m_api_callback.on_request_done (m_api_name, result);
                    delete this;
                }

                ~call_api_thread () { waitTillDone (); }
            public:
                call_api_thread (const char *p_api_name, params_cref p_params, api_callback &p_callback)
                    : m_api_name (p_api_name), m_api_params (p_params), m_api_callback (p_callback)
                { start (); }
            };*/
            

            response_json call_api (const char *p_api_name, params_cref p_params) override
            {
                return static_api_ptr_t<api_invoker>()->invoke (p_api_name, p_params);
            }
            
            /*void call_api_async (const char *p_api_name, params_cref p_params, api_callback &p_callback) override
            {
                new call_api_thread (p_api_name, p_params, p_callback);
            }*/
        };
        static service_factory_single_t<profider_imp> g_api_profider_factory;

        /*
        //bool vk_api::make_get_request (const pfc::string8_fast &url, pfc::string8_fast &answer)
        //{
	       // 
        //}

        /*bool vk_api::call (const char *method, const char *params, Json::Value &out)
        {
	        if (vk_access_token.is_empty () && !vk_login_dialog ().show ())
		        return false;

	        pfc::string8_fast url = 
		        pfc::string_formatter () << "https://api.vkontakte.ru/method/" << method << "?"
		        << params << (params && *params ? "&" : "") << "access_token=" << vk_access_token;
	        pfc::string8_fast answer;

	        if (!make_get_request (url, answer))
		        return false;

	        Json::Reader reader;
	        Json::Value val;

	        out.clear ();
            if (!reader.parse (std::string (answer.get_ptr ()), out, false) || !out.isObject ()) {
                console::formatter () << "foo_vk_uploader: unexpected reply:\n" << answer;
		        return false;
            }

	        if (!(val = out["error_code"]).isNull ()) {
		        if (!val.isInt ())
			        return false;

		        int error_code = val.asInt ();
		        if (error_code == 4 || error_code == 5) {
			        if (!vk_login_dialog ().show () || vk_access_token.is_empty () || !make_get_request (url, answer))
				        return false;
			
			        out.clear ();
                    if (!reader.parse (std::string (answer.get_ptr ()), out, false) || !out.isObject ()) {
                        console::formatter () << "foo_vk_uploader: unexpected reply:\n" << answer;
				        return false;
                    }

			        if (!(val = out["error_code"]).isNull ())
				        return false;
		        }
		        else
			        return false;
	        }
	
	        return true;
        }*/
    }
}