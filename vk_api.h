#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#include "boost/smart_ptr.hpp"

namespace vk_uploader
{
    namespace vk_api
    {   
        class string_constants
        {
        public:
            static const pfc::string8 app_id;

            // for meaning of next 2 const see
            // http://vkontakte.ru/developers.php?oid=-1&p=%D0%94%D0%B8%D0%B0%D0%BB%D0%BE%D0%B3_%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_OAuth
            static const pfc::string8 redirect_url;
            static const pfc::string8 oauth_url;
        };

        __declspec(selectany) const pfc::string8 string_constants::app_id = "2435833";
        __declspec(selectany) const pfc::string8 string_constants::redirect_url = "http://api.vk.com/blank.htm";
        __declspec(selectany) const pfc::string8 string_constants::oauth_url = pfc::string_formatter () 
            << "http://api.vk.com/oauth/authorize?client_id=" << app_id << "&redirect_uri=" << redirect_url << "&scope=audio&display=popup&response_type=token";


        typedef pfc::map_t<pfc::string8, pfc::string8> params_t;
        typedef params_t const & params_cref;
        typedef boost::shared_ptr<Json::Value> result_t;
        
        class api_callback
        {
        public:
            virtual void on_done (const result_t &p_result) = 0;

            virtual void on_error (const pfc::string8 &p_message) = 0;
        };

        class NOVTABLE profider : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(profider)
        public:
            enum { max_api_calls_per_second = 3 };

            // makes synchronous api call 
            virtual result_t call_api (const char *p_api_name, params_cref p_params) = 0;
            inline result_t call_api (const char *p_api_name) { return call_api (p_api_name, params_t ()); }

            // make asynchronous call
            virtual void call_api_async (const char *p_api_name, params_cref p_params, const api_callback &p_callback) = 0;
            inline void call_api_async (const char *p_api_name, const api_callback &p_callback) { call_api_async (p_api_name, params_t (), p_callback); }
        };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID profider::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };


        //typedef pfc::map_t<pfc::string8, pfc::string8> str2str_map;

        //class vk_api
        //{
        //    static const char *g_redirect_url;
        //    static const char *g_app_id;

        //    static pfc::string8 g_auth_url;

        //    static bool make_get_request (const pfc::string8_fast &url, pfc::string8_fast &answer);
        //public:
        //    // api calls error codes
        //    enum error_codes {
        //        unknown_error = 1,
        //        app_is_disabled,
        //        unknown_method,
        //        invalid_sig,
        //        auth_failed
        //    };

        //    static void init ();

        //    static const pfc::string8 &get_auth_url () { return g_auth_url; }
	       // static bool get_url_params (const pfc::string8 &url, str2str_map &params);
        //    static bool is_redirect_url (const pfc::string8 &url) { return url.find_first (g_redirect_url) == 0; }

        //    static bool call (const char *method, const char *params, Json::Value &out);
        //};
    }
}
#endif