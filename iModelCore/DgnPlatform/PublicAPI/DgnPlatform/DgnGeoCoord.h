/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnGeoCoord.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include    <GeoCoord/BaseGeoCoord.h>
#include    <DgnPlatform/DgnPlatform.h>
#include    <DgnPlatform/DgnDb.h>
#include    <DgnPlatform/UnitDefinition.h>
#include    <DgnPlatform/ScanCriteria.h>
#include    <Bentley/bvector.h>
#include    <DgnPlatform/DgnPlatformLib.h>

typedef struct GeoCoordType66 const*    GeoCoordType66CP;
typedef struct GeoCoordType66 *         GeoCoordType66P;
typedef union  ProjectionParams const*  ProjectionParamsCP;
typedef union  ProjectionParams *       ProjectionParamsP;

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<DgnGCS>   DgnGCSPtr;

enum GeoCoordinationState
    {
    NoGeoCoordination   = 0,
    Reprojected         = 1,
    AECTransform        = 2,
    };

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
struct DgnGCS : GeoCoordinates::BaseGCS
{
private:
double                          m_uorsPerBaseUnit;
DPoint3d                        m_globalOrigin;
double                          m_paperScaleFromType66;
bool                            m_datumOrEllipsoidFromUserLib;

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
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
GeoCoordinates::BaseGCSCP baseGCS,
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS 
(
CSParameters& csParameters,
double                  paperScale,
int32_t                 coordSysId,
DgnDbR               cache,
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

void                                InitCacheParameters (DgnDbR cache, double paperScale);
void                                SetDatumOrEllipsoidInUserLibrary ();

public:
DGNPLATFORM_EXPORT bool           GetDatumOrEllipsoidInUserLibrary ();

public:

/*---------------------------------------------------------------------------------**//**
* Creates an instance of DgnGCS  for the given model, looking up the Coordinate
* System parameters from the Coordinate System Library by name.
* @param    coordinateSystemName    IN  The common name of the coordinate system..
* @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated dgnProject 
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static DgnGCSPtr      CreateGCS
(
WCharCP                 coordinateSystemName,
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* Creates a "blank" coordinate system. Does not look in the specified model for
*  a definition. Used in conjunction with InitLatLong method, etc.
* @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated dgnProject
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static DgnGCSPtr      CreateGCS
(
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* Creates a MicroStation coordinate system from a BaseGCS.
* @param    baseGCS                 IN  The base geocoordinate system.
* @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
* @remarks  The DgnGCS  instance is not stored in the designated dgnProject
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static DgnGCSPtr      CreateGCS
(
GeoCoordinates::BaseGCSCP baseGCS,
DgnDbR            dgnProject
);

/*---------------------------------------------------------------------------------**//**
* Creates a copy of the given DgnGCS
* @param    dgnGcs                 IN  The source DgnGCS to copy.
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static DgnGCSPtr CreateGCS
(
DgnGCSCR dgnGcs
);

/*---------------------------------------------------------------------------------**//**
* Calculates the cartesian coordinates in the units specified by the GCS from design coordinates (UORs).
* @param    outCartesian    OUT     The calculated cartesian coordinates in the units specified in the GCS.
* @param    inUors          IN      The design coordinates.
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  void                  CartesianFromUors
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
DGNPLATFORM_EXPORT void                   CartesianFromUors2D
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
DGNPLATFORM_EXPORT  void                  UorsFromCartesian
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
DGNPLATFORM_EXPORT void                   UorsFromCartesian2D
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
DGNPLATFORM_EXPORT  ReprojectStatus       UorsFromLatLong
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
DGNPLATFORM_EXPORT  ReprojectStatus       LatLongFromUors
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
DGNPLATFORM_EXPORT    ReprojectStatus     ReprojectUors
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
DGNPLATFORM_EXPORT  ReprojectStatus       UorsFromLatLong2D
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
DGNPLATFORM_EXPORT  ReprojectStatus       LatLongFromUors2D
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
DGNPLATFORM_EXPORT  ReprojectStatus       ReprojectUors2D
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
DGNPLATFORM_EXPORT  ReprojectStatus       GetLocalTransform
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
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  WCharCP              GetProjectionName
(
WStringR                outputBuffer
) const;

/*---------------------------------------------------------------------------------**//**
* Gets a name suitable for display in user interface.
* @param    outputBuffer    OUT     Buffer to hold the projection name.
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  WCharCP             GetDisplayName
(
WStringR                outputBuffer
) const;

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Writes the GCS parameters to the project.
* @param    dgnProject        IN      The model to write the GCS to.
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  StatusInt Store (DgnDbR dgnProject);
/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* Factory method that constructs an DgnGCS  instance by attempting to locate the DgnGCS property that saves the
* geographic coordinate system parameters in the project, and creating the instance based on those parameters.
* If the DgnGCS for the project has previously been requested, it is cached and this call is very efficient.
* @param    dgnProject        IN      The project to look in.
* @return the DgnGCS for this project or NULL if the project is not geo-located
* @remarks Do not call delete on the returned point
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static DgnGCSP        FromProject
(
DgnDbR            dgnProject
);

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Permanently deletes the saved Geographic Coordinate System parameters saved in the model.
* @param    dgnProject        IN      The model to delete from.
* @param    primaryCoordSys IN      true to delete the primary coordinate system, false to delete the "reference" coordinate system.
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
//DGNPLATFORM_EXPORT  static StatusInt      Delete
//(
//DgnDbR            dgnProject
//);
/*__PUBLISH_SECTION_START__*/

/*__PUBLISH_SECTION_END__*/
private:

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  StatusInt             CreateGeoCoordType66
(
short*                  type66AppData,
uint32_t&                 type66AppDataBytes, // <= set to sizeof of type66AppData in bytes
DgnDbR             project,
bool                    primary
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  StatusInt             GetUnitDefinition
(
UnitDefinitionR             unitDef,
StandardUnit&  standardUnitNumber
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool                   UnitsIdentical
(
DgnGCSCR                 other
) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static StatusInt      GetCSUnitName
(
WStringR                csUnitName,
UnitDefinitionCR        unitDef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  static DgnGCSP        FromGeoCoordType66AppData
(
short const*   type66AppData,
DgnDbR               cache
);

/*__PUBLISH_SECTION_START__*/
};

/*__PUBLISH_SECTION_END__*/

/*=================================================================================**//**
The DgnGeoCoordinationAdmin class implements GeoCoordinateAdmin.
It can be used from any DgnPlatform Host program that wants to provide GeoCoordination services.
* @bsiclass                                                     Barry.Bentley   06/07
+===============+===============+===============+===============+===============+======*/
struct  DgnGeoCoordinationAdmin : public DgnPlatformLib::Host::GeoCoordinationAdmin
{
private:
    mutable IGeoCoordinateServicesP     m_gcrp;
    mutable bool                        m_initializationComplete;
    mutable bool                        m_otfEnabled;
    mutable BeFileName                  m_dataDirectory;

    void CompleteInitialization() const;
    //void AddAuxCoordSystemProcessor(IACSManagerR mgr) const;

    DgnGeoCoordinationAdmin (BeFileNameCR dataDirectory/*, IACSManagerR mgr*/);

    virtual IGeoCoordinateServicesP _GetServices () const override;

    virtual WString _GetDataDirectory () override {return m_dataDirectory.c_str();}

public:
    DGNPLATFORM_EXPORT static DgnGeoCoordinationAdmin* Create (BeFileNameCR dataDirectory/*, IACSManagerR mgr*/);
};

/*__PUBLISH_SECTION_START__*/

END_BENTLEY_DGN_NAMESPACE
