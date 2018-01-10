/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatialEntity.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataObjects.h>

#include <Bentley/DateTime.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//=====================================================================================
//! Resource identifier representing a document location. The uri can designate a compound 
//! document such as a zip file. The uri is then separated in two parts by the presence of a 
//! hash sign (#). The first part indicates the location and the name of a compound 
//! document and the later part identify the file(s) within the compound.
//! @bsiclass                                   Jean-Francois.Cote              9/2016
//=====================================================================================
struct Uri : public RefCountedBase
    {
    public:
        friend struct RealityDataSource;

        //! Create a uri with a fully qualified identifier or with separated parts.
        REALITYDATAPLATFORM_EXPORT static UriPtr Create(Utf8CP resourceIdentifier);
        REALITYDATAPLATFORM_EXPORT static UriPtr Create(Utf8CP source, Utf8CP fileInCompound);

        //! Get the first part of the identifier e.g. the full path of the document.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetSource() const;

        //! Get the later part of the identifier e.g. the file in the compound document. Null if the source is not a compound document.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFileInCompound() const;

        //! Get the complete identifier as a string.
        REALITYDATAPLATFORM_EXPORT Utf8String ToString() const;

    protected:
        Uri() {}
        Uri(Utf8CP source, Utf8CP fileInCompound);

    private:
        Utf8String m_source;
        Utf8String m_fileInCompound;
    };

//=====================================================================================
//! Overview:
//! The following classes represents the foundation of the GeoCoordination Service
//! client SDK. These classes enable to represent the information related to spatial 
//! entities as inventoried by the GeoCoordination Service.
//! The main class is SpatialEntity that represents a data that is geocoordinated. This class
//! does not contain the data proper but rather represents the informative portion of the
//! data. The class contains names, description, data provider, spatial footprint (2D),
//! and so on and makes reference to additional information but a potention link to a 
//! SpatialEntityMetadata instance. 
//!
//! The SpatialEntityMetadata class enables to represent a few fields extracted from 
//! the metadata and possibly a reference to an external page or document containing this
//! metadata. An instance of a metadata may or may not be shared by spatial entities.
//!
//! The spatial entity makes reference to a list of SpatialEntityDataSource. It 
//! indicates the location of the various data sources (files or stream) where can
//! be obtained the data. There can be many data source for a same spatial entity
//! for example when mirror site are used for distribution or if the data
//! is distributed through different file formats or different geographic coordinate
//! system. A data source thus contains the file location (URL) and indications about the
//! format of this file for example if it is a zip file that contains the final file 
//! (a compound document) and the name of the file in the compound (Location in
//! compound) as well as geographic coordinate system and file format (usually file
//! extension).
//!
//! Spatial data sources refer to a SpatialEntityServer class that keeps information
//! about the server. This class contains information about the server proper including
//! authentication required, communication protocol (http, ftp) and data access 
//! protocol (WMS, S3MX, ftp, http). It may contain statitics concerning the
//! robustness of the server (uptime, last time checked, etc)
//!
//! An additional class is provided that is not used by the SpatialEntity model used
//! to represent and store thumbnails. Although unused at the moment it may be
//! in a future to manage thumbnails stored as part of the service.
//=====================================================================================

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//! This class is not currently used but kept as we have not yet elimnated the
//! intention to have some sort of thumbnail service either apart or integrated
//! in the GeoCoordination Service.
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
    //! The provenance is a human readable text to indicate where the thumbnail comes from
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
    REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

    //! Get/Set
    //! A key stored as a string indicating the format of the thumbnail data
    //! It should be restricted to jpg or png
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
    REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

    //! Get/Set
    //! The with of the thumbnail in pixels or 0 if this dimension is unknown
    REALITYDATAPLATFORM_EXPORT uint32_t GetWidth() const;
    REALITYDATAPLATFORM_EXPORT void SetWidth(uint32_t width);

    //! Get/Set
    //! The height of the thumbnail in pixels or 0 if this dimension is unknown
    REALITYDATAPLATFORM_EXPORT uint32_t GetHeight() const;
    REALITYDATAPLATFORM_EXPORT void SetHeight(uint32_t height);

    //! Get/Set
    //! The time stamp indicating the moment where the thumbnail was generated
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetStamp() const;
    REALITYDATAPLATFORM_EXPORT void SetStamp(DateTimeCR date);

    //! Get/Set
    //! The raw data of the thumbnail. It should be a jpg or png file content
    //! Keep the size within reasonable limits.
    REALITYDATAPLATFORM_EXPORT const bvector<Byte>& GetData() const;
    REALITYDATAPLATFORM_EXPORT void SetData(const bvector<Byte>& data);

    //! Get/Set
    //! Text human readable explaining how the thumbnail was generated if applicable.
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

    bool m_isEmpty;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//! This class holds general use metadata. This metadata can be used by many different
//! spatial entities or not depending if the metadata applies to the whole dataset or
//! to a specific spatial entity
//=====================================================================================
struct SpatialEntityMetadata : public RefCountedBase
    {
public:
    //! Create empty metadata.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr Create();

    //! Create from xml file.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr CreateFromFile(Utf8CP filePath, SpatialEntityMetadataCR metadataSeed);

    //! Copy constructor
    REALITYDATAPLATFORM_EXPORT static SpatialEntityMetadataPtr CreateFromMetadata(SpatialEntityMetadataCR metadataSeed);

    //! IsEmpty
    //! Indicates if the metadata is empty and had no fields set.
    REALITYDATAPLATFORM_EXPORT bool IsEmpty() const;

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8CP id);

    //! Get/Set
    //! Human readable text indicating the provenance of the spatial entity
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
    REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

    //! Get/Set
    //! Human readable text indicating the lineage that resulted in the data.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLineage() const;
    REALITYDATAPLATFORM_EXPORT void SetLineage(Utf8CP lineage);

    //! Get/Set
    //! Description of the spatial entity or the dataset the spatial entity belongs to.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);

    //! Get/Set
    //! Contact information to obtain information about the spatial entity
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
    REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

    //! Get/Set
    //! A text indicating the copyright to the data or a URL to a web page containing 
    //! information. The text can be a mix of text and URL. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
    REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

    //! Get/Set
    //! If known or different from Legal field, a text indicating the terms of use applicable to the data
    //! or a URL to a web page containing the information. The text can be a mix of text and URL. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetTermsOfUse() const;
    REALITYDATAPLATFORM_EXPORT void SetTermsOfUse(Utf8CP termsOfUse);

    //! Get/Set
    //! A formated string that contains a list of keywords applicable to the data.
    //! Keywords must be semi-colon separated but can contain space.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetKeywords() const;
    REALITYDATAPLATFORM_EXPORT void SetKeywords(Utf8CP keywords);

    //! Get/Set
    //! A string key indicating the format of the metadata referenced by the MetadataURL.
    //! The valid keys are currently HTML (a web page), ISO-19115 (iso19115 compliant xml),
    //! FGDC (FGDC complaint xml), TEXT (text or unknown format text string)
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
    REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

    //! Get/Set
    //! A reference to an external source for the metadata. It is usually a reference to an 
    //! HTTP or FTP source of information. If the metadata file is distributed via a ZIP (or other archive file) 
    //! it may contain the name of the file within the archive by adding a hash sign, 
    //! then the name of this file. For example:
    //! ftp://enterprise.com/archive.zip#metadata.xml
    //! otherwise the URL links directly to the information.
    //! Note that it is not advisable to make reference to a file such as an archive file of large size
    //! in this field since the intent is to provide a fast location to consult
    //! metadata information prior to using the data. If the metadata file is located
    //! near or within a spatial entity data source, the client application will have access
    //! to this metadata after downloading the data source.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadataUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetMetadataUrl(Utf8CP metadataUrl);

    //! Get/Set the type of the metadata. FGDC and ISO-19115 are the only two formats supported. All other 
    // formats will be indicated as TEXT to be interpreted by a human. Might be empty.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadataType() const;
    REALITYDATAPLATFORM_EXPORT void         SetMetadataType(Utf8CP metadataType);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDisplayStyle() const;
    REALITYDATAPLATFORM_EXPORT void SetDisplayStyle(Utf8CP displayStyle);

protected:
    SpatialEntityMetadata();
    SpatialEntityMetadata(Utf8CP filePath, SpatialEntityMetadataCR metadataSeed);
    SpatialEntityMetadata(SpatialEntityMetadataCR metadataSeed);

    Utf8String m_id;
    Utf8String m_provenance;
    Utf8String m_lineage;
    Utf8String m_description;
    Utf8String m_contactInfo;
    Utf8String m_legal;
    Utf8String m_termsOfUse; 
    Utf8String m_format;
    Utf8String m_keywords;
    Utf8String m_metadataUrl;
    Utf8String m_displayStyle;
    Utf8String m_metadataType;

    bool m_isEmpty;
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//! Class to represent a data server. It indicates server specific information
//! such as communication protocol (ftp, http, ...) authentication parameters
//! data protocol (ftp, WMS, S3MX, ...)
//! A server is used by many datasources.
//=====================================================================================
struct SpatialEntityServer : public RefCountedBase
    {
public:
    //! Create invalid server.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Create();

    //! Create from url.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityServerPtr Create(Utf8CP url, Utf8CP name = NULL);

    //! Get/Set
    //! The Id used to access this SpatialEntityServer on the GCS server
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8CP id);

    //! Get/Set
    //! A key indicating the communication/transport protocol. Usually ‘http’ or ‘ftp’
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProtocol() const;
    REALITYDATAPLATFORM_EXPORT void SetProtocol(Utf8CP protocol);

    //! Get/Set
    //! A key indicating the server type that indicates the protocol to obtain information. 
    //! If empty, then the communication protocol is sufficient to access data. Typical values 
    //! would be ‘ftp’, ‘http’ for plain ftp/http or ‘WMS’, ‘WFS’, or other yet to be defined. 
    //! Note that new values will be introduced soon for the support of CONNECT project share, Reality 
    //! Data Service and other data provider services part of Bentley CONNECT since the CommunicationProtocol 
    //! is insufficient to create a properly formed request.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetType() const;
    REALITYDATAPLATFORM_EXPORT void SetType(Utf8CP type);

    //! Get/Set
    //! Name of server. Example ‘USGS Earth Explorer’
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
    REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

    //! Get/Set
    //! The URL to server
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

    //! Get/Set
    //! Contact Information to obtain information about the server.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
    REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

    //! Get/Set
    //! Text indicating legal information about accessing the server.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
    REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

    //! Get/Set
    //! If known indicates if server is currently online (at last check)
    REALITYDATAPLATFORM_EXPORT bool IsOnline() const;
    REALITYDATAPLATFORM_EXPORT void SetOnline(bool online);

    //! Get/Set
    //! Last time server was checked
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastCheck() const;
    REALITYDATAPLATFORM_EXPORT void SetLastCheck(DateTimeCR time);
    REALITYDATAPLATFORM_EXPORT void SetLastCheck(Utf8CP time);

    //! Get/Set
    //! The last time the server was checked and was on-line.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastTimeOnline() const;
    REALITYDATAPLATFORM_EXPORT void SetLastTimeOnline(DateTimeCR time);
    REALITYDATAPLATFORM_EXPORT void SetLastTimeOnline(Utf8CP time);

    //! Get/Set
    //! Latency of server in seconds if known. A value of zero means unknown. A negative value is invalid.
    REALITYDATAPLATFORM_EXPORT double GetLatency() const;
    REALITYDATAPLATFORM_EXPORT void SetLatency(double latency);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetState() const;
    REALITYDATAPLATFORM_EXPORT void SetState(Utf8CP state);

    //! Get/Set
    //! Indicates if the server serves data meant to be streamed or downloaded. 
    //! This is important to the client in order to either download or not the datasource.
    REALITYDATAPLATFORM_EXPORT bool IsStreamed() const;
    REALITYDATAPLATFORM_EXPORT void SetStreamed(bool streamed);

    //! Get/Set
    //! A key indicating what kind of authentication is required to access the server. 
    //! If no authentication is required, then the field must remain empty. 
    //! For example, the USGS Earth Explorer server requires an Earth Explorer username/password be provided so 
    //! the key ‘EarthExplorer’ will be used. Any Bentley CONNECT service requires a CONNECT authentication so the 
    //! key ‘BentleyCONNECT’ will be indicated. If the authentication is required but unknown 
    //! the ‘UNKNOWN’ should be used.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLoginKey() const;
    REALITYDATAPLATFORM_EXPORT void SetLoginKey(Utf8CP loginKey);

    //! Get/Set
    //! If the LoginKey is unknown by the application, then the present field can be used to determine if the method
    //! to pass authentication is standard or not. Most HTTP sites rely upon a common mechanism 
    //! to transport the authentication token (in the HTTP header). If this is the case, then STANDARD will 
    //! be indicated. Other sites require the token be provided otherwise. In such case 
    //! the method will be indicated CUSTOM.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLoginMethod() const;
    REALITYDATAPLATFORM_EXPORT void SetLoginMethod(Utf8CP loginMethod);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRegistrationPage() const;
    REALITYDATAPLATFORM_EXPORT void SetRegistrationPage(Utf8CP registrationPage);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOrganisationPage() const;
    REALITYDATAPLATFORM_EXPORT void SetOrganisationPage(Utf8CP organisationPage);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFees() const;
    REALITYDATAPLATFORM_EXPORT void SetFees(Utf8CP fees);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetAccessConstraints() const;
    REALITYDATAPLATFORM_EXPORT void SetAccessConstraints(Utf8CP accessConstraints);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT uint64_t GetMeanReachabilityStats() const;
    REALITYDATAPLATFORM_EXPORT void SetMeanReachabilityStats(uint64_t meanReachabilityStats);

