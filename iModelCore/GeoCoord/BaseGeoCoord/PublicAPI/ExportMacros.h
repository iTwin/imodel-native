/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/ExportMacros.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

