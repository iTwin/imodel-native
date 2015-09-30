/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/basegeocoordapi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <string>
#include "ExportMacros.h"

// Forward Declarations
struct IGeoTiffKeysList;

// Forward Declarations
BEGIN_BENTLEY_NAMESPACE
namespace GeoCoordinates {class BaseGCS;}
END_BENTLEY_NAMESPACE

BEGIN_EXTERN_C

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_initialize
(
WCharCP dataDirectory
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED BentleyApi::GeoCoordinates::BaseGCS* baseGeoCoord_allocate
(
);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   BentleyApi::GeoCoordinates::BaseGCS* baseGeoCoord_allocateFromBaseGCSKeyName
(
WCharCP keyName // The coordinate system key name.
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_deallocate
(
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromGeoTiffKeys
(
StatusInt*                        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WString*                 warningOrErrorMsg,  // Error message.
IGeoTiffKeysList*                 geoTiffKeys,        // The GeoTiff key list
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromWellKnownText
(
StatusInt*                         warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WString*                  warningOrErrorMsg,  // Error message.
int32_t                            wktFlavor,          // The WKT Flavor.
WCharCP                            wellKnownText,      // The Well Known Text specifying the coordinate system.
GeoCoordinates::BaseGCS*  pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_reproject
(
double* outCartesianX,
double* outCartesianY,
double  inCartesianX,
double  inCartesianY,
GeoCoordinates::BaseGCS* pSrcGcs,
GeoCoordinates::BaseGCS* pDstGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double baseGeoCoord_getUnitsFromMeters
(
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int baseGeoCoord_getEPSGUnitCode
(
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoCoordinates::BaseGCS* baseGeoCoord_clone
(
GeoCoordinates::BaseGCS* pBaseGcs
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getGeoTiffKeys
(
IGeoTiffKeysList* pList,
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getWellKnownText
(
WStringR                          wellKnownText,      // The WKT.
int32_t                           wktFlavor,          // The WKT Flavor.
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isValid
(
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isEquivalent
(
GeoCoordinates::BaseGCS* pBaseGcs1,
GeoCoordinates::BaseGCS* pBaseGcs2
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_getErrorMessage
(
WStringR    errorStr,
int         errorCode
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getCartesianFromLatLong
(
double*                           pCartesianPt,
double*                           pGeoPt,
GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre 07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_setQuadrant
(
short                             quadrant,
GeoCoordinates::BaseGCS* pBaseGcs
);

END_EXTERN_C
