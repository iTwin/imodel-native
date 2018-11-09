/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_undercut.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_undercutLocalRelTol = 1.0e-14;
void CompressCyclicPointsAndZingers (bvector<DPoint3d> &points, double tolerance)
    {
    DPoint3dOps::Compress (points, tolerance);
    while (points.size () > 1 && points.front ().Distance (points.back ()) < tolerance)
        points.pop_back ();
    double angleTol = Angle::SmallAngle ();
    size_t n = points.size ();
    for (size_t i1 = 0; i1 < n; i1++)
        {
        size_t i0 = i1 == 0 ? n - 1: i1 - 1;
        size_t i2 = (i1 + 1) % n;
        auto xyz0 = points[i0];
        auto xyz1 = points[i1];
        auto xyz2 = points[i2];
        // look from i1 towards each neighbor ..
        auto vector10 = DVec3d::FromStartEnd (xyz1, xyz0);
        auto vector12 = DVec3d::FromStartEnd (xyz1, xyz2);
        if (fabs (vector10.AngleToXY (vector12)) < angleTol)
            {
            points.erase (points.begin () + i1);
            n--;
            }
        }
    }
void PolygonVectorOps__ComputeUndercut_direct
(
TaggedPolygonVectorCR sourceA,
TaggedPolygonVectorCR sourceB,
TaggedPolygonVectorR surfaceAaboveB,
TaggedPolygonVectorR surfaceBbelowA
)
    {
    surfaceAaboveB.clear ();
    surfaceBbelowA.clear ();
    DRange3d totalRangeA = PolygonVectorOps::GetRange (sourceA);
    DRange3d totalRangeB = PolygonVectorOps::GetRange (sourceB);
    //double abstol = s_relTol * rangeA.LargestCoordinate ();
    bvector<ValidatedDRange3d> allRangeA;
    size_t nA = sourceA.size ();
    size_t nB = sourceB.size ();
    double pointTolerance = DoubleOps::SmallCoordinateRelTol () * totalRangeA.LargestCoordinateXY ();
    bvector<size_t> activeIndexA;
    for (size_t iA = 0;iA < nA; iA++)
        {
        DRange3d range;
        DPlane3d plane;
        if (CutAndFillSplitter::GetNonVerticalPolygonPlane (sourceA[iA], plane, range))
            {
            range.low.z = -DBL_MAX;     // catch intersections with all below
            allRangeA.push_back (ValidatedDRange3d (range, 
                    range.IntersectsWith (totalRangeB)
                    ));
            activeIndexA.push_back (iA);
            }
        else
            allRangeA.push_back (ValidatedDRange3d (DRange3d::NullRange (), false));
        }

    ConvexClipPlaneSet clipB;
    bvector<DPoint3d> xyzAClip, xyzWork;
    DVec3d upZ = DVec3d::From (0,0,1);
    SmallIntegerHistogram counts (20);
    for (size_t iB = 0; iB < nB; iB++)
        {
        counts.Record (0);
        DRange3d rangeB;
        DPlane3d planeB;
        bvector<DPoint3d> const &xyzB = sourceB[iB].GetPointsCR ();
        if (  CutAndFillSplitter::GetNonVerticalPolygonPlane (sourceB[iB], planeB, rangeB)
            && clipB.ReloadSweptConvexPolygon (xyzB, upZ, true)
            )
            {
            counts.Record (1);
            for (size_t iA : activeIndexA)
                {
                counts.Record (2);
                if (allRangeA[iA].IsValid () && allRangeA[iA].Value ().IntersectsWith (rangeB))
                    {
                    clipB.ConvexPolygonClip (sourceA[iA].GetPointsCR (), xyzAClip, xyzWork);
                    counts.Record(3);
                    if (xyzAClip.size () > 0)
                        {
                        counts.Record(4);
                        CompressCyclicPointsAndZingers (xyzAClip, pointTolerance);
                        PolygonVectorOps::AddPolygon (surfaceAaboveB, xyzAClip);
                        xyzWork.clear ();
                        // project in reverse order to plane of B
                        for (size_t kA = xyzAClip.size (); kA-- > 0;)
                            {
                            double fB;
                            DPoint3d xyzBi;
                            DRay3d ray = DRay3d::FromOriginAndVector (xyzAClip[kA], upZ);
                            if (ray.Intersect (xyzBi, fB, planeB))
                                xyzWork.push_back (xyzBi);
                            }
                        PolygonVectorOps::AddPolygon (surfaceBbelowA, xyzWork);
                        }
                    }
                }
            }
        }
    }



// test condition for distinguishing barrier polygons.
void PolyfaceQuery::ComputeUndercut
(
PolyfaceHeaderCR polyfaceA,
IPolyfaceVisitorFilter *filterA,
PolyfaceHeaderCR polyfaceB,
IPolyfaceVisitorFilter *filterB,
PolyfaceHeaderPtr &undercutPolyface
)
    {
    undercutPolyface = nullptr;
    DRange3d rangeA = polyfaceA.PointRange ();
    DRange3d rangeB = polyfaceB.PointRange ();

    if (rangeA.high.z < rangeB.low.z)
        return;

    DRange3d inputRange;
    inputRange.UnionOf (rangeA, rangeB);

    TaggedPolygonVector polygonA, polygonB ;

    polyfaceA.AddToTaggedPolygons (polygonA, 0, 0, filterA);
    polyfaceB.AddToTaggedPolygons (polygonB, 0, 0, filterB);

    double planarityAbsTol = s_undercutLocalRelTol * inputRange.low.Distance (inputRange.high);
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    Transform localToWorld, worldToLocal;
    localToWorld.InitFrom (rangeCenter);
    worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);

    PolygonVectorOps::Multiply (polygonA, worldToLocal);
    PolygonVectorOps::Multiply (polygonB, worldToLocal);

    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.

    PolygonVectorOps::TriangulateNonPlanarPolygons (polygonA, planarityAbsTol);
    PolygonVectorOps::TriangulateNonPlanarPolygons (polygonB, planarityAbsTol);
    bvector <TaggedPolygonVector> debugPolygons;
    TaggedPolygonVector out1, out2;
    PolygonVectorOps__ComputeUndercut_direct (polygonA, polygonB, out1, out2);

    PolygonVectorOps::Multiply (out1, localToWorld);
    PolygonVectorOps::Multiply (out2, localToWorld);
    bvector<PolyfaceHeaderPtr> result;
    SavePolygons (result, out1, &out2); // We know (really?) that there will only be one polyface back.
    if (result.size () > 0)
        undercutPolyface = result[0];
    }

