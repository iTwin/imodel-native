/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/GeoCoordinationService.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>

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
struct GeoCoordinationServiceRequest : public RefCountedBase
    {
public:



    enum class HttpRequestType
        {
        GET_Request,
        PUT_Request,
        POST_Request
        }

    //! Returns the full http request string
    Utf8StringCR GetHttpRequestString() const 
        {
        if (!m_validRequestString)
            _PrepareHttpRequestString();

        BeAssert(m_validRequestString);
        BeAssert(m_httpRequestString.size() != 0);

        return m_httpRequestString;
        };

    Utf8StringCR GetRequestPayload() const
        {
        if (!m_validRequestString)
            _PrepareHttpRequestString();

        BeAssert(m_validRequestString);

        return m_requestPayload;
        }

protected:
    // Default constructor
    GeoCoordinationService() : m_validRequestString(false) {}

    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const = 0;

    static Utf8String s_geoCoordinationServer = "https://connect-contextservices.bentley.com/";
    static Utf8String s_geoCoordinationWSGProtocol = "2.4";
    static Utf8String s_geoCoordinationName = "IndexECPlugin-Server";
    static Utf8String s_geoCoordinationSchemaName = "RealityModeling";

    static const Utf8String s_ImageryKey = "Imagery";
    static const Utf8String s_TerrainKey = "Terrain";
    static const Utf8String s_ModelKey = "Model";
    static const Utf8String s_PinnedKey = "Pinned";

    static const Utf8String s_USGSInformationSourceKey = "usgsapi";
    static const Utf8String s_PublicIndexInformationSourceKey = "index";
    static const Utf8String s_AllInformationSourceKey = "all";

    mutable bool m_validRequestString;
    mutable Utf8String m_httpRequestString;
    mutable Utf8String m_requestPayload;

    HttpRequestType m_requestType;
    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! GeoCoordinationServicePagedRequest
//! This class represents a request to the GeoCoordination Service that results in
//! a list of objects of potential unknown length.
//! This class inherits from the general request class but adds additional 
//! control for paging.
//! Default page size is 25 with, of course a start index of 0
//! To advance to next/previous page simply call AdvancePage() or RewingPage()
//=====================================================================================
struct GeoCoordinationServicePagedRequest : public GeoCoordinationServiveRequest
    {
public:

    REALITYDATAPLATFORM_EXPORT StatusInt SetPageSize(uint8_t pageSize) {BeAssert(pageSize > 0); m_pageSize = pageSize;}
    REALITYDATAPLATFORM_EXPORT uint8_t GetPageSize() {return m_pageSize;}
  
    REALITYDATAPLATFORM_EXPORT StatusInt SetStartIndex(uint16_t startIndex) {m_startIndex = startIndex;} 

    REALITYDATAPLATFORM_EXPORT StatusInt AdvancePage() {m_validRequestString = false; m_startIndex += m_pageSize;}
    REALITYDATAPLATFORM_EXPORT StatusInt RewindPage() {m_validRequestString = false; m_startIndex = (m_startIndex <= m_pageSize ? 0 : m_startIndex-m_pageSize);}
 



protected:
    // Default constructor
    GeoCoordinationServicePagedRequest() : m_startIndex(0), m_pageSize(25) {}
    GeoCoordinationServicePagedRequest(uint16_t startIndex, uint8_t pageSize) : m_startIndex(startIndex), m_pageSize(pageSize) {BeAssert(m_pageSize >0);}

    uint16_t m_startIndex;
    uint8_t m_pageSize;

    }


//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityWithDetailsSpatialRequest
//! This class represents a spatial request for SpatialEntityWithDetails class object.
//! This represents the most common GeoCoordination Service request.
//! This request returns the list of SpatialEntityWithDetails objects that 
//! are located within provided spatial area (usually the project area) for the 
//! incdicated classification. Additional parameters can be provided after creation.
//=====================================================================================
struct SpatialEntityWithDetailsSpatialRequest : public GeoCoordinationServicePagedRequest
    {
public:
    //! Create a request for spatial entity with details in the area covered by given polygon for specific classification
    REALITYDATAPLATFORM_EXPORT static SpatialEntityWithDetailsSpatialRequestPtr Create(bvector<GeoPoint2D> projectArea, int classification);

    REALITYDATAPLATFORM_EXPORT StatusInt FilterBySource(int informationSource);
    
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

    int m_informationSourceFilter;
    bool m_informationSourceFilteringSet;
private:
    SpatialEntityWithDetailsSpatialRequest() : m_informationSourceFilteringSet(false), m_requestType(GET_Request) {}
    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! SpatialEntityWithDetailsByIdRequest
//! This class represents a request for specific SpatialEntity with details class object.
//! This request enables accessing details about a specific spatial entity with details.
//=====================================================================================
struct SpatialEntityByDetailsByIdRequest : public GeoCoordinationServiceRequest
    {
public:
    //! Create a request for spatial entity of the given identifier
    REALITYDATAPLATFORM_EXPORT static SpatialEntityByDetailsByIdRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityByDetailsByIdRequest() {}
    }
    
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
    REALITYDATAPLATFORM_EXPORT static SpatialEntityByIdRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityByIdRequest(): m_requestType(GET_Request) {}
    }


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
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourceByIdRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityDataSourceByIdRequest(): m_requestType(GET_Request) {}
    }

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
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerByIdRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityServerByIdRequest(): m_requestType(GET_Request) {}
    }

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
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataByIdRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    SpatialEntityMetadataByIdRequest(): m_requestType(GET_Request) {}
    }

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
    REALITYDATAPLATFORM_EXPORT static PreparedPackageRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    PreparedPackageRequest(): m_requestType(GET_Request) {}
    }

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
    REALITYDATAPLATFORM_EXPORT static PackagePreparationRequestPtr Create(bvector<GeoPoint2D> projectArea, bvector<Utf8String> listOfSpatialEntities);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    PackagePreparationRequest(): m_requestType(POST_Request) {}
    }

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
    REALITYDATAPLATFORM_EXPORT static DownloadReportUploadRequestPtr Create(Utf8StringCR identifier);
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    DownloadReportUploadRequest(): m_requestType(POST_Request) {}
    }


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
    //! The server parameter contains the name of the server including the communication protocol. The default value is
    //! https://connect-contextservices.bentley.com/ 
    //! The WSGProtocol is a string containing the WSG version number. Default is '2.4'
    //! name is the name of the WSG service for the GeoCoordination Service. It should always be "IndexECPlugin-Server"
    //! schemaName is the name of the schema exposing the GeoCoordination Service classes. Default is "RealityModeling"
    //! All fields must be provided if used. Normally the present method shold only be used for development purposes
    //! When accessing one of the dev or qa version of GeoCoordination Service.
    static SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR name, Utf8StringCR schemaName)
        {
        BeAssert(server.size() != 0);
        BeAssert(WSGProtocol.size() != 0);
        BeAssert(name.size() != 0);
        BeAssert(schemaName.size() != 0);

        s_geoCoordinationServer = server;
        s_geoCoordinationWSGProtocol = WSGProtocol;
        s_geoCoordinationName = name;
        s_geoCoordinationSchemaName = schemaName;
        }

    //! Returns the current name of the server
    static Utf8StringCR GetServer();

    //! Results the string containing the WSG protocol version number
    static Utf8StringCR GetWSGProtocol();

    //! Returns the name of the WSG repository containing the GeoCoordination Service objects
    static Utf8StringCR GetName();

    //! Returns the name of the schema defining the classes exposed by the GeoCoordination Service.
    static Utf8StringCR getSchemaName();

    //! The classification codes. The high level interface only supports the four base classification
    enum class Classification
        {
        Imagery = 0x1,
        Terrain = 0x2;
        Model = 0x4;
        Pinned = 0x8;
        }

    enum class InformationSource
        {
        USGS_NationalMap = 0x01;
        PublicIndex = 0x02;
        }
public:
    //! Returns the SpatialEntity object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Request(SpatialEntityByIdRequestCR request);

    //! Returns a partially filled SpatialEntity object or null if an error occured.
    //! Only fields pertinent to a SpatialEntityWithDetails view will be filled 
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Request(SpatialEntityWithDetailsByIdRequestCR request);

    //! Returns a SpatialEntitDataSource or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Request(SpatialEntityDataSourceByIdCR request);

    //! Returns a SpatialEntityServer or null if an error occured
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Request(SpatialEntityServerByIdRequestCR request);

    //! Returns a SpatialEntityMetadata object ot null if an error occured.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr Request(SpatialEntityMetadataByIdRequestCR request);

    //! Returns a list of partially filled SpatialEntity objects. Only fields returned in a
    //! SpatialEntity with details object will be filled.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<SpatialEntityPtr> Request(SpatialEntityWithDetailsSpatialRequestCR request);

    //! Returns the content of the Package requested or an empty vector if an error occured
    REALITYDATAPLATFORM_EXPORT static bvector<byte> Request(PreparedPackageRequestCR request);

    //! Returns the identifier of the prepared package or an empty string if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(PackagePreparationRequestCR request);

    //! Uploads a download report. It is not possible to know if the call worked or not.
    REALITYDATAPLATFORM_EXPORT static void Request(DownloadReportUploadRequestCR request);

    //! Returns the full WSG JSON returned by the spatial entity request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityByIdRequestCR request);

    //! Returns the full WSG JSON returned by the spatial entity with details request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityWithDetailsByIdRequestCR request);

    //! Returns the full WSG JSON returned by the spatial entity data source request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityDataSourceByIdCR request);

    //! Returns the full WSG JSON returned by the spatial entity server request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityServerByIdRequestCR request);

    //! Returns the full WSG JSON returned by the spatial entity metadata request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityMetadataByIdRequestCR request);

    //! Returns the full WSG JSON returned by the spatial entity with details spatial request
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(SpatialEntityWithDetailsSpatialRequestCR request);

    //! Returns the full WSG JSON returned by the package preparation request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(PackagePreparationRequestCR request);

    }


END_BENTLEY_REALITYPLATFORM_NAMESPACE