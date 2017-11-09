/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataSource.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeXml/BeXml.h>
#include <Geom/GeomApi.h>
    
BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct RealityDataSourceSerializer;

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
//! Base class representing a source of data. The data must be of the type of the group 
//! it belongs to. Different strategy can be used to identify and process compound 
//! documents of this type but the most important indication of the content is the 
//! extension of the URI, the present type and the identification of the provider.
//! @bsiclass                                   Jean-Francois.Cote              06/2015
//=====================================================================================
struct RealityDataSource : public RefCountedBase
    {
    public:
        friend struct RealityDataSourceSerializer; 

        // Create source with required attributes.
        REALITYDATAPLATFORM_EXPORT static RealityDataSourcePtr Create(Utf8CP uri, Utf8CP type);
        REALITYDATAPLATFORM_EXPORT static RealityDataSourcePtr Create(UriR uri, Utf8CP type);

        //! Get/Set the source location. It could be a fully qualified URL or a relative path to the package file. Required attribute.
        //! ex: "http://www.Bentley.com/logo.jpg"
        //!     "./imagery/road.jpg"
        REALITYDATAPLATFORM_EXPORT UriCR        GetUri() const;
        REALITYDATAPLATFORM_EXPORT void         SetUri(UriR uri);

        //! Get/Set the type of the final data. Required attribute.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetType() const;
        REALITYDATAPLATFORM_EXPORT void         SetType(Utf8CP type);

        //! Get/Set if the uri refers to a streamed source. Optional attribute.
        REALITYDATAPLATFORM_EXPORT bool         IsStreamed() const;
        REALITYDATAPLATFORM_EXPORT void         SetStreamed(bool isStreamed);

        //! Get/Set the identifier. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
        REALITYDATAPLATFORM_EXPORT void         SetId(Utf8CP id);

        //! Get/Set the copyright notice. It can contain text or links to external documentation. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCopyright() const;
        REALITYDATAPLATFORM_EXPORT void         SetCopyright(Utf8CP copyright);       

        //! Get/Set the term of use. It can contain text or links to external documentation. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetTermOfUse() const;
        REALITYDATAPLATFORM_EXPORT void         SetTermOfUse(Utf8CP termOfUse);   

        //! Get/Set the data provider. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
        REALITYDATAPLATFORM_EXPORT void         SetProvider(Utf8CP provider);

        //! Get/Set the server authentication to access the data through the URI.
        //! The key string additionally provides a hint about the login/password required. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerLoginKey() const;
        REALITYDATAPLATFORM_EXPORT void         SetServerLoginKey(Utf8CP key);
        
        //! Get/Set the server authentication to access the data through the URI.
        //! Can be used to determine how login can be performed. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerLoginMethod() const;
        REALITYDATAPLATFORM_EXPORT void         SetServerLoginMethod(Utf8CP method);
        
        //! Get/Set the server authentication to access the data through the URI.
        //! A web page link to the registration page for user obtention of login credentials on the server. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerRegistrationPage() const;
        REALITYDATAPLATFORM_EXPORT void         SetServerRegistrationPage(Utf8CP link);
        
        //! Get/Set the server authentication to access the data through the URI.
        //! A web page link to the organization page hosting the server. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerOrganisationPage() const;
        REALITYDATAPLATFORM_EXPORT void         SetServerOrganisationPage(Utf8CP link);

        //! Get/Set the size in kilobytes. This value can be approximative. Might be empty.
        REALITYDATAPLATFORM_EXPORT uint64_t     GetSize() const;
        REALITYDATAPLATFORM_EXPORT void         SetSize(uint64_t sizeInKB);

        //! Get/Set the metadata. Contains a fully qualified URI that may refer to 
        //! an external document or to a file part of the compound document. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadata() const;
        REALITYDATAPLATFORM_EXPORT void         SetMetadata(Utf8CP metadata);

        //! Get/Set the type of the metadata. FGDC and ISO-19115 are the only two formats supported. All other 
        // formats will be indicated as TEXT to be interpreted by a human. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadataType() const;
        REALITYDATAPLATFORM_EXPORT void         SetMetadataType(Utf8CP metadataType);

        //! Get/Set the geographic coordinate system keyname to be used as default for the source given. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGeoCS() const;
        REALITYDATAPLATFORM_EXPORT void         SetGeoCS(Utf8CP geoCS);

        //! Get/Set the no data value if it is not integrally part of the format. Might be empty.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetNoDataValue() const;
        REALITYDATAPLATFORM_EXPORT void         SetNoDataValue(Utf8CP nodatavalue);

        //! Get/Set files or documents that are required for the proper interpretation of the source. The sister file 
        //! location is specified using the same pattern as the URI of the source. Might be empty.
        REALITYDATAPLATFORM_EXPORT const bvector<UriPtr>&     GetSisterFiles() const;
        REALITYDATAPLATFORM_EXPORT void                       SetSisterFiles(const bvector<UriPtr>& sisterFiles);   

        //! Get the element name.
        REALITYDATAPLATFORM_EXPORT Utf8CP GetElementName() const;
   
    protected:
        RealityDataSource(){}
        RealityDataSource(Utf8CP uri, Utf8CP type);
        RealityDataSource(UriR uri, Utf8CP type);
        virtual ~RealityDataSource();

        // Must be re-implemented by each child class. Used by serialization.
        virtual Utf8CP _GetElementName() const;

    private:
        UriPtr m_pUri;
        Utf8String m_type;
        bool m_streamed;
        Utf8String m_copyright;
        Utf8String m_termOfUse;
        Utf8String m_id;
        Utf8String m_provider;
        Utf8String m_serverLoginKey;
        Utf8String m_serverLoginMethod;
        Utf8String m_serverRegistrationPage;
        Utf8String m_serverOrganisationPage;
        uint64_t m_size;
        Utf8String m_metadata;
        Utf8String m_metadataType;
        Utf8String m_geocs;
        Utf8String m_nodatavalue;
        bvector<UriPtr> m_sisterFiles;
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
struct WmsDataSource : public RealityDataSource
    {
    DEFINE_T_SUPER(RealityDataSource)

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
        WmsDataSource(){}
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
struct OsmDataSource : public RealityDataSource
    {
    DEFINE_T_SUPER(RealityDataSource)

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
struct MultiBandSource : public RealityDataSource
    {
        DEFINE_T_SUPER(RealityDataSource)

    public:
        friend struct RealityDataSourceSerializer;

        REALITYDATAPLATFORM_EXPORT static MultiBandSourcePtr Create(UriR uri, Utf8CP type);

        //! Get/Set the red band. A band essentially contains a source for the band data. Required.
        REALITYDATAPLATFORM_EXPORT RealityDataSourceCP GetRedBand() const;
        REALITYDATAPLATFORM_EXPORT void SetRedBand(RealityDataSourceR band);

        //! Get/Set the green band. A band essentially contains a source for the band data. Required.
        REALITYDATAPLATFORM_EXPORT RealityDataSourceCP GetGreenBand() const;
        REALITYDATAPLATFORM_EXPORT void SetGreenBand(RealityDataSourceR band);

        //! Get/Set the green band. A band essentially contains a source for the band data. Required.
        REALITYDATAPLATFORM_EXPORT RealityDataSourceCP GetBlueBand() const;
        REALITYDATAPLATFORM_EXPORT void SetBlueBand(RealityDataSourceR band);

        //! Get/Set the panchromatic band. A band essentially contains a source for the band data. Optional.
        REALITYDATAPLATFORM_EXPORT RealityDataSourceCP GetPanchromaticBand() const;
        REALITYDATAPLATFORM_EXPORT void SetPanchromaticBand(RealityDataSourceR band);

    protected:
        MultiBandSource() {}
        MultiBandSource(UriR uri, Utf8CP type);
        virtual ~MultiBandSource();

        virtual Utf8CP _GetElementName() const;

    private:
        RealityDataSourcePtr m_pRedBand;
        RealityDataSourcePtr m_pGreenBand;
        RealityDataSourcePtr m_pBlueBand;
        RealityDataSourcePtr m_pPanchromaticBand;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE

