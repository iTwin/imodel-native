/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
//! ChangeSummaryTestFixture
//=======================================================================================
struct ChangeSummaryTestFixture : public GenericDgnModelTestFixture
{
protected:
    DgnDbPtr m_testDb;
    DgnModelPtr m_testModel;
    DgnCategoryId m_testCategoryId;
    DgnElementId m_testElementId;

    void CreateDgnDb();
    void OpenDgnDb();
    void CloseDgnDb();

    void InsertModel();
    void InsertCategory();
    void InsertElement();
    void ModifyElement();
    void DeleteElement();

    void DumpSqlChangeSet(ChangeSet const& sqlChangeSet, Utf8CP label);
    void DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label);

    void GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary);
    void GetChangeSummaryFromSavedTransactions(ChangeSummary& changeSummary);

    bool ChangeSummaryHasInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode);
    BentleyStatus ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb);
public:
    ChangeSummaryTestFixture() : GenericDgnModelTestFixture(__FILE__, true) {}
    virtual ~ChangeSummaryTestFixture() {m_testDb->SaveChanges();}
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

//=======================================================================================
//! SampleChangeSet
//=======================================================================================
struct SqlChangeSet : BeSQLite::ChangeSet
    {
    virtual ConflictResolution _OnConflict(ConflictCause cause, Changes::Change iter)
        {
        BeAssert(false);
        fprintf(stderr, "Conflict \"%s\"\n", ChangeSet::InterpretConflictCause(cause).c_str());
        return ConflictResolution::Skip;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::CreateDgnDb()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);

    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile(DgnDbTestDgnManager::GetOutputFilePath(L"ChangeSummaryTest.dgndb"));

    DbResult createStatus;
    m_testDb = DgnDb::CreateDgnDb(&createStatus, DgnDbTestDgnManager::GetOutputFilePath(L"ChangeSummaryTest.dgndb"), createProjectParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not create test project";

    m_testDb->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::OpenDgnDb()
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_testDb = DgnDb::OpenDgnDb(&openStatus, DgnDbTestDgnManager::GetOutputFilePath(L"ChangeSummaryTest.dgndb"), openParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

    DgnModelId modelId = m_testDb->Models().QueryFirstModelId();
    m_testModel = m_testDb->Models().GetModel(modelId).get();

    m_testDb->Txns().EnableTracking(true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::CloseDgnDb()
    {
    m_testDb->CloseDb();
    m_testModel = nullptr;
    m_testDb = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::InsertModel()
    {
    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
    DgnClassId classId = m_testDb->Domains().GetClassId(handler);
    m_testModel = handler.Create(DgnModel::CreateParams(*m_testDb, classId, "ChangeSetModel"));

    DgnDbStatus status = m_testModel->Insert();
    ASSERT_TRUE(DgnDbStatus::Success == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::InsertCategory()
    {
    DgnCategories::Category category("ChangeSetTestCategory", DgnCategories::Scope::Physical);
    category.SetRank(DgnCategories::Rank::Application);

    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    DbResult result = m_testDb->Categories().Insert(category, appearance);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    m_testCategoryId = category.GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::InsertElement()
    {
    PhysicalModelP physicalTestModel = dynamic_cast<PhysicalModelP> (m_testModel.get());
    BeAssert(physicalTestModel != nullptr);
    BeAssert(m_testCategoryId.IsValid());

    PhysicalElementPtr testElement = PhysicalElement::Create(*physicalTestModel, m_testCategoryId);

    DPoint3d sizeOfBlock = DPoint3d::From(1, 1, 1);
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(0, 0, 0), sizeOfBlock, true);
    ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(testGeomPtr.IsValid());

    DPoint3d centerOfBlock = DPoint3d::From(0, 0, 0);
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*physicalTestModel, m_testCategoryId, centerOfBlock, YawPitchRollAngles());
    builder->Append(*testGeomPtr);
    BentleyStatus status = builder->SetGeomStreamAndPlacement(*testElement);
    BeAssert(status == SUCCESS);

    m_testElementId = m_testDb->Elements().Insert(*testElement)->GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::ModifyElement()
    {
    RefCountedPtr<PhysicalElement> testElement = m_testDb->Elements().GetForEdit<PhysicalElement>(m_testElementId);
    BeAssert(testElement.IsValid());

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DeleteElement()
    {
    DgnDbStatus dbStatus = m_testDb->Elements().Delete(m_testElementId);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label)
    {
    printf("%s:\n", label);
    changeSummary.Dump();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpSqlChangeSet(ChangeSet const& changeSet, Utf8CP label)
    {
    printf("%s:\n", label);
    
    // BeFileName pathname = m_testHost.BuildProjectFileName(fileName);
    // changeSet.Dump(label, *m_testDb, false, INT_MAX);
    Changes changes(*const_cast<ChangeSet*>(&changeSet));
    for (Changes::Change change : changes)
        {
        Utf8CP tableName;
        int nCols, indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        bvector<Utf8String> columnNames;
        m_testDb->GetColumns(columnNames, tableName);
        BeAssert((int) columnNames.size() == nCols);

        if (opcode == DbOpcode::Delete || opcode == DbOpcode::Update)
            {
            printf("Old:\n");
            for (int ii = 0; ii < nCols; ii++)
                {
                DbValue v = change.GetValue(ii, Changes::Change::Stage::Old);

                printf("%s=", columnNames[ii].c_str());

                if (!v.IsValid())
                    printf("<invalid>");
                else if (v.IsNull())
                    printf("<null>");
                else
                    printf("%s", v.Format(1000).c_str());

                printf(" ");
                }
            printf("\n");
            }

        if (opcode == DbOpcode::Update || opcode == DbOpcode::Insert)
            {
            printf("New:\n");
            for (int ii = 0; ii < nCols; ii++)
                {
                DbValue v = change.GetValue(ii, Changes::Change::Stage::New);

                printf("%s=", columnNames[ii].c_str());

                if (!v.IsValid())
                    printf("<invalid>");
                else if (v.IsNull())
                    printf("<null>");
                else
                    printf("%s", v.Format(1000).c_str());

                printf(" ");
                }
            printf("\n");
            }
        }

    printf("\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    072015
//---------------------------------------------------------------------------------------
bool ChangeSummaryTestFixture::ChangeSummaryHasInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
    {
    Utf8String tableName = changeSummary.GetInstancesTableName();
    ECClassId classId = m_testDb->Schemas().GetECClassId(schemaName, className);

    Utf8PrintfString sql("SELECT NULL FROM %s WHERE ClassId=? AND InstanceId=? AND DbOpcode=?", tableName.c_str());
    CachedStatementPtr statement = m_testDb->GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindInt64(1, classId);
    statement->BindId(2, instanceId);
    statement->BindInt(3, (int) dbOpcode);

    DbResult result = statement->Step();
    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryTestFixture::ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb)
    {
    ECClassCR ecClass = instance.GetClass();
    ECInstanceInserter inserter(dgndb, ecClass);
    if (!inserter.IsValid())
        return ERROR;

    return inserter.Insert(instanceKey, instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary)
    {
    SqlChangeSet sqlChangeSet;
    DbResult result = sqlChangeSet.FromChangeTrack(m_testDb->Txns(), ChangeSet::SetType::Full);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    changeSummary.Free();
    BentleyStatus status = changeSummary.FromSqlChangeSet(sqlChangeSet);
    ASSERT_TRUE(SUCCESS == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::GetChangeSummaryFromSavedTransactions(ChangeSummary& changeSummary)
    {
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(changeSummary, m_testDb->Txns().GetSessionStartId());
    ASSERT_TRUE(status == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    m_testDb->SaveChanges();
    InsertModel();
    InsertCategory();
    InsertElement();
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserts");

    ChangeSummary after inserts:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;152;dgn:ElementGeom;Insert;No
            ECInstanceId;NULL;1
            Geom;NULL;...
            Placement;NULL;...
    1;164;dgn:ElementOwnsGeom;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;130;dgn:AuthorityIssuesElementCode;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;160;dgn:ElementIsInCategory;Insert;No
            ECInstanceId;NULL;1
            SourceECClassId;NULL;174
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;172;dgn:ModelContainsElements;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;174;dgn:PhysicalElement;Insert;No
            CategoryId;NULL;1
            Code;NULL;"ChangeSetTestElementCode - 1"
            CodeAuthorityId;NULL;1
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestElementLabel"
            LastMod;NULL;2.45726e+006
            ModelId;NULL;1
    1;136;dgn:CategoryOwnsSubCategories;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;137;dgn:SubCategory;Insert;No
            CategoryId;NULL;1
            ECInstanceId;NULL;1
            Props;NULL;"{"color":16777215}"
    1;135;dgn:Category;Insert;No
            Code;NULL;"ChangeSetTestCategory"
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestCategory"
            Rank;NULL;2
            Scope;NULL;1
    1;175;dgn:PhysicalModel;Insert;No
            ECInstanceId;NULL;1
            Name;NULL;"ChangeSetModel"
            Props;NULL;"{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
            Space;NULL;1
            Type;NULL;0
            Visibility;NULL;1
    */
    ASSERT_EQ(10, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(m_testModel->GetModelId().GetValueUnchecked()), "dgn", "PhysicalModel", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(m_testCategoryId.GetValueUnchecked()), "dgn", "Category", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Insert));

    m_testDb->SaveChanges();
    ModifyElement();
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after updates");

    ChangeSummary after updates:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;171;dgn:PhysicalElement;Update;No
            Code;"ChangeSetTestElementCode";"ModifiedElementCode"
            ECInstanceId;1;NULL
            Label;"ChangeSetTestElementLabel";"ModifiedElementLabel"
            LastMod;2.45726e+006;2.45726e+006
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Update));

    m_testDb->SaveChanges();
    DeleteElement();
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after deletes");

    ChangeSummary after updates:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;174;dgn:PhysicalElement;Update;No
            Code;"ChangeSetTestElementCode - 1";"ModifiedElementCode - 2"
            ECInstanceId;1;NULL
            Label;"ChangeSetTestElementLabel";"ModifiedElementLabel"
            LastMod;2.45726e+006;2.45726e+006
    ChangeSummary after deletes:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;152;dgn:ElementGeom;Delete;Yes
            ECInstanceId;1;NULL
            Geom;...;NULL
            Placement;...;NULL
    1;164;dgn:ElementOwnsGeom;Delete;Yes
            ECInstanceId;1;NULL
            SourceECInstanceId;1;NULL
            TargetECInstanceId;1;NULL
    1;130;dgn:AuthorityIssuesElementCode;Delete;No
            ECInstanceId;1;NULL
            SourceECInstanceId;1;NULL
            TargetECClassId;174;NULL
            TargetECInstanceId;1;NULL
    1;160;dgn:ElementIsInCategory;Delete;No
            ECInstanceId;1;NULL
            SourceECClassId;174;NULL
            SourceECInstanceId;1;NULL
            TargetECInstanceId;1;NULL
    1;172;dgn:ModelContainsElements;Delete;No
            ECInstanceId;1;NULL
            SourceECInstanceId;1;NULL
            TargetECClassId;174;NULL
            TargetECInstanceId;1;NULL
    1;174;dgn:PhysicalElement;Delete;No
            CategoryId;1;NULL
            Code;"ModifiedElementCode - 2";NULL
            CodeAuthorityId;1;NULL
            ECInstanceId;1;NULL
            Label;"ModifiedElementLabel";NULL
            LastMod;2.45726e+006;NULL
            ModelId;1;NULL
    */
    ASSERT_EQ(6, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Delete));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromSavedTransactions)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    InsertModel();
    InsertCategory();
    InsertElement();

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /* 
    DumpChangeSummary(changeSummary, "After inserts");

    After inserts:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;175;dgn:PhysicalModel;Insert;No
            ECInstanceId;NULL;1
            Name;NULL;"ChangeSetModel"
            Props;NULL;"{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
            Space;NULL;1
            Type;NULL;0
            Visibility;NULL;1
    1;135;dgn:Category;Insert;No
            Code;NULL;"ChangeSetTestCategory"
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestCategory"
            Rank;NULL;2
            Scope;NULL;1
    1;136;dgn:CategoryOwnsSubCategories;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;137;dgn:SubCategory;Insert;No
            CategoryId;NULL;1
            ECInstanceId;NULL;1
            Props;NULL;"{"color":16777215}"
    1;130;dgn:AuthorityIssuesElementCode;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;160;dgn:ElementIsInCategory;Insert;No
            ECInstanceId;NULL;1
            SourceECClassId;NULL;174
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;172;dgn:ModelContainsElements;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;174;dgn:PhysicalElement;Insert;No
            CategoryId;NULL;1
            Code;NULL;"ChangeSetTestElementCode - 1"
            CodeAuthorityId;NULL;1
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestElementLabel"
            LastMod;NULL;2.45726e+006
            ModelId;NULL;1
    1;152;dgn:ElementGeom;Insert;No
            ECInstanceId;NULL;1
            Geom;NULL;...
            Placement;NULL;...
    1;164;dgn:ElementOwnsGeom;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    */
    ASSERT_EQ(10, changeSummary.MakeInstanceIterator().QueryCount());

    ModifyElement();

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "After updates");

    After updates:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;175;dgn:PhysicalModel;Insert;No
            ECInstanceId;NULL;1
            Name;NULL;"ChangeSetModel"
            Props;NULL;"{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
            Space;NULL;1
            Type;NULL;0
            Visibility;NULL;1
    1;135;dgn:Category;Insert;No
            Code;NULL;"ChangeSetTestCategory"
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestCategory"
            Rank;NULL;2
            Scope;NULL;1
    1;136;dgn:CategoryOwnsSubCategories;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;137;dgn:SubCategory;Insert;No
            CategoryId;NULL;1
            ECInstanceId;NULL;1
            Props;NULL;"{"color":16777215}"
    1;130;dgn:AuthorityIssuesElementCode;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;160;dgn:ElementIsInCategory;Insert;No
            ECInstanceId;NULL;1
            SourceECClassId;NULL;174
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;172;dgn:ModelContainsElements;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;174
            TargetECInstanceId;NULL;1
    1;174;dgn:PhysicalElement;Insert;No
            CategoryId;NULL;1
            Code;NULL;"ModifiedElementCode - 2"
            CodeAuthorityId;NULL;1
            ECInstanceId;NULL;1
            Label;NULL;"ModifiedElementLabel"
            LastMod;NULL;2.45726e+006
            ModelId;NULL;1
    1;152;dgn:ElementGeom;Insert;No
            ECInstanceId;NULL;1
            Geom;NULL;...
            Placement;NULL;...
    1;164;dgn:ElementOwnsGeom;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    */
    ASSERT_EQ(10, changeSummary.MakeInstanceIterator().QueryCount());

    DeleteElement();

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "After deletes");

    After deletes:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;173;dgn:PhysicalModel;Insert;No
            ECInstanceId;NULL;1
            Name;NULL;"ChangeSetModel"
            Props;NULL;"{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
            Space;NULL;1
            Type;NULL;0
            Visibility;NULL;1
    1;132;dgn:Category;Insert;No
            Code;NULL;"ChangeSetTestCategory"
            ECInstanceId;NULL;1
            Label;NULL;"ChangeSetTestCategory"
            Rank;NULL;2
            Scope;NULL;1
    1;133;dgn:CategoryOwnsSubCategories;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;1
    1;134;dgn:SubCategory;Insert;No
            CategoryId;NULL;1
            ECInstanceId;NULL;1
            Props;NULL;"{"color":16777215}"
    */
    ASSERT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ValidateInstanceIterator)
    {
    CreateDgnDb();

    InsertModel();
    InsertCategory();
    InsertElement();

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    int countIter = 0;
    for (ChangeSummary::InstanceIterator::const_iterator const& entry : changeSummary.MakeInstanceIterator())
        {
        countIter++;
        UNUSED_VARIABLE(entry);
        }
    ASSERT_EQ(countIter, 10);

    int countQuery = changeSummary.MakeInstanceIterator().QueryCount();
    ASSERT_EQ(countQuery, 10);
    }

extern ECSchemaPtr ReadECSchemaFromDisk(WCharCP schemaPathname);
extern bool ImportECSchema(ECSchemaR ecSchema, DgnDbR project);
extern IECInstancePtr CreateStartupCompanyInstance(ECSchemaCR startupSchema);

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, StructArrayChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary changeSummary(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    ChangeSummary after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Insert;No
            ECInstanceId;NULL;1
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    m_testDb->SaveChanges();

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_ArrayOfAnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    ChangeSummary after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Update;No
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges();

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_ArrayOfAnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    ChangeSummary after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Update;No
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges();
    
    ECClassCP ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    DbResult stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    ChangeSummary after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Delete;Yes
            ECInstanceId;1;NULL
            anglesFoo.Alpha;12.345;NULL
            anglesFoo.Beta;12.345;NULL
            arrayOfIntsFoo;...;NULL
            doubleFoo;12.345;NULL
            intFoo;67;NULL
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Delete));

    m_testDb->SaveChanges();

    ecSqlStmt.Finalize();
    ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, "INSERT INTO StartupCompany.AnglesStruct (Alpha,Beta,Theta) VALUES(1.1,2.2,3.3)");
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ECInstanceKey structInstanceKey;
    stepStatus = ecSqlStmt.Step (structInstanceKey);
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting a plain struct that gets stored in a struct array table");

    ChangeSummary after inserting a plain struct that gets stored in a struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    5;199;StartupCompany:AnglesStruct;Insert;No
            Alpha;NULL;1.1
            Beta;NULL;2.2
            ECInstanceId;NULL;5
            Theta;NULL;3.3
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, structInstanceKey.GetECInstanceId(), "StartupCompany", "AnglesStruct", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, StructArrayChangesFromSavedTransactions)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary changeSummary(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    ChangeSummary after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Insert;No
            ECInstanceId;NULL;1
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_ArrayOfAnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    ChangeSummary after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Insert;No
            ECInstanceId;NULL;1
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_ArrayOfAnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);
    
    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    ChangeSummary after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;237;StartupCompany:Foo;Insert;No
            ECInstanceId;NULL;1
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    ECClassCP ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    DbResult stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    ChangeSummary after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    */
    ASSERT_EQ(0, changeSummary.MakeInstanceIterator().QueryCount());

    ecSqlStmt.Finalize();
    ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, "INSERT INTO StartupCompany.AnglesStruct (Alpha,Beta,Theta) VALUES(1.1,2.2,3.3)");
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ECInstanceKey structInstanceKey;
    stepStatus = ecSqlStmt.Step(structInstanceKey);
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting a plain struct that gets stored in a struct array table");

    ChangeSummary after inserting a plain struct that gets stored in a struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    5;199;StartupCompany:AnglesStruct;Insert;No
            Alpha;NULL;1.1
            Beta;NULL;2.2
            ECInstanceId;NULL;5
            Theta;NULL;3.3
    */
    ASSERT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, structInstanceKey.GetECInstanceId(), "StartupCompany", "AnglesStruct", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    ChangeSummary after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    4;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;4
            Make;NULL;"Tesla"
            Model;NULL;"Model-S"
    5;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;5
            Make;NULL;"Toyota"
            Model;NULL;"Prius"
    2;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;2
            Name;NULL;"AcmeWorks"
    3;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;3
            Name;NULL;"CmeaWorks"
    1;226;StartupCompany:Employee;Insert;No
            ECInstanceId;NULL;1
            FirstName;NULL;"John"
            LastName;NULL;"Doe"
    */
    ASSERT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_testDb->SaveChanges();

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    ChangeSummary after inserting relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    6;231;StartupCompany:EmployeeHardware;Insert;No
            ECInstanceId;NULL;6
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;221
            TargetECInstanceId;NULL;4
    1;228;StartupCompany:EmployeeCompany;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;2
    */
    ASSERT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_testDb->SaveChanges();

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    ChangeSummary after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;228;StartupCompany:EmployeeCompany;Update;No
            ECInstanceId;1;NULL
            SourceECInstanceId;1;NULL
            TargetECInstanceId;2;3
    6;231;StartupCompany:EmployeeHardware;Delete;Yes
            ECInstanceId;6;NULL
            SourceECInstanceId;1;NULL
            TargetECClassId;221;NULL
            TargetECInstanceId;4;NULL
    7;231;StartupCompany:EmployeeHardware;Insert;No
            ECInstanceId;NULL;7
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;221
            TargetECInstanceId;NULL;5
    */
    ASSERT_EQ(3, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Update));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromSavedTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    ChangeSummary after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;226;StartupCompany:Employee;Insert;No
            ECInstanceId;NULL;1
            FirstName;NULL;"John"
            LastName;NULL;"Doe"
    2;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;2
            Name;NULL;"AcmeWorks"
    3;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;3
            Name;NULL;"CmeaWorks"
    4;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;4
            Make;NULL;"Tesla"
            Model;NULL;"Model-S"
    5;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;5
            Make;NULL;"Toyota"
            Model;NULL;"Prius"
    */
    ASSERT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    changeSummary.Free();
    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    ChangeSummary after inserting relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    6;231;StartupCompany:EmployeeHardware;Insert;No
            ECInstanceId;NULL;6
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;221
            TargetECInstanceId;NULL;4
    1;226;StartupCompany:Employee;Insert;No
            ECInstanceId;NULL;1
            FirstName;NULL;"John"
            LastName;NULL;"Doe"
    1;228;StartupCompany:EmployeeCompany;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;2
    2;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;2
            Name;NULL;"AcmeWorks"
    3;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;3
            Name;NULL;"CmeaWorks"
    4;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;4
            Make;NULL;"Tesla"
            Model;NULL;"Model-S"
    5;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;5
            Make;NULL;"Toyota"
            Model;NULL;"Prius"
    */
    ASSERT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    /*
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    changeSummary.Free();
    GetChangeSummaryFromSavedTransactions(changeSummary);

    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    ChangeSummary after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    7;231;StartupCompany:EmployeeHardware;Insert;No
            ECInstanceId;NULL;7
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;221
            TargetECInstanceId;NULL;5
    1;226;StartupCompany:Employee;Insert;No
            ECInstanceId;NULL;1
            FirstName;NULL;"John"
            LastName;NULL;"Doe"
    1;228;StartupCompany:EmployeeCompany;Insert;No
            ECInstanceId;NULL;1
            SourceECInstanceId;NULL;1
            TargetECInstanceId;NULL;3
    2;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;2
            Name;NULL;"AcmeWorks"
    3;217;StartupCompany:Company;Insert;No
            ECInstanceId;NULL;3
            Name;NULL;"CmeaWorks"
    4;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;4
            Make;NULL;"Tesla"
            Model;NULL;"Model-S"
    5;221;StartupCompany:Hardware;Insert;No
            ECInstanceId;NULL;5
            Make;NULL;"Toyota"
            Model;NULL;"Prius"
    */
    ASSERT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChildRelationshipChanges)
    {
    CreateDgnDb();

    InsertModel();
    InsertCategory();
    InsertElement();
    DgnElementId parentElementId = m_testElementId;
    InsertElement();
    DgnElementId childElementId = m_testElementId;

    m_testDb->SaveChanges();

    RefCountedPtr<DgnElement> childElementPtr = m_testDb->Elements().GetForEdit<DgnElement>(childElementId);
    
    DgnDbStatus dbStatus = childElementPtr->SetParentId(parentElementId);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);
    
    childElementPtr->Update(&dbStatus);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    /*
    DumpChangeSummary(changeSummary, "ChangeSummary after setting ParentId");

    ChangeSummary after setting ParentId:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    2;161;dgn:ElementOwnsChildElements;Insert;No
        ECInstanceId;NULL;2
        SourceECClassId;NULL;171
        SourceECInstanceId;NULL;1
        TargetECClassId;NULL;171
        TargetECInstanceId;NULL;2
    2;171;dgn:PhysicalElement;Update;No
        ECInstanceId;2;NULL
        LastMod;2.45726e+006;2.45726e+006
        ParentId;NULL;1
    */
    ASSERT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), "dgn", "ElementOwnsChildElements", DbOpcode::Insert)); // Captured due to change of FK relationship (ParentId column)
    ASSERT_TRUE(ChangeSummaryHasInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Update)); // Captured due to change of ParentId property

    ECClassId relClassId = m_testDb->Schemas().GetECClassId("dgn", "ElementOwnsChildElements");
    ECClassId elClassId = m_testDb->Schemas().GetECClassId("dgn", "PhysicalElement");

    ChangeSummary::Instance instance = changeSummary.GetInstance(elClassId, childElementId);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary::Instance relInstance = changeSummary.GetInstance(relClassId, childElementId);
    ASSERT_TRUE(relInstance.IsValid());

    DbDupValue value(nullptr);

    value = relInstance.GetNewValue("SourceECClassId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(elClassId, value.GetValueInt64());
    ASSERT_EQ(elClassId, relInstance.GetNewValue("SourceECClassId").GetValueInt64());

    value = relInstance.GetNewValue("SourceECInstanceId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), relInstance.GetNewValue("SourceECInstanceId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECClassId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(elClassId, value.GetValueInt64());
    ASSERT_EQ(elClassId, relInstance.GetNewValue("TargetECClassId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECInstanceId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(childElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(childElementId.GetValueUnchecked(), relInstance.GetNewValue("TargetECInstanceId").GetValueInt64());

    value = instance.GetNewValue("ParentId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), instance.GetNewValue("ParentId").GetValueInt64());

    ASSERT_EQ(5, relInstance.MakeValueIterator(changeSummary).QueryCount());
    ASSERT_EQ(3, instance.MakeValueIterator(changeSummary).QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryChangedElements)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    InsertModel();
    InsertCategory();

    bset<DgnElementId> insertedElements;
    for (int ii = 0; ii < 10; ii++)
        {
        InsertElement();
        DgnElementId elementId = m_testElementId;
        insertedElements.insert(elementId);
        }

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    // Query changed elements directly using ECSQL
    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.GeometricElement el WHERE IsChangedInstance(?, el.GetECClassId(), el.ECInstanceId)";
    ECSqlStatus status = stmt.Prepare(*m_testDb, ecsql);
    ASSERT_TRUE(status.IsSuccess());
    
    stmt.BindInt64(1, (int64_t) &changeSummary);

    bset<DgnElementId> changedElements;
    DbResult stepStatus;
    while ((stepStatus = stmt.Step()) == BE_SQLITE_ROW)
        {
        changedElements.insert(stmt.GetValueId<DgnElementId>(0));
        }
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);
    ASSERT_EQ(insertedElements, changedElements);

    // Query elements changed due to changes to related geometry using ECSQL
    stmt.Finalize();
    ecsql = "SELECT el.ECInstanceId FROM dgn.GeometricElement el JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom WHERE IsChangedInstance(?, elg.GetECClassId(), elg.ECInstanceId)";
    status = stmt.Prepare(*m_testDb, ecsql);
    ASSERT_TRUE(status.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    changedElements.empty();
    while ((stepStatus = stmt.Step()) == BE_SQLITE_ROW)
        {
        changedElements.insert(stmt.GetValueId<DgnElementId>(0));
        }
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);
    ASSERT_EQ(insertedElements, changedElements);
    
    // Query changed elements directly using the API
    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    ECClassId elClassId = m_testDb->Schemas().GetECClassId("dgn", "Element");
    changeSummary.QueryByClass(changes, elClassId, true, ChangeSummary::QueryDbOpcode::All);
    changedElements.empty();
    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        changedElements.insert(DgnElementId(iter->first.GetValueUnchecked()));
        }
    ASSERT_EQ(insertedElements, changedElements);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryMultipleSessions)
    {
    CreateDgnDb();
    InsertModel();
    InsertCategory();
    m_testDb->SaveChanges();
    CloseDgnDb();

    int nSessions = 5;
    int nTransactionsPerSession = 5;
    for (int ii = 0; ii < nSessions; ii++)
        {
        OpenDgnDb();

        for (int jj = 0; jj < nTransactionsPerSession; jj++)
            {
            InsertElement();
            m_testDb->SaveChanges();
            }

        CloseDgnDb();
        }

    OpenDgnDb();

    TxnManager::TxnId startTxnId(TxnManager::SessionId(1), 0); // First session, First transaction
    ChangeSummary changeSummary(*m_testDb);
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(changeSummary, startTxnId);
    ASSERT_TRUE(status == DgnDbStatus::Success);

    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT COUNT(*) FROM dgn.Element el WHERE IsChangedInstance(?, el.GetECClassId(), el.ECInstanceId)";
    ECSqlStatus ecSqlStatus = stmt.Prepare(*m_testDb, ecsql);
    ASSERT_TRUE(ecSqlStatus.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    DbResult ecSqlStepStatus = stmt.Step();
    ASSERT_TRUE(ecSqlStepStatus == BE_SQLITE_ROW);

    int actualChangeCount = stmt.GetValueInt(0);
    int expectedChangeCount = nSessions * nTransactionsPerSession;
    ASSERT_EQ(expectedChangeCount, actualChangeCount);
    }
