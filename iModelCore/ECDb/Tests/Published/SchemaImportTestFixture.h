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
        TestItem(Utf8CP schemaXml, bool expectedToSucceeed) : m_schemaXml(schemaXml), m_expectedToSucceed(expectedToSucceeed) {}
        };

protected:
    void AssertSchemaImport(TestItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(ECDbR, bool& asserted, TestItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(bool& asserted, ECDbCR, TestItem const&) const;

    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause = nullptr);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter = false);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereExpWithoutClassIdFilter, std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter = false);

public:
    SchemaImportTestFixture() { ECDbTestProject::Initialize(); }
    virtual ~SchemaImportTestFixture() {}
    };

END_ECDBUNITTESTS_NAMESPACE