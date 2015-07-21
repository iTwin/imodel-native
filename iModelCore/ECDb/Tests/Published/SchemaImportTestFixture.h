/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SchemaImportTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================    
struct SchemaImportTestFixture : public ::testing::Test
    {
public:
    //---------------------------------------------------------------------------------------
    // @bsiclass                                   Krischan.Eberle                  07/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct SchemaImportTestItem
        {
        Utf8String m_schemaXml;
        bool m_expectedToSucceed;
        Utf8String m_assertMessage;

        SchemaImportTestItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXml(schemaXml), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        };

    SchemaImportTestFixture() {}
    virtual ~SchemaImportTestFixture() {}

    //! @param[out] createdECDb if not null, it will be the handle of the ECDb file created by this method
    void AssertSchemaImport(ECDb* createdECDb, SchemaImportTestItem const&, Utf8CP ecdbFileName) const;
    };

END_ECDBUNITTESTS_NAMESPACE