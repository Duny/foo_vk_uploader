#include "stdafx.h"

#include "vk_auth.h"
#include "vk_api.h"

#include "boost/foreach.hpp"
using namespace boost::foreach;

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
        get_api_provider ()->invoke ("audio.editAlbum", 
            list_of<name_value_pair>
                ("title", m_new_title)
                ("album_id", pfc::string_formatter () << m_album_id),
            p_abort);
    }

    void vk_api::audio::move_to_album::run (abort_callback & p_abort)
    {
        get_api_provider ()->invoke ("audio.moveToAlbum", 
            list_of<name_value_pair>
                ("aids",     pfc::string_formatter () << m_audio_id)
                ("album_id", pfc::string_formatter () << m_album_id),
            p_abort);
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
        const auto required_fields = list_of ("server")("audio")("hash");
        
        if (m_result.has_members (required_fields)) {

            url_parameters params;
            std::for_each (required_fields.begin (), required_fields.end (),
                [&] (const char * field)
                {
                    params.push_back (make_pair (field, m_result[field].asCString ()));
                }
            );

            m_result = get_api_provider ()->invoke ("audio.save", params, p_abort);

            if (m_result->isMember ("aid"))
                m_id = m_result["aid"].asUInt ();
            else
                throw pfc::exception ("no 'aid' field in response from audio.save");
        }
        else {
            pfc::string_formatter err_mgs = "Not enough parameters in response from file upload. Expected fields: \n";
            std::for_each (required_fields.begin (), required_fields.end (),
                [&] (const char * field)
                {
                    err_mgs << "(" << field << ")";
                }
            );
            err_mgs << "\nGot:\n" << m_result->toStyledString ().c_str ();
            throw pfc::exception (err_mgs);
        }
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
        typedef vk_api::audio::albums::get method_get;

        return call_api (method_get (), [&] (method_get & albums) { m_cfg_album_list = albums; });
    }

    bool user_album_list::add_item (const pfc::string_base & p_title)
    {
        typedef vk_api::audio::albums::add method_add;

        return call_api (method_add (p_title), [&] (method_add & album_info) { m_cfg_album_list.add_item (album_info); });
    }

    bool user_album_list::remove_item (t_vk_album_id album_id)
    {
        typedef vk_api::audio::albums::del method_del;

        return call_api (method_del (album_id), [&] (method_del &) {
            // Delete album from the list
            auto n = m_cfg_album_list.get_size ();
            while (n --> 0) {
                if (m_cfg_album_list[n].get<1> () == album_id) {
                    m_cfg_album_list.remove_by_idx (n);
                    break;
                }
            }
        });
    }

    bool user_album_list::rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name)
    {
        reset_error ();

        int item_index = find_album (p_current_name);
        if (item_index > -1) {
            typedef vk_api::audio::albums::ren method_ren;

            return call_api (method_ren (m_cfg_album_list[item_index].get<1> (), p_new_name),
                [&] (method_ren &) { m_cfg_album_list[item_index].get<0> () = p_new_name; });
        }
        else {
            m_error << "album \"" << p_current_name << "\" not found";
            return false;
        }
    }
}