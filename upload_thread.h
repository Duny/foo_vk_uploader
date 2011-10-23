#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    namespace upload_queue
    {
        class upload_thread : public threaded_process_callback
        {
        public:
	        upload_thread (const char *p_url, const membuf_ptr &p_data, win32_event &p_event_upload_finished)
                : m_url (p_url), m_data (p_data), m_event_upload_finished (p_event_upload_finished) {}

            const pfc::string8 get_error () const { return m_error; }

        private:
            void run(threaded_process_status & p_status,abort_callback & p_abort);
            void on_done(HWND p_wnd,bool p_was_aborted) override { m_event_upload_finished.set_state (true); }

            pfc::string8 m_url, m_error;
            membuf_ptr m_data;
            win32_event &m_event_upload_finished;
        };
    }
}
#endif