/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/ExportMacros.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__RASTERSCHEMA_BUILD__)
#   define RASTERSCHEMA_EXPORT      EXPORT_ATTRIBUTE
#else
#   define RASTERSCHEMA_EXPORT      IMPORT_ATTRIBUTE
#endif

