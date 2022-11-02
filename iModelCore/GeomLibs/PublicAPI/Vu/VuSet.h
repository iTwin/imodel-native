/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define VU_MAX_FREE_MASK 16
#define VU_MAX_FREE_ARRAY 6


// This structure contains the "easily copied" (int, double, bool) data of a VuSet header.
struct VuPrimitiveData
{
        VuMask  mNewLoopExterior;       /* Mask for exterior side of newly created loop. */
        VuMask  mNewLoopInterior;       /* Mask for interior side of newly created loop. */
        VuMask  mCopyOnSplit;           /* Mask of copied bits when an edge is split */
        VuMask  mCopyAroundVertex;      /* Mask of copied bits when a vertex sector is split. */
        VuMask  mGraph;                 /* Mask bits for the graph as a whole. Addressable
                                                via VUGRAPH_xxxMASK(..) */
        int mExtraBytesPerNode;

        DPoint3d                periods;
        double                  abstol;     // absolute tolerance used in merge
        double                  reltol;     // relative tolerance used in merge
        double                  mergeTol;       // Tolerance used in most recent merge -- result  of abstol, reltol, and data range.

        VuVoidP                 defaultUserId;              // default value to store in userId field when a node is created.
        bool                    bUserDataIsVertexProperty;  // and should be moved around vertex on "join"
        bool                    bUserDataIsEdgeProperty;    // and should be copied to both halves of "split"


        ptrdiff_t               defaultUserId1;             // default value to store in the userId1 field when a node is created.
        bool                    bUserData1IsVertexProperty;
        bool                    bUserData1IsEdgeProperty;
        VuMessagePacket         messagePacket;          /* Application function */
        /* Functions called to allow application operations in the auxilliary data block */
        VuEdgeInsertionFunction edgeInsertionFunction;

VuPrimitiveData (int extraBytesPerNode);
};

// VuArray is a bvector of node pointers with "read position" for use by vu_arrayOpen () and vu_arrayRead () (and a few others)
// A vuArray can be deleted -- it never owns its VuP targets.
// The grabCounter gives the Grab/Return methods a crude validity check.
struct vuArray : bvector<VuP>
{
// Number of completed read's during vu_arrayOpen ()+vu_arrayRead () sequence.
size_t nRead;
// Counter for checking grab/drop matching for array pointers cached in the set header.
ptrdiff_t grabCount;
vuArray ();
// This increments the grabCount (at grab time).   Debug builds assert count==0 on entry.
void RecordGrab ();
// Decrement the grabCount (at drop time).   Debug builds assert count==1 on entry.
void RecordDrop ();
};


// Data packet with a pair of nodes, integer, double, and VuMask.
struct TaggedVuP
{
VuP m_nodeA;
VuP m_nodeB;
double m_a;
int   m_id;
VuMask m_mask;
TaggedVuP (VuP node, double a = 0.0, int id = 0, VuMask mask = 0)
    : m_nodeA(node), m_nodeB (NULL), m_a (a), m_id (id), m_mask (mask)
    {
    }
TaggedVuP (VuP nodeA, VuP nodeB, int id) :
    m_nodeA (nodeA), m_nodeB (nodeB), m_a (0.0), m_id (id), m_mask (0)
    {
    }
};

// Helper class to run flood search.
// Derived classes implement virtuals to record and act on the search sequence.
//
struct VuFloodSearcher
{
bvector<TaggedVuP>m_stack;  // stack recording nodes waiting to be processed.
VuMask m_visitMask;         // mask for visited nodes
VuSetP m_graph;             // containing graph.

bvector<TaggedVuP> m_seedNodes;
// save graph pointer.
void BindGraph (VuSetP pGraph);
// clear pointers and masks from current graph.
void Release ();

VuFloodSearcher ();
GEOMAPI_VIRTUAL ~VuFloodSearcher ();

// Step from outside node and depth to its edge mate inside.
// Return new depth
GEOMAPI_VIRTUAL int StepIntoFace(VuP outside, int outsideDepth, VuP inside);
// Mark seed face.
// Return initial depth value.
GEOMAPI_VIRTUAL int MarkSeedFace(VuP pSeed);

// Run flood search from a particular seed face.
int RunFloodFromSeedFace (VuP pSeed);
// Search for all negative area faces.  Run flood from each.
int RunFloodFromAllNegativeAreaFaces ();
};







// Flood search markup for parity rules:
//  Crossing a boundary flips parity.
struct ParityVuFloodSearcher : VuFloodSearcher
{
VuMask m_boundaryMask;
VuMask m_exteriorMask;
GEOMAPI_VIRTUAL ~ParityVuFloodSearcher (){}
void Bind (VuSetP graph, VuMask boundaryMask, VuMask exteriorMask);
int MarkSeedFace (VuP seed) override;
int StepIntoFace(VuP outside, int outsideDepth, VuP inside) override;
};

// Flood search markup for union rules.
// User code sets singleExteriorMask on outside of each input region.
// Searcher sets compositeExteriorMask.
struct UnionVuFloodSearcher : VuFloodSearcher
{
int m_minimumWindingForInside;
GEOMAPI_VIRTUAL ~UnionVuFloodSearcher (){}
VuMask m_singleExteriorMask;
VuMask m_compositeExteriorMask;
// singleExteriorMask -- mask set on the true outside of all individual regions.
// compositeExteriorMask -- mask to be set on the outside of the union.
// minimumWindingForInside -- winding number to be considered "in".  1 is simple union.  2 is "intersection of two or more"
void Bind (VuSetP graph, VuMask singleExteriorMask, VuMask compositeExteriorMask, int minimumWindingForInside = 1);

int MarkSeedFace (VuP seed) override;

int StepIntoFace(VuP outside, int outsideDepth, VuP inside) override;
};




// bvector of VuArrayP.  The destructor frees all.
struct VuArrays : bvector<VuArrayP>
{
void DeleteAndClear ();  // delete all the arrays and clear.
VuArrays ();
~VuArrays ();   // destructor invokes Clear.
};
// THIS SHOULD ONLY BE ACCESSED WITHIN GEOMLIBS.
// (Included via pch file -- not by VuAPI.h)
struct _VuSet {
public:
      GEOMDLLIMPEXP _VuSet (int extraBytesPerNode = 0);
      GEOMDLLIMPEXP ~_VuSet ();
//private:  // ugh -- can't be private because of unnumerable vu_xxx functions.  (yet?)
       VuPrimitiveData mPrimitiveData;

        VuP lastNodeP;                  /* Last node in cyclic linked list of all vu's in the currently active nodes.
                                              (and each stacked node set has its own lastNodeP) */
        VuP freeNodeP;                  /* First node in null terminated single linked list of
                                                available nodes */
        int nvu;                        /* Number of vertex uses */

        vuArray  mGraphStack;             /* Stack of partial graphs 
                                          (Each partial graph is "identified" only by its linked list of nodes.)
                                        */
        bvector<void*> mNodePool;          /* Array of pointers to blocks of nodes.
                                            The various lists and loops of nodes are threaded
                                            through here.  Node pool manager manages those reached
                                            through freeNodeP, but never touches others.
                                            The VuSet destructor frees all the heap blocks before allowing the bvector to free its block.
                                            */

        /* Cache of mask bits for application-defined marking algorithms */
        bvector <VuMask> mFreeMasks;
        /* Cache of VuArray's that can be loaned to an application */
        VuArrays mFreeArrays;
        int mNumberOfArraysGrabbed;


        bvector<VuVertexSortKey> mClusterSortArray;  // used only by vucluster.cpp -- new/delete with set to avoid reallocation on reuse.
        bvector<VuEdgeSortKey>   mEdgeSortArray;  // used only by vucluster.cpp -- new/delete with set to avoid reallocation on reuse.
        ParityVuFloodSearcher mParityFloodSearcher;
        UnionVuFloodSearcher mUnionFloodSearcher;

public:
GEOMDLLIMPEXP VuMask GrabMask ();
GEOMDLLIMPEXP void DropMask (VuMask mask);

GEOMDLLIMPEXP int CountNodesInSet () const;
GEOMDLLIMPEXP int CountMaskedNodesInSet (VuMask mask) const;
GEOMDLLIMPEXP int CountUnmaskedNodesInSet (VuMask mask) const;

GEOMDLLIMPEXP void SetMaskInSet (VuMask mask);
GEOMDLLIMPEXP void ClearMaskInSet(VuMask mask);
GEOMDLLIMPEXP void ToggleMaskInSet(VuMask mask);

GEOMDLLIMPEXP DRange3d Range () const;
GEOMDLLIMPEXP void VertexTwist (VuP node0P, VuP node1P);

struct TempMask 
{
private:
VuMask m_mask;
VuSetP m_graph;
public:
TempMask (VuSetP graph, bool initialState = false) : m_graph (graph)
    {
    m_mask = graph->GrabMask ();
    if (initialState)
        graph->SetMaskInSet (m_mask);
    else
        graph->ClearMaskInSet (m_mask);
    }
~TempMask ()
    {
    m_graph->DropMask (m_mask);
    }
VuMask Mask (){return m_mask;}
bool IsSetAtNode (VuP node) { return node->HasMask (m_mask);}
void SetAtNode (VuP node){node->SetMask (m_mask);}
void ClearAtNode (VuP node){node->ClearMask (m_mask);}
void SetAroundVertex (VuP node){node->SetMaskAroundVertex (m_mask);}
void SetAroundFace (VuP node){node->SetMaskAroundFace (m_mask);}
};

};


extern UsageSums g_vuGrabbedArraysInSetCount;
extern UsageSums g_vuFinalFreeArrayCapacity;
extern UsageSums g_vuDroppedArraySize;
extern UsageSums g_vuAllocatedNodeCount;

END_BENTLEY_GEOMETRY_NAMESPACE
