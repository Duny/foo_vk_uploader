#ifndef _FOO_VK_UPLOADER_LOGIN_DLG_H_
#define _FOO_VK_UPLOADER_LOGIN_DLG_H_
#endif

namespace vk_uploader
{
    class NOVTABLE login_dialog : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(login_dialog)
    public:
        virtual void show () = 0;
    };


    // {BAF4E1F8-A823-4E5F-AFA4-993D5D274362}
    __declspec(selectany) const GUID login_dialog::class_guid =   
    { 0xbaf4e1f8, 0xa823, 0x4e5f, { 0xaf, 0xa4, 0x99, 0x3d, 0x5d, 0x27, 0x43, 0x62 } };
}