#pragma once

class contextmenu_item_vk_upload : public contextmenu_item_simple
{
private:
	enum menuitem
	{
		upload_item,
	    total_items
	};

    GUID get_parent () { return contextmenu_groups::fileoperations; }
    unsigned get_num_items () { return total_items; }
	void get_item_name (unsigned p_index, pfc::string_base & p_out);
	void context_command (unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller);
	GUID get_item_guid (unsigned p_index);
	bool get_item_description (unsigned p_index,pfc::string_base & p_out);
};