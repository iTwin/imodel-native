#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineGridSurfacePlacementStrategy::SplineGridSurfacePlacementStrategy
(
    SplinePlacementStrategyType strategyType
)   : T_Super()
    , m_manipulationStrategy(SplineGridSurfaceManipulationStrategy::Create(strategyType))
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }