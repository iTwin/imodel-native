/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/BackDoor/ECDb/ECDbTestFixture.h"
#include "PublicApi/BackDoor/ECDb/ECDbTestProject.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
// static
bmap<std::pair<WCharCP, int>, Utf8String> ECDbTestFixture::s_seedECDbs; // empty
bool ECDbTestFixture::s_isInitialized = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbTestFixture::SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(m_ecdb, ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestFixture::CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    Initialize();

    BeFileName ecdbPath;
    // Create and populate a sample project
    {
    Utf8String seedPath;
    bmap<std::pair<WCharCP, int>, Utf8String>::const_iterator seedIter = s_seedECDbs.find(std::make_pair(schemaECXmlFileName, perClassRowCount));
    if (s_seedECDbs.end() == seedIter)
        {
        Utf8String seedName;
        seedName.Sprintf("ECDbTestFixture%d.ecdb", s_seedECDbs.size() + 1);
        ECDbTestProject testProject;
        ECDb& ecdb = testProject.Create(seedName.c_str(), schemaECXmlFileName.c_str(), perClassRowCount);
        seedPath = ecdb.GetDbFileName();
        s_seedECDbs[std::make_pair(schemaECXmlFileName, perClassRowCount)] = seedPath;
        }
    else
        seedPath = seedIter->second;

    BeTest::GetHost().GetOutputRoot(ecdbPath);
    ecdbPath.AppendToPath(WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str());
    BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(ecdbPath).c_str());
    BeFileName::BeCopyFile(BeFileName(seedPath.c_str()), ecdbPath);
    }

    //re-open the file so that we can determine the open mode
    return ecdb.OpenBeSQLiteDb(ecdbPath, openParams);
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
