/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LinePlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"
#include <BuildingShared/Utils/UtilsApi.h>
#include <BuildingShared/Units/UnitsApi.h>

BEGIN_BUILDING_SHARED_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePlacementStrategy::LinePlacementStrategy
(
    LineManipulationStrategyR manipulationStrategy
) 
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePlacementStrategyPtr LinePlacementStrategy::Create
(
    LinePlacementStrategyType strategyType,
    LineManipulationStrategyR manipulationStrategy
)
    {
    switch (strategyType)
        {
        case LinePlacementStrategyType::Points:
            return LinePointsPlacementStrategy::Create(manipulationStrategy);
        case LinePlacementStrategyType::PointsAngle:
            return LinePointsAnglePlacementStrategy::Create(manipulationStrategy, DPlane3d::FromOriginAndNormal({ 0,0,0 }, DVec3d::From(0, 0, 1))); // default plane...
        case LinePlacementStrategyType::PointsLength:
            return LinePointsLengthPlacementStrategy::Create(manipulationStrategy);
        case LinePlacementStrategyType::PointLengthAngle:
            return LinePointLengthAnglePlacementStrategy::Create(manipulationStrategy, DPlane3d::FromOriginAndNormal({ 0,0,0 }, DVec3d::From(0, 0, 1))); // default plane...
        default:
            BeAssert(false);
            return nullptr;
        }
    }

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

    DPoint3d startPoint = _GetKeyPoints().front();
    endPoint = startPoint;
    endPoint.x += m_length; // Set correct distance between points;

    GeometryUtils::RotateLineEndPointToAngleOnPlane(endPoint, startPoint, m_angle, m_workingPlane);
    
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
            GetLineManipulationStrategyForEdit().ReplaceKeyPoint(endPoint, 1);
        else
            {
            DPoint3d firstPoint = _GetKeyPoints().front();
            ResetDynamicKeyPoint();
            T_Super::_AddDynamicKeyPoints({ firstPoint, endPoint });
            }
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePointLengthAnglePlacementStrategy::LinePointLengthAnglePlacementStrategy
(
    LineManipulationStrategyR manipulationStrategy,
    DPlane3d const & workingPlane
)
    : T_Super(manipulationStrategy)
    , m_workingPlane(workingPlane)
    {
    bvector<DPoint3d> keyPoints = GetKeyPoints();
    if (2 == keyPoints.size())
        {
        m_length = keyPoints[0].Distance(keyPoints[1]);
        m_angle = keyPoints[0].PlanarAngleTo(keyPoints[2], m_workingPlane.normal);
        }

    RegisterDoubleProperty(prop_Length());
    RegisterDoubleProperty(prop_Angle());
    RegisterDPlane3dProperty(prop_WorkingPlane());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
LinePointLengthAnglePlacementStrategy::LinePointLengthAnglePlacementStrategy
(
    DPlane3d const & workingPlane
)
    : LinePointLengthAnglePlacementStrategy(*LineManipulationStrategy::Create(), workingPlane)
    {
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
    if (0 == strcmp(LinePlacementStrategy::prop_Length(), key))
        SetLength(value);
    else if (0 == strcmp(LinePlacementStrategy::prop_Angle(), key))
        SetAngle(value);

    UpdateEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointLengthAnglePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (0 == strcmp(LinePlacementStrategy::prop_Length(), key))
        value = GetLength();
    else if (0 == strcmp(LinePlacementStrategy::prop_Angle(), key))
        value = GetAngle();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::_SetProperty
(
    Utf8CP key, 
    DPlane3d const & value
)
    {
    if (0 == strcmp(LinePlacementStrategy::prop_WorkingPlane(), key))
        _SetWorkingPlane(value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointLengthAnglePlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    DPlane3d & value
) const
    {
    if (0 == strcmp(LinePlacementStrategy::prop_WorkingPlane(), key))
        {
        value = _GetWorkingPlane();
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::SetLength(double const & length)
    {
    m_length = length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointLengthAnglePlacementStrategy::GetLength() const
    {
    return m_length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointLengthAnglePlacementStrategy::SetAngle(double const & angle)
    {
    m_angle = angle;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointLengthAnglePlacementStrategy::GetAngle() const
    {
    return m_angle;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointsLengthPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::SetLength(double const & length)
    {
    m_length = length;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointsLengthPlacementStrategy::GetLength() const
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
        GetLineManipulationStrategyForEdit().ReplaceKeyPoint(endPoint, 1);
    else
        {
        int dynamicCount = static_cast<int>(GetKeyPoints().size() - GetLineManipulationStrategyForEdit().GetAcceptedKeyPoints().size());
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
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePointsLengthPlacementStrategy::LinePointsLengthPlacementStrategy
(
    LineManipulationStrategyR manipulationStrategy
)   : T_Super(manipulationStrategy)
    {
    bvector<DPoint3d> keyPoints = GetKeyPoints();
    if (2 == keyPoints.size())
        {
        m_length = keyPoints[0].Distance(keyPoints[1]);
        m_direction = DVec3d::FromStartEnd(keyPoints[0], keyPoints[1]);
        }

    RegisterDoubleProperty(prop_Length());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
LinePointsLengthPlacementStrategy::LinePointsLengthPlacementStrategy() 
    : LinePointsLengthPlacementStrategy(*LineManipulationStrategy::Create())
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsLengthPlacementStrategy::_SetProperty(Utf8CP key, const double & value)
    {
    if (0 == strcmp(prop_Length(), key))
        SetLength(value);
    
    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsLengthPlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (0 == strcmp(prop_Length(), key))
        value = GetLength();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// LinePointsAnglePlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
LinePointsAnglePlacementStrategy::LinePointsAnglePlacementStrategy
(
    LineManipulationStrategyR manipulationStrategy,
    DPlane3d workingPlane
)
    : T_Super(manipulationStrategy)
    , m_workingPlane(workingPlane)
    {
    bvector<DPoint3d> keyPoints = GetKeyPoints();
    if (2 == keyPoints.size())
        {
        m_angle = keyPoints[0].PlanarAngleTo(keyPoints[2], m_workingPlane.normal);
        }
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_SetProperty(Utf8CP key, const double & value)
    {
    if (0 == strcmp(prop_Angle(), key))
        SetAngle(value);
    else
        return;

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsAnglePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (0 == strcmp(prop_Angle(), key))
        value = GetAngle();
    else
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_SetProperty
(
    Utf8CP key,
    DPlane3d const & value
)
    {
    if (0 == strcmp(LinePlacementStrategy::prop_WorkingPlane(), key))
        _SetWorkingPlane(value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             02/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsAnglePlacementStrategy::_TryGetProperty
(
    Utf8CP key,
    DPlane3d & value
) const
    {
    if (0 == strcmp(LinePlacementStrategy::prop_WorkingPlane(), key))
        {
        value = _GetWorkingPlane();
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::SetAngle(double const & angle)
    {
    m_angle = angle;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
double LinePointsAnglePlacementStrategy::GetAngle() const
    {
    return m_angle;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_SetWorkingPlane(DPlane3d const & plane)
    {
    m_workingPlane = plane;

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_AddKeyPoint(DPoint3dCR newKeyPoint)
    {
    if (_GetKeyPoints().size() < 2)
        T_Super::_AddKeyPoint(newKeyPoint);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    if ((!IsDynamicKeyPointSet() && _GetKeyPoints().size() < 2) ||
        (IsDynamicKeyPointSet() && _GetKeyPoints().size() <= 2))
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LinePointsAnglePlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    if (newDynamicKeyPoints.size() <= 2 - _GetKeyPoints().size())
        T_Super::_AddDynamicKeyPoints(newDynamicKeyPoints);

    AdjustEndPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas             01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinePointsAnglePlacementStrategy::AdjustEndPoint()
    {
    bvector<DPoint3d> points = _GetKeyPoints();

    if (2 != points.size())
        return BentleyStatus::SUCCESS;

    DPoint3d startPoint = points.front();
    DPoint3d endPoint = points.back();

    GeometryUtils::RotateLineEndPointToAngleOnPlane(endPoint, startPoint, m_angle, m_workingPlane);

    if (!IsDynamicKeyPointSet())
        GetLineManipulationStrategyForEdit().ReplaceKeyPoint(endPoint, 1);
    else
        {
        int dynamicCount = static_cast<int>(GetKeyPoints().size() - GetLineManipulationStrategyForEdit().GetAcceptedKeyPoints().size());
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

/////////////////////////////////////////////////////////////////////////////////////////
// LineMetesAndBoundsPlacementStrategy
/////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
LineMetesAndBoundsPlacementStrategy::LineMetesAndBoundsPlacementStrategy
(
    DPlane3d plane
) 
    : T_Super(plane)
    , m_directionString("") 
    {
    RegisterCustomProperty(prop_MetesAndBounds());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineMetesAndBoundsPlacementStrategy::_SetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty const& value
)
    {
    if (0 == strcmp(key, LineMetesAndBoundsPlacementStrategy::prop_MetesAndBounds()))
        {
        MetesAndBounds const* mab = dynamic_cast<MetesAndBounds const*>(&value);
        if (nullptr != mab && !mab->GetValue().empty())
            {
            bpair<Utf8String, double> metesAndBounds = mab->GetValue().front();

            double angle;
            UnitConverter::MeetsAndBoundsStringToDouble(angle, metesAndBounds.first.c_str());
            T_Super::_SetProperty(LinePlacementStrategy::prop_Angle(), angle);
            T_Super::_SetProperty(LinePlacementStrategy::prop_Length(), metesAndBounds.second);
            }
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineMetesAndBoundsPlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty& value
) const
    {
    if (0 == strcmp(key, LineMetesAndBoundsPlacementStrategy::prop_MetesAndBounds()))
        {
        double length;
        if (BentleyStatus::SUCCESS != T_Super::_TryGetProperty(LinePlacementStrategy::prop_Length(), length))
            return BentleyStatus::ERROR;

        double angle;
        if (BentleyStatus::SUCCESS != T_Super::_TryGetProperty(LinePlacementStrategy::prop_Angle(), angle))
            return BentleyStatus::ERROR;

        DVec3d direction = DVec3d::From(1, 0, 0);
        direction.RotateXY(angle);
        Utf8String directionString;
        UnitConverter::DirectionToMeetsAndBoundsString(directionString, direction);

        value = MetesAndBounds(directionString.c_str(), length);
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
MetesAndBounds::MetesAndBounds
(
    bvector<Utf8String> directionStrings, 
    bvector<double> lengths
)
    {
    auto directionsIter = directionStrings.begin();
    auto lengthsIter = lengths.begin();
    for (; directionStrings.end() != directionsIter && lengths.end() != lengthsIter
         ; ++directionsIter, ++lengthsIter)
        {
        m_value.push_back(ValuePair(*directionsIter, *lengthsIter));
        }
    }

END_BUILDING_SHARED_NAMESPACE
