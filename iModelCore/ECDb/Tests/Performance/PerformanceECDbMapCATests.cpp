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
void PerformanceECDbMapCATestFixture::ECSqlReadInstances (ECDbR ecdb, bool iterateResultSet)
    {
    m_selectTime = -1.0;
    int instanceId = 1;
    //printf("Start profiling and press key tp continue..."); getchar();
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        Utf8CP selectECSql = kvPair.second.m_selectECSql.c_str ();
        LOG.infov ("\n READ ECSql statement :  %s \n", selectECSql);
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, selectECSql);
        LOG.infov ("\n Back-end Read Sql : %s \n", stmt.GetNativeSql ());
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

            if (iterateResultSet)
                {
                Utf8String propValue;
                int columnCount = stmt.GetColumnCount ();
                ASSERT_EQ ((int)m_propertiesPerClass, columnCount);
                for (int j = 0; j < columnCount; j++)
                    {
                    propValue = stmt.GetValueText (j);
                    ASSERT_FALSE (Utf8String::IsNullOrEmpty (propValue.c_str ()));
                    }
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
void PerformanceECDbMapCATestFixture::ECSqlDeleteInstances (ECDbR ecdb)
    {
    m_deleteTime = -1.0;
    int instanceId = 1;
    //printf("Start profiling and press key tp continue..."); getchar();
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        Utf8CP deleteECSql = kvPair.second.m_deleteECSql.c_str ();
        LOG.infov ("\n Delete ECSql statement : %s \n", deleteECSql);
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, deleteECSql);
        LOG.infov ("\n Back-end Delete Sql : %s \n", stmt.GetNativeSql ());
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
void PerformanceECDbMapCATestFixture::ECSqlUpdateInstances (ECDbR ecdb, bool bindPropertyValues)
    {
    m_updateTime = -1.0;
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP updateECSql = kvPair.second.m_updateECSql.c_str ();
        LOG.infov ("\n Update ECSql statement :  %s \n", updateECSql);
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, updateECSql);
        LOG.infov ("\n Back-end update Sql : %s \n", stmt.GetNativeSql ());
        if (!stat.IsSuccess ())
            return;

        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            if (bindPropertyValues)
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
void PerformanceECDbMapCATestFixture::ECSqlInsertInstances (ECDbR ecdb, bool bindPropertyValues)
    {
    m_insertTime = -1.0;
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_ecsqlTestItems)
        {
        ECClassCP testClass = kvPair.first;
        Utf8CP insertECSql = kvPair.second.m_insertECSql.c_str ();
        LOG.infov ("\n Insert ECSql statement : %s \n", insertECSql);
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare (ecdb, insertECSql);
        LOG.infov ("\n Back-end Insert Sql : %s \n", stmt.GetNativeSql ());
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

            if (bindPropertyValues)
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
void PerformanceECDbMapCATestFixture::GenerateECSqlCRUDTestStatements (ECSchemaR ecSchema, bool hardCodePropertyValues)
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

                updateECSql.append (", ");
                }

            selectECSql.append (prop->GetName ());

            if (hardCodePropertyValues)
                {
                insertECSql.append (prop->GetName ());
                insertValuesSql.append ("'InitValue'");

                updateECSql.append (prop->GetName ());
                updateECSql.append (" = 'UpdatedValue'");
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
    ECSqlInsertInstances (ecdb, true);
    ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
    ECSqlReadInstances (ecdb, false);
    ASSERT_GE (m_selectTime, 0.0) << "ECSQL SELECT test failed";
    ECSqlUpdateInstances (ecdb, true);
    ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    ECSqlDeleteInstances (ecdb);
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
    ECSqlInsertInstances (ecdb, true);
    ECSqlReadInstances (ecdb, false);
    ECSqlUpdateInstances (ecdb, true);
    ECSqlDeleteInstances (ecdb);

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
    ECSqlInsertInstances (ecdb, true);
    ECSqlReadInstances (ecdb, false);
    ECSqlUpdateInstances (ecdb, true);
    ECSqlDeleteInstances (ecdb);

    LOGTODB (TEST_DETAILS, m_insertTime, "Insert time");
    LOGTODB (TEST_DETAILS, m_selectTime, "Select time");
    LOGTODB (TEST_DETAILS, m_updateTime, "Update time");
    LOGTODB (TEST_DETAILS, m_deleteTime, "Delete time");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateSqlCRUDTestStatements (ECSchemaR ecSchema, ECClassR ecClass, bool hardCodePropertyValues)
    {
    Utf8String className = (Utf8String)ecClass.GetName ();

    m_insertSql = Utf8String ("INSERT INTO ts_");
    m_insertSql.append (className).append (" (ECInstanceId, ");
    Utf8String insertValuesSql (") VALUES (?, ");

    m_updateSql = Utf8String ("UPDATE ts_");
    m_updateSql.append (className).append (" SET ");

    m_selectSql = Utf8String ("SELECT ");

    //use view here to make sure referential integrity is ensured. Otherwise it is not comparable to ECSQL.
    m_deleteSql = Utf8String ("DELETE FROM ts_");
    m_deleteSql.append (className).append (" WHERE ECInstanceId = ? ");

    bool isFirstItem = true;
    for (auto prop : ecClass.GetProperties (true))
        {
        if (!isFirstItem)
            {
            m_selectSql.append (", ");

            m_insertSql.append (", ");
            insertValuesSql.append (", ");

            m_updateSql.append (", ");
            }

        m_selectSql.append (prop->GetName ());

        if (hardCodePropertyValues)
            {
            m_insertSql.append (prop->GetName ());
            insertValuesSql.append ("'InitValue'");

            m_updateSql.append (prop->GetName ());
            m_updateSql.append (" = 'UpdatedValue' ");
            }
        else
            {
            m_insertSql.append (prop->GetName ());
            insertValuesSql.append ("?");

            m_updateSql.append (prop->GetName ());
            m_updateSql.append (" = ? ");
            }

        isFirstItem = false;
        }

    m_selectSql.append (" FROM ts_");
    m_selectSql.append (className).append (" WHERE ECInstanceId = ? ");
    m_insertSql.append (insertValuesSql).append (");");
    m_updateSql.append ("WHERE ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::SqlInsertInstances (ECDbR ecdb, ECClassR ecClass, bool bindPropertyValues)
    {
    //Insert Instance using Sql Query.
    const int propertyCount = (int)ecClass.GetPropertyCount (true);
    BeSQLite::Statement stmt;
    StopWatch timer (true);
    if (BE_SQLITE_OK != stmt.Prepare (ecdb, m_insertSql.c_str ()))
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

        if (bindPropertyValues)
            {
            for (int parameterIndex = 2; parameterIndex <= propertyCount + 1; parameterIndex++)
                {
                if (BE_SQLITE_OK != stmt.BindText (parameterIndex, "Init Value", BeSQLite::Statement::MakeCopy::No))
                    {
                    FAIL ();
                    return;
                    }
                }
            }

        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_insertTime = timer.GetElapsedSeconds ();
    LOG.infov ("\n Insert Sql :  %s \n", m_insertSql.c_str());
    LOG.infov ("Scenario - INSERT INTO- 1 class [%d properties each] , %d instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_insertTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::SqlReadInstances (ECDbR ecdb, ECClassR ecClass, bool iterateResultSet)
    {
    //Read Instance using Sql Query.
    BeSQLite::Statement stmt;
    StopWatch timer (true);
    ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, m_selectSql.c_str ()));
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
        ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ());

        if (iterateResultSet)
            {
            Utf8String propValue;
            int columnCount = stmt.GetColumnCount ();
            ASSERT_EQ ((int)m_propertiesPerClass, columnCount);
            for (int j = 0; j < columnCount; j++)
                {
                propValue = stmt.GetValueText (j);
                ASSERT_FALSE (Utf8String::IsNullOrEmpty (propValue.c_str ()));
                }
            }

        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_selectTime = timer.GetElapsedSeconds ();
    stmt.Finalize ();
    LOG.infov ("\n READ Sql : %s \n", m_selectSql.c_str());
    LOG.infov ("Scenario - Read - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_selectTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::SqlUpdateInstances (ECDbR ecdb, ECClassR ecClass, bool bindPropertyValues)
    {
    //Update Instance using Sql Query.
    const int propertyCount = (int)ecClass.GetPropertyCount (true);
    BeSQLite::Statement stmt;
    StopWatch timer (true);
    if (BE_SQLITE_OK != stmt.Prepare (ecdb, m_updateSql.c_str ()))
        {
        FAIL ();
        return;
        }

    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        if (bindPropertyValues)
            {
                for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
                    {
                    ASSERT_EQ (BE_SQLITE_OK, stmt.BindText (parameterIndex, "UpdatedValue", BeSQLite::Statement::MakeCopy::No));
                    }

            ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (propertyCount + 1, (int64_t)(i + 1)));
            }
        else
            {
            ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
            }

        if (BE_SQLITE_DONE != stmt.Step () || ecdb.GetModifiedRowCount () == 0)
            {
            ASSERT_TRUE (false);
            return;
            }

        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    stmt.Finalize ();
    m_updateTime = timer.GetElapsedSeconds ();
    LOG.infov ("\n Update Sql :  %s \n", m_updateSql.c_str());
    LOG.infov ("Scenario - Update - 1 class [%d properties each] , %d Instances per class took %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_updateTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::SqlDeleteInstances (ECDbR ecdb, ECClassR ecClass)
    {
    StopWatch timer (true);
    BeSQLite::Statement stmt;
    ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, m_deleteSql.c_str ()));
    for (size_t i = 0; i < m_instancesPerClass; i++)
        {
        ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt64 (1, (int64_t)(i + 1)));
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    m_deleteTime = timer.GetElapsedSeconds ();
    LOG.infov ("\n Delete Sql :  %s \n", m_deleteSql.c_str());
    LOG.infov ("Scenario - DELETE - 1 class [%d properties each] , %d instances per class took - %.4f s.", m_propertiesPerClass, m_instancesPerClass, m_deleteTime);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::SetUpTestDb (ECDbR ecdb, ECN::ECSchemaPtr &ecSchema, ECClassP &baseClass)
    {
    m_ecsqlTestItems.clear ();

    ECSchema::CreateSchema (ecSchema, "testSchema", 1, 0);
    ASSERT_TRUE (ecSchema.IsValid ());
    ecSchema->SetNamespacePrefix ("ts");

    ASSERT_EQ (ECOBJECTS_STATUS_Success, ecSchema->CreateClass (baseClass, "BaseClass"));
    CreatePrimitiveProperties (*baseClass);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*ecSchema);

    EXPECT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemaCache));
    EXPECT_EQ (BE_SQLITE_OK, ecdb.SaveChanges ());
    }

//---------------------------------------------------------------------------------------
// Scenario: No of Instances 1000000, No of Properties 4:128
//           One time Prepare and then Loop through Binding, Step, ReSet and ClearBinding
//           Bind ECInstanceId instead of auto Generating it

// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlInsertPerformance_BindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        GenerateECSqlCRUDTestStatements (*testSchema, false);

        ECSqlInsertInstances (ecdb, true);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL INSERT against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ECSqlDeleteInstances (ecdb);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //Insert Performance using Sql statements.
        m_insertTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, false);

        SqlInsertInstances (ecdb, *baseClass, true);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";

        testDescription.Sprintf ("SQL INSERT against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlInsertPerformance_DoNotBindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        //Insert Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements (*testSchema, true);

        ECSqlInsertInstances (ecdb, false);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL INSERT against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ECSqlDeleteInstances (ecdb);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //Insert Performance using Sql statement.
        m_insertTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, true);

        SqlInsertInstances (ecdb, *baseClass, false);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";

        testDescription.Sprintf ("SQL INSERT against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_insertTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlReadPerformance)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        //READ Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements (*testSchema, true);

        ECSqlInsertInstances (ecdb, false);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
        ECSqlReadInstances (ecdb, true);
        ASSERT_GE (m_selectTime, 0.0) << "ECSQL SELECT test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL SELECT Prop1 Prop2 Prop3... FROM ONLY against ECClass with %d properties and %d Instances. - with result set iteration.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ECSqlDeleteInstances (ecdb);
        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //READ Performance using Sql statement.
        m_selectTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, true);

        SqlInsertInstances (ecdb, *baseClass, false);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";
        SqlReadInstances (ecdb, *baseClass, true);
        ASSERT_GE (m_selectTime, 0.0) << "SQL SELECT test failed";
        testDescription.Sprintf ("SQL SELECT Prop1 Prop2 Prop3... FROM table of ECClass with %d properties and %d Instances. - with result set iteration.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_selectTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlUpdatePerformance_DoNotBindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        //UPDATE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements (*testSchema, true);

        ECSqlInsertInstances (ecdb, false);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
        ECSqlUpdateInstances (ecdb, false);
        ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL UPDATE ONLY against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ECSqlDeleteInstances (ecdb);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //UPDATE Performance using Sql statement.
        m_updateTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, true);

        SqlInsertInstances (ecdb, *baseClass, false);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";
        SqlUpdateInstances (ecdb, *baseClass, false);
        ASSERT_GE (m_updateTime, 0.0) << "SQL UPDATE test failed";

        testDescription.Sprintf ("SQL UPDATE against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlUpdatePerformance_BindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        //UPDATE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements (*testSchema, false);

        ECSqlInsertInstances (ecdb, true);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
        ECSqlUpdateInstances (ecdb, true);
        ASSERT_GE (m_updateTime, 0.0) << "ECSQL UPDATE test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL UPDATE ONLY against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ECSqlDeleteInstances (ecdb);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //UPDATE Performance using Sql statement.
        m_updateTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, false);

        SqlInsertInstances (ecdb, *baseClass, true);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";
        SqlUpdateInstances (ecdb, *baseClass, true);
        ASSERT_GE (m_updateTime, 0.0) << "SQL UPDATE test failed";

        testDescription.Sprintf ("SQL UPDATE against table of ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_updateTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECDbMapCATestFixture, SqlVsECSqlDeletePerformance)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t)pow (2, i);
        ECDbTestProject testProj;
        ECDbR ecdb = testProj.Create ("CRUDPerformanceSqlVsECSql.ecdb");
        ECSchemaPtr testSchema;
        ECClassP baseClass = nullptr;
        SetUpTestDb (ecdb, testSchema, baseClass);

        //DELETE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements (*testSchema, true);

        ECSqlInsertInstances (ecdb, false);
        ASSERT_GE (m_insertTime, 0.0) << "ECSQL Insert test failed";
        ECSqlDeleteInstances (ecdb);
        ASSERT_GE (m_deleteTime, 0.0) << "ECSQL DELETE test failed";

        Utf8String testDescription;
        testDescription.Sprintf ("ECSQL DELETE FROM ONLY against ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_propertiesPerClass);

        ASSERT_EQ (BE_SQLITE_OK, ecdb.AbandonChanges ());

        //DELETE Performance using Sql statement.
        m_deleteTime = 0.0;

        GenerateSqlCRUDTestStatements (*testSchema, *baseClass, true);

        SqlInsertInstances (ecdb, *baseClass, false);
        ASSERT_GE (m_insertTime, 0.0) << "SQL Insert test failed";
        SqlDeleteInstances (ecdb, *baseClass);
        ASSERT_GE (m_deleteTime, 0.0) << "SQL DELETE test failed";

        testDescription.Sprintf ("SQL DELETE against table ECClass with %d properties and %d Instances.", m_propertiesPerClass, m_instancesPerClass);
        LOGTODB (TEST_DETAILS, m_deleteTime, testDescription.c_str (), (int)m_propertiesPerClass);
        }
    }

END_ECDBUNITTESTS_NAMESPACE