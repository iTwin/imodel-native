/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatialEntity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/SpatialEntity.h>


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr SpatialEntityDataSource::Create()
    {
    return new SpatialEntityDataSource();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   4/2016
//-------------------------------------------------------------------------------------

Utf8StringCR SpatialEntityDataSource::GetId() const { return m_id; }
void SpatialEntityDataSource::SetId(Utf8CP id) { m_id = id; }

Utf8StringCR SpatialEntityDataSource::GetUrl() const { return m_url; }
void SpatialEntityDataSource::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR SpatialEntityDataSource::GetGeoCS() const { return m_geoCS; }
void SpatialEntityDataSource::SetGeoCS(Utf8CP geoCS) { m_geoCS = geoCS; }

Utf8StringCR SpatialEntityDataSource::GetCompoundType() const { return m_compoundType; }
void SpatialEntityDataSource::SetCompoundType(Utf8CP type) { m_compoundType = type; }

uint64_t SpatialEntityDataSource::GetSize() const { return m_size; }
void SpatialEntityDataSource::SetSize(uint64_t size) { m_size = size; }
void SpatialEntityDataSource::SetSize(Utf8CP size) { BeStringUtilities::ParseUInt64(m_size, size); }

Utf8StringCR SpatialEntityDataSource::GetNoDataValue() const { return m_noDataValue; }
void SpatialEntityDataSource::SetNoDataValue(Utf8CP value) { m_noDataValue = value; }

Utf8StringCR SpatialEntityDataSource::GetDataType() const { return m_dataType; }
void SpatialEntityDataSource::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR SpatialEntityDataSource::GetLocationInCompound() const { return m_locationInCompound; }
void SpatialEntityDataSource::SetLocationInCompound(Utf8CP location) { m_locationInCompound = location; }

SpatialEntityServerCP SpatialEntityDataSource::GetServerCP() const { return m_pServer.get(); }
void SpatialEntityDataSource::SetServer(SpatialEntityServerP server) { m_pServer = server; }

Utf8StringCR SpatialEntityDataSource::GetCoordinateSystem() const { return m_coordinateSystem; }
void SpatialEntityDataSource::SetCoordinateSystem(Utf8CP coordSys) { m_coordinateSystem = coordSys; }

bool SpatialEntityDataSource::GetIsMultiband() const { return m_isMultiband; }
void SpatialEntityDataSource::SetIsMultiband(bool isMultiband) { m_isMultiband = isMultiband; }

void SpatialEntityDataSource::GetMultibandUrls(Utf8String& redUrl, Utf8String& greenUrl, Utf8String& blueUrl, Utf8String& panchromaticUrl) const 
    { 
    redUrl = m_redDownloadUrl;
    blueUrl = m_blueDownloadUrl;
    greenUrl = m_greenDownloadUrl;
    panchromaticUrl = m_panchromaticDownloadUrl;
    }

void SpatialEntityDataSource::SetMultibandUrls(Utf8String redUrl, Utf8String greenUrl, Utf8String blueUrl, Utf8String panchromaticUrl) 
    { 
    m_redDownloadUrl = redUrl;
    m_blueDownloadUrl = blueUrl;
    m_greenDownloadUrl = greenUrl;
    m_panchromaticDownloadUrl = panchromaticUrl;
    }

uint64_t SpatialEntityDataSource::GetRedBandSize() const { return m_redSize; }
void SpatialEntityDataSource::SetRedBandSize(uint64_t size) { m_redSize = size; }

uint64_t SpatialEntityDataSource::GetBlueBandSize() const { return m_blueSize; }
void SpatialEntityDataSource::SetBlueBandSize(uint64_t size) { m_blueSize = size; }

uint64_t SpatialEntityDataSource::GetGreenBandSize() const { return m_greenSize; }
void SpatialEntityDataSource::SetGreenBandSize(uint64_t size) { m_greenSize = size; }

uint64_t SpatialEntityDataSource::GetPanchromaticBandSize() const { return m_panchromaticSize; }
void SpatialEntityDataSource::SetPanchromaticBandSize(uint64_t size) { m_panchromaticSize = size; }

long int SpatialEntityDataSource::GetServerId() const { return m_serverId; }
void SpatialEntityDataSource::SetServerId(long int id) const { m_serverId = id; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSource::SpatialEntityDataSource()
    {
    m_size = 0;
    m_redSize = 0;
    m_blueSize = 0;
    m_greenSize = 0;
    m_panchromaticSize = 0;

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityPtr SpatialEntity::Create()
    {
    return new SpatialEntity();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityPtr SpatialEntity::Create(Utf8StringCR identifier, const DateTime& date, Utf8String const & resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name, Utf8StringCR coordSys)
    {
    SpatialEntityPtr mySpatialEntity = new SpatialEntity();

    mySpatialEntity->SetIdentifier(identifier.c_str());
    mySpatialEntity->SetDate(date);
    mySpatialEntity->SetResolution(resolution.c_str());
    mySpatialEntity->SetFootprint(footprint, coordSys);
    mySpatialEntity->SetName(name.c_str());

    return mySpatialEntity;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntity::GetProvider() const { return m_provider; }
void SpatialEntity::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR SpatialEntity::GetProviderName() const { return m_providerName; }
void SpatialEntity::SetProviderName(Utf8CP providerName) { m_providerName = providerName; }

Utf8StringCR SpatialEntity::GetThumbnailURL() const { return m_thumbnailURL; }
void SpatialEntity::SetThumbnailURL(Utf8CP thumbnailURL) { m_thumbnailURL = thumbnailURL; }

Utf8StringCR SpatialEntity::GetThumbnailLoginKey() const { return m_thumbnailLoginKey; }
void SpatialEntity::SetThumbnailLoginKey(Utf8CP thumbnailLoginKey) { m_thumbnailLoginKey = thumbnailLoginKey; }

Utf8StringCR SpatialEntity::GetDataType() const { return m_dataType; }
void SpatialEntity::SetDataType(Utf8CP type) { m_dataType = type; }

DateTimeCR SpatialEntity::GetDate() const { return m_date; }
void SpatialEntity::SetDate(DateTimeCR date) { m_date = date; }

SpatialEntityMetadataCP SpatialEntity::GetMetadataCP() const { return m_pMetadata.get(); }
SpatialEntityMetadataP SpatialEntity::GetMetadataP()  { return m_pMetadata.get(); }
void SpatialEntity::SetMetadata(SpatialEntityMetadataP metadata) { m_pMetadata = metadata; }

SpatialEntityDataSourceCR SpatialEntity::GetDataSource(size_t index) const { return *m_DataSources[index]; }
SpatialEntityDataSourceR SpatialEntity::GetDataSource(size_t index) { return *m_DataSources[index]; }
void SpatialEntity::AddDataSource(SpatialEntityDataSourceR dataSource) { m_DataSources.push_back(&dataSource); }
size_t SpatialEntity::GetDataSourceCount() const {return m_DataSources.size();}

float SpatialEntity::GetOcclusion() const { return m_occlusion; }
void SpatialEntity::SetOcclusion(float cover) { BeAssert(cover <=100.0); m_occlusion = cover; }

uint64_t SpatialEntity::GetApproximateFileSize() const {return m_approximateFileSize;}
void SpatialEntity::SetApproximateFileSize(uint64_t size) {m_approximateFileSize = size;}



//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntity::SpatialEntity()
    {
    m_date = DateTime();

    m_approximateFileSize = 0;
    m_occlusion = 0.0f;
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
void SpatialEntityThumbnail::SetProvenance(Utf8CP provenance) { m_provenance = provenance; m_isEmpty = false;}

Utf8StringCR SpatialEntityThumbnail::GetFormat() const { return m_format; }
void SpatialEntityThumbnail::SetFormat(Utf8CP format) { m_format = format;  m_isEmpty = false;}

uint32_t SpatialEntityThumbnail::GetWidth() const { return m_width; }
void SpatialEntityThumbnail::SetWidth(uint32_t width) { m_width = width;  m_isEmpty = false;}

uint32_t SpatialEntityThumbnail::GetHeight() const { return m_height; }
void SpatialEntityThumbnail::SetHeight(uint32_t height) { m_height = height;  m_isEmpty = false;}

DateTimeCR SpatialEntityThumbnail::GetStamp() const { return m_stamp; }
void SpatialEntityThumbnail::SetStamp(DateTimeCR date) { m_stamp = date;  m_isEmpty = false;}

const bvector<Byte>& SpatialEntityThumbnail::GetData() const { return m_data; }
void SpatialEntityThumbnail::SetData(const bvector<Byte>& data) { m_data = data;  m_isEmpty = false;}

Utf8StringCR SpatialEntityThumbnail::GetGenerationDetails() const { return m_generationDetails; }
void SpatialEntityThumbnail::SetGenerationDetails(Utf8CP details) { m_generationDetails = details;  m_isEmpty = false;}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert       	    11/2016
//-------------------------------------------------------------------------------------
bool SpatialEntityThumbnail::IsEmpty() const { return m_isEmpty; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityThumbnail::SpatialEntityThumbnail() : m_isEmpty(true)
    {
    m_width = 0;
    m_height = 0;
    // Default DataTime is valid for stamp
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
SpatialEntityMetadataPtr SpatialEntityMetadata::CreateFromFile(Utf8CP filePath, SpatialEntityMetadataCR metadataSeed)
    {
    return new SpatialEntityMetadata(filePath, metadataSeed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert          	    01/2017
//-------------------------------------------------------------------------------------
SpatialEntityMetadataPtr SpatialEntityMetadata::CreateFromMetadata(SpatialEntityMetadataCR metadataSeed)
    {
    return new SpatialEntityMetadata(metadataSeed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
Utf8StringCR SpatialEntityMetadata::GetId() const { return m_id; }
void SpatialEntityMetadata::SetId(Utf8CP id) { m_id = id; }

Utf8StringCR SpatialEntityMetadata::GetProvenance() const { return m_provenance; }
void SpatialEntityMetadata::SetProvenance(Utf8CP provenance) { m_provenance = provenance;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetLineage() const { return m_lineage; }
void SpatialEntityMetadata::SetLineage(Utf8CP lineage) { m_lineage = lineage; m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetDescription() const { return m_description; }
void SpatialEntityMetadata::SetDescription(Utf8CP description) { m_description = description;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetContactInfo() const { return m_contactInfo; }
void SpatialEntityMetadata::SetContactInfo(Utf8CP info) { m_contactInfo = info;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetLegal() const { return m_legal; }
void SpatialEntityMetadata::SetLegal(Utf8CP legal) { m_legal = legal;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetTermsOfUse() const { return m_termsOfUse; }
void SpatialEntityMetadata::SetTermsOfUse(Utf8CP termsOfUse) { m_termsOfUse = termsOfUse;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetKeywords() const { return m_keywords; }
void SpatialEntityMetadata::SetKeywords(Utf8CP keywords) { m_keywords = keywords;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetFormat() const { return m_format; }
void SpatialEntityMetadata::SetFormat(Utf8CP format) { m_format = format;  m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetMetadataUrl() const { return m_metadataUrl; }
void SpatialEntityMetadata::SetMetadataUrl(Utf8CP metadataUrl) { m_metadataUrl = metadataUrl; m_isEmpty = false;}

Utf8StringCR SpatialEntityMetadata::GetDisplayStyle() const { return m_displayStyle; }
void SpatialEntityMetadata::SetDisplayStyle(Utf8CP displayStyle) { m_displayStyle = displayStyle; m_isEmpty = false;}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    11/2016
//-------------------------------------------------------------------------------------
bool SpatialEntityMetadata::IsEmpty() const   {return m_isEmpty;}


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata() : m_isEmpty(true)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata(Utf8CP filePath, SpatialEntityMetadataCR metadataSeed) : m_isEmpty(false)
    {
    BeFileName metadataFile(filePath);
    Utf8String provenance(metadataFile.GetFileNameAndExtension());
    m_provenance = provenance;
    Utf8String format(metadataFile.GetExtension());
    m_format = format;

    if (metadataSeed.GetTermsOfUse().size() > 0)
        m_termsOfUse = metadataSeed.GetTermsOfUse();

    if (metadataSeed.GetDescription().size() > 0)
        m_description = metadataSeed.GetDescription();

    if (metadataSeed.GetLegal().size() > 0)
        m_legal = metadataSeed.GetLegal();

    if (metadataSeed.GetContactInfo().size() > 0)
        m_contactInfo = metadataSeed.GetContactInfo();

    if (metadataSeed.GetLineage().size() > 0)
        m_lineage = metadataSeed.GetLineage();

    if (metadataSeed.GetKeywords().size() > 0)
        m_keywords = metadataSeed.GetKeywords();

    if (metadataSeed.GetMetadataUrl().size() > 0)
        m_metadataUrl = metadataSeed.GetMetadataUrl();

    if (metadataSeed.GetDisplayStyle().size() > 0)
        m_displayStyle = metadataSeed.GetDisplayStyle();


    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filePath, NULL);
    if (BEXML_Success == xmlStatus)
        {
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL != pRootNode)
            {
            // Convert to string.
//&&AR We do not keep raw data anymore.
            // pRootNode->GetXmlString(m_data);
            }
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityMetadata::SpatialEntityMetadata(SpatialEntityMetadataCR metadataSeed) : m_isEmpty(metadataSeed.m_isEmpty)
    {
    m_provenance = metadataSeed.GetProvenance();

    m_lineage = metadataSeed.GetLineage();

    m_format = metadataSeed.GetFormat();

    m_termsOfUse = metadataSeed.GetTermsOfUse();

    m_description = metadataSeed.GetDescription();

    m_legal = metadataSeed.GetLegal();

    m_contactInfo = metadataSeed.GetContactInfo();

    m_keywords = metadataSeed.GetKeywords();

    m_metadataUrl = metadataSeed.GetMetadataUrl();

    m_displayStyle = metadataSeed.GetDisplayStyle();
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

Utf8StringCR SpatialEntityServer::GetId() const { return m_id; }
void SpatialEntityServer::SetId(Utf8CP id) { m_id = id; }

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
void SpatialEntityServer::SetLastCheck(DateTimeCR date) { m_lastCheck = date; }
void SpatialEntityServer::SetLastCheck(Utf8CP date) { DateTime::FromString(m_lastCheck, date); }

DateTimeCR SpatialEntityServer::GetLastTimeOnline() const { return m_lastTimeOnline; }
void SpatialEntityServer::SetLastTimeOnline(DateTimeCR date) { m_lastTimeOnline = date; }
void SpatialEntityServer::SetLastTimeOnline(Utf8CP date) { DateTime::FromString(m_lastTimeOnline, date); }

double SpatialEntityServer::GetLatency() const { return m_latency; }
void SpatialEntityServer::SetLatency(double latency) { m_latency = latency; }

Utf8StringCR SpatialEntityServer::GetState() const { return m_state; }
void SpatialEntityServer::SetState(Utf8CP state) { m_state = state; }

Utf8StringCR SpatialEntityServer::GetType() const { return m_type; }
void SpatialEntityServer::SetType(Utf8CP type) { m_type = type; }

Utf8StringCR SpatialEntityServer::GetLoginKey() const { return m_loginKey; }
void SpatialEntityServer::SetLoginKey(Utf8CP loginKey) { m_loginKey = loginKey; }

Utf8StringCR SpatialEntityServer::GetLoginMethod() const { return m_loginMethod; }
void SpatialEntityServer::SetLoginMethod(Utf8CP loginMethod) { m_loginMethod = loginMethod; }

bool SpatialEntityServer::IsStreamed() const { return m_streamed; }
void SpatialEntityServer::SetStreamed(bool streamed) { m_streamed = streamed; }

Utf8StringCR SpatialEntityServer::GetRegistrationPage() const { return m_registrationPage; }
void SpatialEntityServer::SetRegistrationPage(Utf8CP registrationPage) { m_registrationPage = registrationPage; }

Utf8StringCR SpatialEntityServer::GetOrganisationPage() const { return m_organisationPage; }
void SpatialEntityServer::SetOrganisationPage(Utf8CP organisationPage) { m_organisationPage = organisationPage; }

Utf8StringCR SpatialEntityServer::GetFees() const { return m_fees; }
void SpatialEntityServer::SetFees(Utf8CP fees) { m_fees = fees; }

Utf8StringCR SpatialEntityServer::GetAccessConstraints() const { return m_accessConstraints; }
void SpatialEntityServer::SetAccessConstraints(Utf8CP accessConstraints) { m_accessConstraints = accessConstraints; }

uint64_t SpatialEntityServer::GetMeanReachabilityStats() const { return m_meanReachabilityStats; }
void SpatialEntityServer::SetMeanReachabilityStats(uint64_t meanReachabilityStats) { m_meanReachabilityStats = meanReachabilityStats; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityServer::SpatialEntityServer()
    {
    m_online = true;
    m_streamed = false;
    m_latency = 0.0;
    m_meanReachabilityStats = 0;
    }

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
    m_streamed = false;
    m_meanReachabilityStats = 0;

    if (!m_url.empty())
        {
        // Extract protocol and type from url.
        Utf8String protocol(url);
        m_protocol = protocol.substr(0, protocol.find_first_of(":"));
        m_type = m_protocol;

        if (m_name.empty())
            {
            // No server name was provided, try to extract it from url.
            Utf8String nameUrl(url);
            size_t beginPos = nameUrl.find_first_of("://") + 3;
            size_t pos = nameUrl.find_last_of(".");
            size_t endPos = nameUrl.find("/", pos);
            m_name = nameUrl.substr(beginPos, endPos - beginPos);
            }
        }
    }


