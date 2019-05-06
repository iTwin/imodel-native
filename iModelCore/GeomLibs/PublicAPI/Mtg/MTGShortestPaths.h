/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <Mtg/MtgApi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Context for finding shortest path from a seed to all vertices. 
// Search progress is controlled by queries to a caller supplied MTGraphSearchFunctions object
// (or something derived from that)
// The context knows it is walking and marking an MTGGraph.  It does not know particulars
// of what provides path length on each edge -- the MTGGraphSearchFunctions object provides
// that on request of the context.
// 
//<ul>
//<li>A pointer to the graph is carried as private data.
//<li>A mask is borrowed (for the lifetime of the context)
//<li>The context uses MTGNodeId as index into a private array.
//<li>the array entry gives the index of the vertex at that node.
//<li>That vertex index leads to specific search data in a VertexData structure.
//<li>Adding or deleting nodes from the graph during the context lifetime will cause undefined behavior
//</ul>
struct MTGShortestPathContext
    {
    //! Array entry for recording distance to vertices.
    struct VertexData
        {
        double m_a;
        MTGNodeId    m_nodeA;
        MTGNodeId    m_nodeB;
        VertexData() : m_a (0.0), m_nodeA (MTG_NULL_NODEID), m_nodeB (MTG_NULL_NODEID) {}
        VertexData(MTGNodeId nodeA, double a = 0.0, MTGNodeId nodeB = MTG_NULL_NODEID) : m_a (a), m_nodeA (nodeA), m_nodeB (nodeB) {}
        };


    // Callback functions for shortest path search in MTGGraph
    struct MTGGraphSearchFunctions
        {
        MTGGraphP m_graph;
        void SetGraph (MTGGraphP graph) {m_graph = graph;}
        MTGGraphSearchFunctions (MTGGraphP graph) {SetGraph (graph);}
        MTGGraphSearchFunctions (){SetGraph (nullptr);}
        // Return the "length" of an edge as it contributes path length.
        // Default implementation returns 1.0 -- all edges have equal cost.
        GEOMAPI_VIRTUAL double EdgeLength(MTGNodeId node){ return 1.0; }
        // test if a path extending to node, with specified total distance, is allowed.
        // default always returns true.
        GEOMAPI_VIRTUAL bool CanPush(MTGNodeId node, double distance){ return true; }
        // Return false to terminate search.
        GEOMAPI_VIRTUAL bool ContinueSearch(){ return true; }
        // announce an event
        GEOMAPI_VIRTUAL void Announce(char const *message, int data0, int data1){}
        };

    // Direct derived search class to carry graph within facets
    struct MTGFacetsSearchFunctions : MTGGraphSearchFunctions
        {
        MTGFacetsP m_facets;
        MTGFacetsSearchFunctions (MTGFacetsP facets){SetFacets (facets);}
        MTGFacetsSearchFunctions () : m_facets (nullptr) {}

        // stash the facets pointer, also telling the base class about the graph
        void SetFacets (MTGFacetsP facets)
            {
            m_facets = facets;
            SetGraph (facets->GetGraphP ());
            }
        };

    // Callback functions to accesss coordinates in MTGFacets and edge length as search condition
    struct MTGFacetsXYDistanceSearchFunctions : MTGFacetsSearchFunctions
        {
        MTGFacetsXYDistanceSearchFunctions (MTGFacetsP facets){SetFacets (facets);}
        MTGFacetsXYDistanceSearchFunctions () {}

        GEOMAPI_VIRTUAL double EdgeLength(MTGNodeId node){
            MTGNodeId mate = m_graph->EdgeMate (node);
            return m_facets->GetXYZ (node).DistanceXY (m_facets->GetXYZ (mate));
            }
        };

    // Callback functions to accesss coordinates in MTGFacets and edge length as search condition
    struct MTGFacetsXYZDistanceSearchFunctions : MTGFacetsSearchFunctions
        {
        private: MTGFacetsXYZDistanceSearchFunctions () {}
        public:
        MTGFacetsXYZDistanceSearchFunctions (MTGFacetsP facets){SetFacets (facets);}

        GEOMAPI_VIRTUAL double EdgeLength(MTGNodeId node){
            MTGNodeId mate = m_graph->EdgeMate (node);
            return m_facets->GetXYZ (node).Distance (m_facets->GetXYZ (mate));
            }
        };

    // Use dynamic cast to see if functions is a facet searcher or just the base graph searcher.
    // Save facets or graph accordingly
    static void SetGraphOrFacetsInSearchFunctions (MTGGraphSearchFunctions *functions, MTGFacetsP facets)
        {
        MTGFacetsSearchFunctions *facetFunctions = dynamic_cast <MTGFacetsSearchFunctions *>(functions);
        if (nullptr != facetFunctions)
            facetFunctions->SetFacets (facets);
        else
            functions->SetGraph (facets->GetGraphP ());
        };
#define BuildCoordinateBasedSearches_not
#ifdef BuildCoordinateBasedSearches

    // Callback functions:
    // 1) xy edge length is search condition.
    // 2) closest vertex and distance with a distinguished outedge mask is retained.
    // 3) CanPush (a) manages distinguished mask check, and (b) rejects paths that are longer than those seen already
    struct DistinguishedEdgeXYDistanceSearchFunctions : MTGGraphSearchFunctions
        {
        MTGNodeId m_nodeA;
        double m_distanceA;
        MTGMask m_mask;

        MTGNodeId GetTarget () { return m_nodeA; }
        double GetTargetDistance () { return m_distanceA; }

        DistinguishedEdgeXYDistanceSearchFunctions (MTGMask mask)
            : m_mask (mask)
            {
            Clear ();
            }

        void Clear ()
            {
            m_nodeA = MTG_NULL_NODEID;
            m_distanceA = DBL_MAX;
            }
        GEOMAPI_VIRTUAL double EdgeLength(MTGNodeId node){
            MTGNodeId mate = m_graph->EdgeMate (node);
            return node->GetXYZ ().DistanceXY (mate->GetXYZ ());
            }
        GEOMAPI_VIRTUAL bool CanPush(MTGNodeId node, double distance){
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
        bool ContinueSearch () { return m_nodeA == MTG_NULL_NODEID; }
        };

    // Callback functions:
    // 1) xy edge length is search condition.
    // 2) record entry edge and distance when target is reached.
    //   (But allow search to continue because a later path may be shorter)
    // 3) reject path probes for any longer distance.
    struct TargetVertexXYDistanceSearchFunctions : MTGGraphSearchFunctions
        {
        MTGNodeId m_target;       // any node at the target vertex
        MTGNodeId m_entry;        // the node of access to the target vertex.
        double m_targetDistance;
        MTGNodeId GetTarget () { return m_target; }
        MTGNodeId GetEntry () { return m_entry; }

        TargetVertexXYDistanceSearchFunctions (MTGNodeId target)
            : m_target (target)
            {
            Clear ();
            }

        void Clear ()
            {
            m_entry = nullptr;
            m_targetDistance = DBL_MAX;
            }
        GEOMAPI_VIRTUAL double EdgeLength(MTGNodeId node){
            MTGNodeId mate = m_graph->EdgeMate (node);
            return node->GetXYZ ().DistanceXY (mate->GetXYZ ());
            }
        GEOMAPI_VIRTUAL bool CanPush(MTGNodeId node, double distance){
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
#endif
    private:
        GEOMDLLIMPEXP static const int s_nullVertexIndex;
        bvector<VertexData> m_vertexData;
        bvector<int> m_nodeToVertexIndex;
        MTGGraphP m_graph;
        MTGMask m_backEdgeMask;      // mask for a node whose (directed) edge leads back towards the search root.
        MinimumValuePriorityQueue<MTGNodeId> m_heap;

        MTGGraphSearchFunctions *m_functions;

        void SetVertexIndexAtNode (MTGNodeId node, int value)
            {
            if (node >= 0)
                {
                size_t i = (size_t)node;
                for (size_t k = m_nodeToVertexIndex.size (); k < i; k++)
                    m_nodeToVertexIndex.push_back (s_nullVertexIndex);
                m_nodeToVertexIndex[i] = value;
                }
            }

        void SetVertexIndexAroundVertexLoop (MTGNodeId seedNode, int value)
            {
            MTGARRAY_VERTEX_LOOP (currentNode, m_graph, seedNode)
                {
                SetVertexIndexAtNode (currentNode, value);
                }
            MTGARRAY_END_VERTEX_LOOP (currentNode, m_graph, seedNode)
            }
        //! initialize vertex indexing.  This is a full graph operation.
        //!<ul>
        //!<li>Clear all back edges.
        //!<li>Clear all userDataPAsInt.
        //!<li>Clear the vertex array.
        //!</ul>
        void InitializeIncrementalSearchData ()
            {
            m_vertexData.clear ();
            m_graph->ClearMask (m_backEdgeMask);
            // Give each vertex loop an entry in the VertexData array . . .
            MTGARRAY_SET_LOOP (node, m_graph)
                {
                SetVertexIndexAtNode (node, s_nullVertexIndex);
                }
            MTGARRAY_END_SET_LOOP (node, m_graph)
            }

        // Clear the mask and UserDataAsInt around all vertex loops referenced by the m_vertexData array.
        void ResetIncrementalSearchData ()
            {
            for (auto &data : m_vertexData)
                {
                if (data.m_nodeA != MTG_NULL_NODEID)     // (This should always be nonnull)
                    {
                    m_graph->ClearMaskAroundVertex (data.m_nodeA, m_backEdgeMask);
                    SetVertexIndexAroundVertexLoop (data.m_nodeA, s_nullVertexIndex);
                    }
                }
            m_vertexData.clear ();
            }

    public:
        //! Create a search context.
        //!<ul>
        //!<li>the graph pointer is retained in the context.  (Caller is still responsible for freeing it)
        //!<li>the context grabs a mask to use as backEdgeMask
        //!</ul>
        MTGShortestPathContext (MTGGraphP graph)
            : m_graph (graph),
            m_backEdgeMask (graph->GrabMask ()),
            m_functions (nullptr)
            {
            InitializeIncrementalSearchData ();
            }

        ~MTGShortestPathContext ()
            {
            m_graph->DropMask (m_backEdgeMask);
            }

        MTGMask BackEdgeMask () const { return m_backEdgeMask;}

        int NodeToVertexIndex (MTGNodeId node)
            {
            if (node >= 0 && node < ((MTGNodeId)m_nodeToVertexIndex.size ()))
                return m_nodeToVertexIndex[(size_t)node];
            return s_nullVertexIndex;
            }


        double GetDistanceToVertex (MTGNodeId node)
            {
            int index = NodeToVertexIndex (node);
            if (index >= 0 && (size_t)index < m_vertexData.size ())
                return m_vertexData[index].m_a;
            return DBL_MAX;
            }

        double GetEdgeLength (MTGNodeId node)
            {
            return m_functions->EdgeLength (node);
            }

        bool GetVertexData(size_t vertexIndex, MTGNodeId &nodeA, MTGNodeId &nodeB, double distance) const
            {
            if (vertexIndex < m_vertexData.size ())
                {
                nodeA = m_vertexData[vertexIndex].m_nodeA;
                nodeB = m_vertexData[vertexIndex].m_nodeB;
                distance = m_vertexData[vertexIndex].m_a;
                return true;
                }
            nodeA = nodeB = MTG_NULL_NODEID;
            distance = DBL_MAX;
            return false;
            }

        bool GetVertexData(size_t vertexIndex, VertexData &data) const
            {
            if (vertexIndex < m_vertexData.size ())
                {
                data = m_vertexData[vertexIndex];
                return true;
                }
            data = VertexData (MTG_NULL_NODEID, DBL_MAX, MTG_NULL_NODEID);
            return false;
            }

        size_t GetNumVertex () const { return m_vertexData.size (); }

        bool IsUnvisited (MTGNodeId node)
            {
            return DBL_MAX == GetDistanceToVertex (node);
            }
        void SetDistanceToVertex (MTGNodeId node, double distance, bool isBackEdge)
            {
            int index = NodeToVertexIndex (node);
            if (index < 0)
                {
                index = (int)m_vertexData.size ();
                m_vertexData.push_back (VertexData(node, distance, MTG_NULL_NODEID));
                SetVertexIndexAroundVertexLoop (node, index);
                }
            
            if ((size_t)index < m_vertexData.size ())
                {
                m_vertexData[(size_t)index].m_a = distance;
                m_graph->ClearMaskAroundVertex (node, m_backEdgeMask);
                m_vertexData[(size_t)index].m_nodeB = MTG_NULL_NODEID;
                if (isBackEdge)
                    {
                    m_vertexData[(size_t)index].m_nodeB = node;
                    m_graph->SetMaskAt (node, m_backEdgeMask);
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
        size_t SearchFromSeed (MTGNodeId seedNode, MTGGraphSearchFunctions *functions)
            {
            m_functions = functions;
            if (seedNode == MTG_NULL_NODEID)
                return 0;
            m_heap.Clear ();
            ResetIncrementalSearchData ();
            SetDistanceToVertex (seedNode, 0.0, false);
            m_heap.Insert (seedNode, 0.0);
            size_t numVertex = 1;
            // All nodes in heap are already reached and given a first distance, but not used for outreach.
            // A smaller distance may have been installed since.
            double baseDistance;
            MTGNodeId baseNode;
            while (m_functions->ContinueSearch () && m_heap.RemoveMin (baseNode, baseDistance))
                {
                double a = GetDistanceToVertex (baseNode);  // This may have gotten smaller since baseNode was placed on the heap !!!
                                                            // (but it cannot have gotten larger)
                MTGARRAY_VERTEX_LOOP (outEdge, m_graph, baseNode)
                    {
                    MTGNodeId returnEdge = m_graph->EdgeMate (outEdge);      // This is at another vertex
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
                MTGARRAY_END_VERTEX_LOOP (outEdge, m_graph, baseNode)
                }
            return numVertex;
            }
#define Compile_MarkShortestPathBetweenLoops
#ifdef Compile_MarkShortestPathBetweenLoops
    private:
        // return true if a barrier is found. (And nodes is a subset of the vertex loop)
        // return false if no barrier (i.e. full vertex loop in nodes)
        bool CollectSectorNodes (MTGNodeId entryNode, MTGMask barrierMask, bvector<MTGNodeId> &nodes)
            {
            nodes.clear ();
            MTGNodeId baseNode = MTGGraph::NullNodeId;
            MTGARRAY_VERTEX_PREDLOOP (node, m_graph, entryNode)
                {
                if (m_graph->HasMaskAt (node, barrierMask))
                    {
                    baseNode = node;
                    break;
                    }
                }
            MTGARRAY_END_VERTEX_PREDLOOP (node, m_graph, entryNode)

            if (m_graph->IsValidNodeId (baseNode))
                {
                MTGARRAY_VERTEX_LOOP (node, m_graph, baseNode)
                    {
                    nodes.push_back (node);
                    MTGNodeId leftNode = m_graph->FPred (node);
                    if (m_graph->HasMaskAt (leftNode, barrierMask))
                        break;
                    }
                MTGARRAY_END_VERTEX_LOOP (node, m_graph, baseNode)
                return true;
                }
            else
                {
                // put the entire vertex loop in the array.
                MTGARRAY_VERTEX_LOOP (node, m_graph, entryNode)
                    {
                    nodes.push_back (node);
                    }
                MTGARRAY_END_VERTEX_LOOP (node, m_graph, entryNode)
                return false;
                }
            }
        // start at vertex with entryNode.
        // * see if current node is in a boundary sector.  If so, done.
        // * if not, found outbound mask and continue at the far end of that edge.
        // return true if a boundary was reached.
        bool TraceToBoundary (MTGNodeId entryNode, MTGMask barrierMask, MTGMask traceMask, bvector<MTGNodeId> &boundarySectorNodes, bvector<MTGNodeId> &traceNodes)
            {
            MTGNodeId currentSeed = entryNode;
            while (!CollectSectorNodes (currentSeed, barrierMask, boundarySectorNodes))
                {
                MTGNodeId outbound = m_graph->FindMaskAroundVertex (currentSeed, traceMask);
                if (!m_graph->IsValidNodeId (outbound))
                    return false;
                traceNodes.push_back (outbound);
                currentSeed = m_graph->FSucc (outbound);
                }
            // fell through on boundary contact
            return true;
            }

        void SetMaskAroundEdges (bvector<MTGNodeId> const &nodes, MTGMask mask)
            {
            for (auto node : nodes)
                m_graph->SetMaskAroundEdge (node, mask);
            }
    public:
        // Run a search from multiple seeds edges defined as "zero distance"
        // Search terminates when entire graph is reached or functions->ContinueSearch () returns false.
        // <ul>
        // <li>loopMask denotes nodes that are "seeds" 
        // <li>loopMask is assumed to have proper parity around all vertices.
        // <li>All nodes are assigned as singletons in the union-find structure.
        // <li>seedMask nodes and the VSucc neighbors "within sector" are declared "visited" at zero distance in the same cluster.
        // <li>seedMask nodes are "unioned" with their successors reached by FSucc and repeated VPred.
        // <li>Dijkstra search determined shortest paths away from the visited nodes.
        // <li>When shortest path reaches a node with a diff
        // </ul>
 
        void MarkShortestPathBetweenLoops (MTGMask loopMask, MTGMask pathMask, MTGGraphSearchFunctions *functions)
            {
            m_functions = functions;
            m_heap.Clear ();
            MTGGraph::ScopedMask backEdgeMask (*m_graph);
            size_t numNode = m_graph->GetNodeIdCount ();
            struct NodeAssignment
                {
                size_t m_clusterIndex;
                double m_distance;
                NodeAssignment (size_t clusterIndex, double distance)
                    : m_clusterIndex (clusterIndex), m_distance (distance)
                    {}
                };
            bvector<NodeAssignment> nodeToCluster;
            bvector<size_t> clusterUF;
            bvector<MTGNodeId> nodesInSector;   // work array, reused throughout.
            bvector<MTGNodeId> baseToBoundary;
            bvector<MTGNodeId> farToBoundary;
            for (size_t i = 0; i < numNode; i++)
                nodeToCluster.push_back (NodeAssignment (SIZE_MAX, DBL_MAX));
            
         

            // Make each loop edge into its own cluster and 0 distance
            MTGARRAY_SET_LOOP (baseNode, m_graph)
                {
                if (m_graph->HasMaskAt (baseNode, loopMask))
                    {
                    BeAssert ((size_t)baseNode < numNode);
                    nodeToCluster[baseNode] = NodeAssignment (UnionFind::NewClusterIndex (clusterUF), 0.0);
                    }
                }
            MTGARRAY_END_SET_LOOP (baseNode, m_graph)
 

            // Gather outbound edges "around the sector" into the cluster.
            // Merge the bounding outbound edges.
            MTGARRAY_SET_LOOP (baseNode, m_graph)
                {
                if (m_graph->HasMaskAt (baseNode, loopMask))
                    {
                    size_t baseCluster = nodeToCluster[baseNode].m_clusterIndex;
                    CollectSectorNodes (baseNode, loopMask, nodesInSector);
                    MTGNodeId leftEdge = m_graph->FPred (nodesInSector.back ());
                    if (m_graph->HasMaskAt (leftEdge, loopMask))
                        UnionFind::MergeClusters (clusterUF, baseCluster, nodeToCluster[leftEdge].m_clusterIndex);
                    double baseDistance = 0.0;
                    for (size_t i = 1; i < nodesInSector.size (); i++)
                        {
                        MTGNodeId rightEdge = nodesInSector[i];
                        // make this outbound node part of the base cluster at known distance
                        nodeToCluster[rightEdge] = NodeAssignment (baseCluster, baseDistance);

                        // make the far end available with known distance 
                        double edgeLength = functions->EdgeLength (rightEdge);
                        m_heap.Insert (rightEdge, baseDistance + edgeLength);
                        }
                    }
                }
            MTGARRAY_END_SET_LOOP (baseNode, m_graph)

            for (size_t i = 0; i < clusterUF.size (); i++)
                {
                UnionFind::FindClusterRoot (clusterUF, i);
                }

            MTGNodeId baseNode;
            double baseDistance;
            while (m_functions->ContinueSearch () && m_heap.RemoveMin (baseNode, baseDistance))
                {
                MTGNodeId farNode = m_graph->FSucc (baseNode);
                size_t baseCluster = nodeToCluster[baseNode].m_clusterIndex;
                BeAssert (baseCluster != SIZE_MAX);
                size_t farCluster = nodeToCluster[farNode].m_clusterIndex;
                functions->Announce (" Search along edge (base,far)", baseNode, farNode);
 
                if (farCluster == SIZE_MAX)
                    {
                    // baseNode is on the shortest path to the far vertex.
                    // far vertex has never been visited
                    // make the edge mate a back edge
                    functions->Announce ("    New Interior Vertex (base,far)", baseNode, farNode);


                    MTGNodeId mate = m_graph->EdgeMate (baseNode);
                    m_graph->SetMaskAt (mate, backEdgeMask.Get ());
                    // install the known distance
                    CollectSectorNodes (farNode, loopMask, nodesInSector);
                    for (auto rightEdge : nodesInSector)
                        {
                        nodeToCluster [rightEdge] = NodeAssignment (baseCluster, baseDistance);
                        // make the far end available with known distance.
                        if (m_graph->EdgeMate (rightEdge) != baseNode)
                            {
                            double edgeLength = functions->EdgeLength (rightEdge);
                            m_heap.Insert (rightEdge, baseDistance + edgeLength);
                            }
                        }
                    }
                else
                    {

                    size_t baseRoot = UnionFind::FindClusterRoot (clusterUF, baseCluster);
                    size_t farRoot  = UnionFind::FindClusterRoot (clusterUF, farCluster);
                    if (baseRoot == farRoot)
                        {
                        // This is a redundant (and not shortest) path to this vertex.  Leave it alone.
                        // There is already a back edge leaving from somewhere around the vertex.  But we don't need to find it.
                        functions->Announce ("    Second path to same cluster (baseCluster,farCluster)", (int)baseRoot, (int)farRoot);
                        }
                    else
                        {
                        // Paths from distinct clusters have met.
                        // baseNode and farNode are in distinc vertex loops, each with a (possibly zero length) path back to a boundary
                        functions->Announce ("   Path joins clusters (base,far) (baseCluster,farCluster)", (int)baseRoot, (int)farRoot);

                        bool baseOk = TraceToBoundary (baseNode, loopMask, backEdgeMask.Get (), nodesInSector, baseToBoundary);
                        bool farOk =  TraceToBoundary (farNode,  loopMask, backEdgeMask.Get (), nodesInSector, farToBoundary);
                        UnionFind::MergeClusters (clusterUF, baseCluster, farCluster);
                        if (baseOk && farOk)
                            {
                            SetMaskAroundEdges (baseToBoundary, loopMask);
                            SetMaskAroundEdges (farToBoundary, loopMask);
                            m_graph->SetMaskAroundEdge (baseNode, loopMask);
                            }
                        }
                    }
                }
            }
#endif



        // Start at seedNode
        // at each vertex reached:
        //   look for outedge with mask.
        //      clear the path mask (only on the side traversed)
        //      clear the edgeClearMask on both sides.
        //      move to face successor (i.e. another vertex)
        // 
        void TraceAndUnmarkEdges (MTGNodeId seedNode, MTGMask pathMask, MTGMask edgeClearMask, bvector<MTGNodeId> &reversePath, bvector<MTGNodeId> &growingForwardPath, bool pushMate = true)
            {
            reversePath.clear ();
            MTGNodeId node = seedNode;
            for (; node != MTG_NULL_NODEID;)
                {
                node = m_graph->FindMaskAroundVertex(node, pathMask);
                if (node != MTG_NULL_NODEID)
                    {
                    reversePath.push_back (node);
                    m_graph->ClearMaskAt (node, pathMask);
                    node = m_graph->FSucc (node);
                    }
                }
            // the new path part is in reverse -- record the edge mates in reverse order
            // unused - MTGNodeId currentNode = seedNode;
            for (size_t i = reversePath.size (); i > 0;)
                {
                i--;
                MTGNodeId node = reversePath[i];
                MTGNodeId mate = m_graph->EdgeMate (node);
                if (pushMate)
                    growingForwardPath.push_back (mate);
                else
                    growingForwardPath.push_back (node);
                m_graph->ClearMaskAroundEdge (node, edgeClearMask);
                }
            }

    };


#ifdef CompileMTGFlightPlanner
    static double s_pointFilterTolerance = 1.0e-8;
    struct FlightPlanner
        {
        MTGGraphP m_graph;
        struct Masks
            {
            MTGMask primaryBoundary;
            MTGMask primaryExterior;
            MTGMask barrierBoundary;
            MTGMask barrierExterior;
            MTGMask cameraCandidate;
            MTGMask cameraEdge;
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
        MTGGraphP Graph () const { return m_graph; }

        void Clear ()
            {
            vu_reinitializeVuSet (m_graph);
            }

        // ARG.  MakeIndexedLoops doesn't do masks right.  Clear the VU_BOUNDARY_EDGE mask, apply caller's mask
        void CleanupMaskFromMakeIndexedLoop (MTGNodeId node, MTGMask leftMask, MTGMask rightMask)
            {
            MTGNodeId mate = m_graph->EdgeMate (node);
            m_graph->ClearMaskAroundFace (node, VU_BOUNDARY_EDGE);
            mate->ClearMaskAroundFace (VU_BOUNDARY_EDGE);
            m_graph->SetMaskAroundFace (node, leftMask);
            mate->SetMaskAroundFace (rightMask);
            }
        void AddPrimaryBoundary (bvector<DPoint3d>const &points)
            {
            MTGNodeId node = VuOps::MakeIndexedLoopsFromArrayWithDisconnects (m_graph, &points, 0, 0, m_pointFilterTolerance);
            CleanupMaskFromMakeIndexedLoop (node, m_masks.primaryBoundary, m_masks.primaryBoundary);
            }

        void AddBarrierBoundary (bvector<DPoint3d>const &points)
            {
            MTGNodeId node = VuOps::MakeIndexedLoopsFromArrayWithDisconnects (m_graph, &points, 0, 0, m_pointFilterTolerance);
            CleanupMaskFromMakeIndexedLoop (node, m_masks.barrierBoundary, m_masks.barrierBoundary);
            }



        DRange3d DotRange (DPoint3dCR basePoint, DVec3dCR xVector, DVec3dCR yVector)
            {
            DRange3d range;
            range.Init ();
            DVec3d uVector;
            MTGARRAY_SET_LOOP (node, m_graph)
                {
                uVector = node->GetXYZ () - basePoint;
                range.Extend (uVector.DotProduct (xVector), uVector.DotProduct (yVector), 0.0);
                }
            MTGARRAY_END_SET_LOOP (node, m_graph)
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
                MTGNodeId edge = VuOps::MakeEdge (m_graph, xyzA, xyzB, m_masks.cameraCandidate, m_masks.cameraCandidate);
                if (i == 0)
                    {
                    if (splitForBasePoint && xA < 0.0 && xB > 0.0)
                        {
                        MTGNodeId leftNode, rightNode;
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
            MTGARRAY_SET_LOOP (node, m_graph)
                {
                if ((node -> HasMask(m_masks.barrierBoundary) || node->HasMask (m_masks.cameraCandidate))
                    && node->HasMask (m_masks.barrierExterior)
                    && !node->HasMask (m_masks.primaryExterior)
                    )
                    {
                    node->SetMask (m_masks.cameraEdge);
                    }
                }
            MTGARRAY_END_SET_LOOP (node, m_graph);
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

        MTGNodeId AnyNodeAtClosestVertex (DPoint3dCR xyz)
            {
            double d = DBL_MAX;
            MTGNodeId result = nullptr;
            MTGARRAY_SET_LOOP (node, m_graph)
                {
                double d1 = xyz.DistanceSquaredXY (node->GetXYZ ());
                if (d1 < d)
                    {
                    d = d1;
                    result = node;
                    }
                }
            MTGARRAY_END_SET_LOOP (node, m_graph);
            return result;
            }

        void MaskString (MTGNodeId node, char *buffer)
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
        void PrintPathNode (MTGNodeId node, char const *name)
            {
#ifdef FlightPlannerSeesCheck
            if (s_noisy > 1)
                {
                char maskA[1024], maskB[1024];
                MaskString (node, maskA);
                MaskString (m_graph->EdgeMate (node), maskB);

                Check::PrintIndent (2);
                Check::Print (node->id, name);
                Check::Print (node->GetXYZ (), "A");
                printf ("(%s)", maskA);
                printf ("(%s)", maskB);
                Check::Print (m_graph->EdgeMate (node)->GetXYZ (), "B");
                }
#endif
            }

        void PrintReversed (bvector<MTGNodeId> &nodes, char const *name)
            {
            for (size_t i = nodes.size (); i > 0;)
                {
                i--;
                PrintPathNode (nodes[i], name);
                }
            }

        void PlanPath (MTGNodeId seedNode, bvector<MTGNodeId> &pathNodes)
            {
            MTGMask backEdgeMask = m_graph->GrabMask ();
            MTGMask unvisitedCameraMask = m_graph->GrabMask ();
            vu_copyMaskInSet (m_graph, m_masks.cameraEdge, unvisitedCameraMask);
            MTGShortestPathContext::DistinguishedEdgeXYDistanceSearchFunctions searchForCameraEdge (unvisitedCameraMask);
            bvector<MTGNodeId> returnPathNodes;
            MTGNodeId currentNode = seedNode;
            MTGShortestPathContext context (m_graph, backEdgeMask);
            PrintPathNode (seedNode, "BaseXYZ");
            while (currentNode != MTG_NULL_NODEID)
                {
#ifdef FlightPlannerSeesCheck
                if (s_noisy > 1)
                    {
                    Check::PrintIndent (1);
                    Check::Print (m_graph->CountMaskedNodesInSet (unvisitedCameraMask), "#U");
                    }
#endif
                // follow the unvisitedCameraMask forward ....
                MTGNodeId nodeA = currentNode;
                MTGNodeId nodeB;
                while (nullptr != (nodeB = m_graph->FindMaskAroundVertex(nodeA, unvisitedCameraMask)))
                    {
                    PrintPathNode (nodeB, "CAMERA");
                    pathNodes.push_back (nodeB);
                    m_graph->ClearMaskAroundEdge (nodeB, unvisitedCameraMask);
                    nodeA = m_graph->FSucc (nodeB);
                    currentNode = nodeA;
                    }

                searchForCameraEdge.Clear ();
                PrintPathNode (currentNode, "SearchBase");

                context.SearchFromSeed (currentNode, &searchForCameraEdge);
                MTGNodeId target = searchForCameraEdge.GetTarget ();
                if (target == nullptr)
                    {
                    pathNodes.push_back (currentNode);
                    // return home ...
                    MTGShortestPathContext::TargetVertexXYDistanceSearchFunctions searchForTargetNode (seedNode);
                    context.SearchFromSeed (currentNode, &searchForTargetNode);
                    MTGNodeId finalNode = searchForTargetNode.GetEntry ();
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
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
