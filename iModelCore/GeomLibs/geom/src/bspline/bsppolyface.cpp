/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/internal2/BspPrivateApi.h>
#include <Geom/IntegerTypes/Point.h>
#include "msbsplinemaster.h"

#include "GridArrays.cpp"

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

#define NEXT(index, nPnts)              ((index + 1) % (int)nPnts)
#define PREV(index, nPnts)              (index ? index - 1 : (int)nPnts - 1)
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
void CheckZero (double a)
    {
    }
struct MeshRegion
    {
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
#ifdef CompilePrintXY
    static void PrintXY (char const *title, int index, bvector<DPoint2d> &boundary)
        {
        printf ("MeshRegion %s (index %d)  (points %d)\n",
            title, index, (int)boundary.size ());
        for (auto &xy : boundary)
            printf ("   (%.8lg, %.8lg)\n", xy.x, xy.y);
        }
#endif
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   Routines for evaluating Bezier Points/Normals                       |
|                                                                       |
+----------------------------------------------------------------------*/



bool IsSameParam (double a, double b)
    {
    static double s_paramTol = 1.0e-8;
    return fabs (b-a) < s_paramTol;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_scaleVector2d
(
DPoint2d    *outP,
DPoint2dCP  inP,
DPoint2dCP  scaleP
)
    {
    outP->x = inP->x * scaleP->x;
    outP->y = inP->y * scaleP->y;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshOutputHandler
{
virtual ~MeshOutputHandler (){}
virtual StatusInt OutputQuadMesh(DPoint3dP points, DVec3dP normals, DPoint2dP params, int nu, int nv)  {return SUCCESS;}
    
virtual StatusInt OutputTriStrip(DPoint3dP points, DVec3dP normals, DPoint2dP params, int numPoints) {return SUCCESS;}


DPoint2d m_paramScales;
bool m_needNormals;
bool m_needParams;
bvector<DPoint3d> m_points;
bvector<DVec3d>   m_normals;
bvector<DPoint2d> m_params;

MeshOutputHandler (bool needParams, bool needNormals)
    {
    m_needParams = needParams;
    m_needNormals = needNormals;
    m_paramScales.Init (1.0, 1.0);
    }

void SetParamScale (DPoint2dCR scales){m_paramScales = scales;}
void ClearArrays ()
    {
    m_points.clear ();
    m_normals.clear ();
    m_params.clear ();
    }
    
DVec3dP   GetNormalP () { return m_needNormals && m_normals.size () > 0 ? &m_normals[0] : NULL;}
DPoint2dP GetParamP  () { return m_needParams && m_params.size () > 0 ? &m_params[0] : NULL;}
DPoint3dP GetPointP  () { return m_points.size () > 0 ? &m_points[0] : NULL;}

void AddPoint (DPoint3dCR data) {m_points.push_back (data);}
void AddNormal (DVec3dCR data)  {m_normals.push_back (data);}
void AddParam (DPoint2dCR data) {m_params.push_back (data);}
void AddScaledParam (DPoint2dCR data)
    {
    DPoint2d scaledData;
    scaledData.x = data.x * m_paramScales.x;
    scaledData.y = data.y * m_paramScales.y;
    m_params.push_back (scaledData);
    }
    
StatusInt OutputQuadMeshFromArrays(int nu, int nv) {
    if (nu * nv != m_points.size ())
        return ERROR;
    return OutputQuadMesh (GetPointP (), GetNormalP (), GetParamP (), nu, nv);    
    }

StatusInt OutputTriStripFromArrays() {
    if (m_points.size () == 0)
        return SUCCESS;
    return OutputTriStrip (GetPointP (), GetNormalP (), GetParamP (), (int) m_points.size ());    
    }

GridArrays m_grid;
// ASSUME: m_parms is preloaded with proper count (and corresponsds to param range)
// 
StatusInt EvaluateQuadGrid
(
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
    m_points.resize (m_params.size ());
    if (m_needNormals)
        m_normals.resize (m_params.size ());

    m_grid.EvaluateBezier (uLo, uHi, uNum, vLo, vHi, vNum, patchBezP, reverse, m_needNormals);
    m_grid.ResolveCoordinatesAndNormals (patchBezP, reverse);
    //m_params = m_grid.uv;
    m_normals = m_grid.normal;
    m_points  = m_grid.xyz;

    return SUCCESS;
    }
};


// mesh counts on various portions of a bezier patch.
struct DirectionalGridCounts
{
int numLow;
int numInterior;
int numHigh;
void Get (int &interior, int &low, int &high)
    {
    interior = numInterior;
    low = numLow;
    high = numHigh;
    }
};
struct BezierGridCounts
{
DirectionalGridCounts uCounts;
DirectionalGridCounts vCounts;
};


typedef MeshOutputHandler & MeshOutputHandlerR;
struct BuilderParams
    {
    MeshOutputHandlerR                          handler;
    IFacetOptionsR                              options;
    int                                         reverse;                    /* => reverse normal direction(s) */
    int                                         toleranceMode;              /* => tolerance mode */
    double                                      maxEdgeLength;
    TransformP                                  toleranceTransformP;        /* => tolerance transform */
    DPoint3dP                                   toleranceCameraP;           /* => tolerance camera position */
    double                                      toleranceFocalLength;       /* => tolerance focal length */
    bool                                        normalsRequired;            /* => true to return normals */
    bool                                        parametersRequired;         /* => true to return parameters */
    DPoint2d                                    paramScale;                 /* => parameter scale */
    CallbackArgP                                userDataP;                  /* => user data */
    bvector< bvector<BezierGridCounts> > bezierGridCounts;
    bvector<bvector<MSBsplineSurface>> bezierSurfaces;
    bvector<bvector<DRange2d>> bezierRanges;

    bool LoadBeziers(MSBsplineSurfaceCR surface, int &uCount, int &vCount)
        {
        if (SUCCESS == surface.MakeBeziers(bezierSurfaces, &bezierRanges))
            {
            vCount = (int)bezierSurfaces.size ();
            if (vCount > 0 && bezierSurfaces.front ().size () > 0)
                uCount = (int)bezierSurfaces.front ().size ();
            return true;
            }
        uCount = 0;
        vCount = 0;
        return false;
        }
    MSBsplineSurfaceP GetBezierPointer (uint32_t uIndex, uint32_t vIndex, DRange2dR uvRange)
        {
        if (vIndex < bezierSurfaces.size () && uIndex < bezierSurfaces[vIndex].size ())
            {
            uvRange = bezierRanges[vIndex][uIndex];
            return &bezierSurfaces[vIndex][uIndex];
            }
        return nullptr;
        }
    // in (horizontal, along u) slice vIndex, return left low and right high
    bool GetVSliceRange(uint32_t vIndex, DRange2dR uvRange)
        {
        if (vIndex < bezierRanges.size())
            {
            uvRange.low = bezierRanges[vIndex].front ().low;
            uvRange.high = bezierRanges[vIndex].back ().high;
            return true;
            }
        return false;
        }


    int                                         degreeToBezierConst, degreeToBezierLinear;
    int DegreeToMinPerBezier (int degree)
        {
        int count = degreeToBezierConst + degreeToBezierLinear;
        if (count < 1)
            count = 1;
        return count;
        }


    bool ParamsRequired (){return parametersRequired;}

    // When triangulating transitions within bezier patch ...
    // fringe of inner grid
    bvector<DPoint2d>    uvA;
    bvector<int>  indexA;
    // outer edge
    bvector<DPoint2d>    uvB;
    bvector<int>  indexB;
    // consolidated points and normals
    bvector<DPoint3d>    xyzAB;
    bvector<DVec3d>    normalAB;
    bvector<DPoint2d>    uvAB;
    BuilderParams
        (
        MeshOutputHandlerR     _handler,
        IFacetOptionsR         _options,
        int                    _reverse,                    /* => reverse normal direction(s) */
        TransformP             _toleranceTransformP,        /* => tolerance transform */
        DPoint3dP              _toleranceCameraP,           /* => tolerance camera position */
        double                 _toleranceFocalLength,       /* => tolerance focal length */
        DPoint2d               _paramScale                  /* => parameter scale */
        )
        :
        handler (_handler),
        options (_options),
        reverse (_reverse),
        toleranceMode (STROKETOL_ChoordHeight),
        maxEdgeLength (options.GetMaxEdgeLength ()),
        toleranceTransformP (_toleranceTransformP),
        toleranceFocalLength (_toleranceFocalLength),
        normalsRequired (options.GetNormalsRequired ()),
        parametersRequired (options.GetParamsRequired ()),
        paramScale (_paramScale),
        degreeToBezierConst(1),
        degreeToBezierLinear(0)
        {
        }

    virtual ~BuilderParams()
        {
        MSBsplineSurface::ClearBeziers(bezierSurfaces);
        }

    GridArrays grid;
    
    void CorrectGridCounts (bool closedU, bool closedV);
    bool GetGridCounts (int i, int j, int &numU, int &numV, int &numLoU, int &numHiU, int &numLoV, int &numHiV)
        {
        if (j >= 0 && j < (int) bezierGridCounts.size ())
            {
            if (i >= 0 && i < (int) bezierGridCounts[j].size ())
                {
                BezierGridCounts data = bezierGridCounts[j][i];
                data.uCounts.Get (numU, numLoU, numHiU);
                data.vCounts.Get (numV, numLoV, numHiV);
                return true;
                }
            }
        return false;
        }
    };


void GetNeighborIndices (int i, int n, bool closed, int &i0, int &i1)
    {
    if (closed)
        {
        i0 = (i + n - 1) % n;
        i1 = (i + 1) % n;
        }
    else
        {
        i0 = i - 1;
        if (i0 < 0)
            i0 = 0;
        i1 = i + 1;
        if (i1 >= n)
            i1 = n - 1;
        }
    }

static int max3 (int i0, int i1, int i2)
    {
    int i = i0;
    if (i1 > i)
        i = i1;
    if (i2 > i)
        i = i2;
    return i;
    }

static int max2 (int i0, int i1)
    {
    int i = i0;
    if (i1 > i)
        i = i1;
    return i;
    }

// force edge counts to highest of adjacent interior counts.
void BuilderParams::CorrectGridCounts (bool closedU, bool closedV)
    {
    int numJ = (int)bezierGridCounts.size ();
    int numI = (int)bezierGridCounts[0].size ();
    for (int j = 0; j < numJ; j++)
        {
        for (int i = 0; i < numI; i++)
            {
            int i0, i1;
            int j0, j1;
            // These return "neighbor" indices that are always valid -- in "open" case the end index is its own neighbor.
            GetNeighborIndices (i, numI, closedU, i0, i1);
            GetNeighborIndices (j, numJ, closedV, j0, j1);
            DirectionalGridCounts uCounts = bezierGridCounts[j][i].uCounts;
            DirectionalGridCounts vCounts = bezierGridCounts[j][i].vCounts;
            DirectionalGridCounts u0Counts = bezierGridCounts[j0][i].uCounts;
            DirectionalGridCounts u1Counts = bezierGridCounts[j1][i].uCounts;
            DirectionalGridCounts v0Counts = bezierGridCounts[j][i0].vCounts;
            DirectionalGridCounts v1Counts = bezierGridCounts[j][i1].vCounts;
            // We expect low/high counts on each side of an edge to match.
            // but if either interior is higher push the edge up.
            // at fringes, push the interior to the edge.
            if (j0 == j)
                uCounts.numLow = max2 (uCounts.numInterior, uCounts.numLow);
            else
                uCounts.numLow = max3 (uCounts.numInterior, uCounts.numLow, u0Counts.numInterior);
            if (j1 == j)
                uCounts.numHigh = max2 (uCounts.numInterior, uCounts.numHigh);
            else
                uCounts.numHigh = max3 (uCounts.numInterior, uCounts.numHigh, u1Counts.numInterior);

            if (i0 == i)
                vCounts.numLow = max2 (vCounts.numInterior, vCounts.numLow);
            else
                vCounts.numLow = max3 (vCounts.numInterior, vCounts.numLow, v0Counts.numInterior);
            if (i1 == i)
                vCounts.numHigh = max2 (vCounts.numInterior, vCounts.numHigh);
            else
                vCounts.numHigh = max3 (vCounts.numInterior, vCounts.numHigh, v1Counts.numInterior);

            bezierGridCounts[j][i].uCounts = uCounts;
            bezierGridCounts[j][i].vCounts = vCounts;
            }
        }
    }


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


/*---------------------------------------------------------------------------------**//**
* Triangulate in a region known to be a simple strip in which triangulation edgs always go from one side between the uvA and uvB arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_triangulateStripPair
(
    BuilderParams          *mpP,                       /* => mesh parameters */
    MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
    bvector<DPoint2d>  &uvA,            // left side of strip
    bvector<DPoint2d> &uvB,             // right side of strip
    double              uMin,                       /* => u Minimum */
    double              uMax,                       /* => u Maximum */
    double              vMin,                       /* => v Minimum */
    double              vMax                        /* => v Maximum */
);
static int bsppolyface_getCurveSteps
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

static int s_maxStepsInPatchSubdivision = 500;

/*----------------------------------------------------------------------+
|                                                                       |
|   Utility routines                                                    |
|                                                                       |
+----------------------------------------------------------------------*/

static int integerizeStepCount (double value)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void enforceLimits
(
int &value,
int min,
int max
)
    {
    if (value < min)
        value = min;
    if (value > max)
        value = max;
    }

static void enforcePositiveLimits (int &value, int min, int max)
    {
    if (min > 0 && value < min)
        value = min;
    if (max > 0 && value > max)
        value = max;
    }



/*---------------------------------------------------------------------------------**//**
* Pass the output buffer to the triangulation logic
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_triangulateStripPair
(
    BuilderParams *mpP,
    PatchMeshParams *pmpP,
    bvector<DPoint2d> &uvA,
    bvector<DPoint2d> &uvB
)
    {
    bspmesh_triangulateStripPair(mpP, pmpP->bezierP, mpP->uvA, mpP->uvB,
        pmpP->umin, pmpP->umax,
        pmpP->vmin, pmpP->vmax);
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* Copy points from an array to the output buffer, allowing array wrap and checking for double points within the array
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_distinctPointsFromIndices
(
    bvector<DPoint2d> &uv,
    DPoint2d *pointP,
    int nPoint,
    int i0,
    int i1,
    PatchMeshParams *pmpP
)
    {
    uv.clear ();
    int i, j, steps, step;
    DPoint2d vector, point, scaleVector;
    if (i0 < 0 || i1 < 0 || i0 >= nPoint || i1 >= nPoint)
        return;
    scaleVector.x = pmpP->umax - pmpP->umin;
    scaleVector.y = pmpP->vmax - pmpP->vmin;

    uv.push_back(pointP[i0]);

    for (i = i0; i != i1; i = j)
        {
        j = i + 1;
        if (j >= nPoint) j = 0;
        if (pointP[j].x != pointP[i].x || pointP[j].y != pointP[i].y)
            {
            steps = bsppolyface_getCurveSteps
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
            for (step = steps - 1; step >= 0; step--)
                {
                double f = (double)step / (double)steps;
                point.x = pointP[j].x + f * vector.x;
                point.y = pointP[j].y + f * vector.y;
                uv.push_back(point);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Interpolate distinct points from startP..endP into the output buffer If one point is requested, just put startP
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_uniformPoints
(
bvector<DPoint2d> &uv,
DPoint2dCR start,
DPoint2dCR end,
int nPoint
)
    {
    int i;
    int nStep = nPoint - 1;
    DPoint2d point;
    DPoint2d vector;
    uv.clear ();
    if (nPoint == 1)
        {
        uv.push_back(start);
        }
    else if (nPoint >= 1)
        {
        uv.push_back(start);

        vector.x = end.x - start.x;
        vector.y = end.y - start.y;

        for (i = 1; i < nStep; i++)
            {
            double f = (double)i / (double)nStep;
            point.x = start.x + f * vector.x;
            point.y = start.y + f * vector.y;
            uv.push_back(point);
            }

        uv.push_back(end);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
@param BuilderParams mpP IN mesh params
+----------------------------------------------------------------------*/
static void    updateDeviations
(
int     baseIndex,
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
double &polygonLength,
BuilderParams          *mpP                /* => mesh parameters */
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

    polygonLength += mag12;
    if (baseIndex == 0)
        polygonLength += mag01;

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
        if (mpP->options.GetAngleTolerance () > 0.0)
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
double polygonLength,
BuilderParams          *mpP                /* => mesh parameters */
)
    {
    int count;
    double maxEdgeLength = mpP->options.GetMaxEdgeLength ();
    double chordTolerance = mpP->options.GetChordTolerance ();
    double angleTolerance = mpP->options.GetAngleTolerance ();
    static double s_chordErrorFactor = 1.0;  // In SS3, this factor was 10.0 -- applied inside sqrt, so it made about 3X as many
        // facets as expected.
        // But in SS3 elementFacet.cpp, the same factor was applied to the tolerance, so it all canceled out.
        // For Vancouver, the caller factor is not applied, so we leave this at 1.0.
    if (!pCount)
        return;
    if (degree < 1)
        degree = 1;
    count = mpP->DegreeToMinPerBezier (degree);
    if (count < diagonalCount)
        count = diagonalCount;
    if (chordTolerance > 0.0)
        {
        int chordDiffCount = (int) (1.0 + sqrt (s_chordErrorFactor * maxChordDiff/chordTolerance));
        if (chordDiffCount > count)
            count = chordDiffCount;
        }
    if (angleTolerance > 0.0 && maxAngle > angleTolerance)
        {
        int angleCount = (int) (1.0 + (double)degree * maxAngle / angleTolerance);
        if (angleCount > count)
            count = angleCount;
        }
    if (maxEdgeLength > 0)
        {
        int edgeCount = (int) (1.0 + polygonLength / maxEdgeLength);
        if (edgeCount > count)
            count = edgeCount;
        }
    *pCount = count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    applyPatchTolerances
(
int                 *uNum,              /* <= # steps in U */
int                 *vNum,              /* <= # steps in V */
int                 *uLoNum,            /* <= # steps along u Lo boundary */
int                 *uHiNum,            /* <= # steps along u Hi boundary */
int                 *vLoNum,            /* <= # steps along v Lo boundary */
int                 *vHiNum,            /* <= # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* => bezier patch */
BuilderParams          *mpP                /* => mesh parameters */
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
    // diagaonal count is being passed to edges.  Why?  This creates mismatches
    //    because "other side" of edge has different diagonal count.
    //   But whey didn't this show up in SS3 hline?  Dunno.
    static int s_diagonalFactorForLoHi = 0;   // 0 suppresses transfer of diagonal to edges.
    int numPositive, numNegative;
    double polygonLength, polygonLength0, polygonLength1, polygonLength2;
    double          angleTolerance = mpP->options.GetAngleTolerance ();
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
    double chordTolerance = mpP->options.GetChordTolerance ();
    if (chordTolerance <= 0.0)
        diagonalCount = 1;
    else
        diagonalCount = (int) (0.999 + sqrt (diagonalError/chordTolerance));
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

    polygonLength0 = polygonLength1 = polygonLength2 = 0.0;
    /* Find uNum */
    if (uOrder > 2)
        {
        maxDiff = maxLoDiff = maxHiDiff = 0.0;
        maxAngle = maxLoAngle = maxHiAngle = 0.0;
        for (j=0, poleP=poles; j<vOrder; j++, poleP += uOrder)
            {
            polygonLength = 0.0;
            for (i=0, pPtr=poleP; i<uIndex; i++, pPtr++)
                {
                updateDeviations (i, pPtr, pPtr + 1, pPtr + 2,
                            j == 0,
                            j == vOrder - 1,
                            &maxDiff, &maxLoDiff, &maxHiDiff,
                            &maxAngle, &maxLoAngle, &maxHiAngle,
                            polygonLength,
                            mpP
                            );
                }
            if (j == 0)
                polygonLength0 = polygonLength;
            if (polygonLength > polygonLength1)
                polygonLength1 = polygonLength;
            if (j + 1 == vOrder)
                polygonLength2 = polygonLength;
            }
        computeChordCount (uNum,   uDegree, diagonalCount, maxDiff,   maxAngle, polygonLength1, mpP);
        computeChordCount (uLoNum, uDegree, diagonalCount1, maxLoDiff, maxLoAngle, polygonLength0, mpP);
        computeChordCount (uHiNum, uDegree, diagonalCount1, maxHiDiff, maxHiAngle, polygonLength2, mpP);
        }
    else if (uOrder == 2 && mpP->options.GetMaxEdgeLength () > 0.0)
        {
        for (j=0, poleP=poles; j<vOrder; j++, poleP += uOrder)
            {
            polygonLength = poleP[0].Distance (poleP[1]);
            if (j == 0)
                polygonLength0 = polygonLength;
            if (polygonLength > polygonLength1)
                polygonLength1 = polygonLength;
            if (j + 1 == vOrder)
                polygonLength2 = polygonLength;
            }
        computeChordCount (uNum,   uDegree, diagonalCount, 0.0, 0.0, polygonLength1, mpP);
        computeChordCount (uLoNum, uDegree, diagonalCount1, 0.0, 0.0, polygonLength0, mpP);
        computeChordCount (uHiNum, uDegree, diagonalCount1, 0.0, 0.0, polygonLength2, mpP);
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
        polygonLength0 = polygonLength1 = polygonLength2 = 0.0;
        for (j=0, poleP=poles; j < uOrder; j++, poleP++)
            {
            polygonLength = 0.0;
            for (i=0, pPtr=poleP; i < vIndex; i++, pPtr += uOrder)
                {
                updateDeviations (i, pPtr, pPtr + uOrder, pPtr + 2 * uOrder,
                            j == 0,
                            j == uOrder - 1,
                            &maxDiff, &maxLoDiff, &maxHiDiff,
                            &maxAngle, &maxLoAngle, &maxHiAngle,
                            polygonLength,
                            mpP
                            );
                }
            if (j == 0)
                polygonLength0 = polygonLength;
            if (polygonLength > polygonLength1)
                polygonLength1 = polygonLength;
            if (j + 1 == uOrder)
                polygonLength2 = polygonLength;
            }
        computeChordCount (vNum,   vDegree, diagonalCount, maxDiff,   maxAngle, polygonLength1, mpP);
        computeChordCount (vLoNum, vDegree, diagonalCount1, maxLoDiff, maxLoAngle, polygonLength0, mpP);
        computeChordCount (vHiNum, vDegree, diagonalCount1, maxHiDiff, maxHiAngle, polygonLength2, mpP);
        }
    else if (vOrder == 2 && mpP->options.GetMaxEdgeLength () > 0.0)
        {
        polygonLength0 = polygonLength1 = polygonLength2 = 0.0;
        for (j=0, poleP=poles; j < uOrder; j++, poleP++)
            {
            polygonLength = poleP[0].Distance (poleP[uOrder]);
            if (j == 0)
                polygonLength0 = polygonLength;
            if (polygonLength > polygonLength1)
                polygonLength1 = polygonLength;
            if (j + 1 == uOrder)
                polygonLength2 = polygonLength;
            }
        computeChordCount (vNum,   vDegree, diagonalCount, 0.0, 0.0, polygonLength1, mpP);
        computeChordCount (vLoNum, vDegree, diagonalCount1, 0.0, 0.0, polygonLength0, mpP);
        computeChordCount (vHiNum, vDegree, diagonalCount1, 0.0, 0.0, polygonLength2, mpP);
        }
    else
        {
        *vNum = diagonalCount;
        if (vLoNum) *vLoNum = 1;
        if (vHiNum) *vHiNum = 1;
        }
    static int s_suppressInterior = 0;
    if (s_suppressInterior == 2 && uOrder == 2 && vOrder == 2)
        {
        *uNum = *vNum = 1;
        }
        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bsppolyface_getCurveSteps
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_getSurfaceSteps
(
int                 &uNum,              /* <= # steps in U */
int                 &vNum,              /* <= # steps in V */
int                 &uLoSteps,          /* <= # steps along u Lo boundary */
int                 &uHiSteps,          /* <= # steps along u Hi boundary */
int                 &vLoSteps,          /* <= # steps along v Lo boundary */
int                 &vHiSteps,          /* <= # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* => bezier patch */
BuilderParams          *mpP                /* => mesh parameters */
)
    {
    applyPatchTolerances (&uNum, &vNum, &uLoSteps, &uHiSteps, &vLoSteps, &vHiSteps,
                                         patchBezP, mpP);

    int minPerBezier = mpP->options.GetMinPerBezier ();
    int maxPerBezier = mpP->options.GetMaxPerBezier ();
    if (minPerBezier != 0 || maxPerBezier != 0)
        {
        enforcePositiveLimits (uNum,     minPerBezier, maxPerBezier);
        enforcePositiveLimits (uLoSteps, minPerBezier, maxPerBezier);
        enforcePositiveLimits (uHiSteps, minPerBezier, maxPerBezier);
        enforcePositiveLimits (vNum,     minPerBezier, maxPerBezier);
        enforcePositiveLimits (vLoSteps, minPerBezier, maxPerBezier);
        enforcePositiveLimits (vHiSteps, minPerBezier, maxPerBezier);
        }

    enforceLimits (uNum,     1, s_maxStepsInPatchSubdivision);
    enforceLimits (uLoSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (uHiSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (vNum,     1, s_maxStepsInPatchSubdivision);
    enforceLimits (vLoSteps, 1, s_maxStepsInPatchSubdivision);
    enforceLimits (vHiSteps, 1, s_maxStepsInPatchSubdivision);
    }


void SetupPatchesAndGridCountsInSurface
(
BuilderParams *mpP,
MSBsplineSurfaceP surface,
int &uCount,
int &vCount
)
    {
    mpP->LoadBeziers (*surface, uCount, vCount);
    mpP->bezierGridCounts.clear ();
    for (int j=0; j < vCount; j++)
        {
        mpP->bezierGridCounts.push_back (bvector<BezierGridCounts> ());
        for (int i=0; i < uCount; i++)
            {
            BezierGridCounts counts;
            bspmesh_getSurfaceSteps (
                  counts.uCounts.numInterior, counts.vCounts.numInterior,
                  counts.uCounts.numLow, counts.uCounts.numHigh,
                  counts.vCounts.numLow, counts.vCounts.numHigh,
                                      &mpP->bezierSurfaces[j][i], mpP);
            mpP->bezierGridCounts.back ().push_back (counts);
                
            }
        }

    bool uClosed, vClosed;
    surface->IsPhysicallyClosed (uClosed, vClosed);
    uClosed |= surface->uParams.closed != 0;
    vClosed |= surface->vParams.closed != 0;
    mpP->CorrectGridCounts (uClosed, vClosed);
    }



/*---------------------------------------------------------------------------------**//**
// 2 callers -- simple triangulator will be gridded, vu extractor not.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsppolyface_evaluatePointsBezier
(
BuilderParams          *mpP,                       /* => mesh parameters */
bvector<DPoint3d> &points,
bvector<DVec3d>   &normals,
bool                needNormals,
MSBsplineSurface    *patchBezP,     /* => Bezier patch */
bvector<DPoint2d>   &uv,
double              uMin,           /* => u Minimum (entire surface) */
double              uMax,           /* => u Maximum (entire surface) */
double              vMin,           /* => v Minimum (entire surface) */
double              vMax,           /* => v Maximum (entire surface) */
bool                reverseNormal   /* => if true negate normal (dPdV X DPdU) */
)
    {
    mpP->grid.EvaluateBezier (uv, uMin, uMax, vMin, vMax, patchBezP, needNormals);
    mpP->grid.ResolveCoordinatesAndNormals (patchBezP, reverseNormal);
    normals = mpP->grid.normal;
    points = mpP->grid.xyz;
    }

/*---------------------------------------------------------------------------------**//**
* evaluate points and normals.
* for input point i that has output k, indices[i]=k
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsppolyface_evaluatePointsBezierAppend
(
    BuilderParams          *mpP,                       /* => mesh parameters */
    bvector<DPoint3d> &points,
    bvector<DVec3d>   &normals,
    bvector<DPoint2d> &uvOut,
    bvector<int>      &indices,
    bool                needNormals,
    MSBsplineSurface    *patchBezP,     /* => Bezier patch */
    bvector<DPoint2d>   &uv,
    double              uMin,           /* => u Minimum (entire surface) */
    double              uMax,           /* => u Maximum (entire surface) */
    double              vMin,           /* => v Minimum (entire surface) */
    double              vMax,           /* => v Maximum (entire surface) */
    bool                reverseNormal   /* => if true negate normal (dPdV X DPdU) */
)
    {
    mpP->grid.EvaluateBezier(uv, uMin, uMax, vMin, vMax, patchBezP, needNormals);
    mpP->grid.ResolveCoordinatesAndNormals(patchBezP, reverseNormal);
    BeAssert (uv.size () == mpP->grid.xyz.size ());
    if (needNormals)
        BeAssert( uv.size() == mpP->grid.normal.size());

    for (size_t i = 0; i < uv.size (); i++)
        {
        indices.push_back ((int)points.size ());
        uvOut.push_back (uv[i]);
        points.push_back (mpP->grid.xyz[i]);
        }
    for (auto &uvw : mpP->grid.normal)
        normals.push_back(uvw);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int my_compareDoubles
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

/*---------------------------------------------------------------------------------**//**
* Find intersections of a loop with a cut line. Record edges that touch the loop and head downward in the primary intercept array. Record
* other points (e.g. touch points) in the secondary array
* @bsimethod
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
        int curr = my_compareDoubles (yStart, p[currIndex].y);
        int next = my_compareDoubles (yStart, p[nextIndex].y);
        for (size_t i=1; i<numVerts; i++)
            {
            int prev = curr;
            size_t prevIndex = currIndex;
            curr = next;
            currIndex = nextIndex;
            nextIndex = (i+1) % (numVerts-1);
            next = my_compareDoubles (yStart, p[nextIndex].y);
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
* @bsimethod
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

// compress the v coordinates throughout the regions so adjacent ones are not within epsilon.
void bspmesh_compressSmallDeltasInTrimCoordinates(
bvector<MeshRegion> &regions,
int uvIndex,
double epsilon
)
    {
    bvector<double> values;
    for (auto &r : regions)
        {
        for (auto &p : r.boundary)
            values.push_back (uvIndex == 0 ? p.x : p.y);
        }
    std::sort(values.begin (), values.end ());
    bvector<DRange1d> smallRanges;
    // collect nontrivial ranges
    auto n = values.size ();
    for (size_t i0 = 0; i0 < n;)
        {
        double v0 = values[i0];
        double v1 = v0;
        size_t i1 = i0 + 1;
        for (; i1 < n && values[i1] < v0 + epsilon; i1++)
            {
            v1 = values[i1];
            }
        //
        if (v1 > v0)
            {
            smallRanges.push_back (DRange1d::From (v0, v1));
            }
        i0 = i1;
        }
    // force values back to v0 of the range where they occur
    for (auto &r : regions)
        {
        for (auto &p : r.boundary)
            {
            double a = (uvIndex == 0) ? p.x : p.y;
            for (size_t i = 0 ; i < smallRanges.size (); i++)
                {
                if (smallRanges[i].Contains(a))
                    {
                    double b = smallRanges[i].low;
                    if (uvIndex == 0)
                        p.x = b;
                    else
                        p.y = b;
                    }
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    std::sort (extrema.begin(), extrema.end(), std::greater<>());
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
            _Analysis_assume_(intersectionP != nullptr); // see the conditional assignment above.
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_processTriangle
(
BuilderParams          *mpP,
bvector<DPoint2d>  &uv,
bvector<DPoint3d>   &pointP,
bvector<DVec3d>     &normalP,
int                 i0,
int                 i1,
int                 i2
)
    {
    int             iTmp;
    double          dotProduct;

    mpP->handler.ClearArrays ();


   dotProduct = (uv[i2].x - uv[i1].x) * (uv[i0].y - uv[i1].y) -
                (uv[i0].x - uv[i1].x) * (uv[i2].y - uv[i1].y);

   if (mpP->reverse ? dotProduct > 0.0 : dotProduct < 0.0)
        {
        iTmp = i0;
        i0   = i2;
        i2   = iTmp;
        }
    mpP->handler.AddPoint (pointP[i0]);
    mpP->handler.AddPoint (pointP[i1]);
    mpP->handler.AddPoint (pointP[i2]);

    if (mpP->normalsRequired)
        {
        mpP->handler.AddNormal (normalP[i0]);
        mpP->handler.AddNormal (normalP[i1]);
        mpP->handler.AddNormal (normalP[i2]);
        }

    if (mpP->ParamsRequired ())
        {
        mpP->handler.AddScaledParam (uv[i0]);
        mpP->handler.AddScaledParam (uv[i1]);
        mpP->handler.AddScaledParam (uv[i2]);
        }

    return mpP->handler.OutputTriStripFromArrays ();
    }
/*---------------------------------------------------------------------------------**//**
* Triangulate in a region known to be a simple strip in which triangulation edgs always go from one side to the other using either X or Y as
* the sweep direction. Point loop is assumed to have no duplicates. Do not duplicate the end point 'start' index of -1 indicates last point is
* the start
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_triangulateStripPair
(
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d>  &uvA,            // left side of strip
bvector<DPoint2d> &uvB,             // right side of strip
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
)
    {
    mpP->uvAB.clear ();
    mpP->xyzAB.clear ();
    mpP->normalAB.clear ();
    mpP->indexA.clear ();
    mpP->indexB.clear ();
    bvector<Point3d> triangleIndices;
    bsppolyface_evaluatePointsBezierAppend(mpP, mpP->xyzAB, mpP->normalAB,mpP->uvAB, mpP->indexA, mpP->normalsRequired,
        bezierP, uvA,
        uMin, uMax, vMin, vMax, 0 != mpP->reverse);
    bsppolyface_evaluatePointsBezierAppend(mpP, mpP->xyzAB, mpP->normalAB, mpP->uvAB, mpP->indexB, mpP->normalsRequired,
        bezierP, uvB,
        uMin, uMax, vMin, vMax, 0 != mpP->reverse);

    PolylineOps::GreedyTriangulateBetweenLinestringsWithIndices (uvA, mpP->indexA, uvB, mpP->indexB, &triangleIndices, nullptr);
    for (auto &tri : triangleIndices)
        {
        if (SUCCESS !=  bspmesh_processTriangle(mpP,
            mpP->uvAB, mpP->xyzAB, mpP->normalAB,
            tri.x, tri.y, tri.z))
            break;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileCoordinates
(
BuilderParams          *mpP,                       /* => mesh parameters */
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
    int             status = ERROR, totalPoints, i, j;

    DPoint2d        pnt, min, max,
                    paramMin, paramDelta;
    DPoint2d delta;

    totalPoints = nU * nV;

    if ( nU < 2 || nV < 2 )
        return SUCCESS;
        
    mpP->handler.ClearArrays ();

    min.x = u0;
    min.y = v0;
    delta.x = ( u1 - u0 ) / ( nU - 1 );
    delta.y = ( v1 - v0 ) / ( nV - 1 );

    bspmesh_scaleVector2d (&paramMin, &min, paramScaleP);
    bspmesh_scaleVector2d (&paramDelta, &delta, paramScaleP);
    if (mpP->ParamsRequired ())
        {
        for (i=0, pnt.y=paramMin.y;
                i < nV;
                    i++, pnt.y += paramDelta.y)
            for (j=0, pnt.x = paramMin.x;
                    j < nU;
                        j++, pnt.x += paramDelta.x)
                mpP->handler.AddParam (pnt);
        }

    min.x = (min.x - uMin)/(uMax - uMin);
    min.y = (min.y - vMin)/(vMax - vMin);
    max.x = (u1 - uMin)/(uMax - uMin);
    max.y = (v1 - vMin)/(vMax - vMin);

    if (SUCCESS == mpP->handler.EvaluateQuadGrid (bezierP,
                        min.x, max.x,
                        min.y, max.y,
                        nU, nV, reverse)
       )
        status = mpP->handler.OutputQuadMeshFromArrays( nU, nV);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int tilePatchOfBezier
(
BuilderParams          *mpP,                       /* => mesh parameters */
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
    int             status = ERROR, totalPoints, i, j, nU, nV, uMinIndex, uMaxIndex,
                    vMinIndex, vMaxIndex;
    DPoint2d        pnt, min, max,
                    paramMin, paramDelta;

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

    mpP->handler.ClearArrays ();
    
    min.x = uMinIndex * deltaP->x;
    min.y = vMinIndex * deltaP->y;

    bspmesh_scaleVector2d (&paramMin, &min, paramScaleP);
    bspmesh_scaleVector2d (&paramDelta, deltaP, paramScaleP);
    if (mpP->ParamsRequired ())
        {
        for (i=vMinIndex, pnt.y=paramMin.y;
                i <= vMaxIndex;
                    i++, pnt.y += paramDelta.y)
            for (j=uMinIndex, pnt.x = paramMin.x;
                    j <= uMaxIndex;
                        j++, pnt.x += paramDelta.x)
                mpP->handler.AddParam (pnt);
        }

    min.x = (min.x - uMin)/(uMax - uMin);
    min.y = (min.y - vMin)/(vMax - vMin);
    max.x = ((double) uMaxIndex * deltaP->x - uMin)/(uMax - uMin);
    max.y = ((double) vMaxIndex * deltaP->y - vMin)/(vMax - vMin);

    if (SUCCESS == mpP->handler.EvaluateQuadGrid (bezierP,
                        min.x, max.x,
                        min.y, max.y,
                        nU, nV, reverse)
       )
        status = mpP->handler.OutputQuadMeshFromArrays( nU, nV);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_rectangularFaceMesh
(
BuilderParams *mpP,
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
    nu = integerizeStepCount((umax - umin) / duBezier);
    nv = integerizeStepCount((vmax - vmin) / dvBezier);
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

    if (   nu == 2 && nv == 2
        && nu == pmpP->uLoSteps && nu == pmpP->uHiSteps
        && nv == pmpP->vLoSteps && nv == pmpP->vHiSteps
        )
        {
        status = bspmesh_tileCoordinates(mpP, pmpP->bezierP,
            umin, umax, vmin, vmax,
            2,2,
            pmpP->umin, pmpP->umax,
            pmpP->vmin, pmpP->vmax,
            &mpP->paramScale, 0 != mpP->reverse);
        return SUCCESS;
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

    bspmesh_distinctPointsFromIndices(mpP->uvA, pointP, nPoint, iYMin, iXMax, pmpP);
    bspmesh_uniformPoints(mpP->uvB, llTile, lrTile, nu - 1);
    bspmesh_triangulateStripPair(mpP, pmpP, mpP->uvA, mpP->uvB);

    bspmesh_distinctPointsFromIndices(mpP->uvA, pointP, nPoint, iXMax, iYMax, pmpP);
    bspmesh_uniformPoints(mpP->uvB, lrTile, urTile, nv - 1);
    bspmesh_triangulateStripPair(mpP, pmpP, mpP->uvA, mpP->uvB);

    bspmesh_distinctPointsFromIndices(mpP->uvA, pointP, nPoint, iYMax, iXMin, pmpP);
    bspmesh_uniformPoints(mpP->uvB, urTile, ulTile, nu - 1);
    bspmesh_triangulateStripPair(mpP, pmpP, mpP->uvA, mpP->uvB);

    bspmesh_distinctPointsFromIndices(mpP->uvA, pointP, nPoint, iXMin, iYMin, pmpP);
    bspmesh_uniformPoints(mpP->uvB, ulTile, llTile, nv - 1);
    bspmesh_triangulateStripPair(mpP, pmpP, mpP->uvA, mpP->uvB);
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt trySimpleRectangularFaceMesh
(
BuilderParams *mpP,
DPoint2d *pointP,
int nPoint,
int iXMin,              /* => min point in lexical x order (upper left) */
int iXMax,              /* => max point in lexical x order (lower right)*/
int iYMin,              /* => min point in lexical y order (lower left) */
int iYMax,              /* => max point in lexical y order (upper right) */
PatchMeshParams &patchParams   /* => meshing parameters for this bezier patch */
)
    {
    StatusInt status = ERROR;
    if (   patchParams.uSteps == patchParams.uLoSteps
        && patchParams.uSteps == patchParams.uHiSteps
        && patchParams.vSteps == patchParams.vLoSteps
        && patchParams.vSteps == patchParams.vHiSteps
        )
        {
        int nu = patchParams.uSteps;  // These are edge counts !!!
        int nv = patchParams.vSteps;

        double umin = pointP[iXMin].x;
        double umax = pointP[iXMax].x;
        double vmin = pointP[iYMin].y;
        double vmax = pointP[iYMax].y;
        if (   IsSameParam (umin, patchParams.umin)
            && IsSameParam (umax, patchParams.umax)
            && IsSameParam (vmin, patchParams.vmin)
            && IsSameParam (vmax, patchParams.vmax)
            )
            status = bspmesh_tileCoordinates (mpP,patchParams.bezierP,
                        umin, umax, vmin, vmax,
                        nu + 1, nv + 1,
                        patchParams.umin, patchParams.umax,
                        patchParams.vmin, patchParams.vmax,
                        &mpP->paramScale, 0 != mpP->reverse);


        }



    return status;
    }


/*---------------------------------------------------------------------------------**//**
* Mesh a region using simplest possible methods. Return SUCCESS if one or another method worked, ERROR if not.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSimpleBezierRegion
(
BuilderParams          *mpP,                       /* => mesh parameters */
DPoint2d            *pointP,                    /* => slice boundaries */
int                 nPoint,                     /* => number of slice bound points */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
int uIndex,
int vIndex,
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

    DPoint2dOps::LexicalXExtrema ( iXMin, iXMax, pointP, nPoint );
    DPoint2dOps::LexicalYExtrema ( iYMin, iYMax, pointP, nPoint );

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
                                pmpP->uSteps,   pmpP->vSteps,
                                pmpP->uLoSteps, pmpP->uHiSteps,
                                pmpP->vLoSteps, pmpP->vHiSteps,
                              bezierP, mpP);

        mpP->GetGridCounts (uIndex, vIndex,
                                pmpP->uSteps,   pmpP->vSteps,
                                pmpP->uLoSteps, pmpP->uHiSteps,
                                pmpP->vLoSteps, pmpP->vHiSteps);


        if (SUCCESS == trySimpleRectangularFaceMesh (mpP, pointP, nPoint,
                iXMin, iXMax, iYMin, iYMax, patchMeshParams))
            {
            status = SUCCESS;
            }
        else
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_dumpVuTriangles
(
VuSetP              graphP,                     /* => triangulated vertex use graph */
BuilderParams          *mpP,                       /* => mesh parameters */
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
        /* Evaluate the spatial coordinates */
        bsppolyface_evaluatePointsBezier(mpP, points, normals, mpP->normalsRequired, bezierP, params,
                                        bezierUMin, bezierUMax, bezierVMin, bezierVMax,
                                        0 != mpP->reverse );
        /* sweep through the mesh */
        vu_clearMaskInSet( graphP, visitedMask | scratchMask);
        VU_SET_LOOP ( seedP, graphP )
            {
            int count = 0;
            if (SUCCESS != status )
                break;
            face0P = seedP;
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
                    status = bspmesh_processTriangle( mpP, params, points, normals, i0, i1, i2 );
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
* @bsimethod
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
BuilderParams *mpP
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
        steps = bsppolyface_getCurveSteps( uSteps, vSteps,
                                       uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                       &uvCurr, &uvNext, bezierScaleP,
                                       bezierUMin, bezierUMax, bezierVMin, bezierVMax );
        bspmesh_splitEdge( graphP, currP, steps );
        currP = nextP;

        } while ( currP != startP );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
    if (j < 1)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh1_meshBezierRegion
(
BuilderParams          *mpP,                       /* => mesh parameters */
DPoint2d            *points,                    /* => slice boundaries */
int                 nPoints,                    /* => number of slice bound points */
MSBsplineSurface    *bezierP,                   /* => bezier surface buffer */
int patchI,
int patchJ,
double              bezierUMin,                 /* => u Minimum for bezier */
double              bezierUMax,                 /* => u Maximum for bezier */
double              bezierVMin,                 /* => v Minimum for bezier */
double              bezierVMax                  /* => v Maximum for bezier */
)
    {
    VuSetP graphP = vu_newVuSet (0);
    int uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps;
    VuP startP;
    double xScale, yScale;
    static int tristrips = BSPMESH_OUTPUT_TRISTRIP;
    int status = SUCCESS;
    static int processVu = 1;   /* set this to zero to trap unmodified graph in rxnurbs */
    DPoint2d bezierScale, bezierOrigin, blockSize, marginSize;
    bspmesh_getSurfaceSteps (uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                              bezierP, mpP);

    mpP->GetGridCounts (patchI, patchJ, uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps);

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
            status = bspmesh_dumpVuTriangles (
                        graphP, mpP, bezierP,
                        bezierUMin, bezierUMax, bezierVMin, bezierVMax, tristrips );
            }
        }

    vu_freeVuSet (graphP);
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurfaceSlice
(
BuilderParams          *mpP,                       /* => mesh parameters */
bvector<DPoint2d>  &slicePoints,
int                 vIndex,
int                 uNumSegs                   /* => number of U segments */
)
    {
    int             minIndex, maxIndex, j, minLeftIndex, minRightIndex, next,
                    index, maxLeftIndex, maxRightIndex, status;
    double          sliceMin, sliceMax;
    DPoint2d        minLeftPoint, minRightPoint,
                    maxLeftPoint, maxRightPoint;
    static int alwaysUseOldMesher = 0;

    size_t nSlice = slicePoints.size ();
    minIndex = maxIndex = 0;
    sliceMin = sliceMax = slicePoints[0].x;
    for (size_t i = 1; i < nSlice; i++)
        {
        double x = slicePoints[i].x;
        if (x > sliceMax)
            {
            sliceMax = x;
            maxIndex = (int)i;
            }
        if (x < sliceMin)
            {
            sliceMin = x;
            minIndex = (int)i;
            }
        }

    minLeftIndex = minRightIndex = minIndex;
    minLeftPoint = minRightPoint = slicePoints[minIndex];
    for (j=0, status=SUCCESS; j < uNumSegs && status==SUCCESS; j++)
        {
        int uIndex = j;
        DRange2d uvRange;
        MSBsplineSurfaceP myBezierP = mpP->GetBezierPointer(uIndex, vIndex, uvRange);
        double uMin = uvRange.low.x;
        double uMax = uvRange.high.x;
        if (myBezierP == nullptr)
            continue;
        maxLeftPoint.x = maxRightPoint.x = uvRange.high.x;
        if (sliceMin >= uMax || sliceMax <= uMin)
            continue;

        for (maxLeftIndex = minLeftIndex, next = PREV(maxLeftIndex, nSlice);
                slicePoints[next].x < uMax && maxLeftIndex != maxIndex;
                    maxLeftIndex = next, next = PREV(next, nSlice));

        for (maxRightIndex = minRightIndex, next = NEXT(maxRightIndex, nSlice);
                slicePoints[next].x < uMax && maxRightIndex != maxIndex;
                    maxRightIndex = next, next = NEXT (next, nSlice));

        if (maxLeftIndex == maxIndex)
            maxLeftPoint = slicePoints[maxIndex];
        else
            maxLeftPoint.y = bspmesh_horizIntercept (maxLeftPoint.x,
                                                     &slicePoints[maxLeftIndex],
                                                     &slicePoints[PREV(maxLeftIndex, nSlice)]);

        if (maxRightIndex == maxIndex)
            maxRightPoint = slicePoints[maxIndex];
        else
            maxRightPoint.y = bspmesh_horizIntercept (maxRightPoint.x,
                                                      &slicePoints[maxRightIndex],
                                                      &slicePoints[NEXT(maxRightIndex, nSlice)]);

        bvector<DPoint2d> points;
        points.push_back (maxLeftPoint);

        for (index=maxLeftIndex;
                 index != minLeftIndex;
                     index=NEXT(index, nSlice))
            points.push_back (slicePoints[index]);

        points.push_back (minLeftPoint);
        points.push_back (minRightPoint);
        for (index=NEXT(minRightIndex, nSlice);
                index != NEXT(maxRightIndex, nSlice);
                    index=NEXT(index, nSlice))
            points.push_back (slicePoints[index]);

        points.push_back (maxRightPoint);
        
        if  ( alwaysUseOldMesher ||
              SUCCESS != (status = meshSimpleBezierRegion
                                        (
                                        mpP, &points[0], (int)points.size (), myBezierP, uIndex, vIndex,
                                        uvRange.low.x, uvRange.high.x,
                                        uvRange.low.y, uvRange.high.y)
                        )
            )
            {
            status = bspmesh1_meshBezierRegion (mpP, &points[0], (int)points.size (), myBezierP,
                                           uIndex, vIndex,
                uvRange.low.x, uvRange.high.x,
                uvRange.low.y, uvRange.high.y);
            }

        minLeftIndex  = maxLeftIndex;
        minRightIndex = maxRightIndex;
        minLeftPoint  = maxLeftPoint;
        minRightPoint = maxRightPoint;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurfaceRegion
(
BuilderParams          *mpP,                       /* => mesh parameters */
bvector<DPoint2d>    &boundary,                 /* = region boundary */
MSBsplineSurface    *surfaceP
)
    {
    int                 minIndex, maxIndex, nPnts, status, bufSize, j, next,
                        minLeftIndex, maxLeftIndex, minRightIndex, maxRightIndex, index,
                        pointBufSize;
    double              boundMin, boundMax;
    DPoint2d            *pnts, *pntP, *endP, minLeftPoint, maxLeftPoint,
                        minRightPoint, maxRightPoint, *pointP;
    
    pointP = NULL;
    bvector<DPoint2d*> slicePointers;
    bvector<DPoint2d> slicePoints;
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

    bufSize = 0;
    minLeftIndex = minRightIndex = minIndex;
    minLeftPoint = minRightPoint = pnts[minIndex];
    int uNumSegs, vNumSegs;
    SetupPatchesAndGridCountsInSurface (mpP, surfaceP, uNumSegs, vNumSegs);

    for (j=0, status=SUCCESS; j < vNumSegs && status==SUCCESS; j++)
        {
        DRange2d vSliceRange;
        mpP->GetVSliceRange (j, vSliceRange);
        double vMin = vSliceRange.low.y;
        double vMax = vSliceRange.high.y;
        maxLeftPoint.y = maxRightPoint.y = vMax = vSliceRange.high.y;
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

        slicePoints.clear ();
        slicePoints.push_back (maxLeftPoint);

        for (index=maxLeftIndex;
                 index != minLeftIndex;
                     index=NEXT(index, nPnts))
            slicePoints.push_back (pnts[index]);

        slicePoints.push_back (minLeftPoint);
        slicePoints.push_back (minRightPoint);

        for (index=NEXT(minRightIndex, nPnts);
                index != NEXT(maxRightIndex, nPnts);
                    index=NEXT(index, nPnts))
            slicePoints.push_back (pnts[index]);

        slicePoints.push_back (maxRightPoint);
        // MeshRegion::PrintXY ("into meshSurfaceSlice", j, slicePoints);
        status = meshSurfaceSlice (mpP, slicePoints, j, uNumSegs);

        minLeftIndex  = maxLeftIndex;
        minRightIndex = maxRightIndex;
        minLeftPoint  = maxLeftPoint;
        minRightPoint = maxRightPoint;
        vMin = vMax;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileAndCoveSurface
(
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *surfaceP                   /* => surface */
)
    {
    int                 status = SUCCESS;
    bvector<MeshRegion> sourceRegions, monotoneRegions;


    if (surfaceP->numBounds == 0 || ! surfaceP->holeOrigin)
        {
        sourceRegions.push_back (MeshRegion ());
        double uMin, uMax, vMin, vMax;
        surfaceP->GetParameterRegion (uMin, uMax, vMin, vMax);
        MeshRegion &outerRegion = sourceRegions.back ();
        outerRegion.boundary.   reserve (5);
        outerRegion.push_back (uMin,vMin);
        outerRegion.push_back (uMax,vMin);
        outerRegion.push_back (uMax,vMax);
        outerRegion.push_back (uMin,vMax);
        outerRegion.push_back (uMin,vMin);
        }

    if (surfaceP->numBounds)
        {
        for (int i=0; i<surfaceP->numBounds; i++)
            {
            // July 2019 -- caller code formerly killed boundaries if any were minimal points.
            // now ignore them here.
            if (surfaceP->boundaries[i].numPoints > 3)
                {
                sourceRegions.push_back (MeshRegion ());
                MeshRegion &newRegion = sourceRegions.back ();
                newRegion.push_back (surfaceP->boundaries[i]);
                }
            }
        }
    static double s_smallParameterChange = 4.0e-6;
    bspmesh_compressSmallDeltasInTrimCoordinates (sourceRegions, 0, s_smallParameterChange );
    bspmesh_compressSmallDeltasInTrimCoordinates(sourceRegions, 1, s_smallParameterChange);
    bspmesh_divideToMonotonic (sourceRegions, monotoneRegions);

    /* Note: keep going through this loop even after failure (to assure
            tail of list is freed) */
    for (size_t i = 0; i < monotoneRegions.size (); i++)
        {
        if (SUCCESS == status && 3 <= monotoneRegions[i].boundary.size ())
            status = meshSurfaceRegion (mpP, monotoneRegions[i].boundary, surfaceP);
        }
    return status;
    }


static void updateMax
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsppolyface_calculateParameterLengths
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
    double *weights = surface.rational ? surface.weights : NULL;
    if (surface.rational)
        {
        for (int j = 0; j < numV; j++)
            {
            updateMax (j,
                PolylineOps::Length (surface.poles + j * numU, weights + j * numU, 1, numU, closedU),
                        length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax (i,
                PolylineOps::Length (surface.poles + i, weights + i, numU, numV, closedV),
                length.y, maxIndexY);
            }
        }
    else
        {
        for (int j = 0; j < numV; j++)
            {
            updateMax (j,
                PolylineOps::Length (surface.poles + j * numU, NULL, 1, numU, closedU),
                        length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax (i,
                PolylineOps::Length (surface.poles + i, NULL, numU, numV, closedV),
                length.y, maxIndexY);
            }
        }
    }

static void bsppolyface_calculateParameterLengths
(
DPoint2d            *lengthP,
MSBsplineSurface    *surfaceP
)
    {
    int maxIndexI, maxIndexJ;
    bsppolyface_calculateParameterLengths (*surfaceP, *lengthP, maxIndexI, maxIndexJ);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int bspmesh_tileUnboundedSurface
(
BuilderParams          *mpP,
MSBsplineSurface    *surfaceP
)
    {
    int                 i, j, uSteps, vSteps, uNumSegs, vNumSegs;
    int uLoSteps, uHiSteps, vLoSteps, vHiSteps;
    DPoint2d            delta;
    static int s_parameterSelect = 0;

    if (!surfaceP->uParams.closed && !surfaceP->vParams.closed &&
         surfaceP->uParams.order == surfaceP->uParams.numPoles &&
         surfaceP->vParams.order == surfaceP->vParams.numPoles)

        {
        bspmesh_getSurfaceSteps (uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                 surfaceP, mpP);

        delta.x = 1.0 / (double) uSteps;
        delta.y = 1.0 / (double) vSteps;

        return tilePatchOfBezier (mpP, surfaceP, 0, uSteps, 0, vSteps, false,
                             &delta, 0.0, 1.0, 0.0, 1.0, &mpP->paramScale, 0 != mpP->reverse);
        }
    else
        {
        int status = SUCCESS;
        SetupPatchesAndGridCountsInSurface (mpP, surfaceP, uNumSegs, vNumSegs);

        for (j=0; j < vNumSegs; j++)
            for (i=0; i < uNumSegs; i++)
                {
                DRange2d uvRange;
                MSBsplineSurfaceP patchP = mpP->GetBezierPointer(i, j, uvRange);

                // abcde
                bspmesh_getSurfaceSteps (uSteps, vSteps, uLoSteps, uHiSteps, vLoSteps, vHiSteps,
                                         patchP, mpP);
                BezierGridCounts counts = mpP->bezierGridCounts[j][i];
                uLoSteps = counts.uCounts.numLow;
                uSteps = counts.uCounts.numInterior;
                uHiSteps = counts.uCounts.numHigh;

                vLoSteps = counts.vCounts.numLow;
                vSteps = counts.vCounts.numInterior;
                vHiSteps = counts.vCounts.numHigh;


                delta.x = 1.0 / (double) uSteps;
                delta.y = 1.0 / (double) vSteps;

                if (s_parameterSelect == 0)
                    {
                    if ((status = tilePatchOfBezier (mpP, patchP, 0, uSteps, 0, vSteps, false,
                                                &delta, 0.0, 1.0, 0.0, 1.0,
                                                &mpP->paramScale, 0 != mpP->reverse)) != SUCCESS)
                        break;
                    }
                else
                    {
                    if ((status = tilePatchOfBezier (mpP, patchP, 0, uSteps, 0, vSteps, false,
                                                &delta,
                                                uvRange.low.x, uvRange.high.x,
                                                uvRange.low.y, uvRange.high.y,
                                                &mpP->paramScale, 0 != mpP->reverse)) != SUCCESS)
                        break;
                    }
                }

        return status;
        }

    }
// Return true if all boundary coordinates are between -1 and 2.
// This allows "some" straying outside 0,1 but flags really strange things.
// (Is a fringe of 1 enough?)
bool validateBoundary (BsurfBoundary &boundary)
    {
    DRange1d range (-1.0, 2.0);
    static bool s_rejectSmallVertexCount = false;
    if (boundary.numPoints < 4)
        if (s_rejectSmallVertexCount)
            return false;
    for (int i = 0; i < boundary.numPoints; i++)
        if (!(range.Contains (boundary.points[i].x) && range.Contains (boundary.points[i].y)))
            return false;
    return true;
    }
bool validateBoundaries (BsurfBoundary   *boundsP, int numBounds)
    {
    for (int i = 0; i < numBounds; i++)
        {
        if (!validateBoundary (boundsP[i]))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurfaceToPolyface
(
MeshOutputHandlerR handler,
IFacetOptionsR      options,
TransformP                                  toleranceTransformP,        /* => tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* => tolerance camera position */
double                                      toleranceFocalLength,       /* => tolerance focal length */
DPoint2dP                                   parameterScale,             /* => parameter scale */
MSBsplineSurface                            *surfaceP,                  /* => surface to mesh */
bool                                        reverse,                    /* => true to reverse order */
bool                                        covePatchBoundaries        /* => true to cove patch bounds */
)
    {
    DPoint2d resolvedParamScale;
    auto maxPerFace = options.GetMaxPerFace ();
    auto curveMaxPerFace = options.GetCurvedSurfaceMaxPerFace ();
    if (curveMaxPerFace > 0 && curveMaxPerFace < maxPerFace)
        options.SetMaxPerFace(curveMaxPerFace);

    if (parameterScale)
        {
        DPoint2d    length;
    #if defined (BSPMESH_Use_Exact_for_length)
        bsppolyface_calculateParameterLengthsExact (&length, surfaceP);
#else
        bsppolyface_calculateParameterLengths (&length, surfaceP);
#endif
        resolvedParamScale.x = (parameterScale->x < 1.0E-8)  ?
                                   1.0 : length.x / parameterScale->x;
        resolvedParamScale.y = (parameterScale->y < 1.0E-8)  ?
                                   1.0 : length.y / parameterScale->y;
        }
    else
        {
        resolvedParamScale.x = resolvedParamScale.y = 1.0;
        }

    handler.SetParamScale (resolvedParamScale);
    BuilderParams  meshParams (handler, options,
            reverse,
            toleranceTransformP, toleranceCameraP, toleranceFocalLength,
            resolvedParamScale);


    static int alwaysCove = false;
    /* Turn this on to enable pre-fixup of boundaries */
    static int fixBoundaries = false;
    int status = ERROR;

    if ( alwaysCove )
        covePatchBoundaries = true;

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


    if (numBoundsInUse == 0 &&
       (! covePatchBoundaries || (surfaceP->uParams.order == surfaceP->uParams.numPoles &&
                                  surfaceP->vParams.order == surfaceP->vParams.numPoles)))
        status = bspmesh_tileUnboundedSurface (&meshParams, surfaceP);
    else
        status = bspmesh_tileAndCoveSurface (&meshParams, surfaceP);

    options.SetMaxPerFace (maxPerFace);
    return status;
    }




static bool s_firstTriangle012 = false;

struct BSurfMeshContext : MeshOutputHandler
{
// ASSUME !!! params sent to callbacks can be transformed in place.
IPolyfaceConstructionR m_builder;
Transform m_paramTransform;
BSurfMeshContext (IPolyfaceConstructionR builder, TransformCR paramTransform) :
    MeshOutputHandler (builder.GetFacetOptionsR ().GetParamsRequired (), builder.GetFacetOptionsR ().GetNormalsRequired ()),
    m_builder(builder), m_paramTransform (paramTransform)
    {}

StatusInt OutputQuadMesh (DPoint3dP points, DVec3dP normals, DPoint2dP params, int numColumns, int numRows) override
    {
    // YES there is an order switch in the row/column counts.
    if (NULL != params)
        m_paramTransform.Multiply (params, params, numRows * numColumns);
    m_builder.AddRowMajorQuadGrid (points, (DVec3dP)normals, params, (size_t)numColumns, (size_t)numRows);
    return SUCCESS;
    }

StatusInt OutputTriStrip (DPoint3dP points, DVec3dP normals, DPoint2dP params, int nPoints) override
    {
    if (NULL != params)
        m_paramTransform.Multiply (params, params, nPoints);
    m_builder.AddTriStrip (points, (DVec3dP)normals, params, (size_t)nPoints, s_firstTriangle012);
    return SUCCESS;
    }
};



bool TryTransformFromPseudoDistanceRectangle
(
FacetParamMode mode,
DRange2dCR baseRange,
double xDistanceFactor,
double yDistanceFactor,
DRange2dR distanceRange,
DRange2dR targetRange,
TransformR transform
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurface
(
MeshOutputHandlerR handler,
IFacetOptionsR      options,
TransformP                                  toleranceTransformP,        /* => tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* => tolerance camera position */
double                                      toleranceFocalLength,       /* => tolerance focal length */
DPoint2dP                                   parameterScale,             /* => parameter scale */
MSBsplineSurface*                           surfaceP,                   /* => surface to mesh */
bool                                        reverse,                    /* => true to reverse order */
bool                                        covePatchBoundaries        /* => true to cove patch bounds */
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
        double aSurfaceTol = mdlBspline_resolveSurfaceTolerance (surfaceP, options.GetChordTolerance (), sLocalRelTol, sGlobalRelTol);
        double aCurveTol = 0.01;
        bspsurf_openTempTrimmedSurface (&tempSurface, surfaceP, aCurveTol, aSurfaceTol);
        status = meshSurfaceToPolyface (handler, options,
                        toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength,
                        parameterScale,
                        &tempSurface,
                        reverse, covePatchBoundaries);
        bspsurf_closeTempTrimmedSurface (&tempSurface);
        return status;
        }
    else
        return meshSurfaceToPolyface (handler, options,
                        toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength,
                        parameterScale,
                        surfaceP,
                        reverse, covePatchBoundaries);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::Add (MSBsplineSurfaceCR surface)
    {
    static int s_addSmoothed = false;

    static double s_defaultVisibleEdgeAngle = 0.80;
    SynchOptions ();
    DPoint2d sectionLength;
    bspmesh_calculateParameterLengths (&sectionLength, const_cast <MSBsplineSurface*> (&surface));
    Transform surfaceUVToMeshUV;
    DRange2d surfaceUVRange = DRange2d::From (0,0,1,1);
    DRange2d distanceUVRange, targetUVRange;

    TryTransformFromPseudoDistanceRectangle (GetFacetOptionsR ().GetParamMode (), surfaceUVRange, sectionLength.x, sectionLength.y,
                                             distanceUVRange, targetUVRange, surfaceUVToMeshUV);

    if (s_addSmoothed || GetFacetOptionsR ().GetSmoothTriangleFlowRequired () || GetFacetOptionsR ().GetBSurfSmoothTriangleFlowRequired ())
        {
        AddSmoothed (surface);
        }
    else
        {
        BSurfMeshContext context (*this, surfaceUVToMeshUV);
        meshSurface (context, GetFacetOptionsR (), NULL, NULL, 0.0,
                                NULL,
                                const_cast <MSBsplineSurface*> (&surface),
                                false,
                                true);

        }
#ifdef CollectCounts
    size_t count0[7], count1[7], count2[7];
#endif
    PolyfaceHeaderPtr client = GetClientMeshPtr ();
#ifdef CollectCounts
    client->CollectCounts(
        count0[0], count0[1], count0[2], count0[3], count0[4], count0[5], count0[6]);
#endif

    static int s_doCompress = 1;
    static double s_compressFactor = 2.0;
    if (s_doCompress)
        client->Compress (s_compressFactor * DoubleOps::SmallMetricDistance ());
#ifdef CollectCounts
    client->CollectCounts(
        count1[0], count1[1], count1[2], count1[3], count1[4], count1[5], count1[6]);
#endif

    int hiding = GetFacetOptionsR ().GetBsplineSurfaceEdgeHiding ();
    static int s_forceHiding = 0;
    if (s_forceHiding == 1)
        client->MarkAllEdgesVisible();
    else if (s_forceHiding == 2)
        client->MarkTopologicalBoundariesVisible(false);
    else
        {
        if (hiding == 1)
            client->MarkInvisibleEdges (s_defaultVisibleEdgeAngle);
        else if(hiding == 2)
            client->MarkAllEdgesVisible();
        }

#ifdef CollectCounts
    client->CollectCounts(
        count2[0], count2[1], count2[2], count2[3], count2[4], count2[5], count2[6]);
#endif

    SetCurrentFaceParamDistanceRange (distanceUVRange);
    EndFace_internal ();
    }


Public bool bspsurf_closestPoint (MSBsplineSurfaceCR surface, DPoint3dCR spacePoint, SolidLocationDetailR detail)
    {
    BSurfPatch patch;
    size_t numU, numV;
    surface.GetIntervalCounts (numU, numV);
    SolidLocationDetail detail1;
    detail.Init ();
    detail.SetA (DBL_MAX);
    for (size_t i = 0; i < numU; i++)
        {
        for (size_t j = 0; j < numV; j++)
            {
            if (surface.GetPatch (patch, i, j))
                {
                }
            }
        }
    return false;          
    }

/*---------------------------------------------------------------------------------**//**
* Assumes there is a single loop in the graph to be corrected for self-intersections and crossings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspmesh_fixSingleBoundaryLoop
(
    VuSetP          graphP
)
    {
    VuMask          mVisited = vu_grabMask(graphP);

    /* Do this OR in case new boundaries self-intersect. */
    vu_stackPushCopy(graphP);
    vu_orLoops(graphP);

    /* Check that we shouldn't use XOR instead. */
    if (fabs(vu_area(vu_firstNodeInGraph(graphP))) > 0.999)
        {
        vu_stackExchange(graphP);
        vu_xorLoops(graphP);
        }

    vu_stackExchange(graphP);
    VU_SET_LOOP(P, graphP)
        vu_setMask(P, mVisited);
    END_VU_SET_LOOP(P, graphP)
        vu_freeMarkedEdges(graphP, mVisited);
    vu_stackPop(graphP);

    /* Fix "backward" loops created by crossings. */
    vu_clearMaskInSet(graphP, mVisited);
    VU_SET_LOOP(P, graphP)
        {
        if (!vu_getMask(P, mVisited))
            {
            vu_setMaskAroundFace(P, mVisited);
            vu_setMaskAroundFace(vu_edgeMate(P), mVisited);

            if (vu_area(P) > 0.0 && vu_getMask(P, VU_EXTERIOR_EDGE))
                {
                vu_setMaskAroundFace(vu_edgeMate(P), VU_EXTERIOR_EDGE);
                VU_FACE_LOOP(Q, P)
                    vu_clrMask(Q, VU_EXTERIOR_EDGE);
                END_VU_FACE_LOOP(Q, P)
                }
            }
        }
    END_VU_SET_LOOP(P, graphP)
        vu_returnMask(graphP, mVisited);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    BsurfBoundary   *bndP, *endP, *newBounds = NULL;
    static double s_unusualParameterSize = 10.0;

    if (0 == *numBoundsP || NULL == *boundsPP)
        return  SUCCESS;

    DRange2d range;
    DPoint2d localOrigin;
    range.Init();
    int maxPoints = 0;
    bool useLocalOrigin = false;
    for (int i = 0; i < *numBoundsP; i++)
        {
        range.Extend((*boundsPP)[i].points, (*boundsPP)[i].numPoints);
        if ((*boundsPP)[i].numPoints > maxPoints)
            maxPoints = (*boundsPP)[i].numPoints;
        }
    // heuristic: bspline surfaces have parameter loops in 0..1 range.
    // but this is called with uor data that has tolerance problems.
    // if the max coordinate of the range is larger than s_unusualParameterSize, use the range.low as origin.
    localOrigin.Zero();
    if (range.LargestCoordinate() > s_unusualParameterSize
        && !range.Contains(0.0, 0.0))
        {
        useLocalOrigin = true;
        localOrigin = range.low;
        }

    bvector<DPoint2d> localPoints;
    localPoints.reserve((size_t)maxPoints);


    graphP = vu_newVuSet(0);
    mVisited = vu_grabMask(graphP);
    faceArrayP = vu_grabArray(graphP);
    DPoint2d xy;
    // hmph.   vu_makeLoopFromArray has a clumsy 1e-8*largestCoordinate tolerance.
    // If input data is in a small box far from the origin it gets clumped.
    // So if in doubt move things back to a local origin.
    for (bndP = endP = *boundsPP, endP += *numBoundsP; bndP < endP; bndP++)
        {
        if (bndP->numPoints == 0)
            continue;
        vu_stackPush(graphP);
        if (useLocalOrigin)
            {
            localPoints.clear();
            for (int i = 0; i < bndP->numPoints; i++)
                {
                xy = bndP->points[i];
                xy.Subtract(localOrigin);
                localPoints.push_back(xy);
                }
            faceP = vu_makeLoopFromArray(graphP, &localPoints[0], bndP->numPoints, true, true);
            }
        else
            faceP = vu_makeLoopFromArray(graphP, bndP->points, bndP->numPoints, true, true);

        if (!conserveParity)
            bspmesh_fixSingleBoundaryLoop(graphP);
        }
    vu_stackPopAll(graphP);

    if (add01Box)
        {
        DPoint2d points[5];
        points[0].Init(0, 0);
        points[1].Init(1, 0);
        points[2].Init(1, 1);
        points[3].Init(0, 1);
        points[4].Init(0, 0);
        vu_makeLoopFromArray(graphP, points, 5, true, true);
        }

    if (conserveParity)
        vu_xorLoops(graphP);
    else
        vu_orLoops(graphP);

    vu_collectExteriorFaceLoops(faceArrayP, graphP);

    /* check for nested loops */
    for (vu_arrayOpen(faceArrayP); vu_arrayRead(faceArrayP, &faceP); )
        if (true == (nestedLoops = (vu_area(faceP) > 0.0)))
            break;

    if (conserveParity && nestedLoops)
        {
        vu_notLoops(graphP);
        vu_arrayClear(faceArrayP);
        vu_collectExteriorFaceLoops(faceArrayP, graphP);
        }

    // purge zero area faces....
    for (vu_arrayOpen(faceArrayP), face = 0; vu_arrayRead(faceArrayP, &faceP); face++)
        {
        if (vu_faceLoopSize(faceP) == 2)
            {
            vu_arrayRemoveCurrent(faceArrayP);
            }
        }
    newNumBounds = vu_arraySize(faceArrayP);

    if (newNumBounds)
        {
        if (NULL == (newBounds = (BsurfBoundary *)
            (BsurfBoundary*)BSIBaseGeom::Malloc(newNumBounds * sizeof(BsurfBoundary))))
            {
            status = ERROR;
            }
        else
            {
            /*    For each face loop .. */
            memset(newBounds, 0, newNumBounds * sizeof(BsurfBoundary));
            for (vu_arrayOpen(faceArrayP), face = 0; vu_arrayRead(faceArrayP, &faceP); face++)
                {
                /* Count number of points in this face loop.
                    Start at 1 so that line string will be closed.
                    Also get the minimum point in loop. */
                newBounds[face].numPoints = 1;
                minP = faceP;
                VU_FACE_LOOP(P, faceP)
                    {
                    newBounds[face].numPoints += 1;
                    if (vu_below(P, minP))
                        minP = P;
                    }
                END_VU_FACE_LOOP(P, faceP)

                    if (NULL == (newBounds[face].points =
                        (DPoint2d *)BSIBaseGeom::Malloc(newBounds[face].numPoints
                            * sizeof(DPoint2d))))
                        {
                        status = ERROR;
                        break;
                        }

                /* Assign points */
                loopP = newBounds[face].points;
                VU_FACE_LOOP(P, minP)
                    loopP->x = vu_getX(P) + localOrigin.x;
                loopP->y = vu_getY(P) + localOrigin.y;
                loopP++;
                END_VU_FACE_LOOP(P, minP)
                    *loopP = newBounds[face].points[0];
                }

            bsputil_freeBoundary(boundsPP, *numBoundsP);

            *boundsPP = newBounds;
            *numBoundsP = newNumBounds;
            status = SUCCESS;
            }
        }

    vu_returnMask(graphP, mVisited);
    vu_returnArray(graphP, faceArrayP);
    vu_freeVuSet(graphP);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_fixBoundaries
(
    BsurfBoundary   **boundsPP,
    int             *numBoundsP,
    bool            conserveParity
)
    {
    return bspmesh_fixBoundaries(boundsPP, numBoundsP, conserveParity, false);
    }
void unweight(DPoint3dCR xyzw, double w, DPoint3dR xyz)
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

double SumWeightedLengths(DPoint3d const *xyz, double const *weight, int i0, int step, int n, bool wrap)
    {
    double d0 = PolylineOps::Length(xyz + i0, weight + i0, step, n, wrap);
    DPoint3d xyz0, xyz1;
    unweight(xyz[i0], weight[i0], xyz0);
    DPoint3d xyzWrap = xyz0;
    double d = 0.0;
    i0 += step;
    for (int i = 1; i < n; i++, i0 += step, xyz0 = xyz1)
        {
        unweight(xyz[i0], weight[i0], xyz1);
        d += xyz0.Distance(xyz1);
        }
    if (wrap)
        d += xyz0.Distance(xyzWrap);
    return d > 0.0 ? d : d0;
    }

double SumLengths(DPoint3d const *xyz, int i0, int step, int n, bool wrap)
    {
    double d0 = PolylineOps::Length(xyz + i0, NULL, step, n, wrap);
    DPoint3d xyz0 = xyz[i0], xyz1;
    DPoint3d xyzWrap = xyz0;
    i0 += step;
    double d = 0.0;
    for (int i = 1; i < n; i++, i0 += step, xyz0 = xyz1)
        {
        xyz1 = xyz[i0];
        d += xyz0.Distance(xyz1);
        }
    if (wrap)
        d += xyz0.Distance(xyzWrap);
    return d > 0.0 ? d : d0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspmesh_calculateParameterLengths
(
    MSBsplineSurfaceCR surface,
    DPoint2dR          length,
    int &maxIndexX, // index where length.x occurred.
    int &maxIndexY  // index where length.y occurred.
)
    {
    length.Zero();
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
            updateMax(j, SumWeightedLengths(surface.poles, surface.weights,
                j * numU, 1, numU, closedU), length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax(i, SumWeightedLengths(surface.poles, surface.weights,
                i, numU, numV, closedV), length.y, maxIndexY);
            }
        }
    else
        {
        for (int j = 0; j < numV; j++)
            {
            updateMax(j, SumLengths(surface.poles, j * numU, 1, numU, closedU),
                length.x, maxIndexX);
            }
        for (int i = 0; i < numU; i++)
            {
            updateMax(i, SumLengths(surface.poles, i, numU, numV, closedV), length.y, maxIndexY);
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
    bspmesh_calculateParameterLengths(*surfaceP, *lengthP, maxIndexI, maxIndexJ);
    }

static double s_minEdgeFraction = 1.0e-5;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    uLength.clear();
    uTurn.clear();
    uLength.clear();
    vTurn.clear();

    int numU = surface.uParams.numPoles;
    int numV = surface.vParams.numPoles;

    bool closedU = surface.uParams.closed != 0;
    bool closedV = surface.vParams.closed != 0;
    if (surface.rational)
        {
        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uLength.push_back(PolylineOps::Length(surface.poles + jStart, surface.weights + jStart, 1, numU, closedU));
            }
        for (int i = 0; i < numU; i++)
            {
            vLength.push_back(PolylineOps::Length(surface.poles + i, surface.weights + i, numU, numV, closedV));
            }
        double minEdge = s_minEdgeFraction * (DoubleOps::Sum(uLength) + DoubleOps::Sum(vLength));


        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uTurn.push_back(PolylineOps::SumAbsoluteAngles(surface.poles + jStart, surface.weights + jStart, 1, numU, closedU, minEdge));
            }
        for (int i = 0; i < numU; i++)
            {
            vTurn.push_back(PolylineOps::SumAbsoluteAngles(surface.poles + i, surface.weights + i, numU, numV, closedV, minEdge));
            }
        }
    else
        {
        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uLength.push_back(PolylineOps::Length(surface.poles + jStart, NULL, 1, numU, closedU));
            }
        for (int i = 0; i < numU; i++)
            {
            vLength.push_back(PolylineOps::Length(surface.poles + i, NULL, numU, numV, closedV));
            }
        double minEdge = s_minEdgeFraction * (DoubleOps::Sum(uLength) + DoubleOps::Sum(vLength));

        for (int j = 0; j < numV; j++)
            {
            size_t jStart = j * numU;
            uTurn.push_back(PolylineOps::SumAbsoluteAngles(surface.poles + jStart, NULL, 1, numU, closedU, minEdge));
            }
        for (int i = 0; i < numU; i++)
            {
            vTurn.push_back(PolylineOps::SumAbsoluteAngles(surface.poles + i, NULL, numU, numV, closedV, minEdge));
            }


        }
    }
struct BilinearBasisValues
    {
    double s00, s10, s01, s11;
    BilinearBasisValues()
        {
        s00 = s10 = s01 = s11 = 0.0;
        }
    BilinearBasisValues(double fx1, double fy1)
        {
        double fx0 = 1.0 - fx1;
        double fy0 = 1.0 - fy1;
        s00 = fx0 * fy0;
        s10 = fx1 * fy0;
        s01 = fx0 * fy1;
        s11 = fx1 * fy1;
        }
    void Evaluate(DPoint2dR result, DPoint2dCR point00, DPoint2dCR point10, DPoint2dCR point01, DPoint2dCR point11)
        {
        result.x = s00 * point00.x + s10 * point10.x + s01 * point01.x + s11 * point11.x;
        result.y = s00 * point00.y + s10 * point10.y + s01 * point01.y + s11 * point11.y;
        }
    };
#define MAX_XY_QUADRATURE_POINTS 25
struct SurfacePropertiesContext : MeshOutputHandler
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

void BuildTriangleGauss(int selector)
    {
    m_triangleRule.InitStrang(selector);
    }

void BuildQuadGauss(int numX, int numY)
    {
    m_xRule.InitGauss(numX);
    m_yRule.InitGauss(numY);
    m_numQuadGauss = 0;
    for (int i = 0; i < m_xRule.GetNumEval(); i++)
        for (int j = 0; j < m_yRule.GetNumEval(); j++)
            {
            BSIQuadraturePoints::GetXYEval(
                m_xRule, i, 0.0, 1.0,
                m_yRule, j, 0.0, 1.0,
                m_quadXY01[m_numQuadGauss].x,
                m_quadXY01[m_numQuadGauss].y,
                m_quadWeight01[m_numQuadGauss]
            );
            m_quadBasisValues[m_numQuadGauss] = BilinearBasisValues(
                m_quadXY01[m_numQuadGauss].x,
                m_quadXY01[m_numQuadGauss].y
            );
            m_numQuadGauss++;
            }
    }

SurfacePropertiesContext(MSBsplineSurfaceCR surface, bool needParams, bool needNormals, int numGauss)
    : MeshOutputHandler(needParams, needNormals),  m_surface(surface)
    {
    BuildQuadGauss(numGauss, numGauss);
    BuildTriangleGauss(6);  // Ask for Strang rule 7 -- 7 points, precise for polynomial degree 5.
    m_products = DMatrix4d::FromZero();
    m_parameterSpaceArea = 0.0;
    m_numEval = 0;
    }
// Accumulate w * products ....
void AccumulateSurfaceProducts(double u, double v, double w)
    {
    DVec3d dU, dV, cross;
    DPoint3d xyz;
    m_surface.EvaluatePoint(xyz, dU, dV, u, v);
    m_numEval += m_numQuadGauss;
    cross.CrossProduct(dU, dV);
    double dA = w * cross.Magnitude();
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

void AccumulateTriangle(DPoint2dCR uv00, DPoint2dCR uv10, DPoint2dCR uv01)
    {
    DVec2d U, V;
    U.DifferenceOf(uv10, uv00);
    V.DifferenceOf(uv01, uv00);
    double detJ = U.CrossProduct(V);
    DPoint2d uv;
    double u, v, w;
    int numPoints = m_triangleRule.GetNumEval();
    m_parameterSpaceArea += detJ * 0.5;
    // detJ is the local scale factor at any point.
    // the triangle integration weights will add to 0.5 so integrals are for triangle -- no factor of 0.5 on detJ!!!
    for (int i = 0; i < numPoints; i++)
        {
        m_triangleRule.GetEval(i, u, v, w);
        uv.SumOf(uv00, U, u, V, v);
        AccumulateSurfaceProducts(uv.x, uv.y, w * detJ);
        }
    }

// ASSUME uv area on a square !!!!
void AccumulateQuad(DPoint2dCR uv00, DPoint2dCR uv10, DPoint2dCR uv01, DPoint2dCR uv11)
    {
    DPoint2d uv;
    double du = uv11.x - uv00.x;
    double dv = uv11.y - uv00.y;
    double dudv = du * dv;  // ASSUME RECTANGULAR QUAD -- otherwise this needs detJ effect depending on evaluation point !!
    m_parameterSpaceArea += dudv;
    for (int i = 0; i < m_numQuadGauss; i++)
        {
        m_quadBasisValues[i].Evaluate(uv, uv00, uv10, uv01, uv11);
        AccumulateSurfaceProducts(uv.x, uv.y, m_quadWeight01[i] * dudv);
        }
    }

StatusInt OutputQuadMesh(DPoint3dP points, DVec3dP normals, DPoint2dP params, int numColumns, int numRows) override
    {

    for (int j = 1; j < numRows; j++)
        for (int i = 1; i < numColumns; i++)
            {
            int k11 = i + j * numColumns;
            int k01 = k11 - 1;
            int k10 = k11 - numColumns;
            int k00 = k10 - 1;
            AccumulateQuad(
                params[k00], params[k10], params[k01], params[k11]);

            }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Callback to receive triangle strips during mesh range calculation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt OutputTriStrip(DPoint3dP points, DVec3dP normals, DPoint2dP params, int nPoints) override
    {
    for (int i = 2; i < nPoints; i += 1)
        {
        if ((i & 0x01) == 0)    // EVEN
            {
            AccumulateTriangle(params[i - 1], params[i], params[i - 2]);
            }
        else
            {
            AccumulateTriangle(params[i - 2], params[i], params[i - 1]);
            }
        }
    return SUCCESS;
    }
};
/*
virtual StatusInt OutputQuadMesh(DPoint3dP points, DVec3dP normals, DPoint2dP params, int nu, int nv) { return SUCCESS; }

virtual StatusInt OutputTriStrip(DPoint3dP points, DVec3dP normals, DPoint2dP params, int numPoints) { return SUCCESS; }
*/
bool MSBsplineSurface::ComputePrincipalAreaMoments
(
double &area,
DVec3dR centroid,
RotMatrixR axes,
DVec3dR momentxyz
) const
    {
    DMatrix4d products;
    bool stat = ComputeSecondMomentAreaProducts(products);
    Transform localToWorld;
    localToWorld.InitIdentity();
    products.ConvertInertiaProductsToPrincipalAreaMoments(localToWorld, area, centroid, axes, momentxyz);
    return stat;
    }

bool MSBsplineSurface::ComputeSecondMomentAreaProducts(DMatrix4dR products, double relativeTolerancefForFacets, int numGauss, int &numEvaluations) const
    {
    SurfacePropertiesContext context(*this, true, false, numGauss);
    DRange3d range;
    double tolerance = 5.0;
    static bool cove = true;
    auto options = IFacetOptions::Create();
    options->SetParamsRequired(true);
    GetPoleRange(range);
    options->SetChordTolerance(tolerance = relativeTolerancefForFacets * range.low.Distance(range.high));

    if (SUCCESS == meshSurface
        (
        context,
        *options,
        NULL,
        NULL,
        0.0,
        NULL,
        const_cast<MSBsplineSurface *> (this),
        false,
        cove
        ))
        {
        products = context.m_products;
        numEvaluations = context.m_numEval;
        return true;
        }
    products = DMatrix4d::FromZero();
    return false;
    }

bool MSBsplineSurface::ComputeSecondMomentAreaProducts(DMatrix4dR products) const
    {
    int numEval = 0;
    return MSBsplineSurface::ComputeSecondMomentAreaProducts(products, 0.001, 4, numEval);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
