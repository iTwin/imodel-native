/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <Bentley/BeTimeUtilities.h>
#include "testHarness.h"
#include <GeomSerialization/GeomLibsJsonSerialization.h>

// Return coordinates for a rectangle with corners x0,y0 and x1,y1, with interiorFlags all false.
void MakeRectangle (double x0, double y0, double x1, double y1, double z, bvector<DPoint3d> &points, bvector<BoolTypeForVector> &interiorFlags)
    {
    points = bvector<DPoint3d>
        {
        DPoint3d::From (x0, y0, z),
        DPoint3d::From (x1, y0, z),
        DPoint3d::From (x1, y1, z),
        DPoint3d::From (x0, y1, z),
        DPoint3d::From (x0, y0, z)
        };
    interiorFlags.clear ();
    for (size_t i = 0; i < points.size (); i++)
        interiorFlags.push_back (false);
    }

// Return coordianates for a parallelogram with given origin and directions
void MakeParallelogram(DPoint3dCR origin, DVec3dCR xVector, DVec3dCR yVector, bvector<DPoint3d> &points, bvector<BoolTypeForVector> &interiorFlags)
    {
    points.clear ();
    points.push_back (origin);
    points.push_back (origin + xVector);
    points.push_back (origin + xVector + yVector);
    points.push_back (origin + yVector);
    points.push_back (origin);
    interiorFlags.clear ();
    for (size_t i = 0; i < points.size (); i++)
        interiorFlags.push_back (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipNGonSweeps)
    {
    for (int numSide : {12, 4,7,12})
        {
        SaveAndRestoreCheckTransform shifter1 (20,0,0);
        for (auto pass : {-2, -1, 0,1, 2, 4})
            {
            SaveAndRestoreCheckTransform shifter2 (0,30,0);
            IPolyfaceConstructionPtr builder = CreateBuilder (true, true);
            builder->GetFacetOptionsR ().SetMaxPerFace (4);
            builder->AddSweptNGon (numSide, 2.0, 0.0, 3.0, true, true);

            Check::SaveTransformed (builder->GetClientMeshR ());

            // Create a ClipPlaneSet.
            // (Recall:  This is an array of ConvexClipPlaneSets. So we have to build one or more ConvexClipPlanesets and gather them in the clipper.)
            ClipPlaneSet clipper;
            if (pass > 0 && pass < 5)
                {
                ConvexClipPlaneSet convexClipper;
                bvector<DPoint3d> points {DPoint3d::From (-1,-4,0), DPoint3d::From (-1,1,0), DPoint3d::From (5,3,0)};
                convexClipper.AddSweptPolyline (points, DVec3d::From (0,1,3), Angle::FromDegrees (pass * 10.0));
                clipper.push_back (convexClipper);
                Check::SaveTransformed (points);
                Check::Shift (0,10,0);
                Check::SaveTransformed (points);
                }
            else if (pass == -1 || pass == -2)
                {
                // some y-direction sweeps ...
                ConvexClipPlaneSet convexClipper;
                double yy = -3; // y coordinate safely outside the ngon
                double z0 = 0.5;
                double z1 = 1.0;
                double x0 = -1.0;
                double x1 = 1.1;
                bvector<DPoint3d> points {
                    DPoint3d::From (x0, yy, z0),    // for x0 = 1.0, this edge hits nothing but edges going through !!!
                    DPoint3d::From (x0,yy,z1),
                    DPoint3d::From (x1, yy, z1),
                    DPoint3d::From (x1,yy,z0)
                    };
                if (pass == -2)
                    {
                    // close the hole
                    DPoint3d xyz = points[0];
                    points.push_back (xyz);
                    }
                convexClipper.AddSweptPolyline (points, DVec3d::From (0,-1,0), Angle::FromDegrees (0));
                clipper.push_back (convexClipper);
                Check::SaveTransformed (points);
                Check::Shift (0,10,0);
                Check::SaveTransformed (points);

                }
            else
                {
                bvector<DPoint3d> rectanglePoints;
                bvector<BoolTypeForVector> interiorFlag;
                MakeRectangle (1, -2, 3, 3, 0, rectanglePoints, interiorFlag);
                Check::SaveTransformed (rectanglePoints);
                Check::Shift (0,10,0);
                Check::SaveTransformed (rectanglePoints);
                //clipper.push_back (ConvexClipPlaneSet::FromXYBox (xyz0.x, xyz0.y, xyz1.x, xyz1.y));
                clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (rectanglePoints, interiorFlag, true));
                }

            PolyfaceHeaderPtr insideClip;
            ClipPlaneSet::ClipPlaneSetIntersectPolyface (
                    builder->GetClientMeshR (), clipper, true,
                    &insideClip, nullptr);
            if (insideClip.IsValid ())
                Check::SaveTransformed (*insideClip);
            }
        }
    Check::ClearGeometry ("Polyface.ClipNGonSweeps");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipOnEdge)
    {
    for (int clipPass : {0, 1, 2, 3})
        {
        SaveAndRestoreCheckTransform shifter1 (40,0,0);
        for (int meshPass : {0,1})
            {
            SaveAndRestoreCheckTransform shifter2 (20,0,0);
            for (double clipX : {0.9, 1.0})
                {
                SaveAndRestoreCheckTransform shifter3 (5,0,0);
                PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();

                bvector<DPoint3d> meshBase {
                        DPoint3d::From (0,0,0),
                        DPoint3d::From (1,0,0),
                        DPoint3d::From (2,1,0)
                        };

                if (meshPass > 0)
                    meshBase.push_back (DPoint3d::From (1,2,0));

                mesh->AddPolygon (meshBase);
                mesh->SweepToSolid (DVec3d::From (0,0, 2), true);
                Check::SaveTransformed (*mesh);
                // single plane clip
                ClipPlaneSet clipper;
                ConvexClipPlaneSet convexClipper;
                bvector<DPoint3d> clipPoints {DPoint3d::From (clipX,-1,0), DPoint3d::From (clipX,3,0)};

                if (clipPass > 0)       // 2 plane clip
                    clipPoints = bvector<DPoint3d> {
                        DPoint3d::From (clipX,-1,0),
                        DPoint3d::From (clipX,0.25,0),
                        DPoint3d::From (clipX + 1,0.25,0)
                        };

                Check::SaveTransformed (clipPoints);
                convexClipper.AddSweptPolyline (clipPoints, DVec3d::From (0,0,1), Angle::FromDegrees (0));
                if (clipPass == 2)
                    {
                    ClipPlane zPlane (
                            DVec3d::From (0,0,1),
                            DPoint3d::From (0,0, 1.1));
                    convexClipper.Add (ValidatedClipPlane (zPlane, true));
                    }
                if (clipPass == 3)
                    {
                    auto normal = DVec3d::From (0.2, 0, 1).ValidatedNormalize ();
                    ClipPlane zPlane (
                            normal,
                            DPoint3d::From (0,0, 1.1));
                    convexClipper.Add (ValidatedClipPlane (zPlane, true));
                    }
                clipper.push_back (convexClipper);

                Check::Shift (0,20,0);
                PolyfaceHeaderPtr insideC, outsideC;
                ClipPlaneSet::ClipPlaneSetIntersectPolyface (*mesh, clipper, true, &insideC, &outsideC);
                Check::SaveTransformed (insideC);
                Check::Shift (0,5,0);
                Check::SaveTransformed (outsideC);
                }
            }
        }

    Check::ClearGeometry ("Polyface.ClipOnEdge");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipDiamond)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);

    builder->AddSweptNGon (4, 2.0, 0.0, 1.0, true, true);

    for (auto shift : bvector<DVec3d> {
            DVec3d::From (0,0,0),
            DVec3d::From (-1,0,0),
            DVec3d::From (-1,1,0),
            DVec3d::From (-2,0,0),
            DVec3d::From (-2,2,0)
            })
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        Check::SaveTransformed (builder->GetClientMeshR ());
        bvector<DPoint3d> rectanglePoints;
        bvector<BoolTypeForVector> interiorFlag;
        double dx = shift.x;
        double dy = shift.y;
        MakeRectangle (dx + 1.1, dy - 2.023, dx + 3, dy + 3, 0, rectanglePoints, interiorFlag);

        Check::SaveTransformed (rectanglePoints);
        Check::Shift (0,10,0);
        Check::SaveTransformed (rectanglePoints);

        ClipPlaneSet clipper;
        //clipper.push_back (ConvexClipPlaneSet::FromXYBox (xyz0.x, xyz0.y, xyz1.x, xyz1.y));
        clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (rectanglePoints, interiorFlag, true));

        PolyfaceHeaderPtr insideClip;
        ClipPlaneSet::ClipPlaneSetIntersectPolyface (
                builder->GetClientMeshR (), clipper, true,
                &insideClip, nullptr);
        if (insideClip.IsValid ())
            Check::SaveTransformed (*insideClip);

        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.ClipDiamond");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipTunnel)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (true, true);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);


    // tunnel cap in xz plane . .
    bvector<DPoint3d> outerLoop, innerLoop;
    bvector<BoolTypeForVector> outerVisible, innerVisible;
    MakeParallelogram (DPoint3d::From (0,0,0), DVec3d::From (10,0,0), DVec3d::From (0,0,10), outerLoop, outerVisible);
    MakeParallelogram (DPoint3d::From (1,0,1), DVec3d::From (8,0,0), DVec3d::From (0,0,8), innerLoop, innerVisible);

    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (CurveVector::CreateLinear (outerLoop, CurveVector::BOUNDARY_TYPE_Outer));
    parityRegion->Add(CurveVector::CreateLinear (innerLoop, CurveVector::BOUNDARY_TYPE_Inner));
    Check::SaveTransformed (*parityRegion);
    Check::Shift (0,20,0);

    auto solid = ISolidPrimitive::CreateDgnExtrusion (
        DgnExtrusionDetail (parityRegion, DVec3d::From (0,10,0), true));

    builder->AddSolidPrimitive (*solid);

    DVec3d clipOrigin = DVec3d::From (-1,3,0);

    for (auto diagonal : bvector<DVec3d> {
            DVec3d::From (14, 5,0),
            DVec3d::From (14, 5,0),
            DVec3d::From (14, 10,0),
            DVec3d::From (14, 5,0),
            DVec3d::From (5,5,0)
            })
        {
        SaveAndRestoreCheckTransform shifter (30,0,0);
        Check::SaveTransformed (builder->GetClientMeshR ());
        bvector<DPoint3d> rectanglePoints;
        bvector<BoolTypeForVector> interiorFlag;
        double ax = clipOrigin.x;
        double ay = clipOrigin.y;
        double bx = clipOrigin.x + diagonal.x;
        double by = clipOrigin.y + diagonal.y;
        MakeRectangle (ax, ay, bx, by, 0, rectanglePoints, interiorFlag);

        Check::SaveTransformed (rectanglePoints);
        Check::Shift (0,20,0);
        Check::SaveTransformed (rectanglePoints);

        ClipPlaneSet clipper;
        //clipper.push_back (ConvexClipPlaneSet::FromXYBox (xyz0.x, xyz0.y, xyz1.x, xyz1.y));
        clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (rectanglePoints, interiorFlag, true));

        PolyfaceHeaderPtr insideClip;
        ClipPlaneSet::ClipPlaneSetIntersectPolyface (
                builder->GetClientMeshR (), clipper, true,
                &insideClip, nullptr);
        if (insideClip.IsValid ())
            Check::SaveTransformed (*insideClip);
        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.ClipTunnel");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, PolygonClipTunnel)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);


    // tunnel cap in xz plane . .
    bvector<DPoint3d> outerLoop, innerLoop;
    bvector<BoolTypeForVector> outerVisible, innerVisible;
    MakeParallelogram (DPoint3d::From (0,0,0), DVec3d::From (10,0,0), DVec3d::From (0,0,10), outerLoop, outerVisible);
    MakeParallelogram (DPoint3d::From (1,0,1), DVec3d::From (8,0,0), DVec3d::From (0,0,8), innerLoop, innerVisible);

    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (CurveVector::CreateLinear (outerLoop, CurveVector::BOUNDARY_TYPE_Outer));
    parityRegion->Add(CurveVector::CreateLinear (innerLoop, CurveVector::BOUNDARY_TYPE_Inner));
    Check::SaveTransformed (*parityRegion);
    Check::Shift (0,20,0);

    auto solid = ISolidPrimitive::CreateDgnExtrusion (
        DgnExtrusionDetail (parityRegion, DVec3d::From (0,10,0), true));

    builder->AddSolidPrimitive (*solid);

    DVec3d clipOrigin = DVec3d::From (-1,3,0);

    for (auto sweepVector: {DVec3d::From (0,0,1), DVec3d::From (1,1,3)})
        {
        SaveAndRestoreCheckTransform shifter1 (0,50,0);
        for (auto diagonal : bvector<DVec3d> {
                DVec3d::From (14, 5,0),
                DVec3d::From (14, 5,0),
                DVec3d::From (14, 10,0),
                DVec3d::From (14, 5,0),
                DVec3d::From (5,5,0)
                })
            {
            SaveAndRestoreCheckTransform shifter2 (30,0,0);
            Check::SaveTransformed (builder->GetClientMeshR ());
            bvector<DPoint3d> rectanglePoints;
            bvector<BoolTypeForVector> interiorFlag;
            double ax = clipOrigin.x;
            double ay = clipOrigin.y;
            double bx = clipOrigin.x + diagonal.x;
            double by = clipOrigin.y + diagonal.y;
            MakeRectangle (ax, ay, bx, by, 0, rectanglePoints, interiorFlag);

            Check::SaveTransformed (rectanglePoints);
            Check::Shift (0,20,0);
            Check::SaveTransformed (rectanglePoints);

            PolyfaceHeaderPtr insideClip;
            ClipPlaneSet::SweptPolygonClipPolyface (
                    builder->GetClientMeshR (),
                    rectanglePoints, sweepVector,
                    true,
                    &insideClip, nullptr);
            if (insideClip.IsValid ())
                Check::SaveTransformed (*insideClip);
            }
        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.PolygonClipTunnel");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipPolygonPrism)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();

    bvector<bvector<DPoint3d>> profiles;
    profiles.push_back({DPoint3d::From(1,1,0), DPoint3d::From(1,1,3), DPoint3d::From(1,2,3), DPoint3d::From(1,2,2), DPoint3d::From(1,1,0)});
    profiles.push_back({DPoint3d::From(0,1,0), DPoint3d::From(0,1,3), DPoint3d::From(0,2,3), DPoint3d::From(0,2,2), DPoint3d::From(0,1,0)});
    for (auto const& points : profiles)
        {
        auto section = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
        Check::Shift (300, 0, 0);

        auto solid = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(section, DVec3d::From(20,0,0), true));

        IPolyfaceConstructionPtr builder = CreateBuilder(false, false);
        builder->GetFacetOptionsR().SetMaxPerFace(4);
        builder->GetFacetOptionsR().SetMaxEdgeLength(4);
        builder->AddSolidPrimitive (*solid);
        PolyfaceHeaderR polyface = builder->GetClientMeshR ();

        for (auto sweepVector: {DVec3d::From (0,0,1), DVec3d::From (0,0,-1)})
            {
            SaveAndRestoreCheckTransform shifter (100,0,0);
            for (auto diagonal : {DVec3d::From(2,5), DVec3d::From(6,3)})
                {
                SaveAndRestoreCheckTransform shifter1 (50,0,0);
                for (bool reverseRectangle : {false, true})
                    {
                    SaveAndRestoreCheckTransform shifter2(0, 40, 0);

                    bvector<DPoint3d> rectanglePoints;
                    bvector<BoolTypeForVector> _;
                    MakeRectangle(0, 0, diagonal.x, diagonal.y, 0, rectanglePoints, _);
                    if (reverseRectangle)
                        std::reverse (rectanglePoints.begin (), rectanglePoints.end ());
                    Check::SaveTransformed (rectanglePoints);
                    Check::SaveTransformed (polyface);

                    PolyfaceHeaderPtr insideClip, outsideClip;
                    ClipPlaneSet::SweptPolygonClipPolyface(polyface, rectanglePoints, sweepVector, true, &insideClip, &outsideClip);
                    Check::Shift (0,10,0);
                    if (insideClip.IsValid ())
                        Check::SaveTransformed (*insideClip);
                    Check::Shift (0,10,0);
                    if (outsideClip.IsValid ())
                        Check::SaveTransformed (*outsideClip);

                    bvector<bvector<size_t>> componentSeeds;
                    MeshAnnotationVector messages(true);
                    if (Check::True(insideClip->OrientAndCollectManifoldComponents(componentSeeds, messages), "insideClip successfully analyzed"))
                        Check::Size(1, componentSeeds.size(), "insideClip has 1 component");
                    if (Check::True(outsideClip->OrientAndCollectManifoldComponents(componentSeeds, messages), "outsideClip successfully analyzed"))
                        Check::Size(1, componentSeeds.size(), "outsideClip has 1 component");

                    auto volumeTotal = polyface.ValidatedVolume();
                    auto volumeInside = insideClip->ValidatedVolume();
                    auto volumeOutside = outsideClip->ValidatedVolume();
                    if (Check::True(volumeInside.IsValid(), "insideClip has volume") && Check::True(volumeOutside.IsValid(), "outsideClip has volume"))
                        Check::Near(volumeTotal.Value(), volumeInside.Value() + volumeOutside.Value(), "clips have expected total volume");
                    }
                }
            }
        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.ClipPolygonPrism");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, PolygonFixRaggedTunnel)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);


    // tunnel cap in xz plane . .
    bvector<DPoint3d> outerLoop, innerLoop;
    bvector<BoolTypeForVector> outerVisible, innerVisible;
    MakeParallelogram (DPoint3d::From (0,0,0), DVec3d::From (10,0,0), DVec3d::From (0,0,10), outerLoop, outerVisible);
    MakeParallelogram (DPoint3d::From (1,0,1), DVec3d::From (8,0,0), DVec3d::From (0,0,8), innerLoop, innerVisible);

    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (CurveVector::CreateLinear (outerLoop, CurveVector::BOUNDARY_TYPE_Outer));
    parityRegion->Add(CurveVector::CreateLinear (innerLoop, CurveVector::BOUNDARY_TYPE_Inner));
    Check::SaveTransformed (*parityRegion);
    Check::Shift (0,20,0);

    auto solid = ISolidPrimitive::CreateDgnExtrusion (
        DgnExtrusionDetail (parityRegion, DVec3d::From (0,10,0), false));

    builder->AddSolidPrimitive (*solid);

    auto raggedMesh = builder->GetClientMeshPtr ();
    Check::SaveTransformed (raggedMesh);
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.PolygonFixRaggedTunnel");
    }
#include <algorithm>
/*
* struct PointWithAltitudes contains:
* * xyz = a point
* * altitudes[i] = its altitude (positive is OUT) from plane i.
* * outBit = bit map, with bits addressed by the 6 static constants BitLowX, BitHighX, BitLowY, BitHighY, BitLowZ, BitHighZ
*
* The Init method
* * saves xyz
* * sets all 6 altitudes
* * sets the 6 bits of outBit
*/
struct PointWithAltitudes {
    static const uint32_t BitLowX = 0x01;
    static const uint32_t BitHighX = 0x02;
    static const uint32_t BitLowY = 0x04;
    static const uint32_t BitHighY = 0x08;
    static const uint32_t BitLowZ = 0x10;
    static const uint32_t BitHighZ = 0x20;

    DPoint3d xyz;
    // altitude (out positive) from low.x, high.x, low.y. high.y, low.z, high.z
    double altitude[6];
    uint32_t outBit;
    void Init(DPoint3d const &point, DRange3d const & range) {
        xyz = point;
        outBit = 0;
        altitude[0] = range.low.x - point.x;
        if (altitude[0] > 0.0)
            outBit |= BitLowX;
        altitude[1] = point.x - range.high.x;
        if (altitude[1] > 0.0)
            outBit |= BitHighX;

        altitude[2] = range.low.y - point.y;
        if (altitude[2] > 0.0)
            outBit |= BitLowY;
        altitude[3] = point.y - range.high.y;
        if (altitude[3] > 0.0)
            outBit |= BitHighY;

        altitude[4] = range.low.z - point.z;
        if (altitude[4] > 0.0)
            outBit |= BitLowZ;
        altitude[5] = point.z - range.high.z;
        if (altitude[5] > 0.0)
            outBit |= BitHighZ;
        }
    };
/** Array with the 6 plane bits */
static const uint32_t s_planeBits[] =
    {
        PointWithAltitudes::BitLowX,
        PointWithAltitudes::BitHighX,
        PointWithAltitudes::BitLowY,
        PointWithAltitudes::BitHighY,
        PointWithAltitudes::BitLowZ,
        PointWithAltitudes::BitHighZ
    };


/**
 * * return 1 if any part of the triangular area is within the range.
 * * return 0 if the triangle is entirely outside.
 * * return -1 if ambiguous (This should not happen)
 */
extern int32_t ClassifyTriangleInOutRange
(
    DRange3d const & range,
    DPoint3d const &point0,
    DPoint3d const &point1,
    DPoint3d const &point2
)
    {
    PointWithAltitudes points[2][11];	// clip may produce more points. also allow for a wraparound.
    // uint32_t currentIndex = 0;
    points[0][0].Init(point0, range);
    if (points[0][0].outBit == 0)
        return 1;
    points[0][1].Init(point1, range);
    if (points[0][1].outBit == 0)
        return 1;
    points[0][2].Init(point2, range);
    if (points[0][2].outBit == 0)
        return 1;
    // any surviving 1 in the AND of all bits indicates completely outside in that direction . . .
    uint32_t andBits = points[0][0].outBit & points[0][1].outBit & points[0][2].outBit;
    uint32_t orBits = points[0][0].outBit | points[0][1].outBit | points[0][2].outBit;
    if (andBits != 0)
        return 0;
    // Simple tests are ambiguous ... do proper clip
    uint32_t numOld = 3;
    uint32_t numNew;
    uint32_t oldPointsIndex = 0;
    uint32_t newPointsIndex;
    uint32_t i;
    double altitude0, altitude1, fraction;
    for (auto planeSelector = 0; planeSelector < 6; planeSelector++)
        {
        // If the original points did not have both IN and OUT for this plane, don't bother clipping it.
        if ((andBits & s_planeBits[planeSelector]) != (orBits & s_planeBits[planeSelector]))
            {
            newPointsIndex = 1 - oldPointsIndex;
            points[oldPointsIndex][numOld] = points[oldPointsIndex][0];
            altitude0 = points[oldPointsIndex][0].altitude[planeSelector];
            numNew = 0;
            for (i = 1; i <= numOld; i++)
                {
                altitude1 = points[oldPointsIndex][i].altitude[planeSelector];
                if (altitude0 <= 0)
                    {
                    points[newPointsIndex][numNew++] = points[oldPointsIndex][i - 1];
                    }
                if (altitude0 * altitude1 < 0.0)
                    {
                    // the edge is split.
                    // division for the fraction is safe because the altitudes are different enough to have negative product.
                    fraction = altitude0 / (altitude0 - altitude1);
                    points[newPointsIndex][numNew].Init(DPoint3d::FromInterpolate(
                        points[oldPointsIndex][i - 1].xyz,
                        fraction,
                        points[oldPointsIndex][i].xyz), range);
                    if (points[newPointsIndex][numNew].outBit == 0)
                        return 1;
                    numNew++;
                    }
                altitude0 = altitude1;
                }
            if (numNew == 0)
                return 0;
            oldPointsIndex = newPointsIndex;
            numOld = numNew;
            }
        }
    // numOld tells if anything is left ..
    return numOld > 0 ? 1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Range3d,ClassifyTriangle)
    {
    Check::ClearGeometry("Range3d.ClassifyTriangle");
    auto point0 = DPoint3d::From (-343884.83689709584, 153920.80327865816, 9048.5880518645045);
    auto point1 = DPoint3d::From(310187.35062496824, 153347.97073685721, 9048.5880518645045);
    auto point2 = DPoint3d::From(227941.46285813852, -137168.89271681113, 9048.5880518645045);

    auto range = DRange3d::NullRange ();
    range.Extend (23670.156381845838, -44808.704728785844, -9.0949470177292824e-13);
    range.Extend(35505.234572768757, -29872.469819190566, 9050.3983600652864);
    Check::SaveTransformedEdges(range);
    Check::SaveTransformed(std::vector<DPoint3d>({ point0, point1, point2 }));

    for (auto dz : {0.0, -3.0, 3.0})
        {
        auto point0A = point0;
        auto point1A = point1;
        auto point2A = point2;
        point0A.z += dz;
        point1A.z += dz;
        point2A.z += dz;
        auto q = ClassifyTriangleInOutRange (range, point0A, point1A, point2A);
        printf ("  q= %d", q);
        }
    Check::ClearGeometry("Range3d.ClassifyTriangle");
    }

TEST(Range3d, isAnyRangeFaceInsideA)
    {
    // get a first-pierce on each face of the range ...
    // triangle not on any primary plane ...
    bvector<DPoint3d> triangle {DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), DPoint3d::From (0,0,1)};
    auto xyz0 = triangle.front();
    triangle.push_back(xyz0);

    auto range = DRange3d::From (DPoint3d::From (-4,-6,-3), DPoint3d::From (3,4,8));
    Check::SaveTransformedEdges (range);
    for (auto sweepDirection : {
        DVec3d::From (1,0,0),
        DVec3d::From (-1,0,0),
        DVec3d::From (0,1,0),
        DVec3d::From (0,-1,0),
        DVec3d::From (0,0,1),
        DVec3d::From (0,0,-1)
        })
        {
        auto loop = CurveVector::CreateLinear (triangle, CurveVector::BOUNDARY_TYPE_Outer);
        auto solid = ISolidPrimitive::CreateDgnExtrusion(
            DgnExtrusionDetail (loop, sweepDirection * 10.0, false));
        auto clipper = ClipPlaneSet::FromSweptPolygon(&triangle[0], 4, &sweepDirection);
        ClipPlane backPlane (sweepDirection, DPoint3d::From (0,0,0));
        clipper.front ().push_back (backPlane);

        Check::SaveTransformed (*solid);
        bvector<DPoint3d> clippedFace;
        if (clipper.IsAnyRangeFacePointInside(range, &clippedFace))
            Check::SaveTransformed (clippedFace, true);
        }
    Check::ClearGeometry("Range3d.isAnyRangeFaceInsideA");
    }
TEST(Range3d, isAnyRangeFaceInsideB)
    {
    bvector<DPoint3d> triangle{ DPoint3d::From(5, 4, 0), DPoint3d::From(7.5, 3, 0.5), DPoint3d::From (4, 8, 3) };
    auto xyz0 = triangle.front ();
    triangle.push_back (xyz0);
    DVec3d normal = DVec3d::FromCrossProductToPoints(triangle[0], triangle[1], triangle[2]);
    normal.ScaleToLength(3.0);
    auto loop = CurveVector::CreateLinear(triangle, CurveVector::BOUNDARY_TYPE_Outer);
    auto solid = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(loop, normal, false));
    Check::SaveTransformed(*solid);
    auto clipper = ClipPlaneSet::FromSweptPolygon(&triangle[0], triangle.size (), &normal);

    for (double x0 = 0; x0 < 10; x0 += 2.0)
        {
        for (double y0 = 0; y0 < 12; y0 += 3)
            {
            auto range = DRange3d::From(DPoint3d::From(x0, y0, 1), DPoint3d::From(x0 + 1.9, y0 + 2.5, 2));
            Check::SaveTransformedEdges(range);

            bvector<DPoint3d> clippedFace;
            if (clipper.IsAnyRangeFacePointInside(range, &clippedFace))
                {
                Check::Shift (0,0,10);
                Check::SaveTransformed(clippedFace, true);
                Check::SaveTransformedEdges(range);
                Check::Shift (0,0, -10);
                }
            }
        }
    Check::ClearGeometry("Range3d.isAnyRangeFaceInsideB");
    }
TEST(Range3d, isAnyRangeFaceInsideC)
    {
    bvector<DPoint3d> polygon{
            DPoint3d::From(5, 4, 0),
            DPoint3d::From(9,4,0),
            DPoint3d::From(14,16,0),
            DPoint3d::From(10,16,0),
            DPoint3d::From (13,15,0),
            DPoint3d::From (8,6,0),
            DPoint3d::From (5,6,0)};
    auto xyz0 = polygon.front();
    polygon.push_back(xyz0);
    double sweepDistance  = 8.0;
    double zShift = 20.0;
    for (double skewFactor : {0.0, 0.50, 1.0, 2.0})
        {
        SaveAndRestoreCheckTransform shifter (40.0, 0,0);
        DVec3d normal = DVec3d::From (-skewFactor, 0.5 * skewFactor, 1.0);

        auto loop = CurveVector::CreateLinear(polygon, CurveVector::BOUNDARY_TYPE_Outer);
        auto solid = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(loop, sweepDistance * normal, false));
        Check::SaveTransformed(*solid);
        auto clipper = ClipPlaneSet::FromSweptPolygon(&polygon[0], polygon.size(), &normal);

        for (double x0 = 0; x0 < 15; x0 += 2.0)
            {
            for (double y0 = 0; y0 < 20; y0 += 3)
                {
                auto range = DRange3d::From(DPoint3d::From(x0, y0, 1), DPoint3d::From(x0 + 1.9, y0 + 2.5, 2));
                Check::SaveTransformedEdges(range);

                bvector<DPoint3d> clippedFace;
                if (clipper.IsAnyRangeFacePointInside(range, &clippedFace))
                    {
                    Check::Shift(0, 0, zShift);
                    Check::SaveTransformed(clippedFace, true);
                    Check::SaveTransformedEdges(range);
                    Check::Shift(0, 0, -zShift);
                    }
                }
            }
        }
    Check::ClearGeometry("Range3d.isAnyRangeFaceInsideC");
    }
bvector<DPoint3d> CreateStarPolygon(DPoint3dCR origin, double radiusA, double radiusB, int numPoints, bool includeClosurePoint = true)
    {
    if (numPoints < 2)
        numPoints = 2;
    double radiansStep = Angle::TwoPi () / numPoints;
    bvector<DPoint3d> points;
    for (int i = 0; i < numPoints; i++)
        {
        double thetaA = i * radiansStep;
        double thetaB = (i + 0.5) * radiansStep;
        points.push_back (DPoint3d::From (radiusA * cos(thetaA), radiusA * sin(thetaA), 0));
        points.push_back(DPoint3d::From(radiusB * cos(thetaB), radiusB * sin(thetaB), 0));
        }
    auto point0 = points[0];
    if (includeClosurePoint)
        points.push_back (point0);
    return points;
    }
void MarkClipStatus(ClipPlaneSet const & clipper, DPoint3dCR point, double tolerance)
    {
    bool status = clipper.IsPointOnOrInside(point, tolerance);
    double marker0 = 0.05;
    double marker1 = -0.05;
    Check::SaveTransformedMarker (point, status ? marker0 : marker1);
    }
TEST(ClipPlaneSet, InsideStar)
    {
    auto starPolygon = CreateStarPolygon (DPoint3d::From (0,0,0), 3.0, 1.0, 5, true);
    bvector<bvector<DPoint3d>> fragments;
    auto clipper = ClipPlaneSet::FromSweptPolygon (starPolygon.data(), starPolygon.size (), nullptr, &fragments);
    Check::SaveTransformed (starPolygon);
    starPolygon.pop_back ();
    Check::Shift (10,0,0);
    for (auto & f : fragments)
        {
        auto xyz = f.front ();
        f.push_back(xyz);
        Check::SaveTransformed(f);
        }
    for (double tolerance : {0.0, 0.00001, 0.05, -0.05, -0.00001})
        {
        Check::Shift (0,10,0);
        Check::SaveTransformed (starPolygon);
        for (auto &f : fragments)
            {
            bvector<DPoint3d> offsetFragment;
            PolylineOps::OffsetLineString(offsetFragment, f, tolerance, DVec3d::From(0, 0, 1), true, 3.1);
            Check::SaveTransformed (offsetFragment);
            }
        for (size_t i = 0; i < starPolygon.size(); i++)
            {
            size_t i1 = (i + 1) % starPolygon.size ();
            MarkClipStatus(clipper, starPolygon[i], tolerance);
            for (double segmentFraction : {0.3, 0.5, 0.7})
                {
                MarkClipStatus(clipper, DPoint3d::FromInterpolate(starPolygon[i], segmentFraction, starPolygon[i1]), tolerance);
                MarkClipStatus(clipper, DPoint3d::FromInterpolateAndPerpendicularXY(starPolygon[i], segmentFraction, starPolygon[i1], 0.02), tolerance);
                MarkClipStatus(clipper, DPoint3d::FromInterpolateAndPerpendicularXY(starPolygon[i], segmentFraction, starPolygon[i1], -0.02), tolerance);
                }

            size_t i2 = (i + 2) % starPolygon.size ();
            MarkClipStatus(clipper, DPoint3d::FromInterpolate(starPolygon[i], 0.5, starPolygon[i2]), tolerance);
            MarkClipStatus(clipper, DPoint3d::FromInterpolateAndPerpendicularXY(starPolygon[i], 0.4, starPolygon[i2], 0.02), tolerance);
            MarkClipStatus(clipper, DPoint3d::FromInterpolateAndPerpendicularXY(starPolygon[i], 0.6, starPolygon[i2], -0.02), tolerance);
            double a = 1.0 / 3.0;
            DPoint3d xyz = DPoint3d::FromSumOf(starPolygon[i], a, starPolygon[i1], a, starPolygon[i2], a);
            MarkClipStatus(clipper, xyz, tolerance);
            }
        }
    Check::ClearGeometry("ClipPlaneSet.InsideStar");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ClipWithParams)
    {
    DPoint3dDVec3dDVec3d plane (DPoint3d::From (0,0,0), DVec3d::From (2,0,0), DVec3d::From (1,2,0));
    int numX = 2;
    int numY = 2;
    auto mesh = Mesh_XYGrid(numX, numY, 2.0, 2.5, false, true);
    PolyfaceHeaderPtr insideClip;
    PolyfaceHeaderPtr outsideClip;

    ConvexClipPlaneSet convexClipper;
    bvector<DPoint3d> points{ DPoint3d::From(-1,-4,0), DPoint3d::From(1,-4,0), DPoint3d::From(5,3,0) };
    convexClipper.AddSweptPolyline(points, DVec3d::From(0, 0, 1), Angle::FromDegrees (0.0));
    ClipPlaneSet clipper;
    clipper.push_back(convexClipper);

    ClipPlaneSet::ClipPlaneSetIntersectPolyface(
        *mesh, clipper, false,
        &insideClip, &outsideClip);
    Check::SaveTransformed (points);
    Check::SaveTransformed (*mesh);
    Check::Shift (0,20,0);
    Check::SaveTransformed (*insideClip);
    Check::SaveTransformed(*outsideClip);
    Check::ClearGeometry("Polyface.ClipWithParams");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, StrayLinestringClip)
    {
    auto shape = CurveVector::CreateLinear(
        {{0,0,0},{5,0,0},{5,10,0},{0,10,0},{0,0,0} }, CurveVector::BOUNDARY_TYPE_Outer);
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    if (shape.IsValid())
        polyfaceBuilder->AddRegion(*shape);
    auto polyface = polyfaceBuilder->GetClientMeshPtr ();


    ConvexClipPlaneSet convexClipper;
    bvector<DPoint3d> points{
                {0.947914, 1.15212, 0},
                {4.561364, 1.15212, 0},
                {4.561364, 8.936802, 0},
                {0.947914, 8.936802, 0},
                {0.947914, 1.15212, 0}
            };
    convexClipper.AddSweptPolyline(points, DVec3d::From(0, 0, 1), Angle::FromDegrees(0.0));
    ClipPlaneSet clipper;
    clipper.push_back(convexClipper);

    Check::SaveTransformed(points);
    Check::Shift (0,20,0);
    Check::SaveTransformed(*polyface);
    Check::Shift (0,10,0);
    ValidatedDouble tolerance = 1.0e-11;
    for (auto formInteriorFaces : {false})
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        PolyfaceHeaderPtr insideClip;
        PolyfaceHeaderPtr outsideClip;
        ClipPlaneSet::ClipPlaneSetIntersectPolyface(
            *polyface, clipper, formInteriorFaces,
            &insideClip, &outsideClip);
        Check::Shift(0, 20, 0);
        Check::SaveTransformed(*insideClip);
        Check::Shift(0, 20, 0);
        Check::SaveTransformed(*outsideClip);
        PolyfaceHeaderPtr cutSections;
        bvector<bvector<DPoint3d>> linestrings;
        ClipPlaneSet::ClipPlaneSetSectionPolyface(
            *polyface, clipper,
            &cutSections, &linestrings,
            tolerance
            );
        Check::Shift(0, 20, 0);
        Check::SaveTransformed(*cutSections);
        Check::Shift(0, 20, 0);
        Check::SaveTransformed(linestrings);
        bvector<bvector<DPoint3d>> linestringsB;
        ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges (*polyface, clipper, linestringsB);
        Check::Shift(0, 40, 0);
        Check::SaveTransformed(linestringsB);
        }

    Check::ClearGeometry("Polyface.StrayLinestringClip");
    }
bool ReadDgnjsGeometry(bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC);
TEST(geomodeler, Clip)
    {
    bvector<DPoint3d> clipUVW {
        DPoint3d::From (0.1, 0.1, 0),
        DPoint3d::From (0.8, 0.1, 0),
        DPoint3d::From (0.5, 0.4, 0),
        DPoint3d::From (0.5, 0.8, 0),
        DPoint3d::From(0.1, 0.9, 0),
        DPoint3d::From (0.1, 0.1, 0)
        };
    // EDL 12/20
    // These are long running.
    // Keep 1 and 5
    // 1 because it has a few tiny manifold glitches.
    // 5 because it's thick and has awkward sliver triangles that should be addressed someday.
    for (auto filename: {
        // L"assembledMesh0.imjs",
        L"assembledMesh1.imjs",
        // L"assembledMesh2.imjs",
        // L"assembledMesh3.imjs",
        // L"assembledMesh4.imjs",
        L"assembledMesh5.imjs"
        //        , L"assembledMesh6.imjs"
        })
        {
        bvector<IGeometryPtr> originalGeometry;
        ReadDgnjsGeometry(originalGeometry, 1, L"Polyface", L"1220geomodeler", filename);
        // (expect a single mesh -- but loop over the array)
        for (auto &g : originalGeometry)
            {
            auto mesh = g->GetAsPolyfaceHeader();
            if (mesh.IsValid())
                {
                auto range = mesh->PointRange ();
                double shiftY = 1.1 * range.YLength ();
                double shiftX = 1.1 * range.XLength();
                SaveAndRestoreCheckTransform shifter (6 * shiftX, 0, 0);
                Check::SaveTransformed (mesh);
                DPoint3d corners[8];
                range.Get8Corners (corners);
                ConvexClipPlaneSet diagonalClip;
                diagonalClip.AddSweptPolyline(
                        {corners[2], corners[1]},
                        DVec3d::From (0,0,1),
                        Angle::FromDegrees (0));
                ClipPlaneSet clipper;
                clipper.push_back (diagonalClip);
                bvector<bvector<DPoint3d>> cutLines0;
                ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges(*mesh, clipper, cutLines0);
                Check::Shift(0, shiftY, 0);
                Check::SaveTransformed(cutLines0);
                bool isClosed = mesh->IsClosedByEdgePairing ();
                Check::True (isClosed, "closed mesh inputs to clipping");
                // printf ("  %ls  (closed %d)\n", filename, isClosed);

                PolyfaceHeaderPtr insideClip;
                PolyfaceHeaderPtr outsideClip;
                ClipPlaneSet::ClipPlaneSetIntersectPolyface(
                    *mesh, clipper, isClosed,
                    &insideClip, &outsideClip);
                Check::Shift(shiftX, 0);
                Check::SaveTransformed(*insideClip);
                Check::Shift(shiftX, 0, 0);
                Check::SaveTransformed(*outsideClip);

                bvector<DPoint3d> clipXYZ;
                for (auto &uvw : clipUVW)
                    clipXYZ.push_back (range.LocalToGlobal (uvw.x, uvw.y, uvw.z));
                Check::Shift(-3.0 * shiftX, 0, 0);
                for (double zz : {1.0})
                    {
                    auto zVector = DVec3d::From(0, 0, zz);
                    auto clipperB = ClipPlaneSet::FromSweptPolygon(clipXYZ.data (), clipXYZ.size (), &zVector);
                    PolyfaceHeaderPtr insideClipB;
                    ClipPlaneSet::ClipPlaneSetIntersectPolyface(
                        *mesh, clipperB, true,
                        &insideClipB, nullptr);
                    Check::Shift(0, shiftY, 0);
                    Check::SaveTransformed(*insideClipB);
                        {
                        SaveAndRestoreCheckTransform shifter2 (0,0,0);
                        Check::Shift (0,0,10);
                        Check::SaveTransformed (clipXYZ);
                        }
                    Check::Shift (shiftX, 0, 0);
                    bvector<bvector<DPoint3d>> cutLines1;
                    ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges(*mesh, clipperB, cutLines1);
                    Check::SaveTransformed (cutLines1);
                    double tolerance = 1.0e-4 * shiftX;
                    bvector<bvector<DPoint3d>> cutLinesB;
                    for (auto &xyz : cutLines1)
                        {
                        bvector<DPoint3d> xyzB;
                        DPoint3dOps::CompressByChordError(xyzB, xyz, tolerance);
                        cutLinesB.push_back (xyzB);
                        }
                    Check::Shift (shiftX, 0, 0);
                    Check::SaveTransformed(cutLinesB);
                    }
                }
            }
        }
    Check::ClearGeometry("Polyface.geomodelerClip");
    }

TEST(PolyfaceClip, ClipConeMesh)
    {
    // import uncapped cone
    static Utf8Chars s_input(u8R"foo([{"cylinder":{"capped":false,"end":[1116.5833333333335,449.01041666666680,14.583333333333284],"radius":1.0416666666666672,"start":[1116.5833333333335,428.01041666666680,14.583333333333284]}}])foo");
    bvector<IGeometryPtr> geometry;
    Check::True(IModelJson::TryIModelJsonStringToGeometry(&s_input, geometry), "converted json");
    Check::True(geometry.size() >= 1, "converted at least one geometry");
    Check::True(geometry[0].IsValid(), "converted geometry is valid");
    Check::True(geometry[0]->GetAsISolidPrimitive().IsValid(), "converted geometry is a solid primitive");
    Check::True(SolidPrimitiveType_DgnCone == geometry[0]->GetAsISolidPrimitive()->GetSolidPrimitiveType(), "converted geometry is a cone");
    auto cone = geometry[0]->GetAsISolidPrimitive();

    double chordTolerance = 0.0024660693027594815;

    // mesh the cone
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetChordTolerance(chordTolerance);
    options->SetAngleTolerance(msGeomConst_piOver2);
    options->SetMaxPerFace(100);
    options->SetConvexFacetsRequired(true);
    options->SetCurvedSurfaceMaxPerFace(3);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    Check::True(builder->AddSolidPrimitive(*cone), "meshed the cone");
    auto mesh = builder->GetClientMeshPtr();
    Check::True(mesh.IsValid() && mesh->HasFacets(), "mesh has facets");
    Check::SaveTransformed(mesh);

    // generate clipper
    DRange3d range; // the cone is axis-aligned, so this is tight
    Check::True(cone->GetRange(range), "computed cone range");
    range.ScaleAboutCenter(range, 1.1);     // expand range box away from the cone
    range.low.y += range.YLength() / 2;     // clip off the cone front half
    range.high.z -= range.ZLength() / 2;    // clip off the cone top half
    ConvexClipPlaneSet clip(range);
    ClipPlaneSet clipper(clip);
    Check::Size(clipper.size(), 1, "clip set has one set of planes");
    Check::Size(clipper[0].size(), 6, "first convex volume has 6 planes");
    int iVerticalPlane = -1;
    int iHorizontalPlane = -1;
    for (int i = 0; i < 6; ++i) {
        if (clipper[0][i].GetNormal().IsEqual(DVec3d::From(0, 1, 0)))
            iVerticalPlane = i;
        else if (clipper[0][i].GetNormal().IsEqual(DVec3d::From(0, 0, -1)))
            iHorizontalPlane = i;
    }
    Check::True(iVerticalPlane > -1 && iHorizontalPlane > -1, "found expected planes of interest");

    // clip the cone: not recommended on open meshes: these cutFacets are bogus
    auto defaultColinearEdgeTol = ValidatedDouble(2.47e-5);
    PolyfaceHeaderPtr cutFacets;
    bvector<bvector<DPoint3d>> lineStrings0;
    ClipPlaneSet::ClipPlaneSetSectionPolyface(*mesh, clipper, &cutFacets, &lineStrings0, defaultColinearEdgeTol);
    Check::SaveTransformed(cutFacets);
    Check::SaveTransformed(lineStrings0);
    Check::True(cutFacets.IsValid() && cutFacets->HasFacets(), "section has facet(s)");

    // another reason why sectioning an open mesh is ill-advised: some linestrings are missing
    if (Check::Size(1, lineStrings0.size(), "lineStrings0 is nonempty"))
        {
        // verify that the linestring only contains points on the faceted loop
        for (auto const& edgePt : lineStrings0[0])
            {
            bool edgePtIsSectionPt = false;
            for (auto const& facetPt : cutFacets->Point())
                {
                if (edgePt.AlmostEqual(facetPt))
                    {
                    edgePtIsSectionPt = true;
                    break;
                    }
                }
            Check::True(edgePtIsSectionPt, "linestring pts are section vertices");
            }
        }

    // verify that the edges added by triangulation are hidden
    size_t numHalfEdges = 0, numInteriorEdges = 0, numBoundaryEdges = 0, numDoubledEdges = 0, numTrebledEdges = 0, numQuadrupledEdges = 0, numQuintupledPlusEdges = 0, numDegenerateEdges = 0, numHiddenVertices = 0, numVisibleVertices = 0;
    cutFacets->CountSharedEdges(numHalfEdges, numInteriorEdges, numBoundaryEdges, numDoubledEdges, numTrebledEdges, numQuadrupledEdges, numQuintupledPlusEdges, numDegenerateEdges, false, numHiddenVertices, numVisibleVertices);
    size_t numVertices = 0, numFacets = 0, numQuads = 0, numTriangles = 0, numImplicitTriangles = 0, numVisibleEdges = 0, numHiddenEdges = 0;
    cutFacets->CollectCounts(numVertices, numFacets, numQuads, numTriangles, numImplicitTriangles, numVisibleEdges, numHiddenEdges);
    Check::Size(numBoundaryEdges, numVisibleEdges, "the only visible edges are those on the section boundary");
    Check::Size(numImplicitTriangles, numFacets, "expect section facets are all triangles");

    bvector<bvector<DPoint3d>> lineStrings1;
    ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges(*mesh, clipper, lineStrings1);
    Check::SaveTransformed(lineStrings1);

    if (Check::Size(1, lineStrings1.size(), "intersectEdges returned a single linestring"))
        {
        Transform l2w, w2l;
        Check::False(CurveVector::CreateLinear(lineStrings1[0])->IsPlanar(l2w, w2l, range), "cut edge linestring is not planar");

        // verify that ClipPlaneSetPolyfaceIntersectionEdges produces edges on both cutplanes
        for (auto const& edgePt : lineStrings1[0])
            {
            bool edgePtIsOnVerticalPlane = clipper[0][iVerticalPlane].IsPointOn(edgePt, Angle::SmallAngle());
            bool edgePtIsOnHorizontalPlane = clipper[0][iHorizontalPlane].IsPointOn(edgePt, Angle::SmallAngle());
            Check::True(edgePtIsOnHorizontalPlane || edgePtIsOnVerticalPlane, "linestring point is on vertical or horizontal cut plane");
            }
        }

    // verify that ClipPlaneSetSectionPolyface and ClipPlaneSetPolyfaceIntersectionEdges produce the same points along the cut facets
    for (auto const& facetPt : cutFacets->Point())
        {
        bool facetPtIsLineStringPt = false;
        for (auto const& edgePt : lineStrings1[0])
            {
            if (facetPt.AlmostEqual(edgePt))
                {
                facetPtIsLineStringPt = true;
                break;
                }
            }
        Check::True(facetPtIsLineStringPt, "cut facet mesh points are on cut edge linestring");
        }

    // verify that section method lineStrings0 points are a subset of the full edge method lineStrings1 points
    auto lineString1 = ICurvePrimitive::CreateLineString(lineStrings1[0]);
    for (auto const& lineString : lineStrings0)
        {
        for (size_t i = 0; i + 1 < lineString.size(); ++i)
            {
            CurveLocationDetail detail;
            if (Check::True(lineString1->ClosestPointBounded(lineString[i], detail)))
                Check::True(DoubleOps::AlmostEqual(detail.a, 0.0), "pt0 on lineString1");
            if (Check::True(lineString1->ClosestPointBounded(lineString[i + 1], detail)))
                Check::True(DoubleOps::AlmostEqual(detail.a, 0.0), "pt1 on lineString1");
            }
        }

    Check::ClearGeometry("PolyfaceClip.ClipConeMesh");
    }

TEST(Polyface, SweptPolygonClip)
    {
    struct TestCase
        {
        BeFileName          m_fileName;
        bvector<DPoint3d>   m_polygon;
        DVec3d              m_sweepVector;
        size_t              m_maxBoundaryEdges;
        double              m_volume;
        TestCase(BeFileNameCR fileName, bvector<DPoint3d> const& polygon, DVec3dCR sweepVector, size_t maxBoundaryEdges, double volume): m_fileName(fileName), m_polygon(polygon), m_sweepVector(sweepVector), m_maxBoundaryEdges(maxBoundaryEdges), m_volume(volume) {}
        };
    bvector<TestCase> testCases;    // all files and polygons in meters, translated to 1st octant

    BeFileName sourcePath;
    BeTest::GetHost().GetDocumentsRoot(sourcePath);
    sourcePath.AppendToPath(L"GeomLibsTestData").AppendToPath(L"Polyface").AppendToPath(L"Clipping").AppendToPath(L"mesh18K_almost_closed.imjs");

    bvector<DPoint3d> polygon;
    polygon.push_back(DPoint3d::From(220, 216.554504715461, 19.4880999999996));
    polygon.push_back(DPoint3d::From(-10, 216.554504715461, 19.4880999999996));
    polygon.push_back(DPoint3d::From(-10, 216.554504715461, 15.4880999999996));
    polygon.push_back(DPoint3d::From(220, 216.554504715461, 15.4880999999996));
    testCases.push_back(TestCase(sourcePath, polygon, DVec3d::From(0,-1,0), 293, 83448.505341041062));

    if (Check::GetEnableLongTests())
        {
        BeTest::GetHost().GetDocumentsRoot(sourcePath);
        sourcePath.AppendToPath(L"GeomLibsTestData").AppendToPath(L"Polyface").AppendToPath(L"Clipping").AppendToPath(L"mesh300K_almost_closed.imjs");

        polygon.clear();
        polygon.push_back(DPoint3d::From(1150, 1677.88169069408, 64.6504000000004));
        polygon.push_back(DPoint3d::From(-10,  1677.88169069408, 64.6504000000004));
        polygon.push_back(DPoint3d::From(-10,  1677.88169069408, 60.6504000000004));
        polygon.push_back(DPoint3d::From(1150, 1677.88169069408, 60.6504000000004));
        testCases.push_back(TestCase(sourcePath, polygon, DVec3d::From(0,-1,0), 354, 939666.54336569679));
        }

    for (auto const& testCase : testCases)
        {
        bvector<IGeometryPtr> geometry;
        if (!Check::True(GTestFileOps::JsonFileToGeometry(testCase.m_fileName, geometry), "Parse inputs"))
            continue;
        auto mesh = geometry.front()->GetAsPolyfaceHeader();
        if (!Check::True(mesh.IsValid(), "Have mesh"))
            continue;
        if (!Check::True(mesh->HasFacets(), "Nonempty mesh"))
            continue;
        Check::False(mesh->IsClosedByEdgePairing(), "input mesh is not closed");

        auto time0 = BeTimeUtilities::QueryMillisecondsCounter();

        PolyfaceHeaderPtr inside;
        ClipPlaneSet::SweptPolygonClipPolyface(*mesh, testCase.m_polygon, testCase.m_sweepVector, true, &inside, nullptr);

        auto time1 = BeTimeUtilities::QueryMillisecondsCounter();
        printf("  SweptPolygonClipPolyface took: %d ms\n", (int) (time1 - time0));

        if (Check::True(inside.IsValid(), "clip succeeded"))
            {
            Check::SaveTransformed(mesh);
            Check::SaveTransformed(inside);
            // verify some metrics, allowing for future perturbations
            auto vv = inside->ValidatedVolume();
            double volume = 0.0;
            bool tolerant = false;
            if (vv.IsValid())
                {
                volume = vv.Value();
                }
            else
                {
                bvector<FacetEdgeDetail> boundaryEdges;
                inside->CollectEdgeMateData(boundaryEdges);
                std::for_each(boundaryEdges.begin(), boundaryEdges.end(), [](auto const& edge) { Check::SaveTransformed(edge.segment); });
                Check::True(boundaryEdges.size() <= testCase.m_maxBoundaryEdges, "clip boundary edge count is bounded above");
                DPoint3d centroid;
                RotMatrix axes;
                DVec3d moments;
                inside->ComputePrincipalMomentsAllowMissingSideFacets(volume, centroid, axes, moments, true);
                tolerant = true;
                }
            Check::True(volume > 0.0, tolerant ? "clip is volumetric (tolerant)" : "clip is volumetric");
            Check::PushTolerance(ToleranceSelect::ToleranceSelect_Loose);
            Check::Near(volume, testCase.m_volume, "clip volume roughly as expected");
            Check::PopTolerance();
            }
        }

    Check::ClearGeometry("Polyface.SweptPolygonClip");
    }
