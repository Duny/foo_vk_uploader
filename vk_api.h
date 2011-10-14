#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#include "helpers.h"

namespace vk_uploader
{
    namespace vk_api
    {   
        PFC_DECLARE_EXCEPTION (exception_auth_failed, pfc::exception, "Authorization failed");

        __declspec(selectany) extern const char *app_id = "2632594";

        class api_callback
        {
        public:
            virtual void on_request_done (const pfc::string8 &p_api_name, const response_json &p_result) = 0;
        };

        class NOVTABLE profider : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(profider)
        public:
            enum { max_api_calls_per_second = 3 };

            // makes synchronous api call
            virtual response_json call_api (const char *p_api_name, params_cref p_params) = 0;
            inline response_json call_api (const char *p_api_name) { return call_api (p_api_name, url_params ()); }

            // make asynchronous call
            //virtual void call_api_async (const char *p_api_name, params_cref p_params, api_callback &p_callback) = 0;
            //inline void call_api_async (const char *p_api_name, api_callback &p_callback) { call_api_async (p_api_name, url_params (), p_callback); }
        };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID profider::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
    }
}

#include "vk_api_invoker.h"
#include "vk_auth.h"

#endif