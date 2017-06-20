/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/ImporterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterTests.h"

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ImporterTests::GetIBimFileName(BentleyApi::BeFileName& inFile)
    {
    BentleyApi::BeFileName ibimFile(inFile.GetDirectoryName(inFile.c_str()));
    ibimFile.AppendToPath(inFile.GetFileNameWithoutExtension().c_str());
    ibimFile.AppendExtension(L"ibim");
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

