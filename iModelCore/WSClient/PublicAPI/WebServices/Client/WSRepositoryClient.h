/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSRepositoryClient.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

#include <DgnClientFx/Utils/Threading/AsyncResult.h>
#include <DgnClientFx/Utils/Threading/LimitingTaskQueue.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSCreateObjectResponse.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSQuery.h>

#include <Bentley/bset.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to BWSG server data source.
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct IWSRepositoryClient>     IWSRepositoryClientPtr;

typedef AsyncResult<WSObjectsResponse, WSError>         WSObjectsResult;
typedef AsyncResult<WSFileResponse, WSError>            WSFileResult;
typedef AsyncResult<WSCreateObjectResponse, WSError>    WSCreateObjectResult;
typedef AsyncResult<HttpBodyPtr, WSError>               WSChangesetResult;
typedef AsyncResult<void, WSError>                      WSUpdateObjectResult;
typedef AsyncResult<void, WSError>                      WSDeleteObjectResult;
typedef AsyncResult<void, WSError>                      WSUpdateFileResult;

#define WSQuery_CustomParameter_NavigationParentId      "navigationParentId"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient
    {
    public:
        WSCLIENT_EXPORT static const Utf8String InitialSkipToken;

    public:
        WSCLIENT_EXPORT virtual ~IWSRepositoryClient();

        virtual IWSClientPtr GetWSClient() const = 0;
        virtual Utf8StringCR GetRepositoryId() const = 0;

        virtual void SetCredentials(Credentials credentials) = 0;

        //! Checks if supplied credentials are valid for this repository.
        //! @param[in] ct
        //! @return success if credentials are valid for given repository, else error that occurred
        virtual std::shared_ptr<PackagedAsyncTask<AsyncResult<void, WSError>>> VerifyAccess
            (
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Send Navigation request. SendQueryRequest has backward support for WSG 1.x Navigation queries
        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Send query request to retrieve ECInstances.
        //! WSG 1.x Navigation support: add WSQuery_CustomParameter_NavigationParentId to query custom parameters.
        //!         Parent id and query class will be used for Navigation query and empty string will result in Navigation root query.
        //!         Select will be used as property list to select
        //! @param query
        //! @param eTag send previous eTag to check if data was modified and avoid sending if not.
        //! @param skipToken allows using paged mechanism if supported by server and repository. Used to "skip" previous response data.
        //! Supply IWSRepositoryClient::InitialSkipToken to start paged requests. 
        //! For sequental page requests supply previous response skipToken if response was not final.
        //! @param ct
        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Send changeset for multiple created/modified/deleted instances at once.
        //! Supported from WSG 2.1, usage with older server versions will return "not supported" error.
        //! @param changeset JSON serialized to string. IIS defaults request size to 4MB (configurable) so string should accomodate to that
        //! @param uploadProgressCallback upload callback for changeset
        //! @param ct 
        //! @return server response that includes changed instances JSON
        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Create object with any relationships or related objects. Optionally attach file.
        //! Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. When root instanceId is specified, POST will be done to that instance.
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IWSSchemaProvider> IWSSchemaProviderPtr;
struct IWSSchemaProvider
    {
    public:
        virtual ~IWSSchemaProvider()
            {};
        virtual BeFileName GetSchema(WSInfoCR info) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSRepositoryClient : public IWSRepositoryClient
    {
    private:
        std::shared_ptr<struct ClientConnection> m_connection;
        IWSClientPtr m_serverClient;
        mutable LimitingTaskQueue<WSFileResult> m_fileDownloadQueue;

    private:
        WSRepositoryClient(std::shared_ptr<struct ClientConnection> connection);

    public:
        struct Timeout
            {
            struct Connection
                {
                static const uint32_t Default;
                };
            struct Transfer
                {
                static const uint32_t GetObject;
                static const uint32_t GetObjects;
                static const uint32_t FileDownload;
                static const uint32_t Upload;
                };
            };

        //! @param[in] serverUrl - address to supported server/site
        //! @param[in] repositoryId - repository identifier
        //! @param[in] clientInfo - client infomation for licensing and other information
        //! @param[in] customHandler - custom http handler for testing purposes.
        //! @param[in] schemaProvider - schema provider for WSG 1.x servers in order to override their schemas
        //! Note: schema is required for server with WebApi v1.1 as it does not provide one
        WSCLIENT_EXPORT static std::shared_ptr<WSRepositoryClient> Create
            (
            Utf8StringCR serverUrl,
            Utf8StringCR repositoryId,
            ClientInfoPtr clientInfo,
            IWSSchemaProviderPtr schemaProvider = nullptr,
            IHttpHandlerPtr customHandler = nullptr
            );

        //! Set limit for paralel file downloads. Default is 0 - no limit. Useful for older servers that could not cope with multiple
        //! file downloads at once.
        WSCLIENT_EXPORT void SetFileDownloadLimit(size_t limit);

        WSCLIENT_EXPORT IWSClientPtr GetWSClient() const override;
        WSCLIENT_EXPORT Utf8StringCR GetRepositoryId() const override;

        WSCLIENT_EXPORT void SetCredentials(Credentials credentials);

        //! Check if user can access repository
        WSCLIENT_EXPORT std::shared_ptr<PackagedAsyncTask<AsyncResult<void, WSError>>> VerifyAccess
            (
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8String eTag = nullptr,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
