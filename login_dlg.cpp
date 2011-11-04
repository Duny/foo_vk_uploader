#include "stdafx.h"

#include "login_dlg.h"

namespace vk_uploader
{
    namespace
    {
        static CAppModule g_module;

        class myinitquit : public initquit
        {
            void on_init () override { g_module.Init (NULL, NULL, &LIBID_ATLLib); }
            void on_quit () override { g_module.Term (); }
        };
        static initquit_factory_t<myinitquit> g_initquit;
    }

    //
    // login_dlg
    //
    pfc::string8 login_dlg::show (t_login_dialog_action action)
    {
        CMessageLoop theLoop;

        g_module.AddMessageLoop (&theLoop);
        login_dlg login_dialog (action);
        g_module.RemoveMessageLoop ();

        return login_dialog.get_browser_location ();
    }

    BOOL login_dlg::PreTranslateMessage (MSG* pMsg)
    {
        // This was stolen from an SDI app using a form view.
        //
        // Pass keyboard messages along to the child window that has the focus.
        // When the browser has the focus, the message is passed to its containing
        // CAxWindow, which lets the control handle the message.

        if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
            (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
            return FALSE;

        HWND hWndCtl = ::GetFocus ();
        if (IsChild (hWndCtl)) {
            // find a direct child of the dialog from the window that has focus
            while (::GetParent (hWndCtl) != m_hWnd)
                hWndCtl = ::GetParent (hWndCtl);

            // give control a chance to translate this message
            if (::SendMessage (hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg) != 0)
                return TRUE;
        }

        // A normal control has the focus, so call IsDialogMessage() so that
        // the dialog shortcut keys work (TAB, etc.)
        return IsDialogMessage (pMsg);
    }


    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = g_module.GetMessageLoop ();
        if (!pLoop) {
            on_close ();
            return FALSE;
        }
        pLoop->AddMessageFilter (this);

        // query IWebBrowser2 interface
        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;
        
        if (m_action == action_do_login)
            navigate (VK_COM_LOGIN_URL);
        else if (m_action == action_do_relogin)
            navigate (VK_COM_LOGOUT_URL);
       
        ShowWindow (SW_SHOWDEFAULT);
        pLoop->Run ();
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