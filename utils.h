#ifndef _FOO_VK_UPLOADER_UTILS_H_
#define _FOO_VK_UPLOADER_UTILS_H_

namespace vk_uploader
{
    pfc::string8 trim (const pfc::string8 &p_str);

    inline pfc::string8 get_window_text_trimmed (HWND wnd)
    {
        return trim (string_utf8_from_window (wnd).get_ptr ());
    }
}

#endif