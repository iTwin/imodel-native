
#pragma once

#undef UNITS_EXPORT
#ifdef __UNITS_BUILD__
    #define UNITS_EXPORT EXPORT_ATTRIBUTE
#else
    #define UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_UNITS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Units {
#define END_BENTLEY_UNITS_NAMESPACE   } }
#define USING_NAMESPACE_BENTLEY_UNITS using namespace BENTLEY_NAMESPACE_NAME::Units;


#include <algorithm>

using namespace std;

#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>

USING_NAMESPACE_BENTLEY

#include <Units/UnitTypes.h>
#include <Units/UnitRegistry.h>
