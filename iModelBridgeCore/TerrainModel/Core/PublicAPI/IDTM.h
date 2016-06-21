/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/IDTM.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley\Bentley.h>
#include <Bentley\RefCounted.h>
#include <Geom/GeomApi.h>
//__PUBLISH_SECTION_START__
#include <TerrainModel\TerrainModel.h>


BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* Structure to define the fence information
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct DTMFenceParams
    {
    DTMFenceType fenceType;
    DTMFenceOption fenceOption;
    DPoint3dCP points;
    int numPoints;

    DTMFenceParams ()
        {
        fenceType = DTMFenceType::None;
        fenceOption = DTMFenceOption::None;
        points = 0;
        numPoints = 0;
        }
    DTMFenceParams (DTMFenceType fenceType, DTMFenceOption fenceOption, DPoint3dCP points, int numPoints)
        {
        this->fenceType = fenceType;
        this->fenceOption = fenceOption;
        this->points = points;
        this->numPoints = numPoints;
        }
    };

//__PUBLISH_SECTION_START__
typedef bvector<DPoint3d> DTMPointArray;

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMDrapedLinePoint abstract : IRefCounted
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
    virtual DTMStatusInt _GetPointCoordinates (DPoint3d& coordP) const = 0;
    virtual double _GetDistanceAlong () const = 0;        
    virtual DTMDrapedLineCode _GetCode () const = 0;

/*__PUBLISH_SECTION_START__*/
public:
    //! Gets the Point Coordinates for this Draped Point.
    //! @param[out]  coordP     The coordinate.
    //! @return The Status.
    BENTLEYDTM_EXPORT DTMStatusInt GetPointCoordinates (DPoint3d& coordP) const;
    //! Gets the distance along for this Draped Point.
    //! @return The distance along.
    BENTLEYDTM_EXPORT double GetDistanceAlong () const;
    //! Gets the Draped Point Code.
    //! @return The DTMDrapedLinePoint Code.
    BENTLEYDTM_EXPORT DTMDrapedLineCode GetCode () const;
};

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMDrapedLine abstract : IRefCounted
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
    virtual DTMStatusInt _GetPointByIndex (DTMDrapedLinePointPtr& ret, unsigned int index) const = 0;
    virtual DTMStatusInt _GetPointByIndex (DPoint3dP ptP, double* distanceP, DTMDrapedLineCode* codeP, unsigned int index) const = 0;
    virtual unsigned int _GetPointCount () const = 0;

/*__PUBLISH_SECTION_START__*/
public:
    //! Gets the Draped Point by index.
    //! @param[out] ret         The DTMDrapedLinePoint for the index.
    //! @param[in]  index       The index of the point to get.
    //! @return return the Status
    BENTLEYDTM_EXPORT DTMStatusInt GetPointByIndex(DTMDrapedLinePointPtr& ret, unsigned int index) const;
    //! Gets Point information by index
    //! @param[out] ptP         The draped line point, can be null.
    //! @param[out] distanceP   The draped line distance, can be null.
    //! @param[out] codeP       The draped line code, can be null.
    //! @param[in]  index       The index of the point to get.
    //! @return error status.
    BENTLEYDTM_EXPORT DTMStatusInt GetPointByIndex (DPoint3dP ptP, double* distanceP, DTMDrapedLineCode* codeP, unsigned int index) const;
    //! Gets the number of draped line points.
    //! @return the number of points.
    BENTLEYDTM_EXPORT unsigned int GetPointCount() const;
};

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMDraping abstract
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
virtual DTMStatusInt _DrapePoint (double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point) = 0;

virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) = 0;

virtual bool _ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint) = 0;

virtual bool _IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) = 0;

virtual bool _DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) = 0;


/*__PUBLISH_SECTION_START__*/
public:
//! Drapes a point onto the DTM.
//! @param[out] elevation       Elevation of the point. Can be null.     
//! @param[out] slope           Slope on the point. Can be null.     
//! @param[out] aspect          Aspect on the point. Can be null.     
//! @param[out] triangle        Triangle around the point. Can be null.     
//! @param[out] drapeType       Type of draping. Can be null. (should be declared as an DTMEnum !!!).    
//! @param[in]  point           The point to drape.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt DrapePoint (double* elevation, double* slope, double* aspect, DPoint3d triangle[3], int* drapedType, DPoint3dCR point);
//! Drapes a linear feature on to the DTM.
//! @param[out] ret         The DTMDrapedLine result.
//! @param[in]  pts         The points of the linear feature.
//! @param[in] numPoints   The number of points.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints);


//! Projects point on the DTM
//! @param[out]  pointOnDTM           Projected point.
//! @param[in]  w2vMap           The world to view map
//! @param[in]  testPoint           The point to project.
BENTLEYDTM_EXPORT bool ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint);

