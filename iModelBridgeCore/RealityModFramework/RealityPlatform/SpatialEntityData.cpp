/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatialEntityData.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/SpatialEntityData.h>
#include <RealityPlatform/RealityDataDownload.h>


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataPtr SpatialEntityData::Create()
    {
    return new SpatialEntityData();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityData::GetName() const { return m_name; }
void SpatialEntityData::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR SpatialEntityData::GetUrl() const { return m_url; }
void SpatialEntityData::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR SpatialEntityData::GetGeoCS() const { return m_geoCS; }
void SpatialEntityData::SetGeoCS(Utf8CP geoCS) { m_geoCS = geoCS; }

Utf8StringCR SpatialEntityData::GetCompoundType() const { return m_compoundType; }
void SpatialEntityData::SetCompoundType(Utf8CP type) { m_compoundType = type; }

uint64_t SpatialEntityData::GetSize() const { return m_size; }
void SpatialEntityData::SetSize(uint64_t size) { m_size = size; }

Utf8StringCR SpatialEntityData::GetResolution() const { return m_resolution; }
void SpatialEntityData::SetResolution(Utf8CP res) { m_resolution = res; }

Utf8StringCR SpatialEntityData::GetProvider() const { return m_provider; }
void SpatialEntityData::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR SpatialEntityData::GetDataset() const { return m_dataset; }
void SpatialEntityData::SetDataset(Utf8CP dataset) { m_dataset = dataset; }

Utf8StringCR SpatialEntityData::GetClassification() const { return m_classification; }
void SpatialEntityData::SetClassification(Utf8CP classification) { m_classification = classification; }

Utf8StringCR SpatialEntityData::GetDataType() const { return m_dataType; }
void SpatialEntityData::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR SpatialEntityData::GetLocationInCompound() const { return m_locationInCompound; }
void SpatialEntityData::SetLocationInCompound(Utf8CP location) { m_locationInCompound = location; }

DateTimeCR SpatialEntityData::GetDate() const { return m_date; }
void SpatialEntityData::SetDate(DateTimeCR date) { m_date = date; }

const bvector<DPoint2d>& SpatialEntityData::GetFootprint() const { return m_footprint; }
void SpatialEntityData::SetFootprint(bvector<DPoint2d>& footprint) { m_footprint = footprint; }

DRange2dCR SpatialEntityData::GetFootprintExtents() const { return m_footprintExtents; }
void SpatialEntityData::SetFootprintExtents(DRange2dCR footprintExtents) { m_footprintExtents = footprintExtents; }

SpatialEntityThumbnailCR SpatialEntityData::GetThumbnail() const { return *m_pThumbnail; }
void SpatialEntityData::SetThumbnail(SpatialEntityThumbnailR thumbnail) { m_pThumbnail = &thumbnail; }

SpatialEntityMetadataCR SpatialEntityData::GetMetadata() const { return *m_pMetadata; }
void SpatialEntityData::SetMetadata(SpatialEntityMetadataR metadata) { m_pMetadata = &metadata; }

SpatialEntityServerCR SpatialEntityData::GetServer() const { return *m_pServer; }
void SpatialEntityData::SetServer(SpatialEntityServerR server) { m_pServer = &server; }

bool SpatialEntityData::GetIsMultiband() const { return m_isMultiband; }
void SpatialEntityData::SetIsMultiband(bool isMultiband) { m_isMultiband = isMultiband; }

Utf8String SpatialEntityData::GetMultibandUrl() const { return m_multibandDownloadUrl; }
void SpatialEntityData::SetMultibandUrl(Utf8String url) { m_multibandDownloadUrl = url; }

float SpatialEntityData::GetCloudCover() const { return m_cloudCover; }
void SpatialEntityData::SetCloudCover(float cover) { m_cloudCover = cover; }

float SpatialEntityData::GetRedBandSize() const { return m_redSize; }
void SpatialEntityData::SetRedBandSize(float size) { m_redSize = size; }

float SpatialEntityData::GetBlueBandSize() const { return m_blueSize; }
void SpatialEntityData::SetBlueBandSize(float size) { m_blueSize = size; }

float SpatialEntityData::GetGreenBandSize() const { return m_greenSize; }
void SpatialEntityData::SetGreenBandSize(float size) { m_greenSize = size; }

float SpatialEntityData::GetPanchromaticBandSize() const { return m_panchromaticSize; }
void SpatialEntityData::SetPanchromaticBandSize(float size) { m_panchromaticSize = size; }

SQLINTEGER SpatialEntityData::GetMultibandServerId() const { return m_multibandServerId; }
void SpatialEntityData::SetMultibandServerId(SQLINTEGER id) { m_multibandServerId = id; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityData::SpatialEntityData()
    {
    m_size = 0;
    m_date = DateTime();
    m_footprint = bvector<DPoint2d>();
    m_footprintExtents = DRange2d();
    m_pThumbnail = SpatialEntityThumbnail::Create();
    m_pMetadata = SpatialEntityMetadata::Create();
    m_pServer = SpatialEntityServer::Create();
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityThumbnailPtr SpatialEntityThumbnail::Create()
    {
    return new SpatialEntityThumbnail();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityThumbnail::GetProvenance() const { return m_provenance; }
void SpatialEntityThumbnail::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR SpatialEntityThumbnail::GetFormat() const { return m_format; }
void SpatialEntityThumbnail::SetFormat(Utf8CP format) { m_format = format; }

uint32_t SpatialEntityThumbnail::GetWidth() const { return m_width; }
void SpatialEntityThumbnail::SetWidth(uint32_t width) { m_width = width; }

uint32_t SpatialEntityThumbnail::GetHeight() const { return m_height; }
void SpatialEntityThumbnail::SetHeight(uint32_t height) { m_height = height; }

DateTimeCR SpatialEntityThumbnail::GetStamp() const { return m_stamp; }
void SpatialEntityThumbnail::SetStamp(DateTimeCR date) { m_stamp = date; }

const bvector<Byte>& SpatialEntityThumbnail::GetData() const { return m_data; }
void SpatialEntityThumbnail::SetData(const bvector<Byte>& data) { m_data = data; }

Utf8StringCR SpatialEntityThumbnail::GetGenerationDetails() const { return m_generationDetails; }
void SpatialEntityThumbnail::SetGenerationDetails(Utf8CP details) { m_generationDetails = details; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityThumbnail::SpatialEntityThumbnail()
    {
    m_width = THUMBNAIL_WIDTH;
    m_height = THUMBNAIL_HEIGHT;
    m_stamp = DateTime::GetCurrentTimeUtc();
    m_data = bvector<Byte>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadataPtr SpatialEntityMetadata::Create()
    {
    return new SpatialEntityMetadata();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadataPtr SpatialEntityMetadata::CreateFromFile(Utf8CP filePath)
    {
    return new SpatialEntityMetadata(filePath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityMetadata::GetProvenance() const { return m_provenance; }
void SpatialEntityMetadata::SetProvenance(Utf8CP provenance) { m_provenance = provenance; }

Utf8StringCR SpatialEntityMetadata::GetDescription() const { return m_description; }
void SpatialEntityMetadata::SetDescription(Utf8CP description) { m_description = description; }

Utf8StringCR SpatialEntityMetadata::GetContactInfo() const { return m_contactInfo; }
void SpatialEntityMetadata::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR SpatialEntityMetadata::GetLegal() const { return m_legal; }
void SpatialEntityMetadata::SetLegal(Utf8CP legal) { m_legal = legal; }

Utf8StringCR SpatialEntityMetadata::GetFormat() const { return m_format; }
void SpatialEntityMetadata::SetFormat(Utf8CP format) { m_format = format; }

Utf8StringCR SpatialEntityMetadata::GetData() const { return m_data; }
void SpatialEntityMetadata::SetData(Utf8CP data) { m_data = data; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata(Utf8CP filePath)
    {
    BeFileName metadataFile(filePath);
    Utf8String provenance(metadataFile.GetFileNameAndExtension());
    m_provenance = provenance;
    Utf8String format(metadataFile.GetExtension());
    m_format = format;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filePath, NULL);
    if (BEXML_Success == xmlStatus)
        {
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL != pRootNode)
            {
            // Convert to string.
            pRootNode->GetXmlString(m_data);
            }
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServerPtr SpatialEntityServer::Create()
    {
    return new SpatialEntityServer();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServerPtr SpatialEntityServer::Create(Utf8CP url, Utf8CP name)
    {
    return new SpatialEntityServer(url, name);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityServer::GetProtocol() const { return m_protocol; }
void SpatialEntityServer::SetProtocol(Utf8CP protocol) { m_protocol = protocol; }

Utf8StringCR SpatialEntityServer::GetName() const { return m_name; }
void SpatialEntityServer::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR SpatialEntityServer::GetUrl() const { return m_url; }
void SpatialEntityServer::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR SpatialEntityServer::GetContactInfo() const { return m_contactInfo; }
void SpatialEntityServer::SetContactInfo(Utf8CP info) { m_contactInfo = info; }

Utf8StringCR SpatialEntityServer::GetLegal() const { return m_legal; }
void SpatialEntityServer::SetLegal(Utf8CP legal) { m_legal = legal; }

bool SpatialEntityServer::IsOnline() const { return m_online; }
void SpatialEntityServer::SetOnline(bool online) { m_online = online; }

DateTimeCR SpatialEntityServer::GetLastCheck() const { return m_lastCheck; }
void SpatialEntityServer::SetLastCheck(DateTimeCR data) { m_lastCheck = data; }

DateTimeCR SpatialEntityServer::GetLastTimeOnline() const { return m_lastTimeOnline; }
void SpatialEntityServer::SetLastTimeOnline(DateTimeCR data) { m_lastTimeOnline = data; }

double SpatialEntityServer::GetLatency() const { return m_latency; }
void SpatialEntityServer::SetLatency(double latency) { m_latency = latency; }

Utf8StringCR SpatialEntityServer::GetState() const { return m_state; }
void SpatialEntityServer::SetState(Utf8CP state) { m_state = state; }

Utf8StringCR SpatialEntityServer::GetType() const { return m_type; }
void SpatialEntityServer::SetType(Utf8CP type) { m_type = type; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServer::SpatialEntityServer()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServer::SpatialEntityServer(Utf8CP url, Utf8CP name)
    : m_url(url), m_name(name)
    {
    // Default values.
    m_online = true;
    m_lastCheck = DateTime::GetCurrentTimeUtc();
    m_lastTimeOnline = DateTime::GetCurrentTimeUtc();
    m_latency = 0.0;

    if (!m_url.empty())
        {
        // Extract protocol and type from url.
        Utf8String protocol(url);
        m_protocol = protocol.substr(0, protocol.find_first_of(":"));
        m_type = m_protocol;

        if (m_name.empty())
            {
            // No server name was provided, try to extract it from url.
            Utf8String name(url);
            size_t beginPos = name.find_first_of("://") + 3;
            size_t pos = name.find_last_of(".");
            size_t endPos = name.find("/", pos);
            m_name = name.substr(beginPos, endPos - beginPos);
            }
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityStatus SpatialEntityDataHandler::UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath)
    {
    // Get a list of zip files to process.
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(inputDirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.zip", true);

    // Unzip files.    
    for (size_t i = 0; i < fileFoundList.size(); ++i)
        {
        WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
        AString outputDirPathA(outputDirPath);

        // Construct output path.
        WString outputFolderName;
        RealityDataDownload::ExtractFileName(outputFolderName, fileFoundList[i].GetNameUtf8());
        outputFolderName.erase(outputFolderName.find_last_of('.'));
        outputDirPathW.append(outputFolderName);
        BeFileName::CreateNewDirectory(outputDirPathW.c_str());

        WString filenameW(fileFoundList[i].GetName());
        RealityDataDownload::UnZipFile(filenameW, outputDirPathW);
        }

    return SpatialEntityStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
BeFileName SpatialEntityDataHandler::BuildMetadataFilename(Utf8CP dirPath)
    {
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(dirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.xml", false);

    if (fileFoundList.empty())
        return BeFileName();

    // Find the xml file corresponding to the metadata.
    for (BeFileNameCR file : fileFoundList)
        {
        // Create xmlDom from file.
        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, file.GetNameUtf8().c_str());
        if (BEXML_Success != xmlStatus)
            {
            return BeFileName();
            }

        // Make sure the root node is <gmd:MD_Metadata>.
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL == pRootNode)
            return BeFileName();

        if (pRootNode->IsIName("MD_Metadata"))
            return file;
        }

    // No metadata file found.
    return BeFileName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
Utf8String SpatialEntityDataHandler::RetrieveGeocodingFromMetadata(BeFileNameCR filename)
    {
    Utf8String geocoding;

    // Create xmlDom from metadata file.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename.GetNameUtf8().c_str());
    if (BEXML_Success != xmlStatus)
        {
        return NULL;
        }

    pXmlDom->RegisterNamespace("gmd", "http://www.isotc211.org/2005/gmd");

    // Get root node.
    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Get reference system info node.
    BeXmlNodeP pRefSysNode = pRootNode->SelectSingleNode("gmd:referenceSystemInfo");
    if (NULL == pRefSysNode)
        return NULL;

    // Get md reference system node.
    BeXmlNodeP pMdRefNode = pRefSysNode->SelectSingleNode("gmd:MD_ReferenceSystem");
    if (NULL == pMdRefNode)
        return NULL;

    // Get reference system identifier node.
    BeXmlNodeP pRefSysIdNode = pMdRefNode->SelectSingleNode("gmd:referenceSystemIdentifier");
    if (NULL == pRefSysIdNode)
        return NULL;

    // Get rs identifier node.
    BeXmlNodeP pRsIdNode = pRefSysIdNode->SelectSingleNode("gmd:RS_Identifier");
    if (NULL == pRsIdNode)
        return NULL;

    // Get code.
    BeXmlNodeP pCodeNode = pRsIdNode->SelectSingleNode("gmd:code");
    if (NULL == pCodeNode)
        return NULL;

    xmlStatus = pCodeNode->GetContent(geocoding);
    if (BEXML_Success != xmlStatus)
        return NULL;

    return geocoding.Trim();
    }

