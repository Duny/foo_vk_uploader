#include "stdafx.h"

namespace vk_uploader
{
    namespace upload_presets
    {
        PFC_DECLARE_EXCEPTION (exception_preset_not_found, pfc::exception, "");

        namespace cfg
        {
            struct preset_internal
            {
                pfc::string8 m_name;
                GUID m_guid;
                upload_params m_preset;

                preset_internal () {}
                preset_internal (const pfc::string8 &p_name, const GUID &p_guid, const upload_params &p_preset) : m_name (p_name), m_guid (p_guid), m_preset (p_preset) {}

                bool operator== (const preset_internal &other) const { return pfc::stricmp_ascii (m_name, other.m_name) == 0; }
            };

            FB2K_STREAM_READER_OVERLOAD(preset_internal) { stream >> value.m_guid >> value.m_name; value.m_preset.set_data_raw (stream); return stream; }
            FB2K_STREAM_WRITER_OVERLOAD(preset_internal) { stream << value.m_guid << value.m_name; value.m_preset.get_data_raw (stream); return stream; }

            cfg_objList<preset_internal> preset_list (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
        }

        class NOVTABLE manager_imp : public manager
        {
            cfg::preset_internal & find_preset_internal (const pfc::string8 &p_name) const
            {
                for (t_size i = 0, n = cfg::preset_list.get_size (); i < n; i++) {
                    if (pfc::stricmp_ascii (p_name, cfg::preset_list[i].m_name) == 0)
                        return cfg::preset_list[i];
                }

                throw exception_preset_not_found ();
            }

            cfg::preset_internal & find_preset_internal (const GUID &p_guid) const
            {
                for (t_size i = 0, n = cfg::preset_list.get_size (); i < n; i++) {
                    if (p_guid == cfg::preset_list[i].m_guid)
                        return cfg::preset_list[i];
                }

                throw exception_preset_not_found ();
            }

            t_size get_preset_count () const override { return cfg::preset_list.get_count (); }

            const pfc::string8 & get_preset_name (t_size p_index) const override
            {
                static pfc::string8 dummy = "";
                return (p_index < cfg::preset_list.get_count ()) ? cfg::preset_list[p_index].m_name : dummy;
            }

            GUID get_preset_guid (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset_internal (p_name).m_guid;
                } catch (exception_preset_not_found) {
                    return pfc::guid_null;
                }
            }

            bool has_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset_internal (p_guid), true;
                } catch (exception_preset_not_found) { }
                return false;
            }

            const upload_params & get_preset (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset_internal (p_name).m_preset;
                } catch (exception_preset_not_found) {
                    static upload_params dummy;
                    return dummy;
                }
            }

            const upload_params & get_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset_internal (p_guid).m_preset;
                } catch (exception_preset_not_found) {
                    static upload_params dummy;
                    return dummy;
                }
            }

            bool save_preset (const pfc::string8 &p_preset_name, const upload_params &p_preset) override
            {
                try {
                    find_preset_internal (p_preset_name).m_preset = p_preset;
                } catch (exception_preset_not_found) {
                    GUID guid;
                    if (CoCreateGuid (&guid) != S_OK) return false;
                    cfg::preset_list.add_item (cfg::preset_internal (p_preset_name, guid, p_preset));
                }
                return true;
            }

            bool delete_preset (const pfc::string8 &p_name)
            {
                try {
                    cfg::preset_list.remove_item (find_preset_internal (p_name));
                } catch (exception_preset_not_found) {
                    return false;
                }
                return true;
            }
        };
        static service_factory_single_t<manager_imp> g_presets_manager_factory;
    }
}