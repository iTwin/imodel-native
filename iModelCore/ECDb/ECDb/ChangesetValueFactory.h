/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/PmrObjectAllocator.h>
#include <ECDb/ChangesetReader.h>
#include <unordered_set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//=======================================================================================
//! Creates IECSqlValue objects for each property of a changeset row; absent columns are silently skipped.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueFactory final {
private:
    //! Maps SQLite column name to its DbValue for a single changeset row; only non-absent columns are included.
    using ChangeValueMap = std::unordered_map<Utf8String, DbValue>;
    using Stage          = Changes::Change::Stage; //!< Shorthand for the Before/After stage enum.
    ECDbCR                          m_conn;         //!< ECDb connection used for schema lookups and live DB queries.
    ChangesetFilter const&          m_filter;       //!< Controls which properties are included in the output.
    ChangesetSqliteIterator const&  m_iterator;     //!< Provides raw column data from the changeset.
    PmrObjectAllocator<ChangesetValue>& m_alloc;    //!< Allocator for all ChangesetValue instances.
    ChangeValueMap         m_changeValueMap;          //!< Populated per-row by PopulateColumnValues(); maps column name to DbValue.

    //! Context bundle passed through all per-property factory methods.

    struct BuildCtx {
        DbTable const&                  dbTable;
        std::vector<ChangesetValue*>&   fields; // out param
        std::vector<Utf8String>&        changedProps; // out param
    };
    // ------------------------------------------------------------------
    // Schema / mapping helpers
    // ------------------------------------------------------------------

    //! Returns the ClassMap for the table's exclusive root EC class; resolves via parent table for overflow tables.
    const ClassMap* GetRootClassMap(DbTable const& tbl);

    //! Builds an ECSqlPropertyPath from the property map's path segments.
    ECSqlPropertyPath GetPropertyPath(PropertyMap const& propertyMap);

    //! Returns DateTime::Info for DateTime primitive/array properties; returns Unspecified for all other kinds.
    DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);

    // ------------------------------------------------------------------
    // ChangeValueMap accessors
    // ------------------------------------------------------------------

    //! Returns true when @p colName is present in m_changeValueMap with a valid DbValue.
    bool IsInMap(Utf8StringCR colName);

    //! Reads @p colName from m_changeValueMap; asserts the key is present — call only after IsInMap().
    DbValue GetFromMap(Utf8StringCR colName);

    //! Returns the double value stored in @p val, or quiet_NaN if @p val is null or invalid.
    double CheckNullAndGetDoubleValueFromDbValue(DbValue const& val);

    //! Returns the BeInt64Id value stored in @p val, or BeInt64Id(0) if @p val is null or invalid.
    BeInt64Id CheckNullAndGetBeInt64IdValueFromDbValue(DbValue const& val);

    //! Prepares and binds a PK-keyed SELECT statement for @p selectColName; returns nullptr on any failure.
    CachedStatementPtr PreparePkStatement(DbTable const& tbl, Utf8StringCR selectColName);

    //! Resets and clears bindings on @p stmt if not null.
    void ClearStatement(CachedStatementPtr stmt);

    //! Fetches a single real (double) column value from the live DB via PK lookup; returns false on failure.
    bool TryFetchDoubleFromDb(DbColumn const& col, double& outVal);

    //! Fetches a single integer (BeInt64Id) column value from the live DB via PK lookup; returns false on failure.
    bool TryFetchBeInt64IdFromDb(DbColumn const& col, BeInt64Id& outVal);

    // ------------------------------------------------------------------
    // ECSqlColumnInfo construction
    // ------------------------------------------------------------------

    //! Builds ECSqlColumnInfo for a primitive / system property.
    ECSqlColumnInfo MakePrimitiveColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for a struct property.
    ECSqlColumnInfo MakeStructColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for a navigation property.
    ECSqlColumnInfo MakeNavColumnInfo(PropertyMap const& propertyMap);

    //! Builds ECSqlColumnInfo for an array property.
    ECSqlColumnInfo MakeArrayColumnInfo(PropertyMap const& propertyMap);

    // ------------------------------------------------------------------
    // Per-kind field constructors — each Create* appends to ctx.fields/ctx.changedProps when data is present, or is a no-op.
    // ------------------------------------------------------------------

    //! Dispatches to the appropriate Create* method; appends to ctx.fields/ctx.changedProps. Returns SUCCESS or ERROR.
    BentleyStatus CreateValueForProperty(PropertyMap const&, BuildCtx& ctx);

    //! Skips when neither coordinate is in the changeset; returns ERROR when a missing coordinate cannot be fetched from the DB.
    BentleyStatus CreatePoint2d(PropertyMap const&, BuildCtx& ctx);

    //! Skips when no coordinate is in the changeset; returns ERROR when a missing coordinate cannot be fetched from the DB.
    BentleyStatus CreatePoint3d(PropertyMap const&, BuildCtx& ctx);

    //! Skips when the backing column is absent from the changeset. Returns SUCCESS or ERROR.
    BentleyStatus CreatePrimitive(PropertyMap const&, BuildCtx& ctx);

    //! Skips when no data property map is found or the column is absent from the changeset; returns ERROR on failure.
    BentleyStatus CreateSystem(PropertyMap const&, BuildCtx& ctx);

    //! Skips when neither physical component is in the changeset; returns ERROR when a missing component cannot be fetched from the DB.
    BentleyStatus CreateNav(PropertyMap const&, BuildCtx& ctx);

    //! Skips when the backing column is absent from the changeset. Returns SUCCESS or ERROR.
    BentleyStatus CreateArray(PropertyMap const&, BuildCtx& ctx);

    //! Skips when no member has changeset data; returns ERROR if any member value fails to be created.
    BentleyStatus CreateStruct(PropertyMap const&, BuildCtx& ctx);

    //! Creates a fixed-value ChangesetValue from a statically known id.  Always succeeds.
    void CreateFixedId(PropertyMap const&, BeInt64Id, ChangesetValue*& out);

    // ------------------------------------------------------------------
    // High-level resolution helpers used by Create()
    // ------------------------------------------------------------------

    //! Tries to resolve ECClassId from the changeset's class-id column; returns true and sets @p outClassId on success.
    bool TryResolveClassIdFromChangeset(DbTable const& dbTable,ECClassId& outClassId);

    //! Tries to resolve ECClassId via a live DB seek; returns false if the class-id column is virtual or the seek fails.
    bool TryResolveClassIdFromDbSeek(DbTable const& dbTable, ECClassId& outClassId);

    //! Resolves ECInstanceId from m_changeValueMap, creates the field, and populates @p outInstanceIdField; returns ERROR on failure.
    BentleyStatus ResolveInstanceId(ClassMap const& classMap, DbTable const& tbl,
                                           ChangesetValue*& outInstanceIdField);

    //! Creates the ECClassId field as a fixed value from @p resolvedClassId; returns ERROR if the property is not found.
    BentleyStatus ResolveClassIdField(ClassMap const& classMap,
                                             ECClassId resolvedClassId,
                                             DbTable const& tbl, ChangesetValue*& out);

    //! Builds fields for all non-system properties (ECInstanceId and ECClassId excluded); returns ERROR on first failure.
    BentleyStatus BuildPropertyFields(ClassMap const& classMap, BuildCtx& ctx);

    //! Returns true when @p classId is strictly derived from BisCore::Element (returns false for BisCore::Element itself).
    bool IsDerivedFromBisElement(ECClassId classId);

    //! Returns ERROR if any pointer in @p fields is null; used as a sanity guard before returning values to callers.
    BentleyStatus CheckInvalidPtr(std::vector<ChangesetValue*> const& fields) const;

    // ------------------------------------------------------------------
public:
    //! Resolves ECClassId for one changeset row; tries changeset first, then DB seek, then root ClassMap fallback.
    BentleyStatus ResolveClassId(DbTable const& tbl,
                                        ECClassId& resolvedClassIdOut,
                                        bool& classIdFromChangesetOut);

    //! Builds IECSqlValue fields for one changeset row; fills @p changedProps with access paths of all changed properties (e.g. "Pos2d.X", "Owner.Id").
    BentleyStatus Create(DbTable const& tbl,ECClassId resolvedClassId, bool classIdFromChangeset,std::vector<ChangesetValue*>& fields,std::vector<Utf8String>& changedProps);
    
    //! Clears m_changeValueMap; call before processing a new changeset row.
    void ClearChangeValueMap();
    //! Reads column values for @p tableName at @p stage from the iterator and populates m_changeValueMap.
    BentleyStatus PopulateChangeValueMap(Stage stage, Utf8StringCR tableName);
    //! Logs all entries in m_changeValueMap at debug verbosity.
    void DumpChangeValueMap() const;
    
    //! Constructs the factory; all references must outlive this object.
    ChangesetValueFactory(ECDbCR conn,  ChangesetFilter const& filter, ChangesetSqliteIterator const& iterator, PmrObjectAllocator<ChangesetValue>& allocator)
        : m_conn(conn), m_filter(filter), m_iterator(iterator), m_alloc(allocator) {}
};

END_BENTLEY_SQLITE_EC_NAMESPACE
