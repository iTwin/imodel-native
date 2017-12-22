/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    /*if (m_points.size() < 2)
        return nullptr;*/

    //return ICurvePrimitive::CreateLine(m_points[0], m_points[1]);

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointsPlacementStrategy::_SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value)
    {
    /*if (0 == std::strcmp(BUILDINGSHARED_PROP_StartPoint, propertyName))
        {
        if (m_points.size() > 0)
            m_points[0] = value;
        else
            m_points.push_back(value);

        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(BUILDINGSHARED_PROP_EndPoint, propertyName))
        {
        if (m_points.size() > 1)
            m_points[1] = value;
        else if (m_points.size() > 0)
            m_points.push_back(value);
        else
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }*/

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointsPlacementStrategy::_GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const
    {
    /*if (0 == std::strcmp(BUILDINGSHARED_PROP_StartPoint, propertyName)
        && m_points.size() > 0)
        {
        value = m_points[0];
        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(BUILDINGSHARED_PROP_EndPoint, propertyName)
        && m_points.size() > 1)
        {
        value = m_points[1];
        return BentleyStatus::SUCCESS;
        }*/

    return BentleyStatus::ERROR;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointLengthAnglePlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr LinePointLengthAnglePlacementStrategy::_GetCurvePrimitive()
    {
    /*if (m_points.size() < 1 || GeometryUtils::AlmostEqual(0, m_length))
        return nullptr;

    DVec3d lineVec = DVec3d::From(0, m_length, 0);
    ValidatedDVec3d validatedVec = DVec3d::FromRotateVectorAroundVector(lineVec, m_workingPlane.normal, Angle::FromRadians(m_angle));
    if (validatedVec.IsValid())
        lineVec = validatedVec.Value();

    DPoint3d endPoint = m_points[0];
    endPoint.Add(lineVec);

    return ICurvePrimitive::CreateLine(m_points[0], endPoint);*/
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthAnglePlacementStrategy::_SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value)
    {
   /* if (0 == std::strcmp(BUILDINGSHARED_PROP_Point, propertyName))
        {
        if (m_points.size() > 0)
            m_points[0] = value;
        else
            m_points.push_back(value);

        return BentleyStatus::SUCCESS;
        }*/

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthAnglePlacementStrategy::_GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const
    {
    /*if (0 == std::strcmp(BUILDINGSHARED_PROP_Point, propertyName)
        && m_points.size() > 0)
        {
        value = m_points[0];
        return BentleyStatus::SUCCESS;
        }
*/
    return BentleyStatus::ERROR;
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
