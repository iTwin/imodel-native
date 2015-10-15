/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalSelector.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/SpatioTemporalSelector.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDsFromJson(const bvector<GeoPoint2d>& regionOfInterest,
                                                                 Utf8CP data, 
                                                                 SelectionCriteria criteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return selectedIDs;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(data);

    // Select imagery data.
    bvector<Utf8String> imageryIDs = Select(regionOfInterest, dataset->GetImageryGroup(), criteria);
    if (!imageryIDs.empty())
        selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

    // Select terrain data.
    bvector<Utf8String> terrainIDs = Select(regionOfInterest, dataset->GetTerrainGroup(), criteria);
    if (!terrainIDs.empty())
        selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::Select(const bvector<GeoPoint2d>& regionOfInterest,
                                                         const bvector<SpatioTemporalDataPtr>& dataset,
                                                         SelectionCriteria criteria)
    {
    switch (criteria)
        {
        case SelectionCriteria::Date:
            {
            return SelectByDate(regionOfInterest, dataset);
            }
        case SelectionCriteria::Resolution:
            {
            return SelectByResolution(regionOfInterest, dataset);
            }
        case SelectionCriteria::DateAndResolution:
            {
            return SelectByDateAndResolution(regionOfInterest, dataset);
            }
        default:
            {
            return SelectByDateAndResolution(regionOfInterest, dataset);
            }
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByDate(const bvector<GeoPoint2d>& regionOfInterest,
                                                               const bvector<SpatioTemporalDataPtr>& dataset)
    {
    //&&JFC TODO
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
        selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByResolution(const bvector<GeoPoint2d>& regionOfInterest,
                                                                     const bvector<SpatioTemporalDataPtr>& dataset)
    {
    //&&JFC TODO
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
        selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByDateAndResolution(const bvector<GeoPoint2d>& regionOfInterest,
                                                                            const bvector<SpatioTemporalDataPtr>& dataset)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
        selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;

    //&&JFC TODO
    //bvector<Utf8String> selectedIDs = bvector<Utf8String>();
    //
    //// First draft: Included in region of interest.
    //bvector<Utf8String> firstDraftIDs = bvector<Utf8String>();
    //for (const auto& data : dataset)
    //    {
    //    Utf8String identifier = data->GetIdentifier();
    //    identifier;
    //
    //    }
    //
    //
    //
    //
    //
    //
    //
    //
    //return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalDatasetPtr SpatioTemporalDataset::CreateFromJson(Utf8CP data)
    {
    SpatioTemporalDatasetPtr dataset = new SpatioTemporalDataset();

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
    Utf8String resolution;
    Utf8String footprintStr;
    Utf8String classification;

    DateTime date;
    DRange2d footprint;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        // Id
        properties.isMember("Id") ? identifier = properties["Id"].asString() : identifier = "";
            
        // Date
        if (properties.isMember("Date"))
            DateTime::FromString(date, properties["Date"].asCString());
        else
            date = DateTime();

        // Resolution
        properties.isMember("ResolutionInMeters") ? resolution = properties["ResolutionInMeters"].asString() : resolution = "";

        // Footprint
        if (properties.isMember("Footprint"))
            {
            // Convert Utf8String to DRange2d. 
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
            bvector<DPoint2d> footprintPts;
            for (size_t i = 0; i < tokens.size(); i += 2)
                {
                DPoint2d pt;
                pt.x = strtod(tokens[i].c_str(), NULL);
                pt.y = strtod(tokens[i + 1].c_str(), NULL);

                footprintPts.push_back(pt);
                }

            // Create range.
            footprint = DRange2d::From(footprintPts);
            }
        else
            footprint = DRange2d::NullRange();

        // Classification
        properties.isMember("Classification") ? classification = properties["Classification"].asString() : classification = "";


        // Add data to corresponding group.
        if (classification.EqualsI("Imagery"))
            dataset->m_imageryGroup.push_back(SpatioTemporalData::Create(identifier.c_str(), date, resolution.c_str(), footprint));
        else if (classification.EqualsI("Terrain"))
            dataset->m_terrainGroup.push_back(SpatioTemporalData::Create(identifier.c_str(), date, resolution.c_str(), footprint));
        }

    return dataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalDataset::SpatioTemporalDataset()
    {
    // Init.
    //m_imageryGroup = bvector<SpatioTemporalData>();
    //m_terrainGroup = bvector<SpatioTemporalData>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalDataset::~SpatioTemporalDataset()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalDataPtr SpatioTemporalData::Create(Utf8CP identifier, const DateTime& date, Utf8CP resolution, DRange2dCR footprint)
    {
    return new SpatioTemporalData(identifier, date, resolution, footprint);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalData::SpatioTemporalData(Utf8CP identifier, const DateTime& date, Utf8CP resolution, DRange2dCR footprint)
    : m_identifier(identifier),
      m_date(date),
      m_resolution(resolution),
      m_footprint(footprint)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
//SpatioTemporalData::~SpatioTemporalData()
//    {}
