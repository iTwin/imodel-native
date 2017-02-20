/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSRepositoryClient.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSCreateObjectResponse.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSQuery.h>
#include <BeHttp/CompressionOptions.h>

#include <Bentley/bset.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
typedef AsyncResult<void, WSError>                      WSVoidResult;

#define WSQuery_CustomParameter_NavigationParentId      "navigationParentId"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient
    {
    struct RequestOptions;
    typedef std::shared_ptr<RequestOptions> RequestOptionsPtr;

    public:
        // WIP: SkipTokens disabled due to issues.
        WSCLIENT_EXPORT static const Utf8String InitialSkipToken;

    public:
        struct Timeout
            {
            struct Connection
                {
                WSCLIENT_EXPORT static const uint32_t Default;
                };
            struct Transfer
                {
                WSCLIENT_EXPORT static const uint32_t GetObject;
                WSCLIENT_EXPORT static const uint32_t GetObjects;
                WSCLIENT_EXPORT static const uint32_t FileDownload;
                WSCLIENT_EXPORT static const uint32_t Upload;
                WSCLIENT_EXPORT static const uint32_t Default;
                WSCLIENT_EXPORT static const uint32_t LongUpload;
                };
            };

        WSCLIENT_EXPORT virtual ~IWSRepositoryClient();

        virtual IWSClientPtr GetWSClient() const = 0;
        virtual Utf8StringCR GetRepositoryId() const = 0;

        virtual void SetCredentials(Credentials credentials) = 0;

        //! Checks if supplied credentials are valid for this repository.
        //! @param[in] ct
        //! @return success if credentials are valid for given repository, else error that occurred
        virtual AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const = 0;

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
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
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
        //! The query is sent as a GET request with query information formated to the url.
        //! From WSG 2.4 information stored in the WSQuery can be send as a POST request via json content.
        //!          This is done if the URL constructed from query information exceeds the maximum Url Lenght
        //!          By default the maximum Url Lenght is 2048
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
        //! @param options optional request options
        //! @return server response that includes changed instances JSON
        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr,
            RequestOptionsPtr options = nullptr
            ) const = 0;

        //! Create object with any relationships or related objects. Optionally attach file.
        //! @param objectCreationJson must follow WSG 2.0 format for creating objects.
        //! @param filePath [optional] file
        //! @param uploadProgressCallback [optional] upload callback for changeset
        //! @param ct [optional] cancellation token
        //! @note
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. <b>When root instanceId is specified, POST will be done to that instance.</b>
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Create object with any relationships or related objects. Optionally attach file.
        //! @param objectId Used to construct URL for POST request. Any schema, class or instanceId included in the URL will come from this object.
        //!            <br> remoteId is optional. If supplied this implies that there are instances related to that instance that needs creation.
        //! @param objectCreationJson must follow WSG 2.0 format for creating objects.
        //! @param filePath [optional] file
        //! @param uploadProgressCallback [optional] upload callback for changeset
        //! @param ct [optional] cancellation token
        //! @note
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. <b>When root instanceId is specified, POST will be done to that instance, if and only if, the objectId parameter has an empty remoteId.</b>
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Update specified object and optionally a file with one operation
        //! @param objectId object identifier
        //! @param propertiesJson object properties that need to be updated
        //! @param eTag [optional] DEPRECATED - only used for WebApi 1.x. Original instance eTag for server to do check if it did not change.
        //! @param filePath [optional]  file path to upload. Only supported from WebApi 2.4
        //! @param uploadProgressCallback [optional] file upload progress
        //! @param ct [optional] 
        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
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
* @bsiclass                                                     julius.cepukenas    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
//! Request options that can be passed to individual requests to define their properties
struct IWSRepositoryClient::RequestOptions
    {
    private:
        uint64_t m_transferTimeOut;
  
    public:
        WSCLIENT_EXPORT RequestOptions();
        virtual ~RequestOptions() {}

        void SetTransferTimeOut(uint64_t timeOut) {m_transferTimeOut = timeOut;}
        uint64_t GetTransferTimeOut() const {return m_transferTimeOut;}
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSRepositoryClient : public IWSRepositoryClient
    {
    struct Configuration;

    private:
        std::shared_ptr<struct ClientConnection> m_connection;
        IWSClientPtr m_serverClient;
        mutable LimitingTaskQueue<WSFileResult> m_fileDownloadQueue;
        std::shared_ptr<struct Configuration> m_config;

    private:
        WSRepositoryClient(std::shared_ptr<struct ClientConnection> connection);

    public:
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

        WSCLIENT_EXPORT void SetCredentials(Credentials credentials) override;
        WSCLIENT_EXPORT Configuration& Config();

        //! Check if user can access repository
        WSCLIENT_EXPORT AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const override;

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
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
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
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr,
            RequestOptionsPtr options = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName (),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     julius.cepukenas    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSRepositoryClient::Configuration
    {
    public:
        friend WSRepositoryClient;
    
    private:
        ClientConnection& m_connection;

    private:
        Configuration& operator= (const Configuration&) = delete;
        Configuration(ClientConnection& connection) : m_connection(connection) {};

    public:
        //! Set the options of whether requests sent or responses retrieved should be compressed.
        //! By default both, requests and responses are not compressed.
        //! @param options compression options 
        WSCLIENT_EXPORT void SetCompressionOptions(CompressionOptions options);

        //! Set the suggested upper-bounds of request url.
        //! Mostly these bounds are not reinforced by the client
        //! From WSG 2.4 some features uses max url lenght when performing operations
        //! By default the max url lenght is set to 2048
        //! @param lenght
        WSCLIENT_EXPORT void SetMaxUrlLength(size_t lenght);

        WSCLIENT_EXPORT CompressionOptionsCR GetCompressionOptions() const;
        WSCLIENT_EXPORT size_t GetMaxUrlLength() const;

        // TODO: Move WSRepositoryClient::SetCredentials WSRepositoryClient::SetFileDownloadLimit methods
        // to configuration struct
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
