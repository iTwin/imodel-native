/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ExportMacros.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__SCALABLEMESH_SCHEMA_BUILD__)
#   define SCALABLEMESH_SCHEMA_EXPORT      EXPORT_ATTRIBUTE
#else
#   define SCALABLEMESH_SCHEMA_EXPORT      IMPORT_ATTRIBUTE
#endif

