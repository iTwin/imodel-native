/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnV8/Tests/ConverterAppTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterAppTestsBaseFixture.h"

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct ConverterAppTests : public ConverterAppTestsBaseFixture
    {
    DEFINE_T_SUPER(ConverterAppTestsBaseFixture);
    void SetUp();
    void TearDown();
    };

void ConverterAppTests::SetUp()
    {
    T_Super::SetUp();
    }

void ConverterAppTests::TearDown()
    {
    m_wantCleanUp = false;
    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConverterAppTests, VerifyConversionApp_CreateBim)
    {
    BentleyApi::WString fileName = L"Test3d.dgn";
    BentleyApi::BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    ASSERT_PRESENT(inFile.c_str());

    CreateCommand();
    AddInputFile(inFile.c_str());
    AddOutputFile(GetOutRoot());

    ASSERT_EQ(SUCCESS, RunCMD(m_command));
    BentleyApi::BeFileName outFile = GetBimFileName(inFile);
    ASSERT_PRESENT(outFile.c_str());
    }
