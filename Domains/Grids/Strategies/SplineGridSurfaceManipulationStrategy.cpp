#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineGridSurfaceManipulationStrategy::SplineGridSurfaceManipulationStrategy
(
    SplinePlacementStrategyType strategyType
)   : T_Super()
    , m_geometryManipulationStrategy(SplineManipulationStrategy::Create(strategyType))
    , m_geometryPlacementStrategy(m_geometryManipulationStrategy->CreatePlacement())
    , m_surface(nullptr)
    {
        BeAssert(m_geometryManipulationStrategy.IsValid() && "Geometry manipulation strategy is valid");
        BeAssert(m_geometryPlacementStrategy.IsValid() && "Geometry placement strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SplineGridSurfaceManipulationStrategy::_UpdateGridSurface()
    {
        T_Super::_UpdateGridSurface();

        ICurvePrimitivePtr curvePrimitive = m_geometryManipulationStrategy->FinishPrimitive();
        if (curvePrimitive.IsValid())
            return BentleyStatus::ERROR;

        m_surface->SetBaseSpline(curvePrimitive);

        return BentleyStatus::SUCCESS;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SplineGridSurfaceManipulationStrategy::_GetMessage() const
    {
        return ""; // TODO
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementPtr SplineGridSurfaceManipulationStrategy::_FinishElement
(
    Dgn::DgnModelR model
)
    {
        if(!IsComplete())
            return nullptr;

        Dgn::SpatialLocationModelPtr spatialModel = dynamic_cast<Dgn::SpatialLocationModelP>(&model);
        if (spatialModel.IsNull())
            return nullptr;

        SketchGridCPtr grid;
        if (BentleyStatus::ERROR == GetOrCreateGridAndAxis(grid, spatialModel))
            return nullptr;

        ICurvePrimitivePtr curvePrimitive = m_geometryManipulationStrategy->FinishPrimitive();
        if (curvePrimitive.IsValid())
            return nullptr;

        if (m_surface.IsValid())
            return T_Super::_FinishElement();
        
        SketchSplineGridSurface::CreateParams params
        (
            *grid->GetSurfacesModel(),
            *m_axis,
            m_bottomElevation,
            m_topElevation,
            *curvePrimitive
        );
        
        m_surface = SketchSplineGridSurface::Create(params);
        
        if (m_surface.IsNull())
            return nullptr;

        if (!m_surface->GetDgnDb().Txns().InDynamicTxn())
            if (Dgn::RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*m_surface, BeSQLite::DbOpcode::Insert, "Insert Grid Surface"))
                return nullptr;

        if (m_surface->Insert().IsNull())
            return nullptr;
    
        return m_surface; 
    }