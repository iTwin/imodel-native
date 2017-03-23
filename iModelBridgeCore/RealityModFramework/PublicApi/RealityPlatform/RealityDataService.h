/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataService.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/WSGServices.h>
#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatform/RealityDataDownload.h>

#include <Bentley/BeFile.h>
#include <Bentley/BeFilename.h>
#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>

#define CHUNK_SIZE                  (4*1024*1024) //4Mb 
BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Overview:
//! We want RealityData Requests to inherit from WSGURL requests, but there are many
//! functions and variables that won't be used because we'll be calling the
//! RealityDataService class; so we need to override them to avoid confusion, in case
//! a user tries to call methods from the base class
//!
//! @bsiclass                                         Spencer.Mason             12/2016
//! RealityDataUrl
//! This class represents an URL for a RealityData request.
//=====================================================================================
struct RealityDataUrl : public WSGURL
    {
public:

    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataUrl() {};

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSchema() const override;

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRepoId() const override;

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataEnterpriseStat
//! This class returns the size in KB currently used.
//=====================================================================================
struct RealityDataEnterpriseStatRequest : public RealityDataUrl
{
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataEnterpriseStatRequest(Utf8StringCR enterpriseId) { m_validRequestString = false; m_id = enterpriseId; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataEnterpriseStatRequest() {}
};


//=====================================================================================
//! Overview:
//! The present classes serves as an interface to the RealityDataService.
//! Although the RealityData Service is based on a simple WSG-based
//!  REST api, it relies on a variety of interrelated classes and the
//!  capacity to perform spatial or classification related queries renders the
//!  construction of the request slightly tedious.
//! The present classes provide three levels of simplification of accessing the service
//!  and interpreting the results.
//! Before continuing it is recommended to be familiar of the basic classes part of the
//!  model definition (RealityData, RealityDataProjectRelationship, Folder, and Document).
//! 
//! The RealityData service API is based on equivalent EC Classes that represent mainly the
//!  same concepts and the same fields.
//!
//! The first level of abstraction offered in the present higher level class organisation 
//!  helps to compose REST api for common or custom queries. The second level of abstraction 
//!  offers a mechanism to query the server for various common information without
//!  requiring the client to compose the request itself or perform Http request or
//!  interpret Http response.
//! The final abstraction level provides complete cooked up solution for
//! common data obtention from the RealityData Service.
//!
//!
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataByIdRequest
//! This class represents a request for specific RealityData class object.
//=====================================================================================
struct RealityDataByIdRequest : public RealityDataUrl
    {
public:
	// Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataByIdRequest(){}
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataByIdRequest() {}
    };
	
//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataProjectRelationshipByIdRequest
//! This class represents a request for specific RealityDataProjectRelationship 
//!  class object. Need to check if this class is necessary. We can return
//!  all projects that have a link with a certain RealityData
//=====================================================================================
struct RealityDataProjectRelationshipByProjectIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataProjectRelationshipByProjectIdRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataProjectRelationshipByProjectIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataProjectRelationshipByIdRequest
//! This class represents a request for specific RealityDataProjectRelationship 
//!  class object. Need to check if this class is necessary. We can return
//!  all projects that have a link with a certain RealityData
//=====================================================================================
struct RealityDataProjectRelationshipByRealityDataIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataProjectRelationshipByRealityDataIdRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataProjectRelationshipByRealityDataIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataFolderByIdRequest
//! This class represents a request for specific RealityDataFolder class object.
//=====================================================================================
struct RealityDataFolderByIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataFolderByIdRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataFolderByIdRequest(){}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataFolderByIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataDocumentByIdRequest
//! This class represents a request for specific RealityDataDocument class object.
//=====================================================================================
struct RealityDataDocumentByIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentByIdRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataDocumentByIdRequest(){}
	
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataDocumentByIdRequest() {}
    };

struct AzureHandshake : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT AzureHandshake(Utf8String sourcePath, bool isWrite);
    REALITYDATAPLATFORM_EXPORT Utf8StringR GetJsonResponse() { return m_jsonResponse; }
    REALITYDATAPLATFORM_EXPORT BentleyStatus ParseResponse(Utf8StringR azureServer, Utf8StringR azureToken, int64_t& tokenTimer);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
private:
    Utf8String m_jsonResponse;
    bool       m_isWrite;
    AzureHandshake();
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataDocumentContentByIdRequest
//! This class represents a request for specific RealityDataDocument content class object.
//! The present class provides services for the support of azure redirection to blob.
//! The RealityDataService can query the class to check if azure redirection is possible
//!  or not. If the object indicates the redirection is possible but is not yet
//!  redirected then the service will fetch the azure blob redirection request
//!  and call the WSG service. If the blob address to the container is returned
//!  then the service will set the azure blob redirection URL. After which the
//!  object will be set to access directly the blob.
//! Example:
//! RealityDataDocumentContentByIdRequest myRequest("0586-358df-445-de34a-dd286", "RootDocument.3mx");
//! ...
//! if (myRequest.IsAzureRedirectionPossible())
//!     {
//!     if (!myRequest.IsAzureBlobRedirected())
//!         {
//!         Utf8String redirectRequest = myRequest.GetAzureRedirectionRequestUrl();
//!         if (redirectRequest.size() == 0)
//!             myRequest.SetAzureRedirectionPossible(false);
//!         else
//!             SetAzureRedirectionUrlToContainer(blobContainerUrl);
//!         }
//!    // After this the request will provide the proper http ulr, header and body
//!    //  either to the blob or RealityDataService
//!
//!    }
//=====================================================================================
struct RealityDataDocumentContentByIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8StringCR identifier) : m_handshakeRequest(0)
    { m_validRequestString = false; m_id = identifier; }
    
    //REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8CP identifier) : m_identifier(identifier) {}
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(const RealityDataDocumentContentByIdRequest &object); 

    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataDocumentContentByIdRequest(){}
	
    //! This call modify the indentifier of the object. Since we want a 
	//!  different ressource. This can be a folder, document or anything
	//!  else. 
    REALITYDATAPLATFORM_EXPORT void ChangeInstanceId(Utf8String instanceId);

    //! This call creates the URL request to obtain the azure redirection URL.
    REALITYDATAPLATFORM_EXPORT RequestStatus GetAzureRedirectionRequestUrl() const;

    //! Once the azure blob container URL has been obtained it must be set
    //!  using this method after which the object will create azure redirection.
    //REALITYDATAPLATFORM_EXPORT void SetAzureRedirectionUrlToContainer(Utf8String azureContainerUrl);

    //! Indicates that an azure blob redirection url has been set to object
    REALITYDATAPLATFORM_EXPORT bool IsAzureBlobRedirected();

    //! Used to indicate the azure blob redirection is possible or not. The default value is true
    //!  but if the service does not support azure redirection it must be set to false to
    //!  prevent attempts at obtaining redirection.
    REALITYDATAPLATFORM_EXPORT void SetAzureRedirectionPossible(bool possible);

    //! Indicates if azure blob container redirection is possible
    REALITYDATAPLATFORM_EXPORT bool IsAzureRedirectionPossible();

    REALITYDATAPLATFORM_EXPORT int64_t GetTokenTimer() const { return m_azureTokenTimer; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

    mutable AzureHandshake*             m_handshakeRequest;

    mutable Utf8String  m_azureServer;
    mutable Utf8String  m_azureToken;
    mutable bool        m_AzureRedirected;
    mutable bool        m_allowAzureRedirection;
    mutable int64_t     m_azureTokenTimer;
    RealityDataDocumentContentByIdRequest() {}

    };

//! The classification codes. The high level interface only supports the four base classification
//&&AR Most to platform since Classification is shared by both GeoCoordinationService and RealityData Service
enum class Classification
    {
    Imagery = 0x1,
    Terrain = 0x2,
    Model = 0x4,
    Pinned = 0x8
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataFilterCreator
//! Helper module used to compose filter components for RealityData list extraction
//! based on filter criteria such as a type or spatial overlap.
//! The filter takes the form of a string that is provided to filtered request
//=====================================================================================
struct RealityDataFilterCreator
    {
    //! Sets filtering upon the name. 
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByName(Utf8String name);

    //! Sets filtering upon the classification. The classification may contain
    //!  more than one classification by bitwise oring the classification
    //!  values.
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByClassification(Classification classification);

    //! Filters the returned set by the reality data size.
    //! Both the min and max size must be specified
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterBySize(double minSize, double maxSize);

    //! Sets a spatial filter. Only RealityData for which the footprint overlaps (even
    //! partially) the given region will be selected.
    //! The area provided is a list of geo points (longitude/latitude)
    //!  that must form a closed area. The last point of the list must
    //!  be equal to the first point.
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterSpatial(bvector<GeoPoint2d> area, uint64_t coordSys);

    //! Filters the list by owner. Only reality data belonging to given owner
    //!  will be returned. The owner is specified by the email address
    //!  and is case insensitive.
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByOwner(Utf8String owner);

    //! Filters the list by creation date. To indicate either min or max date
    //!  are unbounded simply provide an invalid/unset DataTime object
    //!  If both dates are invalid/unset then the command will return an error
    //!  and no filtering will be set.
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByCreationDate(DateTime minDate, DateTime maxDate);

    //! Filters the list by modification date. To indicate either min or max date
    //!  are unbounded simply provide an invalid/unset DataTime object.
    //! If both dates are invalid/unset then the command will return an error
    //!  and no filtering will be set.  
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByModificationDate(DateTime minDate, DateTime maxDate);

    //! Filters in or out public data as specified
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterVisibility(RealityDataBase::Visibility visibility);
        
    //! Filter by resolution. As resolution may be confusing since minimum resolution is
    //!  expressed a higher number the resolution can be specified in any order and
    //!  internally the resolution will be applied accordingly.
    //! Reality data that have no resolution set will be considered 'unspecified' and
    //!  will be returned whatever the resolution bracket given if filterOutUnspecified is false
    //!  and will be discarded if true
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByResolution(double resMin, double resMax, bool filterOutUnspecified);

    //! Filter by accuracy. As accuracy may be confusing since minimum accuracy is
    //!  expressed a higher number the accuracy can be specified in any order and
    //!  internally the accuracy will be applied accordingly.    
    //! Reality data that have no accuracy set will be considered 'unspecified' and
    //!  will be returned whatever the bracket given if filterOutUnspecified is false
    //!  and will be discarded if true
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified);

    //! Filter by type. The type is specified by a string in the reality data.
    //! The filter type specification here can contain many types
    //!  separated by semi-colons. All reality data of any of the specified types
    //!  will be returned in the list.
    //!  types are case insensitive
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByType(Utf8String types);

    //! Filter by dataset. Only reality data of specified dataset will be returned
    //!  note that Dataset names are case-sensitive.
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByDataset(Utf8String dataset);

    //! Sets filtering upon the group. 
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterByGroup(Utf8String group);

    //! Filter relationship by RealityDataId. Only relationships for specified RealityDataId will be returned
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterRelationshipByRealityDataId(Utf8String realityDataId);

    //! Filter relationship by ProjectId. Only relationships for specified ProjectId will be returned
    REALITYDATAPLATFORM_EXPORT static Utf8String FilterRelationshipByProjectId(Utf8String projectId);
    
    //! Groups all filters inside of parentheses, all criteria must be met ( && )
    REALITYDATAPLATFORM_EXPORT static Utf8String GroupFiltersAND(bvector<Utf8String> filters);

    //! Groups all filters inside of parentheses, only one of the criteria must be met ( || )
    REALITYDATAPLATFORM_EXPORT static Utf8String GroupFiltersOR(bvector<Utf8String> filters);
    };

