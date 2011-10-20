#include "stdafx.h"

#include "helpers.h"
#include "upload_preset.h"

namespace vk_uploader
{
    namespace upload_presets
    {
        PFC_DECLARE_EXCEPTION (exception_preset_not_found, pfc::exception, "");

        namespace cfg
        {
            struct preset_internal : preset
            {
                pfc::string8 m_name;
                GUID m_guid;

                void set (const preset &other)
                {
                    m_album = other.m_album;
                    m_post_on_wall = other.m_post_on_wall;
                }

                bool operator== (const preset_internal &other) const { return m_name == other.m_name; }
            };

            FB2K_STREAM_READER_OVERLOAD(preset_internal) { return stream >> value.m_guid >> value.m_name >> value.m_album >> value.m_post_on_wall; }
            FB2K_STREAM_WRITER_OVERLOAD(preset_internal) { return stream << value.m_guid << value.m_name << value.m_album << value.m_post_on_wall; }

            cfg_objList<preset_internal> presets (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
        }

        class NOVTABLE manager_imp : public manager
        {
            cfg::preset_internal & find_preset (const pfc::string8 &p_name) const
            {
                for (t_size i = 0, n = cfg::presets.get_size (); i < n; i++) {
                    if (pfc::stricmp_ascii (p_name, cfg::presets[i].m_name) == 0)
                        return cfg::presets[i];
                }

                throw exception_preset_not_found ();
            }

            cfg::preset_internal & find_preset (const GUID &p_guid) const
            {
                for (t_size i = 0, n = cfg::presets.get_size (); i < n; i++) {
                    if (p_guid == cfg::presets[i].m_guid)
                        return cfg::presets[i];
                }

                throw exception_preset_not_found ();
            }

            t_size get_preset_count () const override { return cfg::presets.get_count (); }

            const pfc::string8 & get_preset_name (t_size p_index) const override
            {
                static pfc::string8 dummy = "";
                return (p_index < cfg::presets.get_count ()) ? cfg::presets[p_index].m_name : dummy;
            }

            GUID get_preset_guid (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset (p_name).m_guid;
                } catch (exception_preset_not_found) {
                    return pfc::guid_null;
                }
            }

            bool has_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset (p_guid), true;
                } catch (exception_preset_not_found) { }
                return false;
            }

            const preset & get_preset (const pfc::string8 &p_name) const override
            {
                try {
                    return find_preset (p_name);
                } catch (exception_preset_not_found) {
                    static preset dummy;
                    return dummy;
                }
            }

            const preset & get_preset (const GUID &p_guid) const override
            {
                try {
                    return find_preset (p_guid);
                } catch (exception_preset_not_found) {
                    static preset dummy;
                    return dummy;
                }
            }

            bool save_preset (const pfc::string8 &p_preset_name, const preset &p_preset) override
            {
                try {
                    find_preset (p_preset_name).set (p_preset);
                } catch (exception_preset_not_found) {
                    cfg::preset_internal p;

                    if (CoCreateGuid (&p.m_guid) != S_OK)
                        return false;

                    p.m_name = p_preset_name;
                    p.set (p_preset);

                    cfg::presets.add_item (p);
                }
                return true;
            }

            bool delete_preset (const pfc::string8 &p_name)
            {
                try {
                    cfg::presets.remove_item (find_preset (p_name));
                } catch (exception_preset_not_found) {
                    return false;
                }
                return true;
            }
        };
        static service_factory_single_t<manager_imp> g_presets_manager_factory;
    }
}