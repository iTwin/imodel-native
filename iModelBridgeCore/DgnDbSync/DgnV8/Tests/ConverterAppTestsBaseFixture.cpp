/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnV8/Tests/ConverterAppTestsBaseFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterAppTestsBaseFixture.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterAppTestsBaseFixture::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterAppTestsBaseFixture::TearDown()
    {
    m_wantCleanUp = false;
    T_Super::TearDown();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ConverterAppTestsBaseFixture::RunCMD(BentleyApi::WString cmd)
    {
    wprintf(L"Command to Run = %ls \n", cmd.c_str());
    char *cmdLocal = new char[cmd.GetMaxLocaleCharBytes()];
    cmd.ConvertToLocaleChars(cmdLocal);
    int status = system(cmdLocal);
    printf("status = %d \n", status);
    delete cmdLocal;
    return status == 0 ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString ConverterAppTestsBaseFixture::GetOutRoot()
    {
    BentleyApi::BeFileName outPath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(L"Output");
    BentleyApi::WString returnStr(outPath.c_str());
    return returnStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName ConverterAppTestsBaseFixture::GetBimFileName(BentleyApi::BeFileName& inFile)
    {
    BentleyApi::BeFileName bimFile(inFile.GetDirectoryName(inFile.c_str()));
    bimFile.AppendToPath(inFile.GetFileNameWithoutExtension().c_str());
    bimFile.AppendExtension(L"ibim");
    return bimFile;
    }
