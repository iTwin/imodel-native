/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnV8/Tests/MstnBridgeTestsFixture.h $
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

    void SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args);

    void AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement);

    void AddLine(BentleyApi::BeFileName& inputFile);

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
        };
    

    static void RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args);
    
    static void TerminateHost();

    static void SetupTestDirectory(BentleyApi::BeFileNameR dirPath,  BentleyApi::WCharCP dirName,
                                   BentleyApi::BeFileNameCR input1, BentleyApi::BeSQLite::BeGuidCR inputGuid, 
                                   BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid);
    };
