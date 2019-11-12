/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley\Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "msbsplinemaster.h"

static bool AdvanceBezierPair
(
MSBsplineCurveCR curveA,
BCurveSegmentR segmentA,
size_t         &indexA,
MSBsplineCurveCR curveB,
BCurveSegmentR segmentB,
size_t         &indexB,
bool saturate
)
    {
    if (indexA != indexB)
        return false;
    if (!curveA.AdvanceToBezier (segmentA, indexA, saturate)
        || !curveB.AdvanceToBezier (segmentB, indexB, saturate)
        )
        return false;
    if (indexA != indexB)
        return false;
    DRange1d knotRangeA = segmentA.KnotRange ();
    DRange1d knotRangeB = segmentB.KnotRange ();

    if (!MSBsplineCurve::AreSameKnots (knotRangeA.Low (),  knotRangeB.Low ()))
        return false;
    if (!MSBsplineCurve::AreSameKnots (knotRangeA.High (), knotRangeB.High ()))
        return false;

    return true;
    }

bool MSBsplineCurve::AddRuleSurfaceRayIntersections (
bvector<SolidLocationDetail> &pickData,
MSBsplineCurveCR curveA,
MSBsplineCurveCR curveB,
DRay3dCR ray
)
    {
    if (!MSBsplineCurve::AreCompatible (curveA, curveB))
        return false;

    double curveKnotLength = curveA.FractionToKnot (1.0) - curveA.FractionToKnot (0.0);
    BCurveSegment segmentA, segmentB;
    for (size_t iA = 0, iB = 0;
            AdvanceBezierPair (curveA, segmentA, iA, curveB, segmentB, iB, true)
            ;)
        {
        size_t pickIndex0 = pickData.size ();
        if (bsiBezier_addRayRuledSurfaceIntersections
            (pickData, ray,
                segmentA.GetPoleP (),
                segmentB.GetPoleP (),
                (int)segmentA.GetOrder ()))
            {
            // remap bezier fraction to knot fraction.
            // scale tangent vector to knot fraction.
            double bezierKnotLength = segmentA.KnotRange ().Length ();
            double uScale = curveKnotLength / bezierKnotLength;
            for (size_t i = pickIndex0; i < pickData.size ();)
                {
                DPoint2d uv = pickData[i].GetUV ();
                if (DoubleOps::IsIn01 (uv.y)
                    || MSBsplineCurve::AreSameKnots (uv.y, 0.0)
                    || MSBsplineCurve::AreSameKnots (uv.y, 1.0)
                    )
                    {
                    DVec3d tangent = pickData[i].GetUDirection ();
                    double knot = segmentA.FractionToKnot (uv.x);
                    pickData[i].SetU (curveA.KnotToFraction (knot));
                    tangent.Scale (uScale);
                    pickData[i].SetUDirection (tangent);
                    i++;
                    }
                else
                    {
                    pickData.erase (pickData.begin () + i, pickData.begin () + i + 1);
                    // don't increment i after delete!!!
                    }
                }
            }
        }
    return true;
    }

bool MSBsplineCurve::RuledSurfaceClosestPoint (
            SolidLocationDetail &pickData,
            MSBsplineCurveCR curveA,
            MSBsplineCurveCR curveB,
            DPoint3dCR spacePoint
)
    {
    if (!MSBsplineCurve::AreCompatible (curveA, curveB))
        return false;
    SolidLocationDetail pickData1;
    pickData.Init ();
    pickData.SetPickParameter (DBL_MAX);
    //double curveKnotLength = curveA.FractionToKnot (1.0) - curveA.FractionToKnot (0.0);
    BCurveSegment segmentA, segmentB;
    for (size_t iA = 0, iB = 0;
            AdvanceBezierPair (curveA, segmentA, iA, curveB, segmentB, iB, true)
            ;)
        {
        if (bsiBezier_ruledPatchClosestPoint
             (
             pickData1, spacePoint,
             segmentA.GetPoleP (),
             segmentB.GetPoleP (),
             (int)segmentA.GetOrder ())
             )
            {
            if (pickData1.GetPickParameter () < pickData.GetPickParameter ())
                {
                pickData = pickData1;
                pickData.SetU (segmentA.FractionToKnot (pickData1.GetU()));
                }
            }
        }
    return pickData.GetPickParameter () < DBL_MAX;
    }


END_BENTLEY_GEOMETRY_NAMESPACE