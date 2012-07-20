#ifndef _FOO_VK_UPLOADER_HELPERS_H_
#define _FOO_VK_UPLOADER_HELPERS_H_

FB2K_STREAM_READER_OVERLOAD(vk_com_api::audio_album_info) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(vk_com_api::audio_album_info) { return write_tuple (stream, value); }


// keeps information about upload actions
// field_album_name: titleformat script for item album
// field_post_on_wall: makes a new post on the users wall containing newly uploaded items
// field_post_message: adds optional text to the top of the post
typedef boost::tuples::tuple<pfc::string8, bool, pfc::string8> upload_parameters;
enum { field_album_name, field_post_on_wall, field_post_message };

namespace vk_uploader
{
    using namespace vk_com_api;

    // For inplace GUID declaration
    struct create_guid : public GUID
    {
        create_guid (t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11) 
        {
            Data1 = d1;
            Data2 = d2;
            Data3 = d3;
            Data4[0] = d4;  Data4[1] = d5;
            Data4[2] = d6;  Data4[3] = d7;
            Data4[4] = d8;  Data4[5] = d9;
            Data4[6] = d10; Data4[7] = d11;
        }
    };

    class string_utf8_from_combobox
    {
        pfc::string8 m_data;
    public:
        string_utf8_from_combobox (const CComboBox &p_combo, int p_index)
        {
            int str_len = p_combo.GetLBTextLen (p_index);
            if (str_len > 0) {
                pfc::array_t<TCHAR> buf;
                buf.set_size (str_len + 1);
                int actual_len = p_combo.GetLBText (p_index, buf.get_ptr ());
                if (actual_len == str_len)
                    m_data = pfc::stringcvt::string_utf8_from_os (buf.get_ptr ());
            }
        }

        operator const char * () const { return m_data.get_ptr (); }
        operator pfc::string8 const & () const { return m_data; }
    };

    class string_utf8_from_listbox
    {
        pfc::string8 m_data;
    public:
        string_utf8_from_listbox (const CListBox &p_list, int p_index)
        {
            int str_len = p_list.GetTextLen (p_index);
            if (str_len > 0) {
                pfc::array_t<TCHAR> buf;
                buf.set_size (str_len + 1);
                int actual_len = p_list.GetText (p_index, buf.get_ptr ());
                //if (actual_len == str_len) ? Is this right? Check help on GetText
                m_data = pfc::stringcvt::string_utf8_from_os (buf.get_ptr ());
            }
        }

        operator const char * () const { return m_data.get_ptr (); }
        operator const pfc::string8 & () const { return m_data; }
    };

    // Class, holding list of albums in user profile
    class user_album_list
    {
        static pfc::list_t<audio_album_info> g_album_list;

        int find_album (const pfc::string_base & p_name) const
        {
            auto n = g_album_list.get_size ();
            while (n --> 0) {
                if (pfc::stringCompareCaseInsensitive (g_album_list[n].get<0>().get_ptr (), p_name.get_ptr ()) == 0)
                    return n;
            }
            return -1;
        }

        void reset_error ()
        {
            m_aborted = false;
            m_error.reset ();
        }

        template <class t_vk_api_method, typename t_on_ok_callback>
        bool call_api (t_vk_api_method & p_method, const t_on_ok_callback & on_ok)
        {
            reset_error ();
            bool is_ok = p_method.call ();
            if (is_ok) on_ok (p_method);
            else std::tie (m_aborted, m_error) = std::make_pair (p_method.aborted (), p_method.get_error ());
            return is_ok;
        }

        bool m_aborted;
        pfc::string8 m_error;

    public:
        const pfc::list_base_const_t<audio_album_info> & get_albums () const { return g_album_list; }

        bool reload (); // Loads album list from user vk profile

        bool add_item (const pfc::string_base & p_title); // Adds new album to user profile
        bool remove_item (t_vk_album_id album_id); // Deletes album from user profile
        bool rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name);

        bool aborted () const { return m_aborted; }
        const char* get_error () const { return m_error.get_ptr (); }

        // Return 0 if not found
        t_vk_album_id get_album_id_by_name (const pfc::string_base & p_album_name) const
        {
            int item_index = find_album (p_album_name);
            return item_index > -1 ? g_album_list[item_index].get<1>() : 0;
        }
    };

    typedef std::function<void ()> new_thread_callback;
    inline void run_in_separate_thread (const new_thread_callback &p_func)
    {
        class thread_dynamic {
        public:
            PFC_DECLARE_EXCEPTION (exception_creation, pfc::exception, "Could not create thread");

            thread_dynamic (const new_thread_callback &p_func, int priority) : m_func (p_func), m_thread (INVALID_HANDLE_VALUE)
            {
                m_thread = CreateThread (NULL, 0, g_entry, reinterpret_cast<void*>(this), CREATE_SUSPENDED, NULL);
                if (m_thread == NULL) throw exception_creation ();
                SetThreadPriority (m_thread, priority);
                ResumeThread (m_thread);
            }

        private:
            // Must be instantiated with operator new
            ~thread_dynamic () { CloseHandle (m_thread); }

            void threadProc () { m_func (); delete this; }

            static DWORD CALLBACK g_entry (void* p_instance) { return reinterpret_cast<thread_dynamic*>(p_instance)->entry (); }
            unsigned entry () {
                try { threadProc (); } catch (...) {}
                return 0;
            }

            new_thread_callback m_func;
            HANDLE m_thread;

            PFC_CLASS_NOT_COPYABLE_EX (thread_dynamic)
        };

        new thread_dynamic (p_func, THREAD_PRIORITY_BELOW_NORMAL);
    }


    // helper: remove spaces
    pfc::string8 trim (const pfc::string8 & p_str);

    // opens upload setup dialog
    void show_upload_setup_dialog (metadb_handle_list_cref p_items = metadb_handle_list ());
    // starts upload process
    void start_upload (metadb_handle_list_cref p_items, const upload_parameters & p_params);
}

// Helper macros. The main goal is that it accept function without arguments
#define COMMAND_HANDLER_SIMPLE(id, code, func) \
    if (uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) \
    { \
        bHandled = TRUE; \
        lResult = 0; \
        func (); \
        return TRUE; \
    }

#define COMMAND_ID_HANDLER_SIMPLE(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = 0; \
        func(); \
	    return TRUE; \
	}

#define NOTIFY_HANDLER_EX_SIMPLE(id, cd, func) \
	if (uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code && id == ((LPNMHDR)lParam)->idFrom) \
	{ \
		bHandled = TRUE; \
		lResult = 0; \
        func(); \
	    return TRUE; \
	}
#endif