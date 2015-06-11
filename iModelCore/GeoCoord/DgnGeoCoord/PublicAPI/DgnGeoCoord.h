/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnGeoCoord/PublicAPI/DgnGeoCoord.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#if defined (__DGNGEOCOORD_BUILD__)
#   define DGNGEOCOORD_EXPORTED    __declspec(dllexport)
#else
#   define DGNGEOCOORD_EXPORTED    __declspec(dllimport)
#endif

#include    <GeoCoord\basegeocoord.h>
#include    <DgnPlatform/DgnPlatform.h>
#include    <DgnPlatform/DgnFileIO/UnitDefinition.h>
#include    <DgnPlatform/scancriteria.h>
#include    <Bentley/bvector.h>
#include    <DgnPlatform/DgnPlatformLib.h>

typedef struct GeoCoordType66 const*    GeoCoordType66CP;
typedef struct GeoCoordType66 *         GeoCoordType66P;
typedef union  ProjectionParams const*  ProjectionParamsCP;
typedef union  ProjectionParams *       ProjectionParamsP;

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

typedef RefCountedPtr<DgnGCS>   DgnGCSPtr;

enum GeoCoordinationState
    {
    NoGeoCoordination   = 0,
    Reprojected         = 1,
    AECTransform        = 2,
    };

/*=================================================================================**//**
The IGeoCoordinateEventHandler class defines an interface that applications that need
notification of GeoCoordinate events can implement.
* @ingroup geoCoordinate
* @bsiclass                                                     Barry.Bentley   06/07
+===============+===============+===============+===============+===============+======*/
struct          IGeoCoordinateEventHandler
{
/*---------------------------------------------------------------------------------**//**
* This method is called before the GeoCoordinate system of a model is changed.
* @param    oldGCS          IN      The existing GeoCoordinateSystem (NULL if there is none).
* @param    newGCS          IN      The new GeoCoordinateSystem.
* @param    modelRef        IN      The modelRef for which the GCS is changing.
* @param    primaryCoordSys IN      true if changing the primary coordinate system, false if changing the "reference" coordinate system.
* @param    writingToFile   IN      true if changes will be written to the file, false if change saved only as cached.
* @param    reprojectData   IN      true if the data in the model will be reprojected from the existing GCS to the new GCS.
* @return   SUCCESS or a nonzero error code to abort the change of the coordinate system.
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   BeforeCoordinateSystemChanged (DgnGCSP oldGCS, DgnGCSP  newGCS, DgnModelRefP modelRef, bool primaryCoordSys, bool writingToFile, bool reprojectData) = 0;

/*---------------------------------------------------------------------------------**//**
* This method is called after the GeoCoordinate system of a model has been changed.
* @param    oldGCS          IN      The previous GeoCoordinateSystem (NULL if there is none).
* @param    newGCS          IN      The new GeoCoordinateSystem.
* @param    modelRef        IN      The modelRef for which the GCS changed.
* @param    primaryCoordSys IN      true if changed the primary coordinate system, false if changed the "reference" coordinate system.
* @param    writtenToFile   IN      true if changes written to the file, false if change saved only as cached.
* @param    reprojectData   IN      true if the data in the model was reprojected from the existing GCS to the new GCS.
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        AfterCoordinateSystemChanged (DgnGCSP oldGCS, DgnGCSP newGCS, DgnModelRefP modelRef, bool primaryCoordSys, bool writtenToFile, bool reprojectData) = 0;

/*---------------------------------------------------------------------------------**//**
* This method is called before the GeoCoordinate system in a model is deleted.
* @param    currentGCS      IN      The GeoCoordinateSystem that is about to be deleted.
* @param    modelRef        IN      The modelRef for which the GCS is changing.
* @param    primaryCoordSys IN      true if deleting the primary coordinate system, false if deleting the "reference" coordinate system.
* @return   SUCCESS or a nonzero error code to abort deletion of the coordinate system.
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   BeforeCoordinateSystemDeleted (DgnGCSP currentGCS, DgnModelRefP modelRef, bool primaryCoordSys) = 0;

/*---------------------------------------------------------------------------------**//**
* This method is called after the GeoCoordinate system in a model has been deleted.
* @param    currentGCS      IN      The GeoCoordinateSystem that was deleted.
* @param    modelRef        IN      The modelRef for which the GCS was deleted.
* @param    primaryCoordSys IN      true if deleted the primary coordinate system, false if deleted the "reference" coordinate system.
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        AfterCoordinateSystemDeleted (DgnGCSP currentGCS, DgnModelRefP modelRef, bool primaryCoordSys) = 0;

/*---------------------------------------------------------------------------------**//**
* This method is called before a reference geocoordinated state is changed.
* For example if it is changed between "Projected", "AEC Transform", and "No".
* @param    modelRef        IN      The modelRef of the reference that's getting changed.
* @param    oldState        IN      The old state. 0 for no geocoordination, 1 for Projected, 2 for AEC transformed.
* @param    newState        IN      The new state.
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        BeforeReferenceGeoCoordinationChanged (DgnModelRefP modelRef, GeoCoordinationState oldState, GeoCoordinationState newState) = 0;

/*---------------------------------------------------------------------------------**//**
* This method is called after a reference has had changes to its geocoordinated state.
* For example if it is changed between "Projected", "AEC Transform", and "No".
* @param    modelRef        IN      The modelRef of the reference that's getting changed.
* @param    oldState        IN      The old state. 0 for no geocoordination, 1 for Projected, 2 for AEC transformed.
* @param    newState        IN      The new state.
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        AfterReferenceGeoCoordinationChanged (DgnModelRefP modelRef, GeoCoordinationState oldState, GeoCoordinationState newState) = 0;

};

typedef bvector <IGeoCoordinateEventHandler*>   T_GeoCoordEventHandlers;

/*=================================================================================**//**
The MicroStation Geographic Coordinate System class extend the base Geographic Coordinate System
class to provide functionality needed with the context of a design model. The DgnGCS  can be
written to and read from a model. Since the unit definition of the model is known, DgnGCS 
can calculate design file coordinates from geographic coordinates (longitude latitude) and
vice versa.
<p>
Internally, whenever a request is made for the DgnGCS  corresponding to a particular model,
the GCS is retrieved from the file and associated with that model, so subsequent requests
for that GCS are very efficient.
<p>
Since DgnGCS  is derived from GeoCoordinateSystem, it is reference counted, and the usual
reference counting semantics apply -  if a pointer to the instance is retained, call the AddRef() method.
When the reference is no longer needed, call the Release() method.
Application code should never call the "delete" operator on an instance of DgnGCS .
The DgnGCSPtr  type is a smart pointer that makes following those rules easy.<p>
<p>
DgnGCS  instances are always associated with a design model, which is how they obtain the
scaling and global origin needed to go from coordinate system units to design coordinates.
* @ingroup geoCoordinate
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
struct DgnGCS : public BaseGCS, DgnPlatform::DgnModelAppData
{
/*__PUBLISH_SECTION_END__*/
private:
double                          m_uorsPerBaseUnit;
DPoint3d                        m_globalOrigin;
double                          m_paperScaleFromType66;
bool                            m_datumOrEllipsoidFromUserLib;

static T_GeoCoordEventHandlers* s_eventHandlers;

