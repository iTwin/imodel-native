/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

static int s_noisy = 0;
#ifdef BuildMTGDisplayableGeometry
/// Support context for constructing offset to display vu graphs.
struct MTGDisplayableGeometry
{
double m_defaultOffsetDistance;
MTGDisplayableGeometry (double defaultOffsetDistance)
    : m_defaultOffsetDistance (defaultOffsetDistance)
    {
    }

// Return a linesegment.
//  Start at the nominal offset point in this sector.
//  Continue for indicated fraction of edge length.
DPoint3d OffsetPoint (MTGGraph::Walker baseNode)
    {
    MTGGraph::Walker nodeA = baseNode->FPred ();
    MTGGraph::Walker nodeC = baseNode->FSucc ();
    DPoint3d xyzA = nodeA->GetXYZ ();
    DPoint3d xyzB = baseNode->GetXYZ ();
    DPoint3d xyzC = nodeC->GetXYZ ();
    double eBC = xyzB.Distance (xyzC);
    double eAB = xyzA.Distance (xyzB);
    static double s_offsetTriggerFraction = 0.25;
    double offsetDistance = DoubleOps::Min (m_defaultOffsetDistance, s_offsetTriggerFraction * eBC, s_offsetTriggerFraction * eAB);
    DVec3d   vectorAB = DVec3d::FromStartEnd (xyzA, xyzB);
    DVec3d   vectorBC = DVec3d::FromStartEnd (xyzB, xyzC);
    auto perpAB = DVec3d::FromUnitPerpendicularXY (vectorAB);
    auto perpBC = DVec3d::FromUnitPerpendicularXY (vectorBC);
    static double s_turn1 = 2.3;    // hard turn becomes a problem
    if (perpAB.IsValid () && perpBC.IsValid ())
        {
        if (vectorAB.DotProduct (vectorAB) >= 0.0
            || fabs (vectorAB.SmallerUnorientedAngleTo (vectorBC)) <= s_turn1
            )
            {
            // nothing tricky ... just split the angle.
            auto bisector = perpAB.Value () + perpBC.Value ();
            bisector.ScaleToLength (offsetDistance);
            return xyzB + bisector;
            }
        else if (perpAB.Value ().DotProduct (vectorBC) >= 0.0)
            {
            // hard left turn
            }
        else
            {
            // hard right turn
            }

        }
    return xyzB;
    }

// Return a linesegment.
//  Start at the nominal offset point in this sector.
//  Continue for indicated fraction of edge length.
ICurvePrimitivePtr GetOffsetEdge (MTGGraph::Walker baseNode, double f0 = 0.0, double f1 = 1.0)
    {
    DPoint3d xyzA = OffsetPoint (baseNode);
    DPoint3d xyzB = OffsetPoint (baseNode->FSucc ());
    DSegment3d segment = DSegment3d::From
            (
            DPoint3d::FromInterpolate (xyzA, f0, xyzB),
            DPoint3d::FromInterpolate (xyzA, f1, xyzB)
            );
    return ICurvePrimitive::CreateLine (segment);
    }

static CurveVectorPtr PathToCurveVector (bvector<MTGGraph::Walker> &nodes)
    {
    bvector<DPoint3d> points;
    for (auto node : nodes)
        {
        points.push_back (node->GetXYZ ());
        }
    return CurveVector::CreateLinear (points);
    }

static CurveVectorPtr PathToCurveVector (bvector<MTGGraph::Walker> &nodes, double offset)
    {
    MTGDisplayableGeometry context (offset);
    bvector<DPoint3d> points;
    for (auto node : nodes)
        {
        DPoint3d xyz = context.OffsetPoint (node);
        points.push_back (xyz);
        }
    return CurveVector::CreateLinear (points);
    }

static void PathCoordinates (bvector<MTGGraph::Walker> const &nodes, bvector<DPoint3d> &points)
    {
    points.clear ();
    for (auto node : nodes)
        {
        points.push_back (node->GetXYZ ());
        }
    }

static void PathCoordinatesWithOffset (bvector<MTGGraph::Walker> const &nodes, double offset, bvector<DPoint3d> &points)
    {
    MTGDisplayableGeometry context (offset);
    points.clear ();
    for (auto node : nodes)
        {
        DPoint3d xyz = context.OffsetPoint (node);
        points.push_back (xyz);
        }
    }


};

void SaveGraph (MTGGraphP graph, VuMask mask = 0)
    {
    DRange3d range = graph->Range ();
    double offset = 0.005 * range.DiagonalDistanceXY ();
    MTGDisplayableGeometry helper (offset);
    VU_SET_LOOP (node, graph)
        {
        if (mask == 0 || node->HasMask (mask))
            Check::SaveTransformed (*helper.GetOffsetEdge (node));
        }
    END_VU_SET_LOOP (node, graph)
    }


#endif




void MakeEdge (MTGFacets & facets, MTGNodeId &nodeA, size_t indexA, MTGNodeId &nodeB, size_t indexB, ptrdiff_t distance)
    {
    facets.GetGraphP ()->CreateEdge (nodeA, nodeB);
    facets.SetPointIndex (nodeA, indexA);
    facets.SetPointIndex (nodeB, indexB);
    }

void AddTestGraph00 (MTGFacets & facets, MTGNodeId &node0)
    {
    MTGGraphP graph = facets.GetGraphP ();
// Create a graph useful for various tests.
// PRIMARY GRAPH
//                     50       60
//                  C 8--9 D 10---11 F
//                  5                7
//                  |                 \
//              30  |                  \ 40
//                  |                   \
//                  4                    6
//   A 0----------1 B 2----------------3 E
//         10             20
//
//  DISCONNECTED PIECE
//   X 12 ------- 13 Y
//           200

    MTGNodeId node1, node2, node3, node4, node5, node6, node7, node8, node9, node10, node11;
    MTGNodeId node12, node13;

    auto indexA = facets.AddPoint (DPoint3d::From (0,0,0));
    auto indexB = facets.AddPoint ( DPoint3d::From(10, 0, 0));
    auto indexC = facets.AddPoint ( DPoint3d::From(10, 10, 0));
    auto indexD = facets.AddPoint ( DPoint3d::From(15, 10, 0));
    auto indexE = facets.AddPoint ( DPoint3d::From(24, 0, 0));
    auto indexF = facets.AddPoint ( DPoint3d::From(20, 10, 0));
    auto indexX = facets.AddPoint ( DPoint3d::From(0, -10, 0));
    auto indexY = facets.AddPoint ( DPoint3d::From(10, -10, 0));

    MakeEdge(facets, node0, indexA, node1, indexB, 10);
    MakeEdge(facets, node2, indexB, node3, indexE, 20);
    MakeEdge(facets, node4, indexB, node5, indexC, 30);
    MakeEdge(facets, node6, indexE, node7, indexF, 40);
    MakeEdge(facets, node8, indexC, node9, indexD, 50);
    MakeEdge(facets, node10, indexD, node11, indexF, 50);
    MakeEdge(facets, node12, indexX, node13, indexY, 200);

    graph->VertexTwist (node1, node2);
    graph->VertexTwist (node2, node4);
    graph->VertexTwist (node3, node6);
    graph->VertexTwist (node5, node8);
    graph->VertexTwist (node9, node10);
    graph->VertexTwist (node7, node11);
    }


void TestShortestPaths (MTGShortestPathContext::MTGGraphSearchFunctions *functions)
    {
    MTGFacets facets (MTG_Facets_VertexOnly);
    MTGGraphP graph = facets.GetGraphP ();
    MTGShortestPathContext::SetGraphOrFacetsInSearchFunctions (functions, &facets);
    MTGNodeId node0;
    AddTestGraph00(facets, node0);
    MTGShortestPathContext context(graph);
    context.SearchFromSeed(node0, functions);
    if (s_noisy)
        {
        MTGShortestPathContext::VertexData data;
        printf("\n\n ===============  ShortestPathContext Test ===============\n");
        for (size_t i = 0; context.GetVertexData (i, data); i++)
            {
            DPoint3d xyz = facets.GetXYZ (data.m_nodeA);
            printf(" (vtx %d   %.17g %.17g) (a %g) (primary %d)",
                (int)i,
                xyz.x,
                xyz.y,
                data.m_a,
                data.m_nodeA
                );
            MTGNodeId nodeB = data.m_nodeB;
            if (nodeB != MTGGraph::NullNodeId)
                printf("  (back %d %.17g)", nodeB,
                                    functions->EdgeLength (nodeB));
            printf ("\n");
            }
        }
    // EXPECT
    // At each vertex ...
    //   if VU_RULE_EDGE does not appear it is the root vertex.
    //   if VU_RULE_EDGE appears it appears just once, at nodeB, and the target has lower distance.
    size_t numRoot = 0;
    size_t numUnvisited = 0;
    size_t numReached = 0;
    MTGShortestPathContext::VertexData data;
    for (size_t i = 0; context.GetVertexData (i, data); i++)
        {
        // unused - double d = context.m_vertexData[i].m_a;
        MTGNodeId nodeA = data.m_nodeA;
        MTGNodeId nodeB = data.m_nodeB;
        size_t numMask = graph->CountMaskAroundVertex(nodeA, context.BackEdgeMask ());
        DPoint3d xyz0 = facets.GetXYZ(nodeA);
        MTGARRAY_VERTEX_LOOP (node, graph, nodeA)
            {
            Check::Int(context.NodeToVertexIndex (node), (int)i, "All nodes around vertex loop index to same array entry");
            Check::Near(xyz0, facets.GetXYZ(node), "Consistent xyz");
            }
        MTGARRAY_END_VERTEX_LOOP (node, graph, nodeA)
        if (Check::True(numMask == 0 || numMask == 1, "Zero or one back edges at vertex")
            && (nodeB == MTGGraph::NullNodeId
                || Check::True(graph->AreNodesInSameVertexLoop (nodeA, nodeB), "back edge is part of same vertex loop")
                )
            )
            {
            if (numMask == 0)
                {
                if (context.IsUnvisited(nodeA))
                    {
                    Check::False(nodeA == node0, "unvisited node is not seed");
                    Check::True(nodeB == MTGGraph::NullNodeId, "unvisited node has no backedge.");
                    numUnvisited++;
                    }
                else
                    {
                    Check::True(nodeA == node0, "Seed node is root of marked edges.");
                    Check::Near(0.0, context.GetDistanceToVertex(nodeA), "root distance is 0");
                    numRoot++;
                    }
                }
            else if (numMask == 1
                && Check::True(nodeB != MTGGraph::NullNodeId, "Back edge exists when mask is set")
                )
                {
                Check::True(0 != graph->GetMaskAt (nodeB, context.BackEdgeMask ()), "back edge marked");
                Check::True(context.GetDistanceToVertex(graph->FSucc (nodeB)) < context.GetDistanceToVertex(nodeA),
                    "back edge leads closer to root.");
                numReached++;
                }
            }
        }
    // Do some numbers checks that will continue to be right in future expanded datasets ..
    Check::Size(1, numRoot, "Shortest path has 1 root");
    Check::True(numUnvisited == 0, "Shortest path numUnVisited");
    Check::True(numReached >= 4, "Shortest paths numReached");
    }

#ifdef BuildMTGDisplayableGeometry
void DrawDashed (bvector<DSegment1d> const &fractions, DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    bvector<DPoint3d> line;
    for (auto segment : fractions)
        {
        line.clear ();
        line.push_back (DPoint3d::FromInterpolate (xyz0, segment.GetStart (), xyz1));
        line.push_back (DPoint3d::FromInterpolate (xyz0, segment.GetEnd (), xyz1));
        Check::SaveTransformed (line);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MTG,ShortestPaths)
    {
    MTGShortestPathContext::MTGGraphSearchFunctions unitTester;
    TestShortestPaths(&unitTester);

    MTGShortestPathContext::MTGFacetsXYDistanceSearchFunctions xyTester;
    TestShortestPaths (&xyTester);
    }
