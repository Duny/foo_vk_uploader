#ifndef _FOO_VK_UPLOADER_UPLOAD_QUEUE_H_
#define _FOO_VK_UPLOADER_UPLOAD_QUEUE_H_

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

    namespace upload_queue
    {
        void push_back (metadb_handle_list_cref p_items, const upload_params &p_params);
    };
}
#endif