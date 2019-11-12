/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
#include <Geom/implicitbezier.fdf>
#include <Geom/BinaryRangeHeap.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static bool s_debug = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          06/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_allIntersectsBtwCurves
(
DPoint3d        **ppIntPts,              /* <= intersection point(s) on curve */
double          **ppParam0,              /* <= param(s) of pts on curve0 */
double          **ppParam1,              /* <= param(s) of pts on curve1 */
int             *numPoints,            /* <= number of intersections */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision
)
    {
    bvector<double> param0;
    bvector<double> param1;
    bvector<DPoint3d> point0;
    bvector<DPoint3d> point1;
    if (NULL != rotMatrix)
        {
        DMatrix4d matrix = DMatrix4d::From (*rotMatrix);
        curve0->AddCurveIntersectionsXY (&point0, &param0, &point1, &param1, *curve1, &matrix);
        }
    else
        {
        curve0->AddCurveIntersectionsXY (&point0, &param0, &point1, &param1, *curve1, NULL);
        }

    if (NULL != ppParam0)
        *ppParam0 = DoubleOps::MallocAndCopy (param0);
    if (NULL != ppParam1)
        *ppParam1 = DoubleOps::MallocAndCopy (param1);
    if (NULL != ppIntPts)
        *ppIntPts = DPoint3dOps::MallocAndCopy (point0);
    if (NULL != numPoints)
        *numPoints = (int)param0.size ();
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/97
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isMonotonic
(
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
int                 dimension
)
    {
    int             i;
    double          *poleP = (double *) curveP->poles, *endP = poleP + 3 * curveP->params.numPoles, *weightP = curveP->weights,
                    previousValue, currentValue = 0.0, delta, previousDelta = 0.0;
    DVec3d          rows[3];

    if (NULL != rotMatrixP)
        for (i=0; i<3; i++)
            rotMatrixP->GetRow (rows[i], i);

    for ( ; poleP < endP; poleP+=3, weightP++)
        {
        previousValue = currentValue;
        if (NULL != rotMatrixP)
            currentValue = ((DPoint3d *) poleP)->DotProduct (rows[dimension]);
        else
            currentValue = poleP[dimension];

        if (curveP->rational)
            currentValue /= *weightP;

        if (poleP > (double *) curveP->poles)
            {
            delta = currentValue - previousValue;
            if (delta * previousDelta < 0.0)
                return false;

            if (delta != 0.0)
                previousDelta = delta;
            }
        }
    return true;
    }

// Structure to carry a summary of a bezier chunk within a curve.
//
struct BCurveSegmentSummary
{
private:
size_t m_readIndex; // Index of bezier within the curve.  (E.g. for use on GetBezier call)
double m_fraction0; // Start fraction within the bezier.
double m_fraction1; // End fraction within the bezier.
DRange3d m_range;   // Range of the bezier.

bvector <DPoint3d> m_points;   // Lazy cache.
bvector <double> m_params;   // Lazy cache.

public:
BCurveSegmentSummary (size_t readIndex, DRange3dCR range, double fraction0 = 0.0, double fraction1 = 1.0)
    : m_readIndex(readIndex),
      m_range (range),
      m_fraction0(fraction0),
      m_fraction1(fraction1)
    {
    }

void SetRange (DRange3dCR range)
    {
    m_range = range;
    }

struct  Array : bvector <BCurveSegmentSummary>,
                IndexedRangeHeap::IndexedRangeSource
    {
    MSBsplineCurveCP m_curve;
    Transform m_worldToLocal;
    IFacetOptionsCR m_facetOptions;
    
    Array (MSBsplineCurveCP curve, RotMatrixCP worldToLocal, IFacetOptionsCR options)
        : m_curve (curve), m_facetOptions (options)
        {
        if (NULL != worldToLocal)
            m_worldToLocal.InitFrom (*worldToLocal);
        else
            m_worldToLocal.InitIdentity ();
        }
    GEOMAPI_VIRTUAL ~Array (){}

    // Get bezier by bezierIndex in original curve (NOT array index!!!)
    // Work poles are set up in transformed coordinates.
    bool GetBezier (BCurveSegmentR bezier, size_t bezierIndex)
        {
        if (m_curve->GetBezier (bezier, bezierIndex))
            {
            bezier.BuildWorkPoles (m_worldToLocal);        
            return true;
            }
        return false;
        }

    bool Get (size_t index, size_t &bezierIndex, double &fraction0, double &fraction1)
        {
        if (index >= size ())
            return false;
        bezierIndex = at (index).m_readIndex;
        fraction0   = at (index).m_fraction0;
        fraction1   = at (index).m_fraction1;
        return true;
        }

    bool LoadPoints (size_t index)
        {
        size_t bezierIndex;
        double fraction0, fraction1;
        if (!Get (index, bezierIndex, fraction0, fraction1))
            return false;
        bvector<DPoint3d> &points = at(index).m_points;
        bvector<double> &params = at(index).m_params;
        
        if (points.size () > 0)
            return true;
        BCurveSegment bezier;
        if (!m_curve->GetBezier (bezier, bezierIndex))
            return false;
        bezier.BuildWorkPoles (m_worldToLocal);
        bezier.AddStrokes (points, NULL, &params, m_facetOptions, fraction0, fraction1, true, m_curve);
        return true;
        }

    // Get points from entry in array, possibly requiring bezier extraction and stroking from original curve.
    bvector <DPoint3d> *GetPointsP (size_t index)
        {
        size_t bezierIndex;
        double fraction0, fraction1;
        if (!Get (index, bezierIndex, fraction0, fraction1))
            return NULL;
        return &at(index).m_points;
        }

    bvector <double> *GetParamsP (size_t index)
        {
        size_t bezierIndex;
        double fraction0, fraction1;
        if (!Get (index, bezierIndex, fraction0, fraction1))
            return NULL;
        return &at(index).m_params;
        }

    bool GetRange (size_t i0, size_t i1, DRange3dR range) const override
        {
        range.Init ();
        size_t n = size ();
        size_t count = 0;
        for (size_t i = i0; i < n && i <= i1; i++)
            {
            count++;
            range.Extend (at(i).m_range);
            }
        return count > 0;
        }

    DRange3d GetRange () const
        {
        DRange3d range;
        range.Init ();
        for (size_t i = 0, n = size (); i < n; i++)
            range.Extend (at(i).m_range);
        return range;
        }

    // Build array in which each entry is a portion of a single bezier segment and is monotone in both x and y.
    void BuildRangesOnCurveExtrema ()
        {
        clear ();
        BCurveSegment segment;
        bvector <double> fractions;
        for (size_t readIndex = 0;
            m_curve->AdvanceToBezier (segment, readIndex);
            )
            {
            fractions.clear ();
            DRange3d range;
            segment.AddExtrema (fractions, NULL, false, 0, 1, true);
            // (Always get at least 2 back !!!)
            DPoint3d xyz0, xyz1;
            double lowerFraction = fractions[0];    // we know this is 0.0.
            segment.FractionToPoint(xyz0, lowerFraction);
            xyz0.z = 0.0;
            for (size_t i = 1; i < fractions.size (); i++)
                {
                double upperFraction = fractions[i];
                if (!MSBsplineCurve::AreSameKnots (lowerFraction, upperFraction))
                    {
                    if (s_debug)
                        printf ("   (monotone (%g,%g) of (K %g,%g)\n",
                                lowerFraction, upperFraction, segment.UMin (), segment.UMax ());

                    range.Init ();
                    segment.FractionToPoint(xyz1, upperFraction);
                    xyz1.z = 0.0;
                    range.InitFrom (xyz0, xyz1);
                    push_back (BCurveSegmentSummary (segment.Index (), range, lowerFraction, upperFraction));                        
                    // If there are repeated, almost equal fractions, the lower-to-upper update
                    // only happens when the "real" next gets hit.
                    // Hmmm. should dups "near 1" be forced up to 1?
                    lowerFraction = upperFraction;
                    xyz0 = xyz1;
                    }
                }
            }
        }
    };
};


// Algorithmic context for curve-curve self intersection.
static double s_rangeIntersectionFraction = 1.0e-8;
static double s_intersectionApproachFraction = 1.0e-6;
struct CCISelfIntersectionSearcher : IndexedRangeHeap::PairProcessor
{
IndexedRangeHeap m_heap;
BCurveSegmentSummary::Array &m_bezierData;
double m_adjacentIntersectionRangeTol;
double m_approachTolerance;
GEOMAPI_VIRTUAL ~CCISelfIntersectionSearcher () {}
CCISelfIntersectionSearcher (BCurveSegmentSummary::Array &bezierData)
    : m_bezierData (bezierData), m_heap ()
    {
    DRange3d range = m_bezierData.GetRange ();
    DVec3d diagonal = DVec3d::FromStartEnd (range.low, range.high);
    double d = diagonal.MagnitudeXY ();
    m_adjacentIntersectionRangeTol = s_rangeIntersectionFraction * d;
    m_approachTolerance = s_intersectionApproachFraction * d;
    }

void Go ()
    {
    m_heap.Build (1, &m_bezierData, 0, m_bezierData.size () - 1);
    IndexedRangeHeap::Search (m_heap, m_heap, *this);
    }

bool /*PairProcessor::*/NeedProcessing
(
DRange3dCR rangeA, size_t indexA0, size_t indexA1,
DRange3dCR rangeB, size_t indexB0, size_t indexB1
) override
    {
    DRange3d intersection;
    intersection.IntersectIndependentComponentsOf (rangeA, rangeB);

    if (indexB1 < indexA0)  // ASSUME -- A0,A1 is in order.  B0,B1 is in order. We only want "A before B" cases in the sequence.
        return false;
    if (  intersection.low.x > intersection.high.x
       || intersection.low.y > intersection.high.y)
        return false;

    if (indexA0 == indexA1 && indexB0 == indexB1)
        {
        // Leaf level with real intersection !!!
        // ignore exact hit or order inversion ...
        if (indexA0 >= indexB0)
            return false;
        if (indexB0 == indexA0 + 1)
            {
            // Adjacent beziers.
            // Dismiss if intesection looks like just the common point or edge.
            // (Hm.. is the common edge of ranges case really a throwaway?)
            if (   intersection.low.x + m_adjacentIntersectionRangeTol >= intersection.high.x
                || intersection.low.y + m_adjacentIntersectionRangeTol >= intersection.high.y)
                return false;
            }
        return true;
        }
    // real overlap and not the adjacent bezier case ....
    return true;
    }


bvector <CurveLocationDetail> m_locationA;
bvector <CurveLocationDetail> m_locationB;
void /*PairProcessor::*/Process(size_t indexA, size_t indexB) override
    {
    if (indexA == indexB)
        return;
    size_t bezierIndexA, bezierIndexB;
    double fractionA0, fractionA1, fractionB0, fractionB1;
    if (!m_bezierData.LoadPoints (indexA) || !m_bezierData.LoadPoints (indexB))
        return;
    bvector <DPoint3d> const *pointsA = m_bezierData.GetPointsP (indexA);
    bvector <DPoint3d> const *pointsB = m_bezierData.GetPointsP (indexB);
    bvector <double> const *paramsA = m_bezierData.GetParamsP (indexA);
    bvector <double> const *paramsB = m_bezierData.GetParamsP (indexB);
    m_bezierData.Get (indexA, bezierIndexA, fractionA0, fractionA1);
    m_bezierData.Get (indexB, bezierIndexB, fractionB0, fractionB1);
    size_t i0 = m_locationA.size ();
    PolylineOps::AddCloseApproaches (*pointsA, paramsA, *pointsB, paramsB, m_locationA, m_locationB, m_approachTolerance);
    if (s_debug)
        printf (" CCI Bezier real pair (%d %d (%g %g) %d) X (%d %d (%g %g) %d)\n",
            (int)indexA, (int)bezierIndexA, fractionA0, fractionA1, (int)pointsA->size(),
            (int)indexB, (int)bezierIndexB, fractionB0, fractionB1, (int)pointsA->size());
    if (m_locationA.size () > i0)
        {
        BCurveSegment bezierA, bezierB;
        m_bezierData.GetBezier (bezierA, bezierIndexA);
        m_bezierData.GetBezier (bezierB, bezierIndexB);
        size_t i1 = i0;
        // For each linestring intersection:  Refine on the real bezier.  Throw out ones that don't converge.
        for (size_t i = i0; i < m_locationA.size (); i++)
            {
            double fractionA0 = m_locationA[i].fraction;
            double fractionB0 = m_locationB[i].fraction;
            if (s_debug)
                printf ("\n RAW LLInt @ %g,%g\n", fractionA0, fractionB0);
            double fractionA, fractionB;
            DPoint3d pointA, pointB;
            if (BCurveSegment::RefineCloseApproach
                    (
                    bezierA, fractionA0,
                    bezierB, fractionB0, true, true,
                    fractionA, pointA, fractionB, pointB
                    ))
                {
                if (s_debug)
                    printf ("\n    Refine @ %g,%g\n", fractionA, fractionB);
                if (pointA.DistanceXY (pointB) < m_approachTolerance)
                    {
                    m_locationA[i1] = m_locationA[i];
                    m_locationB[i1] = m_locationB[i];
                    m_locationA[i1].fraction = bezierA.FractionToKnot (fractionA);
                    m_locationB[i1].fraction = bezierB.FractionToKnot (fractionB);
                    m_locationA[i1].componentFraction = fractionA;
                    m_locationB[i1].componentFraction = fractionB;
                    if (s_debug)
                        printf (" (xyz %g %g) (fA %g) (fB %g)\n",
                            m_locationA[i1].point.x, m_locationA[i1].point.y,
                            m_locationA[i1].fraction, m_locationB[i1].fraction);
                    i1++;
                    }
                }
            }
        m_locationA.resize (i1);
        m_locationB.resize (i1);
        }
    }

bool GetLocation (CurveLocationDetailR locationA, CurveLocationDetailR locationB, size_t i) const
    {
    if (i < m_locationA.size () && i < m_locationB.size ())
        {
        locationA = m_locationA[i];
        locationB = m_locationB[i];
        return true;
        }
    return false;
    }

size_t GetNumLocation () const { return m_locationA.size ();}

bool IsLive() const override { return true;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_selfIntersections
(
double          **paramPP,               /* <= param(s) of pts on curve */
int             *numPointsP,            /* <= number of intersections */
MSBsplineCurve  *curve,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision
)
    {
    if (paramPP != NULL)
        *paramPP = NULL;

    if (numPointsP != NULL)
        *numPointsP = 0;

    if (isMonotonic (curve, rotMatrix, 0) ||
        isMonotonic (curve, rotMatrix, 1))
        return ERROR;

    IFacetOptionsPtr options = IFacetOptions::New ();
    options->SetAngleTolerance (0.20);
    options->SetCurveParameterMapping (CURVE_PARAMETER_MAPPING_BezierFraction);
    BCurveSegmentSummary::Array componentRanges (curve, rotMatrix, *options);
    componentRanges.BuildRangesOnCurveExtrema ();
    CCISelfIntersectionSearcher searcher (componentRanges);
    searcher.Go ();
    CurveLocationDetail detailA, detailB;
    size_t numIntersection = searcher.GetNumLocation ();
    size_t numOut = 2 * numIntersection;
    if (NULL != numPointsP)
        *numPointsP = (int)numOut;
    // Each hit is TWO parameters, both on this curve.
    if (NULL != paramPP && numIntersection > 0)
        {
        double *outParams = *paramPP = (double*)BSIBaseGeom::Malloc (numOut * sizeof (double));
        size_t k = 0;
        for (size_t i = 0; searcher.GetLocation (detailA, detailB, i); i++)
            {
            outParams[k++] = detailA.fraction;
            outParams[k++] = detailB.fraction;
            }
        }
    return numOut > 0 ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspcci_extIntersectCurves
(
DPoint3d        **intPts,              /* <= intersection point(s) on curve */
double          **param0,              /* <= param(s) of pts on curve0 */
double          **param1,              /* <= param(s) of pts on curve1 */
int             *numPoints,            /* <= number of intersections */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision

)
    {
    return bspcci_allIntersectsBtwCurves (intPts, param0, param1, numPoints, curve0, curve1, tolerance, rotMatrix, false) ;
    }
#ifdef compile_bspcuv_classifyPoint
#define NEAR_ZERO                              0.00001
#define NEAR_ONE                               0.99999

/*---------------------------------------------------------------------------------**//**
* Note: This function can only be called from bspcurv_countRayIntersections
* @bsimethod                                                    Lu.Han          06/94
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspcurv_isTangentReflection
(
DPoint3d            *testDir,   /* => normalized vector */
MSBsplineCurve      *curveP,    /* => curve to test */
double              param       /* => parameter to test */
)
    {
    bool            isReflect = true;
    DPoint3d        tangent, tangent0, tangent1;

    bspcurv_evaluateCurvePoint (NULL, &tangent, curveP, param);
    tangent.Normalize ();
    if (1.0 - fabs (tangent.DotProduct (*testDir)) < NEAR_ZERO)
        {
        if (param < 1.0 - NEAR_ZERO && param > NEAR_ZERO)
            {
            bspcurv_evaluateCurvePoint (NULL, &tangent0, curveP, param - NEAR_ZERO);
            bspcurv_evaluateCurvePoint (NULL, &tangent1, curveP, param + NEAR_ZERO);
            tangent0.Normalize ();
            tangent1.Normalize ();
            tangent0.CrossProduct (tangent0, tangent);
            tangent1.CrossProduct (tangent1, tangent);
            if (tangent0.DotProduct (tangent1) > 0.0)
                isReflect = false;
            }
        }
    else
        isReflect = false;

    return isReflect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_countRayIntersectionsExt
(
int                 *numIntersectionP,      /* <= number of crossings */
int                 *numRepeatP,            /* <= number of crossings with repeated parameter. */
int                 *numStartP,             /* <= number of crossings at the ray start */
DPoint3d            *originP,
DPoint3d            *directionP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
)
    {
    int             numRepeat, numIntersection, numStart;
    bool            first;
    double          *wP, *rayParamP, paramTolerance, tangentDot, perpDot, diff,
                    *curveParamP, *param0P, *param1P, *param2P, *endP;
    double          minPerp, maxPerp;
    double          minTangent, maxTangent;
    double          start, end;
    DPoint3d        rayPoints[2], *pP, *pEnd, delta, unWeighted;
    DVec3d          normal, direction;
    MSBsplineCurve  rayCurve;
    DVec3d          zDir;

    mdlBspline_normalizeCurveKnots (&start, &end, curveP);

    numRepeat = numIntersection = numStart = 0;

    rayPoints[0] = *originP;
    if (directionP)
        {
        /* ASSUME ... direction is in xy plane */
        direction = *(DVec3d*)directionP;
        rotMatrixP->GetRow (zDir, 2);
        normal.CrossProduct (zDir, *directionP);
        }
    else
        {
        rotMatrixP->GetRow (direction, 0);
        rotMatrixP->GetRow (normal, 1);
        }

    normal.Normalize ();
    first = true;
    maxPerp = minPerp = 0.0;  /* Will be replaced on first pass */
    minTangent = maxTangent = 0.0;

    if (curveP->rational)
        {
        for (pP=pEnd=curveP->poles, pEnd += curveP->params.numPoles, wP=curveP->weights;
                pP < pEnd;
                    wP++, pP++)
            {
            unWeighted.Scale (*pP, 1.0 / *wP);
            delta.DifferenceOf (unWeighted, rayPoints[0]);
            tangentDot = delta.DotProduct (direction);
            perpDot = delta.DotProduct (normal);
            if (first)
                {
                maxTangent = minTangent = tangentDot;
                minPerp = maxPerp = perpDot;
                first = false;
                }
            else
                {
                if (tangentDot > maxTangent)
                    maxTangent = tangentDot;
                if (tangentDot < minTangent)
                    minTangent = tangentDot;
                if (perpDot < minPerp)
                    minPerp = perpDot;
                if (perpDot > maxPerp)
                    maxPerp = perpDot;
                }
            }
        }
    else
        {
        for (pP=pEnd=curveP->poles, pEnd += curveP->params.numPoles; pP < pEnd;  pP++)
            {
            delta.DifferenceOf (*pP, rayPoints[0]);
            tangentDot = delta.DotProduct (direction);
            perpDot = delta.DotProduct (normal);
            if (first)
                {
                maxTangent = minTangent = tangentDot;
                minPerp = maxPerp = perpDot;
                first = false;
                }
            else
                {
                if (tangentDot > maxTangent)
                    maxTangent = tangentDot;
                if (tangentDot < minTangent)
                    minTangent = tangentDot;
                if (perpDot < minPerp)
                    minPerp = perpDot;
                if (perpDot > maxPerp)
                    maxPerp = perpDot;
                }
            }
        }

    if (maxTangent <= -tolerance || minPerp > tolerance || maxPerp < -tolerance)
        {
        numIntersection = 0;
        }
    else if (minTangent > tolerance)
        {
        /* The polygon is a connected point set lying entirely on the
                positive x axis.  We must have an even number of crossings.
                Just call it 2.
        */
        numIntersection = 2;
        }
    else if ( maxPerp - minPerp < tolerance)
        {
        /* The polygon has both positive and negative points,
            AND is sitting entirely on the x axis, so it just
            spans the origin.  Call it one crossing.
        */
        numIntersection = 1;
        }
    else
        {
        rayPoints[1].SumOf (rayPoints[0], direction, maxTangent + 1.0);
        bspconv_convertLstringToCurve (reinterpret_cast<int *>(&rayCurve.type),  reinterpret_cast<int *>(&rayCurve.rational),
                                       &rayCurve.display, &rayCurve.params,
                                       &rayCurve.poles, &rayCurve.knots,
                                       &rayCurve.weights, rayPoints, 2);

        bspcci_allIntersectsBtwCurves (NULL, &rayParamP, &curveParamP, &numIntersection,
                                       &rayCurve, curveP, &tolerance, rotMatrixP, false);

        if (numIntersection)
            {
            paramTolerance = fabs (tolerance / (maxTangent + 1.0));
            direction.Normalize ();
            for (param0P = endP = rayParamP, param1P = curveParamP,
                 endP += numIntersection, numRepeat = 0;
                 param0P < endP; param0P++, param1P++)
                {
                if (*param0P < paramTolerance)
                    numStart++;
                else
                    {
                    for (param2P = curveParamP; param2P < param1P; param2P++)
                        {
                        diff = fabs(*param1P - *param2P);
                        if (diff < paramTolerance || (curveP->params.closed && (diff > NEAR_ONE)))
                            {
                            numRepeat++;
                            break;
                            }
                        }

                    if (param2P == param1P)
                        if (bspcurv_isTangentReflection (&direction, curveP, *param1P))
                            numRepeat++;
                    }
                }


            BSIBaseGeom::Free (rayParamP);
            BSIBaseGeom::Free (curveParamP);
            }
        bspcurv_freeCurve (&rayCurve);
        }
    *numIntersectionP = numIntersection;
    *numRepeatP       = numRepeat;
    *numStartP        = numStart;

    mdlBspline_unNormalizeCurveKnots (curveP, start, end);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspcurv_countRayIntersections
(
DPoint3d            *originP,
DPoint3d            *directionP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
)
    {
    int numInt, numBreak, numStart;
    bspcurv_countRayIntersectionsExt (&numInt, &numBreak, &numStart, originP, directionP, curveP, rotMatrixP, tolerance);
    return numInt - numBreak - numStart;
    }
/*---------------------------------------------------------------------------------**//**
* Classify a point with respect to a curve. 0 -- point on curve 1 -- point not on curve; an arbitrarily chosen ray was found with an odd
* number of crossings. 2 -- point not on curve, an arbitrarily chosen ray was found with an even number of crossings (possibly zero) NOTE:
* Also returns 0 if no clear on, in, out could be found.
* @bsimethod                                                    Earlin.Lutz     02/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspcurv_classifyPoint
(
DPoint3d            *originP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
)
    {
    double firstRetryAngle = 0.23423;
    double retryFactor     = 1.183423;
    double maxRetryAngle   = 10.0;
    int numInt, numRepeat, numStart;
    DVec3d xDir, yDir, newDir;
    rotMatrixP->GetRow (xDir, 0);

    bspcurv_countRayIntersectionsExt (&numInt, &numRepeat, &numStart, originP, &xDir, curveP, rotMatrixP, tolerance);
    if (numStart > 0)
        {
        return 0;
        }
    else if (numRepeat == 0)
        {
        return (numInt & 0x01) ? 1 : 2;
        }
    else
        {
        double theta;
        double c, s;
        rotMatrixP->GetRow (yDir, 1);
        for (theta = firstRetryAngle; theta <= maxRetryAngle; theta *= retryFactor)
            {
            c = cos (theta);
            s = sin (theta);
            newDir.x = xDir.x * c + yDir.x * s;
            newDir.y = xDir.y * c + yDir.y * s;
            newDir.z = xDir.z * c + yDir.z * s;
            bspcurv_countRayIntersectionsExt (&numInt, &numRepeat, &numStart, originP, &newDir, curveP, rotMatrixP, tolerance);
            if (numStart > 0)
                {
                return 0;
                }
            else if (numRepeat == 0)
                {
                return (numInt & 0x01) ? 1 : 2;
                }
            }
        }
    return 0;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_segmentCurveIntersection
(
int                 *numIntersectionP,      /* <= number of crossings */
double              **segmentParamPP,       /* <= array of parameters on segment */
double              **curveParamPP,         /* <= array of parameters on curve */
DPoint3d            *startP,
DPoint3d            *endP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
)
    {
    DSegment3d segment = DSegment3d::From (*startP, *endP);
    bvector<double> curveFraction;
    bvector<double> segmentFraction;
    DMatrix4d matrix4d;
    matrix4d.InitIdentity ();
    if (NULL != rotMatrixP)
        matrix4d = DMatrix4d::From (*rotMatrixP);
    curveP->AddLineIntersectionsXY (NULL, &curveFraction,
                                    NULL, &segmentFraction,
                                    segment, false,
            NULL != rotMatrixP ? &matrix4d : NULL
            );
    if (NULL != segmentParamPP)
        *segmentParamPP = DoubleOps::MallocAndCopy (segmentFraction);
    if (NULL != curveParamPP)
        *curveParamPP = DoubleOps::MallocAndCopy (curveFraction);
    if (NULL != numIntersectionP)
        *numIntersectionP = (int)segmentFraction.size ();
    }
END_BENTLEY_GEOMETRY_NAMESPACE