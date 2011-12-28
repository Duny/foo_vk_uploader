#include "stdafx.h"

// preset name, preset GUID (for context menus), preset upload parameters
typedef boost::tuple<pfc::string8, GUID, upload_parameters> upload_preset;

FB2K_STREAM_READER_OVERLOAD(upload_parameters) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(upload_parameters) { return write_tuple (stream, value); }

FB2K_STREAM_READER_OVERLOAD(upload_preset) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(upload_preset) { return write_tuple (stream, value); }  
         
namespace vk_uploader
{
    namespace upload_presets
    {
        PFC_DECLARE_EXCEPTION (exception_preset_not_found, pfc::exception, "");

        class NOVTABLE manager_imp : public manager
        {
            template<int N, class T>
            upload_preset & find_preset_internal (const T &val) const
            {
                for (t_size i = 0, n = m_preset_list.get_size (); i < n; i++) {
                    if (val == m_preset_list[i].get<N> ())
                        return m_preset_list[i];
                }

                throw exception_preset_not_found ();
            }

            t_size get_preset_count () const override { return m_preset_list.get_count (); }

            const pfc::string8 & get_preset_name (t_size p_index) const override
            {
                static pfc::string8 dummy = "";
                return (p_index < m_preset_list.get_count ()) ? m_preset_list[p_index].get<0> () : dummy;
            }

            GUID get_preset_guid (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset_internal<0> (p_name).get<1> ();
                } catch (exception_preset_not_found) {
                    return pfc::guid_null;
                }
            }

            bool has_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset_internal<1> (p_guid), true;
                } catch (exception_preset_not_found) { }
                return false;
            }

            const upload_parameters & get_preset (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset_internal<0> (p_name).get<2> ();
                } catch (exception_preset_not_found) {
                    return m_dummy;
                }
            }

            const upload_parameters & get_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset_internal<1> (p_guid).get<2> ();
                } catch (exception_preset_not_found) {
                    return m_dummy;
                }
            }

            bool save_preset (const pfc::string8 &p_preset_name, const upload_parameters &p_preset) override
            {
                try {
                    find_preset_internal<0> (p_preset_name).get<2> () = p_preset;
                } catch (exception_preset_not_found) {
                    GUID guid;
                    if (CoCreateGuid (&guid) != S_OK) return false;
                    m_preset_list.add_item (boost::make_tuple (p_preset_name, guid, p_preset));
                }
                return true;
            }

            bool delete_preset (const pfc::string8 &p_name)
            {
                try {
                    m_preset_list.remove_item (find_preset_internal<0> (p_name));
                } catch (exception_preset_not_found) {
                    return false;
                }
                return true;
            }

            static upload_parameters m_dummy;
            static cfg_objList<upload_preset> m_preset_list;
        };
        upload_parameters manager_imp::m_dummy;
        cfg_objList<upload_preset> manager_imp::m_preset_list (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);

        namespace { service_factory_single_t<manager_imp> g_presets_manager_factory; }
    }
}