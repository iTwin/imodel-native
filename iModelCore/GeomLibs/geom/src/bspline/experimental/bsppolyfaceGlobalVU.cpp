/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include <Geom/IntegerTypes/Point.h>
#include "msbsplinemaster.h"

#include "GridArrays.cpp"

BEGIN_BENTLEY_NAMESPACE

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
void CheckZero (double a)
    {
    }
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

/*----------------------------------------------------------------------+
|                                                                       |
|   Routines for evaluating Bezier Points/Normals                       |
|                                                                       |
+----------------------------------------------------------------------*/

static void bspmesh_splitEdge
(
VuSetP graphP,          /* <=> Graph where edges are added */
VuP    startP,          /* => start node on edge */
int    steps            /* => number of edges after split. i.e. 1 is noop. */
);



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     12/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshOutputHandler
{
virtual ~MeshOutputHandler (){}
virtual StatusInt OutputQuadMesh (DPoint3dP points, DVec3dP normals, DPoint2dP params, int nu, int nv)
    {
    return SUCCESS;
    }
    
virtual StatusInt OutputTriStrip (DPoint3dP points, DVec3dP normals, DPoint2dP params, int numPoints) {return SUCCESS;}


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
    
virtual StatusInt OutputQuadMeshFromArrays (int nu, int nv)
    {
    if (nu * nv != m_points.size ())
        return ERROR;
    return OutputQuadMesh (GetPointP (), GetNormalP (), GetParamP (), nu, nv);    
    }

virtual StatusInt OutputTriStripFromArrays ()
    {
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
DirectionalGridCounts ()
    : m_range (0.0, 1.0)
    {
    numLow = 0;
    numInterior = 0;
    numHigh = 0;
    }

int numLow;
int numInterior;
int numHigh;
DSegment1d m_range;
void Get (int &interior, int &low, int &high)
    {
    interior = numInterior;
    low = numLow;
    high = numHigh;
    }
double Param0 (){ return m_range.GetStart ();}
double Param1 (){ return m_range.GetEnd ();}
DSegment1d Params () { return m_range;}
void SetParams (double a0, double a1){ m_range = DSegment1d (a0, a1);}
};

struct BezierGridCounts
{
BezierGridCounts ()
    {

    lowerLeft = lowerRight = upperRight = upperLeft = NULL;
    }
DirectionalGridCounts uCounts;
DirectionalGridCounts vCounts;
VuP lowerLeft, lowerRight, upperRight, upperLeft;
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

    bool ParamsRequired (){return parametersRequired;}

    bvector<DPoint2d>   outputUV;
    bvector<DPoint2d*>  outputPointer;
    VuSetP m_graph;
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
        m_graph (NULL)
        {
        }

    ~BuilderParams ()
        {
        if (NULL != m_graph)
          vu_freeVuSet (m_graph);
        }
        
    VuSetP GrabGraph ()
        {
        if (NULL == m_graph)
            m_graph = vu_newVuSet (0);
        return m_graph;
        }

    VuSetP PeekGraph ()
        {
        return m_graph;
        }

    void DropGraph (VuSetP graph)
        {
        vu_reinitializeVuSet (graph);
        m_graph = graph;
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


static int bspmesh_triangulateStrip
(
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *bezierP,                   /* => bezier prepped surface */
bvector<DPoint2d>   &uvPP,                     /* => point coordinates */
int                 iMin,                       /* => index of start of strip */
int                 direction,                  /* => 0 for vertical sweep, 1 for horizontal */
double              uMin,                       /* => u Minimum */
double              uMax,                       /* => u Maximum */
double              vMin,                       /* => v Minimum */
double              vMax                        /* => v Maximum */
);

static int s_maxStepsInPatchSubdivision = 500;




/*----------------------------------------------------------------------+
|                                                                       |
|   Utility routines                                                    |
|                                                                       |
+----------------------------------------------------------------------*/

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
    bsiDRange3d_initFromArray (&range, pXYZ, numXYZ);
    bsiDVec3d_subtractDPoint3dDPoint3d (&diagonal, &range.low, &range.high);
    maxAbs = bsiDRange3d_getLargestCoordinate (&range);
    maxDiagonal = bsiDVec3d_maxAbs (&diagonal);

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


/*----------------------------------------------------------------------+
|                                                                       |
|   Routines for evaluating steps sizes                                 |
|                                                                       |
+----------------------------------------------------------------------*/

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
            bsiDPoint3d_interpolate (&point0, &row0P[i-1], 0.5, &row1P[i]);
            bsiDPoint3d_interpolate (&point1, &row0P[i], 0.5, &row1P[i-1]);
            currDiff = bsiDPoint3d_distanceSquared (&point0, &point1);
            d0 = bsiDPoint3d_distanceSquared (&row0P[i-1], &row1P[i]);
            d1 = bsiDPoint3d_distanceSquared (&row0P[i], &row1P[i-1]);
            d2 = d0 > d1 ? d0 : d1;
            if (bsiTrig_safeDivide (&ratio, currDiff, d2, 0.0)
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

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vec01, pPoint1, pPoint0);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vec02, pPoint2, pPoint0);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vec12, pPoint2, pPoint1);

    if (mpP->toleranceMode & STROKETOL_XYProjection)
        {
        mag12   = bsiDPoint3d_magnitudeXY (&vec12);
        mag01   = bsiDPoint3d_magnitudeXY (&vec01);
        mag02   = bsiDPoint3d_magnitudeXY (&vec02);
        cross   = fabs (bsiDPoint3d_crossProductXY (&vec01, &vec02));
        }
    else
        {
        mag12   = bsiDPoint3d_magnitude (&vec12);
        mag01   = bsiDPoint3d_magnitude (&vec01);
        mag02   = bsiDPoint3d_magnitude (&vec02);
        bsiDPoint3d_crossProduct (&crossProduct, &vec01, &vec02);
        cross       = bsiDPoint3d_magnitude(&crossProduct);
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
                theta = bsiDPoint3d_angleBetweenVectorsXY (&vec01, &vec02);
                }
            else
                {
                theta = bsiDPoint3d_angleBetweenVectors (&vec01, &vec02);
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
    count = degree;
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
* @bsimethod                                                    Lu.Han          08/92
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
    // EDL June 2013.  diagaonal count is being passed to edges.  Why?  This creates mismatches
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
        mdlTMatrix_transformPointArray (poles, mpP->toleranceTransformP, numPoles);

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
    /* Find uNum */
    if (uOrder > 2)
        {
        maxDiff = maxLoDiff = maxHiDiff = 0.0;
        maxAngle = maxLoAngle = maxHiAngle = 0.0;
        polygonLength0 = polygonLength1 = polygonLength2 = 0.0;
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
        polygonLength0 = polygonLength1 = polygonLength2 = 0.0;
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
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
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

void BuildSplitEdge
(
VuSetP graph,
DPoint2d xy0,
DPoint2d xy1,
int n,
VuP &nodeA,
VuP &nodeB
)
    {
    vu_makePair (graph, &nodeA, &nodeB);
    vu_setDPoint2d (nodeA, &xy0);
    vu_setDPoint2d (nodeB, &xy1);
    bspmesh_splitEdge (graph, nodeA, n);
    }

void BuildSplitEdge
(
VuSetP graph,
DPoint3d xy0,
DPoint3d xy1,
int n,
VuP &nodeA,
VuP &nodeB
)
    {
    vu_makePair (graph, &nodeA, &nodeB);
    vu_setDPoint3d (nodeA, &xy0);
    vu_setDPoint3d (nodeB, &xy1);
    bspmesh_splitEdge (graph, nodeA, n);
    }
// ASSUME
// 1) mpP vuGraph read for new edges.
// 2) bezier patches precomputed.
// 3) mpP arrays have edge counts and parameter ranges.
void BuildPatchBoundaryVUEdges
(
BuilderParams *mpP,
int uIndex,
int vIndex
)
    {
    BezierGridCounts &patchData = mpP->bezierGridCounts [vIndex][uIndex];
    int vCount = (int)mpP->bezierGridCounts.size ();
    int uCount = (int)mpP->bezierGridCounts[0].size ();
    double u0 = patchData.uCounts.Param0 ();
    double u1 = patchData.uCounts.Param1 ();
    double v0 = patchData.vCounts.Param0 ();
    double v1 = patchData.vCounts.Param1 ();
    VuSetP graph = mpP->PeekGraph ();
    // Always build lower and left edges.  nodeA is always a corner "inside" this patch,
    // nodeB is the opposite end and on the outside.

    VuP nodeB;
    BuildSplitEdge (graph, DPoint2d::From (u0, v0), DPoint2d::From (u1, v0), patchData.uCounts.numLow, patchData.lowerLeft, nodeB);

    if (vIndex > 0)
        mpP->bezierGridCounts[vIndex - 1][uIndex].upperRight = nodeB;
    BuildSplitEdge (graph, DPoint2d::From (u0, v1), DPoint2d::From (u0, v0), patchData.vCounts.numLow, patchData.upperRight, nodeB);
    if (uIndex > 0)
        mpP->bezierGridCounts[vIndex][uIndex - 1].upperRight = nodeB;

    // Only build upper and right edges at final block
    if (vIndex + 1 == vCount)
        BuildSplitEdge (graph, DPoint2d::From (u1, v1), DPoint2d::From (u0, v1), patchData.vCounts.numHigh, patchData.upperRight, nodeB);
    if (uIndex + 1 == uCount)
        BuildSplitEdge (graph, DPoint2d::From (u1, v0), DPoint2d::From (u1, v1), patchData.uCounts.numHigh, patchData.lowerRight, nodeB);

    }

// ASSUME
// 1) Each patch has identified corners
void BuildPatchInteriorVUEdges
(
BuilderParams *mpP,
int uIndex,
int vIndex
)
    {
    BezierGridCounts &patchData = mpP->bezierGridCounts [vIndex][uIndex];
    // global uv in the graph
    double u0 = patchData.uCounts.Param0 ();
    double u1 = patchData.uCounts.Param1 ();
    double v0 = patchData.vCounts.Param0 ();
    double v1 = patchData.vCounts.Param1 ();
    DBilinearPatch3d patch (DPoint3d::From (u0, v0), DPoint3d::From (u1, v0), DPoint3d::From (u0, v1), DPoint3d::From (u1, v1));
    int numU = (int)patchData.uCounts.numInterior;
    int numV = (int)patchData.vCounts.numInterior;
    VuSetP graph = mpP->PeekGraph ();
    // fractional coordinates of the (internal) grid corners.
    double fuA = 1.0 / (double)numU;
    double fvA = 1.0 / (double)numV;
    double fuB = 1.0 - fuA;
    double fvB = 1.0 - fvA;
    // We build a "complete" grid in the strict interior.
    // Along each fringe the connection to the patch boundary is complicated, so it is deferred until later.
    VuP nodeA, nodeB, nodeC0, nodeC1, nodeD0, nodeD1, nodeE0, nodeE1, nodeF0, nodeF1;
    if (numU == 2 && numV == 2)
        {
        vu_join (graph, patchData.lowerLeft, NULL, &nodeC0, &nodeC1);
        DPoint3d xyz = patch.Evaluate (fuA, fvA);
        vu_setDPoint3d (nodeC1, &xyz);
        vu_join (graph, patchData.lowerRight, nodeC1, &nodeD0, &nodeD1);
        vu_join (graph, patchData.upperRight, nodeD1, &nodeE0, &nodeE1);
        vu_join (graph, patchData.upperLeft, nodeE1, &nodeF0, &nodeF1);
        }
    else if (numU == 2 && numV > 2)
        {
        // Chain runs vertically.
        BuildSplitEdge (graph, patch.Evaluate (fuA, fvA), patch.Evaluate (fuA, fvB), numV, nodeA, nodeB);
        vu_join (graph, patchData.lowerLeft, nodeA, &nodeC0, &nodeC1);
        vu_join (graph, patchData.upperRight, nodeB, &nodeD0, &nodeD1);
        vu_join (graph, patchData.lowerRight, nodeC1, &nodeE0, &nodeE1);
        vu_join (graph, patchData.upperLeft, nodeD1, &nodeF0, &nodeF1);
        }
    else if (numU > 2 && numV == 2)
        {
        BuildSplitEdge (graph, patch.Evaluate (fuA, fvA), patch.Evaluate (fuB, fvA), numU, nodeA, nodeB);
        vu_join (graph, patchData.upperLeft, nodeA, &nodeC0, &nodeC1);
        vu_join (graph, patchData.lowerRight, nodeB, &nodeD0, &nodeD1);
        vu_join (graph, patchData.lowerLeft, nodeC1, &nodeE0, &nodeE1);
        vu_join (graph, patchData.upperRight, nodeD1, &nodeF0, &nodeF1);
        }
    else if (numU > 2 && numV > 2)
        {
        VuP nodeA0, nodeA1, nodeB0, nodeB1;   // 0 is lower current chain, 1 is upper
        // Top two chains awaiting insertion of an interior strut from C0 to the the lower side of C1
        //          A1                   C1
        //         +---------+----------+--------+--------+
        //         |         |                          B1
        //         |A0       |           C0
        //         +---------+----------+--------+--------+
        //                                              B0
        //
        BuildSplitEdge (graph, patch.Evaluate (fuA, fvA), patch.Evaluate (fuB, fvA), numU, nodeA0, nodeB0);
        vu_join (graph, patchData.lowerLeft,  nodeA0, &nodeC0, &nodeC1);
        vu_join (graph, patchData.lowerRight, nodeB0, &nodeD0, &nodeD1);
        // join pairs vertically
        for (int j = 2; j < numV; j++, nodeA0 = nodeA1, nodeB0 = nodeB1)
            {
            BuildSplitEdge (graph, patch.Evaluate (fuA, j * fvA), patch.Evaluate (fuB, j * fvA), numU, nodeA1, nodeB1);
            nodeC0 = nodeA0;   // moves along TOP of upper chain
            nodeC1 = nodeA1;   // moves along TOP of lower chain
            for (int i = 1; i < numU; i++, nodeC0 = vu_fsucc(nodeC0), nodeC1 = vu_fsucc(nodeC1))
                {
                vu_join (graph, nodeC0, vu_vsucc (nodeC0), &nodeD0, &nodeD1);
                }
            }
      // and the final A0,B0 chain is exposed at top for corner links (but B0 is INSIDE)
        vu_join (graph, patchData.upperLeft, nodeA0, &nodeC0, &nodeC1);
        vu_join (graph, patchData.upperRight, vu_vsucc (nodeB0), &nodeD0, &nodeD1);
        }
    }


void SetupGridCountsInPreparedSurface
(
BuilderParams *mpP,
MSBsplineSurfaceP surface,
MSBsplineSurfaceP bezier,
int *uStart,
int *vStart,
int uNumSegs,
int vNumSegs
)
    {
    mpP->bezierGridCounts.clear ();
    for (int j=0; j < vNumSegs; j++)
        {
        mpP->bezierGridCounts.push_back (bvector<BezierGridCounts> ());
        for (int i=0; i < uNumSegs; i++)
            {
            bsprsurf_netPoles (bezier->poles, bezier->weights, uStart[i], vStart[j], surface);
            BezierGridCounts counts;
            bspmesh_getSurfaceSteps (
                  counts.uCounts.numInterior, counts.vCounts.numInterior,
                  counts.uCounts.numLow, counts.uCounts.numHigh,
                  counts.vCounts.numLow, counts.vCounts.numHigh,
                                      bezier, mpP);
            counts.uCounts.SetParams (bezier->uKnots[uStart[i]-1], bezier->uKnots[uStart[i]]);
            counts.vCounts.SetParams (bezier->vKnots[vStart[j]-1], bezier->vKnots[vStart[j]]);
            mpP->bezierGridCounts.back ().push_back (counts);
                
            }
        }

    bool uClosed, vClosed;
    surface->IsPhysicallyClosed (uClosed, vClosed);
    uClosed |= surface->uParams.closed != 0;
    vClosed |= surface->vParams.closed != 0;
    mpP->CorrectGridCounts (uClosed, vClosed);
    }




void AddEvaluatedVuToPolyface (VuSetP graph, IPolyfaceConstructionR builder, MSBsplineSurfaceCR surface);


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




static void ApplyTrimBoundariesToGlobalVUGraph
(
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    &surface                  /* => bezier prepped surface */
)
    {
    bvector <bvector<DPoint2d> > uvBoundaries;
    surface.GetUVBoundaryLoops (uvBoundaries, true);
    VuSetP graph = mpP->PeekGraph ();
    for (size_t i = 0; i < uvBoundaries.size (); i++)
        {
        VuP loop = vu_makeLoopFromArray(graph, &uvBoundaries[i][0], (int)uvBoundaries[i].size (), false,false);
        vu_setMask (loop, VU_BOUNDARY_EDGE);
        vu_setMask (vu_vsucc (loop), VU_BOUNDARY_EDGE);
        }
    // ISSUE -- old code selectively rotated (within each bezier) 90 degrees to get favorable regularization. 
    // ISSUE -- parameter scaling ?

    // We want the smaller subdivision steps to be going vertically
    // This makes vertical sweep encouter new points by the
    // primary (y direction) sort key rather than the
    // secondar (x direction) lexical sort key
    //bool    bRotateForTriangulation = uSteps > vSteps;
    bool bRotateForTriangulation = false;

    if (bRotateForTriangulation)
        vu_rotate90CCW (graph);
    /* fixup crossing loops (there shouldn't be any!!) */
    vu_mergeLoops( graph );
    /* insert additional edges so each face is regular in the y direction,
        i.e. has a single min point, edges that go continuously up to a
        single max point, and back
    */
    vu_regularizeGraph( graph );

    /* Straighten out the inside/outside markings */
    vu_markAlternatingExteriorBoundaries(graph, true);
    /* Stick in the triangles */
    vu_triangulateMonotoneInteriorFaces(graph, false);
    if (bRotateForTriangulation)
        vu_rotate90CW (graph);

#ifdef FLIP_SCALES_KNOWN
    if (bezierScale.x != 0.0 && bezierScale.y != 0.0)
        vu_flipTrianglesToImproveScaledQuadraticAspectRatio(graph,
                            xScale / bezierScale.x,
                            yScale / bezierScale.y
                            );
    else
        vu_flipTrianglesToImproveScaledQuadraticAspectRatio(graph,
                            xScale,
                            yScale
                            );
#else
    vu_flipTrianglesToImproveQuadraticAspectRatio(graph);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int BuildGlobalVUGraph
(
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface    *surfaceP,                  /* => bezier prepped surface */
int                 uNumSegs,                   /* => number of U segments */
int                 vNumSegs,                   /* => number of V segments */
int                 *uStartP,                   /* => starting offsets of U segments */
int                 *vStartP                    /* => starting offsets of U segments */
)
    {
    MSBsplineSurface    bezier;
    bezier.Zero ();
    
    bezier.poles = NULL;
    bezier.weights = NULL;
    StatusInt status = ERROR;
    if (SUCCESS == (status = bspproc_initializeBezierPatch (&bezier, surfaceP)))
        {
        SetupGridCountsInPreparedSurface (mpP, surfaceP, &bezier, uStartP, vStartP, uNumSegs, vNumSegs);

        for (int j = 0; j < vNumSegs; j++)
            {
            for (int i = 0; i < uNumSegs; i++)
                {
                BuildPatchBoundaryVUEdges (mpP, i, j);
                }
            }

        for (int j = 0; j < vNumSegs; j++)
            {
            for (int i = 0; i < uNumSegs; i++)
                {
                BuildPatchInteriorVUEdges (mpP, i, j);
                }
            }
        }
    if (bezier.poles)       msbspline_free (bezier.poles);
    if (bezier.weights)     msbspline_free (bezier.weights);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt MeshAndOutput
(
IPolyfaceConstructionR builder,
BuilderParams          *mpP,                       /* => mesh parameters */
MSBsplineSurface       surface                   /* => surface */
)
    {
    MSBsplineSurface    preppedSurface;
    StatusInt status = SUCCESS;
    preppedSurface.Zero ();

    int * uStart = NULL;
    int * vStart = NULL;
    int uNumSegs, vNumSegs;
    if (SUCCESS == (status = bspproc_prepareSurface (&preppedSurface, &uNumSegs, &vNumSegs,
                                         &uStart, &vStart, &surface)))
        {
        VuSetP graph = mpP->GrabGraph ();
        BuildGlobalVUGraph (mpP, &preppedSurface, uNumSegs, vNumSegs, uStart, vStart);
        ApplyTrimBoundariesToGlobalVUGraph (mpP, surface);
        AddEvaluatedVuToPolyface (mpP->PeekGraph (), builder, surface);
        mpP->DropGraph (graph);
        }

    if (uStart)
        msbspline_free (uStart);
    if (vStart)
        msbspline_free (vStart);
    preppedSurface.ReleaseMem ();

    return SUCCESS;
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
* @bsimethod                                                    Ray.Bentley     05/92
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
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurfaceToPolyface
(
MeshOutputHandlerR handler,
IFacetOptionsR      options,
IPolyfaceConstructionR builder,
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

    if (fixBoundaries)
        bspmesh_fixBoundaries (&surfaceP->boundaries, &surfaceP->numBounds, true);

    status = MeshAndOutput (builder, &meshParams, *surfaceP);

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
* @bsimethod                                                    Earlin.Lutz     08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static int meshSurface
(
MeshOutputHandlerR handler,
IFacetOptionsR      options,
IPolyfaceConstructionR builder,
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
        status = meshSurfaceToPolyface (handler, options, builder,
                        toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength,
                        parameterScale,
                        &tempSurface,
                        reverse, covePatchBoundaries);
        bspsurf_closeTempTrimmedSurface (&tempSurface);
        return status;
        }
    else
        return meshSurfaceToPolyface (handler, options, builder,
                        toleranceTransformP,
                        toleranceCameraP, toleranceFocalLength,
                        parameterScale,
                        surfaceP,
                        reverse, covePatchBoundaries);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
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
        meshSurface (context, GetFacetOptionsR (), *this, NULL, NULL, 0.0,
                                NULL,
                                const_cast <MSBsplineSurface*> (&surface),
                                false,
                                true);

        PolyfaceHeaderPtr client = GetClientMeshPtr ();
        static int s_mark = 1;
        if (s_mark == 1)
            client->MarkInvisibleEdges (s_defaultVisibleEdgeAngle);
        else if(s_mark == 2)
            client->MarkAllEdgesVisible ();
        }

    SetCurrentFaceParamDistanceRange (distanceUVRange);
    EndFace_internal ();
    }



END_BENTLEY_NAMESPACE
