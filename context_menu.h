#pragma once

#include "upload_profiles.h"
#include "upload_setup_dlg.h"

namespace vk_uploader
{
    namespace conext_menu
    {
        class node_root_leaf : public contextmenu_item_node_root_leaf
        {
            bool get_display_data (pfc::string_base &p_out, unsigned &p_displayflags, metadb_handle_list_cref p_data, const GUID &p_caller) override
            {
                p_out = m_profile.m_name;
                return true;
            }

            void execute (metadb_handle_list_cref p_data, const GUID &p_caller) override
            {
                // TODO
                //cfg::upload_profiles.add_item (cfg::upload_profile ());
                //cfg::upload_profiles.remove_all ();
                static_api_ptr_t<upload_profiles::upload_setup_dialog>()->show (p_data);
            }

            bool get_description (pfc::string_base &p_out) override
            {
                p_out = "Uploads selected files to vk.com";
                return true;
            }

            GUID get_guid () override { return m_profile.m_guid; }

            bool is_mappable_shortcut () override { return true; }

            upload_profiles::profile m_profile;

        public:
            node_root_leaf (const upload_profiles::profile &p_profile) : m_profile (p_profile) {}
        };

        class node_root_leaf_default : public node_root_leaf
        {
            bool get_display_data (pfc::string_base &p_out, unsigned &p_displayflags, metadb_handle_list_cref p_data, const GUID &p_caller) override
            {
                p_out.set_string ("Upload to vk...");
                return true;
            }

        public:
            node_root_leaf_default () : 
              node_root_leaf (static_api_ptr_t<upload_profiles::manager>()->get_default_profile ()) {}
        };

        class node_root_popup : public contextmenu_item_node_root_popup
        {
            bool get_display_data (pfc::string_base &p_out, unsigned &p_displayflags, metadb_handle_list_cref p_data, const GUID &p_caller) override
            {
                p_out = "Upload to vk";
                return true;
            }
            
            t_size get_children_count () override
            {
                return static_api_ptr_t<upload_profiles::manager>()->get_count ()
                    + 1 // separator
                    + 1; // default "..." item
            }

            contextmenu_item_node * get_child (t_size p_index) override
            {
                static_api_ptr_t<upload_profiles::manager> api;

                auto profile_count = api->get_count ();
                if (p_index < profile_count)
                    return new node_root_leaf (api->get_profile (p_index));
                else if (p_index == profile_count)
                    return new contextmenu_item_node_separator ();
                else
                    return new node_root_leaf (api->get_default_profile ()); // default "..." item
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
                if (static_api_ptr_t<upload_profiles::manager>()->get_count () > 0)
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
                int i = 0;
            }

            GUID get_parent () override { return contextmenu_groups::fileoperations; }
        };
    }
}