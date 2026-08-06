/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaManager.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct SchemaSyncHelper final {
    enum class ProfileKind {
        BE,
        EC,
        DGN,
    };
    using AliasMap = bmap<Utf8String, Utf8String, CompareIUtf8Ascii>;
    using StringList = bvector<Utf8String>;
    static constexpr auto ALIAS_SYNC_DB = "schema_sync_db";
	static constexpr auto ALIAS_MAIN_DB = "main";
    static constexpr auto TABLE_BE_PROP = "be_Prop";

    static int ForeignKeyCheck(DbCR conn, std::vector<std::string> const& tables, Utf8CP dbAlias);
    static DbResult GetMetaTables(DbR conn, StringList& tables, Utf8CP dbAlias);
    static DbResult DropDataTables(DbR conn);
    static DbResult DropMetaTables(DbR conn);
    static DbResult TryGetAttachDbs(AliasMap& aliasMap, ECDbR conn);
    static DbResult VerifyAlias(ECDbR conn);
    static DbResult GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames);
    static Utf8String Join(StringList const& list, Utf8String delimiter = ",", Utf8String prefix = "", Utf8String postfix = "");
    static Utf8String ToLower(Utf8String const& val);
    static DbResult GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames);
    static DbResult SyncData(ECDbR conn, StringList const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias);
    static DbResult SyncData(ECDbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias);
    static PropertySpec GetPropertySpec(ProfileKind kind);
    static ProfileVersion QueryProfileVersion(DbR db, ProfileKind kind);
    static ProfileVersion QueryProfileVersion(SchemaSync::SyncDbUri syncDbUri, ProfileKind kind);
    static DbResult SaveProfileVersion(SchemaSync::SyncDbUri syncDbUri, ProfileKind kind, ProfileVersion const& ver);
    static DbResult SaveProfileVersion(DbR conn, ProfileKind kind, ProfileVersion const& ver);
    static DbResult SyncProfileTablesSchema(DbR fromDb, DbR toDb);
    static DbResult SyncProfileTablesSchema(DbR thisDb, SchemaSync::SyncDbUri const& syncDbUri, bool thisDbToSyncDb);
    static DbResult UpdateProfileVersion(DbR conn, SchemaSync::SyncDbUri syncDbUri, bool thisDbToSyncDb);
};

//=======================================================================================
//! Helpers for the "upstream" schema sync flow (SchemaSync v2).
//!
//! v1 is symmetric: the briefcase decides ids and physical layout, then mirrors every ec_ table
//! into the sync db, and every other briefcase mirrors the whole thing back. v2 inverts that. The
//! import runs in the sync db, which decides ids and layout exactly once, and a briefcase then
//! adopts only the rows belonging to the schemas it asked for plus their transitive reference
//! closure. Everything else in the sync db - typically schemas another briefcase imported but has
//! not pushed yet - stays out of the briefcase.
//!
//! The closure is computed as id sets in temp tables, and every ec_ table is then copied with a
//! WHERE clause expressed against those sets. Doing it in two phases keeps each table's rule to a
//! single readable predicate instead of one deeply nested query.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSyncUpstreamHelper final {
    using StringList = bvector<Utf8String>;

    //! Temp tables holding the id sets that define the closure. Names are prefixed to avoid
    //! colliding with anything a caller may have put in the temp schema.
    static constexpr auto TEMP_SCHEMA_IDS = "temp.upstream_schema_ids";
    static constexpr auto TEMP_CLASS_IDS = "temp.upstream_class_ids";
    static constexpr auto TEMP_TABLE_IDS = "temp.upstream_table_ids";
    static constexpr auto TEMP_COLUMN_IDS = "temp.upstream_column_ids";
    //! Scratch input for the recursive expansions. A recursive CTE must not read from the same table
    //! the surrounding INSERT is writing to, so each expansion reads this and writes its own set.
    static constexpr auto TEMP_SEED_IDS = "temp.upstream_seed_ids";

    //! Resolves schemaNames in the sync db and expands them over ec_SchemaReference, then derives
    //! the class, physical-table and column id sets that follow from them.
    //! @return BE_SQLITE_NOTFOUND if any requested schema does not exist in the sync db.
    static DbResult BuildClosure(ECDbR conn, Utf8CP syncAlias, StringList const& schemaNames);
    //! Copies exactly the closure's rows from syncAlias into targetAlias. Additive: rows that exist
    //! locally but not in the sync db are left alone (see the note on deletes in the .cpp).
    static DbResult CopyClosure(ECDbR conn, Utf8CP syncAlias, Utf8CP targetAlias);
    //! Drops the temp id-set tables. Safe to call when they do not exist.
    static DbResult DropClosure(ECDbR conn);
    //! Upserts one table, restricted to source rows matching whereClause. Mirrors
    //! SchemaSyncHelper::SyncData's insert half, minus the delete half.
    static DbResult UpsertFiltered(ECDbR conn, Utf8CP tableName, Utf8CP sourceAlias, Utf8CP targetAlias, Utf8CP whereClause);
    //! Number of rows currently in one of the temp id-set tables. For diagnostics and tests.
    static int64_t CountClosureRows(ECDbR conn, Utf8CP tempTableName);
};

END_BENTLEY_SQLITE_EC_NAMESPACE