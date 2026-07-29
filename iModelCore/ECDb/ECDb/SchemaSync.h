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
    friend struct SchemaSync; //!< SchemaSync::ReserveSchemaImport calls private helpers directly.
    //! DDL to create the id-reservation table in the sync-db.
    static constexpr Utf8CP RESERVATION_TABLE_DDL =
        "CREATE TABLE IF NOT EXISTS [schema_reservation_ids] "
        "([TableName] TEXT NOT NULL PRIMARY KEY, "
        "[LastReservedId] INTEGER NOT NULL DEFAULT 0, "
        "[KeyMap] BLOB)";

    //! DDL to create the column-assignment reservation table in the sync-db.
    //! ClassHighWater persists the per-declaring-class high-water ordinal map (schema:class → ordinal).
    static constexpr Utf8CP RESERVATION_COLUMNS_TABLE_DDL =
        "CREATE TABLE IF NOT EXISTS [schema_reservation_columns] "
        "([PhysicalTableName] TEXT NOT NULL PRIMARY KEY, "
        "[KeyMap] BLOB, "
        "[ClassHighWater] BLOB)";

    //! DDL to create the class-hierarchy reservation table in the sync-db (§3a.1b).
    static constexpr Utf8CP RESERVATION_CLASS_HIERARCHY_TABLE_DDL =
        "CREATE TABLE IF NOT EXISTS [schema_reservation_class_hierarchy] "
        "([ClassKey] TEXT NOT NULL PRIMARY KEY, "
        "[Ancestors] BLOB, "
        "[Descendants] BLOB)";

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

    //! Seed both the key→id map and lastReservedId counter from the local db. Runs once at Init.
    static BentleyStatus SeedReservationStoreFromLocalDb(ECDbCR localDb, SchemaReservationStore& store);
    static BentleyStatus LoadReservationStoreFromSyncDb(Db& syncDb, SchemaReservationStore& store);
    static BentleyStatus WriteReservationStoreToSyncDb(Db& syncDb, SchemaReservationStore const& store);
    static void WalkSchemaForReservation(ECN::ECSchemaCR schema, SchemaReservationStore& store,
                                         bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Populate the class-hierarchy store for every class in @p schema (and its reference closure).
    //! Must run AFTER WalkSchemaForReservation (same dependency order) so that ancestor classes from
    //! referenced schemas are already in the store before derived classes are processed.
    static void PopulateClassHierarchyStore(ECN::ECSchemaCR schema,
                                            SchemaReservationClassHierarchyStore& hierarchyStore,
                                            bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Load / write the class-hierarchy store from/to the sync-db.
    static BentleyStatus LoadClassHierarchyStoreFromSyncDb(Db& syncDb,
                                                           SchemaReservationClassHierarchyStore& store);
    static BentleyStatus WriteClassHierarchyStoreToSyncDb(Db& syncDb,
                                                          SchemaReservationClassHierarchyStore const& store);
    //! Seed the class-hierarchy store from the local db at Init time.
    //! Derives transitive ancestor/descendant sets from ec_ClassHasBaseClasses.
    static BentleyStatus SeedClassHierarchyStoreFromLocalDb(ECDbCR localDb,
                                                            SchemaReservationClassHierarchyStore& store);

    //! Seed the column-assignment store and high-water ordinals from the local db. Runs once at Init.
    static BentleyStatus SeedColumnStoreFromLocalDb(ECDbCR localDb, SchemaReservationStore& idStore,
                                                    SchemaReservationColumnStore& colStore);
    static BentleyStatus LoadColumnStoreFromSyncDb(Db& syncDb, SchemaReservationColumnStore& store);
    static BentleyStatus WriteColumnStoreToSyncDb(Db& syncDb, SchemaReservationColumnStore const& store);
    //! Allocate shared-column ordinals for new properties in @p schema. Reuses slots
    //! when no occupant shares a root-to-leaf path with the property owner, using the persisted
    //! @p hierarchyStore as the single source of truth (replaces ECClass::Is(), §3a.1b).
    static void WalkSchemaForColumnReservation(ECN::ECSchemaCR schema,
                                               SchemaReservationStore& idStore,
                                               SchemaReservationColumnStore& colStore,
                                               SchemaReservationClassHierarchyStore const& hierarchyStore,
                                               bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Populate @p index with classKey → ECClass for @p schema and its full reference closure.
    static void CollectClassIndex(ECN::ECSchemaCR schema,
                                  bmap<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii>& index,
                                  bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Reserve ids for the mapping tables ec_Table, ec_PropertyPath, and ec_PropertyMap for
    //! every mapped entity/mixin class in @p schema. Primary-vs-overflow placement for
    //! shared columns is READ FROM @p colStore (the single source of truth produced by the
    //! column-reservation walk) rather than recomputed, so reserve and consume agree.
    //! Must run AFTER WalkSchemaForColumnReservation. Kept structurally separate from it;
    //! shared derivations live in the private helpers below.
    static void WalkSchemaForMappingReservation(ECN::ECSchemaCR schema,
                                                SchemaReservationStore& idStore,
                                                SchemaReservationColumnStore const& colStore,
                                                bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Reserve ec_Table / ec_Column / ec_PropertyPath / ec_PropertyMap ids for every
    //! link-table relationship class in @p schema (Gap D). Runs after the entity walks.
    static void WalkSchemaForRelationshipReservation(ECN::ECSchemaCR schema,
                                                     SchemaReservationStore& idStore,
                                                     bset<Utf8String, CompareIUtf8Ascii>& visited);

    //! Reserve ec_Index / ec_IndexColumn ids by enumerating every index the import will create
    //! (ECClassId auto-indexes, nav-FK indexes, link-table indexes, user DbIndex CA indexes).
    //! Runs after all table/column reservations so referenced ids already exist (Gap F).
    static void WalkSchemaForIndexReservation(ECN::ECSchemaCR schema,
                                              SchemaReservationStore& idStore,
                                              bset<Utf8String, CompareIUtf8Ascii>& visited);

private:
    //! Return true if @p ecClass participates in table mapping (skips relationship, custom-attribute,
    //! and struct classes, and classes whose ClassMap opts out via NotMapped/ExistingTable).
    static bool IsClassMappedForReservation(ECN::ECClassCR ecClass);
    //! Derive the primary physical table name for @p ecClass (TPH root's table, or the class's own).
    //! Returns SUCCESS/ERROR; on success @p tableName holds the derived name.
    static BentleyStatus DerivePrimaryTableName(ECN::ECClassCR ecClass, Utf8StringR tableName);
    //! Resolve the physical table a leaf column lands in. If @p leafColumnKey is present in the
    //! column store, returns the owning physical-table name (primary or overflow); otherwise falls
    //! back to @p primaryTableName (non-shared / deterministic placement).
    static Utf8String ResolveLeafColumnTableName(SchemaReservationColumnStore const& colStore,
                                                 Utf8StringCR leafColumnKey,
                                                 Utf8StringCR primaryTableName);
    //! Reserve ec_PropertyPath (keyed by declaring class) and ec_PropertyMap (keyed by concrete
    //! class + placement) for a single leaf access string of @p mappedClass.
    static void ReserveLeafPropertyReservation(SchemaReservationStore& idStore,
                                               SchemaReservationColumnStore const& colStore,
                                               ECN::ECClassCR mappedClass,
                                               ECN::ECClassCR declaringClass,
                                               ECN::ECPropertyCR rootProperty,
                                               Utf8StringCR accessString,
                                               Utf8StringCR primaryTableName);

    static BentleyStatus ReadTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore& store);
    static BentleyStatus WriteTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore const& store);
    static BentleyStatus SeedLastReservedIdsFromLocalDb(ECDbCR localDb, SchemaReservationStore& store);

    static uint64_t LookupSchemaReferenceId(ECDbCR localDb, Utf8StringCR schemaName, Utf8StringCR refSchemaName);
    static uint64_t LookupClassHasBaseClassesId(ECDbCR localDb, ECN::ECClassCR ecClass, ECN::ECClassCR baseClass);
    static uint64_t LookupFormatCompositeUnitId(ECDbCR localDb, ECN::ECFormatCR fmt, int ordinal);
    static uint64_t LookupRelConstraintId(ECDbCR localDb, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd end);
    static uint64_t LookupRelConstraintClassId(ECDbCR localDb, ECN::ECRelationshipClassCR relClass,
                                               ECN::ECRelationshipEnd end, ECN::ECClassCR constraintClass);
    static uint64_t LookupCustomAttributeId(ECDbCR localDb, uint64_t containerId, int containerType, ECN::ECClassCR caClass);
    static BentleyStatus SeedSchemaFromLocalDb(ECDbCR localDb, ECN::ECSchemaCR schema,
                                               SchemaReservationStore& store,
                                               bset<Utf8String, CompareIUtf8Ascii>& visited);

    static BentleyStatus ReadColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore& store);
    static BentleyStatus WriteColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore const& store);
    static BentleyStatus SeedColumnKeyMapsFromLocalDb(ECDbCR localDb, SchemaReservationStore& idStore,
                                                      SchemaReservationColumnStore& colStore);
    //! Seed the key→id maps for the mapping tables (ec_Table, ec_PropertyPath, ec_PropertyMap,
    //! ec_Index, ec_IndexColumn) and for column kinds not covered by SeedColumnKeyMapsFromLocalDb
    //! (system columns ECInstanceId/ECClassId, nav-FK columns, link-table system columns).
    //! Called from SeedReservationStoreFromLocalDb after the schema-graph walk.
    static BentleyStatus SeedMappingTableKeyMapsFromLocalDb(ECDbCR localDb, SchemaReservationStore& idStore);

    //! Return true if @p slot has no occupant that is the same, an ancestor, or a descendant of @p classKey.
    //! Uses the persisted @p hierarchyStore so the test is correct even when occupants were imported by
    //! a different briefcase and are absent from the current in-memory schema graph (§3a.1b).
    static bool IsSlotReusableByClass(SchemaReservationColumnSlot const& slot,
                                      Utf8StringCR classKey,
                                      SchemaReservationClassHierarchyStore const& hierarchyStore);

    //! Return the first ancestor with ClassMap CA MapStrategy=TablePerHierarchy, or nullptr.
    static ECN::ECClassCP FindTphAncestor(ECN::ECClassCR ecClass);
    //! Return the class that is the root of a joined-table sub-hierarchy for @p ecClass, or nullptr
    //! if @p ecClass is not in a joined-table sub-hierarchy. A class is a joined-table root when
    //! one of its DIRECT bases has the JoinedTablePerDirectSubclass CA; subclasses of that root
    //! also return the root class (they share its table). Mirrors ClassMappingInfo joined-table logic.
    static ECN::ECClassCP FindJoinedTableRoot(ECN::ECClassCR ecClass);
    //! Return the ShareColumnsMode that @p ecClass propagates to its subclasses.
    //! Mirrors the schema-metadata part of TablePerHierarchyInfo::DetermineSharedColumnsInfo.
    static TablePerHierarchyInfo::ShareColumnsMode ComputePropagatedShareMode(
        ECN::ECClassCR ecClass, Nullable<uint32_t>& maxBeforeOverflow);
    //! Return true if @p ecClass itself uses shared-column strategy (ShareColumnsMode == Yes),
    //! delegating to ComputePropagatedShareMode for the inheritance traversal.
    static bool ClassUsesSharedColumns(ECN::ECClassCR ecClass, Nullable<uint32_t>& maxBeforeOverflow);
    //! Return true if @p prop has an explicit ColumnName CA (no reservation needed).
    static bool PropertyHasExplicitColumnName(ECN::ECPropertyCR prop);
    //! Return true if @p relClass maps as a link table (mirrors TryDetermineRelationshipMappingType
    //! but works from the in-memory schema graph without a live db query).
    static bool IsLinkTableRelationship(ECN::ECRelationshipClassCR relClass);
    //! Reserve ec_Column ids (id-only, no slot) for all non-shared property leaves of @p ecClass.
    //! Covers named/dedicated data columns (Gap A) and navigation FK leaves (Gap C).
    static void ReserveNonSharedColumnIds(SchemaReservationStore& idStore, ECN::ECClassCR ecClass);
    //! Reserve ec_Column ids for system columns (ECInstanceId, ECClassId) of a physical table.
    static void ReserveSystemColumnIds(SchemaReservationStore& idStore,
                                       Utf8StringCR tableSpace, Utf8StringCR tableName,
                                       bool hasClassIdColumn);
    //! For every "tableSpace:tableName" entry already in @p store.ecTable, reserve
    //! system column ids (ECInstanceId + ECClassId).  Must be called after entity tables
    //! are populated but before the relationship walk adds link-table entries.
    static void ReserveEntityTableSystemColumnIds(SchemaReservationStore& idStore);
    //! Reserve a single index and its columns. Key: physicalTableName:indexName[:ordinal].
    static void ReserveIndexAndColumns(SchemaReservationStore& idStore,
                                       Utf8StringCR physicalTableName, Utf8StringCR indexName,
                                       int columnCount);
};

END_BENTLEY_SQLITE_EC_NAMESPACE