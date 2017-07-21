/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "DbSchema.h"

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
            :m_name(name), m_type(type), m_pkordinal(pkordinal), m_isnotnull(isnotnull), m_defaultConstraint(defaultConstraint) {}

        DbColumn::Type GetType() const { return m_type; }
        Utf8StringCR GetName() const { return m_name; }
        int GetPrimaryKeyOrdinal() const { return m_pkordinal; }
        bool IsNotNull() const { return m_isnotnull; }
        Utf8StringCR GetDefaultConstraint() const { return m_defaultConstraint; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbSchemaPersistenceManager final : public NonCopyableClass
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
    DbSchemaPersistenceManager();
    ~DbSchemaPersistenceManager();

    static bool IsTableChanged(ECDbCR, DbTable const&);

    static BentleyStatus CreateTable(ECDbCR, DbTable const&);
    static BentleyStatus UpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus AlterTable(ECDbCR, DbTable const&, std::vector<DbColumn const*> const& columnsToAdd);

    static BentleyStatus CreateTriggers(ECDbCR, DbTable const&, bool failIfExists);
    static bool TriggerExistsInDb(ECDbCR, DbTrigger const&);

    static BentleyStatus AppendColumnDdl(Utf8StringR ddl, DbColumn const&);
    //!@param[in] singleFkColumn must not be nullptr if @p embedInColumnDdl is true
    static BentleyStatus AppendForeignKeyToColumnDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&, DbColumn const& singleFkColumn);
    static BentleyStatus AppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void DoAppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void AppendColumnNamesToDdl(Utf8StringR ddl, std::vector<DbColumn const*> const&);

    static BentleyStatus GenerateIndexWhereClause(Utf8StringR ddl, ECDbCR, DbIndex const&);

public:
    static BentleyStatus CreateIndex(ECDbCR, DbIndex const&, Utf8StringCR ddl);

    static BentleyStatus BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR, DbIndex const&);

    static CreateOrUpdateTableResult CreateOrUpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus RepopulateClassHierarchyCacheTable(ECDbCR);
    static BentleyStatus RepopulateClassHasTableCacheTable(ECDbCR);

    static BentleyStatus RunPragmaTableInfo(bvector<SqliteColumnInfo>& colInfos, ECDbCR, Utf8StringCR tableName);

    static bmap<Utf8String, DbTableId, CompareIUtf8Ascii> GetTableDefNamesAndIds(ECDbCR, Utf8CP whereClause = nullptr);
    static bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> GetColumnNamesAndIds(ECDbCR, DbTableId);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE