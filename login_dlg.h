#ifndef _FOO_VK_UPLOADER_LOGIN_DLG_H_
#define _FOO_VK_UPLOADER_LOGIN_DLG_H_
#endif

namespace vk_uploader
{
    class login_dlg :
        public CAxDialogImpl<login_dlg>,
        public CDialogResize<login_dlg>,
        public IDispEventImpl<IDC_IE, login_dlg>
    {
    public:
	    enum { IDD = IDD_LOGIN };

        bool show ();

        pfc::string8 get_final_url () const { return m_final_url; }

    private:
        BEGIN_MSG_MAP_EX(login_dlg)
            CHAIN_MSG_MAP(CAxDialogImpl<login_dlg>)
            CHAIN_MSG_MAP(CDialogResize<login_dlg>)
            COMMAND_ID_HANDLER(IDC_BUTTON_RELOAD, on_reload)
            COMMAND_ID_HANDLER(IDC_BUTTON_CLOSE, on_close)
            MSG_WM_INITDIALOG(on_init_dialog)
            MSG_WM_CLOSE(close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CMainDlg)
            DLGRESIZE_CONTROL(IDC_IE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDC_BUTTON_RELOAD, DLSZ_MOVE_X)
            DLGRESIZE_CONTROL(IDC_BUTTON_CLOSE, DLSZ_MOVE_X)
        END_DLGRESIZE_MAP()

        BEGIN_SINK_MAP(login_dlg)
            SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, on_navigate_complete2)
        END_SINK_MAP()

        LRESULT on_init_dialog (CWindow, LPARAM);
        void on_destroy ();

        void __stdcall on_navigate_complete2 (IDispatch*, VARIANT*);

        HRESULT on_reload (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { reload (); return TRUE; }
        HRESULT on_close (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { close (); return TRUE; }


        void reload ();
        void close () { EndDialog (IDOK); }

        pfc::string8 m_final_url;
        CComPtr<IWebBrowser2> m_wb2;
    };
}