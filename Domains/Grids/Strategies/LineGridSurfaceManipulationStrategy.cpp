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

    Dgn::DgnDbR db = spatialModel->GetDgnDb();

    GridCPtr grid;
    if (m_axis.IsValid())
        {
        // Check if grid name is the same
        grid = db.Elements().Get<Grid>(m_axis->GetGridId());
        if (!m_gridName.empty() && 0 != strcmp(grid->GetName(), m_gridName.c_str()))
            return nullptr; // Grid name is incorrect
        }
    else if (!m_gridName.empty())
        {
        // Create new grid and/or axis
        grid = Grid::TryGet(db,
                            spatialModel->GetModeledElementId(),
                            m_gridName.c_str()).get();
        if (grid.IsNull())
            {
            SketchGridPtr sketchGrid = SketchGrid::Create(*spatialModel,
                                            spatialModel->GetModeledElementId(),
                                            m_gridName.c_str(),
                                            m_bottomElevation,
                                            m_topElevation);
            if (sketchGrid.IsNull())
                return nullptr; // Failed to create grid

            if (sketchGrid->Insert().IsNull())
                return nullptr;

            grid = sketchGrid.get();
            }
        Dgn::DefinitionModelCR defModel = db.GetDictionaryModel();
        m_axis = GeneralGridAxis::CreateAndInsert(defModel, *grid).get();
        }

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
