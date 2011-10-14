#include "stdafx.h"

#include "upload_setup_dlg.h"
#include "vk_api.h"

namespace vk_uploader
{
    namespace upload_presets
    {
        namespace strcvt = pfc::stringcvt;

        namespace cfg
        {
            struct album_t
            {
                album_t (const char *title, t_uint32 id) : m_title (title), m_id (id) {}
                album_t (const Json::Value &p_val) : m_title (p_val["title"].asCString ()), m_id (p_val["album_id"].asUInt ()) {}
                album_t () {}

                pfc::string8 m_title;
                t_uint32 m_id;
            };

            FB2K_STREAM_READER_OVERLOAD(album_t) { return stream >> value.m_title >> value.m_id; }
            FB2K_STREAM_WRITER_OVERLOAD(album_t) { return stream << value.m_title << value.m_id; }

            const GUID guid_dialog_pos = { 0x42daae47, 0x20c, 0x4c25, { 0xba, 0xa1, 0x27, 0x55, 0x7e, 0x75, 0x3d, 0x42 } };;
            const GUID guid_albums = { 0x7a5b3e69, 0xe2b0, 0x4bca, { 0x96, 0xca, 0x3c, 0x4b, 0x52, 0x21, 0xd1, 0x86 } };;

            cfgDialogPosition pos (guid_dialog_pos);
            cfg_objList<album_t> albums (guid_albums);
        }


        class upload_setup_dlg : public CDialogWithTooltip<upload_setup_dlg>
        {
            BEGIN_MSG_MAP_EX(upload_setup_dlg)
                MSG_WM_INITDIALOG(on_init_dialog)
                COMMAND_ID_HANDLER(IDCANCEL, on_cancel)
                COMMAND_ID_HANDLER(IDC_BUTTON_SAVE_PRESET, on_save_preset)
                COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_PRESET, on_load_preset)
                COMMAND_ID_HANDLER(IDC_BUTTON_DELETE_PRESET, on_delete_preset)
                COMMAND_ID_HANDLER(IDC_BUTTON_REFRESH_ALBUMS, on_refresh_albums)
                MSG_WM_CLOSE(close)
                MSG_WM_DESTROY(on_destroy)
            END_MSG_MAP()

            LRESULT on_init_dialog (CWindow wndFocus, LPARAM lInitParam)
            {
                cfg::pos.AddWindow (*this);

                m_combo_albums.Attach (GetDlgItem (IDC_COMBO_ALBUMS));
                m_combo_presets.Attach (GetDlgItem (IDC_COMBO_PRESETS));
                m_check_post_on_wall.Attach (GetDlgItem (IDC_CHECK_POST_ON_WALL));

                get_preset_manager ()->for_each_preset ([&](const pfc::string8 &p_name) { combo_add_string (m_combo_presets, p_name); });

                combo_albums_reset ();
                cfg::albums.for_each ([&](const cfg::album_t &p_album) { combo_albums_add (p_album); });

                ShowWindow (SW_SHOWNORMAL);
                return 0;
            }

            HRESULT on_save_preset (WORD, WORD, HWND, BOOL&)
            {
                pfc::string8 profile_name = get_window_text_trimmed (m_combo_presets);
                if (!profile_name.is_empty ()) {
                    if (get_preset_manager ()->save_preset (profile_name, get_current_preset ())) {
                        strcvt::string_os_from_utf8 p_name (profile_name);
                        if (m_combo_presets.FindStringExact (0, p_name) == CB_ERR)
                            m_combo_presets.AddString (p_name);
                    }
                }
                else
                    ShowTip (m_combo_presets, L"Please enter preset name");

                return TRUE;
            } 

