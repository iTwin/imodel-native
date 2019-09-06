/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

//#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatform/SpatialEntity.h>

#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//=====================================================================================
//! Overview:
//! The present classes serve as interfaces to the GeoCoordination Service.
//! Although the GeoCoordinationS Service is based on a simple WSG-based
//! REST API the GeoCoordination Service relies on a variety of interrelated classes
//! and the capacity to perform spatial or classification related queries renders the
//! construction of the request slightly tedious.
//! The present classes provide three levels of simplification of accessing the service
//! and interpreting the results.
//! Before continuing it is recommended to be familiar of the basic classes part of the
//! model definition (SpatialEntity, StatialEntityDataSource, SpatialEntityMetadata 
//! and SpatailEntityServer).
//! 
//! The GeoCoordination service API is based on equivalent EC Classes that represent mainly the
//! same concepts and the same fields.
//!
//! The first level of abstraction offered in the present higher level class organisation 
//! helps to compose REST API for common or custom queries. The second level of abstraction 
//! offers a mechanism to query the server for various common information without
//! requiring the client to compose the request itself or perform Http request or
//! interpret Http response.
//! The final abstraction level provides complete cooked up solution for
//! common data obtention from the GeoCoordination Service and the spatial data sources
//! it indexes.
//!
//!
//=====================================================================================

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! GeoCoordinationServiceRequest
//! This class represents a request to the GeoCoordination Service.
//=====================================================================================
struct GeoCoordinationServiceRequest : public WSGURL
    {
public:

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSchema() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRepoId() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUserAgent() const override;
    
    REALITYDATAPLATFORM_EXPORT virtual ~GeoCoordinationServiceRequest(){}
protected:
    // Default constructor
    REALITYDATAPLATFORM_EXPORT GeoCoordinationServiceRequest() { m_validRequestString = false; }

    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override = 0;
    };

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              03/2017
//! All fields for a GCS entry, on the server
//=====================================================================================
enum class GeoCoordinationField
    {
    Id,
    Footprint,
    Name,
    Description,
    ContactInformation,
    Keywords,
    Legal,
    TermsOfUse,
    DataSourceType,
    AccuracyInMeters,
    Date,
    Classification,
    FileSize,
    Streamed,
    SpatialDataSourceId,
    ResolutionInMeters,
    ThumbnailURL,
    DataProvider,
    DataProviderName,
    Dataset,
    Occlusion,
    MetadataURL,
    RawMetadataURL,
    RawMetadataFormat,
    SubAPI
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! GeoCoordinationServicePagedRequest
//! This class represents a request to the GeoCoordination Service that results in
//! a list of objects of potential unknown length.
//! This class inherits from the general request class but adds additional 
//! control for paging.
//! Default page size is 25 with, of course a start index of 0
//! To advance to next/previous page simply call AdvancePage() or RewindPage()
//=====================================================================================
struct GeoCoordinationServicePagedRequest : public WSGPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSchema() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRepoId() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUserAgent() const override;


    REALITYDATAPLATFORM_EXPORT GeoCoordinationServicePagedRequest() 
        { 
        m_validRequestString = false; 
        m_requestType = HttpRequestType::GET_Request; 
     //   m_sort = false; 
        m_startIndex = 0; 
        m_pageSize = 25; 
        }

    REALITYDATAPLATFORM_EXPORT void SetFilter(Utf8StringCR filter) { m_filter = filter; }

    //! Sets the sort order for the list. This sorting is performed server-side.
    //! Note that it is not possible to specify two sorts (sort by field a then by filed b is not supported).
    //! The server will decide how sorted groups are ordered.
    //! Note that some fields in the server are considered case-sensitive and others
    //!  case insensitive. The server will apply sort rules accordingly.
    REALITYDATAPLATFORM_EXPORT void SortBy(GeoCoordinationField, bool ascending);

    REALITYDATAPLATFORM_EXPORT virtual ~GeoCoordinationServicePagedRequest(){}

protected:
    virtual void _PrepareHttpRequestStringAndPayload() const override = 0;

    GeoCoordinationServicePagedRequest(uint16_t startIndex, uint8_t pageSize) 
        { 
        m_startIndex = startIndex; 
        m_pageSize = pageSize; 
        BeAssert(m_pageSize >0); 
        }

    Utf8String m_order;
    Utf8String m_filter;
    Utf8String m_sort;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityWithDetailsSpatialRequest
//! This class represents a spatial request for SpatialEntityWithDetails class object.
//! This represents the most common GeoCoordination Service request.
//! This request returns the list of SpatialEntityWithDetails objects that 
//! are located within provided spatial area (usually the project area) for the 
//! indicated classification. Additional parameters can be provided after creation.
//=====================================================================================
struct SpatialEntityWithDetailsSpatialRequest : public GeoCoordinationServicePagedRequest
    {
public:
    //! Create a request for spatial entity with details in the area covered by given polygon for specific classification
    REALITYDATAPLATFORM_EXPORT SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d> projectArea, int classification = -1);

    REALITYDATAPLATFORM_EXPORT void FilterBySource(int informationSource) { m_informationSourceFilter = informationSource; }
    
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

    bvector<GeoPoint2d>     m_projectArea;
    int                     m_informationSourceFilter;
    bool                    m_informationSourceFilteringSet;
private:
    SpatialEntityWithDetailsSpatialRequest() : m_informationSourceFilteringSet(false) { m_requestType = GET_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityWithDetailsByIdRequest
//! This class represents a request for specific SpatialEntity with details class object.
//! This request enables accessing details about a specific spatial entity with details.
//=====================================================================================
struct SpatialEntityWithDetailsByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT SpatialEntityWithDetailsByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityWithDetailsByIdRequest() { m_requestType = HttpRequestType::GET_Request; }
    };
    
//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityByIdRequest
//! This class represents a request for specific SpatialEntity class object.
//! This request enables accessing details about a specific spatial entity.
//=====================================================================================
struct SpatialEntityByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT SpatialEntityByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityByIdRequest() { m_requestType = HttpRequestType::GET_Request; }
    };


//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityDataSourceByIdRequest
//! This class represents a request for specific SpatialEntity data source class object.
//! This request enables accessing details about a specific spatial entity.
//=====================================================================================
struct SpatialEntityDataSourceByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityDataSourceByIdRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityServerByIdRequest
//! This class represents a request for specific SpatialEntity Server class object.
//! This request enables accessing details about a specific spatial entity server.
//=====================================================================================
struct SpatialEntityServerByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT SpatialEntityServerByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityServerByIdRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityMetadataByIdRequest
//! This class represents a request for specific SpatialEntity Metadata class object.
//! This request enables accessing details about a specific spatial entity metadata.
//=====================================================================================
struct SpatialEntityMetadataByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityMetadataByIdRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! PackagePreparationRequest
//! This class represents a request for preparation of a package.
//! This request enables to instruct the GeoCoordination Service to prepare 
//! a package over a designated project area for a listed set of spatial entities. 
//=====================================================================================
struct PackagePreparationRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT PackagePreparationRequest(bvector<GeoPoint2d> projectArea, bvector<Utf8String> listOfSpatialEntities);
   
    REALITYDATAPLATFORM_EXPORT virtual ~PackagePreparationRequest(){}
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    bvector<GeoPoint2d>     m_projectArea;
    bvector<Utf8String>     m_listOfSpatialEntities;
    PackagePreparationRequest() { m_requestType = HttpRequestType::POST_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! PreparedPackageRequest
//! This class represents a request for specific prepared package content.
//! This request enables obtaining the content of the package.
//=====================================================================================
struct PreparedPackageRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT PreparedPackageRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    PreparedPackageRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//=====================================================================================
//! @bsiclass                              Spencer.Mason                10/2018
//! PreparedPackagesRequest
//! This class represents a request for all prepared packages for the current user
//! (as specified by the token)
//=====================================================================================
struct PreparedPackagesRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT PreparedPackagesRequest(Utf8StringCR identifier)
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    PreparedPackagesRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//=====================================================================================
//! @bsiclass                                Spencer.Mason                09/2018
//! LastPackageRequest
//! This class represents a request for the latest prepared package content.
//! This request enables obtaining the content of the package.
//=====================================================================================
struct LastPackageRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT LastPackageRequest()
        { 
        m_validRequestString = false; 
        }
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! DownloadReportUploadRequest
//! This class represents a request for uploading to the GeoCoordination Service
//! a download report.
//=====================================================================================
struct DownloadReportUploadRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT DownloadReportUploadRequest(Utf8StringCR guid, Utf8StringCR identifier, BeFileName report);
   
    REALITYDATAPLATFORM_EXPORT uint64_t GetMessageSize() const { return m_fileSize; }
    REALITYDATAPLATFORM_EXPORT BeFileName GetFileName() const { return m_downloadReport; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    Utf8String          m_guid;
    BeFileName          m_downloadReport;
    uint64_t            m_fileSize;
    DownloadReportUploadRequest(){}
    };

//=====================================================================================
//! @bsiclass                                 Spencer.Mason                   01/2018
//! BingKeyRequest
//! This class represents a request for a Bing Key along with its expirationDate
//=====================================================================================
struct BingKeyRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT BingKeyRequest(Utf8StringCR productId)
        {
        m_validRequestString = false;
        m_id = productId;
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    BingKeyRequest() { m_requestType = HttpRequestType::GET_Request; }
    };

//! Callback function to surface request errors.
//! @param[in] basicMessage Utf8String provided by the specific request
//! @param[in] rawResponse  the raw server response
typedef std::function<void(Utf8String basicMessage, const RawServerResponse& rawResponse)> GeoCoordinationService_ErrorCallBack;

//! Callback function to perform a request
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @param[in] request             Structure containing the parameters of the request
//! @param[out] rawResponse        the response returned by the server
typedef std::function<void(const DownloadReportUploadRequest& request, RawServerResponse& rawResponse)> GeoCoordinationService_RequestCallback;

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! GeoCoordinationService
//! This class represents the GeoCoordination Service proper. This unique class
//! provides functionalty to query or upload to the GeoCoordination Service by basing
//! command upon requests objects.
//! The methods can return properly formatted SpatialEntity objects or the
//! raw JSON or byte-stream resulting from the request.
//=====================================================================================
struct GeoCoordinationService
    {
public:
    //!
    //! The SetServerComponents static method enables to set the GeoCoordination Service URL REST API component strings
    //! The server parameter contains the name of the server including the communication protocol. 
    //! The WSGProtocol is a string containing the WSG version number. Default is '2.4'
    //! name is the name of the WSG service for the GeoCoordination Service. It should always be "IndexECPlugin-Server"
    //! schemaName is the name of the schema exposing the GeoCoordination Service classes. Default is "RealityModeling"
    //! All fields must be provided if used. Normally the present method shold only be used for development purposes
    //! When accessing one of the dev or qa version of GeoCoordination Service.
    REALITYDATAPLATFORM_EXPORT static void SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8String projectId = "")
        {
        BeAssert(server.size() != 0);
        BeAssert(WSGProtocol.size() != 0);
        BeAssert(repoName.size() != 0);
        BeAssert(schemaName.size() != 0);

        s_geoCoordinationServer = server;
        s_geoCoordinationWSGProtocol = WSGProtocol;
        s_geoCoordinationRepoName = repoName;
        s_geoCoordinationSchemaName = schemaName;
        s_geoCoordinationProjectId = projectId;
        }

    REALITYDATAPLATFORM_EXPORT static void SetErrorCallback(GeoCoordinationService_ErrorCallBack errorCallback) { s_errorCallback = errorCallback; }

    //! Returns the current name of the server
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetServerName();

    //! Results the string containing the WSG protocol version number
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetWSGProtocol();

    //! Returns the name of the WSG repository containing the GeoCoordination Service objects
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetRepoName();

    //! Returns the name of the schema defining the classes exposed by the GeoCoordination Service.
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetSchemaName();

    //! Returns the name of the schema defining the classes exposed by the Service.
    REALITYDATAPLATFORM_EXPORT static const bool GetVerifyPeer();

    //! Returns the name of the schema defining the classes exposed by the Service.
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetCertificatePath();

    //! Returns the GUID for the currently associated project
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetProjectId();

    //! Sets the GUID for the associated project
    REALITYDATAPLATFORM_EXPORT static void SetProjectId(Utf8StringCR projectId);

    //! Returns the user agent
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetUserAgent();

    //! Sets the user agent
    REALITYDATAPLATFORM_EXPORT static void SetUserAgent(Utf8StringCR userAgent);

    enum class InformationSource
        {
        USGS_NationalMap = 0x01,
        PublicIndex = 0x02
        };


    //! Returns the SpatialEntity object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Request(const SpatialEntityByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a partially filled SpatialEntity object or null if an error occured.
    //! Only fields pertinent to a SpatialEntityWithDetails view will be filled 
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Request(const SpatialEntityWithDetailsByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a SpatialEntitDataSource or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Request(const SpatialEntityDataSourceByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a SpatialEntityServer or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Request(const SpatialEntityServerByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a SpatialEntityMetadata object ot null if an error occured.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr Request(const SpatialEntityMetadataByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of partially filled SpatialEntity objects. Only fields returned in a
    //! SpatialEntity with details object will be filled.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<SpatialEntityPtr> Request(const SpatialEntityWithDetailsSpatialRequest& request, RawServerResponse& rawResponse);

    //! Returns the identifier of the prepared package or an empty string if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const PackagePreparationRequest& request, RawServerResponse& rawResponse);

    //! Returns the content of the Package requested or an empty vector if an error occured
    REALITYDATAPLATFORM_EXPORT static void Request(const PreparedPackageRequest& request, BeFileName filename, RawServerResponse& rawResponse);

    //! Returns the description of the last Package requested by the current user
    REALITYDATAPLATFORM_EXPORT static void Request(const PreparedPackagesRequest& request, RawServerResponse& rawResponse, bvector<Utf8String>& packageIds);

    //! Returns the description of the last Package requested by the current user
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const LastPackageRequest& request, RawServerResponse& rawResponse);

    //! Uploads a download report. It is not possible to know if the call worked or not.
    REALITYDATAPLATFORM_EXPORT static void Request(const DownloadReportUploadRequest& request, RawServerResponse& rawResponse);

    //! Fetches a Bing Key.
    REALITYDATAPLATFORM_EXPORT static void Request(const BingKeyRequest& request, RawServerResponse& rawResponse, Utf8StringR key, Utf8StringR expirationDate);

    //! Returns the full WSG JSON returned by the package preparation request
    REALITYDATAPLATFORM_EXPORT static RawServerResponse BasicRequest(const GeoCoordinationServiceRequest* request, Utf8StringCR keyword = "instances");

    //! Returns the full WSG JSON returned by the spatial entity with details spatial request
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static RawServerResponse BasicPagedRequest(const GeoCoordinationServicePagedRequest* request, Utf8StringCR keyword = "instances");

    REALITYDATAPLATFORM_EXPORT static void SetRequestCallback(GeoCoordinationService_RequestCallback pi_func) { s_requestCallback = pi_func; }

    static Utf8String s_geoCoordinationServer;
    static Utf8String s_geoCoordinationWSGProtocol;
    static Utf8String s_geoCoordinationRepoName;
    static Utf8String s_geoCoordinationRepoNameWProjectId;
    static Utf8String s_geoCoordinationSchemaName;
    static Utf8String s_geoCoordinationProjectId;
    static Utf8String s_geoCoordinationUserAgent;

    static bool s_verifyPeer;
    static Utf8String s_certificatePath;

    static const Utf8String s_ImageryKey;
    static const Utf8String s_TerrainKey;
    static const Utf8String s_ModelKey;
    static const Utf8String s_PinnedKey;

    static const Utf8String s_USGSInformationSourceKey;
    static const Utf8String s_PublicIndexInformationSourceKey;
    static const Utf8String s_AllInformationSourceKey;

    static GeoCoordinationService_ErrorCallBack s_errorCallback;

private:
    REALITYDATAPLATFORM_EXPORT static bvector<SpatialEntityPtr> SpatialEntityRequestBase(const GeoCoordinationServiceRequest* request, RawServerResponse& rawResponse);

    static GeoCoordinationService_RequestCallback s_requestCallback;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE