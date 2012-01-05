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
#include "boost/assign.hpp"
#include "boost/foreach.hpp"
#include "boost/function.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_io.hpp"
#include "boost/tuple/tuple_comparison.hpp"

using namespace boost::assign;
using namespace boost::foreach;
using boost::function;
using namespace boost::tuples;
using std::make_pair;

// plugin specific includes
#include "resource.h"
#include "helpers.h"

#endif