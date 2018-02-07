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
LineGridSurfaceManipulationStrategy::LineGridSurfaceManipulationStrategy
(
    LinePlacementStrategyType linePlacementStrategyType
)
    : T_Super()
    , m_currentPlacementType(linePlacementStrategyType)
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
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BBS::CurvePrimitivePlacementStrategyPtr LineGridSurfaceManipulationStrategy::_GetGeometryPlacementStrategyP()
    {
    return m_geometryManipulationStrategy->CreateLinePlacementStrategy(m_currentPlacementType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BBS::CurvePrimitivePlacementStrategyCPtr LineGridSurfaceManipulationStrategy::_GetGeometryPlacementStrategy() const
    {
    return m_geometryManipulationStrategy->CreateLinePlacementStrategy(m_currentPlacementType).get();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfaceManipulationStrategy::ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType)
    {
    m_currentPlacementType = newLinePlacementStrategyType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BBS::LinePlacementStrategyType LineGridSurfaceManipulationStrategy::GetCurrentPlacementType() const
    {
    return m_currentPlacementType;
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
    ValidateGridAndAxis(grid, spatialModel);

    bvector<DPoint3d> keyPoints = m_geometryManipulationStrategy->GetKeyPoints();
    if (2 != keyPoints.size())
        return nullptr;

    if (m_surface.IsValid())
        return T_Super::_FinishElement();

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
