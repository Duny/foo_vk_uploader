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
            auth_info m_info;

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
                    console::formatter () << redirect_url;

                    Json::Reader reader;
                    Json::Value val;

                    const char *begin = redirect_url.get_ptr ();
                    const char *end = begin + redirect_url.get_length ();
                    if (!reader.parse (begin, end, val, false) /*|| !val.isObject ()*/)
                        throw exception_auth_failed ("Couldn't parse redirect url as json");
                }
                return m_info;
            }
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}