#ifndef _FOO_VK_UPLOADER_CONFIG_H_
#define _FOO_VK_UPLOADER_CONFIG_H_

namespace vk_uploader
{
    namespace upload_profiles
    {
        struct profile
        {
            profile (const pfc::string8 &p_name = "") : m_name (p_name) {}

            bool operator == (const profile &other) const
            {
                return (m_name == other.m_name) && m_album == other.m_album && m_post_on_wall == other.m_post_on_wall;
            }

            pfc::string8 m_name; // profile name

            pfc::string8 m_album; // put uploaded tracks to the album
            bool m_post_on_wall;
        };
        
        extern profile default_profile;


        PFC_DECLARE_EXCEPTION (exception_profiles_manager, pfc::exception, COMPONENT_NAME);
        PFC_DECLARE_EXCEPTION (exception_create_guid_fail, exception_profiles_manager, COMPONENT_NAME ": CoCreateGuid failed");
        PFC_DECLARE_EXCEPTION (exception_duplicated_profile, exception_profiles_manager, COMPONENT_NAME ": profile with this name already exists");
        PFC_DECLARE_EXCEPTION (exception_profile_not_found, exception_profiles_manager, COMPONENT_NAME ": profile with this name does not exists");

        class NOVTABLE manager : public service_base
        {
        public:
            virtual t_size get_count () const = 0;

            virtual const profile &get_profile (t_size p_index) const = 0;
            virtual void get_profiles (pfc::list_t<profile> &p_out) const = 0;

            virtual GUID get_profile_guid (pfc::string8 &p_name) const = 0;

            // may throw exception on error
            virtual profile new_profile (const pfc::string8 &p_name) const = 0;

            virtual void save_profile (const profile &p_profile) = 0;

            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        };


        // {92CF789D-2064-428A-827D-B04CD5320F83}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x92cf789d, 0x2064, 0x428a, { 0x82, 0x7d, 0xb0, 0x4c, 0xd5, 0x32, 0xf, 0x83 } };
    }
}
#endif