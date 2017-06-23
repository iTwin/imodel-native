/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridPortion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include "SurfaceSet.h"
#include "GridSurface.h"


GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridPortion)

BEGIN_GRIDS_NAMESPACE

typedef bvector<Grids::GridSurfacePtr> GridElementVector;
typedef bmap<Utf8String, GridElementVector> GridAxisMap;
typedef bmap<Utf8String, GridAxisMap> GridNameMap;

#define DEFAULT_AXIS ""
#define HORIZONTAL_AXIS "AxisX"
#define VERTICAL_AXIS "AxisY"

#define GRIDLINE_LENGTH_COEFF 0.1

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridPortion : SurfaceSet
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridPortion, SurfaceSet);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER (GridPortion::T_Super::CreateParams);

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params);
        };

public:
    enum class GridType
        {
        GRID_TYPE_Orthogonal,
        GRID_TYPE_Radial_Plane,
        GRID_TYPE_Radial_Arc,
        GRID_TYPE_Sketch
        };

    struct RadialGridCreateParams
        {
        Dgn::DgnModelCR m_model;
        double m_fullAngle;
        double m_iterationAngle;
        int m_circularCount;
        double m_circularInterval;
        double m_length;
        double m_height;
        DPoint3d m_origin;
        double m_rotationAngle;

        //! Creates create parameters for radial grid
        //! @param[in] model            model to create the grid in
        //! @param[in] fullAngle        angle between first and last grid planes
        //! @param[in] iterationAngl    angle between each grid plane
        //! @param[in] circularCount    grid arcs count
        //! @param[in] circularInterval distance between grid arcs
        //! @param[in] length           length of grid lines
        //! @param[in] height           height of grid lines
        //! @param[in] origin           point where grid begins
        //! @param[in] rotationAngle    angle that grid is rotated by
        RadialGridCreateParams(Dgn::DgnModelCR model, double fullAngle, double iterationAngle, int circularCount, double circularInterval, double length, double height, DPoint3d origin, double rotationAngle) :
            m_model(model),
            m_fullAngle(fullAngle),
            m_iterationAngle(iterationAngle),
            m_circularCount(circularCount),
            m_circularInterval(circularInterval),
            m_length(length),
            m_height(height),
            m_origin(origin),
            m_rotationAngle(rotationAngle)
            {
            }
        };

    struct SketchLineGridCreateParams
        {
        Dgn::DgnModelCR m_model;
        DPoint3d m_startPoint;
        DPoint3d m_endPoint;
        double m_height;
        Utf8String m_axisName;

        //! Creates create parameters for sketch line grid
        //! @param[in] model        model to create grid in
        //! @param[in] startPoint   point where grid line begins
        //! @param[in] endPoint     point where grid line ends
        //! @param[in] height       height of grid line
        //! @param[in] axisName     axis to create grid in
        SketchLineGridCreateParams(Dgn::DgnModelCR model, DPoint3d startPoint, DPoint3d endPoint, double height, Utf8String axisName) :
            m_model(model),
            m_startPoint(startPoint),
            m_endPoint(endPoint),
            m_height(height),
            m_axisName(axisName)
            {
            }
        };

    struct SketchSplineGridCreateParams
        {
        Dgn::DgnModelCR m_model;
        bvector<DPoint3d> m_poles;
        double m_height;
        Utf8String m_axisName;

        //! Creates create parameters for sketch spline grid
        //! @param[in] model    model to create grid in
        //! @param[in] poles    points that spline passes through
        //! @param[in] height   height of grid line
        //! @param[in] axisName axis to create grid in
        SketchSplineGridCreateParams(Dgn::DgnModelCR model, bvector<DPoint3d> poles, double height, Utf8String axisName) :
            m_model(model),
            m_poles(poles),
            m_height(height),
            m_axisName(axisName)
            {
            }
        };

