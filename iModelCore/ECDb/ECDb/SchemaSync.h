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
//! Helpers for the "upstream" schema sync flow.
//!
//! Pull and push are symmetric: the briefcase decides ids and physical layout, then mirrors every
//! ec_ table into the sync db, and every other briefcase mirrors the whole thing back. The upstream
//! flow inverts that. The
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
    //! Copies exactly the closure's rows from syncAlias into targetAlias, and removes rows inside the
    //! closure that the sync db no longer has.
    static DbResult CopyClosure(ECDbR conn, Utf8CP syncAlias, Utf8CP targetAlias);
    //! Drops the temp id-set tables. Safe to call when they do not exist.
    static DbResult DropClosure(ECDbR conn);
    //! Upserts one table, restricted to source rows matching whereClause. Mirrors
    //! SchemaSyncHelper::SyncData's insert half, minus the delete half.
    static DbResult UpsertFiltered(ECDbR conn, Utf8CP tableName, Utf8CP sourceAlias, Utf8CP targetAlias, Utf8CP whereClause);
    //! Deletes target rows matching scopeClause whose primary key is absent from the source.
    //! scopeClause is evaluated against the target, so it must not reach into the source.
    static DbResult DeleteMissing(ECDbR conn, Utf8CP tableName, Utf8CP sourceAlias, Utf8CP targetAlias, Utf8CP scopeClause);
    //! Makes the target's copy of each table equal the source's, writing only the rows that differ.
    //! Used by the upgrade path, where the briefcase is the authority and the sync db is brought in
    //! line with it, deletions included.
    //!
    //! Deliberately not SchemaSyncHelper::SyncData, which upserts. After an upgrade the same logical
    //! row can carry a different id, so an incoming row can collide with a surviving one on a unique
    //! index rather than on the primary key - and ON CONFLICT DO UPDATE then rewrites that surviving
    //! row instead of inserting, which silently loses rows. Equally deliberately not a wholesale
    //! empty-and-refill: the sync db's CloudSqlite blocks are 64 KiB, so rewriting rows that did not
    //! change would make every other client re-download the whole file.
    static DbResult MirrorTables(ECDbR conn, StringList const& tables, Utf8CP sourceAlias, Utf8CP targetAlias);
    //! Re-point a set of schemas at the sync db.
    //!
    //! The schemas arrive resolved against the briefcase - that is what the ordinary import path
    //! produces, and its loading and sanitizing is worth keeping. But the sync db can hold newer
    //! versions of the schemas they reference, and those are the versions that decide the mapping,
    //! so importing them as they stand would make SchemaWriter diff against the wrong thing.
    //!
    //! Each schema is copied through a read context backed by the sync db, which re-locates every
    //! reference rather than carrying the briefcase's over. Copies are made in dependency order and
    //! serve each other, so a schema referencing another schema in @p schemas picks up its copy.
    //! @param[out] reloaded the copies, in dependency order. Import these, not @p schemas.
    static BentleyStatus ReloadAgainstSyncDb(bvector<ECN::ECSchemaPtr>& reloaded, bvector<ECN::ECSchemaCP> const& schemas, ECDbR syncConn);
    //! Number of rows currently in one of the temp id-set tables. For diagnostics and tests.
    static int64_t CountClosureRows(ECDbR conn, Utf8CP tempTableName);
};

END_BENTLEY_SQLITE_EC_NAMESPACE