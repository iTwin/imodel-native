/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as the OBJ produced from this 
// WARNING:   source file is also used in DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Render.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == DgnPlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your SetUpTestCase function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId DgnDbTestUtils::QueryFirstGeometricModelId(DgnDbR db)
    {
    ModelIterator iterator = db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel));
    if (iterator.begin() == iterator.end())
        {
        BeAssert(false && "No GeometricModel found");
        return DgnModelId();
        }

    return (*iterator.begin()).GetModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           03/2017
//---------------------------------------------------------------------------------------
InformationRecordModelPtr DgnDbTestUtils::InsertInformationRecordModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    InformationRecordPartitionCPtr partition = InformationRecordPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    InformationRecordModelPtr model = InformationRecordModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DocumentListModelPtr DgnDbTestUtils::InsertDocumentListModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    DocumentListModelPtr model = DocumentListModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingPtr DgnDbTestUtils::InsertDrawing(DocumentListModelCR model, Utf8CP name)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingPtr drawing = Drawing::Create(model, name);
    EXPECT_TRUE(drawing.IsValid());
    EXPECT_TRUE(drawing->Insert().IsValid());
    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
SectionDrawingPtr DgnDbTestUtils::InsertSectionDrawing(DocumentListModelCR model, Utf8CP name)
    {
    MUST_HAVE_HOST(nullptr);
    SectionDrawingPtr drawing = SectionDrawing::Create(model, name);
    EXPECT_TRUE(drawing.IsValid());
    EXPECT_TRUE(drawing->Insert().IsValid());
    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
Sheet::ElementPtr DgnDbTestUtils::InsertSheet(DocumentListModelCR model, double scale, double height, double width, Utf8CP name)
    {
    MUST_HAVE_HOST(nullptr);
    Sheet::ElementPtr sheet = Sheet::Element::Create(model, scale, DPoint2d::From(width,height), name);
    EXPECT_TRUE(sheet.IsValid());
    EXPECT_TRUE(sheet->Insert().IsValid());
    return sheet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
Sheet::ElementPtr DgnDbTestUtils::InsertSheet(DocumentListModelCR model, double scale, DgnElementId templateId, Utf8CP name)
    {
    MUST_HAVE_HOST(nullptr);
    Sheet::ElementPtr sheet = Sheet::Element::Create(model, scale, templateId, name);
    EXPECT_TRUE(sheet.IsValid());
    EXPECT_TRUE(sheet->Insert().IsValid());
    return sheet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingModelPtr DgnDbTestUtils::InsertDrawingModel(DrawingCR drawing)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingModelPtr model = DrawingModel::Create(drawing);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(drawing.GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
Sheet::ModelPtr DgnDbTestUtils::InsertSheetModel(Sheet::ElementCR sheet)
    {
    MUST_HAVE_HOST(nullptr);
    Sheet::ModelPtr model = Sheet::Model::Create(sheet);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(sheet.GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
LinkModelPtr DgnDbTestUtils::InsertLinkModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    LinkPartitionCPtr partition = LinkPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    LinkModelPtr model = new LinkModel(LinkModel::CreateParams(db, partition->GetElementId()));
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           02/2017
//---------------------------------------------------------------------------------------
DefinitionModelPtr DgnDbTestUtils::InsertDefinitionModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    DefinitionPartitionCPtr partition = DefinitionPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    DefinitionModelPtr model = DefinitionModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           02/2017
//---------------------------------------------------------------------------------------
GenericGroupModelPtr DgnDbTestUtils::InsertGroupInformationModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    GroupInformationPartitionCPtr partition = GroupInformationPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    GenericGroupModelPtr model = GenericGroupModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr DgnDbTestUtils::InsertPhysicalModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           11/2016
//---------------------------------------------------------------------------------------
SpatialLocationModelPtr DgnDbTestUtils::InsertSpatialLocationModel(DgnDbR db, Utf8CP partitionName)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionCPtr partition = SpatialLocationPartition::CreateAndInsert(*rootSubject, partitionName);
    EXPECT_TRUE(partition.IsValid());
    SpatialLocationModelPtr model = SpatialLocationModel::CreateAndInsert(*partition);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::UpdateProjectExtents(DgnDbR db)
    {
    db.GeoLocation().InitializeProjectExtents();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::FitView(DgnDbR db, DgnViewId viewId)
    {
    SpatialViewDefinitionCPtr view = dynamic_cast<SpatialViewDefinitionCP>(ViewDefinition::Get(db, viewId).get());
    ASSERT_TRUE(view.IsValid());

    ViewControllerPtr viewController = view->LoadViewController();
    viewController->GetViewDefinitionR().LookAtVolume(db.GeoLocation().GetProjectExtents());

    DgnDbStatus stat;
    viewController->GetViewDefinitionR().Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertDrawingCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Rank rank)
    {
    MUST_HAVE_HOST(DgnCategoryId());

    DrawingCategory category(db.GetDictionaryModel(), categoryName, rank);
    DrawingCategoryCPtr persistentCategory = category.Insert(appearance);
    EXPECT_TRUE(persistentCategory.IsValid());

    return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertSpatialCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Rank rank)
    {
    MUST_HAVE_HOST(DgnCategoryId());

    SpatialCategory category(db.GetDictionaryModel(), categoryName, rank);
    SpatialCategoryCPtr persistentCategory = category.Insert(appearance);
    EXPECT_TRUE(persistentCategory.IsValid());

    return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           11/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertDrawingCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    return InsertDrawingCategory(db, categoryName, appearance, rank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           11/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertSpatialCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    return InsertSpatialCategory(db, categoryName, appearance, rank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           11/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::GetFirstDrawingCategoryId(DgnDbR db)
    {
    MUST_HAVE_HOST(DgnCategoryId());
    DgnCategoryId categoryId = (*DrawingCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    EXPECT_TRUE(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           11/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::GetFirstSpatialCategoryId(DgnDbR db)
    {
    MUST_HAVE_HOST(DgnCategoryId());
    DgnCategoryId categoryId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    EXPECT_TRUE(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           02/2017
//---------------------------------------------------------------------------------------
DgnSubCategoryId DgnDbTestUtils::InsertSubCategory(DgnDbR db, DgnCategoryId categoryId, Utf8CP name, ColorDefCR color)
    {
    MUST_HAVE_HOST(DgnSubCategoryId());
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    DgnSubCategoryPtr newSubCategory = new DgnSubCategory(DgnSubCategory::CreateParams(db, categoryId, name, appearance));
    EXPECT_TRUE(newSubCategory.IsValid());
    DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
    EXPECT_TRUE(insertedSubCategory.IsValid());
    return insertedSubCategory->GetSubCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
CodeSpecId DgnDbTestUtils::InsertCodeSpec(DgnDbR db, Utf8CP codeSpecName)
    {
    MUST_HAVE_HOST(CodeSpecId());
    CodeSpecPtr codeSpec = CodeSpec::Create(db, codeSpecName);
    EXPECT_TRUE(codeSpec.IsValid());
    DgnDbStatus status = codeSpec->Insert();
    EXPECT_TRUE(DgnDbStatus::Success == status) << WPrintfString(L"%ls - CodeSpec insert into %ls failed with %x", WString(codeSpecName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str(), (int)status).c_str();
    EXPECT_TRUE(codeSpec->GetCodeSpecId().IsValid());
    return codeSpec->GetCodeSpecId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/16
//---------------------------------------------------------------------------------------
ModelSelectorCPtr DgnDbTestUtils::InsertModelSelector(DgnDbR db, Utf8CP name, DgnModelId model)
    {
    ModelSelector modSel(db.GetDictionaryModel(), name);
    modSel.AddModel(model);
    auto modSelPersist = db.Elements().Insert(modSel);
    if (!modSelPersist.IsValid())
        {
        EXPECT_TRUE(false) << " Failed to insert model selector with name =" << name;
        return nullptr;
        }

    auto& models = modSelPersist->GetModels();
    EXPECT_EQ(1, models.size());
    EXPECT_EQ(model, *models.begin());
    return modSelPersist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewDefinitionPtr DgnDbTestUtils::InsertDrawingView(DrawingModelR drawingModel, Utf8CP viewName)
    {
    DgnDbR db = drawingModel.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();
    DrawingViewDefinitionPtr viewDef = new DrawingViewDefinition(dictionary, viewName ? viewName : drawingModel.GetName(), DrawingViewDefinition::QueryClassId(db), drawingModel.GetModelId(), *new CategorySelector(dictionary,""), *new DisplayStyle2d(dictionary,""));
    EXPECT_TRUE(viewDef.IsValid());

    for (ElementIteratorEntryCR categoryEntry : DrawingCategory::MakeIterator(db))
        viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    EXPECT_TRUE(viewDef->Insert().IsValid());
    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnDbTestUtils::InsertCameraView(SpatialModelR model, Utf8CP viewName, DRange3dCP viewVolume, StandardView rot, Render::RenderMode renderMode)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();
    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
    modelSelector->AddModel(model.GetModelId());

    SpatialViewDefinition viewDef(dictionary, viewName ? viewName : model.GetName(), *new CategorySelector(dictionary,""), *new DisplayStyle3d(dictionary,""), *modelSelector);

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        viewDef.GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    EXPECT_TRUE(viewDef.Insert().IsValid());
    return viewDef.GetViewId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2014
//---------------------------------------------------------------------------------------
int DgnDbTestUtils::SelectCountFromECClass(DgnDbR db, Utf8CP className)
    {
    if (!className || !*className)
        return -1;

    Utf8PrintfString sql("SELECT COUNT(*) FROM %s", className);

    ECSqlStatement statement;
    ECSqlStatus status = statement.Prepare(db, sql.c_str());
    if (ECSqlStatus::Success != status)
        return -1;

    if (BE_SQLITE_ROW != statement.Step())
        return -1;

    return statement.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    07/2015
//---------------------------------------------------------------------------------------
int DgnDbTestUtils::SelectCountFromTable(DgnDbR db, Utf8CP tableName)
    {
    if (!tableName || !*tableName)
        return -1;

    EXPECT_TRUE(db.TableExists(tableName));
    Utf8PrintfString sql("SELECT COUNT(*) FROM %s", tableName);

    Statement statement;
    DbResult status = statement.Prepare(db, sql.c_str());
    if (BE_SQLITE_OK != status)
        return -1;

    if (BE_SQLITE_ROW != statement.Step())
        return -1;

    return statement.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2016
//---------------------------------------------------------------------------------------
bool DgnDbTestUtils::CodeValueExists(DgnDbR db, Utf8CP codeValue)
    {
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare(db, "SELECT * FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE CodeValue=? LIMIT 1");
    ECSqlStatus bindStatus = statement.BindText(1, codeValue, IECSqlBinder::MakeCopy::No);
    if ((ECSqlStatus::Success != prepareStatus) || (ECSqlStatus::Success != bindStatus))
        {
        BeAssert(false);
        return false;
        }

    return (BE_SQLITE_ROW == statement.Step());
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
DgnDbPtr DgnDbTestUtils::CreateDgnDb(WCharCP relPath, bool isRoot)
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

    CreateDgnDbParams createDgnDbParams("DgnDbTestUtils");
    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!db.IsValid())
        EXPECT_FALSE(true) << WPrintfString(L"%ls - create failed", fileName.c_str()).c_str();

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
        // The caller is not a test. Caller must be a SetUpTestCase function. We don't know the caller's TC name. At least check that he's specifying a subdirectory.
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
