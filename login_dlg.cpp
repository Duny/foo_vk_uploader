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

    login_dlg::login_dlg (t_login_action action) : m_action (action)
    {
        m_login_url << "http://api.vk.com/oauth/authorize?display=popup&scope=audio,wall&response_type=token&redirect_uri=" << g_blank_html << "&client_id=" << vk_api::app_id;
        m_logout_url << "http://api.vk.com/oauth/logout?client_id=" << vk_api::app_id;

        DoModal (core_api::get_main_window ());
    }

    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;

        pfc::string_formatter login_url, logout_url;
        
        if (m_action == action_login)
            navigate (m_login_url);
        else if (m_action == action_relogin)
            navigate (m_logout_url);
       
        return TRUE;
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_current_location = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);

        if (is_blank_page (m_current_location))
            close ();
            
        if (m_action == action_relogin) {
            m_action = action_login;
            navigate (m_login_url);
        }
    }

    void login_dlg::navigate (const char *to)
    {
        m_current_location.reset ();
        m_wb2->Navigate (CComBSTR (to), nullptr, nullptr, nullptr, nullptr);
    }

    cfgDialogPosition login_dlg::m_pos (guid_inline<0xcc80a64a, 0x45de, 0x4735, 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23>::guid);
}