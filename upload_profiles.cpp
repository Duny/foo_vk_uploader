#include "stdafx.h"

#include "upload_profiles.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        profile default_profile;

        struct profile_internal : profile
        {
            GUID m_guid;
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

        public:
            t_size get_count () const override
            {
                return g_profiles.get_count ();
            }

            const profile &get_profile (t_size p_index) const override
            {
                if (p_index < g_profiles.get_count ())
                    return g_profiles.get_item_ref (p_index);
                else
                    throw exception_profile_not_found ();
            }

            void get_profiles (pfc::list_t<profile> &p_out) const override
            {
                p_out = g_profiles;
            }

            GUID get_profile_guid (pfc::string8 &p_name) const override
            {
                for (t_size i = 0, n = g_profiles.get_size (); i < n; i++)
                    if (pfc::stricmp_ascii (p_name, g_profiles[i].m_name) == 0)
                        return g_profiles[i].m_guid;

                throw exception_profile_not_found ();
            }

            profile new_profile (const pfc::string8 &p_name) const override
            {
                for (t_size i = 0, n = g_profiles.get_size (); i < n; i++)
                    if (pfc::stricmp_ascii (p_name, g_profiles[i].m_name) == 0)
                        throw exception_duplicated_profile ();

                profile_internal p;

                HRESULT res = CoCreateGuid (&p.m_guid);
                if (res != S_OK) throw exception_create_guid_fail ();
                p.m_name = p_name;
                g_profiles.add_item (p);

                return p;
            }

            void save_profile (const profile &p_profile) override
            {
                for (t_size i = 0, n = g_profiles.get_size (); i < n; i++) {
                    if (pfc::stricmp_ascii (p_profile.m_name, g_profiles[i].m_name) == 0) {
                        g_profiles[i].m_album = p_profile.m_album;
                        g_profiles[i].m_post_on_wall = p_profile.m_post_on_wall;
                        return;
                    }
                }
                throw exception_profile_not_found ();
            }
        };

        const GUID manager_imp::g_profiles_guid = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
        cfg_objList<profile_internal> manager_imp::g_profiles (g_profiles_guid);

        static service_factory_single_t<manager_imp> g_profiles_manager_factory;
    }
}