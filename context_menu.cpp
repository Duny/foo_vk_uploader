#include "stdafx.h"

#include "vk_api.h"
#include "upload_preset.h"
#include "upload_queue.h"
#include "upload_thread.h"

namespace vk_uploader
{
    class node_root_leaf : public contextmenu_item_node_root_leaf
    {
        bool get_display_data (pfc::string_base &p_out, unsigned &p_displayflags, metadb_handle_list_cref p_data, const GUID &p_caller) override
        {
            p_out = m_preset_name;
            return true;
        }

        bool get_description (pfc::string_base &p_out) override
        {
            p_out = "Uploads selected files to your vk.com account";
            return true;
        }

        GUID get_guid () override { return get_preset_manager ()->get_preset_guid (m_preset_name); }

        bool is_mappable_shortcut () override { return true; }

    protected:
        pfc::string8 m_preset_name;

    public:
        node_root_leaf (const pfc::string8 &p_preset_name) : m_preset_name (p_preset_name) {}

        void execute (metadb_handle_list_cref p_data, const GUID &p_caller) override
        {
            upload_queue::push_back (p_data, get_preset_manager ()->get_preset (m_preset_name));
        }
    };

    class node_root_leaf_default : public node_root_leaf
    {
        GUID get_guid () override { return guid_inline<0x232ddfd8, 0x6655, 0x4eee, 0xa0, 0x2a, 0x41, 0x26, 0xea, 0x8d, 0x7f, 0xe9>::guid; }

        void execute (metadb_handle_list_cref p_data, const GUID &p_caller) override
        { show_upload_setup_dialog (p_data); }

    public:
        node_root_leaf_default () : node_root_leaf ("Upload to vk.com...") { } // hack to set custom text on the menu item)
    };

    class node_root_popup : public contextmenu_item_node_root_popup
    {
        bool get_display_data (pfc::string_base &p_out, unsigned &p_displayflags, metadb_handle_list_cref p_data, const GUID &p_caller) override
        {
            p_out = "Upload to vk.com";
            return true;
        }

        t_size get_children_count () override
        {
            return get_preset_manager ()->get_preset_count ()
                + 1 // separator
                + 1; // default item
        }

        contextmenu_item_node * get_child (t_size p_index) override
        {
            static_api_ptr_t<upload_presets::manager> api;

            auto preset_count = api->get_preset_count ();
            if (p_index < preset_count)
                return new node_root_leaf (api->get_preset_name (p_index));
            else if (p_index == preset_count)
                return new contextmenu_item_node_separator ();
            else
                return new node_root_leaf_default (); // default item
        }

        bool is_mappable_shortcut () override { return false; }

        GUID get_guid () override { return pfc::guid_null; }
    };

    class upload_to_vk : public contextmenu_item_v2
    {
        //! Retrieves number of menu items provided by this contextmenu_item implementation.
        unsigned get_num_items () override { return 1; }

        //! Instantiates a context menu item (including sub-node tree for items that contain dynamically-generated sub-items).
        contextmenu_item_node_root * instantiate_item (unsigned p_index, metadb_handle_list_cref p_data, const GUID &p_caller) override
        {
            if (get_preset_manager ()->get_preset_count () > 0)
                return new node_root_popup ();
            else
                return new node_root_leaf_default ();
        }

        //! Retrieves GUID of the context menu item.
        GUID get_item_guid (unsigned p_index) override { return pfc::guid_null; }

        //! Retrieves human-readable name of the context menu item.
        void get_item_name (unsigned p_index, pfc::string_base &p_out) override { }

        //! Retrieves item's description to show in the status bar. Set p_out to the string to be displayed and return true if you provide a description, return false otherwise.
        bool get_item_description (unsigned p_index, pfc::string_base &p_out) override { return false; }

        //! Controls default state of context menu preferences for this item: \n
        //! Return DEFAULT_ON to show this item in the context menu by default - useful for most cases. \n
        t_enabled_state get_enabled_state (unsigned p_index) override { return DEFAULT_ON; }

        //! Executes the menu item command without going thru the instantiate_item path. For items with dynamically-generated sub-items, p_node is identifies of the sub-item command to execute.
        void item_execute_simple (unsigned p_index, const GUID &p_node, metadb_handle_list_cref p_data, const GUID &p_caller) override
        {
            static_api_ptr_t<upload_presets::manager> api;

            if (api->has_preset (p_node))
                upload_queue::push_back (p_data, api->get_preset (p_node));
            else
                show_upload_setup_dialog (p_data);
        }

        GUID get_parent () override { return contextmenu_groups::fileoperations; }
    };

    static contextmenu_item_factory_t<upload_to_vk> g_contextmenu_factory;
}