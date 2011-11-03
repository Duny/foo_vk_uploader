#include "stdafx.h"

#include "boost/function.hpp"

#include "vk_api.h"
#include "upload_preset.h"
#include "upload_queue.h"

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

            inline const t_audio_album_info& add_album (const t_audio_album_info &p_album)
            {
                if (IsWindow ()) {
                    auto index = AddString (pfc::stringcvt::string_os_from_utf8 (p_album.first));
                    if (index != CB_ERR) SetItemData (index, static_cast<DWORD_PTR>(p_album.second));
                }
                return p_album;
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
            MSG_WM_INITDIALOG(on_init_dialog)
            COMMAND_ID_HANDLER(IDOK, on_ok)
            COMMAND_ID_HANDLER(IDCANCEL, on_cancel)
            COMMAND_ID_HANDLER(IDC_BUTTON_SAVE_PRESET, on_save_preset)
            COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_PRESET, on_load_preset)
            COMMAND_ID_HANDLER(IDC_BUTTON_DELETE_PRESET, on_delete_preset)
            COMMAND_ID_HANDLER(IDC_BUTTON_REFRESH_ALBUMS, on_refresh_albums)
            COMMAND_ID_HANDLER(IDC_BUTTON_ALBUM_NEW, on_album_new)
            COMMAND_ID_HANDLER(IDC_BUTTON_ALBUM_DELETE, on_album_delete)
            MSG_WM_CLOSE(close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        inline HRESULT on_ok (WORD, WORD, HWND, BOOL&) { upload_queue::push_back (m_items, get_upload_params ()); close (); return TRUE; }

        inline HRESULT on_cancel (WORD, WORD, HWND, BOOL&) { close (); return TRUE; }

        LRESULT on_init_dialog (CWindow wndFocus, LPARAM lInitParam)
        {
            m_pos.AddWindow (*this);

            m_combo_albums.Attach (GetDlgItem (IDC_COMBO_ALBUMS));
            m_combo_presets.Attach (GetDlgItem (IDC_COMBO_PRESETS));
            m_check_post_on_wall.Attach (GetDlgItem (IDC_CHECK_POST_ON_WALL));
            m_edit_post_msg.Attach (GetDlgItem (IDC_EDIT_POST_MESSAGE));

            m_combo_albums.init ();
            m_albums.for_each ([&](const t_audio_album_info &p_album) { m_combo_albums.add_album (p_album); });

            get_preset_manager ()->for_each_preset ([&](const pfc::string8 &p_name) { combo_add_string (m_combo_presets, p_name); });

            ShowWindow (SW_SHOWNORMAL);
            return 0;
        }

        HRESULT on_save_preset (WORD, WORD, HWND, BOOL&)
        {
            pfc::string8_fast profile_name = get_window_text_trimmed (m_combo_presets);
            if (!profile_name.is_empty ()) {
                if (get_preset_manager ()->save_preset (profile_name, get_upload_params ())) {
                    pfc::stringcvt::string_os_from_utf8 p_name (profile_name);
                    if (m_combo_presets.FindStringExact (0, p_name) == CB_ERR)
                        m_combo_presets.AddString (p_name);
                }
                else
                    ShowTip (m_combo_presets, L"An error occurred while saving preset");
            }
            else
                ShowTip (m_combo_presets, L"Please enter preset name");

            return TRUE;
        } 

        HRESULT on_load_preset (WORD, WORD, HWND, BOOL&)
        {
            auto index = m_combo_presets.GetCurSel ();
            if (index != CB_ERR)
                set_current_preset (get_preset_manager ()->get_preset (string_utf8_from_combo (m_combo_presets, index)));
            else
                ShowTip (m_combo_presets, L"Please select preset to load");

            return TRUE;
        }

        HRESULT on_delete_preset (WORD, WORD, HWND, BOOL&)
        {
            auto index = m_combo_presets.GetCurSel ();
            if (index != CB_ERR) {
                get_preset_manager ()->delete_preset (string_utf8_from_combo (m_combo_presets, index));
                m_combo_presets.DeleteString (index);
            }
            else
                ShowTip (m_combo_presets, L"Please select preset to delete");

            return TRUE;
        }

        HRESULT on_refresh_albums (WORD, WORD, HWND, BOOL&)
        {
            run_in_separate_thread ([this] ()
            {
                api_audio_getAlbums album_list;
                if (album_list.is_valid ()) {
                    m_albums.remove_all ();
                    m_combo_albums.init ();
                    for (t_size i = 0, n = album_list.get_count (); i < n; i++)
                        m_albums.add_item (m_combo_albums.add_album (album_list[i]));
                }
                else
                    uMessageBox (core_api::get_main_window (), album_list.get_error (), "Error while reading album list", MB_OK | MB_ICONERROR);
            });

            return TRUE;
        }

        HRESULT on_album_new (WORD, WORD, HWND, BOOL&)
        {
            run_in_separate_thread ([this] ()
            {
                pfc::string8_fast album_title = get_window_text_trimmed (m_combo_albums);
                if (!album_title.is_empty ()) {
                    api_audio_addAlbum result (album_title);
                    if (result.is_valid ())
                        m_albums.add_item (m_combo_albums.add_album (std::make_pair (album_title, result.get_album_id ())));
                    else
                        uMessageBox (core_api::get_main_window (), result.get_error (), "Error while creating new album", MB_OK | MB_ICONERROR);
                }
            });
            
            return TRUE;
        }

        HRESULT on_album_delete (WORD, WORD, HWND, BOOL&)
        {
            auto index = m_combo_albums.GetCurSel ();
            if (index == CB_ERR)
                ShowTip (m_combo_albums, L"Please select album to delete");
            else if (index > 0) { // item with index 0 is reserved for empty album
                t_audio_album_info to_delete = std::make_pair (string_utf8_from_combo (m_combo_albums, index), get_current_album_id ());
                pfc::string8_fast message ("Are you sure what you want to delete album \"");
                message += to_delete.first; message += "\"?";
                int dlg_result = uMessageBox (*this, message, COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION);
                if (dlg_result == IDYES) {
                    api_audio_deleteAlbum result (to_delete.second);
                    if (result.is_valid ()) {
                        m_albums.remove_item (to_delete);
                        m_combo_albums.DeleteString (index);
                    }
                    else
                        ShowTip (m_combo_albums, pfc::stringcvt::string_os_from_utf8 (result.get_error ()));
                }
            }

            return TRUE;
        }

        inline void close () { DestroyWindow (); }

        inline void on_destroy () { m_pos.RemoveWindow (*this); }

        inline upload_params get_upload_params () const
        {
            return upload_params (get_current_album_id (), m_check_post_on_wall.IsChecked (), get_window_text_trimmed (m_edit_post_msg));
        }

        inline void set_current_preset (const upload_params &p)
        {
            set_current_album_by_id (p.m_album_id);
            m_check_post_on_wall.ToggleCheck (p.m_post_on_wall);
            m_edit_post_msg.SetWindowText (pfc::stringcvt::string_os_from_utf8 (p.m_post_mgs));
        }

        inline t_album_id get_current_album_id () const
        {
            auto index = m_combo_albums.GetCurSel ();
            return index != CB_ERR ? static_cast<t_album_id>(m_combo_albums.GetItemData (index)) : 0;
        }

        inline int combo_add_string (CComboBox &p_combo, const char *p_str) { return p_combo.AddString (pfc::stringcvt::string_os_from_utf8 (p_str)); }

        inline void set_current_album_by_id (t_album_id id)
        {
            m_combo_albums.SetCurSel (CB_ERR);
            for (auto n = m_combo_albums.GetCount (), i = 0; i < n; i++) {
                if (id == static_cast<t_album_id>(m_combo_albums.GetItemData (i))) {
                    m_combo_albums.SetCurSel (i);
                    return;
                }
            }
        }

        combobox_album_list m_combo_albums;
        CComboBox m_combo_presets;
        CCheckBox m_check_post_on_wall;
        CEditNoEnterEscSteal m_edit_post_msg;

        metadb_handle_list m_items;

        static cfgDialogPosition m_pos;
        static cfg_objList<t_audio_album_info> m_albums;
    public:
        enum { IDD = IDD_UPLOAD_SETUP };

        static void clear_album_list () { m_albums.remove_all (); }

        upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}
    };

    cfgDialogPosition upload_setup_dlg::m_pos (guid_inline<0x42daae47, 0x20c, 0x4c25, 0xba, 0xa1, 0x27, 0x55, 0x7e, 0x75, 0x3d, 0x42>::guid);
    cfg_objList<t_audio_album_info> upload_setup_dlg::m_albums (guid_inline<0x7a5b3e69, 0xe2b0, 0x4bca, 0x96, 0xca, 0x3c, 0x4b, 0x52, 0x21, 0xd1, 0x86>::guid);

    namespace
    {
        class myinitquit : public initquit
        {
            void on_init () override { show_upload_setup_dialog (); }
            void on_quit () override {}
        };
        //static initquit_factory_t<myinitquit> g_initquit;
    }
}

void show_upload_setup_dialog (metadb_handle_list_cref p_items)
{
    new CWindowAutoLifetime<ImplementModelessTracking<vk_uploader::upload_setup_dlg>>(core_api::get_main_window (), p_items);
}

void clear_album_list ()
{
    vk_uploader::upload_setup_dlg::clear_album_list ();
}
