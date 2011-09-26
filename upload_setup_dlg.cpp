#include "stdafx.h"

#include "upload_setup_dlg.h"
#include "config.h"

namespace vk_uploader
{
    class upload_setup_dlg : public CAxDialogImpl<upload_setup_dlg>
    {
    public:
	    enum { IDD = IDD_UPLOAD_SETUP };

        upload_setup_dlg (metadb_handle_list_cref p_items) : m_items (p_items) {}
        ~upload_setup_dlg ()
        {
            int i = 0;
        }

    private:
        BEGIN_MSG_MAP_EX(upload_setup_dlg)
            MSG_WM_INITDIALOG(on_init_dialog)
            MSG_WM_CLOSE(on_close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        LRESULT on_init_dialog (CWindow wndFocus, LPARAM lInitParam)
        {
            cfg::upload_dialog_pos.AddWindow (*this);

            ShowWindow (SW_SHOWNORMAL);
            return 0;
        }

        void on_close () { DestroyWindow (); }

        void on_destroy () { cfg::upload_dialog_pos.RemoveWindow (*this); }

        metadb_handle_list_cref m_items;
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