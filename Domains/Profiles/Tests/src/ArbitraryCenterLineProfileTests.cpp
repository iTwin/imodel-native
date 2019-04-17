/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryCenterLineProfileTestCase : ProfileValidationTestCase<ArbitraryCenterLineProfile>
    {
public:
    typedef ArbitraryCenterLineProfile::CreateParams CreateParams;

    /*---------------------------------------------------------------------------------**//**
    * Geometry composed of single ICurvePrimitive - Half Pipe shape.
    * @bsimethod                                                                     02/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    IGeometryPtr CreateSingleCurveGeometry() const
        {
        DEllipse3d halfArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (-1.0, 1.0), DPoint3d::From (0.0, 0.0), DPoint3d::From (1.0, 1.0));
        ICurvePrimitivePtr curvePtr = ICurvePrimitive::CreateArc (halfArc);

        return IGeometry::Create (curvePtr);
        }

    /*---------------------------------------------------------------------------------**//**
    * Geometry composed of multiple (two) ICurvePrimitives forming a CurveVector - S shape.
    * @bsimethod                                                                     02/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    IGeometryPtr CreateMultipleCurveGeometry() const
        {
        DEllipse3d topArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (2.0, 2.0), DPoint3d::From (-2.0, 2.0), DPoint3d::From (0.0, 0.0));
        ICurvePrimitivePtr topCurvePtr = ICurvePrimitive::CreateArc (topArc);

        DEllipse3d bottomArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (0.0, 0.0), DPoint3d::From (2.0, -2.0), DPoint3d::From (-2.0, -2.0));
        ICurvePrimitivePtr bottomCurvePtr = ICurvePrimitive::CreateArc (bottomArc);

        CurveVectorPtr curvesPtr = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        curvesPtr->Add (topCurvePtr);
        curvesPtr->Add (bottomCurvePtr);

        return IGeometry::Create (curvesPtr);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_InvalidProfileName_Fail)
    {
    CreateParams params (GetModel(), nullptr, CreateSingleCurveGeometry(), 0.5);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_ValidProfileName_Success)
    {
    CreateParams params (GetModel(), nullptr, CreateSingleCurveGeometry(), 0.5);

    params.name = "ArbitraryCenterLine";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_CurvePrimitiveGeometry_Success)
    {
    IGeometryPtr geometryPtr = CreateSingleCurveGeometry();
    ASSERT_EQ (IGeometry::GeometryType::CurvePrimitive, geometryPtr->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", geometryPtr, 0.5);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert with CurvePrimitive geometry type.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_CurveVectorGeometry_Success)
    {
    IGeometryPtr geometryPtr = CreateMultipleCurveGeometry();
    ASSERT_EQ (IGeometry::GeometryType::CurveVector, geometryPtr->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", geometryPtr, 0.5);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert with CurveVector geometry type.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_SolidPrimitiveGeometry_Fail)
    {
    ISolidPrimitivePtr solidPtr = ISolidPrimitive::CreateDgnSphere (DgnSphereDetail (DPoint3d {0}, 1.0));
    IGeometryPtr geometryPtr = IGeometry::Create (solidPtr);
    ASSERT_EQ (IGeometry::GeometryType::SolidPrimitive, geometryPtr->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", geometryPtr, 0.5);
    EXPECT_FAIL_Insert (params) << "Profile should fail to insert with SolidPrimitive geometry type";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_PolyfaceGeometry_Fail)
    {
    PolyfaceHeaderPtr headerPtr = PolyfaceHeader::CreateQuadGrid (5);
    IGeometryPtr geometryPtr = IGeometry::Create (headerPtr);
    ASSERT_EQ (IGeometry::GeometryType::Polyface, geometryPtr->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", geometryPtr, 0.5);
    EXPECT_FAIL_Insert (params) << "Profile should fail to insert with Polyface geometry type";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_BsplineSurfaceGeometry_Fail)
    {
    MSBsplineSurfacePtr bsplineSurfacePtr = MSBsplineSurface::CreateTrimmedDisk (DEllipse3d::FromCenterRadiusXY (DPoint3d {0}, 1.0));
    IGeometryPtr geometryPtr = IGeometry::Create (bsplineSurfacePtr);
    ASSERT_EQ (IGeometry::GeometryType::BsplineSurface, geometryPtr->GetGeometryType());

    CreateParams params (GetModel(), "ArbitraryCenterLine", geometryPtr, 0.5);
    EXPECT_FAIL_Insert (params) << "Profile should fail to insert with BsplineSurface geometry type";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCenterLineProfileTestCase, Insert_InvalidWallThickness_Fail)
    {
    CreateParams params (GetModel(), "ArbitraryCenterLine", CreateSingleCurveGeometry(), INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);
    }
