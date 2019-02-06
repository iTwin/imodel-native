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
    LShapeProfilePtr CreateProfile()
        {
        LShapeProfile::CreateParams params (GetModel(), "", 10.0, 6.0, 1.0);
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
TEST_F (ProfileTestCase, GetCode_ExistingCode_CorrectFormatString)
    {
    ProfilePtr profilePtr = CreateProfile();
    profilePtr->SetName ("D");
    profilePtr->SetStandardCatalogCode (StandardCatalogCode ("A", "B", "C"));

    Utf8CP pCodeValue = profilePtr->GetCode().GetValue().GetUtf8CP();
    ASSERT_STREQ ("A:B:C:D", pCodeValue) << "StandardCatalogProfile CodeValue should be of format: '<Manufacturer>:<StandardsOrganization>:<Revision>:<Designation>'";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProfileTestCase, GetStandardCatalogCode_InvalidCode_EmptyValues)
    {
    CodeSpecCPtr codeSpecPtr = GetDb().CodeSpecs().GetCodeSpec (PRF_CODESPEC_StandardCatalogProfile);
    ProfilePtr profilePtr = CreateProfile();

    // Missing D part
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->SetCode (codeSpecPtr->CreateCode ("A:B:C")));
    StandardCatalogCode catalogCode = profilePtr->GetStandardCatalogCode();

    ASSERT_STREQ ("", catalogCode.manufacturer.c_str());
    ASSERT_STREQ ("", catalogCode.standardsOrganization.c_str());
    ASSERT_STREQ ("", catalogCode.revision.c_str());
    ASSERT_STREQ ("", catalogCode.designation.c_str());

    // E should get ignored - validation for invalid (e.g. too long ones) values is not performed
    ASSERT_EQ (DgnDbStatus::Success, profilePtr->SetCode (codeSpecPtr->CreateCode ("A:B:C:D:E")));
    catalogCode = profilePtr->GetStandardCatalogCode();

    ASSERT_STREQ ("A", catalogCode.manufacturer.c_str());
    ASSERT_STREQ ("B", catalogCode.standardsOrganization.c_str());
    ASSERT_STREQ ("C", catalogCode.revision.c_str());
    ASSERT_STREQ ("D", catalogCode.designation.c_str());
    }