//! Projects point on the DTM along a given direction
//! @param[out]  pointOnDTM           Projected point.
//! @param[in]  direction           The vector giving the direction of projection
//! @param[in]  testPoint           The point to project.
BENTLEYDTM_EXPORT bool IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint);

//! Drapes a point onto the DTM along a vector.
//! @param[out] endPt           Projected point.   
//! @param[out] slope           Slope on the point. Can be null.     
//! @param[out] aspect          Aspect on the point. Can be null.     
//! @param[out] triangle        Triangle around the point. Can be null.     
//! @param[out] drapeType       Type of draping. Can be null. (should be declared as an DTMEnum !!!).    
//! @param[in]  point           The point to drape.
//! @param[in]  directionOfVector           Direction of vector.
//! @param[in]  slopeOfVector           Slope of vector.
//! @return true if there is an intersection with the mesh.
BENTLEYDTM_EXPORT bool DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector);

    };

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMDrainageFeature abstract : IRefCounted
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
virtual int _GetNumParts() = 0;
virtual bool _GetIsPond(int index) = 0;
virtual DTMStatusInt _GetPoints(DTMPointArray& ret, int index) = 0;
/*__OPUBLISH_SECTION_START__*/
public:
//! Gets the Number of Parts in this Drainage feature.
//! @return number of parts.
BENTLEYDTM_EXPORT int GetNumParts();
//! Gets if this part is a Pond
//! @param[in]  index       index.
//! @returns true if this feature is a pond.
BENTLEYDTM_EXPORT bool GetIsPond (int index);
//! Gets the Points of this feature.
//! @param[out] ret         The Points.
//! @param[in]  index       index.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt GetPoints (DTMPointArray& ret, int index);
};

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
 struct IDTMDrainage abstract
 {
 /*__PUBLISH_SECTION_END__*/
 /*__OPUBLISH_CLASS_VIRTUAL__*/
 protected:
virtual DTMStatusInt _GetDescentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth) = 0;
virtual DTMStatusInt _GetAscentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth) = 0;
virtual DTMStatusInt _TraceCatchmentForPoint (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxPondDepth) = 0;
 
 /*__PUBLISH_SECTION_START__*/
 public:
//! Gets the DescentTrace.
//! @param[out] ret          The Descent Trace.
//! @param[in]  pt           The start point.
//! @param[in]  maxpondDepth The Maximum pond depth.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt GetDescentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth);
//! Gets the AscentTrace.
//! @param[out] ret          The Ascent Trace.
//! @param[in]  pt           The start point.
//! @param[in]  maxpondDepth The Maximum pond depth.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt GetAscentTrace (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double minDepth);
//! Gets the Trace Catchment.
//! @param[out] ret          The Trace Catchment.
//! @param[in]  pt           The start point.
//! @param[in]  maxpondDepth The Maximum pond depth.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt TraceCatchmentForPoint (DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxpondDepth);
};

/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMContouring abstract
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
virtual DTMStatusInt _ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt,double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity, const DTMFenceParams& fence) = 0;
virtual DTMStatusInt _ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt,double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity) = 0;

/*__PUBLISH_SECTION_START__*/
public:
//! Gets the Contour from a point.
//! @param[out] ret           The Contour Points.
//! @param[in]  pt            The contour point.
//! @param[in]  smoothOption  The smooth option.
//! @param[in]  smoothFactor  The smooth factor.
//! @param[in]  smoothDensity The smooth Density.
//! @param[in]  fence         The fence details.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt,double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity, const DTMFenceParams& fence);
//! Gets the Contour from a point.
//! @param[out] ret           The Contour Points.
//! @param[in]  pt            The contour point.
//! @param[in]  smoothOption  The smooth option.
//! @param[in]  smoothFactor  The smooth factor.
//! @param[in]  smoothDensity The smooth Density.
//! @return DTM status.
BENTLEYDTM_EXPORT DTMStatusInt ContourAtPoint (DTMPointArray& ret, DPoint3dCR pt,double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity);
};


/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IDTMVolume abstract
{
/*__PUBLISH_SECTION_END__*/
/*__OPUBLISH_CLASS_VIRTUAL__*/
protected:
    virtual DTMStatusInt _ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) = 0;
    virtual DTMStatusInt _ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh) = 0;
    virtual bool _RestrictVolumeToRegion(uint64_t regionId) = 0;
    virtual void _RemoveAllRestrictions() = 0;

/*__PUBLISH_SECTION_START__*/
public:
//! Compute volume between DTM and mesh.
//! @param[out] cut       Cut volume. Can be null.     
//! @param[out] fill           Fill volume. Can be null.     
//! @param[out] volume          Total volume. Can be null.        
//! @param[in]  mesh           Compute volume between DTM and this surface mesh.
//! @return DTM status.
    BENTLEYDTM_EXPORT DTMStatusInt ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh);

