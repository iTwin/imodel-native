/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/UserInfoTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include "../Helpers/MockHttpHandler.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis                    11/2017
//---------------------------------------------------------------------------------------
struct UserInfoTests : public iModelTestsBase
    {
    BriefcasePtr m_briefcase;
    IWSRepositoryClientPtr m_originalClient;

    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        }

    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    void SetUp() override
        {
        iModelTestsBase::SetUp();
        BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, s_info);
        ASSERT_SUCCESS(briefcaseResult);
        m_briefcase = briefcaseResult.GetValue();
        m_originalClient = s_connection->GetRepositoryClient();
        }
    
    void TearDown() override
        {
        s_connection->SetRepositoryClient(m_originalClient);
        iModelTestsBase::TearDown();
        }

    Utf8String GetNonAdminUserId()
        {
        ClientPtr nonAdminClient = CreateNonAdminClient();
        BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(nonAdminClient, s_info);
        EXPECT_SUCCESS(briefcaseResult);
        if (!briefcaseResult.IsSuccess())
            return "";
        BriefcasePtr briefcase = briefcaseResult.GetValue();
        iModelConnectionPtr connection = briefcase->GetiModelConnectionPtr();
        return connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryAllUsersInfoTest)
    {
    auto userInfoResult = s_connection->GetUserInfoManager().QueryAllUsersInfo()->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
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
    auto userInfoResult = s_connection->GetUserInfoManager().QueryUserInfoById(s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned())->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    EXPECT_EQ(wantedUserId, userInfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryInvalidUserInfoTest)
    {
    auto userInfoResult = s_connection->GetUserInfoManager().QueryUserInfoById("Invalid User Id")->GetResult();
    EXPECT_FALSE(userInfoResult.IsSuccess());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsSeparately)
    {
    auto wantedUserId1 = s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
    auto userInfoResult1 = s_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId1)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult2 = s_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId2)->GetResult();
    EXPECT_SUCCESS(userInfoResult2);

    EXPECT_EQ(wantedUserId1, userInfoResult1.GetValue()->GetId());
    EXPECT_EQ(wantedUserId2, userInfoResult2.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QueryUsersInfoByTwoIdsTogether)
    {
    auto wantedUserId1 = s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult = s_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String>{wantedUserId1, wantedUserId2})->GetResult();
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
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1.c_str());

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2.c_str());

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3.c_str());

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId1.c_str(), userId1.c_str());

        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId2.c_str(), userId2.c_str());

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
    WebServices::ClientInfoPtr clientInfo = IntegrationTestsSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(s_info->GetServerURL(), s_info->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    s_connection->SetRepositoryClient(newClient);

    // Should not succeed when user does not exist
    auto user3InfoResult = s_connection->GetUserInfoManager().QueryUserInfoById(userId3)->GetResult();
    EXPECT_FALSE(user3InfoResult.IsSuccess());

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = s_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return user2
    auto user2InfoResult = s_connection->GetUserInfoManager().QueryUserInfoById(userId2)->GetResult();
    EXPECT_SUCCESS(user2InfoResult);
    EXPECT_EQ(userId2, user2InfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(UserInfoTests, QuerySameUserInfoTwice)
    {
    auto userId = s_connection->QueryBriefcaseInfo(m_briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    double start1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult1 = s_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);
    EXPECT_EQ(userId, userInfoResult1.GetValue()->GetId());
    double end1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    double duration1 = end1 - start1;

    double start2 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult2 = s_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
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
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1.c_str());

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2.c_str());

        Utf8String user23Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s','%s'%%5D", userId2.c_str(), userId3.c_str());

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3.c_str());

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId1.c_str(), userId1.c_str());

        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId2.c_str(), userId2.c_str());

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
    WebServices::ClientInfoPtr clientInfo = IntegrationTestsSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(s_info->GetServerURL(), s_info->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    s_connection->SetRepositoryClient(newClient);

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = s_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return only user1 and user2
    auto bothUsersInfoResult = s_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String> { userId2, userId1, userId3 })->GetResult();
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
