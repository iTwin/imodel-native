/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Raster/ExportMacros.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__RASTER_BUILD__)
#   define RASTER_EXPORT      EXPORT_ATTRIBUTE
#else
#   define RASTER_EXPORT      IMPORT_ATTRIBUTE
#endif

