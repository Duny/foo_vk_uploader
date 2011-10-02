#ifndef _FOO_VK_UPLOADER_VK_API_HELPERS_H_
#define _FOO_VK_UPLOADER_VK_API_HELPERS_H_

namespace vk_uploader
{
    namespace vk_api
    {  
        class string_formatter_hasher_md5 : public pfc::string_formatter
        {
            mutable hasher_md5_state m_state;
            static_api_ptr_t<hasher_md5> m_hasher;
        public:
            string_formatter_hasher_md5 () { m_hasher->initialize(m_state); }

            void get_result (pfc::string8 &p_out) const 
            {
                const char *str = pfc::string_formatter::get_ptr ();
                m_hasher->process_string (m_state, str);
                p_out = pfc::format_hexdump (m_hasher->get_result (m_state).m_data, sizeof (hasher_md5_result), "");
            }
        };

        class signature
        {
            pfc::string8 m_sig;
        public:
            signature (params_cref p_params)
            {
                string_formatter_hasher_md5 hasher;

                hasher << string_constants::get_user_id ();
                for (t_size i = 0, n = p_params.get_size (); i < n; i++)
                    hasher << p_params[i].first << "=" << p_params[i].second;
                //hasher << string_constants::get_sid ();

                hasher.get_result (m_sig);
            }

            operator const char * () const { return m_sig.get_ptr (); }
        };

        class request_url_builder
        {
            pfc::string8 m_url;
        public:
            request_url_builder (const char *p_api_name, params_cref p_params) 
            {
                params_t params = p_params;

                params.add_item (std::make_pair ("api_id", string_constants::app_id));
                params.add_item (std::make_pair ("method", p_api_name));

                //params.sort ();

                params.add_item (std::make_pair ("sig", signature (p_params)));
                params.add_item (std::make_pair ("sid", string_constants::get_sid ()));

                m_url = string_constants::api_frontend_url;
                m_url.add_char ('?');
                for (t_size i = 0, n = params.get_size (); i < n; i++) {
                    m_url += params[i].first;
                    m_url.add_char ('=');
                    m_url += params[i].second;
                    if (i < n - 1) m_url.add_char ('&');
                }
                //m_url = "https://api.vkontakte.ru/method/wall.post
            }

            operator const char * () const { return m_url.get_ptr (); }
        };

        class url_params : public pfc::map_t<pfc::string8 const, pfc::string8>
        {
        public:
            url_params (const pfc::string8 &p_url)
            {
                // find start symbol of url params
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