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

    cfgDialogPosition g_dlg_pos (create_guid (0xd3c77d31, 0xdde8, 0x41f0, 0x96, 0x47, 0x63, 0xb1, 0xad, 0x13, 0x29,0x48));
}

using namespace vk_uploader;

void browser_dialog::on_init_dialog ()
{
    // Init resizing
    DlgResize_Init ();
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