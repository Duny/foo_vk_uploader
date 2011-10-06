#include "stdafx.h"

#include "vk_api.h"
#include "vk_auth.h"
#include "utils.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_api
    {
        const char *redirect_url_ok = "http://vk.com/api/login_success.html";
        const char *redirect_url_err = "http://vk.com/api/login_failure.html";

        class NOVTABLE authorization_imp : public authorization
        {
            mutable pfc::string8 m_user_id;
            mutable pfc::string8 m_secret;
            mutable pfc::string8 m_sid;

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

                skip_prefix (redirect_url, redirect_url_ok);
                skip_prefix (redirect_url, "#session=");
                redirect_url = url_decode (redirect_url);

                value_t data = from_string (redirect_url);
                if (!data || !data->isObject () || data->size () != 5)
                    throw exception_auth_failed ("Couldn't parse redirect url as json");

                try {
                    pfc::string8 new_user_id = pfc::format_uint ((*data)["mid"].asUInt ());
                    pfc::string8 new_secret = (*data)["secret"].asCString ();
                    pfc::string8 new__sid = (*data)["sid"].asCString ();
                    
                    m_user_id = new_user_id;
                    m_secret = new_secret;
                    m_sid = new__sid;
                } catch (...) {
                    throw exception_auth_failed ("Not enough parameters");
                }
            }

            const char *get_user_id () const override
            {
                if (m_user_id.is_empty ()) get_auth ();
                return m_user_id;
            }

            const char *get_secret () const override
            {
                if (m_secret.is_empty ()) get_auth ();
                return m_secret;
            }

            const char *get_sid () const override
            {
                if (m_sid.is_empty ()) get_auth ();
                return m_sid;
            }
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}