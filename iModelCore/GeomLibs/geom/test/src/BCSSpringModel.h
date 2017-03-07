
// unused - static int s_noisy = 0;

#ifdef CompileFromGeomLibsGTest
extern void SaveGraph (VuSetP graph, VuMask mask = 0);
extern void SaveGraphEdges (VuSetP graph, VuMask mask = 0);
extern void SaveGraphEdgesInsideBarrier (VuSetP graph, VuMask loopMask);
#else
void SaveGraph (VuSetP graph, VuMask mask = 0) {}
void SaveGraphEdges (VuSetP graph, VuMask mask = 0) {}
void SaveGraphEdgesInsideBarrier (VuSetP graph, VuMask loopMask) {}
#endif

// BCSSpringModel
// "Building ConceptStation Spring Model"
// 
// A floorplan is defined by lines for the walls.
// A station is a point with an approximate size (single number) for its expected "radius"
// The spring model will:
//   1) form a triangulation of the space, with triangle vertices at the station points.
//   2) iteratively adjust station coordinate to mimic the stations moving around to meet their "size" goals.
//
// Usage sequence:
// 1) Contructor:
//***********    BCSSpringModel sm;
// 2) Insert geometry: (Multiple calls, any order):
//***********        sm.AddWall (wall, 10.0);   // wall coordinates are bvector<DPoint3d>
//***********        sm.AddStation (xyz, size);
// 3) move station points for size conditions:
//***********        sm.SolveSprings ();
// 4) traverse the vu graph to extract area around the stations ..
//***********        sm.CollectStationAreas (bvector<bvector<DPoint3d>> &areas, double shrinkFraction = 0.0)


//

template <typename TElementId>
struct VuSpringModel : private _VuSet
    {

    VuSetP Graph () { return this; }
    
    // Internal struct for station data -- current xyz, original xyz, target radius.
    struct StationData
        {
        TElementId m_elementId;
        DPoint3d m_xyz;
        DPoint3d m_xyzBase;
        double m_radius;
        StationData (DPoint3dCR xyzBase, double radius, TElementId elementId)
            {
            m_elementId = elementId;
            m_xyz = m_xyzBase = xyzBase;
            m_radius = radius;
            }
        StationData () {}
        };

	struct StationPolygon
		{
		TElementId m_elementId;
		bvector<DPoint3d> m_xyz;
		};
    // a movable node stores its station.
    // the station has radius.
    ValidatedDouble GetStationRadius (VuP node)
        {
        ptrdiff_t index = node->GetUserData1 ();
#ifdef VerifyData1Consistency
        size_t errors = 0;
        VU_VERTEX_LOOP (nodeA, node)
            {
            if (index != nodeA->GetUserData1 ())
                errors++;
            }
        END_VU_VERTEX_LOOP (nodeA, node)
#endif
        if (index >= 0 && (size_t)index < m_station.size ())
            return ValidatedDouble (m_station[index].m_radius, true);
        return ValidatedDouble (0.0, false);
        }

    // Internal structure for computing weight values used in the spring iteration.
    struct TriangleWeightFunction : VuScalarFunction
        {
        struct CappedQuadraticFunction
            {
            double m_c0, m_c1, m_c2;
            double m_fMin, m_fMax;
            CappedQuadraticFunction (double c0, double c1, double c2, double fMin = 0.0, double fMax = 10.0)
                : m_c0 (c0), m_c1 (c1), m_c2 (c2), m_fMin (fMin), m_fMax (fMax)
                {}
            virtual ValidatedDouble EvaluateRToR (double u)
                {
                // unused - double f = m_c0 + u * (m_c1 + u * m_c2);
                return DoubleOps::Clamp (u, m_fMin, m_fMax);
                }
            };

        // Internal structure for computing spring effects in triangles.
        TriangleWeightFunction (
            VuSpringModel &springModel,
            int factorSelect,
            CappedQuadraticFunction &edgeFunction    // univariate function for correcting a single edge length ratio.
        )
            : m_springModel (springModel),
            m_factorSelect (factorSelect),
            m_edgeFunction (edgeFunction)
            {}

        //!<ul>
        //!<li>0 - average of ratios of the two edge ratios
        //!<li>1 - geometric mean of two edge ratios
        //!</ul>
        VuSpringModel &m_springModel;
        int m_factorSelect;
        double m_power;
        CappedQuadraticFunction &m_edgeFunction;

        double EdgeFactor (VuP node0)
            {
            VuP node1 = node0->EdgeMate ();
            auto targetA = m_springModel.GetStationRadius (node0);
            auto targetB = m_springModel.GetStationRadius (node1);
            double factor = 1.0;
            if (targetA.IsValid () || targetB.IsValid ())
                {
                double targetLength = targetA.Value () + targetB.Value ();
                DPoint3d xyz0 = node0->GetXYZ ();
                DPoint3d xyz1 = node1->GetXYZ ();
                double factor0;
                DoubleOps::SafeDivide (factor0, xyz0.Distance (xyz1), targetLength, 1.0);
                factor = m_edgeFunction.EvaluateRToR (factor0);
                }
            return factor;
            }
        // return sum of spring factors for the two outbound edges.
        double Evaluate (VuSetP graph, VuP nodeA)
            {
            VuP nodeB = nodeA->VSucc ();
            double factorA = EdgeFactor (nodeA);
            double factorB = EdgeFactor (nodeB);
            if (m_factorSelect == 1)
                {
                return sqrt (factorA * factorB);
                }
            else
                return 0.5 * (factorA * factorB);
            }
        };

    VuSpringModel ()
        : m_wallMask (VU_RULE_EDGE),
        m_fringeMask (VU_BOUNDARY_EDGE),
        m_fringeExteriorMask (VU_EXTERIOR_EDGE),
        // edges joining center points to corners (overrides aspect ratio)
        m_cornerFlipMask (VU_KNOT_EDGE),
        // These are the edges fixed by this class.
        // Note that triangle flipping also honors others in VU_ALL_FIXED_EDGES_MASK
        m_allFixedSpringEdgesMask (VU_RULE_EDGE | VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE | VU_KNOT_EDGE)
        {
        m_shortWallTolerance = 1.0e-6;
        m_totalWallPoints = 0;
        // userData1 is index to station array.  We trust vu will manage this carefully ...
        vu_setDefaultUserData1 (this, -1, true, false);
        }

    // SpringModel member var:  array of stations.
    bvector<StationData> m_station;
    // Spring model member var: total number of points entered as walls.  (Coordinates are not retained -- they are in the VU graph)
    // wall point count is used to guide edge splitting of exterior rectangle
    size_t m_totalWallPoints;

    // Spring model member var:   mask for "wall" edges
    VuMask const m_wallMask;
    // Spring model member var:     mask for (both sides of ) "fringe" (outer rectangle) edges
    VuMask const m_fringeMask;
    // Spring model member var:     mask for outside of fringe
    VuMask const m_fringeExteriorMask;
    // Spring model member var:     mask for edges flipped to extend a space into a corner not reachable by usual flips.
    VuMask const m_cornerFlipMask;
    // Spring model member var:     composite mask with all the edges this class wants fixed.
    VuMask const m_allFixedSpringEdgesMask;
    // Spring model member var:     tolerance for ignoring wall edges.
    double m_shortWallTolerance;

        bool IsStationNode (VuP node) {return node->GetUserData1 () >= 0;}
        bool IsWallNode (VuP node) {return node->HasMask (m_wallMask);}
    // MAIN PUBLIC ENTRIES TO ADD GEOMETRY ....
    // Define a station point and its preferred distance from neighbors.
    void AddStation (DPoint3dCR xyz, double radius, TElementId elementId = TElementId ())
        {
        m_station.push_back (StationData (xyz, radius, elementId));
        }
    // Define a wall with the default short edge tolerance.
    void AddWall (bvector<DPoint3d> const &xyz)
        {
        VuP chainTail, chainHead;
        VuOps::MakeEdges (this, chainTail, chainHead, xyz, m_wallMask, m_wallMask, m_shortWallTolerance);
        m_totalWallPoints += xyz.size ();
        }
    // Define a wall with the specified short edge tolerance.
    void AddWall (bvector<DPoint3d> const &xyz, double maxEdgeLength)
        {
        if (xyz.size () > 1)
            {
            bvector<DPoint3d> newXYZ;
            newXYZ.push_back (xyz[0]);
            for (size_t i = 1; i < xyz.size (); i++)
                {
                DPoint3d xyzA = newXYZ.back ();
                DPoint3d xyzB = xyz[i];
                double d = xyzA.Distance (xyzB);
                if (d > maxEdgeLength)
                    {
                    size_t n = (int)(0.9999999 + d / maxEdgeLength);
                    for (size_t k = 1; k < n; k++)
                        newXYZ.push_back (DPoint3d::FromInterpolate (xyzA, (double)k / (double)n, xyzB));
                    }
                newXYZ.push_back (xyzB);
                }
            AddWall (newXYZ);
            }
        }

    // Define a wall by two points.
    void AddWall (DPoint3dCR xyzA, DPoint3dCR xyzB)
        {
        VuP chainTail, chainHead;
        VuOps::MakeEdge (this, chainTail, chainHead, xyzA, xyzB, m_wallMask, m_wallMask);
        m_totalWallPoints += 2;
        }
    // Define a wall by two points by coordinates
    void AddWall (double xA, double yA, double xB, double yB)
        {
        VuP chainTail, chainHead;
        VuOps::MakeEdge (this, chainTail, chainHead, DPoint3d::From (xA, yA, 0.0), DPoint3d::From (xB, yB, 0.0), m_wallMask, m_wallMask);
        m_totalWallPoints += 2;
        }
    // find the station closest to xyz
    ptrdiff_t ClosestStation (DPoint3dCR xyz, double tol)
        {
        for (size_t i = 0, n = m_station.size (); i < n; i++)
            {
            double d = xyz.DistanceSquaredXY (m_station[i].m_xyzBase);
            if (d < m_shortWallTolerance)
                return (ptrdiff_t)i;
            }
        return -1;
        }

    private:
        // Search the graph.  At each vertex, find the (index of) the closest station.  Store that index in the vertex.
        void IndexStations ()
            {
            // uh oh .. the insertion does not clearly record which vertices are the stations.
            // Just (!!!) search the graph and stations ....
            VuMask visitMask = GrabMask ();
            ClearMaskInSet (visitMask);
            VU_SET_LOOP (seed, this)
                {
                if (!seed->HasMask (visitMask))
                    {
                    seed->SetMaskAroundVertex (visitMask);
                    ptrdiff_t index = ClosestStation (seed->GetXYZ (), m_shortWallTolerance);
                    VU_VERTEX_LOOP (node, seed)
                        node->SetUserData1 (index);
                    END_VU_VERTEX_LOOP (node, seed)
                    }
                }
            END_VU_SET_LOOP (node, this)
            DropMask (visitMask);
            }


        static bool IsNodeInTriangle (VuP nodeA, VuP &nodeB, VuP &nodeC)
            {
            nodeB = nodeA->FSucc ();
            nodeC = nodeB->FSucc ();
            return nodeC->FSucc () == nodeA;
            }

        bool IsFixedEdge (VuP node)
            {
            return node->HasMask (VU_ALL_FIXED_EDGES_MASK);
            }
//
//
//
// * --------*
//  \ A    C/D\
//   \     /   \
//    \   /     \
//     \B/E     F\
//      *---------*
// IF these are all true:
// AB is an edge leaving a station center.
// B and C (hence D,E) are NOT stations.
// F is not a station
// F is visible from A
// at least one of EF and FD is a boundary edge.
//
// THEN
//   Yank <BD> edge at both ends
//   Insert B at A
//   Insert D at F
// * --------*
//  \A\B     C\
//   \   \     \
//    \     \   \
//     \  E   D\F\
//      *---------*
// RETURN one of (nullptr, A, B):
//    if EF and FD are both boundaries, return nullptr.
//  otherwise choose betwen A and B and return the one whose opposite edge is NOT a boudary.
//    if only EF is a boundary return B for use in another pass looking out from B to nonboundary FD
//    if only FD is a boundary return A for use in another pass looking out from A to nonboundary ED (D has moved to F!!!)
//
// Notes
//1) The coordinates of A,E,F,C never change
//2) The coordinates of B and D can change (B moves from E to A, D moves from C to F)
        VuP DoFlipTowardsBoundary (VuP nodeA, int &numFlipToIncrement)
            {
            static double s_areaRelTol = 1.0e-8;
            if (!IsStationNode (nodeA))
                return nullptr;
            auto id = nodeA->GetUserData1 ();
            VuP nodeB, nodeC;
            VuP nodeD, nodeE, nodeF;
            if (!IsNodeInTriangle (nodeA, nodeB, nodeC))
                return nullptr;
            if (IsFixedEdge (nodeB))
                return nullptr;
            nodeD = nodeC->VSucc ();
            if (!IsNodeInTriangle (nodeD, nodeE, nodeF))
                return nullptr;
            if (IsWallNode (nodeB) || IsStationNode (nodeB) || IsStationNode (nodeC) || IsStationNode (nodeF))
                return nullptr;
            bool eWall = IsWallNode (nodeE);
            bool fWall = IsWallNode (nodeF);
            if (eWall || fWall)
                {
                double areaABC = nodeA->CrossXY (nodeB, nodeC);
                double areaDEF = nodeD->CrossXY (nodeE, nodeF);
                if (areaABC <= 0.0 || areaDEF <= 0.0)
                    return nullptr;
                double areaTol = s_areaRelTol * (areaABC + areaDEF);    // and that is positive
                double cEF = vu_cross (nodeA, nodeE, nodeF);
                double cFC = vu_cross (nodeA, nodeF, nodeC);
                double areaAEF = nodeA->CrossXY (nodeE, nodeF);
                double areaAFC = nodeA->CrossXY (nodeF, nodeC);
                if (areaAEF <= areaTol || areaAFC <= areaTol)
                    return nullptr;
                VertexTwist (nodeB, nodeE); // yank nodeB out of its vertex
                VertexTwist (nodeC, nodeD); // yank nodeD out of its vertex
                VertexTwist (nodeB, nodeA);
                nodeB->SetXYZ (nodeA->GetXYZ());
                VertexTwist (nodeD, nodeF);
                nodeD->SetXYZ (nodeF->GetXYZ());
                nodeB->SetUserData1 (id);
                nodeB->SetMask (m_cornerFlipMask);
                nodeD->SetMask (m_cornerFlipMask);
                numFlipToIncrement++;
                if (eWall && !fWall)
                    return nodeB;
                if (fWall && !eWall)
                    return nodeA;
                return nullptr;
                }
            return nullptr;
            }
            int FlipTrianglesSoStationsFillCorners ()
                {
                int numFlip = 0;
                VU_SET_LOOP (seedNode, this)
                    {
                    auto id = seedNode->GetUserData1 ();
                    if (id >= 0)
                        {
                        VuP nodeA = seedNode;
                        VuP nodeA1 = nullptr;
                        while (nullptr != (nodeA1 = DoFlipTowardsBoundary (nodeA, numFlip)))
                            {
                            nodeA = nodeA1;
                            }
                        }
                    }
                END_VU_SET_LOOP (seedNode, this)
                return numFlip;
                }
        void MergeAndTriangulate ()
            {
            DRange3d range = Range ();
            for (auto &station : m_station)
                range.Extend (station.m_xyzBase);

            // Create linework clearly outside the walls
            VuP outsideNode = VuOps::AddExpandedRange (this, range, m_totalWallPoints + m_station.size (), 0.25, m_fringeMask, m_fringeExteriorMask);
            // TR #191917: keep dangling edges in triangulation
            vu_mergeOrUnionLoops (this, VUUNION_UNION);

            vu_regularizeGraph (this);
            outsideNode->SetMaskAroundFace (m_fringeExteriorMask);  // no need for the usual DFS -- we really know the outside. (And the walls might not close)
            vu_splitMonotoneFacesToEdgeLimit (this, 3);
            vu_flipTrianglesToImproveQuadraticAspectRatio (this);

            bvector<DPoint3d> stationBase;
            size_t numStation = m_station.size ();
            if (numStation > 0)
                {
                for (auto &station : m_station)
                    stationBase.push_back (station.m_xyzBase);
                vu_insertAndRetriangulate (this, &stationBase[0], numStation, true);
                }

            IndexStations ();
            Check::ShiftToLowerRight (10.0);
            SaveGraphEdgesInsideBarrier (Graph (), m_wallMask); // ASSUMES wall encloses area.
            FlipTrianglesSoStationsFillCorners ();
            Check::Shift (0,40,0);
            SaveGraphEdgesInsideBarrier (Graph (), m_wallMask); // ASSUMES wall encloses area.

            }
        void ClearMaskAroundVerticesWithOutboundMask (VuMask maskToFind, VuMask maskToClear)
            {
            VU_SET_LOOP (vertexSeed, this)
                {
                if (vertexSeed->HasMask (maskToFind))
                    {
                    VU_VERTEX_LOOP (outboundNode, vertexSeed)
                        {
                        outboundNode->ClearMaskAroundEdge (maskToClear);
                        }
                    END_VU_VERTEX_LOOP (outboundNode, vertexSeed)
                    }
                }
            END_VU_SET_LOOP (vertexSeed, this)
            }
    public:
        // Triangulate the station & walls data.
        //  Optionally do smoothing to simulate springs ..

        void SolveSprings (bool doSmoothing = true)
            {
            for (auto &station : m_station)
                {
                auto disk = CurveVector::CreateDisk (DEllipse3d::FromCenterRadiusXY (station.m_xyzBase, station.m_radius));
                Check::SaveTransformed (*disk);
                }
            SaveGraphEdges (Graph (), m_wallMask);

            MergeAndTriangulate ();

            Check::ShiftToLowerRight (10.0);
            SaveGraphEdgesInsideBarrier (Graph (), m_wallMask); // ASSUMES wall encloses area.
            Check::ShiftToLowerRight (10.0);
            DRange3d range;
            vu_graphRange (Graph (), &range);
            double dX = range.XLength () * 1.10;
            double dY = range.YLength () * 1.10;
            if (doSmoothing)
                {
                double shiftFraction;
                int numSweep;
                VuSpringModel::TriangleWeightFunction::CappedQuadraticFunction edgeWeightFunction (0, 0, 1, 0, 10);
                static int s_springSelect = 0;
                VuSpringModel::TriangleWeightFunction springFunction (*this, s_springSelect, edgeWeightFunction);
                vu_smoothInteriorVertices (Graph (), &springFunction, nullptr, 1.0e-4, 10, 100, 100, &shiftFraction, &numSweep);
                static size_t s_maxIteration = 5;
                VuMask skipMask = m_wallMask;//VU_EXTERIOR_EDGE; // m_wallMask;
                for (size_t iteration = 0; iteration < s_maxIteration; iteration++)
                    {
                    SaveAndRestoreCheckTransform shifter (dX, 0,0);
                    vu_smoothInteriorVertices (Graph (), &springFunction, nullptr, 1.0e-4, 20, 100, 100, &shiftFraction, &numSweep);
                    SaveGraphEdgesInsideBarrier (Graph (), m_wallMask); // ASSUMES wall encloses area.
                    int numFlipA = vu_flipTrianglesToImproveQuadraticAspectRatio (this);
                            Check::Shift (0, dY, 0);
                            SaveGraphEdgesInsideBarrier (Graph (), skipMask); // ASSUMES wall encloses area.
#define FixupCornersDuringIterations
#ifdef FixupCornersDuringIterations
                    int numFlipB = FlipTrianglesSoStationsFillCorners ();
                            Check::Shift (0, dY, 0);
                            SaveGraphEdgesInsideBarrier (Graph (), skipMask); // ASSUMES wall encloses area.
#endif
                    if (numFlipA == 0 && numFlipB == 0)
                        break;
                    }

                SaveGraphEdgesInsideBarrier (Graph (), m_wallMask); // ASSUMES wall encloses area.
                }
            }
        // return polygon coordinates for areas around stations.
        void CollectStationAreas (bvector<StationPolygon> &areas, double shrinkFraction = 0.0)
            {
            _VuSet::TempMask visitMask (Graph (), false);
            areas.clear ();
            VU_SET_LOOP (vertexSeedNode, Graph ())
                {
                if (!vertexSeedNode->HasMask (visitMask.Mask ()))
                    {
                    vertexSeedNode->SetMaskAroundVertex (visitMask.Mask ());
                    auto weight0 = GetStationRadius (vertexSeedNode);
                    if (weight0.IsValid ())
                        {
                        size_t index = vertexSeedNode->GetUserData1 ();
                        TElementId elementId = TElementId ();
                        if (index >= 0 && index < m_station.size ())
                            {
                            elementId = m_station[index].m_elementId;
                            }

                        DPoint3d xyz0 = vertexSeedNode->GetXYZ ();

                        // Alternative way to collect area - just use center and Radius to create circles
                /*        auto disk = CurveVector::CreateDisk (DEllipse3d::FromCenterRadiusXY (xyz0, weight0));
                        IFacetOptionsPtr  facetOptions = IFacetOptions::Create ();
                        facetOptions->SetEdgeChainsRequired (true);
                        facetOptions->SetMaxEdgeLength (0.5);
                        bvector<bvector<DPoint3d>> spaceNewPoints;
                        if (!disk->CollectLinearGeometry (spaceNewPoints, facetOptions.get ()))
                            {
                            continue;
                            }
                        areas.push_back ({ elementId, spaceNewPoints[0] });*/
                        
                        areas.push_back ({ elementId, bvector<DPoint3d> () });
                        VU_VERTEX_LOOP (node0, vertexSeedNode)
                            {
                            VuP node1 = node0->FSucc ();
                            // Always output a point on the edge ...
                            DPoint3d xyz1 = node1->GetXYZ ();
                            auto weight1 = GetStationRadius (node1);
                            auto xyz = DPoint3d::FromWeightedAverage (xyz0, weight1.Value (), xyz1, weight0.Value ());    // reverse weights to push away from heavy end
                            if (xyz0.Distance (xyz) > weight0)
                                {
                                //xyz.Interpolate (xyz0, weight0 / xyz0.Distance (xyz), xyz); // do not go outside of circle.
                                }
                            areas.back ().m_xyz.push_back (xyz);

                            // If it is a triangle of stations, also output weighted centroid
                            VuP node2 = node1->FSucc ();
                            auto weight2 = GetStationRadius (node2);
                            VuP node3 = node2->FSucc ();
                            if (node3 == node0 && weight1.IsValid () && weight2.IsValid ())
                                {
                                DPoint3d xyz2 = node2->GetXYZ ();
                                auto xyz = DPoint3d::FromWeightedAverage (
                                    xyz0, weight1.Value () + weight2.Value (),
                                    xyz1, weight2.Value () + weight0.Value (),
                                    xyz2, weight0.Value () + weight1.Value ()
                                    );
                                areas.back ().m_xyz.push_back (xyz);
                                }
                            }
                        END_VU_VERTEX_LOOP (node0, vertexSeedNode)
                        DPoint3d xyz = areas.back ().m_xyz.front ();
                        areas.back ().m_xyz.push_back (xyz);
                        if (shrinkFraction != 0.0)
                            {
                            for (auto &xyz : areas.back ().m_xyz)
                                xyz = DPoint3d::FromInterpolate (xyz, shrinkFraction, xyz0);
                            }
                        }
                    }
                }
            END_VU_SET_LOOP (vertexSeedNode, Graph ())
            }
};

