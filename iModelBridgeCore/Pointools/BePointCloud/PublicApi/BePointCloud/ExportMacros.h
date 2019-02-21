/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/ExportMacros.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__BEPOINTCLOUD_BUILD__)
#   define BEPOINTCLOUD_EXPORT      EXPORT_ATTRIBUTE
#else
#   define BEPOINTCLOUD_EXPORT      IMPORT_ATTRIBUTE
#endif