enum class RealityDataField
    {
    Id,
    EnterpriseId,
    ContainerName,
    Name,
    Dataset,
    Description,
    RootDocument,
    Size,
    Classification,
    Type,
    Footprint,
    ThumbnailDocument,
    MetadataURL,
    ResolutionInMeters,
    AccuracyInMeters,
    Visibility,
    Listable,
    CreatedTimestamp,
    ModifiedTimestamp,
    OwnedBy,
    Group
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert                    12/2016
//! This class represents a spatial request for Reality Data class object.
//! This represents the most common RealityData Service request.
//! This request returns the list of SpatialEntity objects that 
//!  are located within provided spatial area (usually the project area) for the 
//!  incdicated classification. Additional parameters can be provided by adding a filter
//!  created using the RealityDataFilterCreator module.
//=====================================================================================
struct RealityDataPagedRequest : public WSGPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataPagedRequest() {}
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSchema() const override;
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRepoId() const override;

    REALITYDATAPLATFORM_EXPORT RealityDataPagedRequest() : m_informationSourceFilteringSet(false) { m_validRequestString = false; m_requestType = HttpRequestType::GET_Request; m_sort = false; }

    REALITYDATAPLATFORM_EXPORT void SetFilter(Utf8StringCR filter);
    REALITYDATAPLATFORM_EXPORT void SetQuery(Utf8StringCR query);

    //! Sets the sort order for the list. This sorting is performed server-side.
    //! Note that it is not possible to specify two sorts (sort by field a then by filed b is not supported).
    //! The server will decide how sorted groups are ordered.
    //! Note that some fields in the server are considered case-sensitive and others
    //!  case insensitive. The server will apply sort rules accordingly.
    REALITYDATAPLATFORM_EXPORT void SortBy(RealityDataField, bool ascending);

protected:
    REALITYDATAPLATFORM_EXPORT void _PrepareBaseRequestString() const;
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

    int m_informationSourceFilter;
    bool m_informationSourceFilteringSet;
    Utf8String m_order;
    Utf8String m_filter;
    Utf8String m_query;
    Utf8String m_sort;
    };


//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! for specific enterprise. Usually a CONENCT user only has access to its own enterprise 
//! data only so the enterpriseId specified should be the identifeir of its enterprise.
//! This request will not return public references to reality data from other enterprises
//! marked as public.
//! Note that the present request will only return Reality Data part of an enterprise
//! for which the current CONNECT user has access to. 
//=====================================================================================
struct RealityDataListByEnterprisePagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataListByEnterprisePagedRequest(Utf8StringCR identifier = "", uint16_t startIndex = 0, uint16_t pageSize = 25) { m_validRequestString = false; m_id = identifier; m_startIndex = startIndex; m_pageSize = pageSize; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! explicitely linked to a specific CONNECT Project through the Reality Data Service
//! RealityData/Project registry it maintains. 
//=====================================================================================
struct RealityDataProjectRelationshipByProjectIdPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataProjectRelationshipByProjectIdPagedRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataProjectRelationshipByProjectIdPagedRequest() {}
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! explicitely linked to a specific CONNECT Project through the Reality Data Service
//! RealityData/Project registry it maintains. 
//=====================================================================================
struct RealityDataProjectRelationshipByRealityDataIdPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataProjectRelationshipByRealityDataIdPagedRequest(Utf8StringCR identifier) { m_validRequestString = false; m_id = identifier; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataProjectRelationshipByRealityDataIdPagedRequest() {}
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A request for a list of all documents in a repository
//=====================================================================================
struct AllRealityDataByRootId : public RealityDataDocumentContentByIdRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT AllRealityDataByRootId(Utf8StringCR rootId); 

    REALITYDATAPLATFORM_EXPORT void SetMarker(Utf8String marker) const { m_validRequestString = false; m_marker = marker; }

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFilter() const { return m_filter; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    mutable Utf8String  m_marker;
    Utf8String          m_filter;
    AllRealityDataByRootId() {}
    };

//=====================================================================================
//! @bsimethod                                   Spencer.Mason 02/2017
//! The following are the declaration for callback for the upload process.
//=====================================================================================



//! Callback function to follow the download progression.
//! @param[in] filename    name of the file. 
//! @param[in] progress    Percentage uploaded.
typedef std::function<void(Utf8String filename, double fileProgress, double repoProgress)> RealityDataServiceTransfer_ProgressCallBack;

// ErrorCode --> Curl error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Curl error code:(0)Success (xx)Curl (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Curl English message.
typedef std::function<void(int index, void *pClient, int ErrorCode, const char* pMsg)> RealityDataServiceTransfer_StatusCallBack;

//! Callback function to follow the download progression.
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @return If RealityDataDownload_ProgressCallBack returns any other value The download is canceled for all files.
typedef std::function<int()> RealityDataServiceTransfer_HeartbeatCallBack;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A class used to create a new reality data in the reality data service.
//=====================================================================================
struct RealityDataServiceCreateRequest : public RealityDataUrl
    {
    REALITYDATAPLATFORM_EXPORT RealityDataServiceCreateRequest(Utf8String realityDataId, Utf8String properties);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to modify an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataServiceChangeRequest : public RealityDataUrl
    {
    REALITYDATAPLATFORM_EXPORT RealityDataServiceChangeRequest(Utf8String realityDataId, Utf8String properties);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to delete an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataDelete : public RealityDataByIdRequest
    {
    REALITYDATAPLATFORM_EXPORT RealityDataDelete(Utf8String realityDataId) : RealityDataByIdRequest(realityDataId) { m_requestType = HttpRequestType::DELETE_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to delete an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataDeleteFolder : public RealityDataFolderByIdRequest
    {
    REALITYDATAPLATFORM_EXPORT RealityDataDeleteFolder(Utf8String realityDataId) : RealityDataFolderByIdRequest(realityDataId) { m_requestType = HttpRequestType::DELETE_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to delete an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataDeleteDocument : public RealityDataDocumentByIdRequest
    {
    REALITYDATAPLATFORM_EXPORT RealityDataDeleteDocument(Utf8String realityDataId) : RealityDataDocumentByIdRequest(realityDataId) { m_requestType = HttpRequestType::DELETE_Request; }
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to create a relationship between an existing reality data and a project
//=====================================================================================
struct RealityDataRelationshipCreateRequest : public RealityDataUrl
    {
        REALITYDATAPLATFORM_EXPORT RealityDataRelationshipCreateRequest(Utf8String realityDataId, Utf8String projectId);
    protected:
        REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsimethod                                   Spencer.Mason 02/2017
//! The base class to upload/download classes. This class defines the interface
//! common to both upload and download to/from Reality Data Service
//=====================================================================================
struct RealityDataFileTransfer;

//where the curl upload ended, either in success or failure
struct TransferResult
    {
    int                     errorCode; //code returned by curl
    size_t                  progress; //a percentage of how much of the file was successfully downloaded
    time_t                  timeSpent;
    Utf8String              name;
    };

struct TransferReport
    {
    size_t                  packageId;
    bvector<TransferResult*>  results;
    ~TransferReport()
        {
        for(TransferResult* result : results)
            delete result;
        }

    REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR report);
    };

//=====================================================================================
//! @bsimethod                                   Spencer.Mason 02/2017
//! The base class to upload/download classes. This class defines the interface
//! common to both upload and download to/from Reality Data Service. It is the interface
//! that enables to set callback required to monitor the tranfer progress, it is also where
//! the path to the certificate file is set.
//=====================================================================================
struct RealityDataServiceTransfer : public CurlConstructor
    {
    REALITYDATAPLATFORM_EXPORT RealityDataServiceTransfer(){}

    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataServiceTransfer();

    //! Set proxy informations
    //REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) { m_proxyUrl = proxyUrl; m_proxyCreds = proxyCreds; };

    //! Set certificate path for https upload.
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(BeFileNameCR certificatePath) { m_certPath = certificatePath; }

    //! Set callback to follow progression of the upload.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataServiceTransfer_ProgressCallBack pi_func)
        {
        m_pProgressFunc = pi_func;
        }

    //! Set interval at which to send a progress callback. Default 1% (0.01)
    REALITYDATAPLATFORM_EXPORT void SetProgressStep(double step) { m_progressThreshold = m_progressStep = step; }

    //! Set callback to allow the user to mass cancel all uploads
    REALITYDATAPLATFORM_EXPORT void SetHeartbeatCallBack(RealityDataServiceTransfer_HeartbeatCallBack pi_func)
        {
        m_pHeartbeatFunc = pi_func;
        }

    //! Set callback to know to status, upload done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataServiceTransfer_StatusCallBack pi_func) { m_pStatusFunc = pi_func; }

    //! Start the upload progress for all links.
    REALITYDATAPLATFORM_EXPORT virtual TransferReport* Perform();

    REALITYDATAPLATFORM_EXPORT Utf8String GenerateAzureHandshakeUrl();

    REALITYDATAPLATFORM_EXPORT void OnlyReportErrors(bool onlyErrors) { m_onlyReportErrors = onlyErrors; }

    REALITYDATAPLATFORM_EXPORT int64_t GetTokenTimer() { return m_azureTokenTimer; }

    REALITYDATAPLATFORM_EXPORT virtual bool UpdateTransferAmount(int64_t transferedAmount);

protected:
    void SetupCurlforFile(RealityDataUrl* upload, bool verifyPeer);
    bool SetupNextEntry();
    void ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg);
    Utf8String GetAzureToken();

    AzureHandshake*             m_handshakeRequest;
    bvector<RealityDataFileTransfer*>         m_filesToTransfer;

    void*                       m_pCurlHandle;

    Utf8String                  m_id;
    Utf8String                  m_proxyUrl;
    Utf8String                  m_proxyCreds;
    BeFileName                  m_certPath;
    RealityDataServiceTransfer_ProgressCallBack m_pProgressFunc;
    double                      m_progressStep;
    double                      m_progress;
    double                      m_progressThreshold;
    RealityDataServiceTransfer_StatusCallBack m_pStatusFunc;
    RealityDataServiceTransfer_HeartbeatCallBack m_pHeartbeatFunc;

    Utf8String                  m_azureServer;
    Utf8String                  m_azureToken;
    bvector<Utf8String>         m_headers;

    TransferReport              m_report;
    size_t                      m_curEntry;
    int64_t                     m_azureTokenTimer;

    bool                        m_onlyReportErrors;
    uint64_t                    m_fullTransferSize;
    uint64_t                    m_currentTransferedAmount;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert                    12/2016
//! RealityDataServiceUpload
//! This class represents an upload service for uploading files or datasets to the
//!  reality data service.
//! During the perform the object will rely on CURL in a multithreaded environment 
//!  to upload sources up to the reality data service.
//! The process will attempt to optimise the upload process. To do so it may decide
//!  to group a set of files together in an archive then upload and un-archive the file set
//!  up in the Reality Data Service. If the files are large the upload process
//!  may split up the file and upload in fragments. It may also select to attempt 
//!  a SAS redirection to upload directly to the cloud blob.
//! In case of communication error the upload process will attempt retry to 
//!  complete the operation.
//! At the end of the process the upload will increment the RealityData instance fields
//!  concerning total size. Also note that when SetSourcePath is used a root document
//!  and a thumbnail may be specified. The appropriate RealityData instance fields
//!  will then be updated.
//! It will also start as many threads needed to optimise the process.
//! The present class offers services to upload a file including use of callback
//!  to indicate progress.
//! The service is used by specifying the source path ot the source file or files.
//! One and only one of SetSourcePath(), SetSourceFile() or SetSourceFiles()
//!  will be called.
//=====================================================================================
struct RealityDataServiceUpload : public RealityDataServiceTransfer
    {
    REALITYDATAPLATFORM_EXPORT static Utf8String PackageProperties(bmap<RealityDataField, Utf8String> properties);

    REALITYDATAPLATFORM_EXPORT RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, bool overwrite=false, bool listable = true, RealityDataServiceTransfer_StatusCallBack pi_func = nullptr);

    //! Set the source path which, all files and folders located in this path will be uploaded
    //!  to the designated reality data
    //! @param sourcePath indicates the source path that will be considered the base folder of the reality data.
    //!  all files and folders will recursively be scanned and uploaded.
    //! @param rootDocument The root document for the reality data or empty if the root document
    //!  must not be set or modified. This document is specified relative to the root source path.
    //! @param thumbnailDocument The thumbnail document. It must designate a JPG or PNG file providing a
    //!  a visual overview of the reality data.
	//! Keep in mind that you can upload 1 to many files
    REALITYDATAPLATFORM_EXPORT void SetSourcePath(Utf8String sourcePath, Utf8String rootDocument = "", Utf8String thumbnailDocument = "")
    {m_sourcePath = sourcePath; m_rootDocument = rootDocument; m_thumbnailDocument = thumbnailDocument;}

    //! Sets the RealityDataID that also designates the container to which the data is uploaded
    REALITYDATAPLATFORM_EXPORT void SetRealityDataId(Utf8String realityDataId) { m_id = realityDataId; }

    REALITYDATAPLATFORM_EXPORT bool IsValidUpload() { return m_filesToTransfer.size() > 0; }

protected:
    BentleyStatus CreateUpload(Utf8String properties);

private:
    Utf8String                  m_sourcePath;
    Utf8String                  m_rootDocument;
    Utf8String                  m_thumbnailDocument;
    bool                        m_overwrite;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  02/2017
//! RealityDataServiceDownload
//! This class represents a download service for downloading files or datasets from the
//!  Reality Data Service.
//! During the perform the object will rely on CURL in a multithreaded environment 
//!  to download sources down from the Reality Data Service.
//! The process will attempt to optimise the download process. To do so it may decide
//!  if the files are large the download process
//!  may split up the file and download in fragments. It may also select to attempt 
//!  a SAS redirection to upload directly to the cloud blob (prefered option).
//! In case of communication error the download process will attempt retry to 
//!  complete the operation.
//! It will also start as many threads needed to optimise the process.
//! The present class offers services to upload a file including use of callback
//!  to indicate progress.
//=====================================================================================
struct RealityDataServiceDownload : public RealityDataServiceTransfer
    {
    REALITYDATAPLATFORM_EXPORT RealityDataServiceDownload(BeFileName targetLocation, Utf8String serverId);

    REALITYDATAPLATFORM_EXPORT RealityDataServiceDownload(Utf8String serverId, bvector<RealityDataFileTransfer*> downloadList);
    
private:

    void DownloadFullRepo(BeFileName targetLocation, Utf8String id);

    void DownloadFromNavNode(BeFileName targetLocation, Utf8String id);
    };


//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataService
//! This class represents the RealityData Service proper. This unique class
//! provides functionalty to query or upload to the RealityData Service by basing
//! command upon requests objects.
//! The methods can return properly formatted SpatialEntity objects or the
//! raw JSON or byte-stream resulting from the request.
//! Note that although almost any type of query can be created here some will
//! result in an unauthorized error if rights and permissions do not allow the
//! operation to be performed.
//! Aside the global configuration of the service which is gloablly applied, the 
//! class is stateless and thus threadsafe. Note that all methods are static.
//=====================================================================================
struct RealityDataService
    {
public:
    //!
    //! The SetServerComponents static method enables to set the RealityData Service URL REST API component strings
    //! The server parameter contains the name of the server including the communication protocol. The default value is
    //! https://connect-realitydataservices.bentley.com/ 
    //! The WSGProtocol is a string containing the WSG version number. Default is '2.4'
    //! name is the name of the WSG service for the RealityData Service. It should always be "IndexECPlugin-Server"
    //! schemaName is the name of the schema exposing the RealityData Service classes. Default is "RealityModeling"
    //! All fields must be provided if used. Normally the present method shold only be used for development purposes
    //! When accessing one of the dev or qa version of RealityData Service.
    REALITYDATAPLATFORM_EXPORT static void SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8StringCR certificatePath = "")
        {
        BeAssert(server.size() != 0);
        BeAssert(WSGProtocol.size() != 0);
        BeAssert(repoName.size() != 0);
        BeAssert(schemaName.size() != 0);

        s_realityDataServer = server;
        s_realityDataWSGProtocol = WSGProtocol;
        s_realityDataRepoName = repoName;
        s_realityDataSchemaName = schemaName;

        if(certificatePath.size() == 0)
            s_verifyPeer = false;
        else
            s_verifyPeer = true;
        s_realityDataCertificatePath = certificatePath;
        s_initializedParams = true;
        }

    //! Returns the current name of the server
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetServerName();

    //! Results the string containing the WSG protocol version number
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetWSGProtocol();

    //! Returns the name of the WSG repository containing the RealityData Service objects
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetRepoName();

    //! Returns the name of the schema defining the classes exposed by the RealityData Service.
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetSchemaName();

    //! Returns the name of the schema defining the classes exposed by the RealityData Service.
    REALITYDATAPLATFORM_EXPORT static const bool GetVerifyPeer();

    //! Returns the name of the schema defining the classes exposed by the RealityData Service.
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetCertificatePath();

    //! Validates if server parameters have been set
    REALITYDATAPLATFORM_EXPORT static const bool AreParametersSet();

    //! Returns a list of RealityData objects that overlap the given region
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(const RealityDataPagedRequest& request, RequestStatus& status);

    //! Returns the size in KB for the specify Enterprise, or the default one.
    REALITYDATAPLATFORM_EXPORT static void RealityDataService::Request(const RealityDataEnterpriseStatRequest& request, uint64_t* pNbRealityData, uint64_t* pTotalSizeKB, RequestStatus& status);

    //! Returns the list of all documents in a repo
    REALITYDATAPLATFORM_EXPORT static bvector<bpair<WString, uint64_t>> Request(const AllRealityDataByRootId& request, RequestStatus& status);

    //! Returns the RealityData object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Request(const RealityDataByIdRequest& request, RequestStatus& status);

    //! Returns a RealityDataDocument or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataDocumentPtr Request(const RealityDataDocumentByIdRequest& request, RequestStatus& status);

    //! Returns the content of a RealityData Service document
    REALITYDATAPLATFORM_EXPORT static void Request(RealityDataDocumentContentByIdRequest& request, FILE* file, RequestStatus& status);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataFolderPtr Request(const RealityDataFolderByIdRequest& request, RequestStatus& status);

    //! Returns a list of RealityData objects that belongs to the enterprise.
    //! Notice that the enterprise is not usually provided and the enterprise of the currently
    //! Bentley CONNECT user is used.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(const RealityDataListByEnterprisePagedRequest& request, RequestStatus& status);

    //! Returns a list of RealityDataProjectRelation objects for a specific project.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> Request(const RealityDataProjectRelationshipByProjectIdRequest& request, RequestStatus& status);

    //! Returns a list of RealityDataProjectRelation objects for a specific RealityData.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> Request(const RealityDataProjectRelationshipByRealityDataIdRequest& request, RequestStatus& status);

    //! Returns a list of RealityDataProjectRelation objects for a specific project.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> Request(const RealityDataProjectRelationshipByProjectIdPagedRequest& request, RequestStatus& status);

    //! Returns a list of RealityDataProjectRelation objects for a specific RealityData.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> Request(const RealityDataProjectRelationshipByRealityDataIdPagedRequest& request, RequestStatus& status);

    //! Returns a serverResponse or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataServiceChangeRequest& request, RequestStatus& status);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataServiceCreateRequest& request, RequestStatus& status);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataRelationshipCreateRequest& request, RequestStatus& status);

    //! Returns the full WSG JSON returned by the request
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static RequestStatus PagedRequestToJSON(const RealityDataPagedRequest* request, Utf8StringR jsonResponse, Utf8String keyword = "instances");

    //! Returns the full WSG JSON returned by the Reality Data request
    REALITYDATAPLATFORM_EXPORT static RequestStatus RequestToJSON(const RealityDataUrl* request, Utf8StringR jsonResponse, Utf8String keyword = "instances");

private:
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> _RequestRelationship(const RealityDataUrl* request, RequestStatus& status);
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationshipPtr> _RequestPagedRelationships(const RealityDataPagedRequest* request, RequestStatus& status);

    static Utf8String s_realityDataServer;
    static Utf8String s_realityDataWSGProtocol;
    static Utf8String s_realityDataRepoName;
    static Utf8String s_realityDataSchemaName;
    static bool s_verifyPeer;
    static Utf8String s_realityDataCertificatePath;
    static bool s_initializedParams;

    static const Utf8String s_ImageryKey;
    static const Utf8String s_TerrainKey;
    static const Utf8String s_ModelKey;
    static const Utf8String s_PinnedKey;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE