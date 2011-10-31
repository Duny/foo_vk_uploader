#ifndef _FOO_VK_UPLOADER_VK_API_H_
#define _FOO_VK_UPLOADER_VK_API_H_

#include "vk_auth.h"
#include "helpers.h"

namespace vk_uploader
{
    namespace vk_api
    {   
        PFC_DECLARE_EXCEPTION (exception_auth_failed, pfc::exception, "Authorization failed");

        __declspec(selectany) extern const char *app_id = "2632594";

        class NOVTABLE api_callback : public service_base
        {
        public:
            virtual void on_request_done (const pfc::string8 &p_api_name, const response_json_ptr &p_result) = 0;

            FB2K_MAKE_SERVICE_INTERFACE(api_callback, service_base);
        };


        class NOVTABLE api_profider : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(api_profider)
        public:
            enum { max_api_calls_per_second = 3 };

            // makes synchronous api call
            virtual response_json_ptr call_api (const char *p_api_name, params_cref p_params) = 0;
            inline response_json_ptr call_api (const char *p_api_name) { return call_api (p_api_name, url_params ()); }

            virtual pfc::string8_fast file_upload (const char *p_url, const char *p_file, abort_callback &p_abort) = 0;

            // make asynchronous call
            virtual void call_api_async (const char *p_api_name, params_cref p_params, service_ptr_t<api_callback> p_callback) = 0;
            inline void call_api_async (const char *p_api_name, service_ptr_t<api_callback> p_callback) { call_api_async (p_api_name, url_params (), p_callback); }
        };


        class api_base
        {
        protected:
            pfc::string8 m_error;
        public:
            bool is_valid () const { return m_error.is_empty (); }
            const pfc::string8 &get_error () const { return m_error; }
        };

        class api_audio_getAlbums : public api_base, public pfc::map_t<pfc::string8, t_album_id>
        {
        public:
            // reads a list of user albums (from vk.com profile)
            // represents as a map of album_name=>album_id pairs
            api_audio_getAlbums () {
                response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.getAlbums", url_params ("count", "100"));
                 if (result.is_valid ()) {
                    for (t_size n = result->size (), i = 1; i < n; ++i) {
                        if (!(result[i].isMember ("title"))) {
                            m_error = "no 'title' field in response from audio.getAlbums";
                            break;
                        }
                        else if (!result[i].isMember ("album_id")) {
                            m_error = "no 'album_id' field in response from audio.getAlbums";
                            break;
                        }
                        set (result[i]["title"].asCString (), result[i]["album_id"].asUInt ());
                    }
                }
                else
                    m_error = result.get_error_code ();
            }

            bool have (const char *p_name) const { return have_item (pfc::string8 (p_name)); }
            t_album_id &operator [] (const char *p_name) { return find_or_add (pfc::string8 (p_name)); }
        };

        class api_audio_addAlbum : public api_base
        {
            t_album_id m_id;
        public:
            api_audio_addAlbum (const pfc::string8 &title) {
                response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.addAlbum", url_params ("title", title));
                if (result.is_valid ()) {
                    if (!result->isMember ("album_id")) m_error = "no 'album_id' field in response from audio.addAlbum";
                    else m_id = result["album_id"].asUInt ();
                }
                else
                    m_error = result.get_error_code ();
            }

            t_album_id get_album_id () const { return m_id; }
        };

        class api_audio_deleteAlbum : public api_base
        {
        public:
            api_audio_deleteAlbum (t_album_id id)
            {
                response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.deleteAlbum", url_params ("album_id", pfc::string_formatter () << id));
                if (!result.is_valid () || !result->asBool ())
                    m_error = result.get_error_code ();
            }
        };

        class api_audio_moveToAlbum : public api_base
        {
        public:
            api_audio_moveToAlbum (const pfc::list_t<t_audio_id> &audio_ids, t_album_id album_id)
            {
                pfc::string8_fast aids;
                for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) aids << audio_ids[i] << ",";

                if (audio_ids.get_count ()) {
                    aids.truncate (aids.length () - 1);

                    url_params params ("aids", aids);
                    params["album_id"] = pfc::string_formatter () << album_id;
                    
                    response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.moveToAlbum", params);
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
                response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.getUploadServer");
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
            // answer from vk.com server after file upload (with post)
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

                result = static_api_ptr_t<vk_api::api_profider>()->call_api ("audio.save", params);
                result.assert_valid ();
                if (!result->isMember ("aid")) throw std::exception ("no 'aid' field in response from audio.save");
                m_id = result["aid"].asUInt ();
            }

            t_audio_id get_id () const { return m_id; }
        };

        class api_wall_post : public api_base
        {
        public:
            api_wall_post (const pfc::string8 &msg, const pfc::list_t<t_audio_id> &audio_ids) {
                url_params params;

                if (!msg.is_empty ())
                    params["message"] = msg;

                pfc::string8_fast attachments;
                const char *user_id = static_api_ptr_t<vk_api::authorization>()->get_user_id ();
                for (t_size i = 0, n = audio_ids.get_size (); i < n; ++i) attachments << "audio" << user_id << "_" << audio_ids[i] << ",";
                if (!attachments.is_empty ()) {
                    attachments.truncate (attachments.length () - 1); // remove last ',' symbol
                    params["attachments"] = attachments;
                }

                if (params.get_count ()) {
                    response_json_ptr result = static_api_ptr_t<vk_api::api_profider>()->call_api ("wall.post");
                    if (!result.is_valid ())
                        m_error = result.get_error_code ();
                }
            }
        };


        // {1EABF92D-EA43-4DCC-BCD0-B2B4C9BC003C}
        __declspec(selectany) const GUID api_callback::class_guid = 
        { 0x1eabf92d, 0xea43, 0x4dcc, { 0xbc, 0xd0, 0xb2, 0xb4, 0xc9, 0xbc, 0x0, 0x3c } };

        // {415971BA-5773-4843-9D18-09F28074F5F7}
        __declspec(selectany) const GUID api_profider::class_guid = 
        { 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };
    }

    typedef static_api_ptr_t<vk_api::api_profider> get_api_provider;
}

#include "vk_api_invoker.h"

#endif