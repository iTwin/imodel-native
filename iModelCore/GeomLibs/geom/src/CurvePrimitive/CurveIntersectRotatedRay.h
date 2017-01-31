/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurveIntersectRotatedRay.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


// Vastly overestimated intersection limits
#define BezierIntersectLimit_22  10

// Make a QUADRATIC bezier for which:
// x component is the z component of the segment.
// y component is the sum of segment x component squared and segment y component squared
// 
void SegmentToZRRBezier (DSegment3dCR segment, DPoint4d bezier[3])
    {
    // x component of bezier is z component of segment, inflated to quadratic ...
    bezier[0].x =  segment.point[0].z;
    bezier[0].x = (segment.point[0].z + segment.point[1].z) * 0.5;
    bezier[0].x =  segment.point[1].z;
    
    // y component of bezier is x^2+y^2 for segment ....
    bezier[0].y = segment.point[0].x * segment.point[0].x
                + segment.point[0].y * segment.point[0].y;

    bezier[1].y = segment.point[0].x * segment.point[1].x;

    bezier[2].y = segment.point[1].x * segment.point[1].x
                + segment.point[1].y * segment.point[1].y;

    bezier[0].z = bezier[1].z = bezier[2].z = 0.0;
    bezier[0].w = bezier[1].w = bezier[2].w = 0.0;
    }

struct RotatedRayIntersectionProcessor: public ICurvePrimitiveProcessor
{
bvector<CurveLocationDetail> m_intersectionA;
bvector<CurveLocationDetail> m_intersectionB;
Transform m_worldToLocal;
DRay3d    m_worldRay;
DRay3d    m_localRay;

RotatedRayIntersectionProcessor (DRay3dCR worldRay, TransformCR worldToLocal) :
    m_worldToLocal (worldToLocal)
    {
    m_localRay.InitProduct (worldToLocal, worldRay);
    }


GEOMAPI_VIRTUAL RotatedRayIntersectionProcessor::~RotatedRayIntersectionProcessor ()
    {
    }
void _ProcessLine (ICurvePrimitiveCR curve, DSegment3dCR segment, DSegment1dCP interval) override
    {
    DSegment3d localSegment;
    DSegment3d segmentOfClippedRay;
    localSegment.InitProduct (m_worldToLocal, segment);
    DRange3d segmentRange;
    segmentRange.Init ();
    segmentRange.Extend (localSegment.point[0]);
    segmentRange.Extend (localSegment.point[1]);
    DRange1d rayFractionRange;
    if (!ClipRayToRange (m_localRay, segmentRange, segmentOfClippedRay,rayFractionRange))
        return;

    // Turn the xyz segments into quadratic beziers with
    //    bezier.x is line z == linear, but artificially inflated to quadratic
    //    bezier.y is line x^2 + y^2 == quadratic
    DPoint4d bezierA[3];
    DPoint4d bezierB[3];
    SegmentToZRRBezier (localSegment, bezierA);
    SegmentToZRRBezier (segmentOfClippedRay, bezierB);

    DPoint4d intersectA[BezierIntersectLimit_22], intersectB[BezierIntersectLimit_22];
    double   paramA[BezierIntersectLimit_22], paramB[BezierIntersectLimit_22];
    int numIntersection, numExtra;
    if (bsiBezierDPoint4d_intersectXY_chordal
            (
            intersectA, paramA,
            intersectB, paramB,
            &numIntersection, &numExtra, BezierIntersectLimit_22,
            bezierA, 3, bezierB, 3))
        {
        // Don't say wow yet.  Still have to map the bezier intersections back to xyz
        // bezierA is parameters are exactly fractoins of the global segment.
        // bezierB is a fraction of the clipped ray.
        for (int i = 0; i < numIntersection; i++)
            {
            double rayFraction, segmentFraction = paramA[i];
            DPoint3d segmentXYZ;
            segment.FractionParameterToPoint (segmentXYZ, segmentFraction);
            rayFractionRange.FractionToDouble (paramB[i], rayFraction, 0.0);
            DPoint3d rayXYZ = m_worldRay.FractionParameterToPoint (rayFraction);
            m_intersectionA.push_back (CurveLocationDetail (
                    &curve, segmentFraction, segmentXYZ));
            // local xyz coordinates of the intersection points on the ray and the segment.
            DPoint3d localRayXYZ, localSegmentXYZ;
            segmentOfClippedRay.FractionParameterToPoint (localRayXYZ, rayFraction);
            localSegment.FractionParameterToPoint (localSegmentXYZ, segmentFraction);
            double theta = localSegmentXYZ.AngleToXY (localRayXYZ);
            m_intersectionA.back ().SetDistance (theta);
            m_intersectionB.push_back (CurveLocationDetail (
                    NULL, rayFraction, rayXYZ));
            // Now say WOW !!!
            }
        }
    }

void _ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval) override
    {
    }

void _ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override
    {
    }

void _ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    }

void _ProcessProxyBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    }
};

