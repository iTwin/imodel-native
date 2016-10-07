/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityConversionTools.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/RealityConversionTools.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*----------------------------------------------------------------------------------**//**
* Fills a bvector with SpatialEntityData objects, created with data extracted from JSON
* Currently parses JSON for Name, DataType, Provider, Date, Size, Resolution and Footprint
* None of these fields are guaranteed, and should be validated by the user 
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
StatusInt RealityConversionTools::JsonToSpatialEntityData(Utf8CP data, bvector<SpatialEntityDataPtr>* outData)
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

    // Required informations to get.
    DateTime date;

    Utf8String footprintStr;

    // Loop through all data and get required informations.
    for (const auto& instance : root["instances"])
        {
        if (!instance.isMember("properties"))
            break;

        const Json::Value properties = instance["properties"];

        SpatialEntityDataPtr data = SpatialEntityData::Create();

        // Name
        if (properties.isMember("Name") && !properties["Name"].isNull())
            data->SetName(Utf8CP(properties["Name"].asString().c_str()));

        // DataType
        if (properties.isMember("DataSourceType") && !properties["DataSourceType"].isNull())
            data->SetDataType(Utf8CP(properties["DataSourceType"].asString().c_str()));

        // Provider
        if(properties.isMember("DataProviderName") && !properties["DataProviderName"].isNull())
            data->SetProvider(Utf8CP(properties["DataProviderName"].asString().c_str()));

        // Date
        if (properties.isMember("Date") && !properties["Date"].isNull())
            {
            DateTime::FromString(date, properties["Date"].asCString());
            data->SetDate(date);
            }

        //Size
        if(properties.isMember("FileSize") && !properties["FileSize"].isNull())
            data->SetSize(std::stoi(properties["FileSize"].asString().c_str()));

        // Resolution
        if (properties.isMember("ResolutionInMeters") && !properties["ResolutionInMeters"].isNull())
            data->SetResolution(Utf8CP(properties["ResolutionInMeters"].asString().c_str()));

        // Footprint
        bvector<DPoint2d> footprint = bvector<DPoint2d>();
        if (properties.isMember("Footprint") && !properties["Footprint"].isNull())
            {
            // Convert Utf8String to DPoint2d vector. 
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
                DPoint2d pt;
                pt.x = strtod(tokens[i].c_str(), NULL);
                pt.y = strtod(tokens[i + 1].c_str(), NULL);

                footprint.push_back(pt);
                }

            data->SetFootprint(footprint);

            }

        outData->push_back(data);

        }
    return SUCCESS;
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
* @bsimethod                             Spencer.Mason                            9/2016
+-----------------+------------------+-------------------+-----------------+------------*/
RealityDataDownload::Link_File_wMirrors_wSisters RealityConversionTools::PackageToDownloadOrder(RealityPackage::RealityDataPackagePtr package)
    {
    bvector<RealityPackage::UriPtr> sisters;

    RealityDataDownload::Link_File_wMirrors_wSisters downloadOrder = RealityDataDownload::Link_File_wMirrors_wSisters();
    
    RealityPackage::RealityDataPackage::ImageryGroup files = package->GetImageryGroup();

    for (RealityPackage::ImageryDataPtr file : files)
        {
        RealityDataDownload::mirrorWSistersVector mVector = RealityDataDownload::mirrorWSistersVector();

        size_t mirrorCount = file->GetNumSources();

        RealityDataDownload::sisterFileVector sfVector = RealityDataDownload::sisterFileVector();

        WString filename;
        for (size_t i = 0; i < mirrorCount; ++i)
            {
            RealityPackage::RealityDataSourceCR mirror = file->GetSource(i);

            RealityPackage::UriCR mainFile = mirror.GetUri();

            RealityDataDownload::ExtractFileName(filename, mainFile.ToString());
            sfVector.push_back(std::make_pair(mainFile.ToString(), filename));

            sisters = mirror.GetSisterFiles();
            for (RealityPackage::UriPtr sister : sisters)
                {
                if (sister->GetSource().length() > 0)
                    {
                    RealityDataDownload::ExtractFileName(filename, sister->ToString());
                    sfVector.push_back(std::make_pair(sister->ToString(), filename));
                    }
                }
            mVector.push_back(sfVector);

            }
        downloadOrder.push_back(mVector);
        }
    return downloadOrder;
    }