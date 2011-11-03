#include "stdafx.h"

#include <time.h>

#include "vk_api.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_auth
    {
        namespace cfg
        {
            struct auth_data_t
            {
                bool is_valid () const { 
                    if (m_user_id.is_empty () || m_access_token.is_empty ()) return false;
                    if (m_expires_in > 0 && (time (nullptr) > (m_timestamp + m_expires_in))) return false;
                    return true;
                }

                void reset () {
                    m_user_id.reset ();
                    m_access_token.reset ();
                    m_timestamp = 0;
                    m_expires_in = 0;
                }

                pfc::string8 m_user_id;
                pfc::string8 m_access_token;
                time_t m_timestamp; // time, then auth was done
                t_uint32 m_expires_in; 
            };

            FB2K_STREAM_READER_OVERLOAD(auth_data_t) { return stream >> value.m_user_id >> value.m_access_token >> value.m_timestamp >> value.m_expires_in; }
            FB2K_STREAM_WRITER_OVERLOAD(auth_data_t) { return stream << value.m_user_id << value.m_access_token << value.m_timestamp << value.m_expires_in; }
        }

        class NOVTABLE manager_imp : public manager
        {
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
                    m_auth_data.m_timestamp = time (nullptr);
                    m_auth_data.m_access_token = params["access_token"];
                    m_auth_data.m_user_id = params["user_id"];
                    m_auth_data.m_expires_in = pfc::atoui_ex (params["expires_in"], pfc_infinite);
                }
                else
                    throw std::exception ("Unexpected server redirect");
            }

            const char *get_user_id () override
            {
                if (!m_auth_data.is_valid ())
                    get_auth_data (login_dlg::action_do_login);
                return m_auth_data.m_user_id;
            }

            const char *get_access_token () override
            {
                if (!m_auth_data.is_valid ())
                    get_auth_data (login_dlg::action_do_login);
                return m_auth_data.m_access_token;
            }

            void relogin_user () override
            {
                m_auth_data.reset ();
                get_auth_data (login_dlg::action_do_relogin);
            }

            static cfg_obj<cfg::auth_data_t> m_auth_data;
        };
        cfg_obj<cfg::auth_data_t> manager_imp::m_auth_data (guid_inline<0xa1324e98, 0xc549, 0x4ba8, 0x98, 0xc0, 0x6a, 0x2, 0x46, 0xa9, 0xa, 0x62>::guid);

        static service_factory_single_t<manager_imp> g_auth_factory;
    }
}