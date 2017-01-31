/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/mtgintrn.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Mtg/MtgApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MTG_FREE_MASKS      (0xFFFF0000)
#define MTG_FIRST_FREE_MASK (0x00010000)


/*--------------------------------------------------------------------------+
|<UL>                                                                       |
|<LI>MTG_Node structure is used internally by a limited number of files.    |
|<LI>Therefore the internal structure of a node is only known here.         |
|<LI>Deleted nodes are chained through the fSucc pointers.                  |
|<LI>Deleted nodes are identifyable by vSucc = MTG_DELETED_NODEID.          |
|</UL>                                                                      |
+--------------------------------------------------------------------------*/

/*
Manifold vertex loop macro duo:
    * Iterates over adjacent nodes at the vertex on only one side of the MTG
    * __index is undeclared and iterates over nodeIds starting w/ start.
    * bMask marks the boundary of the traversed side of the MTG.
    * Nodes at the vertex are traversed by vPreds until start is reencountered
        or a bMasked node is encountered: in the former case, iteration halts;
        in the latter case, iteration continues by vPreds starting with the
        node before the first bMasked node reachable via vSuccs from start,
        and halts when start is encountered.
    * Traversal is always by vPreds in the maximal nodespace at the vertex.
    * Both macros must be used together, in the order below:
*/
typedef enum _SVLState {PRED, SUCC, STOP} SVLState;

#define MTG_MANIFOLD_VERTEX_LOOP_BEGIN(__index,pGraph,start,bMask)          \
    {                                                                       \
    SVLState    _s = PRED;                                                  \
    MTGNodeId   __index = start;                                            \
    while (_s != STOP)                                                      \
        {
        // VPred loop:
        //  * code for all vertex nodes on this side of the MTG goes here
        //  * break will exit the loop macro

#define MTG_MANIFOLD_VERTEX_LOOP_END(__index,pGraph,start,bMask)            \
        if (_s == PRED)                                                     \
            {                                                               \
            if (!jmdlMTGGraph_getMask (pGraph, __index, bMask))             \
                __index = jmdlMTGGraph_getVPred (pGraph, __index);          \
            else                                                            \
                _s = SUCC;                                                  \
            }                                                               \
        if (_s == SUCC)                                                     \
            {                                                               \
            __index = jmdlMTGGraph_getVSucc (pGraph, start);                \
            while (_s != PRED)                                              \
                {                                                           \
                if (!jmdlMTGGraph_getMask (pGraph, __index, bMask))         \
                    __index = jmdlMTGGraph_getVSucc (pGraph, __index);      \
                else                                                        \
                    {                                                       \
                    __index = jmdlMTGGraph_getVPred (pGraph, __index);      \
                    _s = PRED;                                              \
                    }                                                       \
                }                                                           \
            }                                                               \
        if (__index == start) _s = STOP;                                    \
        }                                                                   \
    }

// mtgstructs.h gives this class friend status to access private graph members.
struct MTGGraphInternalAccess
{
static void DeleteNodes (MTGGraphP graph){graph->DeleteNodes ();}
static void InitNonNodeParts (MTGGraphP graph, bool b){graph->InitNonNodeParts (b);}
static bool BuildFromFSuccAndEdgeMatePermutations
(
MTGGraph       *pGraph,
const int       *pFSuccArray,
const int       *pEdgeMateArray,
int             numNode
);
static void EmptyNodes (MTGGraph *pGraph, bool preserveLabelDefinitions);
// CopySelectedMasksAndLabels and CopyPartial labels have a subtle difference.
// Both make destNodeId match sourceNodeId for the designated bits.
// CopyPartial leaves all others unchanged in the destination.
// CopySelected sets others mask bits to zero, and other labels to their default values.
// SplitEdge uses CopySelected to transfer edge properties to the new nodes.
static bool CopySelectedMasksAndLabels  // <= false if graph pointer, either node id
                                            // or offset is invalid.
(
MTGGraphP  pGraph,             // <=> graph being modified
MTGNodeId  destNodeId,         // <=> data receiver
MTGNodeId  sourceNodeId,       // => data source
MTGMask maskSelect,         // => selects mask fields to be copied.
MTGLabelMask   labelSelect         // => selects labels to copy. Others are default.
);
static bool CopyPartialLabels
(
MTGGraphP      pGraph,
MTGNodeId      destNodeId,
MTGNodeId      sourceNodeId,
MTGMask maskSelect,
unsigned long   labelSelect
);

};

END_BENTLEY_GEOMETRY_NAMESPACE