/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
@description Consruct a (single) loop from a contiguous subset of an
   array of points.  Omit duplicates within an xy tolerance.
  No disconnect logic -- next level up must pick index intervals.
  Each node is labeled with the index of its corresponding xyz coordinates.
@param graphP IN pointer to receiving graph.
@param pXYZIn IN input points.
@param iFirst IN index of first point of loop.
@param iLast IN index of last point of loop.
@param xyTolerance IN tolerance for declaring adjacent points identical.
@return pointer to some Vu node in the loop.
*/
VuP VuOps::MakeIndexedLoopFromPartialArray
(
VuSetP          graphP,
bvector<DPoint3d> const *pXYZIn,
size_t          iFirst,
size_t          iLast,
VuMask          leftMask,
VuMask          rightMask,
double          xyTolerance
)
    {
    double      dx, dy;
    DPoint3d    lastPoint;
    DPoint3d    currPoint;
    VuP         baseP, insideP, outsideP;

    baseP = VU_NULL;    /* During construction, baseP is the last created VU (if any) */

    /* Strip of trailing points that duplicate first point */
    for (; iLast > iFirst; iLast--)
        {
        dx = fabs (pXYZIn->at(iLast).x - pXYZIn->at(iFirst).x);
        dy = fabs (pXYZIn->at(iLast).y - pXYZIn->at(iFirst).y);
        if (dx > xyTolerance || dy > xyTolerance)
            break;
        }

    if (iLast < iFirst)
        return NULL;

    lastPoint = pXYZIn->at(iLast);

    for (size_t i = iFirst; i <= iLast; i++)
        {
        currPoint = pXYZIn->at(i);
        if (i == iFirst      /* Always put the first point in */
            || fabs (currPoint.x - lastPoint.x) > xyTolerance
            || fabs (currPoint.y - lastPoint.y) > xyTolerance)
            {
            vu_splitEdge (graphP, baseP, &insideP, &outsideP);
            baseP = insideP;
            VU_SET_UVW( insideP, currPoint.x, currPoint.y, currPoint.z);
            VU_SET_UVW(outsideP, currPoint.x, currPoint.y, currPoint.z);
            vu_setUserDataPAsInt ( insideP, (int)i);
            vu_setUserDataPAsInt (outsideP, (int)i);
            lastPoint = currPoint;
            }
        }

    if (baseP)
        {
        vu_setMaskAroundFace (baseP, leftMask);
        vu_setMaskAroundFace (VU_VSUCC(baseP), rightMask);
        }
    return baseP;
    }

VuP     VuOps::MakeIndexedLoopsFromArrayWithDisconnects
(
VuSetP          graphP,
bvector<DPoint3d> const *pXYZIn,
VuMask          leftMask,
VuMask          rightMask,
double          xyTolerance
)
    {
    size_t nPoints = pXYZIn->size ();
    size_t iFirst, iLast, iTest;
    VuP outputP = NULL;
    VuP currP = NULL;
    iTest = 0;
    while (iTest < nPoints)
        {
        iFirst = iTest;
        iTest = iFirst;
        for (iTest = iFirst;
               iTest < nPoints
               && !pXYZIn->at(iTest).IsDisconnect()
            ;
            iTest++)
            {
            /* Nothing to do, just looking ahead */
            }
        iLast = iTest - 1;
        iTest++;
        if (iLast >= iFirst)
            {
            currP = MakeIndexedLoopFromPartialArray
                        (
                        graphP,
                        pXYZIn,
                        iFirst, iLast,
                        leftMask, rightMask, xyTolerance
                        );
            if (!outputP)
                outputP = currP;
            }
        }
    return outputP;
    }


void VuOps::MakeEdge
(
VuSetP          graph,
VuP             &nodeA,
VuP             &nodeB,
DPoint3dCR      xyzA,
DPoint3dCR      xyzB,
VuMask          leftMask,
VuMask          rightMask
)
    {
    vu_makePair (graph, &nodeA, &nodeB);
    nodeA->SetXYZ (xyzA);
    nodeB->SetXYZ (xyzB);
    nodeA->SetMask (leftMask);
    nodeB->SetMask (rightMask);
    }

