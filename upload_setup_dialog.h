#pragma once

class vk_upload_setup_dialog : public CAxDialogImpl<vk_upload_setup_dialog>
{
public:
	enum { IDD = IDD_UPLOAD_SETUP_DLG };

    vk_upload_setup_dialog (metadb_handle_list_cref p_items) : m_items (p_items),
        m_position_tracker (vk_upload_setup_dlg_pos) {};

    void show ();

	BEGIN_MSG_MAP_EX(vk_upload_setup_dialog)
        CHAIN_MSG_MAP_MEMBER(m_position_tracker)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()
	 
	LRESULT OnInitDialog (CWindow wndFocus, LPARAM lInitParam);

    void OnClose () {
        DestroyWindow ();
    }

    virtual void OnFinalMessage (HWND) {
        delete this;
    }

private:
    metadb_handle_list_cref m_items;

    cfgDialogPositionTracker m_position_tracker;
};