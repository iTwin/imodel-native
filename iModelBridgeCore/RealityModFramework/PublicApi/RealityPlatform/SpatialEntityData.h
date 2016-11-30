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


#if (0)
// Temporarily disabled ... to be kept

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct SpatialEntityThumbnail : public RefCountedBase
{
public:
    //! Create invalid thumbnail.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityThumbnailPtr Create();

    //! IsEmpty
    //! Indicates if the metadata is empty and had no fields set.
    REALITYDATAPLATFORM_EXPORT bool IsEmpty() const;    

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

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetThumbnailUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetThumbnailUrl(Utf8CP thumbnailUrl);

protected:
    SpatialEntityThumbnail();

    Utf8String m_provenance;
    Utf8String m_format;
    uint32_t m_width;
    uint32_t m_height;
    DateTime m_stamp;
    bvector<Byte> m_data;
    Utf8String m_generationDetails;
    Utf8String m_thumbnailUrl;


    bool m_isEmpty;
};

#endif

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

    //! IsEmpty
    //! Indicates if the metadata is empty and had no fields set.
    REALITYDATAPLATFORM_EXPORT bool IsEmpty() const;

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

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadataUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetMetadataUrl(Utf8CP metadataUrl);

protected:
    SpatialEntityMetadata();
    SpatialEntityMetadata(Utf8CP filePath);

    Utf8String m_provenance;
    Utf8String m_description;
    Utf8String m_contactInfo;
    Utf8String m_legal;
    Utf8String m_format;
    Utf8String m_data;
    Utf8String m_metadataUrl;

    bool m_isEmpty;
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
struct SpatialEntityDataSource : public RefCountedBase
{
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Create();

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGeoCS() const;
    REALITYDATAPLATFORM_EXPORT void SetGeoCS(Utf8CP geoCS);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOriginalId() const;
    REALITYDATAPLATFORM_EXPORT void SetOriginalId(Utf8CP originalId);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCompoundType() const;
    REALITYDATAPLATFORM_EXPORT void SetCompoundType(Utf8CP type);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT uint64_t GetSize() const; // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetSize(uint64_t size); // in bytes.

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetNoDataValue() const;
    REALITYDATAPLATFORM_EXPORT void SetNoDataValue(Utf8CP value); 

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLocationInCompound() const;
    REALITYDATAPLATFORM_EXPORT void SetLocationInCompound(Utf8CP location);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityServerCR GetServer() const;
    REALITYDATAPLATFORM_EXPORT void SetServer(SpatialEntityServerR server);

    REALITYDATAPLATFORM_EXPORT bool GetIsMultiband() const;
    REALITYDATAPLATFORM_EXPORT void SetIsMultiband( bool isMultiband );

    REALITYDATAPLATFORM_EXPORT void GetMultibandUrls( Utf8String& redUrl, Utf8String& greenUrl, Utf8String& blueUrl, Utf8String& panchromaticUrl ) const;
    REALITYDATAPLATFORM_EXPORT void SetMultibandUrls( Utf8String redUrl, Utf8String greenUrl, Utf8String blueUrl, Utf8String panchromaticUrl );
    
    REALITYDATAPLATFORM_EXPORT uint64_t GetRedBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetRedBandSize( uint64_t size );

    REALITYDATAPLATFORM_EXPORT uint64_t GetBlueBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetBlueBandSize( uint64_t size );

    REALITYDATAPLATFORM_EXPORT uint64_t GetGreenBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetGreenBandSize( uint64_t size );

    REALITYDATAPLATFORM_EXPORT uint64_t GetPanchromaticBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetPanchromaticBandSize( uint64_t size );

    REALITYDATAPLATFORM_EXPORT SQLINTEGER GetServerId() const;
    //serverId is a mutable value so that it can be set on a const ref, before performing a Save()
    REALITYDATAPLATFORM_EXPORT void SetServerId( SQLINTEGER id ) const;

protected:
    SpatialEntityDataSource();

    Utf8String m_url;
    Utf8String m_geoCS;
    Utf8String m_compoundType;
    uint64_t m_size;
    Utf8String m_dataType;
    Utf8String m_locationInCompound;
    SpatialEntityServerPtr m_pServer;
    Utf8String m_noDataValue;
    Utf8String m_originalId;

    bool m_isMultiband = false;
    Utf8String m_redDownloadUrl;
    Utf8String m_blueDownloadUrl;
    Utf8String m_greenDownloadUrl;
    Utf8String m_panchromaticDownloadUrl;
    uint64_t m_redSize;
    uint64_t m_blueSize;
    uint64_t m_greenSize;
    uint64_t m_panchromaticSize;

    mutable SQLINTEGER m_serverId = -1;
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
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetResolution() const; // in meters.
    REALITYDATAPLATFORM_EXPORT void SetResolution(Utf8CP resolution); // in meters.

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
    REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProviderName() const;
    REALITYDATAPLATFORM_EXPORT void SetProviderName(Utf8CP providerName);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetClassification() const;
    REALITYDATAPLATFORM_EXPORT void SetClassification(Utf8CP classification);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;
    REALITYDATAPLATFORM_EXPORT void SetDataset(Utf8CP dataset);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetThumbnailURL() const;
    REALITYDATAPLATFORM_EXPORT void SetThumbnailURL(Utf8CP thumbnailURL);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetThumbnailLoginKey() const;
    REALITYDATAPLATFORM_EXPORT void SetThumbnailLoginKey(Utf8CP thumbnailLoginKey);

    //! Get/Set
    //! The approximate file size is not stored in data model but returned as an indication 
    //! based on the file size of the first data source. Informative field only.
    REALITYDATAPLATFORM_EXPORT uint64_t GetApproximateFileSize() const; // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetApproximateFileSize(uint64_t size); // in bytes.

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT const bvector<DPoint2d>& GetFootprint() const;
    REALITYDATAPLATFORM_EXPORT void SetFootprint(bvector<DPoint2d>& footprint);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT bool HasApproximateFootprint() const;
    REALITYDATAPLATFORM_EXPORT void SetApproximateFootprint(bool approximateFootprint);

    REALITYDATAPLATFORM_EXPORT DRange2dCR GetFootprintExtents() const;
    REALITYDATAPLATFORM_EXPORT void SetFootprintExtents(DRange2dCR footprintExtents);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCR GetDataSource(size_t index) const;
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceR GetDataSource(size_t index);
    REALITYDATAPLATFORM_EXPORT void AddDataSource(SpatialEntityDataSourceR dataSource);
    REALITYDATAPLATFORM_EXPORT size_t GetDataSourceCount() const;


    //! Get/Set
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataCR GetMetadata() const;
    REALITYDATAPLATFORM_EXPORT void SetMetadata(SpatialEntityMetadataR metadata);
  
    REALITYDATAPLATFORM_EXPORT float GetCloudCover() const;
    REALITYDATAPLATFORM_EXPORT void SetCloudCover( float cover );

protected:
    SpatialEntityData();

    Utf8String m_name;
    Utf8String m_resolution;
    Utf8String m_provider;
    Utf8String m_providerName;
    Utf8String m_dataType;
    Utf8String m_classification;
    DateTime m_date;
    Utf8String m_dataset;
    Utf8String m_thumbnailURL;
    Utf8String m_thumbnailLoginKey;
    bvector<DPoint2d> m_footprint;
    DRange2d m_footprintExtents;
    bool m_approximateFootprint;
    uint64_t m_approximateFileSize;
    // SpatialEntityThumbnailPtr m_pThumbnail;
    SpatialEntityMetadataPtr m_pMetadata;
    bvector<SpatialEntityDataSourcePtr> m_DataSources;

    float m_cloudCover = -1.0f;
    //uint64_t m_redSize;
    //uint64_t m_blueSize;
    //uint64_t m_greenSize;
    //uint64_t m_panchromaticSize;

    mutable SQLINTEGER m_serverId = -1;
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