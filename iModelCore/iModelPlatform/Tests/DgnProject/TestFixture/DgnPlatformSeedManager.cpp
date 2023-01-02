/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "DgnPlatformSeedManager.h"
#include <DgnPlatform/PlatformLib.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static BeFileName getOutputPath(WStringCR relPath)
    {
    BeFileName outputPathName;
    BeTest::GetHost().GetOutputRoot(outputPathName);
    outputPathName.AppendToPath(relPath.c_str());
    return outputPathName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WString DgnPlatformSeedManager::SeedDbOptions::ToKey() const
    {
    return WPrintfString(L"%d%d", testDomain, cameraView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnPlatformSeedManager::SeedDbInfo DgnPlatformSeedManager::GetOneSpatialModelSeedDb(SeedDbOptions const& options)
    {
    if (nullptr == PlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnPlatformSeedManager::CreateSeedDbs. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
        return SeedDbInfo();
        }

    SeedDbInfo info;
    info.id = SeedDbId::OneSpatialModel;
    info.physicalPartitionName = "DefaultModel";
    info.options = options;
    info.fileName.SetName(WPrintfString(L"DgnPlatformSeedManager_OneSpatialModel%ls.bim", options.ToKey().c_str()));   // note that we need different files for different combinations of options.
    info.categoryName = "DefaultCategory";

    if (info.options.cameraView)
        info.viewName = "DefaultCameraView";

    if (getOutputPath(info.fileName).DoesPathExist())
        return info;

    if (info.options.testDomain)
        DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);

    //  First request for this seed file. Create it.
    DgnDbPtr db = DgnDbTestUtils::CreateDgnDb(info.fileName, true);
    if (!db.IsValid())
        return info;

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*db, info.physicalPartitionName.c_str());
    DgnDbTestUtils::InsertSpatialCategory(*db, info.categoryName.c_str());

    if (info.options.cameraView)
        DgnDbTestUtils::InsertCameraView(*model, info.viewName.c_str());

    if (info.options.testDomain)
        EXPECT_EQ(SchemaStatus::Success, DgnPlatformTestDomain::GetDomain().ImportSchema(*db));

    db->SaveChanges();
    return info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnPlatformSeedManager::SeedDbInfo DgnPlatformSeedManager::GetSeedDb(SeedDbId seedId, SeedDbOptions const& options)
    {
    if (nullptr == PlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnPlatformSeedManager::GetSeedDbInfo. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
        return SeedDbInfo();
        }

    if (SeedDbId::OneSpatialModel == seedId)
        return GetOneSpatialModelSeedDb(options);

    BeAssert(false && "invalid SeedDbId");
    return SeedDbInfo();
    }

