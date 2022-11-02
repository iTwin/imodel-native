/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>


Public GEOMDLLIMPEXP VuP vu_findMostNegativeAreaFace
(
VuSetP pGraph
)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    double a, aMin;
    VuP pMinAreaSeed = NULL;
    vu_clearMaskInSet (pGraph, visitMask);
    aMin = 0.0;
    // Collect negative area faces ...
    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            vu_setMaskAroundFace (pSeedNode, visitMask);
            a = vu_area (pSeedNode);
            if (a < aMin)
                {
                aMin = a;
                pMinAreaSeed = pSeedNode;
                }
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)
    vu_returnMask (pGraph, visitMask);
    return pMinAreaSeed;
    }


bool cb_is_a_le (TaggedVuP const &dataA, TaggedVuP const &dataB)
    {
    return dataA.m_a <= dataB.m_a;
    }

template<typename T>
void ClearAndReserveGraphSize (VuSetP graph, bvector<T> *data, int divisor)
    {
    if (NULL != data)
        {
        int n= vu_countNodeAllocations (graph);
        if (divisor > 1)
          n /= divisor;
        data->clear ();
        data->reserve ((size_t) n);
        }
    }

//! Apply exteriorMask to pSeed and its face.
Public GEOMDLLIMPEXP void vu_collectFacesByAreaSign
(
VuSetP pGraph,
bvector<TaggedVuP> *pPositiveAreaFaces,
bvector<TaggedVuP> *pNegativeAreaFaces
)
    {
    ClearAndReserveGraphSize (pGraph, pPositiveAreaFaces, 3);
    ClearAndReserveGraphSize (pGraph, pNegativeAreaFaces, 3);

    VuMask visitMask = vu_grabMask (pGraph);
    double a;
    vu_clearMaskInSet (pGraph, visitMask);

    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            vu_setMaskAroundFace (pSeedNode, visitMask);
            a = vu_area (pSeedNode);
            if (a >= 0.0)
                {
                if (NULL != pPositiveAreaFaces)
                    pPositiveAreaFaces->push_back (TaggedVuP (pSeedNode, a, 0, 0));
                }
            else
                {
                if (NULL != pNegativeAreaFaces)
                    pNegativeAreaFaces->push_back (TaggedVuP (pSeedNode, a, 0, 0));
                }
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)
    vu_returnMask (pGraph, visitMask);
    }



//**********************************************************************************************
//     IMPLEMENTATION METHODS FOR UnionVuFloodSearcher
// Flood search markup for parity rules:
//  Crossing a boundary flips parity.

VuFloodSearcher::VuFloodSearcher ()
  {
  m_graph = NULL;
  m_visitMask = 0;
  }


void VuFloodSearcher::BindGraph (VuSetP pGraph)
    {
    assert (NULL == m_graph);
    assert (0 == m_visitMask);
    m_graph = pGraph;
    m_visitMask = vu_grabMask (m_graph);
    m_stack.clear ();
    vu_clearMaskInSet (m_graph, m_visitMask);
    }
void VuFloodSearcher::Release ()
    {
    vu_returnMask (m_graph, m_visitMask);
    m_graph = NULL;
    m_visitMask = 0;
    }

VuFloodSearcher::~VuFloodSearcher ()    {}

// Step from outside node and depth to its edge mate inside.
// Return new depth
int VuFloodSearcher::StepIntoFace (VuP outside, int outsideDepth, VuP inside)  { return outsideDepth + 1;}
// Mark seed face.
// Return initial depth value.
int VuFloodSearcher::MarkSeedFace (VuP pSeed)  {return 0;}

int VuFloodSearcher::RunFloodFromSeedFace (VuP pSeed)
    {
    if (vu_getMask (pSeed, m_visitMask))
        return 0;
    m_stack.clear ();
    vu_setMaskAroundFace (pSeed, m_visitMask);
    m_stack.push_back (TaggedVuP (pSeed, pSeed, MarkSeedFace (pSeed)));
    // Each stack frame has a seed node and a current node "around a face"
    //      nodeA is the seed.
    //      nodeB is the current node.
    // All stack faces have visit mark and exterior set when stacked.
    // Each pass looks the top of stack:
    //    1) if nodeB neightbor is unvisited, mark up the neighbor and push it on stack.
    //    2) if neighbor is visited, advance nodeB.
    //      2a) If new nodeB matches nodeA, pop.
    //      2b) If not, allow the (modified) frame to be top of stack.
    while (!m_stack.empty ())
        {
        TaggedVuP &frame = m_stack.back ();
        VuP pCurr = frame.m_nodeB;
        VuP pMate = vu_edgeMate (pCurr);
        if (!vu_getMask (pMate, m_visitMask))
            {
            vu_setMaskAroundFace (pMate, m_visitMask);
            m_stack.push_back (TaggedVuP (pMate, pMate, StepIntoFace (pCurr, frame.m_id, pMate)));
            }
        else
            {
            frame.m_nodeB = vu_fsucc (pCurr);
            if (frame.m_nodeA == frame.m_nodeB)
                m_stack.pop_back();
            }
        }
    return 1;
    }




