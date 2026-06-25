/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <map>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct CurveExtendFlags
    {
    bool m_extend0;
    bool m_extend1;
    CurveExtendFlags(bool extend = false) :m_extend0(extend), m_extend1(extend) {}
    CurveExtendFlags(bool extend0, bool extend1) : m_extend0(extend0), m_extend1(extend1) {}
    bool HasAnyExtend () const {return m_extend0 || m_extend1;}
    };

// Base class for computations involving all combinations of 2 curve types.
// Base class has stub implementations of
//<ul>
//<li>For all combinations NOT involving a bspline:
//     ProcessAB (curveA, dataA, curveB, dataB, bReverseOrder)
//<li>For all combinations involving a bspline:
//     ProcessAB (curveA, curveB, bReverseOrder)
//</ul>
//
// The stub implementations invoke the ProcessPrimitivePrimitive method.  This does nothing but increment
//     a counter, and may be useful as a breakpoint to look for methods lacking derived class implementation.
//
struct CurveCurveProcessor
{
GEOMAPI_VIRTUAL ~CurveCurveProcessor (){}

protected:
    //! Number of calls to base class stubs
    size_t m_numProcessedByBaseClass;
    //! placement transform
    DMatrix4dCP m_pWorldToLocal;
    //! control flags for extended geometry
    CurveExtendFlags m_extendA;
    CurveExtendFlags m_extendB;
    //! Options for on-demand stroking.
    IFacetOptionsPtr m_strokeOptions;

    IFacetOptionsP GetStrokeOptions ()
        {
        if (NULL == m_strokeOptions.get ())
            {
            m_strokeOptions = IFacetOptions::CreateForCurves ();
            m_strokeOptions->SetAngleTolerance (0.1);
            }
        return m_strokeOptions.get ();
        }
    void SetStrokeOptions(IFacetOptionsCR strokeOptions)
        {
        m_strokeOptions = strokeOptions.Clone();
        }
    CurveExtendFlags GetExtendFlag(uint32_t index, bool reversed)
        {
        if (reversed)
            return index == 0 ? m_extendB : m_extendA;
        else
            return index == 0 ? m_extendA : m_extendB;
        }
    static bool IsLinear (ICurvePrimitiveCP curve)
        {
        ICurvePrimitive::CurvePrimitiveType type = curve->GetCurvePrimitiveType ();
        if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
            return true;
        if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
            return true;
        if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve)
            {
            // NEEDSWORK: check for order==2 AND uniform knots AND unit weights
            }
        return false;
        }

//! processor called by default implementations.
//! Implementors can trap here to apply generic curve logic in absence of special cases.
//! Base class increments counters.
GEOMAPI_VIRTUAL void ProcessPrimitivePrimitive (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder);

explicit CurveCurveProcessor (DMatrix4dCP pWorldToLocal);

bool ValidFraction (CurveLocationDetailCR detail, double fraction, CurveExtendFlags extend)
    {
    bool extensible0 = extend.m_extend0;
    bool extensible1 = extend.m_extend1;
    if (detail.componentIndex != 0)
        extensible0 = false;
    if (detail.componentIndex + 1 != detail.numComponent)
        extensible1 = false;
    if (fraction < 0.0 && !extensible0)
        return false;
    if (fraction > 1.0 && !extensible1)
        return false;
    return true;
    }

// Return true if the fractional position within the edge is internal to the linestring.
//    (false for extensions of internal edges)
bool ValidEdgeFractionWithinLinestring (double f, size_t edgeIndex, size_t numPoint, CurveExtendFlags extend)
    {
    static double s_lineFractionTol = 1.0e-8;
    // interior fractions are always ok ...
    if (f >= -s_lineFractionTol && f <= 1.0 + s_lineFractionTol)
        return true;

    // We are outside the immediate edge...
    if (extend.m_extend0 || extend.m_extend1)
        {
        if (numPoint < 2)
            return true;
        if (edgeIndex <= 0 && extend.m_extend0 && f <= 1.0)
            return true;
        else if (edgeIndex == numPoint - 2 && extend.m_extend1 && f >= 0.0)
            return true;
        }

    return false;
    }

// Given arc angle, return arc fraction shifted according to extend flags.
double ArcAngleToShiftedFraction(DEllipse3dCR ellipse, double radians, CurveExtendFlags const& extend)
    {
    if (extend.HasAnyExtend())
        return Angle::NormalizeToSweep(radians, ellipse.start, ellipse.sweep, extend.m_extend0, extend.m_extend1);
    return ellipse.AngleToFraction(radians);
    }

// Given arc fraction, return arc fraction shifted according to extend flags.
double ArcFractionToShiftedFraction(DEllipse3dCR ellipse, double fraction, CurveExtendFlags const& extend)
    {
    if (extend.HasAnyExtend())
        return ArcAngleToShiftedFraction(ellipse, ellipse.FractionToAngle(fraction), extend);
    return fraction;
    }

