#include "stdafx.h"

#include <time.h>

#include "vk_api.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_api
    {
        namespace cfg
        {
            struct auth_data_t
            {
                bool is_empty () const { return m_user_id.is_empty () || m_access_token.is_empty (); }

                pfc::string8 m_user_id;
                pfc::string8 m_access_token;
                time_t m_timestamp; // time, then auth was done
                t_uint32 m_expires_in; 
            };

            FB2K_STREAM_READER_OVERLOAD(auth_data_t) { return stream >> value.m_user_id >> value.m_access_token >> value.m_timestamp >> value.m_expires_in; }
            FB2K_STREAM_WRITER_OVERLOAD(auth_data_t) { return stream << value.m_user_id << value.m_access_token << value.m_timestamp << value.m_expires_in; }

            cfg_obj<auth_data_t> auth_data (guid_inline<0xa1324e98, 0xc549, 0x4ba8, 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62>::guid);
        }

        class NOVTABLE authorization_imp : public authorization
        {
            void get_auth_data ()
            {
                pfc::string8 redirect_url;
                while (true) {
                    redirect_url = login_dlg ().get_browser_location ();
                    if (is_blank_page (redirect_url))
                        break;
                    else {
                        if (uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                            throw exception_auth_failed ();
                    }
                }

                url_params params (redirect_url);

                if (params.have ("error"))
                    throw exception_auth_failed (params["error"]);
                else if (params.have ("access_token") && params.have ("user_id") && params.have ("expires_in")) {
                    cfg::auth_data.m_timestamp = time (nullptr);
                    cfg::auth_data.m_access_token = params["access_token"];
                    cfg::auth_data.m_user_id = params["user_id"];
                    cfg::auth_data.m_expires_in = pfc::atoui_ex (params["expires_in"], pfc_infinite);
                }
                else
                    throw exception_auth_failed ("Unexpected server redirect");
            }

            void check_auth_data ()
            {
                if (cfg::auth_data.is_empty () || time (nullptr) > (cfg::auth_data.m_timestamp + cfg::auth_data.m_expires_in))
                    get_auth_data ();
            }

            const char *get_user_id () override
            {
                check_auth_data ();
                return cfg::auth_data.m_user_id;
            }

            const char *get_access_token () override
            {
                check_auth_data ();
                return cfg::auth_data.m_access_token;
            }

            void relogin_user () override
            {
                // THIS NEED TO BE TEST!!!!!! Relogin + upload
                login_dlg dlg (action_logout);
                get_auth_data ();
            }
        };
        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}