/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ArcManipulationStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ArcManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.size() < 2)
        return nullptr;

    DPoint3d start = keyPoints[m_startIndex];
    DPoint3d center = keyPoints[m_centerIndex];
    if (keyPoints.size() == 2)
        return ICurvePrimitive::CreateArc(DEllipse3d::FromCenterRadiusXY(center, center.Distance(start)));

    DSegment3d startCenter = DSegment3d::From(start, center);
    DVec3d vec0 = DVec3d::FromStartEnd(center, start);
    DPoint3d closestPoint;
    double closestParam;
    startCenter.ProjectPoint(closestPoint, closestParam, keyPoints[m_middleIndex]);
    DVec3d vec90 = DVec3d::FromStartEnd(closestPoint, keyPoints[m_middleIndex]);
    if (keyPoints.size() == 3)
        return ICurvePrimitive::CreateArc(
            DEllipse3d::FromVectors(center, vec0, vec90, 0, Angle::TwoPi()));

    DPoint3d end = keyPoints[m_endIndex];
    DVec3d vecEnd = DVec3d::FromStartEnd(center, end);
    double sweep = vec0.AngleTo(vecEnd);
    if (keyPoints.size() == 4)
        return ICurvePrimitive::CreateArc(
            DEllipse3d::FromVectors(center, vec0, vec90, 0, sweep));

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void ArcManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if ((IsDynamicKeyPointSet() && keyPoints.size() <= 4) ||
        (!IsDynamicKeyPointSet() && keyPoints.size() < 4))
        T_Super::_AppendKeyPoint(newKeyPoint);
    }