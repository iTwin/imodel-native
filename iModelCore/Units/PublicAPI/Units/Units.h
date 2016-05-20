/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/Units.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#undef UNITS_EXPORT
#ifdef __UNITS_BUILD__
    #define UNITS_EXPORT EXPORT_ATTRIBUTE
#else
    #define UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_UNITS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Units {
#define END_BENTLEY_UNITS_NAMESPACE   } }
#define USING_NAMESPACE_BENTLEY_UNITS using namespace BENTLEY_NAMESPACE_NAME::Units;

#define UNITS_TYPEDEFS(_name_)  \
    BEGIN_BENTLEY_UNITS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_UNITS_NAMESPACE

#include <math.h>
#include <algorithm>

using namespace std;

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Logging/bentleylogging.h>

//USING_NAMESPACE_BENTLEY

#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/UnitRegistry.h>

/*__PUBLISH_SECTION_END__*/
