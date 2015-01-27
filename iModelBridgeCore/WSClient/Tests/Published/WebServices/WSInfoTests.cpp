/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WSInfoTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/Client/WSInfo.h>
#include "WSInfoTests.h"
#include "WebServicesTestsHelper.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

WSInfo CreateWSInfoWithVersion (Utf8CP versionString)
    {
    return WSInfo (StubHttpResponse (HttpStatus::OK, Utf8PrintfString (R"({"serverVersion" : "%s"})", versionString)));
    }

TEST_F (WSInfoTests, IsValid_DefaultCtor_False)
    {
    EXPECT_FALSE (WSInfo ().IsValid ());
    }

TEST_F (WSInfoTests, IsValid_NotExecutedResponse_False)
    {
    HttpResponse response;
    EXPECT_FALSE (WSInfo (response).IsValid ());
    }

TEST_F (WSInfoTests, IsValid_ServerInfoWithVersion_True)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::OK, R"({"serverVersion" : "01.02.09.10"})");
    EXPECT_TRUE (WSInfo (response).IsValid ());
    }

TEST_F (WSInfoTests, IsValid_ServerInfoWithVersionZero_False)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::OK, R"({"serverVersion" : "00.00.00.00"})");
    EXPECT_FALSE (WSInfo (response).IsValid ());
    }

TEST_F (WSInfoTests, IsValid_SuccessResultButMalformedBody_False)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::OK, "a{22");
    EXPECT_FALSE (WSInfo (response).IsValid ());
    }

TEST_F (WSInfoTests, IsValid_NotFoundStatus_False)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::NotFound);
    EXPECT_FALSE (WSInfo (response).IsValid ());
    }

TEST_F (WSInfoTests, Ctor_Default_InvalidWithZeroVersionAndUnknownType)
    {
    WSInfo info;
    EXPECT_FALSE (info.IsValid ());
    EXPECT_EQ (BeVersion (0, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::Unknown, info.GetType ());
    }

TEST_F (WSInfoTests, Ctor_DefaultResponse_InvalidWithZeroVersionAndUnknownType)
    {
    HttpResponse response;
    WSInfo info (response);
    EXPECT_FALSE (info.IsValid ());
    EXPECT_EQ (BeVersion (0, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::Unknown, info.GetType ());
    }

TEST_F (WSInfoTests, Ctor_EmptyResponseBody_InvalidWithZeroVersionAndUnknownType)
    {
    WSInfo info (StubHttpResponse (HttpStatus::OK));
    EXPECT_FALSE (info.IsValid ());
    EXPECT_EQ (BeVersion (0, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::Unknown, info.GetType ());
    }

TEST_F (WSInfoTests, Ctor_ValidServerVersionInBody_ValidWithSameVersionAndWSGType)
    {
    WSInfo info (StubHttpResponse (HttpStatus::OK, R"({"serverVersion" : "01.02.09.10"})"));
    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (1, 2, 9, 10), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, info.GetType ());
    }

TEST_F (WSInfoTests, Ctor_EmptyString_InvalidWithZeroVersionAndUnknownType)
    {
    WSInfo info ("");
    EXPECT_FALSE (info.IsValid ());
    EXPECT_EQ (BeVersion (0, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::Unknown, info.GetType ());
    }

TEST_F (WSInfoTests, Ctor_FromSerialized_SameVersion)
    {
    WSInfo info (WSInfo (BeVersion (1, 2), WSInfo::Type::BentleyWSG).ToString ());

    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (1, 2), info.GetVersion ());
    }

TEST_F (WSInfoTests, Ctor_FromSerialized_SameType)
    {
    EXPECT_EQ (WSInfo::Type::BentleyConnect, WSInfo (WSInfo (BeVersion (1, 0), WSInfo::Type::BentleyConnect).ToString ()).GetType ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, WSInfo (WSInfo (BeVersion (1, 0), WSInfo::Type::BentleyWSG).ToString ()).GetType ());
    EXPECT_EQ (WSInfo::Type::Unknown, WSInfo (WSInfo (BeVersion (1, 0), WSInfo::Type::Unknown).ToString ()).GetType ());
    }

TEST_F (WSInfoTests, GetVersion_ButHttpStatusNotFound_ReturnsZero)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::NotFound);
    EXPECT_EQ (BeVersion (0, 0), WSInfo (response).GetVersion ());
    }

TEST_F (WSInfoTests, GetVersion_SuccessResponseWithHtmlContentAndWithUnknownData_InvalidWithZeroVersionAndUnknownType)
    {
    WSInfo info (StubHttpResponse (HttpStatus::OK, "some html", {{"Content-Type", "text/html"}}));
    EXPECT_FALSE (info.IsValid ());
    EXPECT_EQ (BeVersion (0, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::Unknown, info.GetType ());
    }

TEST_F (WSInfoTests, GetVersion_SuccessResponseWithR1AboutPageHtml_TreatsAsR1AndReturnsOne)
    {
    auto bodyStub = R"(..stub.. <span id="productNameLabel">Bentley Web Services Gateway 01.00</span> ..stub..)";
    WSInfo info (StubHttpResponse (HttpStatus::OK, bodyStub, {{"Content-Type", "text/html"}}));

    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (1, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, info.GetType ());
    }

TEST_F (WSInfoTests, GetVersion_SuccessResponseWithR1BentleyConnectAboutPageHtml_TreatsAsR1AndReturnsOneAsWellAsBentleyConnectType)
    {
    auto bodyStub = R"(..stub.. Web Service Gateway for BentleyCONNECT ..stub.. <span id="versionLabel">1.1.0.0</span> ..stub..)";
    WSInfo info (StubHttpResponse (HttpStatus::OK, bodyStub, {{"Content-Type", "text/html"}}));

    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (1, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::BentleyConnect, info.GetType ());
    }

TEST_F (WSInfoTests, GetVersion_ValidServerVersionInBodyButHttpStatusNonOk_ReturnsZeroVersion)
    {
    HttpResponse response = StubHttpResponse (HttpStatus::InternalServerError, R"({"serverVersion" : "01.02.09.10"})");
    EXPECT_EQ (BeVersion (0, 0), WSInfo (response).GetVersion ());
    }

TEST_F (WSInfoTests, GetVersion_SuccessResponseWithWSGServerHeader_TreatsAsWSG2AndReturnsOneAsWellAsBentleyWSGType)
    {
    auto bodyStub = R"({json: "whatever"})";
    WSInfo info (StubHttpResponse (HttpStatus::OK, bodyStub, {{"Server", "Bentley-WebAPI/2.0"}}));

    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (2, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, info.GetType ());
    }

TEST_F (WSInfoTests, GetVersion_SuccessResponseWithMultipleWSGServerHeader_TreatsAsWSG2AndReturnsOneAsWellAsBentleyWSGType)
    {
    auto bodyStub = R"({json: "whatever"})";
    WSInfo info (StubHttpResponse (HttpStatus::OK, bodyStub, {{"Server", "Bentley-WebAPI/2.0, Microsoft-IIS/8.5"}}));

    EXPECT_TRUE (info.IsValid ());
    EXPECT_EQ (BeVersion (2, 0), info.GetVersion ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, info.GetType ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ZeroVersion_Returns1_1)
    {
    WSInfo info = CreateWSInfoWithVersion ("");
    EXPECT_EQ (BeVersion (1, 1), info.GetWebApiVersion ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ServerR1_Returns1_1)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.00.00.00");
    EXPECT_EQ (BeVersion (1, 1), info.GetWebApiVersion ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ServerR2_Returns1_2)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.01.00.00");
    EXPECT_EQ (BeVersion (1, 2), info.GetWebApiVersion ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ServerR3_Returns1_3)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.02.00.00");
    EXPECT_EQ (BeVersion (1, 3), info.GetWebApiVersion ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ServerR4_Returns2_0)
    {
    WSInfo info = CreateWSInfoWithVersion ("02.00.00.00");
    EXPECT_EQ (BeVersion (2, 0), info.GetWebApiVersion ());
    }

TEST_F (WSInfoTests, GetWebApiVersion_ArgumentGreaterThanWebApiVersion_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("02.00.00.00");
    EXPECT_FALSE (info.IsWebApiSupported (BeVersion (2, 1)));
    }

TEST_F (WSInfoTests, GetWebApiVersion_ArgumentEqualToWebApiVersion_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("02.00.00.00");
    EXPECT_TRUE (info.IsWebApiSupported (BeVersion (2, 0)));
    }

TEST_F (WSInfoTests, GetWebApiVersion_ArgumentLessThanWebApiVersion_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("02.00.00.00");
    EXPECT_TRUE (info.IsWebApiSupported (BeVersion (1, 9)));
    }

TEST_F (WSInfoTests, IsNavigationPropertySelectForAllClassesSupported_ServerR2_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.01.00.00");
    EXPECT_FALSE (info.IsNavigationPropertySelectForAllClassesSupported ());
    }

TEST_F (WSInfoTests, IsNavigationPropertySelectForAllClassesSupported_ServerR3_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.02.00.00");
    EXPECT_TRUE (info.IsNavigationPropertySelectForAllClassesSupported ());
    }

TEST_F (WSInfoTests, IsSchemaDownloadFullySupported_ServerR2_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.01.00.00");
    EXPECT_FALSE (info.IsSchemaDownloadFullySupported ());
    }

TEST_F (WSInfoTests, IsSchemaDownloadFullySupported_ServerR3_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.02.00.00");
    EXPECT_TRUE (info.IsSchemaDownloadFullySupported ());
    }

TEST_F (WSInfoTests, IsR2OrGreater_ServerR1_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.00.00.00");
    EXPECT_FALSE (info.IsR2OrGreater ());
    }

TEST_F (WSInfoTests, IsR2OrGreater_ServerR2_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.01.00.00");
    EXPECT_TRUE (info.IsR2OrGreater ());
    }

TEST_F (WSInfoTests, IsR3OrGreater_ServerR2_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.01.00.00");
    EXPECT_FALSE (info.IsR3OrGreater ());
    }

TEST_F (WSInfoTests, IsR3OrGreater_ServerR3_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.02.00.00");
    EXPECT_TRUE (info.IsR3OrGreater ());
    }

TEST_F (WSInfoTests, IsR4OrGreater_ServerR3_False)
    {
    WSInfo info = CreateWSInfoWithVersion ("01.02.00.00");
    EXPECT_FALSE (info.IsR4OrGreater ());
    }

TEST_F (WSInfoTests, IsR4OrGreater_ServerR4_True)
    {
    WSInfo info = CreateWSInfoWithVersion ("02.00.00.00");
    EXPECT_TRUE (info.IsR4OrGreater ());
    }
