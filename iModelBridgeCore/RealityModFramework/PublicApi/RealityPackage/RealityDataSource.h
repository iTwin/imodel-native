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

    REALITYPACKAGE_EXPORT static RealityDataSourcePtr Create(Utf8CP uri, WCharCP type);

    //! Get/Set the source uri. It could be a full URL or a path relative to the package file.
    //! ex: "http://www.Bentley.com/logo.jpg"
    //!     "./imagery/road.jpg"
    REALITYPACKAGE_EXPORT Utf8StringCR GetUri() const;
    REALITYPACKAGE_EXPORT void         SetUri(Utf8CP uri);

    //! Get/Set The source type.
    REALITYPACKAGE_EXPORT WStringCR GetType() const;
    REALITYPACKAGE_EXPORT void SetType(WCharCP type);
   
protected:
    RealityDataSource(){}
    RealityDataSource(Utf8CP uri, WCharCP type);
    virtual ~RealityDataSource();

    // Must be re-implemented by each child class.  Used by serialization.
    virtual Utf8CP _GetElementName() const;

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

private:
    Utf8String m_uri;
    WString m_type;     
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
    //! You can take a look at PublicApi/RealityPlatform/WMSSource.h for more details on the structure of a MapInfo object.
    REALITYPACKAGE_EXPORT Utf8StringCR  GetMapInfo() const;
    REALITYPACKAGE_EXPORT void          SetMapInfo(Utf8CP mapInfo);

protected:
    WmsDataSource(){}
    WmsDataSource(Utf8CP uri);
    virtual ~WmsDataSource();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

    virtual Utf8CP _GetElementName() const;

private:
    Utf8String m_mapInfo;
};

//=======================================================================================
//! Base class for data source.
//! @bsiclass
//=======================================================================================
struct CompoundDataSource : public RealityDataSource
    {
    DEFINE_T_SUPER(RealityDataSource)

    public:
        friend struct RealityDataSourceSerializer;

        REALITYPACKAGE_EXPORT static CompoundDataSourcePtr Create(Utf8CP uri, WCharCP type);

        //! Get/Set The source data.
        //! The string used here should represent a xml fragment containing all the nodes/infos required for Usgs processing.
        //! You can take a look at PublicApi/RealityPackage/UsgsDataSource.h for more details on the structure of a UsgsData object.
        REALITYPACKAGE_EXPORT Utf8StringCR  Get() const;
        REALITYPACKAGE_EXPORT void          Set(Utf8CP data);

    protected:
        CompoundDataSource() {}
        CompoundDataSource(Utf8CP uri, WCharCP type);
        virtual ~CompoundDataSource();

        virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
        virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

        virtual Utf8CP _GetElementName() const;

    private:
        Utf8String m_data;
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE

