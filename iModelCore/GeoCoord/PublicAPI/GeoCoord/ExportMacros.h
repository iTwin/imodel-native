/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/GeoCoord/ExportMacros.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__BASEGEOCOORD_BUILD__)
#   define BASEGEOCOORD_EXPORTED      EXPORT_ATTRIBUTE
#   define BASEMANAGEDGCS_EXPORTED    EXPORT_ATTRIBUTE
#else
#   define BASEGEOCOORD_EXPORTED      IMPORT_ATTRIBUTE
#   define BASEMANAGEDGCS_EXPORTED    IMPORT_ATTRIBUTE
#endif

