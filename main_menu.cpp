#include "stdafx.h"

#include "vk_com_api_auth.h"


namespace vk_uploader
{
    class mainmenu_group_popup_vk_uploader : public mainmenu_group_popup
    {
        // mainmenu_group overrides
        GUID get_guid () override
        {
            return get_my_guid ();
        }

	    GUID get_parent () override
        {
            return mainmenu_groups::view;
        }

	    t_uint32 get_sort_priority () override
        {
            return mainmenu_commands::sort_priority_dontcare;
        }

        // mainmenu_group_popup overrides
        void get_display_string (pfc::string_base & p_out) override
        {
            p_out = "Vk.com uploader";
        }

    public:

        static GUID get_my_guid ()
        {
            return create_guid (0x7a0d49a5, 0x0e24, 0x42ce, 0xab, 0xa0, 0xdb, 0x8b, 0x2a, 0x9b, 0xe0, 0x81);
        }
    };

    class menucomman_relogin_upload : public mainmenu_commands
    {
        //! Retrieves number of implemented commands. Index parameter of other methods must be in 0....command_count-1 range.
        t_uint32 get_command_count () override { return 2; }

        //! Retrieves GUID of specified command.
        GUID get_command (t_uint32 p_index) override
        {
            switch (p_index) {
                case 0: // Login/relogin
                    return create_guid (0xa639a062, 0xcb28, 0x4a7b, 0x83, 0x89, 0x11, 0xcf, 0xd4, 0x5b, 0x8c, 0xdc);
                case 1: // Show default upload setup dialog
                    return create_guid (0xf6773761, 0xcca6, 0x48c2, 0x9f, 0x27, 0x96, 0xd2, 0xd6, 0x95, 0x71, 0xcd);
                default:
                    return pfc::guid_null;
            }
        }

        //! Retrieves name of item, for list of commands to assign keyboard shortcuts to etc.
        void get_name (t_uint32 p_index, pfc::string_base &p_out) override
        {
            switch (p_index) {
                case 0: // Login/relogin
                    p_out = vk_com_api::get_auth_manager()->user_logged () ? "Relogin" : "Login";
                    p_out += " to vk.com...";
                    break;
                case 1: // Show default upload setup dialog
                    p_out = "Upload to vk.com...";
                    break;
            }
        }

        //! Retrieves item's description for status bar etc.
        bool get_description (t_uint32 p_index, pfc::string_base &p_out) override 
        {
            switch (p_index) {
                case 0: // Login/relogin
                    p_out = "Activates Vk.com Login window.";
                    return true;
                case 1: // Show default upload setup dialog
                    p_out = "Opens Upload Setup window";
                    return true;
            }
            return false;
        }

        //! Retrieves GUID of owning menu group.
        GUID get_parent () override
        {
            return mainmenu_group_popup_vk_uploader::get_my_guid ();
        }

        //! Executes the command. p_callback parameter is reserved for future use and should be ignored / set to null pointer.
        void execute (t_uint32 p_index, service_ptr_t<service_base>) override
        {
            switch (p_index) {
                case 0: // Login/relogin
                    vk_com_api::get_auth_manager()->relogin ();
                    break;
                case 1: // Show default upload setup dialog
                    show_upload_setup_dialog ();
                    break;
            }
        }
    };

    namespace
    {
        mainmenu_commands_factory_t<menucomman_relogin_upload> g_mainmenu_commands_factory;
        mainmenu_commands_factory_t<mainmenu_group_popup_vk_uploader> g_mainmenu_group_popup_factory;
    }
}