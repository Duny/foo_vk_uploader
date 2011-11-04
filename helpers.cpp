#include "stdafx.h"

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


request_url_builder::request_url_builder (const char *p_method_name, params_cref p_params, abort_callback &p_abort) 
{
    m_url << "https://api.vk.com/method/" << p_method_name << "?";

    pfc::string8_fast tmp;
    p_params.enumerate ([&] (const pfc::string8 &p_name, const pfc::string8 &p_value) { pfc::urlEncode (tmp, p_value); m_url << p_name << "=" << tmp << "&"; });

    m_url << "access_token=" << get_auth_manager ()->get_access_token ();
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