protected:
    SpatialEntityServer();
    SpatialEntityServer(Utf8CP url, Utf8CP name);

    Utf8String  m_id;
    Utf8String  m_protocol;
    Utf8String  m_name;
    Utf8String  m_url;
    Utf8String  m_contactInfo;
    Utf8String  m_legal;
    bool        m_online;
    DateTime    m_lastCheck;
    DateTime    m_lastTimeOnline;
    double      m_latency;
    Utf8String  m_state;
    Utf8String  m_type;
    Utf8String  m_loginKey;
    Utf8String  m_loginMethod;
    bool        m_streamed;
    
    Utf8String  m_registrationPage; 
    Utf8String  m_organisationPage;
    Utf8String  m_fees;
    Utf8String  m_accessConstraints;
    uint64_t    m_meanReachabilityStats;
    };

struct RealityDataSourceSerializer;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! A Spatial entity data source defines the storage location of a spatial entity.
//! In conjunction with the server related, it provides all the information
//! necessary to access the data.
//! As a commodity it may also contain information that may be missing from the 
//! source file such as the 'No Data Value' or the 'Geographic Coordinate System'
//! that may be documented but not automatically extracted from the source file.
//=====================================================================================
struct SpatialEntityDataSource : public RefCountedBase
    {
public:
    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Create();
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Create(Utf8CP uri, Utf8CP type);
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDataSourcePtr Create(UriR uri, Utf8CP type);

    //! Get/Set
    //! The Id of the data source
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8CP id);

    //! Get/Set the source location. It could be a fully qualified URL or a relative path to the package file. Required attribute.
    //! ex: "http://www.Bentley.com/logo.jpg"
    //!     "./imagery/road.jpg"
    REALITYDATAPLATFORM_EXPORT UriCR        GetUri() const;
    REALITYDATAPLATFORM_EXPORT void         SetUri(UriR uri);

    //! Get/Set
    //! A formatted string indicating the geographic coordinate system used by the data source. This field 
    //! is mainly used for files that do not support the storage of the geographic coordinate system or for 
    //! file which the geographic coordinate system was omitted. This string takes the form of a key as 
    //! defined in the Bentley Systems main Geographic Coordinate system registry. Optionally 
    //! separated by a semi-colon the Vertical data key can be added.
    //! Example: UTM84-16N;GEOID refers that the UTM zone 16 North on WGS84 with elevations expressed as Geoid height.
    //! Possible value for the vertical datum are: ELLIPSOID, GEOID, NGVD29 and NAVD88 only at the moment.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGeoCS() const;
    REALITYDATAPLATFORM_EXPORT void SetGeoCS(Utf8CP geoCS);

    //! Get/Set
    //! A key indicating the compound document type. If the URL points to the datasource file directly
    //! it should be null or empty. Typical value would be ‘zip’ for a zip file.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCompoundType() const;
    REALITYDATAPLATFORM_EXPORT void SetCompoundType(Utf8CP type);

    //! Get/Set
    //! The size in kilobytes of the data source file.
    REALITYDATAPLATFORM_EXPORT uint64_t GetSize() const; // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetSize(uint64_t size); // in bytes.
    REALITYDATAPLATFORM_EXPORT void SetSize(Utf8CP size);

    //! Get/Set
    //! A string containing the description of the sample value of the data source that should be considered 
    //! as an absence of data instead of a value. This field is very important to recognize the padding
    //! of rotated raster such as those originating from satellites. A pure black padding is then added 
    //! and the format guarantees that the meaningful information will not use this value. The interpretation 
    //! of this value depends on the file format and structure but for grayscale image a single integer 
    //! number is expected (0 for black; 255 or 65536 for pure white). For multi-channel images such as 
    //! color the value will be indicated by placing a value for all channels separated by semi-colons.
    //! For example:
    //! The value 0 means black for a greyscale image, while the value 0;0;0 would mean black for a 
    //! RGB 3-channel color image, the value -9999.0 could indicate no data for a floating-point grid such as DEM files.
    //! If there are less numbers than channels then the number is duplicated for other channels (0 would mean 0;0;0 for a RGB raster).
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetNoDataValue() const;
    REALITYDATAPLATFORM_EXPORT void SetNoDataValue(Utf8CP value); 

    //! Get/Set
    //! The type of the source file. It is usually the extension of the data source 
    //! file such as ‘tif’, ‘dem’, ‘hgt’ and so on.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

    //! Get/Set
    //! The name of the main data source file within the compound. If the data source is no a compound, then
    //! this field must be empty.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLocationInCompound() const;
    REALITYDATAPLATFORM_EXPORT void SetLocationInCompound(Utf8CP location);

    //! Get/Set
    //! The Coordinate System
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCoordinateSystem() const;
    REALITYDATAPLATFORM_EXPORT void SetCoordinateSystem(Utf8CP coordSys);

    REALITYDATAPLATFORM_EXPORT 	long int GetServerId() const;
    //serverId is a mutable value so that it can be set on a const ref, before performing a Save()
    REALITYDATAPLATFORM_EXPORT void SetServerId(long int id ) const;

    //! Get/Set
    //! Since the server is an optional field of the data source it is expressed as a pointer
    //! The pointer returned or given can be null
    REALITYDATAPLATFORM_EXPORT SpatialEntityServerCP GetServerCP() const;
    REALITYDATAPLATFORM_EXPORT SpatialEntityServerP GetServerP();
    REALITYDATAPLATFORM_EXPORT void SetServer(SpatialEntityServerPtr server);

    //! Get/Set
    //! A string containing a key to the data provider. Keys are informally attributed and must 
    //! be representative of the source of the data (not the file distribution organism). 
    //! Current values introduced include ‘USGS’, ‘GeoGratis’. Once attributed to a dataset these 
    //! should be maintained constant. The value is useful for discriminating data eccentricities 
    //! based on the provider. For example, many JP2 Imagery files from USGS do not contain the Geographic 
    //! Coordinate System definition (probable oversight from USGS). Since these JP2 files all use 
    //! the ‘WebMercator’ coordinate system the application could make use of the data provider field 
    //! to impose this coordinate system for JP2 files from this provider only.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
    REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

    //! Get/Set
    //! A name for the provider of the data. It can be identical to the DataProvider key or 
    //! a more elaborate name for human interpretation. For example, the DataProvider GeoGratis 
    //! can be associated with the provider name ‘Government of Canada’.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProviderName() const;
    REALITYDATAPLATFORM_EXPORT void SetProviderName(Utf8CP providerName);

    //! Get/Set
    //! A reference to a metadata object. This object can be used by many spatial entities.
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataCP GetMetadataCP() const;
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataPtr GetMetadataP();
    REALITYDATAPLATFORM_EXPORT void SetMetadata(SpatialEntityMetadataPtr metadata);

    //! Get/Set files or documents that are required for the proper interpretation of the source. The sister file 
    //! location is specified using the same pattern as the URI of the source. Might be empty.
    REALITYDATAPLATFORM_EXPORT const bvector<UriPtr>&     GetSisterFiles() const;
    REALITYDATAPLATFORM_EXPORT void                       SetSisterFiles(const bvector<UriPtr>& sisterFiles);

    //! Get the element name.
    REALITYDATAPLATFORM_EXPORT Utf8CP GetElementName() const;

protected:
    SpatialEntityDataSource();
    SpatialEntityDataSource(UriR uri, Utf8CP type);
    SpatialEntityDataSource(Utf8CP uri, Utf8CP type);

    // Must be re-implemented by each child class. Used by serialization.
    virtual Utf8CP _GetElementName() const;

    Utf8String m_id;
    UriPtr m_pUri;
    Utf8String m_geoCS;
    Utf8String m_compoundType;
    uint64_t m_size = 0;
    Utf8String m_dataType;
    Utf8String m_locationInCompound;
    SpatialEntityServerPtr m_pServer;
    Utf8String m_noDataValue;
    Utf8String m_coordinateSystem;

    Utf8String m_provider;
    Utf8String m_providerName;
    SpatialEntityMetadataPtr m_pMetadata;
    bvector<UriPtr> m_sisterFiles;

    mutable long int m_serverId = -1; //SQLINTEGER long int
};

//=====================================================================================
//! This is a descendant of source type that allows specification of wms data. The WMS 
//! source is fully qualified since in addition to the server URL, it contains the list 
//! of layers, styles, coordinate system, selected delivery format, transparency setting.
//! The WMS source can be seen as raster except requests sent to the server require 
//! variable parameter definition based on content of WMS settings and additional 
//! location of request.
//! @bsiclass                                   Jean-Francois.Cote              06/2015
//=====================================================================================
struct WmsDataSource : public SpatialEntityDataSource
    {
    DEFINE_T_SUPER(SpatialEntityDataSource)

public:
    friend struct RealityDataSourceSerializer;

    REALITYDATAPLATFORM_EXPORT static WmsDataSourcePtr Create(Utf8CP uri);
    REALITYDATAPLATFORM_EXPORT static WmsDataSourcePtr Create(UriR uri);

    //! Get/Set the WMS specific map settings.
    //! The string used here should represent a xml fragment containing all the nodes/infos required for WMS processing.
    //! You can take a look at PublicApi/RealityPlatform/WmsSource.h for more details on the structure of a WmsMapSettings object.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR  GetMapSettings() const;
    REALITYDATAPLATFORM_EXPORT void          SetMapSettings(Utf8CP mapSettings);

protected:
    WmsDataSource() {}
    WmsDataSource(UriR uri);
    virtual ~WmsDataSource();

    virtual Utf8CP _GetElementName() const;

private:
    Utf8String m_mapSettings;
    };

