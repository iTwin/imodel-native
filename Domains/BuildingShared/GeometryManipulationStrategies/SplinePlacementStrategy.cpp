/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/SplinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_BUILDING_SHARED_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////
// SplineControlPointsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////

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
void SplineControlPointsPlacementStrategy::_SetProperty(Utf8CP key, const int & value)
    {
    if (0 == std::strcmp(key, prop_Order()))
        _SetOrder(value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SplineControlPointsPlacementStrategy::_TryGetProperty(Utf8CP key, int & value) const
    {
    if (0 == std::strcmp(key, prop_Order()))
        value = _GetOrder();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
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