#include "stdafx.h"

#include "config.h"

namespace vk_uploader
{
    namespace cfg
    {
        namespace guid
        {
            const GUID dialog_pos = { 0xcc80a64a, 0x44de, 0x4735, { 0x93, 0xdf, 0xc7, 0x92, 0x63, 0xbb, 0x3a, 0x23 } };
        }

        cfgDialogPosition dialog_pos (guid::dialog_pos);
    }
}