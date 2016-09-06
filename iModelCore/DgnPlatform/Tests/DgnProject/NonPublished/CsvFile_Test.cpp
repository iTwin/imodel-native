/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/CsvFile_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/CsvFile.h>
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/

struct CsvFileTests : public testing::Test
{

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, FindSection)
    {
    BeFileStatus status = BeFileStatus::Success;
    BeFileName srcFilePath;
    BeTest::GetHost().GetDocumentsRoot(srcFilePath);
    srcFilePath.AppendToPath(L"DgnDb");
    srcFilePath.AppendToPath(L"TestFile.csv");
    CsvFilePtr csvfile = CsvFile::Open(status, srcFilePath);
    ASSERT_TRUE(csvfile != nullptr);
    WString test = L"003_PetroBras";
    uint64_t filePos = 0;
    StatusInt statusF=csvfile->FindSection(filePos, &test, L"003_PetroBras");
    ASSERT_TRUE(statusF==0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, GetColumnContentsFromSection)
    {
    BeFileStatus status = BeFileStatus::Success;
    BeFileName srcFilePath;
    BeTest::GetHost().GetDocumentsRoot(srcFilePath);
    srcFilePath.AppendToPath(L"DgnDb");
    srcFilePath.AppendToPath(L"TestFile.csv");
    CsvFilePtr csvfile = CsvFile::Open(status, srcFilePath);
    ASSERT_TRUE(csvfile != nullptr);
    WString test = L"003_PetroBras";
    uint64_t filePos = 0;
    csvfile->FindSection(filePos, &test, L"003_PetroBras");
    T_WStringVector columncontents;
    csvfile->GetColumnContentsFromSection(columncontents, L"PDS", L"PDS");
    ASSERT_TRUE(columncontents.size() != 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, GetSections)
    {
    BeFileStatus status = BeFileStatus::Success;
    BeFileName srcFilePath;
    BeTest::GetHost().GetDocumentsRoot(srcFilePath);
    srcFilePath.AppendToPath(L"DgnDb");
    srcFilePath.AppendToPath(L"TestFile.csv");
    CsvFilePtr csvfile = CsvFile::Open(status, srcFilePath);
    ASSERT_TRUE(csvfile != nullptr);
    CsvSectionInfoVector sectionInfo;
    StatusInt statusF=csvfile->GetSections(sectionInfo);
    ASSERT_TRUE(statusF==0);
    ASSERT_TRUE(sectionInfo.size()!=0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, Counttokens)
    {
    CsvFilePtr csvfile;
    WString stringIn = L"1,2,3";
    WChar separatorCharIn = ',';
    uint32_t count=csvfile->CountTokens(stringIn, separatorCharIn,false);
    ASSERT_TRUE(count=3);
    stringIn = L"'1','2','3',''";
    count = csvfile->CountTokens(stringIn, separatorCharIn, true);
    ASSERT_TRUE(count = 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, GetColumns)
    {
    BeFileStatus status = BeFileStatus::Success;
    BeFileName srcFilePath;
    BeTest::GetHost().GetDocumentsRoot(srcFilePath);
    srcFilePath.AppendToPath(L"DgnDb");
    srcFilePath.AppendToPath(L"TestFile.csv");
    CsvFilePtr csvfile = CsvFile::Open(status, srcFilePath);
    ASSERT_TRUE(csvfile != nullptr);
    T_WStringVector columnNames;
    csvfile->GetColumnNames(columnNames,0);
    ASSERT_TRUE(columnNames.size()== 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CsvFileTests, GetRowAndMaxColumnCount)
    {
    BeFileStatus status = BeFileStatus::Success;
    BeFileName srcFilePath;
    BeTest::GetHost().GetDocumentsRoot(srcFilePath);
    srcFilePath.AppendToPath(L"DgnDb");
    srcFilePath.AppendToPath(L"TestFile.csv");
    CsvFilePtr csvfile = CsvFile::Open(status, srcFilePath);
    ASSERT_TRUE(csvfile != nullptr);
    uint32_t rowCount=0; uint32_t colCount=0;
    csvfile->GetRowAndMaxColumnCount(rowCount,colCount,true);
    ASSERT_TRUE(rowCount == 3);
    ASSERT_TRUE(colCount == 3);
    }