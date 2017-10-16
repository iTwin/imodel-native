/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
// ECDbTestFixture
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//no need to release a static non-POD variable (Bentley C++ coding standards)
//static
ProfileVersion const* ECDbTestFixture::s_expectedProfileVersion = new ProfileVersion(4, 0, 0, 1);

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//+---------------+---------------+---------------+---------------+---------------+------
// static
bool ECDbTestFixture::s_isInitialized = false;
ECDbTestFixture::SeedECDbManager* ECDbTestFixture::s_seedECDbManager = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     06/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECDbTestFixture::SeedECDbManager& ECDbTestFixture::SeedECDbs()
    {
    //must not destroy the pointer as it is a static member
    if (s_seedECDbManager == nullptr)
        s_seedECDbManager = new SeedECDbManager();

    return *s_seedECDbManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName)
    {
    CloseECDb();
    return CreateECDb(m_ecdb, ecdbFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2015
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams openParams)
    {
    CloseECDb();
    if (schema.GetType() == SchemaItem::Type::File)
        {
        BeFileName schemaFileName = schema.GetFileName();

        BeFileName seedFilePath;
        if (!SeedECDbs().TryGet(seedFilePath, schemaFileName))
            {
            Utf8String seedFileName;
            seedFileName.Sprintf("seed_%s", schemaFileName.GetNameUtf8().c_str());
            seedFileName.ReplaceAll(".", "_");
            seedFileName.append(".ecdb");

            ECDb seedECDb;
            if (SUCCESS != CreateECDb(seedECDb, seedFileName.c_str()))
                return ERROR;

            if (SUCCESS != TestHelper(seedECDb).ImportSchema(schema))
                {
                EXPECT_TRUE(false) << "Importing schema " << schema.ToString().c_str() << " failed";
                return ERROR;
                }

            seedFilePath = SeedECDbs().Add(schemaFileName, BeFileName(seedECDb.GetDbFileName()));
            }

        return CloneECDb(ecdbFileName, seedFilePath, openParams) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    BeAssert(schema.GetType() == SchemaItem::Type::String);
    BeFileName ecdbPath;
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != CreateECDb(ecdb, ecdbFileName))
        {
        //EXPECT_TRUE(false) << "Creating test ECDb failed (" << ecdbFileName << ")";
        return ERROR;
        }

    if (SUCCESS != TestHelper(ecdb).ImportSchema(schema))
        {
        //EXPECT_TRUE(false) << "Importing schema failed.";
        return ERROR;
        }

    ecdbPath.AssignUtf8(ecdb.GetDbFileName());
    }
    
    //reopen the file after creating and importing the schema
    return BE_SQLITE_OK == m_ecdb.OpenBeSQLiteDb(ecdbPath, openParams) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     06/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbTestFixture::CloseECDb()
    {
    if (m_ecdb.IsDbOpen()) 
        m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan     02/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::OpenECDb(BeFileNameCR filePath, ECDb::OpenParams params)
    {
    if (m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    return m_ecdb.OpenBeSQLiteDb(filePath, params);
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
    CloseECDb();
    return OpenECDb(ecdbFileName, Db::OpenParams(isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite));
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
BentleyStatus ECDbTestFixture::PopulateECDb(int instanceCountPerClass)
    {
    if (!m_ecdb.IsDbOpen())
        {
        EXPECT_FALSE(true) << "ECDb is expected to be open when calling ECDbTestFixture::PopulateECDb";
        return ERROR;
        }

    const bool isReadonly = m_ecdb.IsReadonly();
    BeFileName filePath(m_ecdb.GetDbFileName());
    if (isReadonly)
        {
        CloseECDb();

        if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)))
            {
            EXPECT_FALSE(true) << "Could not re-open test file in readwrite mode for populating it";
            return ERROR;
            }
        }

    if (instanceCountPerClass > 0)
        {
        bvector<ECN::ECSchemaCP> schemas = m_ecdb.Schemas().GetSchemas(true);
        for (ECSchemaCP schema : schemas)
            {
            if (schema->IsStandardSchema() || schema->IsSystemSchema() || schema->GetName().EqualsIAscii("ECDbFileInfo") || schema->GetName().EqualsIAscii("ECDbSystem"))
                continue;

            if (SUCCESS != PopulateECDb(*schema, instanceCountPerClass))
                return ERROR;
            }

        }

    if (BE_SQLITE_OK != m_ecdb.SaveChanges())
        {
        EXPECT_FALSE(true) << "Could not save changes after populating";
        return ERROR;
        }

    if (isReadonly)
        {
        CloseECDb();

        if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)))
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
BentleyStatus ECDbTestFixture::PopulateECDb(ECSchemaCR schema, int instanceCountPerClass)
    {
    if (!m_ecdb.IsDbOpen())
        {
        EXPECT_FALSE(true) << "ECDb is expected to be open when calling ECDbTestFixture::PopulateECDb";
        return ERROR;
        }

    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (!ecClass->IsEntityClass() || ecClass->GetClassModifier() == ECClassModifier::Abstract)
            continue;

        ECInstanceInserter inserter(m_ecdb, *ecClass, nullptr);
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


//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::ReadECSchema(ECSchemaReadContextPtr& context, ECDbCR ecdb, SchemaItem const& schemaItem)
    {
    if (context == nullptr)
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(ecdb.GetSchemaLocater());
        }

    const bool wasFailOnAssert = BeTest::GetFailOnAssert();
    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(false);

    ECSchemaPtr schema = nullptr;
    SchemaReadStatus stat = SchemaReadStatus::Success;
    switch (schemaItem.GetType())
        {
            case SchemaItem::Type::String:
            {
            stat = ECSchema::ReadFromXmlString(schema, schemaItem.GetXmlString().c_str(), *context);
            break;
            }

            case SchemaItem::Type::File:
            {
            // Construct the path to the sample schema
            BeFileName ecSchemaFilePath;
            BeTest::GetHost().GetDocumentsRoot(ecSchemaFilePath);
            ecSchemaFilePath.AppendToPath(L"ECDb");
            ecSchemaFilePath.AppendToPath(L"Schemas");
            ecSchemaFilePath.AppendToPath(schemaItem.GetFileName());

            if (!ecSchemaFilePath.DoesPathExist())
                return ERROR;

            context->AddSchemaPath(ecSchemaFilePath.GetName());
            stat = ECSchema::ReadFromXmlFile(schema, ecSchemaFilePath, *context);
            break;
            }

            default:
            {
            BeAssert(false && "Unhandled SchemaItem::Type");
            stat = SchemaReadStatus::InvalidECSchemaXml;
            break;
            }
        }

    if (wasFailOnAssert)
        BeTest::SetFailOnAssert(true);

    if (SchemaReadStatus::Success != stat)
        {
        context = nullptr;
        return ERROR;
        }

    return SUCCESS;
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
