/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV1.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeXml/BeXml.h>
#include "WebApi.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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

        Http::Request CreateGetRepositoriesRequest(const bvector<Utf8String>& types, const bvector<Utf8String>& providerIds) const;

        static BentleyStatus ParseRepository(JsonValueCR dataSourceJson, WSRepository& repositoryOut, Utf8StringCR serverUrl);
        static WSRepositoriesResult ResolveGetRepositoriesResponse(Http::Response& response, Utf8StringCR serverUrl);
        static WSCreateObjectResult ResolveCreateObjectResponse(Http::Response& response, ObjectIdCR newObjectId, ObjectIdCR relObjectId, ObjectIdCR parentObjectId);
        static WSUpdateObjectResult ResolveUpdateObjectResponse(Http::Response& response);
        static WSObjectsResult ResolveObjectsResponse(Http::Response& response, Utf8StringCR schemaName, Utf8StringCR objectClassName = "");
        static WSFileResult ResolveFileResponse(Http::Response& response, BeFileName filePath);

        static bool IsValidObjectsResponse(Http::ResponseCR response);
        static bool IsJsonResponse(Http::ResponseCR response);

        static bool IsObjectCreationJsonSupported(JsonValueCR objectCreationJson);
        static void GetParametersFromObjectCreationJson
            (
            JsonValueCR objectCreationJson,
            ObjectIdR newObjectId,
            Utf8StringR propertiesOut,
            ObjectIdR relObjectId,
            ObjectIdR parentObjectIdOut
            );

        AsyncTaskPtr<SchemaInfoResult> GetSchemaInfo(ICancellationTokenPtr ct) const;

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
            Http::Request::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr ct
            ) const;

        AsyncTaskPtr<WSFileResult> GetSchema
            (
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            Http::Request::ProgressCallbackCR downloadProgressCallback,
            ICancellationTokenPtr ct
            ) const;

        virtual Utf8String GetSchemaUrl() const;

        AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR propertiesQuery,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const;

    public:
        WebApiV1(std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info);
        virtual ~WebApiV1();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSRepositoryResult> SendGetRepositoryRequest(ICancellationTokenPtr ct = nullptr) const override;

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr ct
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            IWSRepositoryClient::RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
