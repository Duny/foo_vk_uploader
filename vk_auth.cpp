#include "stdafx.h"

// user_id, access_token, timestamp of authentication, expires_in (how long access_token live)
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
            if (!auth_data_is_valid ()) get_auth_data ();
        }

        void get_auth_data ()
        {
            pfc::string8_fast location;
            bool success;

            auto navigate_callback = [&] (browser_dialog *p_dlg)
            {
                location = p_dlg->get_browser_location ();
                console::formatter () << location;

                if (location.find_first ("cancel=1") != pfc_infinite ||
                    location.find_first ("user_denied") != pfc_infinite) { // user pressed "Cancel" button
                    success = false;
                    p_dlg->close ();
                }
                else if (location.find_first ("blank.html#") != pfc_infinite) { // if address contains "blank.html#" (part of VK_COM_BLANK_URL), it means that auth was done successfully 
                    success = true;
                    p_dlg->close ();
                }
            };

            do {
                success = false;
                open_browser_dialog (VK_COM_LOGIN_URL, navigate_callback);
                if (!success && uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                    throw exception_aborted ();
            } while (!success);
            

            url_params params (location);
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

        void relogin () override
        {
            open_browser_dialog (VK_COM_LOGOUT_URL,  [](browser_dialog *p_dlg) { p_dlg->close (); });

            try {
                m_auth_data.val () = auth_data ();
                get_auth_data ();
            }
            catch (exception_aborted) {}
            catch (const std::exception &e) {
                uMessageBox (core_api::get_main_window (), e.what (), "Error during authorization", MB_OK | MB_ICONERROR);
            }
            clear_album_list ();
        }

        static cfg_obj<auth_data> m_auth_data;
    };
    cfg_obj<auth_data> auth_manager_imp::m_auth_data (guid_inline<0xa1324e98, 0xc549, 0x4ba8, 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62>::guid);

    static service_factory_single_t<auth_manager_imp> g_auth_factory;
}