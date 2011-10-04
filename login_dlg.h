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
        enum { IDD = IDD_LOGIN };

        pfc::string8 get_final_url () const { return m_final_url; }

    private:
        BEGIN_MSG_MAP_EX(login_dlg)
        CHAIN_MSG_MAP(CAxDialogImpl<login_dlg>)
        CHAIN_MSG_MAP(CDialogResize<login_dlg>)
        MSG_WM_INITDIALOG(on_init_dialog)
        MSG_WM_CLOSE(close)
        MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(login_dlg)
            DLGRESIZE_CONTROL(IDC_IE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        END_DLGRESIZE_MAP()

        BEGIN_SINK_MAP(login_dlg)
            SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, on_navigate_complete2)
        END_SINK_MAP()

        LRESULT on_init_dialog (CWindow, LPARAM);

        void __stdcall on_navigate_complete2 (IDispatch*, VARIANT *p_url);

        void on_destroy () { m_pos.RemoveWindow (*this); }

        void close () { EndDialog (IDOK); }

        void navigate (const char *to);

        pfc::string8 m_final_url;
        CComPtr<IWebBrowser2> m_wb2;

        static const GUID guid_dialog_pos;
        static cfgDialogPosition m_pos;
    };
}
#endif