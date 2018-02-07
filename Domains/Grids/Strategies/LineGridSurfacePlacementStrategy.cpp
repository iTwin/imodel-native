/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LineGridSurfacePlacementStrategy.cpp $
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
LineGridSurfacePlacementStrategy::LineGridSurfacePlacementStrategy
(
    LinePlacementStrategyType linePlacementStrategyType
) : T_Super()
  , m_manipulationStrategy(LineGridSurfaceManipulationStrategy::Create(linePlacementStrategyType))
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineGridSurfacePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    return m_manipulationStrategy->TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfacePlacementStrategy::_SetProperty(Utf8CP key, double const & value)
    {
    m_manipulationStrategy->SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineGridSurfacePlacementStrategy::ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType)
    {
    m_manipulationStrategy->ChangeCurrentPlacementType(newLinePlacementStrategyType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
BBS::LinePlacementStrategyType LineGridSurfacePlacementStrategy::GetCurrentPlacementType() const
    {
    return m_manipulationStrategy->GetCurrentPlacementType();
    }
