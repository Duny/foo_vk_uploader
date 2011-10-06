#ifndef _FOO_VK_UPLOADER_VK_API_HELPERS_H_
#define _FOO_VK_UPLOADER_VK_API_HELPERS_H_

#include "vk_auth.h"

namespace vk_uploader
{
    namespace vk_api
    {  
        class string_formatter_hasher_md5 : public pfc::string_formatter
        {
        public:
            void get_result (pfc::string8 &p_out) 
            {
                hasher_md5_state state;
                static_api_ptr_t<hasher_md5> hasher;
                hasher->initialize (state);

                hasher->process_string (state, get_ptr ());
                p_out = pfc::format_hexdump (hasher->get_result (state).m_data, sizeof (hasher_md5_result), "");
                /*pfc::string8 s;
                for (t_size i = 0, n = p_out.get_length (); i < n; i++)
                    s.add_char (pfc::ascii_tolower (p_out[i]));
                p_out = s;*/
            }
        };

        class signature
        {
            pfc::string8 m_sig;
        public:
            signature (params_cref p_params)
            {
                string_formatter_hasher_md5 hasher;

                hasher << static_api_ptr_t<authorization>()->get_user_id ();
                for (t_size i = 0, n = p_params.get_size (); i < n; i++)
                    hasher << p_params[i].first << "=" << p_params[i].second;
                hasher << static_api_ptr_t<authorization>()->get_secret ();

                console::formatter () << hasher.get_ptr ();

                hasher.get_result (m_sig);
            }

            operator const char * () const { return m_sig.get_ptr (); }
        };

        class request_url_builder
        {
            pfc::string_formatter m_url;
        public:
            request_url_builder (const char *p_api_name, params_cref p_params) 
            {
                params_t params = p_params;
                params.add_item (std::make_pair ("api_id", vk_api::app_id));
                //params.add_item (std::make_pair ("format", "json"));
                params.add_item (std::make_pair ("method", p_api_name));
                params.add_item (std::make_pair ("v", "3.0"));

                params.sort_t<>([] (const url_parameter &one, const url_parameter &two) -> int 
                    { return pfc::stricmp_ascii (one.first, two.first); }
                );

                params.add_item (std::make_pair ("sig", signature (params)));
                params.add_item (std::make_pair ("sid", static_api_ptr_t<authorization>()->get_sid ()));

                m_url << "http://api.vk.com/api.php?";
                for (t_size i = 0, n = params.get_size (); i < n; i++)
                    m_url << params[i].first << "=" << params[i].second << (i < n - 1 ? "&" : "");
            }

            operator const char * () const { return m_url.get_ptr (); }
        };

        //class url_params : public pfc::map_t<pfc::string8 const, pfc::string8>
        //{
        //public:
        //    url_params (const pfc::string8 &p_url)
        //    {
        //        // find start symbol of url params
        //        t_size pos;

        //        if ((pos = p_url.find_first ('#')) != pfc_infinite || (pos = p_url.find_first ('?')) != pfc_infinite) {
        //            t_size len = p_url.length (), pos2, pos3;
        //            pfc::string8 name, value;
        //            for (pos = pos; pos < len; pos = pos3) {
        //                if ((pos2 = p_url.find_first ('=', pos)) == pfc_infinite)
        //                    break;

        //                if ((pos3 = p_url.find_first ('&', pos2)) == pfc_infinite)
        //                    pos3 = len;

        //                name.reset (); name.prealloc (pos2 - pos);
        //                for (t_size i = pos + 1; i < pos2; i++)
        //                    name.add_char (p_url[i]);

        //                value.reset (); value.prealloc (pos3 - pos2);
        //                for (t_size i = pos2 + 1; i < pos3; i++)
        //                    value.add_char (p_url[i]);

        //                this->find_or_add (name) = value;
        //            }
        //        }
        //    }
        //};
    }
}
#endif