/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Windows.h>
#include <Bentley/BeFile.h>

#include <RealityPlatformTools/SimpleGCSApi.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static bool FilterFunc(SpatialEntityPtr entity)
    {
    Json::Value provider = entity->GetProvider();
    return provider.isString() ? provider.asString().EqualsI("Amazon Landsat 8") : false;
    }

bvector<Utf8String> selectionCallbackFunction(bvector<GeoPoint2d> footprint, SpatialEntityDatasetPtr dataset)
    {
    auto imageryIt(dataset->GetImageryGroupR().begin());
    while (imageryIt != dataset->GetImageryGroupR().end())
        {
        double occlusion = (*imageryIt)->GetOcclusion();
        if ((occlusion  > 50.0) || FilterFunc(*imageryIt))
            {
            imageryIt = dataset->GetImageryGroupR().erase(imageryIt);
            }
        else
            {
            imageryIt++;
            }
        }

    RealityPlatform::SpatioTemporalSelector::ResolutionMap selectedIds = SpatioTemporalSelector::GetIDsByRes(*dataset, footprint);

    //the choice to use low resolution is taken in the callback
    //this choice can be bubbled up to the client, in any way you see fit
    bvector<Utf8String> returnVec = selectedIds[RealityPlatform::ResolutionCriteria::Low]; //here I chose low resolution
    while(returnVec.size() > 3) // here (to speed up the example), I limit amount of files to download
        returnVec.pop_back();

    return returnVec; 
    }

int main(int argc, char *argv[])
    {
    //must perform Setup before using the GCSRequestManager. It will setup
    GCSRequestManager::Setup();

    //to get a bingkey and its expiration date
    Utf8String bingKey, expirationDate;
    GCSRequestManager::SimpleBingKeyRequest("1000", bingKey, expirationDate);

    //to get a list of terrain and imagery files, first create a Geopoint vector that
    //represents the polygon of interest. The final point must be the same as the first
    bvector<GeoPoint2d> space = bvector<GeoPoint2d>();
    space.push_back(GeoPoint2d::From(115.73, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 49.44));

    //you must also select what kind of files you're looking for
    // IMAGERY, TERRAIN, MODEL, and/or PINNED
    bvector<RealityDataBase::Classification> classes = bvector<RealityDataBase::Classification>();
    classes.push_back(RealityDataBase::Classification::IMAGERY);
    classes.push_back(RealityDataBase::Classification::TERRAIN);

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir.c_str());
    outDir.AppendToPath(L"SimpleGCSApiTestDirectory");
    WString directory(outDir);
    if (BeFileName::DoesPathExist(directory.c_str()))
        BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    BeFileName::CreateNewDirectory(directory.c_str());
    
    BeFileName caBundlePath(exeDir);
    caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"cabundle.pem");

    //then you can launch the download, with the polygon points, the file types you want, a callback to a selection function
    //the existing directory in which to put the files and the path to the certificate file you wish to use
    GCSRequestManager::SimplePackageDownload(space, classes, selectionCallbackFunction, outDir, caBundlePath);

    //files will have been downloaded at this point. You can break, if you want to check

    BeFileName::EmptyAndRemoveDirectory(directory.c_str()); //cleaning up files downloaded by the example

    return 0;
    }