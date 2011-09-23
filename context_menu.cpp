#include "stdafx.h"
#include "upload_setup_dialog.h"

static const GUID guid_menuitem_upload_to_vk = { 0x59e2ce3d, 0xf02c, 0x4f65, { 0xb6, 0x7a, 0x29, 0xdd, 0x94, 0xf3, 0xa8, 0x97 } };


void contextmenu_item_vk_upload::get_item_name (unsigned p_index, pfc::string_base & p_out)
{ 
	switch (p_index) {
        case upload_item: {
		    p_out = "Upload to vk.com";
        } break;
	}
}

void contextmenu_item_vk_upload::context_command (unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller)
{
    if (!p_data.get_size ()) return;

    switch (p_index) {
        case upload_item: {
            vk_upload_setup_dialog * dlg = new vk_upload_setup_dialog (p_data);
            dlg->show ();
        } break;
    }
}

GUID contextmenu_item_vk_upload::get_item_guid (unsigned p_index)
{
	switch (p_index) {
        case upload_item: {
			return guid_menuitem_upload_to_vk;
        } break;
	}

	return pfc::guid_null;
}

bool contextmenu_item_vk_upload::get_item_description (unsigned p_index, pfc::string_base & p_out)
{
	switch (p_index) {
        case upload_item: {
	        p_out = "Upload tracks to vk.com";
            return true;
        }
	}

	return false;
}

static contextmenu_item_factory_t<contextmenu_item_vk_upload> g_contextmenu_factory;