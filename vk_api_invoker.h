#ifndef _FOO_VK_UPLOADER_VK_API_INVOKER_H_
#define _FOO_VK_UPLOADER_VK_API_INVOKER_H_

namespace vk_uploader
{
    namespace vk_api
    {
        class NOVTABLE api_invoker : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(api_invoker)
        public:
            virtual value_t invoke (const char *p_api_name, params_cref p_params) = 0;
        };

        // {14E354B1-45C0-412F-9B2E-5D2296A3D691}
        __declspec(selectany) const GUID api_invoker::class_guid = 
        { 0x14e354b1, 0x45c0, 0x412f, { 0x9b, 0x2e, 0x5d, 0x22, 0x96, 0xa3, 0xd6, 0x91 } };
    }
}
#endif