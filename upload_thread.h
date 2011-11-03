#ifndef _FOO_VK_UPLOADER_UPLOAD_THREAD_H_
#define _FOO_VK_UPLOADER_UPLOAD_THREAD_H_

namespace vk_uploader
{
    struct upload_params
    {
        upload_params () {}
        upload_params (t_album_id id, bool post_on_wall, const pfc::string8 &mgs)
            : m_album_id (id), m_post_on_wall (post_on_wall), m_post_mgs (mgs) {}

        template <bool isBigEndian> void set_data_raw (stream_reader_formatter<isBigEndian> &stream)
        { stream >> m_album_id >> m_post_on_wall >> m_post_mgs; }

        template <bool isBigEndian> void get_data_raw (stream_writer_formatter<isBigEndian> &stream) const
        { stream << m_album_id << m_post_on_wall << m_post_mgs; }

        bool operator== (const upload_params &other) const { return m_album_id == other.m_album_id && m_post_on_wall == other.m_post_on_wall && m_post_mgs == other.m_post_mgs; }
        bool operator!= (const upload_params &other) const { return !operator== (other); }

        t_album_id m_album_id; // move uploaded item to specified album. 0 = off
        bool m_post_on_wall; // tell friends about your track
        pfc::string8 m_post_mgs; // add some text in top of post
    };


    class upload_thread : public threaded_process_callback
    {
    public:
	    upload_thread (metadb_handle_list_cref p_items, const upload_params &p_params)
            : m_items (p_items), m_params (p_params) {}

        void start ();

    private:
        void run (threaded_process_status &p_status, abort_callback &p_abort) override;

        void on_done (HWND p_wnd, bool p_was_aborted) override;

        t_audio_id upload_item (const metadb_handle_ptr &p_item, threaded_process_status &p_status, abort_callback &p_abort);

        metadb_handle_list m_items;
        upload_params m_params;
        pfc::string8_fast m_errors;
    };

    inline void start_upload (metadb_handle_list_cref p_items, const upload_params &p_params)
    {
        service_ptr_t<upload_thread> thread = new service_impl_t<upload_thread> (p_items, p_params);
        thread->start ();
    }
}
#endif