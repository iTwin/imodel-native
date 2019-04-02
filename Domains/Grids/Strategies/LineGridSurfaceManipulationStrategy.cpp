/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LineGridSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfaceManipulationStrategyPtr LineGridSurfaceManipulationStrategy::Create
(
    SketchLineGridSurfaceR surface
)
    {
    LineGridSurfaceManipulationStrategyPtr strategy = Create(surface.GetDgnDb());
    strategy->m_surface = &surface;
    DPoint2d start2d, end2d;
    surface.GetBaseLine(start2d, end2d);
    DPoint3d start = DPoint3d::From(start2d.x, start2d.y, 0);
    DPoint3d end = DPoint3d::From(end2d.x, end2d.y, 0);
    strategy->m_geometryManipulationStrategy = LineManipulationStrategy::Create(start, end);
    strategy->m_axis = GridAxis::Get(surface.GetDgnDb(), surface.GetAxisId());
    strategy->m_bottomElevation = surface.GetStartElevation();
    strategy->m_topElevation = surface.GetEndElevation();

    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfaceManipulationStrategyPtr LineGridSurfaceManipulationStrategy::Create
(
    Dgn::DgnDbR db
)
    {
    return new LineGridSurfaceManipulationStrategy(db);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfaceManipulationStrategy::LineGridSurfaceManipulationStrategy
(
    Dgn::DgnDbR db
)
    : T_Super(db)
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
    BBS::LinePlacementStrategyType linePlacementStrategyType,
    Dgn::DgnDbR db
)
    {
    LineGridSurfaceManipulationStrategyPtr strategy = Create(db);
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

    if (m_surface->Insert().IsNull())
        return nullptr;

    return m_surface;
    }
