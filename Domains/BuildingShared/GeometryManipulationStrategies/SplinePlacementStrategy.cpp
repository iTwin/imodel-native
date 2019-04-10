/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/SplinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

BEGIN_BUILDING_SHARED_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////
// SplinePlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
SplinePlacementStrategyType SplinePlacementStrategy::GetType() const
    {
    return _GetType();
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SplineControlPointsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineControlPointsPlacementStrategy::SplineControlPointsPlacementStrategy
(
    SplineControlPointsManipulationStrategyR manipulationStrategy
)
    : T_Super(),
    m_manipulationStrategy(&manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineControlPointsPlacementStrategy::_SetOrder(int const & order)
    {
    m_manipulationStrategy->SetOrder(order);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
int SplineControlPointsPlacementStrategy::_GetOrder() const
    {
    return m_manipulationStrategy->GetOrder();
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SplineThroughPointsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
SplineThroughPointsPlacementStrategy::SplineThroughPointsPlacementStrategy
(
    SplineThroughPointsManipulationStrategyR manipulationStrategy
)
    : T_Super(),
    m_manipulationStrategy(&manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsPlacementStrategy::_SetStartTangent(DVec3d tangent)
    {
    m_manipulationStrategy->SetStartTangent(tangent);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsPlacementStrategy::_RemoveStartTangent()
    {
    m_manipulationStrategy->RemoveStartTangent();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d SplineThroughPointsPlacementStrategy::_GetStartTangent() const
    {
    return m_manipulationStrategy->GetStartTangent();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsPlacementStrategy::_SetEndTangent(DVec3d tangent)
    {
    m_manipulationStrategy->SetEndTangent(tangent);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SplineThroughPointsPlacementStrategy::_RemoveEndTangent()
    {
    m_manipulationStrategy->RemoveEndTangent();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d SplineThroughPointsPlacementStrategy::_GetEndTangent() const
    {
    return m_manipulationStrategy->GetEndTangent();
    }

END_BUILDING_SHARED_NAMESPACE