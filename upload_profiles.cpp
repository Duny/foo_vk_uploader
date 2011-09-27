#include "stdafx.h"

#include "upload_profiles.h"

namespace vk_uploader
{
    /*namespace cfg
    {
        namespace guid
        {
            const GUID upload_profiles = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
            const GUID default_profile = { 0x4d9b8126, 0x1ca, 0x40ea, { 0xb7, 0x99, 0x8f, 0x10, 0x5a, 0xfc, 0x71, 0xee } };
        }

        upload_profile upload_profile::default_ (guid::default_profile, "...");

        cfg_objList<upload_profile> upload_profiles (guid::upload_profiles);
    }*/

    namespace upload_profiles
    {
        FB2K_STREAM_READER_OVERLOAD(profile)
        {
            return stream >> value.m_guid >> value.m_name >> value.m_album >> value.m_post_on_wall;
        }
        FB2K_STREAM_WRITER_OVERLOAD(profile)
        {
            return stream << value.m_guid << value.m_name << value.m_album << value.m_post_on_wall;
        }


        class NOVTABLE manager_imp : public manager
        {
            static const GUID g_profiles_guid;
            static cfg_objList<profile> g_profiles;
            static const profile g_default;

        public:
            t_size get_count () const
            {
                return g_profiles.get_count ();
            }

            const profile &get_profile (t_size p_index) const override
            {
                return g_profiles.get_item_ref (p_index);
            }

            const profile &get_default_profile () const
            {
                return g_default;
            }
        };
        const GUID manager_imp::g_profiles_guid = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
        cfg_objList<profile> manager_imp::g_profiles (g_profiles_guid);
        const profile manager_imp::g_default;

        static service_factory_single_t<manager_imp> g_profiles_manager_factory;
    }
}