//=====================================================================================
//! This is a descendant of source type that allows specification of Open Street Map data.
//! The OSM source is fully qualified since in addition to the server URL it contains
//! the protocol identifier and the coordinates of the region designated. It fully represents 
//! an OSM data blob/file. It also allows specification of alternate URLs to alternate OSM 
//! source representing the same region and the same stream format. Many alternate URLs 
//! can be specified.
//! 
//! This class is deprecated and is only used to serialize or 
//! deserialized version 1.0 of the package.
//!
//! @bsiclass                                   Jean-Francois.Cote              06/2015
//=====================================================================================
struct OsmDataSource : public SpatialEntityDataSource
    {
    DEFINE_T_SUPER(SpatialEntityDataSource)

public:
    friend struct RealityDataSourceSerializer;

    REALITYDATAPLATFORM_EXPORT static OsmDataSourcePtr Create(Utf8CP uri, DRange2dCP bbox);

    //! Get/Set the OSM specific resources.
    //! The string used here should represent a xml fragment containing all the nodes/infos required for OSM processing.
    //! You can take a look at PublicApi/RealityPlatform/OsmSource.h for more details on the structure of an OsmResource object.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR  GetOsmResource() const;
    REALITYDATAPLATFORM_EXPORT void          SetOsmResource(Utf8CP osmResource);

protected:
    OsmDataSource() {}
    OsmDataSource(Utf8CP uri);
    virtual ~OsmDataSource();

    virtual Utf8CP _GetElementName() const;

private:
    Utf8String m_osmResource;
    };

