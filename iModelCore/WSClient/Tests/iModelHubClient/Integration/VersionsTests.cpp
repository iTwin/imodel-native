/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "VersionsTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct VersionsTests : public iModelTestsBase
    {
    // Versions initialized by SetUpTestCase for query tests
    static VersionInfoPtr s_version5;
    static VersionInfoPtr s_version10;
    static VersionInfoPtr s_version15;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        auto behaviourOptions = RequestBehaviorOptions();
        behaviourOptions.DisableOption(RequestBehaviorOptionsEnum::DoNotScheduleRenderThumbnailJob);

        iModelTestsBase::SetUpTestCase(behaviourOptions);
        iModelHubHelpers::PushDefaultView(s_client, s_info);
        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 14);
        iModelHubHelpers::CreateNamedVersion(s_version5, s_connection, "VersionsTests5", 5);
        iModelHubHelpers::CreateNamedVersion(s_version10, s_connection, "VersionsTests10", 10);
        iModelHubHelpers::CreateNamedVersion(s_version15, s_connection, "VersionsTests15", 15);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Andrius.Zonys                   04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ValidateThumbnailSelection(bvector<VersionInfoPtr> versions, Thumbnail::Size expectedSizes)
        {
        for (VersionInfoPtr version: versions)
            {
            if (version->GetId() != s_version5->GetId() &&
                version->GetId() != s_version10->GetId() &&
                version->GetId() != s_version15->GetId())
                continue;

            if (expectedSizes & Thumbnail::Size::Small)
                EXPECT_NE("", version->GetSmallThumbnailId());
            else
                EXPECT_EQ("", version->GetSmallThumbnailId());

            if (expectedSizes & Thumbnail::Size::Large)
                EXPECT_NE("", version->GetLargeThumbnailId());
            else
                EXPECT_EQ("", version->GetLargeThumbnailId());
            }
        }
    };
VersionInfoPtr VersionsTests::s_version5 = nullptr;
VersionInfoPtr VersionsTests::s_version10 = nullptr;
VersionInfoPtr VersionsTests::s_version15 = nullptr;


/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    size_t versionsCount = versionManager.GetAllVersions()->GetResult().GetValue().size();

    VersionInfoPtr version;
    iModelHubHelpers::CreateNamedVersion(version, s_connection, TestCodeName(), 1);
    EXPECT_NE("", version->GetId());
    EXPECT_EQ(s_info->GetUserCreated(), version->GetUserCreated());
    EXPECT_EQ(versionsCount + 1, versionManager.GetAllVersions()->GetResult().GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateUserDoesNotHavePermission)
    {
    iModelConnectionPtr nonAdminConnection = CreateNonAdminConnection();
    VersionsManagerCR nonAdminVersionManager = nonAdminConnection->GetVersionsManager();

    ChangeSetInfoPtr changeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 2);
    VersionInfo version = VersionInfo(TestCodeName(), nullptr, changeSet->GetId());
    VersionInfoResult result = nonAdminVersionManager.CreateVersion(version)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateInvalidPropertiesValues)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();

    ChangeSetInfoPtr changeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 3);
    VersionInfo version = VersionInfo(nullptr, nullptr, changeSet->GetId());
    VersionInfoResult result = versionManager.CreateVersion(version)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidPropertiesValues, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateChangeSetAlreadyHasVersion)
    {
    VersionInfoPtr version1;
    iModelHubHelpers::CreateNamedVersion(version1, s_connection, TestCodeName(), 4);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetInfoPtr changeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 4);
    VersionInfo version2 = VersionInfo(TestCodeName(1), nullptr, changeSet->GetId());
    VersionInfoResult result = versionManager.CreateVersion(version2)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::ChangeSetAlreadyHasVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateBaselineiModelAlreadyHasVersion)
    {
    VersionInfoPtr version1;
    iModelHubHelpers::CreateNamedVersion(version1, s_connection, TestCodeName(), 0);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    VersionInfo version2 = VersionInfo(TestCodeName(1), nullptr, "");
    VersionInfoResult result = versionManager.CreateVersion(version2)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::BaselineiModelAlreadyHasVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateVersionAlreadyExists)
    {
    VersionInfoPtr version1;
    iModelHubHelpers::CreateNamedVersion(version1, s_connection, TestCodeName(), 6);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetInfoPtr changeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 7);
    VersionInfo version2 = VersionInfo(TestCodeName(), nullptr, changeSet->GetId());
    VersionInfoResult result = versionManager.CreateVersion(version2)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::VersionAlreadyExists, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, CreateVersionChangeSetDoesNotExist)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    VersionInfo version2 = VersionInfo(TestCodeName(), nullptr, "0000000000000000000000000000000000000000");
    VersionInfoResult result = versionManager.CreateVersion(version2)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::ChangeSetDoesNotExist, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, UpdateSucceeds)
    {
    VersionInfoPtr version;
    Utf8CP description = "New Description";
    iModelHubHelpers::CreateNamedVersion(version, s_connection, TestCodeName(), 8);

    version->SetName(TestCodeName(1));
    version->SetDescription(description);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    StatusResult result = versionManager.UpdateVersion(*version)->GetResult();
    ASSERT_SUCCESS(result);

    VersionInfoResult queryResult = versionManager.GetVersionById(version->GetId())->GetResult();
    ASSERT_SUCCESS(queryResult);
    VersionInfoPtr updatedVersion = queryResult.GetValue();
    EXPECT_EQ(TestCodeName(1), updatedVersion->GetName());
    EXPECT_EQ(description, updatedVersion->GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, UpdateUserDoesNotHavePermission)
    {
    VersionInfoPtr version;
    iModelHubHelpers::CreateNamedVersion(version, s_connection, TestCodeName(), 9);

    version->SetName(TestCodeName(1));

    iModelConnectionPtr nonAdminConnection = CreateNonAdminConnection();
    VersionsManagerCR versionManager = nonAdminConnection->GetVersionsManager();
    StatusResult result = versionManager.UpdateVersion(*version)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, UpdateVersionAlreadyExists)
    {
    VersionInfoPtr version;
    iModelHubHelpers::CreateNamedVersion(version, s_connection, TestCodeName(), 11);
    VersionInfoPtr version2;
    iModelHubHelpers::CreateNamedVersion(version2, s_connection, TestCodeName(1), 12);

    version2->SetName(TestCodeName());

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    StatusResult result = versionManager.UpdateVersion(*version2)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::VersionAlreadyExists, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, UpdateInvalidPropertiesValues)
    {
    VersionInfoPtr version;
    iModelHubHelpers::CreateNamedVersion(version, s_connection, TestCodeName(), 13);

    version->SetName("");

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    StatusResult result = versionManager.UpdateVersion(*version)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidPropertiesValues, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetVersionChangeSetsSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetVersionChangeSets(s_version5->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(5, result.GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetVersionChangeSetsInvalidVersion)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetVersionChangeSets("")->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionsSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersions(s_version5->GetId(), s_version15->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(10, result.GetValue().size());
    EXPECT_EQ(s_version10->GetChangeSetId(), result.GetValue().at(4)->GetId());
    EXPECT_EQ(s_version15->GetChangeSetId(), result.GetValue().at(9)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionsReverseOrderSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersions(s_version15->GetId(), s_version5->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(10, result.GetValue().size());
    EXPECT_EQ(s_version10->GetChangeSetId(), result.GetValue().at(4)->GetId());
    EXPECT_EQ(s_version15->GetChangeSetId(), result.GetValue().at(9)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionsInvalidVersion)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersions(s_version5->GetId(), nullptr)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidVersion, result.GetError().GetId());

    result = versionManager.GetChangeSetsBetweenVersions(nullptr, s_version5->GetId())->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsAfterVersionSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsAfterVersion(s_version10->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(5, result.GetValue().size());
    EXPECT_EQ(s_version15->GetChangeSetId(), result.GetValue().at(4)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsAfterVersionOnTipSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsAfterVersion(s_version15->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(0, result.GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsAfterVersionInvalidVersion)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsAfterVersion(nullptr)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsAfterBaselineVersionSucceeds)
    {
    VersionInfoPtr version0 = iModelHubHelpers::GetVersionByChangeSetId(s_connection, "");
    if (!version0.IsValid())
        iModelHubHelpers::CreateNamedVersion(version0, s_connection, TestCodeName(), 0);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsAfterVersion(version0->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(15, result.GetValue().size());
    EXPECT_EQ(s_version15->GetChangeSetId(), result.GetValue().at(14)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenBaselineVersionAndChangeSetSucceeds)
    {
    ChangeSetInfoPtr changeset1 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 1);
    ChangeSetInfoPtr changeset2 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 2);

    VersionInfoPtr version0 = iModelHubHelpers::GetVersionByChangeSetId(s_connection, "");
    if (!version0.IsValid())
        iModelHubHelpers::CreateNamedVersion(version0, s_connection, TestCodeName(), 0);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersionAndChangeSet(version0->GetId(), changeset2->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(2, result.GetValue().size());
    EXPECT_EQ(changeset1->GetId(), result.GetValue().at(0)->GetId());
    EXPECT_EQ(changeset2->GetId(), result.GetValue().at(1)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionAndChangeSetSucceeds)
    {
    ChangeSetInfoPtr changeset6 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 6);
    ChangeSetInfoPtr changeset7 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 7);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersionAndChangeSet(s_version5->GetId(), changeset7->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(2, result.GetValue().size());
    EXPECT_EQ(changeset6->GetId(), result.GetValue().at(0)->GetId());
    EXPECT_EQ(changeset7->GetId(), result.GetValue().at(1)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionAndChangeSetReverseOrderSucceeds)
    {
    ChangeSetInfoPtr changeset3 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 3);
    ChangeSetInfoPtr changeset4 = iModelHubHelpers::GetChangeSetByIndex(s_connection, 4);

    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersionAndChangeSet(s_version5->GetId(), changeset3->GetId())->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(2, result.GetValue().size());
    EXPECT_EQ(changeset4->GetId(), result.GetValue().at(0)->GetId());
    EXPECT_EQ(s_version5->GetChangeSetId(), result.GetValue().at(1)->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionAndChangeSetInvalidVersion)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersionAndChangeSet("", iModelHubHelpers::GetChangeSetByIndex(s_connection, 3)->GetId())->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidVersion, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetChangeSetsBetweenVersionAndChangeSetEmptyChangeSetIdSucceeds)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    ChangeSetsInfoResult result = versionManager.GetChangeSetsBetweenVersionAndChangeSet(s_version10->GetId(), "")->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_EQ(10, result.GetValue().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VersionsTests, GetVersionsWithThumbnails)
    {
    VersionsManagerCR versionManager = s_connection->GetVersionsManager();
    bvector<VersionInfoPtr> versions = versionManager.GetAllVersions(nullptr)->GetResult().GetValue();
    ValidateThumbnailSelection(versions, Thumbnail::Size::None);

    // Wait until Thumbnails will be rendered.
    bvector<Utf8String> versionsWithoutThumbnails = { s_version5->GetId(), s_version10->GetId(), s_version15->GetId() };
    for (int i = 0; i <= 20; i++)
        {
        bvector<Utf8String> versionIds = versionsWithoutThumbnails;
        for (Utf8String versionId : versionIds)
            {
            VersionInfoPtr version = versionManager.GetVersionById(versionId, nullptr, Thumbnail::Size::Large)->GetResult().GetValue();
            if ("" == version->GetLargeThumbnailId())
                break;

            versionsWithoutThumbnails.erase(std::remove(versionsWithoutThumbnails.begin(), versionsWithoutThumbnails.end(), versionId), versionsWithoutThumbnails.end());
            }

        if (versionsWithoutThumbnails.empty())
            break;
                
        BeThreadUtilities::BeSleep(15000);
        }

    EXPECT_TRUE(versionsWithoutThumbnails.empty());

    versions = versionManager.GetAllVersions(nullptr, Thumbnail::Size::Small)->GetResult().GetValue();
    ValidateThumbnailSelection(versions, Thumbnail::Size::Small);

    versions = versionManager.GetAllVersions(nullptr, (Thumbnail::Size) (Thumbnail::Size::Small | Thumbnail::Size::Large))->GetResult().GetValue();
    ValidateThumbnailSelection(versions, (Thumbnail::Size) (Thumbnail::Size::Small | Thumbnail::Size::Large));

    versions.clear();
    versions.push_back(versionManager.GetVersionById(s_version5->GetId())->GetResult().GetValue());
    ValidateThumbnailSelection(versions, Thumbnail::Size::None);
    
    versions.clear();
    versions.push_back(versionManager.GetVersionById(s_version5->GetId(), nullptr, Thumbnail::Size::Large)->GetResult().GetValue());
    versions.push_back(versionManager.GetVersionById(s_version10->GetId(), nullptr, Thumbnail::Size::Large)->GetResult().GetValue());
    ValidateThumbnailSelection(versions, Thumbnail::Size::Large);

    versions.clear();
    versions.push_back(versionManager.GetVersionById(s_version5->GetId(), nullptr, (Thumbnail::Size)(Thumbnail::Size::Small | Thumbnail::Size::Large))->GetResult().GetValue());
    ValidateThumbnailSelection(versions, (Thumbnail::Size)(Thumbnail::Size::Small | Thumbnail::Size::Large));

    ThumbnailsManagerCR thumbnailsManager = s_connection->GetThumbnailsManager();
    ThumbnailImageResult result = thumbnailsManager.GetThumbnailById(versions[0]->GetSmallThumbnailId(), Thumbnail::Size::Small)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsValid());
    result = thumbnailsManager.GetThumbnailById(versions[0]->GetLargeThumbnailId(), Thumbnail::Size::Large)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsValid());
    }