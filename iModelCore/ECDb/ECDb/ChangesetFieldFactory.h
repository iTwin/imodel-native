/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! A map from SQLite column name to its DbValue for a single changeset row at one stage.
//! Only columns that are actually present (non-absent) in the changeset are included.
using ColumnValueMap = std::map<Utf8String, DbValue>;

//=======================================================================================
//! Creates IECSqlValue objects for each property of a changeset row.
//! All values are read exclusively from the supplied ColumnValueMap.
//! Properties whose backing columns are absent from the map are silently skipped.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetFieldFactory final {
public:
    static DbResult Create(ECDbCR conn, DbTable const& tbl,
                            ColumnValueMap const& columnValues,
                            std::vector<std::unique_ptr<IECSqlValue>>& fields);

private:
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

    //! Returns true when at least one SQLite column backing @p propertyMap is
    //! present in @p columnValues.
    //! For compound properties (Point2d/3d, Nav, Struct) only one component
    //! column needs to be present.
    static bool PropertyHasChangesetData(PropertyMap const& propertyMap,
                                         ColumnValueMap const& columnValues);

    //! Reads @p colName from @p columnValues.  Asserts that the key is present.
    //! Use only when the caller has already verified IsInMap().
    static DbValue GetFromMap(Utf8StringCR colName, ColumnValueMap const& columnValues);

    //! Fetches a single real column value from the live DB for the row identified by
    //! the table's primary-key column in @p columnValues.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchDoubleFromDb(double& outVal, ECDbCR conn, DbColumn const& col,
                                     ColumnValueMap const& columnValues);

    //! Fetches a single integer column value from the live DB for the row identified by
    //! the table's primary-key column in @p columnValues.
    //! Returns true and sets @p outVal on success; false on any failure.
    static bool TryFetchInt64FromDb(int64_t& outVal, ECDbCR conn, DbColumn const& col,
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
    // ------------------------------------------------------------------

    //! Creates an IECSqlValue for a scalar primitive property.
    //! Delegates Point2d to CreatePoint2d() and Point3d to CreatePoint3d().
    static std::unique_ptr<IECSqlValue> CreatePrimitive(ECDbCR conn, PropertyMap const&,
                                    ColumnValueMap const&);

    //! Creates a full ChangesetPoint2dValue. Missing coordinates are fetched from the live DB.
    static std::unique_ptr<IECSqlValue> CreatePoint2d(ECDbCR conn, PropertyMap const&,
                                   ColumnValueMap const&);

    //! Creates a full ChangesetPoint3dValue. Missing coordinates are fetched from the live DB.
    static std::unique_ptr<IECSqlValue> CreatePoint3d(ECDbCR conn, PropertyMap const&,
                                   ColumnValueMap const&);

    //! Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
    static std::unique_ptr<IECSqlValue> CreateSystem(ECDbCR conn, PropertyMap const&,
                                 ColumnValueMap const&);

    //! Creates an IECSqlValue for a struct property.
    //! Only members whose backing column(s) appear in the map are included.
    static std::unique_ptr<IECSqlValue> CreateStruct(ECDbCR conn, PropertyMap const&,
                                  ColumnValueMap const&);

    //! Creates a full ChangesetNavValue for a navigation property.
    //! Both id and relClassId are resolved: from the changeset if present, otherwise
    //! fetched from the live DB. Returns nullptr if either component cannot be resolved.
    static std::unique_ptr<IECSqlValue> CreateNav(ECDbCR conn, PropertyMap const&,
                               ColumnValueMap const&);

    //! Creates an IECSqlValue for a primitive-array or struct-array property.
    static std::unique_ptr<IECSqlValue> CreateArray(ECDbCR conn, PropertyMap const&,
                                 ColumnValueMap const&);

    //! Creates a fixed-value IECSqlValue whose integer value is known statically or
    //! fetched from the live DB (e.g. virtual RelECClassId, or a DB-fetched id).
    //! @p id may be any BeInt64Id-derived type (ECClassId, ECInstanceId, …).
    static std::unique_ptr<IECSqlValue> CreateFixedId(ECDbCR conn, PropertyMap const&, BeInt64Id);

    //! Dispatches to the appropriate typed Create* without a changeset-data guard.
    //! Used internally by CreateStruct after per-member filtering.
    static std::unique_ptr<IECSqlValue> CreateValueInternal(ECDbCR conn, PropertyMap const&,
                                         ColumnValueMap const&);

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

    //! Tries to resolve classMap + classId by reading the first primary-key value from
    //! @p columnValues and firing a direct SQL query on the live DB to fetch the physical
    //! ECClassId column.  Used when the class-id column is absent from the column values
    //! (e.g. entity tables where ECClassId is not captured in the changeset).
    //! Returns false immediately when the ECClassId column is virtual (no physical row to read).
    //! Returns true and populates @p outClassMap / @p outClassId on success.
    static bool TryResolveClassMapFromDbSeek(DbTable const& dbTable,
                                             ColumnValueMap const& columnValues,
                                             ECDbCR conn,
                                             const ClassMap*& outClassMap,
                                             ECClassId& outClassId);

    //! Reads ECInstanceId from @p columnValues, validates it, and creates the
    //! ECInstanceId field.  Returns false and logs on any failure.
    static bool ResolveInstanceId(ClassMap const& classMap,
                                  ColumnValueMap const& columnValues,
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
                                    ColumnValueMap const& columnValues,
                                    ECDbCR conn,
                                    std::vector<std::unique_ptr<IECSqlValue>>& fields);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
