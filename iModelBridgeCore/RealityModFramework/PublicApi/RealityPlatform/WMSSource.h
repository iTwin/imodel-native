/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WMSSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityPlatformAPI.h>
#include <Bentley/bvector.h>
#include <Bentley/WString.h>

#define WMSSOURCE_PREFIX                        "wms"
#define WMSSOURCE_ELEMENT_Root                  "MapInfo"
#define WMSSOURCE_ELEMENT_Url                   "Url"
#define WMSSOURCE_ELEMENT_Version               "Version"
#define WMSSOURCE_ELEMENT_Layers                "Layers"
#define WMSSOURCE_ELEMENT_Styles                "Styles"
#define WMSSOURCE_ELEMENT_CoordinateSystem      "CoordinateSystem"
#define WMSSOURCE_ELEMENT_Format                "Format"
#define WMSSOURCE_ATTRIBUTE_CoordSysType        "type"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               6/2015
//=====================================================================================
struct WmsSource : public RefCountedBase
    {
    public:
        REALITYDATAPLATFORM_EXPORT static WmsSourcePtr Create();

        REALITYDATAPLATFORM_EXPORT StatusInt Add(MapInfoPtr mapInfo);
        REALITYDATAPLATFORM_EXPORT StatusInt Add(WCharCP url, WCharCP version, WCharCP layers, WCharCP styles, WCharCP crs, WCharCP format);

        REALITYDATAPLATFORM_EXPORT WString ToXmlString();

    private:
        //WMSSource();
        //~WMSSource();

        MapInfoPtr FindMapInfo(WCharCP url);

        bvector<MapInfoPtr> m_mapInfoList;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               5/2015
//=====================================================================================
struct MapInfo : public RefCountedBase
    {
public:
    //! Create WMS MapRequest info with all the required information. 
    static MapInfoPtr Create(WCharCP url, WCharCP version, WCharCP layers, WCharCP styles, WCharCP crs, WCharCP format);

    //! Add layer and corresponding style to an already existing MapInfo.
    void AddLayer(WCharCP layer, WCharCP style);

    //! Get/Set the server url. 
    WStringCR   GetUrl() const;
    void        SetUrl(WCharCP url);

    //! Get/Set the WMS version. 
    WStringCR   GetVersion() const;
    void        SetVersion(WCharCP version);

    //! Get the layer list. 
    bvector<WCharCP>  GetLayers() const;

    //! Get the style list. 
    bvector<WCharCP>  GetStyles() const;

    //! Get/Set the coordinate system. 
    WStringCR   GetCoordinateSystem() const;
    void        SetCoordinateSystem(WCharCP coordSys);

    //! Get/Set the coordinate system type attribute. 
    //! CRS for version 1.3, SRS for 1.1.1 and below.
    WStringCR   GetCoordSysType() const;
    void        SetCoordSysType();

    //! Get/Set the output format. 
    WStringCR   GetFormat() const;
    void        SetFormat(WCharCP format);
    
private:
    MapInfo(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP coordSys, WCharCP format);
    ~MapInfo();

    // Required members.
    WString             m_url;
    WString             m_version;
    bvector<WCharCP>    m_layers;
    bvector<WCharCP>    m_styles;
    WString             m_coordinateSystem;
    WString             m_format;

    // Attributes.
    WString             m_coordSysType;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE