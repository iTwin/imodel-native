/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_BUILDING_SHARED_NAMESPACE

const Utf8CP LinePlacementStrategy::prop_Length = "Length";
const Utf8CP LinePlacementStrategy::prop_Angle = "Angle";

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (_GetKeyPoints().size() < 2)
        T_Super::_AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if ((!IsDynamicKeyPointSet() && _GetKeyPoints().size() < 2) ||
        (IsDynamicKeyPointSet() && _GetKeyPoints().size() <= 2))
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsPlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    if (newDynamicKeyPoints.size() <= 2 - _GetKeyPoints().size())
        T_Super::_AddDynamicKeyPoints(newDynamicKeyPoints);
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointLengthAnglePlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (0 == _GetKeyPoints().size())
        T_Super::_AddKeyPoint(newKeyPoint);

    DPoint3d endPoint;
    if (BentleyStatus::SUCCESS == CalculateEndPoint(endPoint))
        T_Super::_AddKeyPoint(endPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    if (0 == _GetKeyPoints().size() || IsDynamicKeyPointSet())
        {
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint); // for calculating the end point

        DPoint3d endPoint;
        if (BentleyStatus::SUCCESS == CalculateEndPoint(endPoint))
            T_Super::_AddDynamicKeyPoints({ newDynamicKeyPoint, endPoint });
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    if (newDynamicKeyPoints.size() == 1 - _GetKeyPoints().size()) // Only one dynamic key point can be added and only if there are no points
        T_Super::_AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_PopKeyPoint()
    {
    T_Super::_PopKeyPoint();
    T_Super::_PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointLengthAnglePlacementStrategy::CalculateEndPoint(DPoint3dR endPoint)
    {
    if (0 == _GetKeyPoints().size())
        return BentleyStatus::ERROR;

    DVec3d lineVec = DVec3d::From(m_length, 0, 0);
    lineVec.RotateXY(m_angle);

    if (BentleyStatus::ERROR == GeometryUtils::TransformVectorOnPlane(lineVec, lineVec, m_workingPlane))
        return BentleyStatus::ERROR;

    endPoint = _GetKeyPoints().front();
    endPoint.Add(lineVec);
    
    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::UpdateEndPoint()
    {
    DPoint3d endPoint;
    if (BentleyStatus::SUCCESS == CalculateEndPoint(endPoint))
        {
        BeAssert(_GetKeyPoints().size() > 0 && "There should be at least 1 key point");
        if (!IsDynamicKeyPointSet())
            GetLineManipulationStrategyR().ReplaceKeyPoint(endPoint, 1);
        else
            {
            DPoint3d firstPoint = _GetKeyPoints().front();
            ResetDynamicKeyPoint();
            T_Super::_AddDynamicKeyPoints({ firstPoint, endPoint });
            }
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_SetWorkingPlane(DPlane3d const & plane)
    {
    m_workingPlane = plane; 

    UpdateEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_SetProperty(Utf8CP key, const double & value)
    {
    if (0 == strcmp(LinePlacementStrategy::prop_Length, key))
        _SetLength(value);
    else if (0 == strcmp(LinePlacementStrategy::prop_Angle, key))
        _SetAngle(value);

    UpdateEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointLengthAnglePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (0 == strcmp(LinePlacementStrategy::prop_Length, key))
        value = _GetLength();
    else if (0 == strcmp(LinePlacementStrategy::prop_Angle, key))
        value = _GetAngle();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_SetLength(double const & length)
    {
    m_length = length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointLengthAnglePlacementStrategy::_GetLength() const
    {
    return m_length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_SetAngle(double const & angle)
    {
    m_angle = angle;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointLengthAnglePlacementStrategy::_GetAngle() const
    {
    return m_angle;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointsLengthPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_SetLength(double const & length)
    {
    m_length = length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointsLengthPlacementStrategy::_GetLength() const
    {
    return m_length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_AddKeyPoint(DPoint3dCR newKeyPoint)
    {
    if (_GetKeyPoints().size() < 2)
        T_Super::_AddKeyPoint(newKeyPoint);

    bvector<DPoint3d> points = _GetKeyPoints();
    if (2 == points.size())
        m_direction = DVec3d::FromStartEnd(points[0], points[1]);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    if ((!IsDynamicKeyPointSet() && _GetKeyPoints().size() < 2) ||
        (IsDynamicKeyPointSet() && _GetKeyPoints().size() <= 2))
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);

    bvector<DPoint3d> points = _GetKeyPoints();
    if (2 == points.size())
        m_direction = DVec3d::FromStartEnd(points[0], points[1]);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    if (newDynamicKeyPoints.size() <= 2 - _GetKeyPoints().size())
        T_Super::_AddDynamicKeyPoints(newDynamicKeyPoints);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsLengthPlacementStrategy::AdjustEndPoint()
    {
    bvector<DPoint3d> points = _GetKeyPoints();
    
    if (points.size() < 2)
        return BentleyStatus::SUCCESS; // Noting to do

    DPoint3d startPoint = points[0];
    DPoint3d endPoint;
    
    DVec3d direction = m_direction;
    direction.ScaleToLength(m_length);

    endPoint = startPoint;
    endPoint.Add(direction);

    if (!IsDynamicKeyPointSet())
        GetLineManipulationStrategyR().ReplaceKeyPoint(endPoint, 1);
    else
        {
        int dynamicCount = GetKeyPoints().size() - GetLineManipulationStrategyR().GetAcceptedKeyPoints().size();
        if (1 == dynamicCount)
            {
            T_Super::_AddDynamicKeyPoint(endPoint);
            }
        else if (2 == dynamicCount)
            {
            T_Super::_AddDynamicKeyPoints({ startPoint, endPoint });
            }
        else
            {
            BeAssert(false && "In dynamics there must be either 1 or 2 dynamic key points");
            return BentleyStatus::ERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_SetProperty(Utf8CP key, const double & value)
    {
    if (0 == strcmp(BUILDINGSHARED_PROP_Length, key))
        _SetLength(value);
    
    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsLengthPlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (0 == strcmp(BUILDINGSHARED_PROP_Length, key))
        value = _GetLength();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

END_BUILDING_SHARED_NAMESPACE

