/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformBaseType.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if !defined (mdl_resource_compiler) && !defined (mdl_type_resource_generator)
  #include <Geom/GeomApi.h>
  #include "ExportMacros.h"

  #define BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
  #define END_BENTLEY_DGNPLATFORM_NAMESPACE   } END_BENTLEY_NAMESPACE
#else
  #include <Bentley/Bentley.h>

  #define BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
  #define END_BENTLEY_DGNPLATFORM_NAMESPACE 

#endif
