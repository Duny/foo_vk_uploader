#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#include "vk_auth.h"
#include "helpers.h"

#define VK_UPLOADER_APP_ID "2632594"

#define VK_COM_BLANK_URL "http://api.vk.com/blank.html"
#define VK_COM_LOGIN_URL "http://api.vk.com/oauth/authorize?display=popup&scope=audio,wall&response_type=token&client_id="VK_UPLOADER_APP_ID"&redirect_uri="VK_COM_BLANK_URL
#define VK_COM_LOGOUT_URL "http://api.vk.com/oauth/logout?client_id="VK_UPLOADER_APP_ID

namespace vk_uploader
{
    namespace vk_api
    {   
        class NOVTABLE api_profider : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(api_profider)
        public:
            enum { max_api_calls_per_second = 3 };

            virtual response_json_ptr call_api (const char *p_api_name, params_cref p_params) = 0;
            inline response_json_ptr call_api (const char *p_api_name) { return call_api (p_api_name, url_params ()); }

            virtual pfc::string8_fast file_upload (const char *p_url, const char *p_file, abort_callback &p_abort) = 0;
        };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID api_profider::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
    }
}

typedef static_api_ptr_t<vk_uploader::vk_api::api_profider> get_api_provider;

namespace vk_uploader
{
    class api_imp_base
    {
    protected:
        pfc::string8 m_error;
    public:
        bool is_valid () const { return m_error.is_empty (); }
        const pfc::string8 &get_error () const { return m_error; }
    };

    class api_audio_getAlbums : public api_imp_base, public pfc::list_t<t_audio_album_info>
    {
    public:
        // reads a list of user albums (from vk.com profile)
        // represents as a list of album_name=>album_id pairs
        api_audio_getAlbums () {
            response_json_ptr result = get_api_provider ()->call_api ("audio.getAlbums", url_params ("count", "100"));
                if (result.is_valid ()) {
                for (t_size n = result->size (), i = 1; i < n; ++i) {
                    if (!result[i].isMember ("title") || !result[i].isMember ("album_id")) {
                        m_error = "not enough fields in response from audio.getAlbums";
                        break;
                    }
                        
                    this->add_item (std::make_pair (result[i]["title"].asCString (), result[i]["album_id"].asUInt ()));
                }
            }
            else
                m_error = result.get_error_code ();
        }
    };

    class api_audio_addAlbum : public api_imp_base
    {
        t_album_id m_id;
    public:
        api_audio_addAlbum (const pfc::string8 &title) {
            response_json_ptr result = get_api_provider ()->call_api ("audio.addAlbum", url_params ("title", title));
            if (result.is_valid ()) {
                if (!result->isMember ("album_id")) m_error = "no 'album_id' field in response from audio.addAlbum";
                else m_id = result["album_id"].asUInt ();
            }
            else
                m_error = result.get_error_code ();
        }

        t_album_id get_album_id () const { return m_id; }
    };

    class api_audio_deleteAlbum : public api_imp_base
    {
    public:
        api_audio_deleteAlbum (t_album_id id)
        {
            response_json_ptr result = get_api_provider ()->call_api ("audio.deleteAlbum", url_params ("album_id", pfc::string_formatter () << id));
            if (!result.is_valid () || !result->asBool ())
                m_error = result.get_error_code ();
        }
    };

    class api_audio_moveToAlbum : public api_imp_base
    {
    public:
        api_audio_moveToAlbum (const pfc::list_t<t_audio_id> &audio_ids, t_album_id album_id)
        {
            pfc::string_formatter aids;
            for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) aids << audio_ids[i] << ",";

            if (!aids.is_empty ()) {
                aids.truncate (aids.length () - 1);

                url_params params ("aids", aids);
                params["album_id"] = pfc::string_formatter () << album_id;
                    
                response_json_ptr result = get_api_provider ()->call_api ("audio.moveToAlbum", params);
                if (!result.is_valid () || !result->asBool ())
                    m_error = result.get_error_code ();
            }
        }
    };

    class api_audio_getUploadServer
    {
        pfc::string8 m_url;
    public:
        api_audio_getUploadServer () {
            response_json_ptr result = get_api_provider ()->call_api ("audio.getUploadServer");
            result.assert_valid ();
            if (!result->isMember ("upload_url")) throw std::exception ("no 'upload_url' field in response from audio.getUploadServer");
            m_url = result["upload_url"].asCString ();
        }

        operator const char* () const { return m_url.get_ptr (); }
    };

    class api_audio_save
    {
        t_audio_id m_id; // id of newly upload mp3 file
    public:
        // answer from vk.com server after file uploading is finished (with post request)
        api_audio_save (const pfc::string8 &answer) {
            response_json_ptr result (answer);
            result.assert_valid ();
                
            if (!result->isMember ("server")) throw std::exception ("no 'server' field in response from file upload");
            if (!result->isMember ("audio")) throw std::exception ("no 'audio' field in response from file upload");
            if (!result->isMember ("hash")) throw std::exception ("no 'hash' field in response from file upload");

            url_params params;
            params["server"] = result["server"].asCString ();
            params["audio"] = result["audio"].asCString ();
            params["hash"] = result["hash"].asCString ();

            result = get_api_provider ()->call_api ("audio.save", params);
            result.assert_valid ();
            if (!result->isMember ("aid")) throw std::exception ("no 'aid' field in response from audio.save");
            m_id = result["aid"].asUInt ();
        }

        t_audio_id get_id () const { return m_id; }
    };

    class api_wall_post : public api_imp_base
    {
    public:
        api_wall_post (const pfc::string8 &msg, const pfc::list_t<t_audio_id> &audio_ids) {
            url_params params;

            if (!msg.is_empty ()) params["message"] = msg;

            pfc::string8_fast attachments;
            const char *user_id = get_auth_manager ()->get_user_id ();
            for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) attachments << "audio" << user_id << "_" << audio_ids[i] << ",";
            if (!attachments.is_empty ()) {
                attachments.truncate (attachments.length () - 1); // remove last ',' symbol
                params["attachment"] = attachments;
            }

            if (params.get_count ()) {
                response_json_ptr result = get_api_provider ()->call_api ("wall.post", params);
                if (!result.is_valid ())
                    m_error = result.get_error_code ();
            }
        }
    };
}
#endif