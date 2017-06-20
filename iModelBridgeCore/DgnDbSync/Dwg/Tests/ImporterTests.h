/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/ImporterTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// 
#define DWGTOOLKIT_RealDwg 1

#include "../DwgImportInternal.h"
#include <Bentley/BeTest.h>
#include "Tests.h"


#define ASSERT_PRESENT(fileName)        ASSERT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define ASSERT_NOT_PRESENT(fileName)    ASSERT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;
#define EXPECT_PRESENT(fileName)        EXPECT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define EXPECT_NOT_PRESENT(fileName)    EXPECT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;

#define ASSERT_SUCCESS(value)           ASSERT_EQ(SUCCESS,value)
#define ASSERT_NOT_SUCCESS(value)       ASSERT_NE(SUCCESS,value)
#define EXPECT_SUCCESS(value)           EXPECT_EQ(SUCCESS,value)
#define EXPECT_NOT_SUCCESS(value)       EXPECT_NE(SUCCESS,value)

#define ASSERT_NULL(statement)          ASSERT_TRUE (NULL == statement)
#define ASSERT_NOT_NULL(statement)      ASSERT_TRUE (NULL != statement)
#define EXPECT_NULL(statement)          EXPECT_TRUE (NULL == statement)
#define EXPECT_NOT_NULL(statement)      EXPECT_TRUE (NULL != statement)


/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      07/14
+===============+===============+===============+===============+===============+======*/
struct ImporterTests : public ::testing::Test
{
    ImporterTestsHost m_host;
public:

    static BentleyApi::WString GetDataSourcePath();
    static BentleyApi::BeFileName GetInputFileName(BentleyApi::WCharCP filename);
    static BentleyApi::BeFileName GetOutputFileName(BentleyApi::WCharCP filename);
    static void MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename);
    static BentleyApi::BeFileName GetOutputDir();
    static BentleyApi::WString GetOutRoot();
    static BentleyApi::BeFileName GetIBimFileName(BentleyApi::BeFileName& inFile);

    static void DeleteExistingDgnDb(BentleyApi::BeFileNameCR);
    static DgnDbPtr OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode = DgnDb::OpenMode::ReadWrite);
};