/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/DgnDbTestUtils/DgnDbTestUtils.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as the OBJ produced from this 
// WARNING:   source file is also used in DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================

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
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId DgnDbTestUtils::QueryFirstGeometricModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::RepositoryModelId() == modelEntry.GetModelId()) || (DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;
        
        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsGeometricModel())
            return modelEntry.GetModelId();
        }

    BeAssert(false && "No GeometricModel found");
    return DgnModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DocumentListModelPtr DgnDbTestUtils::InsertDocumentListModel(DgnDbR db, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(*rootSubject, Utf8PrintfString("Partition for %s", modelCode.GetValueCP()).c_str()); // create a partition to model
    EXPECT_TRUE(partition.IsValid());
    DocumentListModelPtr model = DocumentListModel::CreateAndInsert(*partition, modelCode);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingPtr DgnDbTestUtils::InsertDrawing(DocumentListModelCR model, DgnCodeCR code, Utf8CP label)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingPtr drawing = Drawing::Create(model, code, label);
    EXPECT_TRUE(drawing.IsValid());
    EXPECT_TRUE(drawing->Insert().IsValid());
    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
SectionDrawingPtr DgnDbTestUtils::InsertSectionDrawing(DocumentListModelCR model, DgnCodeCR code, Utf8CP label)
    {
    MUST_HAVE_HOST(nullptr);
    SectionDrawingPtr drawing = SectionDrawing::Create(model, code, label);
    EXPECT_TRUE(drawing.IsValid());
    EXPECT_TRUE(drawing->Insert().IsValid());
    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
SheetPtr DgnDbTestUtils::InsertSheet(DocumentListModelCR model, DgnCodeCR code, Utf8CP label)
    {
    MUST_HAVE_HOST(nullptr);
    SheetPtr sheet = Sheet::Create(model, code, label);
    EXPECT_TRUE(sheet.IsValid());
    EXPECT_TRUE(sheet->Insert().IsValid());
    return sheet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingModelPtr DgnDbTestUtils::InsertDrawingModel(DrawingCR drawing, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingModelPtr model = DrawingModel::Create(drawing, modelCode);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(drawing.GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
SheetModelPtr DgnDbTestUtils::InsertSheetModel(SheetCR sheet, DgnCode modelCode, DPoint2dCR sheetSize)
    {
    MUST_HAVE_HOST(nullptr);
    SheetModelPtr model = SheetModel::Create(sheet, modelCode, sheetSize);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(sheet.GetSubModelId(), model->GetModelId());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
LinkModelPtr DgnDbTestUtils::InsertLinkModel(DgnDbR db, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    LinkPartitionCPtr partition = LinkPartition::CreateAndInsert(*rootSubject, Utf8PrintfString("Partition for %s", modelCode.GetValueCP()).c_str()); // create a placeholder Subject for this DgnModel to describe
    EXPECT_TRUE(partition.IsValid());
    LinkModelPtr model = new LinkModel(LinkModel::CreateParams(db, partition->GetElementId(), modelCode));
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr DgnDbTestUtils::InsertPhysicalModel(DgnDbR db, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    Utf8PrintfString partitionLabel("Partition for %s", modelCode.GetValueCP());
    PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionLabel.c_str());
    EXPECT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition, modelCode);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
    return model;
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

    ViewControllerPtr viewController = view->LoadViewController();
    viewController->GetViewDefinition().LookAtVolume(db.Units().GetProjectExtents());

    DgnDbStatus stat;
    viewController->GetViewDefinition().Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
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
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Scope scope, DgnCategory::Rank rank)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    return InsertCategory(db, categoryName, appearance, scope, rank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnAuthorityId DgnDbTestUtils::InsertNamespaceAuthority(DgnDbR db, Utf8CP authorityName)
    {
    MUST_HAVE_HOST(DgnAuthorityId());
    DgnAuthorityPtr authority = NamespaceAuthority::CreateNamespaceAuthority(authorityName, db);
    EXPECT_TRUE(authority.IsValid());
    DgnDbStatus status = authority->Insert();
    EXPECT_TRUE(DgnDbStatus::Success == status) << WPrintfString(L"%ls - Authority insert into %ls failed with %x", WString(authorityName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str(), (int)status).c_str();
    EXPECT_TRUE(authority->GetAuthorityId().IsValid());
    return authority->GetAuthorityId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/16
//---------------------------------------------------------------------------------------
ModelSelectorCPtr DgnDbTestUtils::InsertNewModelSelector(DgnDbR db, Utf8CP name, DgnModelId model)
    {
    ModelSelector modSel(db, name);
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
void addAllCategories(DgnDbR db, CategorySelectorR selector)
    {
    for (auto const& categoryId : DgnCategory::QueryCategories(db))
        selector.AddCategory(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingViewDefinitionPtr DgnDbTestUtils::InsertDrawingView(DrawingModelR model, Utf8CP viewDescr)
    {
    auto& db = model.GetDgnDb();
    DrawingViewDefinitionPtr viewDef = new DrawingViewDefinition(db, model.GetName(), DrawingViewDefinition::QueryClassId(db), model.GetModelId(), *new CategorySelector(db,""), *new DisplayStyle(db,""));
    addAllCategories(db, viewDef->GetCategorySelector());
    viewDef->Insert();
    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnDbTestUtils::InsertCameraView(SpatialModelR model, Utf8CP viewName, DRange3dCP viewVolume, StandardView rot, Render::RenderMode renderMode)
    {
    auto& db = model.GetDgnDb();
    CameraViewDefinition viewDef(db, viewName ? viewName : model.GetName(), *new CategorySelector(db,""), *new DisplayStyle3d(db,""), *new ModelSelector(db,""));
    addAllCategories(db, viewDef.GetCategorySelector());
    viewDef.Insert();
    return viewDef.GetViewId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/16
//---------------------------------------------------------------------------------------
CategorySelectorCPtr DgnDbTestUtils::InsertNewCategorySelector(DgnDbR db, Utf8CP name, DgnCategoryIdSet const* categoriesIn)
    {
    CategorySelector catSel(db, name);

    DgnCategoryIdSet const* categories = categoriesIn;
    DgnCategoryIdSet _categories;
    if (nullptr == categories)
        {
        for (auto const& categoryId : DgnCategory::QueryCategories(db))
            _categories.insert(categoryId);
        categories = &_categories;
        }

    if (!categories->empty())
        {
        for (auto id : *categories)
            catSel.AddCategory(id);
        }

    CategorySelectorCPtr catSelPersist = db.Elements().Insert(catSel);
    if (!catSelPersist.IsValid())
        {
        EXPECT_TRUE(false) << " Insertion of CategorySelector with name " << name << " failed";
        return nullptr;
        }

    EXPECT_EQ(catSelPersist.get(), db.Elements().GetElement(catSel.GetElementId()).get());
    
    auto& categoriesStored = catSelPersist->GetCategories();
    EXPECT_EQ(categoriesStored, *categories);

    return catSelPersist;
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

    Utf8PrintfString sql("SELECT COUNT(*) FROM %s", tableName);

    Statement statement;
    DbResult status = statement.Prepare(db, sql.c_str());
    if (BE_SQLITE_OK != status)
        return -1;

    if (BE_SQLITE_ROW != statement.Step())
        return -1;

    return statement.GetValueInt(0);
    }
