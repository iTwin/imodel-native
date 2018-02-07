/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSRepositoryClient.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Threading/LimitingTaskQueue.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSUploadResponse.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSQuery.h>

#include <Bentley/bset.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to BWSG server data source.
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct IWSRepositoryClient>     IWSRepositoryClientPtr;

typedef AsyncResult<WSObjectsResponse, WSError>         WSObjectsResult;
typedef AsyncResult<WSFileResponse, WSError>            WSFileResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSCreateObjectResult;
typedef AsyncResult<HttpBodyPtr, WSError>               WSChangesetResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSUpdateObjectResult;
typedef AsyncResult<WSUploadResponse, WSError>          WSUpdateFileResult;
typedef AsyncResult<void, WSError>                      WSDeleteObjectResult;
typedef AsyncResult<void, WSError>                      WSVoidResult;

#define WSQuery_CustomParameter_NavigationParentId      "navigationParentId"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSRepositoryClient
    {
    struct RequestOptions;
    typedef std::shared_ptr<RequestOptions> RequestOptionsPtr;
    struct JobOptions;
    typedef std::shared_ptr<JobOptions> JobOptionsPtr;

    public:
        enum AuthenticationType
            {
            Basic = 0,
            Windows
            };

    public:
        WSCLIENT_EXPORT static const Utf8String InitialSkipToken;

    public:
        WSCLIENT_EXPORT virtual ~IWSRepositoryClient();

        virtual IWSClientPtr GetWSClient() const = 0;
        virtual Utf8StringCR GetRepositoryId() const = 0;

        virtual void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic) = 0;

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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        //! Create object with any relationships or related objects. Optionally attach file.
        //! Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! NOTES for different server versions:
        //!     WSG 2.0: creation format is fully supported. When root instanceId is specified, POST will be done to that instance.
        //!     WSG 1.x: objectCreationJson can have only one relationship to existing object. This related object will be treated as "parent".
        //!     Server version can be checked by using GetWSClient()->GetServerInfo()
        //! @return JSON representing created data and new file ETag if available
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        //! Create an object with a relation. With optional file attachment
        //! @param relatedObjectId - relation target object (e. g. document for checkin)
        //! @param objectCreationJson - Parameter objectCreationJson must follow WSG 2.0 format for creating objects.
        //! @param filePath - file path to upload
        //! @param uploadProgressCallback - file upload progress
        //! @param ct
        //! @return JSON representing created object and new file ETag if available
        virtual AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        //! DEPRECATED- Use SendUpdateObjectRequestWithOptions instead
        //! Update specified object and optionally a file with one operation
        //! @param objectId - object identifier
        //! @param propertiesJson - object properties that need to be updated
        //! @param eTag - DEPRECATED - only used for WebApi 1.x. Original instance eTag for server to do check if it did not change.
        //! @param filePath - file path to upload. Only supported from WebApi 2.4
        //! @param uploadProgressCallback - file upload progress
        //! @param ct
        //! @return empty JSON and new file ETag if available
        virtual AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequest
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
        JobOptionsPtr m_jobOptions;

    public:
        WSCLIENT_EXPORT RequestOptions() {m_jobOptions = std::make_shared<JobOptions>(); };
        virtual ~RequestOptions() {}

        //! Retrieve options required for WSG asynchronous job operations
        //! Jobs API can be enabled through these options
        JobOptionsPtr GetJobOptions() { return m_jobOptions; }
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
        uint32_t    m_maxWaitTimeMs             = 4000; //4seconds
        uint32_t    m_maxJobDuratationTimeMs    = 3600000; //1 Hour

    public:
        WSCLIENT_EXPORT JobOptions() {};
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
    private:
        std::shared_ptr<struct ClientConnection> m_connection;
        IWSClientPtr m_serverClient;
        mutable LimitingTaskQueue<WSFileResult> m_fileDownloadQueue;

    private:
        WSRepositoryClient(std::shared_ptr<struct ClientConnection> connection);
        
        // Taken from WebServices team .NET implementation
        static Utf8String UrlDecode(Utf8String url);

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
                static const uint32_t UploadProcessing;
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

        WSCLIENT_EXPORT void SetCredentials(Credentials credentials, AuthenticationType type = AuthenticationType::Basic);

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

        //! DEPRECATED- Use SendChangesetRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSChangesetResult> SendChangesetRequest
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSChangesetResult> SendChangesetRequestWithOptions
            (
            HttpBodyPtr changeset,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! DEPRECATED- Use SendCreateObjectRequestWithOptions instead
        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequest
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSCreateObjectResult> SendCreateObjectRequestWithOptions
            (
            ObjectIdCR relatedObjectId,
            JsonValueCR objectCreationJson,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateObjectResult> SendUpdateObjectRequestWithOptions
            (
            ObjectIdCR objectId,
            JsonValueCR propertiesJson,
            Utf8StringCR eTag = nullptr,
            BeFileNameCR filePath = BeFileName(),
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
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
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSUpdateFileResult> SendUpdateFileRequestWithOptions
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            HttpRequest::ProgressCallbackCR uploadProgressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
