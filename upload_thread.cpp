#include "stdafx.h"

namespace vk_uploader
{
    void upload_thread::start ()
    {
        m_errors.reset ();

        threaded_process::g_run_modeless (this, 
            threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_no_focus | threaded_process::flag_show_progress_dual,
            core_api::get_main_window (), "vk.com audio uploading in progress...");
    }

    void upload_thread::run (threaded_process_status &p_status, abort_callback &p_abort)
    {
        pfc::list_t<t_vk_audio_id> aids_list;

        t_size item_count = 0, item_max = m_items.get_size ();;
        m_items.for_each ([&] (const metadb_handle_ptr &p_item)
        {
            p_status.set_item_path (p_item->get_path ());

            pfc::string8_fast reason;
            if (filter_bad_file (p_item, reason))
                m_errors << "Skipping " << p_item->get_path () << ": " << reason << "\n";
            else {
                try {
                    aids_list.add_item (upload_item (p_item, p_status, p_abort));
                }
                catch (exception_aborted) { return; }
                catch (const std::exception &e) {
                    m_errors << p_item->get_path () << " failed: " << e.what () << "\n";
                }
            }

            p_status.set_progress (++item_count, item_max);
        });

        auto album_id = m_params.get<field_album_id> ();
        if (album_id != 0 && album_id != pfc_infinite) {
            try {
                api_audio_moveToAlbum move_to_album (aids_list, album_id, p_abort);
            }
            catch (exception_aborted) { return; }
            catch (const std::exception &e) {
                m_errors << "Error while moving track(s) to album: " << e.what ();
            }
        }

        if (m_params.get<field_post_on_wall> ()) {
            try {
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
        api_audio_getUploadServer url_for_upload (p_abort);
        p_status.set_progress_secondary (1, 3);

        // step 2
        pfc::string8_fast answer = get_api_provider ()->file_upload (url_for_upload, p_item->get_path (), p_abort);
        p_status.set_progress_secondary (2, 3);

        // step 3
        api_audio_save uploaded_audio_file (answer, p_abort);
        t_vk_audio_id id = uploaded_audio_file.get_id ();
        p_status.set_progress_secondary (3, 3);

        return id;
    }

    bool upload_thread::filter_bad_file (metadb_handle_ptr p_item, pfc::string8_fast &p_reason)
    {
        p_reason.reset ();

        // file type (mp3, lossy)
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

        // file size
        const t_size max_file_size = 20 * (1 << 20); // 20Mb

        t_filesize size = p_item->get_filesize ();
        //if (size >= max_file_size) {
        //    p_reason << "file is too big - " << pfc::format_file_size_short (size) << ".(maximum file size is " 
        //        << pfc::format_file_size_short (max_file_size) << ")";
        //    return true;
        //}

        return false;
    }
}