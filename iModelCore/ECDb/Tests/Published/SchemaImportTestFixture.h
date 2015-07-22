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
    struct TestItem
        {
        Utf8String m_schemaXml;
        bool m_expectedToSucceed;
        Utf8String m_assertMessage;

        TestItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXml(schemaXml), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        };

public:
    SchemaImportTestFixture() { ECDbTestProject::Initialize(); }
    virtual ~SchemaImportTestFixture() {}

    void AssertSchemaImport(TestItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(ECDbR, bool& asserted, TestItem const&, Utf8CP ecdbFileName) const;
    };

END_ECDBUNITTESTS_NAMESPACE