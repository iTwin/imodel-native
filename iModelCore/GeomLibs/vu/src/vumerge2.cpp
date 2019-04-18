/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Split an edge at the given 2D point and insert its coordinates on both sides.
* @param graphP IN OUT  graph header
* @param leftP  OUT     new node at point on same side as edgeP
* @param rightP OUT     new node at point on opposite side
* @param edgeP  IN      base node of edge to split
* @param point  IN      coordinates of new nodes
* @group "VU Edges"
* @see vu_splitEdge
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdgeAtPoint
(
VuSetP graphP,
VuP *leftP,
VuP *rightP,
VuP edgeP,
DPoint2d *point
)
    {
    vu_splitEdge(graphP,edgeP,leftP,rightP);
    VU_SET_UV(*leftP,point->x,point->y);
    VU_SET_UV(*rightP,point->x,point->y);
    }


struct VuSortPoint
{
DPoint3d point;
int userData;
double a;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool cb_compareGPUserDataA
(
const VuSortPoint &gpA,
const VuSortPoint &gpB
)
    {
    return (gpA.userData < gpB.userData)
        || (gpA.userData == gpB.userData && gpA.a < gpB.a);
    }

/*=================================================================================**//**
* @bsiclass                                                     Earlin.Lutz     02/2006
+===============+===============+===============+===============+===============+======*/
class VuSortArray
{
public:
bvector <VuSortPoint> mArray;

VuSortArray () :
    mArray ()
    {
    }

void Add (VuSortPoint &gp)
    {
    mArray.push_back (gp);
    }

bool Get (VuSortPoint &value, int index)
    {
    if (index >= 0 && index < (int) mArray.size ())
        {
        value = mArray[index];
        return true;
        }
    return false;
    }

void Sort ()
    {
    if (mArray.size() > 1)
        std::sort (mArray.begin (), mArray.end (), cb_compareGPUserDataA);
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool cb_double_LT_double
(
const double& a,
const double& b
)
    {
    return a < b;
    }

/*=================================================================================**//**
* @bsiclass                                                     Earlin.Lutz     02/2006
+===============+===============+===============+===============+===============+======*/
class FractionArray
{
private:
bvector <double> mArray;

public:
    FractionArray ()
        : mArray ()
        {}

    void Clear ()
        {
        mArray.clear ();
        }

    size_t Size ()
        {
        return mArray.size ();
        }

    void Add (double f)
        {
        mArray.push_back (f);
        }

    bool Get (double &value, int index)
        {
        if (index >= 0 && index < (int) mArray.size ())
            {
            value = mArray[index];
            return true;
            }
        return false;
        }

    void Sort ()
        {
        std::sort (mArray.begin (), mArray.end (), cb_double_LT_double);
        }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_collectUpedges
(
VuArrayP pArray,
VuSetP  pGraph
)
    {
    vu_arrayClear (pArray);
    VU_SET_LOOP (pCurr, pGraph)
        {
        if (vu_below (pCurr, vu_fsucc(pCurr)))
            {
            vu_arrayAdd (pArray, pCurr);
            }
        }
    END_VU_SET_LOOP (pCurr, pGraph)
    }

/*---------------------------------------------------------------------------------**//**
* Sort the source array, then copy to destination with close values replaced by
*   average or equally spaced values.
+---------------+---------------+---------------+---------------+---------------+------*/
static void collapaseDoubles
(
FractionArray &sourceArray,
FractionArray &destArray,
double tol,
double minF,
double maxF
)
    {
    int i, j;
    double f0, f1, f1Max, fCurr, df;
    int numSource = (int) sourceArray.Size ();

    sourceArray.Sort ();
    destArray.Clear ();

    for (i = 0; i < numSource;)
        {
        sourceArray.Get (f0, i);
        i++;
        if (f0 >= minF && f0 <= maxF)
            {
            f1 = f0;
            f1Max = f0 + tol;
            while (sourceArray.Get (fCurr, i)
                    && fCurr <= f1Max)
                {
                f1 = fCurr;
                f1Max = f1 + tol;
                i++;
                }

            df = f1 - f0;
            if (df <= tol)
                {
                /* Make a single point at the average fraction */
                destArray.Add (0.5 * (f0 + f1));

                }
            else
                {
                /* Make equidistant points separated.  These will be separated by
                    at least the tolerance and at most twice the tolerance.
                */
                double nf = (int)(df / tol);
                df /= nf;
                for (j = 0; j <= nf; j++)
                    {
                    fCurr = f0 + df * j;
                    destArray.Add (fCurr);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Search forward in a (sorted) array of vertex pointers for those which
* are in the bounding box of an edge.  Project to edge and record projection fractions.
+---------------+---------------+---------------+---------------+---------------+------*/
static void collectProjections
(
VuSetP pGraph,
FractionArray &fractionArray,
VuArrayP pVertexArray,
int   lowestPossibleVertexIndex,
const DPoint3d *pxyz0,
const DPoint3d *pxyz1,
double xLow,
double yLow,
double xHigh,
double yHigh,
double perpTolSquared,
double vertexTolSquared
)
    {
    int currVertexIndex;
    DPoint3d xyzVertex, xyzEdge;
    VuP pVertexNode;
    DPoint3d edgeVector;
    DPoint3d vertexVector;
    double fraction, distanceSquared;
    double uu, uv;
    int numVertex = vu_arraySize (pVertexArray);

    edgeVector.DifferenceOf (*pxyz1, *pxyz0);
    uu = edgeVector.DotProductXY (edgeVector);

    fractionArray.Clear ();
    for (currVertexIndex = lowestPossibleVertexIndex;
        currVertexIndex < numVertex;
        currVertexIndex++
        )
        {
        pVertexNode = vu_arrayGetVuP (pVertexArray, currVertexIndex);
        vu_getDPoint3d (&xyzVertex, pVertexNode);
        if (xyzVertex.y > yHigh)
            break;

        if (xyzVertex.x >= xLow && xyzVertex.x <= xHigh)
            {
            vertexVector.DifferenceOf (xyzVertex, *pxyz0);
            uv = vertexVector.DotProductXY (edgeVector);
            DoubleOps::SafeDivide (fraction, uv, uu, 0.0);
            xyzEdge.SumOf (*pxyz0, edgeVector, fraction);
            if (    fraction > 0.0
                &&  fraction < 1.0)
                {
                distanceSquared = xyzVertex.DistanceSquaredXY (xyzEdge);
                if (distanceSquared < perpTolSquared)
                    {
                    if (   xyzVertex.DistanceSquaredXY (*pxyz0) > vertexTolSquared
                        && xyzVertex.DistanceSquaredXY (*pxyz1) > vertexTolSquared
                        )
                        {
                        fractionArray.Add (fraction);
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Split edges at projections of vertices.
* @param perpTol IN tolerance band around edges.
* @param vertexTol IN tolerance around vertices.  An off-edge point is not considered
*       if it is within this distance of a vertex.
* @param alongEdgeTol IN tolerance bewteen "nearby" projections along edge.  If
*       a cluster of projections are within this tolerance, they are consolidated into
*       a single point at the average projection.
+---------------+---------------+---------------+---------------+---------------+------*/
static void splitAtFractions
(
VuSetP pGraph,
FractionArray &fraction0Array,
FractionArray &fraction1Array,
VuP pEdgeNode0,
double vertexTol,
double alongEdgeTol
)
    {
    DPoint3d xyz0, xyz1;
    VuP pEdgeNode1;
    size_t num0;
    double edgeLength;

    pEdgeNode1 = vu_fsucc (pEdgeNode0);
    vu_getDPoint3d (&xyz0, pEdgeNode0);
    vu_getDPoint3d (&xyz1, pEdgeNode1);

    num0 = fraction0Array.Size ();
    edgeLength = xyz0.DistanceXY (xyz1);

    if (num0 > 0 && edgeLength > alongEdgeTol)
        {
        int num1, i;
        DPoint3d xyzVertex;
        double fraction, fractionTol, fractionTolAtVertex;
        VuP pBaseNode = pEdgeNode0;
        VuP pLeftNode = NULL, pRightNode = NULL;
        fractionTol = alongEdgeTol / edgeLength;
        fractionTolAtVertex = vertexTol / edgeLength;

        collapaseDoubles (fraction0Array, fraction1Array, fractionTol,
                    fractionTolAtVertex, 1.0 - fractionTolAtVertex);

        num1 = (int) fraction1Array.Size ();
        for (i = 0; i < num1; i++)
            {
            fraction1Array.Get(fraction, i);
            xyzVertex.Interpolate (xyz0, fraction, xyz1);
            vu_splitEdge (pGraph, pBaseNode, &pLeftNode, &pRightNode);
            vu_setDPoint3dAroundVertex (pLeftNode, &xyzVertex);
            pBaseNode = pLeftNode;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Split edges at projections of vertices.
* @param perpTol IN tolerance band around edges.
* @param vertexTol IN tolerance around vertices.  An off-edge point is not considered
*       if it is within this distance of a vertex.
* @param alongEdgeTol IN tolerance bewteen "nearby" projections along edge.  If
*       a cluster of projections are within this tolerance, they are consolidated into
*       a single point at the average projection.
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_splitEdgesNearVertices
(
VuSetP pGraph,
double perpTol,
double vertexTol,
double alongEdgeTol
)
    {
    VuArrayP pVertexArray = vu_grabArray (pGraph);
    VuArrayP pEdgeArray = vu_grabArray (pGraph);
    FractionArray fraction0Array = FractionArray ();
    FractionArray fraction1Array = FractionArray ();
    int lowestPossibleVertexIndex;
    int edgeIndex;
    int numEdge;
    int numVertex;
    DPoint3d xyz0, xyz1;
    DPoint3d xyzVertex;
    DRange3d tolerancedEdgeRange;
    VuP      pEdgeNode0, pEdgeNode1;
    VuP     pVertexNode;
    double yLow, yHigh, xLow, xHigh;
    double perpTolSquared, vertexTolSquared;
    double edgeLength;
    double maxTol;

    vertexTolSquared    = vertexTol * vertexTol;
    perpTolSquared      = perpTol * perpTol;

    maxTol = vertexTol;
    if (perpTol > maxTol)
        maxTol = vertexTol;
    if (alongEdgeTol > maxTol)
        maxTol = alongEdgeTol;

    vu_collectVertexLoops (pVertexArray, pGraph);
    numVertex = vu_arraySize (pVertexArray);
    vu_collectUpedges (pEdgeArray, pGraph);
    vu_arraySort0 (pVertexArray, vu_compareLexicalUV0);
    vu_arraySort0 (pEdgeArray, vu_compareLexicalUV0);

    lowestPossibleVertexIndex = 0;
    numEdge = vu_arraySize (pEdgeArray);
    for (edgeIndex = 0; edgeIndex < numEdge; edgeIndex++)
        {
        pEdgeNode0 = vu_arrayGetVuP (pEdgeArray, edgeIndex);
        pEdgeNode1 = vu_fsucc (pEdgeNode0);
        vu_getDPoint3d (&xyz0, pEdgeNode0);
        vu_getDPoint3d (&xyz1, pEdgeNode1);
        // EDL Aug 2010
        // WHY IS THIS SQRT(distance)?
        // IT'S BEEN HERE FOR A VERY LONG TIME !!!!!
        edgeLength = sqrt (xyz0.DistanceXY (xyz1));
        if (edgeLength <= vertexTol || edgeLength < 2.0 * alongEdgeTol)
            continue;

        tolerancedEdgeRange = DRange3d::From (xyz0, xyz1);
        tolerancedEdgeRange.Extend (maxTol);
        xLow = tolerancedEdgeRange.low.x;
        xHigh = tolerancedEdgeRange.high.x;
        yLow = tolerancedEdgeRange.low.y;
        yHigh = tolerancedEdgeRange.high.y;

        /* Advance over vertices clearly below this and all subsequent edges */
        while (lowestPossibleVertexIndex < numVertex)
            {
            pVertexNode = vu_arrayGetVuP (pVertexArray, lowestPossibleVertexIndex);
            vu_getDPoint3d (&xyzVertex, pVertexNode);
            if (xyzVertex.y >= yLow)
                break;
            lowestPossibleVertexIndex++;
            }

        collectProjections
                    (pGraph, fraction0Array,
                    pVertexArray,
                    lowestPossibleVertexIndex,
                    &xyz0, &xyz1,
                    xLow, yLow, xHigh, yHigh,
                    perpTolSquared,
                    vertexTolSquared
                    );

        splitAtFractions
                    (
                    pGraph,
                    fraction0Array,
                    fraction1Array,
                    pEdgeNode0,
                    vertexTol,
                    alongEdgeTol
                    );
        }

    vu_returnArray (pGraph, pEdgeArray);
    vu_returnArray (pGraph, pVertexArray);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static void recordIntersectionPoint
(
VuSortArray &sorter,
int index,
double fraction,
const DPoint3d *pxyz
)
    {
    VuSortPoint gp;
    gp.point.x = pxyz->x;
    gp.point.y = pxyz->y;
    gp.point.z = pxyz->z;
    gp.userData = index;
    gp.a    = fraction;
    sorter.Add (gp);
    }

/*---------------------------------------------------------------------------------**//**
* Scan forward from index0.  Copy out "a" values for graphics points with same userData
*   value as at index0.
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    unloadGPABlockWithSameUserData
(
VuSortArray &sorter,
FractionArray &fractionArray,
int *pUserData,
int index0
)
    {
    VuSortPoint gp;
    int currIndex;
    int userData;
    fractionArray.Clear ();
    if (sorter.Get (gp, index0))
        {
        *pUserData = userData = gp.userData;
        fractionArray.Add (gp.a);
        for (currIndex = index0 + 1;
                sorter.Get (gp, currIndex)
            && gp.userData == userData;
            currIndex++
            )
            {
            fractionArray.Add (gp.a);
            }
        }
    return fractionArray.Size () > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Get the range of an edge addressed by its lower y node.
* @param pXMin OUT min x of range.
* @param pXMax OUT max x of range.
* @param pYMin OUT min y of range.
* @param pYMax OUT max y of range.
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_edgeRange
(
VuP pNode0,
double *pXMin,
double *pXMax,
double *pYMin,
double *pYMax
)
    {
    double x0, y0;
    double x1, y1;
    vu_getXY (&x0, &y0, pNode0);
    vu_getXY (&x1, &y1, vu_edgeMate(pNode0));
    if (x0 <= x1)
        {
        *pXMin = x0;
        *pXMax = x1;
        }
    else
        {
        *pXMin = x1;
        *pXMax = x0;
        }
    /* Humbug  pNode0 should be lower y, but do the test anyway. */
    if (y0 <= y1)
        {
        *pYMin = y0;
        *pYMax = y1;
        }
    else
        {
        *pYMin = y1;
        *pYMax = y0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if a f is in 0..1 with tolerance
* @param vertexTol IN splits this close to a vertex are skipped.
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    interiorFraction (double f, double tol)
    {
    return f > tol && f < 1.0 - tol;
    }

/*---------------------------------------------------------------------------------**//**
* @description Split edges at clear interior transverse intersections with other edges.
* @param vertexTol IN splits this close to a vertex are skipped.
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_splitEdgesAtIntersections
(
VuSetP pGraph,
double vertexTol,
double alongEdgeTol
)
    {
    VuArrayP pEdgeArray = vu_grabArray (pGraph);
    VuSortArray sorter = VuSortArray ();
    static double sFractionTol = 1.0e-13;
    int iEdgeA, iEdgeB;
    VuP pEdgeA, pEdgeB;
    DPoint3d xyzA, xyzB;
    double fractionA, fractionB;
    int numEdge;
    int gpaIndex0, edgeIndex;
    double xMinA, yMinA, xMaxA, yMaxA;
    double xMinB, yMinB, xMaxB, yMaxB;
    FractionArray fraction0Array = FractionArray ();
    FractionArray fraction1Array = FractionArray ();

    vu_collectUpedges (pEdgeArray, pGraph);
    vu_arraySort0 (pEdgeArray, vu_compareLexicalUV0);
    numEdge = vu_arraySize (pEdgeArray);

    for (iEdgeA = 0; iEdgeA < numEdge; iEdgeA++)
        {
        pEdgeA = vu_arrayGetVuP (pEdgeArray, iEdgeA);
        vu_edgeRange (pEdgeA, &xMinA, &xMaxA, &yMinA, &yMaxA);
        for (iEdgeB = iEdgeA + 1; iEdgeB < numEdge; iEdgeB++)
            {
            pEdgeB = vu_arrayGetVuP (pEdgeArray, iEdgeB);
            vu_edgeRange (pEdgeB, &xMinB, &xMaxB, &yMinB, &yMaxB);
            if (yMinB >= yMaxA)
                break;
            if (xMaxB <= xMinA)
                continue;
            if (xMinB >= xMaxA)
                continue;
            if (   vu_intersectXYEdges (&fractionA, &fractionB, &xyzA, &xyzB, pEdgeA, pEdgeB)
                && interiorFraction (fractionA, sFractionTol)
                && interiorFraction (fractionB, sFractionTol)
                )
                {
                recordIntersectionPoint (sorter, iEdgeA, fractionA, &xyzA);
                recordIntersectionPoint (sorter, iEdgeB, fractionB, &xyzB);
                }
            }
        }

    sorter.Sort ();

    for (gpaIndex0 = 0;
        unloadGPABlockWithSameUserData (sorter, fraction0Array, &edgeIndex, gpaIndex0);
        )
        {
        size_t numFraction = fraction0Array.Size ();
        pEdgeA = vu_arrayGetVuP (pEdgeArray, edgeIndex);
        splitAtFractions (pGraph, fraction0Array, fraction1Array, pEdgeA, vertexTol, alongEdgeTol);
        gpaIndex0 += (int) numFraction;
        }

    vu_returnArray (pGraph, pEdgeArray);
    }

#ifdef COMPILE_TIMERS
#define START_TIMER \
    {\
    int tick0, tick1;\
    if (s_noisy > 0)\
       tick0 = mdlSystem_getTicks ();
#define END_TIMER(pName)\
    if (s_noisy > 0)\
        {\
        tick1 = mdlSystem_getTicks ();\
        printf ("(merge2002 %s %d ticks)\n",\
            pName, tick1 - tick0);\
        if (s_noisy > 999)\
            vu_printFaceLabels (pGraph, "pName");\
        vu_postGraphToTrapFunc (pGraph, "pName", s_debuggerId, trapMask++);\
        }\
    }
#else
#define START_TIMER
#define END_TIMER(__timerName__)\
        vu_postGraphToTrapFunc (pGraph, __timerName__, s_debuggerId, trapMask++);
#endif

/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with specified duplicate edge handling.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @param pGraph     IN OUT  graph header
* @param mergeType  IN      rule to apply to graph for handling duplicate edges: VUUNION_PARITY or VUUNION_KEEP_ONE_AMONG_DUPLICATES, or
*                           any other value to keep all duplicates
* @param absTol     IN      absolute tolerance, passed into ~mvu_toleranceFromGraphXY to compute merge tolerance
* @param relTol     IN      relative tolerance, passed into ~mvu_toleranceFromGraphXY to compute merge tolerance
* @group "VU Booleans"
* @see vu_unionLoops, vu_mergeLoops
* @bsimethod                                    Earlin.Lutz                     02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_merge2002
(
VuSetP pGraph,
VuMergeType mergeType,
double absTol,
double relTol
)
    {
    double baseTol = vu_toleranceFromGraphXY (pGraph, absTol, relTol);
    vu_setMergeTol (pGraph, baseTol);
    int trapMask = 8000;
    static int s_debuggerId = 0;

    START_TIMER
    END_TIMER("merge2002 -- initial")

    START_TIMER
    vu_consolidateClusterCoordinates (pGraph, baseTol);
    END_TIMER ("vertexCluster")


    START_TIMER
    vu_splitEdgesNearVertices (pGraph, baseTol, baseTol, baseTol);
    END_TIMER ("edge split");

    START_TIMER
    vu_consolidateClusterCoordinates (pGraph, baseTol);
    END_TIMER ("edge split");

    START_TIMER
    vu_splitEdgesAtIntersections (pGraph, baseTol, baseTol);
    END_TIMER ("intersection");

    START_TIMER
    vu_consolidateClusterCoordinatesAndSortByAngle (pGraph, mergeType, baseTol);
    END_TIMER ("final cluster and sort");
    s_debuggerId++;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
