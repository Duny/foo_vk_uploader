#include "stdafx.h"

#include "upload_profiles.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        profile default_profile;

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

        public:
            t_size get_count () const override
            {
                return g_profiles.get_count ();
            }

            const profile &get_profile (t_size p_index) const override
            {
                return g_profiles.get_item_ref (p_index);
            }

            void get_profiles (pfc::list_t<profile> &p_out) const override
            {
                p_out = g_profiles;
            }
        };

        const GUID manager_imp::g_profiles_guid = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
        cfg_objList<profile> manager_imp::g_profiles (g_profiles_guid);

        static service_factory_single_t<manager_imp> g_profiles_manager_factory;
    }
}