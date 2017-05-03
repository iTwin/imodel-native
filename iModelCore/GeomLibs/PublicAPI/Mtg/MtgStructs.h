/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Mtg/MtgStructs.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#define __jmtgH__

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MTG_MAX_LABEL_PER_NODE       16

/*----------------------------------------------------------------------+
| Every node has 0 to 16 labels.                                        |
| Every label is an integer.                                            |
| The graph header maintains a mask indicating special properties of    |
| each label.                                                           |
+----------------------------------------------------------------------*/

#define    ___MTGLabelMask_EdgeProperty      0x00000001
#define    ___MTGLabelMask_VertexProperty    0x00000002
#define    ___MTGLabelMask_SectorProperty    0x00000004
#define    ___MTGLabelMask_FaceProperty      0x00000008
#define    ___MTGLabelMask_None       0x00000000



typedef enum
    {
    MTG_LabelMask_EdgeProperty      = ___MTGLabelMask_EdgeProperty,
    MTG_LabelMask_VertexProperty    = ___MTGLabelMask_VertexProperty,
    MTG_LabelMask_SectorProperty    = ___MTGLabelMask_SectorProperty,
    MTG_LabelMask_FaceProperty      = ___MTGLabelMask_FaceProperty,
    MTG_LabelMask_None              = ___MTGLabelMask_None
    } MTGLabelMask;


/*----------------------------------------------------------------------+
Reserved label tags allow specialized apps to get communicate label
layouts without adding args ....
-----------------------------------------------------------------------*/
typedef enum
    {
    MTGReservedLabelTag_ComponentIndex = -7001,
    MTGReservedLabelTag_InternalIndex  = -7002,
    } MTGReservedLabelTag;

/*----------------------------------------------------------------------+
| Some search functions start at any node and use an enumerated         |
| to distinguish the scope (extent) of the search.                      |
-----------------------------------------------------------------------*/
#define    ___MTGScopeNode          0
#define    ___MTGScopeEdge          1
#define    ___MTGScopeVertex       2
#define    ___MTGScopeFace          3
#define    ___MTGScopeComponent    4


typedef enum
    {
    MTG_ScopeNode           = ___MTGScopeNode,
    MTG_ScopeEdge           = ___MTGScopeEdge,
    MTG_ScopeVertex         = ___MTGScopeVertex,
    MTG_ScopeFace           = ___MTGScopeFace,
    MTG_ScopeComponent      = ___MTGScopeComponent
    } MTGMarkScope;


/*----------------------------------------------------------------------+
| Facet structures may have 3 levels of data per vertex:                |
|   VertexOnly -- Each node has a single vertex data label, which is    |
|       the index of the vertex coordinates in the vertex array. No     |
|       normal data is stored.                                          |
|   NormalPerVertex -- Each node has a single vertex data label, which  |
|       is used to index both the vertex coordinates and normal arrays. |
|       Different nodes around a single vertex can have different       |
|       normals only by replicating the vertex coordinates.             |
|   SeparateNormals -- Each node has two index labels.  One leads to    |
|       vertex coordinates, the other to normals.   Multiple nodes at   |
|       a vertex can share the single vertex coordinates but have       |
|       independent normals.                                            |
-----------------------------------------------------------------------*/


#define    ___MTGFacets_NoData              0
#define    ___MTGFacets_VertexOnly          1
#define    ___MTGFacets_NormalPerVertex    2
#define    ___MTGFacets_SeparateNormals    3


typedef enum
    {
    MTG_Facets_NoData           = ___MTGFacets_NoData,
    MTG_Facets_VertexOnly       = ___MTGFacets_VertexOnly,
    MTG_Facets_NormalPerVertex  = ___MTGFacets_NormalPerVertex,
    MTG_Facets_SeparateNormals  = ___MTGFacets_SeparateNormals
    } MTGFacets_NormalMode;


/*-----------------------------------------------------------------------
 Enumeration of kinds of clipping.
 All functions that use this type should have complete enumeration --
 be sure default case drops out in benign way.
-----------------------------------------------------------------------*/
#define    ___MTGClipOp_KeepIn    0
#define    ___MTGClipOp_KeepOut   1
#define    ___MTGClipOp_KeepOn    2

typedef enum
    {
    MTG_ClipOp_KeepIn,
    MTG_ClipOp_KeepOut,
    MTG_ClipOp_KeepOn
    } MTGClipOp;

#define MTG_NULL_NODEID (-1)
#define MTG_NULL_MASK (0)
#define MTG_NOT_VISITED (-1)

#define MTG_MARKER_START_FACE           (-10)
#define MTG_MARKER_END_FACE             (-11)
#define MTG_MARKER_START_VERTEX         (-20)
#define MTG_MARKER_END_VERTEX           (-21)
#define MTG_MARKER_START_COMPONENT      (-30)
#define MTG_MARKER_END_COMPONENT        (-31)

/*--------------------------------------------------------------------------+
| Each node has a 32 bit mask field.  16 masks are offered via              |
| 'grab/drop' mechanism.  The other 16 are reserved for specialized use     |
| by the class and native library.                                          |
| The reserved masks have descriptive names.                                |
| In mjava, see interface MTGGraph.INodeMask                                |
+--------------------------------------------------------------------------*/
#define MTG_EXTERIOR_MASK           0x00000001
#define MTG_BOUNDARY_MASK           0x00000002
#define MTG_CONSTU_MASK             0x00000004
#define MTG_CONSTV_MASK             0x00000008
#define MTG_USEAM_MASK              0x00000010
#define MTG_VSEAM_MASK              0x00000020
#define MTG_BOUNDARY_VERTEX_MASK    0x00000040
#define MTG_PRIMARY_VERTEX_MASK     0x00000080
#define MTG_DIRECTED_EDGE_MASK      0x00000100
#define MTG_PRIMARY_EDGE_MASK       0x00000200

#define MTG_HULL_MASK               0x00000400
#define MTG_SECTION_EDGE_MASK       0x00000800
#define MTG_POLAR_LOOP_MASK         0x00001000

