/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/BackDoor/ECDb/ECDbTestFixture.h"


BEGIN_ECDBUNITTESTS_NAMESPACE

// static
bmap<std::pair<WCharCP, int>, Utf8String> ECDbTestFixture::s_seedDbs; // empty
bool ECDbTestFixture::s_isInitialized = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestFixture::ECDbTestFixture() : m_testProject(nullptr) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestProject& ECDbTestFixture::GetTestProject () const
    {
    return _GetTestProject ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestProject& ECDbTestFixture::SetupTestProject(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    m_testProject = std::move(CreateTestProject(ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount));
    return *m_testProject;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Carole.MacDonald     09/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ECDbTestProject> ECDbTestFixture::CreateTestProject(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    Initialize();

    Utf8String filePath;
    // Create and populate a sample project
    {
    Utf8String seedPath;
    bmap<std::pair<WCharCP, int>, Utf8String>::const_iterator seedIter = s_seedDbs.find(std::make_pair(schemaECXmlFileName, perClassRowCount));
    if (s_seedDbs.end() == seedIter)
        {
        Utf8PrintfString seedName("ECDbTestFixture%d.ecdb", s_seedDbs.size() + 1);
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create(seedName.c_str(), schemaECXmlFileName, perClassRowCount);
        seedPath = ecdb.GetDbFileName();
        s_seedDbs[std::make_pair(schemaECXmlFileName, perClassRowCount)] = seedPath;
        }
    else
        seedPath = seedIter->second;

    BeFileName outFullFileName;
    BeTest::GetHost().GetOutputRoot(outFullFileName);
    WString ecdbFileNameW(ecdbFileName, BentleyCharEncoding::Utf8);
    outFullFileName.AppendToPath(ecdbFileNameW.c_str());
    BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(outFullFileName).c_str());
    BeFileName sourceFile(seedPath.c_str());
    BeFileName::BeCopyFile(sourceFile, outFullFileName);
    filePath.Assign(outFullFileName.GetName());
    }

    //re-open the file so that we can determine the open mode
    auto testProject = std::unique_ptr<ECDbTestProject>(new ECDbTestProject());
    testProject->Open(filePath.c_str(), openParams);
    return move(testProject);
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
