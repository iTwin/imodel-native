/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int     compareDoubles
(
double  y1,
double  y2
)
    {
    if (y1 == y2)
        return (0);
    else if (y1<y2)
        return (1);
    else
        return(-1);
    }


/*----------------------------------------------------------------------+
project pXYZ1 on segment pXYZ0..pXYZ2.
If projection is clearly within the segment, return distance from pXYZ1 to projection.
However, if projection fails or is outside 0..1 in parameter space of the segment,
    return distance to segment midpoint.
@param pXYZ0 IN segment start
@parma pXYZ1 IN off-segment point
@param pXYZ2 IN segment end
+----------------------------------------------------------------------*/
static double chordDeviation
(
DPoint3d *pXYZ0,
DPoint3d *pXYZ1,
DPoint3d *pXYZ2
)
    {
    DPoint3d xyzMid;
    DVec3d vector01, vector02;
    double s;
    static double sMin = 0.00001;
    static double sMax = 0.99999;
    vector01.DifferenceOf (*pXYZ1, *pXYZ0);
    vector02.DifferenceOf (*pXYZ2, *pXYZ0);
    if (DoubleOps::SafeDivide (s,
                vector01.DotProduct (vector02),
                vector02.DotProduct (vector02), 0.0)
        && s >= sMin
        && s <= sMax
        )
        {
        xyzMid.Interpolate (*pXYZ0, s, *pXYZ2);
        }
    else
        {
        xyzMid.Interpolate (*pXYZ0, 0.5, *pXYZ2);
        }
    return xyzMid.Distance (*pXYZ1);
    }

/*----------------------------------------------------------------------+
Examine points following index baseCount
Find the max deviation of any point from the midpoint of the segment
    between its neighbors.
+----------------------------------------------------------------------*/
static double maxChordDeviationInArrayTail
(
bvector<DPoint3d> const& xyzArray,
size_t baseCount
)
    {
    double dMax, di;
    ptrdiff_t i0, numTest;
    ptrdiff_t count = xyzArray.size ();
    ptrdiff_t baseIndex = baseCount - 1;
    numTest = count - baseCount - 1;
    dMax = 0.0;
    DPoint3d xyz0, xyz1, xyz2;
    for (i0 = 0; i0 < numTest; i0++)
        {
        xyz0 = xyzArray[baseIndex + i0];
        xyz1 = xyzArray[baseIndex + i0 + 1];
        xyz2 = xyzArray[baseIndex + i0 + 2];
        di = chordDeviation (&xyz0, &xyz1, &xyz2);
        if (di > dMax)
            dMax = di;
        }
    return dMax;
    }

/*----------------------------------------------------------------------+
@param pCurvePoints OUT points on curve
@param pSurfacePoints OUT points on surface
@param curveTol IN absolute tolerance on curve.
@param surfaceTol IN absolute tolerance on surface.
@param minPoints IN minimum number of points.  Evaluation begins with this number of
        points at uniform parameter steps.
@param curve IN paramter curve.
@param surface IN target surface.
@param param0 IN start param on curve
@param param1 IN end param on curve
+----------------------------------------------------------------------*/
static StatusInt appendPCurveStrokes
(
bvector<DPoint3d> &curvePoints,    // <= evaluated points. MAY NOT BE NULL
bvector<DPoint3d>  &surfacePoints,  // <= evaluated points. MAY NOT BE NULL
double          curveTol,               // => Curve-space tolerance.
double          surfaceTol,             // => Surface-space tolerance.
int             minPoints,              // => minimum number of points.  Evaluation begins with
                                        //     this number of points at uniform parameter steps.
const MSBsplineCurve  *curve,           // => curve structure
const MSBsplineSurface *surface,        // => surface structure
double param0,                          // => start parameter on curve
double param1                           // => end parameter on curve
)
    {
    StatusInt stat = ERROR;
    ptrdiff_t baseCount = surfacePoints.size ();
    BeAssert(baseCount == curvePoints.size());
    DPoint3d curvePoint0, surfacePoint0, curvePoint1, surfacePoint1;
    double delta = param1 - param0;
    double u, u0, u1;
    double baseStep, localStep;
    int i, j, numJ, newNumJ;
    bool    bAddBasePoint;

    static double sStartFactor = 0.00001;
    double startTolSurface;
    double startTolCurve;
    double fCurve, fSurface, fMax, countFactor;
    static double sDefaultCurveTol = 1.0e-3;
    static double sDefaultSurfaceTol = 1.0e-7;
    static int sMaxJ = 2048;
    static double sMinParamStep = 1.0e-5;
    double curveError, surfaceError;
    double curveError0, surfaceError0;
    if (curveTol <= 0.0)
        curveTol = sDefaultCurveTol;
    if (surfaceTol <= 0.0)
        surfaceTol = sDefaultSurfaceTol;

    startTolCurve   = sStartFactor * curveTol;
    startTolSurface = sStartFactor * surfaceTol;

    if (minPoints < 2)
        minPoints = 2;

    bspcurv_evaluateCurvePoint (&curvePoint1, NULL,
            const_cast <MSBsplineCurve*>(curve), param0);
    bspsurf_evaluateSurfacePoint (&surfacePoint1, NULL, NULL, NULL,
                curvePoint1.x, curvePoint1.y,
                const_cast <MSBsplineSurface*>(surface));

    bAddBasePoint = true;
    if (baseCount > 0)
        {
        curvePoint0 = curvePoints.at (baseCount - 1);
        surfacePoint0 = surfacePoints.at (baseCount - 1);
        if (   curvePoint0.Distance (curvePoint1) <= startTolCurve
            || surfacePoint0.Distance (surfacePoint1) <= startTolSurface)
            {
            bAddBasePoint = false;
            }
        }

    if (bAddBasePoint)
        {
        curvePoints.push_back (curvePoint1);
        surfacePoints.push_back (surfacePoint1);
        curvePoint0 = curvePoint1;
        surfacePoint0 = surfacePoint1;
        }


    baseStep = delta / (minPoints - 1);
    for (i = 1; i < minPoints; i++)
        {
        baseCount = surfacePoints.size ();
        u0 = param0 + (i - 1) * baseStep;
        u1 = param0 + i * baseStep;
        if (i == minPoints - 1)
            u1 = param1;
        numJ = 2;
        curveError0 = 1.0e20;
        surfaceError0 = 1.0e20;
        countFactor   = 1.0;
        for (;; surfaceError0 = surfaceError, curveError0 = curveError)
            {
            localStep = baseStep / numJ;
            for (j = 1; j <= numJ; j++)
                {
                u = u0 + j * localStep;
                bspcurv_evaluateCurvePoint (&curvePoint1, NULL,
                            const_cast <MSBsplineCurve*>(curve), u);
                bspsurf_evaluateSurfacePoint (&surfacePoint1, NULL, NULL, NULL,
                        curvePoint1.x, curvePoint1.y,
                        const_cast <MSBsplineSurface*>(surface));
                curvePoints.push_back (curvePoint1);
                surfacePoints.push_back (surfacePoint1);
                }
            curveError   = maxChordDeviationInArrayTail (curvePoints, baseCount);
            surfaceError = maxChordDeviationInArrayTail (surfacePoints, baseCount);
            if (curveError <= curveTol && surfaceError <= surfaceTol)
                {
                stat = SUCCESS;
                break;
                }
            else if (numJ >= sMaxJ || localStep < sMinParamStep)
                {
                stat = SUCCESS; // not really, but for now...
                break;
                }
            else
                {
                fCurve = curveError / curveTol;
                fSurface = surfaceError / surfaceTol;
                fMax = fCurve > fSurface ? fCurve : fSurface;
                countFactor = sqrt (fMax);
                if (countFactor > 16.0)
                    countFactor = 16.0;
                newNumJ = (int) ceil (countFactor * (double)numJ);
                if (newNumJ > sMaxJ)
                    newNumJ = sMaxJ;
                if (newNumJ == numJ)
                    newNumJ = numJ + 1;
                countFactor = (double)newNumJ / (double)numJ;
                numJ = newNumJ;
                curvePoints.resize (baseCount);
                surfacePoints.resize (baseCount);
                }
            }
        }
    return stat;
    }

/*----------------------------------------------------------------------+
@param curvePoints OUT points on curve
@param surfacePoints OUT points on surface
@param curveTol IN absolute tolerance on curve.
@param surfaceTol IN absolute tolerance on surface.
@param minPoints IN minimum number of points.  Evaluation begins with this number of
        points at uniform parameter steps.
@param xyzA IN start point
@param xyzB IN end point
@param surface IN target surface.
+----------------------------------------------------------------------*/
static StatusInt appendSegmentStrokes
(
bvector<DPoint3d>& curvePoints,         // <= evaluated points
bvector<DPoint3d>& surfacePoints,       // <= evaluated points
double          surfaceTol,             // => Surface-space tolerance.
int             minPoints,              // => minimum number of points.  Evaluation begins with
                                        //     this number of points at uniform parameter steps.
DPoint3dCR      xyzA,
DPoint3dCR      xyzB,
const MSBsplineSurface *surface
)
    {
    StatusInt stat = ERROR;
    size_t baseCount = surfacePoints.size();
    BeAssert(baseCount == curvePoints.size());
    DPoint3d curvePoint0, surfacePoint0, curvePoint1, surfacePoint1;
    double baseStep, localStep;
    int i, j, numJ, newNumJ;
    bool    bAddBasePoint;

    static double sStartFactor = 0.00001;
    double startTolSurface;
    double fSurface, countFactor;
    static double sDefaultSurfaceTol = 1.0e-7;
    static double startTolCurve = 1.0e-8;
    static int sMaxJ = 2048;
    static double sMinParamStep = 1.0e-5;
    double surfaceError;
    double surfaceError0;

    if (surfaceTol <= 0.0)
        surfaceTol = sDefaultSurfaceTol;

    startTolSurface = sStartFactor * surfaceTol;

    if (minPoints < 2)
        minPoints = 2;

    curvePoint1 = xyzA;
    bspsurf_evaluateSurfacePoint (&surfacePoint1, NULL, NULL, NULL,
                curvePoint1.x, curvePoint1.y,
                const_cast <MSBsplineSurface*>(surface));

    bAddBasePoint = true;
    if (baseCount > 0)
        {
        curvePoint0 = curvePoints[baseCount - 1];
        surfacePoint0 = surfacePoints[baseCount - 1];
        if (   curvePoint0.Distance (curvePoint1) <= startTolCurve
            || surfacePoint0.Distance (surfacePoint1) <= startTolSurface)
            {
            bAddBasePoint = false;
            }
        }

    if (bAddBasePoint)
        {
        curvePoints.push_back (curvePoint1);
        surfacePoints.push_back (surfacePoint1);
        curvePoint0 = curvePoint1;
        surfacePoint0 = surfacePoint1;
        }


    baseStep = 1.0 / (minPoints - 1);
    for (i = 1; i < minPoints; i++)
        {
        baseCount = surfacePoints.size();
        double u0 = (i - 1) * baseStep;
        double u1 = i * baseStep;
        if (i == minPoints - 1)
            u1 = 1.0;
        numJ = 2;

        surfaceError0 = 1.0e20;
        countFactor   = 1.0;
        for (;; surfaceError0 = surfaceError)
            {
            localStep = baseStep / numJ;
            for (j = 1; j <= numJ; j++)
                {
                double u = u0 + j * localStep;
                curvePoint1.Interpolate (xyzA, u, xyzB);
                bspsurf_evaluateSurfacePoint (&surfacePoint1, NULL, NULL, NULL,
                        curvePoint1.x, curvePoint1.y,
                        const_cast <MSBsplineSurface*>(surface));
                curvePoints.push_back (curvePoint1);
                surfacePoints.push_back (surfacePoint1);
                }
            surfaceError = maxChordDeviationInArrayTail (surfacePoints, baseCount);
            if (surfaceError <= surfaceTol)
                {
                stat = SUCCESS;
                break;
                }
            else if (numJ >= sMaxJ || localStep < sMinParamStep)
                {
                stat = SUCCESS; // not really, but for now...
                break;
                }
            else
                {
                fSurface = surfaceError / surfaceTol;
                countFactor = sqrt (fSurface);
                if (countFactor > 16.0)
                    countFactor = 16.0;
                newNumJ = (int) ceil (countFactor * (double)numJ);
                if (newNumJ > sMaxJ)
                    newNumJ = sMaxJ;
                if (newNumJ == numJ)
                    newNumJ = numJ + 1;
                countFactor = (double)newNumJ / (double)numJ;
                numJ = newNumJ;
                curvePoints.resize (baseCount);
                surfacePoints.resize (baseCount);
                }
            }
        }
    return stat;
    }

static bool    sbIsPreciseTrimEnabled = true;
/*----------------------------------------------------------------------+
@description Inquire the static enabling flag for precise trim.
@returns static flag value.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspsurf_isPreciseTrimEnabled
(
)
    {
    return sbIsPreciseTrimEnabled;
    }
/*----------------------------------------------------------------------+
@description Set the static enabling flag for precise trim.
@param bEnabled IN flag to save.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_setIsPreciseTrimEnabled
(
bool    bEnabled
)
    {
    sbIsPreciseTrimEnabled = bEnabled;
    }

struct SurfaceBreakContext
{
KnotData m_uKnotData, m_vKnotData;
bvector<double> m_uBreaks, m_vBreaks;
bvector<double> m_segmentFractions;
DPoint3d m_xyz0, m_xyz1;

SurfaceBreakContext (){}

void AppendFractions (double a0, double a1, bvector<double> const &aBreaks)
    {
    if (aBreaks.size () > 0)
        {
        double da = a1 - a0;
        double dsda;
        
        if (DoubleOps::SafeDivide (dsda, 1.0, da, 0.0))
            {
            for (double a : aBreaks)
                {
                double s = (a - a0) * dsda;
                if (s > 0.0 && s < 1.0)
                    m_segmentFractions.push_back (s);
                }
            }
        }
    }

bool AnnounceSurface (MSBsplineSurfaceCR surface)
    {
    static size_t s_targetMultiplicity = 2;
    size_t targetMultiplicityU = s_targetMultiplicity;
    size_t targetMultiplicityV = s_targetMultiplicity;
    if (surface.GetUOrder () <= 2)
        targetMultiplicityU = 1;
    if (surface.GetVOrder () <= 2)
        targetMultiplicityV = 1;
    m_uKnotData.LoadSurfaceUKnots (surface);
    m_vKnotData.LoadSurfaceVKnots (surface);
    m_uKnotData.CollectHighMultiplicityActiveKnots (targetMultiplicityU, false, m_uBreaks);
    m_vKnotData.CollectHighMultiplicityActiveKnots (targetMultiplicityV, false, m_vBreaks);
    return true;
    }

// Find u and v break fractions within xyz0..xyz1.
// sort the fractions from 0..1
// purge duplicates.
void AnalyzeSegment (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    m_xyz0 = xyz0;
    m_xyz1 = xyz1;

    // clamp x and y to (possibly non-normalized!) u- and v-knot ranges 
    DRange1d uKnotRange, vKnotRange;
    m_uKnotData.GetActiveKnotRange(uKnotRange.low, uKnotRange.high);
    m_vKnotData.GetActiveKnotRange(vKnotRange.low, vKnotRange.high);
    m_xyz0.x = DoubleOps::Clamp(xyz0.x, uKnotRange.low, uKnotRange.high);
    m_xyz0.y = DoubleOps::Clamp(xyz0.y, vKnotRange.low, vKnotRange.high);
    m_xyz1.x = DoubleOps::Clamp(xyz1.x, uKnotRange.low, uKnotRange.high);
    m_xyz1.y = DoubleOps::Clamp(xyz1.y, vKnotRange.low, vKnotRange.high);
    m_xyz0.z = m_xyz1.z = 0.0;  // these are 2D curves, poles in the uv-knot domain

    m_segmentFractions.clear ();
    m_segmentFractions.push_back (0.0);
    m_segmentFractions.push_back (1.0);
    AppendFractions (m_xyz0.x, m_xyz1.x, m_uBreaks);
    AppendFractions (m_xyz0.y, m_xyz1.y, m_vBreaks);
    std::sort (m_segmentFractions.begin (), m_segmentFractions.end ());
    size_t numAccept = 1;
    static double s_fractionTolerance = 1.0e-8;
    double a = m_segmentFractions[0];
    for (size_t i = 0; i < m_segmentFractions.size (); i++)
        {
        if (m_segmentFractions[i] > a + s_fractionTolerance)
            {
            m_segmentFractions[numAccept++] = a = m_segmentFractions[i];
            }
        }
    // um .. push aside a final fraction "just before 1.0" 
    m_segmentFractions.resize (numAccept);
    if (m_segmentFractions.back () < 1.0)
        m_segmentFractions.back () = 1.0;
    }

bool GetSubSegment (size_t i, DPoint3dR xyz0, DPoint3dR xyz1)
    {
    if (i + 1 < m_segmentFractions.size ())
        {
        xyz0.Interpolate (m_xyz0, m_segmentFractions[i], m_xyz1);
        xyz1.Interpolate (m_xyz0, m_segmentFractions[i+1], m_xyz1);
        return true;
        }
    return false;
    }
};
/*----------------------------------------------------------------------+
@param curvePoints OUT points on curve
@param surfacePoints OUT points on surface
@param curveTol IN absolute tolerance on curve.
@param surfaceTol IN absolute tolerance on surface.
@param minPoints IN minimum number of points.  Evaluation begins with this number of
        points at uniform parameter steps.
@param pCurve IN parameter curve.
@param surface IN target surface.
@param param0 IN start param on curve
@param param1 IN end param on curve
+----------------------------------------------------------------------*/
static StatusInt bspsurf_appendPCurveStrokes
(
bvector<DPoint3d>& curvePoints,         // <= evaluated points
bvector<DPoint3d>& surfacePoints,       // <= evaluated points
double          curveTol,               // => Curve-space tolerance.
double          surfaceTol,             // => Surface-space tolerance.
int             minPoints,              // => minimum number of points.  Evaluation begins with
                                        //     this number of points at uniform parameter steps.
const MSBsplineCurve  *pCurve,           // => curve structure
const MSBsplineSurface *pSurface,        // => surface structure
double param0,                          // => start parameter on curve
double param1,                           // => end parameter on curve
SurfaceBreakContext &breakContext
)
    {
    if (pCurve->params.order == 2)
        {
        DPoint3d xyz0, xyz1;
        int status = SUCCESS;
        int numSeg = pCurve->params.numPoles - 1;
        if (pCurve->params.closed)
            numSeg++;
         
        for (int i0 = 0; SUCCESS == status && i0 < numSeg; i0++)
            {
            int i1 = (i0 + 1) % pCurve->params.numPoles;
            breakContext.AnalyzeSegment (pCurve->poles[i0], pCurve->poles[i1]);
            for (size_t i = 0; breakContext.GetSubSegment (i, xyz0, xyz1); i++)
                {
                status = appendSegmentStrokes (curvePoints,
                        surfacePoints,
                        surfaceTol, 3,
                        xyz0, xyz1,
                        pSurface
                        );
                }
            }
        return status;
        }
    else
        {
        return appendPCurveStrokes (curvePoints, surfacePoints, curveTol, surfaceTol, minPoints,
                    pCurve, pSurface, param0, param1);
        }
    }

Public GEOMDLLIMPEXP StatusInt bspsurf_appendPCurveStrokes
(
EmbeddedDPoint3dArray *pCurvePoints,    // <= evaluated points. MAY NOT BE NULL
EmbeddedDPoint3dArray *pSurfacePoints,  // <= evaluated points. MAY NOT BE NULL
double          curveTol,               // => Curve-space tolerance.
double          surfaceTol,             // => Surface-space tolerance.
int             minPoints,              // => minimum number of points.  Evaluation begins with
                                        //     this number of points at uniform parameter steps.
const MSBsplineCurve  *pCurve,           // => curve structure
const MSBsplineSurface *pSurface,        // => surface structure
double param0,                          // => start parameter on curve
double param1                           // => end parameter on curve
)
    {
    SurfaceBreakContext breakContext;
    breakContext.AnnounceSurface (*pSurface);
    
    return bspsurf_appendPCurveStrokes (*pCurvePoints, *pSurfacePoints, curveTol, surfaceTol, minPoints,
                pCurve, pSurface, param0, param1, breakContext);
    }


/*----------------------------------------------------------------------+
+----------------------------------------------------------------------*/
static void copyEmbeddedDPoint3dArray
(
EmbeddedDPoint3dArray *pArray,  // => array to copy
DPoint3d **ppBuffer,             // <= allocated memory
int      *pNumOut
)
    {
    int numOut = jmdlEmbeddedDPoint3dArray_getCount (pArray);
    if (ppBuffer != NULL)
        {
        *ppBuffer = NULL;
        *pNumOut = 0;
        if (numOut > 0)
            {
            *ppBuffer = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (numOut * sizeof (DPoint3d)));
            if (*ppBuffer != NULL)
                {
                *pNumOut = numOut;
                memcpy (*ppBuffer,
                    jmdlEmbeddedDPoint3dArray_getPtr (pArray, 0),
                    numOut * sizeof (DPoint3d));
                }
            }
        }
    }

/*----------------------------------------------------------------------+
+----------------------------------------------------------------------*/
static void copyEmbeddedDPoint3dArrayToDPoint2dBuffer
(
EmbeddedDPoint3dArray *pArray,  // => array to copy
DPoint2d **ppBuffer,             // <= buffer allocated from BSIBaseGeom::Malloc
int      *pNumOut
)
    {
    int numOut = jmdlEmbeddedDPoint3dArray_getCount (pArray);
    int i;
    if (ppBuffer != NULL)
        {
        *ppBuffer = NULL;
        *pNumOut = 0;
        if (numOut > 0)
            {
            DPoint2d *pDest = *ppBuffer =
                    static_cast<DPoint2d *>(BSIBaseGeom::Malloc (numOut * sizeof (DPoint2d)));
            DPoint3d *pSource = jmdlEmbeddedDPoint3dArray_getPtr (pArray, 0);
            if (pDest != NULL)
                {
                *pNumOut = numOut;
                for (i = 0; i < numOut; i++)
                    {
                    pDest[i].x = pSource[i].x;
                    pDest[i].y = pSource[i].y;
                    }
                }
            }
        }
    }



static StatusInt bspsurf_strokeSingleTrimLoop
(
BsurfBoundary   *pDestBoundary,
BsurfBoundary   *pSourceBoundary,
double     curveTol,
double     surfaceTol,
int        minPoints,
const MSBsplineSurface *pSurface,
SurfaceBreakContext &breakContext
)
    {
    TrimCurve *pCurrTrim;
    StatusInt stat = ERROR;
    bvector<DPoint3d> curvePoints;
    bvector<DPoint3d> surfacePoints;

    int numCurve = 0;

    for (pCurrTrim = pSourceBoundary->pFirst; NULL != pCurrTrim; pCurrTrim = pCurrTrim->pNext)
        {
        numCurve++;
        if (SUCCESS != bspsurf_appendPCurveStrokes (curvePoints, surfacePoints,
                    curveTol, surfaceTol, minPoints,
                    &pCurrTrim->curve, pSurface, 0.0, 1.0, breakContext))
            goto cleanup;
        }
    stat = SUCCESS;

    if (pDestBoundary->points)
        BSIBaseGeom::Free (pDestBoundary->points);
    copyEmbeddedDPoint3dArrayToDPoint2dBuffer (&curvePoints, &pDestBoundary->points, &pDestBoundary->numPoints);

cleanup:
    return stat;
    }


