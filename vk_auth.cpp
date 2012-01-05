#include "stdafx.h"

#include "vk_auth.h"
#include "vk_api.h"
#include "browser.h"


// user_id, access_token, timestamp of authentication, expires_in (how long access_token live)
typedef tuple<pfc::string8, pfc::string8, time_t, t_uint32> auth_data;
enum { field_user_id, field_access_token, field_timestamp, field_expires_in };

FB2K_STREAM_READER_OVERLOAD(auth_data) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(auth_data) { return write_tuple (stream, value); }

namespace vk_uploader
{
    class auth_manager_impl : public vk_auth_manager
    {
        // vk_auth_manager overrides
        bool is_valid () const override
        { 
            if (m_auth_data.get<field_user_id>().is_empty () || m_auth_data.get<field_access_token>().is_empty ())
                return false;

            if (m_auth_data.get<field_expires_in>() > 0 &&
                (time (nullptr) > (m_auth_data.get<field_timestamp>() + m_auth_data.get<field_expires_in>()))) return false;

            return true;
        }

        const pfc::string_base & get_user_id () override
        {
            check_auth_data ();
            return m_auth_data.get<field_user_id>();
        }

        const pfc::string_base & get_access_token () override
        {
            check_auth_data ();
            return m_auth_data.get<field_access_token>();
        }

        void relogin () override
        {
            const auth_data auth_data_empty;

            browser_dialog("Logging out from vk.com", VK_COM_LOGOUT_URL,  [](browser_dialog *p_dlg) { p_dlg->close (); }).show ();

            m_auth_data.val () = auth_data_empty; // Clear current data
            user_album_list ().clear (); // Clear current user album list

            // Get new data
            try { get_auth_data (); }
            catch (exception_aborted) {}
            catch (const std::exception & e) {
                uMessageBox (core_api::get_main_window (), e.what (), "Error during authorization", MB_OK | MB_ICONERROR);
            }
        }

        // Helpers
        void check_auth_data ()
        {
            if (!is_valid ()) get_auth_data ();
        }

        void get_auth_data ()
        {
            pfc::string8_fast location;
            bool success;

            auto navigate_callback = [&] (browser_dialog *p_dlg)
            {
                location = p_dlg->get_browser_location ();

                // Close if user pressed "Cancel" button
                if (location.find_first ("cancel=1") != pfc_infinite || location.find_first ("user_denied") != pfc_infinite) {
                    success = false;
                    p_dlg->close ();
                }
                // if address contains "blank.html#" (part of VK_COM_BLANK_URL), when auth was done successfully 
                else if (location.find_first ("blank.html#") != pfc_infinite) { 
                    success = true;
                    p_dlg->close ();
                }
            };

            do {
                success = false;
                browser_dialog ("Logging to vk.com", VK_COM_LOGIN_URL, navigate_callback).show ();
                if (!success && uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                    throw exception_aborted ();
            } while (!success);
            

            url_parameters params = construct_from_url (location);
            if (have (params, list_of ("error")))
                throw pfc::exception (get (params, "error"));
            else if (have (params, list_of ("access_token")("user_id")("expires_in")))
                m_auth_data.val () = make_tuple (get (params, "user_id"), get (params, "access_token"), time (nullptr), get_uint (params, "expires_in"));
            else
                throw pfc::exception (pfc::string_formatter () << "Unexpected server redirect:" << location);
        }

        // Member variables
        cfg_obj<auth_data> m_auth_data;

    public:
        auth_manager_impl () : m_auth_data (create_guid (0xa1324e98, 0xc549, 0x4ba8, 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62)) {}
    };

    namespace { service_factory_single_t<auth_manager_impl> g_auth_factory; }
}