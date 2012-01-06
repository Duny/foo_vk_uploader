#include "stdafx.h"
#include "browser.h"


namespace
{
    CComModule g_module;

    class myinitquit : public initquit
    {
        void on_init () override { g_module.Init (NULL, NULL, &LIBID_ATLLib); }
        void on_quit () override { g_module.Term (); }
    };
    initquit_factory_t<myinitquit> g_initquit;

    cfgDialogPosition g_dlg_pos (create_guid (0xf7d4e54a, 0x5d5d, 0x482e, 0x89, 0x4a, 0x4f, 0x10, 0x3c, 0x02, 0x50, 0x6a));
}

using namespace vk_uploader;

void browser_dialog::on_init_dialog ()
{
    DlgResize_Init (true, false);
    g_dlg_pos.AddWindow (*this);

    // Query IWebBrowser2 interface
    CAxWindow ie = GetDlgItem (IDC_IE);
    if (ie.IsWindow () != TRUE || ie.QueryControl (&m_wb2) != S_OK)
        close ();
    else {
        SetWindowText (pfc::stringcvt::string_os_from_utf8 (m_title));
        navigate (m_inital_location);
        ShowWindow (SW_SHOWNORMAL);
    }
}   

void browser_dialog::on_destroy ()
{
    g_dlg_pos.RemoveWindow (*this);
    if (m_wb2) m_wb2.Release ();
}

void __stdcall browser_dialog::on_navigate_complete2 (IDispatch*, VARIANT *p_url)
{
    m_current_location = pfc::stringcvt::string_utf8_from_os (p_url->bstrVal);
    m_callback (this);
}