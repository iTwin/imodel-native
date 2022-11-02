/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <GeoCoord/BaseGeoCoord.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/UnitDefinition.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/RangeIndex.h>
#include <Bentley/bvector.h>
#include <DgnPlatform/PlatformLib.h>
#include <memory>

typedef struct GeoCoordType66 const*    GeoCoordType66CP;
typedef struct GeoCoordType66 *         GeoCoordType66P;
typedef union  ProjectionParams const*  ProjectionParamsCP;
typedef union  ProjectionParams *       ProjectionParamsP;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<DgnGCS>   DgnGCSPtr;

enum GeoCoordinationState
    {
    NoGeoCoordination   = 0,
    Reprojected         = 1,
    AECTransform        = 2,
    };

enum GeoCoordInterpretation
    {
    Cartesian = 0,
    XYZ = 1
    };


/*=================================================================================**//**
The DgnGCS class extends the base Geographic Coordinate System
class to provide functionality needed with the context of a DgnDb. The DgnGCS can be
written to and read from a model. Since the unit definition of the model is known, DgnGCS
can calculate DgnDb coordinates from geographic coordinates (longitude latitude) and
vice versa.
<p>
Internally, whenever a request is made for the DgnGCS  corresponding to a particular model,
the GCS is retrieved from the file and associated with that model, so subsequent requests
for that GCS are efficient.
<p>
Since DgnGCS is derived from GeoCoordinateSystem, it is reference counted, and the usual
reference counting semantics apply.
<p>
DgnGCS instances are always associated with a design model, which is how they obtain the
scaling and global origin needed to go from coordinate system units to design coordinates.
* @ingroup geoCoordinate
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DgnGCS : GeoCoordinates::BaseGCS
{
private:
    friend struct DgnGeoLocation;

    double   m_uorsPerBaseUnit;
    DPoint3d m_globalOrigin;
    double   m_paperScaleFromType66;

    DgnGCS();
    DgnGCS(Utf8CP coordinateSystemName);
    DgnGCS(Utf8CP coordinateSystemName, DgnDbR dgnProject);
    DgnGCS(DgnDbR dgnProject);
    DgnGCS(GeoCoordinates::BaseGCSCP baseGCS, DgnDbR dgnProject);
    DgnGCS(CSParameters& csParameters, double paperScale, int32_t coordSysId, DgnDbR cache, bool datumOrEllipsoidFromUserLibrary, CSGeodeticTransformDef const* transform = nullptr);
    DgnGCS(DgnGCSCR sourceGcs);
    ~DgnGCS();

    void InitCacheParameters(DgnDbR cache, double paperScale);

    void SetGlobalOrigin(DPoint3dCR go) {m_globalOrigin = go;}

public:
    DGNPLATFORM_EXPORT bool GetUserDatumBasedOnGridFile();

    // This method is only ever called while converting from DgnV8 to BIM files.
    DGNPLATFORM_EXPORT void SetGlobalOriginAndUnitScaling (DPoint3d& globalOrigin, double uorsPerBaseUnit);

    /*---------------------------------------------------------------------------------**//**
    * Creates an instance of DgnGCS  for the given model, looking up the Coordinate
    * System parameters from the Coordinate System Library by name.
    * @param    coordinateSystemName    IN  The common name of the coordinate system..
    * @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
    * @remarks  The DgnGCS  instance is not stored in the designated dgnProject
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSPtr CreateGCS(Utf8CP coordinateSystemName, DgnDbR dgnProject);

    /*---------------------------------------------------------------------------------**//**
    * Creates a "blank" coordinate system. Does not look in the specified model for
    *  a definition. Used in conjunction with InitLatLong method, etc.
    * @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
    * @remarks  The DgnGCS  instance is not stored in the designated dgnProject
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSPtr CreateGCS(DgnDbR dgnProject);

    /*---------------------------------------------------------------------------------**//**
    * Creates a coordinate system from a BaseGCS.
    * @param    baseGCS                 IN  The base geocoordinate system.
    * @param    dgnProject                IN  The dgnProject to use for the design file unit defintion.
    * @remarks  The DgnGCS  instance is not stored in the designated dgnProject
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSPtr CreateGCS(GeoCoordinates::BaseGCSCP baseGCS, DgnDbR dgnProject);

    /*---------------------------------------------------------------------------------**//**
    * Creates a copy of the given DgnGCS
    * @param    dgnGcs                 IN  The source DgnGCS to copy.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSPtr CreateGCS(DgnGCSCR dgnGcs);

    /*---------------------------------------------------------------------------------**//**
    * Calculates the cartesian coordinates in the units specified by the GCS from design coordinates (UORs).
    * @param    outCartesian    OUT     The calculated cartesian coordinates in the units specified in the GCS.
    * @param    inUors          IN      The design coordinates.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT void CartesianFromUors(DPoint3dR outCartesian, DPoint3dCR inUors) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the cartesian coordinates in the units specified by the GCS from design coordinates (UORs).
    * @param    outCartesian    OUT     The calculated cartesian coordinates in the units specified in the GCS.
    * @param    inUors          IN      The design coordinates.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT void CartesianFromUors2D(DPoint2dR outCartesian, DPoint2dCR inUors) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the design coordinates (UORs) from cartesian coordinates in the units specified by the GCS.
    * @param    outUors        OUT       The calculated design coordinates.
    * @param    inCartesian    INOUT     The cartesian coordinates in the units specified in the GCS.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT void UorsFromCartesian(DPoint3dR outUors, DPoint3dCR inCartesian) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the design coordinates (UORs) from cartesian coordinates in the units specified by the GCS.
    * @param    outUors        OUT       The calculated design coordinates.
    * @param    inCartesian    INOUT     The cartesian coordinates in the units specified in the GCS.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT void UorsFromCartesian2D(DPoint2dR outUors, DPoint2dCR inCartesian) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the design coordinates (UORS) of the input Longitude/Latitude/Elevation point.
    * @param    outUors         OUT     The calculated design coordinates.
    * @param    inLatLong       IN      The longitude,latitude,elevation in the datum of this GCS.
    * @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus UorsFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the longitude, latitude, and elevation from design coordinates (UORS).
    * @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of this GCS.
    * @param    inUors          IN      The input design coordinates.
    * @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus LatLongFromUors(GeoPointR outLatLong, DPoint3dCR inUors) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the longitude, latitude, and elevation from design coordinates (UORS) interpreted as XYZ coordinates.
    * @param    outLatLong      OUT     The calculated longitude,latitude,elevation in the datum of this GCS.
    * @param    inUors          IN      The input design coordinates.
    * @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus LatLongFromUorsXYZ(GeoPointR outLatLong, DPoint3dCR inUors) const;

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
    * @param    inUors          IN      An array holding the input points in design file coordinates.
    * @param    numPoints       IN      The number of points in inUors.
    * @param    destMstnGCS     OUT     The destination DgnGCS .
    * @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus ReprojectUors(DPoint3dP outUorsDest, GeoPointP outLatLongDest, GeoPointP outLatLongSrc, DPoint3dCP inUors, int numPoints, DgnGCSCR destMstnGCS) const;

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
    * @param    inUors          IN      An array holding the input points in design file coordinates.
    * @param    numPoints       IN      The number of points in inUors.
    * @param    interpretation  IN      Indicates how input points should be interpreted.
    * @param    destMstnGCS     OUT     The destination DgnGCS .
    * @return   SUCCESS or a CS_MAP error code if any of the points could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus ReprojectUors(DPoint3dP outUorsDest, GeoPointP outLatLongDest, GeoPointP outLatLongSrc, DPoint3dCP inUors,int numPoints, GeoCoordInterpretation interpretation,DgnGCSCR destMstnGCS) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the 2d design coordinates (UORS) of the input Longitude/Latitude point.
    * @param    outUors         OUT     The calculated design coordinates.
    * @param    inLatLong       IN      The longitude,latitude in the datum of this GCS.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus UorsFromLatLong2D(DPoint2dR outUors, GeoPoint2dCR inLatLong) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the longitude, latitude from 2d design coordinates (UORS).
    * @param    outLatLong      OUT     The calculated longitude,latitude in the datum of this GCS.
    * @param    inUors          IN      The input design coordinates.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus LatLongFromUors2D(GeoPoint2dR outLatLong, DPoint2dCR inUors) const;

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
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus ReprojectUors2D(DPoint2dP outUorsDest, GeoPoint2dP outLatLongDest, GeoPoint2dP outLatLongSrc, DPoint2dCP inUors, int numPoints, DgnGCSCR destMstnGCS) const;

    /*---------------------------------------------------------------------------------**//**
    * BaseGCSCartesianFromUors - Converts from the UORs of a DgnGCS to
    * the Cartesian of the target BaseGCS.
    * @param    outCartesian   OUT Receives the output coordinate.
    * @param    inUors         IN  The input coordinate in UORs.
    * @param    targetGCS      IN  target coordinate system
    * @return REPROJECT_Success if the process was fully successful.
    *         REPROJECT_CSMAPERR_OutOfUsefulRange if at least one conversion used for computing
    *           was out of the normal useful domain of either coordinate system.
    *           This can be interpreted as a warning when the extent is known to extend past the
    *           domain of the GCS. This will occur invariably in GCS such as Danmark 34 system
    *           that use a non-square domain (polygon domain).
    *         REPROJECT_CSMAPERR_VerticalDatumConversionError - Indicates elevation shift could not be
    *           applied due to some configuration file missing. This will not normally affect the
    *           result of the present method but should be reported to the user to
    *           fix the configuration issue.
    *         Any other error is a hard error depending on the value.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ReprojectStatus BaseGCSCartesianFromUors(DPoint3dR  outCartesian, DPoint3dCR inUors, GeoCoordinates::BaseGCSCR  targetGCS) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the best approximate transform that can be applied at the indicated region
    * transform coordinates from this GCS's design coordinates to those of the destination Base GCS.
    * @param    outTransform    OUT     The calculated Transform.
    * @param    extent          IN      The extent in design coordinates (UORs) of this GCS to use to find the transform.
    *                                   This extent must of course be valid (not empty) but shall also
    *                                   define an extent no less than 0.01 of the linear units of the input
    *                                   GCS wide in all dimensions. If the input GCS is longitude/latitude then
    *                                   the extent will be no less than 0.0000001 (1e-07) degrees for the first
    *                                   two ordinates and 0.01[Meter] for the elevation (z) ordinate.
    * @param    maxError        OUT     If provided receives the max error observed over the extent
    * @param    meanError       OUT     If provided receives the mean error observed over the extent
    * @return   SUCCESS or a CS_MAP error code if elementOrigin could not be reprojected.
    * @bsimethod
    * @bsimethod
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT  ReprojectStatus       GetLinearTransformToBaseGCS(TransformP outTransform, DRange3dCR extent,GeoCoordinates::BaseGCSCR destBaseGCS, double* maxError, double* meanError) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the best approximate transform that can be applied at the elementOrigin to
    * transform coordinates from this GCS's design coordinates to those of the destination GCS.
    * @param    outTransform    OUT     The calculated Transform.
    * @param    elementOrigin   IN      The point, in meters, at which the transform will be applied.
    * @param    extent          IN      The extent, in meters, of a vector that tells the span of the data to which
    *                                   the transform will be applied. If NULL, a reasonable guess is used.
    * @param    doRotate        IN      true to allow rotation in the transform.
    * @param    doScale         IN      true to allow scaling in the transform.
    * @param    destMstnGCS     OUT     The destination DgnGCS .
    * @return   SUCCESS or a CS_MAP error code if elementOrigin could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus GetLocalTransform(TransformP outTransform, DPoint3dCR elementOrigin, DPoint3dCP extent, bool doRotate, bool doScale, DgnGCSCR destMstnGCS) const;

    /*---------------------------------------------------------------------------------**//**
    * Calculates the best approximate transform that can be applied at the elementOrigin to
    * transform coordinates from this GCS's design coordinates to those of the destination GCS.
    * @param    outTransform    OUT     The calculated Transform.
    * @param    elementOrigin   IN      The point, in design coordinates (UORs) of this GCS, at which the transform will be applied.
    * @param    extent          IN      The extent, in design coordinates (UORs) of a bvector that tells the span of the data to which
    *                                   the transform will be applied. If NULL, a reasonable guess is used.
    * @param    doRotate        IN      true to allow rotation in the transform.
    * @param    doScale         IN      true to allow scaling in the transform.
    * @param    interpretation  IN      Indicates how the points should be interpreted
    * @param    destMstnGCS     OUT     The destination DgnGCS .
    * @return   SUCCESS or a CS_MAP error code if elementOrigin could not be reprojected.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT ReprojectStatus GetLocalTransform(TransformP outTransform, DPoint3dCR elementOrigin, DPoint3dCP extent, bool doRotate, bool doScale, GeoCoordInterpretation interpretation, DgnGCSCR destMstnGCS) const;

    /*---------------------------------------------------------------------------------**//**
    * Gets the localized name of the Geographic Projection used in the Coordinate System.
    * @param    outputBuffer    OUT     Buffer to hold the projection name.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT Utf8CP GetProjectionName(Utf8StringR outputBuffer) const;

    /*---------------------------------------------------------------------------------**//**
    * Gets a name suitable for display in user interface.
    * @param    outputBuffer    OUT     Buffer to hold the projection name.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT Utf8CP GetDisplayName(Utf8StringR outputBuffer) const;

    /*---------------------------------------------------------------------------------**//**
    * Writes the GCS parameters to the project.
    * @param    dgnProject        IN      The model to write the GCS to.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT  StatusInt Store(DgnDbR dgnProject);

    /*---------------------------------------------------------------------------------**//**
    * Factory method that constructs an DgnGCS  instance by attempting to locate the DgnGCS property that saves the
    * geographic coordinate system parameters in the project, and creating the instance based on those parameters.
    * If the DgnGCS for the project has previously been requested, it is cached and this call is very efficient.
    * @param    dgnProject        IN      The project to look in.
    * @return the DgnGCS for this project or NULL if the project is not geo-located
    * @remarks Do not call delete on the returned point
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSP FromProject(DgnDbR dgnProject);

    /*---------------------------------------------------------------------------------**//**
    * Gets the Paper Scale for this GCS. The Paper scale affects the Cartesian coordinates and
    *  makes measurements unreliable. Its use is not recommended.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT double GetPaperScale() const;

    /*---------------------------------------------------------------------------------**//**
    * Sets the Paper Scale for this GCS. The Paper Scale affects the Cartesian coordinates and
    *  makes measurements unreliable. Its use is not recommended. The default and recommended
    *  value is 1.0.
    * @param    paperScale      IN      The new Paper Scale value.
    * @param    project         IN      The project that this GCS came from.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT StatusInt SetPaperScale(double paperScale, DgnDbR project);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT StatusInt CreateGeoCoordType66(short* type66AppData, uint32_t& type66AppDataBytes, DgnDbR project, bool primary) const;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT StatusInt GetUnitDefinition(UnitDefinitionR unitDef, StandardUnit& standardUnitNumber) const;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT bool UnitsIdentical(DgnGCSCR other) const;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static StatusInt GetCSUnitName(Utf8StringR csUnitName, UnitDefinitionCR unitDef);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DgnGCSP FromGeoCoordType66AppData(short const* type66AppData, DgnDbR cache);


    DGNPLATFORM_EXPORT void PublishedCreateGeoCoordType66(short* type66AppData, uint32_t& type66AppDataBytes, DgnDbR project, bool primary) const;
};

/*=================================================================================**//**
The DgnGeoCoordinationAdmin class implements GeoCoordinateAdmin.
It can be used from any DgnPlatform Host program that wants to provide GeoCoordination services.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  DgnGeoCoordinationAdmin : public PlatformLib::Host::GeoCoordinationAdmin
{
private:
    mutable std::unique_ptr<IGeoCoordinateServices> m_gcrp;
    mutable bool                        m_initializationComplete;
    mutable bool                        m_otfEnabled;
    mutable BeFileName                  m_dataDirectory;

    void CompleteInitialization() const;

    DgnGeoCoordinationAdmin(BeFileNameCR dataDirectory);

    IGeoCoordinateServicesP _GetServices() const override;
    BeFileName _GetDataDirectory() override {return m_dataDirectory;}

public:
    DGNPLATFORM_EXPORT static DgnGeoCoordinationAdmin* Create(BeFileNameCR dataDirectory/*, IACSManagerR mgr*/);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
