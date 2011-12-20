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
        pfc::list_t<t_vk_audio_id> aids_list;

        m_errors.reset ();

        t_size item_count = 0, item_max = m_items.get_size ();;
        m_items.for_each ([&] (const metadb_handle_ptr &p_item)
        {
            p_status.set_title (pfc::string_formatter () << "Uploading " << pfc::string_filename_ext (p_item->get_path ()));

            pfc::string8_fast reason;
            if (filter_bad_file (p_item, reason))
                m_errors << "Skipping " << p_item->get_path () << ": " << reason << "\n";
            else {
                try {
                    aids_list.add_item (upload_item (p_item, p_status, p_abort));
                }
                catch (exception_aborted) { return; }
                catch (const std::exception &e) {
                    m_errors << "Failed " << p_item->get_path () << ": " << e.what () << "\n";
                }
            }
        });

        if (!aids_list.get_size ()) return; // Uploading failed, nothing to do

        auto album_id = m_params.get<field_album_id> ();
        if (album_id != 0 && album_id != pfc_infinite) {
            try {
                p_status.set_item ("moving song(s) to album");
                api_audio_moveToAlbum move_to_album (aids_list, album_id, p_abort);
            }
            catch (exception_aborted) { return; }
            catch (const std::exception &e) {
                m_errors << "Error while moving track(s) to album: " << e.what ();
            }
        }

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