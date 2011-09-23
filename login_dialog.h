#pragma once

class vk_login_dialog : public CAxDialogImpl<vk_login_dialog>
{
public:
	enum { IDD = IDD_LOGIN_DLG };

    vk_login_dialog () : m_size_tracker (vk_login_dlg_size), m_position_tracker (vk_login_dlg_pos) {}

    bool show ();

	BEGIN_MSG_MAP_EX(vk_login_dialog)
        CHAIN_MSG_MAP_MEMBER (m_size_tracker);
        CHAIN_MSG_MAP_MEMBER (m_position_tracker);
		MSG_WM_INITDIALOG(OnInitDialog)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()
	
	LRESULT OnInitDialog (CWindow wndFocus, LPARAM lInitParam);

    LRESULT OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        HWND hWnd = GetDlgItem (IDC_EXPLORER1);
        ::MoveWindow (hWnd, 0, 0, LOWORD (lParam), HIWORD (lParam), TRUE);
        return 0;
    }
	
    HRESULT NavigateComplete (_bstr_t URL);

    void OnClose () { EndDialog (IDOK); }

private:
    DWebBrowserEventsImpl m_events;
    SHDocVw::IWebBrowserAppPtr m_web_browser;

    cfgDialogSizeTracker m_size_tracker;
    cfgDialogPositionTracker m_position_tracker;

    bool m_result;
};