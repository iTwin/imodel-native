/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
//************************************************************************************
// TestHelper
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::ImportSchema(SchemaItem const& schema, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    return ECDbTestFixture::ImportSchema(ecdb, schema);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::ImportSchema(ECDbCR ecdb, SchemaItem const& schema) { return ECDbTestFixture::ImportSchema(ecdb, schema); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                       02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestHelper::HasDataCorruptingMappingIssues(ECDbCR ecdb)
    {
    EXPECT_TRUE(ecdb.IsDbOpen());

    if (!ecdb.IsDbOpen())
        return true;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, SchemaManager::GetValidateDbMappingSql()))
        {
        EXPECT_TRUE(false) << ecdb.GetLastError().c_str();
        return true;
        }

    bool hasError = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        hasError = true;
        LOG.errorv("ECClass '%s:%s' with invalid mapping: %s. Table name: %s - %s", stmt.GetValueText(0),
                   stmt.GetValueText(2), stmt.GetValueText(5), stmt.GetValueText(3), stmt.GetValueText(6));
        }

    return hasError;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String TestHelper::RetrieveDdl(ECDbCR ecdb, Utf8CP entityName, Utf8CP entityType)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=? COLLATE NOCASE AND type=? COLLATE NOCASE");
    if (stmt == nullptr)
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(1, entityName, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(2, entityType, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_ROW != stmt->Step())
        return Utf8String();

    return Utf8String(stmt->GetValueText(0));
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult TestHelper::ExecuteNonSelectECSql(ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult TestHelper::TestHelper::ExecuteInsertECSql(ECInstanceKey& key, ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step(key);
    }


//************************************************************************************
// ECDbTestFixture
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
// static
bool ECDbTestFixture::s_isInitialized = false;
bmap<BeFileName, Utf8String> ECDbTestFixture::s_seedECDbs;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName)
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    return CreateECDb(m_ecdb, ecdbFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams)
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    BeFileName seedFilePath;
    auto seedIter = s_seedECDbs.find(schemaECXmlFileName);
    if (s_seedECDbs.end() == seedIter)
        {
        Utf8String seedFileName;
        seedFileName.Sprintf("seed_%s", schemaECXmlFileName.GetNameUtf8().c_str());
        seedFileName.ReplaceAll(".", "_");
        seedFileName.append(".ecdb");

        ECDb seedECDb;
        if (SUCCESS != CreateECDb(seedECDb, seedFileName.c_str()))
            return ERROR;

        if (SUCCESS != ImportSchema(seedECDb, schemaECXmlFileName))
            {
            EXPECT_TRUE(false) << "Importing schema " << schemaECXmlFileName.GetNameUtf8().c_str() << " failed";
            return ERROR;
            }

        seedFilePath.AssignUtf8(seedECDb.GetDbFileName());
        s_seedECDbs[schemaECXmlFileName] = Utf8String(seedECDb.GetDbFileName());
        }
    else
        seedFilePath.AssignUtf8(seedIter->second.c_str());

    return CloneECDb(m_ecdb, ecdbFileName, seedFilePath, openParams) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams openParams)
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    BeFileName ecdbPath;
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != CreateECDb(ecdb, ecdbFileName))
        {
        EXPECT_TRUE(false) << "Creating test ECDb failed (" << ecdbFileName << ")";
        return ERROR;
        }

    if (SUCCESS != ImportSchema(ecdb, schema))
        {
        EXPECT_TRUE(false) << "Importing schema failed: " << schema.ToString().c_str();
        return ERROR;
        }

    ecdbPath.AssignUtf8(ecdb.GetDbFileName());
    }
    
    //reopen the file after creating and importing the schema
    return BE_SQLITE_OK == m_ecdb.OpenBeSQLiteDb(ecdbPath, openParams) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan     02/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::ReopenECDb()
    {
    if (!m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    BeFileName ecdbFileName(m_ecdb.GetDbFileName());
    const bool isReadonly = m_ecdb.IsReadonly();
    m_ecdb.CloseDb();
    return m_ecdb.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName)
    {
    Initialize();

    Utf8String effectiveFileName;
    if (Utf8String::IsNullOrEmpty(ecdbFileName))
        {
        DateTime timeStamp = DateTime::GetCurrentTimeUtc();
        uint64_t timeStampJd = 0;
        if (SUCCESS != timeStamp.ToJulianDay(timeStampJd))
            return BE_SQLITE_ERROR;

        effectiveFileName.Sprintf("ecdbtest_%" PRIu64 ".ecdb", timeStampJd);
        }
    else
        effectiveFileName.assign(ecdbFileName);

    BeFileName ecdbFilePath = BuildECDbPath(effectiveFileName.c_str());
    if (ecdbFilePath.DoesPathExist())
        {  // Delete any previously created file
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(ecdbFilePath.GetName()))
            {
            EXPECT_FALSE(true) << "Could not delete ecdb file " << ecdbFilePath.GetNameUtf8().c_str();
            return BE_SQLITE_ERROR;
            }
        }

    BeFileName seedFilePath = BuildECDbPath("seed.ecdb");
    if (!seedFilePath.DoesPathExist())
        {
        ECDb seedDb;
        const DbResult stat = seedDb.CreateNewDb(seedFilePath);
        EXPECT_EQ(BE_SQLITE_OK, stat) << "Could not create file " << seedFilePath.GetNameUtf8().c_str() << ": " << seedDb.GetLastError().c_str();
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return CloneECDb(ecdb, effectiveFileName.c_str(), seedFilePath, Db::OpenParams(Db::OpenMode::ReadWrite));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle    10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams)
    {
    Initialize();

    BeFileName clonePath;
    BeTest::GetHost().GetOutputRoot(clonePath);
    clonePath.AppendToPath(BeFileName(cloneFileName));
    BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(clonePath).c_str());
    BeFileName::BeCopyFile(seedFilePath, clonePath);
    return clone.OpenBeSQLiteDb(clonePath, openParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::PopulateECDb(ECDbR ecdb, int instanceCountPerClass)
    {
    if (!ecdb.IsDbOpen())
        {
        EXPECT_FALSE(true) << "ECDb is expected to be open when calling ECDbTestFixture::PopulateECDb";
        return ERROR;
        }

    const bool isReadonly = ecdb.IsReadonly();
    BeFileName filePath(ecdb.GetDbFileName());
    if (isReadonly)
        {
        ecdb.CloseDb();

        if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)))
            {
            EXPECT_FALSE(true) << "Could not re-open test file in readwrite mode for populating it";
            return ERROR;
            }
        }

    if (instanceCountPerClass > 0)
        {
        bvector<ECN::ECSchemaCP> schemas = ecdb.Schemas().GetSchemas(true);
        for (ECSchemaCP schema : schemas)
            {
            if (schema->IsStandardSchema() || schema->IsSystemSchema() || schema->GetName().EqualsIAscii("ECDbFileInfo") || schema->GetName().EqualsIAscii("ECDbSystem"))
                continue;

            if (SUCCESS != PopulateECDb(ecdb, *schema, instanceCountPerClass))
                return ERROR;
            }

        }

    if (BE_SQLITE_OK != ecdb.SaveChanges())
        {
        EXPECT_FALSE(true) << "Could not save changes after populating";
        return ERROR;
        }

    if (isReadonly)
        {
        ecdb.CloseDb();

        if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)))
            {
            EXPECT_FALSE(true) << "Could not re-open test file in read-only mode after having populated it";
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::PopulateECDb(ECDbR ecdb, ECSchemaCR schema, int instanceCountPerClass)
    {
    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (!ecClass->IsEntityClass() || ecClass->GetClassModifier() == ECClassModifier::Abstract)
            continue;

        ECInstanceInserter inserter(ecdb, *ecClass, nullptr);
        if (!inserter.IsValid())
            continue;

        for (int i = 0; i < instanceCountPerClass; i++)
            {
            IECInstancePtr ecInstance = ECDbTestUtility::CreateArbitraryECInstance(*ecClass);
            if (BE_SQLITE_OK != inserter.Insert(*ecInstance))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::ImportSchema(ECDbCR ecdb, SchemaItem const& testItem)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    if (SUCCESS != ReadECSchemaFromString(context, ecdb, testItem))
        return ERROR;

    Savepoint sp(const_cast<ECDbR>(ecdb), "ECSchema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return !TestHelper::HasDataCorruptingMappingIssues(ecdb) ? SUCCESS : ERROR;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::ImportSchema(ECDbCR ecdb, BeFileNameCR schemaXmlFilePath)
    {
    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schema = ReadECSchemaFromDisk(context, ecdb, schemaXmlFilePath);
    if (schema == nullptr)
        return ERROR;

    Savepoint sp(const_cast<ECDbR>(ecdb), "ECSchema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return !TestHelper::HasDataCorruptingMappingIssues(ecdb) ? SUCCESS : ERROR;
        }

    sp.Cancel();
    return ERROR;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestFixture::GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className)
    {
    instances.clear();

    ECN::ECClassCP ecClass = m_ecdb.Schemas().GetClass(schemaName, className);
    EXPECT_TRUE(ecClass != nullptr) << "ECDbTestFixture::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), className);
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare(m_ecdb, ecSql.GetUtf8CP());
    EXPECT_EQ(ECSqlStatus::Success, status) << "ECDbTestFixture::GetInstances> Preparing ECSQL '" << ecSql.GetUtf8CP() << "' failed.";
    if (status != ECSqlStatus::Success)
        return ERROR;

    ECInstanceECSqlSelectAdapter adapter(ecStatement);
    while (BE_SQLITE_ROW == ecStatement.Step())
        {
        ECN::IECInstancePtr instance = adapter.GetInstance();
        BeAssert(instance.IsValid());
        if (instance != nullptr)
            instances.push_back(instance);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECDbTestFixture::Initialize()
    {
    if (!s_isInitialized)
        {
        //establish standard schema search paths (they are in the application dir)
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);

        ECDb::Initialize(temporaryDir, &applicationSchemaDir);
        srand((uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
        s_isInitialized = true;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr ECDbTestFixture::ReadECSchemaFromDisk(ECSchemaReadContextPtr& context, ECDbCR ecdb, BeFileNameCR schemaXmlFileName)
    {
    // Construct the path to the sample schema
    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot(ecSchemaPath);
    ecSchemaPath.AppendToPath(L"ECDb");
    ecSchemaPath.AppendToPath(L"Schemas");

    BeFileName ecSchemaFile(ecSchemaPath);
    ecSchemaFile.AppendToPath(schemaXmlFileName);
    if (!BeFileName::DoesPathExist(ecSchemaFile.GetName()))
        return nullptr;

    if (context == nullptr)
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(ecdb.GetSchemaLocater());
        }

    context->AddSchemaPath(ecSchemaPath.GetName());

    const bool wasFailOnAssert = BeTest::GetFailOnAssert();
    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(false);

    ECSchemaPtr schema = nullptr;
    ECSchema::ReadFromXmlFile(schema, ecSchemaFile.GetName(), *context);
    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(true);
    
    return schema;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::ReadECSchemaFromString(ECSchemaReadContextPtr& context, ECDbCR ecdb, SchemaItem const& schemaItem)
    {
    if (context == nullptr)
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(ecdb.GetSchemaLocater());
        }

    const bool wasFailOnAssert = BeTest::GetFailOnAssert();
    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(false);

    for (Utf8StringCR schemaXml : schemaItem.m_schemaXmlList)
        {
        ECSchemaPtr schema = nullptr;
        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context))
            {
            context = nullptr;

            if (wasFailOnAssert)
                BeTest::SetFailOnAssert(true);

            return ERROR;
            }
        }

    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(true);

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
BeFileName ECDbTestFixture::BuildECDbPath(Utf8CP ecdbFileName)
    {
    BeFileName ecdbPath;
    BeTest::GetHost().GetOutputRoot(ecdbPath);
    ecdbPath.AppendToPath(WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str());
    return ecdbPath;
    }

//************************************************************************************
// SchemaItem
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaItem::ToString() const
    {
    Utf8String schemaXmlList;
    for (Utf8StringCR schemaXml : m_schemaXmlList)
        {
        schemaXmlList.append(schemaXml).append("\r\n");
        }

    return schemaXmlList;
    }

END_ECDBUNITTESTS_NAMESPACE
