/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Compatibility_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    04/2017
//========================================================================================
struct CompatibilityTests : public DgnDbTestFixture
{
    static constexpr Utf8CP GetSpatialCategoryName() {return "TestSpatialCategory";}
    static constexpr Utf8CP GetDefinitionPartitionName() {return "Definition";}
    static constexpr Utf8CP GetPhysicalPartitionName() {return "Physical";}
    static Utf8String GetSubjectName(int index) {return Utf8PrintfString(BIS_CLASS_Subject "%" PRIi32, index);}
    static Utf8String GetGeometryPartName(int index) {return Utf8PrintfString(BIS_CLASS_GeometryPart "%" PRIi32, index);}
    static Utf8String GetCategorySelectorName(int index) {return Utf8PrintfString(BIS_CLASS_CategorySelector "%" PRIi32, index);}
    static Utf8String GetModelSelectorName(int index) {return Utf8PrintfString(BIS_CLASS_ModelSelector "%" PRIi32, index);}
    static Utf8String GetDisplayStyle3dName(int index) {return Utf8PrintfString(BIS_CLASS_DisplayStyle3d "%" PRIi32, index);}
    static Utf8String GetSpatialViewDefinitionName(int index) {return Utf8PrintfString(BIS_CLASS_SpatialViewDefinition "%" PRIi32, index);}
    static Utf8String GetPhysicalElementName(int index) {return Utf8PrintfString(BIS_CLASS_PhysicalElement "%" PRIi32, index);}
    static Utf8String GetSpatialLocationName(int index) {return Utf8PrintfString(BIS_CLASS_SpatialLocationElement "%" PRIi32, index);}
    static Utf8String BuildWhereModelIdEquals(DgnModelId modelId) {return Utf8PrintfString("WHERE Model.Id=%" PRIi64, modelId.GetValue());}

    BE_JSON_NAME(inserted);
    BE_JSON_NAME(updated);

    void InsertSubHierarchy(SubjectCR, int);
    void InsertSpatialCategory();
    void InsertDefinitionModel(SubjectCR);
    DgnGeometryPartPtr InsertGeometryPart(DefinitionModelR, int);
    CategorySelectorPtr InsertCategorySelector(DefinitionModelR, DgnCategoryId, int);
    ModelSelectorPtr InsertModelSelector(DefinitionModelR, DgnModelId, int);
    DisplayStyle3dPtr InsertDisplayStyle3d(DefinitionModelR, int);
    SpatialViewDefinitionPtr InsertSpatialViewDefinition(DefinitionModelR, int, CategorySelectorR, DisplayStyle3dR, ModelSelectorR, DRange3dCR);
    void InsertPhysicalModel(SubjectCR);
    void InsertPhysicalElement(PhysicalModelR, DgnCategoryId, int);
    void InsertSpatialLocation(PhysicalModelR, DgnCategoryId, int);

    void ModifySubHierarchy(SubjectCR, int);
    void ModifyDefinitionModelContents(SubjectCR);
    void ModifyDefinitionElements(DefinitionModelR, Utf8CP);
    void ModifyPhysicalModelContents(SubjectCR);
    void ModifySpatialElements(PhysicalModelR, Utf8CP);

    DgnCategoryId GetSpatialCategoryId();
    DefinitionModelPtr GetDefinitionModel(SubjectCR);
    PhysicalModelPtr GetPhysicalModel(SubjectCR);

    void SetupFromOtherBranch(Utf8CP);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, Seed)
    {
    SetupSeedProject();
    InsertSpatialCategory();

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(BentleyStatus::SUCCESS, db.Schemas().CreateClassViewsInDb());
    ASSERT_EQ(1, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size()) << "Expected just the root Subject";
    InsertSubHierarchy(*db.Elements().GetRootSubject(), 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, Modify)
    {
    SetupSeedProject();
    InsertSpatialCategory();

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(1, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size()) << "Expected just the root Subject";
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    InsertSubHierarchy(*rootSubject, 1);
    ModifySubHierarchy(*rootSubject, 1);
    InsertSubHierarchy(*rootSubject, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
TEST_F(CompatibilityTests, Update)
    {
    SetupFromOtherBranch("BranchName");

    DgnDbR db = GetDgnDb();
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Subject)).BuildIdSet<DgnElementId>().size());
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    ModifySubHierarchy(*rootSubject, 1);
    InsertSubHierarchy(*rootSubject, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertSubHierarchy(SubjectCR parentSubject, int subjectNumber)
    {
    SubjectCPtr subject = Subject::CreateAndInsert(parentSubject, GetSubjectName(subjectNumber).c_str());
    ASSERT_TRUE(subject.IsValid());

    InsertPhysicalModel(*subject);
    InsertDefinitionModel(*subject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifySubHierarchy(SubjectCR parentSubject, int subjectNumber)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode subjectCode = Subject::CreateCode(parentSubject, GetSubjectName(subjectNumber).c_str());
    DgnElementId subjectId = db.Elements().QueryElementIdByCode(subjectCode);
    SubjectCPtr subject = db.Elements().Get<Subject>(subjectId);
    ASSERT_TRUE(subject.IsValid());

    ModifyPhysicalModelContents(*subject);
    ModifyDefinitionModelContents(*subject);
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
DgnCategoryId CompatibilityTests::GetSpatialCategoryId()
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(GetDgnDb(), SpatialCategory::CreateCode(GetDgnDb(), GetSpatialCategoryName()));
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertDefinitionModel(SubjectCR subject)
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
    CategorySelectorPtr categorySelector = new CategorySelector(model, GetCategorySelectorName(index).c_str());
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
void CompatibilityTests::ModifyDefinitionModelContents(SubjectCR subject)
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
                {
                DgnElementPtr element = db.Elements().GetForEdit<DgnElement>(elementEntry.GetElementId());
                ASSERT_TRUE(element.IsValid());
                ASSERT_FALSE(element->GetUserProperties(json_inserted()).isNull());
                DgnCode oldCode = element->GetCode();
                DgnCode newCode(oldCode.GetCodeSpecId(), Utf8PrintfString("%s%s", className, "Updated"), oldCode.GetScope());
                element->SetCode(newCode);
                element->SetUserProperties(json_updated(), DateTime::GetCurrentTime().ToString());
                ASSERT_TRUE(element->Update().IsValid());
                break;
                }

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
void CompatibilityTests::InsertPhysicalModel(SubjectCR subject)
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
void CompatibilityTests::ModifyPhysicalModelContents(SubjectCR subject)
    {
    PhysicalModelPtr model = GetPhysicalModel(subject);
    ModifySpatialElements(*model, BIS_SCHEMA(BIS_CLASS_PhysicalElement));
    ModifySpatialElements(*model, BIS_SCHEMA(BIS_CLASS_SpatialLocationElement));

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
void CompatibilityTests::ModifySpatialElements(PhysicalModelR model, Utf8CP className)
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
                {
                SpatialElementPtr element = db.Elements().GetForEdit<SpatialElement>(elementEntry.GetElementId());
                ASSERT_TRUE(element.IsValid());
                ASSERT_TRUE(element->GetFederationGuid().IsValid());
                Utf8String userLabel(element->GetUserLabel());
                userLabel.append("Updated");
                element->SetUserLabel(userLabel.c_str());
                ASSERT_TRUE(element->Update().IsValid());
                break;
                }

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
void CompatibilityTests::SetupFromOtherBranch(Utf8CP branchName)
    {
    BeFileName testFixtureName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8);
    BeFileName testFileName(branchName, BentleyCharEncoding::Utf8);

    BeFileName sourceFileName;
    BeTest::GetHost().GetOutputRoot(sourceFileName);
    sourceFileName.AppendToPath(testFixtureName);
    sourceFileName.AppendToPath(L"Seed.bim");
    ASSERT_TRUE(sourceFileName.DoesPathExist());

    BeFileName destFileName;
    BeTest::GetHost().GetOutputRoot(destFileName);
    destFileName.AppendToPath(testFixtureName);
    destFileName.AppendToPath(testFileName);
    destFileName.AppendExtension(L"bim");
    ASSERT_FALSE(destFileName.DoesPathExist());

    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(sourceFileName, destFileName, true /*failIfFileExists*/);
    ASSERT_EQ(BeFileNameStatus::Success, copyStatus);

    DbResult openStatus;
    DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite);
    m_db = DgnDb::OpenDgnDb(&openStatus, destFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, openStatus);
    ASSERT_TRUE(m_db.IsValid());
    }
