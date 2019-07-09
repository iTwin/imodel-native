/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "Helpers.h"
#include "iModelTestsBase.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Benas.Kikutis                   11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct StatisticsTests : public iModelTestsBase
    {
    static Utf8String s_userAdminId;
    static Utf8String s_userNonAdminId;
    static DateTime s_changeSetPushDate;

    static BriefcasePtr s_adminBriefcase;
    
    StatisticsProperties briefcasesProperty = StatisticsProperties::BriefcaseCount;
    StatisticsProperties briefcasesLocksProperties = (StatisticsProperties) (StatisticsProperties::BriefcaseCount | StatisticsProperties::OwnedLocksCount);
    StatisticsProperties briefcasesLocksChangeSetDateProperties = (StatisticsProperties) (StatisticsProperties::BriefcaseCount | StatisticsProperties::OwnedLocksCount | StatisticsProperties::LastChangeSetPushDate);
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        PrepareTestStatisticsData(s_changeSetPushDate);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        if (s_adminBriefcase.IsValid())
            {
            s_adminBriefcase = nullptr;
            }
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Benas.Kikutis                   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Benas.Kikutis                   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Benas.Kikutis                   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AssertUserStatistics(StatisticsInfoPtr actualStatistics, int32_t expectedBriefcaseCount, int32_t expectedLocksCount, int32_t expectedChangeSetsCount, DateTime expectedLastChangeSetPushDate)
        {
        EXPECT_EQ(expectedBriefcaseCount, actualStatistics->GetBriefcaseCount());
        EXPECT_EQ(expectedLocksCount, actualStatistics->GetLocksCount());
        EXPECT_EQ(expectedChangeSetsCount, actualStatistics->GetChangeSetsCount());
        EXPECT_EQ(expectedLastChangeSetPushDate, actualStatistics->GetLastChangeSetPushDate());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Benas.Kikutis                   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void PrepareTestStatisticsData(DateTime& dateTime)
        {
        s_adminBriefcase = AcquireAndOpenBriefcase();
        BriefcasePtr briefcase2 = AcquireAndOpenBriefcase();
        DgnDbR db = briefcase2->GetDgnDb();

        s_userAdminId = s_connection->QueryBriefcaseInfo(s_adminBriefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

        PhysicalModelPtr model1 = CreateModel("Model", db);
        db.SaveChanges();

        ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false, false));

        // Create new user
        ClientPtr nonAdminClient;
        iModelHubHelpers::CreateClient(nonAdminClient, IntegrationTestsSettings::Instance().GetValidNonAdminCredentials());
        auto connectionResult = nonAdminClient->ConnectToiModel(*s_info)->GetResult();
        ASSERT_SUCCESS(connectionResult);

        BriefcaseInfoResult acquireResult = connectionResult.GetValue()->AcquireNewBriefcase()->GetResult();
        ASSERT_SUCCESS(acquireResult);
        BriefcaseInfoPtr briefcaseUser2 = acquireResult.GetValue();

        s_userNonAdminId = briefcase2->GetiModelConnection().QueryBriefcaseInfo(briefcaseUser2->GetId())->GetResult().GetValue()->GetUserOwned();
        dateTime = briefcase2->GetiModelConnection().GetChangeSetById(db.Revisions().GetParentRevisionId())->GetResult().GetValue()->GetPushDate();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void QueryUserStatistics(StatisticsInfoPtr& result, Utf8StringCR userId, StatisticsProperties propertiesToSelect)
        {
        StatisticsInfoResult statisticsResult = s_connection->GetStatisticsManager().QueryUserStatistics(userId, propertiesToSelect)->GetResult();
        ASSERT_SUCCESS(statisticsResult);
        result = statisticsResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatisticsInfoPtr QueryUserStatistics(Utf8StringCR userId, StatisticsProperties propertiesToSelect = StatisticsProperties::All)
        {
        StatisticsInfoPtr result;
        QueryUserStatistics(result, userId, propertiesToSelect);
        return result;
        }
    };
Utf8String StatisticsTests::s_userAdminId = "";
Utf8String StatisticsTests::s_userNonAdminId = "";
DateTime StatisticsTests::s_changeSetPushDate = DateTime();
BriefcasePtr StatisticsTests::s_adminBriefcase = nullptr;

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUserStatisticsWithAllStatistics)
    {
    StatisticsInfoPtr statistics = QueryUserStatistics(s_userAdminId);
    EXPECT_EQ(s_userAdminId, statistics->GetUserInfo()->GetId());
    AssertUserStatistics(statistics, 2, 4, 1, s_changeSetPushDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUserStatisticsWithSelectedStatistics)
    {
    StatisticsInfoPtr statistics1 = QueryUserStatistics(s_userAdminId, briefcasesProperty);
    EXPECT_EQ(s_userAdminId, statistics1->GetUserInfo()->GetId());
    AssertUserStatistics(statistics1, 2, -1, -1, DateTime());

    StatisticsInfoPtr statistics2 = QueryUserStatistics(s_userAdminId, briefcasesLocksProperties);
    EXPECT_EQ(s_userAdminId, statistics2->GetUserInfo()->GetId());
    AssertUserStatistics(statistics2, 2, 4, -1, DateTime());

    StatisticsInfoPtr statistics3 = QueryUserStatistics(s_userAdminId, briefcasesLocksChangeSetDateProperties);
    EXPECT_EQ(s_userAdminId, statistics3->GetUserInfo()->GetId());
    AssertUserStatistics(statistics3, 2, 4, -1, s_changeSetPushDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUsersStatisticsWithAllStatistics)
    {
    auto statisticsResult = s_connection->GetStatisticsManager().QueryUsersStatistics(bvector<Utf8String>{s_userAdminId, s_userNonAdminId})->GetResult();
    ASSERT_SUCCESS(statisticsResult);

    EXPECT_EQ(2, statisticsResult.GetValue().size());

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, 1, s_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, 0, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUsersStatisticsWithSelectedStatistics)
    {
    auto statisticsManager = s_connection->GetStatisticsManager();

    auto statisticsResult1 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{s_userAdminId, s_userNonAdminId}, briefcasesProperty)->GetResult();
    ASSERT_SUCCESS(statisticsResult1);

    EXPECT_EQ(2, statisticsResult1.GetValue().size());

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult1.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, -1, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult2 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{s_userAdminId, s_userNonAdminId}, briefcasesLocksProperties)->GetResult();
    ASSERT_SUCCESS(statisticsResult2);

    EXPECT_EQ(2, statisticsResult2.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult2.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult3 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{s_userAdminId, s_userNonAdminId}, briefcasesLocksChangeSetDateProperties)->GetResult();
    ASSERT_SUCCESS(statisticsResult3);

    EXPECT_EQ(2, statisticsResult3.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult3.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, s_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult4 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{s_userAdminId}, briefcasesProperty)->GetResult();
    ASSERT_SUCCESS(statisticsResult4);

    EXPECT_EQ(1, statisticsResult4.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult4.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, -1, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_FALSE(successUser2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryAllUsersStatisticsWithAllStatistics)
    {
    auto statisticsResult = s_connection->GetStatisticsManager().QueryAllUsersStatistics()->GetResult();
    ASSERT_SUCCESS(statisticsResult);

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, 1, s_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, 0, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryAllUsersStatisticsWithSelectedStatistics)
    {
    auto statisticsManager = s_connection->GetStatisticsManager();

    auto statisticsResult1 = statisticsManager.QueryAllUsersStatistics(briefcasesProperty)->GetResult();
    ASSERT_SUCCESS(statisticsResult1);

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult1.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, -1, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult2 = statisticsManager.QueryAllUsersStatistics(briefcasesLocksProperties)->GetResult();
    ASSERT_SUCCESS(statisticsResult2);

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult2.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult3 = statisticsManager.QueryAllUsersStatistics(briefcasesLocksChangeSetDateProperties)->GetResult();
    ASSERT_SUCCESS(statisticsResult3);

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult3.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == s_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, s_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == s_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);
    }

