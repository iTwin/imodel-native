/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceBisDesignTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceBisDesignTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//****************************************************************************************
// PerformanceBisDesignTestFixture
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8CP const PerformanceBisDesignTestFixture::BASE_CLASS_NAME = "Base";
Utf8CP const PerformanceBisDesignTestFixture::ECINSTANCEID_COLUMN_NAME = "ecinstanceid";
Utf8CP const PerformanceBisDesignTestFixture::CLASSID_COLUMN_NAME = "ecclassid";
Utf8CP const PerformanceBisDesignTestFixture::PARENTECINSTANCEID_COLUMN_NAME = "parentecinstanceid";
Utf8CP const PerformanceBisDesignTestFixture::BUSINESSKEY_COLUMN_NAME = "businesskey";
Utf8CP const PerformanceBisDesignTestFixture::GLOBALID_COLUMN_NAME = "globalid";

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceBisDesignTestFixture::RunTest (Utf8CP dbFileName, Context& context)
    {
    auto testSchema = CreateTestSchema (context.GetDomainPropertyCount ());
    Utf8String bisonTestDetails(dbFileName);
    //set up
    {
    Db db;
    CreateTestDb (db, dbFileName);
    Setup (db, context, *testSchema);
    }

    //insert test
        {
        Db db;
        OpenTestDb (db, dbFileName, false);
        StopWatch logInsertTime(true);
        _RunInsertTest (db, context);
        logInsertTime.Stop();
        LOGTODB(TEST_DETAILS, logInsertTime.GetElapsedSeconds(), "Performance BIS Design Studies test. RunInsert for Db name");
        }

    //select test
        {
        Db db;
        OpenTestDb (db, dbFileName, true);
        StopWatch logSelectTime(true);
        _RunSelectTest(db, context);
        logSelectTime.Stop();
        LOGTODB(TEST_DETAILS, logSelectTime.GetElapsedSeconds(), "Performance BIS Design Studies test. RunSelect for Db name");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceBisDesignTestFixture::Setup (DbR db, Context& context, ECSchemaCR testSchema)
    {
    Utf8String createTablesSql;
    Utf8String createIndicesSql;
    _GenerateDdlSql (createTablesSql, createIndicesSql, testSchema);

    auto stat = db.ExecuteSql (Utf8String (createTablesSql).c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Failed to create test tables: " << createTablesSql.c_str ();
    stat = db.ExecuteSql (Utf8String (createIndicesSql).c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Failed to create indices: " << createIndicesSql.c_str ();

    _GenerateSqlTestItems (context, testSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceBisDesignTestFixture::IterateResultSet (BeSQLite::Statement& stmt)
    {
    std::vector<std::pair<int, int>> columnRanges = { { 0, stmt.GetColumnCount () - 1 } };
    IterateResultSet (stmt, columnRanges);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceBisDesignTestFixture::IterateResultSet (BeSQLite::Statement& stmt, std::vector<std::pair<int, int>> const& columnRanges)
    {
    bmap<int, DbValueType> columnTypeMap;
    for (auto const& columnRange : columnRanges)
        {
        for (int i = columnRange.first; i <= columnRange.second; i++)
            {
            columnTypeMap[i] = stmt.GetColumnType (i);
            }
        }

    DbResult stat = BE_SQLITE_OK;
    while ((stat = stmt.Step ()) == BE_SQLITE_ROW)
        {
        for (auto const& columnRange : columnRanges)
            {
            RetrieveValues (stmt, columnTypeMap, columnRange.first, columnRange.second);
            }
        }

    ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step failed for " << stmt.GetSql ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceBisDesignTestFixture::RetrieveValues (BeSQLite::Statement& stmt, bmap<int, DbValueType> const& columnTypeMap, int firstColumnIndex, int lastColumnIndex)
    {
    for (int i = firstColumnIndex; i <= lastColumnIndex; i++)
        {
        const DbValueType type = columnTypeMap.find (i)->second;
        switch (type)
            {
                case DbValueType::NullVal:
                    break;

                default:
                case DbValueType::IntegerVal:
                    stmt.GetValueInt (i);
                    break;

                case DbValueType::FloatVal:
                    stmt.GetValueDouble (i);
                    break;

                case DbValueType::TextVal:
                    stmt.GetValueText (i);
                    break;

                case DbValueType::BlobVal:
                    {
                    stmt.GetColumnBytes (i); //blob size
                    stmt.GetValueBlob (i); //actual blob
                    break;
                    }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceBisDesignTestFixture::BindToParameter
(
BeSQLite::Statement& stmt,
ECN::ECClassCR testClass,
ECN::ECPropertyCR property,
uint64_t ecInstanceId
) const
    {
    const auto parameterMappingIt = m_parameterMapping.find (&property);
    ASSERT_TRUE (parameterMappingIt != m_parameterMapping.end ());
    int parameterIndex = parameterMappingIt->second;

    DbResult stat = BE_SQLITE_OK;

    Utf8String propertyName (property.GetName ());
    if (propertyName.Equals (ECINSTANCEID_COLUMN_NAME))
        {
        stat = stmt.BindInt64 (parameterIndex, ecInstanceId);
        }
    else if (propertyName.Equals (CLASSID_COLUMN_NAME))
        {
        stat = stmt.BindInt64 (parameterIndex, testClass.GetId ());
        }
    else if (propertyName.Equals (PARENTECINSTANCEID_COLUMN_NAME))
        {
        if (ecInstanceId % 5 == 0)
            stat = stmt.BindInt64 (parameterIndex, ecInstanceId / 5);
        }
    else
        {
        switch (property.GetAsPrimitiveProperty ()->GetType ())
            {
                case ECN::PRIMITIVETYPE_Integer:
                    stat = stmt.BindInt (parameterIndex, (-1) * static_cast<int> (ecInstanceId));
                    break;

                case ECN::PRIMITIVETYPE_Double:
                    stat = stmt.BindDouble (parameterIndex, ecInstanceId * 2.5);
                    break;

                case ECN::PRIMITIVETYPE_String:
                    {
                    Utf8String str;
                    str.Sprintf ("Sample string %lld", ecInstanceId);
                    stat = stmt.BindText (parameterIndex, str.c_str (), Statement::MakeCopy::Yes);
                    break;
                    }

                case ECN::PRIMITIVETYPE_Binary:
                    {
                    void * blob = (void *) (&ecInstanceId);
                    stat = stmt.BindBlob (parameterIndex, blob, sizeof (ecInstanceId), Statement::MakeCopy::Yes);
                    break;
                    }

                default:
                    stat = BE_SQLITE_ERROR;
                    break;
            }
        }
    
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Binding value to parameter " << parameterIndex << " failed for " << stmt.GetSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PerformanceBisDesignTestFixture::GetNextECInstanceId () const
    {
    uint64_t id = m_ecInstanceIdSequence;
    m_ecInstanceIdSequence++;
    return id;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceBisDesignTestFixture::AddParameterMapping (ECPropertyCR property, int parameterIndex)
    {
    m_parameterMapping[&property] = parameterIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void PerformanceBisDesignTestFixture::AppendPropDdlToSql (Utf8StringR sql, ECN::ECPropertyCR prop, Utf8CP tableName)
    {
    AppendPropToSql (sql, prop, tableName);

    BeAssert (prop.GetAsPrimitiveProperty () != nullptr);
    const auto primType = prop.GetAsPrimitiveProperty ()->GetType ();
    switch (primType)
        {
            default:
            case PRIMITIVETYPE_Integer:
                sql.append (" INTEGER");
                break;

            case PRIMITIVETYPE_Double:
                sql.append (" DOUBLE");
                break;

            case PRIMITIVETYPE_String:
                sql.append (" TEXT");
                break;

            case PRIMITIVETYPE_Binary:
                sql.append (" BLOB");
                break;
        }

    auto const& propName = prop.GetName ();
    if (propName.Equals (ECINSTANCEID_COLUMN_NAME))
        sql.append (" PRIMARY KEY");
    else if (propName.Equals (CLASSID_COLUMN_NAME))
        sql.append (" NOT NULL");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void PerformanceBisDesignTestFixture::AppendIndexSql (Utf8StringR sql, Utf8CP tableName, Utf8CP columnName, bool isUnique)
    {
    Utf8CP indexSqlTemplate = "CREATE %s INDEX %s_%s ON %s (%s);";
    Utf8String indexSql;
    indexSql.Sprintf (indexSqlTemplate, isUnique ? "UNIQUE" : "",
        tableName, columnName, tableName, columnName);

    sql.append (indexSql);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void PerformanceBisDesignTestFixture::AppendPropToSql (Utf8StringR sql, ECN::ECPropertyCR prop, Utf8CP tableName)
    {
    if (!Utf8String::IsNullOrEmpty (tableName))
        sql.append (tableName).append ("_");

    sql.append (Utf8String (prop.GetName ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String PerformanceBisDesignTestFixture::GetTableName (ECN::ECClassCR testClass)
    {
    return Utf8String (testClass.GetName ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr PerformanceBisDesignTestFixture::CreateTestSchema (int domainPropCount)
    {
    ECSchemaPtr schema = nullptr;
    auto stat = ECSchema::CreateSchema (schema, "BISDesignTest", 1, 0);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    ECClassId classId = 0ULL;
    ECClassCP baseClass = AddTestBaseClass (*schema, classId);
    if (baseClass == nullptr)
        return nullptr;

    classId++;
    ECClassP testClass = AddTestClassStub (*schema, *baseClass, "ClassMixed", classId);
    for (int i = 0; i < domainPropCount; i++)
        {
        if (AddTestClassProperty (*testClass, i + 1, nullptr) != SUCCESS)
            return nullptr;
        }

    classId++;
    testClass = AddTestClassStub (*schema, *baseClass, "ClassInteger", classId);
    PrimitiveType dataType = PRIMITIVETYPE_Integer;
    for (int i = 0; i < domainPropCount; i++)
        {
        if (AddTestClassProperty (*testClass, i + 1, &dataType) != SUCCESS)
            return nullptr;
        }

    classId++;
    testClass = AddTestClassStub (*schema, *baseClass, "ClassDouble", classId);
    dataType = PRIMITIVETYPE_Double;
    for (int i = 0; i < domainPropCount; i++)
        {
        if (AddTestClassProperty (*testClass, i + 1, &dataType) != SUCCESS)
            return nullptr;
        }

    classId++;
    testClass = AddTestClassStub (*schema, *baseClass, "ClassString", classId);
    dataType = PRIMITIVETYPE_String;
    for (int i = 0; i < domainPropCount; i++)
        {
        if (AddTestClassProperty (*testClass, i + 1, &dataType) != SUCCESS)
            return nullptr;
        }

    classId++;
    testClass = AddTestClassStub (*schema, *baseClass, "ClassBlob", classId);
    dataType = PRIMITIVETYPE_Binary;
    for (int i = 0; i < domainPropCount; i++)
        {
        if (AddTestClassProperty (*testClass, i + 1, &dataType) != SUCCESS)
            return nullptr;
        }

    return schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECClassP PerformanceBisDesignTestFixture::AddTestBaseClass (ECSchemaR schema, ECClassId classId)
    {
    ECClassP baseClass = nullptr;
    auto stat = schema.CreateClass (baseClass, BASE_CLASS_NAME);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;
    //make it an abstract class
    baseClass->SetIsCustomAttributeClass (false);
    baseClass->SetIsStruct (false);
    baseClass->SetIsDomainClass (false);
    baseClass->SetId (classId);

    PrimitiveECPropertyP prop = nullptr;
    stat = baseClass->CreatePrimitiveProperty (prop, ECINSTANCEID_COLUMN_NAME, PRIMITIVETYPE_Integer);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    stat = baseClass->CreatePrimitiveProperty (prop, CLASSID_COLUMN_NAME, PRIMITIVETYPE_Integer);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    stat = baseClass->CreatePrimitiveProperty (prop, PARENTECINSTANCEID_COLUMN_NAME, PRIMITIVETYPE_Integer);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    stat = baseClass->CreatePrimitiveProperty (prop, BUSINESSKEY_COLUMN_NAME, PRIMITIVETYPE_String);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    stat = baseClass->CreatePrimitiveProperty (prop, GLOBALID_COLUMN_NAME, PRIMITIVETYPE_String);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;

    return baseClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECClassP PerformanceBisDesignTestFixture::AddTestClassStub (ECSchemaR schema, ECClassCR baseClass, Utf8CP name, ECClassId classId)
    {
    ECClassP testClass = nullptr;
    auto stat = schema.CreateClass (testClass, name);
    if (stat != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        return nullptr;
    //make it a domain class
    testClass->SetIsDomainClass (true);
    testClass->SetIsCustomAttributeClass (false);
    testClass->SetIsStruct (false);
    testClass->SetId (classId);

    testClass->AddBaseClass (baseClass);

    return testClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus PerformanceBisDesignTestFixture::AddTestClassProperty (ECClassR testClass, int propertyNumber, ECN::PrimitiveType const* type)
    {
    Utf8String propName;
    propName.Sprintf ("prop%d", propertyNumber);

    PrimitiveType effectiveType;
    if (type != nullptr)
        effectiveType = *type;
    else
        {
        switch ((propertyNumber - 1) % 4)
            {
                default:
                case 0:
                    effectiveType = PRIMITIVETYPE_Integer;
                    break;

                case 1:
                    effectiveType = PRIMITIVETYPE_Double;
                    break;

                case 2:
                    effectiveType = PRIMITIVETYPE_String;
                    break;

                case 3:
                    effectiveType = PRIMITIVETYPE_Binary;
                    break;
            }
        }


    PrimitiveECPropertyP prop = nullptr;
    const auto stat = testClass.CreatePrimitiveProperty (prop, propName, effectiveType);
    return stat == ECObjectsStatus::ECOBJECTS_STATUS_Success ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void PerformanceBisDesignTestFixture::CreateTestDb (DbR db, Utf8CP fileName)
    {
    BeFileName dbPath (ECDbTestUtility::BuildECDbPath (fileName));
    if (dbPath.DoesPathExist ())
        dbPath.BeDeleteFile ();

    ASSERT_EQ (BE_SQLITE_OK, db.CreateNewDb (dbPath));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void PerformanceBisDesignTestFixture::OpenTestDb (DbR db, Utf8CP fileName, bool readonly)
    {
    BeFileName dbPath (ECDbTestUtility::BuildECDbPath (fileName));
    ASSERT_EQ (BE_SQLITE_OK, db.OpenBeSQLiteDb (dbPath, Db::OpenParams (readonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite)));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceBisDesignTestFixture::Scenario PerformanceBisDesignTestFixture::GetScenario () const
    {
    return _GetScenario ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PerformanceBisDesignTestFixture::ToString () const
    {
    return _ToString ();
    }


//****************************************************************************************
// Performance_BisDesign_TablePerClassScenario_TestFixture
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_TablePerClassScenario_TestFixture::_RunInsertTest (DbR db, PerformanceBisDesignTestFixture::Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();

    StopWatch timer (true);
    for (auto const& kvPair : m_sqlTestItems)
        {
        auto testClass = kvPair.first;
        Utf8StringCR insertSql = kvPair.second.m_insertSql;
        auto propertyCollection = testClass->GetProperties (true);

        Statement stmt;
        auto stat = stmt.Prepare (db, insertSql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for " << insertSql.c_str ();
        for (int i = 0; i < instancesPerClassCount; i++)
            {
            uint64_t ecinstanceId = GetNextECInstanceId ();
            int parameterIndex = 1;
            for (auto prop : propertyCollection)
                {
                //no class id column in this scenario
                if (IsClassIdProperty (*prop))
                    continue;

                BindToParameter (stmt, *testClass, *prop, ecinstanceId);
                parameterIndex++;
                }

            stat = stmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step failed for " << insertSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }

    timer.Stop ();
    const auto classCount = m_sqlTestItems.size ();
    auto totalInserts = instancesPerClassCount * classCount;

   LOG.infov ("Scenario %s - INSERT - %.4f s - %d classes [%d properties each], %d instances per class, %d total inserts.",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        classCount,
        context.GetDomainPropertyCount (),
        instancesPerClassCount,
        totalInserts);

   for (auto const& kvPair : m_sqlTestItems)
       {
       LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), kvPair.second.m_insertSql.c_str ());
       }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_TablePerClassScenario_TestFixture::_RunSelectTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();
    StopWatch timer (true);

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto const& selectSql = kvPair.second.m_selectSql;

        Statement stmt;
        auto stat = stmt.Prepare (db, selectSql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for " << selectSql.c_str ();

        IterateResultSet (stmt);
        }

    timer.Stop ();

    LOG.infov ("Scenario %s - SELECT - %.4f s - %d classes [%d properties each], %d instances per class",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        m_sqlTestItems.size (),
        context.GetDomainPropertyCount (),
        instancesPerClassCount);

    for (auto const& kvPair : m_sqlTestItems)
        {
        LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), kvPair.second.m_selectSql.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_TablePerClassScenario_TestFixture::_GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema)
    {
    for (ECClassCP testClass : testSchema.GetClasses ())
        {
        if (!testClass->GetIsDomainClass ()) //No tests against the abstract base class
            continue;

        SqlTestItem& testItem = m_sqlTestItems[testClass];
        Utf8StringR insertSql = testItem.m_insertSql;

        Utf8String tableName = GetTableName (*testClass);

        insertSql = Utf8String ("INSERT INTO ");
        insertSql.append (tableName).append (" (");
        Utf8String insertValuesSql (") VALUES (");

        bool isFirstItem = true;
        int parameterIndex = 1;
        for (auto prop : testClass->GetProperties (true))
            {
            //no class id column in this scenario
            if (IsClassIdProperty (*prop))
                {
                AddParameterMapping (*prop, -1);
                continue;
                }

            if (!isFirstItem)
                {
                insertSql.append (", ");
                insertValuesSql.append (", ");
                }

            AppendPropToSql (insertSql, *prop, nullptr);
            insertValuesSql.append ("?");

            AddParameterMapping (*prop, parameterIndex);
            parameterIndex++;
            isFirstItem = false;
            }

        insertSql.append (insertValuesSql).append (");");

        Utf8StringR selectSql = testItem.m_selectSql;
        selectSql = Utf8String ("SELECT * FROM ");
        selectSql.append (tableName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_TablePerClassScenario_TestFixture::_GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECSchemaCR testSchema) const
    {
    for (auto testClass : testSchema.GetClasses ())
        {
        if (!testClass->GetIsDomainClass ()) //is base class which which is not needed in this scenario
            continue;

        Utf8String tableName = GetTableName (*testClass);

        //CREATE TABLE
        createTablesSql.append ("CREATE TABLE ").append (tableName).append (" (");

        //add system and domain columns (system columns are the props of the base class)
        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties (true))
            {
            //class id column not needed for this scenario
            if (IsClassIdProperty (*prop))
                continue;

            if (!isFirstItem)
                createTablesSql.append (", ");

            AppendPropDdlToSql (createTablesSql, *prop, nullptr);
            isFirstItem = false;
            }

        createTablesSql.append (");");

        //CREATE INDEX
        AppendIndexSql (createIndicesSql, tableName.c_str (), PARENTECINSTANCEID_COLUMN_NAME, false);
        AppendIndexSql (createIndicesSql, tableName.c_str (), BUSINESSKEY_COLUMN_NAME, false);
        AppendIndexSql (createIndicesSql, tableName.c_str (), GLOBALID_COLUMN_NAME, true);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool Performance_BisDesign_TablePerClassScenario_TestFixture::IsClassIdProperty (ECN::ECPropertyCR prop)
    {
    return prop.GetName ().Equals (CLASSID_COLUMN_NAME);
    }


//****************************************************************************************
// Performance_MasterTableScenario_BisDesignTestFixture
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::_RunInsertTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();

    StopWatch timer (true);
    
    //INSERT is generic, can be used for all ECClasses -> only prepare once
    Statement stmt;
    auto stat = stmt.Prepare (db, m_genericInsertSql.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for INSERT: " << db.GetLastError ().c_str() << " SQL: " << m_genericInsertSql.c_str ();

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto testClass = kvPair.first;
        
        for (int i = 0; i < instancesPerClassCount; i++)
            {
            uint64_t ecInstanceId = GetNextECInstanceId ();
            for (auto prop : testClass->GetProperties (true))
                {
                BindToParameter (stmt, *testClass, *prop, ecInstanceId);
                }

            stat = stmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step failed for INSERT: " << db.GetLastError ().c_str() << " SQL: " << m_genericInsertSql.c_str ();
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }

    timer.Stop ();

    const auto classCount = m_sqlTestItems.size ();
    const auto totalInserts = instancesPerClassCount * classCount;

    LOG.infov ("Scenario %s - INSERT - %.4f s - %d classes [%d properties each], %d instances per class, %d total inserts",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        classCount,
        context.GetDomainPropertyCount (),
        instancesPerClassCount,
        totalInserts);

    LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), m_genericInsertSql.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::_RunSelectTest (DbR db, Context const& context) const
    {
    RunNonGenericSelectTest (db, context);
    RunGenericSelectTest (db, context);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::RunNonGenericSelectTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();
    StopWatch timer (true);

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto const& selectSql = kvPair.second.m_nonGenericSelectSql;
        Statement stmt;
        auto stat = stmt.Prepare (db, selectSql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for non-generic SELECT" << selectSql.c_str ();
        IterateResultSet (stmt);
        }

    timer.Stop ();

    LOG.infov ("Scenario %s - 'SELECT <class columns> FROM MasterTable WHERE CLASSID = <class id>' - %.4f s - %d classes [%d properties each], %d instances per class - %s",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        m_sqlTestItems.size (),
        context.GetDomainPropertyCount (),
        instancesPerClassCount);

    for (auto const& kvPair : m_sqlTestItems)
        {
        LOG.debugv ("Scenario %s - Non-generic SELECT - Test SQL: %s", ToString ().c_str (), kvPair.second.m_nonGenericSelectSql.c_str ());
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::RunGenericSelectTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();
    StopWatch timer (true);

    //SELECT is generic, can be used for all ECClasses -> only prepare once
    Statement stmt;
    auto stat = stmt.Prepare (db, m_genericSelectSql.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for generic SELECT: " << m_genericSelectSql.c_str ();

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto testClass = kvPair.first;
        stat = stmt.BindInt64 (1, testClass->GetId ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Binding ECClassId failed for generic SELECT: " << m_genericSelectSql.c_str ();
        //Does not read values from all columns, but only those belonging to this ECClass

        const std::vector<std::pair<int, int>> columnRanges = { m_systemColumnsSelectClauseRangeInGenericSelect, kvPair.second.m_domainColumnsSelectClauseRangeInGenericSelect };
        IterateResultSet (stmt, columnRanges);

        stmt.Reset ();
        stmt.ClearBindings ();
        }

    timer.Stop ();

    LOG.infov ("Scenario %s - 'SELECT * FROM MasterTable WHERE ECCLASSID = ?' - %.4f s - %d classes [%d properties each], %d instances per class",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        m_sqlTestItems.size (),
        context.GetDomainPropertyCount (),
        instancesPerClassCount);

    LOG.debugv ("Scenario %s - Generic SELECT - Test SQL: %s", ToString ().c_str (), m_genericSelectSql.c_str ());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::_GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema)
    {
    auto baseClass = testSchema.GetClassCP (BASE_CLASS_NAME);
    ASSERT_TRUE (baseClass != nullptr);
    Utf8String tableName = GetTableName (*baseClass);

    //Generic INSERT (add all columns and parameters for all columns, irrelevant columns are not bound which means they are set to NULL)
    m_genericInsertSql.append ("INSERT INTO ").append (tableName).append (" (");
    Utf8String genericInsertValuesSql (") VALUES (");
    
    //Generic SELECT (SELECT * FROM Foo WHERE CLASSID = ?)
    m_genericSelectSql.append ("SELECT ");
    
    int insertParameterIndex = 1; //classid column has already index 1
    int selectClauseItemIndex = 0;

    m_systemColumnsSelectClauseRangeInGenericSelect.first = selectClauseItemIndex;
    bool isFirstItem = true;
    for (auto prop : baseClass->GetProperties (false))
        {
        if (!isFirstItem)
            {
            m_genericInsertSql.append (", ");
            genericInsertValuesSql.append (", ");
            m_genericSelectSql.append (", ");
            }

        //only domain props need to be prefixed with class name
        AppendPropToSql (m_genericInsertSql, *prop, nullptr);
        genericInsertValuesSql.append ("?");

        AddParameterMapping (*prop, insertParameterIndex);

        AppendPropToSql (m_genericSelectSql, *prop, nullptr);

        insertParameterIndex++;
        selectClauseItemIndex++;
        isFirstItem = false;
        }

    m_systemColumnsSelectClauseRangeInGenericSelect.second = selectClauseItemIndex - 1;

    for (auto testClass : testSchema.GetClasses ())
        {
        if (!testClass->GetIsDomainClass ()) //base class was already handled above
            continue;

        Utf8String className (testClass->GetName ());

        SqlTestItem& testItem = m_sqlTestItems[testClass];

        //for insert only consider domain properties (exclude inherited system props)
        testItem.m_domainColumnsSelectClauseRangeInGenericSelect.first = selectClauseItemIndex;
        for (auto prop : testClass->GetProperties (false))
            {
            m_genericInsertSql.append (", ");
            AppendPropToSql (m_genericInsertSql, *prop, className.c_str ());

            genericInsertValuesSql.append (", ?");
            AddParameterMapping (*prop, insertParameterIndex);

            m_genericSelectSql.append (", ");
            AppendPropToSql (m_genericSelectSql, *prop, className.c_str ());

            insertParameterIndex++;
            selectClauseItemIndex++;
            }

        testItem.m_domainColumnsSelectClauseRangeInGenericSelect.second = selectClauseItemIndex - 1;

        //The non-generic SELECT statement has an ECClass specific select clause. So it needs to be
        //prepared for each ECClass. Therefore the WHERE CLASSID expression can be hard-coded too
        //(no binding needed for the class id)
        Utf8StringR nonGenericSelectSql = testItem.m_nonGenericSelectSql;
        nonGenericSelectSql.append ("SELECT ");
        isFirstItem = true;
        for (auto prop : testClass->GetProperties (true))
            {
            if (!isFirstItem)
                nonGenericSelectSql.append (", ");

            const bool isBaseProperty = &prop->GetClass () == baseClass;
            AppendPropToSql (nonGenericSelectSql, *prop, isBaseProperty ? nullptr : className.c_str ());
            isFirstItem = false;
            }

        Utf8String classIdStr;
        classIdStr.Sprintf ("%lld", testClass->GetId ());
        nonGenericSelectSql.append (" FROM ").append (tableName).append (" WHERE ").append (CLASSID_COLUMN_NAME).append (" = ").append (classIdStr);
        }

    m_genericInsertSql.append (genericInsertValuesSql).append (");");
    m_genericSelectSql.append (" FROM ").append (tableName).append (" WHERE ").append (CLASSID_COLUMN_NAME).append (" = ? ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableScenario_TestFixture::_GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECSchemaCR testSchema) const
    {
    bool isFirstItem = true;

    for (auto testClass : testSchema.GetClasses ())
        {
        Utf8String tableName = GetTableName (*testClass);
        const bool isBaseClass = !testClass->GetIsDomainClass ();

        //CREATE TABLE
        if (isBaseClass) //base class represents the master table, so only CREATE TABLE for base class
            createTablesSql.append ("CREATE TABLE ").append (tableName).append (" (");

        //add system columns (from base class) and all domain columns (from all other classes)
        for (auto prop : testClass->GetProperties (false))
            {
            if (!isFirstItem)
                createTablesSql.append (", ");

            AppendPropDdlToSql (createTablesSql, *prop, isBaseClass ? nullptr : tableName.c_str()); //system columns are not prefixed with table name
            isFirstItem = false;
            }

        if (isBaseClass) //indexes only needed once
            {
            //CREATE INDEX
            AppendIndexSql (createIndicesSql, tableName.c_str (), CLASSID_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), PARENTECINSTANCEID_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), BUSINESSKEY_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), GLOBALID_COLUMN_NAME, true);
            }
        }

    //close the master table CREATE TABLE statement now
    createTablesSql.append (");");
    }


//****************************************************************************************
// Performance_MasterTableAndDomainTablesScenario_BisDesignTestFixture
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture::_RunInsertTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();

    StopWatch timer (true);

    Statement masterStmt;
    auto stat = masterStmt.Prepare (db, m_masterInsertSql.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for master insert: " << db.GetLastError ().c_str() << " SQL: " << m_masterInsertSql.c_str ();

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto testClass = kvPair.first;
        auto const& testItem = kvPair.second;
        auto basePropertyCollection = testClass->GetBaseClasses ()[0]->GetProperties (false);
        auto domainPropertyCollection = testClass->GetProperties (false);

        Utf8StringCR domainInsertSql = testItem.m_domainInsertSql;

        Statement domainStmt;
        stat = domainStmt.Prepare (db, domainInsertSql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for domain insert: " << db.GetLastError ().c_str() << " SQL: " << domainInsertSql.c_str ();

        for (int i = 0; i < instancesPerClassCount; i++)
            {
            uint64_t ecInstanceId = GetNextECInstanceId ();
            for (auto prop : basePropertyCollection)
                {
                BindToParameter (masterStmt, *testClass, *prop, ecInstanceId);
                }

            stat = masterStmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step failed for master insert: " << db.GetLastError ().c_str() << " SQL: " << m_masterInsertSql.c_str ();
            masterStmt.Reset ();
            masterStmt.ClearBindings ();

            //ECInstanceId Foreign Key in domain table is not represented by properties, so binding manually here
            domainStmt.BindInt64 (1, ecInstanceId);
            for (auto prop : domainPropertyCollection)
                {
                BindToParameter (domainStmt, *testClass, *prop, ecInstanceId);
                }

            stat = domainStmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Step failed for domain insert: " << db.GetLastError ().c_str() << " SQL: " << domainInsertSql.c_str ();
            domainStmt.Reset ();
            domainStmt.ClearBindings ();
            }
        }

    timer.Stop ();

    const auto classCount = m_sqlTestItems.size ();
    const auto totalInserts = instancesPerClassCount * classCount * 2;

    LOG.infov ("Scenario %s - INSERT - %.4f s - %d classes [%d properties each], %d instances per class, %d total inserts",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        classCount,
        context.GetDomainPropertyCount (),
        instancesPerClassCount,
        totalInserts);

    LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), m_masterInsertSql.c_str ());
    for (auto const& kvPair : m_sqlTestItems)
        {
        LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), kvPair.second.m_domainInsertSql.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture::_RunSelectTest (DbR db, Context const& context) const
    {
    const int instancesPerClassCount = context.GetInstancesPerClassCount ();
    StopWatch timer (true);

    for (auto const& kvPair : m_sqlTestItems)
        {
        auto const& testItem = kvPair.second;
        Utf8StringCR selectSql = testItem.m_selectSql;

        Statement stmt;
        auto stat = stmt.Prepare (db, selectSql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparation failed for select: " << db.GetLastError ().c_str() << " SQL: " << selectSql.c_str ();

        IterateResultSet (stmt);
        }

    timer.Stop ();

    LOG.infov ("Scenario %s - SELECT - %.4f s - %d classes [%d properties each], %d instances per class",
        ToString ().c_str (),
        timer.GetElapsedSeconds (),
        m_sqlTestItems.size (),
        context.GetDomainPropertyCount (),
        instancesPerClassCount);

    for (auto const& kvPair : m_sqlTestItems)
        {
        LOG.debugv ("Scenario %s - Test SQL: %s", ToString ().c_str (), kvPair.second.m_selectSql.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture::_GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema)
    {
    //INSERT: 1 for master table and 1 for domain table
    //SELECT: SELECT m.*, d.* FROM Mastertable m, Domaintable t WHERE m.ECInstanceId = d.ECInstanceId

    auto baseClass = testSchema.GetClassCP (BASE_CLASS_NAME);
    ASSERT_TRUE (baseClass != nullptr);
    Utf8String masterTableName = GetTableName (*baseClass);

    //process base class properties -> for master table insert and select clause's master columns
    m_masterInsertSql.append ("INSERT INTO ").append (masterTableName).append (" (");
    Utf8String masterInsertValuesSql (") VALUES (");
    Utf8String selectClauseSql ("SELECT ");

    int parameterIndex = 1;
    bool isFirstItem = true;
    for (auto prop : baseClass->GetProperties (false))
        {
        if (!isFirstItem)
            {
            m_masterInsertSql.append (", ");
            masterInsertValuesSql.append (", ");
            selectClauseSql.append (", ");
            }

        AppendPropToSql (m_masterInsertSql, *prop, nullptr);
        masterInsertValuesSql.append ("?");

        AddParameterMapping (*prop, parameterIndex);
        //system props use table alias of master table 'm'
        selectClauseSql.append ("m.");
        AppendPropToSql (selectClauseSql, *prop, nullptr);

        parameterIndex++;

        isFirstItem = false;
        }

    m_masterInsertSql.append (masterInsertValuesSql).append (");");

    //process domain classes -> domain table insert and select that joins master and domain table
    for (auto testClass : testSchema.GetClasses ())
        {
        if (!testClass->GetIsDomainClass ()) //base class was already handled above
            continue;

        Utf8String domainTableName (testClass->GetName ());

        SqlTestItem& testItem = m_sqlTestItems[testClass];
        Utf8StringR domainInsertSql = testItem.m_domainInsertSql;
        domainInsertSql.append ("INSERT INTO ").append (domainTableName).append ("(").append (ECINSTANCEID_COLUMN_NAME);

        Utf8String domainInsertValuesSql (") VALUES (?");
        Utf8StringR selectSql = testItem.m_selectSql;
        selectSql.append (selectClauseSql);

        parameterIndex = 2;
        for (auto prop : testClass->GetProperties (false))
            {
            domainInsertSql.append (", ");
            AppendPropToSql (domainInsertSql, *prop, nullptr);
            domainInsertValuesSql.append (", ?");

            AddParameterMapping (*prop, parameterIndex);

            selectSql.append (", d.");
            AppendPropToSql (selectSql, *prop, nullptr);

            parameterIndex++;

            isFirstItem = false;
            }

        domainInsertSql.append (domainInsertValuesSql).append (");");

        selectSql.append (" FROM ").append (masterTableName).append (" m, ").append (domainTableName).append (" d WHERE m.");
        selectSql.append (ECINSTANCEID_COLUMN_NAME).append (" = d.").append (ECINSTANCEID_COLUMN_NAME);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
void Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture::_GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECSchemaCR testSchema) const
    {
    for (auto testClass : testSchema.GetClasses ())
        {
        Utf8String tableName = GetTableName (*testClass);
        const bool isBaseClass = !testClass->GetIsDomainClass ();

        //CREATE TABLE
        createTablesSql.append ("CREATE TABLE ").append (tableName).append (" (");

        if (!isBaseClass)
            createTablesSql.append (ECINSTANCEID_COLUMN_NAME).append (" INTEGER PRIMARY KEY, ");

        bool isFirstItem = true;
        for (auto prop : testClass->GetProperties (false))
            {
            if (!isFirstItem)
                createTablesSql.append (", ");

            AppendPropDdlToSql (createTablesSql, *prop, nullptr);

            isFirstItem = false;
            }

        if (!isBaseClass)
            createTablesSql.append (", FOREIGN KEY (").append (ECINSTANCEID_COLUMN_NAME).append (") REFERENCES ").append (Utf8String (BASE_CLASS_NAME)).append ("(").append (ECINSTANCEID_COLUMN_NAME).append (")");

        createTablesSql.append (");");

        if (isBaseClass) //indexed columns are only in master table
            {
            //CREATE INDEX
            AppendIndexSql (createIndicesSql, tableName.c_str (), CLASSID_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), PARENTECINSTANCEID_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), BUSINESSKEY_COLUMN_NAME, false);
            AppendIndexSql (createIndicesSql, tableName.c_str (), GLOBALID_COLUMN_NAME, true);
            }
        else
            {
            //AppendIndexSql (createIndicesSql, tableName.c_str (), ECINSTANCEID_COLUMN_NAME, true);

            }
        }
    }

END_ECDBUNITTESTS_NAMESPACE

