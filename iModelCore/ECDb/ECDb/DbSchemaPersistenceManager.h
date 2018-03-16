/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "DbSchema.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
//! Represents one row of the result set when running the SQLite pragma table_info
// @bsiclass                                                 Affan.Khan         06/2016
//======================================================================================
struct SqliteColumnInfo final
    {
    private:
        Utf8String m_name;
        DbColumn::Type m_type;
        int m_pkordinal;
        bool m_isnotnull;
        Utf8String m_defaultConstraint;

    public:
        SqliteColumnInfo(Utf8StringCR name, DbColumn::Type type, int pkordinal, bool isnotnull, Utf8StringCR defaultConstraint)
            :m_name(name), m_type(type), m_pkordinal(pkordinal), m_isnotnull(isnotnull), m_defaultConstraint(defaultConstraint)
            {}

        DbColumn::Type GetType() const { return m_type; }
        Utf8StringCR GetName() const { return m_name; }
        int GetPrimaryKeyOrdinal() const { return m_pkordinal; }
        bool IsNotNull() const { return m_isnotnull; }
        Utf8StringCR GetDefaultConstraint() const { return m_defaultConstraint; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbSchemaPersistenceManager final
    {
public:
    enum class CreateOrUpdateTableResult
        {
        Created = 1,
        Updated = 2,
        WasUpToDate = 3,
        Skipped = 4,
        Error = 5
        };

private:
    DbSchemaPersistenceManager() = delete;
    ~DbSchemaPersistenceManager() = delete;

    static bool IsTableChanged(ECDbCR, DbTable const&);

    static BentleyStatus CreateTable(ECDbCR, DbTable const&);
    static BentleyStatus UpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus AlterTable(ECDbCR, DbTable const&, std::vector<DbColumn const*> const& columnsToAdd);

    static BentleyStatus CreateTriggers(ECDbCR, DbTable const&, bool failIfExists);
    static bool TriggerExists(ECDbCR, DbTrigger const&);

    static BentleyStatus AppendColumnDdl(Utf8StringR ddl, DbColumn const&);
    //!@param[in] singleFkColumn must not be nullptr if @p embedInColumnDdl is true
    static BentleyStatus AppendForeignKeyToColumnDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&, DbColumn const& singleFkColumn);
    static BentleyStatus AppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void DoAppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void AppendColumnNamesToDdl(Utf8StringR ddl, std::vector<DbColumn const*> const&);

    static BentleyStatus GenerateIndexWhereClause(Utf8StringR ddl, ECDbCR, DbIndex const&);

public:

    static BentleyStatus RunPragmaTableInfo(std::vector<SqliteColumnInfo>& colInfos, ECDbCR, Utf8StringCR tableName, Utf8CP tableSpace = nullptr);

    static BentleyStatus CreateIndex(ECDbCR, DbIndex const&, Utf8StringCR ddl);

    static BentleyStatus BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR, DbIndex const&);

    static CreateOrUpdateTableResult CreateOrUpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus RepopulateClassHierarchyCacheTable(ECDbCR);
    static BentleyStatus RepopulateClassHasTableCacheTable(ECDbCR);

 
    //!Safe method to cast an integer value to the MapStrategy enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<MapStrategy> ToMapStrategy(int val)
        {
        if (val == Enum::ToInt(MapStrategy::NotMapped) || val == Enum::ToInt(MapStrategy::OwnTable) || val == Enum::ToInt(MapStrategy::TablePerHierarchy) ||
            val == Enum::ToInt(MapStrategy::ExistingTable) ||
            val == Enum::ToInt(MapStrategy::ForeignKeyRelationshipInTargetTable) || val == Enum::ToInt(MapStrategy::ForeignKeyRelationshipInSourceTable))
            return Enum::FromInt<MapStrategy>(val);

        return Nullable<MapStrategy>();
        };

    //!Safe method to cast an integer value to the ShareColumnsMode enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<TablePerHierarchyInfo::ShareColumnsMode> ToShareColumnsMode(int val)
        {
        if (val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly) || val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::No) || val == Enum::ToInt(TablePerHierarchyInfo::ShareColumnsMode::Yes))
            return Enum::FromInt<TablePerHierarchyInfo::ShareColumnsMode>(val);

        return Nullable<TablePerHierarchyInfo::ShareColumnsMode>();
        };

    //!Safe method to cast an integer value to the JoinedTableInfo enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<JoinedTableInfo> ToJoinedTableInfo(int val)
        {
        if (val == Enum::ToInt(JoinedTableInfo::None) || val == Enum::ToInt(JoinedTableInfo::JoinedTable) || val == Enum::ToInt(JoinedTableInfo::ParentOfJoinedTable))
            return Enum::FromInt<JoinedTableInfo>(val);

        return Nullable<JoinedTableInfo>();
        };

    //!Safe method to cast an integer value to the DbTable::Type enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<DbTable::Type> ToDbTableType(int val)
        {
        if (val == Enum::ToInt(DbTable::Type::Existing) || val == Enum::ToInt(DbTable::Type::Joined) || val == Enum::ToInt(DbTable::Type::Overflow) ||
            val == Enum::ToInt(DbTable::Type::Primary) || val == Enum::ToInt(DbTable::Type::Virtual))
            return Enum::FromInt<DbTable::Type>(val);

        return Nullable<DbTable::Type>();
        };

    //!Safe method to cast an integer value to the DbColumn::Kind enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<DbColumn::Kind> ToDbColumnKind(int val)
        {
        if (val == Enum::ToInt(DbColumn::Kind::Default) || val == Enum::ToInt(DbColumn::Kind::ECClassId) || val == Enum::ToInt(DbColumn::Kind::ECInstanceId) || val == Enum::ToInt(DbColumn::Kind::SharedData))
            return Enum::FromInt<DbColumn::Kind>(val);

        return Nullable<DbColumn::Kind>();
        };

    //!Safe method to cast an integer value to the DbColumn::Type enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<DbColumn::Type> ToDbColumnType(int val)
        {
        if (val == Enum::ToInt(DbColumn::Type::Any) || val == Enum::ToInt(DbColumn::Type::Blob) || val == Enum::ToInt(DbColumn::Type::Boolean) || val == Enum::ToInt(DbColumn::Type::Integer) ||
            val == Enum::ToInt(DbColumn::Type::Real) || val == Enum::ToInt(DbColumn::Type::Text) || val == Enum::ToInt(DbColumn::Type::TimeStamp))
            return Enum::FromInt<DbColumn::Type>(val);

        return Nullable<DbColumn::Type>();
        };

    //!Safe method to cast an integer value to the DbColumn::Constraints::Collation enum.
    //!It makes sure the integer is a valid value for the enum.
    static Nullable<DbColumn::Constraints::Collation> ToDbColumnCollation(int val)
        {
        if (val == Enum::ToInt(DbColumn::Constraints::Collation::Binary) || val == Enum::ToInt(DbColumn::Constraints::Collation::NoCase) ||
            val == Enum::ToInt(DbColumn::Constraints::Collation::RTrim) || val == Enum::ToInt(DbColumn::Constraints::Collation::Unset))
            return Enum::FromInt<DbColumn::Constraints::Collation>(val);

        return Nullable<DbColumn::Constraints::Collation>();
        };
    };

END_BENTLEY_SQLITE_EC_NAMESPACE