/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/ArcGridSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfaceManipulationStrategy::ArcGridSurfaceManipulationStrategy
(
    Dgn::DgnDbR db
)
    : T_Super(db) 
    , m_geometryManipulationStrategy(ArcManipulationStrategy::Create())
    {
    BeAssert(m_geometryManipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfaceManipulationStrategyPtr ArcGridSurfaceManipulationStrategy::Create
(
    Dgn::DgnDbR db
)
    {
    return new ArcGridSurfaceManipulationStrategy(db);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfaceManipulationStrategyPtr ArcGridSurfaceManipulationStrategy::Create
(
    BBS::ArcPlacementMethod arcPlacementMethod,
    Dgn::DgnDbR db
)
    {
    ArcGridSurfaceManipulationStrategyPtr strategy = Create(db);
    if (strategy.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    strategy->m_geometryPlacementStrategy = ArcPlacementStrategy::Create(arcPlacementMethod, *strategy->m_geometryManipulationStrategy);
    BeAssert(strategy->m_geometryPlacementStrategy.IsValid());
    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ArcGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    BentleyStatus superStatus = T_Super::_UpdateGridSurface();
    if (BentleyStatus::SUCCESS != superStatus)
        return superStatus;

    ICurvePrimitivePtr arcPrimitive = m_geometryManipulationStrategy->FinishPrimitive();
    DEllipse3d arc;
    if (arcPrimitive.IsNull() || !arcPrimitive->TryGetArc(arc))
        return BentleyStatus::ERROR;

    m_surface->SetBaseArc(arc);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr ArcGridSurfaceManipulationStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
    if (!IsComplete())
        return nullptr;

    Dgn::SpatialLocationModelPtr spatialModel = dynamic_cast<Dgn::SpatialLocationModelP>(&model);
    if (spatialModel.IsNull())
        return nullptr;

    ICurvePrimitivePtr arcPrimitive = m_geometryManipulationStrategy->FinishPrimitive();
    DEllipse3d arc;
    if (arcPrimitive.IsNull() || !arcPrimitive->TryGetArc(arc))
        return nullptr;

    if (m_surface.IsValid())
        return T_Super::_FinishElement();

    SketchGridCPtr grid;
    if (BentleyStatus::ERROR == GetOrCreateGridAndAxis(grid, spatialModel))
        return nullptr;

    SketchArcGridSurface::CreateParams params(*spatialModel, *m_axis, m_bottomElevation, m_topElevation, arc);
    m_surface = SketchArcGridSurface::Create(params);
    if (m_surface.IsNull())
        return nullptr;

    if (m_surface->Insert().IsNull())
        return nullptr;

    return m_surface;
    }