/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPackage/RealityDataPackage.h>
#include <Bentley/BeFileName.h>
#include "RealitySerialization.h"

USING_BENTLEY_NAMESPACE_REALITYPACKAGE

//=======================================================================================
//                              RealityDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
WStringCR RealityDataSource::GetUri() const {return m_uri;} 
void RealityDataSource::SetUri(WCharCP uri) {m_uri=uri;}

WStringCR RealityDataSource::GetType() const  {return m_type;} 
void RealityDataSource::SetType(WCharCP type) {m_type=type;}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP RealityDataSource::_GetElementName() const 
    {
    BeAssert("Child class must override _GetElementName()" && typeid(*this) == typeid(RealityDataSource));  
    return PACKAGE_ELEMENT_Source;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::RealityDataSource(WCharCP uri, WCharCP type)
    {
    BeAssert(!WString::IsNullOrEmpty(uri) && !WString::IsNullOrEmpty(type));
    m_uri = uri;
    m_type = type;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::~RealityDataSource(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::Create(WCharCP uri, WCharCP type)
    {
    if(WString::IsNullOrEmpty(uri) || WString::IsNullOrEmpty(type))
        return NULL;
        
    return new RealityDataSource(uri, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSource::_Read(BeXmlNodeR dataSourceNode)
    {
    if(BEXML_Success != dataSourceNode.GetAttributeStringValue (m_uri, PACKAGE_SOURCE_ATTRIBUTE_Uri) ||
       BEXML_Success != dataSourceNode.GetAttributeStringValue (m_type, PACKAGE_SOURCE_ATTRIBUTE_Type))
        return RealityPackageStatus::MissingSourceAttribute;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSource::_Write(BeXmlNodeR dataSourceNode) const
    {
    if(m_uri.empty() || m_type.empty())
        return RealityPackageStatus::MissingSourceAttribute;
        
    dataSourceNode.AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, m_uri.c_str());
    dataSourceNode.AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, m_type.c_str());

    return RealityPackageStatus::Success;
    }


//=======================================================================================
//                              WmsDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
WStringCR WmsDataSource::GetMapInfo() const { return m_mapInfo; }
void WmsDataSource::SetMapInfo(WCharCP mapInfo) { m_mapInfo = mapInfo; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::WmsDataSource(WCharCP uri)
    :RealityDataSource(uri, WMS_SOURCE_TYPE)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::~WmsDataSource(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP WmsDataSource::_GetElementName() const {return PACKAGE_ELEMENT_WmsSource;}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSourcePtr WmsDataSource::Create(WCharCP uri)
    {
    if(WString::IsNullOrEmpty(uri))
        return NULL;
        
    return new WmsDataSource(uri);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus WmsDataSource::_Read(BeXmlNodeR dataSourceNode)
    {
    // Always read base first.
    RealityPackageStatus status = T_Super::_Read(dataSourceNode);
    if(RealityPackageStatus::Success != status)
        return status;

    // Create MapInfo xml fragment from xml node.
    BeXmlStatus xmlStatus = BEXML_Success;
    xmlStatus = dataSourceNode.GetXmlString(m_mapInfo);
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::XmlReadError;

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus WmsDataSource::_Write(BeXmlNodeR dataSourceNode) const
    {
    // Always write base first.
    RealityPackageStatus status = T_Super::_Write(dataSourceNode);
    if(RealityPackageStatus::Success != status)
        return status;

    //&&JFC TODO Doc why we can accept an empty string here (eventually uri will be the GetCapabilities request instead of the server url).
    if (m_mapInfo.empty())
        return RealityPackageStatus::Success;

    // Create Xml Dom from string.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, m_mapInfo.c_str());
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::XmlReadError;

    // Add root node.
    dataSourceNode.ImportNode(pXmlDom->GetRootElement());

    return status;
    }