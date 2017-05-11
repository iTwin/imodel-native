/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityConversionTools.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


StatusInt RealityConversionTools::JsonToObjectBase(Utf8CP data, Json::Value& json)
    {
    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return ERROR;

    // Parse.
    if (!Json::Reader::Parse(data, json))
        return ERROR;

    // Instances must be a root node.
    if (!json.isMember("instances"))
        return ERROR;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Donald.Morissette                       3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToEnterpriseStat(Utf8CP data, RealityDataEnterpriseStat& statObject)
    {
    Json::Value root(Json::objectValue);
    if(JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    if (!instance.isMember("properties"))
        return ERROR;

    const Json::Value properties = instance["properties"];

    if (properties.isMember("NumberOfRealityData") && !properties["NumberOfRealityData"].isNull())
        statObject.SetNbRealityData(properties["NumberOfRealityData"].asInt64());

    if (properties.isMember("TotalSize") && !properties["TotalSize"].isNull())
        statObject.SetTotalSizeKB(properties["TotalSize"].asInt64());

    if (properties.isMember("OrganizationId") && !properties["OrganizationId"].isNull())
        statObject.SetOrganizationId(properties["OrganizationId"].asString().c_str());

    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        statObject.SetUltimateId(properties["UltimateId"].asString().c_str());

    if (properties.isMember("UltimateSite") && !properties["UltimateSite"].isNull())
        statObject.SetUltimateSite(properties["UltimateSite"].asString().c_str());

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntity(Utf8CP data, bvector<SpatialEntityPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityPtr entity = JsonToSpatialEntity(properties);

        outData->push_back(entity);
        }
    return SUCCESS;
    }

StatusInt RealityConversionTools::JsonToSpatialEntity(Utf8CP data, bmap<Utf8String, SpatialEntityPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityPtr entity = JsonToSpatialEntity(properties);

        outData->Insert(entity->GetName(), entity);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntityDataSource(Utf8CP data, bvector<SpatialEntityDataSourcePtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityDataSourcePtr entity = JsonToSpatialEntityDataSource(properties);

        outData->push_back(entity);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntityServer(Utf8CP data, bvector<SpatialEntityServerPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityServerPtr entity = JsonToSpatialEntityServer(properties);

        outData->push_back(entity);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntityMetadata(Utf8CP data, bvector<SpatialEntityMetadataPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityMetadataPtr entity = JsonToSpatialEntityMetadata(properties);

        outData->push_back(entity);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToRealityData(Utf8CP data, bvector<RealityDataPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        RealityDataPtr realityData = JsonToRealityData(properties);

        outData->push_back(realityData);
        }
    return SUCCESS;
    }

StatusInt RealityConversionTools::JsonToRealityData(Utf8CP data, bmap<Utf8String, RealityDataPtr>* outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        RealityDataPtr realityData = JsonToRealityData(properties);

        outData->Insert(realityData->GetName(), realityData);
        }
    return SUCCESS;
    }


/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataPtr RealityConversionTools::JsonToRealityData(Json::Value properties)
    {
    // Required information to get.
    DateTime date;

    Utf8String footprintStr;

    RealityDataPtr data = RealityData::Create();

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        data->SetIdentifier(Utf8CP(properties["Id"].asString().c_str()));

    // Organization
    if (properties.isMember("OrganizationId") && !properties["OrganizationId"].isNull())
        data->SetOrganizationId(Utf8CP(properties["OrganizationId"].asString().c_str()));

    // Container Name
    if (properties.isMember("ContainerName") && !properties["ContainerName"].isNull())
        data->SetContainerName(Utf8CP(properties["ContainerName"].asString().c_str()));
    
    // Name
    if (properties.isMember("Name") && !properties["Name"].isNull())
        data->SetName(Utf8CP(properties["Name"].asString().c_str()));

    // Dataset
    if (properties.isMember("Dataset") && !properties["Dataset"].isNull())
        data->SetDataset(Utf8CP(properties["Dataset"].asString().c_str()));

    // Group
    if (properties.isMember("Group") && !properties["Group"].isNull())
        data->SetGroup(Utf8CP(properties["Group"].asString().c_str()));

    // Description
    if (properties.isMember("Description") && !properties["Description"].isNull())
        data->SetDescription(Utf8CP(properties["Description"].asString().c_str()));

    // RootDocument
    if (properties.isMember("RootDocument") && !properties["RootDocument"].isNull())
        data->SetRootDocument(Utf8CP(properties["RootDocument"].asString().c_str()));

    // DataType
    if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
        data->SetRealityDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));
    else if (properties.isMember("Type") && !properties["Type"].isNull())
        data->SetRealityDataType(Utf8CP(properties["Type"].asString().c_str()));

    // Classification
    if (properties.isMember("Classification") && !properties["Classification"].isNull())
        data->SetClassificationByTag(Utf8CP(properties["Classification"].asString().c_str()));

    // Streamed
    if (properties.isMember("Streamed") && !properties["Streamed"].isNull())
        data->SetStreamed(properties["Streamed"].asBool());

    // Thumbnail Document
    if (properties.isMember("ThumbnailDocument") && !properties["ThumbnailDocument"].isNull())
        data->SetThumbnailDocument(Utf8CP(properties["ThumbnailDocument"].asString().c_str()));

    // MetadataUrl
    if (properties.isMember("MetadataUrl") && !properties["MetadataUrl"].isNull())
        data->SetMetadataUrl(Utf8CP(properties["MetadataUrl"].asString().c_str()));

    // UltimateId
    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        data->SetUltimateId(Utf8CP(properties["UltimateId"].asString().c_str()));

    // UltimateSite
    if (properties.isMember("UltimateSite") && !properties["UltimateSite"].isNull())
        data->SetUltimateSite(Utf8CP(properties["UltimateSite"].asString().c_str()));

    // Copyright
    if (properties.isMember("Copyright") && !properties["Copyright"].isNull())
        data->SetCopyright(Utf8CP(properties["Copyright"].asString().c_str()));

    // TermsOfUse
    if (properties.isMember("TermsOfUse") && !properties["TermsOfUse"].isNull())
        data->SetTermsOfUse(Utf8CP(properties["TermsOfUse"].asString().c_str()));
 
    // Accuracy
    if (properties.isMember("AccuracyInMeters") && !properties["AccuracyInMeters"].isNull())
        data->SetAccuracy(Utf8CP(properties["AccuracyInMeters"].asString().c_str()));

    // Visibility
    if (properties.isMember("Visibility") && !properties["Visibility"].isNull())
        data->SetVisibilityByTag(Utf8CP(properties["Visibility"].asString().c_str()));

    // Listable
    if (properties.isMember("Listable") && !properties["Listable"].isNull())
        data->SetListable(properties["Listable"].asBool());

    // Date
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime::FromString(date, properties["Date"].asCString());
        data->SetCreationDateTime(date);
        }
    else if (properties.isMember("CreatedTimestamp") && !properties["CreatedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["CreatedTimestamp"].asCString());
        data->SetCreationDateTime(date);
        }

    // Modified Date
    if (properties.isMember("ModifiedTimestamp") && !properties["ModifiedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["ModifiedTimestamp"].asCString());
        data->SetModifiedDateTime(date);
        }
    
    //// Approximate file size
    if(properties.isMember("FileSize") && !properties["FileSize"].isNull())
        data->SetTotalSize(std::stoi(properties["FileSize"].asString().c_str()));
    else if (properties.isMember("Size") && !properties["Size"].isNull())
        data->SetTotalSize(properties["Size"].asInt());

    // Resolution
    if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
        data->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

    // Footprint
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    Utf8String dummy = "";
    if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
        data->SetFootprint(RealityDataBase::RDSJSONToFootprint(properties["Footprint"], dummy));

    if (properties.isMember("OwnedBy") && !properties["OwnedBy"].isNull())
        data->SetOwner(Utf8CP(properties["OwnedBy"].asString().c_str()));

    return data;
    }


#if (1)
/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            03/2017
+-----------------+------------------+-------------------+-----------------+------------*/
Utf8String RealityConversionTools::RealityDataToJson(RealityDataCR realityData, bool includeUnsetProps, bool includeROProps)
    {
    bvector<RealityDataField> properties;

    if (includeUnsetProps || (realityData.GetIdentifier().size() != 0))
        properties.push_back(RealityDataField::Id);

    if (includeROProps && (includeUnsetProps || (realityData.GetOrganizationId().size() != 0)))
        properties.push_back(RealityDataField::OrganizationId);

    if (includeROProps && (includeUnsetProps || (realityData.GetContainerName().size() != 0)))
        properties.push_back(RealityDataField::ContainerName);

    if (includeUnsetProps || (realityData.GetName().size() != 0))
        properties.push_back(RealityDataField::Name);

    if (includeUnsetProps || (realityData.GetDataset().size() != 0))
        properties.push_back(RealityDataField::Dataset);

    if (includeUnsetProps || (realityData.GetDescription().size() != 0))
        properties.push_back(RealityDataField::Description);

    if (includeUnsetProps || (realityData.GetRootDocument().size() != 0))
        properties.push_back(RealityDataField::RootDocument);

    if (includeROProps && (includeUnsetProps || (realityData.GetTotalSize() > 0)))
        properties.push_back(RealityDataField::Size);

    if (includeUnsetProps || (realityData.GetClassification() != RealityDataBase::Classification::UNDEFINED_CLASSIF))
        properties.push_back(RealityDataField::Classification);

    if (includeUnsetProps || (realityData.GetRealityDataType().size() != 0))
        properties.push_back(RealityDataField::Type);

    if (includeUnsetProps || !realityData.IsStreamed()) // Default (true) is considered unset
        properties.push_back(RealityDataField::Streamed);

    if (includeUnsetProps || (realityData.GetFootprint().size() >= 4))
        properties.push_back(RealityDataField::Footprint);

    if (includeUnsetProps || (realityData.GetThumbnailDocument().size() != 0))
        properties.push_back(RealityDataField::ThumbnailDocument);

    if (includeUnsetProps || (realityData.GetMetadataUrl().size() != 0))
        properties.push_back(RealityDataField::MetadataUrl);

    if (includeUnsetProps || (realityData.GetUltimateId().size() != 0))
        properties.push_back(RealityDataField::UltimateId);

    if (includeUnsetProps || (realityData.GetUltimateSite().size() != 0))
        properties.push_back(RealityDataField::UltimateSite);

    if (includeUnsetProps || (realityData.GetCopyright().size() != 0))
        properties.push_back(RealityDataField::Copyright);

    if (includeUnsetProps || (realityData.GetTermsOfUse().size() != 0))
        properties.push_back(RealityDataField::TermsOfUse);

    if (includeUnsetProps || (realityData.GetResolution().size() != 0))
        properties.push_back(RealityDataField::ResolutionInMeters);

    if (includeUnsetProps || (realityData.GetAccuracy().size() != 0))
        properties.push_back(RealityDataField::AccuracyInMeters);

    if (includeUnsetProps || (realityData.GetVisibility() != RealityDataBase::Visibility::UNDEFINED_VISIBILITY))
        properties.push_back(RealityDataField::Visibility);

    if (includeUnsetProps || !realityData.IsListable()) // Default (true) is considered unset
        properties.push_back(RealityDataField::Listable);

    if (includeROProps && (includeUnsetProps || realityData.GetModifiedDateTime().IsValid()))
        properties.push_back(RealityDataField::ModifiedTimestamp);

    if (includeROProps && (includeUnsetProps || realityData.GetCreationDateTime().IsValid()))
        properties.push_back(RealityDataField::CreatedTimestamp);

    if (includeROProps && (includeUnsetProps || (realityData.GetOwner().size() != 0)))
        properties.push_back(RealityDataField::OwnedBy);

    if (includeUnsetProps || (realityData.GetGroup().size() != 0))
        properties.push_back(RealityDataField::Group);

    return RealityDataToJson(realityData, properties);
    }
/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            03/2017
+-----------------+------------------+-------------------+-----------------+------------*/
Utf8String RealityConversionTools::RealityDataToJson(RealityDataCR realityData, bvector<RealityDataField> properties)
    {
    Utf8String propertyString;
    bvector<Utf8String> propertyVector = bvector<Utf8String>();

    for(auto field : properties) // = properties.begin(); it != properties.end(); it.increment())
        {
        propertyString = "";

        switch (field)
            {
        case RealityDataField::Id:
            propertyString.append("\"Id\" : \"");
            propertyString.append(realityData.GetIdentifier());
            propertyString.append("\"");
            break;
        case RealityDataField::OrganizationId:
            propertyString.append("\"OrganizationId\" : \"");
            propertyString.append(realityData.GetOrganizationId());
            propertyString.append("\"");
            break;
        case RealityDataField::ContainerName:
            propertyString.append("\"ContainerName\" : \"");
            propertyString.append(realityData.GetContainerName());
            propertyString.append("\"");
            break;
        case RealityDataField::Name:
            propertyString.append("\"Name\" : \"");
            propertyString.append(realityData.GetName());
            propertyString.append("\"");
            break;
        case RealityDataField::Dataset:
            propertyString.append("\"Dataset\" : \"");
            propertyString.append(realityData.GetDataset());
            propertyString.append("\"");
            break;
        case RealityDataField::Description:
            propertyString.append("\"Description\" : \"");
            propertyString.append(realityData.GetDescription());
            propertyString.append("\"");
            break;
        case RealityDataField::RootDocument:
            propertyString.append("\"RootDocument\" : \"");
            propertyString.append(realityData.GetRootDocument());
            propertyString.append("\"");
            break;
        case RealityDataField::Size:
            propertyString.append("\"Size\" : \"");
            propertyString += Utf8PrintfString("%lu", realityData.GetTotalSize());
            propertyString.append("\"");
            break;
        case RealityDataField::Classification:
            propertyString.append("\"Classification\" : \"");
            propertyString.append(realityData.GetClassificationTag());
            propertyString.append("\"");
            break;
        case RealityDataField::Type:
            propertyString.append("\"Type\" : \"");
            propertyString.append(realityData.GetRealityDataType());
            propertyString.append("\"");
            break;
        case RealityDataField::Streamed:
            propertyString.append("\"Streamed\" : ");
            propertyString.append(realityData.IsStreamed() ? "true" : "false");
            break;
        case RealityDataField::Footprint:
            propertyString.append("\"Footprint\" : ");
            propertyString.append(realityData.GetFootprintString());
            break;
        case RealityDataField::ThumbnailDocument:
            propertyString.append("\"ThumbnailDocument\" : \"");
            propertyString.append(realityData.GetThumbnailDocument());
            propertyString.append("\"");
            break;
        case RealityDataField::MetadataUrl:
            propertyString.append("\"MetadataUrl\" : \"");
            propertyString.append(realityData.GetMetadataUrl());
            propertyString.append("\"");
            break;
        case RealityDataField::UltimateId:
            propertyString.append("\"UltimateId\" : \"");
            propertyString.append(realityData.GetUltimateId());
            propertyString.append("\"");
            break;
        case RealityDataField::UltimateSite:
            propertyString.append("\"UltimateSite\" : \"");
            propertyString.append(realityData.GetUltimateSite());
            propertyString.append("\"");
            break;
        case RealityDataField::Copyright:
            propertyString.append("\"Copyright\" : \"");
            propertyString.append(realityData.GetCopyright());
            propertyString.append("\"");
            break;
        case RealityDataField::TermsOfUse:
            propertyString.append("\"TermsOfUse\" : \"");
            propertyString.append(realityData.GetTermsOfUse());
            propertyString.append("\"");
            break;
        case RealityDataField::ResolutionInMeters:
            propertyString.append("\"ResolutionInMeters\" : \"");
            propertyString.append(realityData.GetResolution());
            propertyString.append("\"");
            break;
        case RealityDataField::AccuracyInMeters:
            propertyString.append("\"AccuracyInMeters\" : \"");
            propertyString.append(realityData.GetAccuracy());
            propertyString.append("\"");
            break;
        case RealityDataField::Visibility:
            propertyString.append("\"Visibility\" : \"");
            propertyString.append(realityData.GetVisibilityTag());
            propertyString.append("\"");
            break;
        case RealityDataField::Listable:
            propertyString.append("\"Listable\" : ");
            propertyString.append(realityData.IsListable() ? "true" : "false");
            break;
        case RealityDataField::ModifiedTimestamp:
            propertyString.append("\"ModifiedTimestamp\" : \"");
            propertyString.append(realityData.GetModifiedDateTime().ToString());
            propertyString.append("\"");
            break;
        case RealityDataField::CreatedTimestamp:
            propertyString.append("\"CreatedTimestamp\" : \"");
            propertyString.append(realityData.GetCreationDateTime().ToString());
            propertyString.append("\"");
            break;
        case RealityDataField::OwnedBy:
            propertyString.append("\"OwnedBy\" : \"");
            propertyString.append(realityData.GetOwner());
            propertyString.append("\"");
            break;
        case RealityDataField::Group:
            propertyString.append("\"Group\" : \"");
            propertyString.append(realityData.GetGroup());
            propertyString.append("\"");
            break;
            }
        propertyVector.push_back(propertyString);
        }
    propertyString = propertyVector[0];
    for(int i = 1; i < propertyVector.size(); ++i)
        {
        propertyString.append(",");
        propertyString.append(propertyVector[i]);
        }
    return propertyString;
    }

#endif

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
SpatialEntityPtr RealityConversionTools::JsonToSpatialEntity(Json::Value properties)
    {
    // Required information to get.
    DateTime date;

    Utf8String footprintStr;

    SpatialEntityPtr data = SpatialEntity::Create();

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        data->SetIdentifier(Utf8CP(properties["Id"].asString().c_str()));
   
    // Name
    if (properties.isMember("Name") && !properties["Name"].isNull())
        data->SetName(Utf8CP(properties["Name"].asString().c_str()));

    // Dataset
    if (properties.isMember("Dataset") && !properties["Dataset"].isNull())
        data->SetDataset(Utf8CP(properties["Dataset"].asString().c_str()));

    // Description
    if (properties.isMember("Description") && !properties["Description"].isNull())
        data->SetDescription(Utf8CP(properties["Description"].asString().c_str()));

    // DataType
    if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
        data->SetDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));
    else if (properties.isMember("Type") && !properties["Type"].isNull())
        data->SetDataType(Utf8CP(properties["Type"].asString().c_str()));

    // Classification
    if (properties.isMember("Classification") && !properties["Classification"].isNull())
        data->SetClassificationByTag(Utf8CP(properties["Classification"].asString().c_str()));

    // Thumbnail URL
    if (properties.isMember("ThumbnailDocument") && !properties["ThumbnailDocument"].isNull())
        data->SetThumbnailURL(Utf8CP(properties["ThumbnailDocument"].asString().c_str()));

    // MetadataURL
    if (properties.isMember("MetadataURL") && !properties["MetadataURL"].isNull())
        {
        if (data->GetMetadataCP() == NULL)
            {
            SpatialEntityMetadataPtr metadata = SpatialEntityMetadata::Create();
            data->SetMetadata(metadata.get());
            }
    
        data->GetMetadataP()->SetMetadataUrl(Utf8CP(properties["MetadataURL"].asString().c_str()));
        }

    if (properties.isMember("AccuracyInMeters") && !properties["AccuracyInMeters"].isNull())
        data->SetAccuracy(Utf8CP(properties["AccuracyInMeters"].asString().c_str()));

    // Provider
    if(properties.isMember("DataProviderName") && !properties["DataProviderName"].isNull())
        data->SetProvider(Utf8CP(properties["DataProviderName"].asString().c_str()));

    // Visibility
    if (properties.isMember("Visibility") && !properties["Visibility"].isNull())
        data->SetVisibilityByTag(Utf8CP(properties["Visibility"].asString().c_str()));

    // Date
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime::FromString(date, properties["Date"].asCString());
        data->SetDate(date);
        }
    else if (properties.isMember("CreatedTimestamp") && !properties["CreatedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["CreatedTimestamp"].asCString());
        data->SetDate(date);
        }
    
    //// Approximate file size
    if(properties.isMember("FileSize") && !properties["FileSize"].isNull())
        data->SetApproximateFileSize(std::stoi(properties["FileSize"].asString().c_str()));
    else if (properties.isMember("Size") && !properties["Size"].isNull())
        data->SetApproximateFileSize(properties["Size"].asInt());

    // Resolution
    if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
        data->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

    // Footprint
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
        data->SetFootprintString(properties["Footprint"].asString().c_str());

    return data;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
SpatialEntityDataSourcePtr RealityConversionTools::JsonToSpatialEntityDataSource(Json::Value properties)
    {
    SpatialEntityDataSourcePtr data = SpatialEntityDataSource::Create();

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        data->SetId(Utf8CP(properties["Id"].asString().c_str()));

    // Main Url
    if (properties.isMember("MainURL") && !properties["MainURL"].isNull())
        data->SetUrl(Utf8CP(properties["MainURL"].asString().c_str()));

    // Parametrized URL
    /*if (properties.isMember("ParametrizedURL") && !properties["ParametrizedURL"].isNull())
        data->SetDataset(Utf8CP(properties["ParametrizedURL"].asString().c_str()));*/

    // CompoundType
    if (properties.isMember("CompoundType") && !properties["CompoundType"].isNull())
        data->SetCompoundType(Utf8CP(properties["CompoundType"].asString().c_str()));

    // Location In Compound
    if (properties.isMember("LocationInCompound") && !properties["LocationInCompound"].isNull())
        data->SetLocationInCompound(Utf8CP(properties["LocationInCompound"].asString().c_str()));

    // DataSourceType
    if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
        data->SetDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));

    // Sister Files
    /*if (properties.isMember("SisterFiles") && !properties["SisterFiles"].isNull())
        data->SetThumbnailURL(Utf8CP(properties["SisterFiles"].asString().c_str()));*/

    // No Data Value
    if (properties.isMember("NoDataValue") && !properties["NoDataValue"].isNull())
        data->SetNoDataValue(Utf8CP(properties["NoDataValue"].asString().c_str()));

    // File Size
    if (properties.isMember("FileSize") && !properties["FileSize"].isNull())
        data->SetSize(Utf8CP(properties["FileSize"].asString().c_str()));

    // Coordinate System
    if (properties.isMember("CoordinateSystem") && !properties["CoordinateSystem"].isNull())
        data->SetCoordinateSystem(Utf8CP(properties["CoordinateSystem"].asString().c_str()));

    // Streamed
    /*if (properties.isMember("Streamed") && !properties["Streamed"].isNull())
        data->SetVisibilityByTag(Utf8CP(properties["Streamed"].asString().c_str()));

    // Metadata
    if (properties.isMember("Metadata") && !properties["Metadata"].isNull())
        data->SetApproximateFileSize(std::stoi(properties["Metadata"].asString().c_str()));*/

    return data;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
SpatialEntityServerPtr RealityConversionTools::JsonToSpatialEntityServer(Json::Value properties)
    {
    SpatialEntityServerPtr data = SpatialEntityServer::Create();

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        data->SetId(Utf8CP(properties["Id"].asString().c_str()));

    // Communication Protocol
    if (properties.isMember("CommunicationProtocol") && !properties["CommunicationProtocol"].isNull())
        data->SetProtocol(Utf8CP(properties["CommunicationProtocol"].asString().c_str()));

    // Streamed
    if (properties.isMember("Streamed") && !properties["Streamed"].isNull())
        data->SetStreamed(properties["Streamed"].asBool());

    // Login Key
    if (properties.isMember("LoginKey") && !properties["LoginKey"].isNull())
        data->SetLoginKey(Utf8CP(properties["LoginKey"].asString().c_str()));

    // Login Method
    if (properties.isMember("LoginMethod") && !properties["LoginMethod"].isNull())
        data->SetLoginMethod(Utf8CP(properties["LoginMethod"].asString().c_str()));

    // Registration Page
    if (properties.isMember("RegistrationPage") && !properties["RegistrationPage"].isNull())
        data->SetRegistrationPage(Utf8CP(properties["RegistrationPage"].asString().c_str()));

    // Organisation Page
    if (properties.isMember("OrganisationPage") && !properties["OrganisationPage"].isNull())
        data->SetOrganisationPage(Utf8CP(properties["OrganisationPage"].asString().c_str()));

    // Name
    if (properties.isMember("Name") && !properties["Name"].isNull())
        data->SetName(Utf8CP(properties["Name"].asString().c_str()));

    // URL
    if (properties.isMember("URL") && !properties["URL"].isNull())
        data->SetUrl(Utf8CP(properties["URL"].asString().c_str()));

    // Server Contact Information
    if (properties.isMember("ServerContactInformation") && !properties["ServerContactInformation"].isNull())
        data->SetContactInfo(Utf8CP(properties["ServerContactInformation"].asString().c_str()));

    // Fees
    if (properties.isMember("Fees") && !properties["Fees"].isNull())
        data->SetFees(Utf8CP(properties["Fees"].asString().c_str()));

    // Legal
    if (properties.isMember("Legal") && !properties["Legal"].isNull())
        data->SetLegal(Utf8CP(properties["Legal"].asString().c_str()));

    // Access Constraints
    if (properties.isMember("AccessConstraints") && !properties["AccessConstraints"].isNull())
        data->SetAccessConstraints(Utf8CP(properties["AccessConstraints"].asString().c_str()));

    // Online
    if (properties.isMember("Online") && !properties["Online"].isNull())
        data->SetOnline(properties["Online"].asBool());

    // Last Check
    if (properties.isMember("LastCheck") && !properties["LastCheck"].isNull())
        data->SetLastCheck(Utf8CP(properties["LastCheck"].asString().c_str()));

    // Last Time Online
    if (properties.isMember("LastTimeOnline") && !properties["LastTimeOnline"].isNull())
        data->SetLastTimeOnline(Utf8CP(properties["LastTimeOnline"].asString().c_str()));

    // Latency
    if (properties.isMember("Latency") && !properties["Latency"].isNull())
        data->SetLatency(properties["Latency"].asDouble());

    // Mean Reachability Stats
    if (properties.isMember("MeanReachabilityStats") && !properties["MeanReachabilityStats"].isNull())
        data->SetMeanReachabilityStats(properties["MeanReachabilityStats"].asInt());

    // State
    if (properties.isMember("State") && !properties["State"].isNull())
        data->SetState(Utf8CP(properties["State"].asString().c_str()));

    // Type
    if (properties.isMember("Type") && !properties["Type"].isNull())
        data->SetType(Utf8CP(properties["Type"].asString().c_str()));

    return data;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            3/2017
+-----------------+------------------+-------------------+-----------------+------------*/
SpatialEntityMetadataPtr RealityConversionTools::JsonToSpatialEntityMetadata(Json::Value properties)
    {
    SpatialEntityMetadataPtr data = SpatialEntityMetadata::Create();

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        data->SetId(Utf8CP(properties["Id"].asString().c_str()));

    // Communication Protocol
    if (properties.isMember("MetadataURL") && !properties["MetadataURL"].isNull())
        data->SetMetadataUrl(Utf8CP(properties["MetadataURL"].asString().c_str()));

    // Streamed
    if (properties.isMember("DisplayStyle") && !properties["DisplayStyle"].isNull())
        data->SetDisplayStyle(properties["DisplayStyle"].asString().c_str());

    // Login Key
    if (properties.isMember("Description") && !properties["Description"].isNull())
        data->SetDescription(Utf8CP(properties["Description"].asString().c_str()));

    // Login Method
    if (properties.isMember("ContactInformation") && !properties["ContactInformation"].isNull())
        data->SetContactInfo(Utf8CP(properties["ContactInformation"].asString().c_str()));

    // Registration Page
    if (properties.isMember("Keywords") && !properties["Keywords"].isNull())
        data->SetKeywords(Utf8CP(properties["Keywords"].asString().c_str()));

    // Organisation Page
    if (properties.isMember("Legal") && !properties["Legal"].isNull())
        data->SetLegal(Utf8CP(properties["Legal"].asString().c_str()));

    // Name
    if (properties.isMember("TermsOfUse") && !properties["TermsOfUse"].isNull())
        data->SetTermsOfUse(Utf8CP(properties["TermsOfUse"].asString().c_str()));

    // URL
    if (properties.isMember("Lineage") && !properties["Lineage"].isNull())
        data->SetLineage(Utf8CP(properties["Lineage"].asString().c_str()));

    // Server Contact Information
    if (properties.isMember("Provenance") && !properties["Provenance"].isNull())
        data->SetProvenance(Utf8CP(properties["Provenance"].asString().c_str()));

    return data;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageStringToDownloadOrder(Utf8CP pSource, WStringP pParseError)
    {
    RealityPackage::RealityPackageStatus status = RealityPackage::RealityPackageStatus::UnknownError;

    RealityPackage::RealityDataPackagePtr package = RealityPackage::RealityDataPackage::CreateFromString(status, pSource, pParseError);
    BeAssert(status == RealityPackage::RealityPackageStatus::Success);
    
    return PackageToDownloadOrder(package);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageFileToDownloadOrder(BeFileNameCR filename, WStringP pParseError)
    {
    RealityPackage::RealityPackageStatus status = RealityPackage::RealityPackageStatus::UnknownError;

    RealityPackage::RealityDataPackagePtr package = RealityPackage::RealityDataPackage::CreateFromFile(status, filename, pParseError);
    BeAssert(status == RealityPackage::RealityPackageStatus::Success);

    return PackageToDownloadOrder(package);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                           11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::sisterFileVector RealityConversionTools::RealityDataToSisterVector(RealityPackage::RealityDataSourceCR dataSource)
    {
    bvector<RealityPackage::UriPtr> sisters;
    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();
    RealityPackage::UriCR mainFile = dataSource.GetUri();
    WString filename;

    RealityDataDownload::ExtractFileName(filename, mainFile.GetSource());
    sfVector.push_back(std::make_pair(mainFile.ToString(), filename));

    sisters = dataSource.GetSisterFiles();
    for (RealityPackage::UriPtr sister : sisters)
        {
        if (sister->GetSource().length() > 0)
            {
            RealityDataDownload::ExtractFileName(filename, sister->GetSource());
            sfVector.push_back(std::make_pair(sister->ToString(), filename));
            }
        }

    return sfVector;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageToDownloadOrder(RealityPackage::RealityDataPackagePtr package)
    {

    RealityDataDownload::Link_File_wMirrors_wSisters downloadOrder = RealityDataDownload::Link_File_wMirrors_wSisters();
    
    RealityPackage::RealityDataPackage::ImageryGroup imageFiles = package->GetImageryGroup();

    for (RealityPackage::ImageryDataPtr file : imageFiles)
        {
        RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

        size_t mirrorCount = file->GetNumSources();
        
        RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

        for (size_t i = 0; i < mirrorCount; ++i)
            {
            RealityPackage::RealityDataSourceR mirror = file->GetSourceR(i);

            RealityPackage::MultiBandSource* mbSource = dynamic_cast<RealityPackage::MultiBandSource*>(&mirror);
            /*RealityPackage::WmsDataSource* wmsSource = dynamic_cast<RealityPackage::WmsDataSource*>(&mirror);
            RealityPackage::OsmDataSource* osmSource = dynamic_cast<RealityPackage::OsmDataSource*>(&mirror);*/

            if(mbSource != nullptr)
                {
                RealityDataDownload::sisterFileVector subVector = RealityDataToSisterVector(*mbSource->GetRedBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(*mbSource->GetBlueBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(*mbSource->GetGreenBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(*mbSource->GetPanchromaticBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                }
            /*else if (wmsSource != nullptr)
                {}
            else if (osmSource != nullptr)
                {}*/
            else
                {
                sfVector = RealityDataToSisterVector(mirror);
                }
            mVector.push_back(sfVector);
            }

        downloadOrder.push_back(mVector);
        }

    RealityPackage::RealityDataPackage::TerrainGroup terrainFiles = package->GetTerrainGroup();

    for (RealityPackage::TerrainDataPtr file : terrainFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(*file));

    /*RealityPackage::RealityDataPackage::ModelGroup modelFiles = package->GetModelGroup();

    for (RealityPackage::ModelDataPtr file : modelFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(*file));*/

    RealityPackage::RealityDataPackage::PinnedGroup pinnedFiles = package->GetPinnedGroup();

    for (RealityPackage::PinnedDataPtr file : pinnedFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(*file));

    return downloadOrder;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                           11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::mirrorWSistersVector RealityConversionTools::RealityDataToMirrorVector(const RealityPackage::PackageRealityData& realityData)
    {
    RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

    size_t mirrorCount = realityData.GetNumSources();

    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

    for (size_t i = 0; i < mirrorCount; ++i)
        {
        mVector.push_back(RealityDataToSisterVector(realityData.GetSource(i)));
        }
    return mVector;
    }
