#ifndef _FOO_VK_UPLOADER_VK_AUTH_H_
#define _FOO_VK_UPLOADER_VK_AUTH_H_

namespace vk_uploader
{
    // Authorization process for desktop apps: http://vkontakte.ru/developers.php?oid=-1&p=Авторизация_клиентских_приложений

    class NOVTABLE vk_auth_manager : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(vk_auth_manager)
    public:
        // All this functions may throw exception_aborted then user cancels authorization (by pressing "Cancel" button in dialog)

        virtual pfc::string8_fast get_user_id () = 0;
        virtual pfc::string8_fast get_access_token () = 0;

        virtual void relogin_user () = 0;
    };

    // {911ED77D-3820-4B8E-BE4F-6EF30029670B}
    __declspec(selectany) const GUID vk_auth_manager::class_guid =
    { 0x911ED77D, 0x3820, 0x4B8E, { 0xBE, 0x4F, 0x6E, 0xF3, 0x00, 0x29, 0x67, 0x0B } };
}
typedef static_api_ptr_t<vk_uploader::vk_auth_manager> get_auth_manager;
#endif