/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/GeomApi.h>
#include "vumem.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct VuPositionDetail;
typedef struct VuPositionDetail & VuPositionDetailR;
typedef struct VuPositionDetail const & VuPositionDetailCR;
typedef struct VuPositionDetail * VuPositionDetailP;
typedef struct VuPositionDetail const * VuPositionDetailCP;
struct VuPositionDetailAnnouncer;


/**
@description Reserved bits in a ~tVuMask.
@group "VU Node Masks"
*/
typedef enum vumaskbits
    {
    VU_NULL_MASK                = 0x00000000,
    /** @description The edge lies on the true boundary of the geometry.
        @remarks This mask is set on <em>both</em> sides of the edge. */
    VU_BOUNDARY_EDGE            = 0x00000001,
    /** @description Set on the "outside" of a boundary edge. */
    VU_EXTERIOR_EDGE            = 0x00000002,
    /** @description The edge runs along a north or south pole singularity of parameter space, and hence has no length when that
        edge collapses to a point in 3D. */
    VU_NULL_EDGE                = 0x00000004,
    /** @description The edge marks a jump in periodic parameter values, in the manner of the international data line on a global map. */
    VU_SEAM_EDGE                = 0x00000008,
    /** @descriptio The node is at an original vertex of the graph. */
    VU_AT_VERTEX                = 0x00000010,
    /** @description The edge, in the context of an underlying B-spline parameterization, passes through a knot. */
    VU_KNOT_EDGE                = 0x00000020,
    /** @description The edge, in the context of an underlying B-spline parameterization, is an isoline (direction unspecified). */
    VU_RULE_EDGE                = 0x00000040,
    /** @description The edge is a grid line (direction unspecified). */
    VU_GRID_EDGE                = 0x00000080,
    /** @description The edge is a slice in the u-parametric direction. */
    VU_U_SLICE                  = 0x00000100,
    /** @description The edge is a slice in the v-parametric direction. */
    VU_V_SLICE                  = 0x00000200,
    /** @description The edge lies along a geometric discontinuity. */
    VU_DISCONTINUITY_EDGE       = 0x00000400,
    /** @description Set within a face that has zero area. */
    VU_NULL_FACE                = 0x00000800,
    /** @description The edge is part of a silhouette line. */
    VU_SILHOUETTE_EDGE          = 0x00001000,
    /** @description The vertex is on a silhouette. */
    VU_SILHOUETTE_VERTEX        = 0x00002000,
    /** @description The edge is the result of a section operation. */
    VU_SECTION_EDGE             = 0x00004000,
    /** @description The edge has been marked up for silhouette debugging. */
    VU_RXNURBS_EDGE             = 0x00008000,
    /** @description The edge is a fixed edge of the graph. */
    VU_ALL_FIXED_EDGES_MASK     = VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE | VU_NULL_EDGE | VU_SEAM_EDGE | VU_KNOT_EDGE
                                   | VU_RULE_EDGE | VU_GRID_EDGE | VU_DISCONTINUITY_EDGE | VU_NULL_FACE,
    } VuMaskBits;

/**
@description Mask bitfield for describing a VU node.
@see VuMaskBits
@group "VU Node Masks"
*/
typedef unsigned int VuMask;

/**
@description Bits in a ~tVuNeighborhoodMask describing the type of neighborhood around a node in its face sector.
@see vu_horizontalNeighborhood
@group "VU Node Masks"
*/
typedef enum vuneighborhoodmaskbits
    {
    /** @description A neighborhood around this node contains space to its left strictly inside its sector. */
    VU_LEFT_NEIGHBORHOOD    = 0x01,
    /** @description A neighborhood around this node contains space to its right strictly inside its sector. */
    VU_RIGHT_NEIGHBORHOOD   = 0x02,
    /** @description A neighborhood around this node contains space above it strictly inside its sector. */
    VU_UPPER_NEIGHBORHOOD   = 0x04,
    /** @description A neighborhood around this node contains space below it strictly inside its sector. */
    VU_LOWER_NEIGHBORHOOD   = 0x08,
    } VuNeighborhoodMaskBits;

/**
@description Mask bitfield for a VU node neighborhood.
@see VuNeighborhoodMaskBits
@group "VU Node Masks"
*/
typedef unsigned int VuNeighborhoodMask;

/**
@description A VuSetP is an opaque pointer to the header of a <A href="ussumvertex_use_graphs.htm">Vertex Use Graph</A>.
@group "VU Graph Header"
*/
typedef struct _VuSet  *VuSetP;

/**
@description A VuP is an opaque pointer to a single node in a ~tVuSetP.
@group "VU Nodes"
*/
typedef struct VuNode *VuP;

/**
@description A VuArrayP is an opaque pointer to a rubber array of ~tVuP.
@group "VU Node Arrays"
*/
typedef struct vuArray *VuArrayP;

/**
@description A VuMarkedEdgeSetP is an opaque pointer to a data type for managing sets of edges in a graph.
@group "VU Marked Edge Sets"
*/
typedef struct vuMarkedEdgeSet *VuMarkedEdgeSetP;

/**
@description The type of a qsort-style callback function for comparing two nodes.
@remarks Nodes are passed with double indirection, as typically required when a system sort routine is
    applied to an array of VU pointers.
@param ppNode0      IN  first node to compare
@param ppNode1      IN  second node to compare
@return -1,0,1 according to sort relationship
@see vu_arraySort0, vu_compareLexicalUV0
@group "VU Node Arrays"
*/
typedef int (*VuSortFunction0) (const VuP* ppNode0, const VuP* ppNode1);

/**
@description Stage of subdivision to be handled by a ~tVuSubdivideEdgeFuncP callback.
@see vu_subdivideAndRetriangulate
@group "VU Meshing"
*/
typedef enum vusubdivisionmessagetype
    {
    /** @description Split the given edge if necessary, and return new nodes as per ~mvu_splitEdgeAtPoint. */
    VU_MSG_TEST_EDGE_SUBDIVISION    = -1,
    /** @description Post-process the new edge at the given newly created node and whose edge mate is pre-existing. */
    VU_MSG_NEW_EDGE_NEWV_OLDV       = -2,
    /** @description Post-process the new edge at the given newly created node and whose edge mate is also newly created. */
    VU_MSG_NEW_EDGE_NEWV_NEWV       = -3
    } VuSubdivisionMessageType;

/**
@description Callback function for edge subdivision.
@param message      IN      stage of subdivision to handle
@param outNode1P    OUT     new node of split edge at point on same side as inNodeP, as per ~mvu_splitEdgeAtPoint.  If no subdivision is performed, return NULL.
@param outNode2P    OUT     new node of split edge at point on opposite side, as per ~mvu_splitEdgeAtPoint.  If no subdivision is performed, return NULL.
@param graphP       IN OUT  graph header
@param inNodeP      IN      base node of edge to consider
@param userDataP    IN      user data pointer
@see vu_subdivideAndRetriangulate
@group "VU Meshing"
*/
typedef void (*VuSubdivideEdgeFuncP)(VuSubdivisionMessageType message, VuP* outNode1P, VuP* outNode2P, VuSetP graphP, VuP inNodeP, void* userDataP);

/**
@description Callback function for announcing a convex face.
@param pPoints  IN  array of vertices of convex polygon, first == last
@param pFlags   IN  array of flags.  pFlags[i] == true iff the edge from pPoints[i] to pPoints[i+1] is a boundary edge (as determined by the
                    parity merge).
@param numverts IN  number of array entries
@param pUserArg IN  user data pointer
@see vu_splitToConvexParts
@group "VU Meshing"
*/
typedef void (*VuTaggedDPoint3dArrayFunction) (DPoint3d* pPoints, int* pFlags, int numverts, void* pUserArg);

/**
@description Callback function for operating on a graph after popping the graph stack with ~mvu_stackPopWithOperation.
@param graphP IN OUT graph header
@group "VU Graph Stack"
*/
typedef void (*VuStackOpFuncP) (VuSetP graphP);

/**
@description Callback function for announcing application-defined intermediate graph states to debuggers via ~mvu_postGraphToTrapFunc.
@param graphP   IN  graph header
@param pAppName IN  application-specific string
@param id0      IN  application-specific identifier
@param id1      IN  application-specific identifier
@see vu_setGraphTrapFunc, vu_postGraphToTrapFunc
@group "VU Debugging"
*/
typedef void (*VuGraphTrapFunc) (VuSetP graphP, char* pAppName, int id0, int id1);

/**
@description Type of boolean or merge operation to perform on the face loops of a graph.
@remarks A boolean operation is typically preceded by a merge step, in which all intersections and containments are found among the face loops.
    Duplicate edges are kept unless VuMergeType is VUUNION_PARITY or VUUNION_KEEP_ONE_AMONG_DUPLICATES.
@see vu_mergeOrUnionLoops, vu_consolidateClusterCoordinatesAndSortByAngle
@group "VU Booleans"
*/
typedef enum vumergetype
    {
    /** @description Remove pairs of duplicate edges during graph merge. */
    VUUNION_PARITY                    = 0,
    /** @description Boolean union. */
    VUUNION_UNION                     = 1,
    /** @description Boolean intersection. */
    VUUNION_INTERSECTION              = 2,
    /** @description Boolean subtraction (union of face A with complement of face B). */
    VUUNION_UNION_COMPLEMENT          = 3,
    /** @description Keep only one edge among all duplicates during graph merge. */
    VUUNION_KEEP_ONE_AMONG_DUPLICATES = 2002
    } VuMergeType;

/**
@description Message type code for ~tVuMessageFunction callback.
@remarks All predefined messages are negative.  Application code can add positive messages.
@see vu_setCMessageFunction
@group "VU Debugging"
*/
typedef enum vumessagetype
    {
    /** @description Edge between non-NULL node0P and node1P is to be split. */
    VU_MESSAGE_ANNOUNCE_PRE_SPLIT_EDGE  = -100,
    /** @description node0P and node1P were just inserted into an edge. */
    VU_MESSAGE_ANNOUNCE_POST_SPLIT_EDGE = -101,
    /** @description node0P and node1P are the new node pair. */
    VU_MESSAGE_ANNOUNCE_MAKE_PAIR       = -110,
    /** @description node0P and node1P are the ends of a new sling. */
    VU_MESSAGE_ANNOUNCE_MAKE_SLING      = -111,
    /** @description node0P and node1P are to be twisted.
        @see vu_vertexTwist */
    VU_MESSAGE_ANNOUNCE_PRE_VTWIST      = -120,
    /** @description node0P and node1P were just twisted.
        @see vu_vertexTwist */
    VU_MESSAGE_ANNOUNCE_POST_VTWIST     = -121,
    /** @description Unexpected operation on the graph. */
    VU_MESSAGE_ANNOUNCE_PANIC           = -200,
    } VuMessageType;

#define VU_STATUS_MESSAGE_NOT_HANDLED -2

/**
@description Callback function to be called at important predefined transitions in a VU graph.
@param message      IN  message describing update or query
@param graphP       IN  parent vu graph
@param node0P       IN  message-dependent node
@param node1P       IN  message-dependent node
@param argP         IN  message-dependent argument (NULL for predefined calls)
@param userDataP    IN  application-specific argument
@return SUCCESS or VU_STATUS_MESSAGE_NOT_HANDLED (ignored by predefined calls)
@see vu_setCMessageFunction
@group "VU Debugging"
*/
typedef int (*VuMessageFunction)(VuMessageType message, VuSetP graphP, VuP node0P, VuP node1P, void* argP, void* userDataP);

/**
@description Message type code for ~tVuEdgeTestFunction callback.
@see vu_testAndExciseSmallEdges
@group "VU Edges"
*/
typedef enum vuedgetestmessagetype
    {
    /** @description Test if the edge should be deleted and if so, return SUCCESS. */
    VUF_MESSAGE_TEST_DELETED_EDGE_CANDIDATE             = 1,
    /** @description Install new vertex coordinates around both vertex loops, and return SUCCESS if successful.
        @remarks Typically, the new coordinates are set to the midpoint of the edge being excised. */
    VUF_MESSAGE_RESET_DELETED_EDGE_VERTEX_COORDINATES   = 2,
    /** @description Return SUCCESS to mark and excise arbitrary degenerate faces, or ERROR to keep them.
        @remarks The callback's edge node0P has already been removed at this point, and should be ignored.  This is an arbitrary choice to
            remove or not to remove a face that has become degenerate as a result of edge excision. */
    VUF_MESSAGE_QUERY_DEGENERATE_FACE_ACTION            = 3,
    } VuEdgeTestMessageType;

/**
@description Callback function for edge excision.
@param message      IN  message describing operation to perform
@param graphP       IN  parent vu graph
@param node0P       IN  edge being tested or removed
@param userDataP    IN  caller context
@return SUCCESS if message-dependent operation succeeded
@see vu_testAndExciseSmallEdges
@group "VU Edges"
*/
typedef StatusInt (*VuEdgeTestFunction)(int message, VuSetP graphP, VuP node0P, void* userDataP);

/**
@description Callback function for triangulation refinement, called to set the refinement priority of an edge.
@param pData        IN  caller context
@param pGraph       IN  vu graph (triangulated)
@param pNode        IN  edge for which to compute priority
@return positive priority for refinement of this edge, or nonpositive to prohibit its refinement
@see vu_refine
@group "VU Meshing"
*/
typedef double (*VuRefinementPriorityFunc)(void* pData, VuSetP pGraph, VuP pNode);

/**
@description Callback function for triangulation refinement, called to split an edge.
@param pData        IN  caller context
@param pGraph       IN  vu graph (triangulated)
@param pBase        IN  edge to split
@param ppLeft       OUT new node at split point on the same side of the edge as pBase
@param ppRight      OUT new node at split point on the opposite side of the edge as pBase
@return whether the edge was split
@see vu_refine
@group "VU Meshing"
*/
typedef bool    (*VuRefinementEdgeSplitFunc)(void* pData, VuSetP pGraph, VuP pBase, VuP* ppLeft, VuP* ppRight);

/**
@description Callback function for triangulation refinement, called after a new edge is joined to a subdivided edge.
@param userDataP    IN  caller context
@param graphP       IN  vu graph (triangulated)
@param node0P       IN  first original node of joined pair
@param node1P       IN  second original node of joined pair
@param new0P        IN  first newly-created node of join edge
@param new1P        IN  second newly-created node of join edge
@see vu_refine, vu_triangulateBetweenCyclesByZ, vu_join
@group "VU Meshing"
*/
typedef void (*VuRefinementJoinFunc)(void* userDataP, VuSetP graphP, VuP node0P, VuP node1P, VuP new0P, VuP new1P);

/**
@description Callback function for triangulation refinement, called to test whether an edge should be flipped in its surrounding quad.
@param pData        IN  caller context
@param pGraph       IN  vu graph (triangulated)
@param pBaseNode    IN  edge to test
@return whether to flip the edge or not
@see vu_refine
@group "VU Meshing"
*/
typedef bool    (*VuRefinementFlipTestFunc)(void* pData, VuSetP pGraph, VuP pBaseNode);

/** @declarealinkgroup  TriangleFlipFunctions (usmthvu_flipTrianglesToImproveQuadraticAspectRatio,
                                               usmthvu_flipTrianglesToImproveScaledQuadraticAspectRatio,
                                               usmthvu_flipTrianglesToImproveScaledPeriodicQuadraticAspectRatio,
                                               usmthvu_flipTrianglesToImproveUAspectRatio,
                                               usmthvu_flipTrianglesToImproveVAspectRatio)
*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Macros                                                              |
|                                                                       |
+----------------------------------------------------------------------*/

#if !defined (VU_DO_NOT_DEFINE_LOOP_MACROS)
#define VU_SET_LOOP(P,SP)       \
        {VuP P = vu_firstNodeInGraph(SP);       \
         for (P = vu_firstNodeInGraph(SP);      \
                P; P = vu_nextNodeInGraph (SP, P))\
            {

#define END_VU_SET_LOOP(P,SP)   \
            }   \
        }

#define VU_FACE_LOOP(Ploop,Pstart)      \
                {VuP Ploop=Pstart;      \
                do{
#define END_VU_FACE_LOOP(Ploop,Pstart)  \
                Ploop = vu_fsucc(Ploop);\
                }while(Ploop != Pstart);\
                }

#define VU_VERTEX_LOOP(Ploop,Pstart)    \
                {VuP Ploop=Pstart;      \
                do{
#define END_VU_VERTEX_LOOP(Ploop,Pstart)\
                Ploop = vu_vsucc(Ploop);\
                }while(Ploop != Pstart);\
                }

#define VU_REVERSE_VERTEX_LOOP(Ploop,Pstart)    \
                {VuP Ploop=Pstart;      \
                do{
#define END_VU_REVERSE_VERTEX_LOOP(Ploop,Pstart)\
                Ploop = vu_vpred (Ploop);\
                }while(Ploop != Pstart);\
                }




#define VU_FILTERED_VERTEX_LOOP(Ploop,Pstart,FilterCondition)   \
                {VuP Ploop=Pstart;      \
                do{                     \
                if ( (FilterCondition) ) {
#define END_VU_FILTERED_VERTEX_LOOP(Ploop,Pstart)\
                }\
                Ploop = vu_vsucc(Ploop);\
                }while(Ploop != Pstart);\
                }
#endif

//! Detail structure for a VuP and integer.
struct VuPInt
{
VuP m_node;
int m_data;
VuPInt (VuP node, int data) : m_node (node), m_data(data) {}
};


//! Detail data for:
//! <ul>
//! <li>NodeId () -- a relevant node</li>
//! <li>XYZ () -- coordinate</li>
//! <li>TopologyScope () -- indicats if the node is representing face, edge, vertex.</li>
//! <li>EdgeFraction () -- position along edge.
//! </ul>
//! Note that the XYZ () coordinate saved in the VuPositionDetail
//! is not necessarily the same as the xyz of the node.  In fact, the
//!    VuPositionDetail does not know its containing graph or facet set
//!    needed to find node coordinates.   VuPositionDetail is just
//!    a data carrier.
//!
struct VuPositionDetail
    {
    enum TopologyScope
        {
        Topo_None = 0,
        Topo_Vertex = 1,
        Topo_Edge = 2,
        Topo_Face = 3
        };
    private:
    VuP m_node;
    double    m_edgeFraction;
    DPoint3d  m_xyz;
    TopologyScope      m_topology;
    double    m_dTag;
    ptrdiff_t m_iTag;
    //! Construct with node, coordinates, and topology indicator.
    VuPositionDetail (VuP node, DPoint3dCR xyz, TopologyScope topo);

    //! Construct with edge-specific data.
    VuPositionDetail (VuP node, DPoint3dCR xyz, double edgeFraction);

    public:
    GEOMDLLIMPEXP VuPositionDetail ();

    // Construct with given iTag but otherwise defaults
    GEOMDLLIMPEXP VuPositionDetail (ptrdiff_t iTag);

    GEOMDLLIMPEXP ptrdiff_t GetITag () const;
    GEOMDLLIMPEXP void SetITag (ptrdiff_t value);
    GEOMDLLIMPEXP void IncrementITag (ptrdiff_t step = 1);

    GEOMDLLIMPEXP double GetDTag () const;
    GEOMDLLIMPEXP void SetDTag (double value);

    //! Construct and return as representative of face.
    GEOMDLLIMPEXP static VuPositionDetail FromFace (VuP node, DPoint3dCR xyz);

    //! Construct and return as representative of edge.
    GEOMDLLIMPEXP static VuPositionDetail FromEdge (VuP node, DPoint3dCR xyz, double edgeFraction);

    //! Construct and return as representative of vertex.
    GEOMDLLIMPEXP static VuPositionDetail FromVertex (VuP node, DPoint3dCR xyz);
    //! Construct and return as representative of vertex.
    GEOMDLLIMPEXP static VuPositionDetail FromVertex (VuP node);

    //! @return true if there is a non-null nodeid.
    GEOMDLLIMPEXP bool HasNodeId () const;
    //! @return edge fraction.
    GEOMDLLIMPEXP double GetEdgeFraction () const;
    //! @return topology type
    GEOMDLLIMPEXP TopologyScope GetTopologyScope () const;

    //! @return true if topology type is face
    GEOMDLLIMPEXP bool IsFace () const;
    //! @return true if topology type is edge
    GEOMDLLIMPEXP bool IsEdge () const;
    //! @return true if topology type is vertex
    GEOMDLLIMPEXP bool IsVertex () const;
    //! @return true if typology type is null
    GEOMDLLIMPEXP bool IsUnclassified () const;
    

    //! @return node id
    GEOMDLLIMPEXP VuP GetNode () const;
    //! @return coordinates
    GEOMDLLIMPEXP DPoint3d GetXYZ () const;
    //! @return vector to coordinates in the other VuPositionDetail
    GEOMDLLIMPEXP DVec3d VectorTo (VuPositionDetail const &other) const;

    //! @return VuPositionDetail for this instance's edge mate;
    //!    The returned VuPositionDetail's edgeFraction is {1 - this->EdgeFraction ())
    //!    to properly identify the "same" position relative to the other side.
    GEOMDLLIMPEXP VuPositionDetail EdgeMate () const;


    //! Move pointer to mate on other side of edge.
    //! All other member data unchanged !!
    GEOMDLLIMPEXP void MoveToEdgeMate ();
    //! Move pointer to successor around face.
    //! All other member data unchanged !!
    GEOMDLLIMPEXP void MoveToFSucc ();
    //! Move pointer to successor around vertex.
    //! All other member data unchanged !!
    GEOMDLLIMPEXP void MoveToVSucc ();
    //! Move pointer to predecessor around face.
    //! All other member data unchanged !!
    GEOMDLLIMPEXP void MoveToFPred ();
    //! Move pointer to predecessor around vertex.
    //! All other member data unchanged !!
    GEOMDLLIMPEXP void MoveToVPred ();

    //! @return x coordinate
    GEOMDLLIMPEXP double GetX () const;
    //! @return y coordinate
    GEOMDLLIMPEXP double GetY () const;
    //! @return z coordinate
    GEOMDLLIMPEXP double GetZ () const;

    // If candidateKey is less than resultKey, replace resultPos and resultKey
    // by the candidate data.
    GEOMDLLIMPEXP static bool UpdateMinimizer
            (
            VuPositionDetail &resultPos, double &resultKey,
            VuPositionDetail const &candidatePos, double candidateKey);

    GEOMDLLIMPEXP static bool UpdateMin
            (
            VuPositionDetail &resultPos,
            VuPositionDetail const &candidatePos
            );

    GEOMDLLIMPEXP static bool UpdateMax
            (
            VuPositionDetail &resultPos,
            VuPositionDetail const &candidatePos
            );
};

//! Callback object for position searches . . .
struct VuPositonDetailAnnouncer
{
//! Called to announce a new position.  Returning false terminates the search.
GEOMAPI_VIRTUAL bool AnnounceAndTestPosition (VuPositionDetailR position) = 0;
};

END_BENTLEY_GEOMETRY_NAMESPACE

#include "capi/vu_capi.h"
#include "capi/vucoord_capi.h"
#include "capi/vumem_capi.h"
#include "capi/vusubset_capi.h"
#include "capi/vuarray_capi.h"
#include "capi/vumod_capi.h"
#include "capi/vumod2_capi.h"
#include "capi/vuconvex_capi.h"
#include "capi/vuaddedges_capi.h"
#include "capi/vuunion_capi.h"
#include "capi/vumerge_capi.h"
#include "capi/vumerge2_capi.h"
#include "capi/vufilter_capi.h"
#include "capi/vutriang_capi.h"
#include "capi/vureg_capi.h"
#include "capi/vustack_capi.h"
#include "capi/vucluster_capi.h"
#include "capi/vuperiodicconnect_capi.h"
#include "capi/vugrid_capi.h"
#include "capi/vuprint_capi.h"
#include "capi/vudangler_capi.h"
#include "capi/vupoly_capi.h"
#include "capi/vupoly_bsplineBoundaryLoops_capi.h"
#include "capi/vumaskops_capi.h"
#include "VuOps.h"
#include "capi/pbfvu_capi.h"
#include "capi/vusmooth_capi.h"
#include "VuMultiClip.h"
#include "capi/vuSearch_capi.h"
#include "capi/vupointin_capi.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define VU_DO_NOT_DEFINE_LOOP_MACROS

/* Masks available to vu_grabMask */
#define VU_FREEMASK0            0x00010000
#define VU_FREEMASK1            0x00020000
#define VU_FREEMASK2            0x00040000
#define VU_FREEMASK3            0x00080000
#define VU_FREEMASK4            0x00100000
#define VU_FREEMASK5            0x00200000
#define VU_FREEMASK6            0x00400000
#define VU_FREEMASK7            0x00800000
#define VU_FREEMASK8            0x01000000
#define VU_FREEMASK9            0x02000000
#define VU_FREEMASKA            0x04000000
#define VU_FREEMASKB            0x08000000
#define VU_FREEMASKC            0x10000000
#define VU_FREEMASKD            0x20000000
#define VU_FREEMASKE            0x40000000
#define VU_FREEMASKF            0x80000000

typedef union
    {
    void *asPointer;
    int   asInt;      // 32 bit int, not sysintptr style !!!
    } VuVoidP;

#define VU_NULL ((VuP)0)

/**
@description Callback called by ~mvu_join on each pre-existing node after the topological join is completed.
@param P IN Pre-existing node whose vertex successor was just inserted
@group "VU Edges"
*/
typedef void (*VuEdgeInsertionFunction)(VuSetP Sp, VuP P);

typedef struct
    {
    VuMessageFunction functionP;        /* Application function */
    void *userDataP;                    /* Application data */
    VuMessageFunction auxFunctionP;     /* For rerouting to mdl */
    void *auxDataP;                     /* For rerouting to mdl */
    } VuMessagePacket, *VuMessagePacketP;

/*----------------------------------------------------------------------+
|Check the graphP's message handler to see if it is nonNull.  If so,    |
| call it and return its return value.  If not, return a default value. |
+----------------------------------------------------------------------*/
#define VU_THROW_MESSAGE(message, graphP, node0P, node1P, argP, defaultValue)   \
        (                                                       \
        graphP->mPrimitiveData.messagePacket.functionP                 \
                ? graphP->mPrimitiveData.messagePacket.functionP (             \
                                message,                        \
                                graphP,                         \
                                node0P,                         \
                                node1P,                         \
                                argP,                           \
                                graphP->mPrimitiveData.messagePacket.userDataP\
                                )                               \
                : defaultValue                                  \
        )

/*
   Base type structure for vu topology with u,v coordinates.
   ALL FIELD REFERENCES IN THIS STRUCUTRE SHOULD BE VIA FUNCTIONS
    (Macros are deprecated)
*/
struct VuNode {
    VuP next;                   /* Next node in the parent graph's arbitrary ordering. */
    VuP fs;                     /* ccw face successor */
    VuP vs;                     /* ccw vertex successor */
    VuMask mask;                /* semantic property bitmap */
    int id;                     /* sequential id from parent graph. */
    VuVoidP internalDataP;      /* internal data pointer field */
    VuVoidP userId;             /* Available to application */
    DPoint3d uv;                /* u,v coordinates. */
    ptrdiff_t userId1;          /* Another user id */
    GEOMDLLIMPEXP int GetId () const;
    GEOMDLLIMPEXP void SetXYZ (DPoint3dCR xyz);
    GEOMDLLIMPEXP void SetXYZ (double x, double y, double z = 0.0);

    GEOMDLLIMPEXP void SetMask (VuMask mask);
    GEOMDLLIMPEXP void ClearMask(VuMask mask);
    GEOMDLLIMPEXP void ToggleMask(VuMask mask);
    GEOMDLLIMPEXP bool HasMask(VuMask mask) const;

    GEOMDLLIMPEXP int  CountMaskAroundFace (VuMask mask) const;
    GEOMDLLIMPEXP int  CountMaskAroundVertex(VuMask mask) const;

    GEOMDLLIMPEXP void SetMaskAroundFace(VuMask mask);
    GEOMDLLIMPEXP void ClearMaskAroundFace(VuMask mask);

    GEOMDLLIMPEXP void SetMaskAroundVertex(VuMask mask);
    GEOMDLLIMPEXP  void ClearMaskAroundVertex (VuMask mask);

    GEOMDLLIMPEXP void SetMaskAroundEdge(VuMask mask);
    GEOMDLLIMPEXP  void ClearMaskAroundEdge(VuMask mask);

    GEOMDLLIMPEXP VuP FindMaskAroundFace(VuMask mask);
    GEOMDLLIMPEXP VuP FindMaskAroundVertex(VuMask mask);
    GEOMDLLIMPEXP VuP FindMaskAroundReverseFace(VuMask mask);
    GEOMDLLIMPEXP VuP FindMaskAroundReverseVertex(VuMask mask);

    GEOMDLLIMPEXP VuP FindNodeAroundFace(VuP node);
    GEOMDLLIMPEXP VuP FindNodeAroundVertex(VuP node);
    // Return the (xy) cross product of vectors from this node to targets ..
    GEOMDLLIMPEXP double CrossXY (VuP nodeA, VuP nodeB) const;

    GEOMDLLIMPEXP DPoint3d GetXYZ () const;
    GEOMDLLIMPEXP ptrdiff_t GetUserData1 () const;
    GEOMDLLIMPEXP ptrdiff_t SetUserData1 (ptrdiff_t value);
    GEOMDLLIMPEXP int GetUserDataAsInt () const;
    GEOMDLLIMPEXP int SetUserDataAsInt(int value);
    GEOMDLLIMPEXP void SetUserDataAsIntAroundVertex (int value);
    GEOMDLLIMPEXP void SetUserDataAsIntAroundFace (int value);

    GEOMDLLIMPEXP VuP FSucc () const;
    GEOMDLLIMPEXP VuP VSucc () const;
    GEOMDLLIMPEXP VuP FPred () const;
    GEOMDLLIMPEXP VuP VPred () const;
    GEOMDLLIMPEXP VuP EdgeMate () const;
    
    };

typedef struct
    {
    VuP pNode;
    int nextInCluster;
    double sortCoordinate;
    DPoint2d xy;
    } VuVertexSortKey;

typedef struct
    {
    int cluster0;
    int cluster1;
    VuP pNode0;
    VuP pNode1;
    double theta;
    } VuEdgeSortKey;



/*----------------------------------------------------------------------+
|                                                                       |
|   Macros                                                              |
|                                                                       |
+----------------------------------------------------------------------*/

#define VU_NEXT(P) ((P)->next)  /* Access to next node in parent graph's order */
#define VU_FSUCC(P) ((P)->fs)   /* Successor around face */
#define VU_VSUCC(P) ((P)->vs)   /* Successor around vertex */
#define VU_FPRED(P) ((P)->vs->fs->vs)   /* Predecessor around face */
#define VU_VPRED(P) ((P)->fs->vs->fs)   /* Predecessor around vertex */
#define VU_EDGE_MATE(P) ((P)->fs->vs)   /* VU on opposing side of edge to left */
#define VU_ANY_NODE_IN_GRAPH(graphP) ((VuP) ((graphP) ? (graphP)->lastNodeP : NULL ))

#define VU_MASK(P)      ((P)->mask)                             /* The complete mask */
#define VU_ID(P)        ((P)->id)                               /* The integer id */

/* These macros address the mask bits in individual node P */
#define VU_GETMASK(P,m) ((P)->mask & (m))       /* Access the mask bits in m */
#define VU_SETMASK(P,m) ((P)->mask |= (m))      /* Set the mask bits in m */
#define VU_CLRMASK(P,m) ((P)->mask &= ~(m))     /* Clear the mask bits in m */

/* These macros address the mask bits in the graph header G */
#define VUGRAPH_GETMASK(G,m) ((G)->mGraph & (m))        /* Access the mask bits in m */
#define VUGRAPH_SETMASK(G,m) ((G)->mGraph |= (m))       /* Set the mask bits in m */
#define VUGRAPH_CLRMASK(G,m)    ((G)->mGraph &= ~(m))   /* Clear the mask bits in m */


#define VU_WRITEMASK(P,m,value) ((value) ? VU_SETMASK((P),(m)) : VU_CLRMASK((P),(m)) )

#define VU_DATA_BLOCK(P) (void*)(P+1)           /* Address of data block allocated after P */

#define VU_USER_ID(P) ((P)->userId)
#define VU_USER_ID_AS_INT(P) ((int)VU_USER_ID(P))

#define VU_U(P)                                         ((P)->uv.x)
#define VU_V(P)                                         ((P)->uv.y)
#define VU_W(P)                                         ((P)->uv.z)
#define VU_SET_V(P,v)                                   ((P)->uv.y = (v))
#define VU_UV(P) (*((DPoint2d*)&(P)->uv))                       /* coordinates as DPoint2d */
#define VU_UVW(P) (*((DPoint3d*)&(P)->uv))                      /* coordinates as DPoint3d */
#define VU_SET_UV(P,u,v)                                (VU_U((P))=(u),VU_V((P))=(v))
#define VU_SET_UVW(P,u,v,w)                             (VU_U((P))=(u),VU_V((P))=(v),VU_W((P))=(w))

#define VU_GET_UV(P,u,v)                                ((u)=(P)->uv.x ,(v)=(P)->uv.y)
#define VU_COPY_UV(P,Q)                 ( ((P)->uv) = ((Q)->uv) )
#define VU_CROSS_PRODUCT_FROM_COMPONENTS(u1,v1,u2,v2) ( (u1)*(v2) - (u2)*(v1) )
#define VU_dU(P) (VU_FSUCC((P))->uv.x - (P)->uv.x)
#define VU_dV(P) (VU_FSUCC((P))->uv.y - (P)->uv.y)

/* Tests for lexical sorting
   P is BELOW Q if the v coordinates are strictly increasing
                                or the v coordinates are strictly equal and the u coordinates increasing
   P is LEFTOF Q if the u coordinates are strictly increasing
                            or the u coordinates are strictly equal and the v coordinates decreasing
*/
#define VU_BELOW(P,Q) ( VU_V((P)) < VU_V((Q)) \
                || (   VU_V((P)) == VU_V((Q)) && VU_U((P)) < VU_U((Q)) ))
#define VU_LEFTOF(P,Q) ( VU_U((P)) < VU_U((Q)) \
                || (   VU_U((P)) == VU_U((Q)) && VU_V((P)) > VU_V((Q)) ))

#define VU_SAME_UV(P,Q) ( VU_U(P) == VU_U(Q) && VU_V(P) == VU_V(Q) )

#define VU_SET_LABEL(P,value)   vu_setInternalDataPAsInt (P, value)
#define VU_COPY_LABEL(P,Q)      vu_copyInternalDataP (P, Q)//(VU_LABEL((P)) = VU_LABEL((Q)))
#define VU_GET_LABEL_AS_INT(P)  vu_getInternalDataPAsInt (P)


#define VU_FACE_LOOP(Ploop,Pstart)      \
                {VuP Ploop=Pstart;      \
                do{

#define VU_REVERSE_FACE_LOOP(Ploop,Pstart)      \
                {VuP Ploop=Pstart;      \
                do{
#define END_VU_REVERSE_FACE_LOOP(Ploop,Pstart)  \
                Ploop = VU_FPRED(Ploop);\
                }while(Ploop != Pstart);\
                }

#define VU_VERTEX_LOOP(Ploop,Pstart)    \
                {VuP Ploop=Pstart;      \
                do{

#define VU_FILTERED_VERTEX_LOOP(Ploop,Pstart,FilterCondition)   \
                {VuP Ploop=Pstart;      \
                do{                     \
                if ( (FilterCondition) ) {
// extract faces from a graph into a polyface.
GEOMDLLIMPEXP PolyfaceHeaderPtr vu_toPolyface (VuSetP graph, VuMask faceExclusionMask);
END_BENTLEY_GEOMETRY_NAMESPACE

