/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSError.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <map>
#include <WebServices/Connect/ImsClient.h>
#include <DgnClientFx/Utils/Http/HttpStatusHelper.h>
#include <BeXml/BeXml.h>

#include "WSError.xliff.h"

#define JSON_ErrorId            "errorId"
#define JSON_ErrorMessage       "errorMessage"
#define JSON_ErrorDescription   "errorDescription"

#define XML_NAMESPACE           "http://schemas.datacontract.org/2004/07/Bentley.Mas.WebApi.Models"
#define XML_NAMESPACE_PREFIX    "error"
#define XML_ROOTNODE_NAME       "ModelError"
#define XML_ErrorId             "error:errorId"
#define XML_ErrorMessage        "error:errorMessage"
#define XML_ErrorDescription    "error:errorDescription"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::Id WSError::ErrorIdFromString(Utf8StringCR errorIdString)
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
        {
        return it->second;
        }

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
WSError::WSError(HttpResponseCR httpResponse) : WSError()
    {
    if (ConnectionStatus::OK == httpResponse.GetConnectionStatus() &&
        LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_INFO))
        {
        LOG.infov
            (
            "Received WSError: %d %s\n"
            "Server response: %s\n",
            httpResponse.GetHttpStatus(),
            httpResponse.GetEffectiveUrl().c_str(),
            httpResponse.GetBody().AsString().c_str()
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
        m_status = Status::ConnectionError;
        m_id = WSError::Id::Unknown;
        return;
        }

    if (SUCCESS == ParseBody(httpResponse))
        {
        return;
        }

    if (ImsClient::IsLoginRedirect(httpResponse) ||                 // Bentley CONNECT login redirect
        HttpStatus::Unauthorized == httpResponse.GetHttpStatus())   // Bentley CONNECT token could not be retrieved
        {
        m_message = HttpError::GetHttpDisplayMessage(HttpStatus::Unauthorized);
        m_data = nullptr;
        m_status = Status::ReceivedError;
        m_id = Id::LoginFailed;
        return;
        }

    SetStatusServerNotSupported();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WSError::WSError(HttpErrorCR httpError)
    {
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

    SetStatusReceivedError(httpError, Id::Unknown, nullptr, nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseBody(HttpResponseCR httpResponse)
    {
    Utf8String contentType = httpResponse.GetHeaders().GetContentType();
    if (contentType.find("application/json") != Utf8String::npos)
        {
        // JSON is main error format
        return ParseJsonError(httpResponse);
        }
    else if (contentType.find("application/xml") != Utf8String::npos)
        {
        // XML format can occur when requesting XML ECSchema from v1.x server.
        return ParseXmlError(httpResponse);
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseJsonError(HttpResponseCR httpResponse)
    {
    Json::Value jsonError = httpResponse.GetBody().AsJson();
    if (!IsValidErrorJson(jsonError))
        {
        return ERROR;
        }

    WSError::Id errorId = ErrorIdFromString(jsonError[JSON_ErrorId].asString());
    Utf8String errorMessage = jsonError[JSON_ErrorMessage].asString();
    Utf8String errorDescription = jsonError[JSON_ErrorDescription].asString();
    SetStatusReceivedError(HttpError(httpResponse), errorId, errorMessage, errorDescription, std::make_shared<const Json::Value>(jsonError));
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetChildNodeContents(BeXmlNodeP node, Utf8CP childNodePath, Utf8StringR contentsOut)
    {
    BeXmlNodeP childNode = node->SelectSingleNode(childNodePath);
    if (nullptr == childNode)
        {
        return ERROR;
        }
    childNode->GetContent(contentsOut);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSError::ParseXmlError(HttpResponseCR httpResponse)
    {
    Utf8String bodyStr = httpResponse.GetBody().AsString();

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, bodyStr.c_str(), bodyStr.size());
    if (BeXmlStatus::BEXML_Success != xmlStatus)
        {
        return ERROR;
        }
    xmlDom->RegisterNamespace(XML_NAMESPACE_PREFIX, XML_NAMESPACE);

    BeXmlNodeP rootNode = xmlDom->GetRootElement();
    if (nullptr == rootNode || Utf8String(XML_ROOTNODE_NAME) != rootNode->GetName())
        {
        return ERROR;
        }

    Utf8String errorIdStr;
    Utf8String errorMessage;
    Utf8String errorDescription;

    if (SUCCESS != GetChildNodeContents(rootNode, XML_ErrorId, errorIdStr) ||
        SUCCESS != GetChildNodeContents(rootNode, XML_ErrorMessage, errorMessage) ||
        SUCCESS != GetChildNodeContents(rootNode, XML_ErrorDescription, errorDescription))
        {
        return ERROR;
        }

    WSError::Id errorId = ErrorIdFromString(errorIdStr);

    SetStatusReceivedError(HttpError(httpResponse), errorId, errorMessage, errorDescription);
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
void WSError::SetStatusReceivedError(HttpErrorCR httpError, Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription, JsonValueCPtr errorData)
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

    // Set message 
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

    // Set description
    m_description = FormatDescription(errorMessage.c_str(), errorDescription.c_str());

    // Set custom properties
    m_data = errorData;

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
* @bsimethod                                                    Vincas.Razma    09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsMemberStringOrNull(JsonValueCR json, Utf8CP name)
    {
    if (!json.isMember(name))
        {
        return false;
        }
    JsonValueCR member = json[name];
    return member.isString() || member.isNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSError::IsValidErrorJson(JsonValueCR jsonError)
    {
    return
        IsMemberStringOrNull(jsonError, JSON_ErrorId) &&
        IsMemberStringOrNull(jsonError, JSON_ErrorMessage) &&
        IsMemberStringOrNull(jsonError, JSON_ErrorDescription);
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
    return m_data ? *m_data : Json::Value::null;
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
