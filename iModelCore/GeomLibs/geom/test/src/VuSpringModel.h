/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/test/src/VuSpringModel.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef CompileFromGeomLibsGTest
extern void SaveGraph (VuSetP graph, VuMask mask = 0);
extern void SaveGraphEdges (VuSetP graph, VuMask mask = 0);
extern void SaveGraphEdgesInsideBarrier (VuSetP graph, VuMask loopMask);
#else
// stub implementations of debug logic called from VuSpringModel
void SaveGraph (VuSetP graph, VuMask mask = 0) {}
void SaveGraphEdges (VuSetP graph, VuMask mask = 0) {}
void SaveGraphEdgesInsideBarrier (VuSetP graph, VuMask loopMask) {}
struct Check
{
static void SaveTransformed(IGeometryPtr const &data) {}
static void SaveTransformed(CurveVectorCR data) {}
static void SaveTransformed(ICurvePrimitiveCR data) {}
static void SaveTransformed(PolyfaceHeaderCR data) {}
static void SaveTransformed(ISolidPrimitiveCR data) {}
static void SaveTransformed (bvector<DPoint3d> const &data) {}
static void SaveTransformed (bvector<bvector<DPoint3d>> const &data) {}
static void SaveTransformed (bvector<DTriangle3d> const &data, bool closed = true) {}
static void SaveTransformed (bvector<DSegment3d> const &data) {}
static void SaveTransformed(MSBsplineSurfacePtr const &data) {}
static void Shift (double dx, double dy, double dz = 0.0) {}
static void Shift (DVec3dCR shift) {}
static void ShiftToLowerRight (double dx = 0.0) {}
static Transform GetTransform () {return Transform::FromIdentity ();}
static void SetTransform (TransformCR transform) {}

static void ClearGeometry (char const *name) {}
};


struct SaveAndRestoreCheckTransform
{
Transform m_baseTransform;
DVec3d m_finalShift;
SaveAndRestoreCheckTransform ()
    {
    m_finalShift.Zero ();
    m_baseTransform = Check::GetTransform ();
    }
SaveAndRestoreCheckTransform (double dxFinal, double dyFinal, double dzFinal)
    {
    m_finalShift.Init (dxFinal, dyFinal, dzFinal);
    m_baseTransform = Check::GetTransform ();
    }
void DoShift ()
    {
    Check::SetTransform (m_baseTransform);
    Check::Shift (m_finalShift.x, m_finalShift.y, m_finalShift.z);
    m_baseTransform = Check::GetTransform ();
    }
~SaveAndRestoreCheckTransform ()
    {
    DoShift ();
    }
};
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
        StationPolygon (TElementId elementId) : m_elementId (elementId) {}
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