    virtual void _OnCleanup (DgnModelR dgnCache) override;

public:
    static DgnPlatform::DgnModelAppData::Key const& GetKey (bool primary);

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS  ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
WCharCP                 coordinateSystemName
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
WCharCP                 coordinateSystemName,
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
BaseGCSCP               baseGCS,
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
CSParameters&           csParameters,
double                  paperScale,
Int32                   coordSysId,
DgnModelP               cache,
bool                    datumOrEllipsoidFromUserLibrary
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS
(
DgnGCSCR sourceGcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
~DgnGCS 
(
);

void                                InitCacheParameters (DgnModelP cache, double paperScale);
void                                SetDatumOrEllipsoidInUserLibrary ();

public:
DGNGEOCOORD_EXPORTED bool           GetDatumOrEllipsoidInUserLibrary ();

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/

public:

/*---------------------------------------------------------------------------------**//**
* Creates an instance of DgnGCS  for the given model, looking up the Coordinate
* System parameters from the Coordinate System Library by name.
* @param    coordinateSystemName    IN  The common name of the coordinate system..
* @param    modelRef                IN  The modelRef to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated modelRef - @see #ToModel
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSPtr      CreateGCS
(
WCharCP                 coordinateSystemName,
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* Creates a "blank" coordinate system. Does not look in the specified model for
*  a definition. Used in conjunction with InitLatLong method, etc.
* @param    modelRef                IN  The modelRef to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated modelRef - @see #ToModel
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSPtr      CreateGCS
(
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* Creates a MicroStation coordinate system from a BaseGCS.
* @param    baseGCS                 IN  The base geocoordinate system.
* @param    modelRef                IN  The modelRef to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated modelRef - @see #ToModel
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSPtr      CreateGCS
(
BaseGCSCP               baseGCS,
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* Creates a copy of the given DgnGCS
* @param    dgnGcs                 IN  The source DgnGCS to copy.
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED static DgnGCSPtr CreateGCS
(
DgnGCSCR dgnGcs
);

/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian coordinates in the units specified by the GCS from design coordinates (UORs).
* @param    outCartesian    OUT     The calculated cartesian coordinates in the units specified in the GCS.
* @param    inUors          IN      The design coordinates.
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  void                  CartesianFromUors
(
DPoint3dR               outCartesian,
DPoint3dCR              inUors
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian coordinates in the units specified by the GCS from design coordinates (UORs).
* @param    outCartesian    OUT     The calculated cartesian coordinates in the units specified in the GCS.
* @param    inUors          IN      The design coordinates.
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED void                   CartesianFromUors2D
(
DPoint2dR               outCartesian,
DPoint2dCR              inUors
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the design coordinates (UORs) from cartesian coordinates in the units specified by the GCS.
* @param    outUors        OUT       The calculated design coordinates.
* @param    inCartesian    INOUT     The cartesian coordinates in the units specified in the GCS.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  void                  UorsFromCartesian
(
DPoint3dR               outUors,
DPoint3dCR              inCartesian
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the design coordinates (UORs) from cartesian coordinates in the units specified by the GCS.
* @param    outUors        OUT       The calculated design coordinates.
* @param    inCartesian    INOUT     The cartesian coordinates in the units specified in the GCS.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED void                   UorsFromCartesian2D
(
DPoint2dR               outUors,
DPoint2dCR              inCartesian
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the design coordinates (UORS) of the input Longitude/Latitude/Elevation point.
* @param    outUors         OUT     The calculated design coordinates.
* @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       UorsFromLatLong
(
DPoint3dR               outUors,
GeoPointCR              inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude, latitude, and elevation from design coordinates (UORS).
* @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of this GCS.
* @param    inUors          IN      The input design coordinates.
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       LatLongFromUors
(
GeoPointR               outLatLong,
DPoint3dCR              inUors
) const;

/*---------------------------------------------------------------------------------**//**
* Reprojects an array of points in the design coordinates (UORs) of this model to the
* design coordinates (UORs) of the design file associated with destMstnGCS.
* @param    outUorsDest     OUT     An array dimensioned to numPoints to hold the calculated UORs.
* @param    outLatLongDest  OUT     An optional array that will be filled with the geographic
*                                   coordinates in the datum of the destMstnGCS. If not NULL,
*                                   the array must be dimensioned to numPoints.
* @param    outLatLongSrc   OUT     An optional array that will be filled with the geographic
*                                   coordinates in the datum of this GCS. If not NULL,
*                                   the array must be dimensioned to numPoints.
* @param    inUors          IN      An array holding the input points in design file coordinaates.
* @param    numPoints       IN      The number of points in inUors.
* @param    destMstnGCS     OUT     The destination DgnGCS .
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED    ReprojectStatus     ReprojectUors
(
DPoint3dP               outUorsDest,
GeoPointP               outLatLongDest,
GeoPointP               outLatLongSrc,
DPoint3dCP              inUors,
int                     numPoints,
DgnGCSCR                destMstnGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the 2d design coordinates (UORS) of the input Longitude/Latitude point.
* @param    outUors         OUT     The calculated design coordinates.
* @param    inLatLong       IN      The longitude,latitude in the datum of this GCS.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       UorsFromLatLong2D
(
DPoint2dR               outUors,
GeoPoint2dCR            inLatLong
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the longitude, latitude from 2d design coordinates (UORS).
* @param    outLatLong      OUT     The calculated longitude,latitude in the datum of this GCS.
* @param    inUors          IN      The input design coordinates.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       LatLongFromUors2D
(
GeoPoint2dR             outLatLong,
DPoint2dCR              inUors
) const;

/*---------------------------------------------------------------------------------**//**
* Reprojects an array of 2d points in the design coordinates (UORs) of this model to the 2d
* design coordinates (UORs) of the design file associated with destMstnGCS.
* @param    outUorsDest     OUT     An array dimensioned to numPoints to hold the calculated UORs.
* @param    outLatLongDest  OUT     An optional array that will be filled with the geographic
*                                   coordinates in the datum of the destMstnGCS. If not NULL,
*                                   the array must be dimensioned to numPoints.
* @param    outLatLongSrc   OUT     An optional array that will be filled with the geographic
*                                   coordinates in the datum of this GCS. If not NULL,
*                                   the array must be dimensioned to numPoints.
* @param    inUors          IN      An array holding the input points in design file coordinaates.
* @param    numPoints       IN      The number of points in inUors.
* @param    destMstnGCS     OUT     The destination DgnGCS .
* @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       ReprojectUors2D
(
DPoint2dP               outUorsDest,
GeoPoint2dP             outLatLongDest,
GeoPoint2dP             outLatLongSrc,
DPoint2dCP              inUors,
int                     numPoints,
DgnGCSCR                destMstnGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Calculates the best approximate transform that can be applied at the elementOrigin to
* transform coordinates from this GCS's design coordinates to those of the destination GCS.
* @param    outTransform    OUT     The calculated Transform.
* @param    elementOrigin   IN      The point, in design coordinates (UORs) of this GCS, at which the transform will be applied.
* @param    extent          IN      The extent, in design coordinates (UORs) of a bvector that tells the span of the data to which
*                                   the transform will be applied. If NULL, a reasonable guess is used.
* @param    doRotate        IN      true to allow rotation in the transform.
* @param    doScale         IN      true to allow scaling in the transform.
* @param    destMstnGCS     OUT     The destination DgnGCS .
* @return   SUCCESS or a CS_MAP error code if elementOrigin could not be reprojected.
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  ReprojectStatus       GetLocalTransform
(
TransformP              outTransform,
DPoint3dCR              elementOrigin,
DPoint3dCP              extent,
bool                    doRotate,
bool                    doScale,
DgnGCSCR                destMstnGCS
) const;

/*---------------------------------------------------------------------------------**//**
* Gets the localized name of the Geographic Projection used in the Coordinate System.
* @param    outputBuffer    OUT     Buffer to hold the projection name.
* @param    bufferSize      IN      dimension of outputBuffer.
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  WCharCP              GetProjectionName
(
WStringR                outputBuffer
) const;

/*---------------------------------------------------------------------------------**//**
* Gets a name suitable for display in user interface.
* @param    outputBuffer    OUT     Buffer to hold the projection name.
* @param    bufferSize      IN      dimension of outputBuffer.
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  WCharCP             GetDisplayName
(
WStringR                outputBuffer
) const;

/*---------------------------------------------------------------------------------**//**
* Writes the GCS parameters to the model.
* @param    modelRef        IN      The model to write the GCS to.
* @param    primaryCoordSys IN      true to write as the primary coordinate system, false to write as the "reference" coordinate system.
* @param    writeToFile     IN      true to save to the file, false to save only as cached.
* @param    reprojectData   IN      true if the data in the model is to be reprojected from the existing GCS to the new GCS.
* @param    reportProblems  IN      true if reprojection problems should be reported to the user.
* @remarks  If writeToFile is false, reprojectData is ignored and the data in the model is not reprojected.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  StatusInt             ToModel
(
DgnModelRefP            modelRef,
bool                    primaryCoordSys,
bool                    writeToFile,
bool                    reprojectData,
bool                    reportProblems
);

/*---------------------------------------------------------------------------------**//**
* Factory method that constructs an DgnGCS  instance by attempting to locate the element that saves the
* geographic coordinate system parameters in the model, and creating the instance based on those parameters.
* If the DgnGCS  for a particular model has previously been requested, it is cached and this call is very efficient.
* @param    modelRef        IN      The model to look in.
* @param    primaryCoordSys IN      true to find the primary coordinate system, false to find the "reference" coordinate system.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSP        FromModel
(
DgnModelRefP            modelRef,
bool                    primaryCoordSys
);

/*---------------------------------------------------------------------------------**//**
* Permanently deletes the saved Geographic Coordinate System parameters saved in the model.
* @param    modelRef        IN      The model to delete from.
* @param    primaryCoordSys IN      true to delete the primary coordinate system, false to delete the "reference" coordinate system.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static StatusInt      DeleteFromModel
(
DgnModelRefP            modelRef,
bool                    primaryCoordSys
);

/*---------------------------------------------------------------------------------**//**
* Factory method that constructs an DgnGCS  instance by attempting to locate the element that saves the
* geographic coordinate system parameters in the specifed DgnModel, and creating the instance based on those parameters.
* If the DgnGCS  for a particular DgnModel has previously been requested, it is cached and this call is very efficient.
* @param    cache           IN      The DgnModel
* @param    primaryCoordSys IN      true to find the primary coordinate system, false to find the "reference" coordinate system.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSP        FromCache
(
DgnModelP               cache,
bool                    primaryCoordSys
);

/*---------------------------------------------------------------------------------**//**
* Method that reloads geographically transformed and geographically projected references, responding to GCS changes in modelRef.
* @param    modelRef        IN      The modelRef for which the GCS has changed.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           ReloadGeoReferences
(
DgnModelRefP            modelRef
);

/*---------------------------------------------------------------------------------**//**
* Sets a coordinate system event handler.
* @param    handler         IN      The event handler.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           SetEventHandler
(
IGeoCoordinateEventHandler*  handler
);

/*---------------------------------------------------------------------------------**//**
* Removes a coordinate system event handler.
* @param    handler         IN      The event handler.
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           RemoveEventHandler
(
IGeoCoordinateEventHandler*  handler
);

/*---------------------------------------------------------------------------------**//**
* Gets the Paper Scale for this GCS. The Paper scale affects the Cartesian coordinates and
*  makes measurements unreliable. Its use is not recommended.
* @bsimethod                                                    Barry.Bentley   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED double                 GetPaperScale () const;

/*---------------------------------------------------------------------------------**//**
* Sets the Paper Scale for this GCS. The Paper Scale affects the Cartesian coordinates and
*  makes measurements unreliable. Its use is not recommended. The default and recommended
*  value is 1.0.
* @param    paperScale      IN      The new Paper Scale value.
* @param    modelRef        IN      The model that this GCS came from.
* @bsimethod                                                    Barry.Bentley   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED StatusInt              SetPaperScale 
(
double paperScale, 
DgnModelRefP modelRef
);

/*__PUBLISH_SECTION_END__*/
private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static ElementRefP    FindType66
(
DgnModelP               cache,
bool                    primary
);

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  StatusInt             CreateGeoCoordType66
(
DgnPlatform::ApplicationElm*         type66,
DgnModelP               cache,
bool                    primary
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  StatusInt             GetUnitDefinition
(
UnitDefinitionR             unitDef,
DgnPlatform::StandardUnit&  standardUnitNumber
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED bool                   UnitsIdentical
(
DgnGCSCR                 other
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static StatusInt      GetCSUnitName
(
WStringR                csUnitName,
UnitDefinitionCR        unitDef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static DgnGCSP        FromGeoCoordType66
(
const DgnPlatform::ApplicationElm*   type66,
DgnModelP               cache
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           GetReprojectionTarget
(
DgnModelRefP           &targetModelRefP,
DgnGCSP                &targetGCSP,
DgnModelRefP            refModelRef,
DgnModelRefP            fallbackParentModelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           SendBeforeGeoCoordinationChanged
(
DgnModelRefP            modelRef,
GeoCoordinationState    oldState,
GeoCoordinationState    newState
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           SendAfterGeoCoordinationChanged
(
DgnModelRefP            modelRef,
GeoCoordinationState    oldState,
GeoCoordinationState    newState
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED  static void           SendAfterGeoCoordinateSystemDeleted
(
DgnGCSP                 currentGCSP,
DgnModelRefP            modelRef,
bool                    primary
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED static bool            UpdateUnitInfo
(
DgnModelP               model
);

/*__PUBLISH_SECTION_START__*/
};

/*__PUBLISH_SECTION_END__*/
/*=================================================================================**//**
The DgnGeoCoordinationAdmin class implements DgnPlatform::GeoCoordinateAdmin.
It can be used from any DgnPlatform Host program that wants to provide GeoCoordination services.
* @bsiclass                                                     Barry.Bentley   06/07
+===============+===============+===============+===============+===============+======*/
struct  DgnGeoCoordinationAdmin : public DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin
{
private:
    mutable IGeoCoordinateServicesP     m_gcrp;
    mutable bool                        m_initializationComplete;
    mutable bool                        m_otfEnabled;
    mutable WString                     m_dataDirectory;

    void CompleteInitialization() const;
    void AddAuxCoordSystemProcessor(IACSManagerR mgr) const;

    DgnGeoCoordinationAdmin (WCharCP dataDirectory, IACSManagerR mgr);

    virtual IGeoCoordinateServicesP _GetServices () const override;
    virtual bool _CanShareDgnFile (DgnFileR, DgnAttachmentR) const override;
    virtual void _OnPostModelFill (DgnModelR, DgnModelFillContextP) const override;

public:
    DGNGEOCOORD_EXPORTED static DgnGeoCoordinationAdmin* Create (WCharCP dataDirectory, IACSManagerR mgr);
};

/*__PUBLISH_SECTION_START__*/

} // ends GeoCoordinate namespace

END_BENTLEY_NAMESPACE
/*__PUBLISH_SECTION_END__*/

#pragma make_public (Bentley::GeoCoordinates::DgnGCS)
