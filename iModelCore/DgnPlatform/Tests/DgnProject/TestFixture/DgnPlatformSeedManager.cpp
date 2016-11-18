/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnPlatformSeedManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnPlatformSeedManager.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == DgnPlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your TC_SETUP function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
static BeFileName getOutputPath(WStringCR relPath)
    {
    BeFileName outputPathName;
    BeTest::GetHost().GetOutputRoot(outputPathName);
    outputPathName.AppendToPath(relPath.c_str());
    return outputPathName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformSeedManager::MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName)
    {
    auto db = OpenSeedDbCopy(relSeedPath, newName);
    if (!db.IsValid())
        return DgnDbStatus::BadRequest;
    auto fn = db->GetFileName();
    actualName.SetName(fn.substr(getOutputPath(L"").length()));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void setBriefcase(DgnDbPtr& db, DgnDb::OpenMode mode)
    {
    if (db->IsBriefcase())
        return;

    BeFileName name(db->GetFileName());

    db->ChangeBriefcaseId(BeBriefcaseId(BeBriefcaseId::Standalone()));
    db->SaveChanges();
    db->CloseDb();

    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
WString DgnPlatformSeedManager::SeedDbOptions::ToKey() const
    {
    return WPrintfString(L"%d%d", testDomain, cameraView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnPlatformSeedManager::SeedDbInfo DgnPlatformSeedManager::GetOneSpatialModelSeedDb(SeedDbOptions const& options)
    {
    if (nullptr == DgnPlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnPlatformSeedManager::CreateSeedDbs. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
        return SeedDbInfo();
        }

    SeedDbInfo info;
    info.id = SeedDbId::OneSpatialModel;
    info.physicalPartitionCode = PartitionAuthority::CreatePartitionCode("DefaultModel", DgnElementId(1ULL)); // Root Subject
    info.options = options;
    info.fileName.SetName(WPrintfString(L"DgnPlatformSeedManager_OneSpatialModel%ls.bim", options.ToKey().c_str()));   // note that we need different files for different combinations of options.
    info.categoryName = "DefaultCategory";

    if (info.options.cameraView)
        info.viewName = "DefaultCameraView";

    if (getOutputPath(info.fileName).DoesPathExist())
        return info;

    //  First request for this seed file. Create it.
    DgnDbPtr db = CreateDgnDb(info.fileName, true, true);
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*db, info.physicalPartitionCode.GetValueCP());
    DgnDbTestUtils::InsertSpatialCategory(*db, info.categoryName.c_str());
    
    if (info.options.cameraView)
        DgnDbTestUtils::InsertCameraView(*model, info.viewName.c_str());

    if (info.options.testDomain)
        EXPECT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    db->SaveSettings();
    db->SaveChanges();
    return info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnPlatformSeedManager::SeedDbInfo DgnPlatformSeedManager::GetSeedDb(SeedDbId seedId, SeedDbOptions const& options)
    {
    if (nullptr == DgnPlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnPlatformSeedManager::GetSeedDbInfo. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
        return SeedDbInfo();
        }

    if (SeedDbId::OneSpatialModel == seedId)
        return GetOneSpatialModelSeedDb(options);

    BeAssert(false && "invalid SeedDbId");
    return SeedDbInfo();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BeFileNameStatus DgnPlatformSeedManager::CreateSubDirectory(WCharCP relPath)
    {
    BeFileName path = getOutputPath(relPath);
    if (path.IsDirectory() || path.DoesPathExist())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - already exists. Use a unique name for your test group's files", path.c_str()).c_str();
        return BeFileNameStatus::AlreadyExists;
        }
    return BeFileName::CreateNewDirectory(path.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnPlatformSeedManager::EmptySubDirectory(WCharCP relPath)
    {
    BeFileName path = getOutputPath(relPath);
    BeFileName::EmptyDirectory(path.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnPlatformSeedManager::CreateDgnDb(WCharCP relPath, bool isRoot, bool mustBeBriefcase)
    {
    MUST_HAVE_HOST(nullptr);

    BeFileName fileName = getOutputPath(relPath);

    if (fileName.GetExtension().empty())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - missing a file extension", relPath).c_str();
        return nullptr;
        }

    if (fileName.DoesPathExist())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - already exists. Use a unique name for your test group's files", fileName.c_str()).c_str();
        return nullptr;
        }

    if (!isRoot && BeFileName::GetDirectoryName(relPath).empty())
        {
        EXPECT_FALSE(true) << "DgnPlatformSeedManager::CreateDgnDb - the destination must be in a sub-directory with the same name as the test group.";
        return nullptr;
        }

    CreateDgnDbParams createDgnDbParams("DgnPlatformSeedManager");
    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!db.IsValid())
        EXPECT_FALSE(true) << WPrintfString(L"%ls - create failed", fileName.c_str()).c_str();

    if (mustBeBriefcase)
        setBriefcase(db, DgnDb::OpenMode::ReadWrite);

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnPlatformSeedManager::OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode)
    {
    BeFileName fileName = getOutputPath(relPath);
    DbResult openStatus;
    DgnDb::OpenParams openParams(mode);
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    if (!db.IsValid())
        EXPECT_FALSE(true) << WPrintfString(L"%ls - open failed with %x", fileName.c_str(), (int)openStatus).c_str();
    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnPlatformSeedManager::OpenSeedDb(WCharCP relSeedPath)
    {
    return OpenDgnDb(relSeedPath, DgnDb::OpenMode::Readonly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
static void supplyMissingDbExtension(WStringR name)
    {
    if (name.find(L".") == WString::npos)
        name.append(L".bim");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnPlatformSeedManager::OpenSeedDbCopy(WCharCP relSeedPathIn, WCharCP newName)
    {
    WString relSeedPath(relSeedPathIn);
    supplyMissingDbExtension(relSeedPath);
        
    BeFileName infileName = getOutputPath(relSeedPath.c_str());
    if (!infileName.DoesPathExist())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - file not found", infileName.c_str()).c_str();
        return nullptr;
        }

    //  Create a unique name for the output file, based on the input seed filename.

    //  1. Establish the basename and extension
    BeFileName ccRelPathBase;
    if (nullptr == newName)
        ccRelPathBase.SetName(relSeedPath);
    else
        {
        ccRelPathBase.SetName(BeFileName::GetDirectoryName(relSeedPath.c_str()));
        ccRelPathBase.AppendToPath(newName);
        supplyMissingDbExtension(ccRelPathBase);
        }
        
    //  2. Make sure that it is located in a subdirectory that is specific to the current test case. (That's how we keep test groups out of each other's way.)
    Utf8String tcname = BeTest::GetNameOfCurrentTestCase();
    if (!tcname.empty())
        {
        WString wtcname(tcname.c_str(), BentleyCharEncoding::Utf8);
        if (!wtcname.Equals(ccRelPathBase.substr(0, tcname.size())))
            {
            WString tmp(ccRelPathBase);
            ccRelPathBase.SetName(wtcname);
            ccRelPathBase.AppendToPath(tmp.c_str());
            }
        }
    else
        {
        // The caller is not a test. Caller must be a TC_SETUP function. We don't know the caller's TC name. At least check that he's specifying a subdirectory.
        if (ccRelPathBase.GetDirectoryName().empty())
            {
            EXPECT_FALSE(true) << "DgnPlatformSeedManager::OpenDgnDbCopy - the destination must be in a sub-directory.";
            return nullptr;
            }
        }

    //  Make sure the output subdirectory exists
    BeFileName subDir = getOutputPath(ccRelPathBase.GetDirectoryName());
    if (!BeFileName::IsDirectory(subDir.c_str()))
        {
        BeFileName::CreateNewDirectory(subDir.c_str());
        }

    //  3. Make sure it's unique
    BeFileName ccfileName;
    BeFileName ccRelPathUnique;
    int ncopies = 0;
    do  {
        if (0 == ncopies)
            ccRelPathUnique = ccRelPathBase;
        else
            ccRelPathUnique.SetName(WPrintfString(L"%ls-%d.%ls", ccRelPathBase.substr(0, ccRelPathBase.find(L".")).c_str(), ncopies, ccRelPathBase.GetExtension().c_str()));
        ccfileName = getOutputPath(ccRelPathUnique.c_str());
        ++ncopies;
        } 
    while (ccfileName.DoesPathExist());

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(infileName.c_str(), ccfileName.c_str(), /*failIfFileExists*/true);
    EXPECT_EQ(BeFileNameStatus::Success, fileStatus) << WPrintfString(L"%ls => %ls - copy failed", infileName.c_str(), ccfileName.c_str()).c_str();

    return OpenDgnDb(ccRelPathUnique.c_str(), DgnDb::OpenMode::ReadWrite);
    }
