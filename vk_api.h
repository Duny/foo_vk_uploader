#pragma once

typedef pfc::map_t<pfc::string8, pfc::string8> str2str_map;

class vk_api
{
    static const char *g_redirect_url;
    static const char *g_app_id;

    static pfc::string8 g_auth_url;

    static bool make_get_request (const pfc::string8_fast &url, pfc::string8_fast &answer);
public:
    // api calls error codes
    enum error_codes {
        unknown_error = 1,
        app_is_disabled,
        unknown_method,
        invalid_sig,
        auth_failed
    };

    static void init ();

    static const pfc::string8 &get_auth_url () { return g_auth_url; }
	static bool get_url_params (const pfc::string8 &url, str2str_map &params);
    static bool is_redirect_url (const pfc::string8 &url) { return url.find_first (g_redirect_url) == 0; }

    static bool call (const char *method, const char *params, Json::Value &out);
};