// Return nodes of two triangles sharing an edge.
// * --------*
//  \ A    C/D\
//   \     /   \
//    \   /     \
//     \B/E     F\
//      *---------*
        static bool IsEdgeBetweenTriangles (VuP nodeA,
            VuP &nodeB,
            VuP &nodeC,
            VuP &nodeD,
            VuP &nodeE,
            VuP &nodeF
            )
            {
            nodeD = nodeE = nodeF = nullptr;
            if (IsNodeInTriangle (nodeA, nodeB, nodeC))
                {
                nodeD = nodeB->VSucc ();
                return IsNodeInTriangle (nodeD, nodeE, nodeF);
                }
            return false;
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
                //double cEF = vu_cross (nodeA, nodeE, nodeF);
                //double cFC = vu_cross (nodeA, nodeF, nodeC);
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
                typename VuSpringModel<TElementId>::TriangleWeightFunction::CappedQuadraticFunction edgeWeightFunction (0, 0, 1, 0, 10);
                static int s_springSelect = 0;
                typename VuSpringModel<TElementId>::TriangleWeightFunction springFunction (*this, s_springSelect, edgeWeightFunction);
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
        void CollectStationAreas (bvector<StationPolygon> &areas, bool preferDirectCentroidPaths = true, double shrinkFraction = 0.0, double directPathFraction = 0.10)
            {
            _VuSet::TempMask skipableEdge (Graph (), false);    // mask to mark edges whose mid-edge points can be skipped.
            areas.clear ();

            if (preferDirectCentroidPaths)
                {
                _VuSet::TempMask visitMask (Graph (), false);
                VU_SET_LOOP (nodeA, this)
                    {
                    VuP nodeB, nodeC, nodeD, nodeE, nodeF;
                    if (IsEdgeBetweenTriangles (nodeA, nodeB, nodeC, nodeD, nodeE, nodeF)
                        && nodeA->CountMaskAroundFace (m_wallMask) == 0
                        && nodeD->CountMaskAroundFace (m_wallMask) == 0
                        && nodeA->CountMaskAroundVertex (m_wallMask) == 0
                        && nodeD->CountMaskAroundVertex (m_wallMask) == 0
                        )
                        {
                        bool leftTest  = IsStationNode (nodeA) && IsStationNode (nodeB) && IsStationNode (nodeC);
                        bool rightTest = IsStationNode (nodeD) && IsStationNode (nodeD) && IsStationNode (nodeF);
                        if (leftTest || rightTest)
                            {
                            nodeA->SetMask (skipableEdge.Mask ());
                            nodeD->SetMask (skipableEdge.Mask ());
                            }                            
                        }
                    }
                END_VU_SET_LOOP (nodeA, this)
                }

            _VuSet::TempMask visitMask (Graph (), false);
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
                        
                        areas.push_back (StationPolygon (elementId));
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
                            if (!node0->HasMask (skipableEdge.Mask ()))
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

//----------------------------------------------------------------------------------------------------------------------
// Space id's are one based !!!!
static const int s_invalidSpaceId = 0;

// the "userDataPAsInt" attribute of a vu node is the space id.
// spaceId == 0 is an unassigned node.
// (i.e. first space gets id 1)
struct SpaceDescriptor
{
DPoint3d m_seedPoint;
int m_id;
double m_targetArea;
double m_currentArea;
bvector<VuP> m_nodes;   // !! Complete list of nodes.
SpaceDescriptor () : m_seedPoint (DPoint3d::From (0,0,0)), m_id(0), m_targetArea (0.0), m_currentArea (0.0) {}
// Constructor -- initialize with a reference coordinate and an id but no nodes.
SpaceDescriptor (DPoint3dCR xyz, int id, double targetArea = 0.0)
    {
    m_id = id;
    m_seedPoint = xyz;
    m_targetArea = targetArea;
    }

int GetId () const { return m_id;}
// Add a node to this space.
// If the node is already in another space, return false.
bool AddNode (VuP node)
    {
    int idInNode = node->GetUserDataAsInt ();
    if (idInNode == m_id)
        return true;
    if (idInNode != 0)
        return false;
    m_nodes.push_back (node);
    node->SetUserDataAsIntAroundFace (m_id);
    return true;
    }
double SumNodeAreas ()
    {
    double area = 0.0;
    for (auto node : m_nodes)
        area += vu_area (node);
    return area;
    }

};

struct GriddedSpaceManager
{
friend struct GriddedSpaceQueries;
friend struct GriddedSpace_FloodFromSingleNode;
private:
VuSetP m_graph;
bvector<bvector <DPoint3d>> m_parityLoops;
bvector<bvector <DPoint3d>> m_chains;
double m_meshSize;
bool m_isoGrid;
bool m_smoothTriangles;
int    m_lastSpaceId;
bmap<int, SpaceDescriptor> m_spaces;
public:
// Constructor -- empty manager
GriddedSpaceManager () :
    m_graph(nullptr),
    m_isoGrid (true),
    m_smoothTriangles (true),
    m_meshSize (0.0)
    {
    }
~GriddedSpaceManager ()
    {
    if (m_graph != nullptr)
        vu_freeVuSet (m_graph);
    }
void SetMeshParams (double meshSize, bool isoGrid = true, bool smoothTriangles = true)
    {
    m_meshSize = meshSize;
    m_isoGrid = isoGrid;
    m_smoothTriangles = smoothTriangles;
    }
int GetSpaceId (VuP node)
    {
    return node->GetUserDataAsInt ();
    }
bool IsBarrierEdge (VuP node)
    {
    return node->HasMask (VU_BOUNDARY_EDGE | VU_RULE_EDGE);
    }

bool IsOccupied (VuP node) {return node->GetUserDataAsInt () != 0;}
VuSetP Graph (){ return m_graph;}

// create a graph for given geometry.
bool TryLoad
(
bvector<bvector <DPoint3d>> &parityLoops,
bvector<bvector <DPoint3d>> &chains
)
    {
    static double s_numGridLine = 20.0;
    if (m_graph != nullptr)
        vu_freeVuSet (m_graph);
    m_graph = nullptr;
    m_parityLoops = parityLoops;
    m_chains      = chains;
    m_lastSpaceId = 0;
    DRange3d range = DRange3d::From (parityLoops);
	if (m_meshSize <= 0.0)
	    m_meshSize = DoubleOps::Max (range.XLength (), range.YLength ()) / s_numGridLine;
    m_graph = VuOps::CreateTriangulatedGrid (m_parityLoops, m_chains, bvector<DPoint3d> (),
                        bvector<double> (), bvector<double> (),
                        m_meshSize, m_meshSize, m_meshSize, m_meshSize, m_isoGrid, m_smoothTriangles);
    return m_graph != nullptr;
    }

int GetSpaceIdAtXYZ (DPoint3dCR xyz)
    {
    VuP node = vu_findContainingFace_linearSearch (m_graph, xyz, VU_EXTERIOR_EDGE);
    if (nullptr == node)
        return s_invalidSpaceId;
    int oldId = node->GetUserDataAsInt ();
    return oldId;
    }

// Try to claim the face containing xyz as a space.
int CreateSpace (DPoint3dCR xyz, double targetArea)
    {
    VuP node = vu_findContainingFace_linearSearch (m_graph, xyz, VU_EXTERIOR_EDGE);
    if (nullptr == node)
        return s_invalidSpaceId;
    int oldId = node->GetUserDataAsInt ();
    if (oldId != 0)
        {
        // this node is already claimed
        return s_invalidSpaceId;
        }
    int id = ++m_lastSpaceId;
    m_spaces[id] = SpaceDescriptor (xyz, id, targetArea);
    m_spaces[id].AddNode (node);
    return id;
    }

// Set mask in each node that (a) has a nonzero UserDataPAsInt and (b) edge mate has a different UserDataPAsInt
size_t SetAllSpaceBoundaryMasks (VuMask mask)
    {
    vu_clearMaskInSet (m_graph, mask);
    size_t numBoundary = 0;
    VU_SET_LOOP(node, m_graph)
        {
        int id = node->GetUserDataAsInt ();
        if (id != s_invalidSpaceId
            && node->EdgeMate ()->GetUserDataAsInt () != id)
            {
            node->SetMask (mask);
            numBoundary++;
            }
        }
    END_VU_SET_LOOP (node, m_graph)
    return numBoundary;
    }

double SpaceIdToArea (int id)
    {
    auto space = m_spaces.find (id);
    if (space == m_spaces.end ())
        return 0.0;
    double area = 0.0;
    for (auto node : space->second.m_nodes)
        {
        area += vu_area (node);
        }
    return area;
    }
    
};

// Helper class to do flood searches in a GriddedSpaceManager
struct GriddedSpace_FloodFromSingleNode
{
GriddedSpaceManager &m_manager;
GriddedSpace_FloodFromSingleNode (GriddedSpaceManager &manager) : m_manager(manager) {}

// Floods use a priority queue (aka heap) to access prefered direction of flood
struct HeapEntry
    {
    int m_id;
    VuP m_node;
    HeapEntry (int id, VuP node) : m_id (id), m_node(node) {}
    HeapEntry () : m_id (0), m_node (nullptr) {}
    };
MinimumValuePriorityQueue <HeapEntry> m_heap;

// Visit the neighbor across each edge of a face.
// (but do not cross barriers, or cross into occupied space)
// push the neighbors onto the priority queue.
void AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (SpaceDescriptor &s, VuP faceSeed)
    {
    VU_FACE_LOOP (edge, faceSeed)
        {
        auto mate = edge->EdgeMate ();
        if (!m_manager.IsBarrierEdge (edge) && m_manager.GetSpaceId (mate) == 0)
            {
            DPoint2d uv;
            int numPos, numNeg;
            double area;
            vu_centroid (&uv, &area, &numPos, &numNeg, mate);
            double d = uv.Distance (DPoint2d::From (s.m_seedPoint));
            m_heap.Insert (HeapEntry (s.m_id, mate), d);
            }
        }
    END_VU_FACE_LOOP (edge, faceSeed)
    }

// flood from a space until it reaches its target area.
// (This may be called repeatedly with larger targetAreas)
bool ExpandSingleSpaceIdToTargetArea (int id, double targetArea)
    {
    m_heap.Clear ();
    auto space = m_manager.m_spaces.find (id);
    if (space == m_manager.m_spaces.end ())
        return false;
    space->second.m_currentArea = 0.0;
    space->second.m_targetArea = targetArea;
    // Sum areas of current faces.
    // Add all neighbors to the heap.
    for (auto node : space->second.m_nodes)
        {
        space->second.m_currentArea += vu_area (node);
        AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (space->second, node);
        }
    HeapEntry entry;
    double distance;
    double a0, a1;
    for (;;)
        {
        a0 = space->second.m_currentArea;
        a1 = space->second.m_targetArea;
        if (a0 > a1)
            break;
        if (!m_heap.RemoveMin (entry, distance))
            break;
        // The facet may have been absorbed from another direction . . . 
        if (!m_manager.IsOccupied (entry.m_node))
            {
            space->second.m_currentArea += vu_area (entry.m_node);
            space->second.AddNode (entry.m_node);
            AddNeighborsToFloodHeap_weightByDistanceFromRefPoint (space->second, entry.m_node);
            }
        }
    return space->second.m_currentArea >= space->second.m_targetArea;
    }

};

// Helper class to implement queries along with a GriddedSpaceManager.
struct GriddedSpaceQueries
{
GriddedSpaceManager &m_manager;
GriddedSpaceQueries (GriddedSpaceManager &manager) : m_manager(manager){}

void SaveSpaceBoundaries (bool showCentroid = false)
    {
    double tolerance = DoubleOps::SmallMetricDistance ();
    _VuSet::TempMask boundaryMask (m_manager.m_graph);
    m_manager.SetAllSpaceBoundaryMasks (boundaryMask.Mask ());
    _VuSet::TempMask visitMask (m_manager.m_graph);
    bvector<VuP> chainNodes;
    bvector<DPoint3d> chain;
    VU_SET_LOOP (seedNode, m_manager.m_graph)
        {
        if (m_manager.IsOccupied (seedNode) && !visitMask.IsSetAtNode (seedNode) && boundaryMask.IsSetAtNode (seedNode))
            {
if (seedNode->HasMask (VU_EXTERIOR_EDGE))  // Something is strange -- this should be filtered by IsOccupied
    continue;
            chainNodes.clear ();
            auto currentNode = seedNode;
            for (; nullptr != currentNode;)
                {
                visitMask.SetAtNode (currentNode);
                chainNodes.push_back (currentNode);
                currentNode->SetMask (visitMask.Mask ());
                currentNode = currentNode->FSucc ()->FindMaskAroundReverseVertex (boundaryMask.Mask ());
                if (currentNode == seedNode)
                    break;
                }
            // (flood regions are closed ... chain should always close)
            chain.clear ();
            for (auto node : chainNodes)
                {
                DPoint3d xyz0 = node->GetXYZ ();
                DPoint3d xyz1 = node->FSucc ()->GetXYZ ();
                if (m_manager.IsBarrierEdge (node))
                    {
                    chain.push_back (xyz0);
                    chain.push_back (xyz1);
                    }
                else
                    {
                    chain.push_back (DPoint3d::FromInterpolate (xyz0, 0.5, xyz1));
                    }
                }
            if (currentNode == seedNode)    // closure -- we expect this
                {
                auto xyz = chain.front ();
                chain.push_back (xyz);
                }
            DPoint3dOps::Compress (chain, tolerance);
            Check::SaveTransformed (chain);
            if (showCentroid)
                {
                DPoint3d centroid;
                double area;
                DVec3d normal;
                PolygonOps::CentroidNormalAndArea (chain, centroid, normal, area);
#define CentroidHashFraction 0.05
                double d = CentroidHashFraction * sqrt (area);
                bvector<DPoint3d> hash;
                hash.push_back (DPoint3d::From (centroid.x + d, centroid.y));
                hash.push_back (DPoint3d::From (centroid.x - d, centroid.y));
                hash.push_back (DPoint3d::From (centroid.x, centroid.y));
                hash.push_back (DPoint3d::From (centroid.x, centroid.y - d));
                hash.push_back (DPoint3d::From (centroid.x, centroid.y + d));
                Check::SaveTransformed (hash);
                }
            }
        }
    END_VU_SET_LOOP (seedNode, m_manager.m_graph)
    }

void CollectSpaceBoundaries (TaggedPolygonVectorR spaces, bool smoothBoundaries)
    {
    spaces.clear ();
    double tolerance = DoubleOps::SmallMetricDistance ();
    _VuSet::TempMask boundaryMask (m_manager.m_graph);
    m_manager.SetAllSpaceBoundaryMasks (boundaryMask.Mask ());
    _VuSet::TempMask visitMask (m_manager.m_graph);
    bvector<VuP> chainNodes;
    bvector<DPoint3d> chain;
    bvector<DPoint3d> chain0;   // unsmoothed.
    VU_SET_LOOP (seedNode, m_manager.m_graph)
        {
        if (m_manager.IsOccupied (seedNode) && !visitMask.IsSetAtNode (seedNode) && boundaryMask.IsSetAtNode (seedNode))
            {
if (seedNode->HasMask (VU_EXTERIOR_EDGE))  // Something is strange -- this should be filtered by IsOccupied
    continue;
            chainNodes.clear ();
            auto currentNode = seedNode;
            for (; nullptr != currentNode;)
                {
                visitMask.SetAtNode (currentNode);
                chainNodes.push_back (currentNode);
                currentNode->SetMask (visitMask.Mask ());
                currentNode = currentNode->FSucc ()->FindMaskAroundReverseVertex (boundaryMask.Mask ());
                if (currentNode == seedNode)
                    break;
                }
            // (flood regions are closed ... chain should always close)
            chain.clear ();
            chain0.clear ();
            for (auto node : chainNodes)
                {
                DPoint3d xyz0 = node->GetXYZ ();
                chain0.push_back (xyz0);
                DPoint3d xyz1 = node->FSucc ()->GetXYZ ();
                if (m_manager.IsBarrierEdge (node))
                    {
                    chain.push_back (xyz0);
                    chain.push_back (xyz1);
                    }
                else
                    {
                    chain.push_back (DPoint3d::FromInterpolate (xyz0, 0.5, xyz1));
                    }
                }
            if (currentNode == seedNode)    // closure -- we expect this
                {
                auto xyz = chain.front ();
                chain.push_back (xyz);
                xyz = chain0.front ();
                chain0.push_back (xyz);
                }
            DPoint3dOps::Compress (chain, tolerance);
            DPoint3dOps::Compress (chain0, tolerance);
            if (smoothBoundaries)
                spaces.push_back (TaggedPolygon (chain, (ptrdiff_t)m_manager.GetSpaceId (seedNode)));
            else
                spaces.push_back (TaggedPolygon (chain0, (ptrdiff_t)m_manager.GetSpaceId (seedNode)));
            Check::SaveTransformed (chain);
            }
        }
    END_VU_SET_LOOP (seedNode, m_manager.m_graph)
    }




void SaveWalls ()
    {
    Check::SaveTransformed (m_manager.m_parityLoops);
    Check::SaveTransformed (m_manager.m_chains);
    }
};
