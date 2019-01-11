/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/MTGGraph.cpp $
|
| Copyright (c) 2001;  Bentley Systems, Inc., 685 Stockton Drive,
|                      Exton PA, 19341-0678, USA.  All Rights Reserved.
|
| This program is confidential, proprietary and unpublished property of Bentley Systems
| Inc. It may NOT be copied in part or in whole on any medium, either electronic or
| printed, without the express written consent of Bentley Systems, Inc.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <Mtg/MTGShortestPaths.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* @dllName mtg */

#define MTG_DELETED_NODEID (-2)     /* also defined in jmdl_mtgmemory.c */
#define MTG_NODE_IS_DELETED(pNode) ((pNode)->vSucc == MTG_DELETED_NODEID)
#define NUM_INTS_IN_BASE_NODE_STRUCT (3)
#define NODE_TO_ARRAY_INDEX(_pGraph,_nodeId) ((_pGraph)->numIntPerNode * (_nodeId))

const int MTGShortestPathContext::s_nullVertexIndex = -1;

static MTG_Node tempNode;

static int  s_mtg_fSucc_field_offset = (int)((int*)&tempNode.fSucc - (int*)&tempNode);
static int  s_mtg_vSucc_field_offset = (int)((int*)&tempNode.vSucc - (int*)&tempNode);

#if defined (wont_compile_on_hp)

static int s_mtg_offset_fvf[3] =
   {
   s_mtg_fSucc_field_offset,
   s_mtg_vSucc_field_offset,
   s_mtg_fSucc_field_offset
   };

static int s_mtg_offset_vfv[3] =
   {
   s_mtg_vSucc_field_offset,
   s_mtg_fSucc_field_offset,
   s_mtg_vSucc_field_offset
   };

static int s_mtg_offset_fvffvf[6] =
   {
   s_mtg_fSucc_field_offset,
   s_mtg_vSucc_field_offset,
   s_mtg_fSucc_field_offset,
   s_mtg_fSucc_field_offset,
   s_mtg_vSucc_field_offset,
   s_mtg_fSucc_field_offset,
   };

#else

static int s_mtg_offset_fvf[3];
static int s_mtg_offset_vfv[3];
static int s_mtg_offset_fvffvf[6];

struct mtg_initialize
    {
    mtg_initialize ()
        {
        s_mtg_offset_fvf[0] = s_mtg_fSucc_field_offset;
        s_mtg_offset_fvf[1] = s_mtg_vSucc_field_offset;
        s_mtg_offset_fvf[2] = s_mtg_fSucc_field_offset;

        s_mtg_offset_vfv[0] = s_mtg_vSucc_field_offset;
        s_mtg_offset_vfv[1] = s_mtg_fSucc_field_offset;
        s_mtg_offset_vfv[2] = s_mtg_vSucc_field_offset;

        s_mtg_offset_fvffvf[0] = s_mtg_fSucc_field_offset;
        s_mtg_offset_fvffvf[1] = s_mtg_vSucc_field_offset;
        s_mtg_offset_fvffvf[2] = s_mtg_fSucc_field_offset;
        s_mtg_offset_fvffvf[3] = s_mtg_fSucc_field_offset;
        s_mtg_offset_fvffvf[4] = s_mtg_vSucc_field_offset;
        s_mtg_offset_fvffvf[5] = s_mtg_fSucc_field_offset;

        };
    };

static mtg_initialize s_dummyStruct;

#endif
void MTGNodeIdPair::Init ()
    {
    nodeId[0] = nodeId[1] = MTG_NULL_NODEID;
    }

/*---------------------------------------------------------------------------------**//**
* @param pGraph => containing graph
* @param ppNode <= array of numNode pointers to MTG_NodeStructures
* @param numNode => number of node pointers requested
* @param startNodeId => index of initial node
* @param pStepOffset => array of offsets from node base to successor node id.
* @return true if entire chain accessed.
* @bsihdr                                       EarlinLutz          10/98
+---------------+---------------+---------------+---------------+---------------+------*/
int     jmdlMTGGraph_getNodePtrChain
(
const   MTGGraph       *pGraph,
        MTG_Node        **ppNode,
        int             numNode,
const   MTGNodeId      nodeId,
const   int             *pOffsetToSuccessorId
)
    {
    int numStep         = numNode - 1;
    MTGNodeId currNodeId = nodeId;
    int numGot = 0;

    if (numNode > 0 && pGraph->IsValidNodeId (currNodeId))
        {
        ppNode[0] = const_cast <MTG_Node *>(pGraph->GetNodeCP (currNodeId));
        numGot = 1;

        for (int i = 0; i < numStep; i++)
            {
            if (pOffsetToSuccessorId[i] == s_mtg_fSucc_field_offset)
                currNodeId = pGraph->FSucc (currNodeId);
            else if (pOffsetToSuccessorId[i] == s_mtg_vSucc_field_offset)
                currNodeId = pGraph->VSucc (currNodeId);
            else
                return numGot;
            if (!pGraph->IsValidNodeId (currNodeId))
                return numGot;
            ppNode[numGot++] = const_cast <MTG_Node *>(pGraph->GetNodeCP (currNodeId));
            }
        }
    return numGot;
    }

bool MTGGraphInternalAccess::CopyPartialLabels
(
MTGGraphP      pGraph,
MTGNodeId      destNodeId,
MTGNodeId      sourceNodeId,
MTGMask maskSelect,
unsigned long   labelSelect
)
    {
    bool    boolstat = false;

    if (pGraph->IsValidNodeId (sourceNodeId)
        && pGraph->IsValidNodeId (destNodeId))
        {
        int numLabels = pGraph->GetLabelCount ();
        int value;
        for (int i = 0; i < numLabels; i++)
            {
            if (labelSelect & pGraph->GetLabelMaskAt (i))
                {
                if (pGraph->TryGetLabel (sourceNodeId, i, value))
                    pGraph->TrySetLabel (destNodeId, i, value);
                }
            }

        if (maskSelect)
            {
            MTGMask destMask, sourceMask;
            if (   pGraph->TryGetFullMask (destNodeId, destMask)
                && pGraph->TryGetFullMask (sourceNodeId, sourceMask))
                {
                destMask &= ~maskSelect;
                destMask |= (sourceMask & maskSelect);
                pGraph->TrySetFullMask (destNodeId, destMask);
                }
            }
        boolstat = true;
        }
    return boolstat;
    }

bool    MTGGraphInternalAccess::CopySelectedMasksAndLabels  // <= false if graph pointer, either node id
                                            // or offset is invalid.
(
MTGGraphP  pGraph,             // <=> graph being modified
MTGNodeId  destNodeId,         // <=> data receiver
MTGNodeId  sourceNodeId,       // => data source
MTGMask maskSelect,         // => selects mask fields to be copied.
MTGLabelMask   labelSelect         // => selects labels to copy. Others are default.
)
    {
    bool    boolstat = false;

    int sourceValue;
    for (int i = 0; pGraph->TryGetLabel (sourceNodeId, i, sourceValue); i++)
        {
        pGraph->TrySetLabel (destNodeId, i,
            pGraph->TestLabelMaskPropertyAt (i, labelSelect)
            ? sourceValue : pGraph->GetLabelDefaultAt (i));
        }
    MTGMask sourceMask;
    if (pGraph->TryGetFullMask (sourceNodeId, sourceMask))
        pGraph->TrySetFullMask (destNodeId, sourceMask & maskSelect);
    boolstat = true;
    return boolstat;
    }




void MTGGraph::ReleaseNode
(
MTGNodeId       nodeId          // Node that his to be freed.   Assumed to have been
                                // disconnected from graph by caller, so its contents are
                                // available for arbitrary use by the free list manager.
)
    {
    MTG_Node *pNode = GetNodeP (nodeId);
    if (pNode)
        {
        pNode->Initialize ();
        SetDefaultLabels (nodeId);
        pNode->fSucc = firstFreeNodeId;
        firstFreeNodeId = nodeId;
        pNode->vSucc = MTG_DELETED_NODEID;
        numActiveNodes--;
        numFreeNodes++;
        }
    }

MTGNodeId MTGGraph::GetNewOrRecycledNode  // <= Recycled or newly allocated node in pGraph
()
    {
    MTG_Node *pNode = NULL;
    MTGNodeId returnId = MTG_NULL_NODEID;
    static int s_recycleNodes = true;
    if (s_recycleNodes && firstFreeNodeId != MTG_DELETED_NODEID)
        {
        pNode = GetNodeP (firstFreeNodeId);
        returnId = firstFreeNodeId;
        firstFreeNodeId = pNode->fSucc;
        numFreeNodes--;
        numActiveNodes++;
        pNode->Initialize ();
        }
    else
        {
        returnId = NewNodeId ();
        if (returnId < 0)
            {
            returnId = MTG_NULL_NODEID; // MTG_NULL_NODEID and the error return are both -1, but just
                                        // make it explicit here.
            }
        else
            {
            pNode = GetNodeP ( returnId);
            numActiveNodes++;
            }
        }

    return returnId;
    }

/*----------------------------------------------------------------------+
| Creates two new nodes in the graph.                                   |
| This is for internal calling only.  Caller connects and applies labels.|
+----------------------------------------------------------------------*/
bool    MTGGraph::CreateTwoNodes     // <= false iff nodes could not be created.
(
MTGNodeId       *pId0,          // <= start id of new edge
MTGNodeId       *pId1,          // <= end id of new edge
MTG_Node        **pNode0,       // <= node pointer
MTG_Node        **pNode1        // <= end node pointer
)
    {
    int id0     = GetNewOrRecycledNode ();
    int id1     = GetNewOrRecycledNode ();
    bool    boolstat;

    if (id0 == MTG_NULL_NODEID || id1 == MTG_NULL_NODEID)
        {
        ReleaseNode (id0);
        ReleaseNode (id1);
        *pId0 = *pId1 = MTG_NULL_NODEID;
        *pNode0 = *pNode1 = NULL;
        boolstat = false;
        }
    else
        {
        *pId0   = id0;
        *pId1   = id1;
        *pNode0 = GetNodeP (id0);
        *pNode1 = GetNodeP (id1);
        boolstat = true;
        }
    return boolstat;
    }



MTGGraph::MTGGraph ()
    {
    InitNonNodeParts (false);
    DeleteNodes ();
    }

void MTGGraph::InitNonNodeParts (bool preserveLabels)
    {
    m_freeMasks = MTG_FREE_MASKS;
    if (!preserveLabels)
        {
        m_edgePropertyMask = MTG_DEFAULT_EDGE_PROPERTY_MASK;
        m_vertexPropertyMask = MTG_DEFAULT_VERTEX_PROPERTY_MASK;
        m_facePropertyMask = MTG_DEFAULT_FACE_PROPERTY_MASK;
        }
    }
    
void MTGGraph::DeleteNodes ()
    {
    m_nodeData.clear ();
    m_nodeLabels.clear ();
    numActiveNodes = 0;
    numFreeNodes = 0;
    firstFreeNodeId = MTG_DELETED_NODEID;
    numActiveNodes = 0;
    numFreeNodes = 0;
    }

void MTGGraph::ClearNodes (bool preserveDefs)
    {
    m_nodeData.clear ();
    if (!preserveDefs)
        {
        m_nodeLabels.clear ();
        }
    InitNonNodeParts (preserveDefs);
    numActiveNodes = 0;
    numFreeNodes = 0;
    firstFreeNodeId = MTG_DELETED_NODEID;
    numActiveNodes = 0;
    numFreeNodes = 0;
    }

// Create a new, zeroed node with default label values.
void MTGGraph::SetDefaultLabels (MTGNodeId nodeId)
    {
    for (size_t i = 0; i < m_nodeLabels.size (); i++)
        {
        if (nodeId < (int)m_nodeLabels[i].size ())
            m_nodeLabels[i][nodeId] = m_nodeLabels[i].m_defaultValue;
        }
    }


// Create a new, zeroed node with default label values.
MTGNodeId MTGGraph::NewNodeId ()
    {
    MTGNodeId newNodeId = (int)m_nodeData.size ();
    m_nodeData.push_back (MTG_Node ());
    for (size_t i = 0; i < m_nodeLabels.size (); i++)
        {
        m_nodeLabels[i].push_back (m_nodeLabels[i].m_defaultValue);
        }
    return newNodeId;
    }

MTG_Node *MTGGraph::GetNodeP (MTGNodeId nodeId)
    {
    if (nodeId < 0 || (size_t)nodeId >= m_nodeData.size ())
        return NULL;
    return m_nodeData.data () + (size_t)nodeId;
    }

MTGNodeId MTGGraph::VSucc (MTGNodeId nodeId) const
    {
    if (IsValidNodeId (nodeId))
        return m_nodeData[(size_t)nodeId].vSucc;
    return MTG_NULL_NODEID;
    }
MTGNodeId MTGGraph::FSucc (MTGNodeId nodeId) const
    {
    if (IsValidNodeId (nodeId))
        return m_nodeData[(size_t)nodeId].fSucc;
    return MTG_NULL_NODEID;
    }

MTGNodeId MTGGraph::VPred (MTGNodeId nodeId) const
    {
    if (IsValidNodeId (nodeId))
        {
        nodeId = m_nodeData[(size_t)nodeId].fSucc;
        if (IsValidNodeId (nodeId))
            {
            nodeId = m_nodeData[(size_t)nodeId].vSucc;
            if (IsValidNodeId (nodeId))
                return m_nodeData[(size_t)nodeId].fSucc;
            }
        }
    return MTG_NULL_NODEID;
    }

MTGNodeId MTGGraph::FPred (MTGNodeId nodeId) const
    {
    if (IsValidNodeId (nodeId))
        {
        nodeId = m_nodeData[(size_t)nodeId].vSucc;
        if (IsValidNodeId (nodeId))
            {
            nodeId = m_nodeData[(size_t)nodeId].fSucc;
            if (IsValidNodeId (nodeId))
                return m_nodeData[(size_t)nodeId].vSucc;
            }
        }
    return MTG_NULL_NODEID;
    }

MTGNodeId MTGGraph::EdgeMate (MTGNodeId nodeId) const
    {
    if (IsValidNodeId (nodeId))
        {
        nodeId = m_nodeData[(size_t)nodeId].fSucc;
        if (IsValidNodeId (nodeId))
            return m_nodeData[(size_t)nodeId].vSucc;
        }
    return MTG_NULL_NODEID;
    }

void MTGGraph::CollectFaceLoops (bvector <MTGNodeId> &faceNodes)
    {
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        if (!GetMaskAt (currNodeId, visitMask))
            {
            faceNodes.push_back (currNodeId);
            SetMaskAroundFace (currNodeId, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    DropMask (visitMask);
    }

void MTGGraph::CollectVertexLoops (bvector <MTGNodeId> &faceNodes)
    {
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        if (!GetMaskAt (currNodeId, visitMask))
            {
            faceNodes.push_back (currNodeId);
            SetMaskAroundVertex (currNodeId, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    DropMask (visitMask);
    }

static void TestMarkAndPush (MTGGraph &graph, bvector<MTGNodeId> &stack, MTGNodeId nodeId, MTGMask visitMask)
    {
    if (!graph.TestAndSetMaskAt (nodeId, visitMask))
        stack.push_back (nodeId);
    }
// Depth first search through connected component of a graph.
// @param [out] component vector of all nodes in component.
// @param [in,out] stack scratch array for use in search.
// @param [in] seed seed node in component.  Assumed NOT marked with visit mask.
// @param [in] visitMask mask to apply to visited nodes.  Assumed cleared throughout component.
static void ExploreComponent (MTGGraph &graph, bvector <MTGNodeId> &component, bvector<MTGNodeId> &stack, MTGNodeId seed, MTGMask visitMask)
    {
    stack.clear ();
    TestMarkAndPush (graph, stack, seed, visitMask);
    while (stack.size () != 0)
        {
        MTGNodeId node = stack.back ();
        component.push_back (node);
        stack.pop_back ();
        TestMarkAndPush (graph, stack, graph.FSucc (node), visitMask);
        TestMarkAndPush (graph, stack, graph.VSucc (node), visitMask);
        }
    }

void MTGGraph::CollectConnectedComponents (bvector <bvector <MTGNodeId> > &components)
    {
    components.clear ();
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    bvector<MTGNodeId> stack;
    MTGARRAY_SET_LOOP (seed, this)
        {
        if (!GetMaskAt (seed, visitMask))
            {
            components.push_back (bvector<MTGNodeId> ());
            ExploreComponent (*this, components.back (), stack, seed, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (seed, this)
    DropMask (visitMask);
    }

// Depth first search through connected component of a graph.
// @param [out] component vector of all nodes in component.
// @param [in,out] stack scratch array for use in search.
// @param [in] seed seed node in component.  Assumed NOT marked with visit mask.
// @param [in] visitMask mask to apply to visited nodes.  Assumed cleared throughout component.
// @param [in] collectMask mask to apply to nodes whose scope (face, vetrtex etc has been collected.
static void ExploreComponent (MTGGraph &graph, bvector <MTGNodeId> &component, bvector<MTGNodeId> &stack, MTGNodeId seed, MTGMask visitMask, MTGMask collectMask, MTGMarkScope scope)
    {
    BeAssert (visitMask != collectMask);
    stack.clear ();
    TestMarkAndPush (graph, stack, seed, visitMask);
    while (stack.size () != 0)
        {
        MTGNodeId node = stack.back ();
        stack.pop_back ();
        // The "visit" and "collect" markups are separate.
        // The "node" sequence of this search is unaffected by the collection scope.
        // This is the collection side:
        if (!graph.TestAndSetMaskAt (node, collectMask))
            {
            // This node (or its face/vertex etc) has not yet been collected.
            switch (scope)
                {
                case MTG_ScopeFace:
                    {
                    graph.SetMaskAroundFace (node, collectMask);
                    component.push_back (node);
                    break;
                    }
                case MTG_ScopeVertex:
                    {
                    graph.SetMaskAroundVertex (node, collectMask);
                    component.push_back (node);
                    break;
                    }
                case MTG_ScopeEdge:
                    {
                    graph.SetMaskAroundEdge (node, collectMask);
                    component.push_back (node);
                    break;
                    }
                case MTG_ScopeComponent:
                    {
                    if (component.size () == 0)
                        component.push_back (node);
                    break;
                    }
                case MTG_ScopeNode:
                    {
                    graph.SetMaskAt (node, collectMask);
                    component.push_back (node);
                    break;
                    }
                }
            }
        // and this is the node side ..            
        TestMarkAndPush (graph, stack, graph.FSucc (node), visitMask);
        TestMarkAndPush (graph, stack, graph.VSucc (node), visitMask);
        }
    }



void MTGGraph::CollectConnectedComponents (bvector <bvector <MTGNodeId> > &components, MTGMarkScope scope)
    {
    components.clear ();
    if (scope == MTG_ScopeNode)
        return CollectConnectedComponents (components);
    MTGMask visitMask = GrabMask ();
    MTGMask collectMask = GrabMask ();
    ClearMask (visitMask);
    ClearMask (collectMask);
    bvector<MTGNodeId> stack;
    MTGARRAY_SET_LOOP (seed, this)
        {
        if (!GetMaskAt (seed, visitMask))
            {
            components.push_back (bvector<MTGNodeId> ());
            ExploreComponent (*this, components.back (), stack, seed, visitMask, collectMask, scope);
            }
        }
    MTGARRAY_END_SET_LOOP (seed, this)
    DropMask (collectMask);
    DropMask (visitMask);
    }

size_t MTGGraph::CountFaceLoops ()
    {
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    size_t n = 0;
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        if (!GetMaskAt (currNodeId, visitMask))
            {
            n++;
            SetMaskAroundFace (currNodeId, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    DropMask (visitMask);
    return n;
    }

size_t MTGGraph::CountVertexLoops ()
    {
    MTGMask visitMask = GrabMask ();
    ClearMask (visitMask);
    size_t n = 0;
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        if (!GetMaskAt (currNodeId, visitMask))
            {
            n++;
            SetMaskAroundVertex (currNodeId, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    DropMask (visitMask);
    return n;
    }


MTG_Node const *MTGGraph::GetNodeCP (MTGNodeId nodeId) const
    {
    if (nodeId < 0 || (size_t)nodeId >= m_nodeData.size ())
        return NULL;
    return m_nodeData.data () + (size_t)nodeId;
    }
size_t MTGGraph::GetNodeIdCount () const {return m_nodeData.size ();}
size_t MTGGraph::GetActiveNodeCount () const {return numActiveNodes;}
int  MTGGraph::GetLabelCount () const {return (int)m_nodeLabels.size ();}
// tests both range and content !!!
bool MTGGraph::IsValidNodeId (MTGNodeId nodeId) const
    {
    if (nodeId < 0)
        return false;
    size_t i = (size_t)nodeId;
    return i < m_nodeData.size () && m_nodeData[i].vSucc != MTG_DELETED_NODEID;
    }

MTGNodeId MTGGraph::AnyValidNodeId () const
    {
    for (size_t i = 0; i < m_nodeData.size (); i++)
        if (m_nodeData[i].vSucc != MTG_DELETED_NODEID)
            return (int)i;
    return MTG_NULL_NODEID;
    }


size_t MTGGraph::WriteToBinaryStream(void*& serialized)
    {
    size_t count = m_nodeData.size()*sizeof(MTG_Node) + m_nodeLabels.size()*(sizeof(LabelSet)+sizeof(int)) + 10 * sizeof(int);
    for (int i = 0; i < (int)m_nodeLabels.size(); i++) count += m_nodeLabels[i].size()*sizeof(int);
    serialized = malloc(count);
    size_t offset = sizeof(int);
    ((int*)serialized)[0] = (int)m_nodeData.size()*sizeof(MTG_Node);

    if (m_nodeData.size() > 0)
        memcpy((char*)serialized+offset, m_nodeData.data(), m_nodeData.size()*sizeof(MTG_Node));

    offset += m_nodeData.size()*sizeof(MTG_Node);
    (*(int*)((char*)serialized + offset)) = (int)m_nodeLabels.size()*(2 * sizeof(int) + sizeof(MTGLabelMask));
    offset += sizeof(int);
    for (int i = 0; i < (int)m_nodeLabels.size(); i++)
        {
        memcpy((char*)serialized + offset+3*sizeof(int)*i, &(m_nodeLabels[i].m_defaultValue), sizeof(int));
        memcpy((char*)serialized + offset + 3 * sizeof(int) * i + sizeof(int), &(m_nodeLabels[i].m_tag), sizeof(int));
        memcpy((char*)serialized + offset + 3 * sizeof(int)* i + 2 * sizeof(int), &(m_nodeLabels[i].m_labelMask), sizeof(MTGLabelMask));
        }
    offset += m_nodeLabels.size()*(2 * sizeof(int) + sizeof(MTGMask));
    for (int i = 0; i < (int)m_nodeLabels.size(); i++)
        {
        (*(int*)((char*)serialized + offset)) = (int)m_nodeLabels[i].size()*sizeof(int);
        offset += sizeof(int);
        memcpy((char*)serialized + offset, m_nodeLabels[i].data(), m_nodeLabels[i].size()*sizeof(int));
        offset += m_nodeLabels[i].size()*sizeof(int);
        }
    memcpy((char*)serialized + offset, &firstFreeNodeId, sizeof(MTGNodeId));
    offset += sizeof(MTGNodeId);
    memcpy((char*)serialized + offset, &numActiveNodes, sizeof(int));
    offset += sizeof(int);
    memcpy((char*)serialized + offset, &numFreeNodes, sizeof(int));
    offset += sizeof(int);
    MTGMask masks[4] = { m_freeMasks, m_edgePropertyMask, m_vertexPropertyMask, m_facePropertyMask };
    memcpy((char*)serialized + offset, masks, 4*sizeof(MTGMask));
    int length = ((int*)serialized)[0];
    assert(length == (int)m_nodeData.size()*sizeof(MTG_Node));
    return count;
    }


void MTGGraph::LoadFromBinaryStream(void* serialized, size_t ct)
    {
    int length = ((int*)serialized)[0];
    assert(length > 0);
    size_t offset = sizeof(int);

    m_nodeData.resize(length / sizeof(MTG_Node));
    memcpy(m_nodeData.data(), (char*)serialized + offset, length);
    offset += length;
    length = (*(int*)((char*)serialized + offset));
    offset += sizeof(int);
    for (int i = 0; i < length / (int)(2 * sizeof(int) + sizeof(MTGLabelMask)); i++)
        {
        m_nodeLabels.push_back(LabelSet(*(int*)( (char*)serialized + offset + 3 * sizeof(int)* i), 
                                *(int*)( (char*)serialized + offset + 3 * sizeof(int)* i + sizeof(int) ),
                               *(MTGLabelMask*)( (char*)serialized + offset + 3 * sizeof(int)*i +2 * sizeof(int) ) ));
        }
    offset += length;
    for (int i = 0; i < (int)m_nodeLabels.size(); i++)
        {
        length = (*(int*)((char*)serialized + offset));
        offset += sizeof(int);
        m_nodeLabels[i].resize(length / sizeof(int));
        memcpy(m_nodeLabels[i].data(), (char*)serialized + offset, length);
        offset += length;
        }
    firstFreeNodeId = (*(MTGNodeId*)((char*)serialized + offset));
    offset += sizeof(MTGNodeId);
    numActiveNodes = (*(int*)((char*)serialized + offset));
    offset += sizeof(int);
    numFreeNodes = (*(int*)((char*)serialized + offset));
    offset += sizeof(int);

    m_freeMasks = (*(MTGMask*)((char*)serialized + offset));
    offset += sizeof(MTGMask);
    m_edgePropertyMask = (*(MTGMask*)((char*)serialized + offset));
    offset += sizeof(MTGMask);
    m_vertexPropertyMask = (*(MTGMask*)((char*)serialized + offset));
    offset += sizeof(MTGMask);
    m_facePropertyMask = (*(MTGMask*)((char*)serialized + offset));
    offset += sizeof(MTGMask);
    }


void MTGGraph::DropMask (MTGMask mask)
    {
    mask &= MTG_FREE_MASKS;     // Make sure we don't recycle a permanent mask!!
    m_freeMasks |= mask;
    m_edgePropertyMask    &= ~mask;
    m_facePropertyMask    &= ~mask;
    m_vertexPropertyMask  &= ~mask;
    }

MTGMask MTGGraph::GrabMask ()
    {
    MTGMask     mask = MTG_FIRST_FREE_MASK;

    while (! (mask & m_freeMasks) && (mask | MTG_FREE_MASKS))
        mask = mask << 1;

    if (! (mask & MTG_FREE_MASKS))      // ERROR : no free masks available
        {
        return  0;
        }

    m_freeMasks &= ~mask;
    return  mask;
    }

void MTGGraph::SetMaskAt (MTGNodeId nodeId, MTGMask mask)
    {
    if (IsValidNodeId (nodeId))
        m_nodeData[nodeId].mask |= mask;
    }

MTGMask MTGGraph::TestAndSetMaskAt (MTGNodeId nodeId, MTGMask mask)
    {
    if (IsValidNodeId (nodeId))
        {
        MTGMask oldMask = m_nodeData[nodeId].mask & mask;
        if (!oldMask)
            m_nodeData[nodeId].mask |= mask;
        return oldMask;
        }
    return 0;
    }

MTGMask MTGGraph::GetMaskAt (MTGNodeId nodeId, MTGMask mask) const
    {
    return IsValidNodeId (nodeId)
        ? (m_nodeData[nodeId].mask & mask)
        : MTG_NULL_MASK;
    }

bool MTGGraph::HasMaskAt (MTGNodeId nodeId, MTGMask mask) const
    {
    return IsValidNodeId (nodeId)
        && ((m_nodeData[nodeId].mask & mask) != 0)
        ;
    }


void MTGGraph::ClearMaskAt (MTGNodeId nodeId, MTGMask mask)
    {
    if (IsValidNodeId (nodeId))
        m_nodeData[nodeId].mask &= ~mask;
    }


void MTGGraph::ClearMask (MTGMask mask)
    {
    for (size_t i = 0, n = m_nodeData.size (); i < n; i++)
        {
        m_nodeData[i].mask &= ~mask;
        }
    }

void MTGGraph::SetMask (MTGMask mask)
    {
    for (size_t i = 0, n = m_nodeData.size (); i < n; i++)
        {
        m_nodeData[i].mask |= mask;
        }
    }

void MTGGraph::ReverseMask (MTGMask mask)
    {
    for (size_t i = 0, n = m_nodeData.size (); i < n; i++)
        {
        m_nodeData[i].mask ^=  mask;
        }
    }


void MTGGraph::SetMaskAroundFace (MTGNodeId nodeId, MTGMask mask)
    {
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        SetMaskAt (currNodeId, mask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    }

void MTGGraph::SetMaskAroundVertex (MTGNodeId nodeId, MTGMask mask)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        SetMaskAt (currNodeId, mask);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    }

void MTGGraph::SetMaskAroundEdge (MTGNodeId nodeId, MTGMask mask)
    {
    SetMaskAt (nodeId, mask);
    SetMaskAt (EdgeMate (nodeId), mask);
    }

void MTGGraph::ClearMaskAroundEdge (MTGNodeId nodeId, MTGMask mask)
    {
    ClearMaskAt (nodeId, mask);
    ClearMaskAt (EdgeMate (nodeId), mask);
    }

size_t MTGGraph::CountNodesAroundFace (MTGNodeId nodeId) const
    {
    size_t n = 0;
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        n++;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    return n;
    }

size_t MTGGraph::CountNodesAroundVertex (MTGNodeId nodeId) const
    {
    size_t n = 0;    
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        n++;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    return n;
    }

bool MTGGraph::AreNodesInSameVertexLoop (MTGNodeId nodeA, MTGNodeId nodeB) const
    {
    if (!IsValidNodeId (nodeA))
        return false;
    if (!IsValidNodeId (nodeB))
        return false;
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeA)
        {
        if (currNodeId == nodeB)
            return true;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeA)
    return false;
    }

bool MTGGraph::AreNodesInSameFaceLoop (MTGNodeId nodeA, MTGNodeId nodeB) const
    {
    if (!IsValidNodeId (nodeA))
        return false;
    if (!IsValidNodeId (nodeB))
        return false;
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeA)
        {
        if (currNodeId == nodeB)
            return true;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeA)
    return false;
    }




    /// Search face loop for mask
MTGNodeId MTGGraph::FindMaskAroundFace (MTGNodeId nodeId, MTGMask mask) const
    {
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        if (GetMaskAt (currNodeId, mask))
            return currNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    return MTG_NULL_NODEID;
    }

/// Search vertex loop for mask
MTGNodeId MTGGraph::FindMaskAroundVertex (MTGNodeId nodeId, MTGMask mask) const
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        if (GetMaskAt (currNodeId, mask))
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    return MTG_NULL_NODEID;
    }

/// Search vertex loop for mask
MTGNodeId MTGGraph::FindMaskAroundVertexPred (MTGNodeId nodeId, MTGMask mask) const
    {
    MTGARRAY_VERTEX_PREDLOOP (currNodeId, this, nodeId)
        {
        if (GetMaskAt (currNodeId, mask))
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_PREDLOOP (currNodeId, this, nodeId)
    return MTG_NULL_NODEID;
    }



/// Search face loop for mmissing mask
MTGNodeId MTGGraph::FindUnmaskedAroundFace (MTGNodeId nodeId, MTGMask mask) const
    {
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        if (!GetMaskAt (currNodeId, mask))
            return currNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    return MTG_NULL_NODEID;
    }

/// Search vertex loop for missing mask
MTGNodeId MTGGraph::FindUnmaskedAroundVertex (MTGNodeId nodeId, MTGMask mask) const
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        if (!GetMaskAt (currNodeId, mask))
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    return MTG_NULL_NODEID;
    }


size_t MTGGraph::CountMaskAroundFace   (MTGNodeId nodeId, MTGMask mask) const
    {
    size_t n = 0;
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        if (GetMaskAt (currNodeId, mask))
            n++;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    return n;
    }
size_t MTGGraph::CountMaskAroundVertex (MTGNodeId nodeId, MTGMask mask) const
    {
    size_t n = 0;    
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        if (GetMaskAt (currNodeId, mask))
            n++;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    return n;
    }
size_t MTGGraph::CountMask   (MTGMask mask) const
    {
    size_t n = 0;    
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        if (GetMaskAt (currNodeId, mask))
            n++;
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    return n;
    }

void MTGGraph::DropMasked (bvector<MTGNodeId> &nodes, MTGMask mask) const
    {
    size_t numAccepted = 0;
    for (size_t i = 0, n = nodes.size (); i < n; i++)
        {
        MTGNodeId nodeId = nodes[i];
        if (GetMaskAt (nodeId, mask))
            {
            }
        else
            {
            nodes[numAccepted++] = nodeId;
            }
        }
    nodes.resize (numAccepted);
    }


size_t MTGGraph::DropMaskedEdges (MTGMask mask)
    {
    size_t numDropped = 0;
    MTGARRAY_SET_LOOP (nodeId, this)
        {
        if (GetMaskAt (nodeId, mask))
            {
            DropEdge (nodeId);
            numDropped++;
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, this)
    return numDropped;
    }



void MTGGraph::DropUnMasked (bvector<MTGNodeId> &nodes, MTGMask mask) const
    {
    size_t numAccepted = 0;
    for (size_t i = 0, n = nodes.size (); i < n; i++)
        {
        MTGNodeId nodeId = nodes[i];
        if (GetMaskAt (nodeId, mask))
            {
            nodes[numAccepted++] = nodeId;
            }
        else
            {
            }
        }
    nodes.resize (numAccepted);
    }


void MTGGraph::ClearMaskAroundFace (MTGNodeId nodeId, MTGMask mask)
    {
    MTGARRAY_FACE_LOOP (currNodeId, this, nodeId)
        {
        ClearMaskAt (currNodeId, mask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, nodeId)
    }

void MTGGraph::ClearMaskAroundVertex (MTGNodeId nodeId, MTGMask mask)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, this, nodeId)
        {
        ClearMaskAt (currNodeId, mask);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, nodeId)
    }




int MTGGraph::DefineLabel
(
int             userTag,
MTGLabelMask   labelType,
int             defaultValue
)
    {
    int offset = -1;
    if (m_nodeLabels.size () < MTG_MAX_LABEL_PER_NODE)
        {
        size_t labelIndex = m_nodeLabels.size ();
        offset = (int)labelIndex;
        m_nodeLabels.push_back (LabelSet (defaultValue, userTag, labelType));
        for (size_t i = 0; i < m_nodeData.size (); i++)
            {
            m_nodeLabels[labelIndex].push_back (defaultValue);
            }
        }
    return offset;
    }

bool MTGGraph::TrySetLabel (MTGNodeId nodeId, int labelIndex, int value)
    {
    if (IsValidNodeId (nodeId) && IsValidLabelIndex (labelIndex))
        {
        m_nodeLabels[labelIndex][nodeId] = value;
        return true;
        }
    return false;
    }

bool MTGGraph::TryGetLabel (MTGNodeId nodeId, int labelIndex, int &value) const
    {
    if (IsValidNodeId (nodeId) && IsValidLabelIndex (labelIndex))
        {
        value = m_nodeLabels [labelIndex][nodeId];
        return true;
        }
    return false;
    }


void MTGGraph::SetMaskProperty (MTGMask mask, MTGLabelMask propertyMask, bool value)
    {
    if (value)
        {
        if (propertyMask & MTG_LabelMask_EdgeProperty)
            m_edgePropertyMask |= mask;

        if (propertyMask & MTG_LabelMask_FaceProperty)
            m_facePropertyMask |= mask;

        if (propertyMask & MTG_LabelMask_VertexProperty)
            m_vertexPropertyMask |= mask;
        }
    else
        {
        if (propertyMask & MTG_LabelMask_EdgeProperty)
            m_edgePropertyMask &= ~mask;

        if (propertyMask & MTG_LabelMask_FaceProperty)
            m_facePropertyMask &= ~mask;

        if (propertyMask & MTG_LabelMask_VertexProperty)
            m_vertexPropertyMask &= ~mask;
        }
    }


MTGMask MTGGraph::GetMaskProperty (MTGMask queryMask, MTGLabelMask selector) const
    {
    MTGMask mask = MTG_NULL_MASK;

    if (selector == MTG_LabelMask_EdgeProperty)
            mask = m_edgePropertyMask | queryMask;

    if (selector == MTG_LabelMask_FaceProperty)
            mask = m_facePropertyMask | queryMask;

    if (selector == MTG_LabelMask_VertexProperty)
            mask = m_vertexPropertyMask | queryMask;

    return mask;
    }

bool MTGGraph::TrySetFullMask (MTGNodeId nodeId, MTGMask value)
    {
    if (IsValidNodeId (nodeId))
        {
        m_nodeData[nodeId].mask = value;
        return true;
        }
    return false;
    }

bool MTGGraph::TryGetFullMask (MTGNodeId nodeId, MTGMask &value) const
    {
    if (IsValidNodeId (nodeId))
        {
        value = m_nodeData[nodeId].mask;
        return true;
        }
    return false;
    }

bool MTGGraph::TrySearchLabelTag (int userTag, int &labelIndex) const
    {
    size_t n = m_nodeLabels.size ();
    for (size_t i = 0; i < n; i++)
        {
        if (m_nodeLabels[i].m_tag == userTag)
            {
            labelIndex = (int) i;
            return true;
            }
        }
    labelIndex = -1;
    return false;
    }

bool MTGGraph::TestLabelMaskPropertyAt (int labelIndex, MTGLabelMask mask) const
    {
    return IsValidLabelIndex (labelIndex)
        && (m_nodeLabels[labelIndex].m_labelMask & mask);
    }

int MTGGraph::GetLabelMaskAt (int labelIndex) const
    {
    if (IsValidLabelIndex (labelIndex))
        return m_nodeLabels[labelIndex].m_labelMask;
    return 0;
    }

int MTGGraph::GetLabelDefaultAt (int labelIndex) const
    {
    if (IsValidLabelIndex (labelIndex))
        return m_nodeLabels[labelIndex].m_defaultValue;
    return 0;
    }


bool MTGGraph::IsValidLabelIndex (int index) const
    {
    return index >= 0 && (size_t)index < m_nodeLabels.size ();
    }



void MTGGraph::ReverseFaceAndVertexLoops ()
    {
    bvector<MTGNodeId> predecessor;
    size_t numNode = GetNodeIdCount ();
    predecessor.reserve (numNode);
    // reverse the face loops ...
    for (size_t i = 0; i < numNode; i++)
        predecessor.push_back (MTG_NULL_NODEID);
    for (size_t i = 0; i < numNode; i++)
        {
        if (IsValidNodeId ((int)i))
            predecessor [m_nodeData[i].fSucc] = (int)i;
        }
    for (size_t i = 0; i < numNode; i++)
        {
        if (IsValidNodeId ((int)i))
            m_nodeData[i].fSucc = predecessor [i];
        }


    // reverse the face loops ...
    predecessor.clear ();
    for (size_t i = 0; i < numNode; i++)
        predecessor.push_back (MTG_NULL_NODEID);
    for (size_t i = 0; i < m_nodeData.size (); i++)
        {
        if (IsValidNodeId ((int)i))
            predecessor [m_nodeData[i].vSucc] = (int)i;
        }
    for (size_t i = 0; i < m_nodeData.size (); i++)
        {
        if (IsValidNodeId ((int)i))
            m_nodeData[i].vSucc = predecessor [i];
        }
    }

bool MTGGraph::Join (
MTGNodeId      &idNewA,
MTGNodeId      &idNewB,
MTGNodeId      idA,
MTGNodeId      idB,
MTGMask maskA,
MTGMask maskB
)
    {
    bool    boolstat = CreateEdge (idNewA, idNewB);
    int offset;
    int value;
    if (boolstat)
        {
        VertexTwist (idNewA, idA);
        VertexTwist (idNewB, idB);
        if (maskA)
            SetMaskAt (idNewA, maskA);
        if (maskB)
            SetMaskAt (idNewB, maskB);

        // Copy vertex labels from the originals to the attached strut nodes.
        int numLabels = GetLabelCount ();
        for (offset = 0; offset < numLabels; offset++)
            {
            if (TestLabelMaskPropertyAt (offset, MTG_LabelMask_VertexProperty))
                {
                TryGetLabel (idA,    offset, value);
                TrySetLabel (idNewA, offset, value);
                TryGetLabel (idB,    offset, value);
                TrySetLabel (idNewB, offset, value);
                }
            }
        }
    return boolstat;
    }


bool MTGGraph::SplitEdge (MTGNodeId &newLeftNodeId, MTGNodeId &newRightNodeId, MTGNodeId baseNodeId)
    {
    MTG_Node *pNode0, *pNode1;
    MTGNodeId mateId = EdgeMate (baseNodeId);
    MTGLabelMask maskSelector = (MTGLabelMask)(m_edgePropertyMask | m_facePropertyMask);
    MTGLabelMask labelSelector = (MTGLabelMask)(MTG_LabelMask_EdgeProperty | MTG_LabelMask_FaceProperty);
    bool    boolstat;

    if (MTG_NULL_NODEID == baseNodeId)
        {
        boolstat = CreateSling (newLeftNodeId, newRightNodeId);
        }
    else
        {
        boolstat = CreateTwoNodes (&newLeftNodeId, &newRightNodeId, &pNode0, &pNode1);

        MTG_Node *pBaseNode = GetNodeP (baseNodeId);
        MTG_Node *pMateNode = GetNodeP (mateId);
        if (boolstat && NULL != pBaseNode && NULL != pMateNode)
            {
            pNode0->vSucc = newRightNodeId;
            pNode1->vSucc = newLeftNodeId;
            pNode0->fSucc = pBaseNode->fSucc;
            pNode1->fSucc = pMateNode->fSucc;
            pBaseNode->fSucc = newLeftNodeId;
            pMateNode->fSucc = newRightNodeId;

            MTGGraphInternalAccess::CopySelectedMasksAndLabels  (this, newLeftNodeId, baseNodeId, maskSelector, labelSelector);
            MTGGraphInternalAccess::CopySelectedMasksAndLabels  (this, newRightNodeId, mateId, maskSelector, labelSelector);
            }
        }
    return boolstat;
    }

bool MTGGraph::DropEdge (MTGNodeId nodeId)
    {
    MTG_Node * pNode = GetNodeP ( nodeId);
    MTGNodeId mateId;

    if (pNode && !MTG_NODE_IS_DELETED(pNode))
        {
        mateId = EdgeMate (nodeId);
        YankEdgeFromVertex (nodeId);
        YankEdgeFromVertex (mateId);
        ReleaseNode (nodeId);
        ReleaseNode (mateId);
        return true;
        }
    return false;
    }

bool MTGGraph::HealEdge (MTGNodeId node0Id)
    {

    MTG_Node * pNode[6];
    MTGNodeId nodeId[7];
    bool    boolstat = false;

    if (   6 == jmdlMTGGraph_getNodePtrChain (this, pNode, 6, node0Id, s_mtg_offset_fvffvf))
        {
        nodeId[0] = node0Id;
        nodeId[1] = pNode[0]->fSucc;
        nodeId[2] = pNode[1]->vSucc;
        nodeId[3] = pNode[2]->fSucc;
        nodeId[4] = pNode[3]->fSucc;
        nodeId[5] = pNode[4]->vSucc;
        nodeId[6] = pNode[5]->fSucc;

        if (    nodeId[6] == nodeId[0]
            &&  pNode[0]->vSucc == nodeId[3]
            &&  pNode[3]->vSucc == nodeId[0]
            &&  nodeId[0] != nodeId[1]
            &&  nodeId[3] != nodeId[4]
            )
            {
            pNode[5]->fSucc = nodeId[1];
            pNode[2]->fSucc = nodeId[4];
            ReleaseNode (nodeId[0]);
            ReleaseNode (nodeId[3]);
            boolstat = true;
            }
        }
    return boolstat;
    }

bool MTGGraph::ExciseSliverFace (MTGNodeId node0Id)
    {
    if (!IsValidNodeId (node0Id))
        return false;
    MTGNodeId node1Id = FSucc (node0Id);
    if (node1Id == node0Id)
        return false;
    if (FSucc (node1Id) != node0Id)
        return false;
    MTGNodeId node0PredId = VPred (node0Id);
    MTGNodeId node0SuccId = VSucc (node0Id);
    MTGNodeId node1PredId = VPred (node1Id);
    MTGNodeId node1SuccId = VSucc (node1Id);
    MTG_Node *node0PredP = GetNodeP (node0PredId);
    MTG_Node *node1PredP = GetNodeP (node1PredId);
    node0PredP->vSucc = node0SuccId;
    node1PredP->vSucc = node1SuccId;
    ReleaseNode (node0Id);
    ReleaseNode (node1Id);
    return true;
    }



bool MTGGraph::CreateEdge (MTGNodeId &nodeId0, MTGNodeId &nodeId1)
    {
    MTG_Node *pNode0;
    MTG_Node *pNode1;
    bool    boolstat = CreateTwoNodes(&nodeId0, &nodeId1, &pNode0, &pNode1);
    if (boolstat)
        {
        pNode0->vSucc = pNode1->fSucc = nodeId0;
        pNode1->vSucc = pNode0->fSucc = nodeId1;
        }
    return boolstat;
    }

bool MTGGraph::CreateSling (MTGNodeId &nodeId0, MTGNodeId &nodeId1)
    {
    MTG_Node *pNode0;
    MTG_Node *pNode1;
    bool    boolstat = CreateTwoNodes (&nodeId0, &nodeId1, &pNode0, &pNode1);
    if (boolstat)
        {
        pNode0->fSucc = pNode1->vSucc = nodeId0;
        pNode1->fSucc = pNode0->vSucc = nodeId1;
        }
    return boolstat;
    }

MTGNodeId MTGGraph::YankEdgeFromVertex (MTGNodeId nodeId)
    {
    MTGNodeId vertexPredecessorId = VPred (nodeId);
    MTGNodeId returnId;
    if (vertexPredecessorId != nodeId)
        {
        VertexTwist (nodeId, vertexPredecessorId);
        returnId = vertexPredecessorId;
        }
    else
        {
        returnId = MTG_NULL_NODEID;
        }
    return returnId;
    }


/**

* @param pGraph    <=> containing graph.
* @param nodeX0 => first node of t
* @param nodeY0 => second node of twist operation
* @return true if nodes are valid
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_edgeTwist
(
MTGGraphP               pGraph,
MTGNodeId       nodeX0,
MTGNodeId       nodeY0
)
    {
    MTG_Node *pEdgeX[4];
    MTG_Node *pEdgeY[4];
    MTGNodeId nodeX2, nodeY2;

    // Get pointers to everything around the two edges:
    if  (
           4 == jmdlMTGGraph_getNodePtrChain (pGraph, pEdgeX, 4, nodeX0, s_mtg_offset_fvf)
        && 4 == jmdlMTGGraph_getNodePtrChain (pGraph, pEdgeY, 4, nodeY0, s_mtg_offset_fvf))
        {
        // extract node ids from the pointers (this is tricky --- each node's id
        // resides at particular places in its predecessor on the extracted path)
        nodeX2 = pEdgeX[1]->vSucc;
        nodeY2 = pEdgeY[1]->vSucc;

        // Update 4 vertex successors to achieve the join/split effect on the edge:
        pEdgeY[3]->vSucc = nodeX0;
        pEdgeX[3]->vSucc = nodeY0;

        pEdgeY[1]->vSucc = nodeX2;
        pEdgeX[1]->vSucc = nodeY2;

        return true;
        }
    return false;
    }

bool    MTGGraph::VertexTwist
(
MTGNodeId       node0,
MTGNodeId       node1
)
    {
    MTG_Node *pEdge0[4];
    MTG_Node *pEdge1[4];
    bool    boolstat = true;

    if  (
           4 == jmdlMTGGraph_getNodePtrChain (this, pEdge0, 4, node0, s_mtg_offset_vfv)
        && 4 == jmdlMTGGraph_getNodePtrChain (this, pEdge1, 4, node1, s_mtg_offset_vfv)
        )
        {
        MTGNodeId tempId;
        // Swap VERTEX SUCCESSORS
        tempId = pEdge0[0]->vSucc;
        pEdge0[0]->vSucc = pEdge1[0]->vSucc;
        pEdge1[0]->vSucc = tempId;
        // Swap FACE PREDCESSORS
        tempId = pEdge0[3]->fSucc;
        pEdge0[3]->fSucc = pEdge1[3]->fSucc;
        pEdge1[3]->fSucc = tempId;
        }
    else
        {
        boolstat = false;
        }

    return boolstat;
    }

void MTGGraphInternalAccess::EmptyNodes (MTGGraph *pGraph, bool preserveLabelDefinitions)
    {
    DeleteNodes (pGraph);
    InitNonNodeParts (pGraph, true);
    }
bool MTGGraphInternalAccess::BuildFromFSuccAndEdgeMatePermutations
(
MTGGraph       *pGraph,
const int       *pFSuccArray,
const int       *pEdgeMateArray,
int             numNode
)
    {
    int i;
    MTG_Node * pCurrNode, *pFSuccNode;
    MTGNodeId currId, fSuccId, mateId;
    bool    boolstat = true;
    const int UNINITIALIZED_NODE_ID = -1;

    EmptyNodes (pGraph, true);

    if (numNode <= 0 || numNode & 0x01)
            boolstat = false;

    for (currId = 0; boolstat && currId < numNode; currId++)
        {
        fSuccId = pFSuccArray[currId];
        mateId  = pEdgeMateArray[currId];

        if (   0 <= mateId
            && mateId < numNode
            && 0 <= fSuccId
            && fSuccId < numNode
            && mateId != currId
            && pEdgeMateArray[mateId] == currId
            )
            {
            // This is what we want.
            }
        else
            {
            // Bad input data.  Edge pairings don't correspond.
            boolstat = false;
            }
        }


    // Initialize the nodes.  If things are allocated as expected, we
    // will get back node id's 0..numNode-1 in order.   It's also OK
    // for the allocator to do it in other orders, as long as the numbers
    //  are confined to that range.
    for (i = 0; boolstat && i < numNode; i++)
        {
        currId = pGraph->GetNewOrRecycledNode ();
        if (i >= numNode)
            {
            // Oooops.. allocation did not happen as expected. Can't count on
            // nodes numbered 0..numNode-1.
            boolstat = false;
            }
        else
            {
            // This is what we want.
            // Inititialize the pointers to allow integrity checks.
            pCurrNode = pGraph->GetNodeP (currId);
            pCurrNode->fSucc = pCurrNode->vSucc = UNINITIALIZED_NODE_ID;
            }
        }

    // Rig all the pointers
    for (currId = 0; boolstat && currId < numNode; currId++)
        {
        fSuccId = pFSuccArray[currId];
        mateId  = pEdgeMateArray[currId];
        pCurrNode = pGraph->GetNodeP (currId);
        pFSuccNode = pGraph->GetNodeP (fSuccId);
        if (   pCurrNode->fSucc != UNINITIALIZED_NODE_ID
            || pCurrNode->fSucc != UNINITIALIZED_NODE_ID
           )
            {
            boolstat = false;
            }
        else
            {
            pCurrNode->fSucc = fSuccId;
            pFSuccNode->vSucc = mateId;
            }
        }

    if (!boolstat)
        EmptyNodes (pGraph, true);

    return boolstat;
    }


void MTGGraph::SetLabel
(
int          labelOffset,
int          labelValue
)
    {
    MTGARRAY_SET_LOOP (currNodeId, this)
        {
        TrySetLabel (currNodeId, labelOffset, labelValue);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, this)
    }

void MTGGraph::SetLabelAroundVertex
(
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, this, vertexNodeId)
        {
        TrySetLabel (currNodeId, labelOffset, labelValue);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, this, vertexNodeId)
    }

void MTGGraph::SetLabelAroundFace
(
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
)
    {
    MTGARRAY_FACE_LOOP (currNodeId, this, vertexNodeId)
        {
        TrySetLabel (currNodeId, labelOffset, labelValue);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, this, vertexNodeId)
    }
END_BENTLEY_GEOMETRY_NAMESPACE
