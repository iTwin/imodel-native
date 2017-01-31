/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_undercut.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


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
                        DPoint3dOps::Compress (xyzAClip, pointTolerance);
                        surfaceAaboveB.push_back (TaggedPolygon (xyzAClip));
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
                        surfaceBbelowA.push_back (TaggedPolygon (xyzWork));
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

    double planarityAbsTol = s_planarityLocalRelTol * inputRange.low.Distance (inputRange.high);
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
                        false);
                    if (currentShard.size () > 2)
                        {
                        DPoint3dOps::Compress (currentShard, pointTolerance);
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
                            false);

                        if (debugPolygons != nullptr)
                            {
                            AddTransformedPolygon (*debugPolygons, currentShard, numDebugStep * debugStep - debugYShift, 3);
                            for (size_t k = count1; k <shards[shard1].size (); k++)
                                AddTransformedPolygon (*debugPolygons, shards[shard1][k], numDebugStep * debugStep, 3);
                            }


                        if (currentShard.size () > 2)
                            {
                            DPoint3dOps::Compress (currentShard, pointTolerance);
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
            DPoint3dOps::Compress (shard, pointTolerance);
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
                    AddPolygonCapture (*outsidePolygons, visitorPoints, 2);
                    }
                else
                    {
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
