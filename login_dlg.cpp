#include "stdafx.h"

#include "config.h"
#include "login_dlg.h"
#include "vk_api.h"

namespace vk_uploader
{
    namespace
    {
        class myinitquit : public initquit
        {
            void on_init () override
            {
                static_api_ptr_t<login_dialog>()->show ();
            }
            void on_quit () override {}
        public:
            myinitquit () {}
        };
        static initquit_factory_t<myinitquit> g_initquit;
    }

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
            COMMAND_ID_HANDLER(IDC_BUTTON_RELOAD, on_reload)
            COMMAND_ID_HANDLER(IDC_BUTTON_CLOSE, on_close)
            MSG_WM_INITDIALOG(on_init_dialog)
            MSG_WM_CLOSE(close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(login_dlg)
            DLGRESIZE_CONTROL(IDC_IE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDC_BUTTON_RELOAD, DLSZ_MOVE_X)
            DLGRESIZE_CONTROL(IDC_BUTTON_CLOSE, DLSZ_MOVE_X)
        END_DLGRESIZE_MAP()

        BEGIN_SINK_MAP(login_dlg)
            SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, on_navigate_complete2)
        END_SINK_MAP()

        LRESULT on_init_dialog (CWindow, LPARAM);
        void on_destroy () { cfg::login_dialog_pos.RemoveWindow (*this); }

        void __stdcall on_navigate_complete2 (IDispatch*, VARIANT*);

        HRESULT on_reload (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { reload (); return TRUE; }
        HRESULT on_close (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { close (); return TRUE; }


        void reload ()
        {
            CComVariant v;
            m_wb2->Navigate (CComBSTR (vk_api::oauth_url), &v, &v, &v, &v);
            m_final_url.reset ();
        }

        void close () { EndDialog (IDOK); }

        pfc::string8 m_final_url;
        CComPtr<IWebBrowser2> m_wb2;
    };

    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        cfg::login_dialog_pos.AddWindow (*this);

        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;

        //reload ();

	    return TRUE;
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_final_url = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);

        // if address starts from redirect url that means 
        // user was successfully authorized and we can close dialog window
        if (m_final_url.find_first (vk_api::redirect_url) == 0)
            close ();
    }
    
    class login_dialog_imp : public login_dialog
    {
        void show () override
        {
            login_dlg ().DoModal (core_api::get_main_window ());
        }
    };
    static service_factory_single_t<login_dialog_imp> g_login_dialog_factory;

    //HRESULT login_dlg::NavigateComplete (_bstr_t URL)
    //{
    //    pfc::string8 url (URL);
    //    str2str_map params;

    //    if (!vk_api::get_url_params (url, params)) {
		  //  console::formatter () << "foo_vk_uploader: can't parse \"" << url << "\"";
		  //  return S_OK;
	   // }

    //    if (params.have_item (pfc::string8 ("error")) || params.have_item (pfc::string8 ("cancel"))) {
    //        OnClose ();
		  //  return S_OK;
    //    }

    //    if (!vk_api::is_redirect_url (url)) // url should start from redirect address
		  //  return S_OK;

	   // if (!params.have_item (pfc::string8 ("access_token"))) {
		  //  console::formatter () << "foo_vk_uploader: access_token not found in responce";
		  //  return S_OK;
	   // }

	   // vk_access_token = params[pfc::string8 ("access_token")];
	   // m_result = true;
	   // EndDialog (TRUE);

    //    return S_OK;
    //}
}