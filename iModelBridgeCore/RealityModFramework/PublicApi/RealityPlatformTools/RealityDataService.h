/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatformTools/RealityDataService.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatform/RealityDataObjects.h>

#include <Bentley/BeFile.h>
#include <Bentley/BeFilename.h>
#include <Bentley/DateTime.h>

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
    REALITYDATAPLATFORM_EXPORT virtual void EncodeId() const override;
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert         05/2018
//! RealityDataLocationRequest
//! This class returns the description of specified data location
//=====================================================================================
struct RealityDataLocationRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataLocationRequest(Utf8StringCR dataLocationId) { m_validRequestString = false; m_id = dataLocationId;}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataLocationRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert         05/2018
//! AllDataLocationsRequest
//! Request for all available data locations
//=====================================================================================
struct AllRealityDataLocationsRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT AllRealityDataLocationsRequest() { m_validRequestString = false; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
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
    REALITYDATAPLATFORM_EXPORT RealityDataEnterpriseStatRequest(Utf8StringCR ultimateId, DateTime date = DateTime::GetCurrentTimeUtc()) { m_validRequestString = false; m_id = ultimateId; m_date = date;}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataEnterpriseStatRequest() {}
    DateTime m_date;
    };

//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataEnterpriseStat
//! This class returns the size in KB currently used.
//=====================================================================================
struct RealityDataAllEnterpriseStatsRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataAllEnterpriseStatsRequest(DateTime date) 
        { 
        m_validRequestString = false; 
        m_date = date;
        }
    REALITYDATAPLATFORM_EXPORT RealityDataAllEnterpriseStatsRequest() 
        {
        m_validRequestString = false; 
        m_date = DateTime::GetCurrentTimeUtc();
        }
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:

    DateTime m_date;
    };
	
//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataServiceStat
//! This class returns the size in KB currently used.
//=====================================================================================
struct RealityDataServiceStatRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataServiceStatRequest(Utf8StringCR ultimateId, DateTime date = DateTime::GetCurrentTimeUtc()) { m_validRequestString = false; m_id = ultimateId; m_date = date;}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataServiceStatRequest() {}
    DateTime m_date;
    };

//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataEnterpriseStat
//! This class returns the size in KB currently used.
//=====================================================================================
struct RealityDataAllServiceStatsRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataAllServiceStatsRequest(DateTime date) 
        { 
        m_validRequestString = false; 
        m_date = date;
        }
    REALITYDATAPLATFORM_EXPORT RealityDataAllServiceStatsRequest() 
        {
        m_validRequestString = false; 
        m_date = DateTime::GetCurrentTimeUtc();
        }
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:

    DateTime m_date;
    };	
	
//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataUserStat
//! This class returns the user stats  currently used.
//=====================================================================================
struct RealityDataUserStatRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataUserStatRequest(Utf8StringCR ultimateId, DateTime date = DateTime::GetCurrentTimeUtc()) { m_validRequestString = false; m_id = ultimateId; m_date = date;}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataUserStatRequest() {}
    DateTime m_date;
    };

//=====================================================================================
//! @bsiclass                                         Donald.Morissette         03/2017
//! RealityDataAllUserStatsRequest
//! This class returns the size in KB currently used.
//=====================================================================================
struct RealityDataAllUserStatsRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataAllUserStatsRequest(DateTime date) 
        { 
        m_validRequestString = false; 
        m_date = date;
        }
    REALITYDATAPLATFORM_EXPORT RealityDataAllUserStatsRequest() 
        {
        m_validRequestString = false; 
        m_date = DateTime::GetCurrentTimeUtc();
        }
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:

    DateTime m_date;
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
//!  model definition (RealityData, RealityDataRelationship, Folder, and Document).
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
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8StringCR identifier) 
        {
        m_validRequestString = false; 
        m_id = identifier; 
        }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataByIdRequest(){}
   
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataByIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                 Spencer.Mason                     12/2017
//! RealityDataExtendedByIdRequest
//! This class represents a request for specific RealityDataExtended class object.
//=====================================================================================
struct RealityDataExtendedByIdRequest : public RealityDataUrl
    {
public:
    // Only identifier is required to retreive RealityData
    REALITYDATAPLATFORM_EXPORT RealityDataExtendedByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataExtendedByIdRequest() {}

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataExtendedByIdRequest() {}
    };
	
//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataRelationshipByProjectIdRequest
//! Requests all relationships for a given ProjectId
//=====================================================================================
struct RealityDataRelationshipByProjectIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataRelationshipByProjectIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataRelationshipByProjectIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataRelationshipByRealityDataIdRequest
//! Requests all relationships for a give Reality Data
//=====================================================================================
struct RealityDataRelationshipByRealityDataIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataRelationshipByRealityDataIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false;
        m_id = identifier; 
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataRelationshipByRealityDataIdRequest() {}
    };


//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataFolderByIdRequest
//! This class represents a request for specific RealityDataFolder class object.
//=====================================================================================
struct RealityDataFolderByIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataFolderByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
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
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentByIdRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataDocumentByIdRequest(){}
	
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataDocumentByIdRequest() {}
    };

//=====================================================================================
//! @bsiclass                                         Spencer.Mason            12/2016
//! AzureHandshake
//! Requests the direct access url in the azure blob for a specified RealityData or 
//! document. Also provides methods to parse the response received
//=====================================================================================
struct AzureHandshake : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT AzureHandshake(Utf8String sourcePath, bool isWrite);
    REALITYDATAPLATFORM_EXPORT AzureHandshake();

    //! Parses the json response received from the server and extracts the server's URL,
    //! the azure token, and the a system timestamp for when the token should be renewed
    REALITYDATAPLATFORM_EXPORT BentleyStatus ParseResponse(Utf8StringCR jsonresponse, Utf8StringR azureServer, Utf8StringR azureToken, int64_t& tokenTimer);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
private:
    bool       m_isWrite;
    };


//=====================================================================================
//! @bsiclass                                         Alain.Robert              12/2016
//! RealityDataDocumentContentByIdRequest
//! This class represents a download attempt of a specific file.
//! The present class provides services for the support of azure redirection to blob.
//! The RealityDataService can query the class to check if azure redirection is possible
//! or not. It will automatically attempt to download the file directly from Azure, unless
//! the user specifies not to do so, using SetAzureRedirectionPossible(false);
//! Example:
//! RealityDataDocumentContentByIdRequest myRequest("0586-358df-445-de34a-dd286/RootDocument.3mx");
//=====================================================================================
struct RealityDataDocumentContentByIdRequest : public RealityDataUrl
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8StringCR identifier) : 
        m_handshakeRequest(nullptr), m_allowAzureRedirection(true), m_AzureRedirected(false),
        m_azureTokenTimer(-1)
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }
    
    //REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8CP identifier) : m_identifier(identifier) {}
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(const RealityDataDocumentContentByIdRequest &object); 

    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataDocumentContentByIdRequest(){}
	
    //! This call modify the indentifier of the object. Since we want a 
	//!  different ressource. This can be a folder, document or anything
	//!  else. 
    REALITYDATAPLATFORM_EXPORT void ChangeInstanceId(Utf8String instanceId);

    //! This call creates the URL request to obtain the azure redirection URL.
    REALITYDATAPLATFORM_EXPORT RawServerResponse GetAzureRedirectionRequestUrl() const;

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
    REALITYDATAPLATFORM_EXPORT virtual void EncodeId() const override;

    mutable AzureHandshake*             m_handshakeRequest;

    mutable Utf8String  m_azureServer;
    mutable Utf8String  m_azureToken;
    mutable bool        m_AzureRedirected;
    mutable bool        m_allowAzureRedirection;
    mutable int64_t     m_azureTokenTimer;
    RealityDataDocumentContentByIdRequest() {}

    };

struct RealityDataFilterCreator; // forward decl

//=====================================================================================
//! @bsiclass                                Spencer.Mason                     04/2017
//! RDSFilter
//! Struct exists only to impose specific usage on client code. Due to a quirk in WSG
//! UriEncoding must only be applied to specific parts of an Url. To ensure this, filters
//! must percolate through the RealityDataFilterCreator
//=====================================================================================
struct RDSFilter
    {
friend struct RealityDataFilterCreator;
public:
    REALITYDATAPLATFORM_EXPORT Utf8StringCR ToString() const { return m_filter; }
private:
    RDSFilter(Utf8StringCR filter) : m_filter(filter){}

    Utf8String m_filter;
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
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByName(Utf8String name);

    //! Sets filtering upon the classification. The classification may contain
    //!  more than one classification by bitwise oring the classification
    //!  values.
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByClassification(RealityDataBase::Classification classification);

    //! Filters the returned set by the reality data size.
    //! Both the min and max size must be specified
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterBySize(uint64_t minSize, uint64_t maxSize);

    //! Sets a spatial filter. Only RealityData for which the footprint overlaps (even
    //! partially) the given region will be selected.
    //! The area provided is a list of geo points (longitude/latitude)
    //!  that must form a closed area. The last point of the list must
    //!  be equal to the first point.
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterSpatial(bvector<GeoPoint2d> area, uint64_t coordSys);

    //! Filters the list by owner. Only reality data belonging to given owner
    //!  will be returned. The owner is specified by the email address
    //!  and is case insensitive.
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByOwner(Utf8String owner);

    //! Filters the list by creation date. To indicate either min or max date
    //!  are unbounded simply provide an invalid/unset DataTime object
    //!  If both dates are invalid/unset then the command will return an error
    //!  and no filtering will be set.
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByCreationDate(DateTime minDate, DateTime maxDate);

    //! Filters the list by last accessed date. To indicate either min or max date
    //!  are unbounded simply provide an invalid/unset DataTime object.
    //! If both dates are invalid/unset then the command will return an error
    //!  and no filtering will be set.  
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByAccessDate(DateTime minDate, DateTime maxDate);

    //! Filters the list by modification date. To indicate either min or max date
    //!  are unbounded simply provide an invalid/unset DataTime object.
    //! If both dates are invalid/unset then the command will return an error
    //!  and no filtering will be set.  
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByModificationDate(DateTime minDate, DateTime maxDate);

    //! Filters in or out public data as specified
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterVisibility(RealityDataBase::Visibility visibility);
        
    //! Filter by resolution. As resolution may be confusing since minimum resolution is
    //!  expressed a higher number the resolution can be specified in any order and
    //!  internally the resolution will be applied accordingly.
    //! Reality data that have no resolution set will be considered 'unspecified' and
    //!  will be returned whatever the resolution bracket given if filterOutUnspecified is false
    //!  and will be discarded if true
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByResolution(double resMin, double resMax, bool filterOutUnspecified);

    //! Filter by accuracy. As accuracy may be confusing since minimum accuracy is
    //!  expressed a higher number the accuracy can be specified in any order and
    //!  internally the accuracy will be applied accordingly.    
    //! Reality data that have no accuracy set will be considered 'unspecified' and
    //!  will be returned whatever the bracket given if filterOutUnspecified is false
    //!  and will be discarded if true
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified);

    //! Filter by type. The type is specified by a string in the reality data.
    //! The filter type specification here can contain many types
    //!  separated by semi-colons. All reality data of any of the specified types
    //!  will be returned in the list.
    //!  types are case insensitive
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByType(Utf8String types);

    //! Filter by dataset. Only reality data of specified dataset will be returned
    //!  note that Dataset names are case-sensitive.
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByDataset(Utf8String dataset);

    //! Sets filtering upon the group. 
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterByGroup(Utf8String group);

    //! Filter relationship by RealityDataId. Only relationships for specified RealityDataId will be returned
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterRelationshipByRealityDataId(Utf8String realityDataId);

    //! Filter relationship by ProjectId. Only relationships for specified ProjectId will be returned
    REALITYDATAPLATFORM_EXPORT static RDSFilter FilterRelationshipByProjectId(Utf8String projectId);
    
    //! Groups all filters inside of parentheses, all criteria must be met ( && )
    REALITYDATAPLATFORM_EXPORT static RDSFilter GroupFiltersAND(bvector<RDSFilter> filters);

    //! Groups all filters inside of parentheses, only one of the criteria must be met ( || )
    REALITYDATAPLATFORM_EXPORT static RDSFilter GroupFiltersOR(bvector<RDSFilter> filters);
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

    REALITYDATAPLATFORM_EXPORT RealityDataPagedRequest() : m_informationSourceFilteringSet(false) 
        { 
        m_validRequestString = false; 
        m_requestType = HttpRequestType::GET_Request; 
        m_sort = false; 
        }

    REALITYDATAPLATFORM_EXPORT void SetFilter(const RDSFilter& filter);
    REALITYDATAPLATFORM_EXPORT void SetQuery(Utf8StringCR query);
    REALITYDATAPLATFORM_EXPORT void SetProject(Utf8StringCR project);

    //! Sets the sort order for the list. This sorting is performed server-side.
    //! Note that it is not possible to specify two sorts (sort by field a then by filed b is not supported).
    //! The server will decide how sorted groups are ordered.
    //! Note that some fields in the server are considered case-sensitive and others
    //!  case insensitive. The server will apply sort rules accordingly.
    REALITYDATAPLATFORM_EXPORT void SortBy(RealityDataField, bool ascending);

protected:
    REALITYDATAPLATFORM_EXPORT void _PrepareBaseRequestString() const;
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    REALITYDATAPLATFORM_EXPORT virtual void EncodeId() const override;

    int m_informationSourceFilter;
    bool m_informationSourceFilteringSet;
    Utf8String m_order;
    Utf8String m_filter;
    Utf8String m_query;
    Utf8String m_sort;
    Utf8String m_project;
    };


//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! DEPRECATED use RealityDataListByUltimateIdPagedRequest
//=====================================================================================
struct RealityDataListByOrganizationPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataListByOrganizationPagedRequest(Utf8StringCR identifier = "", uint16_t startIndex = 0, uint16_t pageSize = 25) 
    { assert(0 && "This function is deprecated, please use RealityDataListByUltimateIdPagedRequest"); }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };


//=====================================================================================
//! @bsiclass                                   Spencer.Mason 08/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! for specific ultimateId. Usually a CONNECT user only has access to its own organization 
//! data only so the ultimateId specified should be the identifier of its organization.
//! This request will not return public references to reality data from other organizations
//! marked as public.
//! Note that the present request will only return Reality Data part of an organization
//! for which the current CONNECT user has access to. 
//=====================================================================================
struct RealityDataListByUltimateIdPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataListByUltimateIdPagedRequest(Utf8StringCR identifier = "", uint16_t startIndex = 0, uint16_t pageSize = 25) { m_validRequestString = false; m_id = identifier; m_startIndex = startIndex; m_pageSize = pageSize; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! explicitly linked to a specific CONNECT Project through the Reality Data Service
//! RealityData/Project registry it maintains. 
//=====================================================================================
struct RealityDataRelationshipByProjectIdPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataRelationshipByProjectIdPagedRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier;
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataRelationshipByProjectIdPagedRequest() {}
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A specialisation of a RealityDataPagedRequest that only obtains reality data
//! explicitly linked to a specific CONNECT Project through the Reality Data Service
//! RealityData/Project registry it maintains. 
//=====================================================================================
struct RealityDataRelationshipByRealityDataIdPagedRequest : public RealityDataPagedRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataRelationshipByRealityDataIdPagedRequest(Utf8StringCR identifier) 
        { 
        m_validRequestString = false; 
        m_id = identifier; 
        }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataRelationshipByRealityDataIdPagedRequest() {}
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! AllRealityDataByRootId
//! A request for a list of all documents in a repository
//=====================================================================================
struct AllRealityDataByRootId : public RealityDataDocumentContentByIdRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT AllRealityDataByRootId(Utf8StringCR rootId); 

    REALITYDATAPLATFORM_EXPORT void SetMarker(Utf8String marker) const 
        { 
        m_validRequestString = false; 
        m_marker = marker; 
        }

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFilter() const { return m_filter; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    mutable Utf8String  m_marker;
    Utf8String          m_filter;
    AllRealityDataByRootId() {}
    };

//=====================================================================================
//! @bsimethod                          Spencer.Mason                          02/2017
//! The following are the declarations for callbacks that can be used with the
//! RealityData uploader or downloader.
//=====================================================================================

//! Callback function to follow the download progression.
//! @param[in] filename    name of the file. 
//! @param[in] progress    Percentage uploaded.
typedef std::function<void(Utf8String filename, double fileProgress, double repoProgress)> RealityDataServiceTransfer_ProgressCallBack;

// ErrorCode --> Tool error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Tool error code:(0)Success (xx)Tool error (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Tool English message.
typedef std::function<void(int index, void *pClient, int ErrorCode, const char* pMsg)> RealityDataServiceTransfer_StatusCallBack;

//! Callback function to follow the download progression.
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @return If RealityDataDownload_ProgressCallBack returns any other value The download is canceled for all files.
typedef std::function<int()> RealityDataServiceTransfer_HeartbeatCallBack;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 02/2017
//! A class used to create a new reality data in the reality data service.
//=====================================================================================
struct RealityDataCreateRequest : public RealityDataUrl
    {
    REALITYDATAPLATFORM_EXPORT RealityDataCreateRequest(Utf8String realityDataId, Utf8String properties);
    REALITYDATAPLATFORM_EXPORT RealityDataCreateRequest(RealityDataCR realityData);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to modify an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataChangeRequest : public RealityDataUrl
    {
    REALITYDATAPLATFORM_EXPORT RealityDataChangeRequest(Utf8String realityDataId, Utf8String properties);
    REALITYDATAPLATFORM_EXPORT RealityDataChangeRequest(RealityDataCR realityData);
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason 03/2017
//! A class used to delete an existing reality data in the reality data service.
//=====================================================================================
struct RealityDataDelete : public RealityDataUrl
    {
    REALITYDATAPLATFORM_EXPORT RealityDataDelete(Utf8String realityDataId) 
        { 
        m_validRequestString = false; 
        m_id = realityDataId; 
        m_requestType = HttpRequestType::DELETE_Request; 
        }
    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataDelete(){}
protected:
    REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataDelete() {}
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
//! @bsiclass                                   Alain.Robert 03/2017
//! A class used to delete a relationship between an existing reality data and a project
//=====================================================================================
struct RealityDataRelationshipDelete : public RealityDataUrl
    {
        REALITYDATAPLATFORM_EXPORT RealityDataRelationshipDelete(Utf8String realityDataId, Utf8String projectId);
    protected:
        Utf8String m_projectId;
        REALITYDATAPLATFORM_EXPORT virtual void _PrepareHttpRequestStringAndPayload() const override;
    };


//=====================================================================================
//! @bsimethod                                   Spencer.Mason          02/2017
//! The base class to upload/download classes. This class defines the interface
//! common to both upload and download to/from Reality Data Service
//=====================================================================================
struct RealityDataFileTransfer;

//=====================================================================================
//! @bsimethod                                   Spencer.Mason          02/2017
//! TransferResult
//! struct that stores information relative to the success or failure of the transfer
//=====================================================================================
struct TransferResult
    {
    int                     errorCode; //code returned by the REST Tool
    size_t                  progress; //a percentage of how much of the file was successfully downloaded
    time_t                  timeSpent;
    Utf8String              name;
    RawServerResponse       response;
    };

//=====================================================================================
//! @bsimethod                                   Spencer.Mason          02/2017
//! TransferReport
//! struct that stores multiple TransferResult objects and can be used to write their
//! Contents to an XML string
//=====================================================================================
struct TransferReport
    {
    bvector<TransferResult*>  results;
    mutable bmap<Utf8String, bool>    transferSuccessMap;
    ~TransferReport()
        {
        for(TransferResult* result : results)
            delete result;
        }

    //! Returns whether every file has been properly transfered
    REALITYDATAPLATFORM_EXPORT bool AllTransferedSuccessfully() const;

    //! Returns the list of failed transfers
    REALITYDATAPLATFORM_EXPORT bvector<Utf8String> GetFailedTransferList() const;

    //! Writes the contents of each TranferResult as an attribute of and XML string
    REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR report) const;
    };


//=====================================================================================
//! @bsimethod                                   Alain.Robert          06/2018
//! TransferError
//! struct that stores an error code and error message related to operation
//! of the RealityDataServiceTransfer structure. This error is usually related
//! to Upload or download creation or set-up prior to starting the transfer.
//=====================================================================================
struct TransferError
{
    TransferError::TransferError(): m_errorCode((int)BentleyStatus::SUCCESS) {}
    enum class TransferErrorOrigin
    {
         RDS_SERVICE,
         OTHER
    };

    TransferErrorOrigin  m_errorOrigin;
    int                  m_errorCode;
    Utf8String           m_errorContext;
    Utf8String           m_errorMessage;
};

inline bool operator==(int errorCode, const TransferError& transferError) {return transferError.m_errorCode == errorCode;}
inline bool operator!=(int errorCode, const TransferError& transferError) {return transferError.m_errorCode != errorCode;}
inline bool operator==(BentleyStatus errorCode, const TransferError& transferError) {return transferError.m_errorCode == (int)errorCode;}
inline bool operator!=(BentleyStatus errorCode, const TransferError& transferError) {return transferError.m_errorCode != (int)errorCode;}

inline bool operator==(const TransferError& transferError, int errorCode) {return transferError.m_errorCode == errorCode;}
inline bool operator!=(const TransferError& transferError, int errorCode) {return transferError.m_errorCode != errorCode;}
inline bool operator==(const TransferError& transferError, BentleyStatus errorCode) {return transferError.m_errorCode == (int)errorCode;}
inline bool operator!=(const TransferError& transferError, BentleyStatus errorCode) {return transferError.m_errorCode != (int)errorCode;}

//! Callback function to prep a request, if necessary
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @param[in] request          Structure containing the url/header/body of the request
//! @param[in] verifyPeer       determines whether the tool should validate the server's certificate
typedef std::function<void(RealityDataUrl* request, bool verifyPeer)> RealityDataServiceTransfer_SetupCallback;

//! Callback function to prep a request, if necessary
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @param[in] filesToTransfer  Structures containing the url/header/body of the requests to execute
//! @param[in] progressFunc     Progress callback function to update, during download
typedef std::function<void(bvector<RealityDataFileTransfer*>& filesToTransfer, RealityDataServiceTransfer_ProgressCallBack& progressFunc)> RealityDataServiceTransfer_MultiRequestCallback;

//=====================================================================================
//! @bsimethod                                   Spencer.Mason 02/2017
//! The base class to upload/download classes. This class defines the interface
//! common to both upload and download to/from Reality Data Service. It is the interface
//! that enables to set callback required to monitor the tranfer progress, it is also where
//! the path to the certificate file is set.
//=====================================================================================
struct RealityDataServiceTransfer : public RequestConstructor
    {
    REALITYDATAPLATFORM_EXPORT RealityDataServiceTransfer() : RequestConstructor() {}

    REALITYDATAPLATFORM_EXPORT virtual ~RealityDataServiceTransfer();

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

    REALITYDATAPLATFORM_EXPORT void SetSetupCallback(RealityDataServiceTransfer_SetupCallback pi_func) { m_setupCallback = pi_func; }

    REALITYDATAPLATFORM_EXPORT void SetMultiRequestCallback(RealityDataServiceTransfer_MultiRequestCallback pi_func) { m_multiRequestCallback = pi_func; }

    //! Start the upload progress for all links.
    //! Returns a reference to the internal transfer report structure.
    REALITYDATAPLATFORM_EXPORT virtual const TransferReport& Perform();

    //! Returns the current TransferReport. 
    //! Normally this io only performed before calling Perfrom() as the Perform()
    //! call clears the report. This can be used to extract the error status
    //! when IsTransferValid() report false.
    REALITYDATAPLATFORM_EXPORT virtual const TransferError& GetError() {return m_creationError;}
    
    //! Specifies if the the transfer report will track every operation or only those that failed
    REALITYDATAPLATFORM_EXPORT void OnlyReportErrors(bool onlyErrors) { m_onlyReportErrors = onlyErrors; }

    //! Returns the system time for when the azure token should be renewed
    REALITYDATAPLATFORM_EXPORT int64_t GetTokenTimer() { return m_azureTokenTimer; }

    //! Validates that files have been found in the paths provided by the user
    REALITYDATAPLATFORM_EXPORT bool IsValidTransfer() { return m_filesToTransfer.size() > 0; }

    //! Returns files being uploaded
    REALITYDATAPLATFORM_EXPORT size_t GetFileCount() { return m_filesToTransfer.size(); }

    //! Returns full size of upload
    REALITYDATAPLATFORM_EXPORT uint64_t GetFullTransferSize() { return m_fullTransferSize; }

    //! Gets the RealityDataID that designates the container with which the data is transfered
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRealityDataId() { return m_id; }

protected:
    REALITYDATAPLATFORM_EXPORT virtual bool UpdateTransferAmount(int64_t transferedAmount);
    void SetupRequestforFile(RealityDataUrl* upload, bool verifyPeer);
    bool SetupNextEntry();
    void ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg);
    REALITYDATAPLATFORM_EXPORT virtual Utf8String GetAzureToken();
    void InitTool();

    AzureHandshake*             m_handshakeRequest;
    bvector<RealityDataFileTransfer*>         m_filesToTransfer;

    void*                       m_pRequestHandle;

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
    TransferError               m_creationError;
    size_t                      m_curEntry;
    int64_t                     m_azureTokenTimer;

    bool                        m_onlyReportErrors;
    uint64_t                    m_fullTransferSize;
    uint64_t                    m_currentTransferedAmount;

    RealityDataServiceTransfer_SetupCallback        m_setupCallback;
    RealityDataServiceTransfer_MultiRequestCallback m_multiRequestCallback;
    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert                    12/2016
//! RealityDataServiceUpload
//! This class represents an upload service for uploading files or datasets to the
//!  reality data service.
//! During the perform the object will rely on the tool in a multithreaded environment 
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
//=====================================================================================
struct RealityDataServiceUpload : public RealityDataServiceTransfer
    {
    //! Converts a map or propety name - property value pair into a JSON property string.
    //! The properties given must only include text types. Boolean types cannot be provided since the property value
    //! is not text.
    //! Consider using RealityConversionTools::RealityDataToJson() instead which will handle all properties correctly.
    REALITYDATAPLATFORM_EXPORT static Utf8String PackageProperties(bmap<RealityDataField, Utf8String> properties);

    //! Constructor
    //! @param uploadPath indicates the source path that will be considered the base folder of the reality data.
    //!  all files and folders will recursively be scanned and uploaded.
    //! Keep in mind that you can upload 1 or many files at any level in a new or existing RDS entry
    //! @param id indicates the Reality Data GUID the folder will be uploaded to. If you provide an empty string,
    //! RealityDataServiceUpload will attempt to create a new RDS entry and use the GUID generated by the server
    //! @param properties all relevant properties (including the root document, etc) to be stored in the RDS entry.
    //!  Property string must be produced by the PackageProperties function to ensure functionality
    //! @param overwrite tells the RDSUpload, if it encounters an entry with the same GUID on the server, whether
    //!  it can overwrite that entry or if it should abort the process
	//! @param listable is a shortcut to allow user code to tell RDSUpload whether the created RDS entry should 
    //!  be listable or not. Alternatively, this can be set in the properties parameter
    //! @param pi_func is a function pointer to a callback that will handle status messages
    //! @param colorList either represents a blackList: 
    //!  files specified in this list will not be uploaded to the server
    //!  or a whiteList: only files specified in this list will be uploaded to the server
    //! @param isBlackList if true, colorList will be interpreted as a blackList, if false colorList is a whiteList
    REALITYDATAPLATFORM_EXPORT RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, bool overwrite=false, bool listable = true, RealityDataServiceTransfer_StatusCallBack pi_func = nullptr, bvector<BeFileName> colorList = bvector<BeFileName>(), bool isBlackList = true, Utf8String proxyUrl = "", Utf8String proxyCreds = "");

    //! Alternate Constructor with a lighter signature
    //! refer to main constructor for parameter explanation
    REALITYDATAPLATFORM_EXPORT RealityDataServiceUpload(BeFileName uploadPath, Utf8String properties, bool overwrite = false, RealityDataServiceTransfer_StatusCallBack pi_func = nullptr, bvector<BeFileName> colorList = bvector<BeFileName>(), bool isBlackList = true, Utf8String proxyUrl = "", Utf8String proxyCreds = "");
protected:
    BentleyStatus CreateUpload(Utf8String properties);
    Utf8String GetAzureToken() override;
    Utf8String GetGuidFromId(Utf8String id);

private:
    bool                        m_overwrite;
    Utf8String                  m_serverPath;
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  02/2017
//! RealityDataServiceDownload
//! This class represents a download service for downloading files or datasets from the
//!  Reality Data Service.
//! During the perform the object will rely on the tool in a multithreaded environment 
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
    REALITYDATAPLATFORM_EXPORT RealityDataServiceDownload(BeFileName targetLocation, Utf8String serverId, RealityDataServiceTransfer_StatusCallBack pi_func = nullptr, Utf8String proxyUrl = "", Utf8String proxyCreds = "");

    REALITYDATAPLATFORM_EXPORT RealityDataServiceDownload(Utf8String serverId, bvector<RealityDataFileTransfer*> downloadList, RealityDataServiceTransfer_StatusCallBack pi_func = nullptr, Utf8String proxyUrl = "", Utf8String proxyCreds = "");

private:

    void DownloadFullRepo(BeFileName targetLocation, Utf8String id);

    void DownloadFromNavNode(BeFileName targetLocation, Utf8String id);
    };


//! Callback function to surface RequestErrors.
//! @param[in] basicMessage Utf8String provided by the specific request
//! @param[in] rawResponse  the raw server response
typedef std::function<void(Utf8String basicMessage, const RawServerResponse& rawResponse)> RealityDataService_ErrorCallBack;

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
    //! The server parameter contains the name of the server including the communication protocol. 
    //! The WSGProtocol is a string containing the WSG version number. 
    //! name is the name of the WSG service for the RealityData Service. It should always be "IndexECPlugin-Server"
    //! schemaName is the name of the schema exposing the RealityData Service classes. Default is "RealityModeling"
    //! All fields must be provided if used. Normally the present method should only be used for development purposes
    //! When accessing one of the dev or qa version of RealityData Service.
    REALITYDATAPLATFORM_EXPORT static void SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8StringCR certificatePath = "");
    REALITYDATAPLATFORM_EXPORT static void SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR repoName, Utf8StringCR schemaName, Utf8StringCR certificatePath, Utf8StringCR projectId);

    REALITYDATAPLATFORM_EXPORT static void SetProjectId(Utf8StringCR projectId);

    REALITYDATAPLATFORM_EXPORT static void SetErrorCallback(RealityDataService_ErrorCallBack errorCallback);

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

    //! Returns the id of the current project (required for write/delete permissions).
    REALITYDATAPLATFORM_EXPORT static Utf8StringCR GetProjectId();

    //! Validates if server parameters have been set
    REALITYDATAPLATFORM_EXPORT static const bool AreParametersSet();

    //! Returns a list of RealityData objects that overlap the given region
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(const RealityDataPagedRequest& request, RawServerResponse& rawResponse);

    //! Returns the specified Data Location.
    REALITYDATAPLATFORM_EXPORT static void RealityDataService::Request(const RealityDataLocationRequest& request, RealityDataLocation& dataLocationObject, RawServerResponse& rawResponse);

    //! Returns the available Data Locations for the enterprise of the user.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataLocation>  RealityDataService::Request(const AllRealityDataLocationsRequest& request,  RawServerResponse& rawResponse);

    //! Returns the size in KB for the specify Enterprise, or the default one.
    REALITYDATAPLATFORM_EXPORT static void RealityDataService::Request(const RealityDataEnterpriseStatRequest& request, RealityDataEnterpriseStat& statObject, RawServerResponse& rawResponse);

    //! Returns a list of EnterpriseStat objects for every organization.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataEnterpriseStat> Request(const RealityDataAllEnterpriseStatsRequest& request, RawServerResponse& rawResponse);

    //! Returns the size in KB for the specify Enterprise, or the default one.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataServiceStat> RealityDataService::Request(const RealityDataServiceStatRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of EnterpriseStat objects for every organization.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataServiceStat> Request(const RealityDataAllServiceStatsRequest& request, RawServerResponse& rawResponse);

    //! Returns the user stats for the specify Enterprise, or the default one.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataUserStat> RealityDataService::Request(const RealityDataUserStatRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of UserStat objects for every organization.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataUserStat> Request(const RealityDataAllUserStatsRequest& request, RawServerResponse& rawResponse);

    //! Returns the list of all documents in a repo
    REALITYDATAPLATFORM_EXPORT static bvector<bpair<WString, uint64_t>> Request(const AllRealityDataByRootId& request, RawServerResponse& rawResponse);

    //! Returns the RealityData object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Request(const RealityDataByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns the RealityDataExtended object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataExtendedPtr Request(const RealityDataExtendedByIdRequest& request, RawServerResponse& rawResponse);

    //! Deletes a RealityData object
    REALITYDATAPLATFORM_EXPORT static void Request(const RealityDataDelete& request, RawServerResponse& rawResponse);

    //! Returns a RealityDataDocument or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataDocumentPtr Request(const RealityDataDocumentByIdRequest& request, RawServerResponse& rawResponse);

    //! Returns the content of a RealityData Service document
    REALITYDATAPLATFORM_EXPORT static void Request(RealityDataDocumentContentByIdRequest& request, BeFile* file, RawServerResponse& rawResponse);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataFolderPtr Request(const RealityDataFolderByIdRequest& request, RawServerResponse& rawResponse);

    //! DEPRECATED use RealityDataListByUltimateIdPagedRequest
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(const RealityDataListByOrganizationPagedRequest& request, RawServerResponse& rawResponse);
    
    //! Returns a list of RealityData objects that belongs to the organization.
    //! Notice that the organization is not usually provided and the organization of the currently
    //! Bentley CONNECT user is used.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(const RealityDataListByUltimateIdPagedRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of RealityDataProjectRelation objects for a specific project.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> Request(const RealityDataRelationshipByProjectIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of RealityDataProjectRelation objects for a specific RealityData.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> Request(const RealityDataRelationshipByRealityDataIdRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of RealityDataProjectRelation objects for a specific project.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> Request(const RealityDataRelationshipByProjectIdPagedRequest& request, RawServerResponse& rawResponse);

    //! Returns a list of RealityDataProjectRelation objects for a specific RealityData.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> Request(const RealityDataRelationshipByRealityDataIdPagedRequest& request, RawServerResponse& rawResponse);

    //! Returns a serverResponse or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataChangeRequest& request, RawServerResponse& rawResponse);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataCreateRequest& request, RawServerResponse& rawResponse);

    //! Creates a relationship between reality data and project
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataRelationshipCreateRequest& request, RawServerResponse& rawResponse);

    //! Deletes a relationship between reality data and project
    REALITYDATAPLATFORM_EXPORT static Utf8String Request(const RealityDataRelationshipDelete& request, RawServerResponse& rawResponse);

    //! Returns the full WSG JSON returned by the request
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static RawServerResponse PagedBasicRequest(const RealityDataPagedRequest* request, Utf8StringCR keyword = "instances");

    //! Returns the full WSG JSON returned by the Reality Data request
    REALITYDATAPLATFORM_EXPORT static RawServerResponse BasicRequest(const RealityDataUrl* request, Utf8StringCR keyword = "instances");

private:
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> _RequestRelationship(const RealityDataUrl* request, RawServerResponse& rawResponse);
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataRelationshipPtr> _RequestPagedRelationships(const RealityDataPagedRequest* request, RawServerResponse& rawResponse);

    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE