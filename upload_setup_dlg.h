#pragma once

#include "upload_profiles.h"

namespace vk_uploader
{
    namespace upload_profiles
    {
        class NOVTABLE upload_setup_dialog : public service_base
        {
        public:
            virtual void show (metadb_handle_list_cref p_items, 
                const upload_profiles::profile &p_profile = upload_profiles::profile ()) = 0;

            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(upload_setup_dialog)
        };


        // {407762AB-7464-4EF1-B3E6-3E95CBB62367}
        __declspec(selectany) const GUID upload_setup_dialog::class_guid = 
        { 0x407762ab, 0x7464, 0x4ef1, { 0xb3, 0xe6, 0x3e, 0x95, 0xcb, 0xb6, 0x23, 0x67 } };
    }
}