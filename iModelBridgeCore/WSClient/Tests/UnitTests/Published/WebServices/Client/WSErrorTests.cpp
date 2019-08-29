/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Client/WSError.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct WSErrorTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_Default_SetsStatusNone)
    {
    EXPECT_EQ(WSError::Status::None, WSError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_Default_LocalizedStringsEmpty)
    {
    EXPECT_EQ("", WSError().GetDisplayMessage());
    EXPECT_EQ("", WSError().GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_Default_IdUnknown)
    {
    EXPECT_EQ(WSError::Id::Unknown, WSError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894649
TEST_F(WSErrorTests, CreateServerNotSupported_NewError_SetsStatusAndLocalizedMessage)
    {
    auto error = WSError::CreateServerNotSupportedError();
    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, CreateFunctionalityNotSupportedError_NewError_SetsStatusAndIdAndLocalizedMessage)
    {
    auto error = WSError::CreateFunctionalityNotSupportedError();
    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::NotSupported, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, IsInstanceNotAvailableError_ChecksAllErrorIds_ReturnTrueOnlyWithNotFoundOrNotEnoughRights)
    {
    for (int id = (int)WSError::Id::Unknown; id < (int)WSError::Id::_Last; id++)
        {
        auto errorId = (WSError::Id)id;
        auto error = WSError(errorId);
        if (errorId == WSError::Id::InstanceNotFound || errorId == WSError::Id::NotEnoughRights)
            EXPECT_TRUE(error.IsInstanceNotAvailableError());
        else
            EXPECT_FALSE(error.IsInstanceNotAvailableError());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_CanceledHttpResponse_SetsStatusCanceled)
    {
    WSError error(StubHttpResponse(ConnectionStatus::Canceled));
    EXPECT_EQ(WSError::Status::Canceled, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpResponseWithNotHandledConnectionStatuses_SetsStatusConnectionError)
    {
    for (int i = (int) ConnectionStatus::None; i <= (int) ConnectionStatus::UnknownError; i++)
        {
        auto status = (ConnectionStatus) i;
        if (status == ConnectionStatus::OK)
            continue;
        if (status == ConnectionStatus::Canceled)
            continue;
        if (status == ConnectionStatus::CertificateError)
            continue;
        EXPECT_EQ(WSError::Status::ConnectionError, WSError(StubHttpResponse(status)).GetStatus());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpResponseWithCertificateError_SetsStatusCertificateError)
    {
    EXPECT_EQ(WSError::Status::CertificateError, WSError(StubHttpResponse(ConnectionStatus::CertificateError)).GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonErrorFormatHasMissingRequiredFieldErrorMessage_SetsStatusServerNotSupported)
    {
    auto body = R"({"errorId":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonErrorFormatCorrectButContentTypeXml_SetsStatusServerNotSupported)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}}));

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_XmlErrorFormatMissingRequiredFieldErrorMessage_SetsStatusServerNotSupported)
    {
    auto body = R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_XmlErrorFormatMissingOptionalFields_ParsesXmlAndSetsError)
    {
    auto body = R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorMessage>TestMessage</errorMessage>
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("TestMessage", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894649
TEST_F(WSErrorTests, Ctor_XmlErrorFormatCorrectAndContentTypeXml_ParsesXmlAndSetsError)
    {
    auto body = R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("TestMessage\nTestDescription", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_XmlErrorFormatCorrectWithNullDescription_ParsesXmlAndSetsError)
    {
    auto body = R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorMessage>Foo</errorMessage>
                        <errorDescription i:nil="true" />
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("Foo", error.GetDisplayDescription());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894649
TEST_F(WSErrorTests, Ctor_XmlAzureBlobNotFoundError_ParsesXmlAndSetsError)
    {
    auto body = R"(<?xml version="1.0" encoding="utf-8"?>
        <Error>
            <Code>BlobNotFound</Code>
            <Message>TestMessage</Message>
        </Error>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::FileNotFound, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("TestMessage", error.GetDisplayDescription());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_XmlAzureOtherError_ParsesXmlAndSetsError)
    {
    auto body = R"(<?xml version="1.0" encoding="utf-8"?>
        <Error>
            <Code>Foo</Code>
            <Message>TestMessage</Message>
        </Error>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", "application/xml"}}));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_NE("", error.GetDisplayMessage());
    EXPECT_EQ("TestMessage", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonErrorFormatWithNullFields_FallbacksToDefaultIdAndLocalizedMessageFromHttpError)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_ClassNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_FileNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"FileNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::FileNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_InstanceNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto body = R"({"errorId":"InstanceNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
    EXPECT_NE(HttpError(httpResponse).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_BadRequestError_SetsRecievedMessageAndDescriptionForUser)
    {
    auto body = R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::BadRequest, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("DESCRIPTION", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_ConflictError_SetsRecievedMessageAndDescriptionForUser)
    {
    auto body = R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})";
    auto httpResponse = StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("DESCRIPTION", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_MissingRequiredFieldErrorMessage_SetsStatusServerNotSupported)
    {
    auto body = R"({"errorId":null, "errorDescription":"DESCRIPTION", "httpStatusCode" : 409})";
    auto httpResponse = StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RequiredFieldsOnly_SetsRecievedMessageAndDescriptionForUser)
    {
    auto body = R"({"errorMessage":"MESSAGE", "httpStatusCode" : 409})";
    auto httpResponse = StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", "application/json"}});
    WSError error(httpResponse);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonValueWithNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto json = ToJson(R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 404})");
    WSError error(json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(HttpError(ConnectionStatus::OK, HttpStatus::NotFound).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonValueWithClassNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto json = ToJson(R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 404})");
    WSError error(json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE(HttpError(ConnectionStatus::OK, HttpStatus::NotFound).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonValueWithoutHttpStatusCodeField_SetsStatusServerNotSupported)
    {
    auto json = ToJson(R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})");
    WSError error(json);

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonValueConflictError_SetsRecievedMessageAndDescriptionForUser)
    {
    auto json = ToJson(R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 409})");
    WSError error(json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("DESCRIPTION", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_JsonValueWithRequiredFieldsOnly_SetsRecievedMessageAndDescriptionForUser)
    {
    auto json = ToJson(R"({"errorMessage":"MESSAGE", "httpStatusCode" : 409})");
    WSError error(json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RapidJsonValueWithNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto json = ToRapidJson(R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 404})");
    WSError error(*json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(HttpError(ConnectionStatus::OK, HttpStatus::NotFound).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RapidJsonValueWithClassNotFoundError_SetsErrorReceivedStatusAndIdWithLocalizedMessage)
    {
    auto json = ToRapidJson(R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 404})");
    WSError error(*json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ClassNotFound, error.GetId());
    EXPECT_NE(HttpError(ConnectionStatus::OK, HttpStatus::NotFound).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_TRUE(error.GetDisplayDescription().find("MESSAGE") != Utf8String::npos);
    EXPECT_TRUE(error.GetDisplayDescription().find("DESCRIPTION") != Utf8String::npos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RapidJsonValueWithoutHttpStatusCodeField_SetsStatusServerNotSupported)
    {
    auto json = ToRapidJson(R"({"errorId":"ClassNotFound", "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION"})");
    WSError error(*json);

    EXPECT_EQ(WSError::Status::ServerNotSupported, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RapidJsonValueConflictError_SetsRecievedMessageAndDescriptionForUser)
    {
    auto json = ToRapidJson(R"({"errorId":null, "errorMessage":"MESSAGE", "errorDescription":"DESCRIPTION", "httpStatusCode" : 409})");
    WSError error(*json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("DESCRIPTION", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_RapidJsonValueWithRequiredFieldsOnly_SetsRecievedMessageAndDescriptionForUser)
    {
    auto json = ToRapidJson(R"({"errorMessage":"MESSAGE", "httpStatusCode" : 409})");
    WSError error(*json);

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    EXPECT_EQ("MESSAGE", error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_ISMRedirectResponse_SetsIdAndLocalizedMessage)
    {
    WSError error(StubHttpResponseWithUrl(HttpStatus::OK, "http://foo/IMS/Account/Login?foo"));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::LoginFailed, error.GetId());
    EXPECT_EQ(HttpError(ConnectionStatus::OK, HttpStatus::Unauthorized).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_ProxyAuthenticationRequiredResponse_SetsIdAndLocalizedMessage)
    {
    WSError error(StubHttpResponse(HttpStatus::ProxyAuthenticationRequired));

    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::ProxyAuthenticationRequired, error.GetId());
    EXPECT_EQ(HttpError(ConnectionStatus::OK, HttpStatus::ProxyAuthenticationRequired).GetDisplayMessage(), error.GetDisplayMessage());
    EXPECT_EQ("", error.GetDisplayDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatus500_SetsIdServerError)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::ServerError, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatus500WithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatusConflict_SetsIdConflict)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::Conflict, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatusConflictWithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::Conflict, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatusBadRequest_SetsIdBadRequest)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::BadRequest, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpStatusBadRequestWithErrorId_SetsIdFromError)
    {
    auto body = R"({"errorId":"NoClientLicense", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::BadRequest, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::NoClientLicense, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_WebApi2ErrorInstanceNotFound_SetsIdInstanceNotFound)
    {
    auto body = R"({"errorId":"InstanceNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::InstanceNotFound, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_WebApi2ErrorRepositoryNotFound_SetsIdRepositoryNotFound)
    {
    auto body = R"({"errorId":"RepositoryNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::RepositoryNotFound, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_WebApi2ErrorSchemaNotFound_SetsIdSchemaNotFound)
    {
    auto body = R"({"errorId":"SchemaNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::SchemaNotFound, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_ErrorFileNotFound_SetsIdFileNotFound)
    {
    auto body = R"({"errorId":"FileNotFound", "errorMessage":null, "errorDescription":null})";
    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    EXPECT_EQ(WSError::Id::FileNotFound, error.GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpErrorCanceled_Canceled)
    {
    HttpError httpError(ConnectionStatus::Canceled, HttpStatus::None);
    WSError error(httpError);
    EXPECT_EQ(WSError::Status::Canceled, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ("", error.GetMessage());
    EXPECT_EQ("", error.GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpErrorConnectionError_ConnectionError)
    {
    HttpError httpError(ConnectionStatus::CouldNotConnect, HttpStatus::None);
    WSError error(httpError);
    EXPECT_EQ(WSError::Status::ConnectionError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(httpError.GetMessage(), error.GetMessage());
    EXPECT_EQ("", error.GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_HttpErrorNotFound_IdUnknown)
    {
    HttpError httpError(ConnectionStatus::OK, HttpStatus::NotFound);
    WSError error(httpError);
    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ(httpError.GetMessage(), error.GetMessage());
    EXPECT_EQ("", error.GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, GetData_JsonResponse_ReturnsSameJsonBody)
    {
    auto body = R"({"errorId":null, "errorMessage":null, "errorDescription":null, "customProperty":"TestData"})";
    WSError error(StubHttpResponse(HttpStatus::InternalServerError, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}}));

    JsonValueCR data = error.GetData();
    EXPECT_NE(Json::Value::GetNull(), data);
    EXPECT_TRUE(data.isMember("customProperty"));
    EXPECT_EQ("TestData", data["customProperty"].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, GetData_CanceledHttpStatus_JsonNull)
    {
    WSError error(StubHttpResponse(ConnectionStatus::Canceled));

    EXPECT_EQ(Json::Value::GetNull(), error.GetData());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, GetData_HttpStatusNotOkOrCanceledResponse_JsonNull)
    {
    EXPECT_EQ(Json::Value::GetNull(), WSError(StubHttpResponse(ConnectionStatus::None)).GetData());
    EXPECT_EQ(Json::Value::GetNull(), WSError(StubHttpResponse(ConnectionStatus::CouldNotConnect)).GetData());
    EXPECT_EQ(Json::Value::GetNull(), WSError(StubHttpResponse(ConnectionStatus::ConnectionLost)).GetData());
    EXPECT_EQ(Json::Value::GetNull(), WSError(StubHttpResponse(ConnectionStatus::Timeout)).GetData());
    EXPECT_EQ(Json::Value::GetNull(), WSError(StubHttpResponse(ConnectionStatus::UnknownError)).GetData());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, GetData_IMSRedirectResponse_JsonNull)
    {
    WSError error(StubHttpResponseWithUrl(HttpStatus::OK, "http://foo/IMS/Account/Login?foo"));

    EXPECT_EQ(Json::Value::GetNull(), error.GetData());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, GetData_XmlBody_JsonNull)
    {
    auto body = R"( <ModelError
                      xmlns:i="http://www.w3.org/2001/XMLSchema-instance"
                      xmlns="http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models">
                        <errorId>ClassNotFound</errorId>
                        <errorMessage>TestMessage</errorMessage>
                        <errorDescription>TestDescription</errorDescription>
                    </ModelError>)";

    WSError error(StubHttpResponse(HttpStatus::NotFound, body, {{"Content-Type", REQUESTHEADER_ContentType_ApplicationXml}}));

    EXPECT_EQ(Json::Value::GetNull(), error.GetData());
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSErrorTests, Ctor_InvalidtHttpError_NoneStatus)
    {
    HttpError httpError;
    WSError error(httpError);
    EXPECT_EQ(WSError::Status::None, error.GetStatus());
    EXPECT_EQ(WSError::Id::Unknown, error.GetId());
    EXPECT_EQ("", error.GetMessage());
    EXPECT_EQ("", error.GetDescription());
    }
