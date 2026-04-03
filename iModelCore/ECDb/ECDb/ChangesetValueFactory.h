/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValue.h"
#include <ECDb/ECChangesetReader.h>
#include <unordered_set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE



//=======================================================================================
//! Creates IECSqlValue objects for each property of a changeset row.
//! All values are read exclusively from the supplied ColumnValueMap.
//! Properties whose backing columns are absent from the map are silently skipped.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueFactory final {
private:
    //! A map from SQLite column name to its DbValue for a single changeset row at one stage.
    //! Only columns that are actually present (non-absent) in the changeset are included.
    using ColumnValueMap = std::unordered_map<Utf8String, DbValue>;
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
    // ColumnValueMap accessors
    // ------------------------------------------------------------------

    //! Returns true when @p colName is present in @p columnValues with a valid DbValue.
    static bool IsInMap(Utf8StringCR colName, ColumnValueMap const& columnValues);

    //! Reads @p colName from @p columnValues.  Asserts that the key is present.
    //! Use only when the caller has already verified IsInMap().
    static DbValue GetFromMap(Utf8StringCR colName, ColumnValueMap const& columnValues);

    //! Returns the double value stored in @p val, or 0.0 if @p val is null.
    static double CheckNullAndGetDoubleValueFromDbValue(DbValue const& val);

    //! Returns the uint64_t value stored in @p val, or 0 if @p val is null.
    static BeInt64Id CheckNullAndGetBeInt64IdValueFromDbValue(DbValue const& val);

    //! Validates that all PK columns of @p tbl are present/valid/non-null in @p columnValues,
    //! prepares a "SELECT [@p selectCol] FROM <table> WHERE [pk1]=? AND ..." statement,
    //! and binds all PK values.  Returns the statement ready to Step(), or nullptr on any failure.
    static CachedStatementPtr PreparePkStatement(ECDbCR conn, DbTable const& tbl,
                                                 Utf8StringCR selectColName,
                                                 ColumnValueMap const& columnValues);

    //! Fetches a single real column value from the live DB for the row identified by
    //! the table's primary-key column in @p columnValues.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchDoubleFromDb(double& outVal, ECDbCR conn, DbColumn const& col,
                                     ColumnValueMap const& columnValues);

    //! Fetches a single integer column value from the live DB for the row identified by
    //! the table's primary-key column in @p columnValues.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchBeInt64IdFromDb(BeInt64Id& outVal, ECDbCR conn, DbColumn const& col,
                                    ColumnValueMap const& columnValues);

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

    //! Single-pass dispatch: checks presence and fills fieldsOut/changedProps in one step.
    static DbResult CreateValueForProperty(ECDbCR conn, PropertyMap const&,
                                           ColumnValueMap const&, DbTable const& dbTable,
                                           std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                           std::vector<Utf8String>& changedProps);

    //! Skips when neither coordinate is in the changeset.
    //! Returns BE_SQLITE_ERROR when a coordinate cannot be fetched from the live DB.
    static DbResult CreatePoint2d(ECDbCR conn, PropertyMap const&,
                                  ColumnValueMap const&,
                                  std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                  std::vector<Utf8String>& changedProps);

    //! Skips when no coordinate is in the changeset.
    //! Returns BE_SQLITE_ERROR when a coordinate cannot be fetched from the live DB.
    static DbResult CreatePoint3d(ECDbCR conn, PropertyMap const&,
                                  ColumnValueMap const&,
                                  std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                  std::vector<Utf8String>& changedProps);

    //! Skips when the backing column is absent from the changeset.
    static DbResult CreatePrimitive(ECDbCR conn, PropertyMap const&,
                                    ColumnValueMap const&,
                                    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                    std::vector<Utf8String>& changedProps);

    //! Skips when the backing column is absent or virtual.
    //! Returns BE_SQLITE_ERROR on internal mapping failures.
    static DbResult CreateSystem(ECDbCR conn, PropertyMap const&,
                                 ColumnValueMap const&, DbTable const& dbTable,
                                 std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                 std::vector<Utf8String>& changedProps);

    //! Skips when neither physical component is in the changeset.
    //! Returns BE_SQLITE_ERROR when a component is partly present but the DB fetch fails.
    static DbResult CreateNav(ECDbCR conn, PropertyMap const&,
                              ColumnValueMap const&,
                              std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                              std::vector<Utf8String>& changedProps);

    //! Skips when the column is absent from the changeset.
    static DbResult CreateArray(ECDbCR conn, PropertyMap const&,
                                ColumnValueMap const&,
                                std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                std::vector<Utf8String>& changedProps);

    //! Skips when no member has changeset data.
    //! Returns BE_SQLITE_ERROR if any member value fails to be created.
    static DbResult CreateStruct(ECDbCR conn, PropertyMap const&,
                                 ColumnValueMap const&, DbTable const& dbTable,
                                 std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
                                 std::vector<Utf8String>& changedProps);

    //! Creates a fixed-value IECSqlValue from a statically known id.  Always succeeds.
    static DbResult CreateFixedId(ECDbCR conn, PropertyMap const&, BeInt64Id,
                                  std::unique_ptr<IECSqlValue>& out);

    // ------------------------------------------------------------------
    // High-level resolution helpers used by Create()
    // ------------------------------------------------------------------

    //! Tries to resolve classMap + classId from the column values map's class-id entry.
    //! Returns true and populates @p outClassMap / @p outClassId on success.
    static bool TryResolveClassMapFromChangeset(DbTable const& dbTable,
                                                ColumnValueMap const& columnValues,
                                                ECDbCR conn,
                                                const ClassMap*& outClassMap,
                                                ECClassId& outClassId);

    //! Tries to resolve classMap + classId via a live DB seek on the first PK column.
    //! Returns false immediately when the ECClassId column is virtual.
    //! Returns true and populates @p outClassMap / @p outClassId on success.
    static bool TryResolveClassMapFromDbSeek(DbTable const& dbTable,
                                             ColumnValueMap const& columnValues,
                                             ECDbCR conn,
                                             const ClassMap*& outClassMap,
                                             ECClassId& outClassId);

    //! Reads ECInstanceId from @p columnValues, validates it, creates the field.
    //! Returns BE_SQLITE_OK and populates outputs on success; BE_SQLITE_ERROR otherwise.
    static DbResult ResolveInstanceId(ClassMap const& classMap,
                                      ColumnValueMap const& columnValues,
                                      ECDbCR conn, DbTable const& tbl,
                                      ECInstanceId& outInstanceId,
                                      std::unique_ptr<IECSqlValue>& outInstanceIdField);

    //! Creates the ECClassId field as a fixed value from the already-resolved @p resolvedClassId.
    //! Returns BE_SQLITE_OK and populates @p out on success; BE_SQLITE_ERROR otherwise.
    static DbResult ResolveClassIdField(ClassMap const& classMap,
                                        ECClassId resolvedClassId,
                                        ECDbCR conn, DbTable const& tbl,
                                        std::unique_ptr<IECSqlValue>& out);

    //! Iterates all remaining properties (excluding ECInstanceId and ECClassId)
    //! and dispatches each to CreateValueForProperty.
    //! Returns BE_SQLITE_OK on success; BE_SQLITE_ERROR immediately on any failure.
    static DbResult BuildPropertyFields(ClassMap const& classMap,
                                        ColumnValueMap const& columnValues,
                                        ECDbCR conn, DbTable const& dbTable,
                                        std::vector<std::unique_ptr<IECSqlValue>>& fields,
                                        std::vector<Utf8String>& changedProps);

    //! Returns true when @p classId is BisCore::Element or any class derived from it.
    static bool isChildClassOfBisCore(ECClassId classId, ECDbCR conn);

    // ------------------------------------------------------------------
public:
    //! Builds the IECSqlValue fields for one changeset row.
    //! If @p changedProps is non-null it is filled with the access paths of all properties
    //! / sub-properties that have data in the changeset.
    //! Examples: "Name", "Pos2d", "Pos2d.X", "Details.Label", "Owner", "Owner.Id".
    static DbResult Create(ECDbCR conn, DbTable const& tbl,
                            ColumnValueMap const& columnValues,
                            std::vector<std::unique_ptr<IECSqlValue>>& fields,
                            ECChangesetReader::Mode mode,
                            std::vector<Utf8String>& changedProps);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
