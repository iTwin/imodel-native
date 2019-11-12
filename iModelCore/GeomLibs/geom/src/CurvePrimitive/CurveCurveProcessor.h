/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Base class for computations involving all combinations of 2 curve types.
// Base class has stub implementations of
//<ul>
//<li>For all combinations NOT involving a bspline:
//     ProcessAB (curveA, dataA, curveB, dataB, bReverseOrder)
//<li>For all combinations involving a bspline:
//     ProcessAB (curveA, curveB, bReverseOrder)
//</ul>
//
// The stub implemtations invoke the ProcessPrimitivePrimitive method.  This does nothing but increment
//     a counter, and may be useful as a breakpoint to look for methods lacking derived class implementation.
//
struct CurveCurveProcessor
{
GEOMAPI_VIRTUAL ~CurveCurveProcessor (){}

protected:
    //! Number of calls to base class stubs
    size_t m_numProcessedByBaseClass;
    //! distance tolerance
    double m_tol;
    //! placement transform
    DMatrix4dCP m_pWorldToLocal;
    //! control flag for extended geometry
    bool m_extend;
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

    static bool IsLinear (ICurvePrimitiveP curve)
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

explicit CurveCurveProcessor (DMatrix4dCP pWorldToLocal, double tol);

// Return true if the a fractional position within an edge is internal to the linestring.
//    (false for extensions of internal edges)
bool validEdgeFractionWithinLinestring (double f, size_t edgeIndex, size_t numPoint)
    {
    static double s_lineFractionTol = 1.0e-8;
    // interior fractions are always ok ...
    if (f >= -s_lineFractionTol && f <= 1.0 + s_lineFractionTol)
        return true;

    // We are outside the immediate edge...
    if (m_extend)
        {
        if (numPoint <= 2)
            return true;
        if (edgeIndex <= 0)
            return f <= 1.0;
        else if (edgeIndex == numPoint - 2)
            return f >= 0.0;
        }
    
    return false;
    }


// Return true if the a fractional position within an edge is internal to the linestring.
//    (false for extensions of internal edges)
bool validArcAngle (double theta, DEllipse3d const &ellipse)
    {
    if (m_extend)
        return true;
    if (ellipse.IsAngleInSweep (theta))
        return true;
    return false;
    }



void SetExtend (bool b) {m_extend=b;}

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
void Transform (DSegment4dR hSeg,DSegment3dCR cSeg);

// Apply the worldToLocal transform to an ellipse
void Transform (DConic4d &hConic, DEllipse3d const &cEllipse);
};


// CurveCurveProcessor with CurveVectorR members to capture results as announced
// by derived classes.
struct CurveCurveProcessAndCollect : public CurveCurveProcessor
{

CurveVectorR m_resultA;
CurveVectorR m_resultB;

CurveCurveProcessAndCollect (CurveVectorR intersectionA, CurveVectorR intersectionB,
            DMatrix4dCP worldToLocal, double tol) :
    CurveCurveProcessor (worldToLocal, tol),
    m_resultA (intersectionA),
    m_resultB (intersectionB)
    {
    }

void CollectPair (
        ICurvePrimitiveP curve0,
        ICurvePrimitiveP curve1,
        double fraction0,
        double fraction1,
        bool bReverseCurveOrder
        )
    {
    if (!bReverseCurveOrder)
        {
        m_resultA.push_back(ICurvePrimitive::CreatePartialCurve (curve0, fraction0, fraction0, 0));
        m_resultB.push_back(ICurvePrimitive::CreatePartialCurve (curve1, fraction1, fraction1, 0));
        }
    else
        {
        m_resultB.push_back(ICurvePrimitive::CreatePartialCurve (curve0, fraction0, fraction0, 0));
        m_resultA.push_back(ICurvePrimitive::CreatePartialCurve (curve1, fraction1, fraction1, 0));
        }
    }

void CollectPairs (
        ICurvePrimitiveP curve0,
        ICurvePrimitiveP curve1,
        bvector<double> const & fraction0,
        bvector<double> const & fraction1,
        bool bReverseCurveOrder
        )
    {
    for (size_t i = 0; i < fraction0.size (); i++)
        {
        CollectPair (curve0, curve1, fraction0[i], fraction1[i], bReverseCurveOrder);
        }
    }

};


// CurveCurveProcessor with members of type T to test and capture results as announced by derived classes.
template <typename T>
struct CurveCurveProcessAndSelectMinimum : public CurveCurveProcessor
{
double m_minWeight;
T m_dataA;
T m_dataB;
int m_numCandidates;

CurveCurveProcessAndSelectMinimum (DMatrix4dCP worldToLocal, double tol) :
    CurveCurveProcessor (worldToLocal, tol), m_numCandidates(0)
    {
    m_minWeight = DBL_MAX;
    }

bool GetResult (double &weight, T &dataA, T &dataB) const
    {
    if (m_numCandidates > 0)
        {
        dataA = m_dataA;
        dataB = m_dataB;
        return true;
        }
    return false;
    }

void TestAndCollectMin (double weight, T const &dataA, T const &dataB, bool bReverseOrder)
    {
    m_numCandidates++;
    if (weight < m_minWeight)
        {
        m_minWeight = weight;
        if (!bReverseOrder)
            {
            m_dataA = dataA;
            m_dataB = dataB;
            }
        else
            {
            m_dataB = dataA;
            m_dataA = dataB;
            }
        }
    }
};


END_BENTLEY_GEOMETRY_NAMESPACE
