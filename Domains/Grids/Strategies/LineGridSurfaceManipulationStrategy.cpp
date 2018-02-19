/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LineGridSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfaceManipulationStrategy::LineGridSurfaceManipulationStrategy()
    : T_Super()
    , m_geometryManipulationStrategy(LineManipulationStrategy::Create())
    , m_surface(nullptr)
    {
    BeAssert(m_geometryManipulationStrategy.IsValid() && "Geometry manipulation strategy is valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    T_Super::_UpdateGridSurface();

    bvector<DPoint3d> keyPoints = m_geometryManipulationStrategy->GetKeyPoints();
    if (2 != keyPoints.size())
        return BentleyStatus::ERROR;

    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1));
    Transform toWorkingPlane = GeometryUtils::FindTransformBetweenPlanes(xyPlane, m_workingPlane);
    Transform toXY = Transform::FromIdentity();
    toXY.InverseOf(toWorkingPlane);

    DPoint3d startPoint, endPoint;
    toXY.Multiply(startPoint, keyPoints[0]);
    toXY.Multiply(endPoint, keyPoints[1]);

    m_surface->SetBaseLine(DPoint2d::From(startPoint), DPoint2d::From(endPoint));

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String LineGridSurfaceManipulationStrategy::_GetMessage() const
    {
    return ""; // TODO
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr LineGridSurfaceManipulationStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
    if (!IsComplete())
        return nullptr;

    Dgn::SpatialLocationModelPtr spatialModel = dynamic_cast<Dgn::SpatialLocationModelP>(&model);
    if (spatialModel.IsNull())
        return nullptr;

    SketchGridCPtr grid;
    if (BentleyStatus::ERROR == GetOrCreateGridAndAxis(grid, spatialModel))
        return nullptr;

    bvector<DPoint3d> keyPoints = m_geometryManipulationStrategy->GetKeyPoints();
    if (2 != keyPoints.size())
        return nullptr;

    if (m_surface.IsValid())
        return T_Super::_FinishElement();

    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal({ 0, 0, 0 }, DVec3d::From(0, 0, 1));
    Transform toWorkingPlane = GeometryUtils::FindTransformBetweenPlanes(xyPlane, m_workingPlane);
    Transform toXY = Transform::FromIdentity();
    toXY.InverseOf(toWorkingPlane);

    DPoint3d startPoint, endPoint;
    toXY.Multiply(startPoint, keyPoints[0]);
    toXY.Multiply(endPoint, keyPoints[1]);

    SketchLineGridSurface::CreateParams params
    (
        *grid->GetSurfacesModel(),
        *m_axis,
        m_bottomElevation,
        m_topElevation,
        DPoint2d::From(startPoint),
        DPoint2d::From(endPoint)
    );

    m_surface = SketchLineGridSurface::Create(params);
    if (m_surface.IsNull())
        return nullptr;

    if (!m_surface->GetDgnDb().Txns().InDynamicTxn())
        if (Dgn::RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*m_surface, BeSQLite::DbOpcode::Insert, "Insert Grid Surface"))
            return nullptr;

    if (m_surface->Insert().IsNull())
        return nullptr;

    return m_surface;
    }
