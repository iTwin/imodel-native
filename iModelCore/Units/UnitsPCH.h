/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#undef UNITS_EXPORT
#ifdef __UNITS_BUILD__
    #define UNITS_EXPORT EXPORT_ATTRIBUTE
#else
    #define UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Units/Units.h>

USING_NAMESPACE_BENTLEY

