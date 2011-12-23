#ifndef _FOO_VK_UPLOADER_HELPERS_H_
#define _FOO_VK_UPLOADER_HELPERS_H_

// helpers for tuple stream i/o
template<class T1>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::cons<T1, boost::tuples::null_type> &value) { return stream >> value.head; }

inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::null_type &value) { return stream; }

template<class T1, class T2>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::cons<T1, T2> &value) { stream >> value.head; return read_tuple (stream, value.tail); }

template<class T1>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::cons<T1, boost::tuples::null_type> &value) { return stream << value.head; }

inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::null_type &value) { return stream; }

template<class T1, class T2>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::cons<T1, T2> &value) { stream << value.head; return write_tuple (stream, value.tail); }


typedef t_uint32 t_vk_album_id;
typedef t_uint32 t_vk_audio_id;

// keeps information about upload actions
// field_album_name: titleformat script for item album
// field_post_on_wall: makes a new post on the users wall containing newly uploaded items
// field_post_message: adds optional text to the top of the post
typedef boost::tuple<pfc::string8, bool, pfc::string8> upload_parameters;
enum { field_album_name, field_post_on_wall, field_post_message };


// used for storing in config information about users albums
// first field is the title of album, second is its id
typedef boost::tuple<pfc::string8, t_vk_album_id> audio_album_info;
FB2K_STREAM_READER_OVERLOAD(audio_album_info) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(audio_album_info) { return write_tuple (stream, value); }


// helper to get "inline" GUID definitions
// some_func (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
struct guid_inline { static const GUID guid; };

template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
__declspec (selectany) const GUID guid_inline<d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11>::guid = { d1, d2, d3, { d4, d5, d6, d7, d8, d9, d10, d11 } };


class user_album_list
{
    static cfg_objList<audio_album_info> m_cfg_album_list;

    inline int find_item (const pfc::string_base & p_name) const
    {
        auto n = m_cfg_album_list.get_size ();
        while (n --> 0) {
            if (pfc::stringCompareCaseInsensitive (m_cfg_album_list[n].get<0> ().get_ptr (), p_name.get_ptr ()) == 0)
                return n;
        }
        return -1;
    }

public:
    inline const pfc::list_base_const_t<audio_album_info> & get_albums () const { return m_cfg_album_list; }
    inline operator const pfc::list_base_const_t<audio_album_info> & () const { return m_cfg_album_list; }

    // All this functions may throw exceptions
    void reload_items (); // Loads albums list from user vk profile (may throw exceptions)
    void add_item (const pfc::string_base & p_title); // Adds new album to user profile
    void remove_item (t_vk_album_id album_id); // Deletes album from user profile
    void rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name);

    inline t_vk_album_id get_album_id_by_name (const pfc::string_base & p_album_name) const
    {
        int item_index = find_item (p_album_name);
        return item_index > -1 ? m_cfg_album_list[item_index].get<1> () : 0;
    }

    // Does nothing with user profile
    inline void clear () { m_cfg_album_list.remove_all (); }
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

    inline operator const char * () const { return m_data.get_ptr (); }
    inline operator const pfc::string8& () const { return m_data; }
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

    inline operator const char * () const { return m_data.get_ptr (); }
    inline operator const pfc::string8& () const { return m_data; }
};

// helper: wraps main_thread_callback
template <typename Func>
void run_from_main_thread (Func f)
{
    struct from_main_thread : main_thread_callback
    {
        void callback_run () override { f (); }
        from_main_thread (Func f) : f (f) {}
        Func f;
    };

    static_api_ptr_t<main_thread_callback_manager>()->add_callback (new service_impl_t<from_main_thread> (f));
}

// helper: wraps pfc::thread

typedef boost::function<void ()> new_thread_callback;
inline void run_in_separate_thread (const new_thread_callback &p_func)
{
    class new_thread_t : pfc::thread
    {
        new_thread_callback m_func;
        void threadProc () override
        {
            m_func ();
            delete this;
        }
        ~new_thread_t () { waitTillDone (); }
    public:
        new_thread_t (const new_thread_callback &p_func) : m_func (p_func) { startWithPriority (THREAD_PRIORITY_BELOW_NORMAL); }
    };

    new new_thread_t (p_func);
}


// helper: remove spaces
pfc::string8 trim (const pfc::string8 &p_str);

// opens upload setup dialog
void show_upload_setup_dialog (metadb_handle_list_cref p_items = metadb_handle_list ());


// helper
#define COMMAND_HANDLER_SIMPLE(id, code, func) \
    if(uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) \
    { \
        bHandled = TRUE; \
        lResult = 0; \
        func (); \
        return TRUE; \
    }

#define COMMAND_ID_HANDLER_SIMPLE(id, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam)) \
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

class response_json_ptr : public boost::shared_ptr<Json::Value>
{
    inline bool is_valid (const Json::Value *p_val) const {
        if (!p_val || p_val->isNull ()) return false;
        if (p_val->isObject ()) return !p_val->isMember ("error");
        return true;
    };

    pfc::string8 get_error_code ()
    {
        const Json::Value *val = get ();
        if (val) {
            if (val->isObject () && val->isMember ("error")) {
                const Json::Value &error = val->get ("error", Json::nullValue);
                if (error.isObject () && error.isMember ("error_msg"))
                    return error["error_msg"].asCString ();
            }
            else
                return val->toStyledString ().c_str ();
        }
        return "Invalid response";
    }

    inline void assert_valid () { if (!is_valid (get ())) throw pfc::exception (get_error_code ()); }

public:
    response_json_ptr (const pfc::string_base & p_data) {
        const char *begin = p_data.get_ptr ();
        const char *end = begin + p_data.get_length ();
        Json::Value val;
            
        if (Json::Reader ().parse (begin, end, val, false)) {
            Json::Value *out;
            if (val.isObject () && val.isMember ("response"))
                out = new Json::Value (val["response"]);
            else
                out = new Json::Value (val);
            reset (out);
        }
        assert_valid ();
    }

    Json::Value &operator[] (Json::UInt index) { return (*get ())[index]; }
    const Json::Value &operator[] (Json::UInt index) const { return (*get ())[index]; }

    Json::Value & operator[] (const char *key) { return (*get ())[key]; }
    const Json::Value & operator[]( const char *key ) const { return (*get ())[key]; }
};


class url_params : public pfc::map_t<pfc::string8, pfc::string8>
{
public:
    url_params () {}
    url_params (const pfc::string_base &p_url); // parsed string for name=value pairs
    // constructs from single p_name=p_value pair
    url_params (const char *p_name, const char *p_value) { find_or_add (pfc::string8 (p_name)) = p_value; }

    bool have (const char *p_name) const { return have_item (pfc::string8 (p_name)); }

    pfc::string8 &operator [] (const char *p_name) { return find_or_add (pfc::string8 (p_name)); }
};
typedef url_params const& params_cref;
        

class request_url_builder
{
    pfc::string8 m_url;
public:
    request_url_builder (const char *p_method_name, params_cref p_params, abort_callback &p_abort);

    operator const char * () const { return m_url.get_ptr (); }
};


#endif