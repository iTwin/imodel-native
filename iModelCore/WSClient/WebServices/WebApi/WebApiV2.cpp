/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/WebApi/WebApiV2.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesInternal.h"
#include "WebApiV2.h"
#include <WebServices/Response/WSObjectsReaderV2.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WebApiV2::WebApiV2 (std::shared_ptr<const ClientConfiguration> configuration) :
WebApi (configuration)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WebApiV2::~WebApiV2 ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetUrl (Utf8StringCR params, Utf8StringCR queryString, Utf8StringCR webApiVersion) const
    {
    Utf8PrintfString url
        (
        "%s/%s/Repositories/%s",
        m_configuration->GetServerUrl ().c_str (),
        webApiVersion.c_str (),
        HttpClient::EscapeString (m_configuration->GetRepositoryId ()).c_str ()
        );

    if (!params.empty ())
        {
        url += "/" + params;
        }

    if (!queryString.empty ())
        {
        url += "?" + queryString;
        }

    BeAssert (url.size () < 2000 && "<Warning> Url length might be problematic as it is longer than most default settings");
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateObjectSubPath (ObjectIdCR objectId) const
    {
    Utf8String param;
    if (!objectId.IsEmpty ())
        {
        param.Sprintf ("%s/%s/%s", objectId.schemaName.c_str (), objectId.className.c_str (), objectId.remoteId.c_str ());
        }
    return param;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateFileSubPath (ObjectIdCR objectId) const
    {
    return CreateObjectSubPath (objectId) + +"/$file";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateClassSubPath (Utf8StringCR schemaName, Utf8StringCR className) const
    {
    return schemaName + "/" + className;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateNavigationSubPath (ObjectIdCR parentId) const
    {
    if (parentId.IsEmpty ())
        {
        return "Navigation/NavNode";
        }
    return CreateObjectSubPath (parentId) + "/NavNode";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::CreateSelectPropertiesQuery (const bset<Utf8String>& properties) const
    {
    Utf8String value = StringUtils::Join (properties.begin (), properties.end (), ",");
    Utf8String query;
    if (!value.empty ())
        {
        query.Sprintf ("$select=%s", value.c_str ());
        }
    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String WebApiV2::GetNullableString (RapidJsonValueCR jsonValue)
    {
    if (jsonValue.IsString ())
        {
        return jsonValue.GetString ();
        }
    BeAssert (jsonValue.IsNull ());
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSRepositoriesResult WebApiV2::ResolveGetRepositoriesResponse (HttpResponse& response)
    {
    if (HttpStatus::OK != response.GetHttpStatus ())
        {
        return WSRepositoriesResult::Error (response);
        }

    auto repositoriesJson = std::make_shared<rapidjson::Document> ();
    response.GetBody ().AsRapidJson (*repositoriesJson);

    auto reader = WSObjectsReaderV2::Create ();
    WSObjectsReader::Instances instances = reader->ReadInstances (repositoriesJson);
    if (!instances.IsValid ())
        {
        return WSRepositoriesResult::Error (WSError::CreateServerNotSupportedError ());
        }

    bvector<WSRepository> repositories;
    for (WSObjectsReader::Instance instance : instances)
        {
        if (!instance.IsValid () ||
            !instance.GetObjectId ().schemaName.Equals ("Repositories") ||
            !instance.GetObjectId ().className.Equals ("RepositoryIdentifier"))
            {
            return WSRepositoriesResult::Error (WSError::CreateServerNotSupportedError ());
            }

        WSRepository repository;

        repository.SetId (instance.GetObjectId ().remoteId);
        repository.SetLocation (GetNullableString (instance.GetProperties ()["Location"]));
        repository.SetPluginId (GetNullableString (instance.GetProperties ()["ECPluginID"]));
        repository.SetLabel (GetNullableString (instance.GetProperties ()["DisplayLabel"]));
        repository.SetDescription (GetNullableString (instance.GetProperties ()["Description"]));

        repositories.push_back (repository);
        }

    return WSRepositoriesResult::Success (repositories);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSCreateObjectResult WebApiV2::ResolveCreateObjectResponse (HttpResponse& response)
    {
    if (HttpStatus::Created == response.GetHttpStatus ())
        {
        return WSCreateObjectResult::Success (response.GetBody ().AsJson ());
        }
    return WSCreateObjectResult::Error (response);
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSUpdateObjectResult WebApiV2::ResolveUpdateObjectResponse (HttpResponse& response)
    {
    if (HttpStatus::OK == response.GetHttpStatus ())
        {
        return WSUpdateObjectResult::Success ();
        }
    return WSUpdateObjectResult::Error (response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSObjectsResult WebApiV2::ResolveObjectsResponse (HttpResponse& response, const ObjectId* objectId)
    {
    HttpStatus status = response.GetHttpStatus ();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        auto reader = WSObjectsReaderV2::Create ();

        auto body = response.GetContent ()->GetBody ();
        auto eTag = response.GetHeaders ().GetETag ();

        return WSObjectsResult::Success (WSObjectsResponse (reader, body, status, eTag));
        }
    return WSObjectsResult::Error (response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
WSFileResult WebApiV2::ResolveFileResponse (HttpResponse& response, BeFileName filePath)
    {
    HttpStatus status = response.GetHttpStatus ();
    if (HttpStatus::OK == status ||
        HttpStatus::NotModified == status)
        {
        return WSFileResult::Success (WSFileResponse (filePath, status, response.GetHeaders ().GetETag ()));
        }
    return WSFileResult::Error (response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSRepositoriesResult> WebApiV2::SendGetRepositoriesRequest
(
const bvector<Utf8String>& types,
const bvector<Utf8String>& providerIds,
ICancellationTokenPtr cancellationToken
) const
    {
    // TODO: implement filtering query by PluginId if needed
    Utf8PrintfString url ("%s/v2.0/Repositories/", m_configuration->GetServerUrl ().c_str ());
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetJsonRequest (url);

    request.SetCancellationToken (cancellationToken);
    return request.PerformAsync ()->Then<WSRepositoriesResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveGetRepositoriesResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetObjectRequest
(
ObjectIdCR objectId,
Utf8StringCR eTag,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateObjectSubPath (objectId));
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetJsonRequest (url, eTag);

    request.SetRetryOptions (HttpRequest::ResetTransfer, 1);
    request.SetConnectionTimeoutSeconds (WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds (WSRepositoryClient::Timeout::Transfer::GetObject);
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSObjectsResult> ([objectId] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse (httpResponse, &objectId);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetChildrenRequest
(
ObjectIdCR parentObjectId,
const bset<Utf8String>& propertiesToSelect,
Utf8StringCR eTag,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateNavigationSubPath (parentObjectId), CreateSelectPropertiesQuery (propertiesToSelect));
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetJsonRequest (url, eTag);

    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSObjectsResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSFileResult> WebApiV2::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateFileSubPath (objectId));
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetRequest (url, eTag);

    request.SetResponseBody (HttpFileBody::Create (filePath));
    request.SetRetryOptions (HttpRequest::ResumeTransfer, 0);
    request.SetConnectionTimeoutSeconds (WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds (WSRepositoryClient::Timeout::Transfer::FileDownload);
    request.SetDownloadProgressCallback (downloadProgressCallback);
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSFileResult> ([filePath] (HttpResponse& httpResponse)
        {
        return  ResolveFileResponse (httpResponse, filePath);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendGetSchemasRequest
(
Utf8StringCR eTag,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateClassSubPath ("MetaSchema", "ECSchemaDef"));
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetJsonRequest (url);

    request.GetHeaders ().SetIfNoneMatch (eTag);
    request.SetConnectionTimeoutSeconds (WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds (WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSObjectsResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSObjectsResult> WebApiV2::SendQueryRequest
(
WSQueryCR query,
Utf8StringCR eTag,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String classes = StringUtils::Join (query.GetClasses ().begin (), query.GetClasses ().end (), ",");
    Utf8String url = GetUrl (CreateClassSubPath (query.GetSchemaName (), classes), query.ToString ());
    HttpRequest request = m_configuration->GetHttpClient ().CreateGetJsonRequest (url);

    request.GetHeaders ().SetIfNoneMatch (eTag);
    request.SetConnectionTimeoutSeconds (WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds (WSRepositoryClient::Timeout::Transfer::GetObjects);
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSObjectsResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveObjectsResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSCreateObjectResult> WebApiV2::SendCreateObjectRequest
(
JsonValueCR objectCreationJson,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String schemaName = objectCreationJson["instance"]["schemaName"].asString ();
    Utf8String className = objectCreationJson["instance"]["className"].asString ();

    Utf8String url = GetUrl (CreateClassSubPath (schemaName, className));
    ChunkedUploadRequest request ("POST", url, m_configuration->GetHttpClient ());

    request.SetHandshakeRequestBody (HttpStringBody::Create (Json::FastWriter ().write (objectCreationJson)), "application/json");
    if (!filePath.empty ())
        {
        request.SetRequestBody (HttpFileBody::Create (filePath), Utf8String (filePath.GetFileNameAndExtension ()));
        }
    request.SetCancellationToken (cancellationToken);
    request.SetUploadProgressCallback (uploadProgressCallback);

    return request.PerformAsync ()->Then<WSCreateObjectResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveCreateObjectResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateObjectResult> WebApiV2::SendUpdateObjectRequest
(
ObjectIdCR objectId,
JsonValueCR propertiesJson,
Utf8String eTag,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateObjectSubPath (objectId));
    HttpRequest request = m_configuration->GetHttpClient ().CreatePostRequest (url);

    request.GetHeaders ().SetContentType ("application/json");
    if (!eTag.empty ())
        {
        request.GetHeaders ().SetIfMatch (eTag);
        }

    Json::Value updateJson;
    JsonValueR instanceJson = updateJson["instance"];

    instanceJson["schemaName"] = objectId.schemaName;
    instanceJson["className"] = objectId.className;
    instanceJson["instanceId"] = objectId.remoteId;
    instanceJson["properties"] = propertiesJson;

    request.SetRequestBody (HttpStringBody::Create (Json::FastWriter ().write (updateJson)));
    request.SetCancellationToken (cancellationToken);
    request.SetUploadProgressCallback (uploadProgressCallback);

    return request.PerformAsync ()->Then<WSUpdateObjectResult> ([] (HttpResponse& httpResponse)
        {
        return ResolveUpdateObjectResponse (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSDeleteObjectResult> WebApiV2::SendDeleteObjectRequest
(
ObjectIdCR objectId,
ICancellationTokenPtr cancellationToken
) const
    {
    Utf8String url = GetUrl (CreateObjectSubPath (objectId));
    HttpRequest request = m_configuration->GetHttpClient ().CreateRequest (url, "DELETE");

    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()->Then<WSDeleteObjectResult> ([] (HttpResponse& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus ())
            {
            return WSUpdateObjectResult::Success ();
            }
        return WSUpdateObjectResult::Error (httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<WSUpdateFileResult> WebApiV2::SendUpdateFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr cancellationToken
) const
    {
    BeFile beFile;
    beFile.Open (filePath, BeFileAccess::Read);

    Utf8String url = GetUrl (CreateFileSubPath (objectId));
    ChunkedUploadRequest request ("PUT", url, m_configuration->GetHttpClient ());

    request.SetRequestBody (HttpFileBody::Create (filePath), Utf8String (filePath.GetFileNameAndExtension ()));
    request.SetCancellationToken (cancellationToken);
    request.SetUploadProgressCallback (uploadProgressCallback);

    return request.PerformAsync ()->Then<WSUpdateFileResult> ([] (HttpResponse& httpResponse)
        {
        if (HttpStatus::OK == httpResponse.GetHttpStatus ())
            {
            return WSUpdateFileResult::Success ();
            }
        return WSUpdateFileResult::Error (httpResponse);
        });
    }
