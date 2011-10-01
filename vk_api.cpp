#include "stdafx.h"


#include "vk_api.h"

namespace vk_uploader
{
    namespace vk_api
    {
        /*const char *vk_api::g_app_id = "2435833";
        const char *vk_api::g_redirect_url = "http://api.vkontakte.ru/blank.html";
        pfc::string8 vk_api::g_auth_url;*/

        //void vk_api::init ()
        //{
        //    {
        //        g_auth_url = "http://api.vkontakte.ru/oauth/authorize?client_id="; g_auth_url += g_app_id;
        //        g_auth_url += "&scope=audio&redirect_uri="; g_auth_url += g_redirect_url;
        //        g_auth_url += "&display=popup&response_type=token";
        //    }
        //}

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