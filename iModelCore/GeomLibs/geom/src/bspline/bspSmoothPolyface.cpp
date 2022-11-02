/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
//#include <Geom/internal2/BspPrivateApi.h>
#include <Vu/capi/vupoly_capi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_qMax = 1.0;
static double s_rMax = 1.0;
static double s_minWeightFactor = 1.0;
static double s_maxWeightfactor = 2.0;
struct VuMSBsplineSurfaceWeightFunction : public VuScalarFunction, VuCoordinateMappingFunction
{
MSBsplineSurfaceCR m_surface;
double m_factor;
double m_power;
VuMSBsplineSurfaceWeightFunction (MSBsplineSurfaceCR surface, double hFactor, double hPower) 
    : m_surface(surface)
    {
    m_factor = hFactor;
    m_power  = hPower;
    }
// map vu graph coordinates to the bspline surface.
bool TryMapCoordinates (VuSetP graph, VuP node, DPoint2dCR uv, DPoint3dR xyz) override
    {
    m_surface.EvaluatePoint (xyz, uv.x, uv.y);
    return true;
    }
// Curvature is (in the limit) the ratio of the differential curved surface are to the plane differential.
// Estimate this as ratio of the full triangle divided by the sum of triangles of the edges with the centroid.
// (And apply caller-supplied scale and power)
double Evaluate (VuSetP graph, VuP node0) override
    {
    if (m_power == 0.0)
        return 1.0;
    DPoint3d uv0, uv1, uv2, uvC;
    DPoint3d xyz0, xyz1, xyz2, xyzC;
    VuP node1 = vu_fsucc (node0);
    VuP node2 = vu_fsucc (node1);
    vu_getDPoint3d (&uv0, node0);
    vu_getDPoint3d (&uv1, node1);
    vu_getDPoint3d (&uv2, node2);
    double a = 1.0 / 3.0;
    uvC.SumOf (uv0, a, uv1, a, uv2, a);
    m_surface.EvaluatePoint (xyz0, uv0.x, uv0.y);
    m_surface.EvaluatePoint (xyz1, uv1.x, uv1.y);
    m_surface.EvaluatePoint (xyz2, uv2.x, uv2.y);
    m_surface.EvaluatePoint (xyzC, uvC.x, uvC.y);
    DVec3d cross01C, cross12C, cross20C, cross012;
    cross01C.CrossProductToPoints (xyz0, xyz1, xyzC);
    cross12C.CrossProductToPoints (xyz1, xyz2, xyzC);
    cross20C.CrossProductToPoints (xyz2, xyz0, xyzC);
    cross012.CrossProductToPoints (xyz0, xyz1, xyz2);
    double magBase = cross012.Magnitude ();
    double magSides = cross01C.Magnitude () + cross12C.Magnitude () + cross20C.Magnitude ();
    double q = 1.0;
    if (magBase > 0.0 && magSides >= magBase)
        q = magSides / magBase;
    s_qMax = DoubleOps::Max (q, s_qMax);
    double r = 1.0 + m_factor * (q - 1.0);
    s_rMax = DoubleOps::Max (r, s_rMax);
    double f = pow (r, m_power);
    if (f < s_minWeightFactor)
        f = s_minWeightFactor;
    if (f > s_maxWeightfactor)
        f = s_maxWeightfactor;
    return f;
    }
};


double VuEdgeSubdivisionTestFunction::ComputeNumEdgesRequired (DPoint2dCR uvA, DPoint2dCR uvB)
    {
    return 1.0;
    }

struct MSBsplineSurfaceRefinementFunction : VuEdgeSubdivisionTestFunction
{
MSBsplineSurfaceCR m_surface;
double m_maxEdgeLength;
double m_angleTol;
double m_chordTol;
public:
MSBsplineSurfaceRefinementFunction (MSBsplineSurfaceCR surface, double chordTol, double angleTol, double maxEdgeLength)
    : m_surface(surface),
    m_chordTol (chordTol),
    m_angleTol (angleTol),
    m_maxEdgeLength (maxEdgeLength)
    {
    }

double ComputeNumEdgesRequired (DPoint2dCR uvA, DPoint2dCR uvB) override
    {
    DRay3d rayA, rayB;
    // Look at normals just inside the ends to avoid crease issues.
    static double s_delta = 1.0e-4;
    DPoint2d uvA1 = DPoint2d::FromInterpolate (uvA, s_delta, uvB);
    DPoint2d uvB1 = DPoint2d::FromInterpolate (uvB, s_delta, uvA);
    if (m_surface.EvaluatePointAndUnitNormal (rayA, uvA1.x, uvA1.y)
      && m_surface.EvaluatePointAndUnitNormal (rayB, uvB1.x, uvB1.y))
        {
        double num = 0.0;
        if (m_maxEdgeLength > 0.0)
            {
            double d = rayA.origin.Distance (rayB.origin);
            if (d > m_maxEdgeLength)
                num = DoubleOps::Max (num, d / m_maxEdgeLength);
            }
        if (m_chordTol > 0.0)
            {
            DPoint3d xyzS, xyzM = DPoint3d::FromInterpolate (rayA.origin, 0.5, rayB.origin);
            DPoint2d uvM = DPoint2d::FromInterpolate (uvA, 0.5, uvB);
            m_surface.EvaluatePoint (xyzS, uvM.x, uvM.y);
            double d = xyzM.Distance (xyzS);
            if (d > m_chordTol)
                num = DoubleOps::Max (num, sqrt (d / m_chordTol));
            }
        if (m_angleTol > 0.0)
            {
            double a = rayA.direction.AngleTo (rayB.direction);
            if (a > m_angleTol)
                num = DoubleOps::Max (num, a / m_angleTol);
            }
        return num;
        }
    return 0.0;
    }
};



static bool IsSliverFace (VuP node)
    {
    return vu_fsucc( vu_fsucc (node)) == node;
    }

size_t AddInteriorBreaks (VuSetP graph, DRange3dCR range, bvector<double> breaks, int uv, VuMask mask)
    {
    size_t numAdded = 0;
    double b0, b1;
    if (uv == 0)
        {
        b0 = range.low.x;
        b1 = range.high.x;
        }
    else
        {
        b0 = range.low.y;
        b1 = range.high.y;
        }
    for (size_t i = 0; i < breaks.size (); i++)
        {
        double b = breaks[i];
        DPoint3d xyz0, xyz1;
        if (b > b0 && b < b1)
            {
            numAdded++;
            if (uv == 0)
                {
                xyz0.Init (b, range.low.y, 0.0);
                xyz1.Init (b, range.high.y, 0.0);
                }
            else
                {
                xyz0.Init (range.low.x, b, 0.0);
                xyz1.Init (range.high.x, b, 0.0);
                }
            VuP node0, node1;
            vu_makePair (graph, &node0, &node1);
            vu_setMask (node0, mask);
            vu_setMask (node1, mask);
            vu_setDPoint3d (node0, &xyz0);
            vu_setDPoint3d (node1, &xyz1);
            }
        }
    return numAdded;
    }
// Mark BOTH sides of numEdges following seed in vertex loop;
static void MarkVSuccEdges (VuP seed, int numEdges, VuMask mask)
    {
    VuP edgeBaseNode = seed;
    for (int i = 0; i < numEdges; i++)
        {
        edgeBaseNode = vu_vsucc (edgeBaseNode);
        vu_setMask (edgeBaseNode, mask);
        VuP edgeMateNode = vu_edgeMate (edgeBaseNode);
        vu_setMask (edgeMateNode, mask);
        }
    }

void vu_deletePairedEdges (VuSetP graph, VuMask mask)
    {
    VuMask deleteMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, deleteMask);
    int numVu = 0;
    int numA = 0;
    VU_SET_LOOP (sectorSeed, graph)
        {
        numVu++;
        if (!IsSliverFace (sectorSeed))
            {
            numA++;
            // walk around the vertex loop counting null and deleted faces ....
            int numNullFace = 0;
            int numDeleted = 0;
            for (VuP next = vu_vsucc (sectorSeed);
                  next != sectorSeed && IsSliverFace (next);
                  next = vu_vsucc (next))
                {
                numNullFace++;
                if (vu_getMask (next, deleteMask))
                    numDeleted++;
                }
            if (numNullFace > 0 && numDeleted == 0)
                {
                int numEdges= numNullFace + 1;
                numEdges -= (numEdges & 0x01);    // only delete an even number of edges.
                MarkVSuccEdges (sectorSeed, numEdges, deleteMask);
                }
            }
        }
    END_VU_SET_LOOP (sectorSeed, graph)
    vu_freeMarkedEdges (graph, deleteMask);
    vu_returnMask (graph, deleteMask);
    }
// (with absolute values)
// accumulate dx into allSums
// if dx > zeroTol AND dy < maxSlope * dx into strongSums
void Accumulate(double dx, double dy, double zeroTol, double maxSlope, double maxDX, UsageSums &allSums, UsageSums &strongSums)
    {
    dx = fabs (dx);
    dy = fabs (dy);
    allSums.Accumulate (dx);
    if (dx > zeroTol && dy < maxSlope * dx)
        strongSums.Accumulate (dx);
    }
//  for each edge
//    accumulate sums of all dx,dy
//    accumualte sums of dx,dy for edges with (dy/dx) < slopeX or (dx/dy) < slopY projection greater than zeroDY,zeroDY
//                and ignoring if larger than maxDX, maxDY
void ComputeDirectionalPropertiesOfPolygons(bvector<bvector<DPoint2d>> &loops,
    bool addClosure,
    double zeroDX,
    double zeroDY,
    double maxDX,
    double maxDY,
    double slopeX,
    double slopeY,
    UsageSums &allXSums,
    UsageSums &allYSums,
    UsageSums &strongXSums,
    UsageSums &strongYSums
)
    {
    allXSums.ClearSums();
    allYSums.ClearSums();
    strongXSums.ClearSums();
    strongYSums.ClearSums();
    for (auto &loop : loops)
        {
        size_t n = loop.size ();
        if (loop.size () > 1)
            {
            DPoint2d xy0 = loop.front ();
            size_t i1 = 1;
            if (addClosure)
                {
                xy0 = loop.back ();
                i1 = 0;
                }
            for (size_t i = i1; i < n; i++)
                {
                DPoint2d xy1 = loop[i];
                double dx = fabs (xy1.x - xy0.x);
                double dy = fabs (xy1.y - xy0.y);
                Accumulate (dx, dy, zeroDX, slopeX, maxDX, allXSums, strongXSums);
                Accumulate(dy, dx, zeroDY, slopeY, maxDY, allYSums, strongYSums);
                xy0 = xy1;
                }
            }
        }
    }
static VuSetP vu_createTriangulated
(
bvector< bvector<DPoint2d> > &boundaries,
bvector <double>    &uBreaks,
bvector <double>    &vBreaks,
double              maxEdgeXLength,
double              maxEdgeYLength,
double              meshXLength,
double              meshYLength,
bool                isometricGrid,
bool                laplacianSmoothing
)
    {
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
    if (isometricGrid)
        skewAngle = msGeomConst_pi / 6.0;
    DRange3d range = DRange3d::From (boundaries, 0.0);

    double xyTolerance = s_shortEdgeToleranceFactor * range.low.Distance (range.high);

    VuMask newNodeMask = VU_BOUNDARY_EDGE;
    bvector<DPoint3d> loop;
    for (size_t i = 0; i < boundaries.size (); i++)
        {
        loop.clear ();
        loop.reserve (boundaries[i].size ());
        for (size_t k = 0; k < boundaries[i].size (); k++)
            loop.push_back (DPoint3d::From (boundaries[i][k]));
        vu_makeIndexedLoopFromArray (graph, 
                const_cast <DPoint3d*> (&loop[0]),
                (int)loop.size (),
                newNodeMask, newNodeMask, xyTolerance);
        }
    double dx = range.high.x - range.low.x;
    double dy = range.high.y - range.low.y;
    if (meshXLength <= 0.0)
        meshXLength = dy / s_defaultCount;
    if (meshYLength <= 0.0)
        meshYLength = dy / s_defaultCount;

    if (s_maxEdge * meshXLength < dx)
        {
        status = ERROR;
        meshXLength = dx / s_maxEdge;
        }
    if (s_maxEdge * meshYLength < dy)
        {
        status = ERROR;
        meshYLength = dy / s_maxEdge;
        }
    
    DPoint3d origin = range.low;
    double ax0, ay0, ax1, ay1;
    DoubleOps::SafeDivide (ax0, range.low.x - origin.x, meshXLength, 0.0);
    DoubleOps::SafeDivide (ay0, range.low.y - origin.y, meshYLength, 0.0);
    int ix0 = (int) floor (ax0);
    int iy0 = (int) floor (ay0);

    DoubleOps::SafeDivide (ax1, range.high.x - origin.x, meshXLength, 0.0);
    DoubleOps::SafeDivide (ay1, range.high.y - origin.y, meshYLength, 0.0);
    int ix1 = (int)ceil (ax1);
    int iy1 = (int)ceil (ay1);

#ifdef DIRECT_GRID
    DSegment3d lineA, lineB;
    lineA.point[0].Init (range.low.x + ix0 * meshXLength, range.low.y + iy0 * meshYLength, range.low.z);
    lineA.point[1].Init (range.low.x + ix1 * meshXLength, range.low.y + iy0 * meshYLength, range.low.z);
    lineB.point[0].Init (range.low.x + ix0 * meshXLength, range.low.y + iy1 * meshYLength, range.low.z);
    lineB.point[1].Init (range.low.x + ix1 * meshXLength, range.low.y + iy1 * meshYLength, range.low.z);

    // Build non-boundary vertical linework of grid.
    for (int ix = ix0; ix <= ix1; ix++)
        {
        DPoint3d xyzA, xyzB;
        double fx = (double)(ix-ix0) / (double)(ix1- ix0);
        xyzA.Interpolate (lineA.point[0], fx, lineA.point[1]);
        xyzB.Interpolate (lineB.point[0], fx, lineB.point[1]);
        VuP nodeA, nodeB;
        vu_makePair (graph, &nodeA, &nodeB);
        vu_setDPoint3d (nodeA, &xyzA);
        vu_setDPoint3d (nodeB, &xyzB);
        for (int iy = iy0 + 1; iy < iy1; iy++)
            {
            double fy = (double)(iy - iy0) / (double)(iy1 - iy0);
            DPoint3d xyz;
            xyz.Interpolate (xyzA, fy, xyzB);
            VuP nodeC, nodeD;
            vu_splitEdge (graph, nodeB, &nodeC, &nodeD);
            vu_setDPoint3d (nodeC, &xyz);
            vu_setDPoint3d (nodeD, &xyz);
            }
        }
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);

    vu_splitLongEdges (graph, 0.0, maxEdgeXLength, maxEdgeYLength, s_maxSubEdge);

    vu_regularizeGraph (graph);
    vu_markAlternatingExteriorBoundaries(graph,true);
    vu_splitMonotoneFacesToEdgeLimit (graph, maxPerFace);
#else
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_deletePairedEdges (graph, VU_BOUNDARY_EDGE);
    if (0 != AddInteriorBreaks (graph, range, uBreaks, 0, VU_RULE_EDGE)
            + AddInteriorBreaks (graph, range, vBreaks, 1, VU_RULE_EDGE)
       )
        {
        // remerge with new edges ....
        vu_mergeOrUnionLoops (graph, VUUNION_UNION);
        }
    int numSplit = vu_splitLongEdges (graph, VU_BOUNDARY_EDGE | VU_RULE_EDGE, 0.0, maxEdgeXLength, maxEdgeYLength, s_maxSubEdge);
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
#endif
    if (maxPerFace == 3 && laplacianSmoothing)
        vu_flipTrianglesToImproveScaledQuadraticAspectRatio (graph, meshYLength, meshXLength);

    return graph;
    }

struct IndexDetail
  {
  size_t m_xyzIndex;
  size_t m_normalIndex;
  size_t m_paramIndex;
  IndexDetail (size_t xyzIndex, size_t paramIndex, size_t normalIndex)
      : m_xyzIndex(xyzIndex), m_normalIndex (normalIndex), m_paramIndex (paramIndex)
      {
      }
  };

static void Evaluate (MSBsplineSurfaceCR surface, VuP node, bool shiftToInterior, DPoint2dR uv, DRay3dR ray)
    {
    // B is the middle of 3 consecutive nodes A B C
    static double s_normalShiftEpsilon = 1.0e-7;
    DRay3d rayD;
    DPoint3d uvwA, uvwB, uvwC;
    vu_getDPoint3d (&uvwB, node);
    uv.x = uvwB.x;
    uv.y = uvwB.y;
    surface.EvaluatePointAndUnitNormal (ray, uv.x, uv.y);
    if (shiftToInterior)
        {
        vu_getDPoint3d (&uvwA, vu_fpred (node));
        vu_getDPoint3d (&uvwC, vu_fsucc (node));
        DPoint3d uvD = DPoint3d::FromSumOf (uvwB, 1.0 - 2.0 * s_normalShiftEpsilon,
              uvwA, s_normalShiftEpsilon,
              uvwC, s_normalShiftEpsilon);
        surface.EvaluatePointAndUnitNormal (rayD, uvD.x, uvD.y);
        ray.direction = rayD.direction;
        }
    }

void AddEvaluatedVuToPolyface (VuSetP graph, IPolyfaceConstructionR builder, MSBsplineSurfaceCR surface)
    {
    VuArrayP    faceArrayP = vu_grabArray (graph);
    VuP         faceP;

    bool needNormals = builder.GetFacetOptionsR ().GetNormalsRequired ();
    bool needParams = builder.GetFacetOptionsR ().GetParamsRequired ();

    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (currNode, graph)
        {
        vu_setUserDataPAsInt (currNode, -1);
        }
    END_VU_SET_LOOP (currNode, graph)
    size_t defaultIndex = SIZE_MAX;
    // Assign vertex numbers.
    bvector<IndexDetail> data;
    DRay3d ray;
    DPoint2d uv;
    VuMask fixedEdgeMask = VU_ALL_FIXED_EDGES_MASK;
    VU_SET_LOOP (currNode, graph)
        {
        if (!vu_getMask (currNode, VU_EXTERIOR_EDGE)
            && !vu_getMask (currNode, visitMask))
            {
            vu_setMaskAroundVertex (currNode, visitMask);
            VuP sectorSearchStart = vu_findMaskAroundVertex (currNode, fixedEdgeMask);
            if (nullptr == sectorSearchStart)
                {
                // smooth vertex -- same normals everywhere.
                Evaluate (surface, currNode, false, uv, ray);
                int dataIndex = (int)data.size ();
                data.push_back (
                  IndexDetail (
                      builder.FindOrAddPoint (ray.origin),
                      needParams  ? builder.FindOrAddParam (uv) : defaultIndex,
                      needNormals ? builder.FindOrAddNormal (ray.direction) : defaultIndex
                      ));

                VU_VERTEX_LOOP (vertexNode, currNode)
                    {
                    vu_setUserDataPAsInt (vertexNode, dataIndex);
                    }
                END_VU_VERTEX_LOOP (vertexNode, currNode);
                }
            else
                {
                // Evaluate independently in each sector bounded by fixed edges ...
                // any interior fixed edge triggers a normal evaluation.
                // apply that normal forward to the subsequent fixed edge (and there is always at least one !!)
                VU_VERTEX_LOOP (breakNode, sectorSearchStart)
                    {
                    if (!vu_getMask (breakNode, VU_EXTERIOR_EDGE) && vu_getMask (breakNode, fixedEdgeMask))
                        {
                        int dataIndex = (int)data.size ();
                        Evaluate (surface, breakNode, true, uv, ray);

                        data.push_back (
                          IndexDetail (
                              builder.FindOrAddPoint (ray.origin),
                              needParams  ? builder.FindOrAddParam (uv) : defaultIndex,
                              needNormals ? builder.FindOrAddNormal (ray.direction) : defaultIndex
                              ));
                        size_t count = 0;
                        VU_VERTEX_LOOP (sectorNode, breakNode)
                            {
                            if (count++ > 0 && vu_getMask (sectorNode, fixedEdgeMask))
                                break;
                            vu_setUserDataPAsInt (sectorNode, dataIndex);
                            }
                        END_VU_VERTEX_LOOP (sectorNode, breakNode)
                        }
                    }
                END_VU_VERTEX_LOOP (breakNode, sectorSearchStart)
                }
            }
        }
    END_VU_SET_LOOP (currNode, graph)
    vu_collectInteriorFaceLoops (faceArrayP, graph);
    vu_arrayOpen (faceArrayP);
    static int s_skipStrangeFaces = 0;
    for (int i = 0; vu_arrayRead (faceArrayP, &faceP); i++)
        {
        // We triangulated.  So of course there are 3 nodes per face.
        // Really?  If the input polygon retraces itself, there will be
        // sliver faces with only 2 edges.  
        if (s_skipStrangeFaces != 0 && vu_faceLoopSize (faceP) > 4)
            continue;
            
        VU_FACE_LOOP (currP, faceP)
            {
            int dataIndex = vu_getUserDataPAsInt (currP);
            bool visible = 0 != vu_getMask (vu_edgeMate (currP), VU_EXTERIOR_EDGE | VU_RULE_EDGE);

            builder.AddPointIndex (data[dataIndex].m_xyzIndex, visible);
            if (needNormals)
              builder.AddNormalIndex (data[dataIndex].m_normalIndex);
            if (needParams)
              builder.AddParamIndex (data[dataIndex].m_paramIndex);

            }
        END_VU_FACE_LOOP (currP, faceP)
        builder.AddPointIndexTerminator ();
        if (needNormals)
            builder.AddNormalIndexTerminator ();
        if (needParams)
            builder.AddParamIndexTerminator ();
        }

    vu_returnMask (graph, visitMask);
    vu_returnArray (graph, faceArrayP);
    }

static size_t s_minOnEdge = 4;
static size_t s_maxOnEdge = 400;
static double s_smallDistanceFraction = 1.0e-6;
void SetParameterStep (
    MSBsplineSurfaceCR surface,
    IFacetOptionsR options,
    double meanEdgeLength,
    double meanEdgeFactor,      // allow split edges up to this factor of meanEdgeLength
    bvector<double> length,
    bvector<double> turn,
    size_t &numEdge,
    double &averageDistance,
    double &totalDistance
    )
    {
    totalDistance = 0.0;
    numEdge = s_minOnEdge;
    for (size_t i = 0; i < length.size (); i++)
        totalDistance += length[i];
    double smallDistance = s_smallDistanceFraction * totalDistance;
    for (size_t i = 0; i < length.size (); i++)
        {
        double myTurn = turn[i];
        // At singular point, angles are bogus -- suppress angle effect
        if (length[i] < smallDistance)
            myTurn = 0.0;
        size_t num1 = options.DistanceAndTurnStrokeCount (length[i], myTurn);
        if (num1 > numEdge)
            numEdge = num1;
        }
    if (meanEdgeLength > 0.0)
        {
        double q;
        if (DoubleOps::SafeDivide(q, totalDistance, meanEdgeLength * meanEdgeFactor, (double)s_maxOnEdge))
            {
            size_t num1 = (size_t)q;
            if (num1 > numEdge)
                numEdge = num1;
            }
        }
    if (numEdge > s_maxOnEdge)
        numEdge = s_maxOnEdge;
    averageDistance = totalDistance / length.size ();
    }
 
// Extract the high multiplicity knots from knotData.
static void CollectHighMultiplicityKnots
(
KnotData &knotData,
bvector<double> &breaks,
size_t excessMultiplicity,   // When multiplicity + excessMultiplicity >= order force an edge
bool   observeDegree = true // true to reduce breakMultiplicity to match degree
)
    {
    breaks.clear ();
    double value;
    size_t multiplicity;
    for (size_t activeIndex = 0; knotData.GetKnotByActiveIndex (activeIndex, value, multiplicity); activeIndex++)
        {
        if (multiplicity + excessMultiplicity >= knotData.order)
            breaks.push_back (value);
        }
    }
