#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

namespace vk_uploader
{
    namespace vk_api
    {   
        PFC_DECLARE_EXCEPTION (exception_auth_failed, pfc::exception, "Authorization failed");

        class string_constants
        {
        public:
            static const pfc::string8 app_id;

            // Desktop authorization http://vkontakte.ru/developers.php?id=-1_21239305&s=1            
            static const pfc::string8 redirect_url_ok, redirect_url_err;
            static const pfc::string8 auth_url;

            static const pfc::string8 api_frontend_url;
        };

        typedef pfc::list_t<std::pair<pfc::string8, pfc::string8>> params_t;
        typedef params_t const & params_cref;
        
        class api_callback
        {
        public:
            virtual void on_done (const value_t &p_result) = 0;
            virtual void on_error (const pfc::string8 &p_message) = 0;
        };

        class NOVTABLE profider : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(profider)
        public:
            enum { max_api_calls_per_second = 3 };

            // makes synchronous api call 
            virtual value_t call_api (const char *p_api_name, params_cref p_params) = 0;
            inline value_t call_api (const char *p_api_name) { return call_api (p_api_name, params_t ()); }

            // make asynchronous call
            virtual void call_api_async (const char *p_api_name, params_cref p_params, const api_callback &p_callback) = 0;
            inline void call_api_async (const char *p_api_name, const api_callback &p_callback) { call_api_async (p_api_name, params_t (), p_callback); }
        };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID profider::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
    }
}
#endif