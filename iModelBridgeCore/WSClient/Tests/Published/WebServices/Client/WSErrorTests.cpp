/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/WSErrorTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Client/WSError.h>
#include "WSErrorTests.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

TEST_F(WSErrorTests, Ctor_Default_SetsStatusNone)
    {
    EXPECT_EQ(WSError::Status::None, WSError().GetStatus());
    }

TEST_F(WSErrorTests, Ctor_Default_LocalizedStringsEmpty)
    {
    EXPECT_EQ("", WSError().GetDisplayMessage());
    EXPECT_EQ("", WSError().GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_Default_IdUnknown)
    {
    EXPECT_EQ(WSError::Id::Unknown, WSError().GetId());
    }

TEST_F(WSErrorTests, CreateServerNotSupported_NewError_SetsStatusAndLocalizedMessage)
    {
    auto error = WSError::CreateServerNotSupportedError();
    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, CreateFunctionalityNotSupportedError_NewError_SetsStatusAndIdAndLocalizedMessage)
    {
    auto error = WSError::CreateFunctionalityNotSupportedError();
    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::NotSupported, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_CanceledHttpResponse_SetsStatusCanceled)
    {
    WSError error(StubHttpResponse(ConnectionStatus::Canceled));
    EXPECT_EQ(WSError::Status::Canceled, error.GetStatus());
    }

TEST_F(WSErrorTests, Ctor_HttpResponseWithConnectionStatusNonOkOrCanceled_SetsStatusNetworkErrorsOccurred)
    {
    EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(ConnectionStatus::None)).GetStatus());
    EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(ConnectionStatus::CouldNotConnect)).GetStatus());
    EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(ConnectionStatus::ConnectionLost)).GetStatus());
    EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(ConnectionStatus::Timeout)).GetStatus());
    EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(ConnectionStatus::UnknownError)).GetStatus());
    }

TEST_F(WSErrorTests, Ctor_JsonErrorFormatHasMissingField_SetsStatusServerNotSupported)
    {
    auto body = R"({"errorId":null, "errorMessage":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

TEST_F(WSErrorTests, Ctor_JsonErrorFormatCorrectButContentTypeXml_SetsStatusServerNotSupported)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

TEST_F(WSErrorTests, Ctor_XmlErrorFormatCorrectAndContentTypeXml_ParsesXmlAndSetsError)
    {
    auto body = R"( <ModelError 
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance" 
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("TestMessage\nTestDescription", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_XmlErrorFormatCorrectWithNullDescription_ParsesXmlAndSetsError)
    {
    auto body = R"( <ModelError 
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance" 
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorMessage>Foo</errorMessage>
                        <errorDescription i:nil="true" />
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("Foo", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_JsonErrorFormatWithNullFields_FallbacksToDefaultIdAndLocalizedMessageFromHttpError)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_ClassNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

TEST_F(WSErrorTests, Ctor_FileNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"FileNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::FileNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

TEST_F(WSErrorTests, Ctor_InstanceNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"InstanceNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

TEST_F(WSErrorTests, Ctor_ISMRedirectResponse_SetsIdLoginFailedWithLocalizedMessage)
    {
    WSError error(StubHttpResponseWithUrl(HttpStatus::OK, "http://foo/IMS/Account/Login?foo"));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::LoginFailed, error.GetId());
    EXPECT_EQ(HttpError::GetHttpDisplayMessage(HttpStatus::Unauthorized), error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

TEST_F(WSErrorTests, Ctor_HttpStatus500_SetsIdServerError)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::ServerError, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_HttpStatus500WithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_HttpStatusConflict_SetsIdConflict)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_HttpStatusConflictWithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_HttpStatusBadRequest_SetsIdBadRequest)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::BadRequest, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_HttpStatusBadRequestWithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi1ErrorIdObjectNotFound_SetsIdInstanceNotFound)
    {
    auto body = R"({"errorId":"ObjectNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi2ErrorInstanceNotFound_SetsIdInstanceNotFound)
    {
    auto body = R"({"errorId":"InstanceNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi1ErrorDatasourceNotFound_SetsIdRepositoryNotFound)
    {
    auto body = R"({"errorId":"DatasourceNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::RepositoryNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi2ErrorRepositoryNotFound_SetsIdRepositoryNotFound)
    {
    auto body = R"({"errorId":"RepositoryNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::RepositoryNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi1ErrorLinkTypeNotFound_SetsIdClassNotFound)
    {
    auto body = R"({"errorId":"LinkTypeNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_WebApi2ErrorSchemaNotFound_SetsIdSchemaNotFound)
    {
    auto body = R"({"errorId":"SchemaNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::SchemaNotFound, error.GetId());
    }

TEST_F(WSErrorTests, Ctor_ErrorFileNotFound_SetsIdFileNotFound)
    {
    auto body = R"({"errorId":"FileNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/json"}}));

    EXPECT_EQ(WSError::Id::FileNotFound, error.GetId());
    }