// Return true if the angle is internal to the (possibly extended) arc.
// In particular, return true if *either* extend flag is set.
bool ValidArcAngle (DEllipse3d const &ellipse, double theta, CurveExtendFlags extend)
    {
    if (extend.HasAnyExtend ())
        return true;
    if (ellipse.IsAngleInSweep (theta))
        return true;
    return false;
    }

// blanket extend setting ...
void SetExtend (bool b) {m_extendA.m_extend0 = m_extendA.m_extend1 = m_extendB.m_extend0 = m_extendB.m_extend1 = b;}
bool HasAnyExtend () const { return m_extendA.HasAnyExtend() || m_extendB.HasAnyExtend(); }
void SetExtend(bool extendA0, bool extendA1, bool extendB0, bool extendB1)
    {
    m_extendA.m_extend0 = extendA0;
    m_extendA.m_extend1 = extendA1;
    m_extendB.m_extend0 = extendB0;
    m_extendB.m_extend1 = extendB1;
    }

public:
GEOMAPI_VIRTUAL void ProcessLineLine (
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB,
        bool bReverseOrder);

GEOMAPI_VIRTUAL void ProcessLineLinestring (
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder);

GEOMAPI_VIRTUAL void ProcessLineArc (
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder);

GEOMAPI_VIRTUAL void ProcessLinestringLinestring (
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder);

GEOMAPI_VIRTUAL void ProcessLinestringArc (
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder);

GEOMAPI_VIRTUAL void ProcessArcArc (
        ICurvePrimitiveP curveA, DEllipse3dCR ellipseA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder);

// Only pass the curve pointer for bsplines ...
GEOMAPI_VIRTUAL void ProcessLineBspline (ICurvePrimitiveP curveA, DSegment3dCR segmentA, ICurvePrimitiveP curveB, bool bReverseOrder);
GEOMAPI_VIRTUAL void ProcessArcBspline (ICurvePrimitiveP curveA, DEllipse3dCR ellipseA, ICurvePrimitiveP curveB, bool bReverseOrder);
GEOMAPI_VIRTUAL void ProcessLinestringBspline (ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
            ICurvePrimitiveP curveB, bool bReverseOrder);
GEOMAPI_VIRTUAL void ProcessBsplineBspline (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder);

void Process (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB);

void Process (CurveVectorCP curvesA, ICurvePrimitiveP curveB);
// Apply the worldToLocal transform to a point with weight
void TransformWeightedPoint (DPoint4d &hPoint, DPoint3d const &cPoint, double w);

// Apply the worldToLocal transform to a segment.
void TransformSegment (DSegment4dR hSeg, DSegment3dCR cSeg);

// Apply the worldToLocal transform to an ellipse
void TransformEllipse (DConic4d &hConic, DEllipse3d const &cEllipse);
};

// Comparator for sorting segments by their points
struct CompareDSegment3d
    {
    // absolute and relative tolerance for coordinate comparisons
    double m_coordTol;

    CompareDSegment3d(double coordTol) : m_coordTol (coordTol) {}

    // lexicographical point comparison: x first, then y, then z
    int compareXYZ(DPoint3dCR p0, DPoint3dCR p1) const;

    // lexicographical segment "less" operator: compare first points of each segment, then the second points
    bool operator()(DSegment3dCR a, DSegment3dCR b) const;
    };

// CurveCurveProcessor to capture 3D close approach results.
struct CurveCurveProcessAndCollectCloseApproaches : public CurveCurveProcessor
{
typedef std::multimap<DSegment3d, CurveLocationDetailPair, CompareDSegment3d> SegmentPairMultiMap;

static inline auto s_segmentPairLess = [](SegmentPairMultiMap::value_type const& entry0, SegmentPairMultiMap::value_type const& entry1) -> bool { return entry0.second.detailA.a < entry1.second.detailA.a; };

double m_maxDistance; // if negative, collect only closest approach
SegmentPairMultiMap m_pairs; // stores close approach equivalence classes

public:
CurveCurveProcessAndCollectCloseApproaches (double maxDistance, DMatrix4dCP worldToLocal, double coordTol) :
    CurveCurveProcessor(worldToLocal), m_maxDistance(maxDistance), m_pairs(CompareDSegment3d(coordTol))
    {
    }

// Whether the instance collects only the closest approach, or all approaches up to m_maxDistance in length.
bool ClosestOnly() const { return m_maxDistance < 0.0; }

// Add a close approach pair to the collection.
void CollectPair(ICurvePrimitiveCP curve0, ICurvePrimitiveCP curve1, double fraction0, double fraction1, bool bReverse);

// Add a close approach pair to the collection.
void CollectPair(ICurvePrimitiveCP curve0, DPoint3dCP point0, double fraction0, ICurvePrimitiveCP curve1, DPoint3dCP point1, double fraction1, bool bReverse);

// Announce the collected, deduplicated close approach(es).
bool GetResults(CurveCurve::ICloseApproachAnnouncer& announce) const;

// Retrieve the closest approach.
bool GetResult(CurveLocationDetailPairR result) const;
};

END_BENTLEY_GEOMETRY_NAMESPACE
