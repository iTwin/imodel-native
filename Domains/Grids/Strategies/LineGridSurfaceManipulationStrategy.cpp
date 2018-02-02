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
    {
    BeAssert(m_geometryManipulationStrategy.IsValid() && "Geometry manipulation strategy is valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    SketchLineGridSurfacePtr surface = dynamic_cast<SketchLineGridSurfaceP>(m_surface.get());
    if (surface.IsNull())
        return BentleyStatus::ERROR;

    T_Super::_UpdateGridSurface();

    bvector<DPoint3d> keyPoints = m_geometryManipulationStrategy->GetKeyPoints();
    if (2 != keyPoints.size())
        return BentleyStatus::ERROR;

    surface->SetBaseLine(DPoint2d::From(keyPoints[0]), DPoint2d::From(keyPoints[1]));

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
BBS::CurvePrimitivePlacementStrategyPtr LineGridSurfaceManipulationStrategy::_GetStrategyForAppend()
    {
    return m_geometryManipulationStrategy->CreateLinePlacementStrategy(m_currentPlacementType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfaceManipulationStrategy::ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType)
    {
    m_currentPlacementType = newLinePlacementStrategyType;
    // TODO properties?
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineGridSurfaceManipulationStrategy::_IsComplete() const
    {
    return m_geometryManipulationStrategy->IsComplete() && T_Super::_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineGridSurfaceManipulationStrategy::_CanAcceptMorePoints() const
    {
    return m_geometryManipulationStrategy->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfaceManipulationStrategy::_SetProperty(Utf8CP key, double const & value)
    {
    T_Super::_SetProperty(key, value);
    m_geometryManipulationStrategy->SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineGridSurfaceManipulationStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (BentleyStatus::ERROR == T_Super::_TryGetProperty(key, value))
        return m_geometryManipulationStrategy->TryGetProperty(key, value);

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr LineGridSurfaceManipulationStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
    if (m_surface.IsValid())
        return T_Super::_FinishElement();

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
