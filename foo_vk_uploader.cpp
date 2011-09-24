#include "stdafx.h"

static CComModule g_com_module;
class vk_uploader_initquit : public initquit
{
    void on_init () {
        g_com_module.Init (NULL, NULL, &LIBID_ATLLib);
    }

    void on_quit () {
        g_com_module.Term ();
    }
};
static initquit_factory_t<vk_uploader_initquit> g_initquit;


DECLARE_COMPONENT_VERSION ("vk.com Uploader", "0.1",
"Allows upload music to the vk.com social network from foobar2000.\n\n\
(c) 2011 Efimenko Dmitriy (majorquake3@gmail.com)");

VALIDATE_COMPONENT_FILENAME ("foo_vk_uploader.dll");