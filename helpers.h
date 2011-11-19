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
// field_album_id: moves uploaded items to the specified album
// field_post_on_wall: makes a new post on the users wall containing newly uploaded items
// field_post_message: adds optional text to the top of the post
typedef boost::tuple<t_vk_album_id, bool, pfc::string8> upload_parameters;
enum { field_album_id, field_post_on_wall, field_post_message };


// used for storing in config information about users albums
// first field is the title of album, second is its id
typedef boost::tuple<pfc::string8, t_vk_album_id> audio_album_info;


// helper to get "inline" GUID definitions
// some_func (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
struct guid_inline { static const GUID guid; };

template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
__declspec (selectany) const GUID guid_inline<d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11>::guid = { d1, d2, d3, { d4, d5, d6, d7, d8, d9, d10, d11 } };


// helper: remove spaces
pfc::string8 trim (const pfc::string8 &p_str);

// opens upload setup dialog
void show_upload_setup_dialog (metadb_handle_list_cref p_items = metadb_handle_list ());
void clear_album_list ();


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
    response_json_ptr (const pfc::string8 & p_data) {
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
    url_params (const pfc::string8 &p_url); // parsed string for name=value pairs
    // constructs from single p_name=p_value pair
    url_params (const char *p_name, const char *p_value) { find_or_add (pfc::string8 (p_name)) = p_value; }

    bool have (const char *p_name) const { return have_item (pfc::string8 (p_name)); }

    pfc::string8 &operator [] (const char *p_name) { return find_or_add (pfc::string8 (p_name)); }
};
typedef url_params const& params_cref;
        

class request_url_builder
{
    pfc::string8_fast m_url;
public:
    request_url_builder (const char *p_method_name, params_cref p_params, abort_callback &p_abort);

    operator const char * () const { return m_url.get_ptr (); }
};


#endif