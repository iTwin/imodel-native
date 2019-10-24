/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ImporterTests.h"

USING_NAMESPACE_DWG

ImporterTestsHost   ImporterTests::s_testsHost;
WString             ImporterTests::s_dwgBridgeRegistryKey;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ImporterTests::GetIBimFileName(BentleyApi::BeFileName& inFile)
    {
    BentleyApi::BeFileName ibimFile(inFile.GetDirectoryName(inFile.c_str()));
    ibimFile.AppendToPath(inFile.GetFileNameWithoutExtension().c_str());
    ibimFile.AppendExtension(L"bim");
    return ibimFile;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString ImporterTests::GetDataSourcePath()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    BentleyApi::WString returnStr(filepath.c_str());
    return returnStr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ImporterTests::GetInputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    filepath.AppendToPath(filename);
    return filepath;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ImporterTests::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTests::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    BentleyApi::BeFileName inFile = GetInputFileName(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inFile, outFile)) << "Unable to copy file \nSource: [" << BentleyApi::Utf8String(inFile.c_str()).c_str() << "]\nDestination: [" << BentleyApi::Utf8String(outFile.c_str()).c_str() << "]";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
void ImporterTests::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::BeFileNameCR inputFileName, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inputFileName, outFile)) << "Unable to copy file \nSource: [" << Utf8String(inputFileName.c_str()).c_str() << "]\nDestination: [" << Utf8String(outFile.c_str()).c_str() << "]";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString ImporterTests::GetOutRoot()
    {
    BentleyApi::BeFileName outPath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(outPath);
    BentleyApi::WString returnStr(outPath.c_str());
    return returnStr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ImporterTests::GetOutputDir()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetOutputRoot (filepath);
    filepath.AppendToPath(L"Output");
    BentleyApi::BeFileName::CreateNewDirectory (filepath);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterTests::DeleteExistingDgnDb(BentleyApi::BeFileNameCR dgnDbFileName)
    {
    BentleyApi::BeFileName dgnDbFileNameAndRelatedFiles(dgnDbFileName);
    dgnDbFileNameAndRelatedFiles.append(L".*");
    BentleyApi::BeFileListIterator iter(dgnDbFileNameAndRelatedFiles, false);
    BentleyApi::BeFileName file;
    while (BentleyApi::SUCCESS == iter.GetNextFileName(file))
        {
        ASSERT_EQ( BentleyApi::BeFileNameStatus::Success , BentleyApi::BeFileName::BeDeleteFile(file) );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr ImporterTests::OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode)
    {
    DbResult fileStatus;

    DgnDb::OpenParams openParams(mode);
    return DgnDb::OpenDgnDb(&fileStatus, projectName, openParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    ImporterTests::SetUpTestCase ()
    {
    DgnPlatformLib::Initialize (s_testsHost);
    DwgImporter::Initialize (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    ImporterTests::TearDownTestCase ()
    {
    s_testsHost.Terminate (false);
    DwgImporter::TerminateDwgHost ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WStringCR ImporterTests::GetDwgBridgeRegistryKey ()
    {
    if (s_dwgBridgeRegistryKey.empty())
        {
        // DwgBridge.dll should reside in the same directory:
        auto handle = ::LoadLibrary (L"DwgBridgeM02.dll");
        if (nullptr != handle)
            {
            auto getRegistryKey = (bool(*)(wchar_t*,const size_t))::GetProcAddress (handle, "?DwgBridge_getBridgeRegistryKey@@YA_NPEA_W_K@Z");
            if (nullptr != getRegistryKey)
                {
                wchar_t reg[100] = { 0 };
                if ((*getRegistryKey)(reg, sizeof(reg)))
                    s_dwgBridgeRegistryKey.assign (reg);
                }
            }
        }
    EXPECT_FALSE (s_dwgBridgeRegistryKey.empty()) << "Cannot get DwgBridge registry key!";
    return  s_dwgBridgeRegistryKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Utf8String  ImporterTests::BuildModelspaceModelname (BeFileNameCR dwgFilename)
    {
    // apply the model naming rule from ConvertConfig.xml
    return Utf8PrintfString ("%ls", dwgFilename.GetFileNameWithoutExtension().c_str());
    }
