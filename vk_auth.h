#ifndef _FOO_VK_UPLOADER_VK_AUTH_H_
#define _FOO_VK_UPLOADER_VK_AUTH_H_

namespace vk_uploader
{
    namespace vk_api
    {
        struct auth_info
        {
            pfc::string8 m_user_id;
            pfc::string8 m_secret;
            pfc::string8 m_sid; // session id
        };

        class NOVTABLE authorization : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(authorization)
        public:
            const auth_info & get_info () const = 0;
        };

        // {911ED77D-3820-4B8E-BE4F-6EF30029670B}
        __declspec(selectany) const GUID authorization::class_guid =
        { 0x911ED77D, 0x3820, 0x4B8E, { 0xBE, 0x4F, 0x6E, 0xF3, 0x00, 0x29, 0x67, 0x0B } };
    }
}
#endif