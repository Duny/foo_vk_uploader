#include "stdafx.h"

#include "vk_auth.h"
#include "vk_api.h"


namespace vk_uploader
{
    cfg_objList<audio_album_info> user_album_list::m_cfg_album_list (create_guid (0x7a5b3e69, 0xe2b0, 0x4bca, 0x96, 0xca, 0x3c, 0x4b, 0x52, 0x21, 0xd1, 0x86));

    void vk_api::audio::albums::get::run (abort_callback & p_abort)
    {
        response_json_ptr result = get_api_provider ()->invoke ("audio.getAlbums", list_of<name_value_pair> ("count", "100"), p_abort);
        for (t_size n = result->size (), i = 1; i < n; ++i) {
            Json::Value & al = result[i];
            if (al.isMember ("title") && al.isMember ("album_id"))
                add_item (make_tuple (al["title"].asCString (), al["album_id"].asUInt ()));
            else
                throw pfc::exception ("not enough fields in response from audio.getAlbums");
        }
    }

    void vk_api::audio::albums::add::run (abort_callback & p_abort)
    {
        response_json_ptr result = get_api_provider ()->invoke ("audio.addAlbum", list_of<name_value_pair> ("title", m_new_album.get<0>()), p_abort);
        if (result->isMember ("album_id"))
            m_new_album.get<1>() = result["album_id"].asUInt ();
        else
            throw pfc::exception ("no 'album_id' field in response from audio.addAlbum");
    }

    void vk_api::audio::albums::del::run (abort_callback & p_abort)
    {
        get_api_provider ()->invoke ("audio.deleteAlbum", list_of<name_value_pair> ("album_id", pfc::string_formatter () << m_id_to_delete), p_abort);
        ///!!!! TEST ME: test error codes from vk server ("Application is disabled. Enable your application or use test mode.", etc)
    }

    void vk_api::audio::albums::ren::run (abort_callback & p_abort)
    {
        get_api_provider ()->invoke ("audio.editAlbum", list_of<name_value_pair> ("title", m_new_title)("album_id", pfc::string_formatter () << m_album_id), p_abort);
    }

    void vk_api::audio::move_to_album::run (abort_callback & p_abort)
    {
        get_api_provider ()->invoke ("audio.moveToAlbum", list_of<name_value_pair> ("aids", pfc::string_formatter () << m_audio_id)("album_id", pfc::string_formatter () << m_album_id), p_abort);
    }

    void vk_api::audio::get_upload_server::run (abort_callback & p_abort)
    {
        response_json_ptr result = get_api_provider ()->invoke ("audio.getUploadServer", p_abort);
        if (result->isMember ("upload_url"))
            m_url = result["upload_url"].asCString ();
        else
            throw pfc::exception ("no 'upload_url' field in response from audio.getUploadServer");
    }

    void vk_api::audio::save::run (abort_callback & p_abort)
    {
        if (m_result->isMember ("server") && m_result->isMember ("audio") && m_result->isMember ("hash")) {
            m_result = get_api_provider ()->invoke ("audio.save", 
                list_of<name_value_pair> ("server", m_result["server"].asCString ())("audio", m_result["audio"].asCString ()) ("hash", m_result["hash"].asCString ()), p_abort);
            if (m_result->isMember ("aid"))
                m_id = m_result["aid"].asUInt ();
            else
                throw pfc::exception ("no 'aid' field in response from audio.save");
        }
        else
            throw pfc::exception (pfc::string_formatter () << "invalid response from file upload:\n" << m_result->toStyledString ().c_str ());
    }

    void vk_api::audio::edit::run (abort_callback & p_abort)
    {
        static_api_ptr_t<titleformat_compiler> p_compiler;
        service_ptr_t<titleformat_object> p_object;
        p_compiler->compile (p_object, "%artist%||%title%");

        pfc::string8 str;
        m_track->format_title (nullptr, str, p_object, &titleformat_text_filter_nontext_chars ());
        auto sep_pos = str.find_first ("||");

        get_api_provider ()->invoke ("audio.edit", 
            list_of<name_value_pair> 
            ("aid", pfc::string_formatter () << m_aid)
            ("oid", get_auth_manager ()->get_user_id ())
            ("artist", pfc::string_part (str.get_ptr (), sep_pos))
            ("title", pfc::string_part (str.get_ptr () + sep_pos + 2, str.get_length () - sep_pos - 2)),
            p_abort);
    }

    void vk_api::wall::post::run (abort_callback & p_abort)
    {
        url_parameters params;

        if (!m_message.is_empty ())
            params.push_back (make_pair ("message", m_message));

        pfc::string8 attachments;
        pfc::string8 user_id = get_auth_manager ()->get_user_id ();
        for (t_size i = 0, n = m_audio_ids.get_size (); i < n; ++i)
            attachments << "audio" << user_id << "_" << m_audio_ids[i] << ",";
        if (!attachments.is_empty ()) {
            attachments.truncate (attachments.length () - 1); // remove last ',' symbol
            params.push_back (make_pair ("attachment", attachments));
        }

        if (params.size ())
            get_api_provider ()->invoke ("wall.post", params, p_abort);
    }

    bool user_album_list::reload ()
    {
        reset_error ();

        vk_api::audio::albums::get method_get_albums;
        bool ok = method_get_albums.call ();
        if (ok)
            m_cfg_album_list = method_get_albums;
        else
            get_method_error_code (method_get_albums);
        return ok;
    }

    bool user_album_list::add_item (const pfc::string_base & p_title)
    {
        reset_error ();

        vk_api::audio::albums::add method_add_album (p_title);
        bool ok = method_add_album.call ();
        if (ok)
            m_cfg_album_list.add_item (method_add_album.get_album_info ());
        else
            get_method_error_code (method_add_album);
        return ok;
    }

    bool user_album_list::remove_item (t_vk_album_id album_id)
    {
        reset_error ();

        // Delete from profile
        vk_api::audio::albums::del method_delete_album (album_id);
        bool ok = method_delete_album.call ();
        if (ok) {
            // Delete from the list
            auto n = m_cfg_album_list.get_size ();
            while (n --> 0) {
                if (m_cfg_album_list[n].get<1> () == album_id) {
                    m_cfg_album_list.remove_by_idx (n);
                    break;
                }
            }
        }
        else
            get_method_error_code (method_delete_album);
        return ok;
    }

    bool user_album_list::rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name)
    {
        reset_error ();

        int item_index = find_album (p_current_name);
        if (item_index > -1) {
            vk_api::audio::albums::ren method_rename_album (m_cfg_album_list[item_index].get<1> (), p_new_name);
            bool ok = method_rename_album.call ();
            if (ok)
                m_cfg_album_list[item_index].get<0> () = p_new_name;
            else
                get_method_error_code (method_rename_album);
            return ok;
        }
        else {
            m_error << "album \"" << p_current_name << "\" not found";
            return false;
        }
    }
}