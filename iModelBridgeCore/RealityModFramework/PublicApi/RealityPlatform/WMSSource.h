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

// Xml Fragment Tags
#define WMSSOURCE_PREFIX                        "wms"

#define WMSSOURCE_ELEMENT_Root                  "MapInfo"
#define WMSSOURCE_ELEMENT_Url                   "Url"
#define WMSSOURCE_ELEMENT_Version               "Version"
#define WMSSOURCE_ELEMENT_Layers                "Layers"
#define WMSSOURCE_ELEMENT_Styles                "Styles"
#define WMSSOURCE_ELEMENT_CoordinateSystem      "CoordinateSystem"
#define WMSSOURCE_ELEMENT_Format                "Format"
#define WMSSOURCE_ELEMENT_Transparent           "Transparent"
#define WMSSOURCE_ELEMENT_BackgroundColor       "BackgroundColor"
#define WMSSOURCE_ELEMENT_VendorSpecific        "VendorSpecific"
#define WMSSOURCE_ELEMENT_BoundingBox           "BoundingBox"
#define WMSSOURCE_ELEMENT_BoundingBoxOrder      "BoundingBoxOrder"
#define WMSSOURCE_ELEMENT_MetaWidth             "MetaWidth"
#define WMSSOURCE_ELEMENT_MetaHeight            "MetaHeight"
#define WMSSOURCE_ELEMENT_MetaTileSize          "MetaTileSize"
#define WMSSOURCE_ELEMENT_UserPassword          "UserPassword"
#define WMSSOURCE_ELEMENT_ProxyUserPassword     "ProxyUserPassword"

#define WMSSOURCE_ATTRIBUTE_CoordSysType        "type"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               6/2015
//=====================================================================================
struct WmsSource : public RefCountedBase
    {
public:
    //! Create an empty container for the MapInfos.
    REALITYDATAPLATFORM_EXPORT static WmsSourcePtr Create();

    //! Add MapInfo.
    REALITYDATAPLATFORM_EXPORT StatusInt Add(MapInfoPtr pMapInfo);

    //! Create xml fragment and return it as a xml string.
    REALITYDATAPLATFORM_EXPORT WString ToXmlString();

private:
    MapInfoPtr FindMapInfo(WCharCP url);

    bvector<MapInfoPtr> m_mapInfoList;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               5/2015
//=====================================================================================
struct MapInfo : public RefCountedBase
    {
public:
    //! Create empty object.
    REALITYDATAPLATFORM_EXPORT static MapInfoPtr Create();

    //! Create WMS MapRequest info with all the required information. 
    REALITYDATAPLATFORM_EXPORT static MapInfoPtr Create(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP crs, WCharCP format, 
                                                        bool isTransparent, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY, 
                                                        WCharCP bboxOrder, size_t width, size_t height);

    //! Add layer and corresponding style to an already existing MapInfo.
    REALITYDATAPLATFORM_EXPORT void AddLayer(WCharCP layer, WCharCP style = L"");

    //! Get/Set the server url. 
    REALITYDATAPLATFORM_EXPORT WStringCR   GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void        SetUrl(WCharCP url);

    //! Get/Set the WMS version. 
    REALITYDATAPLATFORM_EXPORT WStringCR   GetVersion() const;
    REALITYDATAPLATFORM_EXPORT void        SetVersion(WCharCP version);

    //! Get the layer list. 
    REALITYDATAPLATFORM_EXPORT bvector<WCharCP>  GetLayers() const;

    //! Get the style list. 
    REALITYDATAPLATFORM_EXPORT bvector<WCharCP>  GetStyles() const;

    //! Get/Set the coordinate system. 
    REALITYDATAPLATFORM_EXPORT WStringCR   GetCoordinateSystem() const;
    REALITYDATAPLATFORM_EXPORT void        SetCoordinateSystem(WCharCP coordSys);

    //! Get/Set the coordinate system type attribute. 
    //! CRS for version 1.3, SRS for 1.1.1 and below.
    REALITYDATAPLATFORM_EXPORT WStringCR   GetCoordSysType() const;
    REALITYDATAPLATFORM_EXPORT void        SetCoordSysType();

    //! Get/Set the output format. 
    REALITYDATAPLATFORM_EXPORT WStringCR   GetFormat() const;
    REALITYDATAPLATFORM_EXPORT void        SetFormat(WCharCP format);

    //! Get/Set the transparency. 
    REALITYDATAPLATFORM_EXPORT WStringCR    GetTransparency() const;
    REALITYDATAPLATFORM_EXPORT void         SetTransparency(bool isTransparent);

    //! Get/Set the bounding box.
    REALITYDATAPLATFORM_EXPORT bvector<double>  GetBBox() const;
    REALITYDATAPLATFORM_EXPORT void             SetBBox(double minX, double minY, double maxX, double maxY);

    //! Get/Set the bounding box order.
    //! Coordinates shall be listed in the order defined by the CRS.
    REALITYDATAPLATFORM_EXPORT WStringCR   GetBBoxOrder() const;
    REALITYDATAPLATFORM_EXPORT void        SetBBoxOrder(WCharCP bboxOrder);

    //! Get/Set the width of the window in pixels.
    //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
    //! Sub-Resolutions will be generated from this size.
    REALITYDATAPLATFORM_EXPORT size_t  GetMetaWidth() const;
    REALITYDATAPLATFORM_EXPORT void    SetMetaWidth(size_t width);

    //! Get/Set the height of the window in pixels.
    //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
    //! Sub-Resolutions will be generated from this size.
    REALITYDATAPLATFORM_EXPORT size_t  GetMetaHeight() const;
    REALITYDATAPLATFORM_EXPORT void    SetMetaHeight(size_t height);
    
    //! Get/Set the background color.
    //! Hexadecimal red-green-blue color value for the background color (default=0xFFFFFF).
    REALITYDATAPLATFORM_EXPORT WStringCR   GetBackgroundColor() const;
    REALITYDATAPLATFORM_EXPORT void        SetBackgroundColor(WCharCP color);

    //! Get/Set the vendor specific parameters.
    //! Unparsed, vendor specific parameters that will be appended to the request.
    REALITYDATAPLATFORM_EXPORT WStringCR   GetVendorSpecific() const;
    REALITYDATAPLATFORM_EXPORT void        SetVendorSpecific(WCharCP vendorSpecific);

    //! Get/Set the tile size.
    //! The tile size(must be a power of 2) use to for server request.
    //! This setting might be tweaked for performance or to reduce repeated labels or watermarks. 
    //! The value must be within server capabilities.
    REALITYDATAPLATFORM_EXPORT size_t  GetMetaTileSize() const;
    REALITYDATAPLATFORM_EXPORT void    SetMetaTileSize(size_t size);

    //! Get/Set the user password.
    REALITYDATAPLATFORM_EXPORT WStringCR   GetUserPassword() const;
    REALITYDATAPLATFORM_EXPORT void        SetUserPassword(WCharCP password);

    //! Get/Set the proxy user password.
    REALITYDATAPLATFORM_EXPORT WStringCR   GetProxyUserPassword() const;
    REALITYDATAPLATFORM_EXPORT void        SetProxyUserPassword(WCharCP password);

private:
    MapInfo();
    MapInfo(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP coordSys, WCharCP format,
            bool isTransparent, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY,
            WCharCP bboxOrder, size_t width, size_t height);
    ~MapInfo();

    // Required members.
    WString             m_url;
    WString             m_version;
    bvector<WCharCP>    m_layers;
    bvector<WCharCP>    m_styles;
    WString             m_coordinateSystem;
    WString             m_format;
    WString             m_transparency;
    bvector<double>     m_boundingBox;
    WString             m_bboxOrder;
    size_t              m_metaWidth;
    size_t              m_metaHeight;
    
    // Optional members.
    WString             m_backgroundColor;
    WString             m_vendorSpecific;
    size_t              m_metaTileSize;
    WString             m_userPassword;
    WString             m_proxyUserPassword;

    // Attributes.
    WString             m_coordSysType;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE