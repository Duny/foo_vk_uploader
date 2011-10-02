#ifndef _FOO_VK_UPLOADER_CONFIG_H_
#define _FOO_VK_UPLOADER_CONFIG_H_

namespace vk_uploader
{
    namespace upload_profiles
    {
        struct profile
        {
            pfc::string8 m_album; // put uploaded tracks to the album
            bool m_post_on_wall;
        };
        extern profile default_profile;


        PFC_DECLARE_EXCEPTION (exception_profiles_manager, pfc::exception, COMPONENT_NAME);
        PFC_DECLARE_EXCEPTION (exception_profile_not_found, exception_profiles_manager, "Preset with this name does not exists");

        class NOVTABLE manager : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        public:
            virtual t_size get_profile_count () const = 0;

            // returns empty string if index is out of bound
            virtual pfc::string8 get_profile_name (t_size p_index) const = 0;

            // returns pfc::guid_null if profile not found
            virtual GUID get_profile_guid (const pfc::string8 &p_name) const = 0;

            // throws exception_profile_not_found if profile not found
            virtual profile get_profile (const pfc::string8 &p_name) const = 0;

            
            // will create new profile if given profile does not exists
            // returns false on error
            virtual bool save_profile (const pfc::string8 &p_name, const profile &p_profile) = 0;

            // returns false then given profile does not exists
            virtual bool delete_profile (const pfc::string8 &p_name) = 0;
        };

        // {92CF789D-2064-428A-827D-B04CD5320F83}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x92cf789d, 0x2064, 0x428a, { 0x82, 0x7d, 0xb0, 0x4c, 0xd5, 0x32, 0xf, 0x83 } };
    }

    typedef static_api_ptr_t<upload_profiles::manager> get_profile_manager;
}
#endif