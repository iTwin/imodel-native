/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatialEntity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityPlatform/SpatialEntity.h>
#include "RealitySerialization.h"


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM



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
    Utf8String fileInCompound = resIdStr.substr(pos + 1);
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


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr SpatialEntityDataSource::Create()
    {
    return new SpatialEntityDataSource();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr SpatialEntityDataSource::Create(UriR uri, Utf8CP type)
    {
    if (uri.ToString().empty())
        return NULL;

    return new SpatialEntityDataSource(uri, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr SpatialEntityDataSource::Create(Utf8CP uri, Utf8CP type)
    {
    if (Utf8String::IsNullOrEmpty(uri))
        return NULL;

    return new SpatialEntityDataSource(uri, type);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSource::SpatialEntityDataSource(UriR uri, Utf8CP type)
    {
    BeAssert(!uri.ToString().empty());
    m_pUri = &uri;
    m_dataType = type;
    m_pMetadata = SpatialEntityMetadata::Create();
    m_pServer = SpatialEntityServer::Create();
    m_visibility = RealityDataBase::Visibility::UNDEFINED_VISIBILITY;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
SpatialEntityDataSource::SpatialEntityDataSource(Utf8CP uri, Utf8CP type)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(uri));
    m_pUri = Uri::Create(uri);
    m_dataType = type;
    m_pMetadata = SpatialEntityMetadata::Create();
    m_pServer = SpatialEntityServer::Create();
    m_visibility = RealityDataBase::Visibility::UNDEFINED_VISIBILITY;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   4/2016
//-------------------------------------------------------------------------------------

Utf8StringCR SpatialEntityDataSource::GetId() const { return m_id; }
void SpatialEntityDataSource::SetId(Utf8CP id) { m_id = id; }

UriCR SpatialEntityDataSource::GetUri() const { return *m_pUri; }
void SpatialEntityDataSource::SetUri(UriR uri) { m_pUri = &uri; }

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
SpatialEntityServerP SpatialEntityDataSource::GetServerP() { return m_pServer.get(); }
void SpatialEntityDataSource::SetServer(SpatialEntityServerPtr server) { m_pServer = server; }

Utf8StringCR SpatialEntityDataSource::GetCoordinateSystem() const { return m_coordinateSystem; }
void SpatialEntityDataSource::SetCoordinateSystem(Utf8CP coordSys) { m_coordinateSystem = coordSys; }

RealityDataBase::Visibility SpatialEntityDataSource::GetVisibility() const { return m_visibility; }
void SpatialEntityDataSource::SetVisibility(RealityDataBase::Visibility visibility) { m_visibility = visibility; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8String SpatialEntityDataSource::GetVisibilityTag() const
    {
    return GetTagFromVisibility(m_visibility);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
//! Static method that converts a classification tag to a classification
StatusInt SpatialEntityDataSource::GetVisibilityFromTag(RealityDataBase::Visibility& returnedVisibility, Utf8CP visibilityTag)
    {
    return RealityDataBase::GetVisibilityFromTag(returnedVisibility, visibilityTag);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8String SpatialEntityDataSource::GetTagFromVisibility(RealityDataBase::Visibility visibility)
    {
    return RealityDataBase::GetTagFromVisibility(visibility);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
StatusInt SpatialEntityDataSource::SetVisibilityByTag(Utf8CP visibilityTag)
    {
    return GetVisibilityFromTag(m_visibility, visibilityTag);
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
    :SpatialEntityDataSource(uri, WMS_SOURCE_TYPE)
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WmsDataSource::~WmsDataSource() {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP WmsDataSource::_GetElementName() const { return PACKAGE_ELEMENT_WmsSource; }


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
    :SpatialEntityDataSource(uri, OSM_SOURCE_TYPE)
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
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSource::MultiBandSource(UriR uri, Utf8CP type)
    :SpatialEntityDataSource(uri, type)
    {}


void MultiBandSource::SetMultibandUrls(Utf8StringCR redUrl, Utf8StringCR greenUrl, Utf8StringCR blueUrl, Utf8StringCR panchromaticUrl)
    {
    m_pRedBand = SpatialEntityDataSource::Create();
    UriPtr redUri = Uri::Create(redUrl.c_str());
    m_pRedBand->SetUri(*redUri);

    m_pGreenBand = SpatialEntityDataSource::Create();
    UriPtr greenUri = Uri::Create(greenUrl.c_str());
    m_pGreenBand->SetUri(*greenUri);

    m_pBlueBand = SpatialEntityDataSource::Create();
    UriPtr blueUri = Uri::Create(blueUrl.c_str());
    m_pBlueBand->SetUri(*blueUri);

    m_pPanchromaticBand = SpatialEntityDataSource::Create();
    UriPtr panchromaticUri = Uri::Create(panchromaticUrl.c_str());
    m_pPanchromaticBand->SetUri(*panchromaticUri);
    }

void MultiBandSource::GetMultibandUrls(Utf8String& redUrl, Utf8String& greenUrl, Utf8String& blueUrl, Utf8String& panchromaticUrl) const
    { 
    redUrl = m_pRedBand->GetUri().ToString();
    blueUrl = m_pBlueBand->GetUri().ToString();
    greenUrl = m_pGreenBand->GetUri().ToString();
    panchromaticUrl = m_pPanchromaticBand->GetUri().ToString();
    }

uint64_t MultiBandSource::GetRedBandSize() const { return m_pRedBand->GetSize(); }
void MultiBandSource::SetRedBandSize(uint64_t size)
    {
    m_pRedBand->SetSize(size);
    }

uint64_t MultiBandSource::GetBlueBandSize() const { return m_pBlueBand->GetSize(); }
void MultiBandSource::SetBlueBandSize(uint64_t size)
    {
    m_pBlueBand->SetSize(size);
    }

uint64_t MultiBandSource::GetGreenBandSize() const { return m_pGreenBand->GetSize(); }
void MultiBandSource::SetGreenBandSize(uint64_t size)
    {
    m_pGreenBand->SetSize(size);
    }

uint64_t MultiBandSource::GetPanchromaticBandSize() const { return m_pPanchromaticBand->GetSize(); }
void MultiBandSource::SetPanchromaticBandSize(uint64_t size)
    {
    m_pPanchromaticBand->SetSize(size);
    }

MultiBandSourcePtr MultiBandSource::Create()
    {
    return new MultiBandSource();
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSourcePtr MultiBandSource::Create(UriR uri, Utf8CP type)
    {
    if (uri.ToString().empty())
        return NULL;

    return new MultiBandSource(uri, type);
    }

SpatialEntityDataSourceCP MultiBandSource::GetRedBand() const { return m_pRedBand.get(); }
void MultiBandSource::SetRedBand(SpatialEntityDataSourceR band) { m_pRedBand = &band; }

SpatialEntityDataSourceCP MultiBandSource::GetGreenBand() const { return m_pGreenBand.get(); }
void MultiBandSource::SetGreenBand(SpatialEntityDataSourceR band) { m_pGreenBand = &band; }

SpatialEntityDataSourceCP MultiBandSource::GetBlueBand() const { return m_pBlueBand.get(); }
void MultiBandSource::SetBlueBand(SpatialEntityDataSourceR band) { m_pBlueBand = &band; }

SpatialEntityDataSourceCP MultiBandSource::GetPanchromaticBand() const { return m_pPanchromaticBand.get(); }
void MultiBandSource::SetPanchromaticBand(SpatialEntityDataSourceR band) { m_pPanchromaticBand = &band; }

long int SpatialEntityDataSource::GetServerId() const { return m_serverId; }
void SpatialEntityDataSource::SetServerId(long int id) const { m_serverId = id; }

const bvector<UriPtr>& SpatialEntityDataSource::GetSisterFiles() const { return m_sisterFiles; }
void SpatialEntityDataSource::SetSisterFiles(const bvector<UriPtr>& sisterFiles) { m_sisterFiles = sisterFiles; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8CP SpatialEntityDataSource::GetElementName() const
    {
    return _GetElementName();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8CP SpatialEntityDataSource::_GetElementName() const
    {
    BeAssert("Child class must override _GetElementName()" && typeid(*this) == typeid(SpatialEntityDataSource));
    return PACKAGE_ELEMENT_Source;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
Utf8CP MultiBandSource::_GetElementName() const { return PACKAGE_ELEMENT_MultiBandSource; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSource::SpatialEntityDataSource()
    {
    m_pUri = Uri::Create("");
    m_pMetadata = SpatialEntityMetadata::Create();
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
Utf8StringCR SpatialEntity::GetProvider() const 
    {
    if (m_provider.empty())
        {
        for (int i = 0; i < m_DataSources.size(); ++i)
            {
            m_provider = m_DataSources[i]->GetProvider();
            if (m_provider.empty())
                break;
            }
        }

    return m_provider;
    }

void SpatialEntity::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR SpatialEntityDataSource::GetProvider() const { return m_provider; }
void SpatialEntityDataSource::SetProvider(Utf8CP provider) { m_provider = provider; }

Utf8StringCR SpatialEntity::GetProviderName() const
    {
    if (m_providerName.empty())
        {
        for (int i = 0; i < m_DataSources.size(); ++i)
        {
            m_providerName = m_DataSources[i]->GetProviderName();
            if (m_providerName.empty())
                break;
            }
        }

    return m_providerName;
    }

void SpatialEntity::SetProviderName(Utf8CP providerName) { m_providerName = providerName; }

Utf8StringCR SpatialEntityDataSource::GetProviderName() const { return m_providerName; }
void SpatialEntityDataSource::SetProviderName(Utf8CP providerName) { m_providerName = providerName; }

Utf8StringCR SpatialEntity::GetThumbnailURL() const { return m_thumbnailURL; }
void SpatialEntity::SetThumbnailURL(Utf8CP thumbnailURL) { m_thumbnailURL = thumbnailURL; }

Utf8StringCR SpatialEntity::GetThumbnailLoginKey() const { return m_thumbnailLoginKey; }
void SpatialEntity::SetThumbnailLoginKey(Utf8CP thumbnailLoginKey) { m_thumbnailLoginKey = thumbnailLoginKey; }

Utf8StringCR SpatialEntity::GetDataType() const { return m_dataType; }
void SpatialEntity::SetDataType(Utf8CP type) { m_dataType = type; }

DateTimeCR SpatialEntity::GetDate() const { return m_date; }
void SpatialEntity::SetDate(DateTimeCR date) { m_date = date; }

SpatialEntityMetadataCP SpatialEntity::GetMetadataCP() const 
    { 
    if(m_pMetadata == nullptr)
        {
        for(int i = 0; i < m_DataSources.size(); ++i)
            {
            m_pMetadata = m_DataSources[i]->GetMetadataP();
            if(m_pMetadata != nullptr)
                break;
            }
        }

    return m_pMetadata.get();   
    }

SpatialEntityMetadataP SpatialEntity::GetMetadataP() 
    {
    if (m_pMetadata == nullptr)
        {
        for (int i = 0; i < m_DataSources.size(); ++i)
            {
            m_pMetadata = m_DataSources[i]->GetMetadataP();
            if (m_pMetadata != nullptr)
                break;
            }
        }

    return m_pMetadata.get(); 
    }

void SpatialEntity::SetMetadata(SpatialEntityMetadataPtr metadata) { m_pMetadata = metadata; }

SpatialEntityMetadataCP SpatialEntityDataSource::GetMetadataCP() const { return m_pMetadata.get(); }
SpatialEntityMetadataPtr SpatialEntityDataSource::GetMetadataP()  { return m_pMetadata.get(); }
void SpatialEntityDataSource::SetMetadata(SpatialEntityMetadataPtr metadata) { m_pMetadata = metadata; }

Utf8StringCR SpatialEntityMetadata::GetMetadataType() const { return m_metadataType; }
void SpatialEntityMetadata::SetMetadataType(Utf8CP type) { m_metadataType = type; }

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


