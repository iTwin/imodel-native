/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformBaseType.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#ifdef __cplusplus
#include <Geom/GeomApi.h>
#include "ExportMacros.h"
#else
#include <Geom/GeomApi.r.h>
#endif

//--------------------------------------------------------------------
// This file is included by both .cpp/h and .r files
//--------------------------------------------------------------------

#if defined (mdl_resource_compiler) || defined (mdl_type_resource_generator)
    #define BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
    #define END_BENTLEY_DGNPLATFORM_NAMESPACE
#else

    #define BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
    #define END_BENTLEY_DGNPLATFORM_NAMESPACE   } END_BENTLEY_NAMESPACE

#endif // (mdl_resource_compiler) || defined (mdl_type_resource_generator)

/** @endcond */
