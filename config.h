#ifndef _FOO_VK_UPLOADER_CONFIG_H_
#define _FOO_VK_UPLOADER_CONFIG_H_

namespace vk_uploader
{
    namespace cfg
    {
        struct upload_profile
        {
            upload_profile (const GUID &p_guid = pfc::guid_null, const pfc::string8 &p_name = "")
                : m_guid (p_guid), m_name (p_name) {}
            static upload_profile default_;

            bool is_default () const { return IsEqualGUID (default_.m_guid, m_guid) && default_.m_name == m_name; }

            GUID m_guid;
            pfc::string8 m_name; // profile name
        };
        FB2K_STREAM_READER_OVERLOAD(upload_profile)
        {
            return stream >> value.m_guid >> value.m_name;
        }
        FB2K_STREAM_WRITER_OVERLOAD(upload_profile)
        {
            return stream << value.m_guid << value.m_name;
        }

        extern cfg_objList<upload_profile> upload_profiles;
        extern cfgDialogPosition login_dialog_pos, upload_dialog_pos;
    }
}
#endif