#ifndef _FOO_VK_UPLOADER_HELPERS_H_
#define _FOO_VK_UPLOADER_HELPERS_H_

namespace vk_uploader
{
    pfc::string8 trim (const pfc::string8 &p_str);

    inline pfc::string8 get_window_text_trimmed (HWND wnd)
    {
        return trim (string_utf8_from_window (wnd).get_ptr ());
    }

    inline value_t from_string (const pfc::string8 & p_data) {
        const char *begin = p_data.get_ptr ();
        const char *end = begin + p_data.get_length ();
        Json::Value *out = new Json::Value;
        return Json::Reader ().parse (begin, end, *out, false) ? value_t (out) : value_t ();
    }

    class request_url_builder
    {
        pfc::string_formatter m_url;
    public:
        request_url_builder (const char *p_method_name, vk_api::params_cref p_params);

        operator const char * () const { return m_url.get_ptr (); }
    };

    class url_params : public pfc::map_t<pfc::string8 const, pfc::string8>
    {
    public:
        url_params (const pfc::string8 &p_url);

        bool have (const char *p_name) const { return have_item (pfc::string8 (p_name)); }

        pfc::string8 &operator [] (const char *key) { return find_or_add (pfc::string8 (key)); }
    };

    value_t make_error (const char *p_message);

    inline const char *get_error_msg (const value_t &p_val)
    {
        if (p_val->isMember ("error_msg"))
            return (*p_val)["error_msg"].asCString ();
        else
            return "Unknown error";
    }

    inline bool is_response_ok (const value_t &p_val) { return p_val->isObject () && pfc::stricmp_ascii ("response", p_val->begin ().memberName ()) == 0; };
        
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