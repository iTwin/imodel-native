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
    if (!domainSchemaFolder.DoesPathExist())
        return; 

    SetupDomainBimFile("bulkcrud_domainschemas.ecdb", domainSchemaFolder, bisSchemaFolder);
    if (m_failed)
        return;

    TestDataset testDataset;
    ASSERT_EQ(SUCCESS, testDataset.Populate(GetECDb())) << domainSchemaFolder.GetNameUtf8().c_str();

    AssertInsert(testDataset);
    }

END_ECDBUNITTESTS_NAMESPACE
