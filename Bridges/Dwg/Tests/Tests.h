/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnCoreApi.h>
#include <Bentley/BeTest.h>
// use shared DWG editor from DwgImporter unit tests
#include "..\DwgConverter\Tests\DwgFileEditor.h"

#define ASSERT_PRESENT(fileName)        ASSERT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define ASSERT_NOT_PRESENT(fileName)    ASSERT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;
#define EXPECT_PRESENT(fileName)        EXPECT_TRUE ( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File is not present at location :"<<fileName;
#define EXPECT_NOT_PRESENT(fileName)    EXPECT_FALSE( BentleyApi::BeFileName::DoesPathExist (fileName))<<L"File should not be present at location :"<<fileName;

#define ASSERT_SUCCESS(value)           ASSERT_EQ(SUCCESS,value)
#define ASSERT_NOT_SUCCESS(value)       ASSERT_NE(SUCCESS,value)
#define EXPECT_SUCCESS(value)           EXPECT_EQ(SUCCESS,value)
#define EXPECT_NOT_SUCCESS(value)       EXPECT_NE(SUCCESS,value)

#define ASSERT_DWGDBSUCCESS(value)      ASSERT_EQ (DwgDbStatus::Success,value)
#define EXPECT_DWGDBSUCCESS(value)      EXPECT_EQ (DwgDbStatus::Success,value)

#define ASSERT_NULL(statement)          ASSERT_TRUE (NULL == statement)
#define ASSERT_NOT_NULL(statement)      ASSERT_TRUE (NULL != statement)
#define EXPECT_NULL(statement)          EXPECT_TRUE (NULL == statement)
#define EXPECT_NOT_NULL(statement)      EXPECT_TRUE (NULL != statement)
