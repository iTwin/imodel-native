/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

// place points at regular intervals on an ellipse.
void AddPoints (bvector<DPoint3d> &data, DEllipse3dCR source, int count)
    {
    double df = 1.0 / count;
    for (int i = 0; i < count; i++)
        {
        data.push_back (source.FractionToPoint (i * df));
        }
    }

// place count2[j] points on count1 arcs of radius2[i] at step i, i and j wrappping as needed.
void AddPoints (bvector<DPoint3d> &data, DEllipse3dCR arc1, int count1, bvector<double> const &radius2, bvector<int> count2)
    {
    bvector<DPoint3d> point1;
    AddPoints (point1, arc1, count1);
    for (size_t k = 0; k < point1.size (); k++)
        {
        size_t i = k % radius2.size();
        size_t j = k % count2.size ();
        double radius = radius2[i];
        int count = count2[j];
        auto arc2 = DEllipse3d::FromCenterRadiusXY (point1[k], radius);
        AddPoints (data, arc2, count);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(XYPointSearch,HelloWorld)
    {
    DEllipse3d arc1 = DEllipse3d::From (1,1,4,
                5, 1, 1,
                -3,10,2,
                0.0, Angle::Pi ());
    bvector<DPoint3d> points;
    AddPoints (points, arc1, 11, bvector<double> {1, 3, 1.3, 0.3}, bvector<int> {5, 3, 8, 12, 13});
    Check::SaveTransformedMarkers (points, 0.05);

    auto searcher = XYBucketSearch::Create ();
    for (size_t i = 0; i < points.size (); i++)
        searcher->AddPoint (points[i], i);

    for (size_t i = 0; i < points.size (); i++)
        {
        DPoint3d xyz = points[i];
        DPoint3d xyzA;
        size_t id;
        if (Check::True (searcher->ClosestPoint (xyz.x, xyz.y, xyzA, id)))
            {
            Check::Size (i, id, "Direct Point match");
            Check::Near (xyz.x, xyzA.x);
            Check::Near (xyz.y, xyzA.y);
            }
        }

    DRange3d range = DRange3d::From (points);
    bvector<double> splitFractions {0.0, 0.28, 0.9, 1.0};
    bvector<DPoint3d> searchPoint;
    bvector<size_t> searchId;
    Check::Shift (0, 20,0);
    double tileShift = std::max (range.XLength (), range.YLength ());
    // split the whole range into tiles (unequally sized).
    // output searches in each tile.
    // within each tile, we expect to see all the contained points output
    // as both a plus and a circle -- no strays outside.
    for (size_t i = 0; i + 1 < splitFractions.size (); i++)
        {
        SaveAndRestoreCheckTransform shifter1 (tileShift, 0, 0);
        for (size_t j = 0; j + 1 < splitFractions.size (); j++)
            {
            SaveAndRestoreCheckTransform shifter2 (0, tileShift, 0);
            DPoint3d a00 = range.LocalToGlobal (splitFractions[i], splitFractions[j], 0);
            DPoint3d a10 = range.LocalToGlobal (splitFractions[i+1], splitFractions[j], 0);
            DPoint3d a01 = range.LocalToGlobal (splitFractions[i], splitFractions[j+1], 0);
            DPoint3d a11 = range.LocalToGlobal (splitFractions[i+1], splitFractions[j+1], 0);
            searcher->CollectPointsInRangeXY (DRange2d::From (a00.x, a00.y, a11.x, a11.y), searchPoint, searchId);
            Check::SaveTransformed (bvector<DPoint3d>{a00, a10, a11, a01, a00});
            Check::SaveTransformedMarkers (searchPoint, -0.05);
            DRange3d range1 = DRange3d::From (a00, a11);
            size_t numIn = 0;
            for (auto &xyz : points)
                {
                if (range1.IsContainedXY (xyz))
                    {
                    Check::SaveTransformedMarker (xyz, 0.05);
                    numIn++;
                    }
                }
            size_t numCollected = searchPoint.size ();
            Check::Size (numIn, numCollected);
            }
        }
    Check::ClearGeometry ("XYPointSearch.HelloWorld");
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyline,TVertexFixup)
    {
    bvector<DPoint3d> source {
        DPoint3d::From (0,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (0,0,0)
        };

    // get it off the xy plane
    for (auto &xyz : source)
        xyz.z = 0.4 + 0.2 * xyz.x - 0.35 * xyz.y;


    bvector<DPoint3d> spacePoints;
    double tolerance = 1.0e-8;
    double shift = 1.0e-3;
    double df = 1.0 / (double)(source.size () + 3);     // points are scattered at fractions along
    // create some midedge and not quite midedge points
    for (size_t i = 0; i + 1 < source.size (); i++)
        {
        double f = (1 + i) * df;
        // points on the line, but only one interior
        spacePoints.push_back (DPoint3d::FromInterpolate (source[i], f, source[i+1]));
        DPoint3d xyz = spacePoints.back ();
        spacePoints.push_back (DPoint3d::FromInterpolate (source[i], 1.0 + f, source[i+1]));
        spacePoints.push_back (DPoint3d::FromInterpolate (source[i], -f, source[i+1]));
        xyz.z += shift; // close but not on
        spacePoints.push_back (xyz);
        }
    Check::SaveTransformed (source);
    Check::SaveTransformedMarkers (spacePoints, 0.04);
    InsertPointsInPolylineContext context (tolerance);
    bvector<DPoint3d> dest;
    context.InsertPointsInPolyline (source, dest, spacePoints);
    Check::Shift (0, 4, 0);
    Check::SaveTransformed (dest);
    Check::SaveTransformedMarkers (dest, -0.05);


    // do an insertion round with all points off

    Check::Size (2 * source.size () - 1, dest.size (), "one-to-an-edge insertion");
    // raise everything away
    for (auto &xyz : spacePoints)
        {
        xyz.z += shift;
        }
    context.InsertPointsInPolyline (source, dest, spacePoints);
    Check::Size (source.size (), dest.size (), "no insertions expected");
    Check::Shift (0, 4, 0);
    Check::SaveTransformed (dest);
    Check::SaveTransformedMarkers (dest, -0.05);
    Check::ClearGeometry ("Polyline.TVertexFixup");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,TVertexFixup)
    {
    // two grids with exposed edges on the x axis
    double ax = 1.0;
    double bx = 0.4;
    auto mesh1 = UnitGridPolyface (
        DPoint3dDVec3dDVec3d (0,0,0,   ax,0,0,    0,1,0),
        4, 2, false, true);
    // this one grows downward.  It has to be reversed to get the current orientation match.
    auto mesh2 = UnitGridPolyface (
        DPoint3dDVec3dDVec3d (0,0,0,   bx,0,0,    0,-1,0),
        6, 1, false, true);
    mesh2->ReverseIndicesAllFaces ();
    bvector<PolyfaceHeaderPtr> inputs {mesh1, mesh2};

    Check::SaveTransformed (mesh1);
    Check::SaveTransformed (mesh2);
    auto mesh3 = PolyfaceHeader::CloneWithTVertexFixup (inputs);
    Check::Shift (0, 5,0);
    Check::SaveTransformed (mesh3);
    Check::ClearGeometry ("Polyface.TVertexFixup");
    }

