#include "stdafx.h"

DECLARE_COMPONENT_VERSION ("Vk.com Music Uploader", "1.2.5",
"Uploads music to your profile in vk.com/vkontakte.ru social network.\n\n\
(c) 2011-2012 Efimenko Dmitry (majorquake3@gmail.com)");

VALIDATE_COMPONENT_FILENAME (COMPONENT_NAME".dll");


// vk_com_api guid definitions
const GUID vk_com_api::externals::guid_browser_dlg_cfg =
{ 0xf7d4e54a, 0x5d5d, 0x482e, { 0x89, 0x4a, 0x4f, 0x10, 0x3c, 0x02, 0x50, 0x6a } };

const GUID vk_com_api::externals::guid_auth_manager_cfg =
{ 0xa1324e98, 0xc549, 0x4ba8, { 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62 } };

const GUID vk_com_api::externals::auth_manager_class_guid = 
{ 0x911ED77D, 0x3820, 0x4B8E, { 0xBE, 0x4F, 0x6E, 0xF3, 0x00, 0x29, 0x67, 0x0B } };

const GUID vk_com_api::externals::api_provider_class_guid = 
{ 0x415971ba, 0x5773, 0x4843, { 0x9d, 0x18, 0x9, 0xf2, 0x80, 0x74, 0xf5, 0xf7 } };


// vk api string defs
const char * vk_com_api::externals::auth_browser_dlg_caption_prefix = "Vk.com uploader : ";

const char * vk_com_api::externals::vk_api_application_id = "2632594";

const char * vk_com_api::externals::component_name_for_console_log = vk_com_api::externals::auth_browser_dlg_caption_prefix;