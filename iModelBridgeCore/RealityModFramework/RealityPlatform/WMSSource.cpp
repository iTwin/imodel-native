/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WMSSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/WMSSource.h>
#include <BeXml/BeXml.h>

// Xml Fragment Tags
#define WMSSOURCE_PREFIX                        "wms"

#define WMSSOURCE_ELEMENT_Root                  "WmsMapInfo"
#define WMSSOURCE_ELEMENT_Url                   "Url"
#define WMSSOURCE_ELEMENT_BoundingBox           "BoundingBox"
#define WMSSOURCE_ELEMENT_MetaWidth             "MetaWidth"
#define WMSSOURCE_ELEMENT_MetaHeight            "MetaHeight"
#define WMSSOURCE_ELEMENT_Version               "Version"
#define WMSSOURCE_ELEMENT_Layers                "Layers"
#define WMSSOURCE_ELEMENT_Styles                "Styles"
#define WMSSOURCE_ELEMENT_CoordinateSystem      "CoordinateSystem"
#define WMSSOURCE_ELEMENT_Format                "Format"
#define WMSSOURCE_ELEMENT_VendorSpecific        "VendorSpecific"
#define WMSSOURCE_ELEMENT_Transparent           "Transparent"

#define WMSSOURCE_ATTRIBUTE_CoordSysType        "type"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapInfoPtr WmsMapInfo::Create(Utf8CP url, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel)
    {
    return new WmsMapInfo(url, bbox, version, layers, csType, csLabel);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
Utf8StringCR    WmsMapInfo::GetUrl() const { return m_url; }
void            WmsMapInfo::SetUrl(Utf8CP url) { m_url = url; }

DRange2dCR  WmsMapInfo::GetBBox() const { return m_boundingBox; }
void        WmsMapInfo::SetBBox(DRange2dCP bbox) { m_boundingBox = *bbox; }

uint32_t    WmsMapInfo::GetMetaWidth() const { return m_metaWidth; }
void        WmsMapInfo::SetMetaWidth(uint32_t width) { m_metaWidth = width; }

uint32_t    WmsMapInfo::GetMetaHeight() const { return m_metaHeight; }
void        WmsMapInfo::SetMetaHeight(uint32_t height) { m_metaHeight = height; }

Utf8StringCR    WmsMapInfo::GetVersion() const { return m_version; }
void            WmsMapInfo::SetVersion(Utf8CP version) { m_version = version; }

Utf8StringCR    WmsMapInfo::GetLayers() const { return m_layers; }
void            WmsMapInfo::SetLayers(Utf8CP layers) { m_layers = layers; }

Utf8StringCR    WmsMapInfo::GetStyles() const { return m_styles; }
void            WmsMapInfo::SetStyles(Utf8CP styles) { m_styles = styles; }

Utf8StringCR    WmsMapInfo::GetCoordSysType() const { return m_csType; }
void            WmsMapInfo::SetCoordSysType(Utf8CP csType) { m_csType = csType; }

Utf8StringCR    WmsMapInfo::GetCoordSysLabel() const { return m_csLabel; }
void            WmsMapInfo::SetCoordSysLabel(Utf8CP csLabel) { m_csLabel = csLabel; }

Utf8StringCR    WmsMapInfo::GetFormat() const { return m_format; }
void            WmsMapInfo::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR    WmsMapInfo::GetVendorSpecific() const { return m_vendorSpecific; }
void            WmsMapInfo::SetVendorSpecific(Utf8CP vendorSpecific) { m_vendorSpecific = vendorSpecific; }

bool    WmsMapInfo::IsTransparent() const { return m_transparent; }
void    WmsMapInfo::SetTransparency(bool isTransparent) { m_transparent = isTransparent; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void WmsMapInfo::ToXml(Utf8StringR xmlFragment) const
    {
    // Create new dom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    // Add root node.
    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(WMSSOURCE_ELEMENT_Root, NULL, NULL);

    // &&JFC WIP: Add namespace
    //pRootNode->SetNamespace(WMSSOURCE_PREFIX, NULL);

    WString temp;

    // [Required] Url  
    BeStringUtilities::Utf8ToWChar(temp, m_url.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Url, temp.c_str());

    // [Required] Bounding Box
    WString bboxCorners;
    WChar buf[100];
    BeStringUtilities::Snwprintf(buf, L"%f", m_boundingBox.low.x);
    bboxCorners.append(buf);
    bboxCorners.append(L",");
    BeStringUtilities::Snwprintf(buf, L"%f", m_boundingBox.low.y);
    bboxCorners.append(buf);
    bboxCorners.append(L",");
    BeStringUtilities::Snwprintf(buf, L"%f", m_boundingBox.high.x);
    bboxCorners.append(buf);
    bboxCorners.append(L",");
    BeStringUtilities::Snwprintf(buf, L"%f", m_boundingBox.high.y);
    bboxCorners.append(buf);    
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_BoundingBox, bboxCorners.c_str());

    // [Required] Meta Width
    //pRootNode->AddElementUInt64Value(WMSSOURCE_ELEMENT_MetaWidth, m_metaWidth);

    // [Required] Meta Height
    //pRootNode->AddElementUInt64Value(WMSSOURCE_ELEMENT_MetaHeight, m_metaHeight);

    // [Required] Version
    BeStringUtilities::Utf8ToWChar(temp, m_version.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Version, temp.c_str());

    // [Required] Layers
    BeStringUtilities::Utf8ToWChar(temp, m_layers.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Layers, temp.c_str());

    // [Required] Styles
    BeStringUtilities::Utf8ToWChar(temp, m_styles.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Styles, temp.c_str());

    // [Required] Coordinate System
    BeStringUtilities::Utf8ToWChar(temp, m_csLabel.c_str());
    BeXmlNodeP coordSysNode = pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_CoordinateSystem, temp.c_str());
    BeStringUtilities::Utf8ToWChar(temp, m_csType.c_str());
    coordSysNode->AddAttributeStringValue(WMSSOURCE_ATTRIBUTE_CoordSysType, temp.c_str());

    // [Required] Format
    BeStringUtilities::Utf8ToWChar(temp, m_format.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Format, temp.c_str());

    // [Optional] Vendor Specific
    if (!m_vendorSpecific.empty())
        {
        BeStringUtilities::Utf8ToWChar(temp, m_vendorSpecific.c_str());
        pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_VendorSpecific, temp.c_str());
        }

    // [Optional] Transparent
    pRootNode->AddElementBooleanValue(WMSSOURCE_ELEMENT_Transparent, m_transparent);

    // Convert to string.
    pRootNode->GetXmlString(xmlFragment);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
void WmsMapInfo::FromXml(Utf8CP xmlFragment)
    {
    if (NULL == xmlFragment)
        return;

    // Create XmlDom from XmlFragment
    BeXmlStatus xmlStatus = BEXML_ReadError;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlFragment);
    if (BEXML_Success != xmlStatus)
        return;

    BeXmlNodeP pMapInfoNode = pXmlDom->GetRootElement()->SelectSingleNode("MapInfo");
    if (NULL == pMapInfoNode)
        return;

    BeXmlNodeP pChildNode = NULL;

    // GetMap url
    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Url);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_url);

    // Map window
    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_BoundingBox);
    if (NULL != pChildNode)
        {
        WString bboxCorners;
        pChildNode->GetContent(bboxCorners);

        bvector<WString> tokens;
        BeStringUtilities::Split(bboxCorners.c_str(), L",", tokens);

        BeAssert(4 == tokens.size());

        m_boundingBox.InitFrom(BeStringUtilities::Wcstod(tokens[0].c_str(), NULL),
                               BeStringUtilities::Wcstod(tokens[1].c_str(), NULL),
                               BeStringUtilities::Wcstod(tokens[2].c_str(), NULL),
                               BeStringUtilities::Wcstod(tokens[3].c_str(), NULL));
        }
        
    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_MetaWidth);
    if (NULL != pChildNode)
        pChildNode->GetContentUInt32Value(m_metaWidth);

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_MetaHeight);
    if (NULL != pChildNode)
        pChildNode->GetContentUInt32Value(m_metaHeight);

    // Mandatory GetMap parameters
    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Version);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_version);

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Layers);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_layers);

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Styles);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_styles);

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_CoordinateSystem);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(m_csLabel);
        pChildNode->GetAttributeStringValue(m_csType, WMSSOURCE_ATTRIBUTE_CoordSysType);
        }

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Format);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_format);

    // Optional GetMap parameters
    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_VendorSpecific);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_vendorSpecific);

    pChildNode = pMapInfoNode->SelectSingleNode(WMSSOURCE_ELEMENT_Transparent);
    if (NULL != pChildNode)
        pChildNode->GetContentBooleanValue(m_transparent);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapInfo::WmsMapInfo(Utf8CP url, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel)
    : m_url(url), 
      m_boundingBox(bbox), 
      m_metaWidth(10),          // Default.
      m_metaHeight(10),         // Default.
      m_version(version), 
      m_layers(layers), 
      m_styles(""),             // Default.
      m_csType(csType), 
      m_csLabel(csLabel),
      m_format("image/png"),    // The standards says that all servers should support png.
      m_vendorSpecific(""),
      m_transparent(true)       // Default.
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapInfo::~WmsMapInfo() {}
