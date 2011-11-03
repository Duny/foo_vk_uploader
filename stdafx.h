#pragma once

#include "foobar2000\ATLHelpers\ATLHelpers.h"
#include <exdispid.h>
#include <atlframe.h>

#include "resource.h"

// jsoncpp includes
#include "json/reader.h"
#include "json/value.h"

// boost includes
#include "boost/smart_ptr.hpp"

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_io.hpp"
#include "boost/tuple/tuple_comparison.hpp"

// plugin includes
#include "helpers.h"

#include "vk_auth.h"
#include "vk_api.h"
#include "upload_thread.h"
#include "upload_preset.h"

#define COMPONENT_NAME "foo_vk_uploader"