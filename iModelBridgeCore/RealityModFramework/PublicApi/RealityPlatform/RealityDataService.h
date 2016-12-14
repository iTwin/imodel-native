/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataService.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
//! The present classes serve as interfaces to the RealityData Service.
//! Although the RealityData Service is based on a simple WSG-based
//! REST API the RealityData Service relies on a variety of interrelated classes
//! and the capacity to perform spatial or classification related queries renders the
//! construction of the request slightly tedious.
//! The present classes provide three levels of simplification of accessing the service
//! and interpreting the results.
//! Before continuing it is recommended to be familiar of the basic classes part of the
//! model definition (RealityData, Folder, and Document).
//! 
//! The RealityData service API is based on equivalent EC Classes that represent mainly the
//! same concepts and the same fields.
//!
//! The first level of abstraction offered in the present higher level class organisation 
//! helps to compose REST API for common or custom queries. The second level of abstraction 
//! offers a mechanism to query the server for various common information without
//! requiring the client to compose the request itself or perform Http request or
//! interpret Http response.
//! The final abstraction level provides complete cooked up solution for
//! common data obtention from the RealityData Service.
//!
//!
//=====================================================================================


    static const Utf8String s_USGSInformationSourceKey = "usgsapi";
    static const Utf8String s_PublicIndexInformationSourceKey = "index";
    static const Utf8String s_AllInformationSourceKey = "all";



//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataByIdRequest
//! This class represents a request for specific Reality Data class object.
//=====================================================================================
struct RealityDataByIdRequest : public WSGObjectRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8StringCR identifier) {m_identifier = identifier;}
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8CP identifier) {m_identifier = identifier;}


    //! Create a request for reality data of the given identifier
    REALITYDATAPLATFORM_EXPORT static RealityDataByIdRequestPtr Create();
   
protected:
    virtual bool _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataByIdRequest() {}

    Utf8String m_identifier; 


    }


//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataFolderByIdRequest
//! This class represents a request for specific Reality Data Folder class object.
//=====================================================================================
struct RealityDataFolderByIdRequest : public WSGObjectRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8StringCR identifier) {m_identifier = identifier;}
    REALITYDATAPLATFORM_EXPORT RealityDataByIdRequest(Utf8CP identifier) {m_identifier = identifier;}

  
protected:
    virtual bool _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataFolderByIdRequest() {}
    Utf8String m_identifier; 

    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataDocumentByIdRequest
//! This class represents a request for specific Reality Data Document class object.
//=====================================================================================
struct RealityDataDocumentByIdRequest : public WSGObjectRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentByIdRequest(Utf8StringCR identifier) {m_identifier = identifier;}
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentByIdRequest(Utf8CP identifier) {m_identifier = identifier;}

   
protected:
    virtual bool _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataDocumentByIdRequest() {}

    Utf8String m_identifier; 

    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataDocumentContentByIdRequest
//! This class represents a request for specific Reality Data Document content class object.
//! The present class provides services for the support of Azure redirection to blob.
//! The RealityDataService can query the class to check is azure redirection is possible
//! or not. If the object indicates the redirection is possible (IsAzureRedirectionPossible)
//! but is not yet Azure blob redirected (IsAzureBlobRedirected()) then the service will 
//! fetch the azure blob redirection request and call the WSG service.
//! If the blob address to the container is returned then the service will
//! set the Azure blob redirection URL (SetAzureRedirectionURLToContainer())
//! After which the object will be set to access directly the blob.
//! Example:
//! RealityDataDocumentContentByIdRequest myRequest("0586-358df-445-de34a-dd286", "RootDocument.3mx");
//! ...
//! if (myRequest.IsAzureRedirectionPossible())
//!     {
//!     if (!myRequest.IsAzureBlobRedirected())
//!         {
//!         Utf8String redirectRequest = myRequest.GetAzureRedirectionRequestUrl();
//!         if (redirectRequest.size() == 0)
//!             myRequest.SetAzureRedirectionPossible(false); // Something is wrong with the request!
//!         else
//!             {
//!             // Send the request URL then parse the result to obtain the blob container url
//!             SetAzureRedirectionUrlToContainer(blobContainerUrl);
//!             }
//!         }
//!    // After this the request will provide the proper http ulr, header and body
//!    // either to the blob or RealityDataService ...
//!
//!    // if a request fails then authentication shoul be reset (either connect or azure)
//!    }
//=====================================================================================
struct RealityDataDocumentContentByIdRequest : public WSGObjectContentRequest
    {
public:
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8StringCR identifier) {m_identifier = identifier;}
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(Utf8CP identifier) {m_identifier = identifier;}
    REALITYDATAPLATFORM_EXPORT RealityDataDocumentContentByIdRequest(RealityDataDocumentContentByIdRequest object) 

    //! This method takes the last portion of the file identifier to obtain the
    //! containing folder id then adds the given portion. This given portion can be
    //! a file name located in the same folder or include a combination of sub-folder/folder
    //! Notice that this call changes the indentifier of the object.
    //! If the current state of the
    REALITYDATAPLATFORM_EXPORT AddPath(additionalPath);

    //! This call creates the URL request to obtain the azure redirection URL.
    //! If the RealityData service 
    REALITYDATAPLATFORM_EXPORT Utf8String GetAzureRedirectionRequestUrl();

    //! Once the azure blob container URL has been obtained it must be set
    //! using this method after which the object will create azure redirection
    //! http urls
    REALITYDATAPLATFORM_EXPORT StatusInt SetAzureRedirectionUrlToContainer(Utf8String azureContainerUrl);

    //! Indicates that an azure blob redirection url has been set to object
    REALITYDATAPLATFORM_EXPORT bool IsAzureBlobRedirected();

    //! Used to indicate the azure blob redirection is possible or not. The default value is true
    //! but if the service does not support azure redirection it must be set to false to
    //! prevent attempts at obtaining redirection.
    REALITYDATAPLATFORM_EXPORT StatusInt SetAzureRedirectionPossible(bool setPossible);

    //! Indicates if azure blob container redirection is possible
    REALITYDATAPLATFORM_EXPORT bool IsAzureRedirectionPossible();
protected:
    virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

private:
    RealityDataDocumentContentByIdRequest() {}
    Utf8String m_identifier; 
    bool m_AzureRedirected;
    bool m_allowAzureRedirection;
    Utf8String m_AzureRedirectionURL;

    }

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! This class represents a spatial request for SpatialEntityWithDetails class object.
//! This represents the most common RealityData Service request.
//! This request returns the list of SpatialEntityWithDetails objects that 
//! are located within provided spatial area (usually the project area) for the 
//! incdicated classification. Additional parameters can be provided after creation.
//=====================================================================================
struct RealityDataPagedRequest : public WSGObjectListPagedRequest
    {
public:
    enum class RealityDataField
        {
        Id,
        Name,
        Description,
        ContainerName,
        Dataset,
        Enterprise,
        Description,
        RootDocument,
        Size,
        Classification,
        Type
        Footprint,
        ThumbnailDocument,
        ResolutionInMeters,
        PublicAccess,
        ModifiedTimeStamp,
        CreatedTimeStamp,
        OwnedBy
        };

public:
    REALITYDATAPLATFORM_EXPORT RealityDataPagedRequest() : m_informationSourceFilteringSet(false), m_requestType(GET_Request), m_sort(false) {}

    //! Sets the sort order for the list. This sorting is performed server-side.
    //! Note that it is not possible to specify two sorts (sort by field a then by filed b is not supported).
    //! The server will decide how sorted groups are ordered.
    //! Note that some fields in the server are considered case-sensitive and others
    //! case insensitive. The server will apply sort rules accordingly.
    REALITYDATAPLATFORM_EXPORT StatusInt SortBy(RealityDataField, bool ascending);

    //! Sets filtering upon the classification. The classification may contain
    //! more than one classification by bitwise oring the classification
    //! values.
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByClassification(int classification);

    //! Filters the returned set by the reality data size.
    //! Both the min and max size must be specified
    REALITYDATAPLATFORM_EXPORT StatusInt FilterBySize(double minSize, double maxSize);

    //! Sets a spatial filter. Only reality data for which the footprint overlaps (even
    //! partially) the given region will be selected.
    //! The area provided is a list of geo points (longitude/latitude)
    //! that must form a closed area. The last point of the list must
    //! be equal to the first point.
    REALITYDATAPLATFORM_EXPORT StatusInt FilterSpatial(bvector<GeoPoint2d> area);

    //! Filters the list by owner. Only reality data belonging to given owner
    //! will be returned. The owner is specified by the email address
    //! and is case insensitive.
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByOwner(Utf8String owner);

    //! Filters the list by creation date. To indicate either min or max date
    //! are unbounded simply provide an invalid/unset DataTime object
    //! If both dates are invalid/unset then the command will return an error
    //! and no filtering will be set.
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByCreationDate(DataTime minDate, DateTime maxDate);

    //! Filters the list by modification date. To indicate either min or max date
    //! are unbounded simply provide an invalid/unset DataTime object.
    //! If both dates are invalid/unset then the command will return an error
    //! and no filtering will be set.  
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByModificationDate(DataTime minDate, DateTime maxDate);

    //! Filters in or out public data as specified
    REALITYDATAPLATFORM_EXPORT StatusInt FilterPublic(bool public);
        
    //! Filter by resolution. As resolution may be confusing since minimum resolution is
    //! expressed a higher number the resolution can be specified in any order and
    //! internally the resolution will be applied accordingly.
    //! Reality data that have no resolution set will be considered 'unspecified' and
    //! will be returned whatever the resolution bracket given if filterOutUnspecified is false
    //! and will be discarded if true
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByResolution(double resMin, double resMax, bool filterOutUnspecified);

    //! Filter by accuracy. As accuracy may be confusing since minimum accuracy is
    //! expressed a higher number the accuracy can be specified in any order and
    //! internally the accuracy will be applied accordingly.    
    //! Reality data that have no accuracy set will be considered 'unspecified' and
    //! will be returned whatever the bracket given if filterOutUnspecified is false
    //! and will be discarded if true
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByAccuracy(double accuracyMin, double accuracyMax, bool filterOutUnspecified);

    //! Filter by type. The type is specified by a string in the reality data.
    //! The filter type specification here can contain many types
    //! separated by semi-colons. All reality data of any of the specified types
    //! will be returned in the list.
    //! types are case insensitive
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByType(Utf8String types);

    //! Filter by dataset. Only reality data of specified dataset will be returned
    //! note that Dataset names are case-sensitive.
    REALITYDATAPLATFORM_EXPORT StatusInt FilterByDataset(Utf8String dataset);

    
protected:
    virtual Utf8StringCR _PrepareHttpRequestStringAndPayload() const override;

    int m_informationSourceFilter;
    bool m_informationSourceFilteringSet;

    RealityDataField m_sortField;
    bool m_sortAscending;
    bool m_sorted;

private:
    }


//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation, (-1)General error, (-2)Retry the command. 
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ByteCurrent Number of byte currently downloaded.
//! @param[out] ByteTotal   When available, number of total bytes to download.
//! @return If RealityDataDownload_ProgressCallBack returns 0   The download continue.
//! @return If RealityDataDownload_ProgressCallBack returns # 0 The download is canceled for this file.
typedef std::function<int(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)> RealityDataServiceUpload_ProgressCallBack;

// ErrorCode --> Curl error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Curl error code:(0)Success (xx)Curl (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Curl English message.
typedef std::function<void(int index, void *pClient, int ErrorCode, const char* pMsg)> RealityDataServiceUpload_StatusCallBack;

//! Callback function to follow the download progression.
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @return If RealityDataDownload_ProgressCallBack returns any other value The download is canceled for all files.
typedef std::function<int()> RealityDataServiceUpload_HeartbeatCallBack;

//=====================================================================================
//! @bsiclass                                   Alain.Robert              12/2016
//! RealityDataServiceUpload
//! This class represents an upload service for uploading files or datasets to the
//! reality data service.
//! During the perform the object will rely on CURL in a multithreaded environment 
//! to upload sources up to the reality data service.
//! The process will attempt to optimise the upload process. To do so it may decide
//! to group a set of files together in an archive then upload and un-archive the file set
//! up in the Reality Data Service. If the files are large the upload process
//! may split up the file and upload in fragments. It may also select to attempt a SAS redirection
//! to upload directly to the cloud blob.
//! In case of communication error the upload process will attempt retry to 
//! complete the operation.
//! At the end of the process the upload will increment the RealityData instance fields
//! concerning total size. Also note that when SetSourcePath is used a root document
//! and a thumbnail may be specified. The appropriate RealityData instance fields
//! will then be updated.
//! It will also start as many threads needed to optimise the process.
//! The present class offers services to upload a file including use of callback
//! to indicate progress
//! The service is used by specifying the source path ot the source file or files.
//! One and only one of SetSourcePath(), SetSOurceFile() or SetSourceFiles()
//! will be called.
//=====================================================================================
struct RealityDataServiceUpload : public RefCounted
    {
    //where the curl upload ended, either in success or failure
    struct UploadResult
        {
        int                     errorCode; //code returned by curl
        size_t                  uploadProgress; //a percentage of how much of the file was successfully downloaded
        };

    //! Set the source path ... this path must be local
    //! all files and folders located in this path will be uploaded
    //! to the designated reality data
    //! @param sourcePath indicates the source path that will be considered the root of the reality data.
    //!   all files and folders will recursively be scanned and uploaded.
    //! @param rootDocument The root document for the reality data or empty if the root document
    //! must not be set or modified. This document is specified relative to the root source path.
    //! @param thumbnailDocument The thumbnail document. It must designate a JPG or PNG file providing a
    //! a visual overview of the reality data.
    REALITYDATAPLATFORM_EXPORT StatusInt SetSourcePath(Utf8String sourcePath, Utf8String rootDocument, Utf8String thumbnailDocument);
    
    //! This method specifies a single file for upload
    //! It is usually meant to update an existing reality data set
    REALITYDATAPLATFORM_EXPORT StatusInt SetSourceFile(Utf8String sourceFile);

    //! This method specifies a list of files for upload
    //! It is usually meant to update an existing reality data set
    REALITYDATAPLATFORM_EXPORT StatusInt SetSourceFile(bvector<Utf8String> const& sourceFiles);

    //! Sets the RealityDataID that also designates the container to which the data is uploaded
    REALITYDATAPLATFORM_EXPORT StatusInt SetRealityDataId(Utf8String realityDataId);

    //! Set proxy informations
    REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) { m_proxyUrl = proxyUrl; m_proxyCreds = proxyCreds; };

    //! Set certificate path for https upload.
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(WStringCR certificatePath) { m_certPath = certificatePath; };

    //! Set callback to follow progression of the upload.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataServiceUpload_ProgressCallBack pi_func, float pi_step = 0.01) 
                                                                   {m_pProgressFunc = pi_func; m_progressStep = pi_step;};
    //! Set callback to allow the user to mass cancel all uploads
    REALITYDATAPLATFORM_EXPORT void SetHeartbeatCallBack(RealityDataServiceUpload_HeartbeatCallBack pi_func)
                                                                   {m_pHeartbeatFunc = pi_func;};

    //! Set callback to know to status, upload done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataServiceUpload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; };

    //! Start the upload progress for all links.
    REALITYDATAPLATFORM_EXPORT UploadReport* Perform();

    
    }

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
    static SetServerComponents(Utf8StringCR server, Utf8StringCR WSGProtocol, Utf8StringCR name, Utf8StringCR schemaName)
        {
        BeAssert(server.size() != 0);
        BeAssert(WSGProtocol.size() != 0);
        BeAssert(name.size() != 0);
        BeAssert(schemaName.size() != 0);

        s_realityDataServer = server;
        s_realityDataWSGProtocol = WSGProtocol;
        s_realityDataName = name;
        s_realityDataSchemaName = schemaName;
        }

    //! Returns the current name of the server
    static Utf8StringCR GetServer();

    //! Results the string containing the WSG protocol version number
    static Utf8StringCR GetWSGProtocol();

    //! Returns the name of the WSG repository containing the RealityData Service objects
    static Utf8StringCR GetName();

    //! Returns the name of the schema defining the classes exposed by the RealityData Service.
    static Utf8StringCR GetSchemaName();

    //! The classification codes. The high level interface only supports the four base classification
//&&AR Most to platform since Classification is shared by both GeoCoordinationService and RealityData Service
    enum class Classification
        {
        Imagery = 0x1,
        Terrain = 0x2;
        Model = 0x4;
        Pinned = 0x8;
        }


public:

    //! Returns a list of RealityData objects that overlap the given region
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(RealityDataPagedRequestR request);


    //! Returns the RealityData object requested or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataPtr Request(RealityDataByIdRequestCR request);

    //! Returns a RealityDataDocument or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataDocumentPtr Request(RealityDataDocumentByIdCR request);

    //! Returns the content of a RealityData Service document
    REALITYDATAPLATFORM_EXPORT static bvector<byte> Request(RealityDataDocumentContentByIdCR request);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static RealityDataFolderPtr Request(RealityDataFolderByIdCR request);

    //! Returns a list of RealityData objects that belongs to the enterprise.
    //! Notice that the enterprise is not usually provided and the enterprise of the currently
    //! Bentley CONNECT user is used.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataPtr> Request(RealityDataListByEnterprisePagedRequestCR request);

    //! Returns a list of RealityDataProjectRelation objects for a specific project.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static bvector<RealityDataProjectRelationPtr> Request(RealityDataProjectRelationByProjectPagedRequestCR request);


    //! Returns the full WSG JSON returned by the request
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataPagedRequestR request);


    //! Returns the full WSG JSON returned by the Reality Data request
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataByIdRequestCR request);

    //! Returns a RealityDataDocument or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataDocumentByIdCR request);

    //! Returns the content of a RealityData Service document
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataDocumentContentByIdCR request);

    //! Returns a RealityDataFolder or null if an error occured
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataFolderByIdCR request);

    //! Returns the full WSG JSON returned by the request.
    //! Since this request is a paged request it will advance to next page automatically
    //! and return on last page with appropriate status.
    REALITYDATAPLATFORM_EXPORT static Utf8String RequestToJSON(RealityDataListByEnterprisePagedRequestCR request);


    //! Returns a RealityDataServiceUpload object
    REALITYDATAPLATFORM_EXPORT static RealityDataServiceUploadPtr CreateUpload();


private:
    static Utf8String s_realityDataServer = "https://connect-contextservices.bentley.com/";
    static Utf8String s_realityDataWSGProtocol = "2.4";
    static Utf8String s_realityDataName = "IndexECPlugin-Server";
    static Utf8String s_realityDataSchemaName = "RealityModeling";

    static const Utf8String s_ImageryKey = "Imagery";
    static const Utf8String s_TerrainKey = "Terrain";
    static const Utf8String s_ModelKey = "Model";
    static const Utf8String s_PinnedKey = "Pinned";
    }


END_BENTLEY_REALITYPLATFORM_NAMESPACE