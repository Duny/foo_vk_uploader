#include "stdafx.h"

#include "login_dlg.h"
#include "helpers.h"

namespace vk_uploader
{
    class menucomman_relogin : public mainmenu_commands
    {
        //! Retrieves number of implemented commands. Index parameter of other methods must be in 0....command_count-1 range.
        t_uint32 get_command_count () override { return 1; }

        //! Retrieves GUID of specified command.
        GUID get_command (t_uint32 p_index) override
        { return p_index == 0 ? guid_inline<0xa639a062, 0xcb28, 0x4a7b, 0x83, 0x89, 0x11, 0xcf, 0xd4, 0x5b, 0x8c, 0xdc>::guid : pfc::guid_null; }

        //! Retrieves name of item, for list of commands to assign keyboard shortcuts to etc.
        void get_name (t_uint32 p_index, pfc::string_base &p_out) override
        { if (p_index == 0) p_out = "Change vk.com profile / re login..."; }

        //! Retrieves item's description for statusbar etc.
        bool get_description (t_uint32 p_index, pfc::string_base &p_out) override 
        { if (p_index == 0) { p_out = "Activates Vk.com Login window."; return true; } return false; }

        //! Retrieves GUID of owning menu group.
        GUID get_parent () override { return mainmenu_groups::view; }

        //! Executes the command. p_callback parameter is reserved for future use and should be ignored / set to null pointer.
        void execute (t_uint32 p_index, service_ptr_t<service_base>) override
        { if (p_index == 0) login_dlg dlg (action_relogin); }
    };

    static mainmenu_commands_factory_t<menucomman_relogin> g_mainmenu_factiory;
}