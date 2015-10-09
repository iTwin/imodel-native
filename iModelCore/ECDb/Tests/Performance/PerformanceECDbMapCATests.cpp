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
void PerformanceECDbMapCATestFixture::ReadInstances (ECDbR ecdb)
    {
    m_selectTime = -1.0;
    int instanceId = 1;
    //printf("Start profiling and press key tp continue..."); getchar();
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        Utf8CP selectECSql = kvPair.second.m_selectECSql.c_str ();
        LOG.infov ("Select ECSql statement %s", selectECSql);
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, selectECSql);
        LOG.infov ("Back-end Read Sql %s", stmt.GetNativeSql ());
        if (!stat.IsSuccess ())
            return;

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (!stmt.BindInt64 (1, (int64_t)(instanceId++)).IsSuccess ())
                {
                FAIL ();
                return;
                }

            DbResult stat = stmt.Step ();
            if (stat != BE_SQLITE_ROW && stat != BE_SQLITE_DONE)
                {
                FAIL ();
                return;
                }

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    //printf("Stop profiling and press key to continue..."); getchar();
    m_selectTime = timer.GetElapsedSeconds ();
    LOG.infov ("ECSQL SELECT %d instances each against %d classes with %d Properties took %.4f s.", m_instancesPerClass, m_ecsqlTestItems.size (), m_propertiesPerClass, m_selectTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::DeleteInstances (ECDbR ecdb)
    {
    m_deleteTime = -1.0;
    int instanceId = 1;
    //printf("Start profiling and press key tp continue..."); getchar();
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        Utf8CP deleteECSql = kvPair.second.m_deleteECSql.c_str ();
        LOG.infov ("Delete ECSql statement %s", deleteECSql);
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, deleteECSql);
        LOG.infov ("Back-end Delete Sql %s", stmt.GetNativeSql ());
        if (!stat.IsSuccess ())
            {
            ASSERT_TRUE (false);
            return;
            }

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (!stmt.BindInt64 (1, (int64_t)(instanceId++)).IsSuccess ())
                {
                ASSERT_TRUE (false);
                return;
                }

            if (BE_SQLITE_DONE != stmt.Step () || ecdb.GetModifiedRowCount () == 0)
                {
                ASSERT_TRUE (false);
                return;
                }

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    //printf("Stop profiling and press key to continue..."); getchar();
    m_deleteTime = timer.GetElapsedSeconds ();
    LOG.infov ("ECSQL DELETE %d instances each against %d classes with %d Properties took %.4f s.", m_instancesPerClass, m_ecsqlTestItems.size (), m_propertiesPerClass, m_deleteTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::UpdateInstances (ECDbR ecdb, bool BindPropertyValues)
    {
    m_updateTime = -1.0;
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP updateECSql = kvPair.second.m_updateECSql.c_str ();
        LOG.infov ("Update ECSql statement %s", updateECSql);
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, updateECSql);
        LOG.infov ("Back-end update Sql %s", stmt.GetNativeSql ());
        if (!stat.IsSuccess ())
            return;

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (BindPropertyValues)
                {
                for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
                    {
                    if (!stmt.BindText (parameterIndex, ("UpdatedValue"), IECSqlBinder::MakeCopy::No).IsSuccess ())
                        {
                        ASSERT_TRUE (false);
                        return;
                        }
                    }
                if (!stmt.BindInt64 (propertyCount + 1, (int64_t)(instanceId++)).IsSuccess ())
                    {
                    ASSERT_TRUE (false);
                    return;
                    }
                }
            else
                {
                if (!stmt.BindInt64 (1, (int64_t)(instanceId++)).IsSuccess ())
                    {
                    ASSERT_TRUE (false);
                    return;
                    }
                }

            if (BE_SQLITE_DONE != stmt.Step () || ecdb.GetModifiedRowCount () == 0)
                {
                ASSERT_TRUE (false);
                return;
                }

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_updateTime = timer.GetElapsedSeconds ();
    LOG.infov ("ECSQL UPDATE %d instances each against %d classes with %d Properties took %.4f s.", m_instancesPerClass, m_ecsqlTestItems.size (), m_propertiesPerClass, m_updateTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::InsertInstances (ECDbR ecdb, bool BindPropertyValues)
    {
    m_insertTime = -1.0;
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP insertECSql = kvPair.second.m_insertECSql.c_str ();
        LOG.infov ("Insert ECSql statement %s", insertECSql);
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, insertECSql);
        LOG.infov ("Back-end Insert Sql %s", stmt.GetNativeSql ());
        if (!stat.IsSuccess ())
            {
            ASSERT_TRUE (false);
            return;
            }

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (!stmt.BindInt64 (1, (int64_t)(instanceId++)).IsSuccess ())
                {
                ASSERT_TRUE (false);
                return;
                }

            if (BindPropertyValues)
                {
                for (int parameterIndex = 2; parameterIndex <= (propertyCount + 1); parameterIndex++)
                    {
                    if (!stmt.BindText (parameterIndex, "Init Value", IECSqlBinder::MakeCopy::No).IsSuccess ())
                        {
                        ASSERT_TRUE (false);
                        return;
                        }
                    }
                }

            if (BE_SQLITE_DONE != stmt.Step () || ecdb.GetModifiedRowCount () == 0)
                {
                ASSERT_TRUE (false);
                return;
                }

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_insertTime = timer.GetElapsedSeconds ();
    LOG.infov ("ECSQL INSERT %d instances each against %d classes with %d Properties took %.4f s.", m_instancesPerClass, m_ecsqlTestItems.size (), m_propertiesPerClass, m_insertTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateECSqlCRUDTestStatements (ECSchemaR ecSchema, bool hardCodedPropertyValues)
    {
    for (ECClassCP testClass : ecSchema.GetClasses ())
        {
        ECSqlTestItem& testItem = m_ecsqlTestItems[testClass];
        Utf8StringR insertECSql = testItem.m_insertECSql;
        Utf8StringR selectECSql = testItem.m_selectECSql;
        Utf8StringR updateECSql = testItem.m_updateECSql;
        Utf8StringR deleteECSql = testItem.m_deleteECSql;

        Utf8String className = ECSqlBuilder::ToECSqlSnippet (*testClass);

        insertECSql = Utf8String ("INSERT INTO ");
        insertECSql.append (className).append (" (ECInstanceId, ");
        Utf8String insertValuesSql (") VALUES (?, ");

        selectECSql = Utf8String ("SELECT ");

        updateECSql = Utf8String ("UPDATE ONLY ");
        updateECSql.append (className).append (" SET ");

        deleteECSql = Utf8String ("DELETE FROM ONLY ");
        deleteECSql.append (className).append (" WHERE ECInstanceId=?");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties (true))
            {
            if (!isFirstItem)
                {
                selectECSql.append (", ");

                insertECSql.append (", ");
                insertValuesSql.append (", ");

                updateECSql.append (",");
                }

            selectECSql.append (prop->GetName ());

            if (hardCodedPropertyValues)
                {
                insertECSql.append (prop->GetName ());
                insertValuesSql.append ("'InitValue'");

                updateECSql.append (prop->GetName ());
                updateECSql.append (" = 'UpdatedValue' ");
                }
            else
                {
                insertECSql.append (prop->GetName ());
                insertValuesSql.append ("?");

                updateECSql.append (prop->GetName ());
                updateECSql.append ("=? ");
                }

            isFirstItem = false;
            }

        selectECSql.append (" FROM ONLY ").append (className).append (" WHERE ECInstanceId=?");
        insertECSql.append (insertValuesSql).append (")");
        updateECSql.append ("WHERE ECInstanceId=?");
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::CreatePrimitiveProperties (ECClassR ecClass)
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
void PerformanceECDbMapCATestFixture::CreateClassHierarchy (ECSchemaR testSchema, size_t LevelCount, ECClassR baseClass)
    {
    if (LevelCount == 0)
        return;

    ECClassP tmpClass;
    Utf8String className;
    className.Sprintf ("P%d", m_classNamePostFix++);
    ASSERT_EQ (testSchema.CreateClass (tmpClass, className), ECOBJECTS_STATUS_Success);
    ASSERT_EQ (tmpClass->AddBaseClass (baseClass), ECOBJECTS_STATUS_Success);
    CreatePrimitiveProperties (*tmpClass);

    ECClassP tmpClass1;
    className.Sprintf ("P%d", m_classNamePostFix++);
    ASSERT_EQ (testSchema.CreateClass (tmpClass1, className), ECOBJECTS_STATUS_Success);
    ASSERT_EQ (tmpClass1->AddBaseClass (baseClass), ECOBJECTS_STATUS_Success);
    CreatePrimitiveProperties (*tmpClass1);

    CreateClassHierarchy (testSchema, LevelCount - 1, *tmpClass);
    CreateClassHierarchy (testSchema, LevelCount - 1, *tmpClass1);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformance_SharedTable_SharedColumnsForSubClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDPerformance_SharedTable_SharedColumnsForSubClasses.ecdb");

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
    CreateClassHierarchy (*testSchema, hierarchyLevel, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP ("ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Options", ECValue ("SharedColumnsForSubclasses")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.AppliesToSubclasses", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateECSqlCRUDTestStatements (*testSchema, false);
    InsertInstances (ecdb, true);
    ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
    ReadInstances (ecdb);
    ASSERT_GE (m_selectTime, 0.0) << "ECSQL SELECT test failed";
    UpdateInstances (ecdb, true);
    ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    DeleteInstances (ecdb);
    ASSERT_GE (m_deleteTime, 0.0) << "ECSQL DELETE test failed";

    LOGTODB (TEST_DETAILS, m_insertTime, "Insert time", (int)m_instancesPerClass);
    LOGTODB (TEST_DETAILS, m_selectTime, "Select time", (int)m_instancesPerClass);
    LOGTODB (TEST_DETAILS, m_updateTime, "Update time", (int)m_instancesPerClass);
    LOGTODB (TEST_DETAILS, m_deleteTime, "Delete time", (int)m_instancesPerClass);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformance_SharedTableForSubClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDPerformance_SharedTableForSubClasses.ecdb");

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
    CreateClassHierarchy (*testSchema, hierarchyLevel, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP ("ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.AppliesToSubclasses", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateECSqlCRUDTestStatements (*testSchema, false);
    InsertInstances (ecdb, true);
    ReadInstances (ecdb);
    UpdateInstances (ecdb, true);
    DeleteInstances (ecdb);

    LOGTODB (TEST_DETAILS, m_insertTime, "Insert time");
    LOGTODB (TEST_DETAILS, m_selectTime, "Select time");
    LOGTODB (TEST_DETAILS, m_updateTime, "Update time");
    LOGTODB (TEST_DETAILS, m_deleteTime, "Delete time");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformance_DefaultClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDPerformance_DefaultClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");

    auto readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchema (*testSchema);

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, "BaseClass"));
    CreatePrimitiveProperties (*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy (*testSchema, hierarchyLevel, *baseClass);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateECSqlCRUDTestStatements (*testSchema, false);
    InsertInstances (ecdb, true);
    ReadInstances (ecdb);
    UpdateInstances (ecdb, true);
    DeleteInstances (ecdb);

    LOGTODB (TEST_DETAILS, m_insertTime, "Insert time");
    LOGTODB (TEST_DETAILS, m_selectTime, "Select time");
    LOGTODB (TEST_DETAILS, m_updateTime, "Update time");
    LOGTODB (TEST_DETAILS, m_deleteTime, "Delete time");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateSqlCRUDTestStatements (ECSchemaR ecSchema, ECClassR ecClass, Utf8StringR insertSql, Utf8StringR selectSql, Utf8StringR updateSql, Utf8StringR deleteSql, bool hardCodePropertyValues)
    {
    Utf8String className = (Utf8String)ecClass.GetName ();

    insertSql = Utf8String ("INSERT INTO ts_");
    insertSql.append (className).append (" (ECInstanceId, ");
    Utf8String insertValuesSql (") VALUES (?, ");

    updateSql = Utf8String ("UPDATE ts_");
    updateSql.append (className).append (" SET ");

    selectSql = Utf8String ("SELECT ");

    //use view here to make sure referential integrity is ensured. Otherwise it is not comparable to ECSQL.
    deleteSql = Utf8String ("DELETE FROM ts_");
    deleteSql.append (className).append (" WHERE ECInstanceId = ? ");

    bool isFirstItem = true;
    for (auto prop : ecClass.GetProperties (true))
        {
        if (!isFirstItem)
            {
            selectSql.append (", ");

            insertSql.append (", ");
            insertValuesSql.append (", ");

            updateSql.append (", ");
            }

        selectSql.append (prop->GetName ());

        if (hardCodePropertyValues)
            {
            insertSql.append (prop->GetName ());
            insertValuesSql.append ("'InitValue'");

            updateSql.append (prop->GetName ());
            updateSql.append (" = 'UpdatedValue' ");
            }
        else
            {
            insertSql.append (prop->GetName ());
            insertValuesSql.append ("?");

            updateSql.append (prop->GetName ());
            updateSql.append (" = ? ");
            }

        isFirstItem = false;
        }

    selectSql.append (" FROM ts_");
    selectSql.append (className).append (" WHERE ECInstanceId = ? ");
    insertSql.append (insertValuesSql).append (");");
    updateSql.append ("WHERE ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformanceSqlVsECSql)
    {
    m_instancesPerClass = 100000;
    m_propertiesPerClass = 150;
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
    ASSERT_EQ (BE_SQLITE_OK, ecdb.SaveChanges ());

    GenerateECSqlCRUDTestStatements (*testSchema, false);

    InsertInstances (ecdb, true);
    ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
    ReadInstances (ecdb);
    ASSERT_GE (m_selectTime, 0.0) << "ECSQL SELECT test failed";
    UpdateInstances (ecdb, true);
    ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    DeleteInstances (ecdb);
    ASSERT_GE (m_deleteTime, 0.0) << "ECSQL DELETE test failed";

    Utf8String testDescription;
    testDescription.Sprintf ("ECSQL INSERT against ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("ECSQL SELECT * FROM ONLY against ECClass with %d properties - no result set iteration.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("ECSQL UPDATE ONLY against ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("ECSQL DELETE FROM ONLY against ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_instancesPerClass);

    ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

    //CRUD Performance using Sql statements.
    m_insertTime = m_updateTime = m_selectTime = m_deleteTime = 0.0;
    Utf8String insertSql;
    Utf8String updateSql;
    Utf8String selectSql;
    Utf8String deleteSql;
    GenerateSqlCRUDTestStatements (*testSchema, *baseClass, insertSql, selectSql, updateSql, deleteSql, false);

    //Insert Instance using Sql Query.
    BeSQLite::Statement stmt;
    StopWatch timer (true);
    if (BE_SQLITE_OK != stmt.Prepare (ecdb, insertSql.c_str ()))
        {
        FAIL ();
        return;
        }

    int propertyCount = (int)baseClass->GetPropertyCount (true);
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        if (BE_SQLITE_OK != stmt.BindInt64 (1, (int64_t)(i + 1)))
            {
            FAIL ();
            return;
            }

        for (int parameterIndex = 2; parameterIndex <= propertyCount + 1; parameterIndex++)
            {
            if (BE_SQLITE_OK != stmt.BindText (parameterIndex, "Init Value", BeSQLite::Statement::MakeCopy::No))
                {
                FAIL ();
                return;
                }
            }

        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_insertTime = timer.GetElapsedSeconds ();
    LOG.infov ("Scenario - INSERT INTO- 1 class [%d properties each] , %d instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_insertTime);

    //Update Instance using Sql Query.
    timer.Start ();
    if (BE_SQLITE_OK != stmt.Prepare (ecdb, updateSql.c_str ()))
        {
        FAIL ();
        return;
        }

    propertyCount = (int)baseClass->GetPropertyCount (true);
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindText (parameterIndex, "UpdatedValue", BeSQLite::Statement::MakeCopy::No));
            }

        ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (propertyCount + 1, (int64_t)(i + 1)));
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
        ASSERT_EQ (1, ecdb.GetModifiedRowCount ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_updateTime = timer.GetElapsedSeconds ();

    //Read Instance using Sql Query.
    timer.Start ();
    ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, selectSql.c_str ()));
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
        ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_selectTime = timer.GetElapsedSeconds ();
    stmt.Finalize ();
    LOG.infov ("Scenario - Read - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_selectTime);

    //Delete Instance using Sql Query.
    timer.Start ();
    ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, deleteSql.c_str ()));
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_deleteTime = timer.GetElapsedSeconds ();
    LOG.infov ("Scenario - DELETE - 1 class [%d properties each] , %d instances per class took - %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_deleteTime);

    testDescription.Sprintf ("SQL INSERT against table of ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("SQL SELECT * FROM table of ECClass with %d properties - no result set iteration.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("SQL UPDATE against table of ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_instancesPerClass);
    testDescription.Sprintf ("SQL DELETE against table ECClass with %d properties.", m_propertiesPerClass);
    LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_instancesPerClass);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, CRUDPerformanceSqlVsECSqlWithIncrementalNoOfProperties)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_ecsqlTestItems.clear ();
        m_propertiesPerClass = (size_t)pow (2, i);
        ECSchemaPtr testSchema;
        ECDbTestProject test;
        ECDbR ecdb = test.Create ("ECSqlStatementPerformanceTest.ecdb");
        ECSchema::CreateSchema (testSchema, "testSchema", 1, 0);
        ASSERT_TRUE (testSchema.IsValid ());
        testSchema->SetNamespacePrefix ("ts");

        ECClassP baseClass;
        ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, "BaseClass"));
        CreatePrimitiveProperties (*baseClass);

        ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
        schemaCache->AddSchema (*testSchema);

        ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache));
        ASSERT_EQ (BE_SQLITE_OK, ecdb.SaveChanges ());

        GenerateECSqlCRUDTestStatements (*testSchema, true);

        InsertInstances (ecdb, false);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
        ReadInstances (ecdb);
        ASSERT_GE (m_selectTime, 0.0) << "ECSQL SELECT test failed";
        UpdateInstances (ecdb, false);
        ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";
        DeleteInstances (ecdb);
        ASSERT_GE (m_deleteTime, 0.0) << "ECSQL DELETE test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL INSERT against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("ECSQL SELECT * FROM ONLY against ECClass with %d properties and %d Instances. - no result set iteration.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("ECSQL UPDATE ONLY against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("ECSQL DELETE FROM ONLY against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //CRUD Performance using Sql statements.
        m_insertTime = m_updateTime = m_selectTime = m_deleteTime = 0.0;
        Utf8String insertSql;
        Utf8String updateSql;
        Utf8String selectSql;
        Utf8String deleteSql;
        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, insertSql, selectSql, updateSql, deleteSql, true);

        //Insert Instance using Sql Query.
        BeSQLite::Statement stmt;
        StopWatch timer (true);
        if (BE_SQLITE_OK != stmt.Prepare (ecdb, insertSql.c_str ()))
            {
            FAIL ();
            return;
            }

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (BE_SQLITE_OK != stmt.BindInt64 (1, (int64_t)(i + 1)))
                {
                FAIL ();
                return;
                }

            ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        timer.Stop ();
        stmt.Finalize ();
        m_insertTime = timer.GetElapsedSeconds ();
        LOG.infov ("Insert Sql %s", insertSql.c_str());
        LOG.infov ("Scenario - INSERT INTO- 1 class [%d properties each] , %d instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_insertTime);

        //Update Instance using Sql Query.
        timer.Start ();
        if (BE_SQLITE_OK != stmt.Prepare (ecdb, updateSql.c_str ()))
            {
            FAIL ();
            return;
            }

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
            ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
            ASSERT_EQ (1, ecdb.GetModifiedRowCount ());
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        timer.Stop ();
        stmt.Finalize ();
        m_updateTime = timer.GetElapsedSeconds ();
        LOG.infov ("Update Sql %s", updateSql.c_str());
        LOG.infov ("Scenario - Update - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_updateTime);

        //Read Instance using Sql Query.
        timer.Start ();
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, selectSql.c_str ()));
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
            ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ());
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        timer.Stop ();
        m_selectTime = timer.GetElapsedSeconds ();
        stmt.Finalize ();
        LOG.infov ("Select Sql %s", selectSql.c_str());
        LOG.infov ("Scenario - Read - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_selectTime);

        //Delete Instance using Sql Query.
        timer.Start ();
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, deleteSql.c_str ()));
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
            ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        timer.Stop ();
        m_deleteTime = timer.GetElapsedSeconds ();
        LOG.infov ("Delete Sql %s", deleteSql.c_str());
        LOG.infov ("Scenario - DELETE - 1 class [%d properties each] , %d instances per class took - %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_deleteTime);

        testDescription.Sprintf ("SQL INSERT against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("SQL SELECT * FROM table of ECClass with %d properties and %d Instances. - no result set iteration.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("SQL UPDATE against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);
        testDescription.Sprintf ("SQL DELETE against table ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }
END_ECDBUNITTESTS_NAMESPACE