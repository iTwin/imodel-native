/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalData.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    Utf8String identifier;
    Utf8String name;
    Utf8String resolutionStr;
    Utf8String footprintStr;
    Utf8String classification;

    DateTime date;
    double resolution;
    bvector<GeoPoint2d> footprint;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        // Id
        (properties.isMember("Id") && !properties["Id"].isNull()) ? identifier = properties["Id"].asString() : identifier = "";
            
        // Name
        (properties.isMember("Name") && !properties["Name"].isNull()) ? name = properties["Name"].asString() : name = "";

        // Date
        date = DateTime();
        if (properties.isMember("Date") && !properties["Date"].isNull())
            DateTime::FromString(date, properties["Date"].asCString());            

        // Resolution
        resolution = DBL_MAX/1000.0;
        if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
            {
            resolutionStr = properties["ResolutionInMeters"].asString();
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(resolutionStr.c_str(), "x", tokens);
            BeAssert(2 == tokens.size());
            if (2 == tokens.size()) 
                {
                // Convert to double.
                double resX = strtod(tokens[0].c_str(), NULL);
                double resY = strtod(tokens[1].c_str(), NULL);

                resolution = sqrt(resX * resY);
                }
            }

        // Footprint
        footprint = bvector<GeoPoint2d>();
        if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
            {
            // Convert Utf8String to GeoPoint2d vector. 
            // The string should look like this:
            // "{ \"points\" : [[-122.0,35.9],[-122.0,37.0],[-120.9,37.0],[-120.9,35.9],[-122.0,35.9]], \"coordinate_system\" : \"4326\" }"
            footprintStr = properties["Footprint"].asString();
            
            // Extract points.
            footprintStr = footprintStr.substr(footprintStr.find_first_of("["), footprintStr.find_last_of("]") - footprintStr.find_first_of("["));
            size_t delimiterPos = 0;
            while (Utf8String::npos != (delimiterPos = footprintStr.find("[")))
                footprintStr.erase(delimiterPos, 1);
            while (Utf8String::npos != (delimiterPos = footprintStr.find("]")))
                footprintStr.erase(delimiterPos, 1);
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(footprintStr.c_str(), ",", tokens);
            BeAssert(10 == tokens.size()); // 5 points making the box (first point == last point).  

            // Convert to double.
            for (size_t i = 0; i < tokens.size(); i += 2)
                {
                GeoPoint2d pt;
                pt.longitude = strtod(tokens[i].c_str(), NULL);
                pt.latitude = strtod(tokens[i + 1].c_str(), NULL);

                footprint.push_back(pt);
                }
            }

        // Classification
        (properties.isMember("Classification") && !properties["Classification"].isNull()) ? classification = properties["Classification"].asString() : classification = "";


        // Add data to corresponding group.
        if (classification.EqualsI("Imagery"))
            dataset->m_imageryGroup.push_back(SpatialEntity::Create(identifier, date, resolutionStr, footprint, name)); //, properties));
        else if (classification.EqualsI("Terrain"))
            dataset->m_terrainGroup.push_back(SpatialEntity::Create(identifier, date, resolutionStr, footprint, name)); //, properties));
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

