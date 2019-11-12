/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    ASSERT_EQ(SUCCESS, SetupDomainBimFile("bulkcrud_domainschemas.ecdb", domainSchemaFolder, bisSchemaFolder));

    EXPECT_EQ(ExpectedColumn("bis_InformationReferenceElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("bis", "UrlLink", "Url")));
    EXPECT_EQ(ExpectedColumn("bis_InformationReferenceElement", "js1"), GetHelper().GetPropertyMapColumn(AccessString("bis", "EmbeddedFileLink", "Name")));

    /*   TestDataset testDataset;
    ASSERT_EQ(SUCCESS, testDataset.Populate(m_ecdb)) << domainSchemaFolder.GetNameUtf8().c_str();

    AssertInsert(testDataset);
    */
    }

END_ECDBUNITTESTS_NAMESPACE
