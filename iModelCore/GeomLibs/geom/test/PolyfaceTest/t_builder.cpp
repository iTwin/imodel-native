/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

IPolyfaceConstructionPtr CreateBuilder (bool normals, bool params);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,SingularRotation)
    {
    // arcs with start and end on vector90
    for (uint32_t maxPerFace : {3, 100})
        {
        SaveAndRestoreCheckTransform shifter1 (0, 20,0);
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
            SaveAndRestoreCheckTransform shifter2 (20,0,0);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,SingularRotationB)
    {
    // arcs with start and end on vector90
    double f = 1.0 / 10000.0;
    double f0 = 0.0;            // ZERO to move center to origin . ..
    double step = 3323000.0000000014 * f * 4.0;
    for (uint32_t maxPerFace : {3, 100})
        {
        SaveAndRestoreCheckTransform shifter1 (0, 4.0 * step,0);
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
            SaveAndRestoreCheckTransform shifter2 (step,0,0);
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

TEST(PolyfaceBuilder, PolygonEdgeStitch)
        {
        bvector<DPoint3d> polygonA {
            DPoint3d::From (0,0,0),
            DPoint3d::From (1,0,0),
            DPoint3d::From (1,1,0),
            DPoint3d::From (0,1,0)};
        // This is bad order and has a T Vertex...
        bvector<DPoint3d> polygonB{
            DPoint3d::From(1,0,0),
            DPoint3d::From(1,2,0),
            DPoint3d::From(2,1,1),
            DPoint3d::From(2,0,1)
            };

            {
            Check::Shift (4,-2,0);
            auto meshB = PolyfaceHeader::CreateVariableSizeIndexed();
            // IMPORTANT -- do not include "closure" vertex in the polygons.
            meshB->AddPolygon (polygonA);
            meshB->AddPolygon (polygonB);
            meshB->Compress ();
            Check::SaveTransformed(meshB);
            auto meshC = meshB->CloneWithTVertexFixup({ meshB });\

            // fixup clockwise/counterclockwise mix ..
            bvector<bvector<size_t>> componentReadIndices;
            MeshAnnotationVector messages(true);
            meshC->OrientAndCollectManifoldComponents (componentReadIndices, messages);

            Check::Shift(0, 2, 0);
            meshC->MarkTopologicalBoundariesVisible(false);
            Check::SaveTransformed(meshC);
            size_t numOpen;
            size_t numClosed;
            auto boundaryC = meshC->ExtractBoundaryStrings(numOpen, numClosed);
            Check::Shift(0, 2, 0);
            Check::SaveTransformed(boundaryC);
            }
        Check::ClearGeometry ("PolyfaceBuilder.PolygonEdgeStitch");

        }

TEST(PolyfaceBuilder, RegionRotationalSweep)
    {
    CurveVectorPtr solid = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateRectangle(1, 1, 6, 6, 0));
    CurveVectorPtr hole0 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateRectangle(2, 2, 3, 3, 0));
    CurveVectorPtr hole1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromCenterRadiusXY(DPoint3d::From(4.5, 4.5), 0.5)));
    CurveVectorPtr unionRegion = CurveVector::AreaUnion(*hole0, *hole1);
    CurveVectorPtr parityRegion = CurveVector::AreaDifference(*solid, *unionRegion);

    IFacetOptionsPtr options = IFacetOptions::CreateForSurfaces();
    options->SetNormalsRequired(true);
    options->SetParamsRequired(true);
    options->SetAngleTolerance(0.1);
    options->SetParamMode(FACET_PARAM_01BothAxes);

    DRay3d axis = DRay3d::FromOriginAndVector(DPoint3d::FromZero(), DVec3d::From(1,0,0));
    double sweep = Angle::PiOver2();
    bool capped = true;

    if (true)
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
        builder->AddRotationalSweep(unionRegion, axis.origin, axis.direction, sweep, capped);
        Check::SaveTransformed(DSegment3d::From(axis.origin, axis.FractionParameterToPoint(2.0)));
        Check::SaveTransformed(*unionRegion);
        Check::SaveTransformed(*builder->GetClientMeshPtr());
        }

    Check::Shift(10, 0);

    if (true)
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
        builder->AddRotationalSweep(parityRegion, axis.origin, axis.direction, sweep, capped);
        Check::SaveTransformed(DSegment3d::From(axis.origin, axis.FractionParameterToPoint(2.0)));
        Check::SaveTransformed(*parityRegion);
        Check::SaveTransformed(*builder->GetClientMeshPtr());
        }

    Check::ClearGeometry("PolyfaceBuilder.RegionRotationalSweep");
    }
