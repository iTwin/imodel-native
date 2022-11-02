/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
// ECDbTestFixture
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
bool ECDbTestFixture::s_isInitialized = false;
ECDbTestFixture::SeedECDbManager* ECDbTestFixture::s_seedECDbManager = nullptr;
ECSchemaPtr ECDbTestFixture::s_unitsSchema;
ECSchemaPtr ECDbTestFixture::s_formatsSchema;

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName)
    {
    CloseECDb();
    return CreateECDb(m_ecdb, ecdbFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams const& openParams)
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
            seedECDb.SaveChanges();
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
        ecdb.AbandonChanges();
        return ERROR;
        }

    ecdbPath.AssignUtf8(ecdb.GetDbFileName());
    ecdb.SaveChanges();
    }
    
    //reopen the file after creating and importing the schema
    return BE_SQLITE_OK == m_ecdb.OpenBeSQLiteDb(ecdbPath, openParams) ? SUCCESS : ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, void* fileData, uint32_t fileSize, ECDb::OpenParams const& openParams)
    {
    CloseECDb();
    BeFileName ecdbPath;
    if (true) {
        ECDb ecdb;
        if (BE_SQLITE_OK != CreateECDb(ecdb, ecdbFileName)) {
            return ERROR;
        }
        ecdbPath.AssignUtf8(ecdb.GetDbFileName());
    }
    // Write file.
    BeFile binaryFile;
    binaryFile.Create(ecdbPath.GetNameUtf8(), true);
    binaryFile.Write(nullptr, fileData, fileSize);
    binaryFile.Flush();
    binaryFile.Close();
    //reopen the file after creating and importing the schema
    return BE_SQLITE_OK == m_ecdb.OpenBeSQLiteDb(ecdbPath, openParams) ? SUCCESS : ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbTestFixture::CloseECDb()
    {
    if (m_ecdb.IsDbOpen()) 
        m_ecdb.SaveChanges();
        m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::OpenECDb(BeFileNameCR filePath, ECDb::OpenParams const& params)
    {
    if (m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    return m_ecdb.OpenBeSQLiteDb(filePath, params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::ReopenECDb()
    {
    if (!m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    const bool isReadonly = m_ecdb.IsReadonly();
    return ReopenECDb(ECDb::OpenParams(isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestFixture::ReopenECDb(ECDb::OpenParams const& openParams)
    {
    if (!m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    BeFileName ecdbFileName(m_ecdb.GetDbFileName());
    CloseECDb();

    return OpenECDb(ecdbFileName, openParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
        if (BeFileNameStatus::Success != ecdbFilePath.BeDeleteFile())
            {
            EXPECT_FALSE(true) << "Could not delete ecdb file " << ecdbFilePath.GetNameUtf8().c_str();
            return BE_SQLITE_ERROR;
            }

        BeFileName changeCachePath = ECDb::GetDefaultChangeCachePath(ecdbFilePath.GetNameUtf8().c_str());
        if (changeCachePath.DoesPathExist())
            {
            if (BeFileNameStatus::Success != changeCachePath.BeDeleteFile())
                {
                EXPECT_FALSE(true) << "Could not delete change cache file " << changeCachePath.GetNameUtf8().c_str();
                return BE_SQLITE_ERROR;
                }
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

    return CloneECDb(ecdb, effectiveFileName.c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams)
    {
    BeFileName clonePath;
    BeTest::GetHost().GetOutputRoot(clonePath);
    clonePath.AppendToPath(BeFileName(cloneFileName));
    BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(clonePath).c_str());
    BeFileName::BeCopyFile(seedFilePath, clonePath);

    //clone Change cache file
    BeFileName seedChangeCachePath = ECDb::GetDefaultChangeCachePath(seedFilePath.GetNameUtf8().c_str());
    if (seedChangeCachePath.DoesPathExist())
        BeFileName::BeCopyFile(seedChangeCachePath, ECDb::GetDefaultChangeCachePath(clonePath.GetNameUtf8().c_str()));

    return clone.OpenBeSQLiteDb(clonePath, openParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
        bvector<ECN::ECSchemaCP> schemas = m_ecdb.Schemas().GetSchemas();
        for (ECSchemaCP schema : schemas)
            {
            if (schema->IsStandardSchema() || schema->IsSystemSchema() || 
                schema->GetName().EqualsIAscii("ECDbChange") || schema->GetName().EqualsIAscii("ECDbFileInfo") || schema->GetName().EqualsIAscii("ECDbSystem"))
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
// @bsimethod
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
            IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
            ECInstancePopulator::Populate(*instance);
            if (BE_SQLITE_OK != inserter.Insert(*instance))
                return ERROR;
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::ReadECSchema(ECSchemaReadContextPtr& context, ECDbCR ecdb, SchemaItem const& schemaItem)
    {
    if (context == nullptr)
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(ecdb.GetSchemaLocater());
        }

    ScopedDisableFailOnAssertion disableFailOnAssert;

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

    if (SchemaReadStatus::Success != stat)
        {
        context = nullptr;
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
//static
BeFileName ECDbTestFixture::BuildECDbPath(Utf8CP ecdbFileName)
    {
    BeFileName ecdbPath;
    BeTest::GetHost().GetOutputRoot(ecdbPath);
    ecdbPath.AppendToPath(WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str());
    return ecdbPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECSchemaPtr ECDbTestFixture::GetUnitsSchema(bool recreate)
    {
    if(recreate || s_unitsSchema.IsNull())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("Units", 1, 0, 0);
        s_unitsSchema = context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        }
    return s_unitsSchema;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECN::ECSchemaPtr ECDbTestFixture::GetFormatsSchema(bool recreate)
    {
    if (recreate || s_formatsSchema.IsNull())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("Formats", 1, 0, 0);
        s_formatsSchema = context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        }
    return s_formatsSchema;
    }

END_ECDBUNITTESTS_NAMESPACE
