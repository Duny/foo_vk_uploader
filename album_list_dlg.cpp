#include "stdafx.h"

#include "vk_com_api.h"
#include "album_list_dlg.h"


namespace { cfgDialogPosition g_dlg_pos (vk_uploader::create_guid (0xe48d7bd1, 0x109b, 0x495c, 0xb2, 0xe0, 0x3c, 0x4c,0x8e, 0xb9, 0x4a, 0x4b)); }

inline void uErrorBox (const char *p_title, const char *p_message)
{
    uMessageBox (core_api::get_main_window (), p_message, p_title, MB_OK | MB_ICONERROR);
}

namespace vk_uploader
{
    void album_list_dlg::completion_notify_new_album::on_completion (unsigned p_code)
    {
        pfc::string8 album_title = trim (*m_str);
        if ((p_code & InPlaceEdit::KEditMaskReason) == InPlaceEdit::KEditEnter && !album_title.is_empty ()) {
            auto p_listview = m_listview; // Need to declare a copy of m_listview to work correctly
            run_in_separate_thread ([=] () // Need to use separate thread because of UI freezes during http-requests
            {
                listview_helper::set_item_text (p_listview, 0, 0, album_title.get_ptr ());

                user_album_list user_albums;
                if (!user_albums.add_item (album_title)) {
                    uSendMessage (p_listview, LVM_DELETEITEM, 0, 0);
                    if (!user_albums.aborted ())
                        uErrorBox ("Error while creating new album", user_albums.get_error ());
                }
            });
        }
        else
            uSendMessage (m_listview, LVM_DELETEITEM, 0, 0);
    }

    void album_list_dlg::completion_notify_rename_album::on_completion (unsigned p_code)
    {
        user_album_list albums;
        auto album_id = albums.get_album_id_by_name (m_prev_title);
        pfc::string8 new_title = trim (*m_new_title);
        if ((p_code & InPlaceEdit::KEditMaskReason) == InPlaceEdit::KEditEnter && album_id) {
            auto p_listview = m_listview;
            auto p_item_index = m_item_index;
            auto p_prev_title = m_prev_title;
            run_in_separate_thread ([=] ()
            {
                listview_helper::set_item_text (p_listview, p_item_index, 0, new_title.get_ptr ());

                vk_uploader::user_album_list user_albums;
                if (!user_albums.rename_item (p_prev_title, new_title)) {
                   listview_helper::set_item_text (p_listview, p_item_index, 0, p_prev_title.get_ptr ());
                   if (!user_albums.aborted ())
                       uErrorBox ("Error while creating new album", user_albums.get_error ());
                }
            });
        }
    }

    void album_list_dlg::on_init_dialog ()
    {
        g_dlg_pos.AddWindow (*this);

        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);
            
        // Add one column
        LVCOLUMN data = {};
		data.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		data.fmt = LVCFMT_LEFT;
        data.cx = 160;
        data.pszText = L"Album title";
        uSendMessage (p_listview, LVM_INSERTCOLUMN, 0, (LPARAM)&data);

        // Window must have WS_CLIPCHILDREN for inline editing
        ::SetWindowLong (p_listview, GWL_STYLE, ::GetWindowLong (p_listview, GWL_STYLE) | WS_CLIPCHILDREN);

        listview_fill (p_listview, user_album_list ().get_albums ());
    }

    void album_list_dlg::on_destroy ()
    {
        g_dlg_pos.RemoveWindow (*this);
    }

    void album_list_dlg::on_refresh_albums ()
    {
        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);
        run_in_separate_thread ([p_listview] ()
        {
            user_album_list user_albums;
            if (user_albums.reload ())
                listview_fill (p_listview, user_albums.get_albums ());
            else if (!user_albums.aborted ())
                uErrorBox ("Error while reading album list", user_albums.get_error ());
        });
    }

    void album_list_dlg::on_album_new ()
    {
        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);
        if (listview_helper::insert_item (p_listview, 0, "", 0) != pfc_infinite) {
            m_str->reset ();
            InPlaceEdit::Start_FromListView (p_listview, 0, 0, 1, m_str, new service_impl_t<completion_notify_new_album> (p_listview, m_str));
        }
    }

    void album_list_dlg::on_album_rename ()
    {
        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);
        auto selected_item = ListView_GetSingleSelection (p_listview);
        if (selected_item > -1) {
            pfc::string8 album_title;
            listview_helper::get_item_text (p_listview, selected_item, 0, album_title);
            m_str->set_string (album_title);
            InPlaceEdit::Start_FromListView (p_listview, selected_item, 0, 1, m_str, 
                new service_impl_t<completion_notify_rename_album> (p_listview, selected_item, album_title, m_str));
        }
    }

    void album_list_dlg::on_album_delete ()
    {
        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);

        pfc::string8 album_name;
        listview_get_sel_item_text (p_listview, album_name);
        if (!album_name.is_empty ()) {
            pfc::string8_fast message ("Are you sure what you want to delete album \"");
            message << album_name << "\"?";
            int dlg_result = uMessageBox (*this, message, COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION);
            if (dlg_result == IDYES) {
                run_in_separate_thread ([p_listview, album_name] ()
                {
                    user_album_list user_albums;
                    if (user_albums.remove_item (user_albums.get_album_id_by_name (album_name)))
                        listview_fill (p_listview, user_albums.get_albums ()); // Reload items (change to deleting single item instead)
                    else if (!user_albums.aborted ())
                        uErrorBox ("Error while creating new album", user_albums.get_error ());
                });
            }
        }
    }
}