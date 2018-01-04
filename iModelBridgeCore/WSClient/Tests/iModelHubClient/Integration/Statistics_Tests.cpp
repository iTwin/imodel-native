/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Statistics_Tests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include "MockHttpHandler.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis                    11/2017
//---------------------------------------------------------------------------------------
struct StatisticsTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_connection;

    Utf8String m_userAdminId;
    Utf8String m_userNonAdminId;
    DateTime m_changeSetPushDate;

    BriefcasePtr m_adminBriefcase;

    StatisticsProperties briefcasesProperty = StatisticsProperties::BriefcaseCount;
    StatisticsProperties briefcasesLocksProperties = (StatisticsProperties) (StatisticsProperties::BriefcaseCount | StatisticsProperties::OwnedLocksCount);
    StatisticsProperties briefcasesLocksChangeSetDateProperties = (StatisticsProperties) (StatisticsProperties::BriefcaseCount | StatisticsProperties::OwnedLocksCount | StatisticsProperties::LastChangeSetPushDate);

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, nullptr);
        m_connection = ConnectToiModel(*m_client, m_imodel);
        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());

        m_changeSetPushDate = PrepareTestStatisticsData();
        }

    virtual void TearDown() override
        {
        DeleteiModel(m_projectId, *m_client, *m_imodel);
        m_client = nullptr;

        m_adminBriefcase = nullptr;

        IntegrationTestsBase::TearDown();
        }

    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    void AssertUserStatistics(StatisticsInfoPtr actualStatistics, int32_t expectedBriefcaseCount, int32_t expectedLocksCount, int32_t expectedChangeSetsCount, DateTime expectedLastChangeSetPushDate)
        {
        EXPECT_EQ(expectedBriefcaseCount, actualStatistics->GetBriefcaseCount());
        EXPECT_EQ(expectedLocksCount, actualStatistics->GetLocksCount());
        EXPECT_EQ(expectedChangeSetsCount, actualStatistics->GetChangeSetsCount());
        EXPECT_EQ(expectedLastChangeSetPushDate, actualStatistics->GetLastChangeSetPushDate());
        }

    DateTime PrepareTestStatisticsData()
        {
        m_adminBriefcase = AcquireBriefcase();
        auto briefcase2 = AcquireBriefcase();
        DgnDbR db = briefcase2->GetDgnDb();

        m_userAdminId = m_connection->QueryBriefcaseInfo(m_adminBriefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

        auto model1 = CreateModel("Model1", db);
        db.SaveChanges();

        auto result = briefcase2->PullMergeAndPush(nullptr, false, nullptr, CreateProgressCallback())->GetResult();
        EXPECT_SUCCESS(result);
        CheckProgressNotified();

        // Create new user
        auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidNonAdminCredentials());
        auto briefcaseUser2 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel, true);
        auto connection = briefcase2->GetiModelConnectionPtr();
        m_userNonAdminId = connection->QueryBriefcaseInfo(briefcaseUser2->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

        return briefcase2->GetiModelConnection().GetChangeSetById(db.Revisions().GetParentRevisionId())->GetResult().GetValue()->GetPushDate();
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUserStatisticsWithAllStatistics)
    {
    auto statistics = m_connection->GetStatisticsManager().QueryUserStatistics(m_userAdminId)->GetResult().GetValue();
    EXPECT_EQ(m_userAdminId, statistics->GetUserInfo()->GetId());
    AssertUserStatistics(statistics, 2, 4, 1, m_changeSetPushDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUserStatisticsWithSelectedStatistics)
    {
    auto statisticsManager = m_connection->GetStatisticsManager();

    auto statistics1 = statisticsManager.QueryUserStatistics(m_userAdminId, briefcasesProperty )->GetResult().GetValue();
    EXPECT_EQ(m_userAdminId, statistics1->GetUserInfo()->GetId());
    AssertUserStatistics(statistics1, 2, -1, -1, DateTime());

    auto statistics2 = statisticsManager.QueryUserStatistics(m_userAdminId, briefcasesLocksProperties )->GetResult().GetValue();
    EXPECT_EQ(m_userAdminId, statistics2->GetUserInfo()->GetId());
    AssertUserStatistics(statistics2, 2, 4, -1, DateTime());

    auto statistics3 = statisticsManager.QueryUserStatistics(m_userAdminId, briefcasesLocksChangeSetDateProperties )->GetResult().GetValue();
    EXPECT_EQ(m_userAdminId, statistics3->GetUserInfo()->GetId());
    AssertUserStatistics(statistics3, 2, 4, -1, m_changeSetPushDate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           11/2017
//---------------------------------------------------------------------------------------
TEST_F(StatisticsTests, QueryUsersStatisticsWithAllStatistics)
    {
    auto statisticsResult = m_connection->GetStatisticsManager().QueryUsersStatistics(bvector<Utf8String>{m_userAdminId, m_userNonAdminId})->GetResult();
    EXPECT_SUCCESS(statisticsResult);

    EXPECT_EQ(2, statisticsResult.GetValue().size());

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, 1, m_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
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
    auto statisticsManager = m_connection->GetStatisticsManager();

    auto statisticsResult1 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{m_userAdminId, m_userNonAdminId}, briefcasesProperty)->GetResult();
    EXPECT_SUCCESS(statisticsResult1);

    EXPECT_EQ(2, statisticsResult1.GetValue().size());

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult1.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, -1, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult2 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{m_userAdminId, m_userNonAdminId}, briefcasesLocksProperties)->GetResult();
    EXPECT_SUCCESS(statisticsResult2);

    EXPECT_EQ(2, statisticsResult2.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult2.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult3 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{m_userAdminId, m_userNonAdminId}, briefcasesLocksChangeSetDateProperties)->GetResult();
    EXPECT_SUCCESS(statisticsResult3);

    EXPECT_EQ(2, statisticsResult3.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult3.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, m_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult4 = statisticsManager.QueryUsersStatistics(bvector<Utf8String>{m_userAdminId}, briefcasesProperty)->GetResult();
    EXPECT_SUCCESS(statisticsResult4);

    EXPECT_EQ(1, statisticsResult4.GetValue().size());

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult4.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
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
    auto statisticsResult = m_connection->GetStatisticsManager().QueryAllUsersStatistics()->GetResult();
    EXPECT_SUCCESS(statisticsResult);

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, 1, m_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
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
    auto statisticsManager = m_connection->GetStatisticsManager();

    auto statisticsResult1 = statisticsManager.QueryAllUsersStatistics(briefcasesProperty)->GetResult();
    EXPECT_SUCCESS(statisticsResult1);

    bool successUser1 = false;
    bool successUser2 = false;

    for (auto statistics : statisticsResult1.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, -1, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, -1, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult2 = statisticsManager.QueryAllUsersStatistics(briefcasesLocksProperties)->GetResult();
    EXPECT_SUCCESS(statisticsResult2);

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult2.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, DateTime());
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);


    auto statisticsResult3 = statisticsManager.QueryAllUsersStatistics(briefcasesLocksChangeSetDateProperties)->GetResult();
    EXPECT_SUCCESS(statisticsResult3);

    successUser1 = false;
    successUser2 = false;

    for (auto statistics : statisticsResult3.GetValue())
        {
        if (statistics->GetUserInfo()->GetId() == m_userAdminId)
            {
            AssertUserStatistics(statistics, 2, 4, -1, m_changeSetPushDate);
            successUser1 = true;
            }

        if (statistics->GetUserInfo()->GetId() == m_userNonAdminId)
            {
            AssertUserStatistics(statistics, 1, 0, -1, DateTime());
            successUser2 = true;
            }
        }

    EXPECT_TRUE(successUser1);
    EXPECT_TRUE(successUser2);
    }