//=====================================================================================
//! This is a descendant of source type that allows specification of multi band raster 
//! data. Normally it does not apply to any other type. When the source is of this type 
//! then the attribute 'uri' of the source should contain either one of the bands, 
//! ideally the panchromatic if present.
//! @bsiclass                                   Jean-Francois.Cote              06/2015
//=====================================================================================
struct MultiBandSource : public SpatialEntityDataSource
    {
    DEFINE_T_SUPER(SpatialEntityDataSource)

public:
    friend struct RealityDataSourceSerializer;

    REALITYDATAPLATFORM_EXPORT static MultiBandSourcePtr Create();
    REALITYDATAPLATFORM_EXPORT static MultiBandSourcePtr Create(UriR uri, Utf8CP type);
    
    //! Get/Set the multiband URLs. If the data source is a compound document then the bands must be located
    //! in files located in the compound docuemnt otherwise the four values point to external Urls.
    //! We only support four types of bands for multiband sources red, green, blue and panchromatic. The last one is optional
    //! since all information may be contained in the red, green, blue bands.
    REALITYDATAPLATFORM_EXPORT void GetMultibandUrls(Utf8String& redUrl, Utf8String& greenUrl, Utf8String& blueUrl, Utf8String& panchromaticUrl ) const;
    REALITYDATAPLATFORM_EXPORT void SetMultibandUrls(Utf8StringCR redUrl, Utf8StringCR greenUrl, Utf8StringCR blueUrl, Utf8StringCR panchromaticUrl);

    //! Get/Set
    //! The size in kilobytes of the red band or zero if unknown
    REALITYDATAPLATFORM_EXPORT uint64_t GetRedBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetRedBandSize(uint64_t size);

    //! Get/Set
    //! The size in kilobytes of the green band or zero if unknown
    REALITYDATAPLATFORM_EXPORT uint64_t GetGreenBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetGreenBandSize(uint64_t size);

    //! Get/Set
    //! The size in kilobytes of the blue band or zero if unknown
    REALITYDATAPLATFORM_EXPORT uint64_t GetBlueBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetBlueBandSize(uint64_t size);

    //! Get/Set
    //! The size in kilobytes of the panchromatic band or zero if unknown
    REALITYDATAPLATFORM_EXPORT uint64_t GetPanchromaticBandSize() const;
    REALITYDATAPLATFORM_EXPORT void SetPanchromaticBandSize(uint64_t size);

    //! Get/Set the red band. A band essentially contains a source for the band data. Required.
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCP GetRedBand() const;
    REALITYDATAPLATFORM_EXPORT void SetRedBand(SpatialEntityDataSourceR band);

    //! Get/Set the green band. A band essentially contains a source for the band data. Required.
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCP GetGreenBand() const;
    REALITYDATAPLATFORM_EXPORT void SetGreenBand(SpatialEntityDataSourceR band);

    //! Get/Set the green band. A band essentially contains a source for the band data. Required.
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCP GetBlueBand() const;
    REALITYDATAPLATFORM_EXPORT void SetBlueBand(SpatialEntityDataSourceR band);

    //! Get/Set the panchromatic band. A band essentially contains a source for the band data. Optional.
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCP GetPanchromaticBand() const;
    REALITYDATAPLATFORM_EXPORT void SetPanchromaticBand(SpatialEntityDataSourceR band);

protected:
    MultiBandSource() {}
    MultiBandSource(UriR uri, Utf8CP type);
    virtual ~MultiBandSource() {}

    virtual Utf8CP _GetElementName() const;

private:
    SpatialEntityDataSourcePtr m_pRedBand;
    SpatialEntityDataSourcePtr m_pGreenBand;
    SpatialEntityDataSourcePtr m_pBlueBand;
    SpatialEntityDataSourcePtr m_pPanchromaticBand;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! The central class of the Spatial Entity model. It represents a spatial
//! geocoordinated data. It contains all fields necessary to represent the spatial
//! entity except the actual data which is stored in the data sources the spatial
//! entity makes references to.
//=====================================================================================
struct SpatialEntity : public RealityDataBase
{
public:
    friend RealityDataSerializer;

    //! Create invalid data.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Create();

    // Creator for spatio temporal selector ... fills out some of the most basic fields
    REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr Create(Utf8StringCR identifier, const DateTime& date, Utf8StringCR resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name = "", Utf8StringCR coordSys = "4326");

    //! Get/Set
    //! A string containing a key to the data provider. Keys are informally attributed and must 
    //! be representative of the source of the data (not the file distribution organism). 
    //! Current values introduced include ‘USGS’, ‘GeoGratis’. Once attributed to a dataset these 
    //! should be maintained constant. The value is useful for discriminating data eccentricities 
    //! based on the provider. For example, many JP2 Imagery files from USGS do not contain the Geographic 
    //! Coordinate System definition (probable oversight from USGS). Since these JP2 files all use 
    //! the ‘WebMercator’ coordinate system the application could make use of the data provider field 
    //! to impose this coordinate system for JP2 files from this provider only.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
    REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

    //! Get/Set
    //! A name for the provider of the data. It can be identical to the DataProvider key or 
    //! a more elaborate name for human interpretation. For example, the DataProvider GeoGratis 
    //! can be associated with the provider name ‘Government of Canada’.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProviderName() const;
    REALITYDATAPLATFORM_EXPORT void SetProviderName(Utf8CP providerName);

    //! Get/Set
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

    //! Get/Set
    //! URL to the thumbnail
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
    //! The date of the production of the data or an invalid date if it is unknown.
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
    REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

    //! Get/Set
    //! The list of data sources that contain the data of the spatial entity
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceCR GetDataSource(size_t index) const;
    REALITYDATAPLATFORM_EXPORT SpatialEntityDataSourceR GetDataSource(size_t index);
    REALITYDATAPLATFORM_EXPORT void AddDataSource(SpatialEntityDataSourceR dataSource);
    REALITYDATAPLATFORM_EXPORT size_t GetDataSourceCount() const;

    //! Get/Set
    //! A reference to a metadata object. This object can be used by many spatial entities.
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataCP GetMetadataCP() const;
    REALITYDATAPLATFORM_EXPORT SpatialEntityMetadataP GetMetadataP();
    REALITYDATAPLATFORM_EXPORT void SetMetadata(SpatialEntityMetadataPtr metadata);
  
    // Get/Set
    //! A value in percentage between 0 and 100 of the occlusion of the data. This usually means the cloud 
    //! cover for satellite imagery but could be the result of other occlusion 
    //! types (trees preventing radar samples for ground detection…). 
    //! A negative value means UNKNOWN.
    REALITYDATAPLATFORM_EXPORT float GetOcclusion() const;
    REALITYDATAPLATFORM_EXPORT void SetOcclusion( float cover );

    //! Get package version.
    REALITYDATAPLATFORM_EXPORT uint32_t GetMajorVersion() const;
    REALITYDATAPLATFORM_EXPORT void SetMajorVersion(uint32_t major);
    REALITYDATAPLATFORM_EXPORT uint32_t GetMinorVersion() const;
    REALITYDATAPLATFORM_EXPORT void SetMinorVersion(uint32_t minor);

protected:
    SpatialEntity();

    float m_occlusion;
    mutable Utf8String m_provider;
    mutable Utf8String m_providerName;
    Utf8String m_dataType;
    DateTime m_date;
    Utf8String m_thumbnailURL;
    Utf8String m_thumbnailLoginKey;
    mutable SpatialEntityMetadataPtr m_pMetadata;
    bvector<SpatialEntityDataSourcePtr> m_DataSources;

    Utf8String m_description;
    uint64_t m_approximateFileSize;
    uint32_t m_majorVersion;
    uint32_t m_minorVersion;
    }; 
   
END_BENTLEY_REALITYPLATFORM_NAMESPACE