            HRESULT on_load_preset (WORD, WORD, HWND, BOOL&)
            {
                auto index = m_combo_presets.GetCurSel ();
                if (index != CB_ERR) {
                    try {
                        set_current_preset (get_preset_manager ()->get_preset (string_utf8_from_combo (m_combo_presets, index)));
                    } catch (exception_preset_not_found) { }
                }
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
                class album_selection_restorer
                {
                    upload_setup_dlg &m_dlg;
                    t_uint32 m_id;
                public:
                    album_selection_restorer (upload_setup_dlg &dlg) : m_dlg (dlg), m_id (dlg.get_current_album_by_id ()) {}
                    ~album_selection_restorer () { m_dlg.set_current_album_by_id (m_id); }
                };

                response_json result = get_api_provider ()->call_api ("audio.getAlbums", url_params ("count", "100"));
                if (result.is_valid ()) {
                    album_selection_restorer restorer (*this);

                    combo_albums_reset ();
                    cfg::albums.remove_all ();
                    try {
                        const Json::Value &items = result->get ("response", Json::nullValue);
                        for (t_size n = items.size (), i = 1; i < n; ++i)
                            cfg::albums.add_item (combo_albums_add (items[i]));
                    } catch (const std::exception &) {
                        ShowTip (m_combo_albums, L"Unexpected json in album list");
                    }
                }
                else
                    ShowTip (m_combo_albums, strcvt::string_os_from_utf8 (result.get_error ()));

                return TRUE;
            }

            HRESULT on_cancel (WORD, WORD, HWND, BOOL&) { close (); return TRUE; }

            void close () { DestroyWindow (); }

            void on_destroy () { cfg::pos.RemoveWindow (*this); }

            inline preset get_current_preset () const { return preset (get_current_album_by_id (), m_check_post_on_wall.IsChecked ()); }

            void set_current_preset (const preset &p)
            {
                m_check_post_on_wall.ToggleCheck (p.m_post_on_wall);
                set_current_album_by_id (p.m_album);
            }

            t_uint32 get_current_album_by_id () const
            {
                auto index = m_combo_albums.GetCurSel ();
                return (index != 0 && index != CB_ERR) ? (t_uint32)m_combo_albums.GetItemData (index) : pfc_infinite;
            }

            void set_current_album_by_id (t_uint32 id)
            {
                m_combo_albums.SetCurSel (CB_ERR);
                for (auto n = m_combo_albums.GetCount (), i = 0; i < n; i++) {
                    if (id == (t_uint32)m_combo_albums.GetItemData (i)) {
                        m_combo_albums.SetCurSel (i);
                        break;
                    }
                }
            }

            inline int combo_add_string (CComboBox &p_combo, const char *p_str)
            { return p_combo.AddString (pfc::stringcvt::string_os_from_utf8 (p_str)); }

            inline const cfg::album_t& combo_albums_add (const cfg::album_t &p_album)
            {
                auto index = combo_add_string (m_combo_albums, p_album.m_title);
                if (index != CB_ERR) m_combo_albums.SetItemData (index, (DWORD_PTR)p_album.m_id);
                return p_album;
            }

            inline void combo_albums_reset ()
            {
                m_combo_albums.ResetContent ();
                m_combo_albums.AddString (L"");
            }

            CComboBox m_combo_albums, m_combo_presets;
            CCheckBox m_check_post_on_wall;

            metadb_handle_list_cref m_items;
            //pfc::list_t<profile> m_presets;

        public:
            enum { IDD = IDD_UPLOAD_SETUP };

            upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}
        };

        namespace
        {
            class myinitquit : public initquit
            {
                void on_init () override
                {
                    static_api_ptr_t<upload_setup_dialog>()->show (metadb_handle_list ());
            
                }
                void on_quit () override {}
            public:
                myinitquit () {}
            };
            static initquit_factory_t<myinitquit> g_initquit;
        }

        class upload_setup_dialog_imp : public upload_setup_dialog
        {
            void show (metadb_handle_list_cref p_items) override
            {
                new CWindowAutoLifetime<ImplementModelessTracking<upload_setup_dlg>>(core_api::get_main_window (), p_items);
            }
        };
        static service_factory_single_t<upload_setup_dialog_imp> g_upload_dialog_factory;
    }
}