#include "stdafx.h"

#include <time.h>
#include "login_dlg.h"

// user_id, access_token, timestamp (time of authentification), expires_in (how long access_token is available)
typedef boost::tuple<pfc::string8_fast, pfc::string8_fast, time_t, t_uint32> auth_data;
enum { field_user_id, field_access_token, field_timestamp, field_expires_in };

FB2K_STREAM_READER_OVERLOAD(auth_data) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(auth_data) { return write_tuple (stream, value); }

namespace vk_uploader
{
    class NOVTABLE auth_manager_imp : public vk_auth_manager
    {
        inline bool auth_data_is_valid () const
        { 
            if (m_auth_data.get<field_user_id>().is_empty () ||
                m_auth_data.get<field_access_token>().is_empty ())
                return false;

            if (m_auth_data.get<field_expires_in>() > 0 &&
                (time (nullptr) > (m_auth_data.get<field_timestamp>() + m_auth_data.get<field_expires_in>()))) return false;

            return true;
        }

        inline void check_auth_data ()
        {
            if (!auth_data_is_valid ())
                get_auth_data (login_dlg::action_do_login);
        }

        void get_auth_data (login_dlg::t_login_action action)
        {
            pfc::string8 redirect_url;
            while (true) {
                redirect_url = login_dlg (action).get_browser_location ();

                if (redirect_url.find_first ("cancel=1") != pfc_infinite ||
                    redirect_url.find_first ("user_denied") != pfc_infinite) // user pressed "Cancel" button
                    throw exception_aborted ();
                else if (redirect_url.find_first (VK_COM_BLANK_URL) == 0) // if address starts from VK_COM_BLANK_URL, it means that auth was done successfully 
                    break;
                else {
                    if (uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                        throw exception_aborted ();
                }
            }

            url_params params (redirect_url);

            if (params.have ("error"))
                throw std::exception (params["error"]);
            else if (params.have ("access_token") && params.have ("user_id") && params.have ("expires_in")) {
                m_auth_data.get<field_timestamp>() = time (nullptr);
                m_auth_data.get<field_access_token>() = params["access_token"];
                m_auth_data.get<field_user_id>() = params["user_id"];
                m_auth_data.get<field_expires_in>() = pfc::atoui_ex (params["expires_in"], pfc_infinite);
            }
            else
                throw std::exception ("Unexpected server redirect");
        }

        pfc::string8_fast get_user_id () override
        {
            check_auth_data ();
            return m_auth_data.get<field_user_id>();
        }

        pfc::string8_fast get_access_token () override
        {
            check_auth_data ();
            return m_auth_data.get<field_access_token>();
        }

        void relogin_user () override
        {
            m_auth_data.val () = auth_data ();
            get_auth_data (login_dlg::action_do_relogin);
        }

        static cfg_obj<auth_data> m_auth_data;
    };
    cfg_obj<auth_data> auth_manager_imp::m_auth_data (guid_inline<0xa1324e98, 0xc549, 0x4ba8, 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62>::guid);

    static service_factory_single_t<auth_manager_imp> g_auth_factory;
}