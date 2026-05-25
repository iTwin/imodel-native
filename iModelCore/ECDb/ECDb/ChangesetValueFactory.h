/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValue.h"
#include <ECDb/ChangesetReader.h>
#include <unordered_set>
#include <functional>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Creates IECSqlValue objects for each property of a changeset row.
//! All values are read via a ColumnValueGetter callable, keyed by DbColumn.
//! Properties whose backing columns return an invalid DbValue are silently skipped.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueFactory final {
private:
    //! A callable that maps a DbColumn to its DbValue for the current changeset row/stage.
    //! Returns an invalid DbValue when the column has no value in the changeset.
    using ColumnValueGetter = std::function<DbValue(DbColumn const&)>;
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
    // ColumnValueGetter helpers
    // ------------------------------------------------------------------

    //! Returns the double value stored in @p val, or quiet_NaN if @p val is null.
    static double CheckNullAndGetDoubleValueFromDbValue(DbValue const& val);

    //! Returns the uint64_t value stored in @p val, or 0 if @p val is null.
    static BeInt64Id CheckNullAndGetBeInt64IdValueFromDbValue(DbValue const& val);

    static CachedStatementPtr PreparePkStatement(ECDbCR conn, DbTable const& tbl,
                                                 Utf8StringCR selectColName,
                                                 ColumnValueGetter const& getter);

    //! Resets and clears bindings on @p stmt if not null.  Call after each use of a statement prepared by PreparePkStatement() to ensure it is ready for next use.
    static void ClearStatement(CachedStatementPtr stmt);

    //! Fetches a single real column value from the live DB for the row identified by
    //! the table's primary-key column via @p getter.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchDoubleFromDb(double& outVal, ECDbCR conn, DbColumn const& col,
                                     ColumnValueGetter const& getter);

    //! Fetches a single integer column value from the live DB for the row identified by
    //! the table's primary-key column via @p getter.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchBeInt64IdFromDb(BeInt64Id& outVal, ECDbCR conn, DbColumn const& col,
                                        ColumnValueGetter const& getter);

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
    //
    // Convention: each Create* appends directly to @p fieldsOut and inserts to
    //   @p changedProps when the property has changeset data; otherwise it is a no-op.
    //   Any non-OK DbResult is a hard error; the caller must propagate immediately.
    // ------------------------------------------------------------------

    static BentleyStatus CreateValueForProperty(ECDbCR conn, PropertyMap const&,
                                                ColumnValueGetter const&, DbTable const& dbTable,
                                                std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                                std::vector<Utf8String>& changedProps);

    //! Skips when neither coordinate is in the changeset.
    //! Returns ERROR when a coordinate cannot be fetched from the live DB.
    static BentleyStatus CreatePoint2d(ECDbCR conn, PropertyMap const&,
                                       ColumnValueGetter const&, DbTable const&,
                                       std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                       std::vector<Utf8String>& changedProps);

    //! Skips when no coordinate is in the changeset.
    //! Returns ERROR when a coordinate cannot be fetched from the live DB.
    static BentleyStatus CreatePoint3d(ECDbCR conn, PropertyMap const&,
                                       ColumnValueGetter const&, DbTable const&,
                                       std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                       std::vector<Utf8String>& changedProps);

    //! Skips when the backing column is absent from the changeset.
    //! Returns SUCCESS or ERROR.
    static BentleyStatus CreatePrimitive(ECDbCR conn, PropertyMap const&,
                                         ColumnValueGetter const&, DbTable const&,
                                         std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                         std::vector<Utf8String>& changedProps);

    //! Skips when the backing column is absent or virtual.
    //! Returns ERROR on internal mapping failures.
    static BentleyStatus CreateSystem(ECDbCR conn, PropertyMap const&,
                                      ColumnValueGetter const&, DbTable const& dbTable,
                                      std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                      std::vector<Utf8String>& changedProps);

    //! Skips when neither physical component is in the changeset.
    //! Returns ERROR when a component is partly present but the DB fetch fails.
    static BentleyStatus CreateNav(ECDbCR conn, PropertyMap const&,
                                   ColumnValueGetter const&, DbTable const&,
                                   std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                   std::vector<Utf8String>& changedProps);

    //! Skips when the column is absent from the changeset.
    //! Returns SUCCESS or ERROR.
    static BentleyStatus CreateArray(ECDbCR conn, PropertyMap const&,
                                     ColumnValueGetter const&, DbTable const&,
                                     std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                     std::vector<Utf8String>& changedProps);

    //! Skips when no member has changeset data.
    //! Returns ERROR if any member value fails to be created.
    static BentleyStatus CreateStruct(ECDbCR conn, PropertyMap const&,
                                      ColumnValueGetter const&, DbTable const& dbTable,
                                      std::vector<std::unique_ptr<ChangesetValue>>& fieldsOut,
                                      std::vector<Utf8String>& changedProps);

    //! Creates a fixed-value ChangesetValue from a statically known id.  Always succeeds.
    static void CreateFixedId(ECDbCR conn, PropertyMap const&, BeInt64Id,
                              std::unique_ptr<ChangesetValue>& out);

    // ------------------------------------------------------------------
    // High-level resolution helpers used by Create()
    // ------------------------------------------------------------------

    //! Tries to resolve classMap + classId from the getter's class-id entry.
    //! Returns true and populates @p outClassId on success.
    static bool TryResolveClassIdFromChangeset(DbTable const& dbTable,
                                                ColumnValueGetter const& getter,
                                                ECDbCR conn,
                                                ECClassId& outClassId);

    //! Tries to resolve classMap + classId via a live DB seek on the first PK column.
    //! Returns false immediately when the ECClassId column is virtual.
    //! Returns true and populates @p outClassId on success.
    static bool TryResolveClassIdFromDbSeek(DbTable const& dbTable,
                                             ColumnValueGetter const& getter,
                                             ECDbCR conn,
                                             ECClassId& outClassId);

    //! Reads ECInstanceId via @p getter, validates it, creates the field.
    //! Returns SUCCESS and populates outputs on success; ERROR otherwise.
    static BentleyStatus ResolveInstanceId(ClassMap const& classMap,
                                           ColumnValueGetter const& getter,
                                           ECDbCR conn, DbTable const& tbl,
                                           ECInstanceId& outInstanceId,
                                           std::unique_ptr<ChangesetValue>& outInstanceIdField);

    //! Creates the ECClassId field as a fixed value from the already-resolved @p resolvedClassId.
    //! Returns SUCCESS and populates @p out on success; ERROR otherwise.
    static BentleyStatus ResolveClassIdField(ClassMap const& classMap,
                                             ECClassId resolvedClassId,
                                             ECDbCR conn, DbTable const& tbl,
                                             std::unique_ptr<ChangesetValue>& out);

    //! Iterates all remaining properties (excluding ECInstanceId and ECClassId)
    //! and dispatches each to CreateValueForProperty.
    //! Returns SUCCESS on success; ERROR immediately on any failure.
    static BentleyStatus BuildPropertyFields(ClassMap const& classMap,
                                             ColumnValueGetter const& getter,
                                             ECDbCR conn, DbTable const& dbTable,
                                             std::vector<std::unique_ptr<ChangesetValue>>& fields,
                                             std::vector<Utf8String>& changedProps);

    //! Returns true when @p classId is BisCore::Element or any class derived from it.
    static bool IsDerivedFromBisElement(ECClassId classId, ECDbCR conn);

    // ------------------------------------------------------------------
public:
    //! Resolves the ClassMap, ECClassId, and whether the class id came from the changeset
    //! for a single changeset row.  Tries the changeset first, then a live DB seek,
    //! then falls back to the table's root ClassMap.
    //! Returns SUCCESS and populates all three out params on success; ERROR otherwise.
    static BentleyStatus ResolveClassId(ECDbCR conn, DbTable const& tbl,
                                        ColumnValueGetter const& getter,
                                        ECClassId& resolvedClassIdOut,
                                        bool& classIdFromChangesetOut);

    //! Builds the IECSqlValue fields for one changeset row.
    //! If @p changedProps is non-null it is filled with the access paths of all properties
    //! / sub-properties that have data in the changeset.
    //! Returns SUCCESS on success; ERROR otherwise.
    //! Examples of entries written to @p changedProps: "Name", "Pos2d", "Pos2d.X", "Details.Label", "Owner", "Owner.Id".
    static BentleyStatus Create(ECDbCR conn, DbTable const& tbl,
                                ColumnValueGetter const& getter,
                                ECClassId resolvedClassId, bool classIdFromChangeset,
                                std::vector<std::unique_ptr<ChangesetValue>>& fields,
                                ChangesetReader::PropertyFilter propertyFilter,
                                std::vector<Utf8String>& changedProps);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
