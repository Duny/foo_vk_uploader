#include "stdafx.h"

#include "vk_api.h"
#include "vk_auth.h"
#include "vk_api_helpers.h"
#include "utils.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_api
    {
        const char *redirect_url_ok = "http://api.vk.com/blank.html";

        class NOVTABLE authorization_imp : public authorization
        {
            mutable pfc::string8 m_user_id;
            mutable pfc::string8 m_access_token;

            void get_auth () const
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

                if (params.have_item (pfc::string8 ("error")))
                    throw exception_auth_failed (params[pfc::string8 ("error")]);

                if (params.have_item (pfc::string8 ("access_token")) && 
                    params.have_item (pfc::string8 ("user_id"))) {
                    m_access_token = params[pfc::string8 ("access_token")];
                    m_user_id = params[pfc::string8 ("user_id")];
                }
                else
                    throw exception_auth_failed ("Unexpected server redirect");
            }

            const char *get_user_id () const override
            {
                if (m_user_id.is_empty ()) get_auth ();
                return m_user_id;
            }

            const char *get_access_token () const override
            {
                if (m_access_token.is_empty ()) get_auth ();
                return m_access_token;
            }
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}