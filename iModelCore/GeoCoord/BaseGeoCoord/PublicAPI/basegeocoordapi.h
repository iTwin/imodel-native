/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/PublicAPI/basegeocoordapi.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley\Bentley.h>
#include <Bentley\WString.h>
#include <string>

#if defined (CREATE_STATIC_LIBRARIES)
    #undef BASEGEOCOORD_EXPORTED
    #define BASEGEOCOORD_EXPORTED
#else 
    #if defined (__BASEGEOCOORD_BUILD__)
        #define BASEGEOCOORD_EXPORTED    __declspec(dllexport)
    #else
        #define BASEGEOCOORD_EXPORTED    __declspec(dllimport)
    #endif
#endif

// Forward Declarations
struct IGeoTiffKeysList;

// Forward Declarations in Bentley::GeoCoordinates namespace
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
BASEGEOCOORD_EXPORTED Bentley::GeoCoordinates::BaseGCS* baseGeoCoord_allocate
(
);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   Bentley::GeoCoordinates::BaseGCS* baseGeoCoord_allocateFromBaseGCSKeyName
(
WCharCP keyName // The coordinate system key name.
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_deallocate
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromGeoTiffKeys
(
StatusInt*                        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
Bentley::WString*                 warningOrErrorMsg,  // Error message.
IGeoTiffKeysList*                 geoTiffKeys,        // The GeoTiff key list
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromWellKnownText
(
StatusInt*                         warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
Bentley::WString*                  warningOrErrorMsg,  // Error message.
Int32                              wktFlavor,          // The WKT Flavor.
WCharCP                            wellKnownText,      // The Well Known Text specifying the coordinate system.
Bentley::GeoCoordinates::BaseGCS*  pBaseGcs
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
Bentley::GeoCoordinates::BaseGCS* pSrcGcs,
Bentley::GeoCoordinates::BaseGCS* pDstGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double baseGeoCoord_getUnitsFromMeters
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED int baseGeoCoord_getEPSGUnitCode
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED Bentley::GeoCoordinates::BaseGCS* baseGeoCoord_clone
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getGeoTiffKeys
(
IGeoTiffKeysList* pList,
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getWellKnownText
(
WStringR                          wellKnownText,      // The WKT.
Int32                             wktFlavor,          // The WKT Flavor.
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isValid
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isEquivalent
(
Bentley::GeoCoordinates::BaseGCS* pBaseGcs1,
Bentley::GeoCoordinates::BaseGCS* pBaseGcs2
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
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre 07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_setQuadrant
(
short                             quadrant,
Bentley::GeoCoordinates::BaseGCS* pBaseGcs
);

END_EXTERN_C
