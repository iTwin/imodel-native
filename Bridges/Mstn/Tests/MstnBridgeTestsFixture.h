/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTestsFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnDb.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct MstnBridgeTestsFixture : ::testing::Test
    {
    static BentleyApi::BeFileName GetOutputDir();

    static BentleyApi::BeFileName  GetOutputFileName(BentleyApi::WCharCP filename);

    static BentleyApi::BeFileName GetSeedFilePath() { auto path = GetOutputDir(); path.AppendToPath(L"seed.bim"); return path; }

    static BentleyApi::BeFileName GetDgnv8BridgeDllName();

    static BentleyApi::BeFileName GetSeedFile();

    static void SetUpTestCase();

    static BentleyApi::BeFileName getiModelBridgeTestsOutputDir(WCharCP subdir);

    void MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix);

    void SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir = nullptr, WCharCP bridgeRegSubkey = nullptr, bool setCredentials = true, WCharCP iModelName = nullptr);

    void AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement);

    int64_t AddLine(BentleyApi::BeFileName& inputFile, int count = 1);

    struct DbFileInfo
        {
        BentleyApi::Dgn::DgnDbPtr m_db;
        BentleyApi::Dgn::ScopedDgnHost m_host;
        DbFileInfo(BentleyApi::BeFileNameCR fileName);
        int32_t GetElementCount();
        int32_t GetModelCount();
        int32_t GetPhysicalModelCount();
        int32_t GetBISClassCount(CharCP className);
        int32_t GetModelProvenanceCount(BentleyApi::BeSQLite::BeGuidCR fileGuid);
        BentleyApi::BentleyStatus GetiModelElementByDgnElementId(BentleyApi::Dgn::DgnElementId& elementId, int64_t srcElementId);
        };
    

    static void RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args);
    
    static void TerminateHost();

    static void SetupTestDirectory(BentleyApi::BeFileNameR dirPath,  BentleyApi::WCharCP dirName, BentleyApi::WCharCP iModelName,
                                   BentleyApi::BeFileNameCR input1, BentleyApi::BeSQLite::BeGuidCR inputGuid, 
                                   BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid);
    };
