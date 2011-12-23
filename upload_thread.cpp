#include "stdafx.h"

namespace vk_uploader
{
    void upload_thread::start ()
    {
        threaded_process::g_run_modeless (this, 
            threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_no_focus,
            core_api::get_main_window (), "vk.com audio upload...");
    }

    void upload_thread::run (threaded_process_status &p_status, abort_callback &p_abort)
    {
        m_errors.reset ();

        auto album_name_script = m_params.get<field_album_name> ();
        pfc::list_t<t_vk_audio_id> aids_list;
        bool album_list_reloaded = false;
        m_items.for_each ([&] (const metadb_handle_ptr &p_item)
        {
            p_status.set_title (pfc::string_formatter () << "Uploading " << pfc::string_filename_ext (p_item->get_path ()));

            pfc::string8_fast reason;
            if (filter_bad_file (p_item, reason))
                m_errors << "Skipping " << p_item->get_path () << ": " << reason << "\n";
            else {
                // Upload item
                t_vk_audio_id audio_id = pfc_infinite;
                try {
                    audio_id = upload_item (p_item, p_status, p_abort);
                    aids_list.add_item (audio_id);
                }
                catch (exception_aborted) { return; }
                catch (const std::exception &e) {
                    m_errors << "Failed " << p_item->get_path () << ": " << e.what () << "\n";
                }

                // Move track to album
                if (!album_name_script.is_empty () && audio_id != pfc_infinite) {
                    static_api_ptr_t<titleformat_compiler> p_compiler;
                    service_ptr_t<titleformat_object> p_obj;
                    pfc::string8 album_name;

                    p_compiler->compile (p_obj, album_name_script);
                    p_item->format_title (nullptr, album_name, p_obj, &titleformat_text_filter_nontext_chars ());
                    
                    if (!album_name.is_empty ()) {
                        user_album_list albums;
                        auto album_id = albums.get_album_id_by_name (album_name);

                        if (!album_id && !album_list_reloaded) { // Try to refresh album list
                            album_list_reloaded = true;
                            try { albums.reload_items (); album_id = albums.get_album_id_by_name (album_name); }
                            catch (exception_aborted) { return; }
                            catch (const std::exception &e) {
                                m_errors << "Error while reading album list:" << e.what () << "\n";
                            }
                        }

                        try {
                            if (!album_id)
                                throw pfc::exception (pfc::string_formatter () << "album \"" << album_name << "\" doesn't exists\n");
                            p_status.set_item (pfc::string_formatter () << "moving track to album " << album_name);
                            api_audio_moveToAlbum move_to_album (audio_id, album_id, p_abort);
                        }
                        catch (exception_aborted) { return; }
                        catch (const std::exception &e) {
                            m_errors << "Error while moving " << pfc::string_filename_ext (p_item->get_path ()) << " to album: " << e.what () << "\n";
                        }
                    }
                }
            }
        });

        if (!aids_list.get_size ()) return; // Nothing to do

        if (m_params.get<field_post_on_wall> ()) {
            try {
                p_status.set_item ("making post on wall");
                api_wall_post new_post (m_params.get<field_post_message> (), aids_list, p_abort);
            }
            catch (exception_aborted) { return; }
            catch (const std::exception &e) {
                m_errors << "Error while posting message on the wall: " << e.what ();
            }
        }
    }

    void upload_thread::on_done (HWND p_wnd, bool p_was_aborted)
    {
        if (!m_errors.is_empty () && !p_was_aborted)
            popup_message::g_show (m_errors, "Some items failed to upload", popup_message::icon_error);
    }

    t_vk_audio_id upload_thread::upload_item (const metadb_handle_ptr &p_item, threaded_process_status &p_status, abort_callback &p_abort)
    {
        // step 1
        p_status.set_item ("requesting upload server");
        api_audio_getUploadServer url_for_upload (p_abort);

        // step 2
        p_status.set_item ("sending file to server");
        pfc::string8_fast answer = get_api_provider ()->file_upload (url_for_upload, p_item->get_path (), p_abort);

        // step 3
        p_status.set_item ("saving audio in profile");
        api_audio_save uploaded_audio_file (answer, p_abort);
        t_vk_audio_id id = uploaded_audio_file.get_id ();

        return id;
    }

    bool upload_thread::filter_bad_file (metadb_handle_ptr p_item, pfc::string8_fast &p_reason)
    {
        p_reason.reset ();

        // file type must be mp3 (lossy)
        metadb_handle_lock lock (p_item);
        const file_info *p_info;
        if (p_item->get_info_locked (p_info)) {
            const char *codec = p_info->info_get ("codec");
            if (codec) {
                if (pfc::stricmp_ascii ("MP3", codec) != 0) {
                    p_reason << "file is not an mp3.(codec: " << codec << ")";
                    return true;
                }   
            }

            const char *encoding = p_info->info_get ("encoding");
            if (encoding) {
                if (pfc::stricmp_ascii ("lossy", encoding) != 0) {
                    p_reason << "file is not lossy.(encoding: " << encoding << ")";
                    return true;
                }
            }
        }

        return false;
    }
}