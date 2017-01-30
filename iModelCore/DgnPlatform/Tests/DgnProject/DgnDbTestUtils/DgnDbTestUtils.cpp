/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/DgnDbTestUtils/DgnDbTestUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as the OBJ produced from this 
// WARNING:   source file is also used in DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/Sheet.h>
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
    ModelIterator iterator = db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel));
    if (iterator.begin() == iterator.end())
        {
        BeAssert(false && "No GeometricModel found");
        return DgnModelId();
        }

    return (*iterator.begin()).GetModelId();
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
    Sheet::ElementPtr sheet = Sheet::Element::Create(model, scale, height, width, name);
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
    viewController->GetViewDefinition().LookAtVolume(db.GeoLocation().GetProjectExtents());

    DgnDbStatus stat;
    viewController->GetViewDefinition().Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertDrawingCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Rank rank)
    {
    MUST_HAVE_HOST(DgnCategoryId());

    DrawingCategory category(db, categoryName, rank);
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

    SpatialCategory category(db, categoryName, rank);
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
DrawingViewDefinitionPtr DgnDbTestUtils::InsertDrawingView(DrawingModelR model, Utf8CP viewDescr)
    {
    auto& db = model.GetDgnDb();
    DrawingViewDefinitionPtr viewDef = new DrawingViewDefinition(db, model.GetName(), DrawingViewDefinition::QueryClassId(db), model.GetModelId(), *new CategorySelector(db,""), *new DisplayStyle(db,""));

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
    auto& db = model.GetDgnDb();
    CameraViewDefinition viewDef(db, viewName ? viewName : model.GetName(), *new CategorySelector(db,""), *new DisplayStyle3d(db,""), *new ModelSelector(db,""));

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
