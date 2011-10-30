#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    namespace upload_queue
    {
        class upload_thread : public threaded_process_callback
        {
        public:
	        upload_thread (metadb_handle_ptr p_item, const win32_event &p_event_upload_finished)
                : m_item (p_item), m_event_upload_finished (p_event_upload_finished) {}

            void start ();

            bool successed () const { return m_error.is_empty (); }
            const pfc::string8 &get_error () const { return m_error; }
            t_audio_id get_audio_id () const { return m_id; }

        private:
            void run (threaded_process_status &p_status, abort_callback &p_abort) override;

            void on_done (HWND p_wnd, bool p_was_aborted) override;

            metadb_handle_ptr m_item;
            pfc::string8 m_error;
            const win32_event &m_event_upload_finished;
            t_audio_id m_id;
        };
    }
}
#endif