VuP VuOps::MakeEdge
(
VuSetP          graph,
DPoint3dCR      xyzA,
DPoint3dCR      xyzB,
VuMask          leftMask,
VuMask          rightMask
)
    {
    VuP nodeA, nodeB;
    MakeEdge (graph, nodeA, nodeB, xyzA, xyzB, leftMask, rightMask);
    return nodeA;
    }

void VuOps::MakeEdges
(
VuSetP          graph,
VuP             &chainTail,
VuP             &chainHead,
bvector<DPoint3d> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol
)
    {
    chainTail = chainHead = nullptr;
    if (xyz.size () > 1)
        {
        DPoint3d xyzA = xyz.front ();
        double aa = abstol * abstol;
        for (size_t i = 1, n = xyz.size (); i < n; i++)
            {
            DPoint3d xyzB = xyz[i];
            if (xyzA.DistanceSquaredXY (xyzB) > aa)
                {
                VuP nodeA, nodeB;
                MakeEdge (graph, nodeA, nodeB, xyzA, xyzB, leftMask,rightMask);
                if (chainTail == nullptr)
                    {
                    chainTail = nodeA;
                    chainHead = nodeB;
                    }
                else
                    {
                    graph->VertexTwist (chainHead, nodeA);
                    chainHead = nodeB;
                    }
                 xyzA = xyzB;
                }
            }
        }
    }

void VuOps::MakeLoops
(
VuSetP          graph,
bvector<bvector<DPoint3d>> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol
)
    {
    VuP nodeA, nodeB;
    for (auto &loop : xyz)
        MakeLoop (graph, nodeA, nodeB, loop, leftMask, rightMask, abstol);
    }

void VuOps::MakeEdges
(
VuSetP          graph,
bvector<bvector<DPoint3d>> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol
)
    {
    VuP nodeA, nodeB;
    for (auto &chain : xyz)
        MakeEdges (graph, nodeA, nodeB, chain, leftMask, rightMask, abstol);
    }


void VuOps::MakeLoop
(
VuSetP          graph,
VuP             &insideNode,
VuP             &outsideNode,
bvector<DPoint3d> const &xyz,
VuMask          leftMask,
VuMask          rightMask,
double          abstol
)
    {
    MakeEdges (graph, insideNode, outsideNode, xyz, leftMask, rightMask, abstol);
    if (insideNode != nullptr && outsideNode != nullptr)
        {
        double aa = abstol * abstol;
        DPoint3d xyzA = insideNode->GetXYZ ();
        DPoint3d xyzB = outsideNode->GetXYZ ();
        if (xyzA.DistanceSquaredXY (xyzB) > aa)
            {
            // Caller's data did not close .. add another edge.
            VuP nodeA, nodeB;
            MakeEdge (graph, nodeA, nodeB, xyzA, xyzB, leftMask,rightMask);
            graph->VertexTwist (outsideNode, nodeA);
            graph->VertexTwist (insideNode, nodeB);
            outsideNode = nodeB;
            }
        else
            {
            // Caller's data closed .. seal the loop
            graph->VertexTwist (outsideNode, insideNode);
            }
        }
    }
    

/*-----------------------------------------------------------------*//**
* Add numAdd points to a caller-allocated point buffer.  Caller is
* responsible for ensuring adequate space.   Points are spaced
* evenly from the final point in the input buffer towards a
* specified target point.
* @param buffer <=> point buffer
* @param x1 = target x coordinate
* @param y1 => target y coordinate
* @param z1 => target z coordinate
* @param numAdd => number of points to compute and add to buffer.
* @bsimethod                            EarlinLutz      10/01
+----------------------------------------------------------------------*/
static void extendPointString
(
bvector<DPoint3d> &buffer,
double      x1,
double      y1,
double      z1,
size_t      numAdd
)
    {
    size_t i;
    DPoint3d point0, point1;
    double lambda;
    if (numAdd < 1)
        numAdd = 1;
    point1.Init (x1,y1,z1);
    if (buffer.size () > 0)
        {
        point0 = buffer.back ();
        for (i = 1; i < numAdd; i++)
            {
            lambda = (double)i / (double)numAdd;
            buffer.push_back (DPoint3d::FromInterpolate (point0, lambda, point1));
            }
        }
    buffer.push_back (point1);
    }

