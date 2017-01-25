/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityConversionTools.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntity(Utf8CP data, bvector<SpatialEntityPtr>* outData)
    {
    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return ERROR;

    // Parse.
    Json::Value root(Json::objectValue);
    if (!Json::Reader::Parse(data, root))
        return ERROR;

    // Instances must be a root node.
    if (!root.isMember("instances"))
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityPtr data = JsonToSpatialEntity(properties);

        outData->push_back(data);
        }
    return SUCCESS;
    }

StatusInt RealityConversionTools::JsonToSpatialEntity(Utf8CP data, bmap<Utf8String, SpatialEntityPtr>* outData)
    {
    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return ERROR;

    // Parse.
    Json::Value root(Json::objectValue);
    if (!Json::Reader::Parse(data, root))
        return ERROR;

    // Instances must be a root node.
    if (!root.isMember("instances"))
        return ERROR;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityPtr data = JsonToSpatialEntity(properties);

        outData->Insert(data->GetName(), data);
        }
    return SUCCESS;
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

    // Name
    if (properties.isMember("Name") && !properties["Name"].isNull())
        data->SetName(Utf8CP(properties["Name"].asString().c_str()));

    // DataType
    if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
        data->SetDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));

    // Classification
    if (properties.isMember("Classification") && !properties["Classification"].isNull())
        data->SetClassification(Utf8CP(properties["Classification"].asString().c_str()));

    // Provider
    if(properties.isMember("DataProviderName") && !properties["DataProviderName"].isNull())
        data->SetProvider(Utf8CP(properties["DataProviderName"].asString().c_str()));

    // Date
    if (properties.isMember("Date") && !properties["Date"].isNull())
        {
        DateTime::FromString(date, properties["Date"].asCString());
        data->SetDate(date);
        }

    //// Approximate file size
    if(properties.isMember("FileSize") && !properties["FileSize"].isNull())
        data->SetApproximateFileSize(std::stoi(properties["FileSize"].asString().c_str()));

    // Resolution
    if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
        data->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

    // Footprint
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
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
            
        for (size_t i = 0; i < tokens.size(); i += 2)
            {
            GeoPoint2d pt;
            pt.longitude = strtod(tokens[i].c_str(), NULL);
            pt.latitude = strtod(tokens[i + 1].c_str(), NULL);

            footprint.push_back(pt);
            }

        data->SetFootprint(footprint);
        }
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
RealityDataDownload::sisterFileVector RealityConversionTools::RealityDataToSisterVector(RealityPackage::RealityDataSourceCP dataSource)
    {
    bvector<RealityPackage::UriPtr> sisters;
    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();
    RealityPackage::UriCR mainFile = dataSource->GetUri();
    WString filename;

    RealityDataDownload::ExtractFileName(filename, mainFile.GetSource());
    sfVector.push_back(std::make_pair(mainFile.ToString(), filename));

    sisters = dataSource->GetSisterFiles();
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
                RealityDataDownload::sisterFileVector subVector = RealityDataToSisterVector(mbSource->GetRedBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(mbSource->GetBlueBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(mbSource->GetGreenBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                subVector = RealityDataToSisterVector(mbSource->GetPanchromaticBand());
                sfVector.insert(sfVector.end(), subVector.begin(), subVector.end());
                }
            /*else if (wmsSource != nullptr)
                {}
            else if (osmSource != nullptr)
                {}*/
            else
                {
                sfVector = RealityDataToSisterVector(&mirror);
                }
            mVector.push_back(sfVector);
            }

        downloadOrder.push_back(mVector);
        }

    RealityPackage::RealityDataPackage::TerrainGroup terrainFiles = package->GetTerrainGroup();

    for (RealityPackage::TerrainDataPtr file : terrainFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(file));

    /*RealityPackage::RealityDataPackage::ModelGroup modelFiles = package->GetModelGroup();

    for (RealityPackage::ModelDataPtr file : modelFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(file));*/

    RealityPackage::RealityDataPackage::PinnedGroup pinnedFiles = package->GetPinnedGroup();

    for (RealityPackage::PinnedDataPtr file : pinnedFiles)
        downloadOrder.push_back(RealityDataToMirrorVector(file));

    return downloadOrder;
    }

/*----------------------------------------------------------------------------------**//**
* @bsimethod                             Spencer.Mason                           11/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::mirrorWSistersVector RealityConversionTools::RealityDataToMirrorVector(RealityPackage::RealityDataPtr realityData)
    {
    RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

    size_t mirrorCount = realityData->GetNumSources();

    RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

    for (size_t i = 0; i < mirrorCount; ++i)
        {
        mVector.push_back(RealityDataToSisterVector(&(realityData->GetSourceR(i))));
        }
    return mVector;
    }
