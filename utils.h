#ifndef _FOO_VK_UPLOADER_UTILS_H_
#define _FOO_VK_UPLOADER_UTILS_H_

namespace vk_uploader
{
    pfc::string8 trim (const pfc::string8 &p_str);

    inline pfc::string8 get_window_text_trimmed (HWND wnd)
    {
        return trim (string_utf8_from_window (wnd).get_ptr ());
    }

    inline bool skip_prefix (pfc::string8 &p_string, const char *p_prefix)
    {
        if (pfc::strcmp_partial (p_string.get_ptr (), p_prefix) == 0) {
            p_string.remove_chars (0, pfc::strlen_max (p_prefix, pfc_infinite));
            return true;
        }
        return false;
    }

    inline pfc::string8 url_decode (const pfc::string8 &p_str)
    {
        pfc::string8 out;

        for (t_size i = 0, n = p_str.get_length (); i < n; i++) {
            char ch = p_str[i];
            if (ch == '%') {
                if (i < n - 2) {
                    auto res = pfc::char_to_hex (p_str[i + 1]) << 4;
                    res |= pfc::char_to_hex (p_str[i + 2]);
                    ch = (char)res;
                    i += 2;
                }
            }
            out.add_char (ch);
        }
        return out;
    }

    /*class string_utf8_from_combo
    {
    public:
        string_utf8_from_combo (const CComboBox &p_combo, int p_index)
        {
            int str_len = p_combo.GetLBTextLen (p_index);
            if (str_len != CB_ERR) {
                pfc::array_t<TCHAR> buf;
                buf.set_size (str_len + 1);
                int actual_len = p_combo.GetLBText (p_index, buf.get_ptr ());
                if (actual_len - 1 == str_len)
                    m_data = pfc::stringcvt::string_utf8_from_os (buf.get_ptr ());
            }
        }

        inline operator const char * () const { return m_data.get_ptr (); }
        inline t_size length () const  {return m_data.length (); }
        inline bool is_empty () const { return length () == 0; }
        inline const char * get_ptr () const { return m_data.get_ptr (); }

    private:
        pfc::string8 m_data;
    };*/
}

#endif