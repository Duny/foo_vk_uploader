#include "stdafx.h"

#include "vk_api.h"
#include "upload_preset.h"
#include "upload_thread.h"

namespace vk_uploader
{
    namespace upload_queue
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
            pfc::list_t<t_audio_id> aids_list;

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
                    } catch (const std::exception &e) {
                        m_errors << p_item->get_path () << " failed: " << e.what () << "\n";
                    }
                }

                p_status.set_progress (++item_count, item_max);
            });

            if (m_params.m_album_id != 0 && m_params.m_album_id != pfc_infinite) {
                vk_api::api_audio_moveToAlbum move_to_album (aids_list, m_params.m_album_id);
                if (!move_to_album.is_valid ())
                    m_errors << "Error while moving track(s) to album: " << move_to_album.get_error ();
            }

            if (m_params.m_post_on_wall) {
                vk_api::api_wall_post new_post (m_params.m_post_mgs, aids_list);
                if (!new_post.is_valid ())
                    m_errors << "Error while posting message on the wall: " << new_post.get_error ();
            }
        }

        void upload_thread::on_done (HWND p_wnd, bool p_was_aborted)
        {
            if (!m_errors.is_empty () && !p_was_aborted)
                popup_message::g_show (m_errors, "Some items failed to upload", popup_message::icon_error);

            const_cast<win32_event&>(m_event_upload_finished).set_state (true);
        }

        t_audio_id upload_thread::upload_item (const metadb_handle_ptr &p_item, threaded_process_status &p_status, abort_callback &p_abort)
        {
            // step 1
            vk_api::api_audio_getUploadServer url_for_upload;
            p_status.set_progress_secondary (1, 3);

            // step 2
            pfc::string8_fast answer = get_api_provider ()->file_upload (url_for_upload, p_item->get_path (), p_abort);
            p_status.set_progress_secondary (2, 3);

            // step 3
            vk_api::api_audio_save uploaded_audio_file (answer);
            t_audio_id id = uploaded_audio_file.get_id ();
            p_status.set_progress_secondary (3, 3);

            return id;
        }
    }
}