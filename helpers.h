#ifndef _FOO_VK_UPLOADER_HELPERS_H_
#define _FOO_VK_UPLOADER_HELPERS_H_

// helpers for tuple stream i/o
template<class T1>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, cons<T1, null_type> &value) { return stream >> value.head; }

inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, null_type &value) { return stream; }

template<class T1, class T2>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, cons<T1, T2> &value) { stream >> value.head; return read_tuple (stream, value.tail); }

template<class T1>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const cons<T1, null_type> &value) { return stream << value.head; }

inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const null_type &value) { return stream; }

template<class T1, class T2>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const cons<T1, T2> &value) { stream << value.head; return write_tuple (stream, value.tail); }


typedef t_uint32 t_vk_album_id;
typedef t_uint32 t_vk_audio_id;


// keeps information about upload actions
// field_album_name: titleformat script for item album
// field_post_on_wall: makes a new post on the users wall containing newly uploaded items
// field_post_message: adds optional text to the top of the post
typedef tuple<pfc::string8, bool, pfc::string8> upload_parameters;
enum { field_album_name, field_post_on_wall, field_post_message };


// used for storing in config information about users albums
// first field is the title of album, second is id
typedef tuple<pfc::string8, t_vk_album_id> audio_album_info;
FB2K_STREAM_READER_OVERLOAD(audio_album_info) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(audio_album_info) { return write_tuple (stream, value); }


// For inplace GUID declaration
struct create_guid : public GUID
{
	create_guid (t_uint32 p_data1, t_uint16 p_data2, t_uint16 p_data3, t_uint8 p_data41, t_uint8 p_data42, t_uint8 p_data43, t_uint8 p_data44, t_uint8 p_data45, t_uint8 p_data46, t_uint8 p_data47, t_uint8 p_data48) 
	{
		Data1 = p_data1;
		Data2 = p_data2;
		Data3 = p_data3;
		Data4[0] = p_data41;
		Data4[1] = p_data42;
		Data4[2] = p_data43;
		Data4[3] = p_data44;
		Data4[4] = p_data45;
		Data4[5] = p_data46;
		Data4[6] = p_data47;
		Data4[7] = p_data48;
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

class string_url_encoded
{
    pfc::string8 m_data;
public:
    string_url_encoded (const char * p_str)
    {
        pfc::urlEncode (m_data, p_str); 
    }

    operator const char * () const { return m_data.get_ptr (); }
    operator const pfc::string8 & () const { return m_data; }
};


typedef function<void ()> new_thread_callback;
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
inline pfc::string8 trim (const pfc::string8 & p_str)
{
    auto is_trim_char = [] (char c) -> bool { return c == ' ' || c == '\n' || c == '\t'; };

    t_size str_len = p_str.get_length ();

    t_size left = 0;
    while (left < str_len && is_trim_char (p_str[left])) left++;

    t_size right = str_len - 1;
    while (right > left && is_trim_char (p_str[right])) right--;

    return pfc::string_part (p_str.get_ptr () + left, right - left + 1);
}

// opens upload setup dialog
void show_upload_setup_dialog (metadb_handle_list_cref p_items = metadb_handle_list ());
// starts upload process
void start_upload (metadb_handle_list_cref p_items, const upload_parameters & p_params);


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

class response_json_ptr
{
    bool is_valid () const {
        if (!m_val.is_valid ()) return false;
        const Json::Value & p_val = *m_val;
        if (p_val.isNull ()) return false;
        if (p_val.isObject ()) return !p_val.isMember ("error");
        return true;
    };

    void assert_valid ()
    {
        if (!is_valid ()) {
            if (!m_val.is_valid ())
                m_response_text.insert_chars (0, "Invalid response: ");
            else {
                const Json::Value & val = *m_val;
                if (val.isObject () && val.isMember ("error")) {
                    const Json::Value &error = val.get ("error", Json::nullValue);
                    if (error.isObject () && error.isMember ("error_msg"))
                        m_response_text.set_string_ (error["error_msg"].asCString ());
                }
                else
                    m_response_text.set_string_ (val.toStyledString ().c_str ());
            }
            throw pfc::exception (m_response_text.get_ptr ());
        }
    }

    pfc::rcptr_t<Json::Value> m_val;
    mutable pfc::string8 m_response_text;

public:
    response_json_ptr (const pfc::string_base & p_data) : m_response_text (p_data) {
        const char *begin = p_data.get_ptr ();
        const char *end = begin + p_data.get_length ();
        Json::Value val;
            
        if (Json::Reader ().parse (begin, end, val, false)) {
            bool has_response_field = val.isObject () && val.isMember ("response");
            m_val.new_t (has_response_field ? val["response"] : val);
        }

        assert_valid (); // Throw exception if we have bad response
    }

    Json::Value & operator[] (Json::UInt index) { return (*m_val)[index]; }
    const Json::Value & operator[] (Json::UInt index) const { return (*m_val)[index]; }

    Json::Value & operator[] (const char *key) { assert_valid (); return (*m_val)[key]; }
    const Json::Value & operator[] (const char *key) const { return (*m_val)[key]; }

    Json::Value* operator-> () { assert_valid (); return &(*m_val); }
    const Json::Value* operator-> () const { return &(*m_val); }
};

// Represents a list of name=>value string pairs of URL parameters
// (http://wwww.vk.com/?name1=value1&name2=value2&..)
typedef std::pair<pfc::string8, pfc::string8> name_value_pair;
typedef std::vector<name_value_pair> url_parameters;

inline url_parameters construct_from_url (const pfc::string_base & p_url)
{
    url_parameters params;
    t_size pos;
    if ((pos = p_url.find_first ('#')) != pfc_infinite || (pos = p_url.find_first ('?')) != pfc_infinite) {
        t_size len = p_url.length (), pos2, pos3;
        for (pos = pos + 1; pos < len; pos = pos3 + 1) {
            if ((pos2 = p_url.find_first ('=', pos)) == pfc_infinite)
                break;

            if ((pos3 = p_url.find_first ('&', ++pos2)) == pfc_infinite)
                pos3 = len;

            params.push_back (make_pair (
                /*name*/pfc::string_part (p_url.get_ptr () + pos, pos2 - pos - 1), 
                /*value*/pfc::string_part (p_url.get_ptr () + pos2, pos3 - pos2)));
        }
    }
    return params;
}

inline bool have (const url_parameters & p_params, const std::vector<const char*> & p_names)
{ 
    auto par_begin = p_params.begin (), par_end = p_params.end ();
    BOOST_FOREACH (const char * const name, p_names) {
        auto it = std::find_if (par_begin, par_end, [&] (const name_value_pair & p) { return pfc::stringCompareCaseInsensitive (p.first, name) == 0; });
        if (it == par_end) return false;
    }
    return true;
}

// Use carefully, it does not check whatever was p_name found or not. So p_name should exist
inline const pfc::string8 & get (const url_parameters & p_params, const char * p_name)
{
    return (*std::find_if (p_params.begin (), p_params.end (), [&] (const name_value_pair & p)
        { return pfc::stringCompareCaseInsensitive (p.first, p_name) == 0; })).second;
}

inline unsigned get_uint (const url_parameters & p_params, const char * p_name)
{
    return pfc::atoui_ex (get (p_params, p_name), pfc_infinite);
}
#endif