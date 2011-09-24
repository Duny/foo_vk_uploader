#pragma once

namespace vk_uploader
{
    namespace vk_api
    {
        // our registered application id
        extern const __declspec(selectany) pfc::string8 app_id = "2435833";

        // for meaning of next 2 const see 
        // http://vkontakte.ru/developers.php?oid=-1&p=%D0%94%D0%B8%D0%B0%D0%BB%D0%BE%D0%B3_%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_OAuth
        //
        // url where browser is redirected after successful login
        extern const __declspec(selectany) pfc::string8 redirect_url = "http://api.vk.com/blank.htm";
        // url for user authentication
        extern const __declspec(selectany) pfc::string8 oauth_url = pfc::string_formatter () 
            << "http://api.vk.com/oauth/authorize?client_id=" << app_id << "&redirect_uri=" << redirect_url << "&scope=audio&display=popup&response_type=token";
        

        typedef pfc::map_t<pfc::string8, pfc::string8> str2str_map;

        class vk_api
        {
            static const char *g_redirect_url;
            static const char *g_app_id;

            static pfc::string8 g_auth_url;

            static bool make_get_request (const pfc::string8_fast &url, pfc::string8_fast &answer);
        public:
            // api calls error codes
            enum error_codes {
                unknown_error = 1,
                app_is_disabled,
                unknown_method,
                invalid_sig,
                auth_failed
            };

            static void init ();

            static const pfc::string8 &get_auth_url () { return g_auth_url; }
	        static bool get_url_params (const pfc::string8 &url, str2str_map &params);
            static bool is_redirect_url (const pfc::string8 &url) { return url.find_first (g_redirect_url) == 0; }

            static bool call (const char *method, const char *params, Json::Value &out);
        };
    }
}