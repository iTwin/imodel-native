/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
BCurveSegment::BCurveSegment ()
    {
    m_order = m_numKnots = m_workOrder = 0;
    m_index = 0;
    m_uMin = m_uMax = 0.0;
    m_isNullU = true;
    }

DRange1d BCurveSegment::KnotRange () const
    {
    return DRange1d::From (m_uMin, m_uMax);
    }

void BCurveSegment::FractionToPoint (DPoint3dR xyz, double f) const
    {
    bsiBezierDPoint4d_evaluateDPoint3dArray (&xyz, NULL, &m_poles[0], (int)m_order, &f, 1);
    }

void BCurveSegment::FractionToPoint (DPoint3dR xyz, DVec3dR tangent, double f, bool applyKnotScale) const
    {
    bsiBezierDPoint4d_evaluateDPoint3dArray (&xyz, &tangent, &m_poles[0], (int)m_order, &f, 1);
    if (applyKnotScale && m_uMax != m_uMin)
        tangent.Scale (1.0/(m_uMax - m_uMin));
    }

DPoint3d BCurveSegment::FractionToPoint (double f) const
    {
    DPoint3d xyz;
    FractionToPoint (xyz, f);
    return xyz;
    }


double BCurveSegment::FractionToKnot (double fraction) const
    {
    return m_uMin + fraction * (m_uMax - m_uMin);
    }


void BCurveSegment::CopyFrom (BCurveSegmentCR source, DMatrix4dCP matrix)
    {
    // Hmmm..  which is faster? (a) directly assign the entire struct, which copies all the unused poles/workpoles/knots, or (b) individual copies.
    *this = source;
    if (matrix)
        bsiDMatrix4d_multiply4dPoints (matrix, m_poles, source.m_poles, (int)m_order);
    }

void BCurveSegment::CopyFrom (BCurveSegmentCR source, RotMatrixCR matrix)
    {
    *this = source;
    matrix.Multiply (m_poles, source.m_poles, (int)m_order);
    }

void BCurveSegment::CopyFrom (BCurveSegmentCR source, TransformCR matrix)
    {
    *this = source;
    matrix.Multiply (m_poles, source.m_poles, (int)m_order);
    }

void BCurveSegment::SaturateKnots ()    
    {
    bsiBezier_saturateKnotsInInterval ((double*)m_poles, 4, m_knots, (int)m_order, m_isNullU);
    }

bool BCurveSegment::IsRational (bool useWorkPoles) const
    {
    double weightTolerance = Angle::SmallAngle ();
    if (useWorkPoles)
        {
        for (size_t i = 0; i < m_order; i++)
            if (fabs (m_workPoles[i].w - 1.0) > weightTolerance)
                return true;
        }
    else
        {
        for (size_t i = 0; i < m_order; i++)
            if (fabs (m_poles[i].w - 1.0) > weightTolerance)
                return true;
        }
    return false;
    }
void BCurveSegment::AddStrokes
        (
        bvector <DPoint3d> &points,
        bvector <DVec3d> *derivatives,
        bvector<double> *params,
        IFacetOptionsCR options,
        double fractionA,
        double fractionB,
        bool useWorkPoles,
        MSBsplineCurveCP curve
        ) const
    {
    DPoint4dCP polePtr = useWorkPoles ? m_workPoles : m_poles;
    int order = (int) (useWorkPoles ? m_workOrder : m_order);
    DPoint4d poles[MAX_BEZIER_ORDER];
    memcpy (poles, polePtr, order * sizeof (DPoint4d));
    if (fractionA != 0.0 || fractionB != 1.0)
        {
        bsiBezierDPoint4d_subdivideToIntervalInPlace (poles, order, fractionA, fractionB);
        }
    bool isRational = IsRational (useWorkPoles);
    int numEdge = bsiBezierDPoint4d_estimateEdgeCount ( poles, order,
                options.GetChordTolerance (),
                options.GetAngleTolerance (),
                options.GetMaxEdgeLength (),
                !isRational);
    size_t index0 = points.size ();
    bsiBezierDPoint4d_addStrokes (poles, order, points, params, NULL, numEdge, true, fractionA, fractionB);
    if (NULL != params && options.GetCurveParameterMapping () != CURVE_PARAMETER_MAPPING_BezierFraction)
        MSBsplineCurve::MapFractions (params, derivatives, index0, m_uMin, m_uMax, options.GetCurveParameterMapping (), curve);
    }

