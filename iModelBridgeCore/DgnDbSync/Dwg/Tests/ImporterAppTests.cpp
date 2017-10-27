/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterAppTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"
#include "ImporterCommandBuilder.h"

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct ImporterAppTests : public ImporterTests, public ImporterCommandBuilder
{
    BentleyApi::StatusInt RunCMD(BentleyApi::WString);
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ImporterAppTests::RunCMD(BentleyApi::WString cmd)
    {
    wprintf(L"Command to Run = %ls \n" , cmd.c_str());
    char *cmdLocal = new char[cmd.GetMaxLocaleCharBytes()];
    cmd.ConvertToLocaleChars (cmdLocal);
    int status = system(cmdLocal);
    printf("status = %d \n" , status);
    delete cmdLocal;
    return status == 0 ? SUCCESS: ERROR ;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBim)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir() );
    ASSERT_EQ( SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);
    
    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str() )
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBIMandIModel)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addCompressFlag();
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBIMandIModelFromDxf)
    {
    WString fileName = L"tr135537.dxf";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addCompressFlag();
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, Description)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addDescription(L"TestDescription");
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())

    DgnDbPtr db = OpenExistingDgnDb(outFile);
    ASSERT_TRUE(db.IsValid());
    BentleyApi::Utf8String description;
#ifdef FILE_DESCRIPTION
    db->QueryProperty(description, DgnProjectProperty::Description());
#else   // root subject decscription
    SubjectCPtr rootSubject = db->Elements().GetRootSubject ();
    if (rootSubject.IsValid())
        description = rootSubject->GetDescription ();
#endif

    EXPECT_TRUE(description.CompareTo("TestDescription") == 0) << "Description does not match";
    }

