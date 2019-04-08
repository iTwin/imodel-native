/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LineGridSurfacePlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfacePlacementStrategy::LineGridSurfacePlacementStrategy
(
    LinePlacementStrategyType linePlacementStrategyType,
    Dgn::DgnDbR db
)   : T_Super(db)
    , m_manipulationStrategy(LineGridSurfaceManipulationStrategy::Create(linePlacementStrategyType, db))
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
LineGridSurfacePlacementStrategyPtr LineGridSurfacePlacementStrategy::Create
(
    BBS::LinePlacementStrategyType linePlacementStrategyType, 
    Dgn::DgnDbR db
)
    {
    return new LineGridSurfacePlacementStrategy(linePlacementStrategyType, db);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineGridSurfacePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (BentleyStatus::SUCCESS == _GetSketchGridSurfaceManipulationStrategy().TryGetProperty(key, value) ||
        BentleyStatus::SUCCESS == T_Super::_TryGetProperty(key, value))
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfacePlacementStrategy::_SetProperty(Utf8CP key, double const & value)
    {
    m_manipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }