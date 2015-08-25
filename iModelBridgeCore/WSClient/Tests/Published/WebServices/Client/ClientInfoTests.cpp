/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/ClientInfoTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInfoTests.h"
#include <WebServices/Client/ClientInfo.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

TEST_F(ClientInfoTests, DefaultLanguage_Constant_NotEmpty)
    {
    EXPECT_STRNE("", ClientInfo::DefaultLanguage);
    EXPECT_STRNE("", Utf8String(ClientInfo::DefaultLanguage).Trim().c_str());
    }

TEST_F(ClientInfoTests, Create_PassedMandatoryValues_SetsValues)
    {
    auto info = ClientInfo::Create("Test-AppName", BeVersion(4, 2, 6, 9), "TestAppGUID");

    EXPECT_STREQ("Test-AppName", info->GetApplicationName().c_str());
    EXPECT_STREQ("TestAppGUID", info->GetApplicationGUID().c_str());

    EXPECT_EQ(BeVersion(4, 2, 6, 9), info->GetApplicationVersion());

    EXPECT_STRNE("", info->GetSystemDescription().c_str());
    EXPECT_STRNE("", info->GetDeviceId().c_str());
    EXPECT_STREQ("en", info->GetLanguage().c_str());
    }

TEST_F(ClientInfoTests, FillHttpRequestHeaders_ValuesPassedToCreate_SetsCorrespondingHeaders)
    {
    ClientInfo info("Test-AppName", BeVersion(4, 2, 6, 9), "TestAppGUID", "TestDeviceId", "TestSystem");

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);

    EXPECT_STREQ("Test-AppName/4.2 (TestSystem)", headers.GetUserAgent());
    EXPECT_STREQ("TestAppGUID", headers.GetValue("Mas-App-Guid"));
    EXPECT_STREQ("TestDeviceId", headers.GetValue("Mas-Uuid"));
    EXPECT_STREQ("en", headers.GetAcceptLanguage());
    }

TEST_F(ClientInfoTests, GetProductToken_PassedMandatoryValues_SetsValues)
    {
    ClientInfo info("TestAppName", BeVersion(4, 2, 6, 9), "TestAppGUID", "TestDeviceId", "TestSystem");
    EXPECT_STREQ("TestAppName/4.2", info.GetProductToken().c_str());
    }

TEST_F(ClientInfoTests, FillHttpRequestHeaders_SameHeadersWithValues_OverridesExistingValues)
    {
    ClientInfo info("Test-AppName", BeVersion(4, 2, 6, 9), "TestAppGUID", "TestDeviceId", "TestSystem");

    HttpRequestHeaders headers;
    headers.SetUserAgent("OtherAgent");
    headers.SetValue("Mas-App-Guid", "OtherAppId");
    headers.SetValue("Mas-Uuid", "OtherDeviceId");
    info.FillHttpRequestHeaders(headers);

    EXPECT_STREQ("Test-AppName/4.2 (TestSystem)", headers.GetUserAgent());
    EXPECT_STREQ("TestAppGUID", headers.GetValue("Mas-App-Guid"));
    EXPECT_STREQ("TestDeviceId", headers.GetValue("Mas-Uuid"));
    }

TEST_F(ClientInfoTests, FillHttpRequestHeaders_UnrelatedHeadersWithValues_DoesNotRemoveExistingHeaders)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    HttpRequestHeaders headers;
    headers.SetValue("Test-Header", "TestValue");
    info.FillHttpRequestHeaders(headers);

    EXPECT_STREQ("TestValue", headers.GetValue("Test-Header"));
    }

TEST_F(ClientInfoTests, FillHttpRequestHeaders_UsingPrimaryRequestProvider_IncludesPrimaryHeaders)
    {
    HttpRequestHeaders primaryHeaders;
    primaryHeaders.SetValue("Test-Header", "TestValue");
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo", HttpHeaderProvider::Create(primaryHeaders));

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);

    EXPECT_STREQ("TestValue", headers.GetValue("Test-Header"));
    EXPECT_STREQ("Foo", headers.GetValue("Mas-Uuid"));
    }

TEST_F(ClientInfoTests, FillHttpRequestHeaders_UsingPrimaryRequestProviderWithSameHeaders_OverridesHeaders)
    {
    HttpRequestHeaders primaryHeaders;
    primaryHeaders.SetValue("Mas-Uuid", "OtherValue");
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "TestDeviceId", "Foo", HttpHeaderProvider::Create(primaryHeaders));

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);

    EXPECT_STREQ("TestDeviceId", headers.GetValue("Mas-Uuid"));
    }

TEST_F(ClientInfoTests, SetLanguage_DefaultLanguage_LeavesHeaderAsIs)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());

    info.SetLanguage("en");
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());
    }

TEST_F(ClientInfoTests, SetLanguage_DefaultLanguageInDifferentCase_LeavesHeaderAsIs)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());

    info.SetLanguage("eN");
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());
    }

TEST_F(ClientInfoTests, SetLanguage_NotDefaultLanguage_AddsLanguageToHeaderWithFallbackLanguageWithLowerPriority)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());

    info.SetLanguage("en-US");
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en-US, en;q=0.6", headers.GetAcceptLanguage());
    }

TEST_F(ClientInfoTests, SetFallbackLanguage_DifferentFromSetLanguage_AddsLanguageToHeaderWithFallbackLanguageWithLowerPriority)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    HttpRequestHeaders headers;
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en", headers.GetAcceptLanguage());

    info.SetFallbackLanguage("lt");
    info.FillHttpRequestHeaders(headers);
    EXPECT_STREQ("en, lt;q=0.6", headers.GetAcceptLanguage());
    }

TEST_F(ClientInfoTests, SetLanguage_FallbackLanguageInDifferentCase_SetsFallback)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetFallbackLanguage("en");
    info.SetLanguage("eN");
    EXPECT_STREQ("en", info.GetLanguage().c_str());
    }

TEST_F(ClientInfoTests, SetLanguage_NotFallbackLanguage_SetsLanguage)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetFallbackLanguage("lt");
    info.SetLanguage("en-US");
    EXPECT_STREQ("en-US", info.GetLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetFallbackLanguage_NotFallbackLanguage_ReturnsLanguageSet)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetFallbackLanguage("en-US");
    EXPECT_STREQ("en-US", info.GetFallbackLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_Default_ReturnsDefaultLanguage)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    EXPECT_STREQ(ClientInfo::DefaultLanguage, info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_LanguageSetToEmptyAndFallbackLanguageSet_ReturnsFallbackLanguage)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetLanguage("");
    info.SetFallbackLanguage("lt");

    EXPECT_STREQ("lt", info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_LanguageSetToEmptyAndFallbackLanguageSetToEmpty_ReturnsDefaultFallbackLanguage)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetLanguage("");
    info.SetFallbackLanguage("");

    EXPECT_STREQ(ClientInfo::DefaultLanguage, info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_LanguageSetFallbackLanguageSetToEmpty_ReturnsLanguage)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetLanguage("lt");
    info.SetFallbackLanguage("");

    EXPECT_STREQ("lt", info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_LanguageAndFallbackLanguageSet_ReturnsSetLanguagesWithPriority)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetFallbackLanguage("lt");
    info.SetLanguage("en-US");

    EXPECT_STREQ("en-US, lt;q=0.6", info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_SetLanguageIsNotDefaultLanguage_FallbackLanguageWithLowerPriority)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetLanguage("en-US");
    EXPECT_STREQ("en-US, en;q=0.6", info.GetAcceptLanguage().c_str());
    }

TEST_F(ClientInfoTests, GetAcceptLanguage_FallbackLanguageDifferentFromSetLanguage_FallbackLanguageWithLowerPriority)
    {
    ClientInfo info("Foo", BeVersion(1, 0, 0, 0), "Foo", "Foo", "Foo");

    info.SetFallbackLanguage("lt");
    EXPECT_STREQ("en, lt;q=0.6", info.GetAcceptLanguage().c_str());
    }
