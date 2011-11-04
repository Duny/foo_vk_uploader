#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#define VK_UPLOADER_APP_ID "2632594"

#define VK_COM_BLANK_URL "http://api.vk.com/blank.html"
#define VK_COM_LOGIN_URL "http://api.vk.com/oauth/authorize?display=popup&scope=audio,wall&response_type=token&client_id="VK_UPLOADER_APP_ID"&redirect_uri="VK_COM_BLANK_URL
#define VK_COM_LOGOUT_URL "http://api.vk.com/oauth/logout?client_id="VK_UPLOADER_APP_ID

namespace vk_uploader
{  
    class NOVTABLE vk_api_profider : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(vk_api_profider)
    public:
        enum { max_api_calls_per_second = 3 };

        virtual response_json_ptr invoke (const char *p_api_name, params_cref p_params, abort_callback &p_abort) = 0;
        inline response_json_ptr invoke (const char *p_api_name, abort_callback &p_abort) { return invoke (p_api_name, url_params (), p_abort); }

        virtual pfc::string8_fast file_upload (const char *p_url, const char *p_file, abort_callback &p_abort) = 0;
    };

    // {415971BA-5773-4843-9D18-09F28074F5F7}
    __declspec(selectany) const GUID vk_api_profider::class_guid = 
    { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };

    typedef static_api_ptr_t<vk_api_profider> get_api_provider;


    // helper implementations of some apis
    class api_audio_getAlbums : public pfc::list_t<audio_album_info>
    {
    public:
        // reads a list of user albums (from vk.com profile)
        // represents as a list of album_name=>album_id pairs
        api_audio_getAlbums (abort_callback &p_abort) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.getAlbums", url_params ("count", "100"), p_abort);
            for (t_size n = result->size (), i = 1; i < n; ++i) {
                if (!result[i].isMember ("title") || !result[i].isMember ("album_id"))
                    throw pfc::exception ("not enough fields in response from audio.getAlbums");
                        
                this->add_item (std::make_pair (result[i]["title"].asCString (), result[i]["album_id"].asUInt ()));
            }
        }
    };

    class api_audio_addAlbum
    {
        t_vk_album_id m_id;
    public:
        api_audio_addAlbum (const pfc::string8 &title, abort_callback &p_abort) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.addAlbum", url_params ("title", title), p_abort);
            if (!result->isMember ("album_id"))
                throw pfc::exception ("no 'album_id' field in response from audio.addAlbum");
            else
                m_id = result["album_id"].asUInt ();
        }

        t_vk_album_id get_album_id () const { return m_id; }
    };

    class api_audio_deleteAlbum
    {
    public:
        api_audio_deleteAlbum (t_vk_album_id id, abort_callback &p_abort) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.deleteAlbum", url_params ("album_id", pfc::string_formatter () << id), p_abort);
            ///!!!! TEST ME: test error codes from vk server ("Application is disabled. Enable your application or use test mode.", etc)
            /*if (!result.is_valid () || !result->asBool ())
                m_error = result.get_error_code ();*/
        }
    };

    class api_audio_moveToAlbum
    {
    public:
        api_audio_moveToAlbum (
            const pfc::list_t<t_vk_audio_id> &audio_ids, t_vk_album_id album_id, abort_callback &p_abort) {
            pfc::string_formatter aids;
            for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) aids << audio_ids[i] << ",";

            if (!aids.is_empty ()) {
                aids.truncate (aids.length () - 1);

                url_params params ("aids", aids);
                params["album_id"] = pfc::string_formatter () << album_id;
                    
                response_json_ptr result = get_api_provider ()->invoke ("audio.moveToAlbum", params, p_abort);
                ///!!!! TEST ME: test error codes from vk server ("Application is disabled. Enable your application or use test mode.", etc)
                /*if (!result.is_valid () || !result->asBool ())
                    m_error = result.get_error_code ();*/
            }
        }
    };

    class api_audio_getUploadServer
    {
        pfc::string8 m_url;
    public:
        api_audio_getUploadServer (abort_callback &p_abort) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.getUploadServer", p_abort);
            if (!result->isMember ("upload_url"))
                throw pfc::exception ("no 'upload_url' field in response from audio.getUploadServer");
            m_url = result["upload_url"].asCString ();
        }

        operator const char* () const { return m_url.get_ptr (); }
    };

    class api_audio_save
    {
        t_vk_audio_id m_id; // id of newly upload mp3 file
    public:
        // answer from vk.com server after file uploading is finished (with post request)
        api_audio_save (const pfc::string8 &answer, abort_callback &p_abort) {
            response_json_ptr result (answer);
            if (!result->isMember ("server") || !result->isMember ("audio") || !result->isMember ("hash"))
                throw pfc::exception ("not enough fields in response from file upload");

            url_params params;
            params["server"] = result["server"].asCString ();
            params["audio"] = result["audio"].asCString ();
            params["hash"] = result["hash"].asCString ();

            result = get_api_provider ()->invoke ("audio.save", params, p_abort);
            if (!result->isMember ("aid"))
                throw std::exception ("no 'aid' field in response from audio.save");
            m_id = result["aid"].asUInt ();
        }

        t_vk_audio_id get_id () const { return m_id; }
    };

    class api_wall_post
    {
    public:
        api_wall_post (const pfc::string8 &msg, const pfc::list_t<t_vk_audio_id> &audio_ids, abort_callback &p_abort) {
            url_params params;

            if (!msg.is_empty ()) params["message"] = msg;

            pfc::string8_fast attachments;
            const char *user_id = get_auth_manager ()->get_user_id ();
            for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) attachments << "audio" << user_id << "_" << audio_ids[i] << ",";
            if (!attachments.is_empty ()) {
                attachments.truncate (attachments.length () - 1); // remove last ',' symbol
                params["attachment"] = attachments;
            }

            if (params.get_count ())
                get_api_provider ()->invoke ("wall.post", params, p_abort);
        }
    };
}
#endif