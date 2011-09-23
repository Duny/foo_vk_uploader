#pragma once

#include "foobar2000\ATLHelpers\ATLHelpers.h"

#include "resource.h"

#include "upload_thread.h"
#include "context_menu.h"

#include "json/reader.h"
#include "json/value.h"

#include "vk_api.h"

extern const pfc::string8 g_app_id;

// Config variables
extern cfg_string vk_access_token;

extern cfgWindowSize vk_login_dlg_size;
extern cfgDialogPosition vk_login_dlg_pos;

extern cfgDialogPosition vk_upload_setup_dlg_pos;

void upload_items (metadb_handle_list_cref p_items);