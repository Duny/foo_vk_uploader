#ifndef _FOO_VK_UPLOADER_UPLOAD_PRESET_H_
#define _FOO_VK_UPLOADER_UPLOAD_PRESET_H_

namespace vk_uploader
{
    namespace upload_presets
    {
        struct preset
        {
            preset () {}
            preset (t_uint32 album, bool post_on_wall) : m_album (album), m_post_on_wall (post_on_wall) {}

            t_uint32 m_album; // put uploaded tracks to the album
            bool m_post_on_wall;
        };

        class NOVTABLE manager : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        public:
            virtual t_size get_preset_count () const = 0;

            // returns empty string if index is out of bound
            virtual const pfc::string8 & get_preset_name (t_size p_index) const = 0;

            // returns pfc::guid_null if preset not found
            virtual GUID get_preset_guid (const pfc::string8 &p_name) const = 0;

            // returns empty preset if preset not found
            virtual const preset & get_preset (const pfc::string8 &p_name) const = 0;

            // will create new preset if given preset does not exists
            // returns false on error
            virtual bool save_preset (const pfc::string8 &p_name, const preset &p_preset) = 0;

            // returns false then given preset does not exists
            virtual bool delete_preset (const pfc::string8 &p_name) = 0;

            template <typename t_func>
            void for_each_preset (t_func p_func) {
                static_api_ptr_t<upload_presets::manager> api;
                t_size n, max = api->get_preset_count ();
                for (n = 0; n < max; n++) p_func (api->get_preset_name (n));
            }
        };

        // {92CF789D-2064-428A-827D-B04CD5320F83}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x92cf789d, 0x2064, 0x428a, { 0x82, 0x7d, 0xb0, 0x4c, 0xd5, 0x32, 0xf, 0x83 } };
    }

    typedef static_api_ptr_t<upload_presets::manager> get_preset_manager;
}
#endif