#include "stdafx.h"

#include "vk_api.h"
#include "upload_thread.h"

namespace vk_uploader
{
    namespace upload_queue
    {
        void upload_thread::start ()
        {
            threaded_process::g_run_modeless (this, 
                threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_no_focus,
                core_api::get_main_window (), "vk.com audio upload in progress...");
        }

        void upload_thread::run (threaded_process_status &p_status, abort_callback &p_abort)
        {
            try {
                p_status.set_item ("getting server address for upload...");
                vk_api::api_audio_getUploadServer url_for_upload;

                p_status.set_item ("sending file to server...");
                pfc::string8_fast answer = get_api_provider ()->file_upload (url_for_upload, m_item->get_path (), p_abort);

                p_status.set_item ("saving file on server...");
                vk_api::api_audio_save uploaded_audio_file (answer);
                m_id = uploaded_audio_file.get_id ();
                p_status.set_item ("upload finished.");
            } catch (const std::exception &e) {
                m_error = e.what ();
            }
        }

        void upload_thread::on_done (HWND p_wnd, bool p_was_aborted)
        { 
            if (p_was_aborted)
                m_error = "uploading was aborted by user.";
            const_cast<win32_event&>(m_event_upload_finished).set_state (true);
        }
    }
}