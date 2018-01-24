/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/SimpleGCSApiExample.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    GCSRequestManager::Setup();

    Utf8String bingKey, expirationDate;
    GCSRequestManager::SimpleBingKeyRequest("1000", bingKey, expirationDate);

    bvector<GeoPoint2d> space = bvector<GeoPoint2d>();
    space.push_back(GeoPoint2d::From(115.73, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 50.18));
    space.push_back(GeoPoint2d::From(116.53, 49.44));
    space.push_back(GeoPoint2d::From(115.73, 49.44));

    bvector<RealityDataBase::Classification> classes = bvector<RealityDataBase::Classification>();
    classes.push_back(RealityDataBase::Classification::IMAGERY);
    classes.push_back(RealityDataBase::Classification::TERRAIN);

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"SimpleGCSApiTestDirectory");
    WString directory(outDir);
    if (BeFileName::DoesPathExist(directory.c_str()))
        BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    BeFileName::CreateNewDirectory(directory.c_str());
    
    BeFileName caBundlePath(exeDir);
    caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"cabundle.pem");

    GCSRequestManager::SimplePackageDownload(space, classes, selectionCallbackFunction, outDir, caBundlePath);

    //files will have been downloaded at this point. You can break, if you want to check

    BeFileName::EmptyAndRemoveDirectory(directory.c_str()); //cleaning up files downloaded by the example

    return 0;
    }