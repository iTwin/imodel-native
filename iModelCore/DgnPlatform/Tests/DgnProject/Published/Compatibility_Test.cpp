/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/FunctionalDomain.h>

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_EC

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    04/2017
//========================================================================================
struct CompatibilityTests : public DgnDbTestFixture
{
    static constexpr Utf8CP GetCodeSpecName() {return "CompatibilityTests.CodeSpec";}
    static constexpr Utf8CP GetSpatialCategoryName() {return "TestSpatialCategory";}
    static constexpr Utf8CP GetDrawingCategoryName() {return "TestDrawingCategory";}
    static constexpr Utf8CP GetGroupInformationPartitionName() {return "GroupInformation";}
    static constexpr Utf8CP GetPhysicalPartitionName() {return "Physical";}
    static constexpr Utf8CP GetDocumentPartitionName() {return "Document";}
    static constexpr Utf8CP GetDefinitionPartitionName() {return "Definition";}
    static constexpr Utf8CP GetPhysicalElementGroupName() {return "PhysicalElementGroup";}
    static constexpr Utf8CP GetSpatialLocationGroupName() {return "SpatialLocationGroup";}
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
    static Utf8String BuildWhereModelIdEquals(DgnModelId modelId) {return Utf8PrintfString("WHERE Model.Id=%" PRIu64, modelId.GetValue());}

    static void SetUpTestCase();
    void SetUpFromBaselineCopy(Utf8CP, Utf8CP, DbResult);
    void ImportFunctionalSchema();

    BE_JSON_NAME(inserted);
    BE_JSON_NAME(updated);

    void InsertHierarchy(SubjectCR, int);
    void InsertGroupInformationHierarchy(SubjectCR);
    void InsertPhysicalHierarchy(SubjectCR);
    void InsertDocumentHierarchy(SubjectCR);
    void InsertDefinitionHierarchy(SubjectCR);
    void InsertCodeSpec();
    void InsertSpatialCategory();
    void InsertDrawingCategory();
    DgnGeometryPartPtr InsertGeometryPart(DefinitionModelR, int);
    CategorySelectorPtr InsertCategorySelector(DefinitionModelR, DgnCategoryId, int);
    ModelSelectorPtr InsertModelSelector(DefinitionModelR, DgnModelId, int);
    DisplayStyle3dPtr InsertDisplayStyle3d(DefinitionModelR, int);
    SpatialViewDefinitionPtr InsertSpatialViewDefinition(DefinitionModelR, int, CategorySelectorR, DisplayStyle3dR, ModelSelectorR, DRange3dCR);
    PhysicalElementPtr InsertPhysicalElement(PhysicalModelR, DgnCategoryId, int);
    SpatialLocationElementPtr InsertSpatialLocation(PhysicalModelR, DgnCategoryId, int);
    DrawingPtr InsertDrawing(DocumentListModelR, int);
    DrawingModelPtr InsertDrawingModel(DrawingCR);
    DrawingGraphicPtr InsertDrawingGraphic(GraphicalModel2dR, DgnCategoryId, int);
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
    CodeSpecCPtr GetCodeSpec();
    GenericGroupModelPtr GetGroupModel(SubjectCR);
    GenericGroupCPtr GetGroup(DgnDbR, DgnCodeCR);
    DgnCode GetPhysicalElementGroupCode(GroupInformationModelCR model) {return GetCodeSpec()->CreateCode(model, GetPhysicalElementGroupName());}
    DgnCode GetSpatialLocationGroupCode(GroupInformationModelCR model) {return GetCodeSpec()->CreateCode(model, GetSpatialLocationGroupName());}
    GenericGroupCPtr GetPhysicalElementGroup(SubjectCR);
    GenericGroupCPtr GetSpatialLocationGroup(SubjectCR);
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
    InsertCodeSpec();
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
    InsertCodeSpec();
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
TEST_F(CompatibilityTests, ModifyBaseline)
    {
    SetUpFromBaselineCopy("2-0-1-60", TEST_NAME, BE_SQLITE_OK);

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

    InsertGroupInformationHierarchy(*subject);
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
void CompatibilityTests::InsertCodeSpec()
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(GetDgnDb(), GetCodeSpecName(), CodeScopeSpec::CreateModelScope());
    ASSERT_TRUE(codeSpec.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, codeSpec->Insert());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
CodeSpecCPtr CompatibilityTests::GetCodeSpec()
    {
    CodeSpecCPtr codeSpec = GetDgnDb().CodeSpecs().GetCodeSpec(GetCodeSpecName());
    BeAssert(codeSpec.IsValid());
    return codeSpec;
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
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(GetDgnDb().GetDictionaryModel(), GetSpatialCategoryName());
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
    DgnCode newCode(oldCode.GetCodeSpecId(), oldCode.GetScopeElementId(db), oldCode.GetValueUtf8() + "Updated");
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
    GenericGroupCPtr physicalElementGroup = GetPhysicalElementGroup(subject);
    GenericGroupCPtr spatialLocationGroup = GetSpatialLocationGroup(subject);

    for (int i=0; i<3; i++)
        {
        PhysicalElementPtr physicalElement = InsertPhysicalElement(*model, categoryId, i);
        ASSERT_TRUE(physicalElement.IsValid());
        physicalElementGroup->AddMember(*physicalElement);

        SpatialLocationElementPtr spatialLocation = InsertSpatialLocation(*model, categoryId, i);
        ASSERT_TRUE(spatialLocation.IsValid());
        spatialLocationGroup->AddMember(*spatialLocation);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr CompatibilityTests::InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int index)
    {
    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    element->SetUserLabel(GetPhysicalElementName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint3d::From(index, index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnSphereDetail(DPoint3d::FromZero(), 0.25));
    if (!geometryBuilder.IsValid() || !geometry.IsValid())
        return nullptr;

    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    return element->Insert().IsValid() ? element.get() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
SpatialLocationElementPtr CompatibilityTests::InsertSpatialLocation(PhysicalModelR model, DgnCategoryId categoryId, int index)
    {
    GenericSpatialLocationPtr element = GenericSpatialLocation::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    element->SetUserLabel(GetSpatialLocationName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint3d::From(index, index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(0.5, 0.5, 0.5), true));
    if (!geometryBuilder.IsValid() || !geometry.IsValid())
        return nullptr;

    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    return element->Insert().IsValid() ? element.get() : nullptr;
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
DrawingGraphicPtr CompatibilityTests::InsertDrawingGraphic(GraphicalModel2dR model, DgnCategoryId categoryId, int index)
    {
    DrawingGraphicPtr element = DrawingGraphic::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    element->SetUserLabel(GetDrawingGraphicName(index).c_str());
    element->SetFederationGuid(BeGuid(true));
    GeometryBuilderPtr geometryBuilder = GeometryBuilder::Create(model, categoryId, DPoint2d::From(index, index));
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(ICurvePrimitive::CreateRectangle(0, 0, 1+index, 1+index, 0));
    if (!geometryBuilder.IsValid() || !geometry.IsValid())
        return nullptr;

    geometryBuilder->Append(*geometry);
    geometryBuilder->Finish(*element);
    return element->Insert().IsValid() ? element : nullptr;
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
void CompatibilityTests::InsertGroupInformationHierarchy(SubjectCR subject)
    {
    GroupInformationPartitionCPtr partition = GroupInformationPartition::CreateAndInsert(subject, GetGroupInformationPartitionName());
    ASSERT_TRUE(partition.IsValid());
    GenericGroupModelPtr groupModel = GenericGroupModel::CreateAndInsert(*partition);
    ASSERT_TRUE(groupModel.IsValid());

    GenericGroupPtr physicalElementGroup = GenericGroup::Create(*groupModel, GetPhysicalElementGroupCode(*groupModel));
    ASSERT_TRUE(physicalElementGroup.IsValid() && physicalElementGroup->Insert().IsValid());

    GenericGroupPtr spatialLocationGroup = GenericGroup::Create(*groupModel, GetSpatialLocationGroupCode(*groupModel));
    ASSERT_TRUE(spatialLocationGroup.IsValid() && spatialLocationGroup->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
GenericGroupModelPtr CompatibilityTests::GetGroupModel(SubjectCR subject)
    {
    DgnDbR db = subject.GetDgnDb();
    DgnCode partitionCode = GroupInformationPartition::CreateCode(subject, GetGroupInformationPartitionName());
    DgnModelId modelId = db.Models().QuerySubModelId(partitionCode);
    GenericGroupModelPtr model = db.Models().Get<GenericGroupModel>(modelId);
    BeAssert(model.IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
GenericGroupCPtr CompatibilityTests::GetGroup(DgnDbR db, DgnCodeCR code)
    {
    DgnElementId groupId = db.Elements().QueryElementIdByCode(code);
    GenericGroupCPtr group = db.Elements().Get<GenericGroup>(groupId);
    BeAssert(group.IsValid());
    return group;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
GenericGroupCPtr CompatibilityTests::GetPhysicalElementGroup(SubjectCR subject)
    {
    return GetGroup(subject.GetDgnDb(), GetPhysicalElementGroupCode(*GetGroupModel(subject)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
GenericGroupCPtr CompatibilityTests::GetSpatialLocationGroup(SubjectCR subject)
    {
    return GetGroup(subject.GetDgnDb(), GetSpatialLocationGroupCode(*GetGroupModel(subject)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ImportFunctionalSchema()
    {
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_EQ(SchemaStatus::Success, FunctionalDomain::GetDomain().ImportSchema(GetDgnDb()));
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
void CompatibilityTests::SetUpFromBaselineCopy(Utf8CP versionString, Utf8CP destBaseName, DbResult expectedFirstOpenStatus)
    {
    BeFileName sourceFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sourceFileName);
    sourceFileName.AppendToPath(L"CompatibilityTestFiles");
    sourceFileName.AppendToPath(BeFileName(versionString, BentleyCharEncoding::Utf8));
    sourceFileName.AppendToPath(L"CompatibilityTestSeed.bim");
    ASSERT_TRUE(sourceFileName.DoesPathExist());

    BeFileName destFileName;
    BeTest::GetHost().GetOutputRoot(destFileName);
    destFileName.AppendToPath(BeFileName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8));
    BeFileName::CreateNewDirectory(destFileName.GetName());
    ASSERT_TRUE(destFileName.DoesPathExist());
    destFileName.AppendToPath(BeFileName(Utf8PrintfString("%s%s", destBaseName, versionString).c_str(), BentleyCharEncoding::Utf8));
    destFileName.AppendExtension(L"bim");
    
    // Did a previous test run leave the file?
    if (destFileName.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(destFileName.c_str()));

    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(sourceFileName, destFileName, true /*failIfFileExists*/);
    ASSERT_EQ(BeFileNameStatus::Success, copyStatus);

    DbResult openStatus = BE_SQLITE_OK;

    if (BE_SQLITE_OK != expectedFirstOpenStatus)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, destFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        ASSERT_EQ(expectedFirstOpenStatus, openStatus);
        ASSERT_FALSE(db.IsValid());
        }

    if (BE_SQLITE_ERROR_SchemaUpgradeRequired == openStatus)
        {
        DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade));
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
        ASSERT_EQ(BE_SQLITE_OK, openStatus);
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges("SchemaUpgrade"));
        db->CloseDb();
        }

    DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck));
    m_db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, openStatus);
    ASSERT_TRUE(m_db.IsValid());
    }

//========================================================================================
// @bsiclass                           Maha Nasir                               04/2017
//========================================================================================
struct ECInstancesCompatibility : public DgnDbTestFixture
    {
    std::vector<ECClassCP> List;
    std::vector<ECClassCP> ValidClassesForInstanceInsertion;
    DrawingModelPtr drawingModel;

    std::vector<ECClassCP> getDerivedClasses(ECClassCP classToTraverse);
    void InsertInstancesForGeometricElement2d(ECClassCP className);
    void InsertInstancesForGeometricElement3d(ECClassCP className);
    void InsertInstancesForGeometricElementHeirarchy(ECClassCP className);
    void InsertInstancesForDocument(ECClassCP className);
    void InsertInstancesForInformationReferenceElement(ECClassCP className);
    void InsertInstancesForDefinitionElement(ECClassCP className);
    void InsertInstancesForInformationPartitionElement(ECClassCP className);
    void InsertInstancesForInformationContentElementHeirarchy(ECClassCP className);
    DgnElementPtr createElement(ECN::StandaloneECInstancePtr instance);
    std::vector<ECClassCP> GetValidClassesForInstanceInsertion(DgnDbR db);
    void SetUpDbFromBaselineCopy(Utf8CP, Utf8CP, DbResult);
    };

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                            Maha.Nasir                04/17
    ! Returns a vector over all the derived classes of the specified class.
    +---------------+---------------+---------------+---------------+--------------+---*/
    std::vector<ECClassCP> ECInstancesCompatibility::getDerivedClasses(ECClassCP classToTraverse)
        {
        const ECDerivedClassesList& DerivedClasses = classToTraverse->GetDerivedClasses();

        for (ECClassP Class : DerivedClasses)
            {
            if (Class->GetName() != BIS_CLASS_Category && Class->GetName() != BIS_CLASS_Texture  && Class->GetName() != BIS_CLASS_ViewDefinition && Class->GetName() != BIS_CLASS_SubCategory && Class->GetName() != BIS_CLASS_GeometryPart && Class->GetName() != BIS_CLASS_InformationPartitionElement)
                {
                List.push_back(Class);
                if (Class != nullptr)
                    {
                    getDerivedClasses(Class);
                    }
                }
            }
        return List;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                            Maha.Nasir                04/17
    ! Creates the elemnt from the supplied instance.
    +---------------+---------------+---------------+---------------+--------------+---*/
    DgnElementPtr ECInstancesCompatibility::createElement(ECN::StandaloneECInstancePtr instance)
        {
        DgnElementPtr element = m_db->Elements().CreateElement(*instance);
        EXPECT_TRUE(element != nullptr);
        return element;
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts the instances(For only BisCore schema) for GeometricElement2d class heirarchy.
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForGeometricElement2d(ECClassCP className)
        {
        //Emptying vector
        List.clear();
        ASSERT_TRUE(List.empty());

        printf("\nInserting Instances for GeometricElement2d heirarchy: \n");

        //Inserting category
        DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestCategory");
        ASSERT_TRUE(categoryId.IsValid());

        //Inserting sheet Model
        DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "SheetListModel");
        auto sheet = DgnDbTestUtils::InsertSheet(*sheetListModel, 1.0, 1.0, 1.0, "MySheet");
        auto sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);
        DgnModelId m_sheetModelId = sheetModel->GetModelId();

        // Creating view of the sheet model
        DefinitionModelR dictionary = m_db->GetDictionaryModel();
        DrawingViewDefinition view(dictionary, "MyDrawingView", drawingModel->GetModelId(), *new CategorySelector(dictionary, ""), *new DisplayStyle2d(dictionary, ""));
        view.Insert();
        DgnViewId m_viewId = view.GetViewId();
        ASSERT_TRUE(m_viewId.IsValid());

        //Getting GeometricElement2d heirarchy
        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                if (className == BIS_CLASS_ViewAttachment)
                    {
                    ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(m_sheetModelId)));
                    ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("View", ECN::ECValue(m_viewId)));
                    }
                else
                    {
                    ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(drawingModel->GetModelId())));
                    }

                DgnCode code = DgnCode::CreateEmpty();
                ASSERT_EQ(ECN::ECObjectsStatus::Success, ClassInstance->SetValue("Category", ECN::ECValue(categoryId)));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));

                //Creating Element of the specified instance
                DgnElementPtr ele= createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("\nInstance inserted for class:%s", ecClass->GetName().c_str());
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts the instances(For only BisCore schema) for GeometricElement3d class heirarchy.
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForGeometricElement3d(ECClassCP className)
        {
        //Emptying vector
        List.clear();
        ASSERT_TRUE(List.empty());

        printf("\n\nInserting Instances for GeometricElement3d heirarchy: \n\n");

        //Getting the heorarchy of GeometricElement3d
        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                //Setting values for Model and Code
                DgnCode code = DgnCode::CreateEmpty();
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(m_defaultModelId)));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));
                ASSERT_EQ(ECN::ECObjectsStatus::Success, ClassInstance->SetValue("Category", ECN::ECValue(m_defaultCategoryId)));

                //Creating Element of the specified instance
                DgnElementPtr ele = createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("Instance Inserted for Class: %s \n", ecClass->GetName().c_str());
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts the instances(For only BisCore schema classes) of GeometricElement class heirarchy.
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForGeometricElementHeirarchy(ECClassCP className)
        {
        //Getting the immediate derived classes of GeometricElement
        const ECDerivedClassesList& GeometricElementHeirarchy = className->GetDerivedClasses();

        //Traversing through the heirarchy
        for (ECClassCP ecClass : GeometricElementHeirarchy)
            {
            if (ecClass->GetName() == BIS_CLASS_GeometricElement2d)
                {
                InsertInstancesForGeometricElement2d(ecClass);
                }
            else if (ecClass->GetName() == BIS_CLASS_GeometricElement3d)
                {
                InsertInstancesForGeometricElement3d(ecClass);
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts instances for the Document class heirarchy
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForDocument(ECClassCP className)
        {
        printf("\n\nInserting Instances for Document heirarchy:\n\n");

        List.clear();
        ASSERT_TRUE(List.empty());

        //Getting the heirarchy of Document class
        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                //Setting values for Model and Code
                DgnCode code = DgnCode::CreateEmpty();
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(drawingModel->GetModelId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));

                //Creating Element of the specified instance
                DgnElementPtr ele = createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("Instance inserted for class:%s\n", ecClass->GetName().c_str());
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts instances for the InformationReferenceElement class heirarchy
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForInformationReferenceElement(ECClassCP className)
        {
        printf("\n\nInserting instances for InformationReferenceElement heirarchy:\n\n");

        List.clear();
        ASSERT_TRUE(List.empty());

        //Inserting a Link Model.
        LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(*m_db, "TestLinkModel");
        SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();
        ASSERT_TRUE(rootSubject.IsValid());

        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                //Setting values for Model and Code
                DgnCode code = DgnCode::CreateEmpty();
                if (className == BIS_CLASS_Subject)
                    {
                    ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(rootSubject->GetModelId())));
                    }
                else
                    {
                    ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(linkModel->GetModelId())));
                    }

                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));

                //Creating Element of the specified instance
                DgnElementPtr ele = createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("Instance inserted for class:%s\n", ecClass->GetName().c_str());
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts instances for the DefinitionElement class heirarchy
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForDefinitionElement(ECClassCP className)
        {
        printf("\n\nInserting instances for DefinitionElement heirarchy:\n\n");

        List.clear();
        ASSERT_TRUE(List.empty());

        //Inserting a Definition Model.
        DefinitionModelPtr defModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "TestDefinitionModel");
        ASSERT_TRUE(defModel.IsValid());
        DgnModelId model_id = defModel->GetModelId();

        List.push_back(className);

        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                if (className == BIS_CLASS_GeometryPart) // skip since it is an error to insert a bis:GeometryPart with no GeometryStream
                    continue;

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                //Setting values for Model and Code
                DgnCode code = DgnCode::CreateEmpty();
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(model_id)));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));

                //Creating Element of the specified instance
                DgnElementPtr ele = createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("Instance inserted for class:%s\n", ecClass->GetName().c_str());
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts instances for the InformationReferenceElement class heirarchy
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForInformationPartitionElement(ECClassCP className)
        {

        printf("\n\nInserting instances for InformationPartitionElement heirarchy:\n\n");

        List.clear();
        ASSERT_TRUE(List.empty());

        SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();

        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(className);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                //Gets the className
                Utf8StringCR className = ecClass->GetName();
                ASSERT_TRUE(ecClass != nullptr) << "ECClass '" << className << "' not found.";

                //Creates Instance of the given class
                ECN::StandaloneECInstancePtr ClassInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ASSERT_TRUE(ClassInstance.IsValid());

                //Setting values for Model and Code
                DgnCode code = DgnCode::CreateEmpty();
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Model", ECN::ECValue(DgnModel::RepositoryModelId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(*m_db))));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("CodeValue", ECN::ECValue(code.GetValueUtf8CP())));
                ASSERT_EQ(ECObjectsStatus::Success, ClassInstance->SetValue("Parent", ECN::ECValue(rootSubject->GetElementId())));

                //Creating Element of the specified instance
                DgnElementPtr ele = createElement(ClassInstance);
                ASSERT_TRUE(ele.IsValid()) << "Element creation failed for Class: " << className;

                //Inserting the element
                DgnDbStatus stat = DgnDbStatus::Success;
                DgnElementCPtr eleP = ele->Insert(&stat);
                ASSERT_TRUE(eleP.IsValid()) << "Insertion failed for Class: " << className;
                ASSERT_EQ(DgnDbStatus::Success, stat);

                if (stat == DgnDbStatus::Success)
                    {
                    printf("Instance inserted for class:%s\n", ecClass->GetName().c_str());
                    }
                }
            }
        }
    /*---------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Inserts the instances(For only BisCore schema) for InformationContentElement heirarchy.
    +---------------+---------------+---------------+---------------+---------------+------------------*/
    void ECInstancesCompatibility::InsertInstancesForInformationContentElementHeirarchy(ECClassCP className)
        {
        //Getting thye immediate derived classes
        const ECDerivedClassesList& InformationContentElementHeirarchy = className->GetDerivedClasses();

        //Traversing through the immediate derived classes of InformationContentElementHeirarchy
        for (ECClassCP ecClass : InformationContentElementHeirarchy)
            {
            if (ecClass->GetName() == BIS_CLASS_Document)
                {
                InsertInstancesForDocument(ecClass);
                }

            else if (ecClass->GetName() == BIS_CLASS_InformationReferenceElement)
                {
                InsertInstancesForInformationReferenceElement(ecClass);
                }

            else if (ecClass->GetName() == BIS_CLASS_DefinitionElement)
                {
                InsertInstancesForDefinitionElement(ecClass);
                }

            //Uncomment when implementation is fixed.
            //else if (ecClass->GetName() == BIS_CLASS_InformationPartitionElement)
            //    {
            //    InsertInstancesForInformationPartitionElement(ecClass);
            //    }

            }
        }

    /*---------------------------------------------------------------------------------------------------------------**//**
    * @bsimethod                                    Maha.Nasir                          04/17
    //Returns the list of classes in the Element heirarchy of BisCore schema for which the instance insertion is possible.
    +---------------+---------------+---------------+---------------+---------------+------------------+------------------*/
    std::vector<ECClassCP> ECInstancesCompatibility::GetValidClassesForInstanceInsertion(DgnDbR db)
        {
        ECSchemaCP BisSchema = db.Schemas().GetSchema(BIS_ECSCHEMA_NAME);
        EXPECT_TRUE(BisSchema != nullptr);

        ECClassCP ElementClass = BisSchema->GetClassCP(BIS_CLASS_Element);
        EXPECT_TRUE(ElementClass != nullptr);

        std::vector<ECClassCP> DerivedClassList = getDerivedClasses(ElementClass);

        for (ECClassCP ecClass : DerivedClassList)
            {
            if (ecClass->GetSchema().GetName() == BIS_ECSCHEMA_NAME && ecClass->IsEntityClass() && ecClass->GetClassModifier() != ECClassModifier::Abstract)
                {
                ValidClassesForInstanceInsertion.push_back(ecClass);
                }
            }
        return ValidClassesForInstanceInsertion;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Maha Nasir                    04/2017
    // Creates and opens the copy of the perserved Bim
    //---------------------------------------------------------------------------------------
    void ECInstancesCompatibility::SetUpDbFromBaselineCopy(Utf8CP versionString, Utf8CP destBaseName, DbResult expectedFirstOpenStatus)
        {
        //Source File Path
        BeFileName sourceFileName;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(sourceFileName);
        sourceFileName.AppendToPath(L"CompatibilityTestFiles");
        sourceFileName.AppendToPath(BeFileName(versionString, BentleyCharEncoding::Utf8));
        sourceFileName.AppendToPath(L"InstancesCompatibilitySeed.bim");
        ASSERT_TRUE(sourceFileName.DoesPathExist());

        //Destination file path
        BeFileName destFileName;
        BeTest::GetHost().GetOutputRoot(destFileName);
        destFileName.AppendToPath(BeFileName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8));
        BeFileName::CreateNewDirectory(destFileName.GetName());
        ASSERT_TRUE(destFileName.DoesPathExist());
        destFileName.AppendToPath(BeFileName(Utf8PrintfString("%s%s", destBaseName, versionString).c_str(), BentleyCharEncoding::Utf8));
        destFileName.AppendExtension(L"bim");
        ASSERT_FALSE(destFileName.DoesPathExist());

        //Creating copy of the source file.
        BeFileNameStatus copyStatus = BeFileName::BeCopyFile(sourceFileName, destFileName, true);
        ASSERT_EQ(BeFileNameStatus::Success, copyStatus);

        DbResult openStatus = BE_SQLITE_OK;

        //Opening the copy
        if (BE_SQLITE_OK != expectedFirstOpenStatus)
            {
            DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, destFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
            ASSERT_EQ(expectedFirstOpenStatus, openStatus);
            ASSERT_FALSE(db.IsValid());
            }

        if (BE_SQLITE_ERROR_SchemaUpgradeRequired == openStatus)
            {
            DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite);
            openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromDomains();
            DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
            ASSERT_EQ(BE_SQLITE_OK, openStatus);
            ASSERT_TRUE(db.IsValid());
            db->CloseDb();
            }

        m_db = DgnDb::OpenDgnDb(&openStatus, destFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        ASSERT_EQ(BE_SQLITE_OK, openStatus);
        ASSERT_TRUE(m_db.IsValid());
        }

//---------------------------------------------------------------------------------------------
// @bsimethod                                      Maha Nasir                  04/17
// Walks through all the classes of the BisCore schema and inserts instances for each class.
// Note: For now the test bypasses a few classes(namely InformationPartitionElement heirarchy,
//       Category, SubCategory, Texture and ViewDefinition ) which will be dealt later.  
//+---------------+---------------+---------------+---------------+---------------+------------
TEST_F(ECInstancesCompatibility, InstancesCompatibilitySeed)
    {
    SetupSeedProject();
    m_db->Schemas().CreateClassViewsInDb();

    //Getting the BisCore Schema
    ECSchemaCP BisSchema = m_db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);
    ASSERT_TRUE(BisSchema != nullptr);

    //Getting the pointer of the Class
    ECClassCP ElementClass = BisSchema->GetClassCP(BIS_CLASS_Element);
    ASSERT_TRUE(ElementClass != nullptr);

    //Emptying the contents of the vector.
    List.clear();
    ASSERT_TRUE(List.empty());

    //Inserting a Drawing model
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "Drawing");
    drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    ASSERT_TRUE(drawingModel->Is2dModel());

    const ECDerivedClassesList& ElementHeirarchy = ElementClass->GetDerivedClasses();

    for (ECClassP ecClass : ElementHeirarchy)
        {
        List.push_back(ecClass);

        if (ecClass->GetName() == BIS_CLASS_GeometricElement)
            {
            InsertInstancesForGeometricElementHeirarchy(ecClass);
            }

        else if (ecClass->GetName() == BIS_CLASS_InformationContentElement)
            {
            InsertInstancesForInformationContentElementHeirarchy(ecClass);
            }
        }
    }

//---------------------------------------------------------------------------------------------
// @bsimethod                                      Maha Nasir                  04/17
// Reads and verifies the Instances from the preserved Bim
//+---------------+---------------+---------------+---------------+---------------+------------
TEST_F(ECInstancesCompatibility, ReadInstances)
    {
    SetUpDbFromBaselineCopy("2-0-1-60", TEST_NAME, BE_SQLITE_OK);

    DgnDbR db= GetDgnDb();

    std::vector<ECClassCP> ClassList = GetValidClassesForInstanceInsertion(db);

    for (ECClassCP ClassP : ClassList)
        {
        Utf8StringCR ClassName= ClassP->GetName();

        Utf8PrintfString fullClassName("%s.%s", BIS_ECSCHEMA_NAME, ClassName.c_str());
        ElementIterator iter= db.Elements().MakeIterator(fullClassName.c_str(), nullptr, "ORDER BY ECInstanceId");
        ASSERT_TRUE(iter.BuildIdList<DgnElementId>().size() != 0) << "No instance found for Class:" << ClassName;

        //Iterates through the instances of each class
        for(auto element : iter)
            {
             ASSERT_TRUE(element.GetElementId().IsValid());
            }
        }
    }

//---------------------------------------------------------------------------------------------
// @bsimethod                                      Maha Nasir                  06/17
//+---------------+---------------+---------------+---------------+---------------+------------
TEST_F(ECInstancesCompatibility, UpdateInstances)
    {
    SetUpDbFromBaselineCopy("2-0-1-60", TEST_NAME, BE_SQLITE_OK);

    DgnDbR db = GetDgnDb();

    bvector<DgnElementId> idList;
    {
    Utf8PrintfString fullClassName("%s.%s", BIS_ECSCHEMA_NAME, "Element");
    ElementIterator iter = db.Elements().MakeIterator(fullClassName.c_str());
    idList = iter.BuildIdList<DgnElementId>();
    }

    int i = 0;
    for (auto elementId : idList)
        {
        if (elementId.GetValue() != 1099511627800 && elementId.GetValue() != 1099511627818)
            {
            ASSERT_TRUE(db.IsDbOpen());
            ASSERT_TRUE(elementId.IsValid()) << "Invalid ElementId:" << elementId.GetValue();

            DgnElementPtr ele = db.Elements().GetForEdit<DgnElement>(elementId);
            ASSERT_TRUE(ele.IsValid());
            ele->SetUserLabel("Updated");

            ASSERT_EQ(true, ele->Update().IsValid()) << "Update failed for elementId:" << ele->GetElementId().GetValue();
            ASSERT_STREQ("Updated", ele->GetUserLabel());
            i++;
            }
        }
    ASSERT_EQ(50, i);
    }
