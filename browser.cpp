#include "stdafx.h"

namespace vk_uploader
{
    namespace
    {
        static CComModule g_module;

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
    void browser_dialog::on_init_dialog ()
    {
        // Init resizing
        DlgResize_Init ();
        m_pos.AddWindow (*this);

        // query IWebBrowser2 interface
        CAxWindow ie = GetDlgItem (IDC_IE);
        if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
            close ();
        else {
            SetWindowText (pfc::stringcvt::string_os_from_utf8 (m_title));
            navigate (m_inital_location);
            ShowWindow (SW_SHOWNORMAL);
        }
    }   

    void __stdcall browser_dialog::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
    {
        m_current_location = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);
        m_callback (this);
    }

    cfgDialogPosition browser_dialog::m_pos (guid_inline<0xcc80a64a, 0x45de, 0x4735, 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23>::guid);
}