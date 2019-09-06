/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef enum
    {
    TCH_Parity  = 0,
    TCH_All     = 1,
    TCH_Outer   = 2
    } TCH_SelectMode;

/* Internal nominal limit on number of scan lines
   over entire boundary range.
   Due to rounding of scale factors, anything up to twice
   this number goes through.

   This number changes horrible performance to merely bad performance.
   For complex patterns created by multiple, interrelated patterns, applying this
    limit independently creates havoc in the patterns.
   A significantly smaller limit -- say, 1000 or 2000 -- should be applied
    by the application itself and applied consistently.  Because transverse and
    tangential spacings is coupled, this is very tricky!!!!
*/
static int s_maxLines = 20000;

/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param a          => value to append
* @return false if array overflow.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendDouble

(
double *pDest,
int     *pNumOut,
int     maxOut,
double  a
)
    {
    int k = *pNumOut;
    if (k < maxOut)
        {
        pDest[k++] = a;
        *pNumOut = k;
        return true;
        }
    return false;
    }

static int jmdlGPA_roundUpInt
(
double value
)
    {
    return (int)ceil (value);
    }

static int jmdlGPA_roundDownInt
(
double value
)
    {
    return (int)floor (value);
    }
static double jmdlGPA_roundDownDouble
(
double value
)
    {
    return floor (value);
    }

struct HatchArray : bvector <GraphicsPoint>
{
void Add (GraphicsPointCR gp){ push_back (gp);}
void Add (GraphicsPointCR gpA, GraphicsPointCR gpB) { push_back (gpA); push_back (gpB);}
void Add (DPoint4dCR point, bool markBreak)
    {
    GraphicsPoint gp (point);
    if (markBreak)
        gp.mask |= HPOINT_MASK_BREAK;
    push_back (GraphicsPoint (point));
    }
// Bundle 2 consecutive points starting at point index i as a DSegment3d
bool GetSegment (size_t i, DSegment3dR segment, HatchSegmentPosition &position) const
    {
    if (i + 1 < size ())
        {
        at(i).point.GetXYZ (segment.point[0]);
        at(i + 1).point.GetXYZ (segment.point[1]);
        position.hatchLine = at(i).userData;
        position.startDistance = at(i).a;
        position.endDistance = at(i+1).a;
        return true;
        }
    return false;
    }
// Append simple DSegment3d's to bvector.
void AppendSegments (bvector<DSegment3d> &segments, bvector<HatchSegmentPosition> *segmentPosition)
    {
    DSegment3d segment;
    HatchSegmentPosition position;
    for (size_t i = 0; GetSegment (i, segment, position); i += 2)
        {
        segments.push_back (segment);
        if (nullptr != segmentPosition)
            segmentPosition->push_back (position);
        }
    }
};




/*====================================================================================
* Hatch generation notes:
* 1) Collect all crossings of the hatch plane with boundaries.  Each is recorded
*   as (integer) plane number and (double) distance in arbitrary orthogonal direction.
* 2) Sort by plane number and distance.
* 3) Ignoring vertex-on-plane conditions, use alternating points on each plane
*       as in/out pairs.
* 4) Mark vertex-on-plane points.   After sorting, each interval of the paired points requires
*       explicit in-out testing.
*====================================================================================*/

struct GPA_TransformedHatchContext
    {
    HatchArray m_collector;
    Transform    transform;
    Transform    inverse;
    /* Plane k in the cross hatch system is officially
            fixedPlane + k * incrementalPlane.
       BUT ... We are willing to have the knowledge that the incremental plane is
            0,0,0,-1
            used explicitly
    */
    DPoint4d        fixedPlane;
    DPoint4d        incrementalPlane;
    bool            zRangeLimits;
    int             minPlane;
    int             maxPlane;
    double          altitudeTolerance;
    GPA_TransformedHatchContext () :
        zRangeLimits(false)
        {
        }

/*---------------------------------------------------------------------------------**//**
* Initialize the transform, inverse, and plane fields.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool    InitTransform
(
        CurveVectorCR boundary,
TransformCR transformA,
        int                             maxLines
)
    {
    bool    boolstat;
    static double s_endTolerance = 1.0e-8;
    double s_maxGapRelTol = 1.0e-6;
    double maxGapTol = 1.0;

    DRange3d range;
    boundary.GetRange (range);
    maxGapTol = s_maxGapRelTol * range.low.DistanceXY (range.high);

    //cvHatch_appendWithArcsAsBezier (pBoundary, pBoundary);
    //cvHatch_forceBezierAndLinestringEndsToNeighbors (pBoundary, maxGapTol);
    transform = transformA;
    zRangeLimits = false;
    altitudeTolerance = s_endTolerance;
    boolstat = inverse.InverseOf (transformA);

    if (boolstat)
        {

        if (maxLines > 0)
            {
            RotMatrix scaleMatrix1;
            auto zScale = CurveVector::ComputeHatchDensityScale (transformA, range, maxLines);
            if (!zScale.IsValid ())
                return false;
            // Be sure the saved index for each line reflects the scaling.....
            //  in a grouped hole, saved index must correspond to actual plane shared across holes.
            scaleMatrix1.InitFromScaleFactors (1.0, 1.0, zScale.Value ());
            transform = transform *scaleMatrix1;
            if (!inverse.InverseOf (transform))
                return false;
            }
        fixedPlane.Init (
                        inverse.form3d[2][0],
                        inverse.form3d[2][1],
                        inverse.form3d[2][2],
                        inverse.form3d[2][3]
                        );
        incrementalPlane.Init (
                        0.0,
                        0.0,
                        0.0,
                        -1.0
                        );
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool    InitSinglePlaneIntersection
(
CurveVectorCR boundary,
DPlane3dCP                              pPlane
)
    {
    DVec3d uVec, vVec, wVec;
    bool    boolstat = false;
    double s_maxGapRelTol = 1.0e-6;
    double maxGapTol = 1.0;
    DRange3d range;
    if (!boundary.GetRange (range))
        return false;

    maxGapTol = s_maxGapRelTol * range.low.DistanceXY (range.high);

    //cvHatch_appendWithArcsAsBezier (pBoundary, pBoundary);
    //cvHatch_forceBezierAndLinestringEndsToNeighbors (pBoundary, maxGapTol);
    pPlane->normal.GetNormalizedTriad (uVec, vVec, wVec);
    transform.InitFromOriginAndVectors(pPlane->origin, uVec, vVec, wVec);
    boolstat = inverse.InverseOf (transform);

    fixedPlane.Init(
                    inverse.form3d[2][0],
                    inverse.form3d[2][1],
                    inverse.form3d[2][2],
                    inverse.form3d[2][3]
                    );
    incrementalPlane.Init(
                    0.0,
                    0.0,
                    0.0,
                    -1.0
                    );

    zRangeLimits = true;
    minPlane = 0;
    maxPlane = 0;
    altitudeTolerance = 2.0 * maxGapTol;
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddStroke
(
HatchArray          &dest,
GraphicsPointCR     gp0In,
GraphicsPointCR     gp1In,
bool                            normalizeOutput,
double                          daMin
)
    {
    auto gp0 = gp0In;
    auto gp1 = gp1In;
    gp0.mask = gp1.mask = 0;

    if (fabs (gp1.a - gp0.a) < daMin)
        return;
#ifdef doClipPlanes
    if (pContext->pClipPlanes)
        {
        const GraphicsPoint *pClipPlaneBuffer = cvHatch_getConstPtr (pContext->pClipPlanes, 0);
        int numClipPlanes = cvHatch_getCount (pContext->pClipPlanes);
        int i;
        double sMin, sMax, h0, h1;
        double s;
        sMin = 0.0;
        sMax = 1.0;
        /* Find the parameter limits sMin and sMax of the active segment.
            (0 <= sMin <= sMax <= 1) is the fraction parameter range of active points.
        */
        for (i = 0; i < numClipPlanes; i++)
            {
            h0 = gp0.point.DotProduct (*(&pClipPlaneBuffer[i].point));
            h1 = gp1.point.DotProduct (*(&pClipPlaneBuffer[i].point));
            if (h1 >= h0)
                {
                if (h0 > 0.0)
                    return;
                if (h1 > 0.0)
                    {
                    s = -h0 / (h1 - h0);
                    /* Clip away fractional parameters larger than s */
                    if (s < sMin)
                        return;
                    if (s < sMax)
                        sMax = s;
                    }
                }
            else
                {
                if (h1 > 0.0)
                    return;
                if (h0 > 0.0)
                    {
                    s = -h0 / (h1 - h0);
                    /* Clip away fractional paramters smaller than s */
                    if (s > sMax)
                        return;
                    if (s > sMin)
                        sMin = s;
                    }
                }
            }

        if (sMin > 0.0)
            {
            gp0.point.Interpolate (pGP0->point, sMin, pGP1->point);
            gp0.a = (1.0 - sMin) * pGP0->a  + sMin * pGP1->a;
            }

        if (sMax < 1.0)
            {
            gp1.point.Interpolate (pGP0->point, sMax, pGP1->point);
            gp1.a = (1.0 - sMax) * pGP0->a  + sMax * pGP1->a;
            }

        /* Check one more time for short strokes ... */
        if (fabs (gp1.a - gp0.a) < daMin)
            return;
        }
#endif

    if (normalizeOutput)
        {
        if (gp0.point.w != 1.0)
            gp0.point.InitWithNormalizedWeight (gp0.point);
        if (gp1.point.w != 1.0)
            gp1.point.InitWithNormalizedWeight (gp1.point);
        }
    dest.Add (gp0);
    dest.Add (gp1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
void    CollectOutput
(
HatchArray       &dest,
int                             selectMode,
bool                            normalizeOutput
)
    {
    size_t sourceCount = m_collector.size ();
    int i0, i1, i, numCut;
    double aMin, aMax;
    static double s_minFraction = 1.0e-10;
    static double s_minDelta    = 1.0e-14;
    double daMin;

    if (m_collector.size () > 0)
        {
        aMin = aMax = m_collector[0].a;
        for (i0 = 1; i0 < sourceCount; i0++)
            {
            if (m_collector[i0].a < aMin)
                aMin = m_collector[i0].a;
            if (m_collector[i0].a > aMax)
                aMax = m_collector[i0].a;
            }
        daMin = s_minFraction * (aMax - aMin);
        if (daMin < s_minDelta)
            daMin = s_minDelta;

        for (i0 = 0; i0 < sourceCount; i0 = i1)
            {
            /* Find index range for points at same altitute */
            for (  i1 = i0 + 1;
                   i1 < sourceCount
                && m_collector[i1].userData == m_collector[i0].userData;
                   i1++)
                {
                /* Nothing to do -- we're just counting them up */
                }

            numCut = i1 - i0;
            if (numCut <= 1)
                {
                }
            else if (selectMode == TCH_All || numCut <= 3)
                {
                AddStroke (
                        dest,
                        m_collector[i0],
                        m_collector[i1 - 1],
                        normalizeOutput,
                        daMin
                        );
                }
            else if (selectMode == TCH_Outer)
                {
                AddStroke (
                        dest,
                        m_collector[i0],
                        m_collector[i0 + 1],
                        normalizeOutput,
                        daMin
                        );
                AddStroke (
                        dest,
                        m_collector[i1 - 2],
                        m_collector[i1 - 1],
                        normalizeOutput,
                        daMin
                        );
                }
            else
                {
                /* Parity .. left side wins if odd crossing count */
                for (i = i0; i < i1 - 1; i += 2)
                    {
                    AddStroke (
                        dest    ,
                        m_collector[i],
                        m_collector[i + 1],
                        normalizeOutput,
                        daMin
                        );
                    }
                }
            }
        }
    }
};



