/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/SchemaImportTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTestFixture.h"

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

    struct RestrictedSchemaImportECDb : ECDb
        {
        public:
            RestrictedSchemaImportECDb(bool requiresSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport) : ECDb()
                {
                ApplyECDbSettings(false, requiresSchemaImportToken, allowChangesetMergingIncompatibleECSchemaImport);
                }

            SchemaImportToken const* GetSchemaImportToken() const { return GetECDbSettings().GetSchemaImportToken(); }
            bool AllowChangesetMergingIncompatibleSchemaImport() const { return GetECDbSettings().AllowChangesetMergingIncompatibleSchemaImport(); }
        };


protected:
    void AssertSchemaImport(SchemaItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(std::vector<SchemaItem> const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(ECDbR, bool& asserted, SchemaItem const&, Utf8CP ecdbFileName) const;
    void AssertSchemaImport(bool& asserted, ECDbCR, SchemaItem const&) const;

    void AssertIndexExists(ECDbCR, Utf8CP indexName, bool expectedToExist);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause = nullptr);
    void AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName = nullptr);
    void AssertForeignKeyDdl(ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyDdl);
    void AssertColumnCount(ECDbCR ecdb, std::vector<std::pair<Utf8String, int>> const& testItems, Utf8CP scenario = nullptr);
    void AssertColumnNames(ECDbCR ecdb, Utf8CP tableName, std::vector<Utf8String> const& columnNames, Utf8CP scenario = nullptr);

    //!logs the issues if there are any
    static bool HasDataCorruptingMappingIssues(ECDbCR);

    static std::vector<IndexInfo> RetrieveIndicesForTable(ECDbCR, Utf8CP tableName);

public:
    SchemaImportTestFixture() : ECDbTestFixture() {}
    virtual ~SchemaImportTestFixture() {}
    };

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  10/15
//=======================================================================================    
struct DbMappingTestFixture : SchemaImportTestFixture
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
            ForeignKeyRelationshipInTargetTable = 10,
            ForeignKeyRelationshipInSourceTable = 11
            };

        enum class JoinedTableInfo
            {
            None = 0,
            JoinedTable = 1,
            ParentOfJoinedTable = 2
            };

        struct TablePerHierarchyInfo
            {
            enum class ShareColumnsMode
                {
                No = 0,
                Yes = 1,
                ApplyToSubclassesOnly = 2
                };

            ShareColumnsMode m_sharedColumnsMode;
            int m_maxSharedColumnsBeforeOverflow = -1;
            JoinedTableInfo m_joinedTableInfo;

            TablePerHierarchyInfo() : m_sharedColumnsMode(ShareColumnsMode::No), m_joinedTableInfo(JoinedTableInfo::None) {}
            TablePerHierarchyInfo(ShareColumnsMode sharedColumnsMode, int maxSharedColumnsBeforeOverflow, JoinedTableInfo jti) : m_sharedColumnsMode(sharedColumnsMode), m_maxSharedColumnsBeforeOverflow(maxSharedColumnsBeforeOverflow), m_joinedTableInfo(jti) {}
            explicit TablePerHierarchyInfo(JoinedTableInfo jti) : m_sharedColumnsMode(ShareColumnsMode::No), m_joinedTableInfo(jti) {}

            bool IsUnset() const { return m_sharedColumnsMode == ShareColumnsMode::No && m_joinedTableInfo == JoinedTableInfo::None; }
            bool operator==(TablePerHierarchyInfo const& rhs) const { return m_sharedColumnsMode == rhs.m_sharedColumnsMode && m_maxSharedColumnsBeforeOverflow == rhs.m_maxSharedColumnsBeforeOverflow && m_joinedTableInfo == rhs.m_joinedTableInfo; }
            };

        Strategy m_strategy;
        TablePerHierarchyInfo m_tphInfo;

        MapStrategyInfo() : m_strategy(Strategy::NotMapped) {}
        MapStrategyInfo(Strategy strat, JoinedTableInfo joinedTableInfo) : m_strategy(strat), m_tphInfo(joinedTableInfo) {}
        MapStrategyInfo(Strategy strat, TablePerHierarchyInfo const& tphInfo) : m_strategy(strat), m_tphInfo(tphInfo) {}
        };

    struct PropertyAccessString final
        {
        Utf8String m_schemaNameOrAlias;
        Utf8String m_className;
        Utf8String m_propAccessString;

        PropertyAccessString(Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propAccessString) : m_schemaNameOrAlias(schemaNameOrAlias), m_className(className), m_propAccessString(propAccessString) {}
        Utf8String ToString() const { Utf8String str; str.Sprintf("%s:%s.%s", m_schemaNameOrAlias.c_str(), m_className.c_str(), m_propAccessString.c_str()); return str; }
        };

    struct ColumnInfo final
        {
        Utf8String m_tableName;
        Utf8String m_columnName;
        bool m_isVirtual = false;

        ColumnInfo(Utf8CP tableName, Utf8CP columnName) : ColumnInfo(tableName, columnName, false) {}
        ColumnInfo(Utf8CP tableName, Utf8CP columnName, bool isVirtual) : m_tableName(tableName), m_columnName(columnName), m_isVirtual(isVirtual) {}

        bool operator==(ColumnInfo const& rhs) const { return m_tableName.EqualsIAscii(rhs.m_tableName) && m_columnName.EqualsIAscii(rhs.m_columnName) && m_isVirtual == rhs.m_isVirtual; }
        bool operator!=(ColumnInfo const& rhs) const { return !(*this == rhs); }

        Utf8String ToString() const { Utf8String str; str.Sprintf("%s:%s (IsVirtual: %s)", m_tableName.c_str(), m_columnName.c_str(), m_isVirtual ? "true" : "false"); return str; }
        };

    bool TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId) const;
    bool TryGetColumnInfo(std::vector<ColumnInfo>&, ECDbCR ecdb, PropertyAccessString const&) const;
    bool TryGetColumnInfo(std::map<Utf8String, ColumnInfo>& colInfosByAccessString, ECDbCR ecdb, PropertyAccessString const&) const;
    
    void AssertPropertyMapping(ECDbCR, PropertyAccessString const&, std::vector<ColumnInfo> const& expectedColumnInfos) const;
    void AssertPropertyMapping(ECDbCR, PropertyAccessString const&, std::map<Utf8String, ColumnInfo> const& expectedColumnInfosByAccessString) const;

    public:
    DbMappingTestFixture () : SchemaImportTestFixture() {}

    virtual ~DbMappingTestFixture() {}
    };
END_ECDBUNITTESTS_NAMESPACE