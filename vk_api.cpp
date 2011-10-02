#include "stdafx.h"

#include "vk_api_invoker.h"

// information about calling vk api can be found here:
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%92%D0%B7%D0%B0%D0%B8%D0%BC%D0%BE%D0%B4%D0%B5%D0%B9%D1%81%D1%82%D0%B2%D0%B8%D0%B5_%D0%BF%D1%80%D0%B8%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD%D0%B8%D1%8F_%D1%81_API

// information about api methods is located there:
// http://vkontakte.ru/developers.php?o=-1&p=%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5_%D0%BC%D0%B5%D1%82%D0%BE%D0%B4%D0%BE%D0%B2_API&s=0

namespace vk_uploader
{
    namespace vk_api
    {
        class request_builder
        {
            pfc::string8 m_url;
        public:
            request_builder () {}

            operator const char * () const { return m_url.get_ptr (); }
        };

        class profider_imp : public profider
        {            
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
        //bool vk_api::get_url_params (const pfc::string8 &url, str2str_map &params)
        //{
        //    params.remove_all ();

        //    // find starting position of url params
        //    t_size pos;
        //    if ((pos = url.find_first ('#')) == pfc_infinite &&
		      //  (pos = url.find_first ('?')) == pfc_infinite)
        //        return false;
    
        //    t_size len = url.length (), pos2, pos3;
        //    pfc::string8 name, value;
        //    for (pos = pos; pos < len; pos = pos3) {
        //        if ((pos2 = url.find_first ('=', pos)) == pfc_infinite)
        //            break;

        //        if ((pos3 = url.find_first ('&', pos2)) == pfc_infinite)
        //            pos3 = len;
    
        //        name.reset (); name.prealloc (pos2 - pos);
        //        for (t_size i = pos + 1; i < pos2; i++)
        //            name.add_char (url[i]);

        //        value.reset (); value.prealloc (pos3 - pos2);
        //        for (t_size i = pos2 + 1; i < pos3; i++)
        //            value.add_char (url[i]);

        //        params[name] = value;
        //    }

        //    return true;
        //}

        //bool vk_api::make_get_request (const pfc::string8_fast &url, pfc::string8_fast &answer)
        //{
	       // try {
		      //  static_api_ptr_t<http_client> client;

		      //  http_request::ptr request;
		      //  client->create_request ("GET")->service_query_t (request);

		      //  file::ptr response = request->run_ex (url, abort_callback_dummy ());      
		      //  response->read_string_raw (answer, abort_callback_dummy ());
		      //  return true;
	       // }
	       // catch (...) {
		      //  return false;
	       // }
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