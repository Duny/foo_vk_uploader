#include "stdafx.h"

#include "upload_profiles.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        profile default_profile;

        struct profile_internal : profile
        {
            pfc::string8 m_name;
            GUID m_guid;

            profile &operator= (const profile &other)
            {
                m_album = other.m_album;
                m_post_on_wall = other.m_post_on_wall;

                return *this;
            }

            bool operator== (const profile_internal &other) const { return m_name == other.m_name; }
        };

        FB2K_STREAM_READER_OVERLOAD(profile_internal)
        {
            return stream >> value.m_guid >> value.m_name >> value.m_album >> value.m_post_on_wall;
        }
        FB2K_STREAM_WRITER_OVERLOAD(profile_internal)
        {
            return stream << value.m_guid << value.m_name << value.m_album << value.m_post_on_wall;
        }


        class NOVTABLE manager_imp : public manager
        {
            static const GUID g_profiles_guid;
            static cfg_objList<profile_internal> g_profiles;
            
            profile_internal & find_profile (const pfc::string8 &p_name) const
            {
                for (t_size i = 0, n = g_profiles.get_size (); i < n; i++)
                    if (pfc::stricmp_ascii (p_name, g_profiles[i].m_name) == 0)
                        return g_profiles[i];

                throw exception_profile_not_found ();
            }

            t_size get_profile_count () const override
            {
                return g_profiles.get_count ();
            }

            pfc::string8 get_profile_name (t_size p_index) const override
            {
                if (p_index < g_profiles.get_count ())
                    return g_profiles[p_index].m_name;
                else
                    throw exception_profile_not_found ();
            }

            GUID get_profile_guid (const pfc::string8 &p_name) const override
            {
                return find_profile (p_name).m_guid;
            }

            profile get_profile (const pfc::string8 &p_name) const override
            {
                return find_profile (p_name);
            }

            void save_profile (const pfc::string8 &p_profile_name, const profile &p_profile) override
            {
                try {
                    profile_internal &p = find_profile (p_profile_name);
                    p = p_profile;
                } catch (exception_profile_not_found) {
                    profile_internal p;

                    HRESULT res = CoCreateGuid (&p.m_guid);
                    if (res != S_OK) throw exception_create_guid_fail ();
                    p.m_name = p_profile_name;

                    p = p_profile;
                    g_profiles.add_item (p);
                }
            }

            void delete_profile (const pfc::string8 &p_name)
            {
                try {
                    g_profiles.remove_item (find_profile (p_name));
                } catch (...) {}
            }
        };

        const GUID manager_imp::g_profiles_guid = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
        cfg_objList<profile_internal> manager_imp::g_profiles (g_profiles_guid);

        static service_factory_single_t<manager_imp> g_profiles_manager_factory;
    }
}