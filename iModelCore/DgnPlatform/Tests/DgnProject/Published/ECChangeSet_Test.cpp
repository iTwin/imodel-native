/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ECChangeSet_Test.cpp $
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
//! ECChangeSetTestFixture
//=======================================================================================
struct ECChangeSetTestFixture : public GenericDgnModelTestFixture
    {
    protected:
        DgnDbPtr m_testDb;
        DgnModelPtr m_testModel;
        DgnCategoryId m_testCategoryId;
        DgnElementId m_testElementId;

        void CreateProject();
        void InsertModel();
        void InsertCategory();
        void InsertElement();
        void ModifyElement();
        void DeleteElement();

        void DumpSqlChangeSet(ChangeSet const& sqlChangeSet, Utf8CP label);
        void DumpECChangeSet(ECChangeSet const& ecChangeSet, Utf8CP label);

        void GetECChangeSetFromCurrentTransaction(ECChangeSet& ecChangeSet);
        void GetECChangeSetFromSavedTransactions(ECChangeSet& ecChangeSet);

        bool ECChangeSetHasChange(ECChangeSet const& ecChangeSet, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode);
        BentleyStatus ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb);

    public:
        ECChangeSetTestFixture() : GenericDgnModelTestFixture(__FILE__, true) {}
        virtual ~ECChangeSetTestFixture() {}
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
void ECChangeSetTestFixture::CreateProject()
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(true);

    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile(DgnDbTestDgnManager::GetOutputFilePath(L"ECChangeSetTest.dgndb"));

    DbResult createStatus;
    m_testDb = DgnDb::CreateDgnDb(&createStatus, DgnDbTestDgnManager::GetOutputFilePath(L"ECChangeSetTest.dgndb"), createProjectParams);
    ASSERT_TRUE(m_testDb.IsValid()) << "Could not create test project";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ECChangeSetTestFixture::InsertModel()
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
void ECChangeSetTestFixture::InsertCategory()
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
void ECChangeSetTestFixture::InsertElement()
    {
    PhysicalModelP physicalTestModel = dynamic_cast<PhysicalModelP> (m_testModel.get());
    BeAssert(physicalTestModel != nullptr);
    BeAssert(m_testCategoryId.IsValid());

    PhysicalElementPtr testElement = PhysicalElement::Create(*physicalTestModel, m_testCategoryId);
    testElement->SetCode("ChangeSetTestElementCode");
    testElement->SetLabel("ChangeSetTestElementLabel");

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
void ECChangeSetTestFixture::ModifyElement()
    {
    RefCountedPtr<PhysicalElement> testElement = m_testDb->Elements().GetForEdit<PhysicalElement>(m_testElementId);
    BeAssert(testElement.IsValid());

    testElement->SetCode("ModifiedElementCode");
    testElement->SetLabel("ModifiedElementLabel");

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ECChangeSetTestFixture::DeleteElement()
    {
    DgnDbStatus dbStatus = m_testDb->Elements().Delete(m_testElementId);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ECChangeSetTestFixture::DumpECChangeSet(ECChangeSet const& ecChangeSet, Utf8CP label)
    {
    printf("%s:\n", label);
    ecChangeSet.Dump();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ECChangeSetTestFixture::DumpSqlChangeSet(ChangeSet const& changeSet, Utf8CP label)
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
                    printf(v.Format(1000).c_str());

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
                    printf(v.Format(1000).c_str());

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
bool ECChangeSetTestFixture::ECChangeSetHasChange(ECChangeSet const& ecChangeSet, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
    {
    Utf8String tableName = ecChangeSet.GetChangeSetTableName();
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
BentleyStatus ECChangeSetTestFixture::ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb)
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
void ECChangeSetTestFixture::GetECChangeSetFromCurrentTransaction(ECChangeSet& ecChangeSet)
    {
    SqlChangeSet sqlChangeSet;
    DbResult result = sqlChangeSet.FromChangeTrack(m_testDb->Txns(), ChangeSet::SetType::Full);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    ecChangeSet.Free();
    BentleyStatus status = ecChangeSet.FromSqlChangeSet(sqlChangeSet);
    ASSERT_TRUE(SUCCESS == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ECChangeSetTestFixture::GetECChangeSetFromSavedTransactions(ECChangeSet& ecChangeSet)
    {
    DgnDbStatus status = m_testDb->Txns().GetECChangeSet(ecChangeSet, m_testDb->Txns().GetSessionStartId());
    ASSERT_TRUE(status == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, ElementChangesFromCurrentTransaction)
    {
    CreateProject();
    m_testDb->Txns().EnableTracking(true);

    ECChangeSet ecChangeSet(*m_testDb);

    m_testDb->SaveChanges("Test");
    InsertModel();
    InsertCategory();
    InsertElement();
    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserts");

    ECChangeSet after inserts:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;148;dgn:ElementGeom;Insert;No
    1;160;dgn:ElementOwnsGeom;Insert;No
    1;156;dgn:ElementIsInCategory;Insert;No
    1;167;dgn:ModelContainsElements;Insert;No
    1;169;dgn:PhysicalElement;Insert;No
    1;133;dgn:CategoryOwnsSubCategories;Insert;No
    1;134;dgn:SubCategory;Insert;No
    1;132;dgn:Category;Insert;No
    1;171;dgn:PhysicalModel;Insert;No
    */
    ASSERT_EQ(9, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(m_testModel->GetModelId().GetValueUnchecked()), "dgn", "PhysicalModel", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(m_testCategoryId.GetValueUnchecked()), "dgn", "Category", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Insert));

    m_testDb->SaveChanges("Test");
    ModifyElement();
    GetECChangeSetFromCurrentTransaction(ecChangeSet);
    
    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after updates");

    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;169;dgn:PhysicalElement;Update;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Update));

    m_testDb->SaveChanges();
    DeleteElement();
    GetECChangeSetFromCurrentTransaction(ecChangeSet);
    
    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after deletes");

    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;148;dgn:ElementGeom;Delete;Yes
    1;160;dgn:ElementOwnsGeom;Delete;Yes
    1;156;dgn:ElementIsInCategory;Delete;No
    1;167;dgn:ModelContainsElements;Delete;No
    1;169;dgn:PhysicalElement;Delete;No
    */
    ASSERT_EQ(5, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(m_testElementId.GetValueUnchecked()), "dgn", "PhysicalElement", DbOpcode::Delete));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, ElementChangesFromSavedTransactions)
    {
    CreateProject();
    m_testDb->Txns().EnableTracking(true);
    
    ECChangeSet ecChangeSet(*m_testDb);

    InsertModel();
    InsertCategory();
    InsertElement();

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /* 
    DumpECChangeSet(ecChangeSet, "After inserts");

    After inserts:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;171;dgn:PhysicalModel;Insert;No
    1;132;dgn:Category;Insert;No
    1;133;dgn:CategoryOwnsSubCategories;Insert;No
    1;134;dgn:SubCategory;Insert;No
    1;156;dgn:ElementIsInCategory;Insert;No
    1;167;dgn:ModelContainsElements;Insert;No
    1;169;dgn:PhysicalElement;Insert;No
    1;148;dgn:ElementGeom;Insert;No
    1;160;dgn:ElementOwnsGeom;Insert;No
    */
    ASSERT_EQ(9, ecChangeSet.MakeIterator().QueryCount());

    ModifyElement();

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "After updates");

    After updates:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;171;dgn:PhysicalModel;Insert;No
    1;132;dgn:Category;Insert;No
    1;133;dgn:CategoryOwnsSubCategories;Insert;No
    1;134;dgn:SubCategory;Insert;No
    1;156;dgn:ElementIsInCategory;Insert;No
    1;167;dgn:ModelContainsElements;Insert;No
    1;169;dgn:PhysicalElement;Insert;No
    1;148;dgn:ElementGeom;Insert;No
    1;160;dgn:ElementOwnsGeom;Insert;No
    */
    ASSERT_EQ(9, ecChangeSet.MakeIterator().QueryCount());

    DeleteElement();

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "After deletes");

    After deletes:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;171;dgn:PhysicalModel;Insert;No
    1;132;dgn:Category;Insert;No
    1;133;dgn:CategoryOwnsSubCategories;Insert;No
    1;134;dgn:SubCategory;Insert;No
    */
    ASSERT_EQ(4, ecChangeSet.MakeIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, ValidateIterator)
    {
    CreateProject();
    m_testDb->Txns().EnableTracking(true);

    InsertModel();
    InsertCategory();
    InsertElement();

    ECChangeSet ecChangeSet(*m_testDb);
    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    int countIter = 0;
    for (ECChangeSet::Iterator::const_iterator const& entry : ecChangeSet.MakeIterator())
        {
        countIter++;
        UNUSED_VARIABLE(entry);
        }
    ASSERT_EQ(countIter, 9);

    int countQuery = ecChangeSet.MakeIterator().QueryCount();
    ASSERT_EQ(countQuery, 9);
    }

extern ECSchemaPtr ReadECSchemaFromDisk(WCharCP schemaPathname);
extern bool ImportECSchema(ECSchemaR ecSchema, DgnDbR project);
extern IECInstancePtr CreateStartupCompanyInstance(ECSchemaCR startupSchema);

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, StructArrayChangesFromCurrentTransaction)
    {
    CreateProject();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    m_testDb->Txns().EnableTracking(true);

    ECChangeSet ecChangeSet(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting instance with struct array");

    ECChangeSet after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    m_testDb->SaveChanges("Test");

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_ArrayOfAnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after updating one row in the struct array table");

    ECChangeSet after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Update;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges("Test");

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_ArrayOfAnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after deleting one row in the struct array table");

    ECChangeSet after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Update;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges("Test");
    
    ECClassCP ecClass = ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    ECSqlStepStatus stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(ECSqlStepStatus::Done == stepStatus);

    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after deleting instance that contains a struct array");

    ECChangeSet after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Delete;Yes
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Delete));

    m_testDb->SaveChanges("Test");

    ecSqlStmt.Finalize();
    ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, "INSERT INTO StartupCompany.AnglesStruct (Alpha,Beta,Theta) VALUES(1.1,2.2,3.3)");
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ECInstanceKey structInstanceKey;
    stepStatus = ecSqlStmt.Step (structInstanceKey);
    ASSERT_TRUE(ECSqlStepStatus::Done == stepStatus);

    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting a plain struct that gets stored in a struct array table");

    ECChangeSet after inserting a plain struct that gets stored in a struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    5;197;StartupCompany:AnglesStruct;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, structInstanceKey.GetECInstanceId(), "StartupCompany", "AnglesStruct", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, StructArrayChangesFromSavedTransactions)
    {
    CreateProject();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    m_testDb->Txns().EnableTracking(true);

    ECChangeSet ecChangeSet(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting instance with struct array");

    ECChangeSet after inserting instance with struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_ArrayOfAnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after updating one row in the struct array table");

    ECChangeSet after updating one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_ArrayOfAnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);
    
    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after deleting one row in the struct array table");

    ECChangeSet after deleting one row in the struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;235;StartupCompany:Foo;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    ECClassCP ecClass = ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    ECSqlStepStatus stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(ECSqlStepStatus::Done == stepStatus);

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after deleting instance that contains a struct array");

    ECChangeSet after deleting instance that contains a struct array:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    */
    ASSERT_EQ(0, ecChangeSet.MakeIterator().QueryCount());

    ecSqlStmt.Finalize();
    ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, "INSERT INTO StartupCompany.AnglesStruct (Alpha,Beta,Theta) VALUES(1.1,2.2,3.3)");
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ECInstanceKey structInstanceKey;
    stepStatus = ecSqlStmt.Step(structInstanceKey);
    ASSERT_TRUE(ECSqlStepStatus::Done == stepStatus);

    m_testDb->SaveChanges("Test");

    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting a plain struct that gets stored in a struct array table");

    ECChangeSet after inserting a plain struct that gets stored in a struct array table:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    5;197;StartupCompany:AnglesStruct;Insert;No
    */
    ASSERT_EQ(1, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, structInstanceKey.GetECInstanceId(), "StartupCompany", "AnglesStruct", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    CreateProject();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    m_testDb->Txns().EnableTracking(true);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    ECSqlStepStatus stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    ECChangeSet ecChangeSet(*m_testDb);
    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting instances");

    ECChangeSet after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    3;219;StartupCompany:Hardware;Insert;No
    2;215;StartupCompany:Company;Insert;No
    1;224;StartupCompany:Employee;Insert;No
    */
    ASSERT_EQ(5, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_testDb->SaveChanges("Test");

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    ecChangeSet.Free();
    GetECChangeSetFromCurrentTransaction(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting relationships");

    ECChangeSet after inserting relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    6;229;StartupCompany:EmployeeHardware;Insert;No
    1;226;StartupCompany:EmployeeCompany;Insert;No
    */
    ASSERT_EQ(2, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_testDb->SaveChanges("Test");

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    ecChangeSet.Free();
    GetECChangeSetFromCurrentTransaction(ecChangeSet);
    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after updating (deleting and inserting different) relationships");

    ECChangeSet after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;226;StartupCompany:EmployeeCompany;Update;No
    6;229;StartupCompany:EmployeeHardware;Delete;Yes
    7;229;StartupCompany:EmployeeHardware;Insert;No
    */
    ASSERT_EQ(3, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Update));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ECChangeSetTestFixture, RelationshipChangesFromSavedTransaction)
    {
    CreateProject();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    m_testDb->Txns().EnableTracking(true);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    ECSqlStepStatus stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    m_testDb->SaveChanges("Test");

    ECChangeSet ecChangeSet(*m_testDb);
    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting instances");

    ECChangeSet after inserting instances:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    1;224;StartupCompany:Employee;Insert;No
    2;215;StartupCompany:Company;Insert;No
    3;215;StartupCompany:Company;Insert;No
    4;219;StartupCompany:Hardware;Insert;No
    5;219;StartupCompany:Hardware;Insert;No
    */
    ASSERT_EQ(5, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    m_testDb->SaveChanges("Test");

    ecChangeSet.Free();
    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after inserting relationships");

    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    6;229;StartupCompany:EmployeeHardware;Insert;No
    1;224;StartupCompany:Employee;Insert;No
    1;226;StartupCompany:EmployeeCompany;Insert;No
    2;215;StartupCompany:Company;Insert;No
    3;215;StartupCompany:Company;Insert;No
    4;219;StartupCompany:Hardware;Insert;No
    5;219;StartupCompany:Hardware;Insert;No
    */
    ASSERT_EQ(7, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    /*
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == ECSqlStepStatus::Done);

    m_testDb->SaveChanges("Test");

    ecChangeSet.Free();
    GetECChangeSetFromSavedTransactions(ecChangeSet);

    /*
    DumpECChangeSet(ecChangeSet, "ECChangeSet after updating (deleting and inserting different) relationships");

    ECChangeSet after updating (deleting and inserting different) relationships:
    InstanceId;ClassId;ClassName;DbOpcode;IsIndirect
    7;229;StartupCompany:EmployeeHardware;Insert;No
    1;224;StartupCompany:Employee;Insert;No
    1;226;StartupCompany:EmployeeCompany;Insert;No
    2;215;StartupCompany:Company;Insert;No
    3;215;StartupCompany:Company;Insert;No
    4;219;StartupCompany:Hardware;Insert;No
    5;219;StartupCompany:Hardware;Insert;No
    */
    ASSERT_EQ(7, ecChangeSet.MakeIterator().QueryCount());
    ASSERT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    ASSERT_TRUE(ECChangeSetHasChange(ecChangeSet, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }
