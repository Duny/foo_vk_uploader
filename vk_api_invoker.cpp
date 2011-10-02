#include "stdafx.h"

#include "vk_api_invoker.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class api_invoker_imp : public api_invoker
        {
            static DWORD m_first_call_time; // time then was made first call
            static DWORD m_last_call_time; // time of the most resent call
            static t_size m_call_count; // number of calls made since m_first_call_time
            static critical_section m_section;

            result_t invoke (const char *p_api_name, params_cref p_params) override
            {
                return result_t ();
            }
        };

        DWORD api_invoker_imp::m_first_call_time = 0;
        DWORD api_invoker_imp::m_last_call_time = 0;
        t_size api_invoker_imp::m_call_count = 0;
        critical_section api_invoker_imp::m_section;

        static service_factory_single_t<api_invoker_imp> g_api_invoker_factory;
    }
}