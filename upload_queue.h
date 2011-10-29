#ifndef _FOO_VK_UPLOADER_UPLOAD_QUEUE_H_
#define _FOO_VK_UPLOADER_UPLOAD_QUEUE_H_

namespace vk_uploader
{
    namespace upload_queue
    {
        class NOVTABLE manager : public service_base
        {
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        public:
            virtual void push_back (metadb_handle_list_cref p_items, const upload_presets::preset &p_preset) = 0;
        };

        // {F28D8DE2-6A03-489F-8C47-32F0E9A24249}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0xf28d8de2, 0x6a03, 0x489f, { 0x8c, 0x47, 0x32, 0xf0, 0xe9, 0xa2, 0x42, 0x49 } };
    }

    typedef static_api_ptr_t<upload_queue::manager> get_upload_queue;
}
#endif