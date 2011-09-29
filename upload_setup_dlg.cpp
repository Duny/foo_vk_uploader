#include "stdafx.h"

#include "upload_setup_dlg.h"
#include "utils.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        namespace strcvt = pfc::stringcvt;

        class upload_setup_dlg : public CAxDialogImpl<upload_setup_dlg>
        {
        public:
	        enum { IDD = IDD_UPLOAD_SETUP };

            upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}

        private:
            BEGIN_MSG_MAP_EX(upload_setup_dlg)
                MSG_WM_INITDIALOG(on_init_dialog)
                COMMAND_ID_HANDLER(IDCANCEL, on_cancel)
                COMMAND_ID_HANDLER(IDC_BUTTON_SAVE_PROFILE, on_save_profile)
                COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_PROFILE, on_load_profile)
                COMMAND_ID_HANDLER(IDC_BUTTON_DELETE_PROFILE, on_delete_profile)
                MSG_WM_CLOSE(close)
                MSG_WM_DESTROY(on_destroy)
            END_MSG_MAP()

            LRESULT on_init_dialog (CWindow wndFocus, LPARAM lInitParam)
            {
                m_pos.AddWindow (*this);

                m_combo_albums.Attach (GetDlgItem (IDC_COMBO_ALBUMS));
                m_combo_profiles.Attach (GetDlgItem (IDC_COMBO_PRESETS));
                m_check_post_on_wall.Attach (GetDlgItem (IDC_CHECK_POST_ON_WALL));

                static_api_ptr_t<upload_profiles::manager> api;
                for (t_size i = 0, n = api->get_profile_count (); i < n; i++)
                    m_combo_profiles.AddString (strcvt::string_os_from_utf8 (api->get_profile_name (i)));

                ShowWindow (SW_SHOWNORMAL);
                return 0;
            }

            HRESULT on_save_profile (WORD, WORD, HWND, BOOL&)
            {
                pfc::string8 profile_name = get_window_text_trimmed (m_combo_profiles);

                if (profile_name.length () > 0) {
                    try {
                        static_api_ptr_t<upload_profiles::manager>()->save_profile (profile_name, get_current_profile ());
                    }
                    catch (std::exception e) {
                        uMessageBox (*this, e.what (), COMPONENT_NAME, MB_OK | MB_ICONERROR);
                        return FALSE;
                    }
                    catch (...) {
                        return FALSE;
                    }

                    strcvt::string_os_from_utf8 p_name (profile_name);
                    if (m_combo_profiles.FindStringExact (0, p_name) == CB_ERR)
                        m_combo_profiles.AddString (p_name);
                }

                return TRUE;
            }

            HRESULT on_load_profile (WORD, WORD, HWND, BOOL&)
            {
                pfc::string8 profile_name = get_window_text_trimmed (m_combo_profiles);
                if (profile_name.length () > 0) {
                    try {
                        profile p = static_api_ptr_t<upload_profiles::manager>()->get_profile (profile_name);

                        auto i = m_combo_albums.FindStringExact (0, strcvt::string_os_from_utf8 (p.m_album));
                        if (i != CB_ERR)
                            m_combo_albums.SetCurSel (i);
                        else
                            m_combo_albums.SetWindowText (strcvt::string_os_from_utf8 (p.m_album));

                        m_check_post_on_wall.ToggleCheck (p.m_post_on_wall);
                    } catch (...) { }
                }

                return TRUE;
            }

            HRESULT on_delete_profile (WORD, WORD, HWND, BOOL&)
            {
                pfc::string8 profile_name = get_window_text_trimmed (m_combo_profiles);
                if (profile_name.length () > 0) {
                    static_api_ptr_t<upload_profiles::manager>()->delete_profile (profile_name);
                }

                return TRUE;
            }

            HRESULT on_cancel (WORD, WORD, HWND, BOOL&) { close (); return TRUE; }

            void close () { DestroyWindow (); }

            void on_destroy () { m_pos.RemoveWindow (*this); }

            profile get_current_profile ()
            {
                profile p;
                p.m_album = get_window_text_trimmed (m_combo_albums);
                p.m_post_on_wall = m_check_post_on_wall.IsChecked ();
                return p;
            }

            CComboBox m_combo_albums, m_combo_profiles;
            CCheckBox m_check_post_on_wall;

            metadb_handle_list_cref m_items;
            pfc::list_t<profile> m_profiles;

            static const GUID guid_dialog_pos;
            static cfgDialogPosition m_pos;
        };
        const GUID upload_setup_dlg::guid_dialog_pos = { 0x42daae47, 0x20c, 0x4c25, { 0xba, 0xa1, 0x27, 0x55, 0x7e, 0x75, 0x3d, 0x42 } };
        cfgDialogPosition upload_setup_dlg::m_pos (guid_dialog_pos);

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