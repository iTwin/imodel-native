/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../polyface/CoordinateMaps.h"
#include <Mtg/MtgApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

template <typename T1, typename T2>
struct LexicalPair {
T1 m_i0;
T2 m_i1;
LexicalPair (T1 i0, T2 i1)
    {
    m_i0 = i0;
    m_i1 = i1;
    }

bool operator < (LexicalPair const &other) const
    {
    if (m_i0 < other.m_i0)
        return true;
    if (other.m_i0 < m_i0)
        return false;
    return m_i1 < other.m_i1;
    }

void Get (T1 &i0, T2& i1)
    {
    i0 = m_i0;
    i1 = m_i1;
    }
};

//! Array of pairs (i0,i1) with sort/search for gather on clusters of same i0
template <typename T1, typename T2>
struct IndexClusterArray : bvector <LexicalPair<T1, T2> >
{
void Add (T1 i0, T2 i1)
    {
    push_back (LexicalPair <T1, T2>(i0, i1));
    }

void Sort ()
    {
    std::sort (this->begin (), this->end ());
    }

bool TryGetAt (size_t index, T1 &i0, T2 &i1)
    {
    if (index < this->size ())
        {
        this->at(index).Get (i0, i1);
        return true;
        }
    return false;
    }

//! return upper index of block {indexA <= i < upperIndex} with matching i0 member
size_t BlockLimit (size_t indexA)
    {
    size_t n = this->size ();
    size_t indexB = indexA;
    while (indexB < n && this->at(indexA).m_i0 == this->at(indexB).m_i0)
        indexB++;
    return indexB;
    }
};

struct EdgeOrderingState
{
size_t m_candidateIndex;
size_t m_endIndexA;
size_t m_endIndexB;
double m_d2Min;
EdgeOrderingState () : m_d2Min(DBL_MAX), m_endIndexA(0), m_endIndexB(1), m_candidateIndex (0) {}

void Update (DPoint3dCR xyzA, size_t indexA, DPoint3dCR xyzB, size_t indexB, size_t candidateIndex)
    {
    double d2 = xyzA.DistanceSquared (xyzB);
    if (d2 < m_d2Min)
        {
        m_endIndexA = indexA;
        m_endIndexB = indexB;
        m_candidateIndex = candidateIndex;
        m_d2Min = d2;
        }
    }
double GetDMin (){return sqrt (m_d2Min);}
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurveVector::ReorderForSmallGaps ()
    {
    CurveVector::BoundaryType bType = GetBoundaryType ();
    double maxGap = 0.0;

    if (bType == BOUNDARY_TYPE_None
      || bType == BOUNDARY_TYPE_Open
      || bType == BOUNDARY_TYPE_Outer
      || bType == BOUNDARY_TYPE_Inner
      )
        {
        DPoint3d xyz0A, xyz1A, xyzA;
        DPoint3d xyz0B, xyz1B;
        size_t n = size ();
        // ok, this is n^2 time, possibly lots of swapping.  That's tough.
        // 0..numChained-1 is a block of (hopefully) chained curves.
        // look at i0.. for one close to one end of the iA block.
        for (size_t numChained = 1; numChained  < n; numChained++)
            {
            at(0)->GetStartEnd (xyz0A, xyzA);
            at(numChained-1)->GetStartEnd (xyzA, xyz1A);
            EdgeOrderingState searchState;
            for (size_t candidateIndex = numChained; candidateIndex < n; candidateIndex++)
                {
                at(candidateIndex)->GetStartEnd (xyz0B, xyz1B);
                searchState.Update (xyz0A, 0, xyz0B, 0, candidateIndex);
                searchState.Update (xyz0A, 0, xyz1B, 1, candidateIndex);
                searchState.Update (xyz1A, 1, xyz0B, 0, candidateIndex);
                searchState.Update (xyz1A, 1, xyz1B, 1, candidateIndex);
                }
            size_t acceptedCandidateIndex = searchState.m_candidateIndex;
            // swap candiate so it is just after the chain ..
            if (acceptedCandidateIndex > numChained)
                std::swap (at(acceptedCandidateIndex), at(numChained));
            if (searchState.m_endIndexA == 0)
                {
                // move candidate to start of chain ...
                for (size_t i = numChained; i > 0; i--)
                    std::swap (at(i), at(i-1));
                if (searchState.m_endIndexB == 0)
                    at(0)->ReverseCurvesInPlace ();
                }
            else
                {
                if (searchState.m_endIndexB == 1)
                    at(numChained)->ReverseCurvesInPlace ();
                }
            maxGap = DoubleOps::Max (maxGap, searchState.GetDMin ());
            }
        if (bType == BOUNDARY_TYPE_Outer || bType == BOUNDARY_TYPE_Inner)
            {
            if (front()->GetStartEnd (xyz0A, xyz1A) && back()->GetStartEnd (xyz0B, xyz1B))
                {
                maxGap = DoubleOps::Max (maxGap, xyz1B.Distance (xyz0A));
                }
            }
        }
    else
        {
        for (size_t i = 0; i < size (); i++)
            {
            CurveVectorPtr child = at(i)->GetChildCurveVectorP ();
            if (child.IsValid ())
                maxGap = DoubleOps::Max (maxGap, child->ReorderForSmallGaps ());
            }
        }
    return maxGap;
    }


#define MY_XYZ_LABEL_TAG -100
#define MY_EDGE_LABEL_TAG -101
#define MY_LABEL_DEFAULT_VALUE -2
#define MY_START_MASK MTG_DIRECTED_EDGE_MASK
struct ChainAssembler : MTGGraph
{
int m_vertexLabel;
int m_edgeLabel;

// The map "second" indexes back to the (parallel) m_xyz and m_nodeID arrays.
PolyfaceZYXMap m_map;
bvector<DPoint3d> m_xyz;
bvector<MTGNodeId> m_nodeId;

void PrintMinDistances ()
    {
    bvector <size_t> neighbor;
    printf ("\n MIN DISTANCES\n");
    for (size_t i = 0; i < m_xyz.size (); i++)
        {
        double d = DBL_MAX;
        size_t k = i;
        for (size_t j = 0; j < m_xyz.size (); j++)
            {
            if (i != j)
                {
                double d1 = m_xyz[i].Distance (m_xyz[j]);
                if (d1 < d)
                    {
                    k = j;
                    d = d1;
                    }
                }
            }
        printf ("(%d %d (xyz %g.3, %.3g, %.3g)   (d %.2le)\n", (int)i, (int)k, m_xyz[i].x, m_xyz[i].y, m_xyz[i].z, d);
        neighbor.push_back (k);
        }

    for (size_t i = 0; i < m_xyz.size (); i++)
        {
        size_t i1 = neighbor[i];
        size_t i2 = neighbor[i1];
        if (i2 == i)
            printf (" (pair %d %d)\n", (int)i, (int)i1);
        else
            printf (" (cluster %d %d %d)\n", (int)i, (int)i1, (int)i2);
        }
    }
ChainAssembler (double xyzTol)
  : m_map (DPoint3dZYXTolerancedSortComparison (xyzTol, 0.0))
    {
    m_vertexLabel = DefineLabel (MY_XYZ_LABEL_TAG, MTG_LabelMask_VertexProperty, -1);
    m_edgeLabel = DefineLabel (MY_EDGE_LABEL_TAG, MTG_LabelMask_EdgeProperty, -1);
    }

size_t FindOrAddPoint (DPoint3dCR xyz)
    {
    PolyfaceZYXMap::iterator key = m_map.find (xyz);
    if (key != m_map.end ())
        return key->second;
   size_t index = m_xyz.size ();
    m_xyz.push_back (xyz);
    m_nodeId.push_back (MTG_NULL_NODEID);
    m_map[xyz] = index;
    return index;
    }

void AddEdge (size_t edgeIndex, DPoint3dCR xyzA, DPoint3dCR xyzB)
  {
  size_t indexA = FindOrAddPoint (xyzA);
  size_t indexB = FindOrAddPoint (xyzB);

  MTGNodeId nodeIdA, nodeIdB;
  CreateEdge (nodeIdA, nodeIdB);

  if (m_nodeId[indexA] == MTG_NULL_NODEID)
      m_nodeId[indexA] = nodeIdA;
  else
    VertexTwist (nodeIdA, m_nodeId[indexA]);
  if (m_nodeId[indexB] == MTG_NULL_NODEID)
      m_nodeId[indexB] = nodeIdB;
  else
    VertexTwist (nodeIdB, m_nodeId[indexB]);
  TrySetLabel (nodeIdA, m_vertexLabel, (int)indexA);
  TrySetLabel (nodeIdB, m_vertexLabel, (int)indexB);
  TrySetLabel (nodeIdA, m_edgeLabel, (int)edgeIndex);
  TrySetLabel (nodeIdB, m_edgeLabel, (int)edgeIndex);
  SetMaskAt (nodeIdA, MY_START_MASK);
  }

bool TryGetEdgeIndexAndDirection (MTGNodeId nodeId, size_t &index, bool &atStart)
    {
    if (IsValidNodeId (nodeId))
        {
        int label;
        TryGetLabel (nodeId, m_edgeLabel, label);
        index = (size_t)label;
        atStart = 0 != GetMaskAt (nodeId, MY_START_MASK);
        return true;
        }
    return false;        
    }
};

// derference from nodeId to edge index and direction.
// clone the edge or its reverse.
static void ExtendChain (MTGNodeId nodeId, ChainAssembler &assembler, CurveVectorCR primitives, CurveVectorR chain)
    {
    size_t index;
    bool   atStart;
    if (assembler.TryGetEdgeIndexAndDirection (nodeId, index, atStart)
        && index < primitives.size ()
        )
        {
        if (atStart)
            chain.push_back (primitives[index]->Clone ());
        else
            chain.push_back (primitives[index]->CloneBetweenFractions (1.0, 0.0, false));
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::AssembleChains ()
    {
    static double s_relTol = 1.0e-8;
    static int s_print = 0;
    double totalLength = Length ();
    double absTol = s_relTol * totalLength;
    ChainAssembler assembler (absTol);
    CurveVectorPtr allChains = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr allPrimitives = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    allPrimitives->AddPrimitives (*this);
    // Load the primitives with start end.
    // A lot happens (quietly!!!) during each insert ...
    for (size_t i = 0; i < allPrimitives->size (); i++)
        {
        ICurvePrimitiveCP primitive = allPrimitives->at(i).get ();
        DPoint3d xyzA, xyzB;
        if (primitive->GetStartEnd (xyzA, xyzB))
            {
            assembler.AddEdge (i, xyzA, xyzB);
            }
        }

    if (s_print)
        {
        jmdlMTGGraph_printFaceLoops (&assembler);
        jmdlMTGraph_printVertexLoops (&assembler);
        assembler.PrintMinDistances ();
        }

    bvector<MTGNodeId> allSeeds;
    assembler.CollectVertexLoops (allSeeds);
    MTGMask visitMask = assembler.GrabMask ();
    assembler.ClearMask (visitMask);
    // Pull out any chain starting and ending at a vertex with OTHER THAN 2 edges.
    for (size_t i = 0; i < allSeeds.size (); i++)
        {
        MTGNodeId vertexSeedNodeId = allSeeds[i];
        if (!assembler.GetMaskAt (vertexSeedNodeId, visitMask)
            && 2 != assembler.CountNodesAroundVertex (vertexSeedNodeId))
            {
            MTGARRAY_FACE_LOOP (chainSeedNodeId, &assembler, vertexSeedNodeId)
                {
                if (!assembler.GetMaskAt (chainSeedNodeId, visitMask))
                    {
                    MTGNodeId currNodeId = chainSeedNodeId;
                    CurveVectorPtr chain = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
                    do
                        {
                        assembler.SetMaskAroundEdge (currNodeId, visitMask);
                        ExtendChain (currNodeId, assembler, *allPrimitives, *chain);
                        currNodeId = assembler.FSucc (currNodeId);
                        } while (assembler.CountNodesAroundVertex (currNodeId) == 2
                            &&  !assembler.GetMaskAt (currNodeId, visitMask)    // this should never happen, but .....
                            );
                    allChains->push_back (ICurvePrimitive::CreateChildCurveVector (chain));
                    }
                }
            MTGARRAY_END_FACE_LOOP (chainSeedNodeId, &assembler, vertexSeedNodeId)
            }
        }

    // Everything left is in a loop.
    for (size_t i = 0; i < allSeeds.size (); i++)
        {
        MTGNodeId seedNodeId = allSeeds[i];
        if (!assembler.GetMaskAt (seedNodeId, visitMask))
            {
            CurveVectorPtr loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            MTGARRAY_FACE_LOOP (currNodeId, &assembler, seedNodeId)
                {
                assembler.SetMaskAroundEdge (currNodeId, visitMask); 
                ExtendChain (currNodeId, assembler, *allPrimitives, *loop);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, &assembler, seedNodeId)
            allChains->push_back (ICurvePrimitive::CreateChildCurveVector (loop));
            }
        }

    assembler.DropMask (visitMask);
    return allChains;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