/*-----------------------------------------------------------------*//**
* Add a rectangular linestring to a vu graph.
* @param graph <=> receiver graph.
* @param pOutRange <= range of expanded rectangle.
* @param pInRange => Range of rectangle.  The rectangle is at the
*       z level of inRange.low.  xy coordinates are the range rectangle
*       augmented by fringe delta.
* @param absDelta => minimum absolute fringe delta.
* @param relDelta => minumum fringe delta as fraction of overall diagonal.
* @param numOnLongEdge => number of sub edges on long edge.  Short edge
*               receives a proportional number of subedges.
* @bsimethod                            EarlinLutz      10/01
+----------------------------------------------------------------------*/
VuP VuOps::AddRangeBase
(
VuSetP          graph,
DRange3dR       outRange,
DRange3dCR      inRange,
double          absDelta,
double          relDelta,
size_t          numOnLongEdge,
VuMask          interiorMask,
VuMask          exteriorMask 
)
    {
    bvector<DPoint3d> buffer;
    double diagonal = inRange.low.DistanceXY (inRange.high);
    double delta = 0.0;
    double xMin, xMax, yMin, yMax, zz, dx, dy;
    size_t numX, numY;

    if (numOnLongEdge > 24)
        numOnLongEdge = 24;

    if (relDelta > 0.0)
        delta = relDelta * diagonal;

    if (absDelta > delta)
        delta = absDelta;

    xMin = inRange.low.x - delta;
    xMax = inRange.high.x + delta;
    yMin = inRange.low.y - delta;
    yMax = inRange.high.y + delta;
    zz   = inRange.low.z;

    dx = xMax - xMin;
    dy = yMax - yMin;

    if (dx == 0.0 || dy == 0.0)
        return nullptr;

    if (dx > dy)
        {
        numX = numOnLongEdge;
        numY = (size_t)( (numX * dy) / dx);
        }
    else
        {
        numY = numOnLongEdge;
        numX = (size_t) ((numY* dx) / dy);
        }

    if (numX <= 0)
        numX = 1;
    if (numY <= 0)
        numY = 1;

    buffer.push_back (DPoint3d::From (xMin, yMin, zz));
    extendPointString (buffer, xMax, yMin, zz, numX);
    extendPointString (buffer, xMax, yMax, zz, numY);
    extendPointString (buffer, xMin, yMax, zz, numX);
    extendPointString (buffer, xMin, yMin, zz, numY);

    outRange = DRange3d::From (xMin, yMin, zz);
    outRange.Extend (xMax, yMax, zz);

    VuP chainHead = nullptr;
    VuP chainTail = nullptr;
    MakeEdges (graph, chainHead, chainTail, buffer, interiorMask, exteriorMask, 0.0);
    return chainTail;
    }

VuP VuOps::AddExpandedRange (VuSetP graph, DRange3dCR localRange, size_t numPoint, double expansionFracton, VuMask insideMask, VuMask outsideMask)
    {
    static int      s_maxSideEdge = 40;
    static int      s_minSideEdge = 4;
    DRange3d expandedRange;
    int numSideEdge = (int)(0.5 * sqrt ((double)numPoint));
    if (numSideEdge > s_maxSideEdge)
        numSideEdge = s_maxSideEdge;
    if (numSideEdge < s_minSideEdge)
        numSideEdge = s_minSideEdge;
    return AddRangeBase (graph, expandedRange, localRange, 0.0, expansionFracton, numSideEdge, insideMask, outsideMask);
    }

