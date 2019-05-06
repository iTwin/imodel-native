/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <map>
#include <mutex>
#include <WebServices/Connect/ImsClient.h>
#include <BeHttp/HttpStatusHelper.h>
#include <BeXml/BeXml.h>
#include <WebServices/Azure/AzureError.h>

#include "WSError.xliff.h"

#define JSON_ErrorId                "errorId"
#define JSON_ErrorMessage           "errorMessage"
#define JSON_ErrorDescription       "errorDescription"

#define JSON_ErrorHttpStatusCode    "httpStatusCode"

#define XML_NAMESPACE               "http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models"
#define XML_NAMESPACE_PREFIX        "error"
#define XML_ROOTNODE_NAME           "ModelError"
#define XML_ErrorId                 "error:errorId"
#define XML_ErrorMessage            "error:errorMessage"
#define XML_ErrorDescription        "error:errorDescription"

#define XML_Azure_Error         "Error"
#define XML_Azure_Code          "Code"
#define XML_Azure_Message       "Message"
#define XML_Azure_BlobNotFound  "BlobNotFound"

#define HEADER_MasRequestId "Mas-Request-Id"

std::once_flag s_initErrorIdmap;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::Id WSError::ErrorIdFromString(Utf8StringCR errorIdString)
    {
    //static local variables are not thread safe in VS2013 version of c++11 compiler
    //TODO: change this to old way after we move to VS2015
    std::call_once(s_initErrorIdmap, []() {GetErrorIdFromString(""); });
    return GetErrorIdFromString(errorIdString);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Basanta.Kharel    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::Id WSError::GetErrorIdFromString(Utf8StringCR errorIdString)
    {
    static std::map<Utf8String, Id> map =
        {
            {"LoginFailed", Id::LoginFailed},
            {"SslRequired", Id::SslRequired},
            {"NotEnoughRights", Id::NotEnoughRights},
            {"DatasourceNotFound", Id::RepositoryNotFound},
            {"RepositoryNotFound", Id::RepositoryNotFound},
            {"SchemaNotFound", Id::SchemaNotFound},
            {"ClassNotFound", Id::ClassNotFound},
            {"PropertyNotFound", Id::PropertyNotFound},
            {"LinkTypeNotFound", Id::ClassNotFound},
            {"ObjectNotFound", Id::InstanceNotFound},
            {"InstanceNotFound", Id::InstanceNotFound},
            {"FileNotFound", Id::FileNotFound},
            {"NotSupported", Id::NotSupported},
            {"NoServerLicense", Id::NoServerLicense},
            {"NoClientLicense", Id::NoClientLicense},
            {"TooManyBadLoginAttempts", Id::TooManyBadLoginAttempts},
        };

    auto it = map.find(errorIdString);
    if (it != map.end())
        return it->second;

    return Id::Unknown;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError() :
m_status(Status::None),
m_id(Id::Unknown)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(Id errorId) :
m_status(Status::ReceivedError),
m_id(errorId)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(Http::ResponseCR httpResponse) : WSError()
    {
    if (ConnectionStatus::OK == httpResponse.GetConnectionStatus() &&
        LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_INFO))
        {
        Utf8CP requestId = httpResponse.GetHeaders().GetValue(HEADER_MasRequestId);
        LOG.infov
            (
            "Received WSError: %d %s\n"
            "Server response: %s\n"
            "Request ID: '%s'",
            httpResponse.GetHttpStatus(),
            httpResponse.GetEffectiveUrl().c_str(),
            httpResponse.GetBody().AsString().c_str(),
            requestId ? requestId : "n/a"
            );
        }

    if (ConnectionStatus::Canceled == httpResponse.GetConnectionStatus())
        {
        m_data = nullptr;
        m_status = Status::Canceled;
        m_id = WSError::Id::Unknown;
        return;
        }

    if (ConnectionStatus::OK != httpResponse.GetConnectionStatus())
        {
        m_message = HttpError(httpResponse).GetMessage();
        m_data = nullptr;
        
        if (ConnectionStatus::CertificateError == httpResponse.GetConnectionStatus())
            m_status = Status::CertificateError;
        else
            m_status = Status::ConnectionError;

        m_id = WSError::Id::Unknown;
        return;
        }

    if (SUCCESS == ParseBody(httpResponse))
        return;

    if (ImsClient::IsLoginRedirect(httpResponse) ||                 // Bentley CONNECT login redirect
        HttpStatus::Unauthorized == httpResponse.GetHttpStatus())   // Bentley CONNECT token could not be retrieved
        {
        m_message = HttpError(ConnectionStatus::OK, HttpStatus::Unauthorized).GetDisplayMessage();
        m_data = nullptr;
        m_status = Status::ReceivedError;
        m_id = Id::LoginFailed;
        return;
        }

    if (HttpStatus::ProxyAuthenticationRequired == httpResponse.GetHttpStatus())
        {
        m_message = HttpError(httpResponse).GetDisplayMessage();
        m_status = Status::ReceivedError;
        m_id = Id::ProxyAuthenticationRequired;
        return;
        }

    SetStatusServerNotSupported();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(JsonValueCR jsonError)
    {
    int statusInt = jsonError[JSON_ErrorHttpStatusCode].asInt();
    if (statusInt == 0)
        {
        SetStatusServerNotSupported();
        return;
        }

    HttpStatus status = static_cast<HttpStatus>(statusInt);
    if (SUCCESS != ParseJsonError(jsonError, status))
        {
        SetStatusServerNotSupported();
        return;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(RapidJsonValueCR jsonError)
    {
    int statusInt = GetOptionalInt(jsonError, JSON_ErrorHttpStatusCode);
    if (statusInt == 0)
        {
        SetStatusServerNotSupported();
        return;
        }

    HttpStatus status = static_cast<HttpStatus>(statusInt);
    if (SUCCESS != ParseJsonError(jsonError, status))
        {
        SetStatusServerNotSupported();
        return;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(HttpErrorCR httpError) : WSError()
    {
    if (!httpError.IsValid())
        return;

    if (ConnectionStatus::Canceled == httpError.GetConnectionStatus())
        {
        m_status = Status::Canceled;
        m_id = WSError::Id::Unknown;
        return;
        }

    if (ConnectionStatus::OK != httpError.GetConnectionStatus())
        {
        m_message = httpError.GetMessage();
        m_status = Status::ConnectionError;
        m_id = WSError::Id::Unknown;
        return;
        }

    SetStatusReceivedError(httpError, Id::Unknown, nullptr, nullptr, nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(AzureErrorCR azureError) : WSError()
    {
    if (!azureError.IsValid())
        return;

    auto errorId = WSError::Id::Unknown;
    if (XML_Azure_BlobNotFound == azureError.GetCode())
        errorId = WSError::Id::FileNotFound;

    SetStatusReceivedError(azureError, errorId, azureError.GetMessage(), nullptr, nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseBody(Http::ResponseCR httpResponse)
    {
    Utf8String contentType = httpResponse.GetHeaders().GetContentType();
    if (contentType.find(REQUESTHEADER_ContentType_ApplicationJson) != Utf8String::npos)
        {
        // JSON is main error format
        return ParseJsonError(httpResponse);
        }
    else if (contentType.find(REQUESTHEADER_ContentType_ApplicationXml) != Utf8String::npos)
        {
        // XML format can occur when requesting XML ECSchema from v1.x server.
        return ParseXmlError(httpResponse);
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseJsonError(Http::ResponseCR httpResponse)
    {
    Json::Value jsonError = Json::Reader::DoParse(httpResponse.GetBody().AsString());
    return ParseJsonError(jsonError, httpResponse.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseJsonError(JsonValueCR jsonError, HttpStatus status)
    {
    if (!DoesStringFieldExist(jsonError, JSON_ErrorMessage))
        return ERROR;

    WSError::Id errorId = ErrorIdFromString(jsonError[JSON_ErrorId].asString());
    Utf8String errorMessage = jsonError[JSON_ErrorMessage].asString();
    Utf8String errorDescription = jsonError[JSON_ErrorDescription].asString();

    SetStatusReceivedError(HttpError(ConnectionStatus::OK, status), errorId, errorMessage, errorDescription, &jsonError);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseJsonError(RapidJsonValueCR jsonError, HttpStatus status)
    {
    if (!DoesStringFieldExist(jsonError, JSON_ErrorMessage))
        return ERROR;

    WSError::Id errorId = ErrorIdFromString(GetOptionalString(jsonError, JSON_ErrorId));
    Utf8String errorMessage = GetOptionalString(jsonError, JSON_ErrorMessage);
    Utf8String errorDescription = GetOptionalString(jsonError, JSON_ErrorDescription);

    SetStatusReceivedError(HttpError(ConnectionStatus::OK, status), errorId, errorMessage, errorDescription, nullptr); // TODO: pass json
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetChildNodeContents(BeXmlNodeP node, Utf8CP childNodePath, Utf8StringR contentsOut, bool isRequired)
    {
    BeXmlNodeP childNode = node->SelectSingleNode(childNodePath);
    if (nullptr == childNode && isRequired)
        {
        if (isRequired)
            return ERROR;

        contentsOut.clear();
        return SUCCESS;
        }

    childNode->GetContent(contentsOut);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseXmlError(Http::ResponseCR httpResponse)
    {
    Utf8String bodyStr = httpResponse.GetBody().AsString();

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, bodyStr.c_str(), bodyStr.size());
    if (BeXmlStatus::BEXML_Success != xmlStatus)
        return ERROR;

    xmlDom->RegisterNamespace(XML_NAMESPACE_PREFIX, XML_NAMESPACE);

    BeXmlNodeP rootNode = xmlDom->GetRootElement();
    if (nullptr == rootNode)
        return ERROR;

    if (nullptr == rootNode || Utf8String(XML_ROOTNODE_NAME) != rootNode->GetName())
        {
        if (Utf8String(XML_Azure_Error) == rootNode->GetName())
            return ParseXmlAzureError(httpResponse, *xmlDom);
        return ERROR;
        }

    Utf8String errorIdStr;
    Utf8String errorMessage;
    Utf8String errorDescription;

    if (SUCCESS != GetChildNodeContents(rootNode, XML_ErrorId, errorIdStr, false) ||
        SUCCESS != GetChildNodeContents(rootNode, XML_ErrorMessage, errorMessage, true) ||
        SUCCESS != GetChildNodeContents(rootNode, XML_ErrorDescription, errorDescription, false))
        {
        return ERROR;
        }

    WSError::Id errorId = ErrorIdFromString(errorIdStr);

    SetStatusReceivedError(HttpError(httpResponse), errorId, errorMessage, errorDescription, nullptr);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseXmlAzureError(Http::ResponseCR httpResponse, BeXmlDom& xmlDom)
    {
    BeXmlNodeP rootNode = xmlDom.GetRootElement();
    if (nullptr == rootNode)
        return ERROR;

    Utf8String errorCode;
    Utf8String errorMessage;

    if (SUCCESS != GetChildNodeContents(rootNode, XML_Azure_Code, errorCode, true) ||
        SUCCESS != GetChildNodeContents(rootNode, XML_Azure_Message, errorMessage, true))
        {
        return ERROR;
        }

    auto errorId = WSError::Id::Unknown;
    if (XML_Azure_BlobNotFound == errorCode)
        errorId = WSError::Id::FileNotFound;

    SetStatusReceivedError(HttpError(httpResponse), errorId, errorMessage, nullptr, nullptr);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSError::SetStatusServerNotSupported()
    {
    m_message = WSErrorLocalizedString(MESSAGE_ServerNotSupported);
    m_description.clear();
    m_status = Status::ServerNotSupported;
    m_id = WSError::Id::Unknown;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSError::SetStatusReceivedError(HttpErrorCR httpError, Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription, JsonValueCP errorData)
    {
    m_status = Status::ReceivedError;

    HttpStatusType httpStatusType = HttpStatusHelper::GetType(httpError.GetHttpStatus());

    // Set error id
    if (WSError::Id::Unknown != errorId)
        {
        m_id = errorId;
        }
    else if (HttpStatusType::ServerError == httpStatusType)
        {
        m_id = WSError::Id::ServerError;
        }
    else if (HttpStatus::BadRequest == httpError.GetHttpStatus())
        {
        m_id = WSError::Id::BadRequest;
        }
    else if (HttpStatus::Conflict == httpError.GetHttpStatus())
        {
        m_id = WSError::Id::Conflict;
        }
    else
        {
        m_id = WSError::Id::Unknown;
        }

    if (WSError::Id::BadRequest == m_id || WSError::Id::Conflict == m_id)
        {
        // Pass message & description to user
        m_message = errorMessage;
        m_description = errorDescription;
        }
    else
        {
        // Format description and pass custom message for user
        m_description = FormatDescription(errorMessage.c_str(), errorDescription.c_str());

        if (WSError::Id::ClassNotFound == m_id)
            {
            m_message = WSErrorLocalizedString(MESSAGE_ClassNotFound);
            }
        else if (WSError::Id::FileNotFound == m_id)
            {
            m_message = WSErrorLocalizedString(MESSAGE_FileNotFound);
            }
        else if (WSError::Id::InstanceNotFound == m_id)
            {
            m_message = WSErrorLocalizedString(MESSAGE_InstanceNotFound);
            }
        else
            {
            m_message = httpError.GetMessage();
            }
        }

    // Set custom properties
    if (errorData)
        m_data = std::make_shared<Json::Value>(*errorData);
    
    // Fallback to default messages if not enough information received
    if (m_message.empty())
        {
        m_message = WSErrorLocalizedString(MESSAGE_UnknownError);
        if (m_description.empty())
            {
            m_description = httpError.GetMessage();
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSError::DoesStringFieldExist(JsonValueCR json, Utf8CP name)
    {
    if (!json.isMember(name))
        return false;
    JsonValueCR member = json[name];
    return member.isString() || member.isNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSError::DoesStringFieldExist(RapidJsonValueCR json, Utf8CP name)
    {
    if (!json.HasMember(name))
        return false;
    RapidJsonValueCR member = json[name];
    return member.IsString() || member.IsNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP WSError::GetOptionalString(RapidJsonValueCR json, Utf8CP name)
    {
    if (!json.HasMember(name))
        return nullptr;
    RapidJsonValueCR member = json[name];
    if (member.IsNull())
        return nullptr;
    return member.GetString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
int WSError::GetOptionalInt(RapidJsonValueCR json, Utf8CP name)
    {
    if (!json.HasMember(name))
        return 0;
    RapidJsonValueCR member = json[name];
    if (member.IsNull())
        return 0;
    return member.GetInt();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSError::FormatDescription(Utf8StringCR errorMessage, Utf8StringCR errorDescription)
    {
    Utf8String description;

    if (!errorMessage.empty() && !errorDescription.empty())
        {
        description.Sprintf("%s\n%s", errorMessage.c_str(), errorDescription.c_str());
        }
    else if (!errorMessage.empty())
        {
        description = errorMessage;
        }
    else
        {
        description = errorDescription;
        }

    return description;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError WSError::CreateServerNotSupportedError()
    {
    WSError error;
    error.SetStatusServerNotSupported();
    return error;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSError WSError::CreateFunctionalityNotSupportedError()
    {
    WSError error;
    error.m_status = Status::ReceivedError;
    error.m_id = Id::NotSupported;
    error.m_message = WSErrorLocalizedString(MESSAGE_FunctionalityNotSupported);
    return error;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::Status WSError::GetStatus() const
    {
    return m_status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::Id WSError::GetId() const
    {
    return m_id;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Karolis.Dziedzelis   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR WSError::GetData() const
    {
    return m_data ? *m_data : Json::Value::GetNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSError::GetDisplayMessage() const
    {
    return m_message;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSError::GetDisplayDescription() const
    {
    return m_description;
    }
