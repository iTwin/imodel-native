/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

// Return coordianates for a rectangle with corners x0,y0 and x1,y1, with interiorFlags all false.
void MakeRectangle (double x0, double y0, double x1, double y1, double z, bvector<DPoint3d> &points, bvector<bool> &interiorFlags)
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
void MakeParallelogram(DPoint3dCR origin, DVec3dCR xVector, DVec3dCR yVector, bvector<DPoint3d> &points, bvector<bool> &interiorFlags)
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
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipNGonSweeps)
    {
    for (int numSide : {12, 4,7,12})
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        for (auto pass : {-2, -1, 0,1, 2, 4})
            {
            SaveAndRestoreCheckTransform shifter (0,30,0);
            IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
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
                bvector<bool> interiorFlag;
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
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipOnEdge)
    {
    for (int clipPass : {0, 1, 2, 3})
        {
        SaveAndRestoreCheckTransform shifter (40,0,0);
        for (int meshPass : {0,1})
            {
            SaveAndRestoreCheckTransform shifter (20,0,0);
            for (double clipX : {0.9, 1.0})
                {
                SaveAndRestoreCheckTransform shifter (5,0,0);
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
* @bsimethod                                                     Earlin.Lutz  10/17
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
        bvector<bool> interiorFlag;
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
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipTunnel)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);


    // tunnel cap in xz plane . .
    bvector<DPoint3d> outerLoop, innerLoop;
    bvector<bool> outerVisible, innerVisible;
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
        bvector<bool> interiorFlag;
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
* @bsimethod                                                     Earlin.Lutz  10/17
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
    bvector<bool> outerVisible, innerVisible;
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
        SaveAndRestoreCheckTransform shifter (0,50,0);
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
            bvector<bool> interiorFlag;
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
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipPolygonPrism)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    builder->GetFacetOptionsR ().SetMaxEdgeLength (4);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);


    for (auto points : 
        {
        bvector<DPoint3d> {
            DPoint3d::From (1,1,0), DPoint3d::From (1,1,3), DPoint3d::From (1,2,3), DPoint3d::From (1,2,2)},
        bvector<DPoint3d> {
            DPoint3d::From(0,1,0), DPoint3d::From(0,1,3), DPoint3d::From(0,2,3), DPoint3d::From(0,2,2)}
    })
        {
        auto xyz0 = points.front ();
        points.push_back (xyz0);
        auto section = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
        Check::SaveTransformed (*section);
        Check::Shift (300, 0, 0);

        auto solid = ISolidPrimitive::CreateDgnExtrusion (
            DgnExtrusionDetail (section, DVec3d::From (20,0,0), true));

        builder->AddSolidPrimitive (*solid);
        PolyfaceHeaderR polyface = builder->GetClientMeshR ();
        DPoint3d clipOrigin = DPoint3d::From (0,0,0);

        for (auto sweepVector: {DVec3d::From (0,0,1), DVec3d::From (0,0,-1)})
            {
            SaveAndRestoreCheckTransform shifter (100,0,0);
            for (auto diagonal : {
                DVec3d::From (2,5),
                DVec3d::From (6,3)})
                {
                SaveAndRestoreCheckTransform shifter (50,0,0);
                for (bool reverseRectangle : {false, true})
                    {
                    SaveAndRestoreCheckTransform shifter(0, 40, 0);

                    bvector<DPoint3d> rectanglePoints;
                    bvector<bool> interiorFlag;
                    double ax = clipOrigin.x;
                    double ay = clipOrigin.y;
                    double bx = clipOrigin.x + diagonal.x;
                    double by = clipOrigin.y + diagonal.y;
                    MakeRectangle (ax, ay, bx, by, 0, rectanglePoints, interiorFlag);
                    if (reverseRectangle)
                        std::reverse (rectanglePoints.begin (), rectanglePoints.end ());
                    Check::SaveTransformed (rectanglePoints);
                    Check::SaveTransformed (polyface);

                    PolyfaceHeaderPtr insideClip, outsideClip;
                    ClipPlaneSet::SweptPolygonClipPolyface (
                            polyface,
                            rectanglePoints, sweepVector,
                            true,
                            &insideClip, &outsideClip);
                    Check::Shift (0,10,0);
                    if (insideClip.IsValid ())
                        Check::SaveTransformed (*insideClip);
                    Check::Shift (0,10,0);
                    if (outsideClip.IsValid ())
                        Check::SaveTransformed (*outsideClip);
                    }
                }
            }
        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    Check::ClearGeometry ("Polyface.ClipPolygonPrism");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
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
    bvector<bool> outerVisible, innerVisible;
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