void BCurveSegment::AddExtrema
(
bvector<double> &params,
DRange3dP range,
bool mapToKnots,
size_t firstDimension,
size_t lastDimension,
bool includeStartEnd
) const
    {
    DPoint3d tangentPoles[MAX_BEZIER_ORDER];
    double componentPoles[MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    int tangentOrder;
    int numRoot;
    bsiBezierDPoint4d_pseudoTangent (tangentPoles, &tangentOrder, MAX_BEZIER_ORDER,
                            m_poles, (int)m_order, 3);
    size_t size0 = params.size ();

    if (includeStartEnd)
        {
        params.push_back (0.0);
        params.push_back (1.0);
        }

    if (range != NULL)
        {
        range->Init ();
        range->Extend (m_poles[0]);
        range->Extend (m_poles[m_order - 1]);
        }

    if (m_order <= 2)
        return;

    if (firstDimension > 2)
        firstDimension = 2;
    if (lastDimension > 2)
        lastDimension = 2;

    for (size_t dimension = firstDimension; dimension <= lastDimension; dimension++)
        {
        bsiBezier_copyComponent (componentPoles, 0, 1, (double *)tangentPoles, (int)dimension, 3, tangentOrder);
        bsiBezier_univariateRoots (root, &numRoot, componentPoles, tangentOrder);
        for (int i = 0; i < numRoot; i++)
            {
            params.push_back (root[i]);
            if (NULL != range)
                {
                DPoint3d xyz;
                FractionToPoint (xyz, root[i]);
                range->Extend (xyz);
                }
            }
        }

    size_t size1 = params.size ();
    if (size1 > size0)
        {
        DoubleOps::SortTail (params, size0);
        if (mapToKnots)
            for (size_t i = size0; i < size1; i++)
                params[i] = FractionToKnot (params[i]);
        }
    }

//! Find the intersection of the segment UMin, UMax and the given interval.
//! If empty, return false and leave the bezier.
//! If not empty, subdivide the bezier.
bool BCurveSegment::SubdivideToIntersection (DRange1dCR interval)
    {
    if (interval.Contains (m_uMin) && interval.Contains (m_uMax))
        return true;
    DRange1d baseInterval = DRange1d::From (m_uMin, m_uMax);
    DRange1d intersection = DRange1d::FromIntersection (interval, baseInterval);
    double s0, s1;
    if (    intersection.IsNull ()
        || !baseInterval.DoubleToFraction (intersection.low, s0)
        || !baseInterval.DoubleToFraction (intersection.high, s1))
        return false;

    bsiBezierDPoint4d_subdivideToIntervalInPlace (m_poles, (int)m_order, s0, s1);
    m_uMin = intersection.low;
    m_uMax = intersection.high;
    return true;
    }

#define INDEX_LENGTH 0
#define INDEX_X      1
#define INDEX_Y      2
#define INDEX_Z      3
#define MAX_PATH_CENTROID_INTEGRALS 4

static  void   cb_bezierPathIntegrands
(
double  *pF,
double  s,
BCurveSegment *pBezier,
int     numFunc
)
    {
    DPoint3d point;
    DVec3d tangent;
    double a;
    bsiBezierDPoint4d_evaluateDPoint3dArray
                (
                &point, &tangent,
                pBezier->GetWorkPoleP (0), (int)pBezier->GetWorkOrder (),
                &s, 1);

    a = tangent.Magnitude ();
    pF[INDEX_LENGTH] = a;
    if (numFunc >= 4)
        {
        pF[INDEX_X]      = a * point.x;
        pF[INDEX_Y]      = a * point.y;
        pF[INDEX_Z]      = a * point.z;
        }
    }

DPoint4dP BCurveSegment::GetWorkPoleP (size_t index) {return &m_workPoles[index];}
DPoint4dP BCurveSegment::GetPoleP (size_t index) {return &m_poles[index];}
DPoint4dP BCurveSegment::GetPoleP () {return &m_poles[0];}
size_t    BCurveSegment::GetOrder () {return m_order;}
size_t    BCurveSegment::GetWorkOrder () {return m_workOrder;}
bool      BCurveSegment::IsNullU () const {return m_isNullU;}
double    BCurveSegment::UMin () const {return m_uMin;}
double    BCurveSegment::UMax () const {return m_uMax;}
void      BCurveSegment::SetUMin (double u) {m_uMin = u;}
void      BCurveSegment::SetUMax (double u) {m_uMax = u;}
size_t    BCurveSegment::Index () const {return m_index;}
double *BCurveSegment::GetKnotP (size_t index) {return &m_knots[index];}
double *BCurveSegment::GetKnotP () {return &m_knots[0];}
size_t BCurveSegment::GetNumKnots () {return m_numKnots;}

void BCurveSegment::GetPoles (bvector<DPoint4d> &poles)
    {
    poles.clear ();
    for (uint32_t i = 0; i < m_order; i++)
        poles.push_back (m_poles[i]);
    }

void BCurveSegment::BuildWorkPoles (TransformCR transform)
    {
    m_workOrder = m_order;
    transform.Multiply (m_workPoles, m_poles, (int)m_workOrder);
    }

void BCurveSegment::Multiply (TransformCR transform)
    {
    transform.Multiply (m_poles, m_poles, (int)m_workOrder);
    }

bool BCurveSegment::TryGetPoleXYZ (size_t index, DPoint3dR xyz) const
    {
    if (index >= m_order)
        return false;
    return m_poles[index].GetProjectedXYZ (xyz);
    }

struct BezierSegmentWireIntegrationContext : BSIVectorIntegrand
{
BCurveSegment &m_segment;
int m_numFunc;
BezierSegmentWireIntegrationContext (BCurveSegment &segment, bool doCentroid)
    : m_segment(segment), m_numFunc (doCentroid ? 4 : 1)
    {
    }
int  GetVectorIntegrandCount() {return m_numFunc;}
void EvaluateVectorIntegrand (double s, double *pF)
    {
    cb_bezierPathIntegrands (pF, s, &m_segment, m_numFunc);
    }
};

void BCurveSegment::WireIntegrals (double &length, DPoint3dP centroid,
            double fraction0, double fraction1)
    {
    DPoint3d curveOrigin;
    double w;
    size_t i;
    size_t order = m_order;
    m_workOrder = m_order;
    /* Shift the origin to limit numeric problems */
    if (m_poles[0].GetProjectedXYZ(curveOrigin))
        {
        for (i = 0; i < order; i++)
            {
            m_workPoles [i] = m_poles[i];
            w = m_workPoles[i].w;
            m_workPoles[i].x -= w * curveOrigin.x;
            m_workPoles[i].y -= w * curveOrigin.y;
            m_workPoles[i].z -= w * curveOrigin.z;
            }
        }

    double integral[MAX_PATH_CENTROID_INTEGRALS], error[MAX_PATH_CENTROID_INTEGRALS];
    int count;
    static double s_absTol = 0.0;
    static double s_relTol = 1.0e-12;

    BezierSegmentWireIntegrationContext context (*this, nullptr != centroid);
    bsiMath_recursiveNewtonCotes5Vector(integral, error, &count,
        fraction0, fraction1, s_absTol, s_relTol,
        context);


    length = integral[0];
    if (NULL != centroid)
        {
        DVec3d vector0 = DVec3d::From (integral[1], integral[2], integral[3]);
        DVec3d vector1;
        if (vector1.SafeDivide (vector0, length))
            centroid->SumOf (curveOrigin, vector1);
        else
            *centroid = curveOrigin;
        }
    }

void BCurveSegment::WireCentroid (double &length, DPoint3dR centroid,
            double fraction0, double fraction1)
    {
    WireIntegrals (length, &centroid, fraction0, fraction1);
    }

// Needs work: full implementation with integration calls for just length.
void BCurveSegment::Length (double &length,
            double fraction0, double fraction1)
    {
    WireIntegrals (length, NULL, fraction0, fraction1);
    }

// Needs work: full implementation with integration calls for just length.
void BCurveSegment::Length (RotMatrixCP worldToLocal, double &length,
            double fraction0, double fraction1)
    {
    if (nullptr == worldToLocal)
        length = bsiBezierDPoint4d_arcLength (
                    GetPoleP (), (int)m_order,
                    fraction0, fraction1);
    else
        length = bsiBezierDPoint4d_arcLength (worldToLocal,
                    GetPoleP (), (int)m_order,
                    fraction0, fraction1);
    }

double BCurveSegment::PolygonLength () const
    {
    double a = 0.0;
    for (size_t i = 0; i + 1 < m_order; i++)
        {
        a += m_poles[i].RealDistance (m_poles[i+1]);
        }
    return a;
    }

// Newton iteration function for "closest approach between smooth curves".
struct BezierBezierApproachFunction : FunctionRRToRRD
{
BCurveSegmentR m_curveA;
BCurveSegmentR m_curveB;
bool m_useWorkPoles;
bool m_xyOnly;

BezierBezierApproachFunction
        (
        BCurveSegmentR curveA,
        BCurveSegmentR curveB,
        bool useWorkPoles,
        bool xyOnly
        )
    :   m_curveA(curveA), m_curveB (curveB),
        m_useWorkPoles (useWorkPoles), m_xyOnly (xyOnly)
    {}

public:
// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out]f  first function value
// @param [out]g  second function value
// @param [out]dfdu  derivative of f wrt u
// @param [out]dfdv  derivative of f wrt v
// @param [out]dgdu  derivative of g wrt u
// @param [out]dgdv  derivative of g wrt v
// @return true if function was evaluated.
bool EvaluateRRToRRD
(
double uA,
double uB,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv
) override
    {
    DPoint3d pointA, pointB;
    DVec3d d1A, d1B, d2A, d2B;
    if (m_useWorkPoles)
        {
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&pointA, &d1A, &d2A,
                    m_curveA.GetWorkPoleP (0), (int)m_curveA.GetWorkOrder (), &uA, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&pointB, &d1B, &d2B,
                    m_curveB.GetWorkPoleP (0), (int)m_curveB.GetWorkOrder (), &uB, 1);
        }
    else
        {
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&pointA, &d1A, &d2A,
                    m_curveA.GetPoleP (0), (int)m_curveA.GetOrder (), &uA, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&pointB, &d1B, &d2B,
                    m_curveB.GetPoleP (0), (int)m_curveB.GetOrder (), &uB, 1);
        }

    DVec3d chord = DVec3d::FromStartEnd (pointA, pointB);
    f = d1A.DotProduct (chord);
    if (m_xyOnly)
        {
        dfdu = d2A.DotProductXY (chord) - d1A.DotProductXY (d1A);
        dfdv = d1A.DotProductXY (d1B);
        g = d1B.DotProductXY (chord);
        dgdu = - d1B.DotProductXY (d1A);
        dgdv = d2B.DotProductXY (chord) + d1B.DotProductXY (d1B);
        }
    else
        {
        dfdu = d2A.DotProduct (chord) - d1A.DotProduct (d1A);
        dfdv = d1A.DotProduct (d1B);
        g = d1B.DotProduct (chord);
        dgdu = - d1B.DotProduct (d1A);
        dgdv = d2B.DotProduct (chord) + d1B.DotProduct (d1B);
        }
    return true;
    }

};

bool BCurveSegment::RefineCloseApproach
(
BCurveSegmentR curveA,
double fractionAIn,
BCurveSegmentR curveB,
double fractionBIn,
bool useWorkPoles,
bool xyOnly,
double &fractionAOut,
DPoint3dR xyzAOut,
double &fractionBOut,
DPoint3dR xyzBOut
)
    {
    static double s_paramTol = 1.0e-10;
    BezierBezierApproachFunction function (curveA, curveB, useWorkPoles, xyOnly);
    NewtonIterationsRRToRR newton (s_paramTol);
    fractionAOut = fractionAIn;
    fractionBOut = fractionBIn;
    bool stat = newton.RunNewton (fractionAOut, fractionBOut, function);
    curveA.FractionToPoint (xyzAOut, fractionAOut);
    curveB.FractionToPoint (xyzBOut, fractionBOut);
    return stat;
    }