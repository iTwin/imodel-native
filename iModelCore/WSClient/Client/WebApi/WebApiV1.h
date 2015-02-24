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

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApiV1 : public WebApi
    {
    protected:
        struct SchemaInfo
            {
            Utf8String CreateDummyRemoteId () const;
            static bool IsDummySchemaId (ObjectIdCR objectId);

            Utf8String name;
            BeVersion version;
            };

        struct SchemaResponse
            {
            Utf8String eTag;
            bool isModified;
            };

        typedef MobileDgn::Utils::AsyncResult<SchemaResponse, WSError> SchemaResult;
        typedef MobileDgn::Utils::AsyncResult<SchemaInfo, WSError> SchemaInfoResult;

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
        Utf8String GetUrl (Utf8StringCR service, Utf8StringCR params, Utf8StringCR queryString = nullptr, Utf8StringCR webApiVersion = "v1.1") const;
        Utf8String CreateObjectIdParam (ObjectIdCR objectId) const;
        Utf8String CreatePropertiesQuery (const bset<Utf8String>& properties) const;
        Utf8String CreateParentQuery (ObjectIdCR objectId) const;
        Utf8String CreateWebApiVersionPart (Utf8StringCR webApiVersion) const;

        MobileDgn::Utils::HttpRequest CreateGetRepositoriesRequest (const bvector<Utf8String>& types, const bvector<Utf8String>& providerIds) const;

        static BentleyStatus ParseRepository (JsonValueCR dataSourceJson, WSRepository& repositoryOut);
        static WSRepositoriesResult ResolveGetRepositoriesResponse (MobileDgn::Utils::HttpResponse& response);
        static WSCreateObjectResult ResolveCreateObjectResponse (MobileDgn::Utils::HttpResponse& response, Utf8StringCR schemaName, Utf8StringCR className);
        static WSUpdateObjectResult ResolveUpdateObjectResponse (MobileDgn::Utils::HttpResponse& response);
        static WSObjectsResult ResolveObjectsResponse (MobileDgn::Utils::HttpResponse& response, Utf8StringCR schemaName, Utf8StringCR objectClassName = "");
        static WSFileResult ResolveFileResponse (MobileDgn::Utils::HttpResponse& response, BeFileName filePath);

        static bool IsValidObjectsResponse (MobileDgn::Utils::HttpResponseCR response);
        static bool IsJsonResponse (MobileDgn::Utils::HttpResponseCR response);

        static bool IsObjectCreationJsonSupported (JsonValueCR objectCreationJson);
        static void GetParametersFromObjectCreationJson
            (
            JsonValueCR objectCreationJson,
            Utf8StringR schemaNameOut,
            Utf8StringR classNameOut,
            Utf8StringR propertiesOut,
            ObjectIdR parentObjectIdOut
            );

        MobileDgn::Utils::AsyncTaskPtr<SchemaInfoResult> GetSchemaInfo (MobileDgn::Utils::ICancellationTokenPtr cancellationToken) const;

        SchemaInfo GetCachedSchemaInfo () const;
        void SetCachedSchemaInfo (SchemaInfo schemaInfo) const;

        static BentleyStatus ReadSchemaInfoFromFile (BeFileNameCR filePath, SchemaInfo& schemaInfoOut);
        static BentleyStatus ReadSchemaInfoFromXmlString (Utf8StringCR xmlString, SchemaInfo& schemaInfoOut);
        static BentleyStatus ReadSchemaInfoFromXmlDom (BeXmlStatus xmlDomReadStatus, BeXmlDomPtr schemaXmlDom, SchemaInfo& schemaInfoOut);
        static BentleyStatus WriteFileToHttpBody (BeFileNameCR filePath, MobileDgn::Utils::HttpBodyPtr body);

        MobileDgn::Utils::AsyncTaskPtr<SchemaResult> GetSchema
            (
            MobileDgn::Utils::HttpBodyPtr body,
            Utf8StringCR eTag,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken 
            ) const;

        MobileDgn::Utils::AsyncTaskPtr<WSFileResult> GetSchema
            (
            BeFileNameCR filePath,
            Utf8StringCR eTag,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken
            ) const;

        virtual Utf8String GetSchemaUrl () const;

    public:
        WebApiV1 (std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info);
        virtual ~WebApiV1 ();

        static bool IsSupported (WSInfoCR info);

        virtual MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
