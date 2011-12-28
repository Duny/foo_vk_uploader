#include "stdafx.h"

#include "album_list_dlg.h"

namespace vk_uploader
{
    cfgDialogPosition album_list_dlg::m_pos (guid_inline<0xe48d7bd1, 0x109b, 0x495c, 0xb2, 0xe0,0x3c, 0x4c,0x8e, 0xb9, 0x4a, 0x4b>::guid);

    void album_list_dlg::completion_notify_new_album::on_completion (unsigned p_code)
    {
        pfc::string8 album_title = trim (*m_str);
        if ((p_code & InPlaceEdit::KEditMaskReason) != InPlaceEdit::KEditEnter || album_title.is_empty ())
            uSendMessage (m_listview, LVM_DELETEITEM, 0, 0); // New item was inserted in the top of the list
        else {
            auto p_listview = m_listview; // Need to declare a copy of m_listview to work correctly
            run_in_separate_thread ([=] ()
            {
                bool success = false;
                try {
                    listview_helper::set_item_text (p_listview, 0, 0, album_title.get_ptr ());
                    user_album_list ().add_item (album_title);
                    success = true;
                }
                catch (exception_aborted) {}
                catch (const std::exception &e) {
                    uMessageBox (core_api::get_main_window (), e.what (), "Error while creating new album", MB_OK | MB_ICONERROR);
                }

                if (!success) uSendMessage (p_listview, LVM_DELETEITEM, 0, 0);
            });
        }
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
                bool success = false;
                try {
                    listview_helper::set_item_text (p_listview, p_item_index, 0, new_title.get_ptr ());
                    user_album_list ().rename_item (p_prev_title, new_title);
                    success = true;
                }
                catch (exception_aborted) {}
                catch (const std::exception &e) {
                    uMessageBox (core_api::get_main_window (), e.what (), "Error while creating new album", MB_OK | MB_ICONERROR);
                }

                if (!success) listview_helper::set_item_text (p_listview, p_item_index, 0, p_prev_title.get_ptr ());
            });
        }
    }

    void album_list_dlg::on_refresh_albums ()
    {
        HWND p_listview = GetDlgItem (IDC_LISTVIEW_ALBUMS);
        run_in_separate_thread ([p_listview] ()
        {
            try {
                user_album_list p_list;
                p_list.reload_items ();
                listview_fill (p_listview, p_list);
            }
            catch (exception_aborted) {}
            catch (const std::exception &e) {
                uMessageBox (core_api::get_main_window (), e.what (), "Error while reading album list", MB_OK | MB_ICONERROR);
            }
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
                    try {
                        user_album_list albums;
                        albums.remove_item (albums.get_album_id_by_name (album_name));
                        listview_fill (p_listview, albums);
                    }
                    catch (exception_aborted) {}
                    catch (const std::exception &e) {
                        uMessageBox (core_api::get_main_window (), e.what (), "Error while creating new album", MB_OK | MB_ICONERROR);
                    }
                });
            }
        }
    }
}