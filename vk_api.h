#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#define VK_UPLOADER_APP_ID "2632594"

#define VK_COM_BLANK_URL "http://api.vk.com/blank.html"
#define VK_COM_LOGIN_URL "http://api.vk.com/oauth/authorize?display=popup&scope=audio,wall&response_type=token&client_id="VK_UPLOADER_APP_ID"&redirect_uri="VK_COM_BLANK_URL
#define VK_COM_LOGOUT_URL "http://api.vk.com/oauth/logout?client_id="VK_UPLOADER_APP_ID


// Documentation:
//
// Authorization process :
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%90%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F_%D1%81%D0%B0%D0%B9%D1%82%D0%BE%D0%B2

// Calling api methods :
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%92%D1%8B%D0%BF%D0%BE%D0%BB%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5_%D0%B7%D0%B0%D0%BF%D1%80%D0%BE%D1%81%D0%BE%D0%B2_%D0%BA_API

// List of all api methods :
// http://vkontakte.ru/developers.php?o=-1&p=%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5_%D0%BC%D0%B5%D1%82%D0%BE%D0%B4%D0%BE%D0%B2_API&s=0



namespace vk_uploader
{
    typedef pfc::rcptr_t<pfc::array_t<t_uint8>> membuf_ptr;
    
    class NOVTABLE vk_api_provider : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(vk_api_provider)
    public:
        enum { max_api_calls_per_second = 10 }; // Not sure if this is right for desktop apps

        virtual response_json_ptr invoke (const char *p_api_name, const url_parameters & p_params, abort_callback &p_abort) = 0;
        response_json_ptr invoke (const char *p_api_name, abort_callback &p_abort) { return invoke (p_api_name, url_parameters (), p_abort); }

        // Returns raw answer from vk server after file has been uploaded
        virtual membuf_ptr upload_audio_file (const char *p_url, const char *p_file_path, abort_callback &p_abort) = 0;
    };

    typedef static_api_ptr_t<vk_api_provider> get_api_provider;


    // {415971BA-5773-4843-9D18-09F28074F5F7}
    __declspec(selectany) const GUID vk_api_provider::class_guid = 
    { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
}

#include "api_helpers.h"
#endif