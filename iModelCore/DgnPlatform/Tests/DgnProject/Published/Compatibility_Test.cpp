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
    static Utf8String GetSubjectName(int subjectNumber) {return Utf8PrintfString("Subject%" PRIi32, subjectNumber);}
    static Utf8String BuildWhereModelIdEquals(DgnModelId modelId) {return Utf8PrintfString("WHERE Model.Id=%" PRIi64, modelId.GetValue());}

    void InsertSubHierarchy(SubjectCR, int);
    void InsertSpatialCategory();
    void InsertDefinitionModel(SubjectCR);
    void InsertCategorySelector(DefinitionModelR, DgnCategoryId, int);
    void InsertGeometryPart(DefinitionModelR, int);
    void InsertPhysicalModel(SubjectCR);
    void InsertPhysicalElement(PhysicalModelR, DgnCategoryId, int);

    void ModifySubHierarchy(SubjectCR, int);
    void ModifyDefinitionModelContents(SubjectCR);
    void ModifyDefinitionElements(DefinitionModelR, Utf8CP);
    void ModifyPhysicalModelContents(SubjectCR);

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

    InsertDefinitionModel(*subject);
    InsertPhysicalModel(*subject);
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

    ModifyDefinitionModelContents(*subject);
    ModifyPhysicalModelContents(*subject);
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
    DefinitionModelPtr model = DefinitionModel::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    DgnCategoryId categoryId = GetSpatialCategoryId();

    for (int i=0; i<3; i++)
        {
        InsertCategorySelector(*model, categoryId, i);
        InsertGeometryPart(*model, i);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertCategorySelector(DefinitionModelR model, DgnCategoryId categoryId, int index)
    {
    CategorySelector categorySelector(model, Utf8PrintfString("CategorySelector%" PRIi32, index).c_str());
    categorySelector.AddCategory(categoryId);
    ASSERT_TRUE(categorySelector.Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertGeometryPart(DefinitionModelR model, int index)
    {
    DgnGeometryPartPtr geometryPart = DgnGeometryPart::Create(model, Utf8PrintfString("GeometryPart%" PRIi32, index));
    GeometryBuilderPtr geometryPartBuilder = GeometryBuilder::CreateGeometryPart(model.GetDgnDb(), true /*is3d*/);
    ASSERT_TRUE(geometryPart.IsValid() && geometryPartBuilder.IsValid());
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromZero(), DPoint3d::From(1, 1, 1), true));
    ASSERT_TRUE(geometry.IsValid());
    geometryPartBuilder->Append(*geometry);
    geometryPartBuilder->Finish(*geometryPart);
    ASSERT_TRUE(geometryPart->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyDefinitionModelContents(SubjectCR subject)
    {
    DefinitionModelPtr model = GetDefinitionModel(subject);
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_CategorySelector));
    ModifyDefinitionElements(*model, BIS_SCHEMA(BIS_CLASS_GeometryPart));

    DgnDbR db = subject.GetDgnDb();
    Utf8String whereClause = BuildWhereModelIdEquals(model->GetModelId());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_CategorySelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometryPart), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    InsertCategorySelector(*model, GetSpatialCategoryId(), 3);
    InsertGeometryPart(*model, 3);
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_CategorySelector), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometryPart), whereClause.c_str()).BuildIdList<DgnElementId>().size());
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
                DgnCode oldCode = element->GetCode();
                DgnCode newCode(oldCode.GetCodeSpecId(), Utf8PrintfString("%s%s", className, "Updated"), oldCode.GetScope());
                element->SetCode(newCode);
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
        InsertPhysicalElement(*model, categoryId, i);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int index)
    {
    GenericPhysicalObjectPtr physicalObject = GenericPhysicalObject::Create(model, categoryId);
    ASSERT_TRUE(physicalObject.IsValid());
    physicalObject->SetUserLabel(Utf8PrintfString("PhysicalElement%" PRIi32, index).c_str());
    ASSERT_TRUE(physicalObject->Insert().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2017
//---------------------------------------------------------------------------------------
void CompatibilityTests::ModifyPhysicalModelContents(SubjectCR subject)
    {
    DgnDbR db = subject.GetDgnDb();
    PhysicalModelPtr model = GetPhysicalModel(subject);

    int i=0;
    Utf8PrintfString whereClause("WHERE Model.Id=%" PRIi64, model->GetModelId().GetValue());
    for (ElementIteratorEntryCR elementEntry : db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement), whereClause.c_str()))
        {
        switch (i++)
            {
            case 0: // skip first element
                break;

            case 1: // update second element
                {
                GenericPhysicalObjectPtr element = db.Elements().GetForEdit<GenericPhysicalObject>(elementEntry.GetElementId());
                ASSERT_TRUE(element.IsValid());
                Utf8String userLabel(element->GetUserLabel());
                userLabel.append("x");
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

    ASSERT_EQ(2, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
    InsertPhysicalElement(*model, GetSpatialCategoryId(), 3);
    ASSERT_EQ(3, db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement), whereClause.c_str()).BuildIdList<DgnElementId>().size());
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
