#include "stdafx.h"
#include "upload_setup_dialog.h"

LRESULT vk_upload_setup_dialog::OnInitDialog (CWindow wndFocus, LPARAM lInitParam)
{
	HWND hWnd = core_api::get_main_window ();
	SetIcon ((HICON)SendMessage (hWnd, WM_GETICON, ICON_BIG, 0), TRUE);
	SetIcon ((HICON)SendMessage (hWnd, WM_GETICON, ICON_SMALL, 0), FALSE);

	return 0;
}

void vk_upload_setup_dialog::show ()
{
    Create (core_api::get_main_window ());
    ShowWindow (SW_SHOWNORMAL);
}