// test condition for distinguishing barrier polygons.
void PolyfaceQuery::ComputeOverAndUnderXY
(
PolyfaceHeaderCR polyfaceA,
IPolyfaceVisitorFilter *filterA,
PolyfaceHeaderCR polyfaceB,
IPolyfaceVisitorFilter *filterB,
PolyfaceHeaderPtr &polyfaceAOverB,
PolyfaceHeaderPtr &polyfaceBUnderA,
bool computeAndApplyTransform
)
    {
    polyfaceAOverB = nullptr;
    polyfaceBUnderA = nullptr;
    DRange3d rangeA = polyfaceA.PointRange ();
    DRange3d rangeB = polyfaceB.PointRange ();

    if (rangeA.high.z < rangeB.low.z)
        return;

    DRange3d inputRange;
    inputRange.UnionOf (rangeA, rangeB);

    TaggedPolygonVector polygonA, polygonB ;

    polyfaceA.AddToTaggedPolygons (polygonA, 0, 0, filterA);
    polyfaceB.AddToTaggedPolygons (polygonB, 0, 0, filterB);
    double planarityAbsTol = s_undercutLocalRelTol * inputRange.low.Distance (inputRange.high);
    DPoint3d rangeCenter;
    rangeCenter.Interpolate (inputRange.low, 0.5, inputRange.high);

    Transform localToWorld, worldToLocal;
    if (computeAndApplyTransform)
        {
        localToWorld.InitFrom (rangeCenter);
        worldToLocal.InitFrom (-rangeCenter.x, -rangeCenter.y, -rangeCenter.z);
        }
    else
        {
        localToWorld.InitIdentity ();
        worldToLocal.InitIdentity ();
        }

    PolygonVectorOps::Multiply (polygonA, worldToLocal);
    PolygonVectorOps::Multiply (polygonB, worldToLocal);

    worldToLocal.Multiply (inputRange, inputRange);    // worldToLocal is just translation, so this is exact.

    PolygonVectorOps::TriangulateNonPlanarPolygons (polygonA, planarityAbsTol);
    PolygonVectorOps::TriangulateNonPlanarPolygons (polygonB, planarityAbsTol);
    bvector <TaggedPolygonVector> debugPolygons;
    if (&polyfaceAOverB == & polyfaceBUnderA)
        {
        TaggedPolygonVector out1;
        PolygonVectorOps__ComputeUndercut_direct (polygonA, polygonB, out1, out1);
        PolygonVectorOps::Multiply (out1, localToWorld);
        bvector<PolyfaceHeaderPtr> result;
        SavePolygons (result, out1, nullptr);
        polyfaceAOverB = result[0];
        }
    else
        {
        TaggedPolygonVector out1, out2;
        PolygonVectorOps__ComputeUndercut_direct (polygonA, polygonB, out1, out2);

        PolygonVectorOps::Multiply (out1, localToWorld);
        PolygonVectorOps::Multiply (out2, localToWorld);
        bvector<PolyfaceHeaderPtr> result;

        SavePolygons (result, out1, nullptr); // We know (really?) that there will only be at most polyface back.
        if (result.size () > 0)
            polyfaceAOverB = result[0];
        result.clear ();
        SavePolygons(result, out2, nullptr);
        if (result.size () > 0)
            polyfaceBUnderA = result[0];
        }
    }


/*---------------------------------------------------------------------------------**//**
        ShardHealer bodies
+----------------------------------------------------------------------*/
static const int s_nullIndex = -1;

ShardHealer::ShardHealer ()
    {
    m_graph = vu_newVuSet (0);
    vu_setDefaultUserData1 (m_graph, -1, false, false);
    }
ShardHealer::~ShardHealer ()
    {
    vu_freeVuSet (m_graph);
    }

void ShardHealer::SetOriginalXYZ (bvector<DPoint3d> const &xyz)
    {
    m_originalXYZ = xyz;
    m_vertexTolerance = DPoint3dOps::LargestCoordinate (xyz) * Angle::SmallAngle ();
    }

int ShardHealer::FindOriginalIndex (DPoint3dCR xyz)
    {
    size_t closestIndex;
    double d;
    if (DPoint3dOps::ClosestPointXY (m_originalXYZ, nullptr, xyz, closestIndex, d)
        && d <= m_vertexTolerance)
        {
        return (int)closestIndex;
        }
    return -1;
    }

bool ShardHealer::SwapExteriorMasksToNullFaces (VuMask exteriorMask)
    {
    //vu_printFaceLabels (m_graph, "Before swap");
    VU_SET_LOOP (nodeA, m_graph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        VuP nodeC = vu_fsucc (nodeB);
        if (nodeB != nodeA && nodeC == nodeA)
            {
            VuP nodeA1 = vu_edgeMate (nodeA);
            VuP nodeB1 = vu_edgeMate (nodeB);
            if (vu_getMask (nodeA1, exteriorMask)
                && vu_getMask (nodeB1, exteriorMask)
                && !vu_getMask (nodeA, exteriorMask)
                && !vu_getMask (nodeB, exteriorMask)
                )
                {
                VuP nodeA2 = vu_fsucc (nodeA1);
                VuP nodeB2 = vu_fsucc (nodeB1);
                vu_vertexTwist (m_graph, nodeB, nodeA1); // yank far end of A 
                vu_vertexTwist (m_graph, nodeA, nodeA2); // yank A
                vu_vertexTwist (m_graph, nodeA, nodeB1);    // reinsert on the other side
                vu_vertexTwist (m_graph, nodeA1, nodeB2);    // reinsert on the other side
      //          vu_printFaceLabels (m_graph, "Before after swap");
                }
            }
        }
    END_VU_SET_LOOP (nodeB, m_graph)
    return true;
    }

bool ShardHealer::SetupInteriorFaceNumbers (VuMask exteriorMask)
    {

    m_faceClusters.clear ();
    // confirm that all exterior masks match ..
    VU_SET_LOOP (nodeA, m_graph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        if (vu_getMask (nodeA, VU_EXTERIOR_EDGE) != vu_getMask (nodeB, VU_EXTERIOR_EDGE))
            return false;
        }
    END_VU_SET_LOOP (nodeB, m_graph)

    VU_SET_LOOP (nodeA, m_graph)
        {
        vu_setUserData1 (nodeA, s_nullIndex);
        }
    END_VU_SET_LOOP (nodeB, m_graph)

    VU_SET_LOOP (nodeA, m_graph)
        {
        if (!vu_getMask (nodeA, exteriorMask))
            {
            // enter this as a new face
            size_t clusterIndex = UnionFind::NewClusterIndex (m_faceClusters);
            if (vu_getUserData1 (nodeA) == s_nullIndex)
                {
                VU_FACE_LOOP (nodeB, nodeA)
                    vu_setUserData1 (nodeB, (ptrdiff_t)clusterIndex);
                END_VU_FACE_LOOP (nodeB, nodeA)
                }
            }
        }
    END_VU_SET_LOOP (nodeB, m_graph)
    return true;
    }
bool SameClusterAroundVertex (VuP nodeA, bvector<size_t > &faceClusters, VuMask exteriorMask)
    {
    size_t clusterA = UnionFind::FindClusterRoot (faceClusters, (size_t)vu_getUserData1 (nodeA));
    VU_VERTEX_LOOP (nodeB, nodeA)
        {
        VuP nodeC = vu_fsucc(nodeB);
        if (vu_getMask (nodeB, exteriorMask))
            {
            if (vu_fsucc (nodeC ) != nodeB)
                return false;           // nontrivial exterior face
            }
        else
            {
            size_t clusterB = UnionFind::FindClusterRoot (faceClusters, (size_t)vu_getUserData1 (nodeB));
            if (clusterB != clusterA)
                return false;
            }
        }
    END_VU_VERTEX_LOOP (nodeB, nodeA)
    return true;
    }
size_t ShardHealer::MergeFacesAcrossNullFaces
(
VuMask exteriorMask,
VuMask mergeMask        // apply to all 4 edges of deletable pair
)
    {
    size_t numMerge = 0;
    vu_clearMaskInSet (m_graph, mergeMask);
    VU_SET_LOOP (nodeA, m_graph)
        {
        ptrdiff_t clusterA = vu_getUserData1 (nodeA);
        if (clusterA != s_nullIndex
            && !vu_getMask (nodeA, exteriorMask)
            )
            {
            VuP nodeB = vu_edgeMate (nodeA);
            VuP nodeC = vu_fsucc (nodeB);
            VuP nodeD = vu_edgeMate (nodeC);
            if (vu_getMask (nodeB, exteriorMask)
                && vu_getMask (nodeC, exteriorMask)
                && vu_fsucc (nodeC) == nodeB
                && !vu_getMask (nodeD, exteriorMask)
                )
                {
                ptrdiff_t clusterD = vu_getUserData1 (nodeD);
                if (clusterD != s_nullIndex)
                    {
                    size_t parentA = UnionFind::FindClusterRoot (m_faceClusters, (size_t)clusterA);
                    size_t parentD = UnionFind::FindClusterRoot (m_faceClusters, (size_t)clusterD);
                    bool merge = parentA != parentD;
                    if (parentA == parentD)
                        {
                        static int s_checkVertex = 1;
                        if (s_checkVertex)
                            {
                            if (SameClusterAroundVertex (nodeA, m_faceClusters, exteriorMask))
                                merge = true;
                            if (SameClusterAroundVertex (nodeD, m_faceClusters, exteriorMask))
                                merge = true;
                            }
                        }
                    if (merge)
                        {
                        // the parents are distinct faces.
                        // merge them and mark the edges:
                        UnionFind::MergeClusters (m_faceClusters, parentA, parentD);
                        numMerge++;
                        vu_setMask (nodeA, mergeMask);
                        vu_setMask (nodeB, mergeMask);
                        vu_setMask (nodeC, mergeMask);
                        vu_setMask (nodeD, mergeMask);
                        }
                    }
                }
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    return numMerge;
    }

void ShardHealer::AnnotateSuccessorIfTurnOrOriginal (VuP nodeA, VuMask mask)
    {
    static int s_acceptAll = false;
    if (s_acceptAll)
        {
        vu_setMask (nodeA, mask);
        return;
        }
    VuP nodeB = vu_fsucc (nodeA);
    DPoint3d xyzB;
    vu_getDPoint3d (&xyzB, nodeB);
    DVec3d vectorAB, vectorBC;
    vu_getDPoint3dDXY (&vectorAB, nodeA);
    vu_getDPoint3dDXY (&vectorBC, nodeB);
    vu_setMask (nodeB, mask);
    if (/* vectorAB.DotProduct (vectorBC) > 0.0 && */vectorAB.IsParallelTo (vectorBC))
        {
        if (FindOriginalIndex (xyzB) < 0)
            vu_clrMask (nodeB, mask);
        }
    }

bool ShardHealer::HealShards (BVectorCache<DPoint3d> &shards, bvector<DPoint3d> &originalXYZ)
    {
    SetOriginalXYZ (originalXYZ);
    vu_reinitializeVuSet (m_graph);
    for (auto &shard : shards)
        {
        CompressCyclicPointsAndZingers (shard, m_vertexTolerance);
        size_t n = shard.size ();
        if (0 == n)
            continue;

        m_loopIndex.clear ();
        m_loopXYZ.clear ();
        for (size_t i = 0; i < n; i++)
            {
            int originalIndex = FindOriginalIndex (shard[i]);
            m_loopIndex.push_back (originalIndex);
            m_loopXYZ.push_back (shard[i]);
            }
        double area = PolygonOps::AreaXY (shard);
        DPoint3d xyz0 = m_loopXYZ.front ();
        m_loopIndex .push_back (m_loopIndex[0]);
        m_loopXYZ.push_back (xyz0);

        for (size_t iA = 0; iA < n; iA++)
            {
            size_t iB = iA + 1; // safe to wrap !!!
            VuP nodeA, nodeB;
            vu_makePair (m_graph, &nodeA, &nodeB);
            vu_setDPoint3d (nodeA, &m_loopXYZ[iA]);
            vu_setDPoint3d (nodeB, &m_loopXYZ[iB]);
            vu_setUserDataPAsInt (nodeA, (int)m_loopIndex[iA]);
            vu_setUserDataPAsInt (nodeB, (int)m_loopIndex[iB]);
            if (area > 0)
                {
                vu_setMask (nodeA, VU_BOUNDARY_EDGE); 
                vu_setMask (nodeB, VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE);
                }
            else
                {
                vu_setMask (nodeA, VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE);
                vu_setMask (nodeB, VU_BOUNDARY_EDGE); 
                }
            }
        }
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    if (!SwapExteriorMasksToNullFaces (VU_EXTERIOR_EDGE))
        return false;
    if (!SetupInteriorFaceNumbers (VU_EXTERIOR_EDGE))
        return false;
    if (MergeFacesAcrossNullFaces (VU_EXTERIOR_EDGE, VU_RULE_EDGE) > 0)
        {
        // boom -- delete the edges.
        vu_freeMarkedEdges (m_graph, VU_RULE_EDGE);
        shards.ClearToCache ();
        vu_clearMaskInSet (m_graph, VU_RULE_EDGE);
        VU_SET_LOOP (nodeA, m_graph)
            {
            if (!vu_getMask (nodeA, VU_EXTERIOR_EDGE) && !vu_getMask (nodeA, VU_RULE_EDGE))
                {
                // put VU_AT_VERTEX on (a) any node with a turn and (b) any original node
                VU_FACE_LOOP (nodeB, nodeA)
                    {
                    AnnotateSuccessorIfTurnOrOriginal (nodeB, VU_AT_VERTEX);
                    }
                END_VU_FACE_LOOP (nodeB, nodeA)
                m_loopXYZ.clear ();
                vu_setMaskAroundFace (nodeA, VU_RULE_EDGE);
                VU_FACE_LOOP (nodeB, nodeA)
                    {
                    if (vu_getMask (nodeB, VU_AT_VERTEX))
                        {
                        DPoint3d xyz;
                        vu_getDPoint3d (&xyz, nodeB);
                        m_loopXYZ.push_back (xyz);
                        }
                    }
                END_VU_FACE_LOOP (nodeB, nodeA)
                shards.PushCopy (m_loopXYZ);
                }
            }
        END_VU_SET_LOOP (nodeA, m_graph)
        }

    return true;
    }

struct ClipperData
{
size_t m_parentIndex;
DRange3d m_range;
ConvexClipPlaneSet m_convexClip; 
ClipPlaneSet m_clipChain;       //  non convex !!!
ClipperData (){}

};

void PolygonVectorOps::PunchByPlaneSets
(
PolyfaceQueryCR mesh,
TransformCR meshToPunch,       // applied to mesh only !!
TaggedPolygonVectorCR punchPolygons,
TaggedPolygonVector* insidePolygons,
TaggedPolygonVector* outsidePolygons,
TaggedPolygonVector* debugPolygons
)
    {
    UsageSums facetPoints;
    UsageSums clipperPoints;
    SmallIntegerHistogram counters (20);
    SmallIntegerHistogram activeClips (20);
    SmallIntegerHistogram exteriorShards(20);
    StopWatch loadTime;
    StopWatch totalTime;
    StopWatch clipTime;
    totalTime.Start ();
    loadTime.Start ();
    if (nullptr != outsidePolygons)
        outsidePolygons->clear ();
    if (nullptr != insidePolygons)
        insidePolygons->clear ();
    DRange3d meshRange = mesh.PointRange ();//PolygonVectorOps::GetRange (meshPolygons);
    DRange3d punchRange = PolygonVectorOps::GetRange (punchPolygons);

    DVec3d debugStep = DVec3d::From (
            DoubleOps::Max (meshRange.high.x - meshRange.low.x, punchRange.high.x - punchRange.low.x),
            0,
            0
            );
    double numDebugStep = 0.0;


    //double abstol = s_relTol * rangeA.LargestCoordinate ();
    //size_t numFacet = meshPolygons.size ();
    //size_t numPunch = punchPolygons.size ();
    double pointTolerance = DoubleOps::SmallCoordinateRelTol () * meshRange.LargestCoordinateXY ();
    static double s_planeToleranceFactor = 0.01;
    double planeTolerance = s_planeToleranceFactor * pointTolerance;    // tighter tolerance for on plane decision
    // cache ranges for all the facets ....

    // cache clip plane sets for all the clippers ...
    bvector<DPoint3d> hullPoints;
    bvector<ClipperData> clippers;
    bvector<bool> interior;
    for (size_t i = 0; i < punchPolygons.size (); i++)
        {
        bvector<DPoint3d> const &points = punchPolygons[i].GetPointsCR ();
        clipperPoints.Accumulate (points.size ());
        if (points.size () > 2)
            {
            int direction = bsiGeom_testXYPolygonConvex (&points[0], (int)points.size ());
            if (direction > 0)
                {
                hullPoints = points;
                }
            else if (direction < 0)
                {
                hullPoints = points;
                std::reverse (hullPoints.begin (), hullPoints.end ());
                }
            else
                {
                hullPoints.resize (points.size () + 10);
                int numOut;
                bsiDPoint3dArray_convexHullXY (&hullPoints[0], &numOut,
                        (DPoint3dP)&points[0], (int)points.size ());
                hullPoints.resize ((size_t)numOut);
                }
            if (hullPoints.size () > 2)
                {
                double areaXY = PolygonOps::AreaXY (hullPoints);
                interior.clear ();
                // add closure point !!!
                DPoint3d xyz = hullPoints[0];
                hullPoints.push_back (xyz);
                clippers.push_back (ClipperData ());

                clippers.back ().m_range = DRange3d::From (hullPoints);

                if (direction != 0)
                    {
                    for (size_t i = 0; i < hullPoints.size (); i++)
                        interior.push_back (true);
                    clippers.back ().m_convexClip = ConvexClipPlaneSet::FromXYPolyLine (hullPoints, interior ,areaXY > 0.0);
                    }
                else
                    {
                    bvector<bvector<DPoint3d>> shapes;
                    clippers.back ().m_clipChain = ClipPlaneSet::FromSweptPolygon (&points[0], points.size (), nullptr, &shapes);
                    if (debugPolygons != nullptr)
                        {
                        numDebugStep++;
                        for (auto &shape : shapes)
                            AddTransformedPolygon (*debugPolygons, shape, numDebugStep * debugStep, 3);
                        }
                    }
                }
            }
        }
    loadTime.Stop ();
    clipTime.Start ();
    size_t numClipper = clippers.size ();
    bvector<DPoint3d> currentShard, xyzWork1, xyzWork2;
    BVectorCache <DPoint3d> shards[2];
    BVectorCache <DPoint3d> insideShards;
    auto visitor = PolyfaceVisitor::Attach (mesh);
    bvector<DPoint3d> &visitorPoints = visitor->Point ();
    static size_t s_minOutside = 0;     // normally allow everything through.   set to 2 in debugger to trap strange cases
    ShardHealer healer;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        meshToPunch.Multiply (visitorPoints, visitorPoints);
        DRange3d facetRange = DRange3d::From (visitorPoints);
        DVec3d debugYShift = DVec3d::From (0, facetRange.high.y - facetRange.low.y, 0);
        facetPoints.Accumulate (visitorPoints.size ());
        counters.Record (0);
        if (!facetRange.IntersectsWith (punchRange, 2))
            {
            // accept the whole facet.
            if (nullptr != outsidePolygons
                && s_minOutside <= 1        // choke of simple cases for debugging.
                )
                AddPolygon (*outsidePolygons, visitorPoints, 3);
            counters.Record (1);
            continue;
            }
        shards[0].ClearToCache ();
        shards[0].PushCopy (visitorPoints);
        shards[1].ClearToCache ();
        insideShards.ClearToCache ();
        size_t shard0 = 0;  // shards[shard0] contains shards available for more clips.
        size_t shard1 = 1;  // shards[shard1] contains shards thrwn out as external to the current clipper.  These become shard0 for next round (or save at end)

        for (size_t clipperIndex = 0; clipperIndex < numClipper && !shards[shard0].empty ();
            clipperIndex++)
            {
            ClipperData &clipper = clippers.at(clipperIndex);
            if (!facetRange.IntersectsWith (clipper.m_range, 2))
                {
                counters.Record (2);
                continue;
                }
            if (clipper.m_convexClip.size () > 0)
                {
                shards[shard1].ClearToCache ();
                while (!shards[shard0].empty ())
                    {
                    shards[shard0].SwapBackPop (currentShard);
                    clipper.m_convexClip.ConvexPolygonClipInsideOutside (
                        currentShard,
                        currentShard,
                        shards[shard1],
                        xyzWork1, xyzWork2,
                        false,
                        planeTolerance
                        );
                    if (currentShard.size () > 2)
                        {
                        CompressCyclicPointsAndZingers (currentShard, pointTolerance);
                        if (currentShard.size () > 2)
                            insideShards.PushCopy (currentShard);
                        }
                    }
                std::swap (shard0, shard1);
                }
            else if (clipper.m_clipChain.size () > 0)
                {
                shards[shard1].ClearToCache ();
                insideShards.ClearToCache ();
                // save anything inside this clippers multiple convex parts for final reassembly.
                // cycle the outer residual for later clippers.
                for (size_t k = 0; k < clipper.m_clipChain.size () && !shards[shard0].empty (); k++)
                    {
                    ConvexClipPlaneSet &singleClipper = clipper.m_clipChain.at (k);

                    while (!shards[shard0].empty ())
                        {
                        shards[shard0].SwapBackPop (currentShard);
                        size_t count1 = shards[shard1].size ();

                        if (debugPolygons != nullptr)
                            {
                            numDebugStep++;
                            AddTransformedPolygon (*debugPolygons, currentShard, debugYShift + numDebugStep * debugStep, 3);
                            }


                        singleClipper.ConvexPolygonClipInsideOutside (
                            currentShard,
                            currentShard,
                            shards[shard1],
                            xyzWork1, xyzWork2,
                            false,
                            planeTolerance
                            );

                        if (debugPolygons != nullptr)
                            {
                            AddTransformedPolygon (*debugPolygons, currentShard, numDebugStep * debugStep - debugYShift, 3);
                            for (size_t k = count1; k <shards[shard1].size (); k++)
                                AddTransformedPolygon (*debugPolygons, shards[shard1][k], numDebugStep * debugStep, 3);
                            }


                        if (currentShard.size () > 2)
                            {
                            CompressCyclicPointsAndZingers (currentShard, pointTolerance);
                            if (currentShard.size () > 2)
                                insideShards.PushCopy (currentShard);
                            }
                        }
                    std::swap (shard0, shard1);
                    }
                shards[shard1].ClearToCache (); // shards[shard0] has the live things (outside this clipper, awaiting more clip or final save)
                }
            }
        exteriorShards.Record (shards[shard0].size ());
        size_t numOutside = 0;  // To become the number of nontrivial outside shards
        for (auto &shard : shards[shard0])
            {
            CompressCyclicPointsAndZingers (shard, pointTolerance);
            if (shard.size () > 2)
                numOutside++;
            }

        size_t numInside = insideShards.size ();       // These were already compressed
            
        if (numOutside > 0 && outsidePolygons != nullptr)
            {
            if (shards[shard0].size () >= s_minOutside)
                {
                if (numInside == 0)
                    {
                    // clip planes sliced through, but only artificially.   Original facet is unchanged.
                    AddPolygonCapture (*outsidePolygons, visitorPoints, 2);
                    }
                else
                    {
                    healer.HealShards (shards[shard0], visitorPoints);
                    for (auto &shard : shards[shard0])
                        {
                        if (shard.size () > 2)
                            AddPolygonCapture (*outsidePolygons, shard, 2);
                        }
                    }
                }
            }

        if (numInside > 0 && insidePolygons != nullptr)
            {
            if (numOutside == 0)
                {
                AddPolygonCapture (*insidePolygons, visitorPoints, 2);
                }
            else
                {
                if (insideShards.size () > 0)
                    healer.HealShards (insideShards, visitorPoints);
                for (auto &shard : insideShards)
                    {
                    if (shard.size () > 2)
                        AddPolygonCapture (*insidePolygons, shard, 2);
                    }
                }
            }

        }
    clipTime.Stop ();
    totalTime.Stop ();
#define PrintTimes_not
#ifdef PrintTimes
    GEOMAPI_PRINTF (" (Punch (total %g) (load %g) (clip %g))\n",
        (double)totalTime.GetElapsedSeconds(),        
        (double)loadTime.GetElapsedSeconds(),        
        (double)clipTime.GetElapsedSeconds()
        );
#endif
    }

END_BENTLEY_GEOMETRY_NAMESPACE
