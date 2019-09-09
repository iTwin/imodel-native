/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ConverterCommandBuilder.h"
#include <Bentley/BeTest.h>
#include "ConverterTestsBaseFixture.h"

#define ASSERT_PRESENT(fileName)        ASSERT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define ASSERT_NOT_PRESENT(fileName)    ASSERT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;
#define EXPECT_PRESENT(fileName)        EXPECT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define EXPECT_NOT_PRESENT(fileName)    EXPECT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;

#define ASSERT_NOT_SUCCESS(value)       ASSERT_NE(SUCCESS,value)
#define EXPECT_NOT_SUCCESS(value)       EXPECT_NE(SUCCESS,value)

#define ASSERT_NULL(statement)          ASSERT_TRUE (NULL == statement)
#define ASSERT_NOT_NULL(statement)      ASSERT_TRUE (NULL != statement)
#define EXPECT_NULL(statement)          EXPECT_TRUE (NULL == statement)
#define EXPECT_NOT_NULL(statement)      EXPECT_TRUE (NULL != statement)

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      07/14
+===============+===============+===============+===============+===============+======*/
struct ConverterAppTestsBaseFixture : public ConverterCommandBuilder, public ConverterTestBaseFixture
    {
    public:
        BentleyApi::StatusInt RunCMD(BentleyApi::WString);

        DEFINE_T_SUPER(ConverterTestBaseFixture);
        void SetUp();
        void TearDown();

        BentleyApi::WString GetOutRoot();
        BentleyApi::BeFileName GetBimFileName(BentleyApi::BeFileName& inFile);
    };

