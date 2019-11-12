/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CurveCurveProcessor.h"


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// PartialCurveBezier contains an xyzw bezier curve with mapping (ParticalCurveDetail) to a containing curve.
struct PartialCurveBezier
{
PartialCurveDetail m_parent;    // points to parent curve, with start and end parameters in parent curve space
bool m_hasWeights;              // true if there are non-unit weight in m_xyzw;
bvector<DPoint4d> m_xyzw;
bvector<DPoint4d> m_RSquaredZ;
bvector<DPoint3d> m_trigPoles;    // for arcs: bezier-to-(w*cosine, w*sine, w)
DSegment1d m_angleRange;         // for arcs: angle range for trigPole inverse values.

void Clear ()
    {
    m_xyzw.clear ();
    m_trigPoles.clear ();
    }
void SetSegment (DSegment3d segment)
    {
    Clear ();
    m_xyzw.push_back (DPoint4d::From (segment.point[0], 1.0));
    m_xyzw.push_back (DPoint4d::From (segment.point[1], 1.0));
    m_hasWeights = false;
    }
// Define a bezier with aux mapping
void SetAuxBezier (DPoint3dCP csw, int order, DSegment1d angleRange)
    {
    m_angleRange = angleRange;
    for (int i = 0; i < order; i++)
        {
        m_trigPoles.push_back (csw[i]);
        }
    }
// Define a bezier
void SetBezier (DPoint4dCP xyzw, int order)
    {
    Clear ();
    DRange1d wRange;
    for (int i = 0; i < order; i++)
        {
        m_xyzw.push_back (xyzw[i]);
        wRange.Extend (xyzw[i].w);
        }
    m_hasWeights = !wRange.IsSinglePoint (1.0);
    }

PartialCurveBezier (PartialCurveDetailCR parent) : m_parent (parent), m_hasWeights(false), m_angleRange (0,0) {}

void Multiply (TransformCR transform)
    {
    transform.Multiply (m_xyzw, m_xyzw);
    }

void FormRotationalRSquaredAndZ ()
    {
    int termOrder = (int)m_xyzw.size ();
    m_RSquaredZ.clear ();
    if (termOrder == 0)
        return;
    // X*X has degree 2*X.degree, order 2*X.order - 1
    int productOrder = 2 * termOrder - 1;
    m_RSquaredZ.reserve (2 * m_xyzw.size () - 1);
    DPoint4d zero = DPoint4d::From (0,0,0,0);
    for (int i = 0; i < productOrder; i++)
        m_RSquaredZ.push_back (zero);
    double *dest = (double*)&m_RSquaredZ[0];
    double *source = (double*)&m_xyzw[0];
    // RSquaredZ x component is RSquared == X*X + Y*Y
    // .. add X*X to output "X" term
    bsiBezier_accumulateUnivariateProduct (
        dest, 0, 4,
        1.0,
        source, termOrder, 0, 4,
        source, termOrder, 0, 4
        );
    // .. add Y*Y to output "X"
    bsiBezier_accumulateUnivariateProduct (
        dest, 0, 4,
        1.0,
        source, termOrder, 1, 4,
        source, termOrder, 1, 4
        );
    // .. add Z*Z as output "Y"
    bsiBezier_accumulateUnivariateProduct (
        dest, 1, 4,
        1.0,
        source, termOrder, 2, 4,
        source, termOrder, 2, 4
        );
    // .. WW as output "W"
    bsiBezier_accumulateUnivariateProduct (
        dest, 3, 4,
        1.0,
        source, termOrder, 3, 4,
        source, termOrder, 3, 4
        );
    }

void ResolveFraction (TransformCR worldToLocal, double f, CurveLocationDetail &detail, DPoint3dR uvw)
    {
    if (m_trigPoles.size () > 0)
        {
        auto radians = bsiTrig_quadricTrigPointBezierFractionToAngle (&m_trigPoles[0], f);
        radians = Angle::AdjustToSweep (radians, m_angleRange.GetStart (), m_angleRange.Delta ());
        // Do we care about how extrapolations are handled?  I don't think so -- the bezier is a bounded curve, so
        // any intersection will be interior.
        m_angleRange.PointToFraction (radians, f);
        }
    double g = m_parent.ChildFractionToParentFraction (f);
    m_parent.parentCurve->FractionToPoint (g, detail);
    worldToLocal.Multiply (uvw, detail.point);
    detail.a = atan2 (uvw.y, uvw.x);
    }
// retutrn true if dataA[i],dataB[i] duplicates some earlier pair.
static bool IsDuplicateOfEarlierPair
(
CurveLocationDetailCR newA,
CurveLocationDetailCR newB,
bvector<CurveLocationDetail> const &dataA,
bvector<CurveLocationDetail> const &dataB
)
    {
    for (size_t i = 0; i < dataA.size (); i++)
        {
        if (   newA.curve == dataA[i].curve
            && newB.curve == dataB[i].curve
            && DoubleOps::AlmostEqualFraction (newA.fraction, dataA[i].fraction)
            && DoubleOps::AlmostEqualFraction (newB.fraction, dataB[i].fraction)
            )
            return true;
        }
    return false;
    }
// Compute the intersection of the beziers in RSquaredZ space.
// return points on original curves, with local coordinates theta in "a" part of each curve location detail.
static void AppendRSquaredZIntersections
(
TransformCR worldToLocal, 
PartialCurveBezier &curveA,
PartialCurveBezier &curveB,
bvector<CurveLocationDetail> &detailA,
bvector<CurveLocationDetail> &detailB
)
    {
    if (curveA.m_RSquaredZ.size () < 2
        || curveB.m_RSquaredZ.size () < 2
        )
        return;
    DPoint4d *dataA = &curveA.m_RSquaredZ[0];
    DPoint4d *dataB = &curveB.m_RSquaredZ[0];
    int orderA = (int)curveA.m_RSquaredZ.size ();
    int orderB = (int)curveB.m_RSquaredZ.size ();
#define MaxIntersectionsAllowed 30
    int numIntersection, numExtra;
    double paramA[MaxIntersectionsAllowed], paramB[MaxIntersectionsAllowed];
    DPoint4d xyzwA[MaxIntersectionsAllowed], xyzwB[MaxIntersectionsAllowed];
    bsiBezierDPoint4d_intersectXY_chordal (xyzwA, paramA, xyzwB, paramB,
                &numIntersection, &numExtra, MaxIntersectionsAllowed,
                dataA, orderA,
                dataB, orderB
        );
    for (size_t i = 0; i < (size_t)numIntersection; i++)
        {


        // This resolves bezier fraction to curve fractionn and original curve xyz (in space, not in the frame of rotateAroundZ)
        CurveLocationDetail resultA, resultB;
        DPoint3d uvwA, uvwB;
        curveA.ResolveFraction (worldToLocal, paramA[i], resultA, uvwA);
        curveB.ResolveFraction (worldToLocal, paramB[i], resultB, uvwB);
        if (IsDuplicateOfEarlierPair (resultA, resultB, detailA, detailB))
            continue;
        // Reject if local w does not match (artificial root due to squaring z)
        if (DoubleOps::AlmostEqual (uvwA.z, uvwB.z))
            {
            detailA.push_back (resultA);
            detailB.push_back (resultB);
            }
        }
    }
void ExtendRangeByPoles (DRange3dR range)
    {
    range.Extend (m_xyzw);
    }
};

struct BezierReductionVector : bvector<PartialCurveBezier>
{
void Add (PartialCurveDetailCR parent, DSegment3d segment)
    {
    push_back (PartialCurveBezier (parent));
    back ().SetSegment (segment);
    }

void Add (PartialCurveDetailCR parent, BCurveSegmentR segment)
    {
    int order = (int)segment.GetOrder ();
    if (order > 1)
        {
        push_back (PartialCurveBezier (parent));
        back ().SetBezier (segment.GetPoleP (), order);
        }
    }



void Add (PartialCurveDetailCR parent, DEllipse3d arc)
    {
    DPoint4d qPoles[10];     // poles in cone local space
    DPoint3d stPoles[10];    // poles in plane of arc
    double stAngle[10];
    int numSpan, numPoles;
    arc.QuadricBezierPoles (qPoles, stPoles, stAngle, &numPoles, &numSpan, 9);
    double spanStep = 1.0 / (double)numSpan;
    for (int i = 0; i < numSpan; i++)
        {
        double f0 = i * spanStep;
        double f1 = (i+1) * spanStep;
        push_back (PartialCurveBezier (PartialCurveDetail (parent, f0, f1)));
        back ().SetBezier (qPoles  + 2 * i, 3);
        back ().SetAuxBezier (stPoles + 2 * i, 3, DSegment1d (stAngle[i], stAngle[i+1]));
        }        
    }

void Multiply (TransformCR transform)
    {
    for (size_t i = 0; i < size (); i++)
        at(i).Multiply  (transform);

    }

void FormRotationalRSquaredAndZ ()
    {
    for (size_t i = 0; i < size (); i++)
        at(i).FormRotationalRSquaredAndZ ();
    }

void ExtendRangeByPoles (DRange3dR range)
    {
    for (size_t i = 0; i < size (); i++)
        at(i).ExtendRangeByPoles (range);
    }
};

// Collector to build a BezierReductionVector with images of all curves passing through ICurvePrimitiveProcessor.
struct BezierReductionCollector : ICurvePrimitiveProcessor
{
BezierReductionVector &m_curves;
BezierReductionCollector (BezierReductionVector &curves) : m_curves(curves)
    {
    }

void _ProcessLine (ICurvePrimitiveCR curve, DSegment3dCR segment, DSegment1dCP interval) override
    {
    if (interval == nullptr)
        {
        m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve), 0.0, 1.0, 0), segment);
        }
    else
        {
        DSegment3d partialSegment = DSegment3d::FromFractionInterval (segment, *interval);
        m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve), *interval), partialSegment);
        }
    }

void _ProcessArc (ICurvePrimitiveCR curve, DEllipse3dCR arc, DSegment1dCP interval) override
    {
    if (interval == nullptr)
        {
        m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve), 0.0, 1.0, 0), arc);
        }
    else
        {
        DEllipse3d partialArc = DEllipse3d::FromFractionInterval (arc, interval->GetStart (), interval->GetEnd ());
        m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve), *interval), partialArc);
        }
    }

void _ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override
    {
    if (points.size () < 2)
        {
        // no curves !!!
        }
    else if (interval == nullptr)
        {
        double step = 1.0 / (points.size () - 1.0);
        for (size_t i = 0; i + 1 < points.size (); i++)
            {
            double f0 = i * step;
            double f1 = (i+1) * step;
            DSegment3d segment = DSegment3d::From (points[i], points[i+1]);
            m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve), f0, f1, 0), segment);
            }
        }
    else
        {
        }
    }


void _ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    if (interval == nullptr)
        {
        BCurveSegment segment;
//        static double s_bezierFractionTol = 1.0e-10;
        for (size_t i = 0; bcurve.AdvanceToBezier (segment, i);)
            {
            m_curves.Add (PartialCurveDetail (const_cast <ICurvePrimitiveP>(&curve),
                            bcurve.KnotToFraction (segment.UMin ()),
                            bcurve.KnotToFraction (segment.UMax ()),
                            0),
                    segment
                    );
            }
         }
    else
        {
        // should be easy ... convert the interval to a knot range.  Use AdvanceToBezierInKnotRange.
        // Can intervals go backward? that would be require a nasty second copy.
        }
    }




};

void CurveCurve::IntersectRotatedCurveSpaceCurve
(
TransformCR worldToLocal,
CurveVectorCR rotatedCurve,
ICurvePrimitiveCR spaceCurve,
bvector<CurveLocationDetail> &detailA,
bvector<CurveLocationDetail> &detailB
    )
    {
    BezierReductionVector curveA, curveB;
    BezierReductionCollector collectorA (curveA);
    BezierReductionCollector collectorB (curveB);

    detailA.clear ();
    detailB.clear ();

    for (auto &curve : rotatedCurve)
        curve->Process (collectorA);
    spaceCurve.Process (collectorB);
    curveA.Multiply (worldToLocal);
    curveB.Multiply (worldToLocal);
    // If the curves cross the z=0 plane, numerics get bad in the RSquaredZ mapping.
    // Translate along the Z axis to fix ....
    DRange3d range;
    range.Init ();
    curveA.ExtendRangeByPoles (range);
    curveB.ExtendRangeByPoles (range);
    double zBase = range.low.z;
    double dz = range.high.z - range.low.z;
    zBase = range.low.z - dz;
    Transform shiftZ = Transform::From (0,0, -zBase);
    //shiftZB = Transform::FromTranslation (0,0, zBase);
    curveA.Multiply (shiftZ);
    curveB.Multiply (shiftZ);
    curveA.FormRotationalRSquaredAndZ ();
    curveB.FormRotationalRSquaredAndZ ();

    for (size_t ia = 0; ia < curveA.size (); ia++)
        {
        for (size_t ib = 0; ib < curveB.size (); ib++)
            {
            PartialCurveBezier::AppendRSquaredZIntersections (worldToLocal, curveA[ia], curveB[ib], detailA, detailB);
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
