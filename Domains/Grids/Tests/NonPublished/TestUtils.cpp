/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestUtils.h"
#include <Bentley\BeTest.h>
//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void TestUtils::CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs)
    {
    ASSERT_TRUE(lhs.IsValid() && rhs.IsValid()) << "Both curves should be valid";
    ASSERT_TRUE(lhs->IsSameStructureAndGeometry(*rhs)) << "Curves should have same geometry";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void TestUtils::ComparePoints(bvector<DPoint3d> lhs, bvector<DPoint3d> rhs)
    {
    ASSERT_EQ(lhs.size(), rhs.size()) << "There should be an equal amount of points";
    for (size_t i = 0; i < lhs.size(); ++i)
        {
        ASSERT_TRUE(lhs[i].AlmostEqual(rhs[i])) << "Points should be almost equal";
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr TestUtils::CreateSpline(bvector<DPoint3d> poles, int order)
    {
    bvector<double> weights;
    for (DPoint3d point : poles)
        weights.push_back(1.0);

    order = poles.size() < order ? poles.size() : order; // order can't be higher than poles size

    bvector<double> knots;
    for (int i = 0; i < order + poles.size(); ++i)
        knots.push_back(i);

    MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder(poles, &weights, &knots, order, false, false);
    return ICurvePrimitive::CreateBsplineCurve(bspline);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr TestUtils::CreateInterpolationCurve(bvector<DPoint3d> poles, DVec3d startTangent, DVec3d endTangent)
    {
    MSInterpolationCurvePtr curve = MSInterpolationCurve::CreatePtr();
    if (curve.IsNull())
        return nullptr;

    DVec3d tangents[2] = { startTangent, endTangent };

    for (DVec3d & tangent : tangents)
        {
        if (!tangent.IsZero())
            tangent.Normalize();
        }

    if (SUCCESS != curve->InitFromPointsAndEndTangents(poles, false, 0.0, tangents, false, false, false, false))
        return nullptr;

    return ICurvePrimitive::CreateInterpolationCurveSwapFromSource(*curve);
    }