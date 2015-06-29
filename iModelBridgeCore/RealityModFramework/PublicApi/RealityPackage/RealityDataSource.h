/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityDataSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

    REALITYPACKAGE_EXPORT static RealityDataSourcePtr Create(WCharCP uri, WCharCP type);

    //! Get/Set the source uri. It could be a full URL or a path relative to the package file.
    //! ex: "http://www.Bentley.com/logo.jpg"
    //!     "./imagery/road.jpg"
    REALITYPACKAGE_EXPORT WStringCR GetUri() const;
    REALITYPACKAGE_EXPORT void SetUri(WCharCP uri);

    //! Get/Set The source type.
    REALITYPACKAGE_EXPORT WStringCR GetType() const;
    REALITYPACKAGE_EXPORT void SetType(WCharCP type);
   
protected:
    RealityDataSource(){}
    RealityDataSource(WCharCP uri, WCharCP type);
    virtual ~RealityDataSource();

    // Must be re-implemented by each child class.  Used by serialization.
    virtual Utf8CP _GetElementName() const;

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

private:
    WString m_uri;
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

    REALITYPACKAGE_EXPORT static WmsDataSourcePtr Create(WCharCP uri);

    //! Get/Set The source data.
    //&&JFC: Document the fact that we expect a xml fragment here and write tests for these methods.
    REALITYPACKAGE_EXPORT WStringCR GetMapInfo() const;
    REALITYPACKAGE_EXPORT void      SetMapInfo(WCharCP mapInfo);

protected:
    WmsDataSource(){}
    WmsDataSource(WCharCP uri);
    virtual ~WmsDataSource();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataSourceNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataSourceNode) const;

    virtual Utf8CP _GetElementName() const;

private:
    WString m_mapInfo;
};

END_BENTLEY_REALITYPACKAGE_NAMESPACE

