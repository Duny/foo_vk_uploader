#ifndef _FOO_VK_UPLOADER_ALBUM_LIST_DLG_H_
#define _FOO_VK_UPLOADER_ALBUM_LIST_DLG_H_

namespace vk_uploader
{
    // Helpers
    inline void listview_fill (HWND p_listview, const pfc::list_base_const_t<audio_album_info> &p_albums)
    {
        uSendMessage (p_listview, LVM_DELETEALLITEMS, 0, 0);
        p_albums.enumerate ([&](const audio_album_info &p_album) { listview_helper::insert_item (p_listview, 0, p_album.get<0>().get_ptr (), 0); });
    }

    inline void listview_get_sel_item_text (HWND p_listview, pfc::string_base & p_out)
    {
        p_out.reset ();
        int sel_item = ListView_GetSingleSelection (p_listview);
        if (sel_item > -1) listview_helper::get_item_text (p_listview, sel_item, 0, p_out);
    }

    class album_list_dlg : public CDialogImpl<album_list_dlg>
    {
    public:
        enum { IDD = IDD_ALBUM_LIST };

        album_list_dlg () { m_str.new_t (); }

        // Shows dialog. Returns user selected string with album name
        const pfc::string_base & get_album_name () { return DoModal (core_api::get_main_window ()), m_album_name; }

    private:
        BEGIN_MSG_MAP(album_list_dlg)
            MESSAGE_HANDLER_SIMPLE(WM_INITDIALOG, on_init_dialog)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_REFRESH, on_refresh_albums)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_ALBUM_NEW, on_album_new)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_ALBUM_RENAME, on_album_rename)
            COMMAND_ID_HANDLER_SIMPLE(IDC_BUTTON_ALBUM_DELETE, on_album_delete)
            NOTIFY_HANDLER_EX_SIMPLE(IDC_LISTVIEW_ALBUMS, NM_DBLCLK, close<TRUE>)
            COMMAND_ID_HANDLER_SIMPLE(IDOK, close<TRUE>)
            COMMAND_ID_HANDLER_SIMPLE(IDCANCEL, close<FALSE>)
            MSG_WM_CLOSE(close<FALSE>)
            MSG_WM_DESTROY(on_destroy)
        END_MSG_MAP()

        // Message handlers
        void on_init_dialog ();
        void on_destroy ();

        template <BOOL save_album_name>
        void close ()
        {
            if (save_album_name == TRUE)
                listview_get_sel_item_text (GetDlgItem (IDC_LISTVIEW_ALBUMS), m_album_name);
            EndDialog (IDOK);
        }

        void on_refresh_albums ();
        void on_album_new ();
        void on_album_rename ();
        void on_album_delete ();

        // Helpers
        
        class completion_notify_new_album : public completion_notify
        {
            HWND m_listview;
            pfc::rcptr_t<pfc::string_base> m_str;

            void on_completion (unsigned p_code) override;

        public:
            completion_notify_new_album (HWND p_listview, pfc::rcptr_t<pfc::string_base> p_str) : m_listview (p_listview), m_str (p_str) {}
        };

        class completion_notify_rename_album : public completion_notify
        {
            HWND m_listview;
            unsigned m_item_index;
            pfc::string8 m_prev_title;
            pfc::rcptr_t<pfc::string_base> m_new_title;

            void on_completion (unsigned p_code) override;

        public:
            completion_notify_rename_album (HWND p_listview, unsigned p_item_index, const pfc::string_base & p_prev_title, pfc::rcptr_t<pfc::string_base> p_new_title) 
                : m_listview (p_listview), m_item_index (p_item_index), m_prev_title (p_prev_title), m_new_title (p_new_title) {}
        };

        // Member variables

        pfc::string8 m_album_name; // Name of selected album if list
        pfc::rcptr_t<pfc::string8> m_str; // For inplace editing
    };
}
#endif