#include "stdafx.h"

#include "boost/function.hpp"

FB2K_STREAM_READER_OVERLOAD(audio_album_info) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(audio_album_info) { return write_tuple (stream, value); }

namespace vk_uploader
{
    using namespace upload_presets;

    class upload_setup_dlg : public CDialogWithTooltip<upload_setup_dlg>
    {
        class combobox_album_list : public CComboBox
        {
        public:
            inline void init ()
            {
                ResetContent ();
                AddString (L"");
                SetItemData (0, 0);
            }

            inline const audio_album_info& add_album (const audio_album_info &p_album)
            {
                if (IsWindow ()) {
                    auto index = uSendMessageText (*this, CB_ADDSTRING, 0, p_album.get<0> ());
                    if (index != CB_ERR) SetItemData (index, static_cast<DWORD_PTR>(p_album.get<1> ()));
                }
                return p_album;
            }

            inline t_vk_album_id get_selected_album_id () const
            {
                auto index = GetCurSel ();
                return index != CB_ERR ? static_cast<t_vk_album_id>(GetItemData (index)) : 0;
            }

            inline void select_by_id (t_vk_album_id id)
            {
                SetCurSel (CB_ERR);
                for (auto n = GetCount (), i = 0; i < n; i++) {
                    if (id == static_cast<t_vk_album_id>(GetItemData (i))) {
                        SetCurSel (i);
                        return;
                    }
                }
            }
        };

        typedef boost::function<void ()> new_thread_callback;
        void run_in_separate_thread (const new_thread_callback &p_func)
        {
            class new_thread_t : pfc::thread
            {
                new_thread_callback m_func;
                void threadProc () override
                {
                   m_func ();
                   delete this;
                }
                ~new_thread_t () { waitTillDone (); }
            public:
                new_thread_t (const new_thread_callback &p_func) : m_func (p_func) { startWithPriority (THREAD_PRIORITY_BELOW_NORMAL); }
            };

            new new_thread_t (p_func);
        }

