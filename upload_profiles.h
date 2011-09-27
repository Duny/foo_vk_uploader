#ifndef _FOO_VK_UPLOADER_CONFIG_H_
#define _FOO_VK_UPLOADER_CONFIG_H_

namespace vk_uploader
{
    namespace upload_profiles
    {
        struct profile
        {
            profile (const GUID &p_guid = pfc::guid_null, const pfc::string8 &p_name = "")
                : m_guid (p_guid), m_name (p_name) {}

            //bool is_default () const { return IsEqualGUID (default_.m_guid, m_guid) && default_.m_name == m_name; }

            GUID m_guid;
            pfc::string8 m_name; // profile name

            pfc::string8 m_album; // put uploaded tracks in the album
            bool m_post_on_wall;
        };


        class NOVTABLE manager : public service_base
        {
        public:
            virtual t_size get_count () const = 0;
            virtual const profile &get_profile (t_size p_index) const = 0;
            virtual const profile &get_default_profile () const = 0; // special case of profile with default setting

            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        };


        // {92CF789D-2064-428A-827D-B04CD5320F83}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x92cf789d, 0x2064, 0x428a, { 0x82, 0x7d, 0xb0, 0x4c, 0xd5, 0x32, 0xf, 0x83 } };
    }
}
#endif