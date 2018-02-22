/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/ArcGridSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfaceManipulationStrategy::ArcGridSurfaceManipulationStrategy()
    : T_Super() 
    , m_geometryManipulationStrategy(ArcManipulationStrategy::Create())
    {
    BeAssert(m_geometryManipulationStrategy.IsValid());
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

    if (Dgn::RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*m_surface, BeSQLite::DbOpcode::Insert, "Insert Grid Surface"))
        return nullptr;

    if (m_surface->Insert().IsNull())
        return nullptr;

    return m_surface;
    }