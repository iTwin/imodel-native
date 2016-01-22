//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/BaseTestGroup.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "BaseTestGroup.h"
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == DgnPlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your TC_SETUP function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
static BeFileName getOutputPath(WCharCP relPath)
    {
    BeFileName outputPathName;
    BeTest::GetHost().GetOutputRoot(outputPathName);
    outputPathName.AppendToPath(relPath);
    return outputPathName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BeFileNameStatus BaseTestGroup::CreateSubDirectory(WCharCP relPath)
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
void BaseTestGroup::EmptySubDirectory(WCharCP relPath)
    {
    BeFileName path = getOutputPath(relPath);
    BeFileName::EmptyDirectory(path.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr BaseTestGroup::CreateDgnDb(WCharCP relPath)
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

    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(false);

    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, fileName, createProjectParams);
    if (!db.IsValid())
        EXPECT_FALSE(true) << WPrintfString(L"%ls - create failed", fileName.c_str()).c_str();

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr BaseTestGroup::OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode)
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
DgnDbPtr BaseTestGroup::OpenDgnDb(WCharCP relSeedPath)
    {
    return OpenDgnDb(relSeedPath, DgnDb::OpenMode::Readonly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnDbPtr BaseTestGroup::CopyDgnDb(WCharCP redlSeedPath, WCharCP nameOfCopy)
    {
    BeFileName infileName = getOutputPath(redlSeedPath);
    BeFileName ccfileName = getOutputPath(nameOfCopy);

    if (ccfileName.GetExtension().empty())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - missing a file extension", nameOfCopy).c_str();
        return nullptr;
        }

    if (ccfileName.DoesPathExist())
        {
        EXPECT_FALSE(true) << WPrintfString(L"%ls - read-write copy of file already exists. Use a unique name for your test's copy.", ccfileName.c_str()).c_str();
        return nullptr;
        }

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(infileName.c_str(), ccfileName.c_str(), /*failIfFileExists*/true);
    EXPECT_EQ(BeFileNameStatus::Success, fileStatus) << WPrintfString(L"%ls => %ls - copy failed", infileName.c_str(), ccfileName.c_str()).c_str();

    return OpenDgnDb(nameOfCopy, DgnDb::OpenMode::ReadWrite);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialModelPtr BaseTestGroup::InsertSpatialModel(DgnDbR db, Utf8CP modelName)
    {
    MUST_HAVE_HOST(nullptr);

    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr catalogModel = new SpatialModel(DgnModel3d::CreateParams(db, mclassId, DgnModel::CreateModelCode(modelName)));
    DgnDbStatus status = catalogModel->Insert();
    EXPECT_EQ(DgnDbStatus::Success, status) << WPrintfString(L"%ls - insert into %ls failed with %x", modelName, db.GetFileName().c_str(), (int)status).c_str();
    return catalogModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void BaseTestGroup::UpdateProjectExtents(DgnDbR db)
    {
    AxisAlignedBox3d physicalExtents;
    physicalExtents = db.Units().ComputeProjectExtents();
    db.Units().SaveProjectExtents(physicalExtents);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void BaseTestGroup::FitView(DgnDbR db, DgnViewId viewId)
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
DgnCategoryId BaseTestGroup::InsertCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Scope scope, DgnCategory::Rank rank)
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
DgnAuthorityId BaseTestGroup::InsertNamespaceAuthority(DgnDbR db, Utf8CP authorityName)
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
DgnViewId BaseTestGroup::InsertCameraView(SpatialModelR model, Utf8CP nameIn)
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
    viewFlags.SetRenderMode(DgnRenderMode::SmoothShade);
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
