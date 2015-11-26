/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityDataSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPackage/RealityPackage.h>
#include <BeXml/BeXml.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

struct RealityDataSourceSerializer;

//=======================================================================================
//! Base class for all reality data source.
//! @bsiclass
//=======================================================================================
struct RealityDataSource : public RefCountedBase
{
public:
    friend struct RealityDataSourceSerializer; 

    REALITYPACKAGE_EXPORT static RealityDataSourcePtr Create(Utf8CP uri, Utf8CP type);

    //! Get/Set the source uri. It could be a full URL or a path relative to the package file.
    //! ex: "http://www.Bentley.com/logo.jpg"
    //!     "./imagery/road.jpg"
    REALITYPACKAGE_EXPORT Utf8StringCR GetUri() const;
    REALITYPACKAGE_EXPORT void         SetUri(Utf8CP uri);

    //! Get/Set the source type.
    REALITYPACKAGE_EXPORT Utf8StringCR GetType() const;
    REALITYPACKAGE_EXPORT void         SetType(Utf8CP type);

    //! Get/Set the copyright. Might be empty.
    REALITYPACKAGE_EXPORT Utf8StringCR GetCopyright() const;
    REALITYPACKAGE_EXPORT void         SetCopyright(Utf8CP copyright);

    //! Get/Set the provider. Might be empty.
    REALITYPACKAGE_EXPORT Utf8StringCR GetProvider() const;
    REALITYPACKAGE_EXPORT void         SetProvider(Utf8CP provider);

    //! Get/Set the filesize in kilobytes. Default to 0.
    REALITYPACKAGE_EXPORT uint64_t GetFilesize() const;
    REALITYPACKAGE_EXPORT void     SetFilesize(uint64_t filesizeInKB);

    //! Get/Set the full path for the main file in a compound type. Might be empty.
    REALITYPACKAGE_EXPORT Utf8StringCR GetFileInCompound() const;
    REALITYPACKAGE_EXPORT void         SetFileInCompound(Utf8CP filename);

    //! Get/Set the metadata. Might be empty.
    REALITYPACKAGE_EXPORT Utf8StringCR GetMetadata() const;
    REALITYPACKAGE_EXPORT void         SetMetadata(Utf8CP metadata);

    //! Get/Set the sister files. Might be empty.
    REALITYPACKAGE_EXPORT const bvector<Utf8String>& GetSisterFiles() const;
    REALITYPACKAGE_EXPORT void                       SetSisterFiles(const bvector<Utf8String>& sisterFiles);
   
protected:
    RealityDataSource(){}
    RealityDataSource(Utf8CP uri, Utf8CP type);
    virtual ~RealityDataSource();

    // Must be re-implemented by each child class.  Used by serialization.
    virtual Utf8CP _GetElementName() const;

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

private:
    Utf8String m_uri;
    Utf8String m_type;
    Utf8String m_copyright;
    Utf8String m_provider;
    uint64_t m_filesize;
    Utf8String m_fileInCompound;
    Utf8String m_metadata;
    bvector<Utf8String> m_sisterFiles;
};

//=======================================================================================
//! Base class for data source.
//! @bsiclass
//=======================================================================================
struct WmsDataSource : public RealityDataSource
{
    DEFINE_T_SUPER(RealityDataSource)

public:
    friend struct RealityDataSourceSerializer; 

    REALITYPACKAGE_EXPORT static WmsDataSourcePtr Create(Utf8CP uri);

    //! Get/Set The source data.
    //! The string used here should represent a xml fragment containing all the nodes/infos required for WMS processing.
    //! You can take a look at PublicApi/RealityPlatform/WmsSource.h for more details on the structure of a WmsMapSettings object.
    REALITYPACKAGE_EXPORT Utf8StringCR  GetMapSettings() const;
    REALITYPACKAGE_EXPORT void          SetMapSettings(Utf8CP mapSettings);

protected:
    WmsDataSource(){}
    WmsDataSource(Utf8CP uri);
    virtual ~WmsDataSource();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

    virtual Utf8CP _GetElementName() const;

private:
    Utf8String m_mapSettings;
};

//=======================================================================================
//! Base class for data source.
//! @bsiclass
//=======================================================================================
struct OsmDataSource : public RealityDataSource
    {
    DEFINE_T_SUPER(RealityDataSource)

    public:
        friend struct RealityDataSourceSerializer;

        REALITYPACKAGE_EXPORT static OsmDataSourcePtr Create(Utf8CP uri, DRange2dCP bbox);

        //! Get/Set The source data.
        //! The string used here should represent a xml fragment containing all the nodes/infos required for OSM processing.
        //! You can take a look at PublicApi/RealityPlatform/OsmSource.h for more details on the structure of a OsmResource object.
        REALITYPACKAGE_EXPORT Utf8StringCR  GetOsmResource() const;
        REALITYPACKAGE_EXPORT void          SetOsmResource(Utf8CP osmResource);

    protected:
        OsmDataSource() {}
        OsmDataSource(Utf8CP uri);
        virtual ~OsmDataSource();

        virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
        virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

        virtual Utf8CP _GetElementName() const;

    private:
        Utf8String m_osmResource;
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE

