#include "stdafx.h"

#include "login_dlg.h"
#include "vk_api_invoker.h"
#include "vk_api_helpers.h"

namespace vk_uploader
{
    namespace vk_api
    {
        namespace cfg
        {
            /*namespace guid
            {
                const GUID user_id = { 0x2e35f7c1, 0xd070, 0x4f01, { 0xb1, 0x2d, 0x6c, 0x21, 0x7f, 0x12, 0xa2, 0x93 } };
                const GUID secret = { 0xaf320b92, 0x3a7f, 0x490d, { 0xa5, 0x34, 0x91, 0x3a, 0x14, 0xe3, 0x51, 0x68 } };
            }*/

            //static cfg_string cfg_user_id (guid::user_id, ""), cfg_secret (guid::secret, "");
            static pfc::string8 user_id, secret;

            void get_auth ()
            {
                login_dlg dlg;
                dlg.DoModal (core_api::get_main_window ()); 
                url_params params = dlg.get_final_url ();
                console::formatter () << dlg.get_final_url ();

                if (params.have_item (pfc::string8 ("error")))
                    throw exception_auth_failed (params["error"]);
                else if (!params.have_item (pfc::string8 ("user_id")) || !params.have_item (pfc::string8 ("access_token")))
                    throw exception_auth_failed ("Not enough parameters returned by server");

                user_id = params[pfc::string8 ("user_id")];
                secret = params[pfc::string8 ("access_token")];
            }
        }

        const char *string_constants::get_user_id ()
        {
            if (cfg::user_id.is_empty ())
                cfg::get_auth ();
            return cfg::user_id;
        }

        const char *string_constants::get_sid ()
        {
            if (cfg::secret.is_empty ())
                cfg::get_auth ();
            return cfg::secret;
        }

        class profider_imp : public profider
        {            
            // must be instanced with operator new
            class call_api_thread : public pfc::thread
            {
                const char *m_api_name;
                params_t m_api_params;
                const api_callback &m_api_callback;

                void threadProc () override
                {
                    result_t result = static_api_ptr_t<api_invoker>()->invoke (m_api_name, m_api_params);
                    const_cast<api_callback&>(m_api_callback).on_done (result);
                    delete this;
                }

                ~call_api_thread () { waitTillDone (); }
            public:
                call_api_thread (const char *p_api_name, params_cref p_params, const api_callback &p_callback)
                    : m_api_name (p_api_name), m_api_params (p_params), m_api_callback (p_callback)
                { start (); }
            };
            

            result_t call_api (const char *p_api_name, params_cref p_params) override
            {
                return static_api_ptr_t<api_invoker>()->invoke (p_api_name, p_params);
            }
            
            void call_api_async (const char *p_api_name, params_cref p_params, const api_callback &p_callback) override
            {
                new call_api_thread (p_api_name, p_params, p_callback);
            }
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