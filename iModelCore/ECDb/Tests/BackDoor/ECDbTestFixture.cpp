/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"
#include "PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

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
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDb& ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    EXPECT_EQ(BE_SQLITE_OK, CreateECDb(m_ecdb, ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount));
    return GetECDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECDb& ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, bool importArbitraryNumberOfECInstances, ECDb::OpenParams openParams)
    {
    BeFileName fileName(schemaECXmlFileName);
    EXPECT_EQ(BE_SQLITE_OK, CreateECDb(m_ecdb, ecdbFileName, fileName, openParams, importArbitraryNumberOfECInstances ? 3 : 0));
    return GetECDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    bpair<WString, int> seedFileKey(schemaECXmlFileName.c_str(), perClassRowCount);

    BeFileName seedFilePath;
    auto seedIter = s_seedECDbs.find(seedFileKey);
    if (s_seedECDbs.end() == seedIter)
        {
        Utf8String seedFileName;
        seedFileName.Sprintf("seed_%s_%d", schemaECXmlFileName.GetNameUtf8().c_str(), perClassRowCount);
        seedFileName.ReplaceAll(".", "_");
        seedFileName.append(".ecdb");

        CreateSeedECDb(seedFilePath, seedFileName.c_str(), schemaECXmlFileName, perClassRowCount);
        s_seedECDbs[seedFileKey] = seedFilePath.GetNameUtf8();
        }
    else
        seedFilePath.AppendUtf8(seedIter->second.c_str());

    return CloneECDb(ecdb, ecdbFileName, seedFilePath, openParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle    10/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestFixture::CreateSeedECDb(BeFileNameR seedFilePath, Utf8CP seedFileName, BeFileNameCR schemaECXmlFileName, int perClassRowCount)
    {
    Initialize();

    ECDbTestProject testProject;
    ECDb& ecdb = testProject.Create(seedFileName, schemaECXmlFileName.c_str(), perClassRowCount);

    seedFilePath.AppendUtf8(ecdb.GetDbFileName());
    return SUCCESS;
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
