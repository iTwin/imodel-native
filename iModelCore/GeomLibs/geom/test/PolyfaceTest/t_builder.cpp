/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

IPolyfaceConstructionPtr CreateBuilder (bool normals, bool params);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,SingularRotation)
    {
    // arcs with start and end on vector90
    for (uint32_t maxPerFace : {3, 100})
        {
        SaveAndRestoreCheckTransform shifter (0, 20,0);
        for (auto arc : {
            DEllipse3d::FromVectors (
                DPoint3d::From (0,0,0),
                DVec3d::From (3,0,0),
                DVec3d::From (0,0,1),
                0.0, Angle::PiOver2 ()),
            DEllipse3d::FromVectors (
                DPoint3d::From (0,0,0),
                DVec3d::From (3,0,0),
                DVec3d::From (0,0,1),
                -Angle::PiOver2 (), Angle::PiOver2 ()),
            DEllipse3d::FromVectors (
                DPoint3d::From (0,0,0),
                DVec3d::From (3,0,0),
                DVec3d::From (0,0,1),
                -Angle::PiOver2 (), Angle::Pi ())
            })
            {
            SaveAndRestoreCheckTransform shifter (20,0,0);
            auto cp = ICurvePrimitive::CreateArc (arc);
            auto contour = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            contour->Add (cp);
            auto surface = ISolidPrimitive::CreateDgnRotationalSweep (
                    DgnRotationalSweepDetail (contour, arc.center, arc.vector90, Angle::TwoPi (), false));
            Check::SaveTransformed (*surface);
            auto builder = CreateBuilder (true, true);
            builder->GetFacetOptionsR ().SetMaxPerFace (maxPerFace);
            builder->AddSolidPrimitive (*surface);
            Check::Shift (0,10,0);
            Check::SaveTransformed (*builder->GetClientMeshPtr ());
            }   
        }

    Check::ClearGeometry ("PolyfaceBuilder.SingularRotation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,SingularRotationB)
    {
    // arcs with start and end on vector90
    double f = 1.0 / 10000.0;
    double f0 = 0.0;            // ZERO to move center to origin . ..
    double step = 3323000.0000000014 * f * 4.0;
    for (uint32_t maxPerFace : {3, 100})
        {
        SaveAndRestoreCheckTransform shifter (0, 4.0 * step,0);
        for (auto arc : {
            DEllipse3d::FromVectors (
                DPoint3d::From (30599004968.958866, 5965000000.0000000, 1029563429.9932430),
                DVec3d::From (-1.3911094498553216e-009, -6265000.0000000019, -6.3859036797319309e-012),
                DVec3d::From (-3323000.0000000014, 7.3785422216587935e-010, 0.00000000000000000),
                4.7123889803846897, 1.5707963267948957),
            DEllipse3d::FromVectors (
                DPoint3d::From (30599004968.958866, 5965000000.0000000, 1029563429.9932430),
                DVec3d::From (-1.3911094498553216e-009, -6265000.0000000019, -6.3859036797319309e-012),
                DVec3d::From (-3323000.0000000014, 7.3785422216587935e-010, 0.00000000000000000),
                Angle::DegreesToRadians (270), 1.5707963267948957),
            DEllipse3d::FromVectors (
                DPoint3d::From (30599004968.958866, 5965000000.0000000, 1029563429.9932430),
                DVec3d::From (-1.3911094498553216e-009, -6265000.0000000019, -6.3859036797319309e-012),
                DVec3d::From (-3323000.0000000014, 7.3785422216587935e-010, 0.00000000000000000),
                Angle::DegreesToRadians (-90), 1.5707963267948957),
            DEllipse3d::FromVectors (
                DPoint3d::From (30599004968.958866, 5965000000.0000000, 1029563429.9932430),
                DVec3d::From (-1.3911094498553216e-009, -6265000.0000000019, -6.3859036797319309e-012),
                DVec3d::From (-3323000.0000000014, 7.3785422216587935e-010, 0.00000000000000000),
                4.8, 1.0)
            })
            {
            arc.center.Scale (f0);      // BLAST it ... I don't think the huge uors are part of the problem.
            arc.center.Scale (f);
            arc.vector0.Scale (f);
            arc.vector90.Scale (f);
            SaveAndRestoreCheckTransform shifter (step,0,0);
            auto cp = ICurvePrimitive::CreateArc (arc);
            auto contour = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            contour->Add (cp);
            auto surface = ISolidPrimitive::CreateDgnRotationalSweep (
                    DgnRotationalSweepDetail (contour, arc.center, arc.vector90, Angle::TwoPi (), false));
            Check::SaveTransformed (*surface);
            auto builder = CreateBuilder (true, true);
            builder->GetFacetOptionsR ().SetMaxPerFace (maxPerFace);
            builder->AddSolidPrimitive (*surface);
            Check::Shift (0,step,0);
            Check::SaveTransformed (*builder->GetClientMeshPtr ());
            }
        }

    Check::ClearGeometry ("PolyfaceBuilder.SingularRotationB");
    }