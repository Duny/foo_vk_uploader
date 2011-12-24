#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#define VK_UPLOADER_APP_ID "2632594"

#define VK_COM_BLANK_URL "http://api.vk.com/blank.html"
#define VK_COM_LOGIN_URL "http://api.vk.com/oauth/authorize?display=popup&scope=audio,wall&response_type=token&client_id="VK_UPLOADER_APP_ID"&redirect_uri="VK_COM_BLANK_URL
#define VK_COM_LOGOUT_URL "http://api.vk.com/oauth/logout?client_id="VK_UPLOADER_APP_ID

// authorization process :
http://vkontakte.ru/developers.php?oid=-1&p=%D0%90%D0%B2%D1%82%D0%BE%D1%80%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F_%D1%81%D0%B0%D0%B9%D1%82%D0%BE%D0%B2

// calling api methods :
// http://vkontakte.ru/developers.php?oid=-1&p=%D0%92%D1%8B%D0%BF%D0%BE%D0%BB%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5_%D0%B7%D0%B0%D0%BF%D1%80%D0%BE%D1%81%D0%BE%D0%B2_%D0%BA_API

// list of api methods :
// http://vkontakte.ru/developers.php?o=-1&p=%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5_%D0%BC%D0%B5%D1%82%D0%BE%D0%B4%D0%BE%D0%B2_API&s=0

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
        pfc::string8 m_title;
    public:
        api_audio_addAlbum (const pfc::string_base &title, abort_callback &p_abort) : m_title (title) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.addAlbum", url_params ("title", title), p_abort);
            if (!result->isMember ("album_id"))
                throw pfc::exception ("no 'album_id' field in response from audio.addAlbum");
            else
                m_id = result["album_id"].asUInt ();
        }

        inline operator audio_album_info () const  { return boost::make_tuple (m_title, m_id); }
    };

    class api_audio_editAlbum
    {
    public:
        api_audio_editAlbum (t_vk_album_id album_id, const pfc::string_base & p_new_title, abort_callback &p_abort) {
            url_params params ("title", p_new_title);
            params["album_id"] = pfc::string_formatter () << album_id;
            get_api_provider ()->invoke ("audio.editAlbum", params, p_abort);
        }
    };

    class api_audio_deleteAlbum
    {
    public:
        api_audio_deleteAlbum (t_vk_album_id id, abort_callback & p_abort) {
            get_api_provider ()->invoke ("audio.deleteAlbum", url_params ("album_id", pfc::string_formatter () << id), p_abort);
            ///!!!! TEST ME: test error codes from vk server ("Application is disabled. Enable your application or use test mode.", etc)
            /*if (!result.is_valid () || !result->asBool ())
                m_error = result.get_error_code ();*/
        }
    };

    class api_audio_moveToAlbum
    {
        void move (const pfc::list_t<t_vk_audio_id> & p_audio_ids, t_vk_album_id album_id, abort_callback & p_abort) {
            pfc::string_formatter aids;
            for (t_size i = 0, n = p_audio_ids.get_size (); i < n; ++i) aids << p_audio_ids[i] << ",";

            if (!aids.is_empty ()) {
                aids.truncate (aids.length () - 1);

                url_params params ("aids", aids);
                params["album_id"] = pfc::string_formatter () << album_id;
                    
                get_api_provider ()->invoke ("audio.moveToAlbum", params, p_abort);
                ///!!!! TEST ME: test error codes from vk server ("Application is disabled. Enable your application or use test mode.", etc)
                /*if (!result.is_valid () || !result->asBool ())
                    m_error = result.get_error_code ();*/
            }
        }

    public:
        api_audio_moveToAlbum (
            const pfc::list_t<t_vk_audio_id> & p_audio_ids, t_vk_album_id album_id, abort_callback & p_abort) {
            move (p_audio_ids, album_id, p_abort);
        }

        api_audio_moveToAlbum (t_vk_audio_id audio_id, t_vk_album_id album_id, abort_callback & p_abort) {
            pfc::list_t<t_vk_audio_id> aids;
            aids.add_item (audio_id);
            move (aids, album_id, p_abort);
        }
    };

    class api_audio_getUploadServer
    {
        pfc::string8 m_url;
    public:
        api_audio_getUploadServer (abort_callback & p_abort) {
            response_json_ptr result = get_api_provider ()->invoke ("audio.getUploadServer", p_abort);
            if (!result->isMember ("upload_url"))
                throw pfc::exception ("no 'upload_url' field in response from audio.getUploadServer");
            m_url = result["upload_url"].asCString ();
        }

        operator const char* () const { return m_url.get_ptr (); }
    };

    class api_audio_save
    {
        t_vk_audio_id m_id; // id of newly uploaded mp3 file
    public:
        // answer from vk.com server after file uploading is finished (with post request)
        api_audio_save (const pfc::string_base & p_answer, abort_callback & p_abort) {
            response_json_ptr result (p_answer);
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
        api_wall_post (const pfc::string_base & p_msg, const pfc::list_t<t_vk_audio_id> & p_audio_ids, abort_callback & p_abort) {
            url_params params;

            if (!p_msg.is_empty ())
                params["message"] = p_msg;

            pfc::string8_fast attachments;
            pfc::string8_fast user_id = get_auth_manager ()->get_user_id ();
            for (t_size i = 0, n = p_audio_ids.get_size (); i < n; ++i)
                attachments << "audio" << user_id << "_" << p_audio_ids[i] << ",";
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