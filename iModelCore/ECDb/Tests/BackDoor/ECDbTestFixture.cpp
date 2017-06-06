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

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
// static
bmap<bpair<WString, int>, Utf8String> ECDbTestFixture::s_seedECDbs; // empty
bool ECDbTestFixture::s_isInitialized = false;


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
void ECDbTestFixture::SetUp()
    {
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDb& ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName)
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    CreateECDb(m_ecdb, ecdbFileName);
    return GetECDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan     02/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECDb& ECDbTestFixture::Reopen()
    {
    EXPECT_TRUE(m_ecdb.IsDbOpen()) << "Don't call Reopen on a closed ECDb";
    
    if (m_ecdb.IsDbOpen()) 
        {
        BeFileName ecdbFileName(m_ecdb.GetDbFileName());
        const bool isReadonly = m_ecdb.IsReadonly();
        m_ecdb.CloseDb();
        EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(ecdbFileName, Db::OpenParams(isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite)));
        }

    return GetECDb();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDb& ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, int instanceCountPerClass, ECDb::OpenParams openParams)
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    bpair<WString, int> seedFileKey(schemaECXmlFileName.c_str(), instanceCountPerClass);

    BeFileName seedFilePath;
    auto seedIter = s_seedECDbs.find(seedFileKey);
    if (s_seedECDbs.end() == seedIter)
        {
        Utf8String seedFileName;
        seedFileName.Sprintf("seed_%s_%d", schemaECXmlFileName.GetNameUtf8().c_str(), instanceCountPerClass);
        seedFileName.ReplaceAll(".", "_");
        seedFileName.append(".ecdb");

        if (SUCCESS != CreateECDb(seedFilePath, seedFileName.c_str(), schemaECXmlFileName, instanceCountPerClass))
            return m_ecdb; //return a closed ECDb in case of error

        s_seedECDbs[seedFileKey] = seedFilePath.GetNameUtf8();
        }
    else
        seedFilePath.AppendUtf8(seedIter->second.c_str());

    CloneECDb(m_ecdb, ecdbFileName, seedFilePath, openParams);
    return m_ecdb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECDb& ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, int instanceCountPerClass, ECDb::OpenParams openParams) const
    {
    if (m_ecdb.IsDbOpen())
        m_ecdb.CloseDb();

    BeFileName ecdbPath;

    {
    if (BE_SQLITE_OK != CreateECDb(ecdbPath, ecdbFileName, schema, instanceCountPerClass))
        return GetECDb();
    }

    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(ecdbPath, openParams));
    return GetECDb();
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
// @bsimethod                                     Krischan.Eberle    10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::CreateECDb(BeFileNameR filePath, Utf8CP fileName, BeFileNameCR schemaECXmlFileName, int instanceCountPerClass)
    {
    ECDb ecdb;
    if (SUCCESS != CreateECDb(ecdb, fileName))
        return ERROR;

    ECSchemaReadContextPtr schemaReadContext = nullptr;
    ECSchemaPtr schema = ReadECSchemaFromDisk(schemaReadContext, ecdb, schemaECXmlFileName);
    if (schema == nullptr)
        return ERROR;

    if (SUCCESS != ecdb.Schemas().ImportSchemas(schemaReadContext->GetCache().GetSchemas()))
        return ERROR;

    Populate(ecdb, instanceCountPerClass);
    ecdb.SaveChanges();

    filePath.AssignUtf8(ecdb.GetDbFileName());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle    10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::CreateECDb(BeFileNameR filePath, Utf8CP fileName, SchemaItem const& schemaItem, int instanceCountPerClass)
    {
    ECDb ecdb;
    if (SUCCESS != CreateECDb(ecdb, fileName))
        return ERROR;

    ECSchemaReadContextPtr context = nullptr;
    if (SUCCESS != ReadECSchemaFromString(context, ecdb, schemaItem))
        return ERROR;

    if (SUCCESS != ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        return ERROR;

    Populate(ecdb, instanceCountPerClass);
    ecdb.SaveChanges();
    filePath.AppendUtf8(ecdb.GetDbFileName());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName)
    {
    Initialize();

    BeFileName ecdbFilePath = BuildECDbPath(ecdbFileName);
    if (ecdbFilePath.DoesPathExist())
        {  // Delete any previously created file
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(ecdbFilePath.GetName()));
        }

    BeFileName seedFilePath = BuildECDbPath("seed.ecdb");
    if (!seedFilePath.DoesPathExist())
        {
        ECDb seedDb;
        DbResult stat = seedDb.CreateNewDb(seedFilePath);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return CloneECDb(ecdb, ecdbFileName, seedFilePath, Db::OpenParams(Db::OpenMode::ReadWrite));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::Populate(ECDbCR ecdb, int instanceCountPerClass)
    {
    if (instanceCountPerClass > 0)
        {
        bvector<ECN::ECSchemaCP> schemas = ecdb.Schemas().GetSchemas(true);
        for (ECSchemaCP schema : schemas)
            {
            if (schema->IsStandardSchema() || schema->IsSystemSchema() || schema->GetName().EqualsIAscii("ECDbFileInfo") || schema->GetName().EqualsIAscii("ECDbSystem"))
                continue;

            Populate(ecdb, *schema, instanceCountPerClass);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::Populate(ECDbCR ecdb, ECSchemaCR schema, int instanceCountPerClass)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestFixture::GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className)
    {
    instances.clear();

    ECN::ECClassCP ecClass = GetECDb().Schemas().GetClass(schemaName, className);
    EXPECT_TRUE(ecClass != nullptr) << "ECDbTestFixture::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), className);
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare(GetECDb(), ecSql.GetUtf8CP());
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
// @bsimethod                                  Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String ECDbTestFixture::RetrieveDdl(ECDbCR ecdb, Utf8CP entityName, Utf8CP entityType)
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

    ECSchemaPtr schema = nullptr;
    ECSchema::ReadFromXmlFile(schema, ecSchemaFile.GetName(), *context);
    return schema;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::ExecuteNonSelectECSql(ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step();
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


    for (Utf8StringCR schemaXml : schemaItem.m_schemaXmlList)
        {
        ECSchemaPtr schema = nullptr;
        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context))
            {
            context = nullptr;
            return ERROR;
            }
        }

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
END_ECDBUNITTESTS_NAMESPACE
