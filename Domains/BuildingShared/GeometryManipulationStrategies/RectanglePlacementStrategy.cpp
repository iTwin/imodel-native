/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
RectanglePlacementStrategy::RectanglePlacementStrategy
(
    CurveVectorManipulationStrategyR manipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy)
    , m_rotationPointSet(false)
    , m_dynamicRotationPointSet(false)
    {
    m_manipulationStrategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    m_manipulationStrategy->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
RectanglePlacementStrategyPtr RectanglePlacementStrategy::Create()
    {
    return new RectanglePlacementStrategy(*CurveVectorManipulationStrategy::Create());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
RectanglePlacementStrategyPtr RectanglePlacementStrategy::Create
(
    CurveVectorManipulationStrategyR manipulationStrategy
)
    {
    return new RectanglePlacementStrategy(manipulationStrategy);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RectanglePlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    RotMatrix& value
) const
    {
    if (0 == strcmp(key, prop_Rotation()))
        {
        bvector<DPoint3d> points = m_manipulationStrategy->GetKeyPoints();
        
        RotMatrix tmp;
        if (m_dynamicRotationPointSet)
            tmp.InitFrom1Vector(m_dynamicRotationPoint - points.front(), 0, true);
        else if (m_rotationPointSet)
            tmp.InitFrom1Vector(m_rotationPoint - points.front(), 0, true);
        else
            return BentleyStatus::ERROR;
        
        value.InitFrom(Transform::From(tmp).ValidatedInverse());
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> RectanglePlacementStrategy::CalculateLast3Point
(
    DPoint3d start, 
    DPoint3d rotationPoint, 
    DPoint3d end
) const
    {
    DSegment3d rectangleSide = DSegment3d::From(start, rotationPoint);

    DPoint3d projectionPointToExtendedRectangleSide;
    double projectionParamToExtendedRectangleSide;
    rectangleSide.ProjectPoint(projectionPointToExtendedRectangleSide, projectionParamToExtendedRectangleSide, end);

    DVec3d vectorToAnotherRectangleSide = end - projectionPointToExtendedRectangleSide;
    DPoint3d p3 = projectionPointToExtendedRectangleSide + vectorToAnotherRectangleSide;
    DPoint3d p4 = start + vectorToAnotherRectangleSide;

    return {projectionPointToExtendedRectangleSide, p3, p4};
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
void RectanglePlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> points = m_manipulationStrategy->GetKeyPoints();

    if (points.empty())
        {
        T_Super::_AddKeyPoint(newKeyPoint);
        }
    else if (points.size() == 1 && !m_rotationPointSet)
        {
        m_rotationPoint = newKeyPoint;
        m_rotationPointSet = true;
        }
    else if (points.size() <= 4)
        {
        BeAssert(m_rotationPointSet);
        bvector<DPoint3d> last3points = CalculateLast3Point(points[0], m_rotationPoint, newKeyPoint);
        BeAssert(last3points.size() == 3);

        T_Super::_AddKeyPoint(last3points[0]);
        T_Super::_AddKeyPoint(last3points[1]);
        T_Super::_AddKeyPoint(last3points[2]);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
void RectanglePlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    bvector<DPoint3d> points = m_manipulationStrategy->GetKeyPoints();

    if (points.empty())
        {
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);
        }
    else if (points.size() == 1 && !m_rotationPointSet)
        {
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);
        m_dynamicRotationPoint = newDynamicKeyPoint;
        m_dynamicRotationPointSet = true;
        }
    else if (points.size() <= 4)
        {
        BeAssert(m_rotationPointSet);
        bvector<DPoint3d> last3points = CalculateLast3Point(points[0], m_rotationPoint, newDynamicKeyPoint);
        BeAssert(last3points.size() == 3);

        T_Super::_AddDynamicKeyPoints(last3points);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
void RectanglePlacementStrategy::_ResetDynamicKeyPoint()
    {
    T_Super::_ResetDynamicKeyPoint();
    m_dynamicRotationPointSet = false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
void RectanglePlacementStrategy::_PopKeyPoint()
    {
    bvector<DPoint3d> points = m_manipulationStrategy->GetKeyPoints();

    if (points.size() == 4)
        {
        T_Super::_PopKeyPoint();
        T_Super::_PopKeyPoint();
        T_Super::_PopKeyPoint();
        }
    else if (points.size() == 1 && m_rotationPointSet)
        {
        m_rotationPointSet = false;
        }
    else
        {
        T_Super::_PopKeyPoint();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
bool RectanglePlacementStrategy::_IsComplete() const
    {
    return m_manipulationStrategy->GetKeyPoints().size() == 4;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
bool RectanglePlacementStrategy::_CanAcceptMorePoints() const
    {
    return m_manipulationStrategy->GetKeyPoints().size() < 4;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> RectanglePlacementStrategy::_GetKeyPoints() const
    {
    bvector<DPoint3d> points = m_manipulationStrategy->GetKeyPoints();

    if (points.size() == 1 && m_rotationPointSet)
        points.push_back(m_rotationPoint);
    else if (points.size() == 1 && m_dynamicRotationPointSet)
        points.push_back(m_dynamicRotationPoint);

    return points;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2017
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr RectanglePlacementStrategy::_FinishGeometry() const
    {
    CurveVectorPtr geometry = m_manipulationStrategy->Finish(true);
    if (geometry.IsNull())
        return nullptr;

    return IGeometry::Create(geometry);
    }