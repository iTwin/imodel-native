/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include <Geom/IntegerTypes/Point.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define                             DECREASING  -1
#define                             FLAT            0
#define                             INCREASING  1
#define                             MINIMUM_TILE_MARGIN .45
#define                             LEFT_BLOCK_INDEX 1
#define                             RIGHT_BLOCK_INDEX 2
#define                             LOG_FOUR            0.602059991
#define                             fc_tinyVal          1.0e-14

#define EPSILON                     1.0E-8
#define SQUARE_ROOT_TWO             1.414

#define STACK_POINTS                100
#define MAXIMUM_PerspectiveScale    30.0
#define TOLERANCE_Knot              1.0e-10

#define BSPMESH_HORIZONTAL_STRIP 0
#define BSPMESH_VERTICAL_STRIP 1
#define BSPMESH_TRIANGULATE_FROM_LAST_POINT (-1)
#define BSPMESH_TRIANGULATE_FROM_FIRST_POINT (0)

#define BSPMESH_OUTPUT_SINGLE_TRIANGLES (0)
#define BSPMESH_OUTPUT_TRISTRIP         (1)
#define BSPMESH_OUTPUT_TRIFAN           (2)

#define NEXT(index, nPnts)              ((index + 1) % nPnts)
#define PREV(index, nPnts)              (index ? index - 1 : nPnts - 1)
#define NEXTLOWER(index, nPnts, ascend) (ascend ? PREV(index, nPnts) : NEXT(index, nPnts))
#define PREVLOWER(index, nPnts, ascend) (ascend ? NEXT(index, nPnts) : PREV(index, nPnts))

/* Counters for debugging */
#define BSPMESH_COUNTER_TRIANGULATE             0
#define BSPMESH_COUNTER_TRIANGULATESTRIP        1
#define BSPMESH_COUNTER_RECTANGULAR_PATCH       2
#define BSPMESH_COUNTER_IRREGULAR_PATCH         3
#define BSPMESH_COUNTER_TESTED_PATCH            4
#if defined BSPMESH_COUNT_CALLS
static int bspmesh_counters[10] = {0,0,0,0,0, 0,0,0,0,0};
#define INCREMENT_COUNTER(i) (bspmesh_counters[i]++)
#else
#define INCREMENT_COUNTER(i)
#endif

#ifdef USE_CACHE
static int bspmesh_cacheGraph = false;          /* When true, vu mesh graph structures
                                                        are cached across bspmesh_meshSurface
                                                        calls.   This eliminates lots of
                                                        traffic to the memory manager, but can
                                                        only be safely done under control of
                                                        a single MDL app.  (Specifically,
                                                        rendering of a known set of views.)
                                                */
#endif // USE_CACHE

struct MeshRegion
    {
    MeshRegion      *nextP;
    bvector<DPoint2d> boundary;
    void push_back (double x, double y)
        {
        DPoint2d xy;
        xy.x = x;
        xy.y = y;
        boundary.push_back (xy);
        }
    void close ()
        {
        if (boundary.size () > 0)
            {
            DPoint2d xy = boundary[0];
            boundary.push_back (xy);
            }
        }

    void push_back (BsurfBoundary &source)
        {
        int numPoints = source.numPoints;
        if (numPoints <= 0)
            return;
        bool needClosure = source.points[0].x != source.points[numPoints-1].x
                        || source.points[0].y != source.points[numPoints-1].y;
        boundary.reserve (needClosure ? (size_t)numPoints + 1 : (size_t)numPoints);
        for (int i = 0; i < numPoints; i++)
            boundary.push_back (source.points[i]);
        if (needClosure)
            boundary.push_back (source.points[0]);
        }
    };

typedef struct meshParams
    {
    BSplineCallback_AnnounceMeshQuadGrid        meshFunction;               /* => mesh function */
    BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction;           /* => triangle strip function */
    int                                         reverse;                    /* => reverse normal direction(s) */
    double                                      tolerance;                  /* => tolerance */
    int                                         toleranceMode;              /* => tolerance mode */
    TransformP                                  toleranceTransformP;        /* => tolerance transform */
    DPoint3dP                                   toleranceCameraP;           /* => tolerance camera position */
    double                                      toleranceFocalLength;       /* => tolerance focal length */
    bool                                        normalsRequired;            /* => true to return normals */
    bool                                        parametersRequired;         /* => true to return parameters */
    DPoint2d                                    paramScale;                 /* => parameter scale */
    CallbackArgP                                userDataP;                  /* => user data */

    bvector<DPoint2d>   outputUV;
    bvector<DPoint2d*>  outputPointer;
    double              angleTolerance;
    } MeshParams;

/* An Intercept records a crossing with a slice line */
struct Intercept
    {
    DPoint2d            start;
    DPoint2d            end;
    bvector<DPoint2d>   *boundP;
    int                 startIndex;
    int                 endIndex;
    bool                ascending;
    double              sortCoordinate;

    Intercept (double startX, double startY,
        double endX, double endY,
        bvector<DPoint2d> *_boundP,
        int _startIndex, int _endIndex,
        bool _ascending,
        double _sortCoordinate)
        {
        start.x = startX;
        start.y = startY;
        end.x   = endX;
        end.y   = endY;
        boundP = _boundP;
        startIndex = _startIndex;
        endIndex   = _endIndex;
        ascending = _ascending;
        sortCoordinate = _sortCoordinate;
        }


    Intercept (DPoint2dCR _point,
        bvector<DPoint2d> *_boundP,
        int _index,
        bool _ascending,
        double _sortCoordinate)
        {
        start = end = _point;
        boundP = _boundP;
        startIndex = endIndex = _index;
        sortCoordinate = _sortCoordinate;
        }
    };

/* An InterceptHeader holds the counts and current memory pointers for an array of intercepts */
struct InterceptHeader : bvector<Intercept>
    {
    };

typedef struct patchMeshParams
    {
    MSBsplineSurface *bezierP;                          /* active bezier surface */
    double umin, umax, vmin, vmax;
    int    uLoSteps, uHiSteps, vLoSteps, vHiSteps;
    int    uSteps, vSteps;
    } PatchMeshParams;

#ifdef CompileAll

static int bspmesh_triangulate
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d*>  &uvPP,                      /* => pointers to vertices */
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
);

#endif // CompileAll

static int bspmesh_triangulateStrip
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d*>   &uvPP,                     /* => point coordinates */
int                 iMin,                       /* => index of start of strip */
int                 direction,                  /* => 0 for vertical sweep, 1 for horizontal */
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
);

Public GEOMDLLIMPEXP int bspmesh_getCurveSteps
(
int                 uNum,
int                 vNum,
int                 uLoNum,
int                 uHiNum,
int                 vLoNum,
int                 vHiNum,
DPoint2d            *orgP,
DPoint2d            *endP,
DPoint2d            *bezierScale,
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax                  /* => v Maximum for bezier */
);

static DPoint2d unitBoxPoints[] =
        { { 0,0},{ 1,0},{ 1,1},{ 0,1} };

static int s_maxStepsInPatchSubdivision = 500;



#if defined (debug)
MSBsplineSurface    *debugSurfP;
void                *debugUserDataP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void graphics_drawLine
(
DPoint3d    *origin,
DPoint3d    *end,
int         color,
int         style,
int         weight
)
    {
    DPoint3d    tmpPoints[2];
    MSElement   element;

    tmpPoints[0] = *origin;
    tmpPoints[1] = *end;

    mdlLine_create (&element, NULL, tmpPoints);

    element.hdr.dhdr.symb.b.weight = weight;
    element.hdr.dhdr.symb.b.color = color;
    element.hdr.dhdr.symb.b.style = style;

    mdlElement_display (&element, NORMALDRAW);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayUVLine
(
double  x0,
double  y0,
double  x1,
double  y1,
int         color,
int         style,
int         weight
)
    {
    DPoint3d        org, end;

#if defined (SURFACE_DISPLAY)
    bspsurf_evaluateSurfacePoint (&org, NULL, NULL, NULL, x0, y0, debugSurfP);
    bspsurf_evaluateSurfacePoint (&end, NULL, NULL, NULL, x1, y1, debugSurfP);
#else
    org.x = 20.0 + x0 * 600.0;
    org.y = 40.0 + (1.0 - y0) * 400.0;
    end.x = 20.0 + x1 * 600.0;
    end.y = 40.0 + (1.0 - y1) * 400.0;
    org.z = end.z = 100.0;
#endif

    graphics_drawLine (&org, &end, color, style, weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayRegion
(
BsurfBoundary   *boundP,
int             dispColor
)
    {
    int         i;
    static      int color = 1;

    for (i=0; i<boundP->numPoints-1; i++)
        debug_displayUVLine (boundP->points[i].x,   boundP->points[i].y,
                             boundP->points[i+1].x, boundP->points[i+1].y,
                             color++%14, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayPointers
(
DPoint2d        **pointPP,
int             nPoints
)
    {
    int         i;
    static      int color = 1;

    for (i=0; i<nPoints-1; i++)
        debug_displayUVLine (pointPP[i]->x, pointPP[i]->y,
                             pointPP[i+1]->x, pointPP[i+1]->y,
                             color++%14, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayRegions
(
MeshRegion  *mrP
)
    {
    for (; mrP; mrP=mrP->nextP)
        debug_displayRegion (&mrP->boundary, -1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayTriangle
(
DPoint2d    *triangleP
)
    {
    static int      color=0;
    int             curcolor;

    curcolor = color++%14;
    debug_displayUVLine (triangleP[0].x, triangleP[0].y, triangleP[1].x, triangleP[1].y,
                             curcolor, 0, 0);
    debug_displayUVLine (triangleP[1].x, triangleP[1].y, triangleP[2].x, triangleP[2].y,
                             curcolor, 0, 0);
    debug_displayUVLine (triangleP[2].x, triangleP[2].y, triangleP[0].x, triangleP[0].y,
                             curcolor, 0, 0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void debug_displayNormals
(
DPoint3d    *points,
DPoint3d    *normals,
int         nPoints
)
    {
    int         i;
    DPoint3d    normalEnd;

    for (i=0; i<nPoints; i++)
        {
        normalEnd.x = points[i].x + 40.0 * normals[i].x;
        normalEnd.y = points[i].y + 40.0 * normals[i].y;
        normalEnd.z = points[i].z + 40.0 * normals[i].z;

        graphics_drawLine (&points[i], &normalEnd, 5, 0, 0);
        }
    }

#endif  /* debug */

/*----------------------------------------------------------------------+
|                                                                       |
|   Utility routines                                                    |
|                                                                       |
+----------------------------------------------------------------------*/

int integerizeStepCount (double value)
    {
    if (value < 1.0)
        return 1;
    return (int)(value + 0.5);
    }


static double sDefaultAbsTol = 1.0e-14;
static double sDefaultLocalRelTol = 1.0e-8;
static double sDefaultGlobalRelTol = 1.0e-12;

static double computePoleTolerance
(
DPoint3d const *pXYZ,
int      numXYZ,
double absTol,
double localRelTol,
double globalRelTol
)
    {
    double tol;
    double maxAbs, maxDiagonal;
    double a, b;
    DRange3d range;
    DVec3d diagonal;
    range.InitFrom(pXYZ, numXYZ);
    diagonal.DifferenceOf (range.low, range.high);
    maxAbs = range.LargestCoordinate ();
    maxDiagonal = diagonal.MaxAbs ();

    if (absTol < sDefaultAbsTol)
        absTol = sDefaultAbsTol;

    if (localRelTol < sDefaultLocalRelTol)
        localRelTol = sDefaultLocalRelTol;

    if (globalRelTol < sDefaultGlobalRelTol)
        globalRelTol = sDefaultGlobalRelTol;

    tol = absTol;
    a = localRelTol * maxDiagonal;
    b = globalRelTol * maxDiagonal;
    if (a > tol)
        tol = a;
    if (b > tol)
        tol = b;
    return tol;
    }
/*--------------------------------------------------------------------*//**
@param pSurface IN surface to examine.
@param absTol IN absolute tolerance.
@param localRelTol  IN local relative tolerance.
@param globalRelTol IN global relative tolerance.
@param globalRelTol In relative tolerance as fraction of all coordinates
@returns largest absolute tolerance among the choices.
+----------------------------------------------------------------------*/
static double mdlBspline_resolveSurfaceTolerance
(
const MSBsplineSurface  *pSurface,
double                  absTol,
double                  localRelTol,
double                  globalRelTol
)
    {
    return computePoleTolerance
                (
                pSurface->poles,
                pSurface->uParams.numPoles * pSurface->vParams.numPoles,
                absTol, localRelTol, globalRelTol
                );
    }

/*---------------------------------------------------------------------------------**//**
* Enforce lower and upper limits on an integer.
* @bsimethod                                                    Earlin.Lutz     02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void enforceLimits
(
int *pValue,
int min,
int max
)
    {
    if (pValue)
        {
        if (*pValue < min)
            *pValue = min;
        if (*pValue > max)
            *pValue = max;
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_clearOutputBuffer
(
MeshParams *mpP
)
    {
    mpP->outputUV.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* Pass the output buffer to the triangulation logic
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_triangulateOutputBufferAsStrip
(
MeshParams *mpP,
PatchMeshParams *pmpP,
int startIndex,
int direction
)
    {
    size_t n = mpP->outputUV.size ();
    if (n >= 3 )
        {
        //DPoint2d xy = mpP->outputUV[0];
        //mpP->outputUV.push_back (xy);
        /* Make an array of pointers to the points */
        mpP->outputPointer.clear ();
        for (size_t i = 0; i < n; i++ )
            mpP->outputPointer.push_back (&mpP->outputUV[i]);

        return bspmesh_triangulateStrip ( mpP, pmpP->bezierP, mpP->outputPointer,
                            startIndex, direction,
                            pmpP->umin, pmpP->umax,
                            pmpP->vmin, pmpP->vmax );
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Copy points from an array to the output buffer, allowing array wrap and checking for double points within the array
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_addDistinctPointsToOutputBuffer
(
MeshParams *mpP,
DPoint2d *pointP,
int nPoint,
int i0,
int i1,
PatchMeshParams *pmpP
)
    {
    int i,j, steps, step;
    DPoint2d vector, point,scaleVector;
    if ( i0 < 0 || i1 < 0 || i0 >= nPoint || i1 >= nPoint )
        return;
    scaleVector.x = pmpP->umax - pmpP->umin;
    scaleVector.y = pmpP->vmax - pmpP->vmin;

    mpP->outputUV.push_back (pointP[i0]);

    for ( i = i0 ; i != i1 ; i = j )
        {
        j = i + 1;
        if ( j >= nPoint ) j = 0;
        if ( pointP[j].x != pointP[i].x || pointP[j].y != pointP[i].y )
            {
            steps = bspmesh_getCurveSteps
                                (
                                pmpP->uSteps, pmpP->vSteps,
                                pmpP->uLoSteps, pmpP->uHiSteps,
                                pmpP->vLoSteps, pmpP->vHiSteps,
                                pointP + i, pointP + j,
                                &scaleVector,
                                pmpP->umin, pmpP->umax,
                                pmpP->vmin, pmpP->vmax
                                );
           /* Use j as the '0' point of the interpolation to limit fp roundoff error. */
           vector.x = pointP[i].x - pointP[j].x;
           vector.y = pointP[i].y - pointP[j].y;
           for ( step = steps - 1; step >= 0 ; step-- )
                {
                double f = (double) step / (double)steps;
                point.x = pointP[j].x + f*vector.x;
                point.y = pointP[j].y + f*vector.y;
                mpP->outputUV.push_back (point);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Interpolate distinct points from startP..endP into the output buffer If one point is requested, just put startP
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_addLatticePointsToOutputBuffer
(
MeshParams *mpP,
DPoint2d *startP,
DPoint2d *endP,
int nPoint
)
    {
    int i;
    int nStep = nPoint - 1;
    DPoint2d point;
    DPoint2d vector;
    if ( nPoint == 1 )
        {
        mpP->outputUV.push_back (*startP);
        }
    else if ( nPoint >= 1 )
        {
        mpP->outputUV.push_back (*startP);

        vector.x = endP->x - startP->x;
        vector.y = endP->y - startP->y;

        for ( i = 1 ; i < nStep ; i++ )
            {
            double f = (double) i / (double) nStep;
            point.x = startP->x + f * vector.x;
            point.y = startP->y + f * vector.y;
            mpP->outputUV.push_back (point);
            }

        mpP->outputUV.push_back (*endP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static double bspmesh_horizIntercept
(
double      u,
DPoint2d    *orgP,
DPoint2d    *endP
)
    {
    return orgP->y + (u - orgP->x) * (endP->y - orgP->y)/(endP->x - orgP->x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static double bspmesh_verticalIntercept
(
double      v,
DPoint2dCP orgP,
DPoint2dCP endP
)
    {
    return orgP->x + (v - orgP->y) * (endP->x - orgP->x)/(endP->y - orgP->y);
    }


/*---------------------------------------------------------------------------------**//**
* Return the mean x on the line string defined by an intercept struct
* @bsimethod                                                    Earlin.Lutz     11/95
+---------------+---------------+---------------+---------------+---------------+------*/
static double bspmesh_meanXOnIntercept
(
Intercept       *interceptP
)
    {
    double              sumdY;
    double              sumXdY;
    double              dY;
    bvector<DPoint2d>*   boundP = interceptP->boundP;
    int                 boundPoints = (int)boundP->size () - 1;
    DPoint2d            start, end;
    int                 index;
    int                 ascending = interceptP->ascending;
    double              result;

    sumdY = sumXdY = 0.0;

    start = interceptP->start;

    for (index = NEXTLOWER(interceptP->startIndex, boundPoints, ascending);
         index != interceptP->endIndex;
         index = NEXTLOWER(index, boundPoints, ascending))
        {
        end = boundP->at (index);
        dY = end.y - start.y;
        sumXdY += 0.5 * (start.x + end.x) * dY;
        sumdY += dY;
        start = end;
        }


    end = interceptP->end;
    dY = end.y - start.y;
    sumXdY += 0.5 * (start.x + end.x) * dY;
    sumdY += dY;


    if (sumdY != 0.0)
        {
        result = sumXdY / sumdY;
        }
    else
        {
        result = 0.5 * (interceptP->start.x + interceptP->end.x);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_bufferSimpleIntercept
(
InterceptHeader *headerP,
DPoint2dCP pointP,
bvector<DPoint2d>   *boundP,
int             index
)
    {
    headerP->push_back (Intercept (*pointP, boundP, index, false, pointP->x));
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_bufferIntercept
(
InterceptHeader *headerP,
double          xStart,
double          yStart,
double          yEnd,
bvector<DPoint2d>   *boundP,
int             index,
bool            ascending
)
    {
    int         boundPoints, nextIndex;
    double      deltaY;
    DPoint2d    org,end;
    Intercept   *interceptP;

    headerP->push_back (Intercept (
                    xStart, yStart,
                    xStart, yEnd,
                    boundP,
                    index, index,
                    ascending,
                    0.0));
    interceptP = &headerP->back ();

    boundPoints = (int)boundP->size () -1;
    for (interceptP->endIndex = NEXTLOWER(index, boundPoints, ascending),
         nextIndex = NEXTLOWER(interceptP->endIndex, boundPoints, ascending);
             boundP->at (interceptP->endIndex).y > yEnd &&
             boundP->at (interceptP->endIndex).y >= boundP->at (nextIndex).y;
                interceptP->endIndex = nextIndex,
                nextIndex = NEXTLOWER(nextIndex, boundPoints, ascending));

    org = boundP->at (PREVLOWER (interceptP->endIndex, boundPoints, ascending));
    end = boundP->at (interceptP->endIndex);
    deltaY = end.y - org.y;
    if (fabs (deltaY) > fc_epsilon)
        interceptP->end.x = org.x + (end.x - org.x) * (yEnd - org.y) / deltaY;
    else
        interceptP->end.x = end.x;
    interceptP->sortCoordinate = bspmesh_meanXOnIntercept (interceptP);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Routines for evaluating steps sizes                                 |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspmesh_numMeshSizeSurf
(
int                 *uNum,              /* <= # steps in U */
int                 *vNum,              /* <= # steps in V */
int                 *uLoNum,            /* <= # steps along u Lo boundary */
int                 *uHiNum,            /* <= # steps along u Hi boundary */
int                 *vLoNum,            /* <= # steps along v Lo boundary */
int                 *vHiNum,            /* <= # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* => bezier patch */
double              tol,                /* => tolerance */
int                 toleranceMode       /* => tolerance mode */
)
    {
    int             i, j, rational, uOrder, vOrder, itmp, uDegree, vDegree;
    double          tmp, minWeight=1.0, maxDiff=0.0;
    DPoint3d        diffPt, *poleP;

    rational = patchBezP->rational;
    uOrder = patchBezP->uParams.order;
    vOrder = patchBezP->vParams.order;
    uDegree = uOrder - 1;
    vDegree = vOrder - 1;

    /* Calculate the number of steps in u direction */
    for (i = 0; i < vOrder; i++)
        {
        for (j=0, poleP=patchBezP->poles+i*uOrder; j < uDegree; j++, poleP++)
            {
            if (rational)
                {
                tmp=patchBezP->weights[i*uOrder+j];
                minWeight = (tmp < minWeight) ? tmp : minWeight;
                }
            diffPt.DifferenceOf (poleP[1], *poleP);
            tmp = (toleranceMode & STROKETOL_XYProjection) ?
              sqrt (diffPt.x * diffPt.x + diffPt.y * diffPt.y) : diffPt.Magnitude ();

            maxDiff = (maxDiff < tmp) ? tmp : maxDiff;
            }
        }
    itmp = (int)(uDegree * maxDiff / (tol * minWeight));
    *uNum = (itmp < 1) ? 1 : itmp;

    /* Calculate the number of steps in v direction */
    if (rational)
        minWeight = patchBezP->weights[0];

    maxDiff = 0.0;
    for (i = 0; i < vDegree; i++)
        {
        for (j=0, poleP=patchBezP->poles+i*uOrder; j < uOrder; j++, poleP++)
            {
            if (rational)
                {
                tmp=patchBezP->weights[i*uOrder+j];
                minWeight = (tmp < minWeight) ? tmp : minWeight;
                }
            diffPt.DifferenceOf (poleP[uOrder], *poleP);
            tmp = (toleranceMode & STROKETOL_XYProjection) ?
              sqrt (diffPt.x * diffPt.x + diffPt.y * diffPt.y) : diffPt.Magnitude ();

            maxDiff = (maxDiff < tmp) ? tmp : maxDiff;
            }
        }
    itmp = (int)(vDegree * maxDiff / (tol * minWeight));
    *vNum = (itmp < 1) ? 1 : itmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    maxDiagonalError
(
DPoint3d            *poleArrayP,        /* => packed array of poles. */
int                 uOrder,             /* => first direction count */
int                 vOrder,             /* => second direction count */
double              *pMaxAbsErr,        /* => max abs error between diagonal midpoints */
double              *pMaxRelErr         /* => max ratio of midpoint error over diagonal length */
)
    {
    DPoint3d *row0P, *row1P;
    DPoint3d point0, point1;
    double maxDiff, currDiff;
    double d0, d1, d2, ratio, maxRatio;
    int i, j;
    maxDiff = 0.0;
    maxRatio = 0.0;
    for (j = 1; j < vOrder; j++)
        {
        row0P = poleArrayP + (j - 1) * uOrder;
        row1P = poleArrayP + j * uOrder;
        for (i = 1; i < uOrder; i++)
            {
            point0.Interpolate (row0P[i-1], 0.5, row1P[i]);
            point1.Interpolate (row0P[i], 0.5, row1P[i-1]);
            currDiff = point0.DistanceSquared (point1);
            d0 = row0P[i-1].DistanceSquared (row1P[i]);
            d1 = row0P[i].DistanceSquared (row1P[i-1]);
            d2 = d0 > d1 ? d0 : d1;
            if (DoubleOps::SafeDivide (ratio, currDiff, d2, 0.0)
                && ratio > maxRatio)
                maxRatio = ratio;

            if (currDiff > maxDiff)
                maxDiff = currDiff;
            }
        }
    if (pMaxAbsErr)
        *pMaxAbsErr = sqrt (maxDiff);
    if (pMaxRelErr)
        *pMaxRelErr = sqrt (maxRatio);
    }

/*--------------------------------------------------------------------*//**
@description Update evolving maximum chord height and angle deviations.
@param pPoint0 IN first point of 3
@param pPoint1 IN second point of 3
@param pPoint2 IN third point of 3
@param bUpdateLo IN true if this chord is at the "low" end.
@param bUpdateHi IN true if this chord is at the "high" end.
@param pDelta IN OUT evolving chord error
@param pLoDelta IN OUT evolving chord error at low end.
@param pHiDelta IN OUT evolving chord error at high end.
@param pAngle IN OUT evolving angle error
@param pLoAngle IN OUT evolving angle error at low end.
@param pHiAngle IN OUT evolving angle error at high end.
@param MeshParams mpP IN mesh params
+----------------------------------------------------------------------*/
static void    updateDeviations
(
DPoint3d *pPoint0,
DPoint3d *pPoint1,
DPoint3d *pPoint2,
bool    bUpdateLo,
bool    bUpdateHi,
double *pDelta,
double *pLoDelta,
double *pHiDelta,
double *pAngle,
double *pLoAngle,
double *pHiAngle,
MeshParams          *mpP                /* => mesh parameters */
)
    {
    DPoint3d vec01, vec02, vec12;
    double mag01, mag02, mag12;
    double cross;
    double h;
    DPoint3d crossProduct;

    vec01.DifferenceOf (*pPoint1, *pPoint0);
    vec02.DifferenceOf (*pPoint2, *pPoint0);
    vec12.DifferenceOf (*pPoint2, *pPoint1);

    if (mpP->toleranceMode & STROKETOL_XYProjection)
        {
        mag12   = vec12.MagnitudeXY ();
        mag01   = vec01.MagnitudeXY ();
        mag02   = vec02.MagnitudeXY ();
        cross   = fabs (vec01.CrossProductXY (vec02));
        }
    else
        {
        mag12   = vec12.Magnitude ();
        mag01   = vec01.Magnitude ();
        mag02   = vec02.Magnitude ();
        crossProduct.CrossProduct (vec01, vec02);
        cross       = crossProduct.Magnitude ();
        }

    if (   fabs (mag01)  > EPSILON
        && fabs (mag02) > EPSILON
        && fabs (mag12) > EPSILON
        )
        {
        h = cross / mag02;
        if (h > *pDelta)
            *pDelta   = h;
        if (bUpdateLo && h > *pLoDelta)
            *pLoDelta = h;
        if (bUpdateHi && h > *pHiDelta)
            *pHiDelta = h;
        if (mpP->angleTolerance > 0.0)
            {
            double theta;
            // How's your trig?  Got a quick formula for this without redoing the products?
            if (mpP->toleranceMode & STROKETOL_XYProjection)
                {
                theta = vec01.AngleToXY (vec02);
                }
            else
                {
                theta = vec01.AngleTo (vec02);
                }

            if (theta > *pAngle)
                *pAngle = theta;
            if (bUpdateLo && theta > *pLoAngle)
                *pLoAngle = theta;
            if (bUpdateHi && theta > *pHiAngle)
                *pHiAngle = theta;
            }
        }
    }

/*--------------------------------------------------------------------*//**
@description Compute a chord count for various error and degree conditions.
@param pCount OUT computed count.
@param degree IN curve degree
@param diagonalCOunt IN previously determined count which acts as a lower limit.
@param maxChordDiff IN largest chord error in control polygon.
@param maxAngle IN largest angle in control polygon.
@param mpP IN mesh params (provides target tolerances)
+----------------------------------------------------------------------*/
static void computeChordCount
(
int *pCount,
int degree,
int diagonalCount,
double maxChordDiff,
double maxAngle,
MeshParams          *mpP                /* => mesh parameters */
)
    {
    int count;
    if (!pCount)
        return;
    if (degree < 1)
        degree = 1;
    count = degree;
    if (count < diagonalCount)
        count = diagonalCount;
    if (mpP->tolerance > 0.0)
        {
        int chordDiffCount = (int) (1.0 + sqrt (10.0 * maxChordDiff/mpP->tolerance));
        if (chordDiffCount > count)
            count = chordDiffCount;
        }
    if (mpP->angleTolerance > 0.0 && maxAngle > mpP->angleTolerance)
        {
        int angleCount = (int) (1.0 + (double)degree * maxAngle / mpP->angleTolerance);
        if (angleCount > count)
            count = angleCount;
        }
    *pCount = count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_numChoordHeightSurf
(
int                 *uNum,              /* <= # steps in U */
int                 *vNum,              /* <= # steps in V */
int                 *uLoNum,            /* <= # steps along u Lo boundary */
int                 *uHiNum,            /* <= # steps along u Hi boundary */
int                 *vLoNum,            /* <= # steps along v Lo boundary */
int                 *vHiNum,            /* <= # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* => bezier patch */
MeshParams          *mpP                /* => mesh parameters */
)
    {
    int             i, j, rational, numPoles, uOrder, vOrder, uIndex, vIndex,
                    uDegree, vDegree;
    double          maxDiff, maxLoDiff, maxHiDiff,
                    scalar, denominator, minimumZ;
    DPoint3d        *poleP, *pPtr, *endP,
                    poles[MAX_ORDER*MAX_ORDER];
    double          diagonalError;
    double          diagonalRelativeError;
    int             diagonalCount;
    static int s_maxDiagonalCount = 30;
    // EDL June 2013.  diagaonal count is being passed to edges.  Why?  This creates mismatches
    //    because "other side" of edge has different diagonal count.
    //   But whey didn't this show up in SS3 hline?  Dunno.
    static int s_diagonalFactorForLoHi = 0;   // 0 suppresses transfer of diagonal to edges.
    int numPositive, numNegative;
    double          angleTolerance = mpP->angleTolerance;
    static double s_minAngleTolerance = 0.1;
    static double s_maxAngleTolerance = 1.57;
    static double s_diagonalRelTol = 0.06;
    double          maxAngle, maxLoAngle, maxHiAngle;
#ifdef DEBUG_DIAGONAL_CORRECTION
    static int      s_numUDiagonalCorrection = 0;
    static int      s_numVDiagonalCorrection = 0;
    static int      s_numPatch = 0;
    static bool     s_useDiagonalCorrection = true;
#endif

#ifdef DEBUG_DIAGONAL_CORRECTION
    s_numPatch++;
#endif

    if (angleTolerance > 0.0 && angleTolerance < s_minAngleTolerance)
        angleTolerance = s_minAngleTolerance;

    if (angleTolerance > s_maxAngleTolerance)
        angleTolerance = s_maxAngleTolerance;

    rational = patchBezP->rational;
    uOrder = patchBezP->uParams.order;
    vOrder = patchBezP->vParams.order;
    uIndex = uOrder - 2;
    vIndex = vOrder - 2;
    uDegree = uOrder - 1;
    vDegree = vOrder - 1;
    numPoles = uOrder * vOrder;

    if (rational)
        bsputil_unWeightPoles (poles, patchBezP->poles,  patchBezP->weights, numPoles);
    else
        memcpy (poles, patchBezP->poles, numPoles * sizeof(DPoint3d));

    if (mpP->toleranceTransformP)
        mpP->toleranceTransformP->Multiply (poles, numPoles);

    if (mpP->toleranceMode & STROKETOL_Perspective && mpP->toleranceCameraP != NULL)
        {
        minimumZ = mpP->toleranceFocalLength / MAXIMUM_PerspectiveScale;
        numPositive = numNegative = 0;
        /* Do we span the front clip plane? */
        for (poleP=poles, endP=poles+numPoles; poleP<endP; poleP++)
            {
            denominator = mpP->toleranceCameraP->z - poleP->z;
            if (denominator > minimumZ)
                {
                numPositive++;
                }
            else
                {
                numNegative++;
                }
            }
        /* If all on visible side of front clip,
            divide xy by z to get projection on focal plane. */
        if (numNegative == 0)
            {
            // No test for divide by zero --- we know everything is positive.
            for (poleP=poles, endP=poles+numPoles; poleP<endP; poleP++)
                {
                denominator = mpP->toleranceCameraP->z - poleP->z;
                scalar = mpP->toleranceFocalLength / denominator;
                poleP->x = scalar * (poleP->x - mpP->toleranceCameraP->x);
                poleP->y = scalar * (poleP->y - mpP->toleranceCameraP->y);
                }
            }
        }

    maxDiagonalError (poles, uOrder, vOrder, &diagonalError, &diagonalRelativeError);
    if (mpP->tolerance <= 0.0)
        diagonalCount = 1;
    else
        diagonalCount = (int) (0.999 + sqrt (diagonalError/mpP->tolerance));
    if (diagonalRelativeError > s_diagonalRelTol)
        {
        int relCount = (int) (0.999 + sqrt (diagonalRelativeError / s_diagonalRelTol));
        if (relCount > diagonalCount)
            diagonalCount = relCount;
        }
    if (diagonalCount > s_maxDiagonalCount)
        diagonalCount = s_maxDiagonalCount;
    if (diagonalCount < 1)
        diagonalCount = 1;

#ifdef DEBUG_DIAGONAL_CORRECTION
    if (!s_useDiagonalCorrection)
        diagonalCount = 1;
#endif
    int diagonalCount1 = diagonalCount * s_diagonalFactorForLoHi;
    /* Find uNum */
    if (uOrder > 2)
        {
        maxDiff = maxLoDiff = maxHiDiff = 0.0;
        maxAngle = maxLoAngle = maxHiAngle = 0.0;
        for (j=0, poleP=poles; j<vOrder; j++, poleP += uOrder)
            {
            for (i=0, pPtr=poleP; i<uIndex; i++, pPtr++)
                {
                updateDeviations (pPtr, pPtr + 1, pPtr + 2,
                            j == 0,
                            j == vOrder - 1,
                            &maxDiff, &maxLoDiff, &maxHiDiff,
                            &maxAngle, &maxLoAngle, &maxHiAngle,
                            mpP
                            );
                }
            }
        computeChordCount (uNum,   uDegree, diagonalCount, maxDiff,   maxAngle, mpP);
        computeChordCount (uLoNum, uDegree, diagonalCount1, maxLoDiff, maxLoAngle, mpP);
        computeChordCount (uHiNum, uDegree, diagonalCount1, maxHiDiff, maxHiAngle, mpP);
        }
    else
        {
        *uNum = diagonalCount;
        if (uLoNum) *uLoNum = 1;
        if (uHiNum) *uHiNum = 1;
        }

    /* Find vNum */
    if (vOrder > 2)
        {
        maxDiff = maxLoDiff = maxHiDiff = 0.0;
        maxAngle = maxLoAngle = maxHiAngle = 0.0;
        for (j=0, poleP=poles; j < uOrder; j++, poleP++)
            {
            for (i=0, pPtr=poleP; i < vIndex; i++, pPtr += uOrder)
                {
                updateDeviations (pPtr, pPtr + uOrder, pPtr + 2 * uOrder,
                            j == 0,
                            j == uOrder - 1,
                            &maxDiff, &maxLoDiff, &maxHiDiff,
                            &maxAngle, &maxLoAngle, &maxHiAngle,
                            mpP
                            );
                }
            }
        computeChordCount (vNum, vDegree, diagonalCount, maxDiff,   maxAngle, mpP);
        computeChordCount (vLoNum, vDegree, diagonalCount1, maxLoDiff, maxLoAngle, mpP);
        computeChordCount (vHiNum, vDegree, diagonalCount1, maxHiDiff, maxHiAngle, mpP);
        }
    else
        {
        *vNum = diagonalCount;
        if (vLoNum) *vLoNum = 1;
        if (vHiNum) *vHiNum = 1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_getCurveSteps
(
int                 uNum,
int                 vNum,
int                 uLoNum,
int                 uHiNum,
int                 vLoNum,
int                 vHiNum,
DPoint2d            *orgP,
DPoint2d            *endP,
DPoint2d            *bezierScale,
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax                  /* => v Maximum for bezier */
)
    {
    int             steps;
    double          uCount, vCount, deltaU, deltaV;

    deltaU = fabs (endP->x - orgP->x) / bezierScale->x;
    deltaV = fabs (endP->y - orgP->y) / bezierScale->y;

    if (deltaV < EPSILON)
        {
        if (fabs (orgP->y - bezierVMin) < EPSILON)
            steps = integerizeStepCount (deltaU * (double) uLoNum);
        else if (fabs (orgP->y - bezierVMax) < EPSILON)
            steps = integerizeStepCount (deltaU * (double) uHiNum);
        else
            steps = integerizeStepCount (deltaU * (double) uNum);
        }
    else if (deltaU < EPSILON)
        {
        if (fabs (orgP->x - bezierUMin) < EPSILON)
            steps = integerizeStepCount (deltaV * (double) vLoNum);
        else if (fabs (orgP->x - bezierUMax) < EPSILON)
            steps = integerizeStepCount (deltaV * (double) vHiNum);
        else
            steps = integerizeStepCount (deltaV * (double) vNum);
        }
    else
        {
        uCount = integerizeStepCount (deltaU * (double) uNum);
        vCount = integerizeStepCount (deltaV * (double) vNum);

        steps = integerizeStepCount (uCount > vCount ? uCount : vCount);
        }
    return  steps ? steps : 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_getSurfaceSteps
(
int                 *uNum,              /* <= # steps in U */
int                 *vNum,              /* <= # steps in V */
int                 *uLoSteps,          /* <= # steps along u Lo boundary */
int                 *uHiSteps,          /* <= # steps along u Hi boundary */
int                 *vLoSteps,          /* <= # steps along v Lo boundary */
int                 *vHiSteps,          /* <= # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* => bezier patch */
MeshParams          *mpP                /* => mesh parameters */
)
    {
    switch (mpP->toleranceMode & 0x00ff)
        {
        case STROKETOL_ChoordHeight:
            bspmesh_numChoordHeightSurf (uNum, vNum, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                         patchBezP, mpP);
            break;

        case STROKETOL_StrokeSize:
            bspmesh_numMeshSizeSurf (uNum, vNum, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                     patchBezP, mpP->tolerance, mpP->toleranceMode);
            break;

        case STROKETOL_PatchSegments:
            if (patchBezP->uParams.order > 2)
                {
                if (NULL != uNum)       *uNum     = (int) mpP->tolerance;
                if (NULL != uLoSteps)   *uLoSteps = (int) mpP->tolerance;
                if (NULL != uHiSteps)   *uHiSteps = (int) mpP->tolerance;
                }
            else
                {
                if (NULL != uNum)       *uNum     = 1;
                if (NULL != uLoSteps)   *uLoSteps = 1;
                if (NULL != uHiSteps)   *uHiSteps = 1;
                }
            if (patchBezP->vParams.order > 2)
                {
                if (NULL != vNum)       *vNum     = (int) mpP->toleranceFocalLength;
                if (NULL != vLoSteps)   *vLoSteps = (int) mpP->toleranceFocalLength;
                if (NULL != vHiSteps)   *vHiSteps = (int) mpP->toleranceFocalLength;
                }
            else
                {
                if (NULL != vNum)       *vNum     = 1;
                if (NULL != vLoSteps)   *vLoSteps = 1;
                if (NULL != vHiSteps)   *vHiSteps = 1;
                }
            break;
        }

    enforceLimits (uNum,     1, s_maxStepsInPatchSubdivision);
    enforceLimits (uLoSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (uHiSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (vNum,     1, s_maxStepsInPatchSubdivision);
    enforceLimits (vLoSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (vHiSteps, 1, s_maxStepsInPatchSubdivision);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Routines for evaluating Bezier Points/Normals                       |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_computeBlendBernstein
(
double          *bezierVal,     /* <= val of iBlend-th Bernstein poly at u */
int             iBlend,         /* => index of the Bernstein */
int             order,          /* => the order of the Bernstein poly */
double          u               /* => parameter val to be evaluated at */
)
    {
    int         i, itmp, nChoosei=1, degree;
    double      u1, fact1=1.0, fact2=1.0;

    degree = order - 1;
    for (i = 0; i < iBlend; i++)
        {
        fact1 = fact1 * u;
        nChoosei = nChoosei * (degree - i) / (i + 1);
        }

    u1 = 1.0 - u;
    itmp = degree - iBlend;
    for (i = 0; i < itmp; i++)
        {
        fact2 = fact2 * u1;
        }

    *bezierVal = nChoosei * fact1 * fact2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_evaluateMeshBezier
(
DPoint3d            *meshPointsP,   /* <= mesh pts with u direction goes fast */
DPoint3d            *meshNormP,     /* <= mesh normal or NULL */
MSBsplineSurface    *patchBezP,     /* => Bezier Patch */
double              uLo,            /* => lower bound in u direction */
double              uHi,            /* => upper bound in u direction */
double              vLo,            /* => lower bound in v direction */
double              vHi,            /* => upper bound in u direction */
int                 uNum,           /* => num of points in u direction */
int                 vNum,           /* => num of points in v direction */
bool                reverse         /* => true to reverse U direction and normal */
)
    {
    int             i, j, s, t, uOrder, vOrder, uDegree, vDegree,
                    rational, status=SUCCESS, index;
    double          utmp, vtmp, uStep, vStep, *uBlend, *vBlend,
                    *uBlendDer, *vBlendDer, tmpWeights, weight = 0.0, derUWgts = 0.0,
                    derVWgts = 0.0, diffWgts, *uBtr, *pBb, *uNtr = NULL, *pNn, *vBtr,
                    *vNtr = NULL;
    DPoint3d        tmpPoles, pole, derTmpPoles, derUPole, derVPole, diffPoles;

    uBlend = vBlend = uBlendDer = vBlendDer = NULL;

    rational = patchBezP->rational;
    uOrder = patchBezP->uParams.order;
    vOrder = patchBezP->vParams.order;
    uDegree = uOrder - 1;
    vDegree = vOrder - 1;
    uStep = (uHi - uLo) / (uNum-1);
    vStep = (vHi - vLo) / (vNum-1);

    /* Malloc memory for uBlend and vBlend */
    if (!(uBlend = (double*)msbspline_malloc (uOrder*uNum*sizeof(double), NULL)) ||
        !(vBlend = (double*)msbspline_malloc (vOrder*vNum*sizeof(double), NULL)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* Malloc memory for uBlendDer and vBlendDer if necessary */
    if (meshNormP)
        {
        if (!(uBlendDer = (double*)msbspline_malloc (uDegree*uNum*sizeof(double), NULL)) ||
            !(vBlendDer = (double*)msbspline_malloc (vDegree*vNum*sizeof(double), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    /* Calculate uBlend functions */
    if (meshNormP) uNtr=uBlendDer;
    for (i=0, uBtr=uBlend; i<uNum; i++, uBtr+=uOrder)
        {
        utmp=uLo+i*uStep;
        for (t=0, pBb=uBtr; t<uOrder; t++, pBb++)
            bspmesh_computeBlendBernstein(pBb, t, uOrder, utmp);
        if (meshNormP)
            {
            for (t=0, pNn=uNtr; t<uDegree; t++, pNn++)
                bspmesh_computeBlendBernstein(pNn, t, uDegree, utmp);
            uNtr+=uDegree;
            }
        }

    /* Calculate vBlend functions */
    if (meshNormP) vNtr=vBlendDer;
    for (j=0, vBtr=vBlend; j<vNum; j++, vBtr+=vOrder)
        {
        vtmp=vLo+j*vStep;
        for (s=0, pBb=vBtr; s<vOrder; s++, pBb++)
            bspmesh_computeBlendBernstein(pBb, s, vOrder, vtmp);
        if (meshNormP)
            {
            for (s=0, pNn=vNtr; s<vDegree; s++, pNn++)
                bspmesh_computeBlendBernstein(pNn, s, vDegree, vtmp);
            vNtr+=vDegree;
            }
        }

    /* Evaluate points over the mesh */
    for (j = 0; j < vNum; j++)
        for (i = 0; i < uNum; i++)
            {
            index = reverse ? ((j + 1) * uNum - (i + 1)) : (j * uNum + i);
            pole.x = pole.y = pole.z = 0.0;
            weight = 0.0;
            for (s = 0; s < vOrder; s++)
                {
                tmpPoles.x = tmpPoles.y = tmpPoles.z = 0.0;
                tmpWeights = 0.0;
                for (t = 0; t < uOrder; t++)
                    {
                    tmpPoles.SumOf (tmpPoles, *(patchBezP->poles+s*uOrder+t), uBlend[i*uOrder+t]);
                    if (rational)
                        {
                        tmpWeights += patchBezP->weights[s*uOrder+t] *
                                      uBlend[i*uOrder+t];
                        }
                    }
                pole.SumOf (pole, tmpPoles, vBlend[j*vOrder+s]);
                if (rational)
                    {
                    weight += tmpWeights * vBlend[j*vOrder+s];
                    }
                }

            if (meshNormP)
                {
                /* u partial derivatives */
                derUPole.x = derUPole.y = derUPole.z = derUWgts = 0.0;
                for (s = 0; s < vOrder; s++)
                    {
                    derTmpPoles.x = derTmpPoles.y = derTmpPoles.z = tmpWeights = 0.0;
                    for (t = 0; t < uDegree; t++)
                        {
                        diffPoles.DifferenceOf (*(patchBezP->poles+s*uOrder+t+1), *(patchBezP->poles+s*uOrder+t));
                        derTmpPoles.SumOf (derTmpPoles, diffPoles, uBlendDer[i*uDegree+t]);
                        if (rational)
                            {
                            diffWgts = patchBezP->weights[s*uOrder+t+1]-
                                       patchBezP->weights[s*uOrder+t];
                            tmpWeights += diffWgts * uBlendDer[i*uDegree+t];
                            }
                        }
                    derUPole.SumOf (derUPole, derTmpPoles, vBlend[j*vOrder+s]);
                    if (rational)
                        {
                        derUWgts += tmpWeights * vBlend[j*vOrder+s];
                        }
                    }

                /* v partial derivatives */
                derVPole.x = derVPole.y = derVPole.z = 0.0;
                derVWgts = 0.0;
                for (s = 0; s < vDegree; s++)
                    {
                    derTmpPoles.x = derTmpPoles.y = derTmpPoles.z = tmpWeights = 0.0;
                    for (t = 0; t < uOrder; t++)
                        {
                        diffPoles.DifferenceOf (*(patchBezP->poles+(s+1)*uOrder+t), *(patchBezP->poles+s*uOrder+t));
                        derTmpPoles.SumOf (derTmpPoles, diffPoles, uBlend[i*uOrder+t]);
                        if (rational)
                            {
                            diffWgts = patchBezP->weights[(s+1)*uOrder+t]-
                                       patchBezP->weights[s*uOrder+t];
                            tmpWeights += diffWgts * uBlend[i*uOrder+t];
                            }
                        }
                    derVPole.SumOf (derVPole, derTmpPoles, vBlendDer[j*vDegree+s]);
                    if (rational)
                        {
                        derVWgts += tmpWeights * vBlendDer[j*vDegree+s];
                        }
                    }
                }

            if (rational)
                {
                pole.Scale (pole, 1.0/weight);
                }
            meshPointsP[index] = pole;
            if (meshNormP)
                {
                if (rational)
                    {
                    derUPole.SumOf (derUPole, pole, -1.0 *derUWgts);
                    derVPole.SumOf (derVPole, pole, -1.0 *derVWgts);
                    }

                /* Check if derUPole or derVPole dPdV is zero */
                if (derUPole.Magnitude () < fc_epsilon)
                    {
                    vtmp=vLo+j*vStep;
                    vtmp=(vtmp<=fc_epsilon) ? vtmp+fc_epsilon : vtmp-fc_epsilon;
                    bspsurf_evaluateSurfacePoint (NULL, NULL, &derUPole, NULL,
                                        uLo+i*uStep, vtmp, patchBezP);
                    }

                if (derVPole.Magnitude () < fc_epsilon)
                    {
                    utmp=uLo+i*uStep;
                    utmp=(utmp<=fc_epsilon) ? utmp+fc_epsilon : utmp-fc_epsilon;
                    bspsurf_evaluateSurfacePoint (NULL, NULL, NULL, &derVPole,
                                                  utmp, vLo+j*vStep, patchBezP);
                    }

                if (reverse)
                    meshNormP[index].CrossProduct (derVPole, derUPole);
                else
                    meshNormP[index].CrossProduct (derUPole, derVPole);

                meshNormP[index].Normalize ();
                }
            }
wrapup:
    if (uBlend)     msbspline_free (uBlend);
    if (vBlend)     msbspline_free (vBlend);
    if (meshNormP)
        {
        if (uBlendDer)  msbspline_free (uBlendDer);
        if (vBlendDer)  msbspline_free (vBlendDer);
        }

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_hornerSchemeBezier
(
DPoint3d        *ptOut,         /* point on Bezier curve */
double          *wgtOut,        /* NULL if non-rational */
DPoint3d        *polesP,        /* poles of Bezier curve */
double          *weightsP,      /* NULL if non-rational */
int             order,          /* order of Bezier */
double          u               /* value to evaluate at */
)
    {
    int         i, degree, nChoosei;
    double      u1, factor, *wPtr = NULL, sumWgt = 0.0;
    DPoint3d    *pPtr, sumPole;

    /* Initialize tmp variables */
    degree = order - 1;
    nChoosei = 1;
    u1 = 1.0 - u;
    factor = 1.0;
    sumPole.Scale (*polesP, u1);
    if (wgtOut)
        {
        wPtr = weightsP+1;
        sumWgt = *weightsP * u1;
        }

    /* Use Horner Scheme to evaluate Bezier, see Farin's book page 48 */
    for (i=1, pPtr=polesP+1; i<degree; i++, pPtr++)
        {
        factor *= u;
        nChoosei = nChoosei*(order-i)/i;
        if (wgtOut)
            {
            sumWgt = (sumWgt + factor * nChoosei * *wPtr) * u1;
            wPtr++;
            }
        sumPole.SumOf (sumPole, *pPtr, factor*nChoosei);
        sumPole.Scale (sumPole, u1);
        }

    ptOut->SumOf (sumPole, polesP[degree], factor*u);
    if (wgtOut)
        *wgtOut = sumWgt + factor * u * *(weightsP+degree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_evaluatePointsBezier
(
bvector<DPoint3d> &points,
bvector<DVec3d>   &normals,
bool                needNormals,
MSBsplineSurface    *patchBezP,     /* => Bezier patch */
bvector<DPoint2d*> &uvP,
double              uMin,           /* => u Minimum (entire surface) */
double              uMax,           /* => u Maximum (entire surface) */
double              vMin,           /* => v Minimum (entire surface) */
double              vMax,           /* => v Maximum (entire surface) */
bool                reverseNormal   /* => if true negate normal (dPdV X DPdU) */
)
    {
    int             i, j, uOrder, vOrder, uDegree, vDegree;
    bool            rational;
    double          rowWgts[MAX_ORDER], rowUParWgts[MAX_ORDER], rowVParWgts[MAX_ORDER],
                    rowUWgDiff[MAX_ORDER], rowVWgDiff[MAX_ORDER],
                    *rowWtr, *wgtP, *rowUWtr = NULL, *wUDtr = NULL, *rowUWDtr = NULL, tmpUWgt = 0, *rowVWtr = NULL,
                    *wVDtr = 0, *rowVWDtr = 0, tmpVWgt = 0.0, uDelta, vDelta;
    DPoint2d         uv;
    DPoint3d        rowPoles[MAX_ORDER], rowUPoDiff[MAX_ORDER], rowVPoDiff[MAX_ORDER],
                    rowUParPoles[MAX_ORDER], rowVParPoles[MAX_ORDER],
                    *poleP,  *rowPtr, *rowUPtr, *rowUPDtr,
                    *pUDtr, *rowVPtr, *pVDtr, *rowVPDtr;
    DVec3d dPdU, dPdV;
    rational = (0 != patchBezP->rational);
    uOrder = patchBezP->uParams.order;
    vOrder = patchBezP->vParams.order;
    uDegree = uOrder - 1;
    vDegree = vOrder - 1;

    /* Use Horner Scheme on each row at u, then on the column at v */
    uDelta = uMax - uMin;
    vDelta = vMax - vMin;
    
    for (size_t k=0; k < uvP.size (); k++)
        {
        uv.x = (uvP[k]->x - uMin) / uDelta;
        uv.y = (uvP[k]->y - vMin) / vDelta;
        /* Evaluate each row at u value */
        wgtP = NULL;
        rowWtr = NULL;
        if (rational)
            {
            wgtP=patchBezP->weights;
            rowWtr=rowWgts;
            }
        for (i=0, poleP=patchBezP->poles, rowPtr=rowPoles;
                i<vOrder;
                    i++, poleP += uOrder, rowPtr++)
            {
            bspmesh_hornerSchemeBezier(rowPtr, rational ? rowWtr : NULL,
                                       poleP, rational ? wgtP : NULL, uOrder, uv.x);
            if (rational)
                {
                wgtP += uOrder;
                rowWtr++;
                }
            }

        /* Evaluate the column to get the point on the surface */
        DVec3d xyz;
        double   weight = 1.0;
        bspmesh_hornerSchemeBezier(&xyz, rational ? &weight : NULL,
                                   rowPoles, rational ? rowWgts : NULL, vOrder, uv.y);

        if (rational)
            xyz.Scale (xyz, 1.0/weight);
        points.push_back (xyz);

        /* Compute u and v partial derivatives if necessary */
        if (needNormals)
            {
            wgtP = NULL;
            rowUWtr = NULL;
            if (rational)
                {
                wgtP=patchBezP->weights;
                rowUWtr=rowUParWgts;
                }
            for (i=0, poleP=patchBezP->poles, rowUPtr=rowUParPoles;
                    i<vOrder;
                        i++, poleP += uOrder, rowUPtr++)
                {
                /* Form the differences of poles along each row */
                if (rational)
                    {
                    wUDtr=wgtP;
                    rowUWDtr=rowUWgDiff;
                    }
                for (j=0, pUDtr=poleP, rowUPDtr=rowUPoDiff;
                        j<uDegree;
                            j++, pUDtr++, rowUPDtr++)
                    {
                    rowUPDtr->DifferenceOf (pUDtr[1], *pUDtr);
                    if (rational)
                        {
                        *rowUWDtr = *(wUDtr+1) - *wUDtr;
                        wUDtr++;
                        rowUWDtr++;
                        }
                    }

                bspmesh_hornerSchemeBezier (rowUPtr, rational ? rowUWtr : NULL,
                                            rowUPoDiff, rational ? rowUWgDiff : NULL,
                                            uDegree, uv.x);
                if (rational)
                    {
                    wgtP += uOrder;
                    rowUWtr++;
                    }
                }

            /* Evaluate the column to the u partial derivatives */
            bspmesh_hornerSchemeBezier (&dPdU, rational ? &tmpUWgt : NULL,
                                        rowUParPoles,
                                        rational ? rowUParWgts : NULL,
                                        vOrder, uv.y);
            if (rational)
                dPdU.SumOf (dPdU, xyz, -1.0 *tmpUWgt);

            wgtP = NULL;
            rowVWtr = NULL;

            if (rational)
                {
                wgtP=patchBezP->weights;
                rowVWtr=rowVParWgts;
                }
            for (i=0, poleP=patchBezP->poles, rowVPtr=rowVParPoles;
                    i<uOrder;
                        i++, poleP++, rowVPtr++)
                {
                /* Form the differences of poles along each row */
                if (rational)
                    {
                    wVDtr=wgtP;
                    rowVWDtr=rowVWgDiff;
                    }
                for (j=0, pVDtr=poleP, rowVPDtr=rowVPoDiff;
                        j<vDegree;
                            j++, pVDtr += uOrder, rowVPDtr++)
                    {
                    rowVPDtr->DifferenceOf (pVDtr[uOrder], *pVDtr);
                    if (rational)
                        {
                        *rowVWDtr = *(wVDtr+uOrder) - *wVDtr;
                        wVDtr += uOrder;
                        rowVWDtr++;
                        }
                    }

                bspmesh_hornerSchemeBezier (rowVPtr,
                                            rational ? rowVWtr : NULL,
                                            rowVPoDiff,
                                            rational ? rowVWgDiff : NULL,
                                            vDegree, uv.y);
                if (rational)
                    {
                    wgtP++;
                    rowVWtr++;
                    }
                }

            /* Evaluate the column to the u partial derivatives */
            bspmesh_hornerSchemeBezier (&dPdV, rational ? &tmpVWgt : NULL,
                                        rowVParPoles,
                                        rational ? rowVParWgts : NULL,
                                        uOrder, uv.x);
            if (rational)
                dPdV.SumOf (dPdV, xyz, -1.0 *tmpVWgt);

            /* Check if dPdU or zero dPdV is zero */
            if (dPdU.Magnitude () < fc_epsilon)
                {
                bspsurf_evaluateSurfacePoint (NULL, NULL, &dPdU, NULL, uv.x,
                  (uv.y <= fc_epsilon) ? uv.y+fc_epsilon : uv.y-fc_epsilon, patchBezP);
                }

            if (dPdV.Magnitude () < fc_epsilon)
                {
                bspsurf_evaluateSurfacePoint (NULL, NULL, NULL, &dPdV,
                  (uv.x <= fc_epsilon) ? uv.x+fc_epsilon : uv.x-fc_epsilon, uv.y,
                    patchBezP);
                }

            DVec3d normal;
            if (reverseNormal)
                normal.CrossProduct (dPdV, dPdU);
            else
                normal.CrossProduct (dPdU, dPdV);

            normal.Normalize ();
            normals.push_back (normal);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool cb_interceptLess
(
const Intercept &intercept1,
const Intercept &intercept2
)
    {
    return intercept1.sortCoordinate < intercept2.sortCoordinate;
    }


/*---------------------------------------------------------------------------------**//**
* Find intersections of a loop with a cut line. Record edges that touch the loop and head downward in the primary intercept array. Record
* other points (e.g. touch points) in the secondary array
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_computeIntersections
(
InterceptHeader *crossHeaderP,
InterceptHeader *touchHeaderP,
bvector<MeshRegion> &regions,
double          yStart,
double          yEnd
)
    {
    bool        ascending;
    double      x;

    crossHeaderP->clear ();
    touchHeaderP->clear ();

    for (bvector<MeshRegion>::iterator region = regions.begin ();
                region < regions.end ();
                region++)
        {
        size_t numVerts = region->boundary.size ();
        DPoint2d const *p = &region->boundary[0];
        size_t currIndex = 0;
        size_t nextIndex = 1;
        int curr = util_compareDoubles (yStart, p[currIndex].y);
        int next = util_compareDoubles (yStart, p[nextIndex].y);
        for (size_t i=1; i<numVerts; i++)
            {
            int prev = curr;
            size_t prevIndex = currIndex;
            curr = next;
            currIndex = nextIndex;
            nextIndex = (i+1) % (numVerts-1);
            next = util_compareDoubles (yStart, p[nextIndex].y);
            if (curr)
                {
                if (curr*prev < 0)
                    {
                    x = bspmesh_verticalIntercept (yStart, &p[prevIndex], &p[currIndex]);

                    ascending = curr > 0;
                    bspmesh_bufferIntercept (crossHeaderP,
                                             x, yStart, yEnd, &region->boundary,
                                             (int) (ascending ? currIndex : prevIndex),
                                             ascending);
                    }
                }
            else
                {
                if (prev < 0)
                    bspmesh_bufferIntercept (crossHeaderP,
                                             p[currIndex].x, yStart, yEnd, &region->boundary,
                                             (int)currIndex,  true);
                if (next < 0)
                    bspmesh_bufferIntercept (crossHeaderP,
                                             p[currIndex].x, yStart, yEnd, &region->boundary,
                                             (int)currIndex,  false );
                bspmesh_bufferSimpleIntercept (touchHeaderP, p + currIndex, &region->boundary, (int)currIndex );
                }
            }
        }

    std::sort (crossHeaderP->begin (), crossHeaderP->end (), cb_interceptLess);
    std::sort (touchHeaderP->begin (), touchHeaderP->end (), cb_interceptLess);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_extractSimpleIntercepts
(
bvector<DPoint2d> &points,
InterceptHeader const &intercepts,
double x0,
double x1,
int forward
)
    {
    double x;
    ptrdiff_t n = (ptrdiff_t)intercepts.size ();
    ptrdiff_t i;
    if (forward)
        {
        /* Skip over points before the left end */
        for (i = 0; i < n && intercepts[i].start.x <= x0 ; i++)
            {}
        x = x0;
        for (; i < n && intercepts[i].start.x <= x1 ; i++)
            {
            if ( intercepts[i].start.x > x )
                {
                x = intercepts[i].start.x;
                points.push_back (intercepts[i].start);
                }
            }
        }
    else
        {
        /* Skip over points after the right end */
        for ( i = n - 1;
              i >= 0 && intercepts[i].start.x >= x1 ; i--)
            {}
        x = x1;
        for ( ; i >= 0 && intercepts[i].start.x >= x0 ; i--)
            {
            if  (intercepts[i].start.x < x )
                {
                x = intercepts[i].start.x;
                points.push_back (intercepts[i].start);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_divideToMonotonic
(
bvector<MeshRegion> &inputRegions,
bvector<MeshRegion> &monotoneRegions
)
    {
    int             xSlope, firstXSlope, lastXSlope,  ySlope, firstYSlope, lastYSlope,
                    status, pointBufSize, index, nDistinct;
    bool            ascending;
    Intercept       *intersectionP;
    InterceptHeader crossHeader[2], touchHeader[2];
    int             iTop,iBottom;       /* These swap between 0 and 1 on successive cuts */
    double yTop,yBottom,yNext;
    bvector<double> extrema;

    crossHeader[0].clear ();
    touchHeader[0].clear ();
    crossHeader[1].clear ();
    touchHeader[1].clear ();

    status = SUCCESS;
    intersectionP = NULL; 
    monotoneRegions.clear ();
    for (bvector<MeshRegion>::const_iterator sourceRegion = inputRegions.begin ();
                sourceRegion < inputRegions.end (); sourceRegion++)
        {

        firstXSlope = lastXSlope = firstYSlope = lastYSlope = FLAT;
        double lastY = 0.0;
        for (size_t i = 0, i1 = sourceRegion->boundary.size () - 1; i < i1; i++)
            {
            DPoint2d const * pntP = &sourceRegion->boundary[i];
            DPoint2d const * nextPntP = pntP + 1;
            lastY = pntP->y;
            if (nextPntP->x == pntP->x)
                xSlope = FLAT;
            else
                {
                xSlope = (nextPntP->x < pntP->x) ? DECREASING : INCREASING;
                if (lastXSlope * xSlope < 0)
                    extrema.push_back (pntP->y);

                lastXSlope = xSlope;
                if (! firstXSlope )
                    firstXSlope = xSlope;
                }

            if (nextPntP->y == pntP->y)
                ySlope = FLAT;
            else
                {
                ySlope = (nextPntP->y < pntP->y) ? DECREASING : INCREASING;
                if (lastYSlope * ySlope < 0)
                    extrema.push_back (pntP->y);

                lastYSlope = ySlope;
                if (! firstYSlope )
                    firstYSlope = ySlope;
                }
            }
        if (!sourceRegion->boundary.empty ())
            lastY = sourceRegion->boundary.back ().y;
        if (lastXSlope*firstXSlope < 0 || lastYSlope*firstYSlope < 0)
            extrema.push_back (lastY);
        }

    if (extrema.size () == 0)
        goto wrapup;

    // sort DESCENDING and cull duplicates
    mdlUtil_sortDoubles (&extrema[0], (int)extrema.size (), false);
    /* Cull out duplicates */
    nDistinct = 1;
    for (size_t i = 1, n = extrema.size (); i < n; i++)
        {
        if ( extrema[i] < extrema[nDistinct-1] )
            {
            extrema[nDistinct++] = extrema[i];
            }
        }
    extrema.resize (nDistinct);

    if ( nDistinct < 1 )
        goto wrapup;

    iBottom = 1;
    iTop = 0;
    bspmesh_computeIntersections( &crossHeader[iBottom], &touchHeader[iBottom],
                                inputRegions, extrema[0], extrema[1] );
    yBottom = extrema[0];
    /* Top of loop invariant: iBottom is the index of the intercept headers at the
        bottom of the completed portion to yBottom
    */
    for (size_t i=1; i < extrema.size (); i++ )
        {
        /* Swap the old 'bottom' to be the current top */
        yTop = yBottom;
        yBottom = extrema[i];
        iTop = iBottom;
        iBottom = 1 - iTop;
        yNext = i + 1 < extrema.size () ? extrema[i+1] : yBottom - 1.0;

        /* Evaluate the bottom level intersections.  The intersections themselves
                will not be used until the next pass, but the touch points will be
                used in this pass
        */
        bspmesh_computeIntersections (&crossHeader[iBottom], &touchHeader[iBottom],
                                      inputRegions, yBottom, yNext );
        intersectionP = crossHeader[iTop].size() > 0 ? crossHeader[iTop].data () : nullptr;
        /* Pick out intercepts with the top cut.  Walk down the left, pick up the touches
                on the bottom, walk up the right, and pick out the touches on the top.
        */
        for (size_t j=0; j < crossHeader[iTop].size () ; j += 2 )
            {
            monotoneRegions.push_back (MeshRegion ());
            MeshRegion &newRegion = monotoneRegions.back();

            pointBufSize = 0;

            /* Left Side */
            ascending   = intersectionP[j].ascending;
            bvector<DPoint2d> * boundP      = intersectionP[j].boundP;
            int boundPoints = (int)boundP->size () - 1;
            newRegion.boundary.push_back (intersectionP[j].start);

            for (index = NEXTLOWER(intersectionP[j].startIndex, boundPoints, ascending);
                    index != intersectionP[j].endIndex;
                        index = NEXTLOWER(index, boundPoints, ascending))
                newRegion.boundary.push_back (boundP->at(index));

            newRegion.boundary.push_back (intersectionP[j].end);

            bspmesh_extractSimpleIntercepts(newRegion.boundary,
                                touchHeader[iBottom],
                                intersectionP[j].end.x, intersectionP[j+1].end.x,
                                true
                                );
            /* Right Side */
            ascending   = intersectionP[j+1].ascending;
            boundP      = intersectionP[j+1].boundP;
            boundPoints = (int)boundP->size () - 1;

            newRegion.boundary.push_back (intersectionP[j+1].end);
            for (index = PREVLOWER(intersectionP[j+1].endIndex, boundPoints, ascending);
                    index != intersectionP[j+1].startIndex;
                        index = PREVLOWER(index, boundPoints, ascending))
                newRegion.boundary.push_back (boundP->at (index));

            newRegion.boundary.push_back (intersectionP[j+1].start);

            bspmesh_extractSimpleIntercepts(newRegion.boundary,
                                touchHeader[iTop],
                                intersectionP[j].start.x, intersectionP[j+1].start.x,
                                false
                                );

            /* Closure */
            newRegion.close ();
            }
        }

wrapup:

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_scaleVector2d
(
DPoint2d    *outP,
DPoint2d    *inP,
DPoint2d    *scaleP
)
    {
    outP->x = inP->x * scaleP->x;
    outP->y = inP->y * scaleP->y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_processTriangle
(
MeshParams          *mpP,
bvector<DPoint2d*>  &uvPP,
bvector<DPoint3d>   &pointP,
bvector<DVec3d>     &normalP,
int                 i0,
int                 i1,
int                 i2
)
    {
    int             iTmp;
    double          dotProduct;
#ifdef NOTRISTRIPS
    DPoint2d        params[3];
    DPoint3d        points[3], normals[3];
#endif

#if defined (triangle_debug)
    debug_displayUVLine (uvPP[i0]->x, uvPP[i0]->y, uvPP[i1]->x, uvPP[i1]->y, 9, 0, 0);
    debug_displayUVLine (uvPP[i1]->x, uvPP[i1]->y, uvPP[i2]->x, uvPP[i2]->y, 9, 0, 0);
    debug_displayUVLine (uvPP[i2]->x, uvPP[i2]->y, uvPP[i0]->x, uvPP[i0]->y, 9, 0, 0);
#endif

   dotProduct = (uvPP[i2]->x - uvPP[i1]->x) * (uvPP[i0]->y - uvPP[i1]->y) -
                (uvPP[i0]->x - uvPP[i1]->x) * (uvPP[i2]->y - uvPP[i1]->y);

   if (mpP->reverse ? dotProduct > 0.0 : dotProduct < 0.0)
        {
        iTmp = i0;
        i0   = i2;
        i2   = iTmp;
        }
#ifdef NOTRISTRIPS
    points[0]  = pointP[i0];
    points[1]  = pointP[i1];
    points[2]  = pointP[i2];

    if (mpP->normalsRequired)
        {
        normals[0] = normalP[i0];
        normals[1] = normalP[i1];
        normals[2] = normalP[i2];
        }

    if (mpP->parametersRequired)
        {
        bspmesh_scaleVector2d (&params[0], uvPP[i0], &mpP->paramScale);
        bspmesh_scaleVector2d (&params[1], uvPP[i1], &mpP->paramScale);
        bspmesh_scaleVector2d (&params[2], uvPP[i2], &mpP->paramScale);
        }

    return (mpP->triangleFunction)(points, normals, params, 3, mpP->userDataP);
#else
        return tristrip_addTriangle(i0,i1,i2);
#endif
    }
/*---------------------------------------------------------------------------------**//**
* Triangulate in a region known to be a simple strip in which triangulation edgs always go from one side to the other using either X or Y as
* the sweep direction. Point loop is assumed to have no duplicates. Do not duplicate the end point 'start' index of -1 indicates last point is
* the start
* @bsimethod                                                    earlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_triangulateStrip
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d*>  &uvPP,
int                 iMin,                       /* => index of start of strip */
int                 direction,                  /* => 0 for vertical sweep, 1 for horizontal */
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
)
    {
    int                 left, right, base, probe, count, limit, status = SUCCESS;

    double              xLimit, yLimit;
    int nPnts = (int)uvPP.size ();
    INCREMENT_COUNTER(BSPMESH_COUNTER_TRIANGULATESTRIP);

    if (nPnts < 3) return SUCCESS;

    if ( iMin < 0 ) iMin = nPnts - 1;
    if ( iMin >= nPnts ) iMin = 0;

    bvector<DPoint3d> points;
    bvector<DVec3d>normals;

    bspmesh_evaluatePointsBezier (points, normals, mpP->normalsRequired,
                          bezierP, uvPP,
                          uMin, uMax, vMin, vMax, 0 != mpP->reverse);
    tristrip_init
                (
                mpP->parametersRequired ? &uvPP[0] : NULL,
                &points[0],
                mpP->normalsRequired ? &normals[0] : NULL,
                (int(*)())mpP->triangleFunction,
                mpP->userDataP,
                &mpP->paramScale
                );

    limit = nPnts - 2;

    if ( direction == BSPMESH_HORIZONTAL_STRIP )
        {
        base = iMin;
        count = 0;
        left  = PREV(base, nPnts);
        right = NEXT(base, nPnts);
        /* shift base by up to one to either side */
        probe = NEXT(right,nPnts);
        xLimit = 0.5*( uvPP[left]->x + uvPP[base]->x );
        if ( uvPP[probe]->x < xLimit )
            {
            left = base;
            base = right;
            right = probe;
            }
        else
            {
            probe = PREV(left,nPnts);
            xLimit = 0.5*( uvPP[base]->x + uvPP[right]->x );
            if( uvPP[probe]->x < xLimit )
                {
                right = base;
                base = left;
                left = probe;
                }
            }

        while (left != right && count < limit )
            {
            if ( uvPP[left]->x < uvPP[right]->x )
                {
                /* advance along the left */
                probe = NEXT(right,nPnts);
                if ( uvPP[probe]->x > uvPP[right]->x )
                    xLimit = 0.5* (uvPP[probe]->x + uvPP[right]->x);
                else
                    xLimit = uvPP[right]->x;

                while ( right != left && uvPP[left]->x <= xLimit )
                    {
                    if ((status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                          base,right,left )))
                        goto wrapup;
                    base = left;
                    left = PREV(base,nPnts);
                    count++;
                    }
                left = base;
                base = right;
                right = NEXT(right, nPnts);
                }
            else
                {
                /* advance along the right */
                probe = PREV(left,nPnts);
                if ( uvPP[probe]->x > uvPP[left]->x )
                    xLimit = 0.5* (uvPP[probe]->x + uvPP[left]->x);
                else
                    xLimit = uvPP[left]->x;

                while ( right != left && uvPP[right]->x <= xLimit )
                    {
                    if ((status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                          base,right,left )))
                        goto wrapup;
                    base = right;
                    right = NEXT(base,nPnts);
                    count++;
                    }
                right = base;
                base = left;
                left = PREV(base, nPnts);
                }
            }
        }
    else if ( direction == BSPMESH_VERTICAL_STRIP )
        {

        base = iMin;
        count = 0;
        left  = PREV(base, nPnts);
        right = NEXT(base, nPnts);

        base = iMin;
        count = 0;
        left  = PREV(base, nPnts);
        right = NEXT(base, nPnts);
        /* shift base by up to one to either side */
        probe = NEXT(right,nPnts);
        yLimit = 0.5*( uvPP[left]->y + uvPP[base]->y );
        if ( uvPP[probe]->y < yLimit )
            {
            left = base;
            base = right;
            right = probe;
            }
        else
            {
            probe = PREV(left,nPnts);
            yLimit = 0.5*( uvPP[base]->y + uvPP[right]->y );
            if( uvPP[probe]->y < yLimit )
                {
                right = base;
                base = left;
                left = probe;
                }
            }



        while (left != right && count < limit )
            {
            if ( uvPP[left]->y < uvPP[right]->y )
                {
                /* advance along the left */
                probe = NEXT(right,nPnts);
                if ( uvPP[probe]->y > uvPP[right]->y )
                    yLimit = 0.5* (uvPP[probe]->y + uvPP[right]->y);
                else
                    yLimit = uvPP[right]->y;

                while ( right != left && uvPP[left]->y <= yLimit )
                    {
                    if ((status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                          base,right,left )))
                        goto wrapup;
                    base = left;
                    left = PREV(base,nPnts);
                    count++;
                    }
                left = base;
                base = right;
                right = NEXT(right, nPnts);
                }
            else
                {
                /* advance along the right */
                probe = PREV(left,nPnts);
                if ( uvPP[probe]->y > uvPP[left]->y )
                    yLimit = 0.5* (uvPP[probe]->y + uvPP[left]->y);
                else
                    yLimit = uvPP[left]->y;

                while ( right != left && uvPP[right]->y <= yLimit )
                    {
                    if ((status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                          base,right,left )))
                        goto wrapup;
                    base = right;
                    right = NEXT(base,nPnts);
                    count++;
                    }
                right = base;
                base = left;
                left = PREV(base, nPnts);
                }
            }
        }
    wrapup:
    if(status == 0)
        tristrip_flush();

    return status;
    }

struct TriangulationStackFrame
    {
    int         index;
    bool        left;
    TriangulationStackFrame (int _index, bool _left)
        : index(_index), left (_left)
        {
        }
    TriangulationStackFrame ()
        : index(0), left(false)
        {
        }
    };

struct TriangulationStack : bvector <TriangulationStackFrame>
    {
    void push (int index, bool left)
        {
        push_back (TriangulationStackFrame (index, left));
        }
    };

struct IndexPair { int x, y;};
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_triangulate
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d*>   &uvPP,
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
)
    {
    int             i, iLeft, iRight, leftEnd, next, status;
    bool            adjacentX1, adjacentXi, vertical, nextLeft;
    double          normal, crossProduct;
    IndexPair       minIndex, maxIndex;
    DPoint2d        d1, d2, min, max, *prevP, *nextP, *currP, prevDelta, nextDelta;
    bvector<DPoint3d>points;
    bvector<DVec3d> normals;

    int nPnts = (int)uvPP.size () - 1;                   /* Ignore closure point */

    INCREMENT_COUNTER(BSPMESH_COUNTER_TRIANGULATE);

    /* Remove degenerate (double-back) vertices */
    int numAccept = 0;
    for (i=0; i<nPnts && nPnts > 2; i++)
        {
        currP = uvPP[i];
        prevP = uvPP[PREV(i, nPnts)];
        nextP = uvPP[NEXT(i, nPnts)];
        prevDelta.x = prevP->x - currP->x;
        prevDelta.y = prevP->y - currP->y;
        nextDelta.x = nextP->x - currP->x;
        nextDelta.y = nextP->y - currP->y;

        if (fabs (prevDelta.x * nextDelta.y  - nextDelta.x * prevDelta.y) < 1.0E-10 &&
            prevDelta.x * nextDelta.x + prevDelta.y * nextDelta.y >= 0.0)
            {
            }
        else
            {
            uvPP[numAccept] = uvPP[i];
            numAccept++;
            }
        }
    nPnts = numAccept;
    if (nPnts < 3) return SUCCESS;

    bspmesh_evaluatePointsBezier (points, normals, mpP->normalsRequired,
                          bezierP, uvPP,
                          uMin, uMax, vMin, vMax, 0 != mpP->reverse);

    tristrip_init(  
                    mpP->parametersRequired ? &uvPP[0] : NULL,
                    &points[0],
                    mpP->normalsRequired ? &normals[0] : NULL,
                    (int(*)())mpP->triangleFunction,
                    mpP->userDataP,
                    &mpP->paramScale);


    min = max = *uvPP[0];
    minIndex.x = minIndex.y = maxIndex.x = maxIndex.y = 0;
    normal = 0.0;
    prevP = uvPP[nPnts-1];
    for (i=0; i<nPnts; i++)
        {
        if (uvPP[i]->x < min.x)
            {
            minIndex.x = i;
            min.x = uvPP[i]->x;
            }
        if (uvPP[i]->y < min.y)
            {
            minIndex.y = i;
            min.y = uvPP[i]->y;
            }
        if (uvPP[i]->x > max.x)
            {
            maxIndex.x = i;
            max.x = uvPP[i]->x;
            }
        if (uvPP[i]->y > max.y)
            {
            maxIndex.y = i;
            max.y = uvPP[i]->y;
            }

        normal += (prevP->x - uvPP[i]->x) * (prevP->y + uvPP[i]->y);
        prevP = uvPP[i];
        }

    if (vertical = ((max.y - min.y)/(vMax-vMin) > (max.x - min.x)/(uMax-uMin)))
        {
        iLeft      = maxIndex.y;
        leftEnd    = NEXT(minIndex.y, nPnts);
        iRight     = PREV(maxIndex.y, nPnts);
        }
    else
        {
        iLeft      = maxIndex.x;
        leftEnd    = NEXT(minIndex.x, nPnts);
        iRight     = PREV(maxIndex.x, nPnts);
        }


    
    TriangulationStack stack;
    stack.push (iLeft, true);

    iLeft = NEXT(iLeft, nPnts);

    if (vertical ? uvPP[iLeft]->y > uvPP[iRight]->y : uvPP[iLeft]->x > uvPP[iRight]->x)
        {
        stack.push (iLeft, true);
        iLeft = NEXT(iLeft, nPnts);
        }
    else
        {
        stack.push (iLeft, false);
        iRight = PREV(iRight, nPnts);
        }

    while (true)
        {
        if (iLeft != leftEnd &&
            vertical ? uvPP[iLeft]->y > uvPP[iRight]->y : uvPP[iLeft]->x > uvPP[iRight]->x)
            {
            next = iLeft;
            nextLeft = true;
            iLeft = NEXT(iLeft, nPnts);
            }
        else
            {
            next = iRight;
            nextLeft = false;
            iRight = PREV(iRight, nPnts);
            }

        adjacentX1 = (next == PREV(stack[0].index, nPnts)) ||
                     (next == NEXT(stack[0].index, nPnts));
        adjacentXi = (next == PREV(stack.back ().index, nPnts)) ||
                     (next == NEXT(stack.back ().index, nPnts));

        if (adjacentX1 && adjacentXi)
            {
            for (size_t i=0; i < stack.size () - 1; i++)
                if (status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                next, stack[i].index, stack[i+1].index))
                    goto wrapup;

            break;
            }
        else if (adjacentX1)
            {
            for (size_t i=0; i < stack.size () - 1; i++)
                if (status = bspmesh_processTriangle (mpP, uvPP, points, normals, 
                                                next, stack[i].index, stack[i+1].index))
                    goto wrapup;

            stack.front() = stack.back();
            stack.resize (1);
            stack.push (next, nextLeft);
            }
        else if (adjacentXi)
            {
            do
                {
                size_t iStack = stack.size ();
                d1.x = uvPP[stack[iStack-2].index]->x - uvPP[stack[iStack-1].index]->x;
                d1.y = uvPP[stack[iStack-2].index]->y - uvPP[stack[iStack-1].index]->y;
                d2.x = uvPP[next]->x                   - uvPP[stack[iStack-1].index]->x;
                d2.y = uvPP[next]->y                   - uvPP[stack[iStack-1].index]->y;
                crossProduct = stack[iStack-1].left ?
                               d2.x * d1.y - d1.x * d2.y : d1.x * d2.y - d2.x * d1.y;

                if (crossProduct * normal >= 0.0)
                    {
                    if (crossProduct != 0.0 &&
                       (status = bspmesh_processTriangle (mpP, uvPP, points, normals,
                                                          stack[iStack-2].index,
                                                          stack[iStack-1].index, next)))
                        goto wrapup;

                    stack.pop_back ();
                    }
                else
                    {
                    break;
                    }

                } while (stack.size ());

            stack.push (next, nextLeft);
            }
        }
wrapup:
    if(status == SUCCESS)
        tristrip_flush();

    return status;
    }
 #endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileCoordinates
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
double              u0,                         /* => lower u bound */
double              u1,                         /* => upper u bound */
double              v0,                         /* => lower v bound */
double              v1,                         /* => upper v bound*/
int                 nU,                         /* => number of u tile POINTS */
int                 nV,                         /* => number of v tile POINTS */
double              uMin,                       /* => u Minimum for patch */
double              uMax,                       /* => u Maximum for patch */
double              vMin,                       /* => v Minimum for patch */
double              vMax,                       /* => v Maximum for patch*/
DPoint2d            *paramScaleP,               /* => parameter scale */
bool                reverse
)
    {
    int             status, totalPoints, i, j;

    DPoint2d        paramBuf[STACK_POINTS], *paramP = paramBuf, *pntP, pnt, min, max,
                    paramMin, paramDelta;
    DPoint3d        pointBuf[STACK_POINTS], *pointP = pointBuf,
                    normalBuf[STACK_POINTS], *normalP = normalBuf;
    DPoint2d delta;

    totalPoints = nU * nV;

    if ( nU < 2 || nV < 2 )
        return SUCCESS;

    if (totalPoints > STACK_POINTS)
        {
        if (! (pointP = (DPoint3d*)msbspline_malloc (totalPoints * sizeof(DPoint3d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (mpP->normalsRequired &&
            ! (normalP = (DPoint3d*)msbspline_malloc (totalPoints * sizeof(DPoint3d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (mpP->parametersRequired &&
            ! (paramP = (DPoint2d*)msbspline_malloc (totalPoints * sizeof(DPoint2d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    min.x = u0;
    min.y = v0;
    delta.x = ( u1 - u0 ) / ( nU - 1 );
    delta.y = ( v1 - v0 ) / ( nV - 1 );

    bspmesh_scaleVector2d (&paramMin, &min, paramScaleP);
    bspmesh_scaleVector2d (&paramDelta, &delta, paramScaleP);
    if (mpP->parametersRequired)
        {
        for (pntP=paramP, i=0, pnt.y=paramMin.y;
                i < nV;
                    i++, pnt.y += paramDelta.y)
            for (j=0, pnt.x = paramMin.x;
                    j < nU;
                        j++, pnt.x += paramDelta.x)
                *pntP++ = pnt;
        }

    min.x = (min.x - uMin)/(uMax - uMin);
    min.y = (min.y - vMin)/(vMax - vMin);
    max.x = (u1 - uMin)/(uMax - uMin);
    max.y = (v1 - vMin)/(vMax - vMin);
    if (! (status = bspmesh_evaluateMeshBezier (pointP,
                                                mpP->normalsRequired ? normalP : NULL,
                                                bezierP, min.x, max.x, min.y, max.y,
                                                nU, nV, reverse)) && NULL != mpP->meshFunction)
        status = (mpP->meshFunction) (pointP,
                        mpP->normalsRequired ? normalP : NULL,
                        mpP->parametersRequired ? paramP : NULL,
                        nU, nV, mpP->userDataP);

#if defined (normal_debug)
    debug_displayNormals (pointP, normalP, nU*nV);
#endif

wrapup:
    if (pointP  && pointP  != pointBuf)  msbspline_free (pointP);
    if (paramP  && paramP  != paramBuf)  msbspline_free (paramP);
    if (normalP && normalP != normalBuf) msbspline_free (normalP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tile
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
int                 widthMinIndex,              /* => width minimum index */
int                 widthMaxIndex,              /* => width maximum index */
int                 heightMinIndex,             /* => height minimum index */
int                 heightMaxIndex,             /* => height maximum index */
int                 horizontal,                 /* => true if horizontal slice */
DPoint2d            *deltaP,                    /* => step delta */
double              uMin,                       /* => u Minimum for patch */
double              uMax,                       /* => u Maximum for patch */
double              vMin,                       /* => v Minimum for patch */
double              vMax,                       /* => v Maximum for patch*/
DPoint2d            *paramScaleP,               /* => parameter scale */
bool                reverse
)
    {
    int             status, totalPoints, i, j, nU, nV, uMinIndex, uMaxIndex,
                    vMinIndex, vMaxIndex;
    DPoint2d        paramBuf[STACK_POINTS], *paramP = paramBuf, *pntP, pnt, min, max,
                    paramMin, paramDelta;
    DPoint3d        pointBuf[STACK_POINTS], *pointP = pointBuf,
                    normalBuf[STACK_POINTS], *normalP = normalBuf;

    if (horizontal)
        {
        uMinIndex = heightMinIndex;
        uMaxIndex = heightMaxIndex;
        vMinIndex = widthMinIndex;
        vMaxIndex = widthMaxIndex;
        }
    else
        {
        uMinIndex = widthMinIndex;
        uMaxIndex = widthMaxIndex;
        vMinIndex = heightMinIndex;
        vMaxIndex = heightMaxIndex;
        }

    nU = (uMaxIndex - uMinIndex + 1);
    nV = (vMaxIndex - vMinIndex + 1);
    totalPoints = nU * nV;

    if (totalPoints > STACK_POINTS)
        {
        if (! (pointP = (DPoint3d*)msbspline_malloc (totalPoints * sizeof(DPoint3d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (mpP->normalsRequired &&
            ! (normalP = (DPoint3d*)msbspline_malloc (totalPoints * sizeof(DPoint3d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (mpP->parametersRequired &&
            ! (paramP = (DPoint2d*)msbspline_malloc (totalPoints * sizeof(DPoint2d), NULL)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    min.x = uMinIndex * deltaP->x;
    min.y = vMinIndex * deltaP->y;

    bspmesh_scaleVector2d (&paramMin, &min, paramScaleP);
    bspmesh_scaleVector2d (&paramDelta, deltaP, paramScaleP);
    if (mpP->parametersRequired)
        {
        for (pntP=paramP, i=vMinIndex, pnt.y=paramMin.y;
                i <= vMaxIndex;
                    i++, pnt.y += paramDelta.y)
            for (j=uMinIndex, pnt.x = paramMin.x;
                    j <= uMaxIndex;
                        j++, pnt.x += paramDelta.x)
                *pntP++ = pnt;
        }

    min.x = (min.x - uMin)/(uMax - uMin);
    min.y = (min.y - vMin)/(vMax - vMin);
    max.x = ((double) uMaxIndex * deltaP->x - uMin)/(uMax - uMin);
    max.y = ((double) vMaxIndex * deltaP->y - vMin)/(vMax - vMin);
    if (! (status = bspmesh_evaluateMeshBezier (pointP,
                                                mpP->normalsRequired ? normalP : NULL,
                                                bezierP, min.x, max.x, min.y, max.y,
                                                nU, nV, reverse)) && NULL != mpP->meshFunction)
        status = (mpP->meshFunction) (pointP,
                        mpP->normalsRequired ? normalP : NULL,
                        mpP->parametersRequired ? paramP : NULL,
                        nU, nV, mpP->userDataP);

#if defined (normal_debug)
    debug_displayNormals (pointP, normalP, nU*nV);
#endif

wrapup:
    if (pointP  && pointP  != pointBuf)  msbspline_free (pointP);
    if (paramP  && paramP  != paramBuf)  msbspline_free (paramP);
    if (normalP && normalP != normalBuf) msbspline_free (normalP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_rectangularFaceMesh
(
MeshParams *mpP,
DPoint2d *pointP,
int nPoint,
int iXMin,              /* => min point in lexical x order (upper left) */
int iXMax,              /* => max point in lexical x order (lower right)*/
int iYMin,              /* => min point in lexical y order (lower left) */
int iYMax,              /* => max point in lexical y order (upper right) */
PatchMeshParams *pmpP   /* => meshing parameters for this bezier patch */
)
    {
    DPoint2d llTile,lrTile, ulTile, urTile;
    double umin,umax,vmin,vmax; /* min, max of square */
    double duBezier,dvBezier, du, dv;
    int nu,nv;
    int status = SUCCESS;

    INCREMENT_COUNTER(BSPMESH_COUNTER_RECTANGULAR_PATCH);
    duBezier = ( pmpP->umax - pmpP->umin ) / pmpP->uSteps;
    dvBezier = ( pmpP->vmax - pmpP->vmin ) / pmpP->vSteps;

    umin = pointP[iXMin].x;
    umax = pointP[iXMax].x;
    vmin = pointP[iYMin].y;
    vmax = pointP[iYMax].y;

    /*
    nu = integerizeStepCount ( ( pmpP->umax - pmpP->umin ) / duBezier );
    nv = integerizeStepCount ( ( pmpP->vmax - pmpP->vmin ) / dvBezier );
    */
    nu = integerizeStepCount ( (umax-umin) / duBezier );
    nv = integerizeStepCount ( (vmax-vmin) / dvBezier );
    //if ( nu < 2 || nv < 2 ) return ERROR;
    if ( nu < 2 )
        {
        nu = 2;
        }
    if ( nv < 2 )
        {
        nv = 2;
        }
    du = ( umax - umin ) / nu;
    dv = ( vmax - vmin ) / nv;

    /* Corner coordinates of tiled area */
    llTile.x = ulTile.x = umin + du;
    lrTile.x = urTile.x = umax - du;
    llTile.y = lrTile.y = vmin + dv;
    ulTile.y = urTile.y = vmax - dv;
    /* Explicit assignments to prevent roundoff mismatch when there is a single interior
        line */
    if ( nu == 2)
        {
        urTile.x = lrTile.x = llTile.x;
        }
    if ( nv == 2)
        {
        ulTile.y = urTile.y = lrTile.y;
        }

    /* Output tile section */
    if ( nu > 1 && nv > 1 )
        {
        status = bspmesh_tileCoordinates (mpP,pmpP->bezierP,
                         llTile.x, lrTile.x, llTile.y,urTile.y,
                         nu - 1, nv - 1,
                         pmpP->umin, pmpP->umax,
                         pmpP->vmin, pmpP->vmax,
                         &mpP->paramScale, 0 != mpP->reverse);
        }

    if (SUCCESS == status)
        {
     /* Bottom Edge Triangulation */
        bspmesh_clearOutputBuffer( mpP );
        bspmesh_addDistinctPointsToOutputBuffer( mpP, pointP, nPoint, iYMin, iXMax, pmpP );
        bspmesh_addLatticePointsToOutputBuffer ( mpP, &lrTile, &llTile, nu - 1 );
        status = bspmesh_triangulateOutputBufferAsStrip( mpP, pmpP,
                                        BSPMESH_TRIANGULATE_FROM_FIRST_POINT,
                                        BSPMESH_HORIZONTAL_STRIP );
        }

    if (SUCCESS == status)
        {
        /* Right edge */
        bspmesh_clearOutputBuffer( mpP );
        bspmesh_addDistinctPointsToOutputBuffer( mpP, pointP, nPoint, iXMax, iYMax, pmpP );
        bspmesh_addLatticePointsToOutputBuffer ( mpP, &urTile, &lrTile, nv - 1 );
        status = bspmesh_triangulateOutputBufferAsStrip( mpP, pmpP,
                                        BSPMESH_TRIANGULATE_FROM_FIRST_POINT,
                                        BSPMESH_VERTICAL_STRIP );
        }

    if (SUCCESS == status)
        {
        /* Top edge */
        bspmesh_clearOutputBuffer( mpP );
        bspmesh_addLatticePointsToOutputBuffer ( mpP, &ulTile, &urTile, nu - 1 );
        bspmesh_addDistinctPointsToOutputBuffer( mpP, pointP, nPoint, iYMax, iXMin, pmpP );
        status = bspmesh_triangulateOutputBufferAsStrip( mpP, pmpP,
                                        BSPMESH_TRIANGULATE_FROM_LAST_POINT,
                                        BSPMESH_HORIZONTAL_STRIP
                                        );
        }

    if (SUCCESS == status)
        {
        /* Left edge */
        bspmesh_clearOutputBuffer( mpP );
        bspmesh_addLatticePointsToOutputBuffer ( mpP, &llTile, &ulTile, nv - 1 );
        bspmesh_addDistinctPointsToOutputBuffer( mpP, pointP, nPoint, iXMin, iYMin, pmpP );
        status = bspmesh_triangulateOutputBufferAsStrip( mpP, pmpP,
                                        BSPMESH_TRIANGULATE_FROM_LAST_POINT,
                                        BSPMESH_VERTICAL_STRIP );
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_lexicalYExtrema
(
int *iMinP,             /* <= index of point with min y (min x ) */
int *iMaxP,             /* <= index of point with max y (max x ) */
DPoint2d *pointP,       /* => point array */
int nPoint              /* => number of points */
)
    {
    double yMin,yMax;
    DPoint2d *currP, *maxP, *minP;
    int i;
    /* y lexical tests */
    *iMinP = 0;
    *iMaxP = 0;
    yMin = yMax = pointP[0].y;
    currP = minP = maxP = pointP;
    for ( i = 1 , currP = pointP + 1 ; i < nPoint ; i++, currP++ )
        {
        if  ( currP->y < yMin ||
                ( currP->y == yMin && currP->x < minP->x )
            )
            {
            *iMinP = i;
            yMin = currP->y;
            minP = currP;
            }
        else if ( currP->y > yMax ||
                ( currP->y == yMax && currP->x > maxP->x )
            )
            {
            *iMaxP = i;
            yMax = currP->y;
            maxP = currP;
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Find points with min and max x, using -y to resolve equal x (i.e. find lower left when viewing +X as 'up', +Y as 'LEFT')
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_lexicalXExtrema
(
int *iMinP,             /* <= index of point with min x (max y ) */
int *iMaxP,             /* <= index of point with max x (min y ) */
DPoint2d *pointP,       /* => point array */
int nPoint              /* => number of points */
)
    {
    double xMin,xMax;
    DPoint2d *currP, *maxP, *minP;
    int i;
    /* y lexical tests */
    *iMinP = 0;
    *iMaxP = 0;
    xMin = xMax = pointP[0].x;
    currP = minP = maxP = pointP;
    for ( i = 1 , currP = pointP + 1 ; i < nPoint ; i++, currP++ )
        {
        if  ( currP->x < xMin ||
                ( currP->x == xMin && currP->y > minP->y )
            )
            {
            *iMinP = i;
            xMin = currP->x;
            minP = currP;
            }
        else if ( currP->x > xMax ||
                ( currP->x == xMax && currP->y < maxP->y )
            )
            {
            *iMaxP = i;
            xMax = currP->x;
            maxP = currP;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Mesh a region using simplest possible methods. Return SUCCESS if one or another method worked, ERROR if not.
* @bsimethod                                                    Earlin.Lutz     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_meshSimpleBezierRegion
(
MeshParams          *mpP,                       /* => mesh parameters */
DPoint2d            *pointP,                    /* => slice boundaries */
int                 nPoint,                     /* => number of slice bound points */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax                  /* => v Maximum for bezier */
)
    {
    int status = ERROR;
    int iXMin,iXMax,iYMin,iYMax;
    //int lastPoint = nPoint - 1;
    PatchMeshParams patchMeshParams;
    PatchMeshParams *pmpP = &patchMeshParams;

    bspmesh_lexicalXExtrema ( &iXMin, &iXMax, pointP, nPoint );
    bspmesh_lexicalYExtrema ( &iYMin, &iYMax, pointP, nPoint );

    INCREMENT_COUNTER(BSPMESH_COUNTER_TESTED_PATCH);

    if (        pointP[iYMin].y == pointP[iXMax].y
        &&      pointP[iYMax].y == pointP[iXMin].y
        &&      pointP[iYMin].x == pointP[iXMin].x
        &&      pointP[iYMax].x == pointP[iXMax].x
        )
        {
        pmpP->umin = bezierUMin;
        pmpP->umax = bezierUMax;
        pmpP->vmin = bezierVMin;
        pmpP->vmax = bezierVMax;
        pmpP->bezierP = bezierP;

        bspmesh_getSurfaceSteps (
                                &pmpP->uSteps,   &pmpP->vSteps,
                                &pmpP->uLoSteps, &pmpP->uHiSteps,
                                &pmpP->vLoSteps, &pmpP->vHiSteps,
                              bezierP, mpP);
        status = bspmesh_rectangularFaceMesh
                        (
                        mpP,
                        pointP,nPoint,
                        iXMin, iXMax, iYMin, iYMax,
                        pmpP
                        );
        }
    return status;
    }
#define MAX_TRISTRIP_NODES 120
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspmesh_tristripLength
(
VuP             startP,
VuMask          notAStartPointmask,     /* => Mask of visited edges */
VuMask          scratchMask             /* => Mask that can be set and cleared
                                                as required here. Assumed clear
                                                on all nodes at first, and cleared
                                                again at end.
                                        */
)
    {
    VuP node0P, node1P, node2P, node3P;
    int count = 0;
    int toggle = 1;
    int nFace = 0;
    VuP listP[MAX_TRISTRIP_NODES];
    int arrayLimit;
    VuMask stopMask = notAStartPointmask | scratchMask;
    /* Each pass through the loop checks the face from node0P. If
       acceptable, it is marked and node0P advances to the next face
       in tristrip order.   If not, node0P is nulled to cancel the loop
    */
    node0P = startP;
    arrayLimit = MAX_TRISTRIP_NODES - 3;
    for (count = 0; node0P && count < arrayLimit;)
        {
        node1P = vu_fsucc(node0P);
        node2P = vu_fsucc(node1P);
        node3P = vu_fsucc(node2P);
        if (   node3P == node0P
            && !vu_getMask (node0P, stopMask)
            /* Don't bother checking the rest -- these masks all change on
                a per-face basis */
           )
            {
            vu_setMask (node0P, scratchMask);
            vu_setMask (node1P, scratchMask);
            vu_setMask (node2P, scratchMask);
            listP[count++] = node0P;
            listP[count++] = node1P;
            listP[count++] = node2P;
            nFace++;
            if (toggle)
                {
                node0P = vu_edgeMate(node1P);
                }
            else
                {
                node0P = vu_edgeMate(node2P);
                }

            toggle = 1 - toggle;
            }
        else
            {
            node0P = NULL;
            }
        }
    /* The count nodes recorded in listP need to have the scratch bits cleared */
    for (;count--;)
        {
        vu_clrMask (listP[count], scratchMask);
        }
    return nFace;
    }

/*---------------------------------------------------------------------------------**//**
* Given a pointer to some vertex on a face, find the direction to proceed so as to get the longest tristrip. Return a pointer to the best
* start (always on this face).
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static VuP     bspmesh_findLongestStrip
(
VuP             startP,
VuMask          notAStartPointMask,
VuMask          scratchMask,
int             method
)
    {
    VuP faceP = startP;
    if (method == BSPMESH_OUTPUT_TRISTRIP)
        {
        int maxLength = 0;
        int length;
        VU_FACE_LOOP (currP, startP)
            {
            length = bspmesh_tristripLength (currP, notAStartPointMask, scratchMask);
            if (length > maxLength)
                {
                faceP = currP;
                maxLength = length;
                }
            }
        END_VU_FACE_LOOP (currP, startP)
        }
    return faceP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_dumpVuTristrip
(
VuSetP              graphP,                     /* => triangulated vertex use graph */
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax,                 /* => v Maximum for bezier */
int                 method                      /* => BSPMESH_OUTPUT_TRISTRIP, BSPMESH_OUTPUT_TRIFAN */
)
    {

    int nVertex;
    int status = SUCCESS;

    VuArrayP vertexArrayP = vu_grabArray( graphP );
    vu_collectVertexLoops(vertexArrayP,graphP);
    nVertex = vu_arraySize( vertexArrayP );


    bvector<DPoint3d> points;
    bvector<DVec3d> normals;
    bvector<DPoint2d> params;
    bvector<DPoint2d*> uvPP;

    if ( nVertex > 2)
        {
        VuP currP,face0P,face1P, face2P,face3P;
        int i;
        VuMask visitedMask = vu_grabMask ( graphP);
        VuMask notAStartPointMask = visitedMask | VU_EXTERIOR_EDGE;
        VuMask scratchMask = vu_grabMask (graphP);
        /* Collect the parameter space coordinates and set the array indices in the VU nodes */
        for (i=0, vu_arrayOpen( vertexArrayP ); vu_arrayRead( vertexArrayP , &currP ); i++)
            {
            /* Store the id in the vertex */
            VU_VERTEX_LOOP(loopVertexP, currP)
                {
                vu_setUserDataPAsInt (loopVertexP, i);
                }
            END_VU_VERTEX_LOOP(loopVertexP, currP)
            DPoint2d uv;
            uv.x = vu_getX (currP);
            uv.y = vu_getY (currP);
            params.push_back (uv);
            }
            // evaluator wants indirection.
            uvPP.clear ();
            for (size_t i = 0, n = params.size (); i < n; i++)
                uvPP.push_back (&params[i]);
        /* Evaluate the spatial coordinates */
        bspmesh_evaluatePointsBezier( points, normals, mpP->normalsRequired, bezierP, uvPP,
                                        bezierUMin, bezierUMax, bezierVMin, bezierVMax,
                                        0 != mpP->reverse );
        tristrip_init(
                        mpP->parametersRequired ? &uvPP[0] : NULL,
                        &points[0],
                        mpP->normalsRequired ? &normals[0] : NULL,
                        (int(*)())mpP->triangleFunction,
                        mpP->userDataP,
                        &mpP->paramScale
                     );

        /* sweep through the mesh */
        vu_clearMaskInSet( graphP, visitedMask | scratchMask);
        VU_SET_LOOP ( seedP, graphP )
            {
            int count = 0;
            if (SUCCESS != status )
                break;
            face0P = seedP;
#define OPTIMIZE_TRISTRIP_LENGTHS
#ifdef OPTIMIZE_TRISTRIP_LENGTHS
            if (!vu_getMask (face0P, notAStartPointMask))
                face0P = bspmesh_findLongestStrip (
                                        face0P,
                                        notAStartPointMask,
                                        scratchMask,
                                        method);
#endif
            while ( status == SUCCESS && !vu_getMask( face0P, notAStartPointMask ) )
                {
                face1P = vu_fsucc(face0P);
                face2P = vu_fsucc(face1P);
                face3P = vu_fsucc(face2P);
                vu_setMask( face0P, visitedMask );
                vu_setMask( face1P, visitedMask );
                vu_setMask( face2P, visitedMask );
                if ( face3P == face0P ) /* This should always pass. If not triangulator messed up */
                    {
                    int i0 = vu_getUserDataPAsInt ( face0P );
                    int i1 = vu_getUserDataPAsInt( face1P );
                    int i2 = vu_getUserDataPAsInt( face2P );
                    status = bspmesh_processTriangle( mpP, uvPP, points, normals, i0, i1, i2 );
                    /* To get tristrip effect, need to step out through 2nd and 3rd edges of
                        alternate triangles. */
                    count++;
                    if (method == BSPMESH_OUTPUT_TRISTRIP)
                        {
                        if ( 0x01 & count )
                            {
                            face0P = vu_edgeMate(face1P);
                            }
                        else
                            {
                            face0P = vu_edgeMate(face2P);
                            }
                        }
                    else if(method == BSPMESH_OUTPUT_TRIFAN)
                        {
                        face0P = vu_edgeMate(face2P);
                        }
                    }
                }
            }
        END_VU_SET_LOOP ( face0P, graphP)
        status = tristrip_flush();
        vu_returnMask ( graphP, scratchMask );
        vu_returnMask ( graphP, visitedMask );
        }

    vu_returnArray ( graphP, vertexArrayP );

    return status;
    }

static void bspmesh_splitEdge
(
VuSetP graphP,          /* <=> Graph where edges are added */
VuP    startP,          /* => start node on edge */
int    steps            /* => number of edges after split. i.e. 1 is noop. */
)
    {
    DPoint2d delta, newPoint;
    VuP newLeftP, newRightP, nextP;
    DPoint2d uvCurr,uvNext;
    VuP currP = startP;
    int i;

    uvCurr.x = vu_getX(currP);
    uvCurr.y = vu_getY(currP);

    nextP = vu_fsucc( currP );

    uvNext.x = vu_getX(nextP);
    uvNext.y = vu_getY(nextP);

    if( steps > 1)
        {
        delta.x = (uvNext.x - uvCurr.x) / steps;
        delta.y = (uvNext.y - uvCurr.y) / steps;
        /* Always sweep from low to h in v direction to avoid r.o. error */
        if ( delta.y > 0.0)
            {
            for ( i = 1 ; i < steps; i++ )
                {
                newPoint.x = uvCurr.x + i*delta.x;
                newPoint.y = uvCurr.y + i*delta.y;
                vu_splitEdgeAtPoint ( graphP, &newLeftP, &newRightP, currP, &newPoint);
                currP = newLeftP;
                }
            }
        else
            {
            for ( i = 1 ; i < steps; i++ )
                {
                newPoint.x = uvNext.x - i*delta.x;
                newPoint.y = uvNext.y - i*delta.y;
                vu_splitEdgeAtPoint ( graphP, &newLeftP, &newRightP, currP, &newPoint);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_subdivideVuEdgesAroundFace
(
VuSetP graphP,  /* <= parent vu graph conatining the edges being subdivided */
VuP startP,     /* <= any vu on the face */
int uSteps,
int vSteps,
int uLoSteps,
int uHiSteps,
int vLoSteps,
int vHiSteps,
double bezierUMin,
double bezierUMax,
double bezierVMin,
double bezierVMax,
DPoint2d *bezierScaleP,
MSBsplineSurface *bezierP,
MeshParams *mpP
)
    {
    VuP currP = startP;
    VuP nextP;
    DPoint2d uvCurr, uvNext;
    int steps;

    do
        {
        nextP = vu_fsucc(currP);
        vu_getDPoint2d (&uvCurr, currP);
        vu_getDPoint2d (&uvNext, nextP);
        steps = bspmesh_getCurveSteps( uSteps, vSteps,
                                       uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                       &uvCurr, &uvNext, bezierScaleP,
                                       bezierUMin, bezierUMax, bezierVMin, bezierVMax );
        bspmesh_splitEdge( graphP, currP, steps );
        currP = nextP;

        } while ( currP != startP );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_blockIndex
(
int *indexP,    /* <= index of grid line */
VuP node0P,     /* => VU node */
VuP node1P,     /* => VU node */
int select,     /* => one of LEFT_BLOCK_INDEX, RIGHT_BLOCK_INDEX */
double b,       /* => grid line coordinate */
double aOrg,    /* => 0-point on grid line */
double daGrid,  /* => step size on grid line */
double fraction /* => fraction of grid size for rounding */
)
    {
    double a0,a1,b0,b1,s,a,c;
    int status = ERROR;

    b0 = vu_getY( node0P );
    b1 = vu_getY( node1P );
    a0 = vu_getX( node0P );
    a1 = vu_getX( node1P );
    *indexP = 0;

    if ( b0 == b1 ) return ERROR;
    s = (b - b0) / (b1 - b0);
    a = a0 + s * ( a1 - a0 );
    c = ( a - aOrg ) / daGrid;
    if ( select == LEFT_BLOCK_INDEX )
        {
        *indexP = (int) ( c + 1.0 + fraction );
        status = SUCCESS;
        }
    else if ( select == RIGHT_BLOCK_INDEX )
        {
        *indexP = (int) ( c - fraction );
        status = SUCCESS;
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt bspmesh_insertGridInMonotoneVFace
(
VuSetP graphP,          /* <=> graph to have grid boundary added */
VuP     startP,         /* => Any start node on the face */
DPoint2d *uv0P,         /* => origin point for grid */
DPoint2d *uvSizeP,      /* => u,v block sizes */
DPoint2d *marginP       /* => u,v setbacks from boundary. */
)
    {
    VuP minUP, minVP, maxUP, maxVP;
    VuP leftP, rightP;
    VuP nextLeftP = NULL, nextRightP = NULL;
    VuP newLeftGridP, newRightGridP;
    VuP leftJoinP, rightJoinP;
    static int joinToBoundary = 0;
    int j, iLeft, iRight;
    double v, uLeft, uRight ;
    double vMin, vMax;
    vu_findExtrema ( startP, &minUP, &maxUP, &minVP, &maxVP );

    j = (int) ( (1.0 + MINIMUM_TILE_MARGIN) + ( vu_getY(minVP) + marginP->y - uv0P->y) / uvSizeP->y );
    if (j < 0)
        return ERROR;
    v = vMin = uv0P->y + uvSizeP->y * (double)j;
    vMax = vu_getY(maxVP) - MINIMUM_TILE_MARGIN * uvSizeP->y;
    leftP = rightP = minVP;

    for ( ; v < vMax && leftP != maxVP && rightP != maxVP; v += uvSizeP->y )
        {
        /* Walk up both sides until the succeeding edges cross the grid v level */
        while ( leftP != maxVP &&
                ( nextLeftP = vu_fpred(leftP) ,  vu_getY(nextLeftP) < v )
              )
            {
            leftP = nextLeftP;
            }

        while ( rightP != maxVP &&
                ( nextRightP = vu_fsucc(rightP) ,  vu_getY(nextRightP) < v )
              )
            {
            rightP = nextRightP;
            }

        if ( rightP != maxVP && leftP != maxVP
             &&  SUCCESS == bspmesh_blockIndex( &iLeft,  leftP,  nextLeftP,   LEFT_BLOCK_INDEX, v, uv0P->x, uvSizeP->x, MINIMUM_TILE_MARGIN )
             &&  SUCCESS == bspmesh_blockIndex( &iRight, rightP,  nextRightP, RIGHT_BLOCK_INDEX, v, uv0P->x, uvSizeP->x, MINIMUM_TILE_MARGIN )
             &&  iLeft < iRight
             )
            {
            uLeft  = uv0P->x +  iLeft * uvSizeP->x;
            uRight = uv0P->x + iRight * uvSizeP->x;
            vu_makePair( graphP, &newLeftGridP, &newRightGridP );
            vu_setXY (newLeftGridP, uLeft, v);
            vu_setXY (newRightGridP, uRight, v);
            if ( joinToBoundary )
                {
                vu_join( graphP, newRightGridP, rightP, &leftJoinP, &rightJoinP );
                vu_join( graphP, leftP, newLeftGridP,  &leftJoinP, &rightJoinP );
                leftP = leftJoinP;
                }
            bspmesh_splitEdge( graphP, newLeftGridP, iRight - iLeft );
            }
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh1_meshBezierRegion
(
MeshParams          *mpP,                       /* => mesh parameters */
DPoint2d            *points,                    /* => slice boundaries */
int                 nPoints,                    /* => number of slice bound points */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax                  /* => v Maximum for bezier */
)
    {
#ifdef USE_CACHE
    VuSetP graphP = vumesh_grabGraphP();
#else
    VuSetP graphP = vu_newVuSet (0);
#endif
    int uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps;
    VuP startP;
    static int fixedUSteps = 0;
    static int fixedVSteps = 0;
    double xScale, yScale;
    static int tristrips = BSPMESH_OUTPUT_TRISTRIP;
    int status = SUCCESS;
    static int processVu = 1;   /* set this to zero to trap unmodified graph in rxnurbs */
    DPoint2d bezierScale, bezierOrigin, blockSize, marginSize;
    bspmesh_getSurfaceSteps (&uSteps, &vSteps, &uLoSteps, &uHiSteps, &vLoSteps, &vHiSteps,
                              bezierP, mpP);

    if ( uSteps < fixedUSteps )
        uSteps = fixedUSteps;

    if ( vSteps < fixedVSteps )
        vSteps = fixedVSteps;

    bezierScale.x       = bezierUMax - bezierUMin;
    bezierScale.y       = bezierVMax - bezierVMin;

    bezierOrigin.x      = 0.0;
    bezierOrigin.y      = 0.0;

    blockSize.x         = bezierScale.x / uSteps;
    blockSize.y         = bezierScale.y / vSteps;

    marginSize.x        = blockSize.x * MINIMUM_TILE_MARGIN;
    marginSize.y        = blockSize.y * MINIMUM_TILE_MARGIN;

    xScale = (double)uSteps;
    yScale = (double)vSteps;

    startP = vu_makeLoopFromArray(graphP,points,nPoints,false,false);

    if (startP)
        {
        bspmesh_subdivideVuEdgesAroundFace( graphP, startP,
                    uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                    bezierUMin, bezierUMax, bezierVMin, bezierVMax, &bezierScale, bezierP, mpP);

        status = bspmesh_insertGridInMonotoneVFace( graphP, startP, &bezierOrigin, &blockSize, &marginSize );

        if (processVu && SUCCESS == status)
            {
            // We want the smaller subdivision steps to be going vertically
            // This makes vertical sweep encouter new points by the
            // primary (y direction) sort key rather than the
            // secondar (x direction) lexical sort key
            bool    bRotateForTriangulation = uSteps > vSteps;

            if (bRotateForTriangulation)
                vu_rotate90CCW (graphP);
            /* fixup crossing loops (there shouldn't be any!!) */
            vu_mergeLoops( graphP );
            /* insert additional edges so each face is regular in the y direction,
               i.e. has a single min point, edges that go continuously up to a
               single max point, and back
            */
            vu_regularizeGraph( graphP );

            /* Straighten out the inside/outside markings */
            vu_markAlternatingExteriorBoundaries(graphP, true);
            /* Stick in the triangles */
            vu_triangulateMonotoneInteriorFaces(graphP, false);
            if (bRotateForTriangulation)
                vu_rotate90CW (graphP);

            if (bezierScale.x != 0.0 && bezierScale.y != 0.0)
                vu_flipTrianglesToImproveScaledQuadraticAspectRatio(graphP,
                                    xScale / bezierScale.x,
                                    yScale / bezierScale.y
                                    );
            else
                vu_flipTrianglesToImproveScaledQuadraticAspectRatio(graphP,
                                    xScale,
                                    yScale
                                    );

            /* dump the triangles */
            status = bspmesh_dumpVuTristrip (
                        graphP, mpP, bezierP,
                        bezierUMin, bezierUMax, bezierVMin, bezierVMax, tristrips );
            }
        }
#ifdef USE_CACHE
    vumesh_dropGraphP (graphP);
#else
    vu_freeVuSet (graphP);
#endif
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_meshSurfaceSlice
(
MeshParams          *mpP,                       /* => mesh parameters */
DPoint2d            **sliceP,                   /* => slice boundaries */
int                 nSlice,                     /* => number of slice bound points */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
MSBsplineSurface    *surfaceP,                  /* => bezier prepped surface */
int                 vStart,                     /* => offset of slice in knot array */
int                 uNumSegs,                   /* => number of U segments */
int                 *uStartP,                   /* => starting offsets of U segments */
double              *uKnotP,                    /* => pointer to first (nonzero) u knot */
double              *vKnotP,                    /* => pointer to first (nonzero) v knot */
bvector<DPoint2d> &points                       /* => to be cleared and reused here */
)
    {
    int             minIndex, maxIndex, j, minLeftIndex, minRightIndex, next,
                    index, maxLeftIndex, maxRightIndex, uStart, status;
    double          sliceMin, sliceMax, uMin, uMax;
    DPoint2d        **pntP, **endP, minLeftPoint, minRightPoint,
                    maxLeftPoint, maxRightPoint;
    static int alwaysUseOldMesher = 0;

#if defined (slice_debug)
    debug_displayPointers (sliceP, nSlice);
#endif
    points.clear ();
    minIndex = maxIndex = 0;
    sliceMin = sliceMax = sliceP[0]->x;
    for (pntP=sliceP+1, endP=sliceP+nSlice; pntP<endP; pntP++)
        {
        if ((*pntP)->x > sliceMax)
            {
            sliceMax = (*pntP)->x;
            maxIndex = (int)(pntP - sliceP);
            }
        if ((*pntP)->x < sliceMin)
            {
            sliceMin = (*pntP)->x;
            minIndex = (int)(pntP - sliceP);
            }
        }
    uMin = uKnotP[uStartP[0] - 1];
    minLeftIndex = minRightIndex = minIndex;
    minLeftPoint = minRightPoint = *sliceP[minIndex];
    for (j=0, status=SUCCESS; j < uNumSegs && status==SUCCESS; j++)
        {
        uStart = uStartP[j];
        maxLeftPoint.x = maxRightPoint.x = uMax = uKnotP[uStart];
        if (sliceMin >= uMax || sliceMax <= uMin)
            continue;

        for (maxLeftIndex = minLeftIndex, next = PREV(maxLeftIndex, nSlice);
                sliceP[next]->x < uMax && maxLeftIndex != maxIndex;
                    maxLeftIndex = next, next = PREV(next, nSlice));

        for (maxRightIndex = minRightIndex, next = NEXT(maxRightIndex, nSlice);
                sliceP[next]->x < uMax && maxRightIndex != maxIndex;
                    maxRightIndex = next, next = NEXT (next, nSlice));

        if (maxLeftIndex == maxIndex)
            maxLeftPoint = *sliceP[maxIndex];
        else
            maxLeftPoint.y = bspmesh_horizIntercept (maxLeftPoint.x,
                                                     sliceP[maxLeftIndex],
                                                     sliceP[PREV(maxLeftIndex, nSlice)]);

        if (maxRightIndex == maxIndex)
            maxRightPoint = *sliceP[maxIndex];
        else
            maxRightPoint.y = bspmesh_horizIntercept (maxRightPoint.x,
                                                      sliceP[maxRightIndex],
                                                      sliceP[NEXT(maxRightIndex, nSlice)]);

        bvector<DPoint2d> points;
        points.push_back (maxLeftPoint);

        for (index=maxLeftIndex;
                 index != minLeftIndex;
                     index=NEXT(index, nSlice))
            points.push_back (*sliceP[index]);

        points.push_back (minLeftPoint);
        points.push_back (minRightPoint);
        for (index=NEXT(minRightIndex, nSlice);
                index != NEXT(maxRightIndex, nSlice);
                    index=NEXT(index, nSlice))
            points.push_back (*sliceP[index]);

        points.push_back (maxRightPoint);

        bsprsurf_netPoles (bezierP->poles, bezierP->weights, uStart, vStart, surfaceP);

        if  ( alwaysUseOldMesher ||
              SUCCESS != (status = bspmesh_meshSimpleBezierRegion
                                        (
                                        mpP, &points[0], (int)points.size (), bezierP,
                                        uKnotP[uStart-1], uKnotP[uStart],
                                        vKnotP[vStart-1], vKnotP[vStart]
                                        )
                        )
            )
            {
            status = bspmesh1_meshBezierRegion (mpP, &points[0], (int)points.size (), bezierP,
                                           uKnotP[uStart-1], uKnotP[uStart],
                                           vKnotP[vStart-1], vKnotP[vStart]);
            }

        minLeftIndex  = maxLeftIndex;
        minRightIndex = maxRightIndex;
        minLeftPoint  = maxLeftPoint;
        minRightPoint = maxRightPoint;
        uMin = uMax;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_meshSurfaceRegion
(
MeshParams          *mpP,                       /* => mesh parameters */
bvector<DPoint2d>    &boundary,                 /* = region boundary */
MSBsplineSurface    *surfaceP,                  /* => bezier prepped surface */
int                 uNumSegs,                   /* => number of U segments */
int                 vNumSegs,                   /* => number of V segments */
int                 *uStartP,                   /* => starting offsets of U segments */
int                 *vStartP                    /* => starting offsets of U segments */
)
    {
    int                 minIndex, maxIndex, nPnts, status, bufSize, j, next,
                        minLeftIndex, maxLeftIndex, minRightIndex, maxRightIndex, index,
                        pointBufSize, vStart;
    double              *uKnotP, *vKnotP, vMin, vMax, boundMin, boundMax;
    MSBsplineSurface    bezier;
    DPoint2d            *pnts, *pntP, *endP, **sliceP, minLeftPoint, maxLeftPoint,
                        minRightPoint, maxRightPoint, *pointP;
    double          firstU, lastU, firstV, lastV;
    
    bezier.poles = NULL;
    bezier.weights = NULL;
    sliceP = NULL;
    pointP = NULL;
    bvector<DPoint2d*> slicePointers;
    bvector<DPoint2d>  points;  
    pointBufSize = 0;
    pnts  = &boundary[0];
    nPnts = (int)boundary.size () - 1;

    boundMin = boundMax = pnts[0].y;
    minIndex = maxIndex = 0;
    for (pntP=pnts+1, endP = pnts+nPnts; pntP < endP; pntP++)
        {
        if (pntP->y > boundMax)
            {
            boundMax = pntP->y;
            maxIndex = (int)(pntP - pnts);
            }
        if (pntP->y < boundMin)
            {
            boundMin = pntP->y;
            minIndex = (int)(pntP - pnts);
            }
        }

    if (status = bspproc_initializeBezierPatch (&bezier, surfaceP))
        goto wrapup;

    firstU = surfaceP->uKnots[0];
    for (uKnotP=surfaceP->uKnots;
            (*uKnotP < firstU + KNOT_TOLERANCE_BASIS);
                uKnotP++);
    lastU = uKnotP[uStartP[uNumSegs-1]];

    firstV = surfaceP->vKnots[0];
    for (vKnotP=surfaceP->vKnots;
            (*vKnotP < firstV + KNOT_TOLERANCE_BASIS);
                vKnotP++);
    lastV = vKnotP[vStartP[vNumSegs-1]];

    bufSize = 0;
    vMin = vKnotP[vStartP[0] - 1];
    minLeftIndex = minRightIndex = minIndex;
    minLeftPoint = minRightPoint = pnts[minIndex];

    for (j=0, status=SUCCESS; j < vNumSegs && status==SUCCESS; j++)
        {
        vStart = vStartP[j];
        maxLeftPoint.y = maxRightPoint.y = vMax = vKnotP[vStart];
        if (boundMin >= vMax || boundMax <= vMin)
            continue;

        for (maxLeftIndex = minLeftIndex, next = PREV(maxLeftIndex, nPnts);
                pnts[next].y < vMax && maxLeftIndex != maxIndex;
                    maxLeftIndex = next, next = PREV(next, nPnts));

        for (maxRightIndex = minRightIndex, next = NEXT(maxRightIndex, nPnts);
                pnts[next].y < vMax && maxRightIndex != maxIndex;
                    maxRightIndex = next, next = NEXT (next, nPnts));

        if (maxLeftIndex == maxIndex)
            maxLeftPoint = pnts[maxIndex];
        else
            maxLeftPoint.x  = bspmesh_verticalIntercept (maxLeftPoint.y,
                                                         &pnts[maxLeftIndex],
                                                         &pnts[PREV(maxLeftIndex, nPnts)]);

        if (maxRightIndex == maxIndex)
            maxRightPoint = pnts[maxIndex];
        else
            maxRightPoint.x = bspmesh_verticalIntercept (maxRightPoint.y,
                                                         &pnts[maxRightIndex],
                                                         &pnts[NEXT(maxRightIndex, nPnts)]);

        slicePointers.clear ();
        slicePointers.push_back (&maxLeftPoint);

        for (index=maxLeftIndex;
                 index != minLeftIndex;
                     index=NEXT(index, nPnts))
            slicePointers.push_back (&pnts[index]);

        slicePointers.push_back (&minLeftPoint);
        slicePointers.push_back (&minRightPoint);

        for (index=NEXT(minRightIndex, nPnts);
                index != NEXT(maxRightIndex, nPnts);
                    index=NEXT(index, nPnts))
            slicePointers.push_back (&pnts[index]);

        slicePointers.push_back (&maxRightPoint);

        status = bspmesh_meshSurfaceSlice (mpP, &slicePointers[0], (int)slicePointers.size (), &bezier, surfaceP, vStart,
                                           uNumSegs, uStartP, uKnotP, vKnotP,
                                            points);

        minLeftIndex  = maxLeftIndex;
        minRightIndex = maxRightIndex;
        minLeftPoint  = maxLeftPoint;
        minRightPoint = maxRightPoint;
        vMin = vMax;
        }
wrapup:

    if (bezier.poles)       msbspline_free (bezier.poles);
    if (bezier.weights)     msbspline_free (bezier.weights);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileAndCoveSurface
(
MeshParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *surfaceP                   /* => surface */
)
    {
    int                 i, status = SUCCESS, uNumSegs, vNumSegs,
                        *uStart, *vStart;
    bvector<MeshRegion> sourceRegions, monotoneRegions;

    MSBsplineSurface    preppedSurface;

    memset (&preppedSurface, 0, sizeof(preppedSurface));
    uStart = vStart = NULL;

    if (surfaceP->numBounds == 0 || ! surfaceP->holeOrigin)
        {
        sourceRegions.push_back (MeshRegion ());
        MeshRegion &outerRegion = sourceRegions.back ();
        outerRegion.boundary.   reserve (5);
        outerRegion.push_back (0,0);
        outerRegion.push_back (1,0);
        outerRegion.push_back (1,1);
        outerRegion.push_back (0,1);
        outerRegion.push_back (0,0);
        }

    if (surfaceP->numBounds)
        {
        for (i=0; i<surfaceP->numBounds; i++)
            {
            sourceRegions.push_back (MeshRegion ());
            MeshRegion &newRegion = sourceRegions.back ();
            newRegion.push_back (surfaceP->boundaries[i]);
            }
        }

    bspmesh_divideToMonotonic (sourceRegions, monotoneRegions);

    if (status = bspproc_prepareSurface (&preppedSurface, &uNumSegs, &vNumSegs,
                                         &uStart, &vStart, surfaceP))
        goto wrapup;
    /* Note: keep going through this loop even after failure (to assure
            tail of list is freed) */
    for (size_t i = 0; i < monotoneRegions.size (); i++)
        {
        if (SUCCESS == status && 3 <= monotoneRegions[i].boundary.size ())
            status = bspmesh_meshSurfaceRegion (mpP, monotoneRegions[i].boundary, &preppedSurface,
                                                uNumSegs, vNumSegs, uStart, vStart);
        }

wrapup:
    if (uStart)                 msbspline_free (uStart);
    if (vStart)                 msbspline_free (vStart);
    bspsurf_freeSurface (&preppedSurface);

    return status;
    }

void unweight (DPoint3dCR xyzw, double w, DPoint3dR xyz)
    {
    xyz = xyzw;
    if (w <= 0.0 || w == 1.0)
        {
        // leave it alone.
        }
    else
        {
        double a = 1.0 / w;
        xyz.x = xyzw.x * a;
        xyz.y = xyzw.y * a;
        xyz.z = xyzw.z * a;
        }
    }

double SumWeightedLengths (DPoint3d const *xyz, double const *weight, int i0, int step, int n, bool wrap)
    {
    double d0 = PolylineOps::Length (xyz + i0, weight + i0, step, n, wrap);
    DPoint3d xyz0, xyz1;
    unweight (xyz[i0], weight[i0], xyz0);
    DPoint3d xyzWrap = xyz0;
    double d = 0.0;
    i0 += step;
    for (int i = 1; i < n; i++, i0 += step, xyz0 = xyz1)
        {
        unweight (xyz[i0], weight[i0], xyz1);
        d += xyz0.Distance (xyz1);
        }
    if (wrap)
        d += xyz0.Distance (xyzWrap);
    return d > 0.0 ? d : d0;
    }

double SumLengths (DPoint3d const *xyz, int i0, int step, int n, bool wrap)
    {
    double d0 = PolylineOps::Length (xyz + i0, NULL, step, n, wrap);
    DPoint3d xyz0 = xyz[i0], xyz1;
    DPoint3d xyzWrap = xyz0;
    i0 += step;
    double d = 0.0;
    for (int i = 1; i < n; i++, i0 += step, xyz0 = xyz1)
        {
        xyz1 = xyz[i0];
        d += xyz0.Distance (xyz1);
        }
    if (wrap)
        d += xyz0.Distance (xyzWrap);
    return d > 0.0 ? d : d0;
    }

void updateMax
(
int i,
double value,
double &maxValue,
int iSave
)
    {
    if (value > maxValue)
        {
        iSave = i;
        maxValue = value;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_calculateParameterLengths
(
MSBsplineSurfaceCR surface,
DPoint2dR          length,
int &maxIndexX, // index where length.x occurred.
int &maxIndexY  // index where length.y occurred.
)
    {
    length.Zero ();
    int numU = surface.uParams.numPoles;
    int numV = surface.vParams.numPoles;

    bool closedU = surface.uParams.closed != 0;
    bool closedV = surface.vParams.closed != 0;
    maxIndexX = -1;
    maxIndexY = -1;
    if (surface.rational)
        {
        for (int j = 0; j < numV; j++)
            {
            updateMax (j, SumWeightedLengths (surface.poles, surface.weights,
                        j * numU, 1, numU, closedU), length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax (i, SumWeightedLengths (surface.poles, surface.weights,
                        i, numU, numV, closedV), length.y, maxIndexY);
            }
        }
    else
        {
        for (int j = 0; j < numV; j++)
            {
            updateMax (j, SumLengths (surface.poles, j * numU, 1, numU, closedU),
                        length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax (i, SumLengths (surface.poles, i, numU, numV, closedV), length.y, maxIndexY);
            }
        }
    }

GEOMDLLIMPEXP void bspmesh_calculateParameterLengths
(
DPoint2d            *lengthP,
MSBsplineSurface    *surfaceP
)
    {
    int maxIndexI, maxIndexJ;
    bspmesh_calculateParameterLengths (*surfaceP, *lengthP, maxIndexI, maxIndexJ);
    }
    

static double s_minEdgeFraction = 1.0e-5;    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
void bspmesh_calculateControlPolygonLengthsAndTurn
(
MSBsplineSurfaceCR surface,
bvector <double> &uLength,  //!< u direction length of control polygon [i] edges
bvector<double> &uTurn,     //!< u direction turning angle summed accross polygon [i] edges
bvector<double> &vLength,
bvector<double> &vTurn
)
    {
    uLength.clear ();
    uTurn.clear ();
    uLength.clear ();
    vTurn.clear ();

    int numU = surface.uParams.numPoles;
    int numV = surface.vParams.numPoles;

    bool closedU = surface.uParams.closed != 0;
    bool closedV = surface.vParams.closed != 0;
    if (surface.rational)
        {
        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uLength.push_back (PolylineOps::Length (surface.poles + jStart, surface.weights + jStart, 1, numU, closedU));
            }
        for (int i = 0; i < numU; i++)
            {
            vLength.push_back (PolylineOps::Length (surface.poles + i, surface.weights + i, numU, numV, closedV));
            }
        double minEdge = s_minEdgeFraction * (DoubleOps::Sum (uLength) + DoubleOps::Sum (vLength));


        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uTurn.push_back (PolylineOps::SumAbsoluteAngles (surface.poles + jStart, surface.weights + jStart, 1, numU, closedU, minEdge));
            }
        for (int i = 0; i < numU; i++)
            {
            vTurn.push_back (PolylineOps::SumAbsoluteAngles (surface.poles + i, surface.weights + i, numU, numV, closedV, minEdge));
            }
        }
    else
        {
        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uLength.push_back (PolylineOps::Length (surface.poles + jStart, NULL, 1, numU, closedU));
            }
        for (int i = 0; i < numU; i++)
            {
            vLength.push_back (PolylineOps::Length (surface.poles + i, NULL, numU, numV, closedV));
            }
        double minEdge = s_minEdgeFraction * (DoubleOps::Sum (uLength) + DoubleOps::Sum (vLength));

        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uTurn.push_back (PolylineOps::SumAbsoluteAngles (surface.poles + jStart, NULL, 1, numU, closedU, minEdge));
            }
        for (int i = 0; i < numU; i++)
            {
            vTurn.push_back (PolylineOps::SumAbsoluteAngles (surface.poles + i, NULL, numU, numV, closedV, minEdge));
            }


        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileUnboundedSurface
(
MeshParams          *mpP,
MSBsplineSurface    *surfaceP
)
    {
    int                 i, j, uSteps, vSteps, status, uNumSegs, vNumSegs,
                        *uStart=NULL, *vStart=NULL;
    double              *uKnotP, *vKnotP;
    DPoint2d            delta;
    MSBsplineSurface    preppedSurface, bezier;
    static int s_parameterSelect = 0;
    if (!surfaceP->uParams.closed && !surfaceP->vParams.closed &&
         surfaceP->uParams.order == surfaceP->uParams.numPoles &&
         surfaceP->vParams.order == surfaceP->vParams.numPoles)

        {
        bspmesh_getSurfaceSteps (&uSteps, &vSteps, NULL, NULL, NULL, NULL,
                                 surfaceP, mpP);

        delta.x = 1.0 / (double) uSteps;
        delta.y = 1.0 / (double) vSteps;

        return bspmesh_tile (mpP, surfaceP, 0, uSteps, 0, vSteps, false,
                             &delta, 0.0, 1.0, 0.0, 1.0, &mpP->paramScale, 0 != mpP->reverse);
        }
    else
        {
        memset (&bezier, 0, sizeof(bezier));
        if ((status = bspproc_prepareSurface (&preppedSurface, &uNumSegs, &vNumSegs,
                                              &uStart, &vStart, surfaceP)) != SUCCESS)
            return status;

        for (uKnotP=preppedSurface.uKnots;
                (KNOT_TOLERANCE_BASIS >= *uKnotP && *uKnotP < 1.0);
                    uKnotP++);

        for (vKnotP=preppedSurface.vKnots;
                (KNOT_TOLERANCE_BASIS>= *vKnotP && *vKnotP < 1.0);
                    vKnotP++);

        if ((status = bspproc_initializeBezierPatch (&bezier, &preppedSurface)) != SUCCESS)
            goto wrapup;

        for (j=0; j < vNumSegs; j++)
            for (i=0; i < uNumSegs; i++)
                {
                bsprsurf_netPoles (bezier.poles, bezier.weights, uStart[i], vStart[j],
                                   &preppedSurface);

                bspmesh_getSurfaceSteps (&uSteps, &vSteps, NULL, NULL, NULL, NULL,
                                         &bezier, mpP);

                delta.x = 1.0 / (double) uSteps;
                delta.y = 1.0 / (double) vSteps;

    // EDL topaz tree July 28 2012.   The 0 branch appears to be exactly from 8.11.9.
    // The quadgrid mesh function thus always gets evaluation in 01.
    // Why were the knot values commented out?   This seems compltely wrong.
                if (s_parameterSelect == 0)
                    {
                    if ((status = bspmesh_tile (mpP, &bezier, 0, uSteps, 0, vSteps, false,
                                                &delta, 0.0, 1.0, 0.0, 1.0,
                                    /*          uKnotP[uStart[i]-1], uKnotP[uStart[i]],
                                                vKnotP[vStart[j]-1], vKnotP[vStart[j]], */
                                                &mpP->paramScale, 0 != mpP->reverse)) != SUCCESS)
                        break;
                    }
                else
                    {
                    if ((status = bspmesh_tile (mpP, &bezier, 0, uSteps, 0, vSteps, false,
                                                &delta,
                                                uKnotP[uStart[i]-1], uKnotP[uStart[i]],
                                                vKnotP[vStart[j]-1], vKnotP[vStart[j]],
                                                &mpP->paramScale, 0 != mpP->reverse)) != SUCCESS)
                        break;
                    }
                }
wrapup:

        if (bezier.poles)           msbspline_free (bezier.poles);
        if (bezier.weights)         msbspline_free (bezier.weights);
        if (uStart)                 msbspline_free (uStart);
        if (vStart)                 msbspline_free (vStart);

        bspsurf_freeSurface (&preppedSurface);

        return status;
        }

    }
/*---------------------------------------------------------------------------------**//**
* This is invoked by rendering to signal the start and end of large blocks of meshing. The memory used for VU graphs on surfaces is recycled
* within these blocks and returned to the system at the end All other users of bspemsh_meshSurface recycle the memory after each surface.
* @bsimethod                                                    Earlin.Lutz     06/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_setMemoryCaching
(
int     mode            /* true for caching on, false for caching off */
)
    {
#ifdef USE_CACHE
    /* Always free the current saved graph at a switchover. */
    vumesh_freeSavedGraph();
    bspmesh_cacheGraph = mode;
#endif
    }

bool validateBoundaries (BsurfBoundary   *boundsP, int numBounds);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurfaceExt2
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* => mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* => triangle strip function */
double                                      tolerance,                  /* => tolerance */
int                                         toleranceMode,              /* => tolerance mode */
TransformP                                  toleranceTransformP,        /* => tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* => tolerance camera position */
double                                      toleranceFocalLength,       /* => tolerance focal length */
double                                      angleTolerance,             /* => angle tolerance */
DPoint2dP                                   parameterScale,             /* => parameter scale */
MSBsplineSurface                            *surfaceP,                  /* => surface to mesh */
bool                                        normalsRequired,            /* => true to return normals */
bool                                        parametersRequired,         /* => true to return parameters */
bool                                        reverse,                    /* => true to reverse order */
bool                                        covePatchBoundaries,        /* => true to cove patch bounds */
CallbackArgP                                userDataP                   /* => user data */
)
    {
    DPoint2d    length;
    MeshParams  meshParams;
    static int alwaysCove = false;
    /* Turn this on to enable pre-fixup of boundaries */
    static int fixBoundaries = false;
    int status = ERROR;

#if defined (debug)
    debugSurfP = surfaceP;
    debugUserDataP = userDataP;
#endif
    if ( alwaysCove ) covePatchBoundaries = true;
    memset (&meshParams, 0, sizeof(MeshParams));
    meshParams.meshFunction         = meshFunction;
    meshParams.triangleFunction     = triangleFunction;
    meshParams.tolerance            = tolerance;
    meshParams.toleranceMode        = toleranceMode;
    meshParams.toleranceTransformP  = toleranceTransformP;
    meshParams.toleranceCameraP     = toleranceCameraP;
    meshParams.toleranceFocalLength = toleranceFocalLength;
    meshParams.normalsRequired      = normalsRequired;
    meshParams.parametersRequired   = parametersRequired;
    meshParams.reverse              = reverse;
    meshParams.userDataP            = userDataP;
    meshParams.angleTolerance       = angleTolerance;

    int numBoundsInUse = surfaceP->numBounds;
    if (covePatchBoundaries
        && !validateBoundaries (surfaceP->boundaries, surfaceP->numBounds)
        )
        {
        numBoundsInUse = 0;
        covePatchBoundaries = false;
        }

    if (fixBoundaries)
        bspmesh_fixBoundaries (&surfaceP->boundaries, &surfaceP->numBounds, true);

    if (parameterScale)
        {
#if defined (BSPMESH_Use_Exact_for_length)
        bspmesh_calculateParameterLengthsExact (&length, surfaceP);
#else
        bspmesh_calculateParameterLengths (&length, surfaceP);
#endif
        meshParams.paramScale.x = (parameterScale->x < 1.0E-8)  ?
                                   1.0 : length.x / parameterScale->x;
        meshParams.paramScale.y = (parameterScale->y < 1.0E-8)  ?
                                   1.0 : length.y / parameterScale->y;
        }
    else
        {
        meshParams.paramScale.x = meshParams.paramScale.y = 1.0;
        }

    if (numBoundsInUse == 0 &&
       (! covePatchBoundaries || (surfaceP->uParams.order == surfaceP->uParams.numPoles &&
                                  surfaceP->vParams.order == surfaceP->vParams.numPoles)))
        status = bspmesh_tileUnboundedSurface (&meshParams, surfaceP);
    else
        status = bspmesh_tileAndCoveSurface (&meshParams, surfaceP);


#ifdef USE_CACHE
    /* The vu mesher has been cacheing graph structures.  If we haven't been
       warned that we are in a tight start-end block, we have to toss the cache.
    */
    if (!bspmesh_cacheGraph)
        vumesh_freeSavedGraph ();
#endif
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurfaceExt
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* => mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* => triangle strip function */
double                                      tolerance,                  /* => tolerance */
int                                         toleranceMode,              /* => tolerance mode */
TransformP                                  toleranceTransformP,        /* => tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* => tolerance camera position */
double                                      toleranceFocalLength,       /* => tolerance focal length */
double                                      angleTolerance,             /* => angle tolerance */
DPoint2dP                                   parameterScale,             /* => parameter scale */
MSBsplineSurface*                           surfaceP,                   /* => surface to mesh */
bool                                        normalsRequired,            /* => true to return normals */
bool                                        parametersRequired,         /* => true to return parameters */
bool                                        reverse,                    /* => true to reverse order */
bool                                        covePatchBoundaries,        /* => true to cove patch bounds */
CallbackArgP                                userDataP                   /* => user data */
)
    {
    int numBoundary, numPCurveLoop;
    bspsurf_countLoops (surfaceP, &numBoundary, &numPCurveLoop);
    // If there are exact trim curves, temporarily stroke them at our tolerance.
    if (numBoundary > 0 && numPCurveLoop == numBoundary)
        {
        // Umm.. need to allow "angular only" tolerance.
        StatusInt status = ERROR;
        MSBsplineSurface tempSurface;
        static double sLocalRelTol = 1.0e-5;
        static double sGlobalRelTol = 1.0e-8;
        double aSurfaceTol = mdlBspline_resolveSurfaceTolerance (surfaceP, tolerance, sLocalRelTol, sGlobalRelTol);
        double aCurveTol = 0.01;
        bspsurf_openTempTrimmedSurface (&tempSurface, surfaceP, aCurveTol, aSurfaceTol);
        status = bspmesh_meshSurfaceExt2 (meshFunction, triangleFunction,
                        tolerance, toleranceMode, toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength, angleTolerance,
                        parameterScale,
                        &tempSurface,
                        normalsRequired, parametersRequired,
                        reverse, covePatchBoundaries,
                        userDataP);
        bspsurf_closeTempTrimmedSurface (&tempSurface);
        return status;
        }
    else
        return bspmesh_meshSurfaceExt2 (meshFunction, triangleFunction,
                        tolerance, toleranceMode, toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength, angleTolerance,
                        parameterScale,
                        surfaceP,
                        normalsRequired, parametersRequired,
                        reverse, covePatchBoundaries,
                        userDataP);
    }
//REMOVE AAA

/*---------------------------------------------------------------------------------**//**
* Assumes there is a single loop in the graph to be corrected for self-intersections and crossings.
* @bsimethod                                                    Brian.Peters    10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_fixSingleBoundaryLoop
(
VuSetP          graphP
)
    {
    VuMask          mVisited = vu_grabMask (graphP);

    /* Do this OR in case new boundaries self-intersect. */
    vu_stackPushCopy (graphP);
    vu_orLoops (graphP);

    /* Check that we shouldn't use XOR instead. */
    if (fabs (vu_area ( vu_firstNodeInGraph (graphP))) > 0.999)
        {
        vu_stackExchange (graphP);
        vu_xorLoops (graphP);
        }

    vu_stackExchange (graphP);
    VU_SET_LOOP (P, graphP)
        vu_setMask (P, mVisited);
    END_VU_SET_LOOP (P, graphP)
    vu_freeMarkedEdges (graphP, mVisited);
    vu_stackPop (graphP);

    /* Fix "backward" loops created by crossings. */
    vu_clearMaskInSet (graphP, mVisited);
    VU_SET_LOOP (P, graphP)
        {
        if (! vu_getMask (P, mVisited))
            {
            vu_setMaskAroundFace (P, mVisited);
            vu_setMaskAroundFace (vu_edgeMate (P), mVisited);

            if (vu_area (P) > 0.0 && vu_getMask (P, VU_EXTERIOR_EDGE))
                {
                vu_setMaskAroundFace (vu_edgeMate (P), VU_EXTERIOR_EDGE);
                VU_FACE_LOOP (Q, P)
                    vu_clrMask (Q, VU_EXTERIOR_EDGE);
                END_VU_FACE_LOOP (Q, P)
                }
            }
        }
    END_VU_SET_LOOP (P, graphP)
    vu_returnMask (graphP, mVisited);
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_fixBoundaries
(
BsurfBoundary   **boundsPP,
int             *numBoundsP,
bool            conserveParity,
bool            add01Box
)
    {
    int             status = ERROR, newNumBounds, face;
    bool            nestedLoops = 0;
    VuP             faceP, minP;
    VuMask          mVisited;
    VuArrayP        faceArrayP;
    VuSetP          graphP;
    DPoint2d        *loopP;
    BsurfBoundary   *bndP, *endP, *newBounds=NULL;
    static double s_unusualParameterSize = 10.0;

    if (0 == *numBoundsP || NULL == *boundsPP)
        return  SUCCESS;

    DRange2d range;
    DPoint2d localOrigin;
    range.Init ();
    int maxPoints = 0;
    bool useLocalOrigin = false;
    for (int i = 0; i < *numBoundsP; i++)
        {
        range.Extend ((*boundsPP)[i].points, (*boundsPP)[i].numPoints);
        if ((*boundsPP)[i].numPoints > maxPoints)
            maxPoints = (*boundsPP)[i].numPoints;
        }
    // heuristic: bspline surfaces have parameter loops in 0..1 range.
    // but this is called with uor data that has tolerance problems.
    // if the max coordinate of the range is larger than s_unusualParameterSize, use the range.low as origin.
    localOrigin.Zero ();
    if (range.LargestCoordinate () > s_unusualParameterSize
        && !range.Contains (0.0, 0.0))
        {
        useLocalOrigin = true;
        localOrigin = range.low;
        }

    bvector<DPoint2d> localPoints;
    localPoints.reserve ((size_t) maxPoints);


    graphP = vu_newVuSet (0);
    mVisited    = vu_grabMask (graphP);
    faceArrayP  = vu_grabArray (graphP);
    DPoint2d xy;
    // hmph.   vu_makeLoopFromArray has a clumsy 1e-8*largestCoordinate tolerance.
    // If input data is in a small box far from the origin it gets clumped.
    // So if in doubt move things back to a local origin.
    for (bndP = endP = *boundsPP, endP += *numBoundsP; bndP < endP; bndP++)
        {
        if (bndP->numPoints == 0)
            continue;
        vu_stackPush (graphP);
        if (useLocalOrigin)
            {
            localPoints.clear ();
            for (int i = 0; i < bndP->numPoints; i++)
                {
                xy = bndP->points[i];
                xy.Subtract (localOrigin);
                localPoints.push_back (xy);
                }
            faceP = vu_makeLoopFromArray (graphP, &localPoints[0], bndP->numPoints, true, true);
            }
        else
            faceP = vu_makeLoopFromArray (graphP, bndP->points, bndP->numPoints, true, true);

        if (! conserveParity)
            bspmesh_fixSingleBoundaryLoop (graphP);
        }
    vu_stackPopAll (graphP);

    if (add01Box)
        {
        DPoint2d points[5];
        points[0].Init (0,0);
        points[1].Init (1,0);
        points[2].Init (1,1);
        points[3].Init (0,1);
        points[4].Init (0,0);
        vu_makeLoopFromArray (graphP, points, 5, true, true);
        }

    if (conserveParity)
        vu_xorLoops (graphP);
    else
        vu_orLoops (graphP);

    vu_collectExteriorFaceLoops (faceArrayP, graphP);

    /* check for nested loops */
    for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &faceP); )
        if (true == (nestedLoops = (vu_area (faceP) > 0.0)))
            break;

    if (conserveParity && nestedLoops)
        {
        vu_notLoops (graphP);
        vu_arrayClear(faceArrayP);
        vu_collectExteriorFaceLoops (faceArrayP, graphP);
        }

    // purge zero area faces....
    for (vu_arrayOpen (faceArrayP), face = 0; vu_arrayRead (faceArrayP, &faceP); face++)
        {
        if (vu_faceLoopSize (faceP) == 2)
            {
            vu_arrayRemoveCurrent (faceArrayP);
            }
        }
    newNumBounds = vu_arraySize (faceArrayP);

    if (newNumBounds)
        {
        if (NULL == (newBounds = (BsurfBoundary *)
                (BsurfBoundary*)BSIBaseGeom::Malloc (newNumBounds * sizeof (BsurfBoundary))))
            {
            status = ERROR;
            }
        else
            {
            /*    For each face loop .. */
            memset (newBounds, 0, newNumBounds * sizeof(BsurfBoundary));
            for (vu_arrayOpen (faceArrayP), face = 0; vu_arrayRead (faceArrayP, &faceP); face++)
                {
                /* Count number of points in this face loop.
                    Start at 1 so that line string will be closed.
                    Also get the minimum point in loop. */
                newBounds[face].numPoints = 1;
                minP = faceP;
                VU_FACE_LOOP (P, faceP)
                    {
                    newBounds[face].numPoints += 1;
                    if (vu_below (P, minP))
                        minP = P;
                    }
                END_VU_FACE_LOOP (P, faceP)

                if (NULL == (newBounds[face].points =
                            (DPoint2d *) BSIBaseGeom::Malloc (newBounds[face].numPoints
                                                                * sizeof (DPoint2d))))
                    {
                    status = ERROR;
                    break;
                    }

                /* Assign points */
                loopP = newBounds[face].points;
                VU_FACE_LOOP (P, minP)
                    loopP->x = vu_getX (P) + localOrigin.x;
                    loopP->y = vu_getY (P) + localOrigin.y;
                    loopP++;
                END_VU_FACE_LOOP (P, minP)
                *loopP = newBounds[face].points[0];
                }

            bsputil_freeBoundary (boundsPP, *numBoundsP);

            *boundsPP   = newBounds;
            *numBoundsP = newNumBounds;
            status = SUCCESS;
            }
        }

    vu_returnMask (graphP, mVisited);
    vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_fixBoundaries
(
BsurfBoundary   **boundsPP,
int             *numBoundsP,
bool            conserveParity
)
    {
    return bspmesh_fixBoundaries (boundsPP, numBoundsP, conserveParity, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt      bspmesh_fixSurfaceBoundaries
(
MSBsplineSurface    *surfaceP,
bool                conserveParity
)
    {
    StatusInt stat = bspmesh_fixBoundaries (&surfaceP->boundaries, &surfaceP->numBounds, conserveParity, surfaceP->holeOrigin == 0);
    surfaceP->holeOrigin = 1;
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_addBoundaries
(
MSBsplineSurface    *surfaceP,
BsurfBoundary       *boundsP,
int                 numBounds,
int                 flipHoleOrigin
)
    {
    int             status, newNumBounds, face, holeOrigin;
    VuP             faceP, minP;
    VuArrayP        faceArrayP;
    VuSetP          graphP;
    VuMask          mVisited, mDelete;
    DPoint2d        *loopP;
    BsurfBoundary   *bndP, *endP, *newBounds=NULL;

    if (0 == numBounds || NULL == boundsP)
        return  SUCCESS;

    graphP = vu_newVuSet (0);
    mVisited    = vu_grabMask (graphP);
    mDelete     = vu_grabMask (graphP);
    faceArrayP  = vu_grabArray (graphP);

    /* Put existing boundaries in the graph. */
    for (bndP = endP = surfaceP->boundaries, endP += surfaceP->numBounds; bndP < endP; bndP++)
        faceP = vu_makeLoopFromArray (graphP, bndP->points, bndP->numPoints, true, true);
    vu_xorLoops (graphP);

    if (surfaceP->holeOrigin)
        {
        vu_makeLoopFromArray (graphP, unitBoxPoints, 4, true, true);
        vu_xorLoops (graphP);
        }

    /* Put new boundaries in the graph. */
    for (bndP = endP = boundsP, endP += numBounds; bndP < endP; bndP++)
        {
        vu_stackPush (graphP);
        faceP = vu_makeLoopFromArray (graphP, bndP->points, bndP->numPoints, true, true);
        bspmesh_fixSingleBoundaryLoop (graphP);
        }
    vu_stackPopNEntries (graphP, numBounds - 1);

    /* Use XOR to combine all the new boundaries with one another. */
    vu_xorLoops (graphP);
    if (flipHoleOrigin)
        {
        vu_makeLoopFromArray (graphP, unitBoxPoints, 4, true, true);
        vu_xorLoops (graphP);
        }

    /* Use OR to combine all the new boundaries with the old ones. */
    vu_stackPopWithOperation (graphP, vu_orLoops, NULL);

    /* Get rid of loops that run around the whole parameter space. */
    holeOrigin = false;
    vu_clearMaskInSet (graphP, mVisited | mDelete);
    VU_SET_LOOP (P, graphP)
        {
        if (! vu_getMask (P, mVisited))
            {
            vu_setMaskAroundFace (P, mVisited);
            if (fabs (vu_area (P)) >= 0.999)
                {
                vu_setMaskAroundFace (P, mDelete);
                if (vu_getMask (P, VU_EXTERIOR_EDGE))
                    holeOrigin = ! holeOrigin;
                }
            }
        }
    END_VU_SET_LOOP (P, graphP)
    vu_freeMarkedEdges (graphP, mDelete);

    /* Gather the results. */
    vu_collectExteriorFaceLoops (faceArrayP, graphP);
    newNumBounds = vu_arraySize (faceArrayP);

    if (newNumBounds)
        {
        if (NULL == (newBounds = (BsurfBoundary *)
                BSIBaseGeom::Malloc (newNumBounds * sizeof (BsurfBoundary))))
            {
            status = ERROR;
            }
        else
            {
            /*    For each face loop .. */
            for (vu_arrayOpen (faceArrayP), face = 0; vu_arrayRead (faceArrayP, &faceP); face++)
                {
                /* Count number of points in this face loop.
                    Start at 1 so that line string will be closed.
                    Also get the minimum point in loop. */
                newBounds[face].numPoints = 1;
                minP = faceP;
                VU_FACE_LOOP (P, faceP)
                    {
                    newBounds[face].numPoints += 1;
                    if (vu_below (P, minP))
                        minP = P;
                    }
                END_VU_FACE_LOOP (P, faceP)

                if (NULL == (newBounds[face].points =
                            (DPoint2d *) BSIBaseGeom::Malloc (newBounds[face].numPoints
                                                                * sizeof (DPoint2d))))
                    {
                    status = ERROR;
                    break;
                    }

                /* Assign points */
                loopP = newBounds[face].points;
                VU_FACE_LOOP (P, minP)
                    loopP->x = vu_getX (P);
                    loopP->y = vu_getY (P);
                    loopP++;
                END_VU_FACE_LOOP (P, minP)
                *loopP = newBounds[face].points[0];
                }
            status = SUCCESS;
            }
        surfaceP->holeOrigin = holeOrigin;
        }
    else
        {
        surfaceP->holeOrigin = false;
        status = SUCCESS;
        }

    if (surfaceP->numBounds)
        bsputil_freeBoundary (&surfaceP->boundaries, surfaceP->numBounds);
    surfaceP->boundaries = newBounds;
    surfaceP->numBounds  = newNumBounds;

    vu_returnMask (graphP, mVisited);
    vu_returnMask (graphP, mDelete);
    vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);
    return  status;
    }

#ifdef polygon_fixer
/*
@description Use bspmesh_fixBoundaries to resolve self intersections in a polygon.
The output is an array of BsurfBoundary structures.
The array of BsurfBoundary structures is itself allocated by BSIBaseGeom::Malloc.
Each of the BsurfBoundary structures represents 1 loop of the resolved polygon, and has a pointer
    to an array of points.  That array is also allocated by BSIBaseGeom::Malloc.
Caller is responsible to call BSIBaseGeom::Free for the "points" in each loop, and for the array of
    headers.

@param ppBoundaries OUT array of headers.
@param pNumBoundaries OUT number of boundaries returned.
@param pXYZBuffer IN points on input loop.
@param numXYZ IN number of input points.
*/
Public GEOMDLLIMPEXP StatusInt fixPolygon
(
BsurfBoundary **ppBoundaries,
int         *pNumBoundaries,
DPoint3d *pXYZBuffer,
int numXYZ
)
    {
    // Initially create a single boundary loop ....
    BsurfBoundary *pBoundaries = (BsurfBoundary*)BSIBaseGeom::Malloc (sizeof (BsurfBoundary));
    int i;

    pBoundaries->points = (DPoint2d*)BSIBaseGeom::Malloc (numXYZ * sizeof (DPoint2d));
    for (i = 0; i < numXYZ; i++)
        {
        pBoundaries->points[i].x = pXYZBuffer[i].x;
        pBoundaries->points[i].y = pXYZBuffer[i].y;
        }
    pBoundaries->numPoints = numXYZ;

    *ppBoundaries = pBoundaries;
    *pNumBoundaries = 1;
    return bspmesh_fixBoundaries (ppBoundaries, pNumBoundaries, true);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurface
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* IN      mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* IN      triangle strip function */
double                                      tolerance,                  /* IN      tolerance */
int                                         toleranceMode,              /* IN      tolerance mode */
TransformP                                  toleranceTransformP,        /* IN      tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* IN      tolerance camera position */
double                                      toleranceFocalLength,       /* IN      tolerance focal length */
DPoint2dP                                   parameterScale,             /* IN      parameter scale */
MSBsplineSurface const*                     surfaceP,                   /* IN      surface to mesh */
bool                                        normalsRequired,            /* IN      true to return normals */
bool                                        parametersRequired,         /* IN      true to return parameters */
bool                                        reverse,                    /* IN      true to reverse order */
bool                                        covePatchBoundaries,        /* IN      true to cove patch bounds */
CallbackArgP                                userDataP                   /* IN      user data */
)
    {
    return bspmesh_meshSurfaceExt
                (
                meshFunction,
                triangleFunction,
                tolerance,
                toleranceMode,
                toleranceTransformP,
                toleranceCameraP,
                toleranceFocalLength,
                0.0,
                parameterScale,
                const_cast<MSBsplineSurface*>(surfaceP),
                normalsRequired,
                parametersRequired,
                reverse,
                covePatchBoundaries,
                userDataP
                );
    }

// mdlBspline_surfaceMeshRange moved from bsprange.c to eliminate coupling to bspmesh.
static void extendRangePoints
(
DRange3d *  pRange,
DPoint3dCP  pPointArray,
double *    pWeightArray,
int         numPole
)
    {
    int i;
    DPoint3d newPoint;
    for (i = 0; i < numPole; i++)
        {
        if (pWeightArray)
            {
            newPoint.x = pPointArray[i].x / pWeightArray[i];
            newPoint.y = pPointArray[i].y / pWeightArray[i];
            newPoint.z = pPointArray[i].z / pWeightArray[i];
            }
        else
            newPoint = pPointArray[i];

        pRange->Extend (newPoint);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Callback to receive regular mesh during mesh range calculation.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt surfaceRange_meshFunction
(
DPoint3dP       pointP,
DPoint3dP       normalP,
DPoint2dP       paramP,
int             nColumns,
int             nRows,
CallbackArgP    callbackArg
)
    {
    DRange3dP   pRange = static_cast <DRange3dP> (callbackArg);

    extendRangePoints (pRange, pointP, NULL, nColumns * nRows);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Callback to receive triangle strips during mesh range calculation.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt surfaceRange_triangleFunction
(
DPoint3dP       pointP,
DPoint3dP       normalP,
DPoint2dP       paramP,
int             nPoints,
CallbackArgP    callbackArg
)
    {
    DRange3dP   pRange = static_cast <DRange3dP> (callbackArg);

    extendRangePoints (pRange, pointP, NULL, nPoints);
    return SUCCESS;
    }


/*--------------------------------------------------------------------*//**
* @description Compute range of a surface.  This range is based on a
*   mesh to specified tolerance.
* @param pRange <= computed range.
* @param pSurface => surface to examine.
* @param tolerance => mesh tolerance.
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_surfaceMeshRange
(
DRange3d *pRange,
MSBsplineSurface *pSurface,
double tolerance
)
    {
    DRange3d poleRange;
    static double s_defaultToleranceFraction = 1.0e-3;
    pRange->Init ();
    if (tolerance <= 0.0)
        {
        pSurface->GetPoleRange (poleRange);
        if (poleRange.IsNull ())
            return ERROR;
        tolerance = s_defaultToleranceFraction *
                poleRange.low.Distance (poleRange.high);
        if (tolerance <= 0.0)
            return ERROR;
        }

    return bspmesh_meshSurface
            (
            surfaceRange_meshFunction,
            surfaceRange_triangleFunction,
            tolerance,
            STROKETOL_ChoordHeight,
            NULL,
            NULL,
            0.0,
            NULL,
            pSurface,
            false,
            false,
            false,
            false,
            pRange
            );
    }


/*--------------------------------------------------------------------*//**
* @description Compute min and max distances from a plane to a bspline surface.
* @param pSignedLow <= signed coordinate of minimum point when coordinates are
*               measured perpendicular to the surface.
* @param pSignedHigh <= signed coordinate of maximum point when coordinates are
*               measured perpendicular to the surface.
* @param pAbsLow <= absolute shortest distance from plane to any point on the surface.
*               (0 if plane cuts the surface)
* @param pAbsHigh <= absolute farthest distance from plane to any point on the surface.
* @param pOrigin => plane origin.
* @param pNormal => plane normal.
* @param pSurface => surface to examine.
* @param tolerance => tolerance for chordal approximation to surface.
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_planeSurfaceDistance
(
double *pSignedLow,
double *pSignedHigh,
double *pAbsLow,
double *pAbsHigh,
DPoint3d *pOrigin,
DPoint3d *pNormal,
MSBsplineSurface *pSurface,
double tolerance
)
    {
    RotMatrix matrix;
    Transform planeToWorld, worldToPlane;
    MSBsplineSurface localSurface;
    double zLow, zHigh;
    DRange3d meshRange;
    StatusInt status;

    meshRange.Init ();
    /* rotMatrix_orthogonalFromZRow constructs by ROW  */
    rotMatrix_orthogonalFromZRow (&matrix, (DVec3dCP) pNormal);
    matrix.TransposeOf (matrix);

    planeToWorld.InitFrom (matrix, *pOrigin);
    worldToPlane.InverseOf (planeToWorld);

    bspsurf_transformSurface (&localSurface, pSurface, &worldToPlane);

    status = mdlBspline_surfaceMeshRange (&meshRange, &localSurface, tolerance);
    bspsurf_freeSurface (&localSurface);

    zLow  = meshRange.low.z;
    zHigh = meshRange.high.z;

    if (pSignedLow)
        *pSignedLow  = zLow;
    if (pSignedHigh)
        *pSignedHigh = zHigh;
    if (pAbsLow)
        {
        if (zHigh < 0.0)
            *pAbsLow = fabs (zHigh);
        else if (zLow > 0.0)
            *pAbsLow = zLow;
        else
            *pAbsLow = 0.0;
        }

    if (pAbsHigh)
        {
        *pAbsHigh = fabs (zLow);
        if (fabs (zHigh) > *pAbsHigh)
            *pAbsHigh = fabs (zHigh);
        }
    return status;
    }
    
struct BilinearBasisValues
{
double s00, s10, s01, s11;
BilinearBasisValues ()
    {
    s00 = s10 = s01 = s11 = 0.0;
    }
BilinearBasisValues (double fx1, double fy1)
    {
    double fx0 = 1.0 - fx1;
    double fy0 = 1.0 - fy1;
    s00 = fx0 * fy0;
    s10 = fx1 * fy0;
    s01 = fx0 * fy1;
    s11 = fx1 * fy1;
    }
void Evaluate (DPoint2dR result, DPoint2dCR point00, DPoint2dCR point10, DPoint2dCR point01, DPoint2dCR point11)
    {
    result.x = s00 * point00.x + s10 * point10.x + s01 * point01.x + s11 * point11.x;
    result.y = s00 * point00.y + s10 * point10.y + s01 * point01.y + s11 * point11.y;
    }
};
#define MAX_XY_QUADRATURE_POINTS 25
struct SurfacePropertiesContext
{
MSBsplineSurfaceCR m_surface;

BSIQuadraturePoints m_xRule;
BSIQuadraturePoints m_yRule;

BilinearBasisValues m_quadBasisValues[MAX_XY_QUADRATURE_POINTS];
DPoint2d m_quadXY01[MAX_XY_QUADRATURE_POINTS];
double m_quadWeight01[MAX_XY_QUADRATURE_POINTS];
int    m_numQuadGauss;

BSITriangleQuadraturePoints m_triangleRule;

DMatrix4d m_products;
double m_parameterSpaceArea;
int     m_numEval;

void BuildTriangleGauss (int selector)
    {
    m_triangleRule.InitStrang (selector);
    }
    
void BuildQuadGauss (int numX, int numY)
    {
    m_xRule.InitGauss (numX);
    m_yRule.InitGauss (numY);
    m_numQuadGauss = 0;
    for (int i = 0; i < m_xRule.GetNumEval (); i++)
        for (int j = 0; j < m_yRule.GetNumEval (); j++)
            {
            BSIQuadraturePoints::GetXYEval (
                    m_xRule, i, 0.0, 1.0,
                    m_yRule, j, 0.0, 1.0,
                    m_quadXY01[m_numQuadGauss].x,
                    m_quadXY01[m_numQuadGauss].y,
                    m_quadWeight01[m_numQuadGauss]
                    );
            m_quadBasisValues[m_numQuadGauss] = BilinearBasisValues (
                        m_quadXY01[m_numQuadGauss].x,
                        m_quadXY01[m_numQuadGauss].y
                        );
            m_numQuadGauss++;
            }
    }
SurfacePropertiesContext (MSBsplineSurfaceCR surface, int numGauss)
    : m_surface (surface)
    {
    BuildQuadGauss (numGauss, numGauss);
    BuildTriangleGauss (6);  // Ask for Strang rule 7 -- 7 points, precise for polynomial degree 5.
    m_products = DMatrix4d::FromZero ();
    m_parameterSpaceArea = 0.0;
    m_numEval = 0;
    }
// Accumulate w * products ....
void AccumulateSurfaceProducts (double u, double v, double w)
    {
    DVec3d dU, dV, cross;
    DPoint3d xyz;
    m_surface.EvaluatePoint (xyz, dU, dV, u, v);
    m_numEval += m_numQuadGauss;
    cross.CrossProduct (dU, dV);
    double dA = w * cross.Magnitude ();
    m_products.coff[3][3] += dA;

    m_products.coff[0][3] += dA * xyz.x;
    m_products.coff[1][3] += dA * xyz.y;
    m_products.coff[2][3] += dA * xyz.z;
    
    m_products.coff[0][0] += dA * xyz.x * xyz.x;
    m_products.coff[1][1] += dA * xyz.y * xyz.y;
    m_products.coff[2][2] += dA * xyz.z * xyz.z;

    m_products.coff[0][1] += dA * xyz.x * xyz.y;
    m_products.coff[0][2] += dA * xyz.x * xyz.z;
    m_products.coff[1][2] += dA * xyz.y * xyz.z;
    
    m_products.coff[1][0] += dA * xyz.x * xyz.y;
    m_products.coff[2][0] += dA * xyz.x * xyz.z;
    m_products.coff[2][1] += dA * xyz.y * xyz.z;

    m_products.coff[3][0] += dA * xyz.x;
    m_products.coff[3][1] += dA * xyz.y;
    m_products.coff[3][2] += dA * xyz.z;
    
    }

void AccumulateTriangle (DPoint2dCR uv00, DPoint2dCR uv10, DPoint2dCR uv01)
    {
    DVec2d U, V;
    U.DifferenceOf (uv10, uv00);
    V.DifferenceOf (uv01, uv00);
    double detJ = U.CrossProduct (V);
    DPoint2d uv;
    double u, v, w;
    int numPoints = m_triangleRule.GetNumEval ();
    m_parameterSpaceArea += detJ * 0.5;
    // detJ is the local scale factor at any point.
    // the triangle integration weights will add to 0.5 so integrals are for triangle -- no factor of 0.5 on detJ!!!
    for (int i = 0; i < numPoints; i++)
        {
        m_triangleRule.GetEval (i, u, v, w);
        uv.SumOf (uv00, U, u, V, v);
        AccumulateSurfaceProducts (uv.x, uv.y, w * detJ);
        }
    }

// ASSUME uv area on a square !!!!
void AccumulateQuad (DPoint2dCR uv00, DPoint2dCR uv10, DPoint2dCR uv01, DPoint2dCR uv11)
    {
    DPoint2d uv;
    double du = uv11.x - uv00.x;
    double dv = uv11.y - uv00.y;
    double dudv = du * dv;  // ASSUME RECTANGULAR QUAD -- otherwise this needs detJ effect depending on evaluation point !!
    m_parameterSpaceArea += dudv;
    for (int i = 0; i < m_numQuadGauss; i++)
        {
        m_quadBasisValues[i].Evaluate (uv, uv00, uv10, uv01, uv11);
        AccumulateSurfaceProducts (uv.x, uv.y, m_quadWeight01[i] * dudv);
        }
    }

};

/*---------------------------------------------------------------------------------**//**
* Callback to receive regular mesh during mesh range calculation.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt surfaceIntegrals_meshFunction
(
DPoint3d    *pointP,
DPoint3d    *normalP,
DPoint2d    *paramP,
int         nColumns,
int         nRows,
CallbackArgP contextVoidP
)
    {
    SurfacePropertiesContext *context = (SurfacePropertiesContext*)contextVoidP;
    
    for (int j = 1; j < nRows; j++)
      for (int i = 1; i < nColumns; i++)
        {
        int k11 = i + j * nColumns;
        int k01 = k11 - 1;
        int k10 = k11 - nColumns;
        int k00 = k10 - 1;
        context->AccumulateQuad (
                    paramP[k00], paramP[k10], paramP[k01], paramP[k11]);
         
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Callback to receive triangle strips during mesh range calculation.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
static int surfaceIntegrals_triangleFunction
(
DPoint3d    *pointP,
DPoint3d    *normalP,
DPoint2d    *paramP,
int         nPoints,
CallbackArgP contextVoidP
)
    {
    SurfacePropertiesContext *context = (SurfacePropertiesContext*)contextVoidP;
    for (int i = 2; i < nPoints; i += 1)
        {
        if ((i & 0x01) == 0)    // EVEN
            {
            context->AccumulateTriangle (paramP[i-1], paramP[i], paramP[i-2]);
            }
        else
            {
            context->AccumulateTriangle (paramP[i-2], paramP[i], paramP[i-1]);
            }
        }
    return SUCCESS;
    }


bool MSBsplineSurface::ComputePrincipalAreaMoments (
                                  double &area,
                                  DVec3dR centroid,
                                  RotMatrixR axes,
                                  DVec3dR momentxyz
                                  ) const
    {
    DMatrix4d products;
    bool stat = ComputeSecondMomentAreaProducts (products);
    Transform localToWorld;
    localToWorld.InitIdentity ();    
    products.ConvertInertiaProductsToPrincipalAreaMoments (localToWorld, area, centroid, axes, momentxyz);
    return stat;
    }
    

bool MSBsplineSurface::ComputeSecondMomentAreaProducts (DMatrix4dR products, double relativeTolerancefForFacets, int numGauss, int &numEvaluations) const
    {
    SurfacePropertiesContext context (*this, numGauss);
    DRange3d range;
    int toleranceType = STROKETOL_PatchSegments;
    double tolerance = 5.0;
    bool cove = true;
    static int s_toleranceSelect = 1;
    if (s_toleranceSelect == 1)
        {
        GetPoleRange (range);
        toleranceType = STROKETOL_ChoordHeight;
        tolerance = relativeTolerancefForFacets * range.low.Distance (range.high);
        }

    if (SUCCESS == bspmesh_meshSurface
            (
            /*(BSplineCallback_AnnounceMeshQuadGrid)*/surfaceIntegrals_meshFunction,
            /*(BSplineCallback_AnnounceMeshTriangleStrip)*/surfaceIntegrals_triangleFunction,
            tolerance,
            toleranceType,
            NULL,
            NULL,
            0.0,
            NULL,
            (MSBsplineSurfaceCP)this,
            false,
            true,
            false,
            cove,
            &context
            ))
        {
        products = context.m_products;
        numEvaluations = context.m_numEval;
        return true;
        }
    products = DMatrix4d::FromZero ();
    return false;
    }

bool MSBsplineSurface::ComputeSecondMomentAreaProducts (DMatrix4dR products) const
    {
    int numEval = 0;
    return MSBsplineSurface::ComputeSecondMomentAreaProducts (products, 0.001, 4, numEval);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
