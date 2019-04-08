/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeoCoordErrors.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#define GeoCoordError_Base                      0x150000

#define GeoCoordError_BadType66Version          (GeoCoordError_Base | 0x0002)
#define GeoCoordError_BadType66Size             (GeoCoordError_Base | 0x0003)

#define GeoCoordError_InvalidUTMZone            (GeoCoordError_Base | 0x0004)

/*__PUBLISH_SECTION_END__*/

