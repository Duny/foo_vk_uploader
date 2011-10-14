#include "stdafx.h"

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
                login_dlg dlg;
                dlg.DoModal (core_api::get_main_window ()); 
            }
            void on_quit () override {}
        public:
            myinitquit () {}
        };
        //static initquit_factory_t<myinitquit> g_initquit;
    }

    LRESULT login_dlg::on_init_dialog (CWindow, LPARAM)
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            return FALSE;

        pfc::string8 auth_url = "http://api.vk.com/oauth/authorize?display=popup&scope=audio&response_type=token&redirect_uri=http://api.vk.com/blank.html&client_id=";
        auth_url += vk_api::app_id;
        navigate (auth_url);
        //navigate ("vk.com");
        return TRUE;
    }

    void __stdcall login_dlg::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_final_url = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);
        if (m_final_url.find_first ("http://api.vk.com/blank.html") == 0) close ();
    }

    void login_dlg::navigate (const char *to)
    {
        m_final_url.reset ();
        m_wb2->Navigate (CComBSTR (to), nullptr, nullptr, nullptr, nullptr);
    }

    const GUID login_dlg::guid_dialog_pos = { 0xcc80a64a, 0x45de, 0x4735, { 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23 } };
    cfgDialogPosition login_dlg::m_pos (guid_dialog_pos);
}