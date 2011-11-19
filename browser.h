#ifndef _FOO_VK_UPLOADER_BROWSER_H_
#define _FOO_VK_UPLOADER_BROWSER_H_

namespace vk_uploader
{
    class browser_dialog;
    typedef boost::function<void (browser_dialog *p_dlg)> on_navigate_complete_callback_t;

    class browser_dialog :
        public CAxDialogImpl<browser_dialog>,
        public CDialogResize<browser_dialog>,
        public IDispEventImpl<IDC_IE, browser_dialog>
    {
    public:
        enum { IDD = IDD_LOGIN };

        browser_dialog (const char *url, const on_navigate_complete_callback_t &p_callback)
            : m_inital_location (url), m_callback (p_callback) {}

        void show () { DoModal (core_api::get_main_window ()); } 
        void close () { EndDialog (IDOK); }

        void navigate (const char *url) { m_current_location.reset (); if (m_wb2) { m_wb2->Navigate (CComBSTR (url), nullptr, nullptr, nullptr, nullptr); } }
        const pfc::string_base &get_browser_location () const { return m_current_location; }

    private:
        // message maps
        BEGIN_MSG_MAP_EX(browser_dialog)
            CHAIN_MSG_MAP(CAxDialogImpl<browser_dialog>)
            CHAIN_MSG_MAP(CDialogResize<browser_dialog>)
            MESSAGE_HANDLER_SIMPLE(WM_INITDIALOG, on_init_dialog)
            MSG_WM_TIMER(on_timer)
            MSG_WM_CLOSE(close)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        BEGIN_SINK_MAP(browser_dialog)
            SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, on_navigate_complete2)
        END_SINK_MAP()

        // dialog resize
        BEGIN_DLGRESIZE_MAP(browser_dialog)
            DLGRESIZE_CONTROL(IDC_IE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        END_DLGRESIZE_MAP()
        

        // message handlers
        // dialog
        void on_init_dialog ();
        void on_destroy () { m_pos.RemoveWindow (*this); if (m_wb2) m_wb2.Release (); }
        void on_timer (UINT_PTR) { KillTimer (m_timer_id); m_callback (this); }

        // web browser control
        void __stdcall on_navigate_complete2 (IDispatch*, VARIANT *p_url);

        // member variables
        pfc::string8 m_inital_location, m_current_location;
        CComPtr<IWebBrowser2>           m_wb2;
        on_navigate_complete_callback_t m_callback;
        UINT_PTR                        m_timer_id;

        static cfgDialogPosition m_pos;
    };

    // must be called from main thread
    inline void open_browser_dialog (const char *url, const on_navigate_complete_callback_t &p_nav_callback)
    {
        browser_dialog dialog (url, p_nav_callback);
        dialog.show ();
    }
}
#endif