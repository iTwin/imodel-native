/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/SchemaImportTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTestFixture.h"
#include "TestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================    
struct SchemaImportTestFixture : public ECDbTestFixture
    {
public:
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
    void AssertIndexExists(ECDbCR, Utf8CP indexName, bool expectedToExist);
    void AssertIndex(ECDbCR, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause = nullptr);
    void AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName = nullptr);
    void AssertForeignKeyDdl(ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyDdl);
    void AssertColumnCount(ECDbCR ecdb, std::vector<std::pair<Utf8String, int>> const& testItems, Utf8CP scenario = nullptr);
    void AssertColumnNames(ECDbCR ecdb, Utf8CP tableName, std::vector<Utf8String> const& columnNames, Utf8CP scenario = nullptr);

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

        static bool TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId);

    public:
        DbMappingTestFixture() : SchemaImportTestFixture() {}
        virtual ~DbMappingTestFixture() {}

        static ColumnInfo::List GetColumnInfos(ECDbCR ecdb, PropertyAccessString const&);
    };


#define ASSERT_PROPERTYMAPPING(ecdb, propAccessString, expectedColumnInfo) ASSERT_EQ(ColumnInfo::List({expectedColumnInfo}), DbMappingTestFixture::GetColumnInfos(ecdb, propAccessString))
#define ASSERT_PROPERTYMAPPING_MULTICOL(ecdb, propAccessString, expectedColumnInfos) ASSERT_EQ(expectedColumnInfos, DbMappingTestFixture::GetColumnInfos(ecdb, propAccessString))

END_ECDBUNITTESTS_NAMESPACE