void VuOps::CollectLoopsWithMaskSummary
(
VuSetP graph,
TaggedPolygonVectorR polygons,
VuMask mask,            //!< [in] mask to test around each face.
bool wrap       //!< [in] true to add wraparound point to each polygon
)
    {
    polygons.clear ();
    VuMask visitMask = graph->GrabMask ();
    graph->ClearMaskInSet (visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!seed->HasMask (visitMask))
            {
            seed->SetMaskAroundFace (visitMask);
            VuMask andMask = -1;
            VuMask orMask  = 0;
            polygons.push_back (TaggedPolygon ());
            bvector<DPoint3d> &points = polygons.back ().GetPointsR ();
            VU_FACE_LOOP (edge, seed)
                {
                points.push_back (edge->GetXYZ ());
                andMask &= vu_getMask (edge, mask);
                orMask |= vu_getMask (edge, mask);
                }
            END_VU_FACE_LOOP (edge, seed)
            if (wrap)
                points.push_back (seed->GetXYZ ());
            polygons.back ().SetIndexA ((ptrdiff_t)andMask);
            polygons.back ().SetIndexA ((ptrdiff_t)orMask);
            }
        }
    END_VU_SET_LOOP (seed, graph)
    graph->DropMask (visitMask);
    }
// visible from bsppolyface ....
size_t AddInteriorBreaks (VuSetP graph, DRange3dCR range, bvector<double> breaks, int uv, VuMask mask);
void vu_deletePairedEdges (VuSetP graph, VuMask mask);

struct Options_vu_createTrianglated
{
double maxEdgeXLength;
double maxEdgeYLength;
double meshXLength;
double meshYLength;
bool isometricGrid;
};

static VuSetP vu_createTriangulated
(
bvector< bvector<DPoint3d>> const &boundaries,
bvector< bvector<DPoint3d>> const &chains,
bvector<DPoint3d> const &isolatedPoints,
bvector <double> const    &uBreaks,
bvector <double> const    &vBreaks,
Options_vu_createTrianglated const &optionsIn
)
    {
    auto options = optionsIn;   // to be updated
    static int s_maxEdge = 400;
    static int s_maxSubEdge = 100;
    static double s_defaultCount = 10.0;
    //static double s_graphRelTol = 1.0e-10;
    VuSetP      graph = vu_newVuSet (0);
    StatusInt status = SUCCESS;
    int    maxPerFace = 3;

    static double s_shortEdgeToleranceFactor = 1.0e-8;
    static double s_skewAngle = 0.0;
    double skewAngle = s_skewAngle;
    if (options.isometricGrid)
        skewAngle = msGeomConst_pi / 6.0;
    DRange3d range = DRange3d::From (boundaries);

    double xyTolerance = s_shortEdgeToleranceFactor * range.low.Distance (range.high);

    VuOps::MakeLoops (graph, boundaries, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, xyTolerance);
    VuOps::MakeEdges (graph, chains, VU_RULE_EDGE, VU_RULE_EDGE, xyTolerance);

    double dx = range.high.x - range.low.x;
    double dy = range.high.y - range.low.y;
    if (options.meshXLength <= 0.0)
        options.meshXLength = dy / s_defaultCount;
    if (options.meshYLength <= 0.0)
        options.meshYLength = dy / s_defaultCount;

    if (s_maxEdge * options.meshXLength < dx)
        {
        status = ERROR;
        options.meshXLength = dx / s_maxEdge;
        }
    if (s_maxEdge * options.meshYLength < dy)
        {
        status = ERROR;
        options.meshYLength = dy / s_maxEdge;
        }
    
    DPoint3d origin = range.low;
    double ax0, ay0, ax1, ay1;
    DoubleOps::SafeDivide (ax0, range.low.x - origin.x, options.meshXLength, 0.0);
    DoubleOps::SafeDivide (ay0, range.low.y - origin.y, options.meshYLength, 0.0);
    int ix0 = (int) floor (ax0);
    int iy0 = (int) floor (ay0);

    DoubleOps::SafeDivide (ax1, range.high.x - origin.x, options.meshXLength, 0.0);
    DoubleOps::SafeDivide (ay1, range.high.y - origin.y, options.meshYLength, 0.0);
    int ix1 = (int)ceil (ax1);
    int iy1 = (int)ceil (ay1);

    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_deletePairedEdges (graph, VU_BOUNDARY_EDGE);
    if (0 != AddInteriorBreaks (graph, range, uBreaks, 0, VU_RULE_EDGE)
            + AddInteriorBreaks (graph, range, vBreaks, 1, VU_RULE_EDGE)
       )
        {
        // remerge with new edges ....
        vu_mergeOrUnionLoops (graph, VUUNION_UNION);
        }
    int numSplit = vu_splitLongEdges (graph, VU_BOUNDARY_EDGE | VU_RULE_EDGE, 0.0, options.maxEdgeXLength, options.maxEdgeYLength, s_maxSubEdge);
    if (numSplit > 0)
        vu_mergeOrUnionLoops (graph, VUUNION_UNION);

    vu_regularizeGraph (graph);
    vu_markAlternatingExteriorBoundaries(graph,true);
    vu_freeNonMarkedEdges (graph, VU_BOUNDARY_EDGE | VU_RULE_EDGE);

    if (skewAngle != 0.0)
        {
        Transform worldToLocal, localToWorld;
        double s = sin(skewAngle);
        double c = cos (skewAngle);
        localToWorld.InitFromRowValues (
                    1, s, 0, 0,
                    0, c, 0, 0,
                    0, 0, 1, 0);
        localToWorld.SetFixedPoint(range.low);
        worldToLocal.InverseOf (localToWorld);
        vu_transform (graph, &worldToLocal);
        int numX = ix1 - ix0;
        int numY = iy1 - iy0;
        numX = (int)ceil ((1.0 + s) * (double)numX);
        numY = (int)ceil((double)numY / c);
        vu_buildNonInterferingGrid (graph, numX, numY, 0, 0);
        vu_transform (graph, &localToWorld);
        }
    else
        {
        vu_buildNonInterferingGrid (graph, ix1-ix0, iy1-iy0, 0, 0);
        }
    vu_setZInGraph (graph, range.low.z);

    vu_regularizeGraph (graph);
    vu_markAlternatingExteriorBoundaries(graph,true);
    vu_splitMonotoneFacesToEdgeLimit (graph, maxPerFace);

    vu_flipTrianglesToImproveScaledQuadraticAspectRatio (graph, options.meshYLength, options.meshXLength);

    return graph;
    }

