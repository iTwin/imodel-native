/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Vu/capi/vuprint_capi.h>
static int s_noisy = 0;

/// Support context for constructing offset to display vu graphs.
struct VuDisplayableGeometry
{
double m_defaultOffsetDistance;
VuDisplayableGeometry (double defaultOffsetDistance)
    : m_defaultOffsetDistance (defaultOffsetDistance)
    {
    }

// Return a linesegment.
//  Start at the nominal offset point in this sector.
//  Continue for indicated fraction of edge length.
DPoint3d OffsetPoint (VuP baseNode)
    {
    VuP nodeA = baseNode->FPred ();
    VuP nodeC = baseNode->FSucc ();
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
ICurvePrimitivePtr GetOffsetEdge (VuP baseNode, double f0 = 0.0, double f1 = 1.0)
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

static CurveVectorPtr PathToCurveVector (bvector<VuP> &nodes)
    {
    bvector<DPoint3d> points;
    for (auto node : nodes)
        {
        points.push_back (node->GetXYZ ());
        }
    return CurveVector::CreateLinear (points);
    }

static CurveVectorPtr PathToCurveVector (bvector<VuP> &nodes, double offset)
    {
    VuDisplayableGeometry context (offset);
    bvector<DPoint3d> points;
    for (auto node : nodes)
        {
        DPoint3d xyz = context.OffsetPoint (node);
        points.push_back (xyz);
        }
    return CurveVector::CreateLinear (points);
    }

static void PathCoordinates (bvector<VuP> const &nodes, bvector<DPoint3d> &points)
    {
    points.clear ();
    for (auto node : nodes)
        {
        points.push_back (node->GetXYZ ());
        }
    }

static void PathCoordinatesWithOffset (bvector<VuP> const &nodes, double offset, bvector<DPoint3d> &points)
    {
    VuDisplayableGeometry context (offset);
    points.clear ();
    for (auto node : nodes)
        {
        DPoint3d xyz = context.OffsetPoint (node);
        points.push_back (xyz);
        }
    }


};

void SaveGraph (VuSetP graph, VuMask mask)
    {
    DRange3d range = graph->Range ();
    double offset = 0.005 * range.DiagonalDistanceXY ();
    VuDisplayableGeometry helper (offset);
    VU_SET_LOOP (node, graph)
        {
        if (mask == 0 || node->HasMask (mask))
            Check::SaveTransformed (*helper.GetOffsetEdge (node));
        }
    END_VU_SET_LOOP (node, graph)
    }

void SaveGraphEdges (VuSetP graph, VuMask mask)
    {
    VuMask visitMask = graph->GrabMask ();
    graph->ClearMaskInSet (visitMask);
    VU_SET_LOOP (node, graph)
        {
        if (!node->HasMask (visitMask))
            {
            auto mate = node->EdgeMate ();
            node->SetMask (visitMask);
            mate->SetMask (visitMask);
            if (mask == 0 || node->HasMask (mask))
                {
                auto line = ICurvePrimitive::CreateLine (DSegment3d::From (node->GetXYZ (), mate->GetXYZ()));
                Check::SaveTransformed (*line);
                }
            }
        }
    END_VU_SET_LOOP (node, graph)
    graph->DropMask (visitMask);
    }

// sweep from outermost (negative area loop) inwards to loop mask.
// output faces not touched by that sweep.
void SaveGraphEdgesInsideBarrier (VuSetP graph, VuMask loopMask, bool doVoronoi)
    {
    PolyfaceHeaderPtr polyface;
    if (loopMask != 0)
        {
        _VuSet::TempMask exteriorMask (graph);
        vu_floodFromNegativeAreaFaces (graph, loopMask, exteriorMask.Mask ());
        polyface = vu_toPolyface (graph, exteriorMask.Mask ());
        if (polyface.IsValid ())
            Check::SaveTransformed (*polyface);
        }
    else
        {
        polyface = vu_toPolyface (graph, 0);
        if (polyface.IsValid ())
            Check::SaveTransformed (*polyface);
        }
    if (doVoronoi && polyface.IsValid ())
        {
        PolyfaceHeaderPtr delauney, voronoi;
        PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (polyface->Point (), delauney, voronoi);
        if (voronoi.IsValid ())
            {
            DRange3d range = voronoi->PointRange ();
            DPoint3dOps::Add (voronoi->Point (), DVec3d::From (0, range.YLength (), 0));
            Check::SaveTransformed (*voronoi);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MinimumValuePriorityQueue, Bulk)
    {
    MinimumValuePriorityQueue <DPoint3d> queue;
    double fx = 3.0;
    double fy = 7.0;
    double dt = 0.1;
    double ay = 0.2;
    size_t checkPeriod = 27;
    for (size_t i = 0; i < 200; i++)
        {
        double theta = i * dt;
        DPoint3d point = DPoint3d::From(cos(fx * theta), cos(fy * theta + ay), theta);
        queue.Insert(point, point.x);
        if (0 == i % checkPeriod)
            Check::True(queue.Validate(), "Validate Growing Queue");
        }
    size_t num0 = queue.Size();
    size_t num1 = 0;
    double q = -DBL_MAX;
    DPoint3d xyz;
    double value;
    // confirm that values come back sorted on the "value"
    size_t i = 0;
    bvector<double> values;
    while (queue.RemoveMin(xyz, value))
        {
        values.push_back(value);
        Check::LessThanOrEqual(q, value, "Heap removal is increasing");
        num1++;
        if (0 == i % checkPeriod)
            Check::True(queue.Validate(), "Validate Growing Queue");
        i++;
        }
    Check::Size(num0, num1, "Heap count");
    }






// Visit all nodes in set.
// when (index % period) == offset, toggle the mask, where index is the zerobased count of nodes visited.
// return number toggled.
int TogglePeriodic (VuSetP graph, VuMask mask, uint32_t period, uint32_t offset)
    {
    uint32_t index = 0;
    int numToggled = 0;
    VU_SET_LOOP (node, graph)
        {
        if ((index % period) == offset)
            {
            node->ToggleMask (mask);
            numToggled++;
            }
        index++;
        }
    END_VU_SET_LOOP (node, graph)
    return numToggled;
    }

void ExerciseGraphMaskMethods (VuSetP graph, VuMask mask)
    {
    int numNode = graph->CountNodesInSet ();
    graph->ClearMaskInSet (mask);
    Check::Int (0, graph->CountMaskedNodesInSet (mask), "ClearMaskInSet, CountMasked");
    Check::Int(numNode, graph->CountUnmaskedNodesInSet(mask), "ClearMaskInSet, CountUnMasked");

    graph->SetMaskInSet(mask);
    Check::Int(numNode, graph->CountMaskedNodesInSet(mask), "SetMaskInSet, CountMasked");
    Check::Int(0, graph->CountUnmaskedNodesInSet(mask), "SetMaskInSet, CountUnMasked");

    int numToggle = TogglePeriodic (graph, mask, 5, 1); // They start ON.  returned count should be OFF count
    Check::Int(numNode - numToggle, graph->CountMaskedNodesInSet(mask), "ClearMaskInSet, CountMasked");
    Check::Int(numToggle, graph->CountUnmaskedNodesInSet(mask), "ClearMaskInSet, CountUnMasked");


    }




void MakeEdge (VuSetP graph, VuP &nodeA, DPoint3dCR xyzA, VuP &nodeB, DPoint3dCR xyzB, ptrdiff_t distance)
    {
    vu_makePair(graph, &nodeA, &nodeB);
    nodeA->SetXYZ (xyzA);
    nodeB->SetXYZ (xyzB);
    }

void AddTestGraph00 (VuSetP graph, VuP &node0)
    {

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

    VuP node1, node2, node3, node4, node5, node6, node7, node8, node9, node10, node11;
    VuP node12, node13;
    DPoint3d xyzA = DPoint3d::From (0,0,0);
    DPoint3d xyzB = DPoint3d::From(10, 0, 0);
    DPoint3d xyzC = DPoint3d::From(10, 10, 0);
    DPoint3d xyzD = DPoint3d::From(15, 10, 0);
    DPoint3d xyzE = DPoint3d::From(24, 0, 0);
    DPoint3d xyzF = DPoint3d::From(20, 10, 0);
    DPoint3d xyzX = DPoint3d::From(0, -10, 0);
    DPoint3d xyzY = DPoint3d::From(10, -10, 0);

    MakeEdge(graph, node0, xyzA, node1, xyzB, 10);
    MakeEdge(graph, node2, xyzB, node3, xyzE, 20);
    MakeEdge(graph, node4, xyzB, node5, xyzC, 30);
    MakeEdge(graph, node6, xyzE, node7, xyzF, 40);
    MakeEdge(graph, node8, xyzC, node9, xyzD, 50);
    MakeEdge(graph, node10, xyzD, node11, xyzF, 50);
    MakeEdge(graph, node12, xyzX, node13, xyzY, 200);
    vu_vertexTwist(graph, node1, node2);
    vu_vertexTwist(graph, node2, node4);
    vu_vertexTwist(graph, node3, node6);
    vu_vertexTwist(graph, node5, node8);
    vu_vertexTwist (graph, node9, node10);
    vu_vertexTwist(graph, node7, node11);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu, MaskOps)
    {
    VuSetP graph = vu_newVuSet(0);
    VuP node0;
    AddTestGraph00(graph, node0);
    ExerciseGraphMaskMethods(graph, VU_SEAM_EDGE);
    vu_freeVuSet (graph);
    }

void TestShortestPaths (ShortestPathContext::SearchFunctions *functions)
    {
    VuSetP graph = vu_newVuSet(0);
    VuP node0;
    AddTestGraph00(graph, node0);

    ShortestPathContext context(graph, VU_RULE_EDGE);
    context.SearchFromSeed(node0, functions);
    if (s_noisy)
        {
        ShortestPathContext::VertexData data;
        printf("\n\n ===============  ShortestPathContext Test ===============\n");
        for (size_t i = 0; context.GetVertexData (i, data); i++)
            {
            printf(" (vtx %d   %.17g %.17g) (a %g) (primary %d)",
                (int)i,
                data.m_nodeA->GetXYZ ().x,
                data.m_nodeA->GetXYZ().y,
                data.m_a,
                data.m_nodeA->id
                );
            VuP nodeB = data.m_nodeB;
            if (nodeB != nullptr)
                printf("  (back %d %.17g)", nodeB->id,
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
    ShortestPathContext::VertexData data;
    for (size_t i = 0; context.GetVertexData (i, data); i++)
        {
        // unused - double d = context.m_vertexData[i].m_a;
        VuP nodeA = data.m_nodeA;
        VuP nodeB = data.m_nodeB;
        int numMask = vu_countMaskAroundVertex(nodeA, VU_RULE_EDGE);
        DPoint3d xyz0 = nodeA->GetXYZ();
        VU_VERTEX_LOOP(node, nodeA)
            {
            Check::Int(node->GetUserDataAsInt(), (int)i, "All nodes around vertex loop index to same array entry");
            Check::Near(xyz0, node->GetXYZ(), "Consistent xyz");
            }
        END_VU_VERTEX_LOOP(node, nodeA)
        if (Check::True(numMask == 0 || numMask == 1, "Zero or one back edges at vertex")
            && Check::True(vu_findNodeAroundVertex(nodeA, nodeB) == nodeB, "back edge is part of same vertex loop")
            )
            {
            if (numMask == 0)
                {
                if (context.IsUnvisited(nodeA))
                    {
                    Check::False(nodeA == node0, "unvisited node is not seed");
                    Check::True(nodeB == nullptr, "unvisited node has no backedge.");
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
                && Check::True(nodeB != nullptr, "Back edge exists when mask is set")
                )
                {
                Check::True(0 != vu_getMask(nodeB, VU_RULE_EDGE), "back edge marked");
                Check::True(context.GetDistanceToVertex(nodeB->FSucc()) < context.GetDistanceToVertex(nodeA),
                    "back edge leads closer to root.");
                numReached++;
                }
            }
        }
    // Do some numbers checks that will continue to be right in future expanded datasets ..
    Check::Size(1, numRoot, "Shortest path has 1 root");
    Check::True(numUnvisited == 0, "Shortest path numUnVisited");
    Check::True(numReached >= 4, "Shortest paths numReached");
    vu_freeVuSet(graph);
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu,ShortestPaths)
    {
    ShortestPathContext::SearchFunctions unitTester;
    TestShortestPaths(&unitTester);

    ShortestPathContext::XYDistanceSearchFunctions xyTester;
    TestShortestPaths (&xyTester);

    }
#define CompileForGTestWithSaveGraph
#include "ComputeFlightPlan.h"

void TestFlightPlans
(
bvector<bvector<DPoint3d>> const &allBoundaries,
bvector<bvector<DPoint3d>>const &holes,
DPoint3dCR gridReferencePoint,
double spacing,
Angle gridAngle,
DPoint3dCR pathStartEnd
)
    {


    for (auto &boundary : allBoundaries)
        {
        auto baseTransform = Check::GetTransform ();

        DRange3d range;
        range.Init ();
        range.Extend (boundary);

        for (auto &hole : holes)
            range.Extend (hole);

        range.Extend (pathStartEnd);
        double dx = 2.0 * range.XLength ();
        double dy = 1.5 * range.YLength ();


        bvector<FlightPoint> flightPath;
        FlightPlanContext planner;
        planner.ComputeGridFlightPlanOnGrid (boundary, holes, gridReferencePoint, spacing, gridAngle, pathStartEnd, flightPath);

        if (s_noisy)
            {
            for (auto data : flightPath)
                {
                printf ("                        (%.17g,%.17g)\n",
                    data.m_xyz.x - pathStartEnd.x,
                    data.m_xyz.y - pathStartEnd.y
                    );
                printf (" %s %s %s %s\n",
                    data.m_isBoundary ? "BND" : "   ",
                    data.m_isBarrier ? "BAR" : "   ",
                    data.m_isRule ? "RULE" : "    ",
                    data.m_isCamera ? "CAM" : "   "
                    );
                }
            }
        Check::SaveTransformed (boundary);
        for (auto &hole : holes)
            {
            Check::SaveTransformed (hole);
            }
        bvector<DPoint3d> flightXYZ;
        FlightPoint::GetXYZ (flightPath, flightXYZ);
        Check::SaveTransformed (flightXYZ);
        bvector<DPoint3d> offsetPath;
        static double offsetDistance = -0.05;
        static double s_normalZ = 1.0;
        PolylineOps::OffsetLineString (offsetPath, flightXYZ, offsetDistance, DVec3d::From (0,0,s_normalZ), false, Angle::DegreesToRadians (95.0));
        Check::Shift (dx, 0, 0);
        auto transformA = Check::GetTransform ();
        Check::SaveTransformed (flightXYZ);
        Check::Shift (0,dy, 0);
        Check::SaveTransformed (offsetPath);
        Check::SetTransform (transformA);
        Check::Shift (dx, 0, 0);

        bvector<DSegment1d> cameraOffFractions {DSegment1d(0.1,0.3), DSegment1d (0.4,0.6), DSegment1d(0.7,0.9)};
        bvector<DSegment1d> cameraOnFractions{ DSegment1d (0,1) };
        for (size_t i = 0; i + 1 < flightPath.size (); i++)
            {
            if (flightPath[i].m_isRule || flightPath[i].m_isBarrier)
                {
                DrawDashed (cameraOnFractions, flightPath[i].m_xyz, flightPath[i+1].m_xyz);
                }
            else
                {
                DrawDashed (cameraOffFractions, flightPath[i].m_xyz, flightPath[i + 1].m_xyz);
                }
            }
        Check::SetTransform (baseTransform);
        Check::Shift (0, -7.0 * dy, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,FlightPathA)
    {
    bvector<bvector<DPoint3d>> allBoundaries
        {
            {
            DPoint3d::From (0,0,0),
            DPoint3d::From(100,0,0),
            DPoint3d::From(50,50,0),
            DPoint3d::From(0,50,0),
            DPoint3d::From (0,0,0)
            },

            {
            DPoint3d::From(0,0,0),
            DPoint3d::From(100,0,0),
        DPoint3d::From(100,10,0),
        DPoint3d::From(100,20,0),
        DPoint3d::From(100,30,0),
        DPoint3d::From(100,40,0),
        DPoint3d::From(100,50,0),
            DPoint3d::From(50,50,0),
            DPoint3d::From(50,20,0),
            DPoint3d::From(20,20,0),
            DPoint3d::From(20,50,0),
            DPoint3d::From (0,50,0),
            DPoint3d::From (0,0,0)
            },
        };
    bvector<bvector<DPoint3d>> noholes;
    bvector<bvector<DPoint3d>> holes
        {
            {
            DPoint3d::From (10,20,0),
            DPoint3d::From (20,30,0),
            DPoint3d::From (20,60,0),
            DPoint3d::From (10,60,0),
            DPoint3d::From (10,20,0),
                },

            {
            DPoint3d::From (60,20,0),
            DPoint3d::From (70,30,0),
        DPoint3d::From (70,60,0),
        DPoint3d::From (60,60,0),
        DPoint3d::From (60,20,0),
                }
        };
    DPoint3d basePointA = DPoint3d::From (10,12,0);
    DPoint3d basePointB = DPoint3d::From (20, -30, 0);
    DPoint3d basePointC = DPoint3d::From (40,80,0);
    Angle theta0 = Angle::FromDegrees (0.0);
    Angle theta1 = Angle::FromDegrees (15.0);
    TestFlightPlans (allBoundaries, noholes, basePointA, 22.0, theta0, basePointA);
    TestFlightPlans (allBoundaries, noholes, basePointA, 12.0, theta0, basePointA);
    TestFlightPlans (allBoundaries, noholes, basePointA, 12.0, theta1, basePointA);
    TestFlightPlans (allBoundaries, holes, basePointA,   12.0, theta1, basePointB);
    TestFlightPlans (allBoundaries, holes, basePointA,   12.0, theta1, basePointC);
    TestFlightPlans (allBoundaries, holes, basePointA, 8.4, theta1, basePointC);
    TestFlightPlans (allBoundaries, holes, basePointA, 8.4, theta1 * 3.0, basePointC);
    Check::ClearGeometry ("Vu.FlightPathA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu, FlightPathB)
    {
    bvector<bvector<DPoint3d>> allBoundaries
        {
            {
            DPoint3d::From ( 0, 0,0),
            DPoint3d::From (50, 0,0),
            DPoint3d::From (50,50,0),
            DPoint3d::From (0,50,0),
            DPoint3d::From (0,0,0)
            },
        };
    bvector<bvector<DPoint3d>> holes
        {
            {
            DPoint3d::From (10,-2,0),
            DPoint3d::From (30,-2,0),
            DPoint3d::From (30,20,0),
            DPoint3d::From (10,20,0),
            DPoint3d::From (10,-10,0),
            },
        };
    DPoint3d basePointA = DPoint3d::From (30, 12, 0);
    Angle theta0 = Angle::FromDegrees (0.0);
    TestFlightPlans (allBoundaries, holes, basePointA, 22.0, theta0, basePointA);
    Check::ClearGeometry ("Vu.FlightPathB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu, FlightPathC)
    {
    bvector<bvector<DPoint3d>> allBoundaries
        {
            {
            DPoint3d::From (-6440792.208882, 4455055.499158),
            DPoint3d::From (-6440745.753383, 4455051.706867),
        DPoint3d::From (-6440747.412508, 4455087.970609),
        DPoint3d::From (-6440786.520453, 4455086.785523),
        DPoint3d::From (-6440792.208882, 4455055.499158),

            },
        };
    bvector<bvector<DPoint3d>> holes
        {
            {
            DPoint3d::From (-6440782.145598, 4455043.877066),
            DPoint3d::From (-6440752.929242, 4455037.938607),
            DPoint3d::From (-6440758.772513, 4455069.327605),
            DPoint3d::From (-6440776.302327, 4455065.934200),
            DPoint3d::From (-6440782.145598, 4455043.877066)
            },
        };
    DPoint3d basePointA = DPoint3d::From (-6440795.779898, 4455126.591318);
    Angle theta0 = Angle::FromDegrees (0.0);
    for (double spacing : bvector<double>{22.0, 10.3, 5.0})
        TestFlightPlans (allBoundaries, holes, basePointA, spacing, theta0, basePointA);
    Check::ClearGeometry ("Vu.FlightPathC");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu,PrintFuncs)
    {
    VuSetP graph = vu_newVuSet(0);
    VuP node0;
    AddTestGraph00(graph, node0);
    vu_printFaceLabels (graph, "Exercise vu_printFaceLabels");
    GEOMAPI_PRINTF ("OneFace\n");
    vu_printFaceLabelsOneFace (graph, node0);
    vu_freeVuSet(graph);
    }
