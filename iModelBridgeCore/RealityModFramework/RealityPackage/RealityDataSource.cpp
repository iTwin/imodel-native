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
//                                    Uri
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
UriPtr Uri::Create(Utf8CP resourceIdentifier)
    {
    Utf8String resIdStr(resourceIdentifier);
    size_t pos = resIdStr.find("#");
    if (pos == Utf8String::npos)
        return new Uri(resourceIdentifier, NULL);

    Utf8String source = resIdStr.substr(0, pos);
    Utf8String fileInCompound = resIdStr.substr(pos+1);
    return new Uri(source.c_str(), fileInCompound.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
UriPtr Uri::Create(Utf8CP source, Utf8CP fileInCompound)
    {
    return new Uri(source, fileInCompound);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8StringCR Uri::GetSource() const { return m_source; }
Utf8StringCR Uri::GetFileInCompound() const { return m_fileInCompound; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8String Uri::ToString() const
    {
    if (m_fileInCompound.empty())
        return m_source;

    return (m_source + "#" + m_fileInCompound);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Uri::Uri(Utf8CP source, Utf8CP fileInCompound)
    :m_source(source), m_fileInCompound(fileInCompound) 
    {}


//=======================================================================================
//                              RealityDataSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::Create(UriR uri, Utf8CP type)
{
    if (uri.ToString().empty())
        return NULL;

    return new RealityDataSource(uri, type);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::Create(Utf8CP uri, Utf8CP type)
    {
    if (Utf8String::IsNullOrEmpty(uri))
        return NULL;

    return new RealityDataSource(uri, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
UriCR RealityDataSource::GetUri() const { return *m_pUri; }
void RealityDataSource::SetUri(UriR uri) { m_pUri = &uri; }

Utf8StringCR RealityDataSource::GetType() const { return m_type; }
void RealityDataSource::SetType(Utf8CP type) { m_type = type; }

Utf8StringCR RealityDataSource::GetId() const { return m_id; }
void RealityDataSource::SetId(Utf8CP id) { m_id = id; }

Utf8StringCR RealityDataSource::GetCopyright() const { return m_copyright; }
void RealityDataSource::SetCopyright(Utf8CP copyright) { m_copyright = copyright; }

Utf8StringCR RealityDataSource::GetTermOfUse() const { return m_termOfUse; }
void RealityDataSource::SetTermOfUse(Utf8CP termOfUse) { m_termOfUse = termOfUse; }

Utf8StringCR RealityDataSource::GetProvider() const { return m_provider; }
void RealityDataSource::SetProvider(Utf8CP provider) { m_provider = provider; }

uint64_t RealityDataSource::GetSize() const { return m_size; }
void RealityDataSource::SetSize(uint64_t size) { m_size = size; }

Utf8StringCR RealityDataSource::GetMetadata() const { return m_metadata; }
void RealityDataSource::SetMetadata(Utf8CP metadata) { m_metadata = metadata; }

Utf8StringCR RealityDataSource::GetMetadataType() const { return m_metadataType; }
void RealityDataSource::SetMetadataType(Utf8CP type) { m_metadataType = type; }

Utf8StringCR RealityDataSource::GetGeoCS() const { return m_geocs; }
void RealityDataSource::SetGeoCS(Utf8CP geocs) { m_geocs = geocs; }

Utf8StringCR RealityDataSource::GetNoDataValue() const { return m_nodatavalue; }
void RealityDataSource::SetNoDataValue(Utf8CP nodatavalue) { m_nodatavalue = nodatavalue; }

const bvector<UriPtr>& RealityDataSource::GetSisterFiles() const { return m_sisterFiles; }
void RealityDataSource::SetSisterFiles(const bvector<UriPtr>& sisterFiles) { m_sisterFiles = sisterFiles; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8CP RealityDataSource::GetElementName() const
    {
    return _GetElementName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataSource::RealityDataSource(UriR uri, Utf8CP type)
    {
    BeAssert(!uri.ToString().empty());
    m_pUri = &uri;
    m_type = type;

    m_size = 0;     // Default.
    m_sisterFiles = bvector<UriPtr>();      // Create empty.
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::RealityDataSource(Utf8CP uri, Utf8CP type)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(uri));
    m_pUri = Uri::Create(uri);
    m_type = type;

    m_size = 0;     // Default.
    m_sisterFiles = bvector<UriPtr>();      // Create empty.
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::~RealityDataSource() {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP RealityDataSource::_GetElementName() const
    {
    BeAssert("Child class must override _GetElementName()" && typeid(*this) == typeid(RealityDataSource));
    return PACKAGE_ELEMENT_Source;
    }

//=======================================================================================
//                              WmsDataSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
WmsDataSourcePtr WmsDataSource::Create(UriR uri)
{
    if (uri.ToString().empty())
        return NULL;

    return new WmsDataSource(uri);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSourcePtr WmsDataSource::Create(Utf8CP uri)
    {
    if (Utf8String::IsNullOrEmpty(uri))
        return NULL;

    UriPtr pUri = Uri::Create(uri);

    return new WmsDataSource(*pUri);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
Utf8StringCR WmsDataSource::GetMapSettings() const { return m_mapSettings; }
void WmsDataSource::SetMapSettings(Utf8CP mapSettings) { m_mapSettings = mapSettings; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::WmsDataSource(UriR uri)
    :RealityDataSource(uri, WMS_SOURCE_TYPE)
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::~WmsDataSource() {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP WmsDataSource::_GetElementName() const {return PACKAGE_ELEMENT_WmsSource;}



//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
/*
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
*/

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
/*
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
*/

//=======================================================================================
//                              OsmDataSource
//=======================================================================================
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
/*
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
*/

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
/*
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
*/


//=======================================================================================
//                              MultiBandSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSourcePtr MultiBandSource::Create(UriR uri, Utf8CP type)
    {
    if (uri.ToString().empty())
        return NULL;

    return new MultiBandSource(uri, type);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataSourceCP MultiBandSource::GetRedBand() const { return m_pRedBand.get(); }
void MultiBandSource::SetRedBand(RealityDataSourceR band) { m_pRedBand = &band; }

RealityDataSourceCP MultiBandSource::GetGreenBand() const { return m_pGreenBand.get(); }
void MultiBandSource::SetGreenBand(RealityDataSourceR band) { m_pGreenBand = &band; }

RealityDataSourceCP MultiBandSource::GetBlueBand() const { return m_pBlueBand.get(); }
void MultiBandSource::SetBlueBand(RealityDataSourceR band) { m_pBlueBand = &band; }

RealityDataSourceCP MultiBandSource::GetPanchromaticBand() const { return m_pPanchromaticBand.get(); }
void MultiBandSource::SetPanchromaticBand(RealityDataSourceR band) { m_pPanchromaticBand = &band; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSource::MultiBandSource(UriR uri, Utf8CP type)
    :RealityDataSource(uri, type)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSource::~MultiBandSource() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8CP MultiBandSource::_GetElementName() const { return PACKAGE_ELEMENT_MultiBandSource; }

