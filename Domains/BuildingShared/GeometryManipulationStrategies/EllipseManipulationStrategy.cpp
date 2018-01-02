/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/EllipseManipulationStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        return ICurvePrimitive::CreateArc(DEllipse3d::FromCenterRadiusXY(center, center.Distance(start)));

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

    if (GetAcceptedKeyPoints().size() == 4)
        UpdateSweep(GetAcceptedKeyPoints().back());
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
    DPoint3dCR endPoint
) const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 3);
    
    DVec3d vec0 = GetVec0();
    DPoint3d center = GetCenter();
    DVec3d endVec = DVec3d::FromStartEnd(center, endPoint);
    BeAssert(!DoubleOps::AlmostEqual(m_orientation.Magnitude(), 0));
    return vec0.SignedAngleTo(endVec, m_orientation);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::UpdateSweep
(
    DPoint3dCR endPoint
)
    {
    if (m_orientation.IsZero())
        {
        m_orientation = DVec3d::FromCrossProduct(GetVec0(), GetVec90());
        }

    double sweep = CalculateSweep(endPoint);
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
    bvector<DPoint3d> const& currentKeyPoints = GetKeyPoints();
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

    if (GetKeyPoints().size() == 4)
        UpdateSweep(GetKeyPoints().back());
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

    bvector<DPoint3d> const& currentKeyPoints = GetKeyPoints();
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

    if (GetKeyPoints().size() == 4)
        UpdateSweep(GetKeyPoints().back());
    }