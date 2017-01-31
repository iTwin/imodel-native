/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vumem.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
@doctext
@group "VU Memory Management"

<h2>VU Memory Management</h2>

<p>During the life of a VU graph, hundreds of small, uniformly sized nodes are allocated.
Many are deallocated during calculations, and all are deallocated at teardown time.</p>

<p>This section collects low-level functions to manage node-by-node allocation and deallocation in large blocks of nodes.
This leads to far fewer system heap touches than if each node is allocated directly on the heap.</p>

<p>The code is strictly confined to managment of the node blocks---it has no logic directed at internal contents of nodes,
except for referring to the pointer to the next node in the graph (~mvu_nextNodeInGraph), which it uses to chain the pool
of available nodes:</p>

<ul>
<li>In a live node, this pointer forms a cyclic singly-linked list.</li>
<li>In a free node, this pointer forms a linear singly-linked list.</li>
</ul>

<p>These memory management functions should <EM>only</EM> be called from internal code.</p>
*/

#define PAD_INCREMENT 8
#define BLOCK_COUNT 128

static size_t s_numAllocateBlock = 0;
static size_t s_numAllocateNode = 0;
//static size_t s_numInitPool = 0;
static size_t s_numFreePool = 0;
static size_t s_numRecycleNodeLoop = 0;

UsageSums g_vuGrabbedArraysInSetCount = UsageSums  ();
UsageSums g_vuFinalFreeArrayCapacity= UsageSums  ();
UsageSums g_vuAllocatedNodeCount= UsageSums  ();

void PrintUsageSums (char const *name, UsageSums &sums)
    {
    BeConsole::Printf ("%s", name);
    BeConsole::Printf ("(n %8g) (average %9.3g) (stdDev %9.3g) (max %8g) \n", sums.m_sums[0], sums.Mean (), sums.StandardDeviation (), sums.m_max);
    }

static size_t s_printTrigger = 0;
bool VuPrintStats (size_t nvu)
    {
    g_vuAllocatedNodeCount.Accumulate ((double)nvu);
    if (s_printTrigger > 0 && nvu > s_printTrigger)
        {
        PrintUsageSums ("VU Allocated Nodes                         ", g_vuAllocatedNodeCount);
        PrintUsageSums ("VU number of grabbed arrays in vu set cache", g_vuGrabbedArraysInSetCount);
        PrintUsageSums ("VU final array capacity                    ", g_vuFinalFreeArrayCapacity);
        PrintUsageSums ("VU dropped array size                      ", g_vuDroppedArraySize);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
@description Set the function to be called to announce intermediate graph states to debuggers.
@param func IN callback announcement function
@group "VU Debugging"
@see vu_postGraphToTrapFunc
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setGraphTrapFunc
(
VuGraphTrapFunc func
)
    {
    //s_graphTrapFunc = func;
    }

/*---------------------------------------------------------------------------------**//**
@description Trigger an announcement of the graph state to a debugger.
@remarks It is expected that a well-behaved trap function will inspect or copy
   the graph but never modify.
@remarks The multiple identifier arguments allow applications to post many graphs at little cost, while the debugger
   ignores all but a limited number of the posts.
@param pGraph IN graph to post
@param pAppName IN string description of caller
@param id0 IN integer state data
@param id1 IN integer state data
@group "VU Debugging"
@see vu_setGraphTrapFunc
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_postGraphToTrapFunc
(
VuSetP  pGraph,
char const *pAppName,
int     id0,
int     id1
)
    {
#ifdef PrintFromTrapFunc
    static int s_printGraph = 0;
    if (s_printGraph)
        {
        char buffer[1024];
        sprintf (buffer, "%s (%d, %d)\n", pAppName ? pAppName : "postGraphToTrapFunc", id0, id1);
         vu_printFaceLabels (pGraph, buffer);
        }
#endif
    }


/*------------------------------------------------------------------*//**
* @param arrayP <=> add nodes to this array.
* @param listP <=> single thread to this list handle.
* @param extraBytesPerNode => additional (application) data per node.
+----------------------------------------------------------------------*/
static bool        vu_allocateAndThreadNodeBlock
(
bvector<void *> &allBlocks,     // really pointers to BLOCKS of nodes, not singles.
VuP     *listPP,
int     extraBytesPerNode
)
    {
    s_numAllocateBlock++;
    char *firstP, *currP;
    VuP nodeP;
    int i;
    int padSize = ((extraBytesPerNode + PAD_INCREMENT - 1) / PAD_INCREMENT) * PAD_INCREMENT;
    int unitSize = sizeof(VuNode) + padSize;
    int numNode = BLOCK_COUNT;
    int totalBytes = numNode * unitSize;

    firstP = (char *)BSIBaseGeom::Malloc(totalBytes);
    if (!firstP)
        return false;

    memset (firstP, 0, totalBytes);

    nodeP = (VuP)firstP;
    allBlocks.push_back (nodeP);


    for (i = 0; i < numNode; i++)
        {
        currP = firstP + i * unitSize;
        nodeP = (VuP)currP;
        VU_NEXT(nodeP) = *listPP;
        *listPP = nodeP;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Pull one node from the free node pool.
@param graphP IN OUT graph header
@return pointer to allocated node
@group "VU Memory Management"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_allocateVuNode
(
VuSetP graphP
)
    {
    s_numAllocateNode++;
    VuP nodeP = NULL;

    if (!graphP->freeNodeP)
         vu_allocateAndThreadNodeBlock (graphP->mNodePool, &graphP->freeNodeP, graphP->mPrimitiveData.mExtraBytesPerNode);

    if( graphP->freeNodeP )
        {
        nodeP = graphP->freeNodeP;
        graphP->freeNodeP = VU_NEXT(nodeP);
        memset (nodeP, 0, sizeof(VuNode));
        VU_NEXT(nodeP) = NULL;
        }
    return nodeP;
    }





/*---------------------------------------------------------------------------------**//**
@nodoc
@description Move nodes from the cyclic linked list *loopPP to the linear, null-terminated list *listPP.
@param listPP IN handle for null-terminated list (free list)
@param loopPP IN handle for cyclic list (active list)
@group "VU Memory Management"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void vu_recycleNodeLoop (VuP *listPP, VuP *loopPP)
    {
    s_numRecycleNodeLoop++;
     VuP headP,nextP, tailP;
     tailP = *loopPP;
     *loopPP = 0;

     if(tailP)
        {
        headP = VU_NEXT(tailP);
        VU_NEXT(tailP) = 0;     /* so the list is null terminated */
        while(headP)
            {
            nextP = VU_NEXT(headP);
            VU_NEXT(headP) = *listPP;
            *listPP = headP;
            headP = nextP;
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
@description Reset a graph to empty state.
@remarks Memory allocated to the graph is kept so that subsequent graph construction can be fast.
@remarks Fixed masks are left unchanged.
@remarks The free mask pool is left unchanged --- the caller is expected to have returned them.
@remarks All nodes are shifted to the free pool.
@param graphP IN OUT graph header
@group "VU Graph Header"
@see vu_emptyGraph
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void          vu_reinitializeVuSet
(
VuSetP graphP
)
    {
    if (graphP->mNumberOfArraysGrabbed || graphP->mFreeMasks.size () != VU_MAX_FREE_MASK   )
        {
        vu_panicPair ( 0,0);
        }

    graphP->mPrimitiveData.mGraph = 0;
    vu_recycleNodeLoop (&graphP->freeNodeP, &graphP->lastNodeP);
    graphP->nvu = 0;
    graphP->mPrimitiveData.periods.Zero ();
    }



_VuSet::_VuSet (int extraBytesPerNode)
  : mPrimitiveData(extraBytesPerNode)
    {


    /* Initialize the free (grabbable) masks.  They are managed as an array,
    so mark the array empty and reinsert each free mask individually */
    mFreeMasks.clear ();
    DropMask (VU_FREEMASK0);
    DropMask (VU_FREEMASK1);
    DropMask (VU_FREEMASK2);
    DropMask (VU_FREEMASK3);
    DropMask (VU_FREEMASK4);
    DropMask (VU_FREEMASK5);
    DropMask (VU_FREEMASK6);
    DropMask (VU_FREEMASK7);

    DropMask (VU_FREEMASK8);
    DropMask (VU_FREEMASK9);
    DropMask (VU_FREEMASKA);
    DropMask (VU_FREEMASKB);
    DropMask (VU_FREEMASKC);
    DropMask (VU_FREEMASKD);
    DropMask (VU_FREEMASKE);
    DropMask (VU_FREEMASKF);

    lastNodeP = freeNodeP = VU_NULL;
    nvu = 0;

    mNumberOfArraysGrabbed = 0;
    }

_VuSet::~_VuSet ()
    {
    s_numFreePool++;
    for (size_t i = 0; i < mNodePool.size (); i++)
        {
        BSIBaseGeom::Free (mNodePool[i]);
        mNodePool[i] = NULL;
        }

    g_vuGrabbedArraysInSetCount.Accumulate ((double)mFreeArrays.size ());
    for (size_t i = 0; i < mFreeArrays.size(); i++)
        g_vuFinalFreeArrayCapacity.Accumulate ((double)mFreeArrays[i]->capacity ());


    // This assert always returns true -- putting it as assert ensures it (and its possible print) does not appear in PRG build ...
    assert (VuPrintStats (nvu));
    }

    static double s_defaultReltol = 1.0e-9;
    static double s_defaultAbstol = 0.0;

VuPrimitiveData::VuPrimitiveData (int extraBytesPerNode)
    {
    mNewLoopExterior = VU_BOUNDARY_EDGE;
    mNewLoopInterior = VU_BOUNDARY_EDGE;
    mCopyOnSplit =
        VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE | VU_SEAM_EDGE | VU_NULL_EDGE
                | VU_KNOT_EDGE | VU_RULE_EDGE | VU_DISCONTINUITY_EDGE
                | VU_GRID_EDGE
                | VU_SILHOUETTE_EDGE | VU_SECTION_EDGE ;
    mCopyAroundVertex = 0;
    mGraph = 0;
    mExtraBytesPerNode = extraBytesPerNode;

    periods.Zero ();
    abstol = s_defaultAbstol;
    reltol = s_defaultReltol;

    mergeTol = 0.0;

    defaultUserId.asPointer = NULL;
    bUserDataIsVertexProperty = bUserDataIsEdgeProperty = false;

    defaultUserId1 = 0;
    bUserData1IsVertexProperty = bUserData1IsEdgeProperty = false;
    memset (&messagePacket, 0, sizeof (messagePacket));
    edgeInsertionFunction = NULL;
    }


/*---------------------------------------------------------------------------------**//**
@description Allocate (from heap) a new VU graph with no nodes.
@param extraBytesPerVuNode IN unused
@return pointer to newly allocated and initialized graph
@group "VU Graph Header"
@see vu_freeVuSet
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP          vu_newVuSet
(
int extraBytesPerVuNode
)
    {

    VuSetP graphP = new _VuSet (extraBytesPerVuNode);


    return graphP;
    }


/*---------------------------------------------------------------------------------**//**
@description Free a graph header and all associated memory.
@param graphP IN OUT header for graph to be freed.
@return Always returns NULL.
@group "VU Graph Header"
@see vu_newVuSet
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP            vu_freeVuSet
(
VuSetP          graphP
)
    {

    if (!graphP)
        return 0;

    if(    graphP->mNumberOfArraysGrabbed == 0
        || graphP->mFreeMasks.size () != VU_MAX_FREE_MASK   )
        {
        vu_panicPair ( 0,0);
        }

    delete graphP;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                    Earlin.Lutz     03/14
+---------------+---------------+---------------+---------------+---------------+------*/
VuArrays::~VuArrays () { DeleteAndClear ();}
VuArrays::VuArrays (){}
void VuArrays::DeleteAndClear ()
    {
    while (size () > 0)
        {
        delete back ();
        pop_back ();
        }
    }


/*---------------------------------------------------------------------------------**//**
@description Make a "deep" copy of a graph.
@remarks If destination graph is not provided, a new one is created via ~mvu_newVuSet.
    If destination graph is provided, its extra data size must match the source exactly.
@param destGraphP IN OUT preallocated header which receives the graph data, or NULL.
@param sourceGraphP IN source graph.
@group "VU Graph Header"
@return pointer to the copied graph's header.
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuSetP vu_copyVuSet
(
VuSetP          destGraphP,
VuSetP          sourceGraphP
)
    {
    VuP sourceNodeP, destNodeP;
    VuP nextP;
    void *saveP;

    int bytesPerNode;

    if (!sourceGraphP)
        return NULL;

    if (!destGraphP)
        destGraphP = vu_newVuSet (sourceGraphP->mPrimitiveData.mExtraBytesPerNode);

    if (!destGraphP)
        return NULL;
    if (sourceGraphP->mPrimitiveData.mExtraBytesPerNode != destGraphP->mPrimitiveData.mExtraBytesPerNode)
        return NULL;

    bytesPerNode = sizeof (VuNode) + sourceGraphP->mPrimitiveData.mExtraBytesPerNode;
    vu_reinitializeVuSet (destGraphP);

    /* Dest graph needs one node per source graph node.
        Create them in parallel with source.
    */
    sourceNodeP = vu_firstNodeInGraph (sourceGraphP);
    for (;sourceNodeP;
           sourceNodeP = vu_nextNodeInGraph (sourceGraphP, sourceNodeP))
        destNodeP = vu_newVuP (destGraphP);

    /* Walk the two node sets in parallel.
       Save source "user pointer" in dest.
       Make source "user pointer" point to dest node.
    */
    sourceNodeP = vu_firstNodeInGraph (sourceGraphP);
    destNodeP   = vu_firstNodeInGraph (destGraphP);
    for (;sourceNodeP;
         sourceNodeP = vu_nextNodeInGraph (sourceGraphP, sourceNodeP),
         destNodeP   = vu_nextNodeInGraph (destGraphP,   destNodeP))
        {
        vu_copyInternalDataP (destNodeP, sourceNodeP);
        vu_setInternalDataP (sourceNodeP, destNodeP);
        }

    /* Make destination nodes look like source, modulo pointer remap.
       Save source "user pointer" in dest.
       Make source "user pointer" point to dest node.
    */
    sourceNodeP = vu_firstNodeInGraph (sourceGraphP);
    for (;sourceNodeP;
           sourceNodeP = vu_nextNodeInGraph (sourceGraphP, sourceNodeP))
        {
        destNodeP = (VuP)vu_getInternalDataP (sourceNodeP);
        nextP     = destNodeP->next;
        saveP     = vu_getInternalDataP (destNodeP);
        /* Copy all bits.  Immediately fix up the "next" pointer, and
                look at source successors to find corresponding destination successors. */
        memcpy (destNodeP, sourceNodeP, bytesPerNode);
        destNodeP->next = nextP;
        destNodeP->fs = (VuP)vu_getInternalDataP (sourceNodeP->fs);
        destNodeP->vs = (VuP)vu_getInternalDataP (sourceNodeP->vs);
        vu_setInternalDataP (destNodeP, saveP);
        }

    for (VuP sourceNodeP : sourceGraphP->mGraphStack)
        {
        destGraphP->mGraphStack.push_back ((VuP)vu_getInternalDataP (sourceNodeP));
        }

    /* Dest node already has original lptr value from source; that pointer was
        used as pointer, so put it back. */
    for (;sourceNodeP;
          sourceNodeP = vu_nextNodeInGraph (sourceGraphP, sourceNodeP),
          destNodeP   = vu_nextNodeInGraph (destGraphP,   destNodeP)
          )
        {
        vu_copyInternalDataP (sourceNodeP, destNodeP);   // source, dest naming reverses.
        //sourceNodeP->lptr = destNodeP->lptr;
        }

    destGraphP->mPrimitiveData = sourceGraphP->mPrimitiveData;

    destGraphP->mFreeMasks = sourceGraphP->mFreeMasks;

    return destGraphP;
    }


/*---------------------------------------------------------------------------------*//**
@description Move all marked nodes to the free list.
@remarks Note that this is done on a SINGLE NODE basis, hence invalidates graph-level validity if called
    with arbitrary markings.
@param graphP IN graph header
@param mask IN mask for marked nodes.
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void            vu_moveMarkedNodesToFreeList
(
VuSetP graphP,
VuMask mask
)
    {
    VuP firstP = graphP->lastNodeP;
    VuP lastP, nextP;
    //VuP headP = NULL;
    VuP deleteP;

    if( !firstP ) return;
    /* extract the entire nodelist as a null terminated single linked list */
    firstP = VU_NEXT(graphP->lastNodeP);
    VU_NEXT(graphP->lastNodeP) = NULL;
    graphP->lastNodeP = 0;
    /* Move leading non-deleted sections back to the active list, and individual
        deletable nodes to the free list */
    while( firstP )
        {
        if( VU_GETMASK( firstP, mask ) )
            {
            deleteP = firstP;
            firstP = VU_NEXT(deleteP);
            VU_NEXT(deleteP) = graphP->freeNodeP;
            graphP->freeNodeP = deleteP;
            }
        else
            {
            /* Look ahead for the last in a (usually long) chain of
                non-deleted nodes */
            lastP = firstP;
            while( (nextP = VU_NEXT(lastP)) && !VU_GETMASK(nextP, mask) )
                {
                lastP = nextP;
                }
            VU_NEXT(lastP) = firstP; /* firstP..lastP is a cylic linked list .... */
            if(graphP->lastNodeP )
                {
                vu_ntwist(graphP->lastNodeP, lastP); /* which is now merged to the 'new' active list */
                }
            graphP->lastNodeP = lastP;
            firstP = nextP;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
@nodoc "VU Internals"
@description Create a new vertex use in a vu graph.
@remarks This is an ISOLATED, i.e. INVALID node for manifold representations;
    this function is only to be called by privileged functions that promise
    to make TWO VUs and join them properly.
@param graphP IN OUT graph header.
@return pointer to a node with both vertex and face pointers leading back to itself.
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_newVuP
(
VuSetP          graphP
)
    {
    VuP             P = vu_allocateVuNode (graphP);
    VU_NEXT (P) = P;
    VU_FSUCC (P) = P;
    VU_VSUCC (P) = P;
    VU_MASK (P) = 0;
    VU_ID (P) = graphP->nvu++;
    vu_setUserDataP (P, graphP->mPrimitiveData.defaultUserId.asPointer);
    vu_setUserData1 (P, graphP->mPrimitiveData.defaultUserId1);
    if (graphP->lastNodeP)
        {
#ifdef VERIFY_PROPERTIES
        if (VU_ID (VU_NEXT (graphP->lastNodeP)) != graphP->nvu - 2)
            {
            VuP             headP = VU_NEXT (graphP->lastNodeP);
            int             idBad = VU_ID (headP);
            vu_panicPair (P, graphP->lastNodeP);
            }
#endif
        vu_ntwist (P, graphP->lastNodeP);
        }
    else
        {
        graphP->lastNodeP = P;
        }

#ifdef checkLargeGraphs
    if (VU_ID (P) > 10000)
        {
        vu_panic (graphP);
        }
#endif

    return P;
    }


/*---------------------------------------------------------------------------------**//**
@description Set (store in graph) descriptions of periodic properties of x,y,z.
@remarks Period of 0 indicates conventional data in respective direction.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN OUT graph header
@param periodsP IN point whose x,y,z components are x,y,z periods.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setPeriods
(
VuSetP          graphP,
const DPoint3d  *periodsP
)
    {
    if (graphP)
        graphP->mPrimitiveData.periods = *periodsP;
    }

/*---------------------------------------------------------------------------------**//**
@description Return descriptions of periodic properties of x,y,z.
@remarks Period of 0 indicates conventional data in respective direction.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN graph header
@param periodsP OUT point whose x,y,z components are x,y,z periods.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getPeriods
(
VuSetP      graphP,
DPoint3d    *periodsP
)
    {
    if (graphP)
        *periodsP = graphP->mPrimitiveData.periods;
    }


/*---------------------------------------------------------------------------------**//**
@description Set absolute and relative tolerances for the graph.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN OUT graph header
@param abstol IN absolute tolerance
@param reltol IN relative tolerance
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setTol
(
VuSetP          graphP,
double          abstol,
double          reltol
)
    {
    static double s_minreltol = 1.0e-14;
    if (abstol < 0.0)
        abstol = 0.0;
    if (reltol <= s_minreltol)
        reltol = s_minreltol;
    if (graphP)
        {
        graphP->mPrimitiveData.abstol = abstol;
        graphP->mPrimitiveData.reltol = reltol;
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Returns absolute and relative tolerances for the graph.
@remarks This data is available for query; this data is a late (2002) addition to VU, and is not widely used in calculations.
@param graphP IN graph header
@param pAbstol OUT absolute tolerance setting
@param pReltol OUT relative tolerance setting
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getTol
(
VuSetP      graphP,
double      *pAbstol,
double      *pReltol
)
    {
    if (graphP)
        {
        if (pAbstol)
            *pAbstol = graphP->mPrimitiveData.abstol;
        if (pReltol)
            *pReltol = graphP->mPrimitiveData.reltol;
        }
    }

Public GEOMDLLIMPEXP double vu_getMergeTol (VuSetP graphP) { return graphP->mPrimitiveData.mergeTol;}
Public GEOMDLLIMPEXP void  vu_setMergeTol (VuSetP graphP, double tol) { graphP->mPrimitiveData.mergeTol = tol;}


/*---------------------------------------------------------------------------------**//**
@description Store a default value for the userData1 field of each node subsequently created.
@param [inout] graphP graph header
@param [in] value integer value to store.
@parm [in] isVertexProperty true to have the value copied "around vertex" during modifications
@parm [in] isEdgeProperty true to have the value copied "along the edge" during split
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  vu_setDefaultUserData1
(
VuSetP  graphP,
ptrdiff_t value,
bool   isVertexProperty,
bool   isEdgeProperty
)
    {
    graphP->mPrimitiveData.defaultUserId1 = value;
    graphP->mPrimitiveData.bUserData1IsVertexProperty = isVertexProperty;
    graphP->mPrimitiveData.bUserData1IsEdgeProperty   = isEdgeProperty;
    }
/*---------------------------------------------------------------------------------**//**
@description Store a default value for the user data field of each node subsequently created.
@param graphP IN OUT graph header
@param value IN integer value to store.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setDefaultUserDataPAsInt
(
VuSetP  graphP,
int value
)
    {
    graphP->mPrimitiveData.defaultUserId.asInt = value;
    }

/*---------------------------------------------------------------------------------**//**
@description In the graph header, store a flag indicating whether or not the user data pointer
   field of each node is a vertex property and is to be maintained during flip and
   join operations.
@param graphP IN OUT graph header
@param bIsVertexProperty IN true if the user data pointer is to be managed as a vertex
    property during flip and join operations.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setUserDataPIsVertexProperty
(
VuSetP  graphP,
bool    bIsVertexProperty
)
    {
    graphP->mPrimitiveData.bUserDataIsVertexProperty = bIsVertexProperty;
    }


/*---------------------------------------------------------------------------------**//**
@description In the graph header, store a flag indicating whether or not the user data pointer
   field of each node is an edge property and is to be maintained during edge split.
@param graphP IN OUT graph header
@param bIsEdgeProperty IN true if the user data pointer is to be managed as an edge property
        during edge split operation.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setUserDataPIsEdgeProperty
(
VuSetP  graphP,
bool    bIsEdgeProperty
)
    {
    graphP->mPrimitiveData.bUserDataIsEdgeProperty = bIsEdgeProperty;
    }





/*---------------------------------------------------------------------------------**//**
@description Return the number of nodes that have been allocated.  (Actual number of nodes
        may be smaller due to deletions.)
@param graphP IN graph header
@return number of nodes allocated.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     vu_countNodeAllocations
(
VuSetP  graphP
)
    {
    return graphP->nvu;
    }

/*---------------------------------------------------------------------------------**//**
@description Return pointer to first node of a graph traversal.
@remarks The actual order of nodes is arbitrary: there is no inherent relationship to the order of creation.
@param graphP IN graph header
@return pointer to first node of a graph traversal.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_firstNodeInGraph      /* <= first node in entire graph */
(
VuSetP  graphP          /* => graph to select node from */
)
    {
    VuP nodeP = graphP->lastNodeP;
    if (nodeP)
        {
        nodeP = VU_NEXT(nodeP);
        }
    return nodeP;
    }

/*---------------------------------------------------------------------------------**//**
@description Step to the next node in a graph traversal.
@param graphP IN Graph header
@param nodeP IN prior node of traversal
@return successor of nodeP in the global ordering of nodes in the graph.
@group "VU Graph Header"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP     vu_nextNodeInGraph
(
VuSetP  graphP,
VuP     nodeP
)
    {
    if (!nodeP)
        {
        /* nothing to follow */
        }
    else if (nodeP == graphP->lastNodeP)
        {
        nodeP = NULL;   /* Don't walk past the end */
        }
    else
        {
        nodeP = VU_NEXT(nodeP);
        }
    return nodeP;
    }

/*---------------------------------------------------------------------------------**//**
@description Free edges for which either node (start, end) is marked by a given mask.
@remarks The deletion process has two parts:
<ul>
<li>Find each node that has a specified seed mask; mark the node's edge mate and excise both from their vertex loops.</li>
<li>Move all such ndoes to the free list.</li>
</ul>
@remarks Be aware that this function invalidates pointers to the deleted nodes.  This function should not be called from within an algorithm
    that has saved pointers.
@remarks There are two concerns in deletion: integrity and efficiency.  Deleting both VUs on an edge guarantees integrity of the graph
    Deleting many marked edges in one call is more efficient than many calls, each with only a single edge marked, because all edges are
    removed from the (singly linked) node list in a single pass.
@param graphP IN OUT graph to modify
@param seedMask IN mask to identify edges to delete
@group "VU Edges"
@see vu_freeNonMarkedEdges
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_freeMarkedEdges
(
VuSetP graphP,
VuMask seedMask
)
    {
    int seedsFound = 0;

    /* Find the edges and excise them from their vertex loops */
    VU_SET_LOOP ( currP, graphP )
        {
        if( VU_GETMASK( currP, seedMask ) )
            {
            VuP mateP = VU_EDGE_MATE(currP);
            VuP vertPredP = VU_VPRED(currP);
            VU_SETMASK( mateP, seedMask );      /* It may already be set (and excised) but that's ok */
            if( vertPredP != currP )vu_vertexTwist (graphP, currP, vertPredP );
            vertPredP = VU_VPRED(mateP);
            if( vertPredP != currP )vu_vertexTwist (graphP, mateP, vertPredP );
            seedsFound += 1;
            }
        }
    END_VU_SET_LOOP ( currP, graphP )

    if( seedsFound > 0 )
        {
        vu_moveMarkedNodesToFreeList( graphP, seedMask );
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Free edges for which either node (start, end) is <EM>not</EM> marked by a given mask.
@remarks The deletion process has two parts:
<ul>
<li>Find each node that lacks a specified seed mask; mark the node's edge mate and excise both from their vertex loops.</li>
<li>Move all such ndoes to the free list.</li>
</ul>
@remarks Be aware that this function invalidates pointers to the deleted nodes.  This function should not be called from within an algorithm
    that has saved pointers.
@remarks There are two concerns in deletion: integrity and efficiency.  Deleting both VUs on an edge guarantees integrity of the graph
    Deleting many marked edges in one call is more efficient than many calls, each with only a single edge marked, because all edges are
    removed from the (singly linked) node list in a single pass.
@param graphP IN OUT graph to modify
@param seedMask IN mask whose absence identifies edges to delete
@group "VU Edges"
@see vu_freeMarkedEdges
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_freeNonMarkedEdges
(
VuSetP          graphP,
VuMask          seedMask
)
    {
    VuMask      mFree = vu_grabMask (graphP);

    vu_clearMaskInSet (graphP, mFree);

    VU_SET_LOOP ( currP, graphP )
        VU_WRITEMASK (currP, mFree, ! VU_GETMASK (currP, seedMask));
    END_VU_SET_LOOP ( currP, graphP )

    vu_freeMarkedEdges (graphP, mFree);
    vu_returnMask (graphP, mFree);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Return a pointer to the extra data part of a node.
@remarks This pointer leads directly to the data part.  The size of this block is fixed at the time the graph is initialized.
@param nodeP IN node pointer
@return pointer to extra data at tail of node structure
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void *   vu_getExtraDataPointer
(
VuP             nodeP
)
    {
    return VU_DATA_BLOCK (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Query the size (in bytes) of the extra data attached to each node of the graph.
@remarks This size is established at the time the graph is created.
@param graphP IN graph header
@return Byte count for extra data
@group "VU Node Data Fields"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      vu_getExtraDataSize
(
VuSetP          graphP
)
    {
    return graphP->mPrimitiveData.mExtraBytesPerNode;
    }


VuMask _VuSet::GrabMask ()
    {
    VuMask          m = 0;
    if (mFreeMasks.size () > 0)
        {
        m = mFreeMasks.back ();
        mFreeMasks.pop_back ();
        }
    // A mask was requested but none available.
    // Cause 1: Prior callers failed to "return" masks when done.
    // Cause 2: Callers really need more masks than are available.
    //          This really ought to pop up early in development.
    assert (m != 0);
    return m;
    }

void _VuSet::DropMask (VuMask m)
    {
    assert (m != 0);
    mFreeMasks.push_back (m);
    }

int _VuSet::CountNodesInSet () const
    {
    int n = 0;
    VU_SET_LOOP (node, (VuSetP)this)
        {
        n++;
        }
    END_VU_SET_LOOP (node, (VuSetP)this)
    return n;
    }

int _VuSet::CountMaskedNodesInSet(VuMask mask) const
    {
    int n = 0;
    VU_SET_LOOP(node, (VuSetP)this)
        {
        if (node->HasMask (mask))
            n++;
        }
    END_VU_SET_LOOP(node, (VuSetP)this)
        return n;
    }

int _VuSet::CountUnmaskedNodesInSet(VuMask mask) const
    {
    int n = 0;
    VU_SET_LOOP(node, (VuSetP)this)
        {
        if (!node->HasMask(mask))
            n++;
        }
    END_VU_SET_LOOP(node, (VuSetP)this)
        return n;
    }

void _VuSet::SetMaskInSet(VuMask mask)
    {
    VU_SET_LOOP(node, (VuSetP)this)
        {
        node->SetMask(mask);
        }
    END_VU_SET_LOOP(node, (VuSetP)this)
    }


void _VuSet::ToggleMaskInSet(VuMask mask)
    {
    VU_SET_LOOP(node, (VuSetP)this)
        {
        node->ToggleMask (mask);
        }
    END_VU_SET_LOOP(node, (VuSetP)this)
    }

void _VuSet::ClearMaskInSet(VuMask mask)
    {
    VU_SET_LOOP(node, (VuSetP)this)
        {
        node->ClearMask(mask);
        }
    END_VU_SET_LOOP(node, (VuSetP)this)
    }

DRange3d _VuSet::Range () const
    {
    DRange3d range;
    range.Init ();
    VU_SET_LOOP (node, (VuSetP)this)
        {
        range.Extend (node->GetXYZ ());
        }
    END_VU_SET_LOOP (node, (VuSetP)this)
    return range;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
