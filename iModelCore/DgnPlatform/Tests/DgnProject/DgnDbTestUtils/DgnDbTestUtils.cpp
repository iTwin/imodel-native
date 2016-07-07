//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/DgnDbTestUtils/DgnDbTestUtils.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Render.h>
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
DgnDbStatus DgnDbTestUtils::MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName)
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
WString DgnDbTestUtils::SeedDbOptions::ToKey() const
    {
    return WPrintfString(L"%d%d", testDomain, cameraView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbTestUtils::SeedDbInfo DgnDbTestUtils::GetOneSpatialModelSeedDb(SeedDbOptions const& options)
    {
    if (nullptr == DgnPlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnDbTestUtils::CreateSeedDbs. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
        return SeedDbInfo();
        }

    SeedDbInfo info;
    info.id = SeedDbId::OneSpatialModel;
    info.options = options;
    info.fileName.SetName(WPrintfString(L"DgnDbTestUtils_OneSpatialModel%ls.bim", options.ToKey().c_str()));   // note that we need different files for different combinations of options.
    info.modelCode = DgnModel::CreateModelCode("DefaultModel");
    info.categoryName = "DefaultCategory";

    if (info.options.cameraView)
        info.viewName = "DefaultCameraView";

    if (getOutputPath(info.fileName).DoesPathExist())
        return info;

    //  First request for this seed file. Create it.
    DgnDbPtr db = CreateDgnDb(info.fileName, true, true);
    SpatialModelPtr model = InsertSpatialModel(*db, info.modelCode);
    InsertCategory(*db, info.categoryName.c_str());
    
    if (info.options.cameraView)
        InsertCameraView(*model, info.viewName.c_str());

    if (info.options.testDomain)
        EXPECT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    db->SaveSettings();
    db->SaveChanges();
    return info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbTestUtils::SeedDbInfo DgnDbTestUtils::GetSeedDb(SeedDbId seedId, SeedDbOptions const& options)
    {
    if (nullptr == DgnPlatformLib::QueryHost())
        {
        EXPECT_TRUE(false) << "Your TC_SETUP function must set up a host before calling DgnDbTestUtils::GetSeedDbInfo. Just put an instance of ScopedDgnHost on the stack at the top of your function.";
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
BeFileNameStatus DgnDbTestUtils::CreateSubDirectory(WCharCP relPath)
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
void DgnDbTestUtils::EmptySubDirectory(WCharCP relPath)
    {
    BeFileName path = getOutputPath(relPath);
    BeFileName::EmptyDirectory(path.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnDbTestUtils::CreateDgnDb(WCharCP relPath, bool isRoot, bool mustBeBriefcase)
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
        EXPECT_FALSE(true) << "DgnDbTestUtils::CreateDgnDb - the destination must be in a sub-directory with the same name as the test group.";
        return nullptr;
        }

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(false);

    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, fileName, createProjectParams);
    if (!db.IsValid())
        EXPECT_FALSE(true) << WPrintfString(L"%ls - create failed", fileName.c_str()).c_str();

    if (mustBeBriefcase)
        setBriefcase(db, DgnDb::OpenMode::ReadWrite);

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr DgnDbTestUtils::OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode)
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
DgnDbPtr DgnDbTestUtils::OpenSeedDb(WCharCP relSeedPath)
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
DgnDbPtr DgnDbTestUtils::OpenSeedDbCopy(WCharCP relSeedPathIn, WCharCP newName)
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
            EXPECT_FALSE(true) << "DgnDbTestUtils::OpenDgnDbCopy - the destination must be in a sub-directory.";
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

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialModelPtr DgnDbTestUtils::InsertSpatialModel(DgnDbR db, DgnCode modelCode)
    {
    MUST_HAVE_HOST(nullptr);

    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr catalogModel = new SpatialModel(SpatialModel::CreateParams(db, mclassId, modelCode));
    DgnDbStatus status = catalogModel->Insert();
    EXPECT_EQ(DgnDbStatus::Success, status) << WPrintfString(L"%ls - insert into %ls failed with %x", modelCode.GetValue().c_str(), db.GetFileName().c_str(), (int)status).c_str();
    return catalogModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::UpdateProjectExtents(DgnDbR db)
    {
    AxisAlignedBox3d physicalExtents;
    physicalExtents = db.Units().ComputeProjectExtents();
    db.Units().SetProjectExtents(physicalExtents);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::FitView(DgnDbR db, DgnViewId viewId)
    {
    SpatialViewDefinitionCPtr view = dynamic_cast<SpatialViewDefinitionCP>(ViewDefinition::QueryView(viewId, db).get());
    ASSERT_TRUE(view.IsValid());

    ViewControllerPtr viewController = view->LoadViewController(ViewDefinition::FillModels::No);
    viewController->LookAtVolume(db.Units().GetProjectExtents());
    ASSERT_EQ(BE_SQLITE_OK, viewController->Save());

    db.SaveSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Scope scope, DgnCategory::Rank rank)
    {
    MUST_HAVE_HOST(DgnCategoryId());

    DgnCategory category(DgnCategory::CreateParams(db, categoryName, scope, rank));

    DgnCategoryCPtr persistentCategory = category.Insert(appearance);
    EXPECT_TRUE(persistentCategory.IsValid()) << WPrintfString(L"%ls - Category insert into %ls failed", WString(categoryName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str()).c_str();

    return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnAuthorityId DgnDbTestUtils::InsertNamespaceAuthority(DgnDbR db, Utf8CP authorityName)
    {
    MUST_HAVE_HOST(DgnAuthorityId());
    
    DgnAuthorityPtr authority = NamespaceAuthority::CreateNamespaceAuthority(authorityName, db);
    DgnDbStatus status = authority->Insert();
    EXPECT_TRUE(DgnDbStatus::Success == status) << WPrintfString(L"%ls - Authority insert into %ls failed with %x", WString(authorityName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str(), (int)status).c_str();
    return authority.IsValid()? authority->GetAuthorityId(): DgnAuthorityId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/15
//---------------------------------------------------------------------------------------
DgnViewId DgnDbTestUtils::InsertCameraView(SpatialModelR model, Utf8CP nameIn)
    {
    MUST_HAVE_HOST(DgnViewId());
    
    DgnDbR db = model.GetDgnDb();

    Utf8String name;
    if (nullptr == nameIn)
        name = model.GetCode().GetValue();
    else
        name = nameIn;

    CameraViewDefinition view(CameraViewDefinition::CreateParams(db, name, ViewDefinition::Data(model.GetModelId())));
    ViewDefinitionCPtr newViewDef = view.Insert();
    if (!newViewDef.IsValid())
        {
        EXPECT_TRUE(false) << WPrintfString(L"%ls - CameraViewController insert into %ls", WString(name.c_str(),BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str()).c_str();
        return DgnViewId();
        }

    DgnViewId viewId = view.GetViewId();
    
    CameraViewController* viewController = new CameraViewController(db, viewId);
    if (nullptr == viewController)
        {
        EXPECT_TRUE(false) << WPrintfString(L"%ls - CameraViewController ctor failed in %ls", WString(name.c_str(),BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str()).c_str();
        return DgnViewId();
        }

    viewController->SetBaseModelId(model.GetModelId());

    for (auto const& categoryId : DgnCategory::QueryCategories(db))
        viewController->ChangeCategoryDisplay(categoryId, true);

    viewController->SetCameraOn(false);

    viewController->SetOrigin(DPoint3d::From(-5,-5,-5));
    viewController->SetDelta(DVec3d::From(10,10,10));

    auto& viewFlags = viewController->GetViewFlagsR();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    viewFlags.constructions = true;
    viewFlags.dimensions = true;
    viewFlags.weights = true;
    viewFlags.transparency = true;
    viewFlags.fill = true;
    viewFlags.materials = true;
    viewFlags.patterns = true;
    viewFlags.shadows = true;
    viewFlags.grid = true;
    viewFlags.acs = true;

    viewController->Save();

    return viewId;
    }
