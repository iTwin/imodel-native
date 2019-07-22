/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"
#include <RealityPlatformTools/SimpleGCSApi.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
#include <Bentley/BeFileListIterator.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  01/2018
//! SimpleGCSFixture
//=====================================================================================
class SimpleGCSIntegrationTests : public testing::Test
    {
public:
    WCharCP GetDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetTempDir(outDir);
        outDir.AppendToPath(L"RealityDataServiceConsoleTestDirectory");
        return outDir;
        }

    void InitTestDirectory(WCharCP directoryname)
        {
        if (BeFileName::DoesPathExist(directoryname))
            BeFileName::EmptyAndRemoveDirectory(directoryname);
        BeFileName::CreateNewDirectory(directoryname);
        }

    BeFileName GetPemLocation()
        {
        BeFileName outDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(outDir);
        outDir.AppendToPath(L"http").AppendToPath(L"cabundle.pem");
        return outDir;
        }
    };

bvector<Utf8String> Select(bvector<GeoPoint2d> footprint, SpatialEntityDatasetPtr dataset)
    {
    bvector<Utf8String> returnVec = bvector<Utf8String>();

    auto imageryIt(dataset->GetTerrainGroupR().begin());
    SpatialEntityPtr septr = (*imageryIt);

    while (imageryIt != dataset->GetTerrainGroupR().end())
        {
        if (((*imageryIt)->GetApproximateFileSize() > 1) &&
            ((*imageryIt)->GetApproximateFileSize() < septr->GetApproximateFileSize()))
            septr = (*imageryIt);
        imageryIt++;
        }

    returnVec.push_back(septr->GetIdentifier());

    return returnVec;
    }

TEST_F(SimpleGCSIntegrationTests, DownloadTest)
    {
    GCSRequestManager::Setup();

    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(-79.25, 38.57));
    myFootprint.push_back(GeoPoint2d::From(-79.05, 38.57));
    myFootprint.push_back(GeoPoint2d::From(-79.05, 38.77));
    myFootprint.push_back(GeoPoint2d::From(-79.25, 38.77));
    myFootprint.push_back(GeoPoint2d::From(-79.25, 38.57));

    bvector<RealityDataBase::Classification> classes;
    classes.push_back(RealityDataBase::Classification::MODEL);
    classes.push_back(RealityDataBase::Classification::TERRAIN);
    classes.push_back(RealityDataBase::Classification::IMAGERY);

    WString directory(GetDirectory());
    directory.append(L"SimpleGCSDownloadTest");
    InitTestDirectory(directory.c_str());

    GCSRequestManager::SimplePackageDownload(myFootprint, classes, Select, BeFileName(directory.c_str()), GetPemLocation(), nullptr);

    BeFileListIterator fileIt = BeFileListIterator(directory.c_str(), true);

    BeFileName fileName;

    int filecount = 0;
    while (fileIt.GetNextFileName(fileName) == BentleyStatus::SUCCESS)
        filecount++;

    ASSERT_TRUE(filecount > 1); //ensure there's a package file and at least 1 downloaded file, in the folder

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }