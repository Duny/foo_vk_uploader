#ifndef _FOO_VK_UPLOADER_VK_AUTH_H_
#define _FOO_VK_UPLOADER_VK_AUTH_H_

namespace vk_uploader
{
    namespace vk_api
    {
        // Authorization process for desktop apps: http://vkontakte.ru/developers.php?oid=-1&p=Авторизация_клиентских_приложений

        class NOVTABLE authorization : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(authorization)
        public:
            virtual const char *get_user_id () = 0;
            virtual const char *get_access_token () = 0;

            virtual void relogin_user () = 0;
        };

        // {911ED77D-3820-4B8E-BE4F-6EF30029670B}
        __declspec(selectany) const GUID authorization::class_guid =
        { 0x911ED77D, 0x3820, 0x4B8E, { 0xBE, 0x4F, 0x6E, 0xF3, 0x00, 0x29, 0x67, 0x0B } };
    }
}
#endif