//! Compute volume between DTM and closed mesh.
//! @param[out] cut       Cut volume. Can be null.     
//! @param[out] fill           Fill volume. Can be null.     
//! @param[out] volume          Total volume. Can be null.        
//! @param[in]  mesh           Compute volume between DTM and this surface mesh.
//! @return DTM status.
    BENTLEYDTM_EXPORT DTMStatusInt ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh);

//! Clips volume computations to a given region.     
//! @param[in]  regionId        ID of the region to consider
//! @return true if the region is found within the DTM.
BENTLEYDTM_EXPORT bool RestrictVolumeToRegion(uint64_t regionId);

//! Use the whole DTM for further computations.     
BENTLEYDTM_EXPORT void RemoveAllRestrictions();
};
/*__PUBLISH_SECTION_END__*/

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* Interface implemented by DTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/

typedef std::function<void(DTMStatusInt status,double flatArea, double slopeArea)> DTMAreaValuesCallback;
typedef std::function<bool()> DTMCancelProcessCallback;
struct IDTM abstract : IRefCounted
{
//__PUBLISH_SECTION_END__
//__PUBLISH_CLASS_VIRTUAL__
protected: 
virtual int64_t _GetPointCount () = 0;
virtual IDTMDraping* _GetDTMDraping () = 0;
virtual IDTMVolume* _GetDTMVolume() = 0;
virtual IDTMDrainage* _GetDTMDrainage () = 0;
virtual IDTMContouring* _GetDTMContouring () = 0;
virtual DTMStatusInt _GetRange(DRange3dR range) = 0;
virtual BcDTMP _GetBcDTM() = 0;
virtual DTMStatusInt _GetBoundary(DTMPointArray& ret) = 0;
virtual DTMStatusInt _CalculateSlopeArea (double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints) = 0;
virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) = 0;
virtual DTMStatusInt _GetTransformDTM (DTMPtr& transformedDTM, TransformCR transformation) = 0;
virtual bool         _GetTransformation (TransformR transformation) = 0;
virtual DTMStatusInt _ExportToGeopakTinFile(WCharCP fileNameP) = 0;

//__PUBLISH_SECTION_START__
public:
//! Gets the number of points of the DTM.
//! @return The number of points of the DTM..
BENTLEYDTM_EXPORT int64_t GetPointCount ();
//__PUBLISH_SECTION_END__

//! Gets the draping interface.
//! @return The draping interface.
BENTLEYDTM_EXPORT IDTMDraping* GetDTMDraping ();

//! Gets the drainage interface.
//! @return The drainage interface.
BENTLEYDTM_EXPORT IDTMDrainage* GetDTMDrainage ();

//! Gets the contouring interface.
//! @return The contouring interface.
BENTLEYDTM_EXPORT IDTMContouring* GetDTMContouring ();

//! Gets the draping interface.
//! @return The draping interface.
BENTLEYDTM_EXPORT IDTMVolume* GetDTMVolume();
//__PUBLISH_SECTION_START__

//! Gets the range of the DTM.
//! @param[out] range        The range
//! @return error status.
BENTLEYDTM_EXPORT DTMStatusInt GetRange (DRange3dR range);

//__PUBLISH_SECTION_END__

//! Gets the boundary of the DTM.
//! @return the PointArray of the boundary
BENTLEYDTM_EXPORT DTMStatusInt GetBoundary (DTMPointArray& ret);

//! Gets the Slope Area for a DTM.
//! @param[out] flatArea     The flat area.
//! @param[out] slopeArea    The slope area.
//! @param[in] pts           The points of the area.
//! @param[in] numPoints     The number of points of the area.
//! @return error status.
BENTLEYDTM_EXPORT DTMStatusInt CalculateSlopeArea (double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints);

BENTLEYDTM_EXPORT DTMStatusInt CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback);
//__PUBLISH_SECTION_START__

//! Gets a Transformed copy of the DTM.
//! @param[out] transformedDTM The transformed DTM.
//! @param[in] transformation  The transformation to apply.
//! @return error status.
BENTLEYDTM_EXPORT DTMStatusInt GetTransformDTM (DTMPtr& transformedDtm, TransformCR transformation);

//! Gets the Current Transformation for this DTM.
//! @param[out] transformation The transformation.
//! @return true if this is an identity transformation.
BENTLEYDTM_EXPORT bool GetTransformation (TransformR transformation);

//! Save this DTM to a geopak tin file.
//! @param[in] geopak tin file name.
//! @return true if save operation succeed.
BENTLEYDTM_EXPORT DTMStatusInt ExportToGeopakTinFile(WCharCP fileNameP);

//! Gets the BcDTM of the Current DTM if this is a BcDTM.
//! @return the BcDTM.
BENTLEYDTM_EXPORT BcDTMP GetBcDTM ();
};

END_BENTLEY_TERRAINMODEL_NAMESPACE
//__PUBLISH_SECTION_END__