void VuOps_runScaledXYSmoothing
(
VuSetP graph,
double xLength,
double yLength
)
    {
    static double s_smoothTol = 0.001;
    static int s_maxSmoothPass = 100;
    static int s_flipInterval = 4;
    double shiftFraction;
    int numSweep;
    static int s_maxFlipsPerEdgePerPass = 5;
    vu_smoothInteriorVertices (graph, nullptr, nullptr,
            s_smoothTol, s_maxSmoothPass,s_flipInterval, s_maxFlipsPerEdgePerPass, &shiftFraction, &numSweep);
    }

VuSetP VuOps::CreateTriangulatedGrid
(
bvector<bvector<DPoint3d>>  const &parityBoundaries,
bvector<bvector<DPoint3d>>  const &openChains,
bvector<DPoint3d>           const &isolatedPoints,
bvector <double>            const &uBreaks,
bvector <double>            const &vBreaks,
double              maxEdgeXLength,
double              maxEdgeYLength,
double              meshXLength,
double              meshYLength,
bool                isometricGrid,
bool                laplacianSmoothing
)
    {
    Options_vu_createTrianglated options;
    options.maxEdgeXLength = maxEdgeXLength;
    options.maxEdgeYLength = maxEdgeYLength;
    options.meshXLength = meshXLength;
    options.meshYLength = meshYLength;
    options.isometricGrid = isometricGrid;

    auto graph = vu_createTriangulated (parityBoundaries, openChains, isolatedPoints, uBreaks, vBreaks, options);
    if (nullptr != graph && laplacianSmoothing)
        {
        VuOps_runScaledXYSmoothing (graph, meshXLength, meshYLength);
        }
    return graph;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
