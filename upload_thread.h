#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    namespace upload_queue
    {
        class upload_thread : public threaded_process_callback
        {
        public:
	        upload_thread (const char *p_file, const win32_event &p_event_upload_finished)
                : m_file (p_file), m_event_upload_finished (p_event_upload_finished) {}
            ~upload_thread () {
                int i = 0;
            }

            bool successed () const { return m_error.is_empty (); }
            const pfc::string8 &get_error () const { return m_error; }

        private:
            void run (threaded_process_status &p_status, abort_callback &p_abort);

            void on_done (HWND p_wnd, bool p_was_aborted) override;

            typedef pfc::array_t<t_uint8> membuf_ptr;
            void get_file_contents (const char *p_location, membuf_ptr &p_out);

            pfc::string8 m_file, m_error;
            const win32_event &m_event_upload_finished;
        };
    }
}
#endif