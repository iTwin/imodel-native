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
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaReservationHelper final {
    //! DDL to create the id-reservation table in the sync-db.
    //! KeyMap is a FlexBuffer BLOB: a map from content-key (TEXT) to reserved id (UInt64).
    static constexpr Utf8CP RESERVATION_TABLE_DDL =
        "CREATE TABLE IF NOT EXISTS [schema_reservation_ids] "
        "([TableName] TEXT NOT NULL PRIMARY KEY, "
        "[LastReservedId] INTEGER NOT NULL DEFAULT 0, "
        "[KeyMap] BLOB)";

    //! DDL to create the column-assignment reservation table in the sync-db.
    //! KeyMap is a FlexBuffer BLOB: a map from property-content-key (TEXT) to a
    //! two-element vector [columnOrd (UInt64), columnId (UInt64)].
    static constexpr Utf8CP RESERVATION_COLUMNS_TABLE_DDL =
        "CREATE TABLE IF NOT EXISTS [schema_reservation_columns] "
        "([PhysicalTableName] TEXT NOT NULL PRIMARY KEY, "
        "[LastUsedColumnOrd] INTEGER NOT NULL DEFAULT 0, "
        "[KeyMap] BLOB)";

    // Table-name constants for all reserved EC metadata and mapping tables.
    static constexpr Utf8CP RES_TABLE_SCHEMA         = "ec_Schema";
    static constexpr Utf8CP RES_TABLE_SCHEMAREF      = "ec_SchemaReference";
    static constexpr Utf8CP RES_TABLE_CLASS          = "ec_Class";
    static constexpr Utf8CP RES_TABLE_CLASSBASES     = "ec_ClassHasBaseClasses";
    static constexpr Utf8CP RES_TABLE_PROPERTY       = "ec_Property";
    static constexpr Utf8CP RES_TABLE_ENUM           = "ec_Enumeration";
    static constexpr Utf8CP RES_TABLE_KOQ            = "ec_KindOfQuantity";
    static constexpr Utf8CP RES_TABLE_UNITSYSTEM     = "ec_UnitSystem";
    static constexpr Utf8CP RES_TABLE_PHENOMENON     = "ec_Phenomenon";
    static constexpr Utf8CP RES_TABLE_UNIT           = "ec_Unit";
    static constexpr Utf8CP RES_TABLE_FORMAT         = "ec_Format";
    static constexpr Utf8CP RES_TABLE_FORMATUNIT     = "ec_FormatCompositeUnit";
    static constexpr Utf8CP RES_TABLE_PROPCAT        = "ec_PropertyCategory";
    static constexpr Utf8CP RES_TABLE_RELCONSTRAINT  = "ec_RelationshipConstraint";
    static constexpr Utf8CP RES_TABLE_RELCONSTRCLASS = "ec_RelationshipConstraintClass";
    static constexpr Utf8CP RES_TABLE_CA             = "ec_CustomAttribute";
    static constexpr Utf8CP RES_TABLE_TABLE          = "ec_Table";
    static constexpr Utf8CP RES_TABLE_COLUMN         = "ec_Column";
    static constexpr Utf8CP RES_TABLE_PROPMAP        = "ec_PropertyMap";
    static constexpr Utf8CP RES_TABLE_PROPPATH       = "ec_PropertyPath";
    static constexpr Utf8CP RES_TABLE_INDEX          = "ec_Index";
    static constexpr Utf8CP RES_TABLE_INDEXCOL       = "ec_IndexColumn";

    static BentleyStatus ReadTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore& store);
    static BentleyStatus WriteTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore const& store);
    static void SeedLastReservedIdsFromLocalDb(ECDbCR localDb, SchemaReservationStore& store);
    static BentleyStatus LoadReservationStoreFromSyncDb(Db& syncDb, SchemaReservationStore& store);
    static BentleyStatus WriteReservationStoreToSyncDb(Db& syncDb, SchemaReservationStore const& store);
    static void WalkSchemaForReservation(ECN::ECSchemaCR schema, SchemaReservationStore& store,
                                         bset<Utf8String, CompareIUtf8Ascii>& visited);

    // Column-assignment reservation helpers (§3a).
    static Utf8String FindPrimaryTableForClass(ECDbCR localDb, ECN::ECClassCR ecClass);
    static BentleyStatus ReadColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore& store);
    static BentleyStatus WriteColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore const& store);
    static BentleyStatus SeedLastUsedColumnOrdsFromLocalDb(ECDbCR localDb, SchemaReservationColumnStore& store);
    static BentleyStatus LoadColumnStoreFromSyncDb(Db& syncDb, SchemaReservationColumnStore& store);
    static BentleyStatus WriteColumnStoreToSyncDb(Db& syncDb, SchemaReservationColumnStore const& store);
    //! Walk @p schema and allocate per-physical-table column ordinals for every new property
    //! that maps to a shared-column or overflow physical table. Queries @p localDb for
    //! existing class-map information to determine the physical table name.
    static void WalkSchemaForColumnReservation(ECN::ECSchemaCR schema, ECDbCR localDb,
                                               SchemaReservationStore& idStore,
                                               SchemaReservationColumnStore& colStore,
                                               bset<Utf8String, CompareIUtf8Ascii>& visited);
};

END_BENTLEY_SQLITE_EC_NAMESPACE