#define MTG_ALL_MASK_BITS           0xFFFFFFFF

/*--------------------------------------------------------------------------+
| Searches which can halt at a face, edge, or vertex return one of these    |
| constants.                                                                |
+--------------------------------------------------------------------------*/
#define MTG_SEARCH_UNKNOWN          0
#define MTG_SEARCH_EXTERIOR_FACE    1
#define MTG_SEARCH_ON_VERTEX        2
#define MTG_SEARCH_ON_EDGE          3
#define MTG_SEARCH_INTERIOR_FACE    4

#define MTGTRIANGULATION_FIELDS(__EmbeddedIntArrayType__)\
    /**                                                 \
    * Tolerance for declaring points identical.         \
    */                                                  \
    double                  tolerance;                  \
    /**                                                 \
    * Possible node id for restarting searches.         \
    */                                                  \
    MTGNodeId               previousNodeId;             \
    /**                                                 \
    * Array of edges that may need to be adjusted.      \
    */                                                  \
    __EmbeddedIntArrayType__  edgeCandidateArrayHdr;




#define MTGFACETSSEARCHCONTEXT_FIELDS                   \
    /**                                                 \
    * Tolerance for declaring points identical.         \
    */                                                  \
    double                  tolerance;                  \
    /**                                                 \
    * Possible node id for restarting searches.         \
    */                                                  \
    MTGNodeId               previousNodeId;             \
    /**                                                 \
    * boundary crossing control.                        \
    */                                                  \
    bool                    crossBoundaries;





typedef     int MTGNodeId;
typedef     int MTGMask;
//typedef struct _EmbeddedStructArray EmbeddedStructArray;

struct MTGNodeIdPair
    {
    MTGNodeId nodeId[2];
    void Init ();
    MTGNodeIdPair ()
        {
        nodeId[0] = MTG_NULL_NODEID;
        nodeId[1] = MTG_NULL_NODEID;
        }
    MTGNodeIdPair (MTGNodeId node0, MTGNodeId node1)
        {
        nodeId[0] = node0;
        nodeId[1] = node1;
        }
    };

/*---------------------------------------------------------------------------------**//**
*
* An MTGGraph structure defines connectivity of a graph structure for manifold
* topology.
*
* @bsiclass                                                     EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
struct MTG_Node
    {
    MTGNodeId   vSucc;
    MTGNodeId   fSucc;
    MTGMask     mask;
    MTG_Node ()
        {
        vSucc = fSucc = 0;
        mask = 0;
        }
    MTG_Node (MTGNodeId v, MTGNodeId f, MTGMask m)
        {
        vSucc = v;
        fSucc = f;
        mask  = m;
        }

    void Initialize ()
        {
        vSucc = fSucc = 0;
        mask = 0;
        }
    };


struct MTGGraph
{
public:
    static const MTGNodeId NullNodeId = MTG_NULL_NODEID;
private:
// Vector of label values.
//  To be indexed by nodeId.
//  Carries default value, tag, behavior mask.
struct LabelSet : bvector <int>
    {
    int m_defaultValue;
    int m_tag;
    MTGLabelMask m_labelMask;
    LabelSet (int defaultValue, int tag, MTGLabelMask labelMask)
        : m_defaultValue (defaultValue),
          m_tag (tag),
          m_labelMask (labelMask)
        {
        }
    };

bvector<MTG_Node>       m_nodeData;
bvector<LabelSet>  m_nodeLabels;

MTGNodeId               firstFreeNodeId;// Start of free list.
int                     numActiveNodes; // Total minus free list
int                     numFreeNodes;   // Free list

MTGMask                 m_freeMasks;      // number of masks available for lending
MTGMask            m_edgePropertyMask;       // masks that apply as edge properties
MTGMask            m_vertexPropertyMask;     // masks that apply as vertex properties
MTGMask            m_facePropertyMask;   // masks that apply as face properties

// DEPRECATED node deletion -- this also destroys label definitions.
void DeleteNodes ();
// DEPRECATED
void InitNonNodeParts (bool preserveLabels);


// Create a new, zeroed node with default label values.
MTGNodeId NewNodeId ();
// Get an available node, possibly from recycles.
MTGNodeId GetNewOrRecycledNode ();
// Return ids and pointer to a pair of new nodes.
bool CreateTwoNodes
(
MTGNodeId       *pId0,          // <= start id of new edge
MTGNodeId       *pId1,          // <= end id of new edge
MTG_Node        **pNode0,       // <= node pointer
MTG_Node        **pNode1        // <= end node pointer
);
// put nodeId in the linked list of free nodes.
// The node is assumed to have been disuconnected by the caller
// so its fields are free for use in free list management.
void ReleaseNode (MTGNodeId);
friend struct MTGGraphInternalAccess;


bool TryGetFullMask (MTGNodeId, MTGMask &value) const;
bool TrySetFullMask (MTGNodeId, MTGMask value);

public:

GEOMDLLIMPEXP MTGGraph ();

/// delete nodes. (But preserve memory allocations)
/// @param [in] preserveDefs true if caller will reuse graph with same label layout and mask properties.
GEOMDLLIMPEXP void ClearNodes (bool preserveDefs);


/// Insert a new vertex internal to existing edge.
/// @param [out] newLeftNodeId newly created node on same side as baseNodeId.
/// @param [out] newRightNodeId newly created node on opposite side.
/// @param [in] baseNodeId node at base of edge being split.
GEOMDLLIMPEXP bool SplitEdge (MTGNodeId &newLeftNodeId, MTGNodeId &newRightNodeId, MTGNodeId baseNodeId);

/// Create an edge between two existing nodes.
/// @param [out] idNewA new node in vertex loop at idA.
/// @param [out] idNewB new node in vertex loop at idB.
/// @param [in] idA existing node
/// @param [in] idB existing node
/// @param [in] maskA mask to apply on the outbound side
/// @param [in] maskB mask to apply on the outbound side
GEOMDLLIMPEXP bool Join (
MTGNodeId      &idNewA,
MTGNodeId      &idNewB,
MTGNodeId      idA,
MTGNodeId      idB,
MTGMask maskA,
MTGMask maskB
);

// Detach both ends of an edge from the graph and drop the nodes to the free list.
GEOMDLLIMPEXP bool DropEdge (MTGNodeId nodeId);
// Drop all edges with mask at either end.  Return number dropped.
GEOMDLLIMPEXP size_t DropMaskedEdges (MTGMask mask);
GEOMDLLIMPEXP void SetMaskProperty (MTGMask mask, MTGLabelMask prop, bool value);
GEOMDLLIMPEXP MTGMask GetMaskProperty (MTGMask mask, MTGLabelMask propertyMask) const;


MTG_Node *GetNodeP (MTGNodeId nodeId);
MTG_Node const *GetNodeCP (MTGNodeId nodeId) const;
// Apply all defaults to labels of specified node.
void SetDefaultLabels (MTGNodeId nodeId);

/// Return the number of nodes (some of which may be unused) in the node numbering.
GEOMDLLIMPEXP size_t GetNodeIdCount () const;
/// Return the number of actually active nodes.
GEOMDLLIMPEXP size_t GetActiveNodeCount () const;
/// Return the number of labels defined.
GEOMDLLIMPEXP int GetLabelCount () const;
// tests both range and content !!!
GEOMDLLIMPEXP bool IsValidNodeId (MTGNodeId nodeId) const;
/// Tests if index is a valid nodeId, but does not test if it is actually in used.
GEOMDLLIMPEXP bool IsValidLabelIndex (int index) const;
/// Return any valid node id in the graph.
GEOMDLLIMPEXP MTGNodeId AnyValidNodeId () const;

//EG SM: added for serialization, wip
GEOMDLLIMPEXP size_t WriteToBinaryStream(void*& serialized);
GEOMDLLIMPEXP void LoadFromBinaryStream(void* serialized, size_t ct);

/// Node masks.
/// Every node has a 32 bit mask.
/// To obtain a single bit mask for application use, call graph->GrabMask ().
/// At end of algorithm release the bit with graph->DropMask(mask).
/// GrabMask does NOT clear or set the mask in the whole set.  Use graph->SetMask (mask)
///   or graph->ClearMask(mask) to set or clear at all nodes.
GEOMDLLIMPEXP MTGMask GrabMask ();
/// Return a mask bit to the available pool.
GEOMDLLIMPEXP void DropMask (MTGMask mask);
/// Set mask bit to one in a single node.
GEOMDLLIMPEXP void SetMaskAt (MTGNodeId nodeId, MTGMask mask);
/// Clear mask bit to zero in a single node.
GEOMDLLIMPEXP void ClearMaskAt (MTGNodeId nodeId, MTGMask mask);
/// Return masked bit in nodeId.  Set mask bit to one.
GEOMDLLIMPEXP MTGMask TestAndSetMaskAt (MTGNodeId nodeId, MTGMask mask);
/// Return masked bits (logical or with mask)
GEOMDLLIMPEXP MTGMask GetMaskAt (MTGNodeId nodeId, MTGMask mask) const;
/// Test if mask is set -- return as simple bool
GEOMDLLIMPEXP bool HasMaskAt (MTGNodeId ndoeId, MTGMask mask) const;
/// Clear mask bit at all nodes in the graph.
GEOMDLLIMPEXP void ClearMask (MTGMask mask);
/// Set mask bit to one at all nodes in the graph.
GEOMDLLIMPEXP void SetMask (MTGMask mask);
/// Reverse mask bit at all nodes in the graph
GEOMDLLIMPEXP void ReverseMask (MTGMask mask);

/// Set mask bit to one at all nodes around a face.
GEOMDLLIMPEXP void SetMaskAroundFace (MTGNodeId nodeId, MTGMask mask);
/// Set mask bit to one at all nodes around a vertex.
GEOMDLLIMPEXP void SetMaskAroundVertex (MTGNodeId nodeId, MTGMask mask);
/// Set mask bit to one at all nodes around an edge (2 sides)
GEOMDLLIMPEXP void SetMaskAroundEdge (MTGNodeId nodeId, MTGMask mask);
/// Clear mask bit to zero at all nodes around a face.
GEOMDLLIMPEXP void ClearMaskAroundFace (MTGNodeId nodeId, MTGMask mask);
/// Clear mask bit to zero at all nodes around a vertex.
GEOMDLLIMPEXP void ClearMaskAroundVertex (MTGNodeId nodeId, MTGMask mask);
/// Clear mask bit to zero at all nodes around an edge.
GEOMDLLIMPEXP void ClearMaskAroundEdge (MTGNodeId nodeId, MTGMask mask);
/// Count the nodes around a face.
GEOMDLLIMPEXP size_t CountNodesAroundFace   (MTGNodeId nodeId) const;
/// Count the nodes around a vertex.
GEOMDLLIMPEXP size_t CountNodesAroundVertex (MTGNodeId nodeId) const;
/// Count nodes with mask around a face
GEOMDLLIMPEXP size_t CountMaskAroundFace   (MTGNodeId nodeId, MTGMask mask) const;

/// Search face loop for mask
GEOMDLLIMPEXP MTGNodeId FindMaskAroundFace (MTGNodeId nodeId, MTGMask mask) const;
/// Search vertex loop for mask
GEOMDLLIMPEXP MTGNodeId FindMaskAroundVertex (MTGNodeId nodeId, MTGMask mask) const;
/// Search vertex loop for mask
GEOMDLLIMPEXP MTGNodeId FindMaskAroundVertexPred (MTGNodeId nodeId, MTGMask mask) const;

/// Search face loop for mmissing mask
GEOMDLLIMPEXP MTGNodeId FindUnmaskedAroundFace (MTGNodeId nodeId, MTGMask mask) const;
/// Search vertex loop for missing mask
GEOMDLLIMPEXP MTGNodeId FindUnmaskedAroundVertex (MTGNodeId nodeId, MTGMask mask) const;

// Test if nodeA, nodeB are in the same vertex loop.
GEOMDLLIMPEXP bool AreNodesInSameVertexLoop (MTGNodeId nodeA, MTGNodeId nodeB) const;
// Test if nodeA, nodeB are in the same vertex loop.
GEOMDLLIMPEXP bool AreNodesInSameFaceLoop (MTGNodeId nodeA, MTGNodeId nodeB) const;


/// Count nodes with mask around a vertex
GEOMDLLIMPEXP size_t CountMaskAroundVertex (MTGNodeId nodeId, MTGMask mask) const;
/// Count masked nodes in entire graph.
GEOMDLLIMPEXP size_t CountMask   (MTGMask mask) const;

GEOMDLLIMPEXP int DefineLabel (int userTag, MTGLabelMask labelType, int defaultValue);
GEOMDLLIMPEXP bool TrySetLabel (MTGNodeId nodeId, int labelIndex, int value);
GEOMDLLIMPEXP bool TryGetLabel (MTGNodeId nodeId, int labelIndex, int &value) const;
GEOMDLLIMPEXP bool TrySearchLabelTag (int userTag, int &labelIndex) const;

GEOMDLLIMPEXP void SetLabelAroundVertex (MTGNodeId vertexNodeId, int labelOffset, int labelValue);

GEOMDLLIMPEXP void SetLabelAroundFace (MTGNodeId faceNodeId, int labelOffset, int labelValue);
GEOMDLLIMPEXP void SetLabel (int labelOffset, int labelValue);

GEOMDLLIMPEXP int GetLabelMaskAt (int labelIndex) const;
GEOMDLLIMPEXP bool TestLabelMaskPropertyAt (int labelIndex, MTGLabelMask mask) const;
GEOMDLLIMPEXP int GetLabelDefaultAt (int labelIndex) const;

GEOMDLLIMPEXP void ReverseFaceAndVertexLoops ();
/// Navigate to the successor of nodeId around its vertex.
GEOMDLLIMPEXP MTGNodeId VSucc (MTGNodeId nodeId) const;
/// Navigate to the successor of nodeId around its face.
GEOMDLLIMPEXP MTGNodeId FSucc (MTGNodeId nodeId) const;
/// Navigate to the predecessor of nodeId around its vertex.
GEOMDLLIMPEXP MTGNodeId VPred (MTGNodeId nodeId) const;
/// Navigate to the predecessor of nodeId around its face.
GEOMDLLIMPEXP MTGNodeId FPred (MTGNodeId nodeId) const;
/// Navigate to the edge mate of nodeId (at opposite end and opposite side of same edge.)
GEOMDLLIMPEXP MTGNodeId EdgeMate (MTGNodeId nodeId) const;

// Count the number of face loops
GEOMDLLIMPEXP size_t CountFaceLoops ();
// Count the number of vertex loops.
GEOMDLLIMPEXP size_t CountVertexLoops ();

// Place one node from each face loop (of the entire graph) in the bvector.
GEOMDLLIMPEXP void CollectFaceLoops (bvector <MTGNodeId> &faceNodes);
// Place one node from each vertex loop (of the entire graph) in the bvector.
GEOMDLLIMPEXP void CollectVertexLoops (bvector <MTGNodeId> &vertexNodes);

// Collect nodeId's by component.
// @param components array with one vector of nodes per component.
// @param scope one of:
//<pre>
//<ul>
//<li>MTG_ScopeNode collect all nodes.
//<li>MTG_ScopeEdge collect one node per edge
//<li>MTG_ScopeVertex collect one node per vertex
//<li>MTG_ScopeFace collect one node per face
//<li>MTG_ScopeComponetn collect one node per component
//</pre>
GEOMDLLIMPEXP void CollectConnectedComponents (bvector <bvector <MTGNodeId> > &components, MTGMarkScope scope);

// Collect nodeId's by component.
GEOMDLLIMPEXP void CollectConnectedComponents (bvector <bvector <MTGNodeId> > &components);



// Pack the bvector to remove nodes that have the mask.
GEOMDLLIMPEXP void DropMasked (bvector <MTGNodeId> &nodes, MTGMask mask) const;
// Pack the bvector to remove nodes that do not have the mask.
GEOMDLLIMPEXP void DropUnMasked (bvector <MTGNodeId> &nodes, MTGMask mask) const;

GEOMDLLIMPEXP bool VertexTwist (MTGNodeId nodeIdA, MTGNodeId nodeIdB);
// Yank then end of a single edge from a vertex.
// @return if this is a dangling edge, return null.  Otherwise return
//    the input node's original predecessor.  That is, return a representative
//    of the rest of the vertex loop.
GEOMDLLIMPEXP MTGNodeId YankEdgeFromVertex (MTGNodeId nodeId);
// Inverse of Split.
// If nodeId is at a vertex with exactly 2 nodes (i.e. a vertex with one incoming
// and one outgoing edge), delete it and reconnect
//  its neighborhood so a single edge is left.
GEOMDLLIMPEXP bool HealEdge (MTGNodeId nodeId);

// Inverse of Split.
// If nodeId is in a face with exactly two nodes (i.e. a sliver between two parallel edges)
// delete the sliver and reconnect adjacent nodes so a single edge is left.
// The two nodes on the sliver are deleted.  The two nodes on the other sides of the parallel edges remain.
GEOMDLLIMPEXP bool ExciseSliverFace (MTGNodeId nodeId);


// Create two nodes joined as a "stick" edge.
// This is a one face loop with two nodes.
// This is two vertices, each with singleton vertex loops.
GEOMDLLIMPEXP bool CreateEdge (MTGNodeId &nodeId0, MTGNodeId &nodeId1);
// Create two nodes joined as a sling loop.
// This is a two face loops, each with one node.
// This is a single vertex loop, with the two new nodes.
GEOMDLLIMPEXP bool CreateSling (MTGNodeId &nodeId0, MTGNodeId &nodeId1);

// MTGMask that is grabbed and cleared on creation.
// The ScopedMask object remembers its graph.
struct ScopedMask
    {
    private:
    MTGGraph &m_graph;
    MTGMask m_mask;
    public:
    MTGMask Get (){return m_mask;}
    MTGMask GetAt (MTGNodeId node){return m_graph.GetMaskAt (node, m_mask);}
    void SetAt (MTGNodeId node){m_graph.SetMaskAt (node, m_mask);}
    void ClearAt (MTGNodeId node){m_graph.ClearMaskAt (node, m_mask);}
    bool IsSetAt (MTGNodeId node){return 0 != m_graph.GetMaskAt (node, m_mask);}

    void SetAroundFace (MTGNodeId seedNodeId)
        {
        m_graph.SetMaskAroundFace (seedNodeId, m_mask);
        }
    void SetAroundVertex (MTGNodeId seedNodeId)
        {
        m_graph.SetMaskAroundVertex (seedNodeId, m_mask);
        }

    ScopedMask (MTGGraph &graph)
        : m_graph (graph),
        m_mask (graph.GrabMask ())
        {
        BeAssert (m_mask != 0);
        graph.ClearMask (m_mask);
        }
    ~ScopedMask ()
        {
        m_graph.DropMask (m_mask);
        }
    };
// object that walks around a graph.
// This has a nodeId and reference to the graph.
struct Walker {
    private:
    MTGNodeId m_nodeId;
    MTGGraph  *m_graph;
    public:
    Walker (MTGGraph *graph, MTGNodeId nodeId) : m_graph (graph), m_nodeId (nodeId){}
    Walker (MTGGraph &graph, MTGNodeId nodeId) : m_graph (&graph), m_nodeId (nodeId){}
    Walker (MTGGraph &graph) : m_graph (&graph), m_nodeId (NullNodeId){}
    Walker (MTGGraph *graph) : m_graph (graph), m_nodeId (NullNodeId){}

    MTGNodeId NodeId (){return m_nodeId;}
    void MoveTo (MTGNodeId nodeId){m_nodeId = nodeId;}

    bool IsNullNode (){return m_nodeId == NullNodeId;}
    bool IsValidNode (){return m_nodeId != NullNodeId;}

    void MoveToVSucc (){m_nodeId = m_graph->VSucc (m_nodeId);}
    void MoveToFSucc (){m_nodeId = m_graph->FSucc(m_nodeId);}
    void MoveToVPred (){m_nodeId = m_graph->VPred (m_nodeId);}
    void MoveToFPred (){m_nodeId = m_graph->FPred (m_nodeId);}
    void MoveToEdgeMate (){m_nodeId = m_graph->EdgeMate (m_nodeId);}

    Walker VSucc ()   {return Walker (m_graph, m_graph->VSucc (m_nodeId));}
    Walker FSucc ()   {return Walker (m_graph, m_graph->FSucc(m_nodeId));}
    Walker VPred ()   {return Walker (m_graph, m_graph->VPred (m_nodeId));}
    Walker FPred ()   {return Walker (m_graph, m_graph->FPred (m_nodeId));}
    Walker EdgeMate (){return Walker (m_graph, m_graph->EdgeMate (m_nodeId));}



    // strong equality test including checking the graph pointer.
    bool operator == (Walker &other){return m_nodeId == other.m_nodeId && m_graph == other.m_graph;}
    bool operator != (Walker &other){return m_nodeId != other.m_nodeId && m_graph != other.m_graph;}
    // equality test for the node, assuming same graph.
    bool operator == (MTGNodeId &other){return m_nodeId == other;}
    bool operator != (MTGNodeId &other){return m_nodeId != other;}
};
};


typedef MTGGraph * MTGGraphP;
typedef MTGGraph const * MTGGraphCP;

/*--------------------------------------------------------------------------+
| An MTGFacets structure is the combination of:                     |
|<UL>                                                                       |
|<LI>Topology data in an MTGGraph                                           |
|<LI>vertex coordinate data in a VArrayDPoint3d                             |
|<LI>optional normal data in a VArrayDPoint3d                               |
|</UL>                                                                      |
|                                                                           |
| The enumerated value normalStructure indicates how vertex and normal      |
| coordinates are represented:                                              |
|<UL>                                                                       |
|<LI>MTG_Facets_VertexOnly -- No normals. Each VU has a vertex index.       |
|<LI>MTG_Facets_NormalPerVertex -- Each VU has a single index which is both |
|       its vertex and normal index.   When several faces meet with         |
|       multiple normals at a common vertex the vertex data must be         |
|       duplicated for each distinct normal.                                |
|<LI>MTG_Facets_SeparateNormals -- Each VU has separate indices for vertex  |
|       and normal data.   When several faces meet with multiple normals    |
|       at a common vertex the vertex data appears only once in the vertex  |
|       array.                                                              |
|</UL>                                                                      |
+--------------------------------------------------------------------------*/
#ifdef __cplusplus
struct _MTGFacets
    {
public:
void SetNormalMode (MTGFacets_NormalMode normalMode, int numVertex = 0, int numNormal = 0);
public:
/*** Controls data storage at vertices.*/
MTGFacets_NormalMode       normalMode;
/*** Offset to normal indices in graph nodes.*/
int                         normalLabelOffset;
/*** Offset to vertex indices in graph nodes.*/
int                         vertexLabelOffset;
/*** The connectivity structure.*/
MTGGraph            graphHdr;
/*** Vertex coordinates.*/
bvector<DPoint3d> vertexArrayHdr;
/*** Node or vertex normals.*/
bvector<DPoint3d> normalArrayHdr;
/*** Parametric coordinates in parent surface.*/
bvector<DPoint3d> param1ArrayHdr;
/*** Additional data at nodes.*/
bvector<DPoint3d> param2ArrayHdr;
/*** General error indicator for the facets.*/
int                         status;

public:
GEOMDLLIMPEXP _MTGFacets (MTGFacets_NormalMode);
GEOMDLLIMPEXP _MTGFacets ();
GEOMDLLIMPEXP ~_MTGFacets ();
//! Property access with names similar to PolyfaceHeader:
bvector<DPoint3d> &Point (){return vertexArrayHdr;}
bvector<DPoint3d> &Normal (){return normalArrayHdr;}
bvector<DPoint3d> &Param1 (){return param1ArrayHdr;}
bvector<DPoint3d> &Param2 (){return param2ArrayHdr;}

size_t AddPoint (DPoint3dCR xyz){size_t i = vertexArrayHdr.size (); vertexArrayHdr.push_back (xyz); return i;}
size_t AddNormal (DPoint3dCR xyz){size_t i = normalArrayHdr.size (); normalArrayHdr.push_back (xyz); return i;}
size_t AddParam1 (DPoint3dCR xyz){size_t i = param1ArrayHdr.size (); param1ArrayHdr.push_back (xyz); return i;}
size_t AddParam2 (DPoint3dCR xyz){size_t i = param2ArrayHdr.size (); param2ArrayHdr.push_back (xyz); return i;}

GEOMDLLIMPEXP bool SetPointIndex (MTGNodeId nodeId, size_t index);

// Get pointer to topology
MTGGraphCP GEOMDLLIMPEXP GetGraphCP () const;
MTGGraphP GEOMDLLIMPEXP GetGraphP ();
// Sum the volumes of tetrahedra from an origin to (implicit triangulation of) each face listed in the node array.
// The node array is expected to be a complete connected componet, e.g. as returned from CollectConnectedComponnent
// with scoep MTG_ScopeFace.
double GEOMDLLIMPEXP SumTetrahedralVolumeFromNodesPerFace (DPoint3dCR origin, bvector<MTGNodeId> const &nodePerFace) const;

// Return a (vector) which is zero if the nodes are a closed volume.
DVec3d GEOMDLLIMPEXP SumTetrahedralVolumeChecksumFromNodesPerFace (DPoint3dCR origin, bvector<MTGNodeId> const &nodePerFace) const;

// Return a (vector) which is zero if the nodes are a closed volume.
GEOMDLLIMPEXP DRange3d RangeFromNodesPerFace (bvector<MTGNodeId> const &nodePerFace) const;
// Get all vertex coordinates around a face.
// optionally add wraparound.
GEOMDLLIMPEXP bool NodeToVertexCoordinatesAroundFace (MTGNodeId node, bvector<DPoint3d> &xyz, int numWrap = 0) const;

GEOMDLLIMPEXP ValidatedDPlane3d NodeToFacePlaneNormal (MTGNodeId nodeId, bvector<DPoint3d> &xyz) const;
//! Compute face plane normals.
//! Index them through the normalLabelOffset.
GEOMDLLIMPEXP bool BuildFacePlaneNormals ();
// get the vertex coordinates at a node.
GEOMDLLIMPEXP bool  NodeToVertexCoordinates (MTGNodeId node, DPoint3dR xyz) const;
// get the normal at a node.
GEOMDLLIMPEXP bool  NodeToNormalCoordinates (MTGNodeId node, DVec3dR xyz) const;

// get the vertex index at a node
GEOMDLLIMPEXP bool NodeToVertexIndex (MTGNodeId node, ptrdiff_t &index) const;

DPoint3d GetXYZ (MTGNodeId node) const
    {
    DPoint3d xyz;
    return NodeToVertexCoordinates (node, xyz) ? xyz : DPoint3d::From (0,0,0);
    }


// Send selected faces to a builder.
bool GEOMDLLIMPEXP AddFacesToPolyface
(
IPolyfaceConstructionR builder,
bvector<MTGNodeId> const &nodePerFace,
bool reverseFaceOrder
);
    };
#endif
typedef struct _MTGFacets MTGFacets;
typedef struct _MTGFacets *             MTGFacetsP;
typedef struct _MTGFacets const *       MTGFacetsCP;

/*--------------------------------------------------------------------------+
| An MTGTriangulation structure holds status data used to construct         |
| triangulations as MTGFacet sets.                                          |
+--------------------------------------------------------------------------*/
struct _MTGTriangulation;
#ifdef __cplusplus
struct _MTGTriangulation
    {
    MTGTRIANGULATION_FIELDS(EmbeddedIntArray)
    };
#endif

typedef struct _MTGTriangulation MTGTriangulation;
typedef struct _MTGTriangulation * MTGTriangulationP;
typedef struct _MTGTriangulation const * MTGTriangulationCP;


/*--------------------------------------------------------------------------+
| An MTGFacetSearchContext holds data to aid repeated search in a facet set.
+--------------------------------------------------------------------------*/
struct _MTGFacetsSearchContext;
#ifdef __cplusplus
struct _MTGFacetsSearchContext
    {
    MTGFACETSSEARCHCONTEXT_FIELDS
    };
#endif
typedef struct _MTGFacetsSearchContext MTGFacetsSearchContext;

typedef void (*MTGNodeFunc)(MTGGraph *, MTGNodeId, void *);

/* Define opaque types FIRST so that msgeom can proceed through .fdf files
   without seeing the rest of mtg.h (which needs pieces of msgeom.h)
*/
typedef struct _MTG_VisGraph MTG_VisGraph;


typedef struct _MTG_NodeArrayHeader MTG_NodeArrayHeader;
typedef int (*MTG_SortFunction)(const void *, const void *);

typedef struct  denseVoxelIndex DenseVoxelIndex;
typedef struct  rollingBallContext RollingBallContext;


/*--------------------------------------------------------------+
| Standard label tags.                                          |
+--------------------------------------------------------------*/
#define MTG_BREP_EDGE_TAG           (-10001)
#define MTG_NODE_PARTNER_TAG        (-10002)
#define MTG_NODE_COLOR_TAG          (-10003)
#define MTG_LABEL_TAG_POLYFACE_READINDEX    (-10004)

#ifndef HPOINT_MASK_GROUP_BREAK
#define HPOINT_MASK_GROUP_BREAK         0x00020000  /* End of group of loops (major breaks) in loop containment */
#endif

#define MTG_DEFAULT_EDGE_PROPERTY_MASK \
            MTG_EXTERIOR_MASK       \
        |   MTG_BOUNDARY_MASK       \
        |   MTG_CONSTU_MASK         \
        |   MTG_CONSTV_MASK         \
        |   MTG_USEAM_MASK          \
        |   MTG_VSEAM_MASK          \
        |   MTG_PRIMARY_EDGE_MASK   \
        |   MTG_DIRECTED_EDGE_MASK

#define MTG_DEFAULT_FACE_PROPERTY_MASK \
            MTG_NULL_MASK

#define MTG_DEFAULT_VERTEX_PROPERTY_MASK \
            MTG_BOUNDARY_VERTEX_MASK       \
        |    MTG_PRIMARY_VERTEX_MASK


#define MTGARRAY_SET_LOOP(index,pGraph) \
        {MTGNodeId index = (MTGNodeId)((pGraph)->GetNodeIdCount ()); \
        while (--index > -1)                \
            {if ((pGraph)->IsValidNodeId (index)){

#define MTGARRAY_END_SET_LOOP(index,pGraph) }}}

#define MTGARRAY_SET_LOOP_TESTED(index,pGraph,predicate) \
        {MTGNodeId index = (MTGNodeId)((pGraph)->GetNodeIdCount ()); \
        while (--index > -1)                \
            {if ((pGraph)->IsValidNodeId (index) && predicate){

#define MTGARRAY_END_SET_LOOP_TESTED(index,pGraph) }}}


#define MTGARRAY_VERTEX_LOOP(index,pGraph,start) \
        {MTGNodeId index = start; if (index != MTG_NULL_NODEID) { do {

#define MTGARRAY_END_VERTEX_LOOP(index,pGraph,start) \
        index = (pGraph)->VSucc (index);  \
        } while (index != start);}}

#define MTGARRAY_VERTEX_PREDLOOP(index,pGraph,start) \
        {MTGNodeId index = start; if (index != MTG_NULL_NODEID) { do {

#define MTGARRAY_END_VERTEX_PREDLOOP(index,pGraph,start) \
        index = (pGraph)->VPred (index);  \
        } while (index != start);}}

#define MTGARRAY_FACE_LOOP(index,pGraph,start) \
        {MTGNodeId index = start; if (index != MTG_NULL_NODEID) { do {

#define MTGARRAY_END_FACE_LOOP(index,pGraph,start) \
        index = (pGraph)->FSucc (index); \
        } while (index != start);}}

typedef struct _MTG_RectangularGridMasks
    {
    MTGMask         constUMask;     /* => Label for ALL constant u edges. */
    MTGMask         constVMask;     /* => Label for ALL constant v edges. */
    MTGMask         seamUMask;      /* => If not MTG_NULL_MASK; first and last constant u edges */
                                    /*      are a seam with this label. */
    MTGMask         seamVMask;      /* => If not MTG_NULL_MASK; first and last constant v edges */
                                    /*      are a seam with this label. */
    MTGMask         boundaryInteriorMask;  /* => mask for inside main boundary. */
    MTGMask         boundaryExteriorMask;  /* => mask for outside of main boundary. */
    MTGMask         cornerVertexMask;      /* => mask for primary corner vertices. */
    MTGMask         boundaryVertexMask;    /* => mask for boundary; non-corner vertices */
    MTGMask         interiorVertexMask;     /* => mask for vanilla interior vertices */
    MTGMask         interiorEdgeMask;       /* => mask for all interior edges */
    MTGMask         *pConstUMaskArray;  /* => array of constU masks (may be NULL) (0..nu for non-periodic, 0..nu-1 for periodic) */
                                        /* (Applied to both sides of edges) */
    MTGMask         *pConstVMaskArray;  /* => array of constV masks (may be NULL) (0..nv for non-periodic, 0..nv-1 for periodic) */
                                        /* (Applied to both sides of edges) */

    } MTG_RectangularGridMasks;


struct MTG_MarkSet
{
private:
    bvector <MTGNodeId> nodes;
    MTGMask         mask;   // The MarkSet constructor grabs this from the graph
    MTGMarkScope   scope;
    MTGGraph       *pGraph;

public:
    GEOMDLLIMPEXP  MTG_MarkSet (MTGGraphP _graph, MTGMarkScope _scope);
    GEOMDLLIMPEXP MTG_MarkSet ();   // Construct with no graph.  Must follow with Attach 
    GEOMDLLIMPEXP void Attach (MTGGraphP _graph, MTGMarkScope _scope);
    GEOMDLLIMPEXP ~MTG_MarkSet ();
    GEOMDLLIMPEXP void AddNode (MTGNodeId nodeId);
    // Execute AddNode at each node in _scope.  This method's _scope is independent of the markset scope.
    GEOMDLLIMPEXP void AddNodesInScope (MTGNodeId nodeId, MTGMarkScope _scope);
    GEOMDLLIMPEXP void RemoveNode (MTGNodeId nodeId);
    GEOMDLLIMPEXP bool IsNodeInSet (MTGNodeId nodeId) const;
    GEOMDLLIMPEXP MTGMask GetMask () const;
    GEOMDLLIMPEXP MTGNodeId ChooseAndRemoveNode ();
    GEOMDLLIMPEXP void Clear ();

    GEOMDLLIMPEXP int InitIteratorIndex () const;
    GEOMDLLIMPEXP bool TryGetNextNode (int &iteratorIndex, MTGNodeId &nodeId) const;

private:
    bool TryPopCandidate (MTGNodeId &nodeId);
    bool TryGetCandidateAt (size_t index, MTGNodeId &nodeId) const;
};


/* IO callbacks: each returns # ints/doubles/floats read/written */
typedef int (*MTG_readIntsFn)(int *pBuffer, void *pStream, int num);
typedef int (*MTG_readDoublesFn)(double *pBuffer, void *pStream, int num);
typedef int (*MTG_readFloatsFn)(float *pBuffer, void *pStream, int num);

typedef int (*MTG_writeIntsFn)(void *pStream, const int *pBuffer, int num);
typedef int (*MTG_writeDoublesFn)(void *pStream, const double *pBuffer, int num);
typedef int (*MTG_writeFloatsFn)(void *pStream, const float *pBuffer, int num);

/* Callback for scanning faces in facets */
typedef bool    (*MTGFacets_FaceScanFunction)
    (
    MTGFacets *,                        // Containing facets.
    EmbeddedIntArray *pNodeIdArray,     // Nodes around face
    EmbeddedDPoint3dArray *pXYZArray,   // Coordinates around face
    void *vpUserData                    // Caller context.
    );

/* mtg/facetdisk callback bundle */
typedef struct _MTG_IOFuncs
    {
    MTG_readIntsFn      pReadInts;
    MTG_readDoublesFn   pReadDoubles;
    MTG_writeIntsFn     pWriteInts;
    MTG_writeDoublesFn  pWriteDoubles;
    } MTG_IOFuncs;

/* embeddedarray IO callback bundle */
typedef struct _MTG_IOFuncsExt
    {
    MTG_readIntsFn      pReadInts;
    MTG_readDoublesFn   pReadDoubles;
    MTG_writeIntsFn     pWriteInts;
    MTG_writeDoublesFn  pWriteDoubles;
    MTG_readFloatsFn    pReadFloats;
    MTG_writeFloatsFn   pWriteFloats;
    } MTG_IOFuncsExt;

typedef void (*MTG_floatFaceFunction)(int numVertex, float *pData);
typedef void (*MTG_floatXYZRGBFunction)(float *pXYZ, float *pRGB, int numVertex, void *pUserData);

typedef void (*MTG_indexedFaceFunction)(int *pIndices, int numVertex, int userData);

typedef void (*MTG_startPolygonFunction)        (void);
typedef void (*MTG_addLoopToPolygonFunction)    (EmbeddedDPoint3dArray *);
typedef void (*MTG_finishPolygonFunction)       (void *, void *);

typedef bool    (*MTG_NodeBoolFunc) (const MTGGraph *, MTGNodeId, MTGMask, void *);

/*----------------------------------------------------------------------+
| When a facet set is clipped, these bit settings select which parts    |
| are deleted.                                                          |
+----------------------------------------------------------------------*/

#define  MTGClip_deleteNone  0x00000000
#define  MTGClip_deleteIn    0x00000001
#define  MTGClip_deleteOut   0x00000002

typedef unsigned int MTG_ClipActionMask;

typedef bool (*GPAPairFunc_DSegment4dDSegment4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DSegment4d		*pSegment0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DSegment4d		*pSegment1
	);

typedef bool (*GPAPairFunc_DSegment4dDConic4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DSegment4d		*pSegment0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DConic4d		*pConic1
	);

typedef bool (*GPAPairFunc_DSegment4dBezierDPoint4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DSegment4d		*pSegment0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DPoint4d		*pPoleArray1,
	int			order
	);

typedef bool (*GPAPairFunc_DConic4dDConic4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DConic4d		*pConic0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DConic4d		*pConic1
	);

typedef bool (*GPAPairFunc_DConic4dBezierDPoint4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DConic4d		*pConic0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DPoint4d		*pPoleArray1,
	int			order
	);

typedef bool (*GPAPairFunc_BezierDPoint4dBezierDPoint4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource0,
	int			index0,
	DPoint4d		*pPoleArray0,
	int			order0,
	GraphicsPointArrayCP pSource1,
	int			index1,
	DPoint4d		*pPoleArray1,
	int			order
	);

typedef bool (*GPAFunc_DSegment4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource,
	int			index,
	DSegment4d		*pSegment0
	);

typedef bool (*GPAFunc_DConic4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource,
	int			index,
	DConic4d		*pConic
	);

typedef bool (*GPAFunc_BezierDPoint4d)
	(
	void			*pContext,
	GraphicsPointArrayCP pSource,
	int			index,
	DPoint4d		*pPoleArray,
	int			order
	);

typedef void (*GPAFunc_TangentialIntegrand)
	(
	void			*pContext,      /* Caller Context */
	double			*pIntegrand,    /* Array of integrands computed by the callback */
	int			numIntegrand,   /* Number of integrands expected */
	DPoint3d		*pPoint,	/* Point on curve (homogeneous) */
	DPoint3d		*pTangent       /* Tangent vector (homogeneous) */
	);


typedef bool (*GPAPairFunc_IndexIndex)
    (
    void    *pContext,
    GraphicsPointArrayCP pSource0,
    int     index0,
    GraphicsPointArrayCP pSource1,
    int     index1
    );

typedef bool (*GPATripleFunc_IndexIndexIndex)
    (
    void    *pContext,
    GraphicsPointArrayCP pSource0,
    int     index0,
    GraphicsPointArrayCP pSource1,
    int     index1,
    GraphicsPointArrayCP pSource2,
    int     index2
    );

/* mtgreduce mask bundle (e.g., for bridging superfaces) */
typedef struct _MTG_ReduceMasks
    {
    MTGMask boundary;   /* MTG boundary edge */
    MTGMask bridge;     /* bridge edges within a superface */
    MTGMask circle;     /* half-edge mask on circular superfaces */
    MTGMask cylsurf;    /* cylindrical surface superedges */
    MTGMask edge;       /* superface edges */
    MTGMask exterior;   /* unusable or redundant faces in MTG */
    MTGMask punctured;  /* superfaces with hole(s) */
    MTGMask temp1;      /* temporary flag */
    MTGMask temp2;      /* temporary flag */
    MTGMask vertex;     /* superface vertices */
    MTGMask visited;    /* visited nodes (temporary) */
    } MTG_ReduceMasks;



END_BENTLEY_GEOMETRY_NAMESPACE
