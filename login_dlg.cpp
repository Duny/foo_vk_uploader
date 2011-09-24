#include "stdafx.h"

#include "login_dlg.h"
#include "vk_api.h"
#include "config.h"

namespace vk_uploader
{
    class myinitquit : public initquit
    {
        void on_init () override
        {
            login_dlg ().show ();
        }
        void on_quit () override {}
    public:
        myinitquit () {}
    };
    static initquit_factory_t<myinitquit> g_initquit;


    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        cfg::dialog_pos.AddWindow (*this);

        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;

        reload ();

	    return TRUE;
    }

    bool login_dlg::show ()
    {
        DoModal (core_api::get_main_window ());
	    return !m_final_url.is_empty ();
    }
    
    void login_dlg::on_destroy ()
    {
        cfg::dialog_pos.RemoveWindow (*this);
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_final_url = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);

        // if address starts from redirect url that means 
        // user was successfully authorized and we can close dialog window
        if (m_final_url.find_first (vk_api::redirect_url) == 0)
            close ();
    }

    void login_dlg::reload ()
    {
        CComVariant v;
        m_wb2->Navigate (CComBSTR (vk_api::oauth_url), &v, &v, &v, &v);
        m_final_url.reset ();
    }
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