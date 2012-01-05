#include "stdafx.h"

#include "vk_api.h"


// Configuration
namespace
{
    GUID guid_vk_advconfig_branch = { 0xafe40ad7, 0x4112, 0x4363, { 0x97, 0x3f, 0x71, 0xab, 0x07, 0xb5,0x89,0x2e }};

    advconfig_branch_factory g_vk_advconfig_branch ("Vk.com music uploader",
        guid_vk_advconfig_branch,
        advconfig_branch::guid_branch_tools, 0);
    
    advconfig_checkbox_factory g_allways_create_album ("Always create album",
        create_guid (0x7f51105d, 0x6f51, 0x4f9e, 0xbe, 0xc1, 0x5f, 0x89, 0xef, 0x43, 0x61, 0x9d),
        /*parent guid*/guid_vk_advconfig_branch,
        /*priority*/0,
        /*p_initialstate*/true);
}

using namespace vk_uploader;

class upload_thread : public threaded_process_callback
{
public:
	upload_thread (metadb_handle_list_cref p_items, const upload_parameters &p_params)
        : m_items (p_items), m_params (p_params) {}

    void start ()
    {
        threaded_process::g_run_modeless (this, 
            threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_no_focus,
            core_api::get_main_window (), "vk.com audio upload...");
    }

private:
    void run (threaded_process_status & p_status, abort_callback & p_abort) override;

    void on_done (HWND p_wnd, bool p_was_aborted) override
    {
        if (!m_errors.is_empty () && !p_was_aborted)
            popup_message::g_show (m_errors, "Some items failed to upload", popup_message::icon_error);
    }

    const char* filter_bad_file (const metadb_handle_ptr & p_item);

    metadb_handle_list m_items;
    upload_parameters m_params;
    pfc::string8 m_errors;
};


void upload_thread::run (threaded_process_status & p_status, abort_callback & p_abort)
{
    m_errors.reset ();

    // Prepare titleformatting object for album name calculation
    static_api_ptr_t<titleformat_compiler> p_compiler;
    service_ptr_t<titleformat_object> p_object;
    p_compiler->compile (p_object, m_params.get<field_album_name> ());

    pfc::list_t<t_vk_audio_id> audio_ids; // Used (not always) for posting on wall
    bool album_list_reloaded = false; // We can reload list of user albums once if needed

    for (t_size i = 0, num_items = m_items.get_size (); i < num_items; ++i) {
        const metadb_handle_ptr & p_item = m_items[i];
        pfc::string_filename_ext  file_name (p_item->get_path ());

        p_status.set_title (pfc::string_formatter () << "Uploading " << file_name);

        if (auto reason = filter_bad_file (p_item)) {
            m_errors << "Skipping \"" << file_name << "\": " << reason << "\n";
            continue;
        }

        // Upload track
        //
        bool track_needs_correction = false; // Call "audio.edit" to force artist & album

        // Step 1: Get URL for upload
        p_status.set_item ("requesting upload server");

        vk_api::audio::get_upload_server url_for_upload;
        if (!url_for_upload.call (p_abort)) {
            if (url_for_upload.aborted ()) return; // Operation aborted
            else m_errors << "Failed " << file_name << ": " << url_for_upload.get_error () << "\n";
            continue;
        }
        
        // Step 2: Send file content to server using url from step #1
        p_status.set_item ("sending file to server");
        membuf_ptr answer_raw = get_api_provider ()->upload_audio_file (url_for_upload, p_item->get_path (), p_abort);
        pfc::array_t<t_uint8> & answer_bytes = *answer_raw;
        // Parse answer
        pfc::string8 answer;
        for (t_size i = 0, num_bytes = answer_bytes.get_size (); i < num_bytes; ++i) {
            auto c = answer_bytes[i];
            if (c == 0) track_needs_correction = true;
            else answer.add_char (c);
        }

        // Step 3: Save audio in user profile
        p_status.set_item ("saving audio in profile"); 
        vk_api::audio::save method_save (answer);
        if (!method_save.call (p_abort)) {
            if (method_save.aborted ()) return; // Operation aborted
            else m_errors << "Error while saving audio in profile \"" << file_name << "\": " << method_save.get_error () << "\n";
            continue;
        }

        // Do post-upload tasks
        //
        auto audio_id = method_save.get_id ();
        audio_ids.add_item (audio_id); // Save id for the post on wall

        // Edit tracks info
        if (track_needs_correction) {
            p_status.set_item ("editing audio information"); 
            vk_api::audio::edit method_edit (p_item, audio_id);
            if (!method_edit.call (p_abort)) {
                if (method_edit.aborted ()) return; // Operation aborted
                else m_errors << "Error while editing \"" << file_name << "\": " << method_edit.get_error () << "\n";
            }
        }

        // Format item album title
        //
        pfc::string8 album_name;
        p_item->format_title (nullptr, album_name, p_object, &titleformat_text_filter_nontext_chars ());

        // Move tracks to albums whose album_name does not evaluate to empty string
        if (!album_name.is_empty ()) {
            user_album_list user_albums;
            auto album_id = user_albums.get_album_id_by_name (album_name);

            // If album_name not found in album list, try to fetch album list from user profile
            if (!album_id && !album_list_reloaded) {
                album_list_reloaded = true;
                if (user_albums.reload ()) album_id = user_albums.get_album_id_by_name (album_name);
                else if (user_albums.aborted ()) return; // Operation aborted
                else m_errors << "Error while reading album list:" << user_albums.get_error () << "\n";
            }

            // If album_name still not found then try to create it
            if (!album_id && g_allways_create_album.get ()) {
                if (user_albums.add_item (album_name)) album_id = user_albums.get_album_id_by_name (album_name);
                else if (user_albums.aborted ()) return; // Operation aborted
                else m_errors << "Error while creating new album:" << user_albums.get_error () << "\n";    
            }

            // Try Once again
            if (album_id) {
                p_status.set_item (pfc::string_formatter () << "moving track to album \"" << album_name << "\"");
                // Move track to the album
                vk_api::audio::move_to_album method_move_to_album (audio_id, album_id);
                if (!method_move_to_album.call (p_abort)) {
                    if (method_move_to_album.aborted ()) return; // Operation aborted
                    else m_errors << "Error while moving \"" << file_name << "\" to album: " << method_move_to_album.get_error () << "\n";
                }
            }
        }
    }

    if (!audio_ids.get_size ()) return; // Nothing to do more

    if (m_params.get<field_post_on_wall> ()) {
        p_status.set_item ("making post on the wall");
        // Post new message on user wall this all uploaded tracks attached
        vk_api::wall::post method_post_on_wall (m_params.get<field_post_message> (), audio_ids);
        if (!method_post_on_wall.call (p_abort) && !method_post_on_wall.aborted ())
            m_errors << "Error while posting message on the wall: " << method_post_on_wall.get_error ();
    }
}

const char* upload_thread::filter_bad_file (const metadb_handle_ptr & p_item)
{
    static pfc::string8 g_reason;

    g_reason.reset ();
    // file type must be mp3
    metadb_handle_lock lock (p_item);
    const file_info *p_info;
    if (p_item->get_info_locked (p_info)) {
        const char *codec = p_info->info_get ("codec");
        if (codec && pfc::stricmp_ascii ("MP3", codec) != 0) {
            g_reason << "file is not an mp3 (codec: " << codec << ")";
            return g_reason.get_ptr ();
        }
    }
    return nullptr;
}

void start_upload (metadb_handle_list_cref p_items, const upload_parameters &p_params)
{
    service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (p_items, p_params);
    thread->start ();
}