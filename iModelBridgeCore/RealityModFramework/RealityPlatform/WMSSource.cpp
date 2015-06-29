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
//WmsSource::WmsSource()
//    {
//    m_mapRequestList.resize(0);
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
//WmsSource::~WmsSource()
//    {
//    // Ptr release
//    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WmsSourcePtr WmsSource::Create()
    {
    WmsSourcePtr sourcePtr = new WmsSource();
    sourcePtr->AddRef();
    return sourcePtr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
StatusInt WmsSource::Add(MapInfoPtr mapRequest)
    {
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
StatusInt WmsSource::Add(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP crs, WCharCP format)
    {
    MapInfoPtr foundMapInfoPtr = FindMapInfo(url);
    if (0 == foundMapInfoPtr.get())
        {
        // Create a new WMS map request.
        MapInfoPtr mapInfoPtr = MapInfo::Create(url, version, layer, style, crs, format);
        if (0 == mapInfoPtr.get())
            return ERROR;

        m_mapInfoList.push_back(mapInfoPtr);
        }
    else
        {
        // The request already exist, just append the new layer and style to the list.
        foundMapInfoPtr->AddLayer(layer, style);
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
        for (bvector<MapInfoPtr>::iterator mapInfoIt = m_mapInfoList.begin(); mapInfoIt != m_mapInfoList.end(); ++mapInfoIt)
            {
            // Add MapRequest info
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Url, (*mapInfoIt)->GetUrl().c_str());
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Version, (*mapInfoIt)->GetVersion().c_str());
            BeXmlNodeP coordSysNode = pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_CoordinateSystem, (*mapInfoIt)->GetCoordinateSystem().c_str());
            coordSysNode->AddAttributeStringValue(WMSSOURCE_ATTRIBUTE_CoordSysType, (*mapInfoIt)->GetCoordSysType().c_str());
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Format, (*mapInfoIt)->GetFormat().c_str());

            // Create layers string
            WString layers;
            bvector<WCharCP> layerList = (*mapInfoIt)->GetLayers();
            for (size_t pos = 0; pos < layerList.size(); ++pos)
                {
                if (layers.empty())
                    layers += layerList[pos];
                else
                    {
                    layers += L",";
                    layers += layerList[pos];
                    }
                }
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Layers, layers.c_str());

            // Create styles string
            WString styles;
            bvector<WCharCP> styleList = (*mapInfoIt)->GetStyles();
            for (size_t pos = 0; pos < styleList.size(); ++pos)
                {
                if (styles.empty())
                    styles += styleList[pos];
                else
                    {
                    styles += L",";
                    styles += styleList[pos];
                    }
                }
            pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Styles, styles.c_str());
            }
        }

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
WStringCR MapInfo::GetUrl() const { return m_url; }
void MapInfo::SetUrl(WCharCP url) { m_url = url; }

WStringCR MapInfo::GetVersion() const { return m_version; }
void MapInfo::SetVersion(WCharCP version) { m_version = version; }

bvector<WCharCP> MapInfo::GetLayers() const { return m_layers; }
bvector<WCharCP> MapInfo::GetStyles() const { return m_styles; }

WStringCR MapInfo::GetCoordinateSystem() const { return m_coordinateSystem; }
void MapInfo::SetCoordinateSystem(WCharCP coordSys) { m_coordinateSystem = coordSys; }

WStringCR MapInfo::GetFormat() const { return m_format; }
void MapInfo::SetFormat(WCharCP format) { m_format = format; }

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
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfoPtr MapInfo::Create(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP crs, WCharCP format)
    {
    MapInfoPtr mapInfoPtr = new MapInfo(url, version, layer, style, crs, format);
    mapInfoPtr->SetCoordSysType();

    return mapInfoPtr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void MapInfo::AddLayer(WCharCP layer, WCharCP style)
    {
    m_layers.push_back(layer);
    m_styles.push_back(style);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfo::MapInfo(WCharCP url, WCharCP version, WCharCP layer, WCharCP style, WCharCP coordSys, WCharCP format)
    : m_url(url), m_version(version), m_coordinateSystem(coordSys), m_format(format)
    {
    AddLayer(layer, style);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
MapInfo::~MapInfo() {}
