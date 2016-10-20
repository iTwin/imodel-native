/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatialEntityData.h $
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
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct CurlHolder
{
public:
    CurlHolder() : m_curl(curl_easy_init()) {}
    ~CurlHolder() { if (NULL != m_curl) curl_easy_cleanup(m_curl); }
    CURL* Get() const { return m_curl; }

private:
    CURL* m_curl;
};

struct SpatialEntityServer;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! Status codes for SpatialEntity traversal operations.
//=====================================================================================
enum class SpatialEntityStatus
    {
    Success = SUCCESS,      // The operation was successful.
    ClientError,
    CurlError,
    DataExtractError,
    DownloadError,
    // *** Add new here.
    UnknownError = ERROR,   // The operation failed with an unspecified error.
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
// Defines the traversal observer interface as an abstract class.
// The traversal observer that must inherit from the present one must implement
// a behavior for the OnFileListed, OnFileDownloaded and OnDataExtracted method.
//=====================================================================================
struct ISpatialEntityTraversalObserver
{
public:
    //! OnFileListed is called whenever a file was listed for download. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file) = 0;

    //! OnFileDownloaded is called whenever a download is completed. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnFileDownloaded(Utf8CP file) = 0;

    //! OnDataExtracted is called whenever an extraction is completed by the 
    //! data handler. The data object given defines the data discovered. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnDataExtracted(SpatialEntityDataCR data) = 0;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityClient : public RefCountedBase
{
public:
    //! Mapping of the remote and local link to a file.
    typedef bvector<bpair<Utf8String, Utf8String>> RepositoryMapping;
    
    //! Download all files from root and saved them in the specified directory. Default is temp directory.
    REALITYDATAPLATFORM_EXPORT SpatialEntityStatus DownloadContent(Utf8CP outputDirPath = NULL) const;

    //! Get a list of all files from all directories starting at root.
    REALITYDATAPLATFORM_EXPORT SpatialEntityStatus GetFileList(bvector<Utf8String>& fileList) const;

    //! Get complete data from root. This includes base, source, thumbnail, metadata and server details.
    //! An observer must be set to catch the data after each extraction.
    REALITYDATAPLATFORM_EXPORT SpatialEntityStatus GetData() const;

    //! Get server url.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerUrl() const;

    //! Get server name.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const;

    //! Get file pattern.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;
	
    //! Get file pattern.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFilePattern() const;

    //! Get classification.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetClassification() const;

    //! Get repository mapping (remote and local repository).
    REALITYDATAPLATFORM_EXPORT const RepositoryMapping& GetRepositoryMapping() const;

    //! Set the ISpatialEntityTraversalObserver to be called after each extraction. Only one observer
    //! can be set. Typically, the user of the handler would implement the ISpatialEntityTraversalObserver
    //! interface and send "this" as the argument to this method.
    REALITYDATAPLATFORM_EXPORT void SetObserver(ISpatialEntityTraversalObserver* pObserver);

protected:
    REALITYDATAPLATFORM_EXPORT SpatialEntityClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classsification);
    REALITYDATAPLATFORM_EXPORT ~SpatialEntityClient();

    //! Recurse into sub directories and create a list of all files.
    virtual SpatialEntityStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const = 0;

    virtual SpatialEntityDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const = 0;

    //! Check if content is a directory or not.
    REALITYDATAPLATFORM_EXPORT bool IsDirectory(Utf8CP content) const;

    //! Callback when a file is downloaded to construct the data repository mapping.
    static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
    
    REALITYDATAPLATFORM_EXPORT static ISpatialEntityTraversalObserver* GetObserver();

    BeFileName m_certificatePath;
    SpatialEntityServerPtr m_pServer;
    Utf8String m_datasetName;
    static ISpatialEntityTraversalObserver* m_pObserver;
    static RepositoryMapping m_dataRepositories;
    static int m_retryCount;
    Utf8String m_filePattern;
    bool m_extractThumbnails;
    Utf8String m_classification;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityRequest : public RefCountedBase
{
public:
    //! Get response from request.
    virtual SpatialEntityResponsePtr Perform();

    //! Get url.
    Utf8StringCR GetUrl() const { return m_url; }

    //! Get/Set method option.
    Utf8StringCR GetMethod() const { return m_method; }
    void SetMethod(Utf8CP method) { m_method = method; }

    //! Get/Set dirList option.
    bool IsDirListOnly() const { return m_dirListOnly; }
    void SetDirListOnly(bool isDirListOnly) { m_dirListOnly = isDirListOnly; }

    //! Get/Set verbose option.
    bool IsVerbose() const { return m_verbose; }
    void SetVerbose(bool isVerbose) { m_verbose = isVerbose; }

protected:
    REALITYDATAPLATFORM_EXPORT SpatialEntityRequest(Utf8CP url);

    Utf8String m_url;
    Utf8String m_method;
    bool m_dirListOnly;
    bool m_verbose;
};


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityResponse : public RefCountedBase
{
public:
    //! Create invalid response with SpatialEntityStatus::UnknownError.
    static SpatialEntityResponsePtr Create();

    //! Create response.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityResponsePtr Create(Utf8CP effectiveUrl, Utf8CP m_content, SpatialEntityStatus traversalStatus);

    //! Get url.
    Utf8StringCR GetUrl() const;

    //! Get content.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContent() const;

    //! Get status.
    REALITYDATAPLATFORM_EXPORT SpatialEntityStatus GetStatus() const;

    //! Return if request was a success or not.
    REALITYDATAPLATFORM_EXPORT bool IsSuccess() const;

protected:
    SpatialEntityResponse(Utf8CP effectiveUrl, Utf8CP content, SpatialEntityStatus status);

    Utf8String m_effectiveUrl;
    Utf8String m_content;
    SpatialEntityStatus m_status;
};

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct SpatialEntityThumbnail : public RefCountedBase
{
public:
    //! Create invalid thumbnail.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityThumbnailPtr Create();

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
    REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
    REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT uint32_t GetWidth() const;
    REALITYDATAPLATFORM_EXPORT void SetWidth(uint32_t width);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT uint32_t GetHeight() const;
    REALITYDATAPLATFORM_EXPORT void SetHeight(uint32_t height);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetStamp() const;
    REALITYDATAPLATFORM_EXPORT void SetStamp(DateTimeCR date);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT const bvector<Byte>& GetData() const;
    REALITYDATAPLATFORM_EXPORT void SetData(const bvector<Byte>& data);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGenerationDetails() const;
    REALITYDATAPLATFORM_EXPORT void SetGenerationDetails(Utf8CP details);

protected:
    SpatialEntityThumbnail();

    Utf8String m_provenance;
    Utf8String m_format;
    uint32_t m_width;
    uint32_t m_height;
    DateTime m_stamp;
    bvector<Byte> m_data;
    Utf8String m_generationDetails;
};


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct SpatialEntityMetadata : public RefCountedBase
{
public:
    //! Create empty metadata.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr Create();

    //! Create from xml file.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr CreateFromFile(Utf8CP filePath);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
    REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
    REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
    REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
    REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetData() const;
    REALITYDATAPLATFORM_EXPORT void SetData(Utf8CP data);

protected:
    SpatialEntityMetadata();
    SpatialEntityMetadata(Utf8CP filePath);

    Utf8String m_provenance;
    Utf8String m_description;
    Utf8String m_contactInfo;
    Utf8String m_legal;
    Utf8String m_format;
    Utf8String m_data;
};


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct SpatialEntityServer : public RefCountedBase
{
public:
    //! Create invalid server.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Create();

    //! Create from url.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Create(Utf8CP url, Utf8CP name = NULL);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProtocol() const;
    REALITYDATAPLATFORM_EXPORT void SetProtocol(Utf8CP protocol);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
    REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
    REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
    REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT bool IsOnline() const;
    REALITYDATAPLATFORM_EXPORT void SetOnline(bool online);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastCheck() const;
    REALITYDATAPLATFORM_EXPORT void SetLastCheck(DateTimeCR time);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastTimeOnline() const;
    REALITYDATAPLATFORM_EXPORT void SetLastTimeOnline(DateTimeCR time);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT double GetLatency() const;
    REALITYDATAPLATFORM_EXPORT void SetLatency(double latency);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetState() const;
    REALITYDATAPLATFORM_EXPORT void SetState(Utf8CP state);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetType() const;
    REALITYDATAPLATFORM_EXPORT void SetType(Utf8CP type);

protected:
    SpatialEntityServer();
    SpatialEntityServer(Utf8CP url, Utf8CP name);

    Utf8String m_protocol;
    Utf8String m_name;
    Utf8String m_url;
    Utf8String m_contactInfo;
    Utf8String m_legal;
    bool m_online;
    DateTime m_lastCheck;
    DateTime m_lastTimeOnline;
    double m_latency;
    Utf8String m_state;
    Utf8String m_type;
};


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityData : public RefCountedBase
{
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataPtr Create();

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
    REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGeoCS() const;
    REALITYDATAPLATFORM_EXPORT void SetGeoCS(Utf8CP geoCS);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCompoundType() const;
    REALITYDATAPLATFORM_EXPORT void SetCompoundType(Utf8CP type);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT uint64_t GetSize() const; // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetSize(uint64_t size); // in bytes.

                                                            //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetResolution() const; // in meters.
    REALITYDATAPLATFORM_EXPORT void SetResolution(Utf8CP resolution); // in meters.

                                                                      //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
    REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetClassification() const;
    REALITYDATAPLATFORM_EXPORT void SetClassification(Utf8CP classification);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLocationInCompound() const;
    REALITYDATAPLATFORM_EXPORT void SetLocationInCompound(Utf8CP location);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;
    REALITYDATAPLATFORM_EXPORT void SetDataset(Utf8CP dataset);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT const bvector<DPoint2d>& GetFootprint() const;
    REALITYDATAPLATFORM_EXPORT void SetFootprint(bvector<DPoint2d>& footprint);

    REALITYDATAPLATFORM_EXPORT DRange2dCR GetFootprintExtents() const;
    REALITYDATAPLATFORM_EXPORT void SetFootprintExtents(DRange2dCR footprintExtents);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityThumbnailCR GetThumbnail() const;
    REALITYDATAPLATFORM_EXPORT void SetThumbnail(SpatialEntityThumbnailR thumbnail);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataCR GetMetadata() const;
    REALITYDATAPLATFORM_EXPORT void SetMetadata(SpatialEntityMetadataR metadata);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityServerCR GetServer() const;
    REALITYDATAPLATFORM_EXPORT void SetServer(SpatialEntityServerR server);

    REALITYDATAPLATFORM_EXPORT bool GetIsMultiband() const;
    REALITYDATAPLATFORM_EXPORT void SetIsMultiband( bool isMultiband );

    REALITYDATAPLATFORM_EXPORT Utf8String GetMultibandUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetMultibandUrl( Utf8String url );
    
    REALITYDATAPLATFORM_EXPORT float GetCloudCover() const;
    REALITYDATAPLATFORM_EXPORT void SetCloudCover( float cover );

    REALITYDATAPLATFORM_EXPORT float GetRedBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetRedBandSize( float size );

    REALITYDATAPLATFORM_EXPORT float GetBlueBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetBlueBandSize( float size );

    REALITYDATAPLATFORM_EXPORT float GetGreenBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetGreenBandSize( float size );

    REALITYDATAPLATFORM_EXPORT float GetPanchromaticBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetPanchromaticBandSize( float size );

    REALITYDATAPLATFORM_EXPORT SQLINTEGER GetMultibandServerId() const;
    REALITYDATAPLATFORM_EXPORT void SetMultibandServerId( SQLINTEGER id );

protected:
    SpatialEntityData();

    Utf8String m_name;
    Utf8String m_url;
    Utf8String m_geoCS;
    Utf8String m_compoundType;
    uint64_t m_size;
    Utf8String m_resolution;
    Utf8String m_provider;
    Utf8String m_dataType;
    Utf8String m_classification;
    Utf8String m_locationInCompound;
    DateTime m_date;
    Utf8String m_dataset;
    bvector<DPoint2d> m_footprint;
    DRange2d m_footprintExtents;
    SpatialEntityThumbnailPtr m_pThumbnail;
    SpatialEntityMetadataPtr m_pMetadata;
    SpatialEntityServerPtr m_pServer;

    bool m_isMultiband = false;
    Utf8String m_multibandDownloadUrl;
    float m_cloudCover;
    float m_redSize;
    float m_blueSize;
    float m_greenSize;
    float m_panchromaticSize;
    SQLINTEGER m_multibandServerId;
}; 
   

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//!
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityDataHandler
{
public:
    //! Unzip files.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityStatus UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath);

protected:
    //! Http data extraction.
    REALITYDATAPLATFORM_EXPORT static BeFileName BuildMetadataFilename(Utf8CP filePath);

    //! Geocoding lookup.
    REALITYDATAPLATFORM_EXPORT static Utf8String RetrieveGeocodingFromMetadata(BeFileNameCR filename);
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE