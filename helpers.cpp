#include "stdafx.h"

#include "vk_api.h"

namespace vk_uploader
{
    pfc::string8 trim (const pfc::string8 &p_str)
    {
        auto is_trim_char = [] (char c) -> bool { return c == ' ' || c == '\n' || c == '\t'; };

        t_size str_len = p_str.get_length ();

        t_size left = 0;
        while (left < str_len && is_trim_char (p_str[left])) left++;

        t_size right = str_len - 1;
        while (right > left && is_trim_char (p_str[right])) right--;

        pfc::string8 p_out = p_str;
        p_out.truncate (right + 1);
        p_out.remove_chars (0, left);
        return p_out;
    }

    bool filter_bad_file (metadb_handle_ptr p_item, pfc::string8_fast &p_reason)
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

    void get_file_contents (const char *p_path, membuf_ptr &p_out)
    {
        file_ptr p_file;
        abort_callback_impl p_abort;

        filesystem::g_open_read (p_file, p_path, p_abort);
        if (p_file.is_valid ()) {
            t_size file_size = (t_size)p_file->get_size (p_abort);
            p_out.set_size (file_size);
            t_size read = p_file->read (p_out.get_ptr (), file_size, p_abort);
            if (read != file_size)
                throw exception_io ();
        }
        else
            throw exception_io_not_found ();
    }

    request_url_builder::request_url_builder (const char *p_method_name, params_cref p_params) 
    {
        m_url << "https://api.vk.com/method/" << p_method_name << "?";

        pfc::string8_fast tmp;
        p_params.enumerate ([&] (const pfc::string8 &p_name, const pfc::string8 &p_value) { pfc::urlEncode (tmp, p_value); m_url << p_name << "=" << tmp << "&"; });

        m_url << "access_token=" << static_api_ptr_t<vk_api::authorization>()->get_access_token ();

        //popup_message::g_show (m_url, "");
    }

    url_params::url_params (const pfc::string8 &p_url)
    {
        t_size pos;

        if ((pos = p_url.find_first ('#')) != pfc_infinite || (pos = p_url.find_first ('?')) != pfc_infinite) {
            t_size len = p_url.length (), pos2, pos3;
            for (pos = pos + 1; pos < len; pos = pos3 + 1) {
                if ((pos2 = p_url.find_first ('=', pos)) == pfc_infinite)
                    break;

                if ((pos3 = p_url.find_first ('&', ++pos2)) == pfc_infinite)
                    pos3 = len;

                pfc::string8_fast name, value;
                name = pfc::string_part (p_url.get_ptr () + pos, pos2 - pos - 1);
                value = pfc::string_part (p_url.get_ptr () + pos2, pos3 - pos2);

                this->find_or_add (name) = value;
            }
        }
    }
}