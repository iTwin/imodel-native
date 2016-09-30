/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/WMSSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPackage/WMSSource.h>
#include <BeXml/BeXml.h>

// Xml Fragment Tags
#define WMSSOURCE_PREFIX                        "wms"

#define WMSSOURCE_ELEMENT_Root                  "WmsMapSettings"
#define WMSSOURCE_ELEMENT_Uri                   "Uri"
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

USING_NAMESPACE_BENTLEY_REALITYPACKAGE

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapSettingsPtr WmsMapSettings::Create(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel)
    {
    return new WmsMapSettings(uri, bbox, version, layers, csType, csLabel);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapSettingsPtr WmsMapSettings::CreateFromXml(Utf8CP xmlFragment)
    {
    if (NULL == xmlFragment)
        return 0;

    // Create XmlDom from XmlFragment
    BeXmlStatus xmlStatus = BEXML_ReadError;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlFragment);
    if (BEXML_Success != xmlStatus)
        return 0;

    BeXmlNodeP pMapSettingsNode = pXmlDom->GetRootElement()->SelectSingleNode("WmsMapSettings");
    if (NULL == pMapSettingsNode)
        return 0;

    // Mandatory GetMap parameters
    Utf8String uri;
    DRange2d bbox;
    Utf8String version;
    Utf8String layers;
    Utf8String csType;
    Utf8String csLabel;

    BeXmlNodeP pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Uri);
    if (NULL != pChildNode)
        pChildNode->GetContent(uri);

    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_BoundingBox);
    if (NULL != pChildNode)
        {
        WString bboxCorners;
        pChildNode->GetContent(bboxCorners);

        bvector<WString> tokens;
        BeStringUtilities::Split(bboxCorners.c_str(), L",", tokens);

        BeAssert(4 == tokens.size());

        bbox.InitFrom(BeStringUtilities::Wcstod(tokens[0].c_str(), NULL),
                      BeStringUtilities::Wcstod(tokens[1].c_str(), NULL),
                      BeStringUtilities::Wcstod(tokens[2].c_str(), NULL),
                      BeStringUtilities::Wcstod(tokens[3].c_str(), NULL));
        }
    
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Version);
    if (NULL != pChildNode)
        pChildNode->GetContent(version);

    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Layers);
    if (NULL != pChildNode)
        pChildNode->GetContent(layers);

    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_CoordinateSystem);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(csLabel);
        pChildNode->GetAttributeStringValue(csType, WMSSOURCE_ATTRIBUTE_CoordSysType);
        }

    WmsMapSettingsPtr pMapSettings = WmsMapSettings::Create(uri.c_str(), bbox, version.c_str(), layers.c_str(), csType.c_str(), csLabel.c_str());

    // Optional GetMap parameters
    Utf8String styles;
    Utf8String format;
    uint32_t metaWidth;
    uint32_t metaHeight;
    Utf8String vendorSpecific;
    bool transparent;

    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Styles);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(styles);
        pMapSettings->SetStyles(styles.c_str());
        }
        
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Format);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(format);
        pMapSettings->SetFormat(format.c_str());
        }
        
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_MetaWidth);
    if (NULL != pChildNode)
        {
        pChildNode->GetContentUInt32Value(metaWidth);
        pMapSettings->SetMetaWidth(metaWidth);
        }
        
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_MetaHeight);
    if (NULL != pChildNode)
        {
        pChildNode->GetContentUInt32Value(metaHeight);
        pMapSettings->SetMetaHeight(metaHeight);
        }
        
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_VendorSpecific);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(vendorSpecific);
        pMapSettings->SetVendorSpecific(vendorSpecific.c_str());
        }
        
    pChildNode = pMapSettingsNode->SelectSingleNode(WMSSOURCE_ELEMENT_Transparent);
    if (NULL != pChildNode)
        {
        pChildNode->GetContentBooleanValue(transparent);
        pMapSettings->SetTransparency(transparent);
        }
    
    return pMapSettings;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
Utf8StringCR    WmsMapSettings::GetUri() const { return m_uri; }
void            WmsMapSettings::SetUri(Utf8CP uri) { m_uri = uri; }

DRange2dCR  WmsMapSettings::GetBBox() const { return m_boundingBox; }
void        WmsMapSettings::SetBBox(DRange2dCP bbox) { m_boundingBox = *bbox; }

uint32_t    WmsMapSettings::GetMetaWidth() const { return m_metaWidth; }
void        WmsMapSettings::SetMetaWidth(uint32_t width) { m_metaWidth = width; }

uint32_t    WmsMapSettings::GetMetaHeight() const { return m_metaHeight; }
void        WmsMapSettings::SetMetaHeight(uint32_t height) { m_metaHeight = height; }

Utf8StringCR    WmsMapSettings::GetVersion() const { return m_version; }
void            WmsMapSettings::SetVersion(Utf8CP version) { m_version = version; }

Utf8StringCR    WmsMapSettings::GetLayers() const { return m_layers; }
void            WmsMapSettings::SetLayers(Utf8CP layers) { m_layers = layers; }

Utf8StringCR    WmsMapSettings::GetStyles() const { return m_styles; }
void            WmsMapSettings::SetStyles(Utf8CP styles) { m_styles = styles; }

Utf8StringCR    WmsMapSettings::GetCoordSysType() const { return m_csType; }
void            WmsMapSettings::SetCoordSysType(Utf8CP csType) { m_csType = csType; }

Utf8StringCR    WmsMapSettings::GetCoordSysLabel() const { return m_csLabel; }
void            WmsMapSettings::SetCoordSysLabel(Utf8CP csLabel) { m_csLabel = csLabel; }

Utf8StringCR    WmsMapSettings::GetFormat() const { return m_format; }
void            WmsMapSettings::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR    WmsMapSettings::GetVendorSpecific() const { return m_vendorSpecific; }
void            WmsMapSettings::SetVendorSpecific(Utf8CP vendorSpecific) { m_vendorSpecific = vendorSpecific; }

bool    WmsMapSettings::IsTransparent() const { return m_transparent; }
void    WmsMapSettings::SetTransparency(bool isTransparent) { m_transparent = isTransparent; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void WmsMapSettings::ToXml(Utf8StringR xmlFragment) const
    {
    // Create new dom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    // Add root node.
    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(WMSSOURCE_ELEMENT_Root, NULL, NULL);

    // &&JFC WIP: Add namespace
    //pRootNode->SetNamespace(WMSSOURCE_PREFIX, NULL);

    WString temp;

    // [Required] Uri  
    BeStringUtilities::Utf8ToWChar(temp, m_uri.c_str());
    pRootNode->AddElementStringValue(WMSSOURCE_ELEMENT_Uri, temp.c_str());

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
WmsMapSettings::WmsMapSettings(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel)
    : m_uri(uri),
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
WmsMapSettings::~WmsMapSettings() {}
