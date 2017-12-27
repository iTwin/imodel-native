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
    if ((IsDynamicKeyPointSet() && keyPoints.size() <= 4) ||
        (!IsDynamicKeyPointSet() && keyPoints.size() < 4))
        {
        if (keyPoints.size() >= 3)
            UpdateSweep(newKeyPoint);
        T_Super::_AppendKeyPoint(newKeyPoint);
        }
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
DVec3d EllipseManipulationStrategy::GetVec90() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    BeAssert(keyPoints.size() >= 3);

    DPoint3d start = GetStart();
    DPoint3d center = GetCenter();

    DSegment3d startCenter = DSegment3d::From(start, center);
    DPoint3d closestPoint;
    double closestParam;
    startCenter.ProjectPoint(closestPoint, closestParam, keyPoints[s_middleIndex]);
    return DVec3d::FromStartEnd(closestPoint, keyPoints[s_middleIndex]);
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
    DVec3d orientation;
    return vec0.SignedAngleTo(endVec, orientation);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void EllipseManipulationStrategy::UpdateSweep
(
    DPoint3dCR endPoint
)
    {
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
    if (currentKeyPoints.size() >= 3)
        UpdateSweep(newDynamicKeyPoint);

    T_Super::_AppendDynamicKeyPoint(newDynamicKeyPoint);
    }