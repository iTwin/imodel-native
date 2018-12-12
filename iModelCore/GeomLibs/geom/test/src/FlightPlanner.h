#pragma once

#include <Vu/VuApi.h>
#include <Vu/VuSet.h>


// Find shortest path from seed to all vertices.   Mark short path edges from vertices backwards
// ASSUME ... edge cost is marked as GetUserDataPAsInt.
// ON RETURN:
// Each vertex has one entry in paths.
// Each node has its vertex index as node->GetUserDataAsInt ()
// 
struct ShortestPathContext
    {
    //! Array entry for recording distance to vertices.
    struct VertexData
        {
        double m_a;
        VuP    m_nodeA;
        VuP    m_nodeB;
        VertexData () : m_a (0.0), m_nodeA (nullptr), m_nodeB (nullptr) {}
        VertexData (VuP nodeA, double a = 0.0, VuP nodeB = nullptr) : m_a (a), m_nodeA (nodeA), m_nodeB (nodeB) {}
        };


    // Callback functions for shortest path search in VU graphs.
    struct SearchFunctions
        {
        // Return the "length" of an edge as it contributes path length.
        // Default implementation returns 1.0 -- all edges have equal cost.
        virtual double EdgeLength (VuP node) { return 1.0; }
        // test if a path extending to node, with specified total distance, is allowed.
        // default always returns true.
        virtual bool CanPush (VuP node, double distance) { return true; }
        // Return false to terminate search.
        virtual bool ContinueSearch () { return true; }
        };

    // Callback functions to use edge length as search condition
    struct XYDistanceSearchFunctions : SearchFunctions
        {
        virtual double EdgeLength (VuP node)
            {
            VuP mate = node->EdgeMate ();
            return node->GetXYZ ().DistanceXY (mate->GetXYZ ());
            }
        };

    // Callback functions:
    // 1) xy edge length is search condition.
    // 2) closest vertex and distance with a distinguished outedge mask is retained.
    // 3) CanPush (a) manages distinguished mask check, and (b) rejects paths that are longer than those seen already
    struct DistinguishedEdgeXYDistanceSearchFunctions : SearchFunctions
        {
        VuP m_nodeA;
        double m_distanceA;
        VuMask m_mask;

        VuP GetTarget () { return m_nodeA; }
        double GetTargetDistance () { return m_distanceA; }

        DistinguishedEdgeXYDistanceSearchFunctions (VuMask mask)
            : m_mask (mask)
            {
            Clear ();
            }

        void Clear ()
            {
            m_nodeA = nullptr;
            m_distanceA = DBL_MAX;
            }
        virtual double EdgeLength (VuP node)
            {
            VuP mate = node->EdgeMate ();
            return node->GetXYZ ().DistanceXY (mate->GetXYZ ());
            }
        virtual bool CanPush (VuP node, double distance)
            {
            if (distance >= m_distanceA)
                return false;       // this path is already longer than an accepted path.
            if (node->CountMaskAroundVertex (m_mask) > 0)
                {
                // this node is closer and has the distinguished mask.
                m_nodeA = node;
                m_distanceA = distance;
                }
            return true;
            }
        bool ContinueSearch () { return m_nodeA == nullptr; }
        };

    // Callback functions:
    // 1) xy edge length is search condition.
    // 2) record entry edge and distance when target is reached.
    //   (But allow search to continue because a later path may be shorter)
    // 3) reject path probes for any longer distance.
    struct TargetVertexXYDistanceSearchFunctions : SearchFunctions
        {
        VuP m_target;       // any node at the target vertex
        VuP m_entry;        // the node of access to the target vertex.
        double m_targetDistance;
        VuP GetTarget () { return m_target; }
        VuP GetEntry () { return m_entry; }

        TargetVertexXYDistanceSearchFunctions (VuP target)
            : m_target (target)
            {
            Clear ();
            }

        void Clear ()
            {
            m_entry = nullptr;
            m_targetDistance = DBL_MAX;
            }
        virtual double EdgeLength (VuP node)
            {
            VuP mate = node->EdgeMate ();
            return node->GetXYZ ().DistanceXY (mate->GetXYZ ());
            }
        virtual bool CanPush (VuP node, double distance)
            {
            if (distance >= m_targetDistance)
                return false;       // this path is already longer than an accepted path.
            if (nullptr != node->FindNodeAroundVertex (m_target))
                {
                // this node is closer and has the distinguished mask.
                m_entry = node;
                m_targetDistance = distance;
                }
            return true;
            }
        bool ContinueSearch () { return true; } // Search terminates indirectly because distance condition rejects further probes.
        };

    private:
        bvector<VertexData> m_vertexData;
        VuSetP m_graph;
        VuMask m_backEdgeMask;      // mask for a node whose (directed) edge leads back towards the search root.
        MinimumValuePriorityQueue<VuP> m_heap;

        SearchFunctions *m_functions;


        //! initialize vertex indexing.  This is a full graph operation.
        //!<ul>
        //!<li>Clear all back edges.
        //!<li>Clear all userDataPAsInt.
        //!<li>Clear the vertex array.
        //!</ul>
        void InitializeIncrementalSearchData ()
            {
            m_vertexData.clear ();
            vu_clearMaskInSet (m_graph, m_backEdgeMask);
            // Give each vertex loop an entry in the VertexData array . . .
            VU_SET_LOOP (node, m_graph)
                {
                vu_setUserDataPAsInt (node, -1);
                }
            END_VU_SET_LOOP (node, m_graph)
            }

        // Clear the mask and UserDataAsInt around all vertex loops referenced by the m_vertexData array.
        void ResetIncrementalSearchData ()
            {
            for (auto &data : m_vertexData)
                {
                if (data.m_nodeA != nullptr)     // (This should always be nonnull)
                    {
                    data.m_nodeA->ClearMaskAroundVertex (m_backEdgeMask);
                    data.m_nodeA->SetUserDataAsIntAroundVertex (-1);
                    }
                }
            m_vertexData.clear ();
            }

    public:
        //! Create a search context.
        //!<ul>
        //!<li>the graph pointer is retained in the context.  (Caller is still responsible for freeing it)
        //!<li>the backEdgeMask is remembered for use during searches.
        //!<il>The UserDataPAsInt value in all nodes is used as vertex index
        //!</ul>
        ShortestPathContext (VuSetP graph, VuMask backEdgeMask)
            : m_graph (graph),
            m_backEdgeMask (backEdgeMask),
            m_functions (nullptr)
            {
            InitializeIncrementalSearchData ();
            }


        double GetDistanceToVertex (VuP node)
            {
            size_t index = (size_t)node->GetUserDataAsInt ();
            if (index < m_vertexData.size ())
                return m_vertexData[index].m_a;
            return DBL_MAX;
            }

        double GetEdgeLength (VuP node)
            {
            return m_functions->EdgeLength (node);
            }

        bool GetVertexData (size_t vertexIndex, VuP &nodeA, VuP &nodeB, double distance) const
            {
            if (vertexIndex < m_vertexData.size ())
                {
                nodeA = m_vertexData[vertexIndex].m_nodeA;
                nodeB = m_vertexData[vertexIndex].m_nodeB;
                distance = m_vertexData[vertexIndex].m_a;
                return true;
                }
            nodeA = nodeB = nullptr;
            distance = DBL_MAX;
            return false;
            }

        bool GetVertexData (size_t vertexIndex, VertexData &data) const
            {
            if (vertexIndex < m_vertexData.size ())
                {
                data = m_vertexData[vertexIndex];
                return true;
                }
            data = VertexData (nullptr, DBL_MAX, nullptr);
            return false;
            }

        size_t GetNumVertex () const { return m_vertexData.size (); }

        bool IsUnvisited (VuP node)
            {
            return DBL_MAX == GetDistanceToVertex (node);
            }
        void SetDistanceToVertex (VuP node, double distance, bool isBackEdge)
            {
            int index = node->GetUserDataAsInt ();
            if (index < 0)
                {
                index = (int)m_vertexData.size ();
                m_vertexData.push_back (VertexData (node, DBL_MAX, nullptr));
                node->SetUserDataAsIntAroundVertex (index);
                }
            
            if ((size_t)index < m_vertexData.size ())
                {
                m_vertexData[(size_t)index].m_a = distance;
                node->ClearMaskAroundVertex (m_backEdgeMask);
                m_vertexData[(size_t)index].m_nodeB = nullptr;
                if (isBackEdge)
                    {
                    m_vertexData[(size_t)index].m_nodeB = node;
                    vu_setMask (node, m_backEdgeMask);
                    }
                else
                    {
                    }
                }
            }
        // Run a search from a seedNode.
        // Search terminates when entire graph is reached or functions->ContinueSearch () returns false.
        // this can be called repeatedly.
        // After a search:
        // 1) The m_vertexData array identifies all vertex loops that have been assigned a distance.
        //          m_nodeA = any node around the vertex.
        //          m_nodeB = the outbound edge going backwards to the search start
        // 2) m_nodeB (and only m_nodeB) has the backEdge mask.
        size_t SearchFromSeed (VuP seedNode, SearchFunctions *functions)
            {
            m_functions = functions;
            SearchFunctions unitWeightFunctions;
            if (m_functions == nullptr)
                m_functions = &unitWeightFunctions;
            if (seedNode == nullptr)
                return 0;
            m_heap.Clear ();
            ResetIncrementalSearchData ();
            SetDistanceToVertex (seedNode, 0.0, false);
            m_heap.Insert (seedNode, 0.0);
            size_t numVertex = 1;
            // All nodes in heap are already reached and given a first distance, but not used for outreach.
            // A smaller distance may have been installed since.
            double baseDistance;
            VuP baseNode;
            while (m_functions->ContinueSearch () && m_heap.RemoveMin (baseNode, baseDistance))
                {
                double a = GetDistanceToVertex (baseNode);  // This may have gotten smaller since baseNode was placed on the heap !!!
                                                            // (but it cannot have gotten larger)
                VU_VERTEX_LOOP (outEdge, baseNode)
                    {
                    VuP returnEdge = outEdge->EdgeMate ();      // This is at another vertex
                    double b = GetEdgeLength (returnEdge);
                    double c0 = GetDistanceToVertex (returnEdge);
                    if (IsUnvisited (returnEdge))
                        numVertex++;
                    double c1 = a + b;
                    if (c1 < c0 && m_functions->CanPush (returnEdge, c1))
                        {
                        SetDistanceToVertex (returnEdge, c1, true);
                        m_heap.Insert (returnEdge, c1);
                        }
                    }
                END_VU_VERTEX_LOOP (outEdge, baseNode)
                }
            return numVertex;
            }


        // Start at seedNode
        // at each vertex reached:
        //   look for outedge with mask.
        //      clear the path mask (only on the side traversed)
        //      clear the edgeClearMask on both sides.
        //      move to face successor (i.e. another vertex)
        // 
        void TraceAndUnmarkEdges (VuP seedNode, VuMask pathMask, VuMask edgeClearMask, bvector<VuP> &reversePath, bvector<VuP> &growingForwardPath, bool pushMate = true)
            {
            reversePath.clear ();
            VuP node = seedNode;
            for (; node != nullptr;)
                {
                node = node->FindMaskAroundVertex (pathMask);
                if (node != nullptr)
                    {
                    reversePath.push_back (node);
                    node->ClearMask (pathMask);
                    node = node->FSucc ();
                    }
                }
            // the new path part is in reverse -- record the edge mates in reverse order
            // unused - VuP currentNode = seedNode;
            for (size_t i = reversePath.size (); i > 0;)
                {
                i--;
                VuP node = reversePath[i];
                VuP mate = node->EdgeMate ();
                if (pushMate)
                    growingForwardPath.push_back (mate);
                else
                    growingForwardPath.push_back (node);
                node->ClearMaskAroundEdge (edgeClearMask);
                }
            }

    };



    static double s_pointFilterTolerance = 1.0e-8;
    struct FlightPlanner
        {
        VuSetP m_graph;
        struct Masks
            {
            VuMask primaryBoundary;
            VuMask primaryExterior;
            VuMask barrierBoundary;
            VuMask barrierExterior;
            VuMask cameraCandidate;
            VuMask cameraEdge;
            };
        Masks m_masks;
        double m_pointFilterTolerance;

        FlightPlanner
            (
                double pointFilterTolerance //!< [in] tolerance for filtering nearby input points.
                )
            {
            m_graph = vu_newVuSet (0);
            m_masks.primaryBoundary = VU_BOUNDARY_EDGE;
            m_masks.primaryExterior = VU_EXTERIOR_EDGE;
            m_masks.barrierBoundary = VU_SEAM_EDGE;
            m_masks.barrierExterior = VU_KNOT_EDGE;
            m_masks.cameraCandidate = VU_RULE_EDGE;
            m_masks.cameraEdge = VU_GRID_EDGE;
            m_pointFilterTolerance = s_pointFilterTolerance;
            if (pointFilterTolerance > m_pointFilterTolerance)
                m_pointFilterTolerance = pointFilterTolerance;
            }
        ~FlightPlanner ()
            {
            vu_freeVuSet (m_graph);
            m_graph = nullptr;
            }
        VuSetP Graph () const { return m_graph; }

        void Clear ()
            {
            vu_reinitializeVuSet (m_graph);
            }

        // ARG.  MakeIndexedLoops doesn't do masks right.  Clear the VU_BOUNDARY_EDGE mask, apply caller's mask
        void CleanupMaskFromMakeIndexedLoop (VuP node, VuMask leftMask, VuMask rightMask)
            {
            VuP mate = node->EdgeMate ();
            node->ClearMaskAroundFace (VU_BOUNDARY_EDGE);
            mate->ClearMaskAroundFace (VU_BOUNDARY_EDGE);
            node->SetMaskAroundFace (leftMask);
            mate->SetMaskAroundFace (rightMask);
            }
        void AddPrimaryBoundary (bvector<DPoint3d>const &points)
            {
            VuP node = VuOps::MakeIndexedLoopsFromArrayWithDisconnects (m_graph, &points, 0, 0, m_pointFilterTolerance);
            CleanupMaskFromMakeIndexedLoop (node, m_masks.primaryBoundary, m_masks.primaryBoundary);
            }

        void AddBarrierBoundary (bvector<DPoint3d>const &points)
            {
            VuP node = VuOps::MakeIndexedLoopsFromArrayWithDisconnects (m_graph, &points, 0, 0, m_pointFilterTolerance);
            CleanupMaskFromMakeIndexedLoop (node, m_masks.barrierBoundary, m_masks.barrierBoundary);
            }



        DRange3d DotRange (DPoint3dCR basePoint, DVec3dCR xVector, DVec3dCR yVector)
            {
            DRange3d range;
            range.Init ();
            DVec3d uVector;
            VU_SET_LOOP (node, m_graph)
                {
                uVector = node->GetXYZ () - basePoint;
                range.Extend (uVector.DotProduct (xVector), uVector.DotProduct (yVector), 0.0);
                }
            END_VU_SET_LOOP (node, m_graph)
                range.Extend (0.0, 0.0, 0.0);
            return range;
            }

        size_t AddRuleLines (DPoint3dCR basePoint, DVec3d ruleDirection, double spacing, bool splitForBasePoint)
            {
            size_t numLines = 0;
            // Find range in parallel and perpendicular directions ...
            DVec3d unitX = ruleDirection;
            unitX.z = 0.0;
            unitX.Normalize ();
            DVec3d unitY = DVec3d::FromCCWPerpendicularXY (unitX);
            DRange3d ruleRange = DotRange (basePoint, unitX, unitY);

            ptrdiff_t iLow = (ptrdiff_t)floor (ruleRange.low.y / spacing);
            ptrdiff_t iHigh = (ptrdiff_t)ceil (ruleRange.high.y / spacing);


            double xA = ruleRange.low.x;
            double xB = ruleRange.high.x;
            BeAssert (iLow <= 0 && iHigh >= 0);     // we know the range includes the base point
            for (ptrdiff_t i = iLow; i <= iHigh; i++)
                {
                double y = i * spacing;
                DPoint3d xyzA = basePoint + unitX * xA + unitY * y;
                DPoint3d xyzB = basePoint + unitX * xB + unitY * y;
                VuP edge = VuOps::MakeEdge (m_graph, xyzA, xyzB, m_masks.cameraCandidate, m_masks.cameraCandidate);
                if (i == 0)
                    {
                    if (splitForBasePoint && xA < 0.0 && xB > 0.0)
                        {
                        VuP leftNode, rightNode;
                        vu_splitEdge (m_graph, edge, &leftNode, &rightNode);
                        leftNode->SetXYZ (basePoint);
                        rightNode->SetXYZ (basePoint);
                        }
                    }
                numLines++;
                }
            return numLines;
            }

        // a cameraEdge is:  a cameraCandidate edge that is interior to primary and exterior to barriers.
        void MarkCameraLines ()
            {
            m_graph->ClearMaskInSet (m_masks.cameraEdge);
            VU_SET_LOOP (node, m_graph)
                {
                if ((node -> HasMask(m_masks.barrierBoundary) || node->HasMask (m_masks.cameraCandidate))
                    && node->HasMask (m_masks.barrierExterior)
                    && !node->HasMask (m_masks.primaryExterior)
                    )
                    {
                    node->SetMask (m_masks.cameraEdge);
                    }
                }
            END_VU_SET_LOOP (node, m_graph);
            }
        // find intersections among various geometry.
        // flood from outside to mark primary and  barrier areas
        void Merge (bool purgeTrueExterior, bool purgeBarrierInterior)
            {
            vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
            vu_regularizeGraph (m_graph);
            vu_floodFromNegativeAreaFaces (m_graph, m_masks.primaryBoundary | m_masks.barrierBoundary, m_masks.primaryExterior);
            vu_floodFromNegativeAreaFaces (m_graph, m_masks.barrierBoundary, m_masks.barrierExterior);
            MarkCameraLines ();

            if (purgeTrueExterior)
                vu_freeEdgesByMaskCount (m_graph, m_masks.primaryExterior, false, false, true);

            if (purgeBarrierInterior)
                vu_freeEdgesByMaskCount (m_graph, m_masks.barrierExterior, true, false, false);

#ifdef FlightPlannerSeesCheck
            if (s_noisy)
                {
                Check::Print (vu_eulerCharacteristic (m_graph), "\nBase Graph Genus");
                Check::Print (m_graph->CountNodesInSet (), "\nTotal nodes");

                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.primaryBoundary), "primaryPolygon");
                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.primaryExterior), "primaryExterior");

                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.barrierBoundary), "barrierPolygon");
                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.barrierExterior), "barrierExterior");

                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.cameraCandidate), "cameraCandate (from rule lines)");
                Check::PrintIndent (0); Check::Print (m_graph->CountMaskedNodesInSet (m_masks.cameraEdge), "cameraEdge (interior part of rule line)");
                }
#endif
            }

        VuP AnyNodeAtClosestVertex (DPoint3dCR xyz)
            {
            double d = DBL_MAX;
            VuP result = nullptr;
            VU_SET_LOOP (node, m_graph)
                {
                double d1 = xyz.DistanceSquaredXY (node->GetXYZ ());
                if (d1 < d)
                    {
                    d = d1;
                    result = node;
                    }
                }
            END_VU_SET_LOOP (node, m_graph);
            return result;
            }

        void MaskString (VuP node, char *buffer)
            {
            int i = 0;
            if (node->HasMask (m_masks.primaryBoundary))
                buffer[i++] = 'B';
            if (node->HasMask (m_masks.primaryExterior))
                buffer[i++] = 'X';
            if (node->HasMask (m_masks.barrierBoundary))
                buffer[i++] = 'b';
            if (node->HasMask (m_masks.barrierExterior))
                buffer[i++] = 'x';
            if (node->HasMask (m_masks.cameraEdge))
                buffer[i++] = 'C';
            if (node->HasMask (m_masks.cameraCandidate))
                buffer[i++] = 'c';
            buffer[i++] = 0;
            }
        void PrintPathNode (VuP node, char const *name)
            {
#ifdef FlightPlannerSeesCheck
            if (s_noisy > 1)
                {
                char maskA[1024], maskB[1024];
                MaskString (node, maskA);
                MaskString (node->EdgeMate (), maskB);

                Check::PrintIndent (2);
                Check::Print (node->id, name);
                Check::Print (node->GetXYZ (), "A");
                printf ("(%s)", maskA);
                printf ("(%s)", maskB);
                Check::Print (node->EdgeMate ()->GetXYZ (), "B");
                }
#endif
            }

        void PrintReversed (bvector<VuP> &nodes, char const *name)
            {
            for (size_t i = nodes.size (); i > 0;)
                {
                i--;
                PrintPathNode (nodes[i], name);
                }
            }

        void PlanPath (VuP seedNode, bvector<VuP> &pathNodes)
            {
            VuMask backEdgeMask = m_graph->GrabMask ();
            VuMask unvisitedCameraMask = m_graph->GrabMask ();
            vu_copyMaskInSet (m_graph, m_masks.cameraEdge, unvisitedCameraMask);
            ShortestPathContext::DistinguishedEdgeXYDistanceSearchFunctions searchForCameraEdge (unvisitedCameraMask);
            bvector<VuP> returnPathNodes;
            VuP currentNode = seedNode;
            ShortestPathContext context (m_graph, backEdgeMask);
            PrintPathNode (seedNode, "BaseXYZ");
            while (currentNode != nullptr)
                {
#ifdef FlightPlannerSeesCheck
                if (s_noisy > 1)
                    {
                    Check::PrintIndent (1);
                    Check::Print (m_graph->CountMaskedNodesInSet (unvisitedCameraMask), "#U");
                    }
#endif
                // follow the unvisitedCameraMask forward ....
                VuP nodeA = currentNode;
                VuP nodeB;
                while (nullptr != (nodeB = nodeA->FindMaskAroundVertex (unvisitedCameraMask)))
                    {
                    PrintPathNode (nodeB, "CAMERA");
                    pathNodes.push_back (nodeB);
                    nodeB->ClearMaskAroundEdge (unvisitedCameraMask);
                    nodeA = nodeB->FSucc ();
                    currentNode = nodeA;
                    }

                searchForCameraEdge.Clear ();
                PrintPathNode (currentNode, "SearchBase");

                context.SearchFromSeed (currentNode, &searchForCameraEdge);
                VuP target = searchForCameraEdge.GetTarget ();
                if (target == nullptr)
                    {
                    pathNodes.push_back (currentNode);
                    // return home ...
                    ShortestPathContext::TargetVertexXYDistanceSearchFunctions searchForTargetNode (seedNode);
                    context.SearchFromSeed (currentNode, &searchForTargetNode);
                    VuP finalNode = searchForTargetNode.GetEntry ();
                    if (nullptr != finalNode)
                        {
                        context.TraceAndUnmarkEdges (finalNode, backEdgeMask, unvisitedCameraMask, returnPathNodes, pathNodes, true);
                        PrintReversed (returnPathNodes, "..");
                        }
                    break;
                    }
                // All backEdge paths end at the target ...
                context.TraceAndUnmarkEdges (target, backEdgeMask, unvisitedCameraMask, returnPathNodes, pathNodes, true);
                PrintReversed (returnPathNodes, "...");
                currentNode = target;
                }
            m_graph->DropMask (unvisitedCameraMask);
            m_graph->DropMask (backEdgeMask);
            }


        };



