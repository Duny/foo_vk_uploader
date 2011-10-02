#ifndef _FOO_VK_UPLOADER_VK_API_INVOKER_H_
#define _FOO_VK_UPLOADER_VK_API_INVOKER_H_

#include "vk_api.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class NOVTABLE api_invoker : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(api_invoker)
        public:
            virtual result_t invoke (const char *p_api_name, params_cref p_params) = 0;
        };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID api_invoker::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
    }
}
#endif