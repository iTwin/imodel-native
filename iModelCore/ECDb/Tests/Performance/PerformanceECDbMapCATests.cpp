/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceECDbMapCATests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::ReadInstances(ECDbR ecdb)
    {
    //printf("Start profiling and press key tp continue..."); getchar();
    StopWatch timer(true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        Utf8CP selectSql = kvPair.second.m_selectECSql.c_str();

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, selectSql);
        ASSERT_EQ (ECSqlStatus::Success, stat) << "preparation failed for " << selectSql;
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, (int) (i+1)));
            ASSERT_EQ(ECSqlStepStatus::HasRow, stmt.Step()) << "step failed for " << selectSql;
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    //printf("Stop profiling and press key to continue..."); getchar();
    m_selectTime = timer.GetElapsedSeconds();
    LOG.infov("ECSQL SELECT %d instances each from %d classes took %.4f s.", m_instancesPerClass, m_sqlTestItems.size(), m_selectTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::DeleteInstances(ECDbR ecdb)
    {
    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        Utf8CP deleteSql = kvPair.second.m_deleteECSql.c_str();

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, deleteSql);
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Preparation Failed for " << deleteSql;
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, (int) (i + 1)));
            ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "step failed for " << deleteSql;
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_deleteTime = timer.GetElapsedSeconds ();
    LOG.infov("ECSQL DELETE %d instances each from %d classes took %.4f s.", m_instancesPerClass, m_sqlTestItems.size(), m_deleteTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::UpdateInstances(ECDbR ecdb)
    {
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP updateSql = kvPair.second.m_updateECSql.c_str();
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare(ecdb, updateSql);
        ASSERT_EQ(ECSqlStatus::Success, stat) << "preparation failed for " << updateSql;
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
                {
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(parameterIndex, ("UpdatedValue"), IECSqlBinder::MakeCopy::No));
                }

            ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(propertyCount + 1, instanceId++));
            ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "step failed for " << updateSql;
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_updateTime = timer.GetElapsedSeconds ();
    LOG.infov("ECSQL UPDATE %d instances each from %d classes took %.4f s.", m_instancesPerClass, m_sqlTestItems.size(), m_updateTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::InsertInstances(ECDbR ecdb)
    {
    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP insertSql = kvPair.second.m_insertECSql.c_str();
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, insertSql);
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Preparation failed for " << insertSql;
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, (int64_t) (i + 1)));

            for (int parameterIndex = 2; parameterIndex <= (propertyCount+1); parameterIndex++)
                {
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(parameterIndex, "Init Value", IECSqlBinder::MakeCopy::No));
                }

            ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "Step failed for " << insertSql;
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_insertTime = timer.GetElapsedSeconds ();
    LOG.infov("ECSQL INSERT %d instances each from %d classes took %.4f s.", m_instancesPerClass, m_sqlTestItems.size(), m_insertTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateECSqlCRUDTestStatements(ECSchemaR ecSchema)
    {
    for (ECClassCP testClass : ecSchema.GetClasses())
        {
        ECSqlTestItem& testItem = m_sqlTestItems[testClass];
        Utf8StringR insertSql = testItem.m_insertECSql;
        Utf8StringR selectSql = testItem.m_selectECSql;
        Utf8StringR updateSql = testItem.m_updateECSql;
        Utf8StringR deleteSql = testItem.m_deleteECSql;

        Utf8String className = ECSqlBuilder::ToECSqlSnippet(*testClass);

        insertSql = Utf8String("INSERT INTO ");
        insertSql.append(className).append(" (ECInstanceId");
        Utf8String insertValuesSql(") VALUES (?");

        selectSql = Utf8String("SELECT * FROM ONLY ");
        selectSql.append(className).append(" WHERE ECInstanceId=?");

        updateSql = Utf8String("UPDATE ONLY ");
        updateSql.append(className).append(" SET ");

        deleteSql = Utf8String("DELETE FROM ONLY ");
        deleteSql.append(className).append(" WHERE ECInstanceId=?");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties(true))
            {
            if (!isFirstItem)
                updateSql.append(",");

            insertSql.append(",").append(prop->GetName());
            insertValuesSql.append(",").append("?");

            updateSql.append(prop->GetName());
            updateSql.append("=? ");

            isFirstItem = false;
            }

        insertSql.append(insertValuesSql).append(")");
        updateSql.append("WHERE ECInstanceId=?");
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::CreatePrimitiveProperties(ECClassR ecClass)
    {
    PrimitiveECPropertyP primitiveP;
    for (size_t i = 0; i < m_propertiesPerClass; i++)
        {
        Utf8String temp;
        temp.Sprintf ("_Property%d", i);
        Utf8String propName = ecClass.GetName ();
        propName.append (temp);
        ecClass.CreatePrimitiveProperty (primitiveP, propName, PrimitiveType::PRIMITIVETYPE_String);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::CreateClassHierarchy(ECSchemaR testSchema, size_t LevelCount, ECClassR baseClass)
    {
    if (LevelCount == 0)
        return;

    ECClassP tmpClass;
    Utf8String className;
    className.Sprintf("P%d", m_classNamePostFix++);
    ASSERT_EQ(testSchema.CreateClass(tmpClass, className), ECOBJECTS_STATUS_Success);
    ASSERT_EQ(tmpClass->AddBaseClass(baseClass), ECOBJECTS_STATUS_Success);
    CreatePrimitiveProperties(*tmpClass);

    ECClassP tmpClass1;
    className.Sprintf("P%d", m_classNamePostFix++);
    ASSERT_EQ(testSchema.CreateClass(tmpClass1, className), ECOBJECTS_STATUS_Success);
    ASSERT_EQ(tmpClass1->AddBaseClass(baseClass), ECOBJECTS_STATUS_Success);
    CreatePrimitiveProperties(*tmpClass1);

    CreateClassHierarchy(testSchema, LevelCount - 1, *tmpClass);
    CreateClassHierarchy(testSchema, LevelCount - 1, *tmpClass1);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATestFixture, InstanceInsertionWithSharedColumnsForSubclasses)
    {
    m_instancesPerClass = 100;
    m_propertiesPerClass = 4;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDOperationWithSharedColumns.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");

    auto readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    auto ecdbmapKey = SchemaKey ("ECDbMap", 1, 0);
    auto ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());
    readContext->AddSchema (*testSchema);
    testSchema->AddReferencedSchema (*ecdbmapSchema);

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, "BaseClass"));
    CreatePrimitiveProperties (*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy (*testSchema, 7, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP ("ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Options", ECValue ("SharedColumnsForSubclasses")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.IsPolymorphic", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateECSqlCRUDTestStatements (*testSchema);
    InsertInstances (ecdb);
    ReadInstances (ecdb);
    UpdateInstances (ecdb);
    DeleteInstances (ecdb);

    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_insertTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Insert time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_selectTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "SELECT time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_updateTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Update time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_deleteTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Delete time"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATestFixture, InstanceInsertionWithOutSharedColumnsForSubClasses)
    {
    m_instancesPerClass = 100;
    m_propertiesPerClass = 4;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDOperationWithOutSharedColumnsForSubClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");

    auto readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    auto ecdbmapKey = SchemaKey ("ECDbMap", 1, 0);
    auto ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());
    readContext->AddSchema (*testSchema);
    testSchema->AddReferencedSchema (*ecdbmapSchema);

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, "BaseClass"));
    CreatePrimitiveProperties (*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy (*testSchema, 7, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP ("ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.IsPolymorphic", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateECSqlCRUDTestStatements (*testSchema);
    InsertInstances (ecdb);
    ReadInstances (ecdb);
    UpdateInstances (ecdb);
    DeleteInstances (ecdb);

    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_insertTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Insert time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_selectTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "SELECT time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_updateTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Update time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_deleteTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Delete time"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateSqlCRUDTestStatements (ECSchemaR ecSchema, ECClassR ecClass, Utf8StringR insertSql, Utf8StringR selectSql, Utf8StringR updateSql, Utf8StringR deleteSql)
    {
    Utf8String className = (Utf8String)ecClass.GetName ();

    insertSql = Utf8String ("INSERT INTO ts_");
    insertSql.append (className).append (" (");
    Utf8String insertValuesSql (") VALUES (");

    updateSql = Utf8String ("UPDATE ts_");
    updateSql.append (className).append (" SET ");

    selectSql = Utf8String ("SELECT * FROM ts_");
    selectSql.append (className).append (" WHERE ECInstanceId = ? ");

    deleteSql = Utf8String ("DELETE FROM ts_");
    deleteSql.append (className).append (" WHERE ECInstanceId = ? ");

    bool isFirstItem = true;
    for (auto prop : ecClass.GetProperties (true))
        {
        if (!isFirstItem)
            {
            insertSql.append (", ");
            insertValuesSql.append (", ");

            updateSql.append (", ");
            }

        insertSql.append (prop->GetName ());
        insertValuesSql.append ("?");

        updateSql.append (prop->GetName ());
        updateSql.append (" = ? ");

        isFirstItem = false;
        }

    insertSql.append (insertValuesSql).append (");");
    updateSql.append ("WHERE ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformanceSqlVsECSql)
    {
    m_instancesPerClass = 100000;
    m_propertiesPerClass = 100;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ECSqlStatementPerformanceTest.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, "BaseClass"));
    CreatePrimitiveProperties (*baseClass);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*testSchema);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache));
    ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());

    GenerateECSqlCRUDTestStatements (*testSchema);
    InsertInstances (ecdb);
    UpdateInstances(ecdb);
    ReadInstances(ecdb);
    DeleteInstances (ecdb);

    ASSERT_EQ(BE_SQLITE_OK, ecdb.AbandonChanges());

    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_insertTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "ECSql Insert Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_selectTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "ECSql Select Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_updateTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "ECSql Update Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_deleteTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "ECSql Delete Time"));

    //CRUD Performance using Sql statements.
    m_insertTime = m_updateTime = m_selectTime = m_deleteTime = 0.0;
    Utf8String insertSql;
    Utf8String updateSql;
    Utf8String selectSql;
    Utf8String deleteSql;
    GenerateSqlCRUDTestStatements (*testSchema, *baseClass, insertSql, selectSql, updateSql, deleteSql);

    //Insert Instance using Sql Query.
    const int propertyCount = (int)baseClass->GetPropertyCount (true);
    BeSQLite::Statement stmt;
    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, insertSql.c_str ())) << "Statement Prepare failed for " << insertSql.c_str ();
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindText (parameterIndex, "Init Value", BeSQLite::Statement::MakeCopy::No));
            }

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Step failed for " << insertSql.c_str();
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_insertTime = timer.GetElapsedSeconds ();
    LOG.infov("Scenario - INSERT INTO- 1 class [%d properties each] , %d instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_insertTime);

    //Update Instance using Sql Query.
    timer.Start ();
    int instanceCount = 1;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, updateSql.c_str ())) << "Statement Prepare failed for " << updateSql.c_str ();
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
            {
            ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindText(parameterIndex, "UpdatedValue", BeSQLite::Statement::MakeCopy::No));
            }

        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindInt (propertyCount + 1, instanceCount));
        instanceCount++;
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "step failed for " << updateSql.c_str ();
        ASSERT_EQ(1, ecdb.GetModifiedRowCount());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_updateTime = timer.GetElapsedSeconds ();
    LOG.infov("Scenario - UPDATE - 1 class [%d properties each] , %d instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_updateTime);

    //Read Instance using Sql Query.
    instanceCount = 1;
    timer.Start ();
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, selectSql.c_str ())) << "Statement Prepare failed for " << selectSql.c_str ();
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindInt (1, instanceCount));
        instanceCount++;
        ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ()) << "step failed for " << selectSql.c_str ();
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_selectTime = timer.GetElapsedSeconds ();
    stmt.Finalize ();
    LOG.infov("Scenario - Read - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_selectTime);

    //Delete Instance using Sql Query.
    instanceCount = 1;
    timer.Start ();
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, deleteSql.c_str ())) << "Statement Prepare failed for " << deleteSql.c_str ();
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindInt (1, instanceCount));
        instanceCount++;
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ()) << "step failed for " << deleteSql.c_str ();
        ASSERT_EQ(1, ecdb.GetModifiedRowCount());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_deleteTime = timer.GetElapsedSeconds ();
    LOG.infov("Scenario - DELETE - 1 class [%d properties each] , %d instances per class took - %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_deleteTime);

    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_insertTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "SQl Insert Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_selectTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "SQl SELECT Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_updateTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "SQl Update Time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_deleteTime, "PerformanceECDbMapCATestFixture.CRUDPerformanceSqlVsECSql", "SQl Delete Time"));
    }

END_ECDBUNITTESTS_NAMESPACE