#include "stdafx.h"
#include "WebBrowserCtrl.h"
#include "login_dialog.h"
//#include "Wininet.h"

LRESULT vk_login_dialog::OnInitDialog (CWindow wndFocus, LPARAM lInitParam)
{
	m_web_browser = 0;
	HRESULT result = GetDlgControl (IDC_EXPLORER1, __uuidof (SHDocVw::IWebBrowserAppPtr), (void**)&m_web_browser);
    if (result != S_OK || !m_web_browser)
        return FALSE;

	m_events.SetParent (this);

	// subscribe for the web browse event source
	LPCONNECTIONPOINTCONTAINER CPC = NULL;
	LPCONNECTIONPOINT CP = NULL;
	m_web_browser->QueryInterface (IID_IConnectionPointContainer, (LPVOID*)&CPC);
	CPC->FindConnectionPoint (__uuidof(SHDocVw::DWebBrowserEventsPtr), &CP);
	DWORD cookie;
	result = CP->Advise ((LPUNKNOWN)&m_events, &cookie);
    if (result != S_OK)
        return FALSE;

	HWND hWnd = core_api::get_main_window ();
	SetIcon ((HICON)SendMessage (hWnd, WM_GETICON, ICON_BIG, 0), TRUE);
	SetIcon ((HICON)SendMessage (hWnd, WM_GETICON, ICON_SMALL, 0), FALSE);

    m_result = false;

//	BOOL b = InternetSetCookie (L"http://vkontakte.ru/", NULL, L"Expires = Sat,01-Jan-2000 00:00:00 GMT");

    //m_web_browser->Navigate ("vkontakte.ru");
    m_web_browser->Navigate (_bstr_t (vk_api::get_auth_url ()));

	return TRUE;
}

bool vk_login_dialog::show ()
{
    DoModal (core_api::get_main_window ());
	return m_result;
}

HRESULT vk_login_dialog::NavigateComplete (_bstr_t URL)
{
    pfc::string8 url (URL);
    str2str_map params;

    if (!vk_api::get_url_params (url, params)) {
		console::formatter () << "foo_vk_uploader: can't parse \"" << url << "\"";
		return S_OK;
	}

    if (params.have_item (pfc::string8 ("error")) || params.have_item (pfc::string8 ("cancel"))) {
        OnClose ();
		return S_OK;
    }

    if (!vk_api::is_redirect_url (url)) // url should start from redirect address
		return S_OK;

	if (!params.have_item (pfc::string8 ("access_token"))) {
		console::formatter () << "foo_vk_uploader: access_token not found in responce";
		return S_OK;
	}

	vk_access_token = params[pfc::string8 ("access_token")];
	m_result = true;
	EndDialog (TRUE);

	// delete cookie
	//MSHTML::IHTMLDocument2Ptr doc (m_web_browser->Document);
	//doc->put_cookie (BSTR (_bstr_t ("remixsid=")));
	//doc->clear ();

    return S_OK;
}