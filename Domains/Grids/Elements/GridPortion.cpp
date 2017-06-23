#include "PublicApi/GridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <GeometryUtils.h>
#include <BuildingUtils.h>
#include "PublicApi\GridArcSurface.h"
#include "PublicApi\GridPlaneSurface.h"
#include <UnitConverter.h>
#include <DimensionHandler.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridPortion)

#define CIRCULAR_GRID_EXTEND_LENGTH 5
#define SPLINE_ORDER 3

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::GridPortion
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (params.m_dgndb.GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridSurface);
        if (catId.IsValid ())
            DoSetCategoryId (catId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::CreateParams           GridPortion::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnElement::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return CreateParams(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortionPtr                 GridPortion::Create 
(
Dgn::DgnModelCR model
)
    {
    return new GridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::CreateParams::CreateParams (DgnElement::CreateParams const& params)
    : T_Super (params)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GridPortion::CreatePlaneGridExtrusionDetail(double length, double height)
    {
    DVec3d gridNormal = DVec3d::From(1.0, 0.0, 0.0);

    DPoint3d startPoint = DPoint3d::FromZero();
    DPoint3d endPoint = DPoint3d::FromSumOf(startPoint, gridNormal, length);

    return CreatePlaneGridExtrusionDetail(startPoint, endPoint, height);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GridPortion::CreatePlaneGridExtrusionDetail(DPoint3d startPoint, DPoint3d endPoint, double height)
    {
    bvector<DPoint3d> points = { startPoint, endPoint };

    CurveVectorPtr base = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_None);
    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GridPortion::CreateArcGridExtrusionDetail(double radius, double angle, double height)
    {
    double extendAngle = UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH) / radius;
    if (angle > 2 * (msGeomConst_pi - extendAngle))
        extendAngle = (2 * msGeomConst_pi - angle) / 3; //Do not allow arc overlapping

    DPoint3d center = DPoint3d::FromZero();

    DPoint3d start = center;
    GeometryUtils::AddRotatedVectorToPoint(start, DVec3d::FromStartEnd(center, DPoint3d::From(0.0, radius)), extendAngle);

    DPoint3d end = center;
    GeometryUtils::AddRotatedVectorToPoint(end, DVec3d::FromStartEnd(center, DPoint3d::From(0.0, radius)), -(angle + extendAngle));

    CurveVectorPtr base = CurveVector::Create(GeometryUtils::CreateArc(center, start, end, false));

    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GridPortion::CreateSketchSplineGridExtrusionDetail(bvector<DPoint3d> poles, double height)
    {
    bvector<double> weights;
    for (DPoint3d pole : poles)
        weights.push_back(1.0);

    int order = poles.size() < SPLINE_ORDER ? poles.size() : SPLINE_ORDER;

    bvector<double> knots;
    for (int i = 0; i < order + poles.size(); ++i)
        knots.push_back(i);

    MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder(poles, &weights, &knots, order, false, false);
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateBsplineCurve(bspline);
    CurveVectorPtr base = CurveVector::Create(curvePrimitive);

    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridSurfacePtr GridPortion::CreateGridSurface(Dgn::DgnModelCR model, DgnExtrusionDetail extDetail, GridType type)
    {
    switch (type)
        {
        case GridType::GRID_TYPE_Orthogonal:
        case GridType::GRID_TYPE_Radial_Plane:
            return GridPlaneSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
        case GridType::GRID_TYPE_Radial_Arc:
            return GridArcSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
        case GridType::GRID_TYPE_Sketch:
            return GridSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
        default:
            return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridAxisMap GridPortion::CreateRadialGrid(RadialGridCreateParams params)
    {
    GridAxisMap radialGrid;

    DgnExtrusionDetail extDetail = CreatePlaneGridExtrusionDetail(params.m_length, params.m_height);
    GridPlaneSurfacePtr baseGridPlane = dynamic_cast<GridPlaneSurface *>(CreateGridSurface(params.m_model, extDetail, GridType::GRID_TYPE_Radial_Plane).get());
    if (!baseGridPlane.IsValid())
        return GridAxisMap();

    baseGridPlane->MoveToPoint(params.m_origin);
    baseGridPlane->RotateXY(params.m_rotationAngle);

    // Create plane grids
    for (double i = 0; i <= params.m_fullAngle; i += params.m_iterationAngle)
        {
        GridPlaneSurfacePtr planeSurface = dynamic_cast<GridPlaneSurface *>(baseGridPlane->Clone().get());
        if (!planeSurface.IsValid())
            return GridAxisMap();

        planeSurface->RotateXY(i);
        radialGrid[DEFAULT_AXIS].push_back(planeSurface);
        }

    for (int i = 0; i < params.m_circularCount; ++i)
        {
        DgnExtrusionDetail extDetail = CreateArcGridExtrusionDetail((i + 1) * params.m_circularInterval, params.m_fullAngle, params.m_height);
        GridArcSurfacePtr arcSurface = dynamic_cast<GridArcSurface *>(CreateGridSurface(params.m_model, extDetail, GridType::GRID_TYPE_Radial_Arc).get());
        if (!arcSurface.IsValid())
            return GridAxisMap();

        arcSurface->MoveToPoint(params.m_origin);
        arcSurface->RotateXY(params.m_rotationAngle);
        radialGrid[DEFAULT_AXIS].push_back(arcSurface);
        }

    return radialGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridAxisMap GridPortion::CreateSketchLineGrid(SketchLineGridCreateParams params)
    {
    GridAxisMap sketchGrid;

    DgnExtrusionDetail extDetail = CreatePlaneGridExtrusionDetail(params.m_startPoint, params.m_endPoint, params.m_height);
    GridSurfacePtr sketchLineGrid = CreateGridSurface(params.m_model, extDetail, GridType::GRID_TYPE_Sketch);

    sketchGrid[params.m_axisName].push_back(sketchLineGrid);

    return sketchGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridAxisMap GridPortion::CreateSketchSplineGrid(SketchSplineGridCreateParams params)
    {
    GridAxisMap sketchGrid;

    DgnExtrusionDetail extDetail = CreateSketchSplineGridExtrusionDetail(params.m_poles, params.m_height);
    GridSurfacePtr sketchSplineGrid = CreateGridSurface(params.m_model, extDetail, GridType::GRID_TYPE_Sketch);

    sketchGrid[params.m_axisName].push_back(sketchSplineGrid);

    return sketchGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus GridPortion::InsertGridMapElements(GridAxisMap elements)
    {
    for (bpair<Utf8String, GridElementVector> pair : elements)
        for (GridSurfacePtr gridSurface : pair.second)
            if (BentleyStatus::ERROR == BuildingUtils::InsertElement(gridSurface))
                return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus GridPortion::CreateAndInsertRadialGrid(RadialGridCreateParams params)
    {
    GridAxisMap grid = CreateRadialGrid(params);

    return InsertGridMapElements(grid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus GridPortion::CreateAndInsertSketchLineGrid(SketchLineGridCreateParams params)
    {
    GridAxisMap grid = CreateSketchLineGrid(params);

    return InsertGridMapElements(grid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus GridPortion::CreateAndInsertSketchSplineGrid(SketchSplineGridCreateParams params)
    {
    GridAxisMap grid = CreateSketchSplineGrid(params);

    return InsertGridMapElements(grid);
    }

END_GRIDS_NAMESPACE