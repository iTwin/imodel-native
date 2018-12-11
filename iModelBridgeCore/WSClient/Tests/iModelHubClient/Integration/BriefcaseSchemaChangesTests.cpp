/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/BriefcaseSchemaChangesTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "BriefcaseSchemaChangesTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcaseSchemaChangesTests : public iModelTestsBase
    {
    static VersionInfoPtr       s_version1;
    static VersionInfoPtr       s_version2;
    static VersionInfoPtr       s_version3;
    static VersionInfoPtr       s_version4;
    static VersionInfoPtr       s_version5;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();

        BriefcasePtr briefcase = AcquireAndOpenBriefcase();

        iModelHubHelpers::AddChangeSets(briefcase, 4);
        iModelHubHelpers::CreateNamedVersion(s_version1, s_connection, "Version1", 2);

        EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().LockSchemas().Result());
        PushSchemaChanges(briefcase, "BriefcaseSchemaChangesTests");
        iModelHubHelpers::CreateNamedVersion(s_version2, s_connection, "Version2", 5);

        iModelHubHelpers::AddChangeSets(briefcase, 3, 4);
        iModelHubHelpers::CreateNamedVersion(s_version3, s_connection, "Version3", 6);
        iModelHubHelpers::CreateNamedVersion(s_version4, s_connection, "Version4", 7);

        EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().LockSchemas().Result());
        PushSchemaChanges(briefcase, "BriefcaseSchemaChangesTests2");
        iModelHubHelpers::CreateNamedVersion(s_version5, s_connection, "Version5", 9);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectDbIsUpToDate(DgnDbCR db)
        {
        EXPECT_EQ(iModelHubHelpers::GetLastChangeSet(s_connection)->GetId(), db.Revisions().GetParentRevisionId());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void PushSchemaChanges(BriefcasePtr briefcase, Utf8StringCR tableName)
        {
        briefcase->GetDgnDb().CreateTable(tableName.c_str(), "Id INTEGER PRIMARY KEY, Column1 INTEGER");
        ASSERT_TRUE(briefcase->GetDgnDb().Txns().HasDbSchemaChanges());
        briefcase->GetDgnDb().SaveChanges();
        ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void OpenBriefcaseWithSchemaChanges(BriefcasePtr& briefcase, bvector<ChangeSetInfoPtr>& changeSets,
        RevisionProcessOption upgradeOptions = RevisionProcessOption::Merge)
        {
        ChangeSetsResult changeSetsToMerge = s_connection->DownloadChangeSets(changeSets)->GetResult();
        ASSERT_SUCCESS(changeSetsToMerge);
        briefcase->GetDgnDb().CloseDb();
        DbResult result;
        BriefcaseResult briefcaseResult = s_client->OpenBriefcase(s_client->OpenWithSchemaUpgrade(&result,
            briefcase->GetDgnDb().GetFileName(), changeSetsToMerge.GetValue(), upgradeOptions))->GetResult();
        ASSERT_EQ(DbResult::BE_SQLITE_OK, result);
        ASSERT_SUCCESS(briefcaseResult);
        briefcase = briefcaseResult.GetValue();
        }
    };
VersionInfoPtr BriefcaseSchemaChangesTests::s_version1 = nullptr;
VersionInfoPtr BriefcaseSchemaChangesTests::s_version2 = nullptr;
VersionInfoPtr BriefcaseSchemaChangesTests::s_version3 = nullptr;
VersionInfoPtr BriefcaseSchemaChangesTests::s_version4 = nullptr;
VersionInfoPtr BriefcaseSchemaChangesTests::s_version5 = nullptr;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseSchemaChangesTests, UpdateToVersionPullWithoutReinstating)
    {
    //--Merge------->|--*Merge----------------------------->|
    //--------------------------------|<--Reverse-----------|
    //--------------------------------|---*Pull---------------->----Create+Push---->|
    BriefcasePtr briefcase = AcquireAndOpenBriefcase(false);

    //merge to version1
    ASSERT_SUCCESS(iModelHubHelpers::UpdateToVersion(briefcase, s_version1));

    //merge to version4 should fail
    StatusResult updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version4, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(s_version1->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //merge to version4 with reopen
    ChangeSetsInfoResult changeSetsResult = s_connection->GetVersionsManager().GetChangeSetsBetweenVersionAndChangeSet(s_version4->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to version2
    updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version2);
    ASSERT_SUCCESS(updateResult);
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //pull should fail
    ChangeSetsResult pullResult = iModelHubHelpers::PullMergeAndPush(briefcase, false, true, true, false);
    ASSERT_FAILURE(pullResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, pullResult.GetError().GetId());

    //merge to tip with reopen
    changeSetsResult = s_connection->GetChangeSetsAfterId(s_version2->GetChangeSetId())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);

    //#9001 Enable UpdateBriefcaseToVersion and UpdateBriefcaseToChangeSet tests after bug fixes in DgnDb.
    //ExpectDbIsUpToDate(briefcase->GetDgnDb());

    //push additional schema changes
    //PushSchemaChanges(briefcase, GetTestInfo().name());
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseSchemaChangesTests, UpdateToVersionPullWithReinstating)
    {
    ////------------------*Merge----------------------------->|
    ////---------------|<-*Reverse----------------------------|
    ////---------------|--*Reinstate--->|---Reinstate-->|---------*Reinstate+Merge--->|
    BriefcasePtr briefcase = AcquireAndOpenBriefcase(false);

    //merge to version4 should fail
    StatusResult updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version4, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ("", briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //merge to version4 with reopen
    ChangeSetsInfoResult changeSetsResult = s_connection->GetVersionsManager().GetChangeSetsBetweenVersionAndChangeSet(s_version4->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to version1 should fail
    updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version1, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to version1 with reopen
    changeSetsResult = s_connection->GetVersionsManager().GetChangeSetsBetweenVersionAndChangeSet(s_version1->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reverse);
    EXPECT_EQ(s_version1->GetChangeSetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to version2 should fail
    updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version2, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(s_version1->GetChangeSetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to version2 with reopen
    changeSetsResult = s_connection->GetVersionsManager().GetChangeSetsBetweenVersionAndChangeSet(s_version2->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);
    EXPECT_EQ(s_version2->GetChangeSetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to version3
    updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version3);
    ASSERT_SUCCESS(updateResult);
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate+merge to version5 should fail
    updateResult = iModelHubHelpers::UpdateToVersion(briefcase, s_version5, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(s_version4->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate+merge to version5 with reopen
    changeSetsResult = s_connection->GetVersionsManager().GetChangeSetsBetweenVersionAndChangeSet(s_version5->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);
    EXPECT_EQ(s_version5->GetChangeSetId(), briefcase->GetLastChangeSetPulled());

    //#9001 Enable UpdateBriefcaseToVersion and UpdateBriefcaseToChangeSet tests after bug fixes in DgnDb.
    //EXPECT_EQ(s_version5->GetChangeSetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseSchemaChangesTests, UpdateToChangeSetPullWithoutReinstating)
    {
    //--Merge------->|--*Merge----------------------------->|
    //--------------------------------|<--Reverse-----------|
    //--------------------------------|---*Pull---------------->----Create+Push---->|
    BriefcasePtr briefcase = AcquireAndOpenBriefcase(false);

    ChangeSetsInfoResult changeSetsResult = s_connection->GetAllChangeSets()->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    auto changeSets = changeSetsResult.GetValue();

    ChangeSetInfoPtr changeSet2 = changeSets.at(1);
    ChangeSetInfoPtr changeSet5 = changeSets.at(4);
    ChangeSetInfoPtr changeSet6 = changeSets.at(5);
    ChangeSetInfoPtr changeSet7 = changeSets.at(6);
    ChangeSetInfoPtr changeSet9 = changeSets.at(8);

    //merge to changeSet2
    ASSERT_SUCCESS(iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet2));

    //merge to changeSet7 should fail
    StatusResult updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet7, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(changeSet2->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //merge to changeSet7 with reopen
    changeSetsResult = s_connection->GetChangeSetsBetween(changeSet7->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to changeSet5
    updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet5);
    ASSERT_SUCCESS(updateResult);
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //pull should fail
    ChangeSetsResult pullResult = iModelHubHelpers::PullMergeAndPush(briefcase, false, true, true, false);
    ASSERT_FAILURE(pullResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, pullResult.GetError().GetId());

    //merge to tip with reopen
    changeSetsResult = s_connection->GetChangeSetsAfterId(changeSet5->GetId())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);

    //#9001 Enable UpdateBriefcaseToVersion and UpdateBriefcaseToChangeSet tests after bug fixes in DgnDb.
    //ExpectDbIsUpToDate(briefcase->GetDgnDb());

    //push additional schema changes
    //PushSchemaChanges(briefcase, GetTestInfo().name());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcaseSchemaChangesTests, UpdateToChangeSetPullWithReinstating)
    {
    ////------------------*Merge----------------------------->|
    ////---------------|<-*Reverse----------------------------|
    ////---------------|--*Reinstate--->|---Reinstate-->|---------*Reinstate+Merge--->|
    BriefcasePtr briefcase = AcquireAndOpenBriefcase(false);

    ChangeSetsInfoResult changeSetsResult = s_connection->GetAllChangeSets()->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    auto changeSets = changeSetsResult.GetValue();

    ChangeSetInfoPtr changeSet2 = changeSets.at(1);
    ChangeSetInfoPtr changeSet5 = changeSets.at(4);
    ChangeSetInfoPtr changeSet6 = changeSets.at(5);
    ChangeSetInfoPtr changeSet7 = changeSets.at(6);
    ChangeSetInfoPtr changeSet9 = changeSets.at(8);

    //merge to changeSet7 should fail
    StatusResult updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet7, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ("", briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //merge to changeSet7 with reopen
    changeSetsResult = s_connection->GetChangeSetsBetween(changeSet7->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to changeSet2 should fail
    updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet2, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reverse to changeSet2 with reopen
    changeSetsResult = s_connection->GetChangeSetsBetween(changeSet2->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reverse);
    EXPECT_EQ(changeSet2->GetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to changeSet5 should fail
    updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet5, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(changeSet2->GetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to changeSet5 with reopen
    changeSetsResult = s_connection->GetChangeSetsBetween(changeSet5->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);
    EXPECT_EQ(changeSet5->GetId(), briefcase->GetLastChangeSetPulled());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate to changeSet6
    updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet6);
    ASSERT_SUCCESS(updateResult);
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate+merge to ChangeSet5 should fail
    updateResult = iModelHubHelpers::UpdateToChangeSet(briefcase, changeSet9, false);
    ASSERT_FAILURE(updateResult);
    EXPECT_EQ(Error::Id::ReverseOrReinstateSchemaChangesOnOpen, updateResult.GetError().GetId());
    EXPECT_EQ(changeSet7->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());

    //reinstate+merge to ChangeSet5 with reopen
    changeSetsResult = s_connection->GetChangeSetsBetween(changeSet9->GetId(), briefcase->GetLastChangeSetPulled())->GetResult();
    OpenBriefcaseWithSchemaChanges(briefcase, changeSetsResult.GetValue(), RevisionProcessOption::Reinstate);
    EXPECT_EQ(changeSet9->GetId(), briefcase->GetLastChangeSetPulled());
    
    //#9001 Enable UpdateBriefcaseToVersion and UpdateBriefcaseToChangeSet tests after bug fixes in DgnDb.
    //EXPECT_EQ(changeSet9->GetId(), briefcase->GetDgnDb().Revisions().GetParentRevisionId());
    }
