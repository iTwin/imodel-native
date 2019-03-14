/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfileTestCase : ProfileValidationTestCase<Profile>
    {
    LShapeProfilePtr CreateProfile (Utf8CP pName = "")
        {
        LShapeProfile::CreateParams params (GetModel(), pName, 10.0, 6.0, 0.5);
        return LShapeProfile::Create (params);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, SetStandardCatalogCode_ValidValues_Success)
    {
    ProfilePtr profilePtr = CreateProfile();

    profilePtr->SetName ("test_designation");
    StandardCatalogCode catalogCode ("test_manufacturer", "test_organization", "test_revision");

    ASSERT_EQ (DgnDbStatus::Success, profilePtr->SetStandardCatalogCode (catalogCode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, SetStandardCatalogCode_ExistingCode_UpdatedCode)
    {
    ProfilePtr profilePtr = CreateProfile();
    StandardCatalogCode catalogCode ("test_manufacturer", "test_organization", "test_revision");

    profilePtr->SetName ("test_first");
    profilePtr->SetStandardCatalogCode (catalogCode);
    ASSERT_STREQ ("test_first", profilePtr->GetStandardCatalogCode().designation.c_str());

    profilePtr->SetName ("test_second");
    profilePtr->SetStandardCatalogCode (catalogCode);
    ASSERT_STREQ ("test_second", profilePtr->GetStandardCatalogCode().designation.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, Insert_StandardCatalogCodeSet_Success)
    {
    ProfilePtr profilePtr = CreateProfile();

    profilePtr->SetName ("D");
    profilePtr->SetStandardCatalogCode (StandardCatalogCode ("A", "B", "C"));

    DgnDbStatus status;
    profilePtr->Insert (&status);
    ASSERT_EQ (DgnDbStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProfileTestCase, Update_UpdatedProfile_UpdatedCardinalPoints)
    {
    LShapeProfilePtr profilePtr = CreateProfile("P");
    profilePtr->SetWidth(2.0);
    profilePtr->SetDepth(2.0);
    profilePtr->Insert();

    bvector<CardinalPoint> cardinalPoints = profilePtr->GetCardinalPoints();
    ASSERT_EQ(19, cardinalPoints.size()) << "Inserted profile should have 19 standard cardinal points";

    // Basic bounding box points
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[0].location.x);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[0].location.y);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[1].location.x);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[1].location.y);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[2].location.x);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[2].location.y);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[3].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[3].location.y);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[4].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[4].location.y);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[5].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[5].location.y);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[6].location.x);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[6].location.y);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[7].location.x);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[7].location.y);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[8].location.x);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[8].location.y);

    DPoint3d centroid;
    double area;
    ASSERT_TRUE(profilePtr->GetShape()->GetAsCurveVector()->CentroidAreaXY(centroid, area));

    // Geometric centroid points
    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[9].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[9].location.y);
    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[10].location.x);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[10].location.y);
    EXPECT_DOUBLE_EQ(-1.0, cardinalPoints[11].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[11].location.y);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[12].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[12].location.y);
    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[13].location.x);
    EXPECT_DOUBLE_EQ(1.0, cardinalPoints[13].location.y);

    // Shear points
    for (int i = 14; i < 19; ++i)
        {
        EXPECT_DOUBLE_EQ(0.0, cardinalPoints[i].location.x);
        EXPECT_DOUBLE_EQ(0.0, cardinalPoints[i].location.y);
        }

    // Update profile and check if profile points have updated
    profilePtr->SetWidth(4.0);
    profilePtr->SetDepth(4.0);
    profilePtr->Update();

    cardinalPoints = profilePtr->GetCardinalPoints();
    ASSERT_EQ(19, cardinalPoints.size()) << "Updated profile should have 19 standard cardinal points";

    // Basic bounding box points
    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[0].location.x);
    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[0].location.y);

    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[1].location.x);
    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[1].location.y);

    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[2].location.x);
    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[2].location.y);

    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[3].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[3].location.y);

    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[4].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[4].location.y);

    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[5].location.x);
    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[5].location.y);

    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[6].location.x);
    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[6].location.y);

    EXPECT_DOUBLE_EQ(0.0, cardinalPoints[7].location.x);
    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[7].location.y);

    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[8].location.x);
    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[8].location.y);

    ASSERT_TRUE(profilePtr->GetShape()->GetAsCurveVector()->CentroidAreaXY(centroid, area));

    // Geometric centroid points
    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[9].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[9].location.y);

    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[10].location.x);
    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[10].location.y);

    EXPECT_DOUBLE_EQ(-2.0, cardinalPoints[11].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[11].location.y);

    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[12].location.x);
    EXPECT_DOUBLE_EQ(centroid.y, cardinalPoints[12].location.y);

    EXPECT_DOUBLE_EQ(centroid.x, cardinalPoints[13].location.x);
    EXPECT_DOUBLE_EQ(2.0, cardinalPoints[13].location.y);

    // Shear points
    for (int i = 14; i < 19; ++i)
        {
        EXPECT_DOUBLE_EQ(0.0, cardinalPoints[i].location.x);
        EXPECT_DOUBLE_EQ(0.0, cardinalPoints[i].location.y);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, SetStandardCatalogCode_NullPointer_RemovedCode)
    {
    ProfilePtr profilePtr = CreateProfile();
    StandardCatalogCode catalogCode ("test_manufacturer", "test_organization", "test_revision");

    profilePtr->SetName ("test_first");
    profilePtr->SetStandardCatalogCode (catalogCode);
    ASSERT_STREQ ("test_first", profilePtr->GetStandardCatalogCode().designation.c_str());

    profilePtr->SetStandardCatalogCode (nullptr);
    ASSERT_STREQ ("", profilePtr->GetStandardCatalogCode().designation.c_str());
    ASSERT_EQ (nullptr, profilePtr->GetCode().GetValue().GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, SetStandardCatalogCode_InvalidValues_ErrorStatus)
    {
    ProfilePtr profilePtr = CreateProfile();

    StandardCatalogCode catalogCode ("test_manufacturer", "test_organization", "test_revision");

    profilePtr->SetName (nullptr);
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    profilePtr->SetName ("");
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    profilePtr->SetName ("test_designation");

    catalogCode.manufacturer = nullptr;
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    catalogCode.manufacturer = "";
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    catalogCode.standardsOrganization = nullptr;
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    catalogCode.standardsOrganization = "";
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    catalogCode.revision = nullptr;
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));

    catalogCode.revision = "";
    ASSERT_EQ (DgnDbStatus::InvalidCode, profilePtr->SetStandardCatalogCode (catalogCode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetStandardCatalogCode_NoCode_EmptyValues)
    {
    ProfilePtr profilePtr = CreateProfile();
    profilePtr->SetName ("test");

    StandardCatalogCode catalogCode = profilePtr->GetStandardCatalogCode();
    ASSERT_STREQ ("", catalogCode.manufacturer.c_str());
    ASSERT_STREQ ("", catalogCode.standardsOrganization.c_str());
    ASSERT_STREQ ("", catalogCode.revision.c_str());
    ASSERT_STREQ ("", catalogCode.designation.c_str());
    ASSERT_EQ (nullptr, profilePtr->GetCode().GetValue().GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetStandardCatalogCode_ExistingCode_ValidValues)
    {
    ProfilePtr profilePtr = CreateProfile();
    profilePtr->SetName ("test_designation");
    profilePtr->SetStandardCatalogCode (StandardCatalogCode ("test_manufacturer", "test_organization", "test_revision"));

    StandardCatalogCode catalogCode = profilePtr->GetStandardCatalogCode();
    ASSERT_STREQ ("test_manufacturer", catalogCode.manufacturer.c_str());
    ASSERT_STREQ ("test_organization", catalogCode.standardsOrganization.c_str());
    ASSERT_STREQ ("test_revision", catalogCode.revision.c_str());
    ASSERT_STREQ ("test_designation", catalogCode.designation.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetStandardCatalogCode_LoadedInstance_ValidValues)
    {
    ProfilePtr profilePtr = CreateProfile();
    profilePtr->SetName ("test_designation");
    profilePtr->SetStandardCatalogCode (StandardCatalogCode ("test_manufacturer", "test_organization", "test_revision"));

    profilePtr->Insert();
    DgnElementId profileId = profilePtr->GetElementId();

    GetDb().Elements().ClearCache();
    ProfileCPtr loadedProfilePtr = Profile::Get (GetDb(), profileId);

    StandardCatalogCode catalogCode = loadedProfilePtr->GetStandardCatalogCode();
    ASSERT_STREQ ("test_manufacturer", catalogCode.manufacturer.c_str());
    ASSERT_STREQ ("test_organization", catalogCode.standardsOrganization.c_str());
    ASSERT_STREQ ("test_revision", catalogCode.revision.c_str());
    ASSERT_STREQ ("test_designation", catalogCode.designation.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCode_ExistingCode_CorrectFormatString)
    {
    ProfilePtr profilePtr = CreateProfile();
    profilePtr->SetName ("D");
    profilePtr->SetStandardCatalogCode (StandardCatalogCode ("A", "B", "C"));

    Utf8CP pCodeValue = profilePtr->GetCode().GetValue().GetUtf8CP();
    ASSERT_STREQ ("A B C D", pCodeValue) << "StandardCatalogProfile CodeValue should be of format: '<Manufacturer>:<StandardsOrganization>:<Revision>:<Designation>'";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoints_NonInsertedInstance_EmptyVector)
    {
    ProfilePtr profilePtr = CreateProfile ("P");

    ASSERT_EQ (0, profilePtr->GetCardinalPoints().size()) << "Newly created, not inserted profile should have 0 CardinalPoints";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoints_NonInsertedInstanceOneCustomPoint_OneItem)
    {
    ProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->AddCustomCardinalPoint (CardinalPoint ("test", DPoint2d()));

    ASSERT_EQ (1, profilePtr->GetCardinalPoints().size()) << "Newly created, not inserted profile should have only user added custom CardinalPoints";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoints_InsertedInstance_19StandardPoints)
    {
    LShapeProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->SetWidth (2.0);
    profilePtr->SetDepth (2.0);
    profilePtr->Insert();

    bvector<CardinalPoint> cardinalPoints = profilePtr->GetCardinalPoints();
    ASSERT_EQ (19, cardinalPoints.size()) << "Inserted profile should have 19 standard cardinal points";

    EXPECT_STREQ ("BottomLeft", cardinalPoints[0].name.c_str());
    EXPECT_STREQ ("BottomCenter", cardinalPoints[1].name.c_str());
    EXPECT_STREQ ("BottomRight", cardinalPoints[2].name.c_str());
    EXPECT_STREQ ("MidDepthLeft", cardinalPoints[3].name.c_str());
    EXPECT_STREQ ("MidDepthCenter", cardinalPoints[4].name.c_str());
    EXPECT_STREQ ("MidDepthRight", cardinalPoints[5].name.c_str());
    EXPECT_STREQ ("TopLeft", cardinalPoints[6].name.c_str());
    EXPECT_STREQ ("TopCenter", cardinalPoints[7].name.c_str());
    EXPECT_STREQ ("TopRight", cardinalPoints[8].name.c_str());
    EXPECT_STREQ ("GeometricCentroid", cardinalPoints[9].name.c_str());
    EXPECT_STREQ ("BottomInLineWithGeometricCentroid", cardinalPoints[10].name.c_str());
    EXPECT_STREQ ("LeftInLineWithGeometricCentroid", cardinalPoints[11].name.c_str());
    EXPECT_STREQ ("RightInLineWithGeometricCentroid", cardinalPoints[12].name.c_str());
    EXPECT_STREQ ("TopInLineWithGeometricCentroid", cardinalPoints[13].name.c_str());
    EXPECT_STREQ ("ShearCenter", cardinalPoints[14].name.c_str());
    EXPECT_STREQ ("BottomInLineWithShearCenter", cardinalPoints[15].name.c_str());
    EXPECT_STREQ ("LeftInLineWithShearCenter", cardinalPoints[16].name.c_str());
    EXPECT_STREQ ("RightInLineWithShearCenter", cardinalPoints[17].name.c_str());
    EXPECT_STREQ ("TopInLineWithShearCenter", cardinalPoints[18].name.c_str());

    // Basic bounding box points
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[0].location.x);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[0].location.y);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[1].location.x);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[1].location.y);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[2].location.x);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[2].location.y);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[3].location.x);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[3].location.y);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[4].location.x);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[4].location.y);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[5].location.x);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[5].location.y);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[6].location.x);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[6].location.y);
    EXPECT_DOUBLE_EQ (0.0, cardinalPoints[7].location.x);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[7].location.y);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[8].location.x);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[8].location.y);

    DPoint3d centroid;
    double area;
    ASSERT_TRUE (profilePtr->GetShape()->GetAsCurveVector()->CentroidAreaXY (centroid, area));

    // Geometric centroid points
    EXPECT_DOUBLE_EQ (centroid.x, cardinalPoints[9].location.x);
    EXPECT_DOUBLE_EQ (centroid.y, cardinalPoints[9].location.y);
    EXPECT_DOUBLE_EQ (centroid.x, cardinalPoints[10].location.x);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[10].location.y);
    EXPECT_DOUBLE_EQ (-1.0, cardinalPoints[11].location.x);
    EXPECT_DOUBLE_EQ (centroid.y, cardinalPoints[11].location.y);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[12].location.x);
    EXPECT_DOUBLE_EQ (centroid.y, cardinalPoints[12].location.y);
    EXPECT_DOUBLE_EQ (centroid.x, cardinalPoints[13].location.x);
    EXPECT_DOUBLE_EQ (1.0, cardinalPoints[13].location.y);

    // Shear points
    for (int i = 14; i < 19; ++i)
        {
        EXPECT_DOUBLE_EQ (0.0, cardinalPoints[i].location.x);
        EXPECT_DOUBLE_EQ (0.0, cardinalPoints[i].location.y);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoints_InsertedInstance_StandardPointsInsertedAtBeggining)
    {
    LShapeProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->AddCustomCardinalPoint (CardinalPoint ("test", DPoint2d::From (1.0, 2.0)));
    profilePtr->Insert();

    bvector<CardinalPoint> cardinalPoints = profilePtr->GetCardinalPoints();
    ASSERT_EQ (20, cardinalPoints.size());
    ASSERT_STREQ ("BottomLeft", cardinalPoints[0].name.c_str()) << "StandardCardinal points should be inserted at the beggining of the array";
    ASSERT_STREQ ("test", cardinalPoints[19].name.c_str()) << "Custom, user-defined cardinal points should be at the end of the array.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoint_StandardPointEnum_Success)
    {
    LShapeProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->Insert();

    CardinalPoint cardinalPoint;
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::BottomLeft, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::BottomCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::BottomRight, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::MidDepthLeft, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::MidDepthCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::MidDepthRight, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::TopLeft, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::TopCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::TopRight, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::GeometricCentroid, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::BottomInLineWithGeometricCentroid, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::LeftInLineWithGeometricCentroid, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::RightInLineWithGeometricCentroid, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::TopInLineWithGeometricCentroid, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::ShearCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::BottomInLineWithShearCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::LeftInLineWithShearCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::RightInLineWithShearCenter, cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint (StandardCardinalPoint::TopInLineWithShearCenter, cardinalPoint));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoint_ByName_Success)
    {
    LShapeProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->Insert();
    profilePtr->AddCustomCardinalPoint (CardinalPoint ("test", DPoint2d()));

    CardinalPoint cardinalPoint;
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint ("BottomLeft", cardinalPoint));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->GetCardinalPoint ("test", cardinalPoint));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetCardinalPoint_InvalidName_NotFoundError)
    {
    LShapeProfilePtr profilePtr = CreateProfile ("P");
    profilePtr->Insert();

    CardinalPoint cardinalPoint;
    ASSERT_EQ (DgnDbStatus::NotFound, profilePtr->GetCardinalPoint ("test", cardinalPoint));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, AddCustomCardinalPoint_NewCustomPoint_Success)
    {
    ProfilePtr profilePtr = CreateProfile ("Profile");

    CardinalPoint point ("test", DPoint2d::From (1.0, 2.0));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->AddCustomCardinalPoint (point));

    DgnDbStatus status;
    profilePtr->Insert (&status);
    ASSERT_EQ (DgnDbStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, AddCustomCardinalPoint_ExistingCustomPoint_DuplicateNameError)
    {
    ProfilePtr profilePtr = CreateProfile ("Profile");

    CardinalPoint point ("test", DPoint2d::From (1.0, 2.0));
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->AddCustomCardinalPoint (point));
    ASSERT_EQ (DgnDbStatus::DuplicateName, profilePtr->AddCustomCardinalPoint (point));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, RemoveCustomCardinalPoint_ExistingCustomPoint_Success)
    {
    ProfilePtr profilePtr = CreateProfile ("Profile");

    CardinalPoint point ("test", DPoint2d::From (1.0, 2.0));
    profilePtr->AddCustomCardinalPoint (point);

    ASSERT_EQ (DgnDbStatus::Success, profilePtr->RemoveCustomCardinalPoint ("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, RemoveCustomCardinalPoint_StandardCardinalPoint_DeletionProhibitedError)
    {
    ProfilePtr profilePtr = CreateProfile ("Profile");
    profilePtr->Insert();

    ASSERT_EQ (DgnDbStatus::DeletionProhibited, profilePtr->RemoveCustomCardinalPoint ("BottomRight"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, RemoveCustomCardinalPoint_NonExistintPoint_NotFoundError)
    {
    ProfilePtr profilePtr = CreateProfile ("Profile");
    profilePtr->Insert();

    ASSERT_EQ (DgnDbStatus::NotFound, profilePtr->RemoveCustomCardinalPoint ("test"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, StandardCardinalPointToString_StandardCardinalPoint_String)
    {
    EXPECT_STREQ ("BottomLeft", StandardCardinalPointToString (StandardCardinalPoint::BottomLeft));
    EXPECT_STREQ ("BottomCenter", StandardCardinalPointToString (StandardCardinalPoint::BottomCenter));
    EXPECT_STREQ ("BottomRight", StandardCardinalPointToString (StandardCardinalPoint::BottomRight));
    EXPECT_STREQ ("MidDepthLeft", StandardCardinalPointToString (StandardCardinalPoint::MidDepthLeft));
    EXPECT_STREQ ("MidDepthCenter", StandardCardinalPointToString (StandardCardinalPoint::MidDepthCenter));
    EXPECT_STREQ ("MidDepthRight", StandardCardinalPointToString (StandardCardinalPoint::MidDepthRight));
    EXPECT_STREQ ("TopLeft", StandardCardinalPointToString (StandardCardinalPoint::TopLeft));
    EXPECT_STREQ ("TopCenter", StandardCardinalPointToString (StandardCardinalPoint::TopCenter));
    EXPECT_STREQ ("TopRight", StandardCardinalPointToString (StandardCardinalPoint::TopRight));
    EXPECT_STREQ ("GeometricCentroid", StandardCardinalPointToString (StandardCardinalPoint::GeometricCentroid));
    EXPECT_STREQ ("BottomInLineWithGeometricCentroid", StandardCardinalPointToString (StandardCardinalPoint::BottomInLineWithGeometricCentroid));
    EXPECT_STREQ ("LeftInLineWithGeometricCentroid", StandardCardinalPointToString (StandardCardinalPoint::LeftInLineWithGeometricCentroid));
    EXPECT_STREQ ("RightInLineWithGeometricCentroid", StandardCardinalPointToString (StandardCardinalPoint::RightInLineWithGeometricCentroid));
    EXPECT_STREQ ("TopInLineWithGeometricCentroid", StandardCardinalPointToString (StandardCardinalPoint::TopInLineWithGeometricCentroid));
    EXPECT_STREQ ("ShearCenter", StandardCardinalPointToString (StandardCardinalPoint::ShearCenter));
    EXPECT_STREQ ("BottomInLineWithShearCenter", StandardCardinalPointToString (StandardCardinalPoint::BottomInLineWithShearCenter));
    EXPECT_STREQ ("LeftInLineWithShearCenter", StandardCardinalPointToString (StandardCardinalPoint::LeftInLineWithShearCenter));
    EXPECT_STREQ ("RightInLineWithShearCenter", StandardCardinalPointToString (StandardCardinalPoint::RightInLineWithShearCenter));
    EXPECT_STREQ ("TopInLineWithShearCenter", StandardCardinalPointToString (StandardCardinalPoint::TopInLineWithShearCenter));
    }
