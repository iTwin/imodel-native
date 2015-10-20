/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalData.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SpatioTemporalData.h"

#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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
        properties.isMember("Id") ? identifier = properties["Id"].asString() : identifier = "";
            
        // Date
        date = DateTime();
        if (properties.isMember("Date"))
            DateTime::FromString(date, properties["Date"].asCString());            

        // Resolution
        resolution = 0.0;
        if (properties.isMember("ResolutionInMeters"))
            {
            resolutionStr = properties["ResolutionInMeters"].asString();
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(resolutionStr.c_str(), "x", tokens);
            BeAssert(2 == tokens.size());

            // Convert to double.
            double resX = strtod(tokens[0].c_str(), NULL);
            double resY = strtod(tokens[1].c_str(), NULL);

            resolution = sqrt(resX * resY);
            }

        // Footprint
        footprint = bvector<GeoPoint2d>();
        if (properties.isMember("Footprint"))
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
        properties.isMember("Classification") ? classification = properties["Classification"].asString() : classification = "";


        // Add data to corresponding group.
        if (classification.EqualsI("Imagery"))
            dataset->m_imageryGroup.push_back(SpatioTemporalData::Create(identifier.c_str(), date, resolution, footprint));
        else if (classification.EqualsI("Terrain"))
            dataset->m_terrainGroup.push_back(SpatioTemporalData::Create(identifier.c_str(), date, resolution, footprint));
        }

    return dataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr>&   SpatioTemporalDataset::GetImageryGroup() const { return m_imageryGroup; }
const bvector<SpatioTemporalDataPtr>&   SpatioTemporalDataset::GetTerrainGroup() const { return m_terrainGroup; }

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
SpatioTemporalDataPtr SpatioTemporalData::Create(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint)
    {
    return new SpatioTemporalData(identifier, date, resolution, footprint);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
Utf8StringCR    SpatioTemporalData::GetIdentifier() const { return m_identifier; }
const DateTime& SpatioTemporalData::GetDate() const { return m_date; }
const double&   SpatioTemporalData::GetResolution() const { return m_resolution; }
HFCPtr<HGF2DShape>  SpatioTemporalData::GetFootprint() const { return m_footprint; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalData::SpatioTemporalData(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint)
    : m_identifier(identifier),
      m_date(date),
      m_resolution(resolution)    
    {
    // Convert GeoPoint2d vector to HGF2DShape.
    HGF2DPositionCollection ptsCollection;
    for (const auto& point : footprint)
        {
        ptsCollection.push_back(HGF2DCoord<double>(point.longitude, point.latitude));
        }

    m_footprint = new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
SpatioTemporalData::~SpatioTemporalData()
    {}
