/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as the OBJ produced from this
// WARNING:   source file is also used in DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/PlatformLib.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatformInternal/DgnCore/ElementGraphics.fb.h>
#include "flatbuffers/flatbuffers.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
using namespace flatbuffers;

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == PlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your SetUpTestCase function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::UpdateProjectExtents(DgnDbR db)
    {
    db.GeoLocation().InitializeProjectExtents();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertDrawingCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    return InsertDrawingCategory(db, categoryName, appearance, rank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::GetFirstDrawingCategoryId(DgnDbR db)
    {
    MUST_HAVE_HOST(DgnCategoryId());
    DgnCategoryId categoryId = (*DrawingCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    EXPECT_TRUE(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
* @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::EmptySubDirectory(WCharCP relPath)
    {
    BeFileName path = getOutputPath(relPath);
    BeFileName::EmptyDirectory(path.c_str());
    }

