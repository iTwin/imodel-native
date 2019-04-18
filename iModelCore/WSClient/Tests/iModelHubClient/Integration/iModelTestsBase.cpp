/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

iModelConnectionPtr iModelTestsBase::s_connection = nullptr;
iModelInfoPtr iModelTestsBase::s_info = nullptr;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::SetUpTestCase(RequestBehaviorOptions behaviourOptions)
    {
    iModelHubHost& host = iModelHubHost::Instance();
    host.CleanOutputDirectory();

    IntegrationTestsBase::SetUpTestCase(behaviourOptions);
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, TestCaseiModelName()));

    //Create iModel with no changesets
    iModelResult result = CreateEmptyiModel(TestCaseiModelName(), true);
    ASSERT_SUCCESS(result);
    s_info = result.GetValue();
    s_connection = CreateiModelConnection(s_info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::TearDownTestCase()
    {
    if (s_connection.IsValid())
        s_connection = nullptr;
    if (s_info.IsValid())
        s_info = nullptr;
    iModelHubHelpers::DeleteiModelByName(s_client, TestCaseiModelName());
    IntegrationTestsBase::TearDownTestCase();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::SetUp()
    {
    IntegrationTestsBase::SetUp();
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName()));
    ASSERT_SUCCESS(iModelHubHelpers::AbandonAllBriefcases(s_client, s_connection));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::TearDown()
    {
    if (m_info.IsValid())
        m_info = nullptr;
    iModelHubHelpers::AbandonAllBriefcases(s_client, s_connection);
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());
    IntegrationTestsBase::TearDown();
    }

Utf8String iModelTestsBase::TestCaseiModelName()
    {
    auto unitTest = ::testing::UnitTest::GetInstance();
    auto testcase = unitTest->current_test_case();
    return Utf8String(testcase->name());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::AcquireAndOpenBriefcase(BriefcasePtr& briefcase, iModelInfoPtr info, bool pull)
    {
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, info, pull);
    ASSERT_SUCCESS(briefcaseResult);
    briefcase = briefcaseResult.GetValue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BriefcasePtr iModelTestsBase::AcquireAndOpenBriefcase(bool pull)
    {
    BriefcasePtr briefcase;
    AcquireAndOpenBriefcase(briefcase, s_info, pull);
    return briefcase;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::CreateNonAdminConnection(iModelConnectionPtr& connection, iModelInfoPtr info)
    {
    ClientPtr nonAdminClient = CreateNonAdminClient();
    iModelConnectionResult connectionResult = nonAdminClient->ConnectToiModel(*info)->GetResult();
    ASSERT_SUCCESS(connectionResult);
    connection = connectionResult.GetValue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelConnectionPtr iModelTestsBase::CreateNonAdminConnection()
    {
    iModelConnectionPtr connection;
    CreateNonAdminConnection(connection, s_info);
    return connection;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelTestsBase::CreateTestiModel()
    {
    iModelResult result = IntegrationTestsBase::CreateEmptyiModel(GetTestiModelName());
    ASSERT_SUCCESS(result);
    m_info = result.GetValue();
    }
