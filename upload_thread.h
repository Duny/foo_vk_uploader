#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    class upload_thread : public threaded_process_callback
    {
    public:
	    upload_thread (metadb_handle_list_cref p_items, const upload_parameters &p_params)
            : m_items (p_items), m_params (p_params) {}

        void start ();

    private:
        void run (threaded_process_status &p_status, abort_callback &p_abort) override;

        void on_done (HWND p_wnd, bool p_was_aborted) override;

        t_vk_audio_id upload_item (const metadb_handle_ptr &p_item, threaded_process_status &p_status, abort_callback &p_abort);

        bool filter_bad_file (metadb_handle_ptr p_item, pfc::string8_fast &p_reason);

        metadb_handle_list m_items;
        upload_parameters m_params;
        pfc::string8_fast m_errors;
    };

    inline void start_upload (metadb_handle_list_cref p_items, const upload_parameters &p_params)
    {
        service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (p_items, p_params);
        thread->start ();
    }
}
#endif