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
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfaceManipulationStrategyPtr LineGridSurfaceManipulationStrategy::Create
(
    BBS::LinePlacementStrategyType linePlacementStrategyType
)
    {
    LineGridSurfaceManipulationStrategyPtr strategy = Create();
    if (strategy.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    strategy->m_geometryPlacementStrategy = LinePlacementStrategy::Create(linePlacementStrategyType, *strategy->m_geometryManipulationStrategy);
    BeAssert(strategy->m_geometryPlacementStrategy.IsValid());
    return strategy;
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

    TransformPointsOnXYPlane(keyPoints);

    m_surface->SetBaseLine(DPoint2d::From(keyPoints[0]), DPoint2d::From(keyPoints[1]));

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

    TransformPointsOnXYPlane(keyPoints);

    SketchLineGridSurface::CreateParams params
    (
        *grid->GetSurfacesModel(),
        *m_axis,
        m_bottomElevation,
        m_topElevation,
        DPoint2d::From(keyPoints[0]),
        DPoint2d::From(keyPoints[1])
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
