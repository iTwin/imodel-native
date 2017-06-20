/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterBaseFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::SetUp()
    {
    DgnPlatformLib::Initialize(m_host, true); // this initializes the DgnDb libraries
    
    if (!s_initialized)                     // *** NEEDS WORK: I can't currently terminate the V8 host, so I can't call Converter::Initialize more than once
        {
        s_initialized = true;        
        BentleyApi::BeFileName v8DllsRelativeDir(L"DgnV8");
        //Converter::Initialize (m_host, v8DllsRelativeDir, 0, nullptr);
        DwgImporter::InitializeDgnHost(m_host);
        }

    m_options.SetProgressMeter(&m_meter);

    BentleyApi::BeFileName configFileName;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(configFileName);
    configFileName.AppendToPath(L"ImportConfig.xml");
    m_options.SetConfigFile(configFileName);
    m_options.SetSkipUnchangedFiles(false);  // file time granularity is 1 second. That's too long for an automated test.

    BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::TearDown()
    {
    if (m_wantCleanUp)
        BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    m_host.Terminate(false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrintError (WCharCP fmt, ...)
    {
    va_list args; 
    va_start (args, fmt); 
    WPrintfString str (fmt, args);
    va_end(args);

    fwprintf(stderr, str.c_str());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::DoConvert(BentleyApi::BeFileNameCR outputName, BentleyApi::BeFileNameCR inputFileName)
    {
    // start the importer either by creating a new or updating existing DgnDb:
    DwgImporter*    importer = new DwgImporter(m_options);
    ASSERT_NOT_NULL(importer);
    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);
    // create a new or update existing output DgnDb:
    status = importer->CreateNewDgnDb(outputName);
    ASSERT_SUCCESS(status);
    // ready to import DWG to DgnDb:
    status = importer->Process ();
    ASSERT_SUCCESS(status);

    delete importer;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::DoUpdate(BentleyApi::BeFileNameCR outputName, BentleyApi::BeFileNameCR inputFileName, bool expectFailure)
    {

    // start the importer either by creating a new or updating existing DgnDb:
    DwgImporter*    importer = new DwgUpdater(m_options);
    ASSERT_NOT_NULL(importer);

    // try open the inpurt DWG file:
    auto status = importer->OpenDwgFile(inputFileName);
    ASSERT_SUCCESS(status);

    // create a new or update existing output DgnDb:
    status = importer->OpenExistingDgnDb(outputName);
    ASSERT_SUCCESS(status);

    // ready to import DWG to DgnDb:
    status = importer->Process ();
    
    ASSERT_SUCCESS(status);
    delete importer;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputV8FileName, bool doConvert)
    {
    ImporterTests::MakeWritableCopyOf(m_dwgFileName, inputV8FileName);
    m_dgnDbFileName = GetOutputFileName(outputDgnDbFileName);
    DeleteExistingDgnDb(m_dgnDbFileName);
    if (doConvert)
        DoConvert(m_dgnDbFileName, m_dwgFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTestBaseFixture::MakeCopyOfFile(BentleyApi::BeFileNameR refV8File, BentleyApi::WCharCP suffix)
    {
    refV8File = BentleyApi::BeFileName(BentleyApi::BeFileName::FileNameParts::DevAndDir, m_dwgFileName.c_str());
    refV8File.AppendToPath(m_dwgFileName.GetFileNameWithoutExtension().c_str());
    refV8File.append(suffix);
    refV8File.append(L".");
    refV8File.append(m_dwgFileName.GetExtension().c_str());
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(m_dwgFileName.c_str(), refV8File.c_str()));
    }