static double s_xySize = 0.04;
static double s_edgeXSizeFactor = 0.95;
static double s_edgeYSizeFactor = 0.95;
static double s_meshXSizeFactor = 0.95;
//static double s_meshYSizeFactor = 0.95;
static double s_baseFactor = 1.0;
static double s_power = 0.0;
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddSmoothed (MSBsplineSurfaceCR surface)
    {
    bvector <bvector<DPoint2d> > uvBoundaries;
    surface.GetUVBoundaryLoops (uvBoundaries, true);
    KnotData uKnots, vKnots;
    uKnots.LoadSurfaceUKnots (surface);
    vKnots.LoadSurfaceVKnots (surface);
    bvector <double> uBreaks, vBreaks;
    static int s_excessMultiplicity = 1;
    CollectHighMultiplicityKnots (uKnots, uBreaks, s_excessMultiplicity, true);
    CollectHighMultiplicityKnots (vKnots, vBreaks, s_excessMultiplicity, true);
    bvector<double> uLength;
    bvector<double> vLength;
    bvector<double> uTurn;
    bvector<double> vTurn;
    bspmesh_calculateControlPolygonLengthsAndTurn (surface,
                uLength, uTurn,
                vLength, vTurn);
    double xSize, ySize;    // parametric chord size
    double xLength, yLength;    // physical total length
    double averageXBezierLength, averageYBezierLength;  // averages per bezier span
    size_t numXStep, numYStep;

    static double s_meanEdgeFactor = 5.0;       // Keep mesh edges less than this factor time mean
    // first round SetParameterStep gets length sums ...
    SetParameterStep (surface, GetFacetOptionsR (), 0.0, s_meanEdgeFactor, uLength, uTurn, numXStep, averageXBezierLength, xLength);
    SetParameterStep (surface, GetFacetOptionsR (), 0.0, s_meanEdgeFactor, vLength, vTurn, numYStep, averageYBezierLength, yLength);

    UsageSums allX, allY, strongX, strongY;
    static double s_zeroUV = 1.0e-7;
    static double s_slopeLimit = 2.0;
    double xSlopeLimit = s_slopeLimit * xLength / yLength;
    double ySlopeLimit = s_slopeLimit * yLength / xLength;
    double s_bigUV = 0.5;
    ComputeDirectionalPropertiesOfPolygons(uvBoundaries, false, s_zeroUV, s_zeroUV, s_bigUV, s_bigUV, xSlopeLimit, ySlopeLimit, allX, allY, strongX, strongY);
    double meanDX = strongX.Mean() * xLength;
    double meanDY = strongY.Mean() * yLength;

    SetParameterStep(surface, GetFacetOptionsR(), meanDX, s_meanEdgeFactor, uLength, uTurn, numXStep, averageXBezierLength, xLength);
    SetParameterStep(surface, GetFacetOptionsR(), meanDY, s_meanEdgeFactor, vLength, vTurn, numYStep, averageYBezierLength, yLength);

    // ASSUME count and length are both nonzero ...
    // Adjust for approximate overall length to get simliar step in each direction ...
    double hx = xLength / numXStep;         // Apparent physical sizes in each direction.
    double hy = yLength / numYStep;
    xSize = 1.0 / numXStep;
    ySize = 1.0 / numYStep;
    if (hx > hy)                            // need more x steps
        xSize *= hy / hx;
    else
        ySize *= hx / hy;
    
    VuSetP parametricMesh =  vu_createTriangulated (uvBoundaries, 
                uBreaks, vBreaks,
                xSize * s_edgeXSizeFactor, ySize * s_edgeYSizeFactor,
                xSize * s_meshXSizeFactor, ySize * s_meshXSizeFactor,
                true, true);

    //static double s_symmetryFactor = 0.75;
    static double s_smoothTol = 0.001;
    static int s_maxSmoothPass = 100;
    static int s_flipInterval = 4;
    static int s_maxFlipsPerEdgePerPass = 5;
    double shiftFraction;
    int numSweep;
    static int s_defaultRefinementSweeps  = 1;
    static int s_doParametricFlip = 0;
    //static int s_flipTriangles = 1;
    static double s_edgeLengthFactor = 1.0;
    int numRefinementSweep = s_defaultRefinementSweeps;// GetFacetOptionsR ().GetNumRefinementSweeps ()
    if (numRefinementSweep > 0)
      {
      for (int sweep = 0; sweep < numRefinementSweep; sweep++)
          {
          MSBsplineSurfaceRefinementFunction refinementObject (surface,
                  GetFacetOptionsR ().GetChordTolerance (),
                  GetFacetOptionsR ().GetAngleTolerance (),
                  GetFacetOptionsR ().GetMaxEdgeLength () * s_edgeLengthFactor
                  );
          size_t numSplit = vu_refineSurface (parametricMesh, refinementObject);
          if (sweep > 0 && numSplit == 0)   // Force smoothing after first pass, not later.
              break;
          if (GetFacetOptionsR ().GetDoSpatialLaplaceSmoothing ())
              {
              s_qMax = s_rMax = 0.0;
              VuMSBsplineSurfaceWeightFunction weightObject (surface, s_baseFactor / (s_xySize * s_xySize), s_power);
              vu_smoothInteriorVertices (parametricMesh, &weightObject, &weightObject, s_smoothTol, s_maxSmoothPass,s_flipInterval, s_maxFlipsPerEdgePerPass, &shiftFraction, &numSweep);
              }
          else if (s_doParametricFlip)
            vu_flipTrianglesToImproveScaledQuadraticAspectRatio (parametricMesh, 1.0/ xLength, 1.0/ yLength);  

          if (numSplit == 0)
              break;
          }
      }        
    AddEvaluatedVuToPolyface (parametricMesh, *this, surface);
    vu_freeVuSet (parametricMesh);

    }
END_BENTLEY_GEOMETRY_NAMESPACE
