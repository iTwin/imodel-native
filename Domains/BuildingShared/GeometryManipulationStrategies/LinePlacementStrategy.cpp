/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_BUILDING_SHARED_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr LinePointsPlacementStrategy::_GetCurvePrimitive()
    {
    return m_manipulatorStrategy->FinishPrimitive();
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointLengthAnglePlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr LinePointLengthAnglePlacementStrategy::_GetCurvePrimitive()
    {
    return m_manipulatorStrategy->FinishPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthAnglePlacementStrategy::_SetPropertyValueDouble(Utf8CP propertyName, const double & value)
    {
    if (0 == std::strcmp(BUILDINGSHARED_PROP_Length, propertyName))
        {
        m_length = value;
        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(BUILDINGSHARED_PROP_Angle, propertyName))
        {
        m_angle = value;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthAnglePlacementStrategy::_GetPropertyValueDouble(Utf8CP propertyName, double & value) const
    {
    if (0 == std::strcmp(BUILDINGSHARED_PROP_Length, propertyName))
        {
        value = m_length;
        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(BUILDINGSHARED_PROP_Angle, propertyName))
        {
        value = m_angle;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
LinePointLengthAnglePlacementStrategyPtr LinePointLengthAnglePlacementStrategy::Create(DPlane3d const & plane)
    {
    return new LinePointLengthAnglePlacementStrategy(plane);
    }

END_BUILDING_SHARED_NAMESPACE
