#ifndef _FOO_VK_UPLOADER_VK_API_HELPERS_H_
#define _FOO_VK_UPLOADER_VK_API_HELPERS_H_

#include "vk_auth.h"

namespace vk_uploader
{
    namespace vk_api
    {  
        class request_url_builder
        {
            pfc::string_formatter m_url;
        public:
            request_url_builder (const char *p_method_name, params_cref p_params) 
            {
                m_url << "https://api.vk.com/method/" << p_method_name << "?";
                for (t_size i = 0, n = p_params.get_size (); i < n; i++)
                    m_url << p_params[i].first << "=" << p_params[i].second;
                m_url << "access_token=" << static_api_ptr_t<authorization>()->get_access_token ();
            }

            operator const char * () const { return m_url.get_ptr (); }
        };

        class url_params : public pfc::map_t<pfc::string8 const, pfc::string8>
        {
        public:
            url_params (const pfc::string8 &p_url)
            {
                t_size pos;

                if ((pos = p_url.find_first ('#')) != pfc_infinite || (pos = p_url.find_first ('?')) != pfc_infinite) {
                    t_size len = p_url.length (), pos2, pos3;
                    pfc::string8 name, value;
                    for (pos = pos; pos < len; pos = pos3) {
                        if ((pos2 = p_url.find_first ('=', pos)) == pfc_infinite)
                            break;

                        if ((pos3 = p_url.find_first ('&', pos2)) == pfc_infinite)
                            pos3 = len;

                        name.reset (); name.prealloc (pos2 - pos);
                        for (t_size i = pos + 1; i < pos2; i++)
                            name.add_char (p_url[i]);

                        value.reset (); value.prealloc (pos3 - pos2);
                        for (t_size i = pos2 + 1; i < pos3; i++)
                            value.add_char (p_url[i]);

                        this->find_or_add (name) = value;
                    }
                }
            }
        };
    }
}
#endif