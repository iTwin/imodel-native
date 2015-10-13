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
struct SchemaImportTestFixture : public ECDbTestFixture
    {
public:
    //---------------------------------------------------------------------------------------
    // @bsiclass                                   Krischan.Eberle                  07/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct TestItem
        {
        std::vector<Utf8String> m_schemaXmlList;
        bool m_expectedToSucceed;
        Utf8String m_assertMessage;

        TestItem(std::vector<Utf8String> const& schemaXmlList, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXmlList(schemaXmlList), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        TestItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXmlList({schemaXml}), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        TestItem(Utf8CP schemaXml, bool expectedToSucceeed) : m_schemaXmlList({Utf8String(schemaXml)}), m_expectedToSucceed(expectedToSucceeed) {}
        };

    struct IndexInfo
        {
        Utf8String m_name;
        Utf8String m_tableName;
        Utf8String m_ddl;

        IndexInfo(Utf8CP name, Utf8CP tableName, Utf8CP ddl) : m_name(name), m_tableName(tableName), m_ddl(ddl) {}
        };

protected:
    void AssertSchemaImport(TestItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(std::vector<TestItem> const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(ECDbR, bool& asserted, TestItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(bool& asserted, ECDbCR, TestItem const&) const;

    void AssertIndexExists(ECDbCR, Utf8CP indexName, bool expectedToExist);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause = nullptr);
    void AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName = nullptr);

    static std::vector<IndexInfo> RetrieveIndicesForTable(ECDbCR, Utf8CP tableName);

public:
    SchemaImportTestFixture() {}
    virtual ~SchemaImportTestFixture() {}
    };

END_ECDBUNITTESTS_NAMESPACE