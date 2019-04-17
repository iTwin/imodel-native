/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Mtg/MtgApi.h>

void CheckCounts (MTGGraph &graph, size_t numVertex, size_t numFace, size_t numComponent)
    {
    bvector<MTGNodeId> faces;
    bvector<MTGNodeId> vertices;
    bvector<bvector <MTGNodeId> >components;
    graph.CollectFaceLoops (faces);
    graph.CollectVertexLoops (vertices);
    graph.CollectConnectedComponents (components);
    Check::Size (numVertex, vertices.size (), "vertex count");
    Check::Size (numFace, faces.size (), "face count");
    Check::Size (numComponent, components.size (), "face count");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG, CreateEdge)
    {
    MTGGraph graph;
    MTGNodeId nodeA, nodeB;
    graph.CreateEdge (nodeA, nodeB);
    Check::False (nodeA == nodeB, "CreateEdge::Distinct nodes");
    Check::Int (nodeB, graph.FSucc (nodeA), "(A--B):B = A.FSucc");
    Check::Int (nodeA, graph.FSucc (nodeB), "(A--B):A = B.FSucc");

    Check::Int (nodeB, graph.FPred (nodeA), "(A--B):B = A.FPred");
    Check::Int (nodeA, graph.FPred (nodeB), "(A--B):A = B.FPred");

    Check::Int (nodeA, graph.VSucc (nodeA), "(A--B):A = A.VSucc");
    Check::Int (nodeB, graph.VSucc (nodeB), "(A--B):B = B.VSucc");

    Check::Int (nodeA, graph.VPred (nodeA), "(A--B):A = A.VPred");
    Check::Int (nodeB, graph.VPred (nodeB), "(A--B):B = B.VPred");

    Check::Int (nodeB, graph.EdgeMate (nodeA), "(A--B):B = A.EdgeMate");
    Check::Int (nodeA, graph.EdgeMate (nodeB), "(A--B):A = B.EdgeMate");

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG, CreateSling)
    {
    MTGGraph graph;
    MTGNodeId nodeA, nodeB;
    graph.CreateSling (nodeA, nodeB);
    Check::False (nodeA == nodeB, "CreateEdge::Distinct nodes");

    Check::Int (nodeA, graph.FSucc (nodeA), "(AB):A = A.FSucc");
    Check::Int (nodeB, graph.FSucc (nodeB), "(AB):B = B.FSucc");

    Check::Int (nodeA, graph.FPred (nodeA), "(AB):A = A.FPred");
    Check::Int (nodeB, graph.FPred (nodeB), "(AB):A = B.FPred");

    Check::Int (nodeB, graph.VSucc (nodeA), "(AB):B = A.VSucc");
    Check::Int (nodeA, graph.VSucc (nodeB), "(AB):A = B.VSucc");

    Check::Int (nodeB, graph.VPred (nodeA), "(AB):B = A.VPred");
    Check::Int (nodeA, graph.VPred (nodeB), "(AB):A = B.VPred");

    Check::Int (nodeB, graph.EdgeMate (nodeA), "(AB):B = A.EdgeMate");
    Check::Int (nodeA, graph.EdgeMate (nodeB), "(AB):A = B.EdgeMate");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG, Masks)
    {
    MTGGraph graph;
    MTGNodeId nodeA, nodeB;
    MTGNodeId nodeC, nodeD;
    int numSplit = 3;

    Check::True (MTG_NULL_NODEID == graph.AnyValidNodeId (), "NewGraph, AnyValidNodeId");

    graph.SplitEdge (nodeA, nodeB, MTG_NULL_NODEID);
    MTGNodeId nodeQ = graph.AnyValidNodeId ();
    Check::True (nodeQ == nodeA || nodeQ == nodeB, "AnyValidNodeId");

    for (int i = 0; i < numSplit; i++)
        {
        graph.SplitEdge (nodeC, nodeD, nodeA);
        Check::Size (2, graph.CountNodesAroundVertex (nodeC), "edgeSplit.Vsize==2");
        Check::Size (2 + i, graph.CountNodesAroundFace (nodeA), "edgeSplit.Fsize");
        }


    graph.SplitEdge (nodeC, nodeD, MTG_NULL_NODEID);
    CheckCounts (graph, 2 + numSplit, 4, 2);

    graph.VertexTwist (nodeC, nodeA);
    CheckCounts (graph, 1 + numSplit, 3, 1);

    Check::Size (2 * (2 + numSplit), graph.GetNodeIdCount (), "node count");
    Check::Size (2 * (2 + numSplit), graph.GetActiveNodeCount (), "node count");
    
    size_t numNode = graph.GetActiveNodeCount ();
    MTGMask myMask = MTG_PRIMARY_EDGE_MASK;
    graph.SetMask (myMask);
    Check::Size (numNode, graph.CountMask (myMask), "SetMask");
    graph.ClearMask (myMask);
    Check::Size (0, graph.CountMask (myMask), "SetMask");

    graph.SetMaskAroundFace (nodeB, myMask);
    Check::Size (graph.CountNodesAroundFace (nodeB),
                 graph.CountMask (myMask), "mask on single face");
    Check::Size (1, graph.CountMaskAroundVertex (nodeB, myMask),
                        "face mask counted around vertex");
    graph.ClearMaskAroundFace (nodeB, myMask);
    Check::Size (0, graph.CountMask (myMask), "ClearMaskAroundFace");



    graph.SetMaskAroundVertex (nodeB, myMask);
    Check::Size (graph.CountNodesAroundVertex (nodeB),
                 graph.CountMask (myMask), "mask on single vertex");
    Check::Size (1, graph.CountMaskAroundFace (nodeB, myMask),
                        "vertex mask counted around face");
    graph.ClearMaskAroundVertex (nodeB, myMask);
    Check::Size (0, graph.CountMask (myMask), "ClearMaskAroundVertex");

    MTGNodeId nodeE = graph.FSucc (nodeB);
    if (Check::Size (2, graph.CountNodesAroundVertex (nodeE), "Expect healable"))
        {
        size_t v0 = graph.CountVertexLoops ();
        size_t f0 = graph.CountFaceLoops ();
        size_t num0 = graph.GetActiveNodeCount ();
        Check::True (graph.HealEdge (nodeE), "HealEdge");
        size_t num1 = graph.GetActiveNodeCount ();
        Check::Size (num1 + 2, num0, "NodeCount after EdgeHeal");
        Check::Size (f0, graph.CountFaceLoops (), "EdgeHeal,CountFaceLoops");
        Check::Size (v0, 1 + graph.CountFaceLoops (), "EdgeHeal,CountVertexLoops");
        }
    }


void CreateGrid
(
MTGGraph &graph,
size_t numXEdge,
size_t numYEdge,
bvector< bvector <MTGNodeId> > & nodes
)
    {
    if (numXEdge < 1)
        numXEdge = 1;
    if (numYEdge < 1)
        numYEdge = 1;
    nodes.clear ();
    MTGNodeId nodeIdA0, nodeIdA1;   
    MTGNodeId nodeIdC, nodeIdD;
    graph.CreateEdge (nodeIdA0, nodeIdA1);
    if (numXEdge > 1)
        jmdlMTGGraph_multipleSplit (&graph, nodeIdA0, (int)numXEdge - 1, 0, 0);

    // nodeIdA0 is at left of base.
    // Build an edge upwards from nodeIdA0.  This is the first right vertical edge
    // At each step:
    //   Join from the rightmost vertical to the next base edge.
    //   Split the joiner -- this leaves a quad behind.
    //   Advance the join point on the base.

    // Bare upper edge
    //       E
    //      *---------*-------*-----*
    // Build initial strut
    //      *
    //      |D,F
    //      |
    //     C|  E
    //      *---------*-------*-----*
    // Recurring start of inside loop -- E is along base horizontal, F is top of exposed vertical.
    //      *---------*-------*
    //      |D        |       |F
    //      |         |       |
    //     C|         |       | E
    //      *---------*-------*-----*-----*


    MTGNodeId diagonalTop, diagonalBottom, nodeIdG;
    MTGNodeId nodeIdE = nodeIdA0;
    for (size_t i = 0; i < numYEdge; i++)
        {
        jmdlMTGGraph_join (&graph, &nodeIdC, &nodeIdD, nodeIdE, MTG_NULL_NODEID, 0, 0);
        MTGNodeId nodeIdF = nodeIdD;
        nodes.push_back (bvector<MTGNodeId> ());
        for (size_t i = 0; i < numXEdge; i++)
            {
            nodes.back ().push_back (nodeIdE);
            nodeIdE = graph.FSucc (nodeIdE);
            jmdlMTGGraph_join (&graph, &diagonalTop, &diagonalBottom, nodeIdF, nodeIdE, 0, 0);
            jmdlMTGGraph_splitEdge (&graph, &nodeIdF, &nodeIdG, diagonalTop);
            }
        nodeIdE = graph.VSucc  (nodeIdD);  // E moves to new outside top.
        }
    }

void CheckGrid (size_t numX, size_t numY)
    {
    MTGGraphP graph = new MTGGraph ();
    bvector<bvector<MTGNodeId> > nodes;
    CreateGrid (*graph, numX, numY, nodes);
    bvector<bvector<MTGNodeId> > originalFSucc = nodes;
    bvector<bvector<MTGNodeId> > originalVSucc = nodes;
    // save successors ...
    for (size_t i = 0; i < numX; i++)
        for (size_t j = 0; j < numY; j++)
            {
            originalFSucc[j][i] = graph->FSucc (nodes[j][i]);
            originalVSucc[j][i] = graph->VSucc (nodes[j][i]);
            Check::False (nodes[j][i] == originalFSucc[j][i]);
            Check::False (nodes[j][i] == originalVSucc[j][i]);
            }

    CheckCounts (*graph, (numX + 1) * (numY + 1), numX * numY + 1, 1);
    for (size_t i = 0; i < numX; i++)
        {
        for (size_t j = 0; j < numY; j++)
            {
            MTGNodeId lowerLeftOfQuad = nodes[j][i];
            auto node = nodes[j][i];
            auto fs = originalFSucc[j][i];
            auto vs = originalVSucc[j][i];
            Check::Size (4, graph->CountNodesAroundFace (lowerLeftOfQuad), "Quad in Grid");
            Check::True (graph->AreNodesInSameFaceLoop (node, fs), "FS check");
            Check::True (graph->AreNodesInSameVertexLoop (node, vs), "VS check");
            Check::False (graph->AreNodesInSameFaceLoop (node, vs), "FS check false");
            Check::False (graph->AreNodesInSameVertexLoop (node, fs), "VS check false");
            }
        }
    graph->ReverseFaceAndVertexLoops ();
    // verify that all successor relations were reversed.
    for (size_t i = 0; i < numX; i++)
        for (size_t j = 0; j < numY; j++)
            {
            Check::Int (nodes[j][i], graph->FSucc (originalFSucc[j][i]), "FSucc reversed");
            Check::Int (nodes[j][i], graph->VSucc (originalVSucc[j][i]), "VSucc reversed");
            }

    void * binaryData;
    size_t binaryCount = graph->WriteToBinaryStream (binaryData);
    MTGGraphP graph1 = new MTGGraph ();
    graph1->LoadFromBinaryStream (binaryData, binaryCount);
    CheckCounts (*graph1, (numX + 1) * (numY + 1), numX * numY + 1, 1);
    delete graph;
    delete graph1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,Grid11)
    {
    CheckGrid (1,1);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,Grid23)
    {
    CheckGrid (2,3);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTG,Grid85)
    {
    CheckGrid (8,5);
    }

