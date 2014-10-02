/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnGeoCoord/PublicAPI/DgnGeoCoordApi.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform\IGeoCoordReproject.h>
#include <DgnGeoCoord\DgnGeoCoord.h>

#if defined (__DGNGEOCOORD_BUILD__)
#   define DGNGEOCOORD_EXPORTED    __declspec(dllexport)
#else
#   define DGNGEOCOORD_EXPORTED    __declspec(dllimport)
#endif

BEGIN_EXTERN_C

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED    DgnGCSP    dgnGeoCoord_readCoordinateSystem
(
DgnModelRefP    modelRef,
bool            primary
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED    void    dgnGeoCoord_initialize
(
const char*     dataDirectory
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED    StatusInt   dgnGeoCoord_reprojectToGCS
(
DgnModelRefP    modelRef,
DgnGCSP         newGCS,
bool            reportProblems
);

// the functions below are not exported, called only within the DLL.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatform::IGeoCoordinateReprojectionSettingsP  dgnGeoCoord_getRefReprojectionSettings
(
DgnModelRefP modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       dgnGeoCoord_editRefReprojectionSettings
(
DgnModelRefListP modelRefList
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       dgnGeoCoord_saveDefaultRefReprojectionSettings
(
DgnModelRefP    modelRef
);

/*---------------------------------------------------------------------------------**//**
* NOTE: Link with GCSDialog.lib for this method.
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            dgnGeoCoord_openCoordinateDialog
(
Bentley::GeoCoordinates::DgnGCSPtr& selectedGCS,
DgnGCSP                             initialGCSP,
WCharCP                             favoritesFileList,
DgnModelRefP                        modelRef,
bool                                onlyGeoTiff
);

/*---------------------------------------------------------------------------------**//**
* NOTE: Link with GCSDialog.lib for this method.
* @bsimethod                                                   Marc.Bedard  10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            dgnGeoCoord_openCoordinateDetailsDialog
(
DgnGCSP         initialGCSP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            dgnGeoCoord_canSubstituteLinearTransformForReprojection
(
DgnAttachmentP  refP,
DgnModelP       refCache,
DgnGCSP         referenceGCS,
DgnModelRefP    parentModelRef,
DgnGCSP         parentGCS
);


END_EXTERN_C

