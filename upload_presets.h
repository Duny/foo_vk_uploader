#ifndef _FOO_VK_UPLOADER_UPLOAD_PRESETS_H_
#define _FOO_VK_UPLOADER_UPLOAD_PRESETS_H_

namespace vk_uploader
{
    class NOVTABLE presets_manager : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(presets_manager)
    public:
        virtual t_size get_preset_count () const = 0;

        // returns empty string if index is out of bound
        virtual const pfc::string_base & get_preset_name (t_size p_index) const = 0;

        // returns pfc::guid_null if preset not found
        virtual GUID get_preset_guid (const pfc::string_base & p_name) const = 0;
        GUID get_preset_guid (t_size p_index) const { return get_preset_guid (get_preset_name (p_index)); }

        virtual bool has_preset (const GUID & p_guid) const = 0;

        // returns empty upload_params if not found
        virtual const upload_parameters & get_preset (const pfc::string_base & p_name) const = 0;
        virtual const upload_parameters & get_preset (const GUID & p_guid) const = 0;

        // will create new preset if given preset does not exists
        // returns false on error
        virtual bool save_preset (const pfc::string_base & p_name, const upload_parameters & p_preset) = 0;

        // returns false then given preset does not exists
        virtual bool delete_preset (const pfc::string_base & p_name) = 0;

        template <typename t_func>
        void for_each_preset (t_func p_func) {
            static_api_ptr_t<presets_manager> api;
            t_size n, max = api->get_preset_count ();
            for (n = 0; n < max; n++) p_func (api->get_preset_name (n));
        }
    };

    typedef static_api_ptr_t<vk_uploader::presets_manager> get_presets_manager;


    // {92CF789D-2064-428A-827D-B04CD5320F83}
    __declspec(selectany) const GUID presets_manager::class_guid = 
    { 0x92cf789d, 0x2064, 0x428a, { 0x82, 0x7d, 0xb0, 0x4c, 0xd5, 0x32, 0xf, 0x83 } };
}
#endif