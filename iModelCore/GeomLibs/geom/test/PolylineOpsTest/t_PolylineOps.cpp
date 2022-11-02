/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

static int s_noisy = 0;
void TestXYSingleIntersection (size_t a0, size_t a1, size_t b0, size_t b1)
    {
    }

void BuildSteps (bvector<DPoint3d> &xyz, size_t numLeft, size_t numRight, double yLeft, double yRight)
    {
    xyz.clear ();
    double a0 = 1.0 - (double)numLeft;
    for (size_t i = 0; i < numLeft; i++)
        xyz.push_back ( DPoint3d::From (a0 + (double)i, yLeft, 0.0));
    for (size_t i = 0; i < numRight; i++)
        xyz .push_back ( DPoint3d::From (1.0 + (double)i, yRight, 0.0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps,XYIntersections1)
    {
    bvector<size_t> counts;
    counts.push_back (1);
    counts.push_back (2);
    counts.push_back (3);
    counts.push_back (4);
    size_t numFibonacci = 2;
    for (size_t i = 0; i < numFibonacci; i++)
        {
        size_t k = counts.size () - 2;
        counts.push_back (counts[k] + counts[k+1]);
        }
    double tolerance = 0.0;
    bvector<CurveLocationDetail> dataA, dataB;
    bvector<DPoint3d> xyzA;
    bvector<DPoint3d> xyzB;
    for (size_t i0 = 0; i0 < counts.size (); i0++)
        {
        Check::StartScope ("numALeft", counts[i0]);
        for (size_t i1 = 0; i1 < counts.size (); i1++)
            {
            Check::StartScope ("numARight", counts[i1]);
            for (size_t j0 = 0; j0 < counts.size (); j0++)
                {
                Check::StartScope ("numBLeft", counts[j0]);
                for (size_t j1 = 0; j1 < counts.size (); j1++)
                    {
                    Check::StartScope ("numBRight", counts[j1]);
                    BuildSteps (xyzA, counts[i0], counts[i1], 0.0, 1.0);
                    BuildSteps (xyzB, counts[j0], counts[j1], 1.0, 0.0);
                    PolylineOps::CollectIntersectionsAndCloseApproachesXY (
                            xyzA, NULL,
                            xyzB, NULL,
                            dataA, dataB,
                            tolerance);
                    Check::Size (1, dataA.size (), "Single intersection");
                    Check::Size (1, dataB.size (), "Single intersection");
                    Check::EndScope ();
                    }
                Check::EndScope ();
                }
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }

void AppendWithXIndex (bvector<DPoint3d> &xyz, size_t n, double y, double z = 0.0)
    {
    for (size_t i = 0; i < n; i++)
        {
        xyz.push_back (DPoint3d::From ((double)xyz.size (), y, z));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps,XYIntersections2)
    {
    bvector<size_t> counts;
    counts.push_back (1);
    counts.push_back (2);
    counts.push_back (3);
    counts.push_back (4);
    size_t numFibonacci = 2;
    for (size_t i = 0; i < numFibonacci; i++)
        {
        size_t k = counts.size () - 2;
        counts.push_back (counts[k] + counts[k+1]);
        }
    double tolerance = 0.0;
    bvector<CurveLocationDetail> dataA, dataB;
    bvector<DPoint3d> xyzA;
    bvector<DPoint3d> xyzB;
    for (size_t i0 = 0; i0 < counts.size (); i0++)
        {
        size_t numBlock = counts[i0];
        Check::StartScope ("numBlock", numBlock);
        for (size_t i1 = 0; i1 < counts.size (); i1++)
            {
            size_t blockLength = counts[i1];
            Check::StartScope ("blockLength", counts[i1]);
            xyzA.clear ();
            xyzB.clear ();
            for (size_t i = 0; i < numBlock; i++)
                {
                AppendWithXIndex (xyzA, blockLength, 0.0);
                AppendWithXIndex (xyzA, 1, 2.0);
                }
            AppendWithXIndex (xyzA, 1, 0.0);
            AppendWithXIndex (xyzB, xyzA.size (), 1.0);
            size_t numExpected = 2 * numBlock;
            PolylineOps::CollectIntersectionsAndCloseApproachesXY (
                    xyzA, NULL,
                    xyzB, NULL,
                    dataA, dataB,
                    tolerance);
            Check::Size (numExpected, dataA.size (), "Repeating intersection");
            Check::Size (numExpected, dataB.size (), "Repeating intersection");
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu, PlanarSubdivision)
    {
    // sample data: some more-or-less vertical lines, some more or less horizontal ...
    DSegment3d lines[] =
        {
          DSegment3d::From ( 0,0,0, 1,5,0),
          DSegment3d::From ( 4,0,0, 4,6,0),
          DSegment3d::From ( 6,1,0, 7,5,0),
          DSegment3d::From ( 0,1,0, 8,2,0),
          DSegment3d::From ( -1,4,0, 10,5,0),
          DSegment3d::From ( 0,4,0,  9,5,0),
        };
  
    // Create an empty graph ...
    VuSetP graph = vu_newVuSet (0);
    // Add each line .... VU_BOUNDARY_EDGE bit will be set so we can tell original lines from added lines, e.g. after trianglation
    for (int i = 0; i < _countof (lines); i++)
        vu_makeEdgesFromArray3d (graph, lines[i].point, 2, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, -1.0, -1.0);

    // Find intersections among all pairs (with full connectivity)....
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);

    // there is a node in each corner of each face (including the clocwise outer loop)
    // Set a marker bit OFF at all nodes:
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);

    int numFace = 0;
    // Vist every node.
    bool doPrint = Check::PrintDeepStructs ();
    VU_SET_LOOP (faceSeed, graph)
        {
        // If marker is NOT set, this is the first node seen on this face:
        if (!vu_getMask (faceSeed, visitMask))
            {
            // Set the marker bit all around the face so we know we've been here:
            vu_setMaskAroundFace (faceSeed, visitMask);

            double a = vu_area (faceSeed);
            if (doPrint)
                {
                printf ("** Face %d  (area %.17g)", numFace, a);
                if (a < 0.0)
                    printf ("   EXTERNAL !!!");
                printf ("\n");
                }
            // Access coordinates around the face ...
            VU_FACE_LOOP (vertex, faceSeed)
                {
                DPoint3d xyz;
                vu_getDPoint3d (&xyz, vertex);
                if (doPrint)
                    printf ("   (%.17g, %.17g)\n", xyz.x, xyz.y);
                }
            END_VU_FACE_LOOP (vertex, faceSeed)
            numFace++;
            }
        }
    END_VU_SET_LOOP (faceSeed, graph)

    vu_returnMask (graph, visitMask);
    vu_freeVuSet (graph);
    }

#ifdef TryHeapZOne
#include <RmgrTools/Tools/Heapzone.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Vu, HeapZone)
    {
    HeapZone * myHeap = new HeapZone ('VuSt', true);

    //delete myHeap;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps, ClosestPointClosure)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From(0,0,0),
        DPoint3d::From(3,0,0),
        DPoint3d::From(3,8,0),
        DPoint3d::From(0,8,0)
        };
    DPoint3d xyzA = DPoint3d::From (1,4,0);     // close to closure edge
    DPoint3d xyzB = DPoint3d::From (2,4,0);     // closer to vertical real edge
    DPoint3d xyzA1, xyzB1;
    DPoint3d xyzRight = DPoint3d::From(3, 4, 0);
    DPoint3d xyzLeft = DPoint3d::From(0, 4, 0);
    // with unclosed, both project to the real edge
    double globalFraction;
    size_t edgeIndex, numEdge;
    double edgeFraction, xyDistance;
    PolylineOps::ClosestPointXY(points, false, xyzA,
        nullptr,globalFraction, xyzA1, edgeIndex, numEdge, edgeFraction, xyDistance);
    Check::Near (globalFraction, (edgeIndex + edgeFraction) / numEdge);
    Check::Size (numEdge + 1, points.size ());
    Check::Size(edgeIndex, 1);
    PolylineOps::ClosestPointXY(points, false, xyzB, 
        nullptr, globalFraction, xyzB1, edgeIndex, numEdge, edgeFraction, xyDistance);
    Check::Near(globalFraction, (edgeIndex + edgeFraction) / numEdge);
    Check::Size(numEdge + 1, points.size());
    Check::Size(edgeIndex, 1);


    Check::Near (xyzA1, xyzRight);
    Check::Near(xyzB1, xyzRight);

    // with closed, split right and left
    PolylineOps::ClosestPointXY(points, true, xyzA,
        nullptr, globalFraction, xyzA1, edgeIndex, numEdge, edgeFraction, xyDistance);
    Check::Near(globalFraction, (edgeIndex + edgeFraction) / numEdge);
    Check::Size(numEdge + 1, points.size());
    Check::Size(edgeIndex + 1, points.size());
    Check::Size(edgeIndex, 3);


    PolylineOps::ClosestPointXY(points, true, xyzB,
        nullptr, globalFraction, xyzB1, edgeIndex, numEdge, edgeFraction, xyDistance);
    Check::Near(globalFraction, (edgeIndex + edgeFraction) / numEdge);
    Check::Size(numEdge + 1, points.size());
    Check::Size(edgeIndex, 1);

    Check::Near(xyzA1, xyzLeft);
    Check::Near(xyzB1, xyzRight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps,CollectZPairsForOrderedXPoints)
    {
    bvector<DPoint3d> xyzA
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (3,0,1),
        DPoint3d::From (5,0,1),
        DPoint3d::From (8,0,1),
        };
    bvector<DPoint3d> xyzB
        {
        DPoint3d::From (0,0,2),
        DPoint3d::From (0.5,0,1),
        DPoint3d::From (1.5,0,2),
        DPoint3d::From (3,0,5),
        DPoint3d::From (7,0,5),
        DPoint3d::From (9,0,6),
        };
    bvector <CurveLocationDetailPair> pairs;
    PolylineOps::CollectZPairsForOrderedXPoints (xyzA, xyzB, pairs);

    for (size_t i = 0; i < pairs.size (); i++)
        {
        // same x in both parts of the pair.
        Check::Near (pairs[i].detailA.point.x, pairs[i].detailB.point.x, "x part");
        DPoint3d xyzA1, xyzB1;
        double fA, fB;
        // each point is actually "on" the polyline, and fraction is right ...
        PolylineOps::ClosestPoint (xyzA, false, pairs[i].detailA.point, fA, xyzA1);
        Check::Near (xyzA1, pairs[i].detailA.point);
        Check::Near (fA, pairs[i].detailA.fraction);
        PolylineOps::ClosestPoint (xyzB, false, pairs[i].detailB.point, fB, xyzB1);
        Check::Near (xyzB1, pairs[i].detailB.point);
        Check::Near (fB, pairs[i].detailB.fraction);

        if (i > 0)
            {
            Check::True (pairs[i-1].detailA.point.x <= pairs[i].detailA.point.x, "advancing x");
            Check::True (pairs[i-1].detailA.fraction <= pairs[i].detailA.fraction, "advancing fractionA");
            Check::True (pairs[i-1].detailB.fraction <= pairs[i].detailB.fraction, "advancing fractionB");
            }
        }

    bvector<CurveLocationDetailPair> pairA, pairB;
    CurveLocationDetailPair::SplitByDeltaZ (pairs, 2.0, &pairA, &pairB);
    Check::Size (pairs.size (), pairA.size () + pairB.size ());
    size_t iMin0, iMax0;
    size_t iMinA, iMaxA;
    size_t iMinB, iMaxB;
    CurveLocationDetailPair::DeltaZExtremes (pairs, iMin0, iMax0);
    CurveLocationDetailPair::DeltaZExtremes (pairA, iMinA, iMaxA);
    CurveLocationDetailPair::DeltaZExtremes (pairB, iMinB, iMaxB);
    double min0 = pairs[iMin0].DeltaZ ();
    double max0 = pairs[iMax0].DeltaZ ();
    double minA = pairA[iMinA].DeltaZ ();
    double maxA = pairA[iMaxA].DeltaZ ();
    double minB = pairB[iMinB].DeltaZ ();
    double maxB = pairB[iMaxB].DeltaZ ();
    Check::Near (min0, minA);
    Check::Near (max0, maxB);
    Check::True (maxA < minB);
    Check::True (min0 < max0);
    Check::True (minA < maxA);
    Check::True (minB < maxB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveLocationDetailPair,LocalMinMax)
    {
    auto curveA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    auto curveB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    curveA->Add (ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 10,0,1)));
        bvector<DPoint3d> stringB {
            DPoint3d::From (0,0,1),
            DPoint3d::From (2,0,2),
            DPoint3d::From (4,0,1),
            DPoint3d::From (4.25,0,1),
            DPoint3d::From (4.5,0,1),
            DPoint3d::From (5,0,2),
            DPoint3d::From (6,0,3),
            DPoint3d::From (7,0,1),
            DPoint3d::From (15,0,20)
            };

    curveB->Add (ICurvePrimitive::CreateLineString (stringB));

    bvector<CurveLocationDetailPair> allPairs;
    bvector<CurveLocationDetailPair> localMin, localMax;
    CurveCurve::CollectLocalZApproachExtremaOrderedX (*curveA, *curveB, localMin, localMax);
    CurveCurve::CollectOrderedXVerticalStrokes (*curveA, *curveB, allPairs);
    // hm .. not true in general -- flat spots can create duplicate entries.
    Check::True (localMin.size () + localMax.size ()  <= allPairs.size ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps,GreedyTriangulation1)
    {
    bvector<DPoint3d> xyzA {
        DPoint3d::From (0,0,0)
        };

    bvector<DPoint3d> xyzB {
        DPoint3d::From (0,1,0)
        };

    DVec3d vectorA = DVec3d::From (1,0,0);
    DVec3d vectorB = DVec3d::From (1,0,0);
    // build simple ladder to the right ...
    for (size_t k = 1; k < 4; k++)
        {
        xyzA.push_back (DPoint3d::FromSumOf (xyzA.back (), vectorA));
        xyzB.push_back (DPoint3d::FromSumOf (xyzB.back (), vectorB));
        bvector<DTriangle3d> triangles;
        PolylineOps::GreedyTriangulationBetweenLinestrings (xyzA, xyzB, triangles);
        Check::Size (xyzA.size () + xyzB.size () - 2, triangles.size ());
        // We expect the triangle centroids to advance along x direction ...
        if (triangles.size () > 1)
            {
            double a = 1.0 / 3.0;
            DVec3d normal0, normal1;
            DPoint3d xyz0, xyz1;
            triangles[0].EvaluateNormal (a, a, xyz0, normal0);
            for (size_t i = 1; i < triangles.size (); i++)
                {
                triangles[i].EvaluateNormal (a, a, xyz1, normal1);
                Check::True (normal0.DotProduct (normal1) > 0.0, "triangle normals do not flip");
                Check::True (xyz1.x > xyz0.x, "triangle centers advance in x");
                xyz0 = xyz1;
                normal0 = normal1;
                }
            }
        }
    // add different number of steps on each side -- but still in ladder blocks.
    size_t numAFragment = 3;
    size_t numBFragment = 5;
    vectorA.Scale (1.0 / numAFragment);
    vectorB.Scale (1.0 / numBFragment);
    for (size_t k1 = 1; k1 < 3; k1++)
        {
        for (size_t k = 0; k < numAFragment; k++)
            xyzA.push_back (DPoint3d::FromSumOf (xyzA.back (), vectorA));
        for (size_t k = 0; k < numBFragment; k++)
            xyzB.push_back (DPoint3d::FromSumOf (xyzB.back (), vectorB));
        bvector<DTriangle3d> triangles;
        PolylineOps::GreedyTriangulationBetweenLinestrings (xyzA, xyzB, triangles);
        Check::Size (xyzA.size () + xyzB.size () - 2, triangles.size ());
        // We expect the triangle centroids to advance along x direction ...
        if (triangles.size () > 1)
            {
            double a = 1.0 / 3.0;
            DVec3d normal0, normal1;
            DPoint3d xyz0, xyz1;
            triangles[0].EvaluateNormal (a, a, xyz0, normal0);
            for (size_t i = 1; i < triangles.size (); i++)
                {
                triangles[i].EvaluateNormal (a, a, xyz1, normal1);
                Check::True (normal0.DotProduct (normal1) > 0.0, "triangle normals do not flip");
                Check::True (xyz1.x > xyz0.x, "triangle centers advance in x");
                xyz0 = xyz1;
                normal0 = normal1;
                }
            }
        }


    }

// For each node around face:
// (unconditionally) set markMask
// (conditional) If neither the node or its mate has uncrossableEdgeMask, push the mate on the stack.
void vu_markAndPushMatesAroundFace (VuP seed, bvector<VuP> &stack, VuMask markMask, VuMask uncrossableEdgeMask)
    {
    VU_FACE_LOOP (node, seed)
        {
        vu_setMask (node, markMask);
        VuP mate = vu_edgeMate (node);
        if (!vu_getMask (node, uncrossableEdgeMask) && !vu_getMask (mate, uncrossableEdgeMask))
            stack.push_back (mate);
        }
    END_VU_FACE_LOOP (node, seed)
    }
// ASSUME the graph is merged and connected (so there is only one negative area face)
// Find the single negative area face.
// Mark it with applyMask
// Recursive flood search, crossing all edges NOT marked (unmarked on EITHER SIDE) by barrierMask.
void vu_floodFromMostNegativeAreaFaceToBarrier (VuSetP graph, VuMask barrierMask, VuMask applyMask)
    {
    bvector<VuP> stack;
    VuP firstSeed = vu_findMostNegativeAreaFace (graph);
    vu_clearMaskInSet (graph, applyMask);
    stack.push_back (firstSeed);
    while (!stack.empty ())
        {
        VuP seed = stack.back ();
        stack.pop_back ();
        if (!vu_getMask (seed, applyMask))
            vu_markAndPushMatesAroundFace (seed, stack, applyMask, barrierMask);
        }
    }


void vu_mergeAndCollectOuterBoundaries (bvector<bvector<DPoint3d>> const &polylines, bvector<bvector<DPoint3d>> &outerBoundaries)
    {
    outerBoundaries.clear ();
    VuSetP graph = vu_newVuSet (0);
    // Add each line .... VU_BOUNDARY_EDGE bit will be set so we can tell original lines from added lines, e.g. after trianglation
    for (size_t i = 0; i < polylines.size (); i++)
        vu_makeEdgesFromArray3d (graph,
                const_cast <DPoint3d*> (&polylines[i][0]), (int)polylines[i].size (), VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, -1.0, -1.0);
    // Find intersections among all pairs and sort edges around each vertex . . . 
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    // Add edges so that linework inside holes gets connected to the containing geometry ...
    vu_regularizeGraph (graph);
    // mark the true outer edges.  
    vu_floodFromMostNegativeAreaFaceToBarrier (graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    // Regularization edges can exist "on the outside of original geometry" (inlets, between components).   They are "double exterior".   Purge them.
    // Also purge anything (including originals that is entirely inside.
    // (CLAIM:  this also purges danglers outside the closed loops)
    vu_freeEdgesByMaskCount (graph, VU_EXTERIOR_EDGE, true, false, true);

    // Now the only thing left is true outermost loops.  CW side is marked exterior.  CCW is not.
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            // this is the first encounter with this face.   Mark the whole face visited, and extract if interior
            vu_setMaskAroundFace (seed, visitMask);
            if (!vu_getMask(seed, VU_EXTERIOR_EDGE))
                {
                outerBoundaries.push_back (bvector<DPoint3d>());
                DPoint3d xyz;
                VU_FACE_LOOP (node, seed)
                    {
                    vu_getDPoint3d (&xyz, node);
                    outerBoundaries.back ().push_back (xyz);
                    }
                END_VU_FACE_LOOP (node, seed)
                // add closure point ..
                vu_getDPoint3d (&xyz, seed);
                outerBoundaries.back ().push_back (xyz);

                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, visitMask);
    vu_freeVuSet (graph);
    }

void PrintXY (bvector<bvector<DPoint3d>> const &data, char const *name)
    {
    if (s_noisy == 0)
        return;
    printf ("\n %s\n", name);
    for (size_t i = 0; i < data.size (); i++)
        {
        printf ("\n** (polyline %d) (points %d)",  (int)i, (int) data[i].size ());
        if (data[i].size () > 2)
            printf ("\n");
        for (size_t j = 0; j < data[i].size (); j++)
            {
            DPoint3d xyz = data[i][j];
            printf ("     (%.16g,%.16g)", xyz.x, xyz.y);
            if (j + 1 == data[i].size () || 0 == ((j+1) % 5))
                printf ("\n");
            }
        }
    }

double LengthXY (bvector<bvector<DPoint3d>> const &data)
    {
    double a = 0.0;
    for (auto &polyline : data)
        {
        for (size_t i = 1; i < polyline.size (); i++)
            a += polyline[i-1].Distance (polyline[i]);
        }
    return a;
    }
void AddSegment (bvector<bvector<DPoint3d>> &data, double xA, double yA, double xB, double yB)
    {
    data.push_back (bvector<DPoint3d>{DPoint3d::From (xA,yA,0), DPoint3d::From (xB, yB, 0)});
    }

void AddRectangle (bvector<bvector<DPoint3d>> &data, double xA, double yA, double xB, double yB)
    {
    data.push_back (bvector<DPoint3d>
        {
        DPoint3d::From (xA,yA,0),
        DPoint3d::From (xB,yA,0),
        DPoint3d::From (xB,yB,0),
        DPoint3d::From (xA,yB,0),
        DPoint3d::From (xA,yA,0)
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Vu,OuterBoundaryMerge)
    {
    double expectedLength = 0.0;


    bvector<bvector<DPoint3d>> inputLines;
    AddRectangle (inputLines, 0,0,1,1);
    // UNIT SQUARE AT 00 as SINGLE POLYLINE
    expectedLength += 4.0;

    PrintXY (inputLines, "INPUT Unit square single polyline");
    bvector<bvector<DPoint3d>> loops;
    vu_mergeAndCollectOuterBoundaries (inputLines, loops);
    PrintXY (loops, "Expect Single unit Square");
    Check::Size (1, loops.size (), "one loops");
    Check::Near (expectedLength, LengthXY (loops), "Single Unit Square");

    // Make a neighbor rectangle, this time with 4 separate sticks ..
    AddSegment (inputLines, 3,0, 5,0);  // bottom
    AddSegment (inputLines, 3,1, 5,1);  // top
    AddSegment (inputLines, 3,0, 3,1);  // left
    AddSegment (inputLines, 5,0, 5,1);  // right
    expectedLength += 6.0;

    PrintXY (inputLines, "INPUT add neighbor rectangle as sticks");
    vu_mergeAndCollectOuterBoundaries (inputLines, loops);
    PrintXY (loops, "OUTPUT 2 rectangles");
    Check::Size (2, loops.size (), "two loops");
    Check::Near(expectedLength, LengthXY (loops), "Two loops");

    // Make a skinny rectangle that merges it all into one blob ..
    AddRectangle (inputLines, -1.0,0.25,6,0.75);
    PrintXY (inputLines, "INPUT crossing rectangle merges to single blob");
    expectedLength = 17.0;
    vu_mergeAndCollectOuterBoundaries (inputLines, loops);
    PrintXY (loops, "OUTPUT outer blob");
    Check::Size (1, loops.size (), "one blob");
    Check::Near (expectedLength, LengthXY (loops), "Two loops");

    // Surround it all by a big rectangle that does NOT touch.
    // Confirm that the searches "see" that and only consider the true outer ...
    AddRectangle (inputLines, -5,-5, 10,10);
    expectedLength = 60.0;
    PrintXY (inputLines, "INPUT surround by large square");
    vu_mergeAndCollectOuterBoundaries (inputLines, loops);
    PrintXY (loops, "OUTPUT large square");
    Check::Size (1, loops.size (), "large square");
    Check::Near (expectedLength, LengthXY (loops), "large sqaure");

    // Make a single line that dangles outward and inward
    // Confirm that it gets removed
    AddSegment (inputLines, -4,-4, -8, -4);
    expectedLength = 60.0;
    PrintXY (inputLines, "INPUT with single dangler");
    vu_mergeAndCollectOuterBoundaries (inputLines, loops);
    PrintXY (loops, "OUTPUT large square after dangler");
    Check::Size (1, loops.size (), "large square after dangler");
    Check::Near (expectedLength, LengthXY (loops), "large sqaure after dangler");

    }	
void PrintCurveStrokes (DPoint3dDoubleUVCurveArrays &data, char const *name)
    {
    Check::PrintHeading ("Strokes", name);
    for (size_t i = 0; i < data.m_xyz.size (); i++)
        {
        printf (" %d (xyz %g,%g,%g) (f %g) (U %g,%g,%g)\n",
                (int)i,
                data.m_xyz[i].x, data.m_xyz[i].y, data.m_xyz[i].z,
                data.m_f[i],
                data.m_vectorU[i].x, data.m_vectorU[i].y, data.m_vectorU[i].z);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyline,AddStrokesDEllipse3d)
    {
    DEllipse3d arc = DEllipse3d::From (0,0,0,   4,0,0, 0,2,0, 0.0, Angle::TwoPi ());
    bvector<DPoint3d> stroke0, stroke1;
    auto options = IFacetOptions::CreateForCurves ();
    // accept default angle tolerance ..
    PolylineOps::AddStrokes (arc, stroke0, *options);
    PolylineOps::AddStrokes (arc, stroke1, *options, true, 0.0, 0.5);
    // EXPECT (with inside knowledge) exactly half the stroke 
    Check::Near (PolylineOps::Length (stroke0), 2.0 * PolylineOps::Length (stroke1), "Strokes of full, half arc");
    }

// Create a polyline on the x axis with points at x=0,1,..xMax (inclusive)
// ExecuteAddStrokes and check length
void testSimplePolylineStroke (size_t xMax, double startFraction, double endFraction, IFacetOptionsCR options)
    {
    Check::StartScope ("PolylineStroke along x axis");
    Check::StartScope ("StartFraction", startFraction);
    Check::StartScope ("EndFraction", endFraction);
    bvector<DPoint3d> points;
    for (size_t i = 0; i <= xMax; i++)
        points.push_back (DPoint3d::From ((double)i, 0.0, 0.0));
    auto cp = ICurvePrimitive::CreateLineString (points);
    DPoint3dDoubleUVCurveArrays strokes;
    double originalLength = fabs (points.back().x);

    cp->AddStrokes (strokes, options, startFraction, endFraction);
    //PolylineOps::AddStrokes (points, strokes, options, startFraction, endFraction, nullptr);
    double strokeLength = PolylineOps::Length (strokes.m_xyz);
    double expectedLength = fabs ((endFraction - startFraction) * originalLength);
    if (Check::PrintDeepStructs ())
        {
        printf ("\n  LENGTHS %g,%g\n", expectedLength, strokeLength);
        printf ("\n  FRACTION RANGE %g,%g\n", startFraction, endFraction);
        PrintCurveStrokes (strokes, "strokes");
        }
    if (Check::Near (expectedLength, strokeLength, "Polyline length"))
        {
        for (size_t i = 0; i < strokes.m_f.size (); i++)
            {
            double xF = strokes.m_f[i] * (double)xMax;
            Check::Near (xF, strokes.m_xyz[i].x, "fraction matchup");
            }
        }
    Check::EndScope ();
    Check::EndScope ();
    Check::EndScope ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyline, Stroke)
    {
    auto options = IFacetOptions::CreateForCurves ();
    for (double maxEdgeLength : bvector<double>{10.0, 0.0625})
        {
        options->SetMaxEdgeLength (maxEdgeLength);
        // the whole curve ..
        testSimplePolylineStroke (4, 0.0, 1.0, *options);
        // various clips with exact hits
        testSimplePolylineStroke (4, 0.0, 0.5, *options);
        testSimplePolylineStroke (4, 0.5, 1.0, *options);
        testSimplePolylineStroke (4, 0.5, 0.75, *options);
        // and some real clips ..
        testSimplePolylineStroke (4, 0.0, 0.9, *options);
        testSimplePolylineStroke (4, 0.1, 1.0, *options);

        // And reversals ...
        testSimplePolylineStroke (4, 1.0, 0.0, *options);
        testSimplePolylineStroke (4, 0.9, 0.1, *options);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyline, GreedyTriangleA)
    {
    // triangulation between simple translates
    double a = 4.0;
    double b = 2.0;
    bvector<DPoint3d> pathA {
        DPoint3d::From (0,0,0),
        DPoint3d::From (a,0,0),
        DPoint3d::From (a,b,0),
        DPoint3d::From (0,b,0),
        DPoint3d::From (0,0,0)
        };
    IFacetOptionsPtr surfaceOptions = IFacetOptions::CreateForSurfaces();
    surfaceOptions->SetMaxPerFace (10);

    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    double xStep = 1.5 * a;
    auto tiltAngle = Angle::FromDegrees (10.0);
    for (double z : bvector<double> {1,2,3})
        {
        SaveAndRestoreCheckTransform shifter1 (8.0 * xStep, 0,0);
        for (auto splitSize : bvector <DPoint2d> {
                DPoint2d::From (4.0, 4.0),
                DPoint2d::From (4.0, 2.0),
                DPoint2d::From (4.0, 1.5),
                DPoint2d::From (2.0, 0.7)}
                )
            {
            SaveAndRestoreCheckTransform shifter2 (0, 3.0 * b, 0);

            auto pathB = pathA;
            bvector<int> indices;
            DPoint3dOps::Add (pathB, DVec3d::From (0,0,z));
            bvector<DPoint3d> pathA1, pathB1;
            options->SetMaxEdgeLength (splitSize.x);
            PolylineOps::AddStrokes (pathA, pathA1, *options);
            options->SetMaxEdgeLength (splitSize.y);
            PolylineOps::AddStrokes (pathB, pathB1, *options);
            bvector<DTriangle3d> triangleA;
            PolylineOps::GreedyTriangulationBetweenLinestrings (pathA1, pathB1, triangleA, &indices, tiltAngle);
            Check::SaveTransformed (triangleA);


            Check::Shift  (xStep, 0, 0);
            IPolyfaceConstructionPtr builder1 = IPolyfaceConstruction::Create(*surfaceOptions);
            builder1->AddTriangles (triangleA);
            auto mesh1 = builder1->GetClientMeshPtr ();
            if (mesh1.IsValid ())
                Check::SaveTransformed (*mesh1);


#ifdef TestRuledPattern
            Check::Shift  (xStep, 0, 0);
            MTGFacetsP pFacets = jmdlMTGFacets_grab();
            jmdlMTGFacets_setNormalMode(pFacets, MTG_Facets_VertexOnly, 0, 0);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*surfaceOptions);

            if (jmdlMTGFacets_ruledPatternDPoint3dArrayBoundaries(pFacets,
                pathA1.data(), static_cast<int>(pathA1.size()),
                pathB1.data(), static_cast<int>(pathB1.size())))
                {
                AddMTGFacetsToIndexedPolyface(pFacets, *builder->GetClientMeshPtr(), MTG_EXTERIOR_MASK, MTG_NULL_MASK);
                }
            jmdlMTGFacets_drop(pFacets);
            auto mesh = builder->GetClientMeshPtr ();
            if (mesh.IsValid ())
                Check::SaveTransformed (*mesh);
#endif
            }
        }
    Check::ClearGeometry ("Polyline.GreedyTriangleA");
    }

bvector<DPoint3d> SplitBySimpleLength (bvector<DPoint3d> const &data, double a)
    {
    double b = PolylineOps::Length (data);
    bvector<DPoint3d> out;
    auto cp = ICurvePrimitive::CreateLineString (data);
    double q = b / a;
    size_t n = (int)(q + 0.999999);
    double df = 1.0 / n;
    out.push_back (data[0]);
    for (size_t i = 1; i < n; i++)
        {
        double fraction = i * df;
        DPoint3d xyz;
        cp->FractionToPoint (fraction, xyz);
        out.push_back (xyz);
        }
    out.push_back (data.back ());
    return out;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyline, GreedyTriangleB)
    {
    // triangulation between simple translates
    double a = 4.0;
    double b = 2.0;
    bvector<DPoint3d> pathA {
        DPoint3d::From (0,0,0),
        DPoint3d::From (a,0,0),
        DPoint3d::From (a,b,0),
        DPoint3d::From (0,b,0),
        DPoint3d::From (0,0,0)
        };
    double dzPath = 0.75;

    IFacetOptionsPtr surfaceOptions = IFacetOptions::CreateForSurfaces();
    surfaceOptions->SetMaxPerFace (10);
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    double xStep = 1.5 * a;
    auto tiltAngle = Angle::FromDegrees (10.0);
    for (double z : bvector<double> {1, 2, 3})
        {
        SaveAndRestoreCheckTransform shifter1 (8.0 * xStep, 0,0);
        for (auto splitSize : bvector <DPoint2d> {
                DPoint2d::From (4.0, 4.0),
                DPoint2d::From (4.0, 2.0),
                DPoint2d::From (4.0, 1.5),
                DPoint2d::From (2.0, 0.7),
                DPoint2d::From (0.8, 0.4),
                DPoint2d::From (0.69, 0.378),
                DPoint2d::From (0.32, 0.147)
                }
                )
            {
            SaveAndRestoreCheckTransform shifter2 (0, 3.0 * b, 0);
            auto baseFrame = Check::GetTransform ();
            auto pathB = pathA;
            bvector<int> indices;
            DPoint3dOps::Add (pathB, DVec3d::From (0,0,z));
            auto pathA1 = SplitBySimpleLength (pathA, splitSize.x);
            auto pathB1 = SplitBySimpleLength (pathB, splitSize.y);
            bvector<DTriangle3d> triangleA;
            PolylineOps::GreedyTriangulationBetweenLinestrings (pathA1, pathB1, triangleA, &indices, tiltAngle);
            if (Check::True (triangleA.size () > 0))
                {
                IPolyfaceConstructionPtr builder1 = IPolyfaceConstruction::Create(*surfaceOptions);
                builder1->AddTriangles (triangleA);

                Check::SaveTransformed (pathA1);
                Check::Shift (0,0, dzPath);
//                Check::SaveTransformed (triangleA);
                Check::SaveTransformed (builder1->GetClientMeshR ());
                Check::Shift (0,0, dzPath);
                Check::SaveTransformed (pathB1);

                Check::SetTransform (baseFrame);


#ifdef TestRuledPattern
                Check::Shift  (xStep, 0, 0);
                MTGFacetsP pFacets = jmdlMTGFacets_grab();
                jmdlMTGFacets_setNormalMode(pFacets, MTG_Facets_VertexOnly, 0, 0);
                IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*surfaceOptions);

                if (jmdlMTGFacets_ruledPatternDPoint3dArrayBoundaries(pFacets,
                    pathA1.data(), static_cast<int>(pathA1.size()),
                    pathB1.data(), static_cast<int>(pathB1.size())))
                    {
                    AddMTGFacetsToIndexedPolyface(pFacets, *builder->GetClientMeshPtr(), MTG_EXTERIOR_MASK, MTG_NULL_MASK);
                    }
                jmdlMTGFacets_drop(pFacets);
                auto mesh = builder->GetClientMeshPtr ();
                if (mesh.IsValid ())
                    Check::SaveTransformed (*mesh);
#endif
                }
            }
        }
    Check::ClearGeometry ("Polyline.GreedyTriangleB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyline, GreedyDegenerate)
    {
    // Triangulation between a line (?!) and a rectangle
    double a = 3.0;
    double c = 1.0;
    IFacetOptionsPtr surfaceOptions = IFacetOptions::CreateForSurfaces();
    surfaceOptions->SetMaxPerFace (10);
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    double xStep = 1.5 * a;
    double yStep = 10.0;
    auto tiltAngle = Angle::FromDegrees (10.0);
    for (double yTop : bvector<double> {1, 2, 5,8})
        {

        // A closed path in the xz plane at y = yType:
        bvector<DPoint3d> pathB {
            DPoint3d::From (0,yTop,0),
            DPoint3d::From (a,yTop,0),
            DPoint3d::From (a,yTop,c),
            DPoint3d::From (0,yTop,c),
            DPoint3d::From (0,yTop,0)
            };
        SaveAndRestoreCheckTransform shifter1 (3.0 * xStep, 0,0);
        for (bool close : {false, true})
            {
            SaveAndRestoreCheckTransform shifter2 (xStep, 0,0);
            for (auto splitPoints : {0, 1, 2, 5})
                {
                SaveAndRestoreCheckTransform shifter3 (0, 3.0 * yStep, 0);
                bvector<DPoint3d> pathA;
                pathA.push_back (DPoint3d::From (0,0,0));
                for (int i = 0; i < splitPoints; i++)
                    pathA.push_back (DPoint3d::From (a * (i + 1) / (splitPoints + 1), 0, 0));
                pathA.push_back(DPoint3d::From (a,0,0));

                if (close)
                    {
                    // copy in reverse order (from one before the end)
                    for (size_t i = pathA.size () - 1; i-- > 0;)
                        {
                        DPoint3d xyz = pathA[i];
                        pathA.push_back (xyz);
                        }
                    }

                bvector<DTriangle3d> triangleA;
                bvector<int> indices;   
                PolylineOps::GreedyTriangulationBetweenLinestrings (pathA, pathB, triangleA, &indices, tiltAngle);
                Check::SaveTransformed (pathA);
                Check::SaveTransformed (pathB);
                Check::Shift (0, yStep, 0);

                IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*surfaceOptions);
                builder->AddTriangles (triangleA);
                Check::SaveTransformed (builder->GetClientMeshR ());
                }
            }
        }
    Check::ClearGeometry ("Polyline.GreedyDegenerate");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Dpoint3dOps, CompressAll)
    {
    size_t const nPts = (size_t)1e2;
    bvector<DPoint3d> input (nPts + 1);
    for (size_t i = 0; i <= nPts; ++i)
        input[i] = DPoint3d::From ((double) i / nPts, 0, 0);

    //Add points to remove
    input[30].y = 1.0e-3;
    input[60].y = 5.0 * 1.0e-3;

    Check::SaveTransformed (input);
    Check::Shift (DVec3d::From (2, 0, 0));

    bvector<DPoint3d> result;
    DPoint3dOps::CompressByChordError (result, input, 1.0e-2);

    //Check that only the 2 endpoints are remaining
    Check::True (result.size () == 2);
    if (result.size () == 2)
        {
        double const refTol = 1.0e-3;
        Check::Near (result[0], DPoint3d::FromZero (), nullptr, refTol);
        Check::Near (result[1], DPoint3d::From (1, 0, 0), nullptr, refTol);
        }

    Check::SaveTransformed (result);
    Check::ClearGeometry ("Dpoint3dOps.CompressAll");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Dpoint3dOps, CompressKeep)
    {
    size_t const nPts = (size_t)1e3;
    bvector<DPoint3d> input (nPts + 1);
    for (size_t i = 0; i <= nPts; ++i)
        input[i] = DPoint3d::From ((double) i / nPts, 0, 0);

    //Add a point to keep
    input[70].y = 1.01e-2;

    Check::SaveTransformed (input);
    Check::Shift (DVec3d::From (2, 0, 0));

    bvector<DPoint3d> result;
    DPoint3dOps::CompressByChordError (result, input, 1.0e-2);

    //Check that not everything has been compressed, and that at least 99% of points are removed
    Check::True (3 < result.size () && result.size() < (nPts / 100));

    //Check that the expected point to keep has been actually kept
    Check::True (
        std::find_if (
            result.begin (),
            result.end (),
            [&input] (DPoint3dCR a) 
                {
                return 
                    DVec3d::FromStartEnd (input[70], a)
                    .MagnitudeSquared () 
                    < 1.0e-16;
                }) != result.end ()
            );

    Check::SaveTransformed (result);
    Check::ClearGeometry ("Dpoint3dOps.CompressKeep");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Dpoint3dOps, CompressBspline)
    {
    bvector<DPoint3d> pole0
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (-1,1,0),
        DPoint3d::From (-1,3,0),
        DPoint3d::From (2,3, 0),
        DPoint3d::From (1,0,0)
        };

    MSBsplineCurvePtr bcurve0 = MSBsplineCurve::CreateFromPolesAndOrder (pole0, nullptr, nullptr, 4, false, true);
    Check::SaveTransformed (*ICurvePrimitive::CreateBsplineCurve (bcurve0));
    Check::Shift (DVec3d::From (5, 0, 0));

    size_t const nPts = (size_t)1e3;
    bvector<DPoint3d> input (nPts + 1);
    for (size_t i = 0; i <= nPts; ++i)
        bcurve0->FractionToPoint (input[i], (double) i / nPts);

    Check::SaveTransformed (*ICurvePrimitive::CreateLineString (input));
    Check::Shift (DVec3d::From (5, 0, 0));

    bvector<DPoint3d> result;
    DPoint3dOps::CompressByChordError (result, input, 5e-3);

    Check::True (result.size () < input.size () / 10);

    ICurvePrimitivePtr lineStr = ICurvePrimitive::CreateLineString (result);

    Check::SaveTransformed (*lineStr);
    Check::ClearGeometry ("Dpoint3dOps.CompressBSpline");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Dpoint3dOps, CompressLoop)
    {
    //This curve is a loop (endPt = startPt)
    bvector<DPoint3d> pole0
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (-1,1,0),
        DPoint3d::From (-1,3,0),
        DPoint3d::From (2,3, 0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (0,0,0)
        };

    MSBsplineCurvePtr bcurve0 = MSBsplineCurve::CreateFromPolesAndOrder (pole0, nullptr, nullptr, 4, false, true);
    Check::SaveTransformed (*ICurvePrimitive::CreateBsplineCurve (bcurve0));
    Check::Shift (DVec3d::From (5, 0, 0));

    size_t const nPts = (size_t)1e3;
    bvector<DPoint3d> input (nPts + 1);
    for (size_t i = 0; i <= nPts; ++i)
        bcurve0->FractionToPoint (input[i], (double) i / nPts);

    Check::SaveTransformed (*ICurvePrimitive::CreateLineString (input));
    Check::Shift (DVec3d::From (5, 0, 0));

    bvector<DPoint3d> result;
    DPoint3dOps::CompressByChordError (result, input, 5e-3);

    //We check that endPt == startPt
    Check::True (result.size () < input.size () / 10);
    Check::True (result.size () > 2);
    if (result.size() > 2)
        Check::Near (result[0], result.back ());

    ICurvePrimitivePtr lineStr = ICurvePrimitive::CreateLineString (result);

    Check::SaveTransformed (*lineStr);
    Check::ClearGeometry ("Dpoint3dOps.CompressBSplineLoop");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolylineOps, OnSameSegment)
    {
    Check::True (PolylineOps::PolylineFractionsOnSameSegment( 0.1, 0.1, 2));
    Check::False(PolylineOps::PolylineFractionsOnSameSegment(0.1, 0.47, 4));
    for (double f = 0.05; f < 1.0; f += 0.11083241234897)
        {
        Check::True(PolylineOps::PolylineFractionsOnSameSegment(f, f, 4));
        Check::True(PolylineOps::PolylineFractionsOnSameSegment(f, f, 3));
        double g = f + 0.422;
        if (g < 1.0)
            {
            Check::False(PolylineOps::PolylineFractionsOnSameSegment(f, g, 5));
            Check::False(PolylineOps::PolylineFractionsOnSameSegment(f, g, 4));
            }
        g = f - 0.4;
        if (g > 0.0)
            {
            Check::False(PolylineOps::PolylineFractionsOnSameSegment(f, g, 5));
            Check::False(PolylineOps::PolylineFractionsOnSameSegment(f, g, 4));
            }
        }
    }

TEST(PolylineOps, FractionToFrenetFrame)
    {
    bvector<DPoint3d> points;
    DPoint3d elbow = DPoint3d::From (10,0,-1);
    DVec3d vectorA = DVec3d::From (-10,-1,3);
    DVec3d vectorB = DVec3d::From (5,8,0);
    auto cross = DVec3d::FromCrossProduct (vectorB, vectorA);
    for (double f = 1.0; f > 0.0; f -= 0.18)
        points.push_back (elbow + f * vectorA);
    auto xyz0 = points.front();
    points.insert(points.begin(), xyz0);
    points.insert(points.begin(), xyz0);
    points.insert(points.begin(), xyz0);
    points.push_back (elbow);
    points.push_back(elbow);
    points.push_back(elbow);
    for (double f = 0.0; f < 1.0; f += 0.18)
        points.push_back(elbow + f * vectorB);
    Transform frame;
    for (double f = 0.0; f < 1.0; f += 0.0243243)
        {
        Check::True (PolylineOps::FractionToFrenetFrame (points, f, frame));
        DVec3d zVector = frame.GetMatrixColumn (2);
        Check::Parallel (cross, zVector);
        }

    bvector<DPoint3d> pointA;
    Check::False(PolylineOps::FractionToFrenetFrame(pointA, 0.0, frame));
    pointA.push_back (elbow);
    Check::False(PolylineOps::FractionToFrenetFrame(pointA, 0.0, frame));
    pointA.push_back(elbow);
    Check::False(PolylineOps::FractionToFrenetFrame(pointA, 0.0, frame));
    pointA.push_back (elbow + vectorB);
    Check::True (PolylineOps::FractionToFrenetFrame(pointA, 0.0, frame));
    }
double SumLengths(bvector<DSegment3d> const &segments)
    {
    double sum = 0.0;
    for (auto &s : segments)
        sum += s.Length ();
    return sum;
    }

TEST(PolylineOps, ProjectedParameterRange)
    {
    bvector<DPoint3d> points;
    DPoint3d elbow = DPoint3d::From(10, 0, -1);
    DVec3d vectorA = DVec3d::From(-10, -1, 3);
    for (double f = 1.0; f > 0.0; f -= 0.18)
        points.push_back(elbow + f * vectorA);
    auto xyz0 = points.front();
    points.insert(points.begin(), xyz0);
    points.insert(points.begin(), xyz0);
    points.insert(points.begin(), xyz0);
    points.push_back(elbow);
    points.push_back(elbow);
    points.push_back(elbow);

    double tol = 1.0e-14;
    DRay3d ray0 = DRay3d::FromOriginAndVector(
        DPoint3d::From (1,0,0), 
        DVec3d::From (10,2,3));
    auto range1 = PolylineOps::ProjectedParameterRange (points, ray0, 0, 1);
    auto range2A = PolylineOps::ProjectedParameterRange(points, ray0, 0.45, 0.82);
    auto range2B = PolylineOps::ProjectedParameterRange(points, ray0, 0.82, 0.45);
    Check::True (range2A.IsSameMinMax(range2B, tol), "Parameter range for reversed fractions");
    DRay3d ray1 = ray0;
    ray1.direction.Scale (2.0);
    auto range1B = PolylineOps::ProjectedParameterRange(points, ray1, 0, 1);
    Check::Near (range1.Length (), 2.0 * range1B.Length ());

    }
TEST(PolylineOps, AssemblePolylines)
    {
    DPoint3d point00 = DPoint3d::From (0,0,0);
    DPoint3d point10 = DPoint3d::From(1, 0, 0);
    DPoint3d point11 = DPoint3d::From(1, 1, 0);
    DPoint3d point01 = DPoint3d::From(0, 1, 0);
    DPoint3d point22 = DPoint3d::From(2, 2, 0);
    DPoint3d point21 = DPoint3d::From(2, 1, 0);
    bvector<DSegment3d> segments{
        DSegment3d::From (point00, point10),
        DSegment3d::From(point10, point11),
        DSegment3d::From(point00, point01),
        DSegment3d::From(point01, point11),
        DSegment3d::From (point22, point21),
        DSegment3d::From(point11, point22),
        DSegment3d::From(point21, point10),
        };

    bvector<bvector<DPoint3d>> polylines;
    PolylineOps::AssemblePolylines(segments, polylines);
    Check::Size (3, polylines.size ());

    bvector <DSegment3d> segmentsB;
    for (auto &s : segments)
        {
        SaveAndRestoreCheckTransform shifter (10,0,0);
        segmentsB.push_back (s);

        PolylineOps::AssemblePolylines(segmentsB, polylines);
        Check::SaveTransformed(segmentsB);
        Check::Shift(0, 3, 0);
        Check::SaveTransformed(polylines);
        double r = 0.02;
        double f = 0.025;
        for (auto &p : polylines)
            {
            Check::SaveTransformedMarker(
                    DPoint3d::FromInterpolate (p[0], f, p[1]),
                    -r);
            size_t n = p.size ();
            Check::SaveTransformedMarker(
                DPoint3d::FromInterpolate(p[n-1], f, p[n-2]),
                -r * 0.25);
            }
        Check::Near (SumLengths (segmentsB),
                PolylineOps::Length (polylines));
        }
    Check::ClearGeometry ("PolylineOps.AssemblePolylines");
    }

TEST(PolylineOps, PolygonIntersectPlane)
    {
    bvector<DPoint3d> tiltedPolygon {
        DPoint3d::From (1,0,0), DPoint3d::From (1,1,0), DPoint3d::From(0,1,1), DPoint3d::From (0,0,1), DPoint3d::From (1,0,0)
        };

    DPlane3d planeA = DPlane3d::FromOriginAndNormal (0.0, 0.5, 0.0, 0.0, 1.0, 0.0);
    DPlane3d planeB = DPlane3d::FromOriginAndNormal(0.0, -1.0, 0.0, 0.0, 1.0, 0.0);
    static const int MAX_OUT = 10;
    DPoint3d xyzOut[MAX_OUT];
    double tol = 1.0e-12;
    int numXYZOut;
    bool allOn;
    Check::True (bsiPolygon_intersectDPlane3dExt (xyzOut, &numXYZOut, &allOn, MAX_OUT,
            tiltedPolygon.data (), (int) tiltedPolygon.size (),
            &planeA,
            tol));
    Check::False (allOn, "NOT an allOn case");
    Check::Int (2, numXYZOut, "2 intersection points");

    Check::True(bsiPolygon_intersectDPlane3dExt(xyzOut, &numXYZOut, &allOn, MAX_OUT,
        tiltedPolygon.data(), (int)tiltedPolygon.size(),
        &planeB,
        tol));
    Check::False(allOn, "NOT an allOn case");
    Check::Int(0, numXYZOut, "2 intersection points");

    DPlane3d planeC = DPlane3d::From3Points (tiltedPolygon[0], tiltedPolygon[1], tiltedPolygon[2]);
    Check::True(bsiPolygon_intersectDPlane3dExt(xyzOut, &numXYZOut, &allOn, MAX_OUT,
        tiltedPolygon.data(), (int)tiltedPolygon.size(),
        &planeC,
        tol));
    Check::True (allOn);

//    Check::ClearGeometry("PolylineOps.AssemblePolylines");
    }

TEST(PolylineOps, PolygonClashXYZ)
    {
    bvector<DPoint3d> tiltedPolygon{
        DPoint3d::From(1,0,0), DPoint3d::From(1,1,0), DPoint3d::From(0,1,1), DPoint3d::From(0,0,1), DPoint3d::From(1,0,0)
        };

    double ay = 0.5;
    double by = 2.0;
    double az = 0.1;
    bvector<DPoint3d> zNormalPolygonA{
        DPoint3d::From (0,ay, 0), DPoint3d::From (  5,ay,az), DPoint3d::From (5,ay,1)};

    bvector<DPoint3d> zNormalPolygonB{
        DPoint3d::From(0,by, 0), DPoint3d::From(5,by,az), DPoint3d::From(5,by,1) };


    Check::True (bsiDPoint3dArray_polygonClashXYZ(
        tiltedPolygon.data (), (int)tiltedPolygon.size (),
        zNormalPolygonA.data(), (int)zNormalPolygonA.size()
        ));

    Check::False(bsiDPoint3dArray_polygonClashXYZ(
        tiltedPolygon.data(), (int)tiltedPolygon.size(),
        zNormalPolygonB.data(), (int)zNormalPolygonB.size()
        ));


    Check::True(bsiDPoint3dArray_polylineClashPolygonXYZ(
        zNormalPolygonA.data(), (int)zNormalPolygonA.size(), false,
        tiltedPolygon.data(), (int)tiltedPolygon.size()
    ));

    Check::False(bsiDPoint3dArray_polylineClashPolygonXYZ(
        zNormalPolygonB.data(), (int)zNormalPolygonB.size(), false,
        tiltedPolygon.data(), (int)tiltedPolygon.size()
    ));


    }

TEST(PolylineOps, IsColinear)
    {
    int numPoints = 10;
    bvector<DPoint3d> points;
    for (int i = 0; i < numPoints; ++i)
        points.push_back(DPoint3d::FromXYZ(i,i,i));
    Check::True(PolylineOps::IsColinear(points), "Straight line, no backtracks");
    std::swap(points[2], points[8]);
    Check::True(PolylineOps::IsColinear(points), "Straight line, interior point backtracks");
    std::swap(points[8], points[2]);
    std::swap(points[0], points[5]);
    Check::False(PolylineOps::IsColinear(points), "Straight line, interior point projects before 01 segment");
    std::swap(points[5], points[0]);
    std::swap(points[5], points[numPoints - 1]);
    Check::False(PolylineOps::IsColinear(points), "Straight line, interior point projects after 01 segment");
    std::swap(points[numPoints - 1], points[5]);
    points[3].z -= (1 + sqrt(1.5) * numPoints) * DoubleOps::SmallCoordinateRelTol();
    Check::False(PolylineOps::IsColinear(points), "Non-linear polyline, just outside default tol");
    }
