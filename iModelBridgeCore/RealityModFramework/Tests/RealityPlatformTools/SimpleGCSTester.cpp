//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatformTools/SimpleGCSTester.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"
#include <RealityPlatformTools/SimpleGCSApi.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
#include <Bentley/BeFileListIterator.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::Mock;
using ::testing::Return;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  01/2018
//! SimpleGCSFixture
//=====================================================================================
class SimpleGCSFixture : public testing::Test// : public MockWSGRequestFixture
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

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  01/2018
//=====================================================================================
TEST_F(SimpleGCSFixture, GCSRequestManagerTest)
    {
    GCSRequestManager::Setup("");

    ASSERT_TRUE(!GeoCoordinationService::GetServerName().empty());
    ASSERT_TRUE(!GeoCoordinationService::GetWSGProtocol().empty());
    ASSERT_EQ(GeoCoordinationService::GetRepoName(), "IndexECPlugin--Server");
    ASSERT_EQ(GeoCoordinationService::GetSchemaName(), "RealityModeling");
    }

bvector<Utf8String> Select(bvector<GeoPoint2d> footprint, SpatialEntityDatasetPtr dataset)
    {
    bvector<Utf8String> returnVec = bvector<Utf8String>();

    auto imageryIt(dataset->GetTerrainGroupR().begin());
    SpatialEntityPtr septr = (*imageryIt);

    while (imageryIt != dataset->GetTerrainGroupR().end())
        {
        if(((*imageryIt)->GetApproximateFileSize() > 1) &&
            ((*imageryIt)->GetApproximateFileSize() < septr->GetApproximateFileSize()))
            septr = (*imageryIt);
        imageryIt++;
        }

    returnVec.push_back(septr->GetIdentifier());

    return returnVec;
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  01/2018
//=====================================================================================
TEST_F(SimpleGCSFixture, SimpleGCSDownloadTest)
    {
    GCSRequestManager::Setup("");

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
    ASSERT_TRUE(fileIt.GetNextFileName(fileName) == BentleyStatus::SUCCESS); //ensure there's a file in the folder

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }
