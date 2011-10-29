#include "stdafx.h"

#include "vk_api.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class api_profider_imp : public api_profider
        {            
            class api_call_thread : public pfc::thread
            {
                const char *m_api_name;
                url_params m_api_params;
                service_ptr_t<api_callback> m_api_callback;

                void threadProc () override
                {
                    response_json_ptr result = static_api_ptr_t<api_invoker>()->invoke (m_api_name, m_api_params);
                    m_api_callback->on_request_done (m_api_name, result);
                }
            public:
                api_call_thread (const char *p_api_name, params_cref p_params, service_ptr_t<api_callback> &p_callback)
                    : m_api_name (p_api_name), m_api_params (p_params), m_api_callback (p_callback)
                { start (); }
                ~api_call_thread () { waitTillDone (); }
            };
            

            response_json_ptr call_api (const char *p_api_name, params_cref p_params) override
            {
                return static_api_ptr_t<api_invoker>()->invoke (p_api_name, p_params);
            }
            
            void call_api_async (const char *p_api_name, params_cref p_params, service_ptr_t<api_callback> p_callback) override
            {
                api_call_thread (p_api_name, p_params, p_callback);
            }
        };
        static service_factory_single_t<api_profider_imp> g_api_profider_factory;
    }
}