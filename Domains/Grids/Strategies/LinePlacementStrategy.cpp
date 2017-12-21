/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/LinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/GridsApi.h>
#include <ConstraintSystem/ConstraintSystemApi.h>

BEGIN_GRIDS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr LinePointsPlacementStrategy::_GetCurvePrimitive()
    {
    if (m_points.size() < 2)
        return nullptr;

    return ICurvePrimitive::CreateLine(m_points[0], m_points[1]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointsPlacementStrategy::_SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value)
    {
    if (0 == std::strcmp(GRIDS_PROP_StartPoint, propertyName))
        {
        if (m_points.size() > 0)
            m_points[0] = value;
        else
            m_points.push_back(value);

        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(GRIDS_PROP_EndPoint, propertyName))
        {
        if (m_points.size() > 1)
            m_points[1] = value;
        else
            m_points.push_back(value);
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointsPlacementStrategy::_GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const
    {
    if (0 == std::strcmp(GRIDS_PROP_StartPoint, propertyName)
        && m_points.size() > 0)
        {
        value = m_points[0];
        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(GRIDS_PROP_EndPoint, propertyName)
        && m_points.size() > 1)
        {
        value = m_points[1];
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointsPlacementStrategy::_AddPoint(DPoint3d point)
    {
    if (m_points.size() >= 2)
        return BentleyStatus::ERROR; // Only 2 points can be in the point pool

    return T_Super::_AddPoint(point);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr LinePointLengthPlacementStrategy::_GetCurvePrimitive()
    {
    if (m_points.size() < 1 || !m_useLength || !m_useAngle)
        return nullptr;

    DVec3d lineVec = DVec3d::FromXYAngleAndMagnitude(m_angle, m_length);
    if (BentleyStatus::ERROR == GeometryUtils::ProjectVectorOnPlane(lineVec, m_workingPlane))
        return nullptr;

    DPoint3d endPoint = m_points[0];
    endPoint.Add(lineVec);

    return ICurvePrimitive::CreateLine(m_points[0], endPoint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthPlacementStrategy::_SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value)
    {
    if (0 == std::strcmp(GRIDS_PROP_Point, propertyName))
        {
        if (m_points.size() > 0)
            m_points[0] = value;
        else
            m_points.push_back(value);

        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthPlacementStrategy::_GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const
    {
    if (0 == std::strcmp(GRIDS_PROP_Point, propertyName)
        && m_points.size() > 0)
        {
        value = m_points[0];
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthPlacementStrategy::_SetPropertyValueDouble(Utf8CP propertyName, const double & value)
    {
    if (0 == std::strcmp(GRIDS_PROP_Length, propertyName))
        {
        m_length = value;
        m_useLength = true;

        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(GRIDS_PROP_Angle, propertyName))
        {
        m_angle = value;
        m_useAngle = true;

        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthPlacementStrategy::_GetPropertyValueDouble(Utf8CP propertyName, double & value) const
    {
    if (0 == std::strcmp(GRIDS_PROP_Length, propertyName)
        && m_useLength)
        {
        value = m_length;
        return BentleyStatus::SUCCESS;
        }

    if (0 == std::strcmp(GRIDS_PROP_Angle, propertyName)
        && m_useAngle)
        {
        value = m_angle;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
BentleyStatus LinePointLengthPlacementStrategy::_AddPoint(DPoint3d point)
    {
    if (m_points.size() >= 1)
        return BentleyStatus::ERROR; // Only 1 point can be in the point pool

    return T_Super::_AddPoint(point);
    }

END_GRIDS_NAMESPACE
