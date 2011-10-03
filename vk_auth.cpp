#include "stdafx.h"

#include "vk_api.h"
#include "vk_auth.h"

namespace vk_uploader
{
    namespace vk_api
    {
        class NOVTABLE authorization_imp : public authorization
        {
        };

        static service_factory_single_t<authorization_imp> g_auth_factory;
    }
}