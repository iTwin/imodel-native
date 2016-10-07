/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());

    ECSchemaPtr schema = nullptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schema, schemaReadContext, schemaECXmlFileName);
    if (schema == nullptr)
        return ERROR;

    if (SUCCESS != ecdb.Schemas().ImportECSchemas(schemaReadContext->GetCache().GetSchemas()))
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

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    for (Utf8StringCR schemaXml : schemaItem.m_schemaXmlList)
        {
        if (SUCCESS != ECDbTestUtility::ReadECSchemaFromString(context, schemaXml.c_str()))
            return ERROR;
        }

    if (SUCCESS != ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas()))
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

    BeFileName ecdbFilePath = ECDbTestUtility::BuildECDbPath(ecdbFileName);
    if (ecdbFilePath.DoesPathExist())
        {  // Delete any previously created file
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(ecdbFilePath.GetName()));
        }

    return ecdb.CreateNewDb(ecdbFilePath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::Populate(ECDbCR ecdb, int instanceCountPerClass)
    {
    if (instanceCountPerClass > 0)
        {
        bvector<ECN::ECSchemaCP> schemas = ecdb.Schemas().GetECSchemas(true);
        for (ECSchemaCP schema : schemas)
            {
            if (schema->IsStandardSchema() || schema->IsSystemSchema() || schema->GetName().EqualsI("ecdb_fileinfo"))
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

        ECInstanceInserter inserter(ecdb, *ecClass);
        if (!inserter.IsValid())
            continue;

        for (int i = 0; i < instanceCountPerClass; i++)
            {
            IECInstancePtr ecInstance = ECDbTestUtility::CreateArbitraryECInstance(*ecClass);
            if (SUCCESS != inserter.Insert(*ecInstance))
                return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestFixture::GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className)
    {
    instances.clear();

    ECN::ECClassCP ecClass = GetECDb().Schemas().GetECClass (schemaName, className);
    EXPECT_TRUE (ecClass != nullptr) << "ECDbTestFixture::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql ("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), className);
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (GetECDb(), ecSql.GetUtf8CP());
    EXPECT_EQ(ECSqlStatus::Success, status) << "ECDbTestFixture::GetInstances> Preparing ECSQL '" << ecSql.GetUtf8CP () << "' failed.";
    if (status != ECSqlStatus::Success)
        return ERROR;

    ECInstanceECSqlSelectAdapter adapter (ecStatement);
    while (BE_SQLITE_ROW == ecStatement.Step())
        {
        ECN::IECInstancePtr instance = adapter.GetInstance();
        BeAssert (instance.IsValid());
        if (instance != nullptr)
            instances.push_back (instance);
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
        srand((uint32_t) BeTimeUtilities::QueryMillisecondsCounter());
        s_isInitialized = true;
        }
    }

END_ECDBUNITTESTS_NAMESPACE
