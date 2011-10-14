#include "stdafx.h"

#include <time.h>

#include "vk_api.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_api
    {
        const char *redirect_url_ok = "http://api.vk.com/blank.html";

        class NOVTABLE authorization_imp : public authorization
        {
            pfc::string8 m_user_id;
            pfc::string8 m_access_token;
            time_t m_timestamp; // time then auth was done
            t_uint32 m_expires_in; 
            
            void get_auth ()
            {
                pfc::string8 redirect_url;
                
                bool done = false;
                while (!done) {
                    login_dlg dlg;
                    dlg.DoModal (core_api::get_main_window ()); 
                    redirect_url = dlg.get_final_url ();
                    if (redirect_url.find_first (redirect_url_ok) == 0)
                        done = true;
                    else {
                        if (uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                            throw exception_auth_failed ();
                    }
                }

                url_params params (redirect_url);

                if (params.have ("error"))
                    throw exception_auth_failed (params[pfc::string8 ("error")]);

                if (params.have ("access_token") && params.have ("user_id") && params.have ("expires_in")) {
                    m_timestamp = time (nullptr);
                    m_access_token = params["access_token"];
                    m_user_id = params["user_id"];
                    m_expires_in = pfc::atoui_ex (params["expires_in"], pfc_infinite);
                }
                else
                    throw exception_auth_failed ("Unexpected server redirect");
            }

            void check_auth ()
            {
                if (m_user_id.is_empty () || m_access_token.is_empty () || 
                    time (nullptr) > (m_timestamp + m_expires_in))
                    get_auth ();
            }

            const char *get_user_id () override
            {
                check_auth ();
                return m_user_id;
            }

            const char *get_access_token () override
            {
                check_auth ();
                return m_access_token;
            }
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}