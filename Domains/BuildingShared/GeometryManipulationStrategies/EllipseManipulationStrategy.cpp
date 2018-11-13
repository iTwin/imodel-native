/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/EllipseManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

#define SWEEP_BREAK Angle::PiOver2()

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr EllipseManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.size() < 2)
        return nullptr;

    DPoint3d start = GetStart();
    DPoint3d center = GetCenter();
    if (keyPoints.size() == 2)
        return ICurvePrimitive::CreateArc(DEllipse3d::FromCenterNormalRadius(center, m_workingPlane.normal, center.Distance(start)));

    DVec3d vec0 = GetVec0();
    DVec3d vec90 = GetVec90();
    if (keyPoints.size() == 3)
        return ICurvePrimitive::CreateArc(
            DEllipse3d::FromVectors(center, vec0, vec90, 0, Angle::TwoPi()));

    if (keyPoints.size() == 4)
        return ICurvePrimitive::CreateArc(
            DEllipse3d::FromVectors(center, vec0, vec90, 0, m_sweep));

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();

    if (GetAcceptedKeyPoints().size() == 4)
        return;

    if (GetAcceptedKeyPoints().size() == 2)
        {
        if (DoubleOps::AlmostEqual(CalculateVec90(keyPoints[0], keyPoints[1], newKeyPoint).Magnitude(), 0))
            return;
        }

    T_Super::_AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d EllipseManipulationStrategy::GetStart() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 1);
    return keyPoints[s_startIndex];
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d EllipseManipulationStrategy::GetCenter() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 2);
    return keyPoints[s_centerIndex];
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DVec3d EllipseManipulationStrategy::GetVec0() const
    {
    DPoint3d start = GetStart();
    DPoint3d center = GetCenter();

    return DVec3d::FromStartEnd(center, start);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DVec3d EllipseManipulationStrategy::CalculateVec90
(
    DPoint3dCR start,
    DPoint3dCR center,
    DPoint3dCR vec90Point
) const
    {
    DSegment3d startCenter = DSegment3d::From(start, center);
    DPoint3d closestPoint;
    double closestParam;
    startCenter.ProjectPoint(closestPoint, closestParam, vec90Point);

    if (closestPoint.AlmostEqual(vec90Point))
        return DVec3d::From(0, 0, 0);

    return DVec3d::FromStartEnd(closestPoint, vec90Point);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DVec3d EllipseManipulationStrategy::GetVec90() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 3);

    DPoint3d start = GetStart();
    DPoint3d center = GetCenter();
    return CalculateVec90(start, center, keyPoints[s_vec90EndIndex]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DVec3d EllipseManipulationStrategy::GetEndVec() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 4);

    DPoint3d center = keyPoints[s_centerIndex];
    DPoint3d end = keyPoints[s_endIndex];
    return DVec3d::FromStartEnd(center, end);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
double EllipseManipulationStrategy::CalculateSweep
(
    DPoint3dCR start,
    DPoint3dCR center,
    DPoint3dCR end
) const
    {
    DVec3d vec0 = DVec3d::FromStartEnd(center, start);
    DVec3d endVec = DVec3d::FromStartEnd(center, end);
    BeAssert(!DoubleOps::AlmostEqual(m_workingPlane.normal.Magnitude(), 0));
    return vec0.SignedAngleTo(endVec, m_workingPlane.normal);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::UpdateSweep
(
    DPoint3dCR start,
    DPoint3dCR center,
    DPoint3dCR end
)
    {
    if (m_workingPlane.normal.IsZero())
        {
        DVec3d centerToStart = DVec3d::FromStartEnd(center, start);
        DVec3d centerToEnd = DVec3d::FromStartEnd(center, end);
        if (centerToStart.IsZero() || centerToEnd.IsZero())
            return;

        centerToStart.Normalize();
        centerToEnd.Normalize();
        _SetProperty(prop_Normal(), DVec3d::FromCrossProduct(centerToStart, centerToEnd));
        }

    if (m_workingPlane.normal.IsZero())
        return;

    double sweep = CalculateSweep(start, center, end);
    if (DoubleOps::AlmostEqual(m_sweep, 0) || DidSweepDirectionChange(sweep))
        {
        m_sweep = sweep;
        }
    else
        {
        if (m_sweep > 0)
            m_sweep = sweep < 0 ? Angle::TwoPi() + sweep : sweep;
        else if (m_sweep < 0)
            m_sweep = sweep > 0 ? -Angle::TwoPi() + sweep : sweep;
        else
            m_sweep = sweep;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
bool EllipseManipulationStrategy::DidSweepDirectionChange
(
    double newSweep
) const
    {
    if (m_sweep > 0)
        {
        if (newSweep > 0)
            return false;
        else
            {
            double tmpSweep = Angle::TwoPi() + newSweep;
            return fabs(tmpSweep - m_sweep) > SWEEP_BREAK;
            }
        }
    else if (m_sweep < 0)
        {
        if (newSweep < 0)
            return false;
        else
            {
            double tmpSweep = -Angle::TwoPi() + newSweep;
            return fabs(tmpSweep - m_sweep) > SWEEP_BREAK;
            }
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if(GetAcceptedKeyPoints().size() == 2)
        {
        if (DoubleOps::AlmostEqual(CalculateVec90(GetAcceptedKeyPoints()[0], GetAcceptedKeyPoints()[1], newDynamicKeyPoint).Magnitude(), 0))
            {
            m_sweep = m_sweep < 0 ? -Angle::Pi() : Angle::Pi();
            T_Super::_AppendDynamicKeyPoint(newDynamicKeyPoint);
            return;
            }
        }

    T_Super::_AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    if (newDynamicKeyPoints.empty())
        return;

    if (newDynamicKeyPoints.size() == 1)
        {
        _AppendDynamicKeyPoint(newDynamicKeyPoints.front());
        return;
        }

    DPoint3d start, center, vec90Point;
    if (GetAcceptedKeyPoints().size() == 2)
        {
        start = GetAcceptedKeyPoints()[0];
        center = GetAcceptedKeyPoints()[1];
        vec90Point = newDynamicKeyPoints.front();
        }
    else if (GetAcceptedKeyPoints().size() == 1 && newDynamicKeyPoints.size() >= 2)
        {
        start = GetAcceptedKeyPoints()[0];
        center = newDynamicKeyPoints[0];
        vec90Point = newDynamicKeyPoints[1];
        }
    else if (GetAcceptedKeyPoints().empty() && newDynamicKeyPoints.size() >= 3)
        {
        start = newDynamicKeyPoints[0];
        center = newDynamicKeyPoints[1];
        vec90Point = newDynamicKeyPoints[2];
        }

    if (GetAcceptedKeyPoints().size() + newDynamicKeyPoints.size() >= 3)
        {
        if (DoubleOps::AlmostEqual(CalculateVec90(start, center, vec90Point).Magnitude(), 0))
            {
            m_sweep = m_sweep < 0 ? -Angle::Pi() : Angle::Pi();
            T_Super::_AppendDynamicKeyPoints(newDynamicKeyPoints);
            return;
            }
        }

    T_Super::_AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_OnKeyPointsChanged()
    {
    if (GetKeyPoints().size() == 4)
        {
        UpdateSweep(GetStart(), GetCenter(), GetKeyPoints()[s_endIndex]);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_SetProperty
(
    Utf8CP key,
    DVec3d const& value
)
    {
    T_Super::_SetProperty(key, value);

    if (0 == strcmp(prop_Normal(), key))
        {
        DVec3d tmpVec = value;
        if (!DoubleOps::AlmostEqual(tmpVec.Normalize(), 0))
            m_workingPlane.normal = tmpVec;
        else
            m_workingPlane.normal.Zero();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                09/2018
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::_SetProperty
(
    Utf8CP key,
    DPlane3d const& value
)
    {
    T_Super::_SetProperty(key, value);

    if (0 == strcmp(prop_WorkingPlane(), key))
        {
        m_workingPlane = value;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus EllipseManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    DVec3d& value
) const
    {
    if (0 == strcmp(prop_Normal(), key))
        {
        if (!DoubleOps::AlmostEqual(m_workingPlane.normal.Magnitude(), 0))
            {
            value = m_workingPlane.normal;
            return BentleyStatus::SUCCESS;
            }
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                09/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus EllipseManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    DPlane3d& value
) const
    {
    if (0 == strcmp(prop_WorkingPlane(), key))
        {
        value = m_workingPlane;
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool EllipseManipulationStrategy::_IsComplete() const
    {
    return GetAcceptedKeyPoints().size() == 3;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool EllipseManipulationStrategy::_CanAcceptMorePoints() const
    {
    return !_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr EllipseManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<IGeometryPtr> EllipseManipulationStrategy::_FinishConstructionGeometry() const
    {
    return bvector<IGeometryPtr>();
    }