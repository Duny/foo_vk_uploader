#include "stdafx.h"

#include "upload_setup_dlg.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        class upload_setup_dlg : public CAxDialogImpl<upload_setup_dlg>
        {
        public:
	        enum { IDD = IDD_UPLOAD_SETUP };

            upload_setup_dlg (metadb_handle_list_cref p_items, const upload_profiles::profile &p_profile) 
                : m_items (p_items), m_profile (p_profile) {}

        private:
            BEGIN_MSG_MAP_EX(upload_setup_dlg)
                MSG_WM_INITDIALOG(on_init_dialog)
                COMMAND_ID_HANDLER(IDCANCEL, on_cancel)
                MSG_WM_CLOSE(close)
                MSG_WM_DESTROY(on_destroy)
            END_MSG_MAP()

            LRESULT on_init_dialog (CWindow wndFocus, LPARAM lInitParam)
            {
                m_pos.AddWindow (*this);

                ShowWindow (SW_SHOWNORMAL);
                return 0;
            }

            HRESULT on_cancel (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { close (); return TRUE; }

            void close () { DestroyWindow (); }

            void on_destroy () { m_pos.RemoveWindow (*this); }

            metadb_handle_list_cref m_items;
            upload_profiles::profile m_profile;

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
            void show (metadb_handle_list_cref p_items, const upload_profiles::profile &p_profile) override
            {
                new CWindowAutoLifetime<ImplementModelessTracking<upload_setup_dlg>>(core_api::get_main_window (), p_items, p_profile);
            }
        };
        static service_factory_single_t<upload_setup_dialog_imp> g_upload_dialog_factory;
    }
}