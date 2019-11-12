/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Mtg/MtgApi.h>

#include "MTGLoopDetector.h"

#include "MTGMultiCellularTetrahedralTopology.h"

// (pairing structure for MTGLoopDetector tests)
struct VertexPair
    {
    int m_a, m_b;
    VertexPair (int a, int b) : m_a(a), m_b(b) {}
    static void AddTo (bvector<VertexPair> &edges, int a, int b)
        {
        edges.push_back (VertexPair(a,b));
        }
    static void Append (bvector<VertexPair> &dest, bvector<VertexPair> &source, size_t numAdd)
        {
        for (size_t i = 0; i < source.size () && i < numAdd; i++)
            {
            dest.push_back (source[i]);
            }
        }
    };

// Run the loop alarmer with edge indcies i0, i0+step, etc. (cyclic)
// (If edges has a prime number of entries, this will always reach all of them!!!)
// Return 0 if no loop found.
// Return numEdgesAdded to when loop found.
size_t TestMTGLoopDetector (
MTGLoopDetector &tester,
bvector<VertexPair> &edges,
size_t i0,
size_t step,
bvector<int> &loop,
size_t expectedLoopSize
)
    {
    size_t n = edges.size ();
    tester.Clear ();
    size_t i = i0;
    bvector<int> cycleNodes;
    for (size_t numEdgesAdded = 0;;)
        {
        numEdgesAdded++;
        if (tester.AddEdgeAndTestForCycle (edges[i].m_a, edges[i].m_b, loop))
            return numEdgesAdded;
        i += step;
        i %= n;
        if (i == i0)
            break;
        }
    return 0;
    }

bool IsOrderedCycle (bvector<int> &data, int step)
    {
    int wrap = (int)data.size ();
    for (size_t i = 0; i < data.size (); i++)
        {
        int i0 = data[i];
        int i1 = data[(i+1) % wrap];
        int j1 = i0 + step;
        if (j1 < 0)
            j1 += wrap;
        else if (j1 == wrap)
            j1 = 0;
        if (i1 != j1)
            return false;
        }
    return true;
    }
// Test GabrielLoops for numEdgesInLoop edges in the loop plus various branches.
// Loop count should be at least 3 and less than 99
void TestLoopDetector (uint32_t numEdgesInLoop)
    {
    MTGLoopDetector tester;
    bvector<VertexPair> loopEdges;
    // Loop is a trivial loop ...
    bvector<int>loop;
    for (uint32_t i = 0; i < numEdgesInLoop; i++)
        {
        VertexPair::AddTo (loopEdges, i, (i + 1) % numEdgesInLoop);
        }

    // other edges has additional edges.
    // There are no loops with these edges.
    // No component of this graph has more than one vertex numbered below 100
    bvector<VertexPair>otherEdges;
    VertexPair::AddTo (otherEdges, 0,103);
    VertexPair::AddTo (otherEdges, 2,104);
    VertexPair::AddTo (otherEdges, 104,108);
    VertexPair::AddTo (otherEdges, 105,109);
    VertexPair::AddTo (otherEdges, 108, 110);
    VertexPair::AddTo (otherEdges, 108, 112);
    VertexPair::AddTo (otherEdges, 115, 112);
    VertexPair::AddTo (otherEdges, 115, 117);
    VertexPair::AddTo (otherEdges, 117, 119);
    VertexPair::AddTo (otherEdges, 119, 120);
    VertexPair::AddTo (otherEdges, 119, 121);
    VertexPair::AddTo (otherEdges, 121, 122);

    // TestMTGLoopDetector requires a prime number of edges in its graph.
    // (which enables systematic order shuffling in the tests)
    bvector<size_t> primes;
    primes.push_back ( 3);
    primes.push_back ( 5);
    primes.push_back ( 7);
    primes.push_back (11);
    primes.push_back (13);
    primes.push_back (17);
    primes.push_back (19);
    primes.push_back (23);
    primes.push_back (31);
    primes.push_back (37);

    for (size_t totalNumEdges : primes)
        {
        if (totalNumEdges < numEdgesInLoop)
            continue;
        if (numEdgesInLoop + otherEdges.size () < totalNumEdges)
            break;
        // Start with the loop.
        // Add distraction edges up to a prime . . .
        bvector<VertexPair> allEdges = loopEdges;
        VertexPair::Append (allEdges, otherEdges,
                totalNumEdges - numEdgesInLoop
                );

        for (size_t i0 = 0; i0 < allEdges.size (); i0++)
            {
            for (size_t step = 1; step < allEdges.size (); step++)
                {
                size_t numAdded = TestMTGLoopDetector (tester, allEdges, i0, step, loop, numEdgesInLoop);
                Check::True (numAdded > 0, "Closure is expected");
                Check::True ((ptrdiff_t)numAdded >= (ptrdiff_t)numEdgesInLoop, "Premature closure");
                Check::Size ((size_t)numEdgesInLoop, loop.size (), "loop count");
                Check::True (
                    IsOrderedCycle (loop, 1) || IsOrderedCycle (loop, -1), "VertexIndex loop");
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,LoopDetector)
    {
    TestLoopDetector (3);
    TestLoopDetector (4);
    TestLoopDetector (5);
    TestLoopDetector (6);
    }

  
void CheckInteriorTetrahedralGraph (MTGMultiCellularTetrahedralTopology &graph,
      size_t expectedComponents,
      size_t expectedBoundaryFaces
      )
    {
    bvector<MTGNodeId> faceSeeds, vertexSeeds;    
    bvector<bvector<MTGNodeId>> componentNodes;
    graph.CollectFaceLoops (faceSeeds);
    graph.CollectVertexLoops (vertexSeeds);
    graph.CollectConnectedComponents (componentNodes);
    size_t numErrors = 0;
    size_t numExterior = 0;
    for (MTGNodeId faceSeed : faceSeeds)
        {
        if (graph.CountNodesAroundFace (faceSeed) != 3)
            numErrors++;
        // Confirm:  If there is a face mate, it points right back.
        MTGNodeId mate = graph.FaceMate (faceSeed);
        if (MTG_NULL_NODEID == mate)
            numExterior++;
        else
            {
            if (graph.FaceMate (mate) != faceSeed)
                numErrors++;
            }
        }
    
    for (MTGNodeId vertexSeed : vertexSeeds)
        {
        if (graph.CountNodesAroundVertex (vertexSeed) != 3)
            numErrors++;
        }

    for (size_t i = 0; i < componentNodes.size (); i++)
        if (componentNodes[i].size () != 12)
            numErrors++;
    if (expectedComponents != componentNodes.size ())
        numErrors++;
    if (expectedBoundaryFaces > 0)
        Check::Size (expectedBoundaryFaces, numExterior, "Tetrahedral Graph bounary faces");

    MTGMask vSuccMask = graph.GrabMask ();
    MTGMask vPredMask = graph.GrabMask ();
    graph.ClearMask (vSuccMask);
    graph.ClearMask (vPredMask);
    UsageSums chainLengths;

    // Here "boundary" means a node "on the inside" but just adjacent to exterior.
    // build up pairs that are "mates" in the extended sense of sharing a true exterior edge, but reachable only
    // by jumping through an interior chain
    bvector<MTGNodeIdPair> boundaryPairs;

    MTGARRAY_SET_LOOP (boundaryA, &graph)
        {
        if (MTG_NULL_NODEID == graph.FaceMate (boundaryA) && !graph.GetMaskAt (boundaryA, vSuccMask))
            {
            // A,B,C,D all have no face mate.
            // boundaryA,boundaryC are "mates" for A.VStar.F.VStar.FLoop -- where VStar means multiple VSuccFaceMate
            // steps, i.e. ignore boundary distractions.
            MTGNodeId terminalNodeC, terminalNodeA, boundaryB, boundaryC, boundaryD;
            size_t chainLengthA, chainLengthC;
            graph.MarkVSuccFaceMateChain (boundaryA, vSuccMask, boundaryB, terminalNodeA, chainLengthA);
            boundaryC = graph.FSucc (boundaryB);
            graph.MarkVSuccFaceMateChain (boundaryC, vSuccMask, boundaryD, terminalNodeC, chainLengthC);
            chainLengths.Accumulate (chainLengthA);
            if (chainLengthA != chainLengthC)
                numErrors++;
            MTGNodeId boundaryE = graph.FSucc (boundaryD);
            if (boundaryE != boundaryA)
                numErrors++;
            if (terminalNodeA != MTG_NULL_NODEID || terminalNodeC != MTG_NULL_NODEID)
                numErrors++;
            boundaryPairs.push_back (MTGNodeIdPair (boundaryA, boundaryC));
            }
        }
    MTGARRAY_END_SET_LOOP (boundaryA, &graph)
    graph.DropMask (vPredMask);
    graph.DropMask (vSuccMask);
    Check::Size (expectedBoundaryFaces * 3, boundaryPairs.size () * 2, "boundary pair count");

    Check::Size (0, numErrors, "Tetrahedral graph topology");
    return;
    }

size_t CountMask (
MTGGraph &graph,
bvector<MTGNodeId> &nodes,
MTGMask mask
)
    {
    size_t n = 0;
    for (MTGNodeId node : nodes)
        if (graph.GetMaskAt (node, mask))
            n++;
    return n;
    }

//! Accumulate counts of true/false values passed to Count ()
struct SizeCounter
{
public:
size_t m_numInvalid;
bvector<size_t> m_counters;


SizeCounter (size_t maxExpectedValue)
    {
    m_numInvalid = 0;
    m_counters.resize (maxExpectedValue + 1, 0);
    }

void Report (size_t value)
    {
    if (value < m_counters.size ())
        m_counters[value]++;
    else
        m_numInvalid++;
    }
};

void CheckCompleteMulticellularGraph
(
MTGMultiCellularTetrahedralTopology &graph,
size_t numExpectedInterior,
size_t numExpectedExterior,
bool noisy = false
)
    {
    bvector<bvector<MTGNodeId>> componentNodes;
    graph.CollectConnectedComponents (componentNodes);
    size_t numExterior = 0;
    size_t numInterior = 0;
    size_t numMixed = 0;
    // CONFIRM: Every face is full exterior or fully interior.
    for (size_t i = 0; i < componentNodes.size (); i++)
        {
        size_t n = CountMask (graph, componentNodes[i], MTG_EXTERIOR_MASK);
        if (n == 0)
            numInterior++;
        else if (n == componentNodes[i].size ())
            numExterior++;
        else
            numMixed++;
        }
    if (numExpectedInterior > 0)
        Check::Size (numExpectedInterior, numInterior, "Interior components");
    if (numExpectedExterior > 0)
        Check::Size (numExpectedExterior, numExterior, "Interior components");
    Check::Size (0, numMixed, "Mixed components");

    // Count interior and exterior faces
    SizeCounter exteriorVertexCounts (100);
    bvector<MTGNodeId> faceSeed, vertexSeed;
    graph.CollectFaceLoops (faceSeed);
    graph.CollectVertexLoops (vertexSeed);
    size_t numExteriorFaces = 0;
    size_t numInteriorFaces = 0;
    size_t numExteriorVertices = 0;
    size_t numInteriorVertices = 0;
    bmap<size_t, size_t> vertexCounters;
    for (MTGNodeId node : vertexSeed)
        {
        if (graph.GetMaskAt (node, MTG_EXTERIOR_MASK))
            {
            numExteriorVertices++;
            exteriorVertexCounts.Report (graph.CountNodesAroundVertex (node));
            }
        else
            numInteriorVertices++;
        }

    for (MTGNodeId node : faceSeed)
        {
        if (graph.GetMaskAt (node, MTG_EXTERIOR_MASK))
            {
            numExteriorFaces++;
            }
        else
            numInteriorFaces++;
        }

// CONFIRM: Every node has a vertex index.
    size_t numErrors = 0; 

    MTGARRAY_SET_LOOP (node, &graph)
        {
        int vertexIndex = -1;
        graph.TryGetVertexIndexAtNode (node, vertexIndex);
        if (vertexIndex < 0)
            numErrors++;
        }
    MTGARRAY_END_SET_LOOP (node, &graph)

// CONFIRM: Every interior face has adjacent tetrahedra
    bvector<MTGNodeId> nodePerFace;
    graph.CollectDoubleSidedFaces (nodePerFace, false);
    for (MTGNodeId seed : nodePerFace)
        {
        int vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD1, vertexIndexD2;
        bool isInterior = !graph.IsExteriorNodeOrFaceMate (seed);
        bool hasTetrahedra = graph.GetAdjacentTetrahedraVertexIndices (seed,
                  vertexIndexA, vertexIndexB, vertexIndexC,
                  vertexIndexD1, vertexIndexD2, true);
        if (isInterior != hasTetrahedra)
            numErrors++;
        }




    Check::Size (0, numErrors, "Tetrahedral Queries");

    if (noisy)
        {
        printf (" Mutlicellular complex (totalManifoldFaces (multicount) %d) (totalManifoldVertexLoops (multicount) %d)\n",
                    (int)faceSeed.size (), (int)vertexSeed.size ());
        printf (" (exteriorFaces (multicount) %d) (interiorFaces (multicount) %d)\n",
                    (int)numExteriorFaces, (int)numInteriorFaces);
        printf (" (exteriorVertexLoops %d) (interiorFaces %d)\n", (int)numExteriorVertices, (int)numInteriorFaces);
        printf (" (exteriorVertexLoopsMonsterCount %d)\n", (int)exteriorVertexCounts.m_numInvalid);
        for (size_t i = 0; i < exteriorVertexCounts.m_counters.size (); i++)
            {
            size_t n = exteriorVertexCounts.m_counters[i];
            if (n != 0)
                printf ("    (edgesAtExteriorVertex %d  numVertex %d)\n", (int)i, (int)n);
            }
        }
    }

bool IsTriangulated (MTGMultiCellularTetrahedralTopology &graph)
    {
    // This counts each face once per node.
    //  Question: would it be faster to mark so the face is only counted once?
    //   Probably not -- that would have
    //     1) ClearMaskInSet -- graph visit control + mask clear
    //     2) graph visit control + mask check for seed
    //         2a) at each seed, one visit for loop count, 1 for mask set.
    // total 5 touches.
    // 
    // If triangulated, there are 4 touches -- once in graph visit control, 3 times in loop counts.

    MTGARRAY_SET_LOOP (node, &graph)
        {
        if (!graph.Is3NodeFaceLoop (node))
            return false;
        }
    MTGARRAY_END_SET_LOOP (node, &graph)
    return true;
    }

bool IsTrivalentInterior (MTGMultiCellularTetrahedralTopology &graph)
    {
    // This counts each face once per node.
    //  Question: would it be faster to mark so the face is only counted once?
    //   Probably not -- that would have
    //     1) ClearMaskInSet -- graph visit control + mask clear
    //     2) graph visit control + mask check for seed
    //         2a) at each seed, one visit for loop count, 1 for mask set.
    // total 5 touches.
    // 
    // If triangulated, there are 4 touches -- once in graph visit control, 3 times in loop counts.

    MTGARRAY_SET_LOOP (node, &graph)
        {
        if (!graph.IsExteriorNode (node))
            if (!graph.Is3NodeVertexLoop (node))
              return false;
        }
    MTGARRAY_END_SET_LOOP (node, &graph)
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,TetrahedralAssembler)
    {
    MTGMultiCellularTetrahedralTopology graph;
    graph.AddTetrahedron (0,1,2,3);
    CheckInteriorTetrahedralGraph (graph, 1, 4);
    graph.AddTetrahedron (2,1,0,4);
    CheckInteriorTetrahedralGraph (graph, 2, 6);
    graph.AddTetrahedron (1,3,0,5);
    CheckInteriorTetrahedralGraph (graph, 3, 8);
    Check::True (graph.WrapExteriorGraphAroundCompleteInteriorGraph (), "Assemble exterior");
    CheckCompleteMulticellularGraph (graph, 3, 1);
    Check::True (IsTriangulated (graph), "Triangulated graph");
    Check::True (IsTrivalentInterior (graph), "Trivalent interior");
    }
#if defined (_WIN32) && !defined(BENTLEY_WINRT)
#define TestTetrahedralFiles
#ifdef TestTetrahedralFiles
void TestTetrahedralFile (char const *filename)
    {
    MTGMultiCellularTetrahedralTopology graph;
    FILE* m_fp = fopen(filename, "r");
    if (nullptr != m_fp)
        {
        printf ("\n\n Tetrahedral solid from file %s\n", filename);
        int vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD;
        size_t numTet = 0;
        for (;4 == fscanf (m_fp, "%d %d %d %d", &vertexIndexA, &vertexIndexB, &vertexIndexC, &vertexIndexD);)
            {
            numTet++;
            graph.AddTetrahedron (vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD);
            }
        printf ("  (numTetrahedra %d)\n", (int)numTet);
        Check::True (graph.WrapExteriorGraphAroundCompleteInteriorGraph (), "Assemble exterior");
        CheckCompleteMulticellularGraph (graph, numTet, 0); // we know how many tets, not how many exterior.
        Check::True (IsTriangulated (graph), "Triangulated graph");


        }
    fclose (m_fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,TetrahedralAssemblerByFile)
    {
    static bool s_doTest0 = false;
    static bool s_doTest1 = false;
    if (s_doTest0)
        TestTetrahedralFile ("d:/tmp/tetFile0.tet");
    if (s_doTest1)
        TestTetrahedralFile ("d:/mskfiles/mesh/2015/03B Bois Tetrahedra/march10/Tetrahedrons.index");
    }
#endif

#endif
