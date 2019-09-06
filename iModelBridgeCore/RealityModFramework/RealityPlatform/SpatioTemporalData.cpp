/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/SpatialEntity.h>

#include <RealityPlatform/SpatioTemporalData.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatialEntityDatasetPtr SpatialEntityDataset::CreateFromJson(Utf8CP data)
    {
    SpatialEntityDatasetPtr dataset = new SpatialEntityDataset();

    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return dataset;

    // Parse.
    Json::Value root(Json::objectValue);
    if (!Json::Reader::Parse(data, root))
        return dataset;

    // Instances must be a root node.
    if (!root.isMember("instances"))
        return dataset;

    // Required informations to get.
    Utf8String footprintStr;
    Utf8String classification;

    DateTime date;

    SpatialEntityPtr sePtr;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        sePtr = SpatialEntity::Create();

        if (properties.isMember("Id") && !properties["Id"].isNull())
            sePtr->SetIdentifier(Utf8CP(properties["Id"].asString().c_str()));

        // Name
        if (properties.isMember("Name") && !properties["Name"].isNull())
            sePtr->SetName(Utf8CP(properties["Name"].asString().c_str()));

        // Dataset
        if (properties.isMember("Dataset") && !properties["Dataset"].isNull())
            sePtr->SetDataset(Utf8CP(properties["Dataset"].asString().c_str()));

        // Description
        if (properties.isMember("Description") && !properties["Description"].isNull())
            sePtr->SetDescription(Utf8CP(properties["Description"].asString().c_str()));

        // DataType
        if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
            sePtr->SetDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));
        else if (properties.isMember("Type") && !properties["Type"].isNull())
            sePtr->SetDataType(Utf8CP(properties["Type"].asString().c_str()));

        // Classification
        if (properties.isMember("Classification") && !properties["Classification"].isNull())
            classification = properties["Classification"].asString();
        else 
            classification = "";
        sePtr->SetClassificationByTag(classification.c_str());

        // Thumbnail URL
        if (properties.isMember("ThumbnailURL") && !properties["ThumbnailURL"].isNull())
            sePtr->SetThumbnailURL(Utf8CP(properties["ThumbnailURL"].asString().c_str()));

        // MetadataURL
        if (properties.isMember("MetadataURL") && !properties["MetadataURL"].isNull())
            {
            if (sePtr->GetMetadataCP() == NULL)
                {
                SpatialEntityMetadataPtr metadata = SpatialEntityMetadata::Create();
                sePtr->SetMetadata(metadata.get());
                }

            sePtr->GetMetadataP()->SetMetadataUrl(Utf8CP(properties["MetadataURL"].asString().c_str()));
            }

        if (properties.isMember("AccuracyInMeters") && !properties["AccuracyInMeters"].isNull())
            sePtr->SetAccuracy(Utf8CP(properties["AccuracyInMeters"].asString().c_str()));

        // Provider
        if (properties.isMember("DataProvider") && !properties["DataProvider"].isNull())
            sePtr->SetProvider(Utf8CP(properties["DataProvider"].asString().c_str()));

        // ProviderName
        if (properties.isMember("DataProviderName") && !properties["DataProviderName"].isNull())
            sePtr->SetProviderName(Utf8CP(properties["DataProviderName"].asString().c_str()));

        // Visibility
        if (properties.isMember("Visibility") && !properties["Visibility"].isNull())
            sePtr->SetVisibilityByTag(Utf8CP(properties["Visibility"].asString().c_str()));

        // Date
        if (properties.isMember("Date") && !properties["Date"].isNull())
            {
            DateTime::FromString(date, properties["Date"].asCString());
            sePtr->SetDate(date);
            }
        else if (properties.isMember("CreatedTimestamp") && !properties["CreatedTimestamp"].isNull())
            {
            DateTime::FromString(date, properties["CreatedTimestamp"].asCString());
            sePtr->SetDate(date);
            }

        //// Approximate file size
        if (properties.isMember("FileSize") && !properties["FileSize"].isNull())
            sePtr->SetApproximateFileSize(std::stoi(properties["FileSize"].asString().c_str()));
        else if (properties.isMember("Size") && !properties["Size"].isNull())
            sePtr->SetApproximateFileSize(properties["Size"].asInt());

        // Resolution
        if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
            sePtr->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

        // Footprint
        bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
        if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
            sePtr->SetFootprintString(properties["Footprint"].asString().c_str());
        
        // Add data to corresponding group.
        if (classification.EqualsI("Imagery"))
            dataset->m_imageryGroup.push_back(sePtr);
        else if (classification.EqualsI("Terrain"))
            dataset->m_terrainGroup.push_back(sePtr);
        }

    return dataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatialEntityPtr>&   SpatialEntityDataset::GetImageryGroup() const { return m_imageryGroup; }
bvector<SpatialEntityPtr>&         SpatialEntityDataset::GetImageryGroupR() { return m_imageryGroup; }
const bvector<SpatialEntityPtr>&   SpatialEntityDataset::GetTerrainGroup() const { return m_terrainGroup; }
bvector<SpatialEntityPtr>&         SpatialEntityDataset::GetTerrainGroupR() { return m_terrainGroup; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
SpatialEntityDataset::SpatialEntityDataset()
    {
    // Init.
    //m_imageryGroup = bvector<SpatialEntity>();
    //m_terrainGroup = bvector<SpatialEntity>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatialEntityDataset::~SpatialEntityDataset()
    {}

