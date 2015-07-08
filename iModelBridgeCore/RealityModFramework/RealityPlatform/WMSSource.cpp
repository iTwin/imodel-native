/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WMSSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/WMSSource.h>
#include <BeXml/BeXml.h>

USING_BENTLEY_NAMESPACE_REALITYPLATFORM

//=======================================================================================
//                                      WmsSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WmsSourcePtr WmsSource::Create()
    {
    WmsSourcePtr sourcePtr = new WmsSource();
    sourcePtr->AddRef(); //&&JFC
    return sourcePtr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
StatusInt WmsSource::Add(MapInfoPtr pMapInfo)
    {
    MapInfoPtr pFoundMapInfo = FindMapInfo(pMapInfo->GetUrl().c_str());

    // If this is a new Wms server, add it, else only append the layer and style to the existing one.
    if (0 == pFoundMapInfo.get())
        {
        m_mapInfoList.push_back(pMapInfo);
        }
    else
        {
        pFoundMapInfo->AddLayer(pMapInfo->GetLayers()[0], pMapInfo->GetStyles()[0]);
        }        

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WString WmsSource::ToXmlString()
    {
    // Create new dom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    // Add root node.
    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(WMSSOURCE_ELEMENT_Root, NULL, NULL);

    // &&JFC WIP: Add namespace
    //pRootNode->SetNamespace(WMSSOURCE_PREFIX, NULL);

    if (!m_mapInfoList.empty())
        {
        // Add all Wms info.
        for (bvector<MapInfoPtr>::iterator mapInfoIt = m_mapInfoList.begin(); mapInfoIt != m_mapInfoList.end(); ++mapInfoIt)
            {
            // [Required] Url
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Url, (*mapInfoIt)->GetUrl().c_str());

            // [Required] Version
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Version, (*mapInfoIt)->GetVersion().c_str());

            // [Required] Layers
            WString layers;
            bvector<WCharCP> layerList = (*mapInfoIt)->GetLayers();
            for (size_t pos = 0; pos < layerList.size(); ++pos)
                {
                if (!layers.empty())
                    layers += L",";

                layers += layerList[pos];
                }
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Layers, layers.c_str());

            // [Required] Styles
            WString styles;
            bvector<WCharCP> styleList = (*mapInfoIt)->GetStyles();
            for (size_t pos = 0; pos < styleList.size(); ++pos)
                {
                if (!styles.empty())
                    styles += L",";

                styles += styleList[pos];
                }
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Styles, styles.c_str());

            // [Required] Coordinate System
            BeXmlNodeP coordSysNode = pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_CoordinateSystem, (*mapInfoIt)->GetCoordinateSystem().c_str());
            coordSysNode->AddAttributeStringValue(WMSSOURCE_ATTRIBUTE_CoordSysType, (*mapInfoIt)->GetCoordSysType().c_str());

            // [Required] Format
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Format, (*mapInfoIt)->GetFormat().c_str());

            // [Required] Transparent
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Transparent, (*mapInfoIt)->GetTransparency().c_str());

            // [Optional] Background Color
            WString backgroundColor = (*mapInfoIt)->GetBackgroundColor();
            if (!backgroundColor.empty())
                pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_BackgroundColor, backgroundColor.c_str());

            // [Optional] Vendor Specific
            WString vendorSpecific = (*mapInfoIt)->GetVendorSpecific();
            if (!vendorSpecific.empty())
                pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_VendorSpecific, vendorSpecific.c_str());

            // [Required] Bounding Box
            WString bbox;
            WChar buf[100];
            bvector<double> bboxPts = (*mapInfoIt)->GetBBox();
            for (size_t pos = 0; pos < bboxPts.size(); ++pos)
                {
                if (!bbox.empty())
                    bbox += L",";

                BeStringUtilities::Snwprintf(buf, L"%f", bboxPts[pos]);
                bbox += buf;
                }
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_BoundingBox, bbox.c_str());

            // [Required] Bounding Box Order
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_BoundingBoxOrder, (*mapInfoIt)->GetBBoxOrder().c_str());

            // [Required] Meta Width
            pRootNode->AddElementUInt64Value(WMSSOURCE_ELEMENT_MetaWidth, (*mapInfoIt)->GetMetaWidth());
            
            //size_t width = 0;
            //width = (*mapInfoIt)->GetMetaWidth();
            //if (0 != width)
            //    {
            //    WChar buf[10];
            //    if (SUCCESS == BeStringUtilities::Itow(buf, static_cast<int>(width), _countof(buf), 10))
            //        pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_MetaWidth, buf);
            //    }

            // [Required] Meta Height
            pRootNode->AddElementUInt64Value(WMSSOURCE_ELEMENT_MetaHeight, (*mapInfoIt)->GetMetaHeight());

            // [Optional] Meta Tile Size
            size_t tileSize = (*mapInfoIt)->GetMetaTileSize();
            if (0 != tileSize)
                pRootNode->AddElementUInt64Value(WMSSOURCE_ELEMENT_MetaTileSize, tileSize);
                
            // [Optional] User Password
            WString userPassword = (*mapInfoIt)->GetUserPassword();
            if (!userPassword.empty())
                pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_UserPassword, userPassword.c_str());

            // [Optional] Proxy User Password
            WString proxyUserPassword = (*mapInfoIt)->GetProxyUserPassword();
            if (!proxyUserPassword.empty())
                pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_ProxyUserPassword, proxyUserPassword.c_str());
            }
        }

    // Convert xml fragment to string.
    WString xmlString = NULL;
    pRootNode->GetXmlString(xmlString);

    return xmlString;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
MapInfoPtr WmsSource::FindMapInfo(WCharCP url)
    {
    if (!m_mapInfoList.empty())
        {
        for (bvector<MapInfoPtr>::iterator mapInfoIt = m_mapInfoList.begin(); mapInfoIt != m_mapInfoList.end(); ++mapInfoIt)
            {
            if (0 == BeStringUtilities::Wcsicmp(url, (*mapInfoIt)->GetUrl().c_str()))
                return (*mapInfoIt);
            }
        }

    return 0;
    }

//=======================================================================================
//                                      MapInfo
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WStringCR   MapInfo::GetUrl() const { return m_url; }
void        MapInfo::SetUrl(WCharCP url) { m_url = url; }

WStringCR   MapInfo::GetVersion() const { return m_version; }
void        MapInfo::SetVersion(WCharCP version) { m_version = version; }

bvector<WCharCP>    MapInfo::GetLayers() const { return m_layers; }

bvector<WCharCP>    MapInfo::GetStyles() const { return m_styles; }

WStringCR   MapInfo::GetFormat() const { return m_format; }
void        MapInfo::SetFormat(WCharCP format) { m_format = format; }

WStringCR   MapInfo::GetBBoxOrder() const { return m_bboxOrder; }
void        MapInfo::SetBBoxOrder(WCharCP bboxOrder) { m_bboxOrder = bboxOrder; }

size_t  MapInfo::GetMetaWidth() const { return m_metaWidth; }
void    MapInfo::SetMetaWidth(size_t width) { m_metaWidth = width; }

size_t  MapInfo::GetMetaHeight() const { return m_metaHeight; }
void    MapInfo::SetMetaHeight(size_t height) { m_metaHeight = height; }

WStringCR   MapInfo::GetBackgroundColor() const { return m_backgroundColor; }
void        MapInfo::SetBackgroundColor(WCharCP color) { m_backgroundColor = color; }

WStringCR   MapInfo::GetVendorSpecific() const { return m_vendorSpecific; }
void        MapInfo::SetVendorSpecific(WCharCP vendorSpecific) { m_vendorSpecific = vendorSpecific; }

size_t  MapInfo::GetMetaTileSize() const { return m_metaTileSize; }
void    MapInfo::SetMetaTileSize(size_t size) { m_metaTileSize = size; }

WStringCR   MapInfo::GetUserPassword() const { return m_userPassword; }
void        MapInfo::SetUserPassword(WCharCP password) { m_userPassword = password; }

WStringCR   MapInfo::GetProxyUserPassword() const { return m_proxyUserPassword; }
void        MapInfo::SetProxyUserPassword(WCharCP password) { m_proxyUserPassword = password; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WStringCR MapInfo::GetCoordinateSystem() const { return m_coordinateSystem; }
void MapInfo::SetCoordinateSystem(WCharCP coordSys) 
    { 
    m_coordinateSystem = coordSys;
    SetCoordSysType();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WStringCR MapInfo::GetCoordSysType() const { return m_coordSysType; }
void MapInfo::SetCoordSysType() 
    { 
    // Default.
    m_coordSysType = L"CRS"; 

    if (m_version != L"1.3.0")
        m_coordSysType = L"SRS";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WStringCR MapInfo::GetTransparency() const { return m_transparency; }
void MapInfo::SetTransparency(bool isTransparent) 
    { 
    m_transparency = L"FALSE";

    if (isTransparent)
        m_transparency = L"TRUE"; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
bvector<double> MapInfo::GetBBox() const { return m_boundingBox; }
void MapInfo::SetBBox(double minX, double minY, double maxX, double maxY)
    {
    m_boundingBox.push_back(minX);
    m_boundingBox.push_back(minY);
    m_boundingBox.push_back(maxX);
    m_boundingBox.push_back(maxY);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfoPtr MapInfo::Create()
    {
    return new MapInfo();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfoPtr MapInfo::Create(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP crs, WCharCP format,
                           bool isTransparent, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY,
                           WCharCP bboxOrder, size_t width, size_t height)
    {
    return new MapInfo(url, version, layer, style, crs, format, isTransparent, bboxMinX, bboxMinY, bboxMaxX, bboxMaxY, bboxOrder, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfo::MapInfo() 
    {
    //Default
    SetBackgroundColor(L"");
    SetVendorSpecific(L"");
    SetMetaTileSize(0);
    SetUserPassword(L"");
    SetProxyUserPassword(L"");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfo::MapInfo(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP coordSys, WCharCP format, 
                 bool isTransparent, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY, 
                 WCharCP bboxOrder, size_t width, size_t height)
    {
    SetUrl(url);
    SetVersion(version);
    SetCoordinateSystem(coordSys);
    SetFormat(format);
    SetTransparency(isTransparent);
    SetBBox(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);
    SetBBoxOrder(bboxOrder);
    SetMetaWidth(width);
    SetMetaHeight(height);

    //Default
    SetBackgroundColor(L"");
    SetVendorSpecific(L"");
    SetMetaTileSize(0);
    SetUserPassword(L"");
    SetProxyUserPassword(L"");

    AddLayer(layer, style);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfo::~MapInfo() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void MapInfo::AddLayer(WCharCP layer, WCharCP style)
    {
    m_layers.push_back(layer);
    m_styles.push_back(style);
    }
