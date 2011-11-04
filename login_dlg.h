#ifndef _FOO_VK_UPLOADER_LOGIN_DLG_H_
#define _FOO_VK_UPLOADER_LOGIN_DLG_H_

namespace vk_uploader
{
    class login_dlg :
        public CAxDialogImpl<login_dlg>,
        public CDialogResize<login_dlg>,
        public IDispEventImpl<IDC_IE, login_dlg>
    {
    public:
        enum t_login_dialog_action { action_do_login, action_do_relogin };

        enum { IDD = IDD_LOGIN };

        // returns last browser location
        static pfc::string8 show (t_login_dialog_action action);
        
    private:
        login_dlg (t_login_dialog_action action): m_action (action) { DoModal (core_api::get_main_window ()); }

        // helpers
        void navigate (const char *to) { m_current_location.reset (); if (m_wb2) { m_wb2->Navigate (CComBSTR (to), nullptr, nullptr, nullptr, nullptr);} }
        const pfc::string8 & get_browser_location () const { return m_current_location; }


        // message maps
        BEGIN_MSG_MAP_EX(login_dlg)
            CHAIN_MSG_MAP(CAxDialogImpl<login_dlg>)
            CHAIN_MSG_MAP(CDialogResize<login_dlg>)
            MSG_WM_INITDIALOG(on_init_dialog)
            MSG_WM_CLOSE(on_close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        BEGIN_SINK_MAP(login_dlg)
            SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, on_navigate_complete2)
        END_SINK_MAP()

        // dialog resize
        BEGIN_DLGRESIZE_MAP(login_dlg)
            DLGRESIZE_CONTROL(IDC_IE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        END_DLGRESIZE_MAP()
        

        // message handlers
        // dialog
        LRESULT on_init_dialog (CWindow, LPARAM);
        void login_dlg::on_destroy () { m_pos.RemoveWindow (*this); if (m_wb2) m_wb2.Release (); }
        void on_close () { EndDialog (IDOK);}
        // web browser control
        void __stdcall on_navigate_complete2 (IDispatch*, VARIANT *p_url);


        // member variables
        pfc::string8          m_current_location;
        CComPtr<IWebBrowser2> m_wb2;
        t_login_dialog_action m_action;

        static cfgDialogPosition m_pos;
    };
}
#endif