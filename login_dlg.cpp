#include "stdafx.h"

#include "login_dlg.h"

namespace vk_uploader
{
    namespace
    {
        static CComModule g_module;

        class myinitquit : public initquit
        {
            void on_init () override { g_module.Init (NULL, NULL, &LIBID_ATLLib); /*login_dlg::show (login_dlg::action_do_login);*/ }
            void on_quit () override { g_module.Term (); }
        };
        static initquit_factory_t<myinitquit> g_initquit;
    }

    //
    // login_dlg
    //
    pfc::string8 login_dlg::show (t_login_dialog_action action)
    {
        login_dlg login_dialog (action);
        return login_dialog.get_browser_location ();
    }

    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        // query IWebBrowser2 interface
        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;
                    
        if (m_action == action_do_login)
            navigate (VK_COM_LOGIN_URL);
        else if (m_action == action_do_relogin)
            navigate (VK_COM_LOGOUT_URL);
       
        ShowWindow (SW_SHOWDEFAULT);
        return TRUE;
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_current_location = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);

        if (m_current_location.find_first (VK_COM_BLANK_URL) == 0 ||
            m_current_location.find_first ("cancel=1") != pfc_infinite) {
            on_close ();
            return;
        }
            
        if (m_action == action_do_relogin) {
            m_action = action_do_login;
            navigate (VK_COM_LOGIN_URL);
        }
    }

    cfgDialogPosition login_dlg::m_pos (guid_inline<0xcc80a64a, 0x45de, 0x4735, 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23>::guid);
}