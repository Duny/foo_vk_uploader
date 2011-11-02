#include "stdafx.h"

#include "login_dlg.h"
#include "vk_api.h"

namespace vk_uploader
{
    namespace
    {
        class myinitquit : public initquit
        {
            CComModule m_module;
            void on_init () override
            {
                m_module.Init (NULL, NULL, &LIBID_ATLLib);
                //login_dlg dlg;
            }
            void on_quit () override { m_module.Term (); }
        };
        static initquit_factory_t<myinitquit> g_initquit;
    }

    

    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;
        
        if (m_action == action_do_login)
            navigate (VK_COM_LOGIN_URL);
        else if (m_action == action_do_relogin)
            navigate (VK_COM_LOGOUT_URL);
       
        return TRUE;
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_current_location = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);

        if (m_current_location.find_first (VK_COM_BLANK_URL) == 0) {
            close ();
            return;
        }
            
        if (m_action == action_do_relogin) {
            m_action = action_do_login;
            navigate (VK_COM_LOGIN_URL);
        }
    }

    void login_dlg::navigate (const char *to)
    {
        m_current_location.reset ();
        m_wb2->Navigate (CComBSTR (to), nullptr, nullptr, nullptr, nullptr);
    }

    cfgDialogPosition login_dlg::m_pos (guid_inline<0xcc80a64a, 0x45de, 0x4735, 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23>::guid);
}