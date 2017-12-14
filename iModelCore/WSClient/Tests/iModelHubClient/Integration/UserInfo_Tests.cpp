/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/UserInfo_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct UserInfoTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_connection;

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, nullptr);
        m_connection = ConnectToiModel(*m_client, m_imodel);
        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    virtual void TearDown() override
        {
        DeleteiModel(m_projectId, *m_client, *m_imodel);
        m_client = nullptr;

        IntegrationTestsBase::TearDown();
        }

    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    Utf8String GetNonAdminUserId()
        {
        auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidNonAdminCredentials());
        auto briefcase2 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel, true);
        auto connection = briefcase2->GetiModelConnectionPtr();
        return connection->QueryBriefcaseInfo(briefcase2->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryAllUsersInfoTest)
    {
    auto briefcase = AcquireBriefcase();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryAllUsersInfo()->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
    bool success = false;

    for (auto userInfo : userInfoResult.GetValue())
        {
        if (userInfo->GetId() == wantedUserId)
            success = true;
        }

    EXPECT_TRUE(success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUserInfoTest)
    {
    auto briefcase = AcquireBriefcase();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned())->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    EXPECT_EQ(wantedUserId, userInfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryInvalidUserInfoTest)
    {
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUserInfoById("Invalid User Id")->GetResult();
    EXPECT_FALSE(userInfoResult.IsSuccess());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsSeparately)
    {
    auto briefcase1 = AcquireBriefcase();
    auto wantedUserId1 = m_connection->QueryBriefcaseInfo(briefcase1->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
    auto userInfoResult1 = m_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId1)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult2 = m_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId2)->GetResult();
    EXPECT_SUCCESS(userInfoResult2);

    EXPECT_EQ(wantedUserId1, userInfoResult1.GetValue()->GetId());
    EXPECT_EQ(wantedUserId2, userInfoResult2.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsTogether)
    {
    auto briefcase1 = AcquireBriefcase();
    auto wantedUserId1 = m_connection->QueryBriefcaseInfo(briefcase1->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String>{wantedUserId1, wantedUserId2})->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    EXPECT_EQ(2, userInfoResult.GetValue().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsSeparately_OneAddedLater)
    {
    Utf8String userId1 = "0cdb50ea-c6c0-4790-9d31-f3395fcf6d3d";
    Utf8String userId2 = "105dcc93-e9c5-4265-8537-167caca31c98";
    Utf8String userId3 = "5a9e0150-873c-4fcd-98c0-491ba8065efb";

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();

    mockHandler->ForAnyRequest([=] (Http::RequestCR request)
        {
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1);

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2);

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3);

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId1, userId1);

        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId2, userId2);

        if (request.GetUrl().EndsWith("UserInfo"))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user1Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user2Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith("/v2.0/Plugins"))
            {
            auto httpResponseContent = Http::HttpResponseContent::Create(HttpStringBody::Create());
            httpResponseContent->GetHeaders().SetValue("Server", "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00");
            return Http::Response(httpResponseContent, "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
            }
        else if (request.GetUrl().EndsWith(user3Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create("{\"instances\":[]}")), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        return Http::Response(Http::HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, Http::HttpStatus::None);
        });

    // Set other wsclient
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(m_imodel->GetServerURL(), m_imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Should not succeed when user does not exist
    auto user3InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId3)->GetResult();
    EXPECT_FALSE(user3InfoResult.IsSuccess());

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return user2
    auto user2InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId2)->GetResult();
    EXPECT_SUCCESS(user2InfoResult);
    EXPECT_EQ(userId2, user2InfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QuerySameUserInfoTwice)
    {
    auto briefcase = AcquireBriefcase();
    auto userId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    double start1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult1 = m_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);
    EXPECT_EQ(userId, userInfoResult1.GetValue()->GetId());
    double end1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    double duration1 = end1 - start1;

    double start2 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult2 = m_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
    EXPECT_SUCCESS(userInfoResult2);
    EXPECT_EQ(userId, userInfoResult2.GetValue()->GetId());
    double end2 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    double duration2 = end2 - start2;

    //Expect cached retrieval to be faster than 1ms
    EXPECT_LT(duration2, 1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsTogether_OneAddedLater)
    {
    Utf8String userId1 = "8ff4834a-4754-4bbd-80b4-46770a50fcf4";
    Utf8String userId2 = "1dbbee67-196c-4c9d-96a1-ba4ef15fced8";
    Utf8String userId3 = "4fd49aef-142f-4eae-98e3-48eb0cee5cc7";

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();

    mockHandler->ForAnyRequest([=] (Http::RequestCR request)
        {
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1);

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2);

        Utf8String user23Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s','%s'%%5D", userId2, userId3);

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3);

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId1, userId1);

        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId2, userId2);

        if (request.GetUrl().EndsWith("UserInfo"))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user1Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user2Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith("/v2.0/Plugins"))
            {
            auto httpResponseContent = Http::HttpResponseContent::Create(HttpStringBody::Create());
            httpResponseContent->GetHeaders().SetValue("Server", "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00");
            return Http::Response(httpResponseContent, "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
            }
        else if (request.GetUrl().EndsWith(user3Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create("{\"instances\":[]}")), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user23Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        return Http::Response(Http::HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, Http::HttpStatus::None);
        });

    // Set other wsclient
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(m_imodel->GetServerURL(), m_imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return only user1 and user2
    auto bothUsersInfoResult = m_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String> { userId2, userId1, userId3 })->GetResult();
    EXPECT_SUCCESS(bothUsersInfoResult);

    bool found1 = false;
    bool found2 = false;
    bool foundOther = false;
    for (auto userInfo : bothUsersInfoResult.GetValue())
        {
        if (userInfo->GetId() == userId1)
            found1 = true;
        else if (userInfo->GetId() == userId2)
            found2 = true;
        else
            foundOther = true;
        }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
    EXPECT_FALSE(foundOther);
    }
