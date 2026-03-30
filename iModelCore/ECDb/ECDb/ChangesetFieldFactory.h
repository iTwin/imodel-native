/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "TableView.h"
#include "ChangesetValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//=======================================================================================
//! Creates IECSqlValue objects for each property of a changeset row.
//! For columns absent from the changeset (e.g. partial Point2d/3d updates), values are
//! read from the live database using the ECInstanceId primary key.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetFieldFactory final {
public:
    static DbResult Create(ECDbCR conn, DbTable const& tbl,
                            Changes::Change const& change,
                            Changes::Change::Stage const& stage,
                            std::vector<std::unique_ptr<IECSqlValue>>& fields);

private:
    using Stage = Changes::Change::Stage;

    // ------------------------------------------------------------------
    // Schema / mapping helpers
    // ------------------------------------------------------------------

    //! Returns the ClassMap for the table's exclusive root EC class.
    //! For overflow tables the root class is resolved from the parent table.
    static const ClassMap* GetRootClassMap(DbTable const& tbl, ECDbCR conn);

    //! Builds an ECSqlPropertyPath from the property map's path segments.
    static ECSqlPropertyPath GetPropertyPath(PropertyMap const& propertyMap);

    //! Returns DateTime::Info for DateTime primitive/array properties;
    //! returns a default Unspecified DateTime::Info for all other kinds.
    static DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);

    // ------------------------------------------------------------------
    // TableView helpers
    // ------------------------------------------------------------------

    //! Creates a fresh TableView for @p tbl (no caching).
    static std::unique_ptr<TableView> CreateTableView(ECDbCR conn, DbTable const& tbl);

    //! Returns @p cachedView when it covers @p propTable; otherwise creates a new
    //! TableView stored in @p ownerStorage and returns a pointer to it.
    static TableView const* GetTableView(DbTable const& propTable,
                                         TableView const& cachedView,
                                         std::unique_ptr<TableView>& ownerStorage,
                                         ECDbCR conn);

    // ------------------------------------------------------------------
    // Changeset value accessors
    // ------------------------------------------------------------------

    //! Returns true when colIdx is valid and the changeset carries an explicit
    //! (non-absent) value for it at the given stage.
    static bool IsInChangeset(int colIdx, Changes::Change const& change, Stage stage);

    //! Returns true when at least one SQLite column backing @p propertyMap is
    //! present in the changeset for @p stage.
    //! For compound properties (Point2d/3d, Nav, Struct) only one component
    //! column needs to be present.
    static bool PropertyHasChangesetData(PropertyMap const& propertyMap,
                                         TableView const& tbl,
                                         Changes::Change const& change, Stage stage);

    //! Reads colIdx from the changeset.  Asserts that the value is present.
    //! Use only when the caller has already verified IsInChangeset().
    static DbValue GetChangesetValue(int colIdx, Changes::Change const& change, Stage stage);

    //! Reads colIdx from the changeset when present, falling back to the live
    //! database via @p tableView otherwise.
    //! Use only for partial-update compound properties (Point2d/3d, Nav).
    static DbValue GetValueWithDbFallback(int colIdx, Changes::Change const& change, Stage stage,
                                          TableView const& tableView, ECInstanceId instanceId);

    // ------------------------------------------------------------------
    // ECSqlColumnInfo construction
    // ------------------------------------------------------------------

    //! Builds ECSqlColumnInfo for a primitive / system property.
    static ECSqlColumnInfo MakePrimitiveColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for a struct property.
    static ECSqlColumnInfo MakeStructColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for a navigation property.
    static ECSqlColumnInfo MakeNavColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for an array property.
    static ECSqlColumnInfo MakeArrayColumnInfo(PropertyMap const& propertyMap);

    // ------------------------------------------------------------------
    // Per-kind field constructors
    // ------------------------------------------------------------------

    //! Creates an IECSqlValue for a single-column primitive property.
    //! For Point2d/3d, any coordinate absent from the changeset is filled from
    //! the live DB so the full coordinate is always returned.
    static std::unique_ptr<IECSqlValue> CreatePrimitive(ECDbCR conn, PropertyMap const&,
                                    TableView const&, Changes::Change const&,
                                    Stage, ECInstanceId);

    //! Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
    static std::unique_ptr<IECSqlValue> CreateSystem(ECDbCR conn, PropertyMap const&,
                                 TableView const&, Changes::Change const&,
                                 Stage);

    //! Creates an IECSqlValue for a struct property.
    //! Only members whose backing column(s) appear in the changeset are included.
    static std::unique_ptr<IECSqlValue> CreateStruct(ECDbCR conn, PropertyMap const&,
                                  TableView const&, Changes::Change const&,
                                  Stage, ECInstanceId);

    //! Creates an IECSqlValue for a navigation property.
    //! Either sub-component absent from the changeset is filled from the live DB.
    static std::unique_ptr<IECSqlValue> CreateNav(ECDbCR conn, PropertyMap const&,
                               TableView const&, Changes::Change const&,
                               Stage, ECInstanceId);

    //! Creates an IECSqlValue for a primitive-array or struct-array property.
    static std::unique_ptr<IECSqlValue> CreateArray(ECDbCR conn, PropertyMap const&,
                                 TableView const&, Changes::Change const&,
                                 Stage);

    //! Creates a fixed-value IECSqlValue for a class-id property whose value is
    //! known statically at mapping time (e.g. virtual RelECClassId).
    static std::unique_ptr<IECSqlValue> CreateFixedClassId(ECDbCR conn, PropertyMap const&, ECN::ECClassId);

    //! Dispatches to the appropriate typed Create* without a changeset-data guard.
    //! Used internally by CreateStruct after per-member filtering.
    static std::unique_ptr<IECSqlValue> CreateValueInternal(ECDbCR conn, PropertyMap const&,
                                         TableView const&, Changes::Change const&,
                                         Stage, ECInstanceId);

    // ------------------------------------------------------------------
    // High-level resolution helpers used by Create()
    // ------------------------------------------------------------------

    //! Tries to resolve classMap + classId from the changeset's class-id column.
    //! Returns true and populates @p outClassMap / @p outClassId on success.
    static bool TryResolveClassMapFromChangeset(TableView const& tableView,
                                                Changes::Change const& change, Stage stage,
                                                ECDbCR conn,
                                                const ClassMap*& outClassMap,
                                                ECClassId& outClassId);

    //! Tries to resolve classMap + classId by reading the first primary-key value from
    //! the changeset, seeking the live database row via @p tableView, and reading the
    //! ECClassId column.  Used when the class-id column is absent from the changeset
    //! (e.g. entity tables where ECClassId is virtual and not stored in the changeset).
    //! Returns true and populates @p outClassMap / @p outClassId on success.
    static bool TryResolveClassMapFromDbSeek(TableView const& tableView,
                                             Changes::Change const& change, Stage stage,
                                             ECDbCR conn,
                                             const ClassMap*& outClassMap,
                                             ECClassId& outClassId);

    //! Reads ECInstanceId from the changeset, validates it, and creates the
    //! ECInstanceId field.  Returns false and logs on any failure.
    static bool ResolveInstanceId(ClassMap const& classMap,
                                  TableView const& tableView,
                                  Changes::Change const& change, Stage stage,
                                  ECDbCR conn, DbTable const& tbl,
                                  ECInstanceId& outInstanceId,
                                  std::unique_ptr<IECSqlValue>& outInstanceIdField);

    //! Creates the ECClassId field as a fixed value from the already-resolved @p resolvedClassId.
    static std::unique_ptr<IECSqlValue> ResolveClassIdField(ClassMap const& classMap,
                                        ECClassId resolvedClassId,
                                        ECDbCR conn, DbTable const& tbl);

    //! Iterates all remaining properties of @p classMap (excluding ECInstanceId and
    //! ECClassId) and appends each changed property's field to @p fields.
    static bool BuildPropertyFields(ClassMap const& classMap,
                                    TableView const& tableView,
                                    Changes::Change const& change, Stage stage,
                                    ECInstanceId instanceId,
                                    ECDbCR conn,
                                    std::vector<std::unique_ptr<IECSqlValue>>& fields);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