protected:
    explicit GRIDELEMENTS_EXPORT GridPortion (T_Super::CreateParams const& params);
    friend struct GridPortionHandler;

    static  GRIDELEMENTS_EXPORT CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

    //! Creates extrusion detail for orthogonal grid on plane {(0,0,0), (length,0,0), (0,0,height)}
    //! @param[in] length   length of grid
    //! @param[in] height   height of grid
    //! @return             formed grid extrusion detail
    static DgnExtrusionDetail CreatePlaneGridExtrusionDetail(double length, double height);

    //! Creates extrusion detail for a grid with line {starPoint, endPoint} and given height
    //! @param[in] startPoint   starting point for grid plane
    //! @param[in] endPoint     ending point for grid plane
    //! @param[in] height       grid height
    //! @return                 formed grid extrusion detail
    static DgnExtrusionDetail CreatePlaneGridExtrusionDetail(DPoint3d startPoint, DPoint3d endPoint, double height);

    //! Creates extrusion detail for arc with center point (0,0,0)
    //! @param[in] radius   radius of arc from center
    //! @param[in] angle    arc angle
    //! @param[in] height   height of grid
    //! @return             formed grid extrusion detail
    static DgnExtrusionDetail CreateArcGridExtrusionDetail(double radius, double angle, double height);

    //! Creates extrusion detail for spline with given poles and height
    //! @param[in] poles    points for spline
    //! @param[in] height   height of grid
    //! @return             formed grid extrusion detail
    static DgnExtrusionDetail CreateSketchSplineGridExtrusionDetail(bvector<DPoint3d> poles, double height);

    //! Creates a grid surface
    //! @param[in] model        a model to create grid in
    //! @param[in] extDetail    grid's extrusion detail
    //! @param[in] type         type of grid. 
    //!                         GRID_TYPE_Orthogonal and GRID_TYPE_Radial_Plane will create a GridPlaneSurface
    //!                         GRID_TYPE_Radial_Arc will create a GridArcSurface
    //!                         GRID_TYPE_Sketch will create a GridSurface
    //! @return                 pointer to the grid surface
    static GridSurfacePtr CreateGridSurface(Dgn::DgnModelCR model, DgnExtrusionDetail extDetail, GridType type);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPortion, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static GridPortionPtr Create (Dgn::DgnModelCR model);

    //! Inserts all grid elements in given map
    //! @param[in] elements a grid axis map containing grid elements
    //! @return             BentleyStatus::SUCCESS if no error has occured while inserting elements
    static BentleyStatus InsertGridMapElements(GridAxisMap elements);

    //! Creates radial grid and returns it as a map where DEFAULT_AXIS maps to grid elements
    //! @param[in] params   grid parameters containing information about the grid. For more info look up RadialGridCreateParams
    //! @return             GridAxisMap containing the grid surfaces
    GRIDELEMENTS_EXPORT static GridAxisMap CreateRadialGrid(RadialGridCreateParams params);

    //! Creates a sketch line grid and returns it as a map where given axis in create params maps to the grid element
    //! @param[in] params   grid parameters containing information about the grid. For more info look up SketchLineGridCreateParams
    //! @return             GridAxisMap containing the grid plane
    GRIDELEMENTS_EXPORT static GridAxisMap CreateSketchLineGrid(SketchLineGridCreateParams params);

    //! Creates a sketch spline grid and returns it as a map where given axis in create params maps to the grid element
    //! @param[in] params   grid parameters containing information about the grid. For more info look up SketchSplineGridCreateParams
    //! @return             GridAxisMap containing the grid surface
    GRIDELEMENTS_EXPORT static GridAxisMap CreateSketchSplineGrid(SketchSplineGridCreateParams params);

    //! Creates radial grid and inserts its elements to db
    //! @param[in] params   grid parameters containing information about the grid. For more info look up RadialGridCreateParams
    //! @return             BentleyStatus::SUCCESS if no error has occured while inserting elements.
    GRIDELEMENTS_EXPORT static BentleyStatus CreateAndInsertRadialGrid(RadialGridCreateParams params);

    //! Creates sketch line grid and inserts its element to db
    //! @param[in] params   grid parameters containing information about the grid. For more info look up SketchLineGridCreateParams
    //! @return             BenleyStatus::SUCCESS if no error has occured while inserting elements.
    GRIDELEMENTS_EXPORT static BentleyStatus CreateAndInsertSketchLineGrid(SketchLineGridCreateParams params);

    //! Creates sketch spline grid and inserts its element to db
    //! @param[in] params   grid parameters containing information about the grid. For more info look up SketchSplineGridCreateParams
    GRIDELEMENTS_EXPORT static BentleyStatus CreateAndInsertSketchSplineGrid(SketchSplineGridCreateParams params);
};

END_GRIDS_NAMESPACE