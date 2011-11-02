#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    namespace upload_queue
    {
        class upload_thread : public threaded_process_callback
        {
        public:
	        upload_thread (metadb_handle_list_cref p_items, const upload_params &p_params, const win32_event &p_event_upload_finished)
                : m_items (p_items), m_params (p_params), m_event_upload_finished (p_event_upload_finished) {}

            void start ();

        private:
            void run (threaded_process_status &p_status, abort_callback &p_abort) override;

            void on_done (HWND p_wnd, bool p_was_aborted) override;

            t_audio_id upload_item (const metadb_handle_ptr &p_item, threaded_process_status &p_status, abort_callback &p_abort);

            metadb_handle_list m_items;
            upload_params m_params;
            const win32_event &m_event_upload_finished;
            pfc::string8_fast m_errors;
        };
    }
}
#endif