/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SchemaImportTestFixture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    void AssertColumnCount(ECDbCR ecdb, std::vector<std::pair<Utf8String, int>> const& testItems, Utf8CP scenario);

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
    struct MapStrategyInfo
        {
        enum class Strategy
            {
            NotMapped,
            OwnTable,
            TablePerHierarchy,
            ExistingTable,
            SharedTable,
            ForeignKeyRelationshipInSourceTable = 100,
            ForeignKeyRelationshipInTargetTable = 101
            };

        enum class JoinedTableInfo
            {
            None = 0,
            JoinedTable = 1,
            ParentOfJoinedTable = 2
            };

        struct TablePerHierarchyInfo
            {
            bool m_useSharedColumns;
            int m_sharedColumnCount;
            Utf8String m_excessColumnName;
            JoinedTableInfo m_joinedTableInfo;

            TablePerHierarchyInfo() : m_useSharedColumns(false), m_sharedColumnCount(-1), m_joinedTableInfo(JoinedTableInfo::None) {}
            TablePerHierarchyInfo(bool useSharedColumns, int sharedColumnCount, JoinedTableInfo jti) : m_useSharedColumns(useSharedColumns), m_sharedColumnCount(sharedColumnCount), m_joinedTableInfo(jti) {}
            TablePerHierarchyInfo(bool useSharedColumns, int sharedColumnCount, Utf8CP excessColumnName, JoinedTableInfo jti) : m_useSharedColumns(useSharedColumns), m_sharedColumnCount(sharedColumnCount), m_excessColumnName(excessColumnName), m_joinedTableInfo(jti) {}
            explicit TablePerHierarchyInfo(JoinedTableInfo jti) : m_useSharedColumns(false), m_sharedColumnCount(-1), m_joinedTableInfo(jti) {}

            bool IsUnset() const { return !m_useSharedColumns && m_joinedTableInfo == JoinedTableInfo::None; }
            bool operator==(TablePerHierarchyInfo const& rhs) const { return m_useSharedColumns == rhs.m_useSharedColumns && m_sharedColumnCount == rhs.m_sharedColumnCount && m_joinedTableInfo == rhs.m_joinedTableInfo && m_excessColumnName.Equals(rhs.m_excessColumnName); }
            };

        Strategy m_strategy;
        TablePerHierarchyInfo m_tphInfo;

        MapStrategyInfo() : m_strategy(Strategy::NotMapped) {}
        MapStrategyInfo(Strategy strat, JoinedTableInfo joinedTableInfo) : m_strategy(strat), m_tphInfo(joinedTableInfo) {}
        MapStrategyInfo(Strategy strat, TablePerHierarchyInfo const& tphInfo) : m_strategy(strat), m_tphInfo(tphInfo) {}
        };

    bool TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId) const;

public:
    ECDbMappingTestFixture () : SchemaImportTestFixture() {}

    virtual ~ECDbMappingTestFixture() {}
    };
END_ECDBUNITTESTS_NAMESPACE