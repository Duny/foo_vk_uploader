#ifndef _FOO_VK_UPLOADER_STDAFX_H_
#define _FOO_VK_UPLOADER_STDAFX_H_


#define COMPONENT_NAME "foo_vk_uploader"


// crt includes
#include <time.h>

// foobar2000 includes
#include "foobar2000/ATLHelpers/ATLHelpers.h"
#include <exdispid.h>
#include <atlframe.h>

// jsoncpp includes
#include "json/reader.h"
#include "json/value.h"

// boost includes
#include "boost/function.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_io.hpp"
#include "boost/tuple/tuple_comparison.hpp"

// plugin includes
#include "resource.h"
#include "helpers.h"
#include "vk_auth.h"
#include "vk_api.h"
#include "upload_thread.h"
#include "upload_preset.h"
#include "browser.h"

#endif