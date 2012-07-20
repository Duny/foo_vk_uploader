#include "stdafx.h"
#include "vk_com_api.h"

namespace vk_uploader
{
    pfc::string8 trim (const pfc::string8 & p_str)
    {
        auto is_trim_char = [] (char c) -> bool { return c == ' ' || c == '\n' || c == '\t'; };

        t_size str_len = p_str.get_length ();

        t_size left = 0;
        while (left < str_len && is_trim_char (p_str[left])) left++;

        t_size right = str_len - 1;
        while (right > left && is_trim_char (p_str[right])) right--;

        return pfc::string_part (p_str.get_ptr () + left, right - left + 1);
    }

    
    pfc::list_t<audio_album_info> user_album_list::g_album_list;

    bool user_album_list::reload ()
    {
        typedef vk_com_api::audio::albums::get method_get;

        return call_api
            (
                method_get (),
                
                [&] (method_get & albums) // on_ok callback
                {
                    g_album_list = albums;
                }
            );
    }

    bool user_album_list::add_item (const pfc::string_base & p_title)
    {
        typedef vk_com_api::audio::albums::add method_add;

        return call_api
            (
                method_add (p_title),

                [&] (method_add & album_info) // on_ok callback
                {
                    g_album_list.add_item (album_info);
                }
            );
    }

    bool user_album_list::remove_item (t_vk_album_id album_id)
    {
        typedef vk_com_api::audio::albums::del method_del;

        return call_api 
            (
                method_del (album_id),

                [&] (method_del &) // on_ok callback
                {
                    // Delete album from the list
                    auto n = g_album_list.get_size ();
                    while (n --> 0) {
                        if (g_album_list[n].get<1>() == album_id) {
                            g_album_list.remove_by_idx (n);
                            break;
                        }
                    }
                }
            );
    }

    bool user_album_list::rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name)
    {
        reset_error ();

        int item_index = find_album (p_current_name);
        if (item_index > -1) {
            typedef vk_com_api::audio::albums::ren method_ren;

            return call_api
                (
                    method_ren (g_album_list[item_index].get<1>(), p_new_name),

                    [&] (method_ren &) // on_ok callback
                    {
                        g_album_list[item_index].get<0>() = p_new_name;
                    }
                );
        }
        else {
            m_error << "album \"" << p_current_name << "\" not found";
            return false;
        }
    }
}