        BEGIN_MSG_MAP_EX(upload_setup_dlg)
            MESSAGE_HANDLER_SIMPLE (WM_INITDIALOG, on_init_dialog)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_SAVE_PRESET, on_save_preset)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_DELETE_PRESET, on_delete_preset)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_REFRESH_ALBUMS, on_refresh_albums)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_ALBUM_NEW, on_album_new)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_ALBUM_DELETE, on_album_delete)
            COMMAND_HANDLER_SIMPLE(IDC_COMBO_PRESETS, CBN_SELCHANGE, on_preset_changed)
            COMMAND_ID_HANDLER_SIMPLE(IDC_CHECK_POST_ON_WALL, on_post_on_wall_toggle)
            COMMAND_ID_HANDLER_SIMPLE(IDOK, on_ok)
            COMMAND_ID_HANDLER_SIMPLE(IDCANCEL, on_cancel)
            MSG_WM_CLOSE(on_close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()
        
        void on_init_dialog ()
        {
            m_pos.AddWindow (*this);

            m_combo_albums.Attach (GetDlgItem (IDC_COMBO_ALBUMS));
            m_combo_presets.Attach (GetDlgItem (IDC_COMBO_PRESETS));
            m_check_post_on_wall.Attach (GetDlgItem (IDC_CHECK_POST_ON_WALL));
            m_edit_post_msg.Attach (GetDlgItem (IDC_EDIT_POST_MESSAGE));

            get_preset_manager ()->for_each_preset ([&](const pfc::string8 &p_name)
            {
                uSendMessageText (m_combo_presets, CB_ADDSTRING, 0, p_name);
            });

            m_combo_albums.init ();
            m_albums.for_each ([&](const audio_album_info &p_album) { m_combo_albums.add_album (p_album); });

            m_edit_post_msg.EnableWindow (FALSE);

            ShowWindow (SW_SHOWNORMAL);
        }

        void on_save_preset ()
        {
            pfc::string8_fast profile_name = get_window_text_trimmed (m_combo_presets);
            if (!profile_name.is_empty ()) {
                if (get_preset_manager ()->save_preset (profile_name, get_upload_params ())) {
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
                get_preset_manager ()->delete_preset (string_utf8_from_combo (m_combo_presets, index));
                m_combo_presets.DeleteString (index);
            }
            else
                ShowTip (m_combo_presets, L"Please select preset to delete");
        }

        void on_refresh_albums ()
        {
            run_in_separate_thread ([this] ()
            {
                try {
                    abort_callback_impl p_abort;
                    api_audio_getAlbums album_list (p_abort);
                    m_albums.remove_all ();
                    m_combo_albums.init ();
                    for (t_size i = 0, n = album_list.get_count (); i < n; i++)
                        m_albums.add_item (m_combo_albums.add_album (album_list[i]));
                }
                catch (exception_aborted) {}
                catch (const std::exception &e) {
                    uMessageBox (core_api::get_main_window (), e.what (), "Error while reading album list", MB_OK | MB_ICONERROR);
                }
            });
        }

        void on_album_new ()
        {
            pfc::string8_fast album_title = get_window_text_trimmed (m_combo_albums);
            if (album_title.is_empty ())
                ShowTip (m_combo_albums, L"Please enter album title");
            else {
                run_in_separate_thread ([=, this] ()
                {
                    try {
                        abort_callback_impl p_abort;
                        api_audio_addAlbum result (album_title, p_abort);
                        m_albums.add_item (m_combo_albums.add_album (boost::make_tuple (album_title, result.get_album_id ())));
                    }
                    catch (exception_aborted) {}
                    catch (const std::exception &e) {
                        uMessageBox (core_api::get_main_window (), e.what (), "Error while creating new album", MB_OK | MB_ICONERROR);
                    }
                });
            }
        }

        void on_album_delete ()
        {
            auto index = m_combo_albums.GetCurSel ();
            if (index == CB_ERR)
                ShowTip (m_combo_albums, L"Please select album to delete");
            else if (index > 0) { // item with index 0 is reserved for empty album
                audio_album_info to_delete = boost::make_tuple (string_utf8_from_combo (m_combo_albums, index), m_combo_albums.get_selected_album_id ());
                pfc::string8_fast message ("Are you sure what you want to delete album \"");
                message += to_delete.get<0> (); message += "\"?";
                int dlg_result = uMessageBox (*this, message, COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION);
                if (dlg_result == IDYES) {
                    run_in_separate_thread ([=, this] ()
                    {
                        try {
                            abort_callback_impl p_abort;
                            api_audio_deleteAlbum result (to_delete.get<1> (), p_abort);
                            m_albums.remove_item (to_delete);
                            m_combo_albums.DeleteString (index);
                        }
                        catch (exception_aborted) {}
                        catch (const std::exception &e) {
                            uMessageBox (core_api::get_main_window (), e.what (), "Error while creating new album", MB_OK | MB_ICONERROR);
                        }
                    });
                }
            }
        }

        inline void on_preset_changed ()
        {
            auto index = m_combo_presets.GetCurSel ();
            if (index != CB_ERR)
                set_current_preset (get_preset_manager ()->get_preset (string_utf8_from_combo (m_combo_presets, index)));
        }

        inline void on_post_on_wall_toggle ()
        {
            m_edit_post_msg.EnableWindow (m_check_post_on_wall.IsChecked ());
        }
        
        inline void on_ok () { start_upload (m_items, get_upload_params ()); on_close (); }
        inline void on_cancel () { on_close (); }
        inline void on_close () { DestroyWindow (); }
        inline void on_destroy () { m_pos.RemoveWindow (*this); }

        // helpers
        inline upload_parameters get_upload_params () const
        {
            return boost::make_tuple (m_combo_albums.get_selected_album_id (), m_check_post_on_wall.IsChecked (), get_window_text_trimmed (m_edit_post_msg));
        }

        inline void set_current_preset (const upload_parameters &p)
        {
            m_combo_albums.select_by_id (p.get<field_album_id> ());
            m_check_post_on_wall.ToggleCheck (p.get<field_post_on_wall> ());
            m_edit_post_msg.SetWindowText (pfc::stringcvt::string_os_from_utf8 (p.get<field_post_message> ()));
            m_edit_post_msg.EnableWindow (p.get<field_post_on_wall> ());
        }

        inline pfc::string8_fast get_window_text_trimmed (HWND wnd) const
        {
            return ::IsWindow (wnd) ? trim (string_utf8_from_window (wnd).get_ptr ()) : "";
        }

        class string_utf8_from_combo
        {
            pfc::string8 m_data;
        public:
            string_utf8_from_combo (const CComboBox &p_combo, int p_index)
            {
                int str_len = p_combo.GetLBTextLen (p_index);
                if (str_len != CB_ERR) {
                    pfc::array_t<TCHAR> buf;
                    buf.set_size (str_len + 1);
                    int actual_len = p_combo.GetLBText (p_index, buf.get_ptr ());
                    if (actual_len == str_len)
                        m_data = pfc::stringcvt::string_utf8_from_os (buf.get_ptr ());
                }
            }

            inline operator const char * () const { return m_data.get_ptr (); }
            inline operator const pfc::string8& () const { return m_data; }
        };

        // member variables
        combobox_album_list m_combo_albums;
        CComboBox m_combo_presets;
        CCheckBox m_check_post_on_wall;
        CEditNoEnterEscSteal m_edit_post_msg;

        metadb_handle_list m_items;

        static cfgDialogPosition m_pos;
        static cfg_objList<audio_album_info> m_albums;
    public:
        enum { IDD = IDD_UPLOAD_SETUP };

        static void clear_album_list () { m_albums.remove_all (); }

        upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}
    };

    cfgDialogPosition upload_setup_dlg::m_pos (guid_inline<0x42daae47, 0x20c, 0x4c25, 0xba, 0xa1, 0x27, 0x55, 0x7e, 0x75, 0x3d, 0x42>::guid);
    cfg_objList<audio_album_info> upload_setup_dlg::m_albums (guid_inline<0x7a5b3e69, 0xe2b0, 0x4bca, 0x96, 0xca, 0x3c, 0x4b, 0x52, 0x21, 0xd1, 0x86>::guid);
}

void show_upload_setup_dialog (metadb_handle_list_cref p_items)
{
    new CWindowAutoLifetime<ImplementModelessTracking<vk_uploader::upload_setup_dlg>>(core_api::get_main_window (), p_items);
}

void clear_album_list ()
{
    vk_uploader::upload_setup_dlg::clear_album_list ();
}