/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Compatibility_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/FunctionalDomain.h>

USING_NAMESPACE_BENTLEY_DPTEST

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    04/2017
//========================================================================================
struct CompatibilityTests : public DgnDbTestFixture
{
    static constexpr Utf8CP GetSpatialCategoryName() {return "TestSpatialCategory";}
    static constexpr Utf8CP GetDrawingCategoryName() {return "TestDrawingCategory";}
    static constexpr Utf8CP GetPhysicalPartitionName() {return "Physical";}
    static constexpr Utf8CP GetDocumentPartitionName() {return "Document";}
    static constexpr Utf8CP GetDefinitionPartitionName() {return "Definition";}
    static constexpr double GetSheetScale() {return 1.0;}
    static DPoint2d GetSheetSize() {return DPoint2d::From(0.1, 0.3);}
    static Utf8String GetSubjectName(int index) {return Utf8PrintfString(BIS_CLASS_Subject "%" PRIi32, index);}
    static Utf8String GetGeometryPartName(int index) {return Utf8PrintfString(BIS_CLASS_GeometryPart "%" PRIi32, index);}
    static Utf8String GetCategorySelectorName(int index) {return Utf8PrintfString(BIS_CLASS_CategorySelector "%" PRIi32, index);}
    static Utf8String GetModelSelectorName(int index) {return Utf8PrintfString(BIS_CLASS_ModelSelector "%" PRIi32, index);}
    static Utf8String GetDisplayStyle3dName(int index) {return Utf8PrintfString(BIS_CLASS_DisplayStyle3d "%" PRIi32, index);}
    static Utf8String GetSpatialViewDefinitionName(int index) {return Utf8PrintfString(BIS_CLASS_SpatialViewDefinition "%" PRIi32, index);}
    static Utf8String GetPhysicalElementName(int index) {return Utf8PrintfString(BIS_CLASS_PhysicalElement "%" PRIi32, index);}
    static Utf8String GetSpatialLocationName(int index) {return Utf8PrintfString(BIS_CLASS_SpatialLocationElement "%" PRIi32, index);}
    static Utf8String GetDrawingName(int index) {return Utf8PrintfString(BIS_CLASS_Drawing "%" PRIi32, index);}
    static Utf8String GetDrawingGraphicName(int index) {return Utf8PrintfString(BIS_CLASS_DrawingGraphic "%" PRIi32, index);}
    static Utf8String GetSheetName(int index) {return Utf8PrintfString(BIS_CLASS_Sheet "%" PRIi32, index);}
    static Utf8String BuildWhereModelIdEquals(DgnModelId modelId) {return Utf8PrintfString("WHERE Model.Id=%" PRIi64, modelId.GetValue());}

    static void SetUpTestCase();
    void SetUpFromBaselineCopy(BeFileNameCR, Utf8CP, DbResult);
    void ImportFunctionalSchema();

    BE_JSON_NAME(inserted);
    BE_JSON_NAME(updated);

    void InsertHierarchy(SubjectCR, int);
    void InsertPhysicalHierarchy(SubjectCR);
    void InsertDocumentHierarchy(SubjectCR);
    void InsertDefinitionHierarchy(SubjectCR);
    void InsertSpatialCategory();
    void InsertDrawingCategory();
    DgnGeometryPartPtr InsertGeometryPart(DefinitionModelR, int);
    CategorySelectorPtr InsertCategorySelector(DefinitionModelR, DgnCategoryId, int);
    ModelSelectorPtr InsertModelSelector(DefinitionModelR, DgnModelId, int);
    DisplayStyle3dPtr InsertDisplayStyle3d(DefinitionModelR, int);
    SpatialViewDefinitionPtr InsertSpatialViewDefinition(DefinitionModelR, int, CategorySelectorR, DisplayStyle3dR, ModelSelectorR, DRange3dCR);
    void InsertPhysicalElement(PhysicalModelR, DgnCategoryId, int);
    void InsertSpatialLocation(PhysicalModelR, DgnCategoryId, int);
    DrawingPtr InsertDrawing(DocumentListModelR, int);
    DrawingModelPtr InsertDrawingModel(DrawingCR);
    void InsertDrawingGraphic(GraphicalModel2dR, DgnCategoryId, int);
    Sheet::ElementPtr InsertSheet(DocumentListModelR, int);
    Sheet::ModelPtr InsertSheetModel(Sheet::ElementCR);

    void ModifyHierarchy(SubjectCR, int);
    void ModifyPhysicalHierarchy(SubjectCR);
    void ModifyDocumentHierarchy(SubjectCR);
    void ModifyDefinitionHierarchy(SubjectCR);
    void ModifyGeometricElements(GeometricModelR, Utf8CP);
    void ModifyDocumentElements(DocumentListModelR, Utf8CP);
    void ModifyDefinitionElements(DefinitionModelR, Utf8CP);
    void ModifyElementCode(DgnDbR, DgnElementId);
    void ModifyGeometricElement(DgnDbR, DgnElementId);
    void ModifySubModel(DgnDbR, DgnElementId);
    void DeleteElementAndSubModel(DgnDbR, DgnElementId);

    DgnCategoryId GetSpatialCategoryId();
    DgnCategoryId GetDrawingCategoryId();
    PhysicalModelPtr GetPhysicalModel(SubjectCR);
    DocumentListModelPtr GetDocumentListModel(SubjectCR);
    DefinitionModelPtr GetDefinitionModel(SubjectCR);
};

//---------------------------------------------------------------------------------------
// NOTE: This unit test produces "CompatibilityTestSeed.bim" which is saved in the "CompatibilityTestFiles" product by each build and used for cross version/stream compatibility testing.
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, CompatibilityTestSeed)
    {
    SetupSeedProject();
    ImportFunctionalSchema();
    InsertSpatialCategory();
    InsertDrawingCategory();

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(BentleyStatus::SUCCESS, db.Schemas().CreateClassViewsInDb());
    ASSERT_EQ(1, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size()) << "Expected just the root Subject";
    InsertHierarchy(*db.Elements().GetRootSubject(), 1);
    }

//---------------------------------------------------------------------------------------
// This unit test ensures that the "Modify" and "Insert" tests work with a matching combination of DgnPlatform and DgnDb file format.
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, ModifyCurrent)
    {
    SetupSeedProject();
    InsertSpatialCategory();
    InsertDrawingCategory();

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(1, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size()) << "Expected just the root Subject";
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    InsertHierarchy(*rootSubject, 1);
    ModifyHierarchy(*rootSubject, 1);
    InsertHierarchy(*rootSubject, 2);
    }

//---------------------------------------------------------------------------------------
// This unit test runs the "Modify" and "Insert" tests using the current DgnPlatform against saved baselines of the DgnDb file format.
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, DISABLED_ModifyBaseline_1_0_0)
    {
    SetUpFromBaselineCopy(BeFileName(L"d:/data/dgndb/Baseline/BisCore-1.0.0-PreHoldouts.bim"), TEST_NAME, BE_SQLITE_ERROR_SchemaImportRequired);

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size());
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    ModifyHierarchy(*rootSubject, 1);
    InsertHierarchy(*rootSubject, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertHierarchy(SubjectCR parentSubject, int subjectNumber)
    {
    SubjectCPtr subject = Subject::CreateAndInsert(parentSubject, GetSubjectName(subjectNumber));
    ASSERT_TRUE(subject.IsValid());

    InsertPhysicalHierarchy(*subject);
    InsertDocumentHierarchy(*subject);
    InsertDefinitionHierarchy(*subject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyHierarchy(SubjectCR parentSubject, int subjectNumber)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode subjectCode = Subject::CreateCode(parentSubject, GetSubjectName(subjectNumber));
    DgnElementId subjectId = db.Elements().QueryElementIdByCode(subjectCode);
    SubjectCPtr subject = db.Elements().Get<Subject>(subjectId);
    ASSERT_TRUE(subject.IsValid());

    ModifyPhysicalHierarchy(*subject);
    ModifyDocumentHierarchy(*subject);
    ModifyDefinitionHierarchy(*subject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertSpatialCategory()
    {
    DgnDbTestUtils::InsertSpatialCategory(GetDgnDb(), GetSpatialCategoryName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertDrawingCategory()
    {
    DgnDbTestUtils::InsertDrawingCategory(GetDgnDb(), GetDrawingCategoryName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DgnCategoryId CompatibilityTests::GetSpatialCategoryId()
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(GetDgnDb(), SpatialCategory::CreateCode(GetDgnDb(), GetSpatialCategoryName()));
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DgnCategoryId CompatibilityTests::GetDrawingCategoryId()
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(GetDgnDb(), DrawingCategory::CreateCode(GetDgnDb().GetDictionaryModel(), GetDrawingCategoryName()));
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertDefinitionHierarchy(SubjectCR subject)
    {
    DefinitionPartitionCPtr partition = DefinitionPartition::CreateAndInsert(subject, GetDefinitionPartitionName());
    ASSERT_TRUE(partition.IsValid());
    DefinitionModelPtr definitionModel = DefinitionModel::CreateAndInsert(*partition);
    ASSERT_TRUE(definitionModel.IsValid());
    DgnCategoryId categoryId = GetSpatialCategoryId();
    PhysicalModelPtr physicalModel = GetPhysicalModel(subject);

    for (int i=0; i<3; i++)
        {
        DgnGeometryPartPtr geometryPart = InsertGeometryPart(*definitionModel, i);
        CategorySelectorPtr categorySelector = InsertCategorySelector(*definitionModel, categoryId, i);
        ModelSelectorPtr modelSelector = InsertModelSelector(*definitionModel, physicalModel->GetModelId(), i);
        DisplayStyle3dPtr displayStyle = InsertDisplayStyle3d(*definitionModel, i);
        DRange3d volume = physicalModel->QueryModelRange();
        ASSERT_TRUE(geometryPart.IsValid());
        ASSERT_TRUE(categorySelector.IsValid());
        ASSERT_TRUE(modelSelector.IsValid());
        ASSERT_TRUE(displayStyle.IsValid());
        ASSERT_FALSE(volume.IsNull());

        SpatialViewDefinitionPtr spatialViewDefinition = InsertSpatialViewDefinition(*definitionModel, i, *categorySelector, *displayStyle, *modelSelector, volume);
        ASSERT_TRUE(spatialViewDefinition.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
CategorySelectorPtr CompatibilityTests::InsertCategorySelector(DefinitionModelR model, DgnCategoryId categoryId, int index)
    {
    CategorySelectorPtr categorySelector = new CategorySelector(model, GetCategorySelectorName(index));
    categorySelector->AddCategory(categoryId);
    categorySelector->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    return categorySelector->Insert().IsValid() ? categorySelector : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
ModelSelectorPtr CompatibilityTests::InsertModelSelector(DefinitionModelR model, DgnModelId selectedModelId, int index)
    {
    ModelSelectorPtr modelSelector = new ModelSelector(model, GetModelSelectorName(index));
    modelSelector->AddModel(selectedModelId);
    modelSelector->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    return modelSelector->Insert().IsValid() ? modelSelector : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr CompatibilityTests::InsertDisplayStyle3d(DefinitionModelR model, int index)
    {
    DisplayStyle3dPtr displayStyle = new DisplayStyle3d(model, GetDisplayStyle3dName(index));
    displayStyle->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    return displayStyle->Insert().IsValid() ? displayStyle : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
SpatialViewDefinitionPtr CompatibilityTests::InsertSpatialViewDefinition(DefinitionModelR model, int index, CategorySelectorR categorySelector, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector, DRange3dCR volume)
    {
    SpatialViewDefinitionPtr viewDefinition = new OrthographicViewDefinition(model, GetSpatialViewDefinitionName(index), categorySelector, displayStyle, modelSelector);
    viewDefinition->SetStandardViewRotation(StandardView::Iso);
    viewDefinition->LookAtVolume(volume);
    viewDefinition->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    return viewDefinition->Insert().IsValid() ? viewDefinition : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr CompatibilityTests::InsertGeometryPart(DefinitionModelR model, int index)
    {
    DgnGeometryPartPtr geometryPart = DgnGeometryPart::Create(model, GetGeometryPartName(index));
    GeometryBuilderPtr geometryPartBuilder = GeometryBuilder::CreateGeometryPart(model.GetDgnDb(), true /*is3d*/);
    BeAssert(geometryPart.IsValid() && geometryPartBuilder.IsValid());
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(1+index, 1+index, 1+index), true));
    BeAssert(geometry.IsValid());
    geometryPartBuilder->Append(*geometry);
    geometryPartBuilder->Finish(*geometryPart);
    geometryPart->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    return geometryPart->Insert().IsValid() ? geometryPart : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyDefinitionHierarchy(SubjectCR subject)
    {
    DefinitionModelPtr model = GetDefinitionModel(subject);
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_GeometryPart));
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_CategorySelector));
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_ModelSelector));
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_DisplayStyle3d));
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition));

    DgnDbR db = subject.GetDgnDb();
    Utf8String whereClause = BuildWhereModelIdEquals(model->GetModelId());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_CategorySelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_ModelSelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DisplayStyle3d), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometryPart), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition), whereClause.c_str()).BuildIdList<DgnElementId>().size());

    DgnGeometryPartPtr geometryPart = InsertGeometryPart(*model, 3);
    CategorySelectorPtr categorySelector = InsertCategorySelector(*model, GetSpatialCategoryId(), 3);
    ModelSelectorPtr modelSelector = InsertModelSelector(*model, GetPhysicalModel(subject)->GetModelId(), 3);
    DisplayStyle3dPtr displayStyle = InsertDisplayStyle3d(*model, 3);
    ASSERT_TRUE(geometryPart.IsValid());
    ASSERT_TRUE(categorySelector.IsValid());
    ASSERT_TRUE(modelSelector.IsValid());
    ASSERT_TRUE(displayStyle.IsValid());
    
    SpatialViewDefinitionPtr spatialViewDefinition = InsertSpatialViewDefinition(*model, 3, *categorySelector, *displayStyle, *modelSelector, GetPhysicalModel(subject)->QueryModelRange());
    ASSERT_TRUE(spatialViewDefinition.IsValid());

    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_CategorySelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_ModelSelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DisplayStyle3d), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometryPart), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyDefinitionElements(DefinitionModelR model, Utf8CP className)
    {
    DgnDbR db = model.GetDgnDb();
    int i=0;
    Utf8String whereClause = BuildWhereModelIdEquals(model.GetModelId());
    for (ElementIteratorEntryCR elementEntry : db.Elements().MakeIterator(className, whereClause.c_str()))
        {
        switch (i++)
            {
            case 0: // skip first element
                break;

            case 1: // update second element
                ModifyElementCode(db, elementEntry.GetElementId());
                break;

            case 2: // delete third element
                ASSERT_EQ(DgnDbStatus::Success, db.Elements().Delete(elementEntry.GetElementId()));
                break;

            default: // should only be 3 elements
                ASSERT_TRUE(false);
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyElementCode(DgnDbR db, DgnElementId elementId)
    {
    DgnElementPtr element = db.Elements().GetForEdit<DgnElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    ASSERT_FALSE(element->GetUserProperties(json_inserted()).isNull());
    DgnCode oldCode = element->GetCode();
    DgnCode newCode(oldCode.GetCodeSpecId(), oldCode.GetValue() + "Updated", oldCode.GetScope());
    element->SetCode(newCode);
    element->SetUserProperties(json_updated(), DateTime::GetCurrentTime().ToString());
    ASSERT_TRUE(element->Update().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DefinitionModelPtr CompatibilityTests::GetDefinitionModel(SubjectCR subject)
    {
    DgnDbR db = subject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(subject, GetDefinitionPartitionName());
    DgnModelId modelId = db.Models().QuerySubModelId(partitionCode);
    DefinitionModelPtr model = db.Models().Get<DefinitionModel>(modelId);
    BeAssert(model.IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertPhysicalHierarchy(SubjectCR subject)
    {
    PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(subject, GetPhysicalPartitionName());
    ASSERT_TRUE(partition.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    DgnCategoryId categoryId = GetSpatialCategoryId();

    for (int i=0; i<3; i++)
        {
        InsertPhysicalElement(*model, categoryId, i);
        InsertSpatialLocation(*model, categoryId, i);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int index)
    {
    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
    ASSERT_TRUE(element.IsValid());
    element->SetUserLabel(GetPhysicalElementName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint3d::From(index, index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnSphereDetail(DPoint3d::FromZero(), 0.25));
    ASSERT_TRUE(geometryBuilder.IsValid() && geometry.IsValid());
    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    ASSERT_TRUE(element->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertSpatialLocation(PhysicalModelR model, DgnCategoryId categoryId, int index)
    {
    GenericSpatialLocationPtr element = GenericSpatialLocation::Create(model, categoryId);
    ASSERT_TRUE(element.IsValid());
    element->SetUserLabel(GetSpatialLocationName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint3d::From(index, index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(0.5, 0.5, 0.5), true));
    ASSERT_TRUE(geometryBuilder.IsValid() && geometry.IsValid());
    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    ASSERT_TRUE(element->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyPhysicalHierarchy(SubjectCR subject)
    {
    PhysicalModelPtr model = GetPhysicalModel(subject);
    ModifyGeometricElements(*model, BIS_SCHEMA(BIS_CLASS_PhysicalElement));
    ModifyGeometricElements(*model, BIS_SCHEMA(BIS_CLASS_SpatialLocationElement));

    DgnDbR db = subject.GetDgnDb();
    Utf8String whereClause = BuildWhereModelIdEquals(model->GetModelId());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialLocationElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    
    InsertPhysicalElement(*model, GetSpatialCategoryId(), 3);
    InsertSpatialLocation(*model, GetSpatialCategoryId(), 3);
    
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialLocationElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyGeometricElements(GeometricModelR model, Utf8CP className)
    {
    DgnDbR db = model.GetDgnDb();
    int i=0;
    Utf8String whereClause = BuildWhereModelIdEquals(model.GetModelId());
    for (ElementIteratorEntryCR elementEntry : db.Elements().MakeIterator(className, whereClause.c_str()))
        {
        switch (i++)
            {
            case 0: // skip first element
                break;

            case 1: // update second element
                ModifyGeometricElement(db, elementEntry.GetElementId());
                break;

            case 2: // delete third element
                ASSERT_EQ(DgnDbStatus::Success, db.Elements().Delete(elementEntry.GetElementId()));
                break;

            default: // should only be 3 elements
                ASSERT_TRUE(false);
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyGeometricElement(DgnDbR db, DgnElementId elementId)
    {
    GeometricElementPtr element = db.Elements().GetForEdit<GeometricElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    ASSERT_TRUE(element->GetFederationGuid().IsValid());
    Utf8String userLabel(element->GetUserLabel());
    userLabel.append("Updated");
    element->SetUserLabel(userLabel.c_str());
    ASSERT_TRUE(element->Update().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr CompatibilityTests::GetPhysicalModel(SubjectCR subject)
    {
    DgnDbR db = subject.GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(subject, GetPhysicalPartitionName());
    DgnModelId modelId = db.Models().QuerySubModelId(partitionCode);
    PhysicalModelPtr model = db.Models().Get<PhysicalModel>(modelId);
    BeAssert(model.IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertDocumentHierarchy(SubjectCR subject)
    {
    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(subject, GetDocumentPartitionName());
    ASSERT_TRUE(partition.IsValid());
    DocumentListModelPtr model = DocumentListModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());

    for (int i=0; i<3; i++)
        {
        DrawingPtr drawing = InsertDrawing(*model, i);
        Sheet::ElementPtr sheet = InsertSheet(*model, i);
        ASSERT_TRUE(drawing.IsValid());
        ASSERT_TRUE(sheet.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DrawingPtr CompatibilityTests::InsertDrawing(DocumentListModelR model, int index)
    {
    DrawingPtr drawing = Drawing::Create(model, GetDrawingName(index));
    drawing->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    if (!drawing->Insert().IsValid())
        return nullptr;

    DrawingModelPtr drawingModel = InsertDrawingModel(*drawing);
    if (!drawingModel.IsValid())
        return nullptr;

    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr CompatibilityTests::InsertDrawingModel(DrawingCR drawing)
    {
    DrawingModelPtr model = DrawingModel::Create(drawing);
    BeAssert(model.IsValid());
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;

    DgnCategoryId categoryId = GetDrawingCategoryId();
    for (int i=0; i<3; i++)
        InsertDrawingGraphic(*model, categoryId, i);

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertDrawingGraphic(GraphicalModel2dR model, DgnCategoryId categoryId, int index)
    {
    DrawingGraphicPtr element = DrawingGraphic::Create(model, categoryId);
    ASSERT_TRUE(element.IsValid());
    element->SetUserLabel(GetDrawingGraphicName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint2d::From(index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(ICurvePrimitive::CreateRectangle(0, 0, 1+index, 1+index, 0));
    ASSERT_TRUE(geometryBuilder.IsValid() && geometry.IsValid());
    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    ASSERT_TRUE(element->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
Sheet::ElementPtr CompatibilityTests::InsertSheet(DocumentListModelR model, int index)
    {
    Sheet::ElementPtr sheet = Sheet::Element::Create(model, GetSheetScale(), GetSheetSize(), GetSheetName(index).c_str());
    sheet->SetUserProperties(json_inserted(), DateTime::GetCurrentTime().ToString());
    if (!sheet->Insert().IsValid())
        return nullptr;

    Sheet::ModelPtr sheetModel = InsertSheetModel(*sheet);
    if (!sheetModel.IsValid())
        return nullptr;

    return sheet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
Sheet::ModelPtr CompatibilityTests::InsertSheetModel(Sheet::ElementCR sheet)
    {
    Sheet::ModelPtr model = Sheet::Model::Create(sheet);
    BeAssert(model.IsValid());
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;

    DgnCategoryId categoryId = GetDrawingCategoryId();
    for (int i=0; i<3; i++)
        InsertDrawingGraphic(*model, categoryId, i);

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyDocumentHierarchy(SubjectCR subject)
    {
    DocumentListModelPtr model = GetDocumentListModel(subject);
    ModifyDocumentElements(*model, BIS_SCHEMA(BIS_CLASS_Drawing));
    ModifyDocumentElements(*model, BIS_SCHEMA(BIS_CLASS_Sheet));

    DgnDbR db = subject.GetDgnDb();
    Utf8String whereClause = BuildWhereModelIdEquals(model->GetModelId());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Drawing), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Sheet), whereClause.c_str()).BuildIdList<DgnElementId>().size());

    InsertDrawing(*model, 3);
    InsertSheet(*model, 3);

    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Drawing), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Sheet), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyDocumentElements(DocumentListModelR model, Utf8CP className)
    {
    DgnDbR db = model.GetDgnDb();
    int i=0;
    Utf8String whereClause = BuildWhereModelIdEquals(model.GetModelId());
    for (ElementIteratorEntryCR elementEntry : db.Elements().MakeIterator(className, whereClause.c_str()))
        {
        switch (i++)
            {
            case 0: // modify sub-model
                ModifySubModel(db, elementEntry.GetElementId());
                break;

            case 1: // update second element
                ModifyElementCode(db, elementEntry.GetElementId());
                break;

            case 2: // delete third element
                DeleteElementAndSubModel(db, elementEntry.GetElementId()); 
                break;

            default: // should only be 3 elements
                ASSERT_TRUE(false);
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifySubModel(DgnDbR db, DgnElementId elementId)
    {
    DgnElementPtr element = db.Elements().GetForEdit<DgnElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    GraphicalModel2dPtr subModel = element->GetSub<GraphicalModel2d>();
    ASSERT_TRUE(subModel.IsValid());
    ModifyGeometricElements(*subModel, BIS_SCHEMA(BIS_CLASS_DrawingGraphic));

    Utf8String whereClause = BuildWhereModelIdEquals(subModel->GetModelId());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DrawingGraphic), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    InsertDrawingGraphic(*subModel, GetDrawingCategoryId(), 3);
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DrawingGraphic), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::DeleteElementAndSubModel(DgnDbR db, DgnElementId elementId)
    {
    DgnElementPtr element = db.Elements().GetForEdit<DgnElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    GeometricModelPtr subModel = element->GetSub<GeometricModel>();
    ASSERT_TRUE(subModel.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, subModel->Delete());
    ASSERT_EQ(DgnDbStatus::Success, db.Elements().Delete(elementId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
DocumentListModelPtr CompatibilityTests::GetDocumentListModel(SubjectCR subject)
    {
    DgnDbR db = subject.GetDgnDb();
    DgnCode partitionCode = DocumentPartition::CreateCode(subject, GetDocumentPartitionName());
    DgnModelId modelId = db.Models().QuerySubModelId(partitionCode);
    DocumentListModelPtr model = db.Models().Get<DocumentListModel>(modelId);
    BeAssert(model.IsValid());
    return model;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ImportFunctionalSchema()
    {
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_EQ(BE_SQLITE_OK, FunctionalDomain::GetDomain().ImportSchema(GetDgnDb()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::SetUpTestCase()
    {
    ScopedDgnHost host;
    DgnPlatformSeedManager::SeedDbOptions seedDbOptions(false, false); // don't want the DgnPlatformTest schema to be part of the compatibility test
    DgnDbTestFixture::s_seedFileInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, seedDbOptions); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::SetUpFromBaselineCopy(BeFileNameCR sourceFileName, Utf8CP destBaseName, DbResult expectedFirstOpenStatus)
    {
    BeFileName destFileName;
    BeTest::GetHost().GetOutputRoot(destFileName);
    destFileName.AppendToPath(BeFileName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8));
    destFileName.AppendToPath(BeFileName(destBaseName, BentleyCharEncoding::Utf8));
    destFileName.AppendExtension(L"bim");
    ASSERT_FALSE(destFileName.DoesPathExist());
    ASSERT_TRUE(sourceFileName.DoesPathExist());

    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(sourceFileName, destFileName, true /*failIfFileExists*/);
    ASSERT_EQ(BeFileNameStatus::Success, copyStatus);

    DbResult openStatus;
    DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite);

    if (BE_SQLITE_OK != expectedFirstOpenStatus)
        {
        m_db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
        ASSERT_EQ(expectedFirstOpenStatus, openStatus);
        ASSERT_FALSE(m_db.IsValid());
        openParams.SetEnableSchemaImport(DgnDb::OpenParams::EnableSchemaImport::Yes);
        }

    m_db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, openStatus);
    ASSERT_TRUE(m_db.IsValid());
    }
