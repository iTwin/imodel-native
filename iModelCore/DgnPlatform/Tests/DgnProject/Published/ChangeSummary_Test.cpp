/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

// The counts recorded by change summary are quite sensitive to changes in the schema and implementation...
// Turn this on for debugging.
// #define DUMP_CHANGE_SUMMARY 1

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeSummaryTestFixture : ChangeTestFixture
    {
    DEFINE_T_SUPER(ChangeTestFixture)
    protected:
        WCharCP m_testFileName = L"ChangeSummaryTest.dgndb";

        void CreateDgnDb() { T_Super::CreateDgnDb(m_testFileName); }
        void OpenDgnDb() { T_Super::OpenDgnDb(m_testFileName); }

        void ModifyElement(DgnElementId elementId);
        void DeleteElement(DgnElementId elementId);

        void DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label);
        void DumpSqlChanges(DgnDbCR dgnDb, Changes const& sqlChanges, Utf8CP label);

        void GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary);
        void GetChangeSummaryFromSavedTransactions(ChangeSummary& changeSummary);

        bool ChangeSummaryContainsInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode);
        BentleyStatus ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb);

    public:
        virtual void SetUp() override {}
        virtual void TearDown() override { if (m_testDb.IsValid()) m_testDb->SaveChanges(); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_testDb->Elements().GetForEdit<PhysicalElement>(elementId);
    BeAssert(testElement.IsValid());

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DeleteElement(DgnElementId elementId)
    {
    DgnDbStatus dbStatus = m_testDb->Elements().Delete(elementId);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label)
    {
#ifdef DUMP_CHANGE_SUMMARY
    printf("%s:\n", label);
    changeSummary.Dump();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpSqlChanges(DgnDbCR dgnDb, Changes const& changes, Utf8CP label)
    {
    printf("%s:\n", label);
    for (Changes::Change change : changes)
        {
        Utf8CP tableName;
        int nCols, indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        bvector<Utf8String> columnNames;
        dgnDb.GetColumns(columnNames, tableName);
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
bool ChangeSummaryTestFixture::ChangeSummaryContainsInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
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
    AbortOnConflictChangeSet sqlChangeSet;
    DbResult result = sqlChangeSet.FromChangeTrack(m_testDb->Txns(), ChangeSet::SetType::Full);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    changeSummary.Free();
    BentleyStatus status = changeSummary.FromChangeSet(sqlChangeSet);
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
    DgnElementId elementId = InsertElement(0, 0, 0);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserts");

    /*
    ChangeSummary after inserts:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    2;191;dgn:PhysicalModel;Insert;No
	    Code;NULL;"ChangeSetModel"
	    CodeAuthorityId;NULL;6
	    CodeNameSpace;NULL;""
	    Props;NULL;"{"azimuth":-9.2559631349317831e+061,"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
	    Visibility;NULL;1
    2;153;dgn:Category;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;""
	    Descr;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    Rank;NULL;2
	    Scope;NULL;1
    3;204;dgn:SubCategory;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;"ChangeSetTestCategory"
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    ParentId;NULL;2
	    Props;NULL;"{"color":16777215}"
    4;139;dgn:PhysicalElement;Insert;No
	    CategoryId;NULL;2
	    CodeAuthorityId;NULL;1
	    CodeNameSpace;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;2
    4;163;dgn:ElementGeom;Insert;No
	    Geom;NULL;...
	    InPhysicalSpace;NULL;1
	    Placement;NULL;...
    3;172;dgn:ElementOwnsChildElements;Insert;No
	    SourceECClassId;NULL;204
	    SourceECInstanceId;NULL;2
	    TargetECClassId;NULL;204
	    TargetECInstanceId;NULL;3
    4;175;dgn:ElementOwnsGeom;Insert;No
	    SourceECClassId;NULL;139
	    SourceECInstanceId;NULL;4
	    TargetECClassId;NULL;163
	    TargetECInstanceId;NULL;4
    */
    EXPECT_EQ(8, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(m_testModel->GetModelId().GetValueUnchecked()), "dgn", "PhysicalModel", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(m_testCategoryId.GetValueUnchecked()), "dgn", "Category", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Insert));

    m_testDb->SaveChanges();
    ModifyElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after updates");

    /*
    ChangeSummary after updates:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    4;139;dgn:PhysicalElement;Update;No
	    LastMod;2.45733e+006;2.45733e+006
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Update));

    m_testDb->SaveChanges();
    DeleteElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after deletes");

    /*
    ChangeSummary after deletes:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    4;139;dgn:PhysicalElement;Delete;No
	    CategoryId;2;NULL
	    CodeAuthorityId;1;NULL
	    CodeNameSpace;"";NULL
	    LastMod;2.45733e+006;NULL
	    ModelId;2;NULL
    4;163;dgn:ElementGeom;Delete;Yes
	    Geom;...;NULL
	    InPhysicalSpace;1;NULL
	    Placement;...;NULL
    4;175;dgn:ElementOwnsGeom;Delete;Yes
	    SourceECClassId;139;NULL
	    SourceECInstanceId;4;NULL
	    TargetECClassId;163;NULL
	    TargetECInstanceId;4;NULL
    */
    EXPECT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Delete));
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
    DgnElementId elementId = InsertElement(0, 0, 0);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After inserts");

    /* 
    After inserts:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    2;191;dgn:PhysicalModel;Insert;No
	    Code;NULL;"ChangeSetModel"
	    CodeAuthorityId;NULL;6
	    CodeNameSpace;NULL;""
	    Props;NULL;"{"azimuth":-9.2559631349317831e+061,"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
	    Visibility;NULL;1
    2;153;dgn:Category;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;""
	    Descr;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    Rank;NULL;2
	    Scope;NULL;1
    3;204;dgn:SubCategory;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;"ChangeSetTestCategory"
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    ParentId;NULL;2
	    Props;NULL;"{"color":16777215}"
    4;139;dgn:PhysicalElement;Insert;No
	    CategoryId;NULL;2
	    CodeAuthorityId;NULL;1
	    CodeNameSpace;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;2
    4;163;dgn:ElementGeom;Insert;No
	    Geom;NULL;...
	    InPhysicalSpace;NULL;1
	    Placement;NULL;...
    3;172;dgn:ElementOwnsChildElements;Insert;No
	    SourceECClassId;NULL;204
	    SourceECInstanceId;NULL;2
	    TargetECClassId;NULL;204
	    TargetECInstanceId;NULL;3
    4;175;dgn:ElementOwnsGeom;Insert;No
	    SourceECClassId;NULL;139
	    SourceECInstanceId;NULL;4
	    TargetECClassId;NULL;163
	    TargetECInstanceId;NULL;4
    */
    EXPECT_EQ(8, changeSummary.MakeInstanceIterator().QueryCount());

    ModifyElement(elementId);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After updates");

    /*
    After updates:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    2;191;dgn:PhysicalModel;Insert;No
	    Code;NULL;"ChangeSetModel"
	    CodeAuthorityId;NULL;6
	    CodeNameSpace;NULL;""
	    Props;NULL;"{"azimuth":-9.2559631349317831e+061,"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
	    Visibility;NULL;1
    2;153;dgn:Category;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;""
	    Descr;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    Rank;NULL;2
	    Scope;NULL;1
    3;204;dgn:SubCategory;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;"ChangeSetTestCategory"
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    ParentId;NULL;2
	    Props;NULL;"{"color":16777215}"
    4;139;dgn:PhysicalElement;Insert;No
	    CategoryId;NULL;2
	    CodeAuthorityId;NULL;1
	    CodeNameSpace;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;2
    4;163;dgn:ElementGeom;Insert;No
	    Geom;NULL;...
	    InPhysicalSpace;NULL;1
	    Placement;NULL;...
    3;172;dgn:ElementOwnsChildElements;Insert;No
	    SourceECClassId;NULL;204
	    SourceECInstanceId;NULL;2
	    TargetECClassId;NULL;204
	    TargetECInstanceId;NULL;3
    4;175;dgn:ElementOwnsGeom;Insert;No
	    SourceECClassId;NULL;139
	    SourceECInstanceId;NULL;4
	    TargetECClassId;NULL;163
	    TargetECInstanceId;NULL;4
    */
    EXPECT_EQ(8, changeSummary.MakeInstanceIterator().QueryCount());

    DeleteElement(elementId);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After deletes");

    /*
    After deletes:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    2;191;dgn:PhysicalModel;Insert;No
	    Code;NULL;"ChangeSetModel"
	    CodeAuthorityId;NULL;6
	    CodeNameSpace;NULL;""
	    Props;NULL;"{"azimuth":-9.2559631349317831e+061,"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}"
	    Visibility;NULL;1
    2;153;dgn:Category;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;""
	    Descr;NULL;""
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    Rank;NULL;2
	    Scope;NULL;1
    3;204;dgn:SubCategory;Insert;No
	    Code;NULL;"ChangeSetTestCategory"
	    CodeAuthorityId;NULL;3
	    CodeNameSpace;NULL;"ChangeSetTestCategory"
	    LastMod;NULL;2.45733e+006
	    ModelId;NULL;1
	    ParentId;NULL;2
	    Props;NULL;"{"color":16777215}"
    3;172;dgn:ElementOwnsChildElements;Insert;No
	    SourceECClassId;NULL;204
	    SourceECInstanceId;NULL;2
	    TargetECClassId;NULL;204
	    TargetECInstanceId;NULL;3
    */
    EXPECT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ValidateInstanceIterator)
    {
    CreateDgnDb();

    InsertModel();
    InsertCategory();
    InsertElement(0, 0, 0);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    int countIter = 0;
    for (ChangeSummary::InstanceIterator::const_iterator const& entry : changeSummary.MakeInstanceIterator())
        {
        countIter++;
        UNUSED_VARIABLE(entry);
        }
    EXPECT_EQ(countIter, 8);

    int countQuery = changeSummary.MakeInstanceIterator().QueryCount();
    EXPECT_EQ(countQuery, 8);
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
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    /*
    ChangeSummary after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Insert;No
        anglesFoo.Alpha;NULL;12.345
        anglesFoo.Beta;NULL;12.345
        arrayOfIntsFoo;NULL;...
        doubleFoo;NULL;12.345
        intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    m_testDb->SaveChanges();

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_AnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    /*
    ChangeSummary after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Update;No
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges();

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_AnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    /*
    ChangeSummary after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Update;No
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    /*
    ChangeSummary after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Delete;No
            anglesFoo.Alpha;12.345;NULL
            anglesFoo.Beta;12.345;NULL
            arrayOfIntsFoo;...;NULL
            doubleFoo;12.345;NULL
            intFoo;67;NULL
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Delete));

    m_testDb->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, StructArrayChangesFromSavedTransactions)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    /*
    ChangeSummary after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Insert;No
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_AnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    /*
    ChangeSummary after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Insert;No
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_AnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    /*
    ChangeSummary after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;261;StartupCompany:Foo;Insert;No
            anglesFoo.Alpha;NULL;12.345
            anglesFoo.Beta;NULL;12.345
            arrayOfIntsFoo;NULL;...
            doubleFoo;NULL;12.345
            intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    /*
    ChangeSummary after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    */
    EXPECT_EQ(0, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
    ChangeSummary after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;250;StartupCompany:Employee;Insert;No
            FirstName;NULL;"John"
            LastName;NULL;"Doe"
    2;241;StartupCompany:Company;Insert;No
            Name;NULL;"AcmeWorks"
    3;241;StartupCompany:Company;Insert;No
            Name;NULL;"CmeaWorks"
    4;245;StartupCompany:Hardware;Insert;No
            Make;NULL;"Tesla"
            Model;NULL;"Model-S"
    5;245;StartupCompany:Hardware;Insert;No
            Make;NULL;"Toyota"
            Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
    ChangeSummary after inserting relationships:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;252;StartupCompany:EmployeeCompany;Insert;No
            SourceECClassId;250;250
            SourceECInstanceId;1;1
            TargetECClassId;NULL;241
            TargetECInstanceId;NULL;2
    6;255;StartupCompany:EmployeeHardware;Insert;No
            SourceECClassId;NULL;250
            SourceECInstanceId;NULL;1
            TargetECClassId;NULL;245
            TargetECInstanceId;NULL;4
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
    ChangeSummary after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    6;255;StartupCompany:EmployeeHardware;Delete;No
	    SourceECClassId;250;NULL
	    SourceECInstanceId;1;NULL
	    TargetECClassId;245;NULL
	    TargetECInstanceId;4;NULL
    7;255;StartupCompany:EmployeeHardware;Insert;No
	    SourceECClassId;NULL;250
	    SourceECInstanceId;NULL;1
	    TargetECClassId;NULL;245
	    TargetECInstanceId;NULL;5
    1;252;StartupCompany:EmployeeCompany;Update;No
	    SourceECClassId;250;250
	    SourceECInstanceId;1;1
	    TargetECClassId;241;241
	    TargetECInstanceId;2;3
    */
    EXPECT_EQ(3, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Update));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromSavedTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
    ChangeSummary after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;250;StartupCompany:Employee;Insert;No
	    FirstName;NULL;"John"
	    LastName;NULL;"Doe"
    2;241;StartupCompany:Company;Insert;No
	    Name;NULL;"AcmeWorks"
    3;241;StartupCompany:Company;Insert;No
	    Name;NULL;"CmeaWorks"
    4;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Tesla"
	    Model;NULL;"Model-S"
    5;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Toyota"
	    Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
    ChangeSummary after inserting relationships:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;250;StartupCompany:Employee;Insert;No
	    FirstName;NULL;"John"
	    LastName;NULL;"Doe"
    2;241;StartupCompany:Company;Insert;No
	    Name;NULL;"AcmeWorks"
    3;241;StartupCompany:Company;Insert;No
	    Name;NULL;"CmeaWorks"
    4;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Tesla"
	    Model;NULL;"Model-S"
    5;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Toyota"
	    Model;NULL;"Prius"
    1;252;StartupCompany:EmployeeCompany;Insert;No
	    SourceECClassId;NULL;250
	    SourceECInstanceId;NULL;1
	    TargetECClassId;NULL;241
	    TargetECInstanceId;NULL;2
    6;255;StartupCompany:EmployeeHardware;Insert;No
	    SourceECClassId;NULL;250
	    SourceECInstanceId;NULL;1
	    TargetECClassId;NULL;245
	    TargetECInstanceId;NULL;4
    */
    EXPECT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

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

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
    ChangeSummary after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    1;250;StartupCompany:Employee;Insert;No
	    FirstName;NULL;"John"
	    LastName;NULL;"Doe"
    2;241;StartupCompany:Company;Insert;No
	    Name;NULL;"AcmeWorks"
    3;241;StartupCompany:Company;Insert;No
	    Name;NULL;"CmeaWorks"
    4;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Tesla"
	    Model;NULL;"Model-S"
    5;245;StartupCompany:Hardware;Insert;No
	    Make;NULL;"Toyota"
	    Model;NULL;"Prius"
    1;252;StartupCompany:EmployeeCompany;Insert;No
	    SourceECClassId;NULL;250
	    SourceECInstanceId;NULL;1
	    TargetECClassId;NULL;241
	    TargetECInstanceId;NULL;3
    7;255;StartupCompany:EmployeeHardware;Insert;No
	    SourceECClassId;NULL;250
	    SourceECInstanceId;NULL;1
	    TargetECClassId;NULL;245
	    TargetECInstanceId;NULL;5
    */
    EXPECT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChildRelationshipChanges)
    {
    CreateDgnDb();

    InsertModel();
    InsertCategory();
    DgnElementId parentElementId = InsertElement(0, 0, 0);
    DgnElementId childElementId = InsertElement(1, 1, 1);

    m_testDb->SaveChanges();

    RefCountedPtr<DgnElement> childElementPtr = m_testDb->Elements().GetForEdit<DgnElement>(childElementId);
    
    DgnDbStatus dbStatus = childElementPtr->SetParentId(parentElementId);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);
    
    childElementPtr->Update(&dbStatus);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after setting ParentId");

    /*
    ChangeSummary after setting ParentId:
    InstanceId;ClassId;ClassName;DbOpcode;Indirect
    5;139;dgn:PhysicalElement;Update;No
            LastMod;2.45733e+006;2.45733e+006
            ParentId;NULL;4
    5;172;dgn:ElementOwnsChildElements;Insert;No
            SourceECClassId;NULL;139
            SourceECInstanceId;NULL;4
            TargetECClassId;NULL;139
            TargetECInstanceId;NULL;5
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), "dgn", "ElementOwnsChildElements", DbOpcode::Insert)); // Captured due to change of FK relationship (ParentId column)
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Update)); // Captured due to change of ParentId property

    ECClassId relClassId = m_testDb->Schemas().GetECClassId("dgn", "ElementOwnsChildElements");
    ECClassId elClassId = m_testDb->Schemas().GetECClassId("dgn", "PhysicalElement");

    ChangeSummary::Instance instance = changeSummary.GetInstance(elClassId, childElementId);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary::Instance relInstance = changeSummary.GetInstance(relClassId, childElementId);
    ASSERT_TRUE(relInstance.IsValid());

    DbDupValue value(nullptr);

    value = relInstance.GetNewValue("SourceECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId, value.GetValueInt64());
    EXPECT_EQ(elClassId, relInstance.GetNewValue("SourceECClassId").GetValueInt64());

    value = relInstance.GetNewValue("SourceECInstanceId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), relInstance.GetNewValue("SourceECInstanceId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId, value.GetValueInt64());
    EXPECT_EQ(elClassId, relInstance.GetNewValue("TargetECClassId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECInstanceId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(childElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(childElementId.GetValueUnchecked(), relInstance.GetNewValue("TargetECInstanceId").GetValueInt64());

    value = instance.GetNewValue("ParentId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), instance.GetNewValue("ParentId").GetValueInt64());

    EXPECT_EQ(4, relInstance.MakeValueIterator(changeSummary).QueryCount());
    EXPECT_EQ(2, instance.MakeValueIterator(changeSummary).QueryCount());
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
        DgnElementId elementId = InsertElement(ii, 0, 0);
        insertedElements.insert(elementId);
        }

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    // Query changed elements directly using ECSQL
    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.PhysicalElement el WHERE IsChangedInstance(?, el.GetECClassId(), el.ECInstanceId)";
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
    EXPECT_EQ(insertedElements, changedElements);

    // Query elements changed due to changes to related geometry using ECSQL
    stmt.Finalize();
    ecsql = "SELECT el.ECInstanceId FROM dgn.Element el JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom WHERE IsChangedInstance(?, elg.GetECClassId(), elg.ECInstanceId)";
    status = stmt.Prepare(*m_testDb, ecsql);
    ASSERT_TRUE(status.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    changedElements.empty();
    while ((stepStatus = stmt.Step()) == BE_SQLITE_ROW)
        {
        changedElements.insert(stmt.GetValueId<DgnElementId>(0));
        }
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);
    EXPECT_EQ(insertedElements, changedElements);
    
    // Query changed elements directly using the API
    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    ECClassId elClassId = m_testDb->Schemas().GetECClassId("dgn", "Element");
    changeSummary.QueryByClass(changes, elClassId, true, ChangeSummary::QueryDbOpcode::All);
    changedElements.empty();
    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        changedElements.insert(DgnElementId(iter->first.GetValueUnchecked()));
        }
    EXPECT_EQ(insertedElements.size()+2, changedElements.size()); // category and sub-category...
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
            InsertElement(ii, jj, 0);
            m_testDb->SaveChanges();
            }

        CloseDgnDb();
        }

    OpenDgnDb();

    TxnManager::TxnId startTxnId(TxnManager::SessionId(1), 0); // First session, First transaction
    ChangeSummary changeSummary(*m_testDb);
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(changeSummary, startTxnId);
    ASSERT_TRUE(status == DgnDbStatus::Success);

    DumpChangeSummary(changeSummary, "ChangeSummary after multiple sessions");

    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT COUNT(*) FROM dgn.Element el WHERE IsChangedInstance(?, el.GetECClassId(), el.ECInstanceId)";
    ECSqlStatus ecSqlStatus = stmt.Prepare(*m_testDb, ecsql);
    ASSERT_TRUE(ecSqlStatus.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    DbResult ecSqlStepStatus = stmt.Step();
    ASSERT_TRUE(ecSqlStepStatus == BE_SQLITE_ROW);

    int actualChangeCount = stmt.GetValueInt(0);
    int expectedChangeCount = nSessions * nTransactionsPerSession + 2 /*category and subcategory*/;
    EXPECT_EQ(expectedChangeCount, actualChangeCount);
    }
