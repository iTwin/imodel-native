/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSUploadResponse.h>
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

typedef AsyncResult<WSResponse, WSError>                WSResult;
typedef AsyncResult<WSObjectsResponse, WSError>         WSObjectsResult;
typedef AsyncResult<WSFileResponse, WSError>            WSFileResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSCreateObjectResult;
typedef AsyncResult<HttpBodyPtr, WSError>               WSChangesetResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSUpdateObjectResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSUpdateFileResult;
typedef AsyncResult<void, WSError>                      WSDeleteObjectResult;
typedef AsyncResult<WSRepository, WSError>              WSRepositoryResult;

#define WSQuery_CustomParameter_NavigationParentId      "navigationParentId"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient
    {
    public:
        enum AuthenticationType
            {
            Basic = 0,
            Windows
            };

    public:
        struct IRepositoryInfoListener
            {
            virtual ~IRepositoryInfoListener() {};
            virtual void OnInfoReceived(WSRepositoryCR info) = 0;
            };

    struct RequestOptions;
    typedef std::shared_ptr<RequestOptions> RequestOptionsPtr;
    struct ActivityOptions;
    typedef std::shared_ptr<ActivityOptions> ActivityOptionsPtr;
    struct JobOptions;
    typedef std::shared_ptr<JobOptions> JobOptionsPtr;

    public:
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
                WSCLIENT_EXPORT static const uint32_t UploadProcessing;
                };
            };

    public:
        WSCLIENT_EXPORT virtual ~IWSRepositoryClient();

        virtual IWSClientPtr GetWSClient() const = 0;
        virtual Utf8StringCR GetRepositoryId() const = 0;

        virtual void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic) = 0;

        //! DEPRECATED- Use VerifyAccessWithOptions instead
        //! Checks if supplied credentials are valid for this repository.
        //! @param[in] ct
        //! @return success if credentials are valid for given repository, else error that occurred
        virtual AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const = 0;

        //! Checks if supplied credentials are valid for this repository.
        //! @param[in] options optional request options
        //! @param[in] ct
        //! @return success if credentials are valid for given repository, else error that occurred
        virtual AsyncTaskPtr<WSVoidResult> VerifyAccessWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const = 0;

        //! Register for ServerInfo received events
        virtual void RegisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) = 0;
        //! Unregister from ServerInfo received events
        virtual void UnregisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) = 0;

        //! DEPRECATED- Use GetInfoWithOptions instead
        //! Returns repository or queries server if needs updating
        virtual AsyncTaskPtr<WSRepositoryResult> GetInfo(ICancellationTokenPtr ct = nullptr) const = 0;

        //! Returns repository or queries server if needs updating
        virtual AsyncTaskPtr<WSRepositoryResult> GetInfoWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const = 0;

        //! DEPRECATED- Use SendGetObjectRequestWithOptions instead
        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendGetChildrenRequestWithOptions instead
        //! Send Navigation request. SendQueryRequest has backward support for WSG 1.x Navigation queries
        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Send Navigation request. SendQueryRequest has backward support for WSG 1.x Navigation queries
        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendGetChildrenRequestWithOptions instead
        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendGetFileRequestWithOptions instead
        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendGetFileRequestWithOptions instead
        virtual AsyncTaskPtr<WSResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            Http::HttpBodyPtr responseBodyOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSResult> SendGetFileRequestWithOptions
            (
            ObjectIdCR objectId,
            Http::HttpBodyPtr responseBodyOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendGetSchemasRequestWithOptions instead
        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequestWithOptions
            (
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendQueryRequestWithOptions instead
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
        //! If server needs an initial skip token to start the paged mechanism supply IWSRepositoryClient::InitialSkipToken
        //! For sequental page requests supply previous response skipToken if response was not final.
        //! @param ct
        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
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
        //! If server needs an initial skip token to start the paged mechanism supply IWSRepositoryClient::InitialSkipToken
        //! For sequental page requests supply previous response skipToken if response was not final.
        //! @param options optional request options
        //! @param ct
        virtual AsyncTaskPtr<WSObjectsResult> SendQueryRequestWithOptions
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendChangesetRequestWithOptions instead
        //! Send changeset for multiple created/modified/deleted instances at once.
        //! Supported from WSG 2.1, usage with older server versions will return "not supported" error.
        //! @param changeset JSON serialized to string. IIS defaults request size to 4MB (configurable) so string should accomodate to that
        //! @param uploadProgressCallback upload callback for changeset
        //! @param ct
        //! @return server response that includes changed instances JSON
        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Send changeset for multiple created/modified/deleted instances at once.
        //! Supported from WSG 2.1, usage with older server versions will return "not supported" error.
        //! @param changeset JSON serialized to string. IIS defaults request size to 4MB (configurable) so string should accomodate to that
        //! @param uploadProgressCallback upload callback for changeset
        //! @param options optional request options
        //! @param ct 
        //! @return server response that includes changed instances JSON
        virtual AsyncTaskPtr<WSChangesetResult> SendChangesetRequestWithOptions
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        //! Create object with any relationships or related objects. Optionally attach file.
        //! @param objectCreationJson must follow WSG 2.0 format for creating objects. URL will use "instanceId" field unless "changeState" : "new" is set.
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
        //! Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. When root instanceId is specified, POST will be done to that instance.
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        //! @return JSON representing created data and new file ETag if available
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead        //! Create an object with a relation. With optional file attachment
        //! Create object with any relationships or related objects. Optionally attach file.
        //! @param relatedObjectId valid object id that new object should be related to
        //! @param objectCreationJson must follow WSG 2.0 format for creating objects. URL will use "instanceId" field unless "changeState" : "new" is set. 
        //! @param filePath [optional] file path to upload
        //! @param uploadProgressCallback [optional] upload callback for changeset
        //! @param ct [optional] cancellation token
        //! @note
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. <b>When root instanceId is specified, POST will be done to that instance, if and only if, the objectId parameter has an empty remoteId.</b>
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        //! @return JSON representing created data and new file ETag if available
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Create an object with a relation. With optional file attachment
        //! @param relatedObjectId - relation target object (e. g. document for checkin)
        //! @param objectCreationJson - Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! @param filePath - file path to upload
        //! @param uploadProgressCallback - file upload progress
        //! @param options optional request options
        //! @param ct
        //! @return JSON representing created object and new file ETag if available
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendUpdateObjectRequestWithOptions instead
        //! Update specified object and optionally a file with one operation
        //! @param objectId object identifier
        //! @param propertiesJson object properties that need to be updated
        //! @param eTag [optional] DEPRECATED - only used for WebApi 1.x. Original instance eTag for server to do check if it did not change.
        //! @param filePath [optional]  file path to upload. Only supported from WebApi 2.4
        //! @param uploadProgressCallback [optional] file upload progress
        //! @param ct [optional] 
        //! @return empty JSON and new file ETag if available
        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Update specified object and optionally a file with one operation
        //! @param objectId - object identifier
        //! @param propertiesJson - object properties that need to be updated
        //! @param eTag - DEPRECATED - only used for WebApi 1.x. Original instance eTag for server to do check if it did not change.
        //! @param filePath - file path to upload. Only supported from WebApi 2.4
        //! @param uploadProgressCallback - file upload progress
        //! @param options optional request options
        //! @param ct
        //! @return empty JSON and new file ETag if available
        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendDeleteObjectRequestWithOptions instead
        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendUpdateFileRequestWithOptions instead
        //! Update file on server for given object.
        //! @param objectId - object identifier to update file for
        //! @param filePath - path to file to use for update
        //! @param uploadProgressCallback - file upload progress
        //! @param ct
        //! @return empty JSON and and new file ETag if available
        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! Update file on server for given object.
        //! @param objectId - object identifier to update file for
        //! @param filePath - path to file to use for update
        //! @param uploadProgressCallback - file upload progress
        //! @param options optional request options
        //! @param ct
        //! @return empty JSON and and new file ETag if available
        virtual AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequestWithOptions
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
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
        virtual ~IWSSchemaProvider() {};
        virtual BeFileName GetSchema(WSInfoCR info) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     julius.cepukenas    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
//! Request options that can be passed to individual requests to define their properties
struct IWSRepositoryClient::RequestOptions
    {
    private:
        uint64_t m_transferTimeOut = IWSRepositoryClient::Timeout::Transfer::Default;
        ActivityOptionsPtr m_activityOptions = std::make_shared<ActivityOptions>();
        JobOptionsPtr m_jobOptions = std::make_shared<JobOptions>();

    public:
        RequestOptions() {};
        virtual ~RequestOptions() {};

        void SetTransferTimeOut(uint64_t timeOut) {m_transferTimeOut = timeOut;}
        uint64_t GetTransferTimeOut() const {return m_transferTimeOut;}

        //! Retrieve options for activity
        ActivityOptionsPtr GetActivityOptions() { return m_activityOptions; }

        //! Retrieve options required for WSG asynchronous job operations
        //! Jobs API can be enabled through these options
        JobOptionsPtr GetJobOptions() { return m_jobOptions; }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mantas.Smicius    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient::ActivityOptions
    {
    public:
        enum class HeaderName
            {
            MasRequestId,
            XCorrelationId,
            Default = MasRequestId
            };

    private:
        HeaderName m_headerName = HeaderName::Default;
        Utf8String m_activityId;

    public:
        //! Set header name for Activity id in each Http request to WSG
        void SetHeaderName(HeaderName headerName) { m_headerName = headerName; }

        //! Get header name for Activity id in each Http request to WSG
        HeaderName GetHeaderName() const { return m_headerName; }

        //! Set custom activity id for all Http requests in current Api method.
        //! If activity id is not set then unique value will be generated.
        //! It is not recommended to use custom activity id value. Optional
        void SetActivityId(Utf8StringCR activityId) { m_activityId = activityId; }

        //! Get activity id for all Http requests in current Api method
        Utf8StringCR GetActivityId() const { return m_activityId; }

        //! Check if activity id is set
        bool HasActivityId() const { return !m_activityId.empty(); }

        //! Converts specified HeaderName enum value to String
        WSCLIENT_EXPORT static Utf8String HeaderNameToString(HeaderName headerName);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     julius.cepukenas    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
//! WSG Asynchronous Job related options
struct IWSRepositoryClient::JobOptions
    {
    private:
        bool        m_enableJobsIfPossible      = false;
        double      m_waitTimeIncreaseCoef      = 1.2;
        uint32_t    m_initialWaitTimeMs         = 100;
        uint32_t    m_maxWaitTimeMs             = 4000; // 4 Seconds
        uint32_t    m_maxJobDuratationTimeMs    = 3600000; //1 Hour

    public:
        JobOptions() {};
        virtual ~JobOptions() {};

        //! Enable/Disable request to be executed via Job Api if Possible
        void EnableJobsIfPossible() { m_enableJobsIfPossible = true; }
        void DisableJobs() { m_enableJobsIfPossible = false; }
        bool IsJobsApiEnabled() const { return m_enableJobsIfPossible; }

        //! Total time of how long to wait for request to be executed via Job request
        void SetMaxTotalWaitTimeMs(uint32_t maxDuration) { m_maxJobDuratationTimeMs = maxDuration; };
        uint32_t GetMaxTotalWaitTimeMs() { return m_maxJobDuratationTimeMs; };

        //! The Jobs request is checked in intervals
        //! Time of first interval
        void SetInitialWaitIntervalMs(uint32_t initialWait) { m_initialWaitTimeMs = initialWait; };
        uint32_t GetInitialWaitIntervalMs() { return m_initialWaitTimeMs; };

        //! The Jobs request is checked in intervals
        //! Maximum time of wait interval
        void SetMaxWaitIntervalTimeMs(uint32_t maxWait) { m_maxWaitTimeMs = maxWait; };
        uint32_t GetMaxWaitIntervalTimeMs() { return m_maxWaitTimeMs; };

        //! The Jobs request is checked in intervals
        //! Coeficiant of wait interval increase
        void SetWaitIntervalTimeIncreaseCoef(double increaseCof) { m_waitTimeIncreaseCoef = increaseCof; };
        double GetWaitIntervalTimeIncreaseCoef() { return m_waitTimeIncreaseCoef; };
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
        mutable LimitingTaskQueue<WSResult> m_fileDownloadQueue;
        std::shared_ptr<struct Configuration> m_config;
        std::shared_ptr<struct RepositoryInfoProvider> m_infoProvider;

    private:
        WSRepositoryClient(std::shared_ptr<struct ClientConnection> connection);
        
        // Taken from WebServices team .NET implementation
        static Utf8String UrlDecode(Utf8String url);
        static Utf8String ParsePluginIdFromRepositoryId(Utf8StringCR repositoryId);

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

        //! @param[in] serverUrl - address to supported server/site
        //! @param[in] serviceVersion - service version to use. Non-empty value forces /svX.Y/ URLs.
        //! @param[in] repositoryId - repository identifier
        //! @param[in] clientInfo - client infomation for licensing and other information
        //! @param[in] customHandler - custom http handler for testing purposes.
        //! @param[in] schemaProvider - schema provider for WSG 1.x servers in order to override their schemas
        //! Note: schema is required for server with WebApi v1.1 as it does not provide one
        WSCLIENT_EXPORT static std::shared_ptr<WSRepositoryClient> Create
            (
            Utf8StringCR serverUrl,
            BeVersionCR serviceVersion,
            Utf8StringCR repositoryId,
            ClientInfoPtr clientInfo,
            IWSSchemaProviderPtr schemaProvider = nullptr,
            IHttpHandlerPtr customHandler = nullptr
            );

        //! Parses repository URL to WSRepository. Includes server URL, repository ID, location and plugin ID.
        //! @param[in] url - URL to repository
        //! @param[out] remainingPathOut - remaining URL path and/or query after repository identifier
        //! @return parsed WSRepository or invalid if there was an error
        WSCLIENT_EXPORT static WSRepository ParseRepositoryUrl(Utf8StringCR url, Utf8StringP remainingPathOut = nullptr);

        //! Set limit for paralel file downloads. Default is 0 - no limit. Useful for older servers that could not cope with multiple
        //! file downloads at once.
        WSCLIENT_EXPORT void SetFileDownloadLimit(size_t limit);

        WSCLIENT_EXPORT IWSClientPtr GetWSClient() const override;
        WSCLIENT_EXPORT Utf8StringCR GetRepositoryId() const override;

        WSCLIENT_EXPORT void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic) override;
        WSCLIENT_EXPORT Configuration& Config();

        //! DEPRECATED- Use VerifyAccessWithOptions instead
        //! Check if user can access repository
        WSCLIENT_EXPORT AsyncTaskPtr<WSVoidResult> VerifyAccess(ICancellationTokenPtr ct = nullptr) const override;

        //! Check if user can access repository
        WSCLIENT_EXPORT AsyncTaskPtr<WSVoidResult> VerifyAccessWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const override;

        WSCLIENT_EXPORT void RegisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) override;
        WSCLIENT_EXPORT void UnregisterRepositoryInfoListener(std::weak_ptr<IRepositoryInfoListener> listener) override;

        //! DEPRECATED- Use GetInfoWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSRepositoryResult> GetInfo(ICancellationTokenPtr ct = nullptr) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSRepositoryResult> GetInfoWithOptions(RequestOptionsPtr options = nullptr, ICancellationTokenPtr ct = nullptr) const override;

        //! DEPRECATED- Use SendGetObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetObjectRequest
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendGetChildrenRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendGetChildrenRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequest
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetChildrenRequestWithOptions
            (
            ObjectIdCR parentObjectId,
            const bset<Utf8String>& propertiesToSelect,
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendGetFileRequest with http response body instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendGetFileRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            Http::HttpBodyPtr responseBodyOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSResult> SendGetFileRequestWithOptions
            (
            ObjectIdCR objectId,
            Http::HttpBodyPtr responseBodyOut,
            Utf8StringCR eTag = nullptr,
            Http::Request::ProgressCallbackCR downloadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendGetSchemasRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequest
            (
            Utf8StringCR eTag = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendGetSchemasRequestWithOptions
            (
            Utf8StringCR eTag = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendQueryRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendQueryRequest
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSObjectsResult> SendQueryRequestWithOptions
            (
            WSQueryCR query,
            Utf8StringCR eTag = nullptr,
            Utf8StringCR skipToken = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendChangesetRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSChangesetResult> SendChangesetRequestWithOptions
            (
            HttpBodyPtr changeset,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendUpdateObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendDeleteObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequest
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSDeleteObjectResult> SendDeleteObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendUpdateFileRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequestWithOptions
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
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

        WSCLIENT_EXPORT void SetPersistenceProviderId(Utf8StringCR id);
        WSCLIENT_EXPORT Utf8StringCR GetPersistenceProviderId() const;

        // TODO: Move WSRepositoryClient::SetCredentials WSRepositoryClient::SetFileDownloadLimit methods
        // to configuration struct
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
