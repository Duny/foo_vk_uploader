#include "stdafx.h"

#include "config.h"

namespace vk_uploader
{
    namespace cfg
    {
        namespace guid
        {
            const GUID upload_profiles = { 0xbfeaa7ea, 0x6810, 0x41c6, { 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49 } };
            const GUID default_profile = { 0x4d9b8126, 0x1ca, 0x40ea, { 0xb7, 0x99, 0x8f, 0x10, 0x5a, 0xfc, 0x71, 0xee } };
        }

        upload_profile upload_profile::default_ (guid::default_profile, "...");

        cfg_objList<upload_profile> upload_profiles (guid::upload_profiles);
    }
}