/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatformTools/RealityConversionTools.h>

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
* @bsimethod                             Alain.Robert                       05/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToDataLocations(Utf8CP data, bvector<RealityDataLocation>& outData)
    {
    Json::Value root(Json::objectValue);
    if (JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        RealityDataLocation location;

        const Json::Value identifier = instance["instanceId"];
        location.SetIdentifier(identifier.asString().c_str());

        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];
        if (SUCCESS != JsonToDataLocation(properties, location))
            return ERROR;

        outData.push_back(location);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                       05/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToDataLocation(Json::Value properties, RealityDataLocation& locationObject)
    {
    if (properties.isMember("Provider") && !properties["Provider"].isNull())
        locationObject.SetProvider(properties["Provider"].asString().c_str());

    if (properties.isMember("Location") && !properties["Location"].isNull())
        locationObject.SetLocation(properties["Location"].asString().c_str());

    return SUCCESS;    
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                       05/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToDataLocation(Utf8CP data, RealityDataLocation& locationObject)
    {
    Json::Value root(Json::objectValue);
    if(JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];

    const Json::Value identifier = instance["instanceId"];
    locationObject.SetIdentifier(identifier.asString().c_str());

    if (!instance.isMember("properties"))
        return ERROR;

    const Json::Value properties = instance["properties"];

    return RealityConversionTools::JsonToDataLocation(properties, locationObject);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                       04/2019
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToPublicKeys(Utf8CP data, bvector<RealityDataPublicKey>& outData)
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

        RealityDataPublicKey publicKey;
        if (SUCCESS != JsonToPublicKey(properties, publicKey))
            return ERROR;

        outData.push_back(publicKey);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                       04/2019
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToPublicKey(Json::Value properties, RealityDataPublicKey& publicKeyObject)
    {
    DateTime date;

    if (properties.isMember("Id") && !properties["Id"].isNull())
        publicKeyObject.SetIdentifier(properties["Id"].asString().c_str());

    if (properties.isMember("RealityDataId") && !properties["RealityDataId"].isNull())
        publicKeyObject.SetRealityDataId(properties["RealityDataId"].asString().c_str());

    if (properties.isMember("Description") && !properties["Description"].isNull())
        publicKeyObject.SetDescription(properties["Description"].asString().c_str());

    if (properties.isMember("UserId") && !properties["UserId"].isNull())
        publicKeyObject.SetUserId(properties["UserId"].asString().c_str());

    if (properties.isMember("CreatedTimestamp") && !properties["CreatedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["CreatedTimestamp"].asCString());
        publicKeyObject.SetCreationDateTime(date);
        }

    if (properties.isMember("ModifiedTimestamp") && !properties["ModifiedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["ModifiedTimestamp"].asCString());
        publicKeyObject.SetModifiedDateTime(date);
        }

    if (properties.isMember("UltimateReferenceId") && !properties["UltimateReferenceId"].isNull())
        publicKeyObject.SetUltimateId(properties["UltimateReferenceId"].asString().c_str());

    if (properties.isMember("AuthorizedUserIds") && !properties["AuthorizedUserIds"].isNull())
        publicKeyObject.SetAuthorizedUserIds(properties["AuthorizedUserIds"].asString().c_str());

    // Valid Date
    if (properties.isMember("ValidUntilDate") && !properties["ValidUntilDate"].isNull())
        {
        DateTime::FromString(date, properties["ValidUntilDate"].asCString());
        publicKeyObject.SetValidUntilDate(date);
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                       05/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToPublicKey(Utf8CP data, RealityDataPublicKey& publicKeyObject)
    {
    Json::Value root(Json::objectValue);
    if(JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];

    const Json::Value identifier = instance["instanceId"];
    publicKeyObject.SetIdentifier(identifier.asString().c_str());

    if (!instance.isMember("properties"))
        return ERROR;

    const Json::Value properties = instance["properties"];

    return RealityConversionTools::JsonToPublicKey(properties, publicKeyObject);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToEnterpriseStats(Utf8CP data, bvector<RealityDataEnterpriseStat>& outData)
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

        RealityDataEnterpriseStat stat;
        if (SUCCESS != JsonToEnterpriseStat(properties, stat))
            return ERROR;

        outData.push_back(stat);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToEnterpriseStat(Json::Value properties, RealityDataEnterpriseStat& statObject)
    {
    if (properties.isMember("NumberOfRealityData") && !properties["NumberOfRealityData"].isNull())
        statObject.SetNbRealityData(properties["NumberOfRealityData"].asInt64());

    if (properties.isMember("TotalSize") && !properties["TotalSize"].isNull())
        statObject.SetTotalSizeKB(properties["TotalSize"].asDouble());

    if (properties.isMember("OrganizationId") && !properties["OrganizationId"].isNull())
        statObject.SetOrganizationId(properties["OrganizationId"].asString().c_str());

    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        statObject.SetUltimateId(properties["UltimateId"].asString().c_str());

    if (properties.isMember("UltimateSite") && !properties["UltimateSite"].isNull())
        statObject.SetUltimateSite(properties["UltimateSite"].asString().c_str());

    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime dt = DateTime();
        DateTime::FromString(dt, properties["Date"].asString().c_str());
        statObject.SetDate(dt);
        }

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

    return RealityConversionTools::JsonToEnterpriseStat(properties, statObject);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToServiceStats(Utf8CP data, bvector<RealityDataServiceStat>& outData)
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

        RealityDataServiceStat stat;
        if (SUCCESS != JsonToServiceStat(properties, stat))
            return ERROR;

        outData.push_back(stat);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToServiceStat(Json::Value properties, RealityDataServiceStat& statObject)
    {
    if (properties.isMember("NumberOfRealityData") && !properties["NumberOfRealityData"].isNull())
        statObject.SetNbRealityData(properties["NumberOfRealityData"].asInt64());

    if (properties.isMember("TotalSize") && !properties["TotalSize"].isNull())
        statObject.SetTotalSizeKB(properties["TotalSize"].asDouble());

    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        statObject.SetUltimateId(properties["UltimateId"].asString().c_str());

    if (properties.isMember("ServiceId") && !properties["ServiceId"].isNull())
        statObject.SetServiceId(properties["ServiceId"].asString().c_str());
	
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime dt = DateTime();
        DateTime::FromString(dt, properties["Date"].asString().c_str());
        statObject.SetDate(dt);
        }

    return SUCCESS;    
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToServiceStat(Utf8CP data, RealityDataServiceStat& statObject)
    {
    Json::Value root(Json::objectValue);
    if(JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    if (!instance.isMember("properties"))
        return ERROR;

    const Json::Value properties = instance["properties"];

    return RealityConversionTools::JsonToServiceStat(properties, statObject);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToUserStats(Utf8CP data, bvector<RealityDataUserStat>& outData)
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

        RealityDataUserStat stat;
        if (SUCCESS != JsonToUserStat(properties, stat))
            return ERROR;

        outData.push_back(stat);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToUserStat(Json::Value properties, RealityDataUserStat& statObject)
    {
    if (properties.isMember("NumberOfRealityData") && !properties["NumberOfRealityData"].isNull())
        statObject.SetNbRealityData(properties["NumberOfRealityData"].asInt64());

    if (properties.isMember("TotalSize") && !properties["TotalSize"].isNull())
        statObject.SetTotalSizeKB(properties["TotalSize"].asDouble());

    if (properties.isMember("UserId") && !properties["UserId"].isNull())
        statObject.SetUserId(properties["UserId"].asString().c_str());

    if (properties.isMember("UserEmail") && !properties["UserEmail"].isNull())
        statObject.SetUserEmail(properties["UserEmail"].asString().c_str());	
	
    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        statObject.SetUltimateId(properties["UltimateId"].asString().c_str());

    if (properties.isMember("ServiceId") && !properties["ServiceId"].isNull())
        statObject.SetServiceId(properties["ServiceId"].asString().c_str());

    if (properties.isMember("DataLocationGuid") && !properties["DataLocationGuid"].isNull())
        statObject.SetDataLocationGuid(properties["DataLocationGuid"].asString().c_str());
	
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime dt = DateTime();
        DateTime::FromString(dt, properties["Date"].asString().c_str());
        statObject.SetDate(dt);
        }

    return SUCCESS;    
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2018
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToUserStat(Utf8CP data, RealityDataUserStat& statObject)
    {
    Json::Value root(Json::objectValue);
    if(JsonToObjectBase(data, root) == ERROR)
        return ERROR;

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    if (!instance.isMember("properties"))
        return ERROR;

    const Json::Value properties = instance["properties"];

    return RealityConversionTools::JsonToUserStat(properties, statObject);
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

        RealityDataPtr realityData = RealityData::Create();
        JsonToRealityData(realityData, properties);

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
        
        RealityDataPtr realityData = RealityData::Create();
        JsonToRealityData(realityData, properties);

        outData->Insert(realityData->GetName(), realityData);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            12/2017
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToRealityDataExtended(Utf8CP data, bvector<RealityDataExtendedPtr>* outData)
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

        RealityDataExtendedPtr realityData = RealityDataExtended::Create();
        JsonToRealityData(realityData, properties);

        if (properties.isMember("OriginService") && !properties["OriginService"].isNull())
            realityData->SetOriginService(Utf8CP(properties["OriginService"].asString().c_str()));
        if (properties.isMember("UsePermissionOverride") && !properties["UsePermissionOverride"].isNull())
            realityData->SetUsePermissionOverride(Utf8CP(properties["UsePermissionOverride"].asString().c_str()));
        if (properties.isMember("ManagePermissionOverride") && !properties["ManagePermissionOverride"].isNull())
            realityData->SetManagePermissionOverride(Utf8CP(properties["ManagePermissionOverride"].asString().c_str()));
        if (properties.isMember("AssignPermissionOverride") && !properties["AssignPermissionOverride"].isNull())
            realityData->SetAssignPermissionOverride(Utf8CP(properties["AssignPermissionOverride"].asString().c_str()));

        outData->push_back(realityData);
        }
    return SUCCESS;
    }


/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
void RealityConversionTools::JsonToRealityData(RealityDataPtr realityData, Json::Value properties)
    {
    // Required information to get.
    DateTime date;

    Utf8String footprintStr;

    // Id
    if (properties.isMember("Id") && !properties["Id"].isNull())
        realityData->SetIdentifier(Utf8CP(properties["Id"].asString().c_str()));

    // Organization
    if (properties.isMember("OrganizationId") && !properties["OrganizationId"].isNull())
        realityData->SetOrganizationId(Utf8CP(properties["OrganizationId"].asString().c_str()));

    // Container Name
    if (properties.isMember("ContainerName") && !properties["ContainerName"].isNull())
        realityData->SetContainerName(Utf8CP(properties["ContainerName"].asString().c_str()));
    
    // Data Location
    if (properties.isMember("DataLocationGuid") && !properties["DataLocationGuid"].isNull())
        realityData->SetDataLocationGuid(Utf8CP(properties["DataLocationGuid"].asString().c_str()));

    // Name
    if (properties.isMember("Name") && !properties["Name"].isNull())
        realityData->SetName(Utf8CP(properties["Name"].asString().c_str()));

    // Dataset
    if (properties.isMember("Dataset") && !properties["Dataset"].isNull())
        realityData->SetDataset(Utf8CP(properties["Dataset"].asString().c_str()));

    // Group
    if (properties.isMember("Group") && !properties["Group"].isNull())
        realityData->SetGroup(Utf8CP(properties["Group"].asString().c_str()));

    // Description
    if (properties.isMember("Description") && !properties["Description"].isNull())
        realityData->SetDescription(Utf8CP(properties["Description"].asString().c_str()));

    // RootDocument
    if (properties.isMember("RootDocument") && !properties["RootDocument"].isNull())
        realityData->SetRootDocument(Utf8CP(properties["RootDocument"].asString().c_str()));

    // DataType
    if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
        realityData->SetRealityDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));
    else if (properties.isMember("Type") && !properties["Type"].isNull())
        realityData->SetRealityDataType(Utf8CP(properties["Type"].asString().c_str()));

    // Classification
    if (properties.isMember("Classification") && !properties["Classification"].isNull())
        realityData->SetClassificationByTag(Utf8CP(properties["Classification"].asString().c_str()));

    // Streamed
    if (properties.isMember("Streamed") && !properties["Streamed"].isNull())
        realityData->SetStreamed(properties["Streamed"].asBool());

    // Thumbnail Document
    if (properties.isMember("ThumbnailDocument") && !properties["ThumbnailDocument"].isNull())
        realityData->SetThumbnailDocument(Utf8CP(properties["ThumbnailDocument"].asString().c_str()));

    // MetadataUrl
    if (properties.isMember("MetadataUrl") && !properties["MetadataUrl"].isNull())
        realityData->SetMetadataUrl(Utf8CP(properties["MetadataUrl"].asString().c_str()));

    // UltimateId
    if (properties.isMember("UltimateId") && !properties["UltimateId"].isNull())
        realityData->SetUltimateId(Utf8CP(properties["UltimateId"].asString().c_str()));

    // UltimateSite
    if (properties.isMember("UltimateSite") && !properties["UltimateSite"].isNull())
        realityData->SetUltimateSite(Utf8CP(properties["UltimateSite"].asString().c_str()));

    // Copyright
    if (properties.isMember("Copyright") && !properties["Copyright"].isNull())
        realityData->SetCopyright(Utf8CP(properties["Copyright"].asString().c_str()));

    // TermsOfUse
    if (properties.isMember("TermsOfUse") && !properties["TermsOfUse"].isNull())
        realityData->SetTermsOfUse(Utf8CP(properties["TermsOfUse"].asString().c_str()));
 
    // Accuracy
    if (properties.isMember("AccuracyInMeters") && !properties["AccuracyInMeters"].isNull())
        realityData->SetAccuracy(Utf8CP(properties["AccuracyInMeters"].asString().c_str()));

    // Visibility
    if (properties.isMember("Visibility") && !properties["Visibility"].isNull())
        realityData->SetVisibilityByTag(Utf8CP(properties["Visibility"].asString().c_str()));

    // Listable
    if (properties.isMember("Listable") && !properties["Listable"].isNull())
        realityData->SetListable(properties["Listable"].asBool());

    // Date
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime::FromString(date, properties["Date"].asCString());
        realityData->SetCreationDateTime(date);
        }
    else if (properties.isMember("CreatedTimestamp") && !properties["CreatedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["CreatedTimestamp"].asCString());
        realityData->SetCreationDateTime(date);
        }

    // Modified Date
    if (properties.isMember("ModifiedTimestamp") && !properties["ModifiedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["ModifiedTimestamp"].asCString());
        realityData->SetModifiedDateTime(date);
        }
    
    //// Approximate file size
    if(properties.isMember("FileSize") && !properties["FileSize"].isNull())
        realityData->SetTotalSize(std::stoi(properties["FileSize"].asString().c_str()));
    else if (properties.isMember("Size") && !properties["Size"].isNull())
        realityData->SetTotalSize(properties["Size"].asInt());

    if (properties.isMember("SizeUpToDate") && !properties["SizeUpToDate"].isNull())
        realityData->SetSizeUpToDate(properties["SizeUpToDate"].asBool());

    // Resolution
    if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
        realityData->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

    // Footprint
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    Utf8String dummy = "";
    if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
        realityData->SetFootprint(RealityDataBase::RDSJSONToFootprint(properties["Footprint"], dummy));

    if (properties.isMember("OwnedBy") && !properties["OwnedBy"].isNull())
        realityData->SetOwner(Utf8CP(properties["OwnedBy"].asString().c_str()));

    if (properties.isMember("CreatorId") && !properties["CreatorId"].isNull())
        realityData->SetCreatorId(Utf8CP(properties["CreatorId"].asString().c_str()));

    if (properties.isMember("Hidden") && !properties["Hidden"].isNull())
        realityData->SetHidden(properties["Hidden"].asBool());

    if (properties.isMember("DelegatePermissions") && !properties["DelegatePermissions"].isNull())
        realityData->SetDelegatePermissions(properties["DelegatePermissions"].asBool());

    if (properties.isMember("LastAccessedTimestamp") && !properties["LastAccessedTimestamp"].isNull())
        {
        DateTime::FromString(date, properties["LastAccessedTimestamp"].asCString());
        realityData->SetLastAccessedDateTime(date);
        }

    if (properties.isMember("ApproximateFootprint") && !properties["ApproximateFootprint"].isNull())
        realityData->SetApproximateFootprint(properties["ApproximateFootprint"].asBool());
    }



/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            03/2017
+-----------------+------------------+-------------------+-----------------+------------*/
Utf8String RealityConversionTools::RealityDataToJson(RealityDataCR realityData, bool includeUnsetProps, bool includeROProps, bool includeIds)
    {
    bvector<RealityDataField> properties;

    if (includeUnsetProps || (realityData.GetIdentifier().size() != 0))
        properties.push_back(RealityDataField::Id);

    if (includeROProps && (includeUnsetProps || (realityData.GetOrganizationId().size() != 0)))
        properties.push_back(RealityDataField::OrganizationId);

    if (includeROProps && (includeUnsetProps || (realityData.GetContainerName().size() != 0)))
        properties.push_back(RealityDataField::ContainerName);

    if (includeROProps && (includeUnsetProps || (realityData.GetDataLocationGuid().size() != 0)))
        properties.push_back(RealityDataField::DataLocationGuid);

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

    if (includeROProps && (includeUnsetProps || !realityData.IsSizeUpToDate())) // Default (true) is considered unset
        properties.push_back(RealityDataField::SizeUpToDate);

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

    if (includeIds && (includeUnsetProps || (realityData.GetUltimateId().size() != 0)))
        properties.push_back(RealityDataField::UltimateId);

    if (includeIds && (includeUnsetProps || (realityData.GetUltimateSite().size() != 0)))
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

    if (includeROProps && (includeUnsetProps || realityData.GetLastAccessedDateTime().IsValid()))
        properties.push_back(RealityDataField::LastAccessedTimestamp);

    if (includeROProps && (includeUnsetProps || (realityData.GetOwner().size() != 0)))
        properties.push_back(RealityDataField::OwnedBy);

    if (includeROProps && (includeUnsetProps || (realityData.GetCreatorId().size() != 0)))
        properties.push_back(RealityDataField::CreatorId);

    if (includeUnsetProps || (realityData.GetGroup().size() != 0))
        properties.push_back(RealityDataField::Group);

    if (includeUnsetProps || realityData.IsHidden()) // Default (false) is considered unset
        properties.push_back(RealityDataField::Hidden); 
    
    if (includeUnsetProps || realityData.HasDelegatePermissions()) // Default (false) is considered unset
        properties.push_back(RealityDataField::DelegatePermissions);

    if (includeUnsetProps || realityData.HasApproximateFootprint()) // Default (false) is considered unset
        properties.push_back(RealityDataField::ApproximateFootprint);

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
        case RealityDataField::DataLocationGuid:
            propertyString.append("\"DataLocationGuid\" : \"");
            propertyString.append(realityData.GetDataLocationGuid());
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
        case RealityDataField::SizeUpToDate:
            propertyString.append("\"SizeUpToDate\" : ");
            propertyString.append(realityData.IsSizeUpToDate() ? "true" : "false");
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
        case RealityDataField::LastAccessedTimestamp:
            propertyString.append("\"LastAccessedTimestamp\" : \"");
            propertyString.append(realityData.GetLastAccessedDateTime().ToString());
            propertyString.append("\"");
            break;
        case RealityDataField::OwnedBy:
            propertyString.append("\"OwnedBy\" : \"");
            propertyString.append(realityData.GetOwner());
            propertyString.append("\"");
            break;
        case RealityDataField::CreatorId:
            propertyString.append("\"CreatorId\" : \"");
            propertyString.append(realityData.GetCreatorId());
            propertyString.append("\"");
            break;
        case RealityDataField::Group:
            propertyString.append("\"Group\" : \"");
            propertyString.append(realityData.GetGroup());
            propertyString.append("\"");
            break;
        case RealityDataField::Hidden:
            propertyString.append("\"Hidden\" : ");
            propertyString.append(realityData.IsHidden() ? "true" : "false");
            break;
        case RealityDataField::DelegatePermissions:
            propertyString.append("\"DelegatePermissions\" : ");
            propertyString.append(realityData.HasDelegatePermissions() ? "true" : "false");
            break;
        case RealityDataField::ApproximateFootprint:
            propertyString.append("\"ApproximateFootprint\" : ");
            propertyString.append(realityData.HasApproximateFootprint() ? "true" : "false");
            break;
            }
        propertyVector.push_back(propertyString);
        }

    // For when no properties designated.
    if (0 == propertyString.size())
        return "";

    propertyString = propertyVector[0];
    for(size_t i = 1; i < propertyVector.size(); ++i)
        {
        propertyString.append(",");
        propertyString.append(propertyVector[i]);
        }
    return propertyString;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Alain.Robert                            04/2019
+-----------------+------------------+-------------------+-----------------+------------*/
Utf8String RealityConversionTools::RealityDataPublicKeyToJson(RealityDataPublicKeyCR realityDataPublicKey, bool includeUnsetProps, bool includeROProps)
    {

    Utf8String propertyString;

    propertyString = "";
    bool fieldAdded = false;

    if (includeUnsetProps || (realityDataPublicKey.GetIdentifier().size() != 0))
        {
        propertyString.append("\"Id\" : \"");
        propertyString.append(realityDataPublicKey.GetIdentifier());
        propertyString.append("\"");
        fieldAdded = true;
        }



    if (includeUnsetProps || (realityDataPublicKey.GetRealityDataId().size() != 0))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }
        propertyString.append("\"RealityDataId\" : \"");
        propertyString.append(realityDataPublicKey.GetRealityDataId());
        propertyString.append("\"");
        fieldAdded = true;
        }


    if (includeUnsetProps || (realityDataPublicKey.GetDescription().size() != 0))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }

        propertyString.append("\"Description\" : \"");
        propertyString.append(realityDataPublicKey.GetDescription());
        propertyString.append("\"");
        fieldAdded = true;
        }




    if (includeUnsetProps || (realityDataPublicKey.GetUserId().size() != 0))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }

        propertyString.append("\"UserId\" : \"");
        propertyString.append(realityDataPublicKey.GetUserId());
        propertyString.append("\"");
        fieldAdded = true;
        }




    if (includeROProps && (includeUnsetProps || realityDataPublicKey.GetModifiedDateTime().IsValid()))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }

        propertyString.append("\"ModifiedTimestamp\" : \"");
        propertyString.append(realityDataPublicKey.GetModifiedDateTime().ToString());
        propertyString.append("\"");
        fieldAdded = true;
        }


    if (includeROProps && (includeUnsetProps || realityDataPublicKey.GetCreationDateTime().IsValid()))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }
        propertyString.append("\"CreatedTimestamp\" : \"");
        propertyString.append(realityDataPublicKey.GetCreationDateTime().ToString());
        propertyString.append("\"");
        fieldAdded = true;
        }


    if (includeROProps && (includeUnsetProps || (realityDataPublicKey.GetUltimateId().size() != 0)))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }
        propertyString.append("\"UltimateReferenceId\" : \"");
        propertyString.append(realityDataPublicKey.GetUltimateId());
        propertyString.append("\"");
        fieldAdded = true;
        }



    if (includeUnsetProps || (realityDataPublicKey.GetAuthorizedUserIds().size() != 0))
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }
        propertyString.append("\"AuthorizedUserIds\" : \"");
        propertyString.append(realityDataPublicKey.GetAuthorizedUserIds());
        propertyString.append("\"");
        fieldAdded = true;
        }

    if (includeUnsetProps || realityDataPublicKey.GetValidUntilDate().IsValid())
        {
        if (fieldAdded)
            {
            propertyString.append(",");
            fieldAdded = false;
            }
        propertyString.append("\"ValidUntilDate\" : \"");
        propertyString.append(realityDataPublicKey.GetValidUntilDate().ToString());
        propertyString.append("\"");
        }

    return propertyString;
    }

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
    if (properties.isMember("ThumbnailURL") && !properties["ThumbnailURL"].isNull())
        data->SetThumbnailURL(Utf8CP(properties["ThumbnailURL"].asString().c_str()));

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
    if(properties.isMember("DataProvider") && !properties["DataProvider"].isNull())
        data->SetProvider(Utf8CP(properties["DataProvider"].asString().c_str()));

    // ProviderName
    if(properties.isMember("DataProviderName") && !properties["DataProviderName"].isNull())
        data->SetProviderName(Utf8CP(properties["DataProviderName"].asString().c_str()));

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
        {
        UriPtr uri = Uri::Create(Utf8CP(properties["MainURL"].asString().c_str()));
        data->SetUri(*uri);
        }

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
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageStringToDownloadOrder(Utf8CP pSource, WStringP pParseError, BeFileNameCR destinationFolder, bool skipStreams)
    {
    RealityPlatform::RealityPackageStatus status = RealityPlatform::RealityPackageStatus::UnknownError;

    RealityPlatform::RealityDataPackagePtr package = RealityPlatform::RealityDataPackage::CreateFromString(status, pSource, pParseError);
    BeAssert(status == RealityPlatform::RealityPackageStatus::Success);
    
    return PackageToDownloadOrder(package, destinationFolder, skipStreams);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageFileToDownloadOrder(BeFileNameCR filename, WStringP pParseError, BeFileNameCR destinationFolder, bool skipStreams)
    {
    RealityPlatform::RealityPackageStatus status = RealityPlatform::RealityPackageStatus::UnknownError;

    RealityPlatform::RealityDataPackagePtr package = RealityPlatform::RealityDataPackage::CreateFromFile(status, filename, pParseError);
    BeAssert(status == RealityPlatform::RealityPackageStatus::Success);

    return PackageToDownloadOrder(package, destinationFolder, skipStreams);
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                           11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::sisterFileVector RealityConversionTools::SpatialEntityDataSourceToSisterVector(RealityPlatform::SpatialEntityDataSourceCR dataSource, BeFileNameCR destinationFolder)
    {
    bvector<RealityPlatform::UriPtr> sisters;
    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();
    RealityPlatform::UriCR mainFile = dataSource.GetUri();
    WString filename;

    RealityDataDownload::ExtractFileName(filename, mainFile.GetSource());
    if (!destinationFolder.empty())
        {
        BeFileName fullpath = BeFileName(destinationFolder.c_str());
        fullpath.AppendToPath(filename.c_str());
        sfVector.push_back(RealityDataDownload::url_file_pair(mainFile.ToString(), fullpath.c_str()));
        }
    else
        sfVector.push_back(RealityDataDownload::url_file_pair(mainFile.ToString(), filename));

    sisters = dataSource.GetSisterFiles();
    for (RealityPlatform::UriPtr sister : sisters)
        {
        if (sister->GetSource().length() > 0)
            {
			filename.clear();
            RealityDataDownload::ExtractFileName(filename, sister->GetSource());
            if(!destinationFolder.empty())
                {
                BeFileName fullpath = BeFileName(destinationFolder.c_str());
                fullpath.AppendToPath(filename.c_str());
                sfVector.push_back(RealityDataDownload::url_file_pair(sister->ToString(), fullpath.c_str()));
                }
            else
                sfVector.push_back(RealityDataDownload::url_file_pair(sister->ToString(), filename));
            }
        }

    return sfVector;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageToDownloadOrder(RealityPlatform::RealityDataPackagePtr package, BeFileNameCR destinationFolder, bool skipStreams)
    {
    RealityDataDownload::Link_File_wMirrors_wSisters downloadOrder = RealityDataDownload::Link_File_wMirrors_wSisters();
    
    bvector<PackageRealityDataPtr> imageFiles = package->GetImageryGroup();

    for (PackageRealityDataPtr file : imageFiles)
        {
        RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

        size_t mirrorCount = file->GetDataSourceCount();
        
        RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

        for (size_t i = 0; i < mirrorCount; ++i)
            {
            RealityPlatform::SpatialEntityDataSourceR mirror = file->GetDataSource(i);

            RealityPlatform::MultiBandSource* mbSource = dynamic_cast<RealityPlatform::MultiBandSource*>(&mirror);
            
            if(mbSource != nullptr)
                {
                RealityDataDownload::sisterFileVector subVector = SpatialEntityDataSourceToSisterVector(*mbSource->GetRedBand(), destinationFolder);
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = SpatialEntityDataSourceToSisterVector(*mbSource->GetBlueBand(), destinationFolder);
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = SpatialEntityDataSourceToSisterVector(*mbSource->GetGreenBand(), destinationFolder);
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = SpatialEntityDataSourceToSisterVector(*mbSource->GetPanchromaticBand(), destinationFolder);
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                }
            /*else if (wmsSource != nullptr)
                {}
            else if (osmSource != nullptr)
                {}*/
            else
                {
                sfVector = SpatialEntityDataSourceToSisterVector(mirror, destinationFolder);
                }
            mVector.push_back(sfVector);
            }

        downloadOrder.push_back(mVector);
        }

    bvector<PackageRealityDataPtr> terrainFiles = package->GetTerrainGroup();

    for (PackageRealityDataPtr file : terrainFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(*file, destinationFolder));

    bvector<PackageRealityDataPtr> modelFiles = package->GetModelGroup();

    RealityDataDownload::mirrorWSistersVector mVector;
    for (PackageRealityDataPtr file : modelFiles)
        {
        mVector = RealityDataToMirrorVector(*file, destinationFolder, skipStreams);
        if(!mVector.empty())
            downloadOrder.push_back(mVector);
        }

    bvector<PackageRealityDataPtr> pinnedFiles = package->GetPinnedGroup();

    for (PackageRealityDataPtr file : pinnedFiles)
        {
        mVector = RealityDataToMirrorVector(*file, destinationFolder, skipStreams);
        if (!mVector.empty())
            downloadOrder.push_back(mVector);
        }

    bvector<PackageRealityDataPtr> undefinedfiles = package->GetUndefinedGroup();

	for (PackageRealityDataPtr file : undefinedfiles)
        {
        mVector = RealityDataToMirrorVector(*file, destinationFolder, skipStreams);
        if (!mVector.empty())
		    downloadOrder.push_back(mVector);
        }

    return downloadOrder;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                           11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::mirrorWSistersVector RealityConversionTools::RealityDataToMirrorVector(const RealityPlatform::PackageRealityData& realityData, BeFileNameCR destinationFolder, bool skipStreams)
    {
    RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

    size_t mirrorCount = realityData.GetDataSourceCount();

    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

    for (size_t i = 0; i < mirrorCount; ++i)
        {
        if(!skipStreams || !realityData.GetDataSource(i).GetServerCP()->IsStreamed())
            mVector.push_back(SpatialEntityDataSourceToSisterVector(realityData.GetDataSource(i), destinationFolder));
        }
    return mVector;
    }
