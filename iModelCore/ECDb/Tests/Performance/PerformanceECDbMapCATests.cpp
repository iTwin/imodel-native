/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitTests/NonPublished/ECDb/ECDbTestProject.h"
#include "PerformanceTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiClass                                       Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECDbMapCATestFixture : public PerformanceTestFixture
    {
    public:
        size_t m_classNamePostFix = 1;
        size_t m_instancesPerClass = 0;
        double m_InsertTime, m_UpdateTime, m_SelectTime, m_DeleteTime;
        struct ECSqlTestItem
            {
            Utf8String m_insertECSql;
            Utf8String m_selectECSql;
            Utf8String m_updateECSql;
            Utf8String m_deleteECSql;
            };
        bmap<ECN::ECClassCP, ECSqlTestItem> m_sqlTestItems;
        virtual void InitializeTestDb () override {}
        void CreateClassHierarchy (ECSchemaR testSchema, size_t LevelCount, ECClassR baseClass);
        void CreatePrimitiveProperties (ECClassR ecClass, size_t noOfProperties);
        void GenerateCRUDTestStatements (ECSchemaR ecSchema);
        void InsertInstances (ECDbR ecdb);
        void ReadInstances (ECDbR ecdb);
        void UpdateInstances (ECDbR ecdb);
        void DeleteInstances (ECDbR ecdb);
        void GenerateReadUpdateDeleteStatements (ECSchemaR ecSchema);
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::ReadInstances(ECDbR ecdb)
    {
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        Utf8StringCR selectSql = kvPair.second.m_selectECSql;

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, selectSql.c_str ());
        ASSERT_EQ (ECSqlStatus::Success, stat) << "preparation failed for " << selectSql.c_str ();
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            EXPECT_EQ (stmt.BindInt (1, instanceId++), ECSqlStatus::Success);
            EXPECT_EQ (stmt.Step (), ECSqlStepStatus::HasRow) << "step failed for " << selectSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_SelectTime = timer.GetElapsedSeconds ();
    auto const classCount = m_sqlTestItems.size ();
    auto totalInserts = m_instancesPerClass*classCount;
    LOG.infov ("\n SELECT %d Instances took - %.4f s.", totalInserts, timer.GetElapsedSeconds ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::DeleteInstances(ECDbR ecdb)
    {
    int instanceId = 1;
    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        Utf8String deleteSql = kvPair.second.m_deleteECSql;

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, deleteSql.c_str ());
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Preparation Failed for " << deleteSql.c_str ();
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            EXPECT_EQ (stmt.BindInt (1, instanceId++), ECSqlStatus::Success);
            EXPECT_EQ (stmt.Step (), ECSqlStepStatus::Done) << "step failed for " << deleteSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_DeleteTime = timer.GetElapsedSeconds ();
    auto const classCount = m_sqlTestItems.size ();
    auto totalInserts = m_instancesPerClass*classCount;
    LOG.infov ("\n DELETE %d Instances took - %.4f s.\n", totalInserts, timer.GetElapsedSeconds ());
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
        Utf8String updateSql = kvPair.second.m_updateECSql;
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, updateSql.c_str ());
        EXPECT_EQ (ECSqlStatus::Success, stat) << "preparation failed for " << updateSql.c_str ();
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
                {
                EXPECT_EQ (ECSqlStatus::Success, stmt.BindText (parameterIndex++, ("UpdatedValue"), IECSqlBinder::MakeCopy::No));
                }

            EXPECT_EQ (ECSqlStatus::Success, stmt.BindInt (propertyCount + 1, instanceId++));
            EXPECT_EQ (ECSqlStepStatus::Done, stmt.Step ()) << "step failed for " << updateSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_UpdateTime = timer.GetElapsedSeconds ();
    auto const classCount = m_sqlTestItems.size ();
    auto totalInserts = m_instancesPerClass*classCount;
    LOG.infov ("\n UPDATE %d Instances took - %.4f s.", totalInserts, timer.GetElapsedSeconds ());
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
        Utf8StringCR insertSql = kvPair.second.m_insertECSql;
        const int propertyCount = (int)testClass->GetPropertyCount (true);

        ECSqlStatement stmt;
        auto stat = stmt.Prepare (ecdb, insertSql.c_str ());
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Preparation failed for " << insertSql.c_str ();
        for (size_t i = 0; i < m_instancesPerClass; i++)
            {
            int parameterIndex = 1;
            for (int parameterIndex = 1; parameterIndex <= propertyCount; parameterIndex++)
                {
                EXPECT_EQ (ECSqlStatus::Success, stmt.BindText (parameterIndex++, ("Init Value"), IECSqlBinder::MakeCopy::No));
                }

            EXPECT_EQ (stmt.Step (), ECSqlStepStatus::Done) << "Step failed for " << insertSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    timer.Stop ();
    m_InsertTime = timer.GetElapsedSeconds ();
    auto const classCount = m_sqlTestItems.size ();
    auto totalInserts = m_instancesPerClass*classCount;

    LOG.infov ("Scenario - INSERT - %d classes [4 properties each] , %d instances per class , %d total inserts took - %.4f s.",
               classCount,
               m_instancesPerClass,
               totalInserts,
               timer.GetElapsedSeconds());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::GenerateCRUDTestStatements(ECSchemaR ecSchema)
    {
    for (ECClassCP testClass : ecSchema.GetClasses ())
        {
        ECSqlTestItem& testItem = m_sqlTestItems[testClass];
        Utf8StringR insertSql = testItem.m_insertECSql;
        Utf8StringR selectSql = testItem.m_selectECSql;
        Utf8StringR updateSql = testItem.m_updateECSql;
        Utf8StringR deleteSql = testItem.m_deleteECSql;

        Utf8String className = (Utf8String)testClass->GetName ();

        insertSql = Utf8String ("INSERT INTO ts.");
        insertSql.append (className).append (" (");
        Utf8String insertValuesSql (") VALUES (");

        selectSql = Utf8String ("SELECT * FROM ONLY ts.");
        selectSql.append (className).append (" WHERE ECInstanceId = ?");

        updateSql = Utf8String ("UPDATE ts.");
        updateSql.append (className).append (" SET ");

        deleteSql = Utf8String ("DELETE FROM ts.");
        deleteSql.append (className).append (" WHERE ECInstanceId = ?");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties (true))
            {
            if (!isFirstItem)
                {
                insertSql.append (", ");
                insertValuesSql.append (", ");

                updateSql.append (", ");
                }

            insertSql.append ((Utf8String)prop->GetName ());
            insertValuesSql.append ("?");

            updateSql.append ((Utf8String)prop->GetName ());
            updateSql.append (" = ? ");

            isFirstItem = false;
            }

        insertSql.append (insertValuesSql).append (");");
        updateSql.append ("WHERE ECInstanceId = ?");
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceECDbMapCATestFixture::CreatePrimitiveProperties(ECClassR ecClass, size_t noOfProperties)
    {
    PrimitiveECPropertyP primitiveP;
    for (size_t i = 0; i < 4; i++)
        {
        WString temp;
        temp.Sprintf (L"_Property%d", i);
        WString propName = ecClass.GetName ();
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
    else
        {
        ECClassP tmpClass;
        WString className;
        className.Sprintf (L"P%d", m_classNamePostFix++);
        EXPECT_EQ (testSchema.CreateClass (tmpClass, className), ECOBJECTS_STATUS_Success);
        EXPECT_EQ (tmpClass->AddBaseClass (baseClass), ECOBJECTS_STATUS_Success);
        CreatePrimitiveProperties (*tmpClass, 4);

        ECClassP tmpClass1;
        className.Sprintf (L"P%d", m_classNamePostFix++);
        EXPECT_EQ (testSchema.CreateClass (tmpClass1, className), ECOBJECTS_STATUS_Success);
        EXPECT_EQ (tmpClass1->AddBaseClass (baseClass), ECOBJECTS_STATUS_Success);
        CreatePrimitiveProperties (*tmpClass1, 4);

        CreateClassHierarchy (testSchema, LevelCount - 1, *tmpClass);
        CreateClassHierarchy (testSchema, LevelCount - 1, *tmpClass1);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATestFixture, InstanceInsertionWithSharedColumnsForSubclasses)
    {
    m_InsertTime = m_UpdateTime = m_SelectTime = m_DeleteTime = 0.0;
    m_instancesPerClass = 100;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDOperationWithSharedColumns.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, L"testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix (L"ts");

    auto readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    auto ecdbmapKey = SchemaKey (L"ECDbMap", 1, 0);
    auto ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());
    readContext->AddSchema (*testSchema);
    testSchema->AddReferencedSchema (*ecdbmapSchema);

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, L"BaseClass"));
    CreatePrimitiveProperties (*baseClass, 4);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy (*testSchema, 7, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP (L"ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue (L"MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue (L"MapStrategy.Options", ECValue ("SharedColumnsForSubclasses")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue (L"MapStrategy.IsPolymorphic", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateCRUDTestStatements (*testSchema);
    InsertInstances (ecdb);
    ReadInstances (ecdb);
    UpdateInstances (ecdb);
    DeleteInstances (ecdb);

    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_InsertTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Insert time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_SelectTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "SELECT time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_UpdateTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Update time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_DeleteTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithSharedColumnsForSubclasses", "Delete time"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATestFixture, InstanceInsertionWithOutSharedColumnsForSubClasses)
    {
    m_InsertTime = m_UpdateTime = m_SelectTime = m_DeleteTime = 0.0;
    m_instancesPerClass = 100;
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("CRUDOperationWithOutSharedColumnsForSubClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, L"testSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix (L"ts");

    auto readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    auto ecdbmapKey = SchemaKey (L"ECDbMap", 1, 0);
    auto ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());
    readContext->AddSchema (*testSchema);
    testSchema->AddReferencedSchema (*ecdbmapSchema);

    ECClassP baseClass;
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testSchema->CreateClass (baseClass, L"BaseClass"));
    CreatePrimitiveProperties (*baseClass, 4);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy (*testSchema, 7, *baseClass);

    auto ca = ecdbmapSchema->GetClassCP (L"ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue (L"MapStrategy.Strategy", ECValue ("SharedTable")) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (customAttribute->SetValue (L"MapStrategy.IsPolymorphic", ECValue (true)) == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (baseClass->SetCustomAttribute (*customAttribute) == ECOBJECTS_STATUS_Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));

    GenerateCRUDTestStatements (*testSchema);
    InsertInstances (ecdb);
    ReadInstances (ecdb);
    UpdateInstances (ecdb);
    DeleteInstances (ecdb);

    PerformanceTestingFrameWork performanceObjSchemaImport;
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_InsertTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Insert time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_SelectTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "SELECT time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_UpdateTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Update time"));
    EXPECT_TRUE (performanceObjSchemaImport.writeTodb (m_DeleteTime, "PerformanceECDbMapCATestFixture.InstanceInsertionWithOutSharedColumnsForSubClasses", "Delete time"));
    }


END_ECDBUNITTESTS_NAMESPACE