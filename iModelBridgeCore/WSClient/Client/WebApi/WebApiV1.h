/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV1.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeXml/BeXml.h>
#include "WebApi.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApiV1 : public WebApi
    {
    protected:
        struct SchemaInfo
            {
            Utf8String CreateDummyRemoteId() const;
            static bool IsDummySchemaId(ObjectIdCR objectId);

            Utf8String name;
            BeVersion version;
            };

        struct SchemaResponse
            {
            Utf8String eTag;
            bool isModified;
            };

        typedef AsyncResult<SchemaResponse, WSError> SchemaResult;
        typedef AsyncResult<SchemaInfo, WSError> SchemaInfoResult;

    public:
        static Utf8CP const SERVICE_Navigation;
        static Utf8CP const SERVICE_Objects;
        static Utf8CP const SERVICE_Files;
        static Utf8CP const SERVICE_Schema;

    protected:
        WSInfo m_info;
        mutable BeMutex m_schemaInfoCS;
        mutable SchemaInfo m_schemaInfo;

    protected:
        Utf8String GetUrl(Utf8StringCR service, Utf8StringCR params, Utf8StringCR queryString = nullptr, Utf8StringCR webApiVersion = "v1.1") const;
        Utf8String CreateObjectIdParam(ObjectIdCR objectId) const;
        Utf8String CreatePropertiesQuery(const bset<Utf8String>& properties) const;
        Utf8String CreatePropertiesQuery(Utf8StringCR propertiesList) const;
        Utf8String CreateParentQuery(ObjectIdCR objectId) const;
        Utf8String CreateWebApiVersionPart(Utf8StringCR webApiVersion) const;
        Utf8String GetMaxWebApi() const;

        HttpRequest CreateGetRepositoriesRequest(const bvector<Utf8String>& types, const bvector<Utf8String>& providerIds) const;

        static BentleyStatus ParseRepository(JsonValueCR dataSourceJson, WSRepository& repositoryOut);
        static WSRepositoriesResult ResolveGetRepositoriesResponse(HttpResponse& response);
        static WSCreateObjectResult ResolveCreateObjectResponse(HttpResponse& response, Utf8StringCR schemaName, Utf8StringCR className);
        static WSUpdateObjectResult ResolveUpdateObjectResponse(HttpResponse& response);
        static WSObjectsResult ResolveObjectsResponse(HttpResponse& response, Utf8StringCR schemaName, Utf8StringCR objectClassName = "");
        static WSFileResult ResolveFileResponse(HttpResponse& response, BeFileName filePath);

        static bool IsValidObjectsResponse(HttpResponseCR response);
        static bool IsJsonResponse(HttpResponseCR response);

        static bool IsObjectCreationJsonSupported(JsonValueCR objectCreationJson);
        static void GetParametersFromObjectCreationJson
            (
            JsonValueCR objectCreationJson,
            Utf8StringR schemaNameOut,
            Utf8StringR classNameOut,
            Utf8StringR propertiesOut,
            ObjectIdR parentObjectIdOut
            );

        AsyncTaskPtr<SchemaInfoResult> GetSchemaInfo(ICancellationTokenPtr cancellationToken) const;

        SchemaInfo GetCachedSchemaInfo() const;
        void SetCachedSchemaInfo(SchemaInfo schemaInfo) const;

        static BentleyStatus ReadSchemaInfoFromFile(BeFileNameCR filePath, SchemaInfo& schemaInfoOut);
        static BentleyStatus ReadSchemaInfoFromXmlString(Utf8StringCR xmlString, SchemaInfo& schemaInfoOut);
        static BentleyStatus ReadSchemaInfoFromXmlDom(BeXmlStatus xmlDomReadStatus, BeXmlDomPtr schemaXmlDom, SchemaInfo& schemaInfoOut);
        static BentleyStatus WriteFileToHttpBody(BeFileNameCR filePath, HttpBodyPtr body);

        AsyncTaskPtr<SchemaResult> GetSchema
            (
            HttpBodyPtr body,
            Utf8StringCR eTag,
            HttpRequest::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr cancellationToken
            ) const;

        AsyncTaskPtr<WSFileResult> GetSchema
            (
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            HttpRequest::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr cancellationToken
            ) const;

        virtual Utf8String GetSchemaUrl() const;

        AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR propertiesQuery,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const;

    public:
        WebApiV1(std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info);
        virtual ~WebApiV1();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr cancellationToken
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
