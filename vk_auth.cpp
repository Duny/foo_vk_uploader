#include "stdafx.h"

#include "vk_api.h"
#include "vk_auth.h"
#include "utils.h"
#include "login_dlg.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class NOVTABLE authorization_imp : public authorization
        {
            mutable auth_info m_info;

            const auth_info & get_info () const override
            {
                if (m_info.m_secret.is_empty () || m_info.m_sid.is_empty () || m_info.m_user_id.is_empty ()) {
                    pfc::string8 redirect_url;

                    bool done = false;
                    while (!done) {
                        login_dlg dlg;
                        dlg.DoModal (core_api::get_main_window ()); 
                        redirect_url = dlg.get_final_url ();
                        if (redirect_url.find_first (vk_api::string_constants::redirect_url_ok) == 0)
                            done = true;
                        else {
                            if (uMessageBox (core_api::get_main_window (), "Try again?", "vk.com authorization", MB_YESNO | MB_ICONQUESTION) == IDNO)
                                throw exception_auth_failed ();
                        }
                    }

                    skip_prefix (redirect_url, vk_api::string_constants::redirect_url_ok);
                    skip_prefix (redirect_url, "#session=");
                    redirect_url = url_decode (redirect_url);

                    value_t data = from_string (redirect_url);
                    if (!data || !data->isObject () || data->size () != 5)
                        throw exception_auth_failed ("Couldn't parse redirect url as json");

                    try {
                        auth_info new_info;
                        new_info.m_user_id = pfc::format_uint ((*data)["mid"].asUInt ());
                        new_info.m_secret = (*data)["secret"].asCString ();
                        new_info.m_sid = (*data)["sid"].asCString ();
                        m_info = new_info;
                    } catch (...) {
                        throw exception_auth_failed ("Not enough parameters");
                    }
                }
                return m_info;
            }
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}