/*----------------------------------------------------------------------+
@param pDestBoundary IN destination boundary.  Prior linear loop is freed.
            Count and points are assigned.  Trim list is NOT addressed.
@param pSourceBoundary IN source boundary.  Trim list is addressed.  Linear loop
            is not addressed through pSourceBoundary.
@param curveTol IN tolerance on curves
@param surfaceTol IN tolerance on surface
@param minPoints IN min points per curve.
@param surface IN target surface.
@returns SUCCESS if all trims evaluated.  If any evaluation fails,
        evaluation halts without changing the pDestBoundary.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_strokeSingleTrimLoop
(
BsurfBoundary   *pDestBoundary,
BsurfBoundary   *pSourceBoundary,
double     curveTol,
double     surfaceTol,
int        minPoints,
const MSBsplineSurface *pSurface
)
    {
    SurfaceBreakContext breakContext;
    breakContext.AnnounceSurface (*pSurface);
    return bspsurf_strokeSingleTrimLoop (pDestBoundary, pSourceBoundary, curveTol, surfaceTol, minPoints,
                    pSurface, breakContext);
    }

/*----------------------------------------------------------------------+
@param pSurface IN OUT surface whose exact trims are to be restroked.
@param curveTol IN tolerance on curves.
@param surfaceTol IN tolerance on surface
@param surface IN target surface.
@remark if both tolerances are negative, default values are supplied.
@returns SUCCESS if all trims evaluated.  If any evaluation fails, the surface
        will have a mixture of old and new polyline loops.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_restrokeTrimLoops
(
MSBsplineSurface    *pSurface,
double     curveTol,
double     surfaceTol
)
    {
    int boundaryIndex;
    int numError = 0;
    static double sCurveTol = 0.01;
    static double sSurfaceRelTol = 1.0e-4;

    SurfaceBreakContext breakContext;
    breakContext.AnnounceSurface (*pSurface);
    if (curveTol < 0.0 && surfaceTol < 0.0)
        {
        curveTol = sCurveTol;
        DRange3d range;
        pSurface->GetPoleRange (range);
        surfaceTol = sSurfaceRelTol * range.low.Distance (range.high);
        }

    for (boundaryIndex = 0; boundaryIndex < pSurface->numBounds; boundaryIndex++)
        {
        if (SUCCESS != bspsurf_strokeSingleTrimLoop
                            (
                            &pSurface->boundaries[boundaryIndex],
                            &pSurface->boundaries[boundaryIndex],
                            curveTol, surfaceTol, 2, pSurface, breakContext))
            numError++;
        }
    return numError == 0 ? SUCCESS : ERROR;
    }

/*----------------------------------------------------------------------+
Copy non-boundary data bitwise from source header to dest header.
Build reevaluated boundaries (lines) with no trim curves in the dest.
This is used by bspmesh, which needs trim boundaries at the same tolerance
as it is using for faceting the body of the face.
@param pTempSurface OUT surface to use for temporary operation.
@param pSourceSurface IN original surface.
@param curveTol IN tolerance for parameter space evaluation of curves.
@param surfaceTol IN tolernace for surface error.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_openTempTrimmedSurface
(
MSBsplineSurface    *pTempSurface,
MSBsplineSurface    *pSourceSurface,
double     curveTol,
double     surfaceTol
)
    {
    int boundaryIndex;
    int numBounds = pSourceSurface->numBounds;
    int numError = 0;
    *pTempSurface = *pSourceSurface;
    pTempSurface->boundaries = NULL;
    if (numBounds > 0)
        pTempSurface->boundaries = (BsurfBoundary*)BSIBaseGeom::Calloc (numBounds, sizeof (BsurfBoundary));
    for (boundaryIndex = 0; boundaryIndex < numBounds; boundaryIndex++)
        {
        if (SUCCESS != bspsurf_strokeSingleTrimLoop
                    (
                    &pTempSurface->boundaries[boundaryIndex],
                    &pSourceSurface->boundaries[boundaryIndex],
                    curveTol,
                    surfaceTol,
                    2,
                    pTempSurface
                    ))
            numError++;
        }
    return numError == 0 ? SUCCESS : ERROR;
    }

/*----------------------------------------------------------------------+
Free boundary data in surface.  NULL out remaining parts (i.e. pole pointers
    that were copied by bspcurv_openTempTrimmedSurface)
@param pTempSurface IN OUT surface to close.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_closeTempTrimmedSurface
(
MSBsplineSurface    *pTempSurface
)
    {
    for (int boundaryIndex = 0; boundaryIndex < pTempSurface->numBounds; boundaryIndex++)
        {
        BSIBaseGeom::Free (pTempSurface->boundaries[boundaryIndex].points);
        }
    BSIBaseGeom::Free (pTempSurface->boundaries);
    pTempSurface->Zero();
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
Stroke a parameter space curve and map to surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_strokePCurve
(
DPoint3d **ppCurveBuffer,    // <= evaluated points. MAY NOT BE NULL
DPoint3d **ppSurfaceBuffer,    // <= evaluated points. MAY NOT BE NULL
int       *pNumPoints,                // <= number evaluated points
double     curveTol,               // => Curve-space tolerance.
double     surfaceTol,             // => Surface-space tolerance.
int        minPoints,              // => minimum number of points.  Evaluation begins with
                                        //     this number of points at uniform parameter steps.
const MSBsplineCurve  *curve,           // => curve structure
const MSBsplineSurface *surface,        // => surface structure
double param0,                          // => start parameter
double param1                           // => end parameter
)
    {
    bvector<DPoint3d> curvePoints;
    bvector<DPoint3d> surfacePoints;

    SurfaceBreakContext breakContext;
    breakContext.AnnounceSurface (*surface);

    StatusInt status = bspsurf_appendPCurveStrokes
                (
                curvePoints, surfacePoints,
                curveTol, surfaceTol, minPoints,
                curve, surface, param0, param1, breakContext
                );

    size_t numCurvePoints   = curvePoints.size ();
    size_t numSurfacePoints = surfacePoints.size ();

    if (   SUCCESS == status
        && numCurvePoints == numSurfacePoints
        && numCurvePoints > 0)
        {
        copyEmbeddedDPoint3dArray (&curvePoints,   ppCurveBuffer,   pNumPoints);
        copyEmbeddedDPoint3dArray (&surfacePoints, ppSurfaceBuffer, pNumPoints);
        }

    return status;
    }

/*----------------------------------------------------------------------+
@param pSurface IN subject surface
@param pNumLinearBoundaries OUT number of polyline loops.
@param pNumPCurveLoops OUT number of precise curves.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_countLoops
(
const MSBsplineSurface *pSurface,
int *pNumLinearBoundaries,
int *pNumPCurveLoops
)
    {
    *pNumPCurveLoops = 0;
    *pNumLinearBoundaries = pSurface->numBounds;
    for (int boundaryIndex = 0; boundaryIndex < pSurface->numBounds; boundaryIndex++)
        {
        if (pSurface->boundaries[boundaryIndex].pFirst != NULL)
            *pNumPCurveLoops += 1;
        }
    }


/*----------------------------------------------------------------------+
Allocate a trim curve link
Fill in its curve point.
Insert as the tail of CYLCIC linked list.
@param ppHead IN OUT head link
@param pCurve IN curve pointer to place in new link.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_allocateAndInsertCyclic
(
TrimCurve **ppHead,
MSBsplineCurve *pCurve
)
    {
    TrimCurve* pLink= (TrimCurve *) BSIBaseGeom::Calloc (1, sizeof (TrimCurve));
    TrimCurve *pHead = *ppHead;
    if (pCurve)
        pLink->curve = *pCurve;
    pLink->pPrevious = pLink->pNext = pLink;

    DPoint3d xyz0, xyz1;
    bspcurv_evaluateCurvePoint (&xyz0, NULL,
                const_cast <MSBsplineCurve*>(pCurve), 0.0);
    bspcurv_evaluateCurvePoint (&xyz1, NULL,
                const_cast <MSBsplineCurve*>(pCurve), 1.0);

    bspcurv_openCurve (&pLink->curve, &pLink->curve, 0.0);
    if (NULL == pHead)
        {
        *ppHead = pLink;
        }
    else
        {
        TrimCurve *pTail    = pHead->pPrevious;
        pTail->pNext        = pLink;
        pLink->pPrevious    = pTail;
        pLink->pNext        = pHead;
        pHead->pPrevious    = pLink;
        }
    }

/*----------------------------------------------------------------------+
@description Split the curve at C1 discontinuities.  Pass each segment
to bspTrimCurve_allocateAndInsertCyclic.
The segments (or copy of linear curve) are retained in the trim.  Caller is
always responsible for freeing the original curve.
@param ppHead IN OUT head link
@param pCurve IN curve pointer to place in new link.
@param bPreserveLinear IN true to copy linear curves directly.  (Linear curve segments
    are handled efficiently by strokers.)
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_allocateAndInsertCyclicC1Segments
(
TrimCurve **ppHead,
MSBsplineCurve *pCurve,
bool    bPreserveLinear
)
    {
    MSBsplineCurve *pCurveArray;
    int numCurve;
    if (bPreserveLinear && pCurve->params.order == 2)
        {
        MSBsplineCurve copyCurve;
        bspcurv_copyCurve (&copyCurve, pCurve);
        bspTrimCurve_allocateAndInsertCyclic (ppHead, &copyCurve);
        }
    else
        {
        if (SUCCESS == bsputil_segmentC1DiscontinuousCurve (&pCurveArray, &numCurve, pCurve))
            {
            for (int i = 0; i < numCurve; i++)
                {
                bspTrimCurve_allocateAndInsertCyclic (ppHead, &pCurveArray[i]);
                }
            BSIBaseGeom::Free (pCurveArray);
            }
        }
    }

/*----------------------------------------------------------------------+
Reduce a cyclic trim curve list to linear list.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_breakCyclicList
(
TrimCurve **ppHead
)
    {
    TrimCurve *pHead = *ppHead;
    if (NULL != pHead)
        {
        TrimCurve *pTail = pHead->pPrevious;
        pHead->pPrevious = NULL;
        if (NULL != pTail)
            pTail->pNext = NULL;
        }
    }

/*----------------------------------------------------------------------+
Free the links and curves in a TrimCurve chain.
Both cyclic and linear chains are allowed.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_freeList
(
TrimCurve **ppHead
)
    {
    bspTrimCurve_breakCyclicList (ppHead);
    TrimCurve *pHead = *ppHead;
    TrimCurve *pNext = NULL;
    while (pHead)
        {
        pNext = pHead->pNext;
        bspcurv_freeCurve (&pHead->curve);
        pHead->pNext = pHead->pPrevious = NULL;
        BSIBaseGeom::Free (pHead);
        pHead = pNext;
        }
    *ppHead = NULL;
    }

/*----------------------------------------------------------------------+
@description remove PCurve trim but leave polylines unchanged.
@param pSurf IN OUT subject surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_removePCurveTrim
(
MSBsplineSurface *pSurf
)
    {
    BsurfBoundary *pBoundary;

    for (int i = 0; i < pSurf->numBounds; i++)
        {
        pBoundary = &pSurf->boundaries[i];
        bspTrimCurve_freeList (&pBoundary->pFirst);
        pBoundary->pFirst = NULL;
        }
    }

/*----------------------------------------------------------------------+
@param pSurf IN OUT subject surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_freeBoundaries
(
MSBsplineSurface *pSurf
)
    {
    BsurfBoundary *pBoundary;

    for (int i = 0; i < pSurf->numBounds; i++)
        {
        pBoundary = &pSurf->boundaries[i];
        if (NULL != pBoundary->points)
            BSIBaseGeom::Free (pBoundary->points);
        bspTrimCurve_freeList (&pBoundary->pFirst);
        pBoundary->pFirst = NULL;
        pBoundary->points = NULL;
        pBoundary->numPoints = 0;
        }

    BSIBaseGeom::Free (pSurf->boundaries);
    pSurf->boundaries = NULL;
    pSurf->numBounds  = 0;
    }

/*----------------------------------------------------------------------+
Append all transformed copy of boundaries from the source surface to the destination.
@param out IN OUT destination surface. Non-boundary data is not altered.
@param in  IN source surface
@param pTransform IN transform (parameter space to parameter space) to apply to trim as copied.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_appendCopyBoundaries
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in,
Transform const *pTransform
)
    {
    int                 allocSize;
    BsurfBoundary       *inBndP, *outBndP, *endB;\
    int index0 = out->numBounds;

    if (in->numBounds == 0)
        return SUCCESS;

    if (out->boundaries == NULL)
        {
        allocSize = in->numBounds * sizeof(BsurfBoundary);
        if (NULL == (out->boundaries =
                     (BsurfBoundary*)BSIBaseGeom::Calloc (in->numBounds, sizeof(BsurfBoundary))))
            return ERROR;
        out->numBounds = in->numBounds;
        }
    else
        {
        allocSize = (out->numBounds + in->numBounds) * sizeof(BsurfBoundary);
        if (NULL == (out->boundaries =
                     (BsurfBoundary*)BSIBaseGeom::Realloc (out->boundaries, allocSize)))
            return ERROR;
        memset (&out->boundaries[out->numBounds], 0, in->numBounds * sizeof (BsurfBoundary));
        out->numBounds  += in->numBounds;
        }

    for (inBndP = endB = in->boundaries,
         outBndP = out->boundaries + index0,
         endB += in->numBounds;
         inBndP < endB; inBndP++, outBndP++)
        {
        TrimCurve   *inTrimP, *outTrimP, *lastTrimP = NULL;

        allocSize = inBndP->numPoints * sizeof(DPoint2d);
        if (outBndP->points == NULL)
            {
            if (NULL == (outBndP->points = (DPoint2d*)BSIBaseGeom::Malloc (allocSize)))
                return ERROR;
            }

        outBndP->numPoints = inBndP->numPoints;
        memcpy (outBndP->points, inBndP->points, allocSize);
        if (pTransform)
            pTransform->Multiply (outBndP->points, outBndP->points, outBndP->numPoints);

        // Copy trim boundaries.
        for (inTrimP = inBndP->pFirst; NULL != inTrimP; inTrimP = inTrimP->pNext)
            {
            StatusInt       status;
            if (NULL == (outTrimP = (TrimCurve*)BSIBaseGeom::Calloc (1, sizeof (TrimCurve))))
                return ERROR;

            if (SUCCESS != (status = bspcurv_copyCurve (&outTrimP->curve, &inTrimP->curve)))
                return status;

            if (pTransform)
                bspcurv_transformCurve (&outTrimP->curve, &outTrimP->curve, pTransform);

            if (NULL == outBndP->pFirst)
                {
                outBndP->pFirst = lastTrimP  =  outTrimP;
                }
            else
                {
                _Analysis_assume_(lastTrimP != nullptr);
                outTrimP->pPrevious = lastTrimP;
                lastTrimP->pNext = outTrimP;
                lastTrimP = outTrimP;
                }
            }

        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
@description Transform both linear and curve data in a single boundary.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformBoundary
(
BsurfBoundary *pBoundary,
Transform const *pTransform
)
    {
    TrimCurve *pCurrTrim;

    if (pBoundary == NULL)
        return;

    if (pBoundary->numPoints > 0 && NULL != pBoundary->points)
        pTransform->Multiply (pBoundary->points, pBoundary->points, pBoundary->numPoints);

    if (NULL != pBoundary->pFirst)
        {
        for (pCurrTrim = pBoundary->pFirst; NULL != pCurrTrim; pCurrTrim = pCurrTrim->pNext)
            {
            bspcurv_transformCurve (&pCurrTrim->curve, &pCurrTrim->curve, pTransform);
            }
        }
    }
/*----------------------------------------------------------------------+
@description Transform all trim data (linear, curve) in place.  This is a
   parameter-space to parameter space transformation.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformAllBoundaries
(
MSBsplineSurface *pSurface,
Transform const  *pTransform
)
    {
    bspsurf_transformBoundaries (pSurface, pTransform, 0, pSurface->numBounds);
    }

/*----------------------------------------------------------------------+
@description Transform selected trim data (linear, curve) in place.  This is a
   parameter-space to parameter space transformation.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
@param index0 IN first index to be transformed
@param num    IN number to transform
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformBoundaries
(
MSBsplineSurface *pSurface,
Transform const  *pTransform,
int             index0,
int             num
)
    {
    int index1 = index0 + num;
    if (index0 < 0)
        index0 = 0;
    if (index1 >= pSurface->numBounds)
        index1 = pSurface->numBounds;
    for (int i = index0; i < index1; i++)
        {
        bspsurf_transformBoundary (&pSurface->boundaries[i], pTransform);
        }
    }

/*----------------------------------------------------------------------+
Find the parity classification of a point with respect to a loop, using a single specified
   dirction and returning failure if the scan line logic is too complicated to reach a conclusion.
@returns false if unable to classify because of tangency or complex vertex neighborhood.
@param ppHEAD IN head of curve chain.
@param parity OUT parity result.
@param pParamArray IN OUT scratch array.
@param ax IN test point x coordinate
@param ay IN test point y coordinate.
@param ux IN scan direction x component.   Assumed unit vector.
@param uy IN scan direction y component    Assumed unit vector.
+----------------------------------------------------------------------*/
static bool bspTrimCurve_attmptPointOutOnInLoopScan
(
TrimCurve *pHead,
bvector<double> &paramArray,
bvector<DPoint3d> &pointArray,
double ax,
double ay,
double ux,
double uy,
int &parity
)
    {
    static double abstol = 1.0e-8;
    static double paramTol = 1.0e-6;
    DPoint3d curvePoint;
    int numLeft = 0, numRight = 0;
    double nx = -uy;
    double ny = ux;
    DPlane3d plane = DPlane3d::FromOriginAndNormal (ax, ay, 0, nx, ny, 0);
    TrimCurve *pCurr;
    parity = 0;
    // Count simple crossings to left and right.
    // Any ON is immediate true exit
    // Any awkward crossing (endpoint, double root) is immediate false exit.
    for (pCurr = pHead; NULL != pCurr; pCurr = pCurr->pNext)
        {
        paramArray.clear ();
        pCurr->curve.AddPlaneIntersections(&pointArray, &paramArray, plane);
        size_t   numParam = paramArray.size ();
        // ASSUME parameters are sorted ....
        double currParam, prevParam = 0.0;
        for (size_t i = 0; i < numParam; i++, prevParam = currParam)
            {
            currParam = paramArray[i];
            curvePoint = pointArray[i];
            if (   fabs (curvePoint.x - ax) < abstol
                || fabs (curvePoint.y - ay) < abstol)
                {
                parity = 0;
                return true;
                }
            // FAIL if scan line touched endpoint ...
            if (fabs (currParam) < paramTol)
                return false;
            if (fabs (currParam) > 1.0 - paramTol)
                return false;
            double u = (curvePoint.x - ax) * ux + (curvePoint.y - ay) * uy;
            // Count as if single roots ... Will quit later of double detected...
            if (u > abstol)
                numRight++;
            else if (u < -abstol)
                numLeft++;
            else
                return false;
            }


        // Look at INTERVALS.   Quit if any interval is SHORT or has same altitude sign as neighbor.
        //
        if (numParam > 0)
            {
            double hA = 0.0, hB = 0.0;
            double u0, u1;
            u0 = 0.0;
            for (size_t i = 0; i <= numParam; i++, prevParam = u1)
                {
                // Evalaute curve at midpoint of an interval ...
                if (i == 0)
                    {
                    u0 = 0.0;
                    u1 = paramArray[i];
                    }
                else if (i == numParam)
                    {
                    u0 = prevParam;
                    u1 = 1.0;
                    }
                else
                    {
                    u0 = prevParam;
                    u1 = paramArray[i];
                    }
                // Sort error or double root ...
                if (u1 < u0 + abstol)
                    return false;
                double uu = 0.5 * (u0 + u1);
                bspcurv_evaluateCurvePoint (&curvePoint, NULL,
                    const_cast <MSBsplineCurve*>(&pCurr->curve), uu);
                hB = (curvePoint.x - ax) * nx + (curvePoint.y - ay) * ny;
                if (fabs(hB) < abstol)
                    return false;
                if (i == 0)
                    hA = hB;
                else if (hA * hB > 0.0)
                    return false;       // double root.
                hA = hB;
                }
            }
        }

    // Number of left and right crossings summed for all curves should be both even or both odd ....
    if ((numLeft + numRight) & 0x01)
        return false;

    parity = numLeft & 0x01 ? 1 : -1;
    return true;
    }

/*----------------------------------------------------------------------+
Find the parity classification of a point with respect to a loop.
@returns true if safely classified
@param pHead IN head of curve chain.
@param pParamArray IN OUT scratch array
@param ax IN test point coordinate.
@param ay IN test point coordinate
@param parity OUT 0 if ON any curve in the loop, 1 if inside by parity rules, -1 if outside.
+----------------------------------------------------------------------*/
static bool bspTrimCurve_pointOutOnInLoop
(
TrimCurve *pHead,
bvector<double>  &paramArray,
bvector<DPoint3d> &pointArray,
double ax,
double ay,
int &parity
)
    {
    static int sMaxTest = 10;
    int i;
    static double sTheta0 = 0.21389237834;
    static double sThetaFactor = 1.8234203;
    double theta = sTheta0;
    for (i = 0; i < sMaxTest; i++)
        {
        double ux = cos (theta);
        double uy = sin (theta);
        if (bspTrimCurve_attmptPointOutOnInLoopScan (pHead, paramArray, pointArray, ax, ay, ux, uy, parity))
            return true;
        theta *= sThetaFactor;
        }
    parity = 0;
    return false;
    }

/*----------------------------------------------------------------------+
@description test if a parametric point is within trim boundaries.  Uses precise trims first.
@param uvP IN parametric point.
@param boundP IN array of boundaries
@param numBounds IN number of boundaries
@param holeOrigin IN false inverts sense of parity logic.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsputil_pointInBounds
(
DPoint2d        *uvP,
BsurfBoundary   *boundaryArrayP,
int             numBounds,
bool            holeOrigin
)
    {
    int             crossings;
    int i;
    BsurfBoundary *currBoundaryP;
    bool    result  = false;
    if (numBounds == 0)
        return true;    /* maybe should check in range ? */
    bvector<double> paramArray;
    bvector<DPoint3d> pointArray;

    crossings = 0;
    for (i = 0; i < numBounds; i++)
        {
        currBoundaryP = &boundaryArrayP[i];
        if (NULL != currBoundaryP->pFirst)
            {
            int singleOutOnInLoop;
            if (!bspTrimCurve_pointOutOnInLoop (currBoundaryP->pFirst,
                            paramArray, pointArray,
                            uvP->x, uvP->y,
                            singleOutOnInLoop))
                goto cleanup;
            if (singleOutOnInLoop > 0)
                crossings++;
            else if (singleOutOnInLoop == 0)
                {
                crossings++;
                break;
                }
            }
        else
            {
            int parity = PolygonOps::PointPolygonParity (*uvP,
                        currBoundaryP->points, (size_t) currBoundaryP->numPoints, 1.0e-10);
            if (parity > 0)
                crossings++;
            else if (parity == 0)
                {
                crossings++;
                break;
                }
            }
        }

    result = ((crossings & 0x01) ? holeOrigin : ! holeOrigin);
cleanup:
    return result;
    }


/*---------------------------------------------------------------------------------**//**

+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspUtil_initializeBsurfBoundary
(
BsurfBoundary* pBBoundary
)
    {
    if (pBBoundary == NULL)
        return ERROR;

    pBBoundary->numPoints = 0;
    pBBoundary->points = NULL;
    pBBoundary->pFirst = NULL;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspsurf_intersectBoundariesWithUVLine
(
bvector<double>             *pFractionArray,
double                      value,
const MSBsplineSurface      *pSurface,
int                         horizontal
)
    {
    int             prev, curr, next, numSpans, bufSize, numPoints;
    double          scanHeight, near1;
    DPoint2d        *p;
    BsurfBoundary   *currBound, *endB;
    int numLinear, numPCurve;
    // static double sParamTol = 1.0e-8;

    scanHeight = value;
    near1 = 1.0 - fc_epsilon;
    if (scanHeight < fc_epsilon)
            scanHeight = fc_epsilon;
    if (scanHeight > near1)
            scanHeight = near1;

    pFractionArray->clear ();

    bspsurf_countLoops (pSurface, &numLinear, &numPCurve);
    if (   numPCurve > 0)
        {
        DPlane3d plane;
        bvector<double> curveParamArray;
        bvector<DPoint3d> curvePointArray;

        if (!pSurface->holeOrigin)
            {
            pFractionArray->push_back ( 0.0);
            pFractionArray->push_back ( 1.0);
            }
        if (horizontal)
            {
            plane.origin.Init (0.0, scanHeight, 0.0);
            plane.normal.Init (0.0, 1.0, 0.0);
            }
        else
            {
            plane.origin.Init (scanHeight,  0.0, 0.0);
            plane.normal.Init (1.0,       0.0, 0.0);
            }

        for (int i = 0; i < pSurface->numBounds; i++)
            {
            for (TrimCurve *pCurr = pSurface->boundaries[i].pFirst;
                    NULL != pCurr;
                    pCurr = pCurr->pNext)
                {
                curveParamArray.clear ();
                curvePointArray.clear ();
                pCurr->curve.AddPlaneIntersections (&curvePointArray, &curveParamArray, plane);
                size_t numCurveParam = curveParamArray.size ();
                for (size_t k = 0; k < numCurveParam; k++)
                    {
                    DPoint3d xyz = curvePointArray[k];
                    pFractionArray->push_back ( horizontal ? xyz.x : xyz.y);
                    }
                }
            }
        std::sort (pFractionArray->begin (), pFractionArray->end ());
        }
    else if (pSurface->numBounds > 0)
        {
        numSpans = bufSize = 0;
        near1 = 1.0 - fc_epsilon;
        if (!pSurface->holeOrigin)
            {
            pFractionArray->push_back ( 0.0);
            pFractionArray->push_back ( 1.0);
            }
        for (currBound=endB=pSurface->boundaries, endB += pSurface->numBounds;
             currBound < endB; currBound++)
            {
            p = currBound->points;
            numPoints = currBound->numPoints;

            /* Ignore trailing duplicates */
            for (;numPoints > 1 && p[numPoints-1].x == p[numPoints - 2].x &&
                                   p[numPoints-1].y == p[numPoints - 2].y; numPoints--);

            /* Ignore leading duplicates */
            for (;numPoints > 1 && p[0].x == p[1].x &&
                                   p[0].y == p[1].y; numPoints--, p++);

            if (numPoints < 2)
                {
                }
            else if (horizontal)
                {
                curr = compareDoubles (scanHeight, p[0].y);
                next = compareDoubles (scanHeight, p[1].y);
                for (int i=1; i < numPoints; i++)
                    {
                    prev = curr;
                    curr = next;
                    next = compareDoubles (scanHeight,
                                                p[(i+1) % (numPoints-1)].y);
                    if (curr)
                        {
                        if (curr*prev < 0)
                            {
                            double f;
                            double g;
                            if (DoubleOps::SafeDivide (f, p[i].x - p[i-1].x, p[i].y - p[i-1].y, 0.0))
                                {
                                g = p[i-1].x + (scanHeight - p[i-1].y) * f;
                                pFractionArray->push_back ( g);
                                }
                            }
                        }
                    else
                        {
                        if (prev*next < 0)
                            {
                            pFractionArray->push_back ( p[i].x);
                            }
                        else
                            {
                            curr = prev;
                            }
                        }
                    }
                }
             else
                {
                curr = compareDoubles (scanHeight, p[0].x);
                next = compareDoubles (scanHeight, p[1].x);
                for (int i=1; i < numPoints; i++)
                    {
                    prev = curr;
                    curr = next;
                    next = compareDoubles (scanHeight,
                                                p[(i+1) % (numPoints-1)].x);
                    if (curr)
                        {
                        if (curr*prev < 0)
                            {
                            double f;
                            DoubleOps::SafeDivide (f, p[i].y - p[i-1].y, p[i].x - p[i-1].x, 0.0);
                            double g = p[i-1].y + (scanHeight - p[i-1].x) * f;
                            pFractionArray->push_back ( g);
                            }
                        }
                    else
                        {
                        if (prev*next < 0)
                            {
                            pFractionArray->push_back ( p[i].y);
                            }
                        else
                            {
                            curr = prev;
                            }
                        }
                    }
                }
            }
        std::sort (pFractionArray->begin (), pFractionArray->end ());
        }
    else
        {
        pFractionArray->push_back ( 0.0);
        pFractionArray->push_back ( 1.0);
        }
    }
// EDL May 1 2013   holeOrigin== 0 is "normal" case with outer boundary active (e.g. with no trim loops)
void MSBsplineSurface::SetOuterBoundaryActive (bool active) {holeOrigin = active ? 0 : 1;}

bool MSBsplineSurface::IsOuterBoundaryActive () const {return holeOrigin == 0;}

bool MSBsplineSurface::AddTrimBoundary (bvector<DPoint2d> const &uvPoints)
    {
    if (uvPoints.size () == 0)
        return false;
    BsurfBoundary *pBoundary = (BsurfBoundary*)BSIBaseGeom::Malloc (sizeof (BsurfBoundary));
    bspUtil_initializeBsurfBoundary (pBoundary);
    int numPoints = pBoundary->numPoints = (int)uvPoints.size ();
    pBoundary->points = (DPoint2d*)BSIBaseGeom::Malloc (numPoints * sizeof (DPoint2d));
    memcpy (pBoundary->points, &uvPoints[0], numPoints * sizeof (DPoint2d));
    bsputil_addBoundaries (this, &pBoundary, 1);
    return true;
    }

bool MSBsplineSurface::AddTrimBoundary (bvector<DPoint3d> const &uvwPoints)
    {
    if (uvwPoints.size () == 0)
        return false;
    BsurfBoundary *pBoundary = (BsurfBoundary*)BSIBaseGeom::Malloc (sizeof (BsurfBoundary));
    bspUtil_initializeBsurfBoundary (pBoundary);
    int numPoints = pBoundary->numPoints = (int)uvwPoints.size ();
    pBoundary->points = (DPoint2d*)BSIBaseGeom::Malloc (numPoints * sizeof (DPoint2d));
    for (size_t i = 0, n = uvwPoints.size (); i < n; i++)
        pBoundary->points[i].Init (uvwPoints[i].x, uvwPoints[i].y);
    bsputil_addBoundaries (this, &pBoundary, 1);
    return true;
    }
static double s_setback = 0.0;
 void MSBsplineSurface::GetUVBoundaryLoops (bvector< bvector<DPoint2d> > &uvBoundaries, bool addOuterLoopIfActive) const
    {
    uvBoundaries.clear ();
    uvBoundaries.reserve ((size_t) numBounds);
    if (s_setback != 0.0)
        {
        uvBoundaries.push_back (bvector<DPoint2d> ());
        double uMin, uMax, vMin, vMax;
        GetKnotRange (uMin, uMax, BSSURF_U);
        uMin = uMin + s_setback;
        uMax = uMax - s_setback;
        GetKnotRange (vMin, vMax, BSSURF_V);
        vMin = vMin + s_setback;
        vMax = vMax - s_setback;
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMin));
        uvBoundaries.back ().push_back (DPoint2d::From (uMax, vMin));
        uvBoundaries.back ().push_back (DPoint2d::From (uMax, vMax));
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMax));
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMin));
        return;
        }
    if (addOuterLoopIfActive && (!holeOrigin || numBounds == 0))
        {
        uvBoundaries.push_back (bvector<DPoint2d> ());
        double uMin, uMax, vMin, vMax;
        GetKnotRange (uMin, uMax, BSSURF_U);
        GetKnotRange (vMin, vMax, BSSURF_V);
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMin));
        uvBoundaries.back ().push_back (DPoint2d::From (uMax, vMin));
        uvBoundaries.back ().push_back (DPoint2d::From (uMax, vMax));
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMax));
        uvBoundaries.back ().push_back (DPoint2d::From (uMin, vMin));
        }
    for (int i = 0; i < numBounds; i++)
        {
        int n = boundaries[i].numPoints;
        uvBoundaries.push_back (bvector <DPoint2d> ());
        for (int k = 0; k < n; k++)
            uvBoundaries.back ().push_back (boundaries[i].points[k]);
        }
    }

 void MSBsplineSurface::GetUVBoundaryLoops (bvector< bvector<DPoint2d> > &uvBoundaries, bool addOuterLoopIfActive, bool cleanupParity) const
    {
    uvBoundaries.clear ();
    if (!cleanupParity)
        {
        GetUVBoundaryLoops (uvBoundaries, addOuterLoopIfActive);
        }
    else
        {
        bvector< bvector<DPoint2d> > rawBoundaries;
        GetUVBoundaryLoops (rawBoundaries, addOuterLoopIfActive);
        vu_fixupLoopParity (uvBoundaries, rawBoundaries, 3, 1);
        }
    }

void MSBsplineSurface::FixupBoundaryLoopParity ()
    {
    bvector<bvector<DPoint2d> > loops;
    GetUVBoundaryLoops (loops, true, true);
    bspsurf_freeBoundaries (this);
    SetOuterBoundaryActive (false);
    for (size_t i = 0; i < loops.size (); i++)
        AddTrimBoundary (loops[i]);
    }



CurveVectorPtr MSBsplineSurface::GetUVBoundaryCurves (bool addOuterLoopIfActive, bool preferCurves) const
    {
    CurveVectorPtr out = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    if (addOuterLoopIfActive && !holeOrigin)
        out->push_back (
              ICurvePrimitive::CreateChildCurveVector (
                CurveVector::CreateRectangle (0.0, 0.0, 1.0, 1.0, 0.0, CurveVector::BOUNDARY_TYPE_Outer)));
    for (int i = 0; i < numBounds; i++)
        {
        if (preferCurves && boundaries[i].pFirst != NULL)
            {
            CurveVectorPtr loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            TrimCurve *pCurrTrim;
            for (pCurrTrim = boundaries[i].pFirst; NULL != pCurrTrim; pCurrTrim = pCurrTrim->pNext)
                {
                loop->push_back (ICurvePrimitive::CreateBsplineCurve (pCurrTrim->curve));
                }
            out->push_back (ICurvePrimitive::CreateChildCurveVector (loop));
            }
        else
            {
            out->push_back (
                ICurvePrimitive::CreateChildCurveVector (
                    CurveVector::CreateLinear (boundaries[i].points, (size_t)boundaries[i].numPoints, CurveVector::BOUNDARY_TYPE_Outer, false)));
            }
        }
    return out;
    }

// recursively find references (Ptr!!) to loops ...
// bsurfs carry unannotated parity lists.   So all loops have to be blurred into one parity list.
// hypothetically overlaps within unions would have incorrect parity interpretation.  Caller is responsible for making unions quasidisjoint.
void CollectInnerOuterLoopRefs (bvector<CurveVectorPtr> &loops, CurveVectorR source)
    {
    if (source.IsClosedPath ())
        loops.push_back (&source);
    else if (source.IsPhysicallyClosedPath ())
        {
        loops.push_back (&source);
        loops.back ()->SetBoundaryType (
            loops.size () == 1 ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Outer
            );
        }
    else if (source.IsParityRegion () || source.IsUnionRegion ())
        {
        for (size_t i = 0; i < source.size (); i++)
            {
            CurveVectorPtr child = source[i]->GetChildCurveVectorP ();
            if (child.IsValid ())
                CollectInnerOuterLoopRefs (loops, *child);
            }
        }
    }

StatusInt bspsurf_transferTrimToSurface (MSBsplineSurface *surfaceP, TrimCurve **trimLoop);

bvector<DPoint3d> const * GetSingleLinearLoop (CurveVectorCR cv)
    {
    if (cv.size() == 1)
        return cv.at(0)->GetLineStringCP ();
    return nullptr;
    }
bool AddAsSimpleTrim (MSBsplineSurfaceR surface, bvector<CurveVectorPtr> &loops)
    {
    for (auto &cvptr : loops)
        if (nullptr == GetSingleLinearLoop (*cvptr))
            return false;
    // simple insertion ..
    for (auto &cvptr : loops)
        {
        bvector<DPoint3d> const *points = GetSingleLinearLoop (*cvptr);
        surface.AddTrimBoundary (*points);
        }
    return true;
    }

void MSBsplineSurface::SetTrim (CurveVectorR curves)
    {
    bspsurf_freeBoundaries (this);
    bvector<CurveVectorPtr> loops;
    if (curves.size () == 0)
        holeOrigin = false;
    else
        {
        CollectInnerOuterLoopRefs (loops, curves);
        if (AddAsSimpleTrim (*this, loops))
            {
            }
        else
            {
            for (size_t i = 0; i < loops.size (); i++)
                {
                TrimCurve *trimLoop = NULL;
                CurveVectorCR loop = *loops[i];
                for (size_t j = 0; j <  loop.size (); j++)
                    {            
                    MSBsplineCurve edgeCurve;
                    if (loop[j]->GetMSBsplineCurve (edgeCurve, 0.0, 1.0))
                        {
                        bspTrimCurve_allocateAndInsertCyclic (&trimLoop, &edgeCurve);
                        }
                    }
                bspsurf_transferTrimToSurface (this, &trimLoop);
                }
            bspsurf_restrokeTrimLoops (this, -1.0, -1.0);
            }
        holeOrigin = true;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int MSBsplineSurface::GetIntNumBounds () const {return numBounds;}
size_t MSBsplineSurface::GetNumBounds () const {return (size_t)numBounds; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MSBsplineSurface::GetNumPointsInBoundary (size_t boundaryIndex) const
    {
    if (boundaryIndex < (size_t)numBounds)
        return boundaries[boundaryIndex].numPoints;
    return false;
    }
    
int MSBsplineSurface::GetIntNumPointsInBoundary (int boundaryIndex) const
    {
    if (boundaryIndex >= 0 && boundaryIndex < numBounds)
        return boundaries[boundaryIndex].numPoints;
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2dCP MSBsplineSurface::GetBoundaryUVCP (size_t i) const
    {
    if (i < (size_t)numBounds)
        return boundaries[i].points;
    return NULL;
    }
DPoint2dCP MSBsplineSurface::GetBoundaryUVCP (int i) const
    {
    if (i >= 0 && i < numBounds)
        return boundaries[i].points;
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d MSBsplineSurface::GetBoundaryUV (size_t boundaryIndex, size_t pointIndex) const
    {
    DPoint2d uv;
    TryGetBoundaryUV (boundaryIndex, pointIndex, uv);
    return uv;
    }
DPoint2d MSBsplineSurface::GetBoundaryUV (int boundaryIndex, int pointIndex) const
    {
    DPoint2d uv;
    TryGetBoundaryUV (boundaryIndex, pointIndex, uv);
    return uv;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::TryGetBoundaryUV (size_t boundaryIndex, size_t pointIndex, DPoint2dR uv) const
    {
    if (boundaryIndex < (size_t)numBounds
        && pointIndex < (size_t)boundaries[boundaryIndex].numPoints)
        {
        uv = boundaries[boundaryIndex].points[pointIndex];
        return true;
        }
    uv.Zero ();
    return false;
    }
bool MSBsplineSurface::TryGetBoundaryUV (int boundaryIndex, int pointIndex, DPoint2dR uv) const
    {
    if (boundaryIndex >= 0 && boundaryIndex < numBounds
        && pointIndex >= 0 && pointIndex < boundaries[boundaryIndex].numPoints)
        {
        uv = boundaries[boundaryIndex].points[pointIndex];
        return true;
        }
    uv.Zero ();
    return false;
    }
double MSBsplineSurface::BoundaryLoopArea (size_t boundaryIndex) const
    {
    if (boundaryIndex >= (size_t)numBounds)
        return 0.0;
    return PolygonOps::Area (boundaries[boundaryIndex].points, (size_t)boundaries[boundaryIndex].numPoints);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
