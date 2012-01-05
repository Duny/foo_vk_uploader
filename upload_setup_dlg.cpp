#include "stdafx.h"

#include "vk_auth.h"
#include "upload_presets.h"
#include "album_list_dlg.h"

namespace { cfgDialogPosition g_dlg_pos (create_guid (0x42daae47, 0x20c, 0x4c25, 0xba, 0xa1, 0x27, 0x55, 0x7e, 0x75, 0x3d, 0x42)); }

namespace vk_uploader
{
    class upload_setup_dlg : public CDialogWithTooltip<upload_setup_dlg>
    {
        BEGIN_MSG_MAP_EX(upload_setup_dlg)
            MESSAGE_HANDLER_SIMPLE (WM_INITDIALOG, on_init_dialog)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_PICK_ALBUM, on_pick_album)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_SAVE_PRESET, on_save_preset)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_DELETE_PRESET, on_delete_preset)
            COMMAND_HANDLER_SIMPLE(IDC_COMBO_PRESETS, CBN_SELCHANGE, on_preset_changed)
            COMMAND_ID_HANDLER_SIMPLE(IDC_CHECK_POST_ON_WALL, on_post_on_wall_toggle)
            COMMAND_ID_HANDLER_SIMPLE(IDOK, on_ok)
            COMMAND_ID_HANDLER_SIMPLE(IDCANCEL, on_close)
            MSG_WM_CLOSE(on_close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()
        
        void on_init_dialog ()
        {
            g_dlg_pos.AddWindow (*this);

            m_edit_album_name.Attach (GetDlgItem (IDC_EDIT_ALBUM));
            m_combo_presets.Attach (GetDlgItem (IDC_COMBO_PRESETS));
            m_check_post_on_wall.Attach (GetDlgItem (IDC_CHECK_POST_ON_WALL));
            m_edit_post_msg.Attach (GetDlgItem (IDC_EDIT_POST_MESSAGE));

            get_presets_manager ()->for_each_preset ([&](const pfc::string_base & p_name) { uSendMessageText (m_combo_presets, CB_ADDSTRING, 0, p_name); });

            ShowWindow (SW_SHOWNORMAL);
        }

        void on_pick_album ()
        {
            pfc::string8 album_name = album_list_dlg ().get_album_name ();
            if (!album_name.is_empty ())
                m_edit_album_name.InsertText (-1, pfc::stringcvt::string_os_from_utf8 (album_name), FALSE, TRUE);
        }

        void on_save_preset ()
        {
            pfc::string8_fast profile_name = get_window_text_trimmed (m_combo_presets);
            if (!profile_name.is_empty ()) {
                if (get_presets_manager ()->save_preset (profile_name, get_upload_params ())) {
                    if (uSendMessageText (m_combo_presets, CB_FINDSTRINGEXACT, 0, profile_name) == CB_ERR)
                        uSendMessageText (m_combo_presets, CB_ADDSTRING, 0, profile_name);
                }
                else
                    ShowTip (m_combo_presets, L"An error occurred while saving preset");
            }
            else
                ShowTip (m_combo_presets, L"Please enter preset name");
        } 
        
        void on_delete_preset ()
        {
            auto index = m_combo_presets.GetCurSel ();
            if (index != CB_ERR) {
                get_presets_manager ()->delete_preset (string_utf8_from_combobox (m_combo_presets, index));
                m_combo_presets.DeleteString (index);
            }
            else
                ShowTip (m_combo_presets, L"Please select preset to delete");
        }

        
        void on_preset_changed ()
        {
            auto index = m_combo_presets.GetCurSel ();
            if (index != CB_ERR)
                set_current_preset (get_presets_manager ()->get_preset (string_utf8_from_combobox (m_combo_presets, index)));
        }

        void on_post_on_wall_toggle () { m_edit_post_msg.EnableWindow (m_check_post_on_wall.IsChecked ()); }
        void on_ok () { start_upload (m_items, get_upload_params ()); on_close (); }
        void on_close () { DestroyWindow (); }
        void on_destroy () { g_dlg_pos.RemoveWindow (*this); }

        // helpers
        upload_parameters get_upload_params () const
        {
            return make_tuple (get_window_text_trimmed (m_edit_album_name), m_check_post_on_wall.IsChecked (), get_window_text_trimmed (m_edit_post_msg));
        }

        void set_current_preset (const upload_parameters &p)
        {
            m_edit_album_name.SetWindowText (pfc::stringcvt::string_os_from_utf8 (p.get<field_album_name> ()));
            m_check_post_on_wall.ToggleCheck (p.get<field_post_on_wall> ());
            m_edit_post_msg.SetWindowText (pfc::stringcvt::string_os_from_utf8 (p.get<field_post_message> ()));
            m_edit_post_msg.EnableWindow (p.get<field_post_on_wall> ());
        }

        pfc::string8_fast get_window_text_trimmed (HWND wnd) const
        {
            return ::IsWindow (wnd) ? trim (string_utf8_from_window (wnd).get_ptr ()) : "";
        }

        // member variables
        CComboBox m_combo_presets;
        CCheckBox m_check_post_on_wall;
        CEdit     m_edit_post_msg, m_edit_album_name;

        metadb_handle_list m_items;
    public:
        enum { IDD = IDD_UPLOAD_SETUP };

        upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}
    };
}

void show_upload_setup_dialog (metadb_handle_list_cref p_items)
{
    new CWindowAutoLifetime<ImplementModelessTracking<vk_uploader::upload_setup_dlg>>(core_api::get_main_window (), p_items);
}