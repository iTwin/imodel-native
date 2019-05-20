/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

const Utf8CP SketchGridSurfaceManipulationStrategy::prop_BottomElevation = "BottomElevation";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_TopElevation = "TopElevation";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Axis = "Axis";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Name = "Name";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_WorkingPlane = "WorkingPlane";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Length = "Length";
const Utf8CP SketchGridSurfaceManipulationStrategy::prop_Angle = "Angle";

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    IPlanGridSurface* surface = _GetPlanGridSurfaceP();
    if (nullptr == surface)
        return BentleyStatus::ERROR;

    if (surface->GetThisElem().GetAxisId() != m_axis->GetElementId())
        surface->GetThisElemR().SetAxisId(m_axis->GetElementId());

    if (surface->GetStartElevation() != m_bottomElevation)
        surface->SetStartElevation(m_bottomElevation);

    if (surface->GetEndElevation() != m_topElevation)
        surface->SetEndElevation(m_topElevation);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr SketchGridSurfaceManipulationStrategy::_FinishElement()
    {
    IPlanGridSurface* surface = _GetPlanGridSurfaceP();
    BeAssert(nullptr != surface && "Shouldn't be called with invalid surface");
    
    if (nullptr == surface || !IsComplete())
        return nullptr;

    if (BentleyStatus::ERROR == _UpdateGridSurface())
        return nullptr;

    if (surface->GetThisElemR().Update().IsNull())
        return nullptr;

    return &surface->GetThisElemR();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    double & value
) const
    {
    if (0 == strcmp(key, prop_BottomElevation))
        value = m_bottomElevation;
    else if (0 == strcmp(key, prop_TopElevation))
        value = m_topElevation;
    else
        return _GetGeometryManipulationStrategy().TryGetProperty(key, value);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::GetOrCreateGridAndAxis(SketchGridCPtr& grid, Dgn::SpatialLocationModelPtr spatialModel)
    {
    Dgn::DgnDbR db = spatialModel->GetDgnDb();

    if (m_axis.IsValid())
        {
        // Check if grid name is the same
        grid = db.Elements().Get<SketchGrid>(m_axis->GetGridId());
        if (!m_gridName.empty() && 0 != strcmp(grid->GetName(), m_gridName.c_str()))
            return BentleyStatus::ERROR; // Grid name is incorrect
        }
    else if (!m_gridName.empty())
        {
        // Create new grid and/or axis
        grid = dynamic_cast<SketchGrid*>(Grid::TryGet(db,
                            spatialModel->GetModeledElementId(),
                            m_gridName.c_str()).get());

        if (grid.IsNull())
            {
            SketchGridPtr sketchGrid = SketchGrid::Create(*spatialModel,
                                                          spatialModel->GetModeledElementId(),
                                                          m_gridName.c_str(),
                                                          m_bottomElevation,
                                                          m_topElevation);
            if (sketchGrid.IsNull())
                return BentleyStatus::ERROR; // Failed to create grid

            Placement3d planePlacement = Placement3d();
            planePlacement.TryApplyTransform
            (
                GeometryUtils::FindTransformBetweenPlanes
                (
                    DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1)),
                    m_workingPlane
                )
            );
            sketchGrid->SetPlacement(planePlacement);
            if (sketchGrid->Insert().IsNull())
                return BentleyStatus::ERROR;

            grid = sketchGrid.get();
            }
       
         
        m_axis = GeneralGridAxis::CreateAndInsert(*grid).get();
        if (m_axis.IsNull())
            return BentleyStatus::ERROR;
        }

    if (m_workingPlane.origin.AlmostEqual({ 0, 0, 0 }) && m_workingPlane.normal.AlmostEqual(DVec3d::From(0, 0, 1)))
        SetProperty(prop_WorkingPlane, grid->GetPlane());

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_OnWorkingPlaneChanged(DPlane3d const& original)
    {
    bvector<DPoint3d> allKeyPoints = _GetCurvePrimitiveManipulationStrategy().GetKeyPoints();
    bvector<DPoint3d> acceptedKeyPoints = _GetCurvePrimitiveManipulationStrategy().GetAcceptedKeyPoints();
    for (size_t i = 0; i < acceptedKeyPoints.size(); ++i)
        {
        DPoint3d replacement = TransformPointBetweenPlanes(acceptedKeyPoints[i], original, m_workingPlane);
        ReplaceKeyPoint(replacement, i);
        }

    if (allKeyPoints.size() != acceptedKeyPoints.size())
        {
        size_t dynamicCount = allKeyPoints.size() - acceptedKeyPoints.size();
        
        bvector<DPoint3d> replacements;
        for (size_t i = 0; i < dynamicCount; ++i)
            {
            DPoint3d replacement = TransformPointBetweenPlanes(allKeyPoints[i + acceptedKeyPoints.size()], original, m_workingPlane);
            replacements.push_back(replacement);
            }

        InsertDynamicKeyPoints(replacements, 0);
        }

    if (m_axis.IsValid())
        {
        GridPtr grid = m_axis->GetDgnDb().Elements().GetForEdit<SketchGrid>(m_axis->GetGridId());
        BeAssert(grid.IsValid() && "Axis' grid should be valid");

        Placement3d planePlacement = Placement3d();
        planePlacement.TryApplyTransform
        (
            GeometryUtils::FindTransformBetweenPlanes
            (
                DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1)),
                m_workingPlane
            )
        );
        grid->SetPlacement(planePlacement);
        grid->Update();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
SketchGridSurfaceManipulationStrategy::SketchGridSurfaceManipulationStrategy
(
    Dgn::DgnDbR db
)
    : T_Super(db)
    , m_axis(nullptr)
    , m_gridName("")
    , m_bottomElevation(0)
    , m_topElevation(0)
    , m_workingPlane(DPlane3d::FromOriginAndNormal(DPoint3d::From(0, 0, 0), DVec3d::From(0, 0, 1)))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d SketchGridSurfaceManipulationStrategy::_AdjustPoint
(
    DPoint3d point
) const
    {
    DPoint3d projected = point;
    m_workingPlane.ProjectPoint(projected, point);
    return projected;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d SketchGridSurfaceManipulationStrategy::TransformPointBetweenPlanes(DPoint3d const & point, DPlane3d const & from, DPlane3d const & to)
    {
    DPoint3d result = point;
    GeometryUtils::FindTransformBetweenPlanes(from, to).Multiply(result);
    return result;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> SketchGridSurfaceManipulationStrategy::_GetKeyPoints() const
    {
    return _GetGeometryManipulationStrategy().GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return _GetGeometryManipulationStrategy().IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_ResetDynamicKeyPoint()
    {
    _GetGeometryManipulationStrategyForEdit().ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_IsComplete() const
    {
    if (!_GetGeometryManipulationStrategy().IsComplete())
        return false;

    if (m_axis.IsNull() && m_gridName.empty())
        return false;

    if (m_topElevation <= m_bottomElevation)
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SketchGridSurfaceManipulationStrategy::_CanAcceptMorePoints() const
    {
    return _GetGeometryManipulationStrategy().CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    double const & value
)
    {
    if (0 == strcmp(key, prop_BottomElevation))
        {
        m_bottomElevation = value;
        }
    else if (0 == strcmp(key, prop_TopElevation))
        m_topElevation = value;
    else
        _GetGeometryManipulationStrategyForEdit().SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    Utf8String & value
) const
    {
    if (0 == strcmp(key, prop_Name))
        {
        value = m_gridName;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    DPlane3d & value
) const
    {
    if (0 == strcmp(key, prop_WorkingPlane))
        {
        value = m_workingPlane;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfaceManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    Dgn::DgnElementCP & value
) const
    {
    /*if (0 == strcmp(key, prop_Axis))
        value = *m_axis.get();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;*/

    return BentleyStatus::ERROR; // TODO Change DgnElement to DgnElementPtr
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    Dgn::DgnElementCP const & value
)
    {
    if (0 == strcmp(key, prop_Axis))
        m_axis = dynamic_cast<GridAxisCP>(value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    Utf8String const & value
)
    {
    if (0 == strcmp(key, prop_Name))
        {
        m_gridName = value;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::_SetProperty(Utf8CP key, DPlane3d const & value)
    {
    if (0 == strcmp(key, prop_WorkingPlane))
        {
        DPlane3d originalPlane = m_workingPlane;
        m_workingPlane = value;
        _OnWorkingPlaneChanged(originalPlane);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfaceManipulationStrategy::TransformPointsOnXYPlane(bvector<DPoint3d>& points)
    {
    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1));
    Transform toWorkingPlane = GeometryUtils::FindTransformBetweenPlanes(xyPlane, m_workingPlane);
    Transform toXY = Transform::FromIdentity();
    toXY.InverseOf(toWorkingPlane);

    for (DPoint3dR point : points)
        {
        toXY.Multiply(point);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SketchGridSurfaceManipulationStrategy::GetMessage() const
    {
    return _GetMessage();
    }
