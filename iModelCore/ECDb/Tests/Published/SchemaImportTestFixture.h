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
    struct IndexInfo
        {
        Utf8String m_name;
        Utf8String m_tableName;
        Utf8String m_ddl;

        IndexInfo(Utf8CP name, Utf8CP tableName, Utf8CP ddl) : m_name(name), m_tableName(tableName), m_ddl(ddl) {}
        };

protected:
    void AssertSchemaImport(SchemaItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(std::vector<SchemaItem> const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(ECDbR, bool& asserted, SchemaItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(bool& asserted, ECDbCR, SchemaItem const&) const;

    void AssertIndexExists(ECDbCR, Utf8CP indexName, bool expectedToExist);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause = nullptr);
    void AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName = nullptr);

    static std::vector<IndexInfo> RetrieveIndicesForTable(ECDbCR, Utf8CP tableName);

public:
    SchemaImportTestFixture() : ECDbTestFixture() {}
    virtual ~SchemaImportTestFixture() {}
    };

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  10/15
//=======================================================================================    
struct ECDbMappingTestFixture : SchemaImportTestFixture
    {
protected:
    //This is a mirror of the internal MapStrategy used by ECDb and persisted in the DB.
    //The values can change, so in that case this struct needs to be updated accordingly.
    struct PersistedMapStrategy
        {
        enum class Strategy
            {
            NotMapped,
            OwnTable,
            SharedTable,
            ExistingTable,

            ForeignKeyRelationshipInTargetTable = 100,
            ForeignKeyRelationshipInSourceTable = 101
            };

        enum class Options
            {
            None = 0,
            SharedColumns = 1,
            ParentOfJoinedTable = 2,
            JoinedTable = 4
            };

        Strategy m_strategy;
        Options m_options;
        bool m_appliesToSubclasses;

        PersistedMapStrategy() : m_strategy(Strategy::NotMapped), m_options(Options::None), m_appliesToSubclasses(false) {}
        PersistedMapStrategy(Strategy strategy, Options options, bool appliesToSubclasses) : m_strategy(strategy), m_options(options), m_appliesToSubclasses(appliesToSubclasses) {}
        };

    bool TryGetPersistedMapStrategy(PersistedMapStrategy& strategy, ECDbCR ecdb, ECN::ECClassId classId) const;

public:
    ECDbMappingTestFixture () : SchemaImportTestFixture() {}

    virtual ~ECDbMappingTestFixture() {}
    };
END_ECDBUNITTESTS_NAMESPACE