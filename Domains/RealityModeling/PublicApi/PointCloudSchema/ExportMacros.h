/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/ExportMacros.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__POINTCLOUDSCHEMA_BUILD__)
#   define POINTCLOUDSCHEMA_EXPORT      EXPORT_ATTRIBUTE
#else
#   define POINTCLOUDSCHEMA_EXPORT      IMPORT_ATTRIBUTE
#endif

