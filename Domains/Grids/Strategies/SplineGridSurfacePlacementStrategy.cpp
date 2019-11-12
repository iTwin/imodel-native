/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineGridSurfacePlacementStrategy::SplineGridSurfacePlacementStrategy
(
    SplinePlacementStrategyType strategyType,
    Dgn::DgnDbR db
)   : T_Super(db)
    , m_manipulationStrategy(SplineGridSurfaceManipulationStrategy::Create(strategyType, db))
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineGridSurfacePlacementStrategyPtr SplineGridSurfacePlacementStrategy::Create
(
    BBS::SplinePlacementStrategyType strategyType, 
    Dgn::DgnDbR db
)
    {
    return new SplineGridSurfacePlacementStrategy(strategyType, db);
    }