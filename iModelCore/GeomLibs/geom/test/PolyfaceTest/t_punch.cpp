/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_punch.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PunchOneSidedDTM)
    {
    // Sort of a dtm ...
    size_t numX = 15;
    size_t numY = 20;
    auto dtm = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.3, 0.0, -0.25);
    dtm->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    Check::SaveTransformed (*dtm);

    // a cutter .....
    double z = 5.0;
    bvector<DPoint3d> points {
            DPoint3d::From (1,2,z),
            DPoint3d::From (8,2,z),
            DPoint3d::From (8,4,z),
            DPoint3d::From (9,7,z),
            DPoint3d::From (10,8,z),
            DPoint3d::From (10,10,z),
            DPoint3d::From (2,10,z),
            DPoint3d::From (1,2,z)};
    PolyfaceHeaderPtr puncher = PolyfaceHeader::CreateVariableSizeIndexed();
    puncher->AddPolygon (points);

    Check::SaveTransformed (*puncher);

    Check::Shift (0,40,0);
    PolyfaceHeaderPtr inside, outside;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*puncher, *dtm, &inside, &outside);
    Check::SaveTransformed (*inside);

    Check::Shift (0,40,0);
    Check::SaveTransformed (*outside);
    
    Check::ClearGeometry ("Polyface.PunchOneSidedDTM");
    }
#ifdef BuildThickMesh
PolyfaceHeaderPtr buildThickMesh (
int numX,       // grid count in the x direction
int numY,       // grid count in the y direction
double deltaZ   // Z shift to apply from first mesh to second.
)
    {
    auto dtm1 = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.3, 0.0, -0.25);
    auto dtm2 = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.4, 0.0, -0.5);
    if (deltaZ < 0.0)
        dtm2->ReverseIndicesAllFaces ();
    else
        dtm1->ReverseIndicesAllFaces ();
    dtm2->Transform (Transform::From (DVec3d::From (0,0,deltaZ)));

    dtm1->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    dtm2->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);

    builder->AddPolyface (*dtm1);
    builder->AddPolyface (*dtm2);
    return builder->GetClientMeshPtr ();
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PunchThickSurface)
    {

    // A thick mesh with a dtm-like surface
    size_t numX = 15;
    size_t numY = 20;
    auto thickMesh = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.3, 0.0, -0.25);
    thickMesh->SweepToSolid (DVec3d::From (0,0, 2), true);

    Check::SaveTransformed (*thickMesh);

    // a cutter .....
    double z = 5.0;
    bvector<DPoint3d> points {
            DPoint3d::From (1,2,z),
            DPoint3d::From (8,2,z),
            DPoint3d::From (8,4,z),
            DPoint3d::From (9,7,z),
            DPoint3d::From (10,8,z),
            DPoint3d::From (10,10,z),
            DPoint3d::From (2,10,z),
            DPoint3d::From (1,2,z)};
    PolyfaceHeaderPtr puncher = PolyfaceHeader::CreateVariableSizeIndexed();
    puncher->AddPolygon (points);

    Check::SaveTransformed (*puncher);
    Check::Shift (20, 0,0);

    PolyfaceHeaderPtr insideMesh, outsideMesh;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*puncher, *thickMesh, &insideMesh, &outsideMesh);

    {
    SaveAndRestoreCheckTransform shifter (20,0,0);
    PolyfaceHeaderPtr healedInside;
    PolyfaceHeader::HealVerticalPanels (*insideMesh, true, false, healedInside);

    Check::SaveTransformed (*insideMesh);
    Check::Shift (0,30,0);

    Check::SaveTransformed (*healedInside);
    }


    {
    SaveAndRestoreCheckTransform shifter (20,0,0);
    PolyfaceHeaderPtr healedOutside;
    PolyfaceHeader::HealVerticalPanels (*outsideMesh, true, false, healedOutside);

    Check::SaveTransformed (*outsideMesh);
    Check::Shift (0,30,0);

    Check::SaveTransformed (*healedOutside);
    }
    
    Check::ClearGeometry ("Polyface.PunchThickSurface");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PunchSymmetric)
    {
    bvector<DPoint3d> targetPoints {{2,0,0},{0,2,0},{-2,0,0},{0,-2,0}};
    bvector<DPoint3d> punchPoints {{5,0,0},{3,2,0},{1,0,0},{3,-2,0}};

    PolyfaceHeaderPtr punch = PolyfaceHeader::CreateVariableSizeIndexed();
    PolyfaceHeaderPtr target = PolyfaceHeader::CreateVariableSizeIndexed();

    target->AddPolygon(targetPoints);
    punch->AddPolygon(punchPoints);

    PolyfaceHeaderPtr outside, inside;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*punch, *target, &inside, &outside);
    Check::SaveTransformed (target);
    Check::SaveTransformed (punch);
    Check::Shift (0, 5, 0);
    Check::SaveTransformed (inside);
    Check::SaveTransformed (outside);
    Check::ClearGeometry ("Polyface.PunchSymmetric");
    }