// Find all negative area faces.
// Sort by (signed area)
// Start flood from each, beginning with most negative.
// (Hence sliver faces with epsilon negative area will be flooded from their true exterior and will
//     become interior faces.)
int VuFloodSearcher::RunFloodFromAllNegativeAreaFaces ()
    {
    int numComponent = 0;
    m_seedNodes.clear ();
    vu_collectFacesByAreaSign (m_graph, NULL, &m_seedNodes);
    std::sort (m_seedNodes.begin(), m_seedNodes.end (), cb_is_a_le);
    for (size_t i = 0; i < m_seedNodes.size (); i++)
        numComponent += RunFloodFromSeedFace (m_seedNodes[i].m_nodeA);
    return numComponent;
    }



//**********************************************************************************************
//     IMPLEMENTATION METHODS FOR UnionVuFloodSearcher
// Flood search markup for parity rules:
//  Crossing a boundary flips parity.
void ParityVuFloodSearcher::Bind (VuSetP graph, VuMask boundaryMask, VuMask exteriorMask)
    {
    BindGraph (graph);
    m_boundaryMask = boundaryMask;
    m_exteriorMask = exteriorMask;
    vu_clearMaskInSet (graph, m_exteriorMask);
    }

int ParityVuFloodSearcher::MarkSeedFace (VuP seed)
    {
    vu_setMaskAroundFace (seed, m_exteriorMask);
    return 0;
    }

int ParityVuFloodSearcher::StepIntoFace (VuP outside, int outsideDepth, VuP inside)
    {
    int newDepth = outsideDepth;
    if (vu_getMask (outside, m_boundaryMask))
        newDepth = 1 - newDepth;
    if (0 == (newDepth & 0x01))
        vu_setMaskAroundFace (inside, m_exteriorMask);
    return newDepth;
    }


//**********************************************************************************************
//     IMPLEMENTATION METHODS FOR UnionVuFloodSearcher
// singleExteriorMask -- mask set on the true outside of all individual regions.
// compositeExteriorMask -- mask to be set on the outside of the union.
void UnionVuFloodSearcher::Bind (VuSetP graph, VuMask singleExteriorMask, VuMask compositeExteriorMask,
int minimumWindingForInside
)
    {
    BindGraph (graph);
    m_minimumWindingForInside = minimumWindingForInside;
    m_singleExteriorMask = singleExteriorMask;
    m_compositeExteriorMask = compositeExteriorMask;
    vu_clearMaskInSet (graph, m_compositeExteriorMask);
    }

int UnionVuFloodSearcher::MarkSeedFace (VuP seed)
    {
    vu_setMaskAroundFace (seed, m_compositeExteriorMask);
    return 0;
    }

int UnionVuFloodSearcher::StepIntoFace (VuP outside, int outsideDepth, VuP inside)
    {
    int newDepth = outsideDepth;
    if (vu_getMask (outside, m_singleExteriorMask))
        newDepth++;
    if (vu_getMask (inside, m_singleExteriorMask))
        newDepth--;
    if (newDepth < m_minimumWindingForInside)
        vu_setMaskAroundFace (inside, m_compositeExteriorMask);
    return newDepth;
    }

//**********************************************************************************************
Public GEOMDLLIMPEXP int vu_parityFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask boundaryMask,
VuMask exteriorMask
)
    {
    pGraph->mParityFloodSearcher.Bind (pGraph, boundaryMask, exteriorMask);
    int value = pGraph->mParityFloodSearcher.RunFloodFromAllNegativeAreaFaces ();
    pGraph->mParityFloodSearcher.Release ();
    return value;
    }

Public GEOMDLLIMPEXP int vu_windingFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask singleExteriorMask,
VuMask compositeExteriorMask,
int numberForInterior
)
    {
    pGraph->mUnionFloodSearcher.Bind (pGraph, singleExteriorMask, compositeExteriorMask, numberForInterior);
    int value = pGraph->mUnionFloodSearcher.RunFloodFromAllNegativeAreaFaces ();
    pGraph->mUnionFloodSearcher.Release ();
    return value;
    }



//**********************************************************************************************
//     IMPLEMENTATION METHODS FOR WindingNumberVuFloodSearcher
// singleExteriorMask -- mask set on the true outside of all individual regions.
// m_nodeDepths -- evolving vector of face depths.

// Flood search markup for parity rules:
//  singleExteriorMask on outside increases winding number.
//  single Exteriormask on inside decreases winding number.
// NOTE This class binds to its graph and output array only in its constructor --
//    unlike union and parity classes which can bind and release repeatedly.
struct WindingNumberVuFloodSearcher : VuFloodSearcher
{
VuMask m_singleExteriorMask;
bvector<VuPInt> &m_nodeDepths;
WindingNumberVuFloodSearcher
(
VuSetP graph,
VuMask singleExteriorMask,
bvector<VuPInt> &nodeDepths
)
    : m_nodeDepths (nodeDepths), m_singleExteriorMask (singleExteriorMask)
    {
    BindGraph (graph);
    }
GEOMAPI_VIRTUAL ~WindingNumberVuFloodSearcher (){}

int MarkSeedFace (VuP seed) override;
int StepIntoFace (VuP outside, int outsideDepth, VuP inside) override;
};

int WindingNumberVuFloodSearcher::MarkSeedFace (VuP seed)
    {
    m_nodeDepths.push_back (VuPInt(seed, 0));
    return 0;
    }

int WindingNumberVuFloodSearcher::StepIntoFace (VuP outside, int outsideDepth, VuP inside)
    {
    int newDepth = outsideDepth;
    if (vu_getMask (outside, m_singleExteriorMask))
        newDepth++;
    if (vu_getMask (inside, m_singleExteriorMask))
        newDepth--;
    m_nodeDepths.push_back (VuPInt(inside, newDepth));
    return newDepth;
    }


Public GEOMDLLIMPEXP int vu_windingFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask singleExteriorMask,
bvector<VuPInt> &nodeDepth
)
    {
    WindingNumberVuFloodSearcher searcher (pGraph, singleExteriorMask, nodeDepth);
    nodeDepth.clear ();
    int value = searcher.RunFloodFromAllNegativeAreaFaces ();
    return value;
    }

//**********************************************************************************************
static VuP FSuccThenVPredUntilMaskedMate (VuP seed, VuMask barrierMask, VuMask visitMask)
    {
    VuP vertexSeed = vu_fsucc (seed);
    VU_REVERSE_VERTEX_LOOP (candidate, vertexSeed)
        {
        vu_setMask (candidate, visitMask);
        VuP mate = vu_edgeMate (candidate);
        if (vu_getMask (mate, barrierMask))
            return candidate;
        }
    END_VU_REVERSE_VERTEX_LOOP (candidate, vertexSeed)
    return nullptr;
    }

//**********************************************************************************************
Public void vu_searchMaximalUnmaskedFaces (VuSetP graph, VuMask exteriorMask, bvector<bvector<VuP>> &loops)
    {
    loops.clear ();
    VuMask visitMask = vu_grabMask (graph);
    VuMask outputMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    vu_clearMaskInSet (graph, outputMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMask (seed, visitMask);
            if (   !vu_getMask (seed, exteriorMask)
                &&  vu_getMask (vu_edgeMate (seed), exteriorMask)
                )                
                {
                loops.push_back (bvector<VuP>());
                auto &loop = loops.back ();
                loop.push_back (seed);
                vu_setMask (seed, outputMask);
                VuP walker = seed;
                while (nullptr != (walker = FSuccThenVPredUntilMaskedMate (walker, exteriorMask, visitMask))
                            && !vu_getMask (walker, outputMask)
                    )
                    {
                    vu_setMask (walker, outputMask);
                    loop.push_back (walker);
                    }
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, outputMask);
    vu_returnMask (graph, visitMask);
    }

//**********************************************************************************************
Public void vu_collectMaskedFaces (VuSetP graph, VuMask mask, bool maskCondition, bvector<bvector<VuP>> &loops)
    {
    loops.clear ();
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            if (maskCondition == (0 != vu_getMask (seed, mask)))
                {
                loops.push_back (bvector<VuP>());
                auto &loop = loops.back ();
                VU_FACE_LOOP (vertex, seed)
                    loop.push_back (vertex);
                END_VU_FACE_LOOP (vertex, seed)
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, visitMask);
    }




END_BENTLEY_GEOMETRY_NAMESPACE