/*---------------------------------------------------------------------------------**//**
* Lexical comparison of two hatch candidates.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlGPA_compareHatchCandidates
(
const GraphicsPoint   *pPoint0,
const GraphicsPoint   *pPoint1
)
    {
    if (pPoint1->userData > pPoint0->userData)
        return -1;

    if (pPoint1->userData < pPoint0->userData)
        return 1;

    if (pPoint1->a > pPoint0->a)
        return -1;

    if (pPoint1->a < pPoint0->a)
        return 1;

    return 0;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a single dash to the output.  Parameter values are assumed to be ordered, and
* only the dash portion within a0<a<a1 will be output.
*
* @param pGP0   => line start point.
* @param pGP1   => line end point.
* @param a0     => interpolation knot for start
* @param a1     => interpolation knot for end
* @param b0     => interpolated start parameter
* @param b1     => interpolated end parameter
* @param divDeltaA => 1 over (a1 - a0), to save the divide.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    InsertInterpolatedDash
(
bvector<DSegment3d> &segments,
        DPoint3dCR           xyz0,
        DPoint3dCR           xyz1,
        double                  a0,
        double                  a1,
        double                  b0,
        double                  b1,
        double                  divDeltaA
)
    {
    if (b1 > a0)
        {
        if (b1 > a1)
            b1 = a1;
        if (b0 < a0)
            b0 = a0;
        DPoint3d xyzA, xyzB;
        xyzA.SumOf (xyz0, (a1 - b0) * divDeltaA, xyz1, (b0 - a0) * divDeltaA);
        xyzB.SumOf (xyz0, (a1 - b1) * divDeltaA, xyz1, (b1 - a0) * divDeltaA);
/*
        point0.SumOf(pGP0->point, (a1 - b0) * divDeltaA, pGP1->point, (b0 - a0) * divDeltaA);

        point1.SumOf(pGP0->point, (a1 - b1) * divDeltaA, pGP1->point, (b1 - a0) * divDeltaA);
*/
        segments.push_back (DSegment3d::From (xyzA, xyzB));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DashData::DashData (size_t maxDash)
    {
    m_maxDash = maxDash;
    m_period = 0;
    m_dashLengths.clear ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DashData::SetDashLengths (double const *lengths, uint32_t count)
    {
    m_period = 0.0;
    m_dashLengths.clear ();
    for (uint32_t i = 0; i < count; i++)
        AddDash (lengths[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DashData::SetDashLengths (bvector<double> const &lengths)
    {
    m_period = 0.0;
    m_dashLengths.clear ();
    for (auto a : lengths)
        AddDash (a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DashData::AddDash (double length)
    {
    m_dashLengths.push_back (length); m_period += fabs (length);
    }
#ifdef abc
/*---------------------------------------------------------------------------------**//**
*
* Apply dash patterning to a single line.
*
* @param pGP0 => line start point.
* @param pGP1 => line end point.
* @param pDashLengths => array of dash lengths.  Negative lengths are "off".
*   (Remark: It seems obvious that alternate entries will have alternate signs.
*   If not, we just do as we're told.)
* @param numDashLength => number of dash lengths in pattern.
* @param dashPeriod => total of all dash lengths.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpandSingleLineDashPattern
(
bvector<DPoint3d> &collector,
        DPoint3dCR xyz0,
        double a0,
        DPoint3dCR xyz1,
        double a1,
const   double                      *pDashLengths,
        int                         numDashLength,
        double                      dashPeriod,
        size_t                      maxCollectorPoints
)
    {
    if (dashPeriod <= 0.0)
        dashPeriod = jmdlGPA_computeDashPeriod (pDashLengths, numDashLength);
    if (dashPeriod <= 0.0)
        return;

    if (a0 > a1)
        {
        ExpandSingleLineDashPattern (collector, xyz1, a1, xyz1, a0,
                                    pDashLengths, numDashLength, dashPeriod, maxCollectorPoints);
        }
    else
        {
        double b0 = jmdlGPA_roundDownDouble (a0 / dashPeriod) * dashPeriod;
        double db;
        double b1;

        double delta = ((a1 - a0) == 0.0)? 1.0 : (a1 - a0);
        double divDeltaA = 1.0 / delta;
        //double divDeltaA = 1.0 / (a1 - a0);
        int i;

        for (i = 0; b0 < a1 && collector.size () < maxCollectorPoints;)
            {
            db = pDashLengths[i];
            b1 = b0 + fabs (db);
            if (db >= 0.0)
                InsertInterpolatedDash
                            (
                            collector,
                            xyz0, xyz1,
                            a0, a1, b0, b1,
                            divDeltaA);
            b0 = b1;
            if (++i >= numDashLength)
                i = 0;
            }
        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DashData::AppendDashes
(
DSegment3dCR segment,               //!< [in] segment to be expanded
double startDistance,               //!< [in] distance (along containing line, scaled for dash lengths) to start of this segment.
double endDistance,                 //!< [in] distance (along containing line, scaled for dash lengths) to end of this segment.
bvector<DSegment3d> &segments       //!< [out] segments
)
    {
    if (m_period <= 0.0 || m_dashLengths.empty ())
        return;

    if (endDistance < startDistance)
        {
        AppendDashes (segment, endDistance, startDistance, segments);
        }
    else
        {
        double b0 = jmdlGPA_roundDownDouble (startDistance / m_period) * m_period;
        double db;
        double b1;
        size_t numDashLength = m_dashLengths.size ();
        double delta = endDistance - startDistance;
        if (delta == 0.0)
            delta = 1.0;
        double divDeltaA = 1.0 / delta;

        for (size_t i = 0; b0 < endDistance && segments.size () < m_maxDash;)
            {
            db = m_dashLengths[i];
            b1 = b0 + fabs (db);
            if (db >= 0.0)
                InsertInterpolatedDash
                (
                    segments,
                    segment.point[0], segment.point[1],
                    startDistance, endDistance, b0, b1,
                    divDeltaA);
            b0 = b1;
            if (++i >= numDashLength)
                i = 0;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DashData::AppendDashes
(
bvector<DSegment3d> const &segmentA,               //!< [in] segments to be expanded
bvector<HatchSegmentPosition> const &positionA,     //!< [in] dash position data.  Assumed compatible with segmentA
bvector<DSegment3d> &segmentB       //!< [out] segments
)
    {
    for (size_t i = 0; i < segmentA.size (); i++)
        AppendDashes (segmentA[i], positionA[i].startDistance, positionA[i].endDistance, segmentB);
    }

/*---------------------------------------------------------------------------------**//**
* Split a double into integer and fraction parts, specifically for use in identifying
*   parallel cutting planes, hence possibly requiring referene to the context for
*   spacing information.
* @param pIndex     <= integer part.  Always algebraically less than or equal to height.
* @param pFraction  <= fractional part.  At least zero, less than one.
* @param height     => floating point height.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_splitHeight
(
GPA_TransformedHatchContext     *pContext,
int                             *pIndex,
double                          *pFraction,
double                          height
)
    {
    int index;
    if (height >= 0)
        {
        index = (int)height;
        }
    else
        {
        index = -1 - (int) (-height);
        }

    *pIndex = index;
    *pFraction = height - index;
    }


/*---------------------------------------------------------------------------------**//**
* Find the extremal, integerized values of k such that f + k * g = 0, where f and g are
*   bezier polynomials.
* @return true if indices were calculated.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_crossingExtrema
(
int         *pk0,
int         *pk1,
double      *pF,
double      *pG,
int         numPole
)
    {
    int k0, k1, i;
    int m0, m1;
    double a;

    if (numPole <= 0)
        {
        k0 = 0;
        k1 = -1;
        }
    else
        {
        k0 = k1 = 0;    // Will be overwritten on first pass.
        for (i = 0; i < numPole; i++)
            {
            if (DoubleOps::SafeDivide (a, -pF[i], pG[i], 0.0))
                {
                m0 = jmdlGPA_roundDownInt (a);
                m1 = jmdlGPA_roundUpInt   (a);
                if (i == 0)
                    {
                    k0 = m0;
                    k1 = m1;
                    }
                else
                    {
                    if (m0 < k0)
                        k0 = m0;
                    if (m1 > k1)
                        k1 = m1;
                    }
                }
            }
        }
    if (pk0)
        *pk0 = k0;
    if (pk1)
        *pk1 = k1;
    return k1 >= k0;
    }


/*---------------------------------------------------------------------------------**//**
* Apply the context's inverse transform to points.
* @param pOut <= transformed points
* @param pIn  => original points
* @param numPoint => number of points.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_applyInverse
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pOut,
const   DPoint4d                *pIn,
int                             numPoint
)
    {
    pContext->inverse.Multiply (pOut, pIn, numPoint);
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlGPA_TCH_getPointHeight
(
GPA_TransformedHatchContext     *pContext,
const DPoint4d                  *pPoint
)
    {
    return pPoint->z / pPoint->w;
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_planeIndexToPlane
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pPlane,
        int                     index
)
    {
    *pPlane = pContext->fixedPlane;
    pPlane->w -= (double)index;
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlGPA_TCH_pointToSortCoordinate
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pPoint
)
    {
    double localX =
              pContext->inverse.form3d[0][0] * pPoint->x
            + pContext->inverse.form3d[0][1] * pPoint->y
            + pContext->inverse.form3d[0][2] * pPoint->z
            + pContext->inverse.form3d[0][3] * pPoint->w;
    if (pPoint->w == 1.0)
        return localX;

    return localX / pPoint->w;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutLine
(
GPA_TransformedHatchContext *pContext,
const   DPoint4d            *pPlane,
const   DPoint4d            *pLineStart,
const   DPoint4d            *pLineEnd,
const   DPoint4d            *pLocalStart,
const   DPoint4d            *pLocalEnd,
        int                 index
)
    {
    double h0, h1, ah;
    double f0, f1;
    double a0, a1;
    DPoint4d hPoint, hLocalPoint;
    double hTol = pContext->altitudeTolerance;

    GraphicsPoint gPoint;
    h0 = pPlane->DotProduct (*pLineStart);
    h1 = pPlane->DotProduct (*pLineEnd);

    a0 = fabs (h0);
    a1 = fabs (h1);

    if (a0 < hTol)
        a0 = 0.0;
    if (a1 < hTol)
        a1 = 0.0;

    /* Ignore dead horizontal */
    if (a0 <= hTol && a1 <= hTol)
        return;

    /* Ignore local minimum */
    if (a0 <= hTol && h1 > hTol)
        return;

    if (a1 <= hTol && h0 > hTol)
        return;

    /* Ignore segment completely to one side of plane */
    if (a0 * a1 != 0.0 && h0 * h1 > 0.0)
        return;

    ah = 1.0 / (h1 - h0);
    f0 = h1 * ah;
    f1 = - h0 * ah;
    hPoint.SumOf(*pLineStart, f0, *pLineEnd, f1);
    hLocalPoint.SumOf(*pLocalStart, f0, *pLocalEnd, f1);

    gPoint.point    = hPoint;
    gPoint.a        = hLocalPoint.x / hLocalPoint.w;
    gPoint.userData = index;
    gPoint.mask     = 0;
#ifdef HATCH_DEBUG
    if (s_noisy)
        printf ("Line cut (%d,%lf)\n", gPoint.userData, gPoint.a);
#endif
    pContext->m_collector.Add (gPoint);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_applyPlaneLimits
(
GPA_TransformedHatchContext     *pContext,
int                             *pPlane0,
int                             *pPlane1
)
    {
    int plane0, plane1;
    if (pContext->zRangeLimits)
        {
        if (*pPlane0 < pContext->minPlane)
            *pPlane0 = pContext->minPlane;
        if (*pPlane1 > pContext->maxPlane)
            *pPlane1 = pContext->maxPlane;
        }

    /* Watch for integer overflow case!!
        If plane1 is at maxint, "next" integer wraps to negative.
    */
    plane0 = *pPlane0;
    plane1 = *pPlane1;
    plane1++;
    if (plane1 < *pPlane1)
        {
        *pPlane1 -= 1;
        }
    }
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectPolylineCrossings
(
GPA_TransformedHatchContext     *pContext,
bvector<DPoint3d> const &points
)
    {
    GraphicsPoint gp0, gp1;
    DPoint4d    p0, p1;
    int     index0, index1;
    int     plane0, plane1, plane;
    DPoint4d    currPlane;
    double  h0 = 0, h1;
    double f0, f1;
    double nearOne = 1.0 - 1.0e-8;
    for (size_t i = 0; i < points.size (); i++, gp0 = gp1)
        {
        gp1 = GraphicsPoint (points[i]);
        jmdlGPA_TCH_applyInverse (pContext, &p1, &gp1.point, 1);

        h1 = jmdlGPA_TCH_getPointHeight (pContext, &p1);
        if (i > 0)
            {
            if (h0 < h1)
                {
                jmdlGPA_TCH_splitHeight (pContext, &index0, &f0, h0);
                jmdlGPA_TCH_splitHeight (pContext, &index1, &f1, h1);
                }
            else
                {
                jmdlGPA_TCH_splitHeight (pContext, &index0, &f0, h1);
                jmdlGPA_TCH_splitHeight (pContext, &index1, &f1, h0);
                }

            plane0 = index0;
            if (f0 > 0.0)       /* Tolerance ?? */
                plane0++;
            plane1 = index1;    /* Tolerance ?? */
            if (f1 > nearOne)
                plane1++;

            jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
#ifdef HATCH_DEBUG
            if (s_noisy)
                printf ("(Line (%12.2lf,%12.2lf) (%12.2lf,%12.2lf) (delta %12.2lf,%12.2lf) planes %d %d)\n",
                            gp0.point.x, gp0.point.y, gp1.point.x, gp1.point.y,
                            gp1.point.x - gp0.point.x,
                            gp1.point.y - gp0.point.y,
                            plane0, plane1);
#endif
            for (plane = plane0; plane <= plane1; plane++)
                {
                jmdlGPA_TCH_planeIndexToPlane (pContext, &currPlane, plane);
                jmdlGPA_TCH_cutLine (pContext, &currPlane, &gp0.point, &gp1.point,
                                                        &p0, &p1, plane);
                }
            }
        gp0     = gp1;
        p0      = p1;
        h0      = h1;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    closeToAngle
(
double theta,
double theta0,
bool    bExpandTolerance,
double expansionFactor
)
    {
    if (bExpandTolerance)
        {
        // This is a +-PI normalization...
        double alpha = bsiTrig_getNormalizedAngle (theta - theta0);
        return fabs (alpha) < expansionFactor * bsiTrig_smallAngle ();
        }
    else
        return bsiTrig_equalAngles (theta, theta0); // Start crossing -- need to find negative points nearby
    }

/*---------------------------------------------------------------------------------**//**
* Generate intersections of a plane with an ellipse
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutDEllipse3d
(
        GPA_TransformedHatchContext    *pContext,
        DEllipse3dCR ellipse,
        DPoint4dCR          plane,
        int                 index
)
    {
    DPoint3d trigPoint[2];
    DPoint4d arcPoints[2];
    bool     endAngleIsNearPlane [2];
    double endAngle[2];
    GraphicsPoint gPoint;
    double hTol = pContext->altitudeTolerance;
    int i;
    double h;
    bool    bAccept;
    double theta;
    double hCenter;
    static double s_expansionFactor = 1.0e6;
    hCenter = plane.DotProduct (ellipse.center, 1.0);

    int  numUnbounded = ellipse.IntersectPlane (trigPoint, plane);

    endAngle[0] = ellipse.start;
    endAngle[1] = ellipse.start + ellipse.sweep;
    for (i = 0; i < 2; i++)
        {
        ellipse.Evaluate (arcPoints[i], endAngle[i]);
        h = plane.DotProduct (arcPoints[i]);
        endAngleIsNearPlane[i] = fabs (h) < hTol;
        }
    bool isFullCircle = Angle::IsFullCircle (ellipse.sweep);
    for (i = 0; i < numUnbounded; i++)
        {
        double dHdTheta, cc, ss, hCenter;
        bAccept = false;
#ifdef libraryOK
        theta = trigPoint[i].z;
        cc = cos (theta);
        ss = sin (theta);
#else
        // library code has x,y flipped in atan call. Recompute it ...
        cc = trigPoint[i].x;
        ss = trigPoint[i].y;
        theta = Angle::Atan2 (ss, cc);
        trigPoint[i].z = theta;
#endif
        auto derivVector = DVec3d::FromSumOf (ellipse.vector0, -ss, ellipse.vector90, cc);
        dHdTheta = ellipse.sweep * plane.DotProduct (derivVector, 0.0);
        hCenter = plane.DotProduct (ellipse.center, 1.0);
        if (closeToAngle (theta, endAngle[0], endAngleIsNearPlane[0], s_expansionFactor))
            {
            if (isFullCircle)
                {
                bAccept = dHdTheta > hTol;
                }
            else if (dHdTheta < -hTol)
                bAccept = true;
            else if (   dHdTheta < hTol
                    &&  hCenter < 0.0)
                {
                bAccept = true;
                }
            }
        else if (closeToAngle (theta, endAngle[1], endAngleIsNearPlane[1], s_expansionFactor)) // End crossing -- need to find positivve points nearby
            {
            if (dHdTheta > hTol)
                bAccept = true;
            else if (   dHdTheta > -hTol
                    && hCenter < 0.0)
                {
                bAccept = true;
                }
            }
        else if (Angle::InSweepAllowPeriodShift (trigPoint[i].z, ellipse.start, ellipse.sweep))
            {
            // Only accept simple crossings.
            if (fabs (dHdTheta) > hTol)
                bAccept = true;
            }

        //printf ("%s (%d,%d) q=%10.4lf (q0,dq)=(%10.4lf,%10.4lf)\n", bAccept ? "+  " : "  -", i, numUnbounded, theta, theta0, sweep);
        if (bAccept)
            {
            ellipse.Evaluate (gPoint.point, trigPoint[i].z);
            gPoint.a     = jmdlGPA_TCH_pointToSortCoordinate (pContext, &gPoint.point);
            gPoint.userData = index;
            gPoint.mask     = 0;
            pContext->m_collector.Add (gPoint);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void            jmdlGPA_TCH_getHomogeneousPlaneTerms
(
const   GPA_TransformedHatchContext    *pContext,
        DPoint4d                *pH0,
        DPoint4d                *pH1
)
    {
    *pH0 = pContext->fixedPlane;
    *pH1 = pContext->incrementalPlane;
    }

/*---------------------------------------------------------------------------------**//**
*
* @return true if indices were calculated.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_TCH_ellipseExtrema
(
GPA_TransformedHatchContext    *pContext,
int         *pPlane0,
int         *pPlane1,
DEllipse3dCR ellipse
)
    {
#define MAX_SORT_DIST 4
#define MAX_CLIPPED_SORT_DIST 6
    double alpha0, alpha1, beta0, beta1, gamma0, gamma1;
    double alpha, beta, gamma;
    double sortDist[4];
    double clippedSortDist[6];
    int numSortDist, numClippedSort;
    double theta;
    DPoint4d H0, H1;
    double a;
    int i;
    bool    result = false;

    jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);

    alpha0 = H0.DotProduct (ellipse.center, 1.0);
    alpha1 = H1.DotProduct (ellipse.center, 1.0);

    beta0 = H0.DotProduct (ellipse.vector0, 0.0);
    beta1 = H1.DotProduct (ellipse.vector0, 0.0);

    gamma0 = H0.DotProduct (ellipse.vector90, 0.0);
    gamma1 = H1.DotProduct (ellipse.vector90, 0.0);

    alpha = alpha0 * alpha0 - beta0 * beta0 - gamma0 * gamma0;
    beta  = 2.0 * (alpha0 * alpha1 - beta0 * beta1 - gamma0 * gamma1);
    gamma = alpha1 * alpha1 - beta1 * beta1 - gamma1 * gamma1;

    numSortDist = bsiMath_solveQuadratic (sortDist, gamma, beta, alpha);

    if (numSortDist < 2)
        {
        /* ugh. Badly conditioned data.  Just evaluate a bunch of points
           in the ellipse range and find out where they hit. */
        DPoint4d currPoint;
        int numPoint = 16;
        int numOK = 0;
        double df = 1.0 / (numPoint - 1);
        double hMin = DBL_MAX, hMax = - DBL_MAX;
        for (i = 0; i < numPoint; i++)
            {
            ellipse.Evaluate (currPoint, ellipse.start + i * df * ellipse.sweep);
            if (DoubleOps::SafeDivide (a,
                                -H0.DotProduct (currPoint),
                                H1.DotProduct (currPoint),
                                0.0))
                {
                if (numOK == 0)
                    hMin = hMax = a;
                else if (a < hMin)
                    hMin = a;
                else if (a > hMax)
                    hMax = a;
                numOK++;
                }
            }
        if (numOK > 0)
            {
            *pPlane0 = jmdlGPA_roundDownInt (hMin);
            *pPlane1 = jmdlGPA_roundUpInt (hMax);
            return true;
            }
        }

    /* Needs work: hyperbolic case */

    if (ellipse.IsFullEllipse ())
        {
        /* Just accept the tangent distances as limits */
        if (numSortDist == 2)
            {
            DoubleOps::Sort (sortDist, numSortDist, true);
            *pPlane0 = jmdlGPA_roundDownInt (sortDist[0]);
            *pPlane1 = jmdlGPA_roundUpInt (sortDist[1]);
            result = true;
            }
        }
    else
        {
        numClippedSort = 0;
        for (i = 0; i < numSortDist; i++)
            {
            a = sortDist[i];
            alpha = alpha0 + a * alpha1;
            beta  = beta0  + a * beta1;
            gamma = gamma0 + a * gamma1;
            theta = Angle::Atan2 (gamma, beta);
            /* HACK .. a little confused on signs here. Only one of these
                angles matters, but really cheap to test both. */
            if (ellipse.IsAngleInSweep (theta))
                appendDouble (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            theta += msGeomConst_pi;
            if (ellipse.IsAngleInSweep (theta))
                appendDouble (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }
        DPoint3d point0, point1;
        ellipse.EvaluateEndPoints (point0, point1);
        if (DoubleOps::SafeDivide (a,
                            -H0.DotProduct (point0, 1.0),
//                            -H0.DotProduct (*pStartPoint),
                            H1.DotProduct (point0, 1.0),
//                            H1.DotProduct (*pStartPoint),
                            0.0))
            {
            appendDouble (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }

        if (DoubleOps::SafeDivide (a,
                            -H0.DotProduct (point1, 1.0),
//                            -H0.DotProduct (*pEndPoint),
                            H1.DotProduct (point1, 1.0),
//                            H1.DotProduct (*pEndPoint),
                            0.0))
            {
            appendDouble (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }

        if (numClippedSort >= 1)
            {
            DoubleOps::Sort (clippedSortDist, numClippedSort, true);
            *pPlane0 = jmdlGPA_roundDownInt (clippedSortDist[0]);
            *pPlane1 = jmdlGPA_roundUpInt (clippedSortDist[numClippedSort - 1]);
            result = true;
            }

        }

    return result;
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectDEllipse3dCrossings
(
GPA_TransformedHatchContext    *pContext,
DEllipse3dCR ellipse
)
    {
    DPoint4d H0, H1, H;
    int plane, plane0, plane1;
    jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);
    if (jmdlGPA_TCH_ellipseExtrema (pContext, &plane0, &plane1, ellipse))
        {
        jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
#ifdef HATCH_DEBUG
        if (s_noisy)
            printf ("(Ellipse (%12.2lf,%12.2lf) (U %12.2lf,%12.2lf) (V %12.2lf,%12.2lf) planes %d %d)\n",
                        ellipse.center.x, ellipse.center.y,
                        ellipse.vector0.x, ellipse.vector0.y,
                        ellipse.vector90.x, ellipse.vector90.y,
                        plane0, plane1);
        //printf ("\n\n Planes %d to %d\n", plane0, plane1);
#endif
        for (plane = plane0; plane <= plane1; plane++)
            {
            //printf ("              -------------------------------------------- PLANE %d\n", plane);
            H.SumOf(H0, H1, (double)plane);
            jmdlGPA_TCH_cutDEllipse3d (pContext, ellipse, H, plane);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Generate intersections of a plane with a bezier, where the plane intersections are
* defined by a univariate bezier which is a linear combination of two others.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutCombinationBezier
(
        GPA_TransformedHatchContext    *pContext,
        DPoint4d            *pPoleArray,
        double              *pHeight0Array,
        double              *pHeight1Array,
        double              a,
        int                 numPole,
        int                 index
)
    {
    int i;
    int numRoot;
    if (numPole < 1)
        return;
    double heightPole[MAX_BEZIER_CURVE_ORDER];
    double rootArray[MAX_BEZIER_CURVE_ORDER];

    DPoint4d    rootPointArray[MAX_BEZIER_CURVE_ORDER];
    GraphicsPoint gPoint;
    DPoint3d      cPoint;
    // Tolerance for identifying endpoints.
    // Endpoints with near-zero altitude according to global
    //   toleranced will be forced to exact zero, and we
    //   expect the root finder to generate an exact 0 or 1
    //   parameter. So the parametric tolerance for endpoint
    //   is tight ...
    static double s_paramRelTol = 1.0e-14;
    static double s_derivativeTol = 1.0e-8;
    double poleTol = pContext->altitudeTolerance;
    bool    bAccept;

    for (i = 0; i < numPole; i++)
        {
        heightPole[i] = pHeight0Array[i] + a * pHeight1Array[i];
        }

    if (fabs(heightPole[0]) < poleTol)
        heightPole[0] = 0.0;

    if (fabs (heightPole[numPole-1]) < poleTol)
        heightPole[numPole - 1] = 0.0;

    bsiBezier_univariateRoots (rootArray, &numRoot, heightPole, numPole);

    if (numRoot > 0)
        {
        bsiBezierDPoint4d_evaluateArray
                (rootPointArray, NULL, pPoleArray, numPole, rootArray, numRoot);
        for (i = 0; i < numRoot; i++)
            {
            double u = rootArray[i];
            double f, df, ddf;
            int k;
            bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, heightPole, numPole, 1, rootArray[i]);
            bAccept = false;
            // We want a simple crossing or a local MAX
            // At start or end, look for a nonzero pole to decide
            if (u < s_paramRelTol)
                {
                for (k = 1; k < numPole; k++)
                    {
                    if (heightPole[k] > s_derivativeTol)
                        break;  // Heading positive ....
                    else if (heightPole[k] < -s_derivativeTol)
                        {
                        bAccept = true;
                        break;
                        }
                    }
                }
            else if (u > 1.0 - s_paramRelTol)
                {
                for (k = numPole - 2; k >= 0; k--)
                    {
                    if (heightPole[k] < -s_derivativeTol)
                        {
                        bAccept = true;
                        break;
                        }
                    }
                }
            else if (fabs (df) > s_derivativeTol)
                {
                bAccept = true; // simple crossing.
                }
            else
                {
                // local min. In usual case we see both sides of the hit and can skip it.
                // Hope it's not an inflection too!!
                // bAccept = ddf < 0.0;
                }

            if (bAccept)
                {
                gPoint.point = rootPointArray[i];
                gPoint.point.GetXYZ (cPoint);
                gPoint.a     = jmdlGPA_TCH_pointToSortCoordinate (pContext, &gPoint.point);
                gPoint.userData = index;
                gPoint.mask     = 0;
                pContext->m_collector.Add (gPoint);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    collectBezierCrossings
(
GPA_TransformedHatchContext     *pContext,
BCurveSegment segment
)
    {
    DPoint4d H0, H1;
    double   poleArray0[MAX_BEZIER_CURVE_ORDER];
    double   poleArray1[MAX_BEZIER_CURVE_ORDER];
    int plane, plane0, plane1;
    jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);
    DPoint4dP poles = segment.GetPoleP ();
    auto numPole = (int)segment.GetOrder ();
    for (int i = 0; i <  numPole; i++)
        {
        poleArray0[i] = H0.DotProduct (poles[i]);
        poleArray1[i] = H1.DotProduct (poles[i]);
        }
    if (jmdlGPA_crossingExtrema (&plane0, &plane1, poleArray0, poleArray1, numPole))
        {
        jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
        for (plane = plane0; plane <= plane1; plane++)
            {
            jmdlGPA_TCH_cutCombinationBezier
                    (pContext, poles, poleArray0, poleArray1, (double)plane, numPole, plane);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectBsplineCrossings
(
GPA_TransformedHatchContext     *pContext,
MSBsplineCurveCR curve
)
    {
    BCurveSegment segment;
    for (size_t i = 0; curve.AdvanceToBezier (segment, i, true);)
        {
        collectBezierCrossings (pContext, segment);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectCrossings
(
GPA_TransformedHatchContext     *pContext,
CurveVectorCR curves
)
    {
    DSegment3d segment;
    DEllipse3d arc;
    bvector<DPoint3d> const *points;
    bvector<DPoint3d> segmentPoints;
    MSBsplineCurveCP bcurve;
    for (auto &curve : curves)
        {
        points = curve->GetLineStringCP ();
        if (nullptr != points)
            {
            jmdlGPA_TCH_collectPolylineCrossings (pContext, *points);
            }
        else if (curve->TryGetLine (segment))
            {
            segmentPoints.clear ();
            segmentPoints.push_back (segment.point[0]);
            segmentPoints.push_back (segment.point[1]);
            jmdlGPA_TCH_collectPolylineCrossings (pContext, segmentPoints);
            }
        else if (curve->TryGetArc (arc))
            {
            jmdlGPA_TCH_collectDEllipse3dCrossings (pContext, arc);
            }
        else if (nullptr != (bcurve = curve->GetProxyBsplineCurveCP ()))
            {
            jmdlGPA_TCH_collectBsplineCrossings (pContext, *bcurve);
            }
        else
            {
            auto child = curve->GetChildCurveVectorP ();
            if (child.IsValid ())
                jmdlGPA_TCH_collectCrossings (pContext, *child);
            }
        }
    }





/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_sort
(
GPA_TransformedHatchContext     *pContext
)
    {
    size_t sourceCount = pContext->m_collector.size ();

    if (sourceCount > 1)
        {
        qsort (
            pContext->m_collector.data (),
            sourceCount,
            sizeof (GraphicsPoint),
            (int (*)(const void *, const void *))jmdlGPA_compareHatchCandidates);
        }
    }

#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Crosshatch elements come in with coordinate system set up to assure that x is a good sort direction.
* Single-plane section cuts do not -- all the cut points might have identical x coordinates.
* For a planar cuttee, the cut will produce colinear points.  A vector between any reasonably well separated
*   pair of cut points will be a good sort direction.  To get well separated, find the point farthest
*   from point 0 -- this will be at least half the cut length.
* REMARK:  Earlier code here assumed the cut points were in the local system, so only xy variation was
*    possible. This is wrong.  The cut points are global.  Must use true vectors to get sortable coordinate.
* @bsimethod                                                    EarlinLutz      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_chooseSortCoordinate
(
GPA_TransformedHatchContext     *pContext
)
    {
    DPoint3d xyzBase, xyzCurr, xyzFar;
    DVec3d lineVector;
    double d, dMax;
    int i;
    if (sourceCount <= 1)
        return;

    // Find the point farthest from point 0 ...
    pSourceArray[0].point.GetProjectedXYZ (xyzBase);
    dMax = 0.0;
    for (i = 1; i < sourceCount; i++)
        {
        GraphicsPoint *pCurr = &pSourceArray[i];
        pCurr->point.GetProjectedXYZ (xyzCurr);
        d = xyzBase.DistanceSquared (xyzCurr);
        if (d > dMax)
            {
            dMax = d;
            xyzFar = xyzCurr;
            }
        }

    // Assign sort coordinate along maximal direction ..
    lineVector.NormalizedDifference(xyzFar, xyzBase);
    for (i = 0; i < sourceCount; i++)
        {
        GraphicsPoint *pCurr = &pSourceArray[i];
        pCurr->point.GetProjectedXYZ (xyzCurr);
        pCurr->a = xyzCurr.DotDifference(xyzBase, lineVector);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_setClassification
(
GraphicsPoint   *pGP,
bool            bInBit,
bool            bOutBit
)
    {
    if (pGP)
        {

        }
    }

#endif
/*---------------------------------------------------------------------------------**//**
* @description Return a scale factor to be applied to the z-axis of the hatch transform so that
* at most maxLines scan planes are defined within specified range.
* @param transform => proposed hatch transform.  xy plane is hatch plane.  z direction
*       is advance vector between successive planes.
* @param range range of data.
* @param maxLines => max number of lines allowed in specified ranges. If 0 or negative, a default
*               is applied.
* @return scale factor to apply to z vector. Any error condition returns 1.0.  Example error
*       conditions are (a) null range, and (b) singular transform.
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDouble CurveVector::ComputeHatchDensityScale
(
TransformCR transform,
DRange3dCR worldRange,
int                             maxLines
)
    {
    double scale = 1.0;
    DRange3d localRange;
    Transform inverse;
    double dNumLines;
    double dMaxLines = (double)maxLines;
    double bigNum = (double)(0x7FffFFff);

    if (!inverse.InverseOf (transform))
        return ValidatedDouble (1.0, false);

    if (worldRange.IsNull ())
        return ValidatedDouble (1.0, false);

    inverse.Multiply (localRange, worldRange);

    /* Converted V7 files sometimes ask for 1 UOR spacing.
    Just blow these off
    */
    if (fabs (localRange.high.z) > bigNum)
        return ValidatedDouble (1.0, false);
    if (fabs (localRange.low.z) > bigNum)
        return ValidatedDouble (1.0, false);

    if (dMaxLines <= 0)
        dMaxLines = (double)s_maxLines;

    dNumLines = localRange.high.z - localRange.low.z;
    if (dNumLines > dMaxLines)
        {
        double exactScale = dNumLines / dMaxLines;
        int integerScale = jmdlGPA_roundUpInt (exactScale);
        scale = (double)integerScale;
        }

    return ValidatedDouble (scale, true);
    }


/*---------------------------------------------------------------------------------**//**
*
* Generate crosshatch in the loops of the boundary array.  Add the crosshatch to the
* instance array.
*
* On return the cross hatch is a sequence of alternating start/stop graphics points.
* On each graphics point of the hatch, the user int is the hatch line index and the
* double value is the sort coordinate along the hatch line.
*
* @param pTranfsorm         => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.   In/out is determined by parity rules
*                               after x-direction sort.
* @param pClipRangePoints   => any number of points which are vertices of the clip box.
*                                   Hatch planes which fall outside the range of these points
*                                   in hatch space (after transformation) are not considered.
* @param pClipPlanes        => any number of clip planes, expressed as DPoint4d plane coefficients.
*                                   Negative points of each halfspace are IN.  The convex
*                                   clip region is accepted.
* @param selectRule         => selects inside/outside rule.  On each scan line,
*
*                               0 -- simple parity -- alternating in out
*                               1 -- maximal stroke -- only first and last crossings matter
*                               2 -- outer -- if 4 or more crossings, only first and last
*                                       pairs matter.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     cvHatch_addTransformedCrossHatchClipped
(
HatchArray &collector,
CurveVectorCR curves,
TransformCR transformA,
int     selectRule
)
    {
    GPA_TransformedHatchContext context;
    if (context.InitTransform (curves, transformA, s_maxLines))
        {
        jmdlGPA_TCH_collectCrossings (&context, curves);
        jmdlGPA_TCH_sort (&context);
        context.CollectOutput (collector, selectRule, true);
        }
    }

#ifdef CompileSinglePlaneCase
/*---------------------------------------------------------------------------------**//**
Generate the intersection (sticks) of a plane with the area bounded by loops in the boundary GPA.
On return the intersection is a sequence of alternating start/stop graphics points.

@param pPlane   IN plane to intersect.
@param selectRule           => selects inside/outside rule.  On each scan line,
@bsimethod                                                      EarlinLutz      03/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     cvHatch_intersectDPlane3d
(
        HatchArray &collector,
GraphicsPointArrayCP pBoundary,
DPlane3dCP                          pPlane
)
    {
    GPA_TransformedHatchContext context;
    GraphicsPointArray  *pScratchArray = cvHatch_grab ();

#ifdef HATCH_DEBUG
    if (s_noisy)
        printf("\n ADD TRANSFORMED CROSSHATCH\n");
#endif
    if (jmdlGPA_TCH_initSinglePlaneIntersection (&context, pBoundary, pScratchArray, pPlane))
        {
        jmdlGPA_TCH_collectCrossings (&context);
        jmdlGPA_TCH_chooseSortCoordinate (&context);
        jmdlGPA_TCH_sort (&context);
        jmdlGPA_TCH_collectOutput (&context, pHatchCollector, 0, true);
        }
    }


/*---------------------------------------------------------------------------------**//**
@description Inspect the ranges of multiple arrays.   Determine an appropriate scale factor
    to apply to the transform to limit line counts.
@param pBoundaryArray1 IN array of boundaries
@param numBoundary1 IN number of boundareis in pBoundaryArray1
@param pBoundaryArray2 IN array of boundaries
@param numBoundary2 IN number of boundaries in pBoundaryArray2
@param pTransformIn IN nominal transform (from hatch space to world.  integer z is a hatch plane index)
@param pClipRangePoints IN range of points in clippers.
@param pTransformOut OUT transform with z scale applied to limit lines.
@param pScael OUT applied scale factor.
@returns false if data just looks horrible.
@param
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     cvHatch_applyLineLimitsToTransform
(
GraphicsPointArrayP      *pBoundaryArray1,
int                      numBoundary1,
GraphicsPointArrayP      *pBoundaryArray2,
int                      numBoundary2,
const   Transform     *pTransformIn,
GraphicsPointArrayP      pClipRangePoints,
int                      maxHatchLines,
Transform             *pTransformOut,
double                   *pScale
)
    {
    double zCurr;
    double zMax = 1.0;
    int i;

    if (pScale)
        *pScale = 1.0;
    if (pTransformOut)
        *pTransformOut = *pTransformIn;

    for (i = 0; i < numBoundary1; i++)
        {
        if (!ComputeHatchDensityScale
                        (&zCurr, pTransformIn, pBoundaryArray1[i], pClipRangePoints, maxHatchLines))
            return false;
        if (zCurr > zMax)
            zMax = zCurr;
        }

    for (i = 0; i < numBoundary2; i++)
        {
        if (!ComputeHatchDensityScale
                        (&zCurr, pTransformIn, pBoundaryArray2[i], pClipRangePoints, maxHatchLines))
            return false;
        if (zCurr > zMax)
            zMax = zCurr;
        }

    if (pScale)
        *pScale = zMax;
    if (zMax > 1.0 && pTransformIn != NULL)
        {
        RotMatrix scaleMatrix;
        scaleMatrix.InitFromScaleFactors (1.0, 1.0, zMax);
        pTransformOut->InitProduct (*pTransformIn, scaleMatrix);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Generate all points that might be endpoints of crosshatch in a certain direction.
*
* The returned hatch contains every point where the boundary geometry cuts a plane
* in the transformed coordinate system parallel to the xy plane and with integer z.
* In each candidate graphics point, the "point" field contains global coordinates, the
* "a" field contains the distance along the x direction, and the "userData" field contains the
* z (height) index.
*
* The cut points are sorted by userData and a.
*
* @param pTransformm        => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     cvHatch_addTransformedCrossHatchCutPoints
(
HatchCollector &collector,
GraphicsPointArrayCP pBoundary,
const   Transform                *pTransform
)
    {
    GPA_TransformedHatchContext context;
    if (InitTransform (&context, pBoundary, collector, pTransform, NULL, NULL, s_maxLines))
        {
        jmdlGPA_TCH_collectCrossings (&context);
        jmdlGPA_TCH_sort (&context);
        }
    jmdlGPA_TCH_teardown (&context);
    }


/*---------------------------------------------------------------------------------**//**
* Compute point set difference (GPA0)-(GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     cvHatch_crossHatchDifference
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    int i0 = 0;
    int i1 = 0;
    /* We really expect y values to be on integer levels */
    int n0 = cvHatch_getCount (pGPA0);
    int n1 = cvHatch_getCount (pGPA1);
    const GraphicsPoint *pGP0base = cvHatch_getConstPtr (pGPA0, 0);
    const GraphicsPoint *pGP1base = cvHatch_getConstPtr (pGPA1, 0);
    const GraphicsPoint *pGP0 = NULL, *pGP1 = NULL;
    GraphicsPoint gpStart, gpEnd;
    bool    feed0, feed1;
    /* If odd counts, ignore the last.  Odd count is caller error, don't worry about preserving it. */
    int m0 = n0 & ~0x01;
    int m1 = n1 & ~0x01;
    cvHatch_empty (pGPAOut);

    if (m0 >= 2)
        {
        feed0 = true;
        feed1 = true;
        i0 = -2;
        i1 = -2;

        for (; i0 < m0 && i1 < m1;)
            {
            if (feed0)
                {
                i0 += 2;
                if (i0 >= m0)
                    break;
                pGP0 = pGP0base + i0;
                gpStart = pGP0[0];
                gpEnd   = pGP0[1];
                feed0 = false;
                }

            if (feed1)
                {
                i1 += 2;
                if (i1 >= m1)
                    break;
                pGP1 = pGP1base + i1;
                feed1 = false;
                }

            if (i0 >= m0 || i1 >= m1)
                break;
            if (gpStart.userData < pGP1[0].userData)
                {
                cvHatch_addGraphicsPointSegment (pGPAOut, &gpStart, &gpEnd);
                feed0 = true;
                }
            else if (gpStart.userData > pGP1[0].userData)
                {
                feed1 = true;
                }
            else /* (gpStart.userData == pGP1[0].userData) */
                {
                if (pGP1[0].a >= gpEnd.a)
                    {
                    /*  ----------00000000000000------------
                        --------------------------1111111111 */
                    cvHatch_addGraphicsPointSegment (pGPAOut, &gpStart, &gpEnd);
                    feed0 = true;
                    }
                else if (pGP1[0].a < gpStart.a)
                    {
                    if (pGP1[1].a <= gpStart.a)
                        {
                        /*  -------------00000000000000-------
                            -----111--------------------------
                        */
                        feed1 = true;
                        }
                    else if (pGP1[1].a <= gpEnd.a)
                        {
                        /*  -------------00000000000000-------
                            -----1111111111111????????????????  */
                        feed1 = true;
                        gpStart = pGP1[1];
                        }
                    else /* if (pGP1[1].a >= gpEnd.a) */
                        {
                        /*  -------------00000000000000-------
                            -----111111111111111111111111111??? */
                        feed0 = true;
                        }
                    }
                else
                    {
                    /*  ----------00000000000000------------
                        ------------------?????????????????? */
                    cvHatch_addGraphicsPointSegment (pGPAOut, &gpStart, &pGP1[0]);
                    if (pGP1[1].a >= gpEnd.a)
                        {
                        /*  ----------00000000000000------------
                            ------------------1111111111???????? */
                        feed0 = true;
                        }
                    else
                        {
                        /*  ----------00000000000000------------
                            ------------------111???????????????
                        */
                        gpStart = pGP1[1];
                        feed1 = true;
                        }
                    }
                }
            }
        }
    /* Add all trailing segments ... */
    if (i0 < m0)
        {
        /* Force the current segment out with the (possibly modified)
            local copy of the start. */
        cvHatch_addGraphicsPointSegment (pGPAOut, &gpStart, pGP0 + 1);
        pGP0 += 2;
        i0 += 2;
        for (; i0 < m0; i0 += 2, pGP0 += 2)
            {
            cvHatch_addGraphicsPointSegment (pGPAOut, pGP0, pGP0 + 1);
            }
        }
    }

typedef void (*BooleanSweepFunction)
    (
    GraphicsPointArray      *pGPAOut,
                int         *pOldSource,        /* Initialized to -1 by sweeper. Subsequent values changed only by the callback. */
            GraphicsPoint   *pOld,
    const   GraphicsPoint   *pNew,
                int         newSource   /* 0 if from pGPA0, 1 if from pGPA1, -1 if cleanup at end */
    );


/*---------------------------------------------------------------------------------**//**
* Boolean union logic for an old and new segment, forcing the old on out if no overlap.
* Assumes old start at or behind new start.
* @param pOldActive <=> flag to say if old data is alive.
* @param pOld <=> saved segment for merging.  TWO graphics points.
* @param pNew => new segment.  TWO graphics points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mergeFunc_union
(
GraphicsPointArray      *pGPAOut,
            int         *pOldSource,
        GraphicsPoint   *pOld,
const   GraphicsPoint   *pNew,
            int         newSource
)
    {
    if (!pNew)
        {
        if (*pOldSource >= 0)
            cvHatch_addGraphicsPointSegment (pGPAOut, pOld, pOld + 1);
        *pOldSource = -1;
        }
    else if (*pOldSource < 0)
        {
        /* Just save */
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew) < 0)
        {
        cvHatch_addGraphicsPointSegment (pGPAOut, pOld, pOld + 1);
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else
        {
        if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew + 1) < 0)
                pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Boolean intersection an old and new segment, forcing the old on out if no overlap.
* Assumes old start at or behind new start.
* @param pOldActive <=> flag to say if old data is alive.
* @param pOld <=> saved segment for merging.  TWO graphics points.
* @param pNew => new segment.  TWO graphics points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mergeFunc_intersection
(
GraphicsPointArray  *pGPAOut,
            int         *pOldSource,
        GraphicsPoint   *pOld,
const   GraphicsPoint   *pNew,
            int         newSource
)
    {
    if (!pNew)
        {
        *pOldSource = -1;
        }
    else if (*pOldSource < 0 || *pOldSource == newSource)
        {
        /* Just save */
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew) < 0)
        {
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else
        {
        if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew + 1) < 0)
            {
            cvHatch_addGraphicsPointSegment (pGPAOut, pNew, pOld + 1);
            pOld[0] = pOld[1];
            pOld[1] = pNew[1];
            *pOldSource = newSource;
            }
        else
            {
            cvHatch_addGraphicsPointSegment (pGPAOut, pNew, pNew + 1);
            pOld[0] = pNew[1];
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Compute point set booleans for a pair of hatch sets
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @param mergeFunc0 => function to progress to a segment in pGPA0, and for the residual
*                   (with no new data).
* @param mergeFunc1 => function to progress to a segment in pGAP1
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    cvHatch_booleanSweep
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1,
BooleanSweepFunction    mergeFunc
)
    {
    int i0 = 0;
    int i1 = 0;
    int n0 = cvHatch_getCount (pGPA0);
    int n1 = cvHatch_getCount (pGPA1);
    const GraphicsPoint *pGP0base = cvHatch_getConstPtr (pGPA0, 0);
    const GraphicsPoint *pGP1base = cvHatch_getConstPtr (pGPA1, 0);
    GraphicsPoint gpOld[2];
    GraphicsPoint const *pGP0;
    GraphicsPoint const *pGP1;
    int oldSource = -1;

    /* If odd counts, ignore the last.  Odd count is caller error, don't worry about preserving it. */
    int m0 = n0 & ~0x01;
    int m1 = n1 & ~0x01;
    cvHatch_empty (pGPAOut);
    for (i0 = i1 = 0; i0 < m0 || i1 < m1;)
        {
        pGP0 = pGP0base + i0;
        pGP1 = pGP1base + i1;
        if (i0 < m0 && i1 < m1)
            {
            if (jmdlGPA_compareHatchCandidates (pGP0, pGP1) < 0)
                {
                mergeFunc (pGPAOut, &oldSource, gpOld, pGP0, 0);
                i0 += 2;
                }
            else
                {
                mergeFunc (pGPAOut, &oldSource, gpOld, pGP1, 1);
                i1 += 2;
                }
            }
        else if (i0 < m0)
            {
            mergeFunc (pGPAOut, &oldSource, gpOld, pGP0, 0);
            i0 += 2;
            }
        else
            {
            mergeFunc (pGPAOut, &oldSource, gpOld, pGP1, 1);
            i1 += 2;
            }
        }
    mergeFunc (pGPAOut, &oldSource, gpOld, NULL, -1);
    }



/*---------------------------------------------------------------------------------**//**
* Compute point set union (GPA0) + (GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     cvHatch_crossHatchUnion
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    cvHatch_booleanSweep (pGPAOut, pGPA0, pGPA1, mergeFunc_union);
    }


/*---------------------------------------------------------------------------------**//**
* Compute point set intersection (GPA0) + (GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     cvHatch_crossHatchIntersection
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    cvHatch_booleanSweep (pGPAOut, pGPA0, pGPA1, mergeFunc_intersection);
    }

#endif

//! Return a curve vector (of type BOUNDARY_TYPE_None) containing hatch sticks.
void CurveVector::CreateXYHatch (
bvector<DSegment3d> &sticks,         //!< [out] computed hatch segments
bvector<HatchSegmentPosition> *segmentPosition, //!< [out] For each stick, description of hatch level and distance along.
CurveVectorCR        boundary,      //!< [in] boundary curves.
DPoint3dCR           startPoint,    //!< [in] Start point for hatch lines
double               angleRadians,  //!< [in] angle from X axis.
double               spacing,       //!< [in] spacing perpendicular to hatch direction
int                  selectRule     //!< 0 for parity rules, 1 for longest possible strokes (first to last crossings), 2 for leftmsot and rightmost of parity set.
)
    {
    Transform localToWorld;
    double c = cos (angleRadians);
    double s = sin (angleRadians);
    localToWorld.InitFromRowValues
        (
        c * spacing, 0, -s * spacing, startPoint.x,
        s * spacing, 0,  c * spacing, startPoint.y,
        0,      spacing, 0.0,         startPoint.z
        );

    HatchArray segments;
    cvHatch_addTransformedCrossHatchClipped (segments, boundary, localToWorld, selectRule);
    sticks.clear ();
    if (segmentPosition)
        segmentPosition->clear ();
    segments.AppendSegments (sticks, segmentPosition);
    }

//! Return a curve vector (of type BOUNDARY_TYPE_None) containing hatch sticks.
void CurveVector::CreateHatch (
bvector<DSegment3d> &sticks,
bvector<HatchSegmentPosition> *segmentPosition, //!< [out] For each stick, description of hatch level and distance along.
CurveVectorCR        boundary,      //!< [in] boundary curves.
TransformCR          worldToIntegerZPlanes, //< [in] Transform to space where each integer Z value is a cut plane.
int                  selectRule     //!< 0 for parity rules, 1 for longest possible strokes (first to last crossings), 2 for leftmsot and rightmost of parity set.
)
    {
    HatchArray segments;
    cvHatch_addTransformedCrossHatchClipped (segments, boundary, worldToIntegerZPlanes, selectRule);
    sticks.clear ();
    if (segmentPosition)
        segmentPosition->clear ();
    segments.AppendSegments (sticks, segmentPosition);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
