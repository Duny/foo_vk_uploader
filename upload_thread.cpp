#include "stdafx.h"
#include "vk_com_api.h"

namespace vk_uploader
{
    namespace configuration
    {

        advconfig_branch_factory vk_advconfig_branch (
            /*const char * p_name*/"Vk.com music uploader",
            /*const GUID & p_guid*/create_guid (0xafe40ad7, 0x4112, 0x4363, 0x97, 0x3f, 0x71, 0xab, 0x07, 0xb5,0x89,0x2e),
            /*const GUID & p_parent*/advconfig_branch::guid_branch_tools, 
            /*double p_priority*/0);

        advconfig_checkbox_factory allways_create_album (
            /*const char * p_name*/"Always create album",
            /*const GUID & p_guid*/create_guid (0x7f51105d, 0x6f51, 0x4f9e, 0xbe, 0xc1, 0x5f, 0x89, 0xef, 0x43, 0x61, 0x9d),
            /*const GUID & p_parent*/vk_advconfig_branch.get_class_guid (), // Construction order depended code
            /*double p_priority*/0,
            /*bool p_initialstate*/true);
    }


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


// Define this true, run and upload any file
// Results will be to console
#define ENABLE_API_CALL_BENCHMARK false
    void do_vk_api_call_benchmark (abort_callback & p_abort)
    {
        t_size api_calls_made;
        DWORD dwStartTime = GetTickCount ();
        for (api_calls_made = 0; api_calls_made < 10; ++api_calls_made)
        {
            if (GetTickCount () - dwStartTime >= 1000)
                break;

            vk_com_api::audio::get_upload_server url_for_upload;
            if (!url_for_upload.call (p_abort)) {
                if (url_for_upload.aborted ())
                    break; // Operation aborted
                else
                    console::formatter () << "Failed : " << url_for_upload.get_error () << "\n";
            }
        }
        
        console::formatter () << "Time elapsed: " << t_size (GetTickCount () - dwStartTime) << " ms, calls made: " << api_calls_made;
    }

    void upload_thread::run (threaded_process_status & p_status, abort_callback & p_abort)
    {
        if (ENABLE_API_CALL_BENCHMARK)
        {
            do_vk_api_call_benchmark (p_abort);
            return;
        }

        m_errors.reset ();

        pfc::list_t<t_vk_audio_id> audio_ids; // For posting on wall

        for (t_size i = 0, num_items = m_items.get_size (); i < num_items; ++i) {
            const metadb_handle_ptr & p_item = m_items[i];
            const pfc::string_filename_ext file_name (p_item->get_path ());

            p_status.set_title (pfc::string_formatter () << "Uploading " << file_name);

            if (auto reason = filter_bad_file (p_item)) {
                m_errors << "Skipping \"" << file_name << "\": " << reason << "\n";
                continue;
            }

            // Upload track
            //

            // Step 1: Get URL for upload
            p_status.set_item ("requesting upload server");

            vk_com_api::audio::get_upload_server url_for_upload;
            if (!url_for_upload.call (p_abort)) {
                if (url_for_upload.aborted ()) return; // Operation aborted
                else m_errors << "Failed " << file_name << ": " << url_for_upload.get_error () << "\n";
                continue;
            }
        
            // Step 2: Send file content to server using url from step #1
            p_status.set_item ("sending file to server");
            vk_com_api::membuf_ptr answer_ptr = vk_com_api::get_provider ()->upload_audio_file (url_for_upload, p_item->get_path (), p_abort);
            const pfc::array_t<t_uint8> & answer_bytes = *answer_ptr;
            // Parse answer
            pfc::string8 answer_text;
            for (t_size i = 0, num_bytes = answer_bytes.get_size (); i < num_bytes; ++i)
                if (auto c = answer_bytes[i]/*c != 0*/) answer_text.add_char (c);

            // Step 3: Save audio in user profile
            p_status.set_item ("saving audio in profile"); 
            vk_com_api::audio::save method_save (answer_text, p_item);
            if (!method_save.call (p_abort)) {
                if (method_save.aborted ()) return; // Operation aborted
                else m_errors << "Error while saving audio in profile \"" << file_name << "\": " << method_save.get_error () << "\n";
                continue;
            }

            // Do post-upload tasks
            //
            auto audio_id = method_save.get_id ();
            audio_ids.add_item (audio_id); // Save id for the post on wall

            // Format item album title
            //
            pfc::string8 album_name;
            {
                service_ptr_t<titleformat_object> p_object;
                static_api_ptr_t<titleformat_compiler> p_compiler;
                p_compiler->compile (p_object, m_params.get<field_album_name>());
                p_item->format_title (nullptr, album_name, p_object, &titleformat_text_filter_nontext_chars ());
            }

            // Move tracks to albums whose album_name does not evaluate to empty string
            if (!album_name.is_empty ()) {
                user_album_list user_albums;
                t_vk_audio_id   album_id = 0;

                // Reload album list
                p_status.set_item (pfc::string_formatter () << "requesting album list");
                if (user_albums.reload ()) album_id = user_albums.get_album_id_by_name (album_name);
                else if (user_albums.aborted ()) return; // Operation aborted
                else m_errors << "Error while loading user album list:" << user_albums.get_error () << "\n";

                // If album_name not found then try to create it
                if (!album_id && configuration::allways_create_album) {
                    p_status.set_item (pfc::string_formatter () << "creating album \"" << album_name << "\"");

                    if (user_albums.add_item (album_name)) album_id = user_albums.get_album_id_by_name (album_name);
                    else if (user_albums.aborted ()) return; // Operation aborted
                    else m_errors << "Error while creating new album:" << user_albums.get_error () << "\n";    
                }

                // Final try
                if (album_id) {
                    p_status.set_item (pfc::string_formatter () << "moving track to album \"" << album_name << "\"");
                    // Move track to the album
                    vk_com_api::audio::move_to_album method_move_to_album (audio_id, album_id);
                    if (!method_move_to_album.call (p_abort)) {
                        if (method_move_to_album.aborted ()) return; // Operation aborted
                        else m_errors << "Error while moving \"" << file_name << "\" to album: " << method_move_to_album.get_error () << "\n";
                    }
                }
            }
        }

        if (!audio_ids.get_size ()) return; // Nothing to do more

        if (m_params.get<field_post_on_wall>()) {
            p_status.set_item ("making post on the wall");
            // Make post on user wall with all uploaded tracks as attaches
            vk_com_api::wall::post method_post_on_wall (m_params.get<field_post_message>(), audio_ids);
            if (!method_post_on_wall.call (p_abort) && !method_post_on_wall.aborted ())
                m_errors << "Error while posting message on the wall: " << method_post_on_wall.get_error ();
        }
    }

    const char * upload_thread::filter_bad_file (const metadb_handle_ptr & p_item)
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
            if (p_item->get_filesize () > vk_com_api::max_mp3_file_size) {
                g_reason << "file size is too big: " << pfc::format_file_size_short (p_item->get_filesize ()) 
                    << " / "<< pfc::format_file_size_short (vk_com_api::max_mp3_file_size) << " max";
                return g_reason.get_ptr ();
            }
        }
        return nullptr;
    }

    void start_upload (metadb_handle_list_cref p_items, const upload_parameters & p_params)
    {
        service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (p_items, p_params);
        thread->start ();
    }
}