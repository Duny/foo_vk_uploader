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

    request_url_builder::request_url_builder (const char *p_method_name, vk_api::params_cref p_params) 
    {
        m_url << "https://api.vk.com/method/" << p_method_name << "?";
        for (t_size i = 0, n = p_params.get_size (); i < n; i++)
            m_url << p_params[i].first << "=" << p_params[i].second << "&";
        m_url << "access_token=" << static_api_ptr_t<vk_api::authorization>()->get_access_token ();
    }

    url_params::url_params (const pfc::string8 &p_url)
    {
        t_size pos;

        if ((pos = p_url.find_first ('#')) != pfc_infinite || (pos = p_url.find_first ('?')) != pfc_infinite) {
            t_size len = p_url.length (), pos2, pos3;
            for (pos = pos + 1; pos < len; pos = pos3 + 1) {
                if ((pos2 = p_url.find_first ('=', pos)) == pfc_infinite)
                    break;

                if ((pos3 = p_url.find_first ('&', pos2 + 1)) == pfc_infinite)
                    pos3 = len;

                pfc::string8_fast name, value;
                name = pfc::string_part (p_url.get_ptr () + pos, pos2 - pos);
                value = pfc::string_part (p_url.get_ptr () + pos2 + 1, pos3 - pos2 - 1);

                this->find_or_add (name) = value;
            }
        }
    }

    value_t make_error (const char *p_message) {
        static const Json::StaticString error ("error"), error_code ("error_code"), error_msg ("error_msg");

        // build json like this:
        // { "error" : { "error_code" : 3, "error_msg" : "Unknown method passed" } }

        Json::Value error_object;
        error_object[error_code] = -1;
        error_object[error_msg] = p_message;

        Json::Value *error_value = new Json::Value;
        (*error_value)[error] = error_object;

        return value_t (error_value);
    }
}