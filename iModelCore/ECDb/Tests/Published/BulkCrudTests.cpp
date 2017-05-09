/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BulkCrudTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BulkCrudTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BulkBisDomainCrudTestFixture, Test)
    {
    BeFileName bisSchemaFolder;
    BeFileName domainSchemaFolder = GetDomainSchemaFolder(bisSchemaFolder);
    ASSERT_EQ(SUCCESS, SetupDomainBimFile("bulkcrud_domainschemas.ecdb", domainSchemaFolder, bisSchemaFolder)) << domainSchemaFolder.GetNameUtf8().c_str();

    BeFileName testDataJsonFile;
    BeTest::GetHost().GetOutputRoot(testDataJsonFile);
    testDataJsonFile.AppendToPath(L"bulkcrud_domainschemas_data.json");
    ASSERT_EQ(SUCCESS, CreateTestData(GetECDb(), testDataJsonFile)) << domainSchemaFolder.GetNameUtf8().c_str();
    }

END_ECDBUNITTESTS_NAMESPACE
