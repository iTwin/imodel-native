/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPackage/RealityDataPackage.h>
#include <Bentley/BeFileName.h>
#include "RealitySerialization.h"

USING_NAMESPACE_BENTLEY_REALITYPACKAGE

//=======================================================================================
//                              RealityDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------

bool RealityDataSource::IsMultiBand() const {return _IsMultiBand();}

Utf8StringCR RealityDataSource::GetUri() const { return m_uri; }
void RealityDataSource::SetUri(Utf8CP uri) { m_uri = uri; }

Utf8StringCR RealityDataSource::GetType() const { return m_type; }
void RealityDataSource::SetType(Utf8CP type) { m_type = type; }

Utf8StringCR RealityDataSource::GetCopyright() const { return m_copyright; }
void RealityDataSource::SetCopyright(Utf8CP copyright) { m_copyright = copyright; }

Utf8StringCR RealityDataSource::GetId() const { return m_id; }
void RealityDataSource::SetId(Utf8CP id) { m_id = id; }

Utf8StringCR RealityDataSource::GetProvider() const { return m_provider; }
void RealityDataSource::SetProvider(Utf8CP provider) { m_provider = provider; }

uint64_t RealityDataSource::GetFileSize() const { return m_filesize; }
void RealityDataSource::SetFileSize(uint64_t filesize) { m_filesize = filesize; }

Utf8StringCR RealityDataSource::GetFileInCompound() const { return m_fileInCompound; }
void RealityDataSource::SetFileInCompound(Utf8CP filename) { m_fileInCompound = filename; }

Utf8StringCR RealityDataSource::GetMetadata() const { return m_metadata; }
void RealityDataSource::SetMetadata(Utf8CP metadata) { m_metadata = metadata; }

const bvector<Utf8String>& RealityDataSource::GetSisterFiles() const { return m_sisterFiles; }
void RealityDataSource::SetSisterFiles(const bvector<Utf8String>& sisterFiles) { m_sisterFiles = sisterFiles; }


// Default _IsMultiBand() implementation to be overriden by multiband class.
bool RealityDataSource::_IsMultiBand() const {return false;}

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
RealityDataSource::RealityDataSource(Utf8CP uri, Utf8CP type)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(uri) && !Utf8String::IsNullOrEmpty(type));
    m_uri = uri;
    m_type = type;

    m_filesize = 0;     // Default.
    m_sisterFiles = bvector<Utf8String>();      // Create empty.
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::~RealityDataSource(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::Create(Utf8CP uri, Utf8CP type)
    {
    if (Utf8String::IsNullOrEmpty(uri) || Utf8String::IsNullOrEmpty(type))
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

    // &&JFC TODO: Create an object for url.
    // Split full uri if it is a compound type (uri#fileInCompound).
    m_fileInCompound = "";
    if (Utf8String::npos != m_uri.find("#"))
        {
        size_t delimPos = m_uri.find("#");
        m_fileInCompound = m_uri.substr(delimPos + 1);
        m_uri = m_uri.substr(0, delimPos - 1);
        }

    // Optional fields.
    dataSourceNode.GetContent(m_copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    dataSourceNode.GetContent(m_id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    dataSourceNode.GetContent(m_provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    dataSourceNode.GetContentUInt64Value(m_filesize, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Filesize);
    dataSourceNode.GetContent(m_metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);

    // &&JFC TODO: Create bvector from comma-separated list.
    Utf8String sisterFilesAsString;
    dataSourceNode.GetContent(sisterFilesAsString, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_SisterFiles);


    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSource::_Write(BeXmlNodeR dataSourceNode) const
    {
    if(m_uri.empty() || m_type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    // Construct full uri if compound type (uri#fileInCompound).
    if (m_fileInCompound.empty())
        dataSourceNode.AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, m_uri.c_str());
    else
        {
        Utf8String compoundUri = m_uri + "#" + m_fileInCompound;
        dataSourceNode.AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, compoundUri.c_str());
        }
    

    
    dataSourceNode.AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, m_type.c_str());

    // Optional fields.
    if (!m_copyright.empty())
        dataSourceNode.AddElementStringValue(PACKAGE_ELEMENT_Copyright, m_copyright.c_str());

    if (!m_id.empty())
        dataSourceNode.AddElementStringValue(PACKAGE_ELEMENT_Id, m_id.c_str());

    if (!m_provider.empty())
        dataSourceNode.AddElementStringValue(PACKAGE_ELEMENT_Provider, m_provider.c_str());

    if (0 != m_filesize)
        dataSourceNode.AddElementUInt64Value(PACKAGE_ELEMENT_Filesize, m_filesize);

    if (!m_metadata.empty())
        dataSourceNode.AddElementStringValue(PACKAGE_ELEMENT_Metadata, m_metadata.c_str());

    // Create comma-separated list from bvector.
    Utf8String sisterFilesAsString;
    if (!sisterFilesAsString.empty())
        dataSourceNode.AddElementStringValue(PACKAGE_ELEMENT_SisterFiles, sisterFilesAsString.c_str());


    return RealityPackageStatus::Success;
    }


//=======================================================================================
//                              WmsDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
Utf8StringCR WmsDataSource::GetMapSettings() const { return m_mapSettings; }
void WmsDataSource::SetMapSettings(Utf8CP mapSettings) { m_mapSettings = mapSettings; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::WmsDataSource(Utf8CP uri)
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
WmsDataSourcePtr WmsDataSource::Create(Utf8CP uri)
    {
    if (Utf8String::IsNullOrEmpty(uri))
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

    // Create MapSettings xml fragment from xml node.
    BeXmlStatus xmlStatus = BEXML_Success;
    xmlStatus = dataSourceNode.GetXmlString(m_mapSettings);
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

    if (m_mapSettings.empty())
        return RealityPackageStatus::Success;

    // Create Xml Dom from string.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, m_mapSettings.c_str());
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::XmlReadError;

    // Add root node.
    dataSourceNode.ImportNode(pXmlDom->GetRootElement());

    return status;
    }

//=======================================================================================
//                              OsmDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
Utf8StringCR OsmDataSource::GetOsmResource() const { return m_osmResource; }
void OsmDataSource::SetOsmResource(Utf8CP osmResource) { m_osmResource = osmResource; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmDataSource::OsmDataSource(Utf8CP uri)
    :RealityDataSource(uri, OSM_SOURCE_TYPE)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmDataSource::~OsmDataSource() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
Utf8CP OsmDataSource::_GetElementName() const { return PACKAGE_ELEMENT_OsmSource; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmDataSourcePtr OsmDataSource::Create(Utf8CP uri, DRange2dCP bbox)
    {
    if (Utf8String::IsNullOrEmpty(uri))
        return NULL;

    // Convert bbox to a comma separated string.
    Utf8String result;
    Utf8PrintfString lowPtStr(LATLONG_PRINT_FORMAT_COMMA ",", bbox->low.x, bbox->low.y);
    result.append(lowPtStr);
    Utf8PrintfString highPtStr(LATLONG_PRINT_FORMAT_COMMA ",", bbox->high.x, bbox->high.y);
    result.append(highPtStr);

    // Remove extra comma
    if (result.size() > 1)
        result.resize(result.size() - 1);

    // Append bbox to uri.
    Utf8String fullUri;
    fullUri.append(uri);
    fullUri.append("bbox=");
    fullUri.append(result);

    return new OsmDataSource(fullUri.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
RealityPackageStatus OsmDataSource::_Read(BeXmlNodeR dataSourceNode)
    {
    // Always read base first.
    RealityPackageStatus status = T_Super::_Read(dataSourceNode);
    if (RealityPackageStatus::Success != status)
        return status;

    // Create Osm data source xml fragment from xml node.
    BeXmlStatus xmlStatus = BEXML_Success;
    xmlStatus = dataSourceNode.GetXmlString(m_osmResource);
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::XmlReadError;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
RealityPackageStatus OsmDataSource::_Write(BeXmlNodeR dataSourceNode) const
    {
    // Always write base first.
    RealityPackageStatus status = T_Super::_Write(dataSourceNode);
    if (RealityPackageStatus::Success != status)
        return status;

    if (m_osmResource.empty())
        return RealityPackageStatus::Success;

    // Create Xml Dom from string.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, m_osmResource.c_str());
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::XmlReadError;

    // Add root node.
    dataSourceNode.ImportNode(pXmlDom->GetRootElement());

    return status;
    }
