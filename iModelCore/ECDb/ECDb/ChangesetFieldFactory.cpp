/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using Stage = Changes::Change::Stage;

//=============================================================================
// Schema / mapping helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// Resolves the ClassMap for the table's root EC class.
// For overflow tables the root class lives on the parent table; for all other tables
// it is read directly from the table's exclusive-root mapping.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
const ClassMap* ChangesetFieldFactory::GetRootClassMap(DbTable const& tbl, ECDbCR conn) {
    ECClassId rootClassId;
    if (tbl.GetType() == DbTable::Type::Overflow) {
        // Overflow tables carry no class-id column; the root class is owned by the parent.
        rootClassId = tbl.GetLinkNode().GetParent()->GetTable().GetExclusiveRootECClassId();
    } else {
        rootClassId = tbl.GetExclusiveRootECClassId();
    }

    const auto rootClass = conn.Schemas().Main().GetClass(rootClassId);
    if (rootClass == nullptr)
        return nullptr;
    return conn.Schemas().Main().GetClassMap(*rootClass);
}

//---------------------------------------------------------------------------------------
// Builds an ECSqlPropertyPath from the property map's path segments.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath ChangesetFieldFactory::GetPropertyPath(PropertyMap const& propertyMap) {
    ECSqlPropertyPath path;
    for (auto& part : propertyMap.GetPath())
        path.AddEntry(part->GetProperty());
    return path;
}

//---------------------------------------------------------------------------------------
// Returns DateTime::Info for DateTime primitive/array properties; returns a default
// Unspecified DateTime::Info for all other property kinds.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info ChangesetFieldFactory::GetDateTimeInfo(PropertyMap const& propertyMap) {
    DateTime::Info info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    if (propertyMap.GetType() != PropertyMap::Type::PrimitiveArray &&
        propertyMap.GetType() != PropertyMap::Type::Primitive)
        return info;

    if (auto prop = propertyMap.GetProperty().GetAsPrimitiveArrayProperty()) {
        if (prop->GetType() == PRIMITIVETYPE_DateTime)
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *prop) == ECObjectsStatus::Success)
                return info;
    }
    if (auto prop = propertyMap.GetProperty().GetAsPrimitiveProperty()) {
        if (prop->GetType() == PRIMITIVETYPE_DateTime)
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *prop) == ECObjectsStatus::Success)
                return info;
    }
    return info;
}

//=============================================================================
// TableView helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// Creates a fresh TableView for @p tbl on every call (no caching).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<TableView> ChangesetFieldFactory::CreateTableView(ECDbCR conn, DbTable const& tbl) {
    return TableView::Create(conn, tbl);
}

//---------------------------------------------------------------------------------------
// Returns @p cachedView when it covers @p propTable (same table id); otherwise creates
// a new TableView stored in @p ownerStorage.  This avoids re-preparing the SELECT
// statement for the common case where all properties live in the same SQLite table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView const* ChangesetFieldFactory::GetTableView(DbTable const& propTable,
                                                      TableView const& cachedView,
                                                      std::unique_ptr<TableView>& ownerStorage,
                                                      ECDbCR conn) {
    if (cachedView.GetId() == propTable.GetId())
        return &cachedView;

    ownerStorage = TableView::Create(conn, propTable);
    if (ownerStorage == nullptr)
        LOG.errorv("Failed to create TableView for table '%s'.", propTable.GetName().c_str());
    return ownerStorage.get();
}

//=============================================================================
// Changeset value accessors
//=============================================================================

//---------------------------------------------------------------------------------------
// Returns true when @p colIdx is within the changeset's column range and the changeset
// carries an explicit (non-absent) value for it at the given stage.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::IsInChangeset(int colIdx, Changes::Change const& change, Stage stage) {
    return colIdx >= 0 && colIdx < change.GetColumnCount() && change.GetValue(colIdx, stage).IsValid();
}

//---------------------------------------------------------------------------------------
// Reads @p colIdx from the changeset.  Must only be called after IsInChangeset() has
// returned true — asserts in debug if the value turns out to be absent.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbValue ChangesetFieldFactory::GetChangesetValue(int colIdx, Changes::Change const& change, Stage stage) {
    DbValue v = change.GetValue(colIdx, stage);
    BeAssert(v.IsValid() && "GetChangesetValue called but column absent from changeset");
    return v;
}

//---------------------------------------------------------------------------------------
// Returns a DbValue for a single column.
//   • Fast path: column present in the changeset — return it directly.
//   • Slow path: column absent — seek the live database row via @p tableView and return
//     the persisted value.
// This fallback is ONLY for partial-update compound properties (Point2d/3d, Nav) where
// one sub-column may be modified while the other is not.
// Callers must ensure colIdx >= 0.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbValue ChangesetFieldFactory::GetValueWithDbFallback(int colIdx, Changes::Change const& change,
                                                       Stage stage, TableView const& tableView,
                                                       ECInstanceId instanceId) {
    DbValue v = change.GetValue(colIdx, stage);
    if (v.IsValid())
        return v;

    // Column absent from changeset — read from the live database via the TableView.
    tableView.Seek(instanceId);
    return tableView.GetSqliteStmt().GetColumnValue(colIdx);
}

//---------------------------------------------------------------------------------------
// Returns true when at least one SQLite column backing @p propertyMap is present in the
// changeset for @p stage.
//
// Rules per property kind:
//   System  — checks the specific class-id or data column.
//   Primitive — single column; Point2d/3d require at least one coordinate column.
//   Navigation — at least one of: id column, relClassId column (if physical).
//   Struct  — at least one member satisfies this predicate recursively.
//   Array   — the single JSON blob column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::PropertyHasChangesetData(PropertyMap const& propertyMap,
                                                     TableView const& tbl,
                                                     Changes::Change const& change, Stage stage) {
    const auto& prop = propertyMap.GetProperty();

    if (propertyMap.IsSystem()) {
        const auto prim = prop.GetAsPrimitiveProperty();
        if (prim == nullptr)
            return false;
        const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extType == ExtendedTypeHelper::ExtendedType::ClassId)
            return IsInChangeset(tbl.GetClassIdCol(), change, stage);
        if (extType == ExtendedTypeHelper::ExtendedType::SourceClassId)
            return IsInChangeset(tbl.GetSourceClassIdCol(), change, stage);
        if (extType == ExtendedTypeHelper::ExtendedType::TargetClassId)
            return IsInChangeset(tbl.GetTargetClassIdCol(), change, stage);

        const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
        const auto dataMap = sysMap.GetDataPropertyMaps().front();
        if (dataMap->GetColumn().IsVirtual())
            return false;
        return IsInChangeset(tbl.GetColumnIndexOf(dataMap->GetColumn()), change, stage);
    }

    if (prop.GetIsPrimitive()) {
        const auto prim = prop.GetAsPrimitiveProperty();
        if (prim->GetType() == PRIMITIVETYPE_Point2d) {
            const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
            // At least one coordinate changed — absent coordinate filled via GetValueWithDbFallback.
            return IsInChangeset(tbl.GetColumnIndexOf(pt2dMap.GetX().GetColumn()), change, stage) ||
                   IsInChangeset(tbl.GetColumnIndexOf(pt2dMap.GetY().GetColumn()), change, stage);
        }
        if (prim->GetType() == PRIMITIVETYPE_Point3d) {
            const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
            // At least one coordinate changed — absent coordinates filled via GetValueWithDbFallback.
            return IsInChangeset(tbl.GetColumnIndexOf(pt3dMap.GetX().GetColumn()), change, stage) ||
                   IsInChangeset(tbl.GetColumnIndexOf(pt3dMap.GetY().GetColumn()), change, stage) ||
                   IsInChangeset(tbl.GetColumnIndexOf(pt3dMap.GetZ().GetColumn()), change, stage);
        }
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        return IsInChangeset(tbl.GetColumnIndexOf(primMap.GetColumn()), change, stage);
    }

    if (prop.GetIsNavigation()) {
        const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
        // At least one of: id or relClassId (if physical) is in the changeset.
        if (PropertyHasChangesetData(navMap.GetIdPropertyMap(), tbl, change, stage))
            return true;
        const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
        if (relClassIdMap.GetColumn().IsVirtual())
            return false;
        return IsInChangeset(tbl.GetColumnIndexOf(relClassIdMap.GetColumn()), change, stage);
    }

    if (prop.GetIsStruct()) {
        // Struct is "changed" when at least one of its member columns is in the changeset.
        const auto& structMap = propertyMap.GetAs<StructPropertyMap>();
        for (const auto& memberMap : structMap)
            if (PropertyHasChangesetData(*memberMap, tbl, change, stage))
                return true;
        return false;
    }

    if (prop.GetIsArray()) {
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        return IsInChangeset(tbl.GetColumnIndexOf(primMap.GetColumn()), change, stage);
    }

    return false;
}

//=============================================================================
// ColumnInfo factory helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// Builds the ECSqlColumnInfo for a primitive (or Point2d/3d) property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo ChangesetFieldFactory::MakePrimitiveColumnInfo(PropertyMap const& propertyMap) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    return ECSqlColumnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        GetDateTimeInfo(propertyMap),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
}

//---------------------------------------------------------------------------------------
// Builds the ECSqlColumnInfo for a struct property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo ChangesetFieldFactory::MakeStructColumnInfo(PropertyMap const& propertyMap) {
    const auto structProp = propertyMap.GetProperty().GetAsStructProperty();
    return ECSqlColumnInfo(
        ECN::ECTypeDescriptor::CreateStructTypeDescriptor(),
        DateTime::Info(),
        &structProp->GetType(),
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
}

//---------------------------------------------------------------------------------------
// Builds the ECSqlColumnInfo for a navigation property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo ChangesetFieldFactory::MakeNavColumnInfo(PropertyMap const& propertyMap) {
    const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
    return ECSqlColumnInfo(
        ECN::ECTypeDescriptor::CreateNavigationTypeDescriptor(navProp->GetType(), navProp->IsMultiple()),
        GetDateTimeInfo(propertyMap),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
}

//---------------------------------------------------------------------------------------
// Builds the ECSqlColumnInfo for an array (primitive-array or struct-array) property.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo ChangesetFieldFactory::MakeArrayColumnInfo(PropertyMap const& propertyMap) {
    const auto& prop = propertyMap.GetProperty();
    ECN::ECTypeDescriptor desc;
    if (prop.GetIsStructArray())
        desc = ECN::ECTypeDescriptor::CreateStructArrayTypeDescriptor();
    else
        desc = ECN::ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(
            prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());

    return ECSqlColumnInfo(
        desc,
        GetDateTimeInfo(propertyMap),
        prop.GetIsStructArray() ? &prop.GetAsStructArrayProperty()->GetStructElementType() : nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
}

//=============================================================================
// Typed IECSqlValue factory methods
//=============================================================================

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a single-column primitive property.
//
// Scalar types (bool, int, string, etc.):
//   Read directly from the changeset.  Caller must have verified IsInChangeset().
//
// Point2d / Point3d (compound types):
//   Use GetValueWithDbFallback for each coordinate — one coordinate may be absent from
//   the changeset when only the other changed; the live DB fills the missing value.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreatePrimitive(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage, ECInstanceId instanceId) {

    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();

    if (prim->GetType() == ECN::PRIMITIVETYPE_Point2d) {
        // Partial-update: if only one coordinate changed the other is absent from the
        // changeset.  GetValueWithDbFallback fetches absent coordinates from the live DB.
        const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
        const int xCol = tbl.GetColumnIndexOf(pt2dMap.GetX().GetColumn());
        const int yCol = tbl.GetColumnIndexOf(pt2dMap.GetY().GetColumn());
        const DbValue xVal = GetValueWithDbFallback(xCol, change, stage, tbl, instanceId);
        const DbValue yVal = GetValueWithDbFallback(yCol, change, stage, tbl, instanceId);
        if (!xVal.IsValid() || !yVal.IsValid()) {
            LOG.errorv("Failed to read Point2d coordinate from live DB for property '%s'.",
                       propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
        return std::make_unique<ChangesetPoint2dValue>(MakePrimitiveColumnInfo(propertyMap), xVal, yVal);
    }

    if (prim->GetType() == PRIMITIVETYPE_Point3d) {
        // Same partial-update handling as Point2d — absent coordinates are filled from DB.
        const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
        const int xCol = tbl.GetColumnIndexOf(pt3dMap.GetX().GetColumn());
        const int yCol = tbl.GetColumnIndexOf(pt3dMap.GetY().GetColumn());
        const int zCol = tbl.GetColumnIndexOf(pt3dMap.GetZ().GetColumn());
        const DbValue xVal = GetValueWithDbFallback(xCol, change, stage, tbl, instanceId);
        const DbValue yVal = GetValueWithDbFallback(yCol, change, stage, tbl, instanceId);
        const DbValue zVal = GetValueWithDbFallback(zCol, change, stage, tbl, instanceId);
        if (!xVal.IsValid() || !yVal.IsValid() || !zVal.IsValid()) {
            LOG.errorv("Failed to read Point3d coordinate from live DB for property '%s'.",
                       propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
        return std::make_unique<ChangesetPoint3dValue>(MakePrimitiveColumnInfo(propertyMap), xVal, yVal, zVal);
    }

    // Scalar primitive — the column is guaranteed present (caller verified IsInChangeset).
    // No DB fallback: if it was not in the changeset the property would have been skipped.
    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    const int nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    return std::make_unique<ChangesetPrimitiveValue>(
        MakePrimitiveColumnInfo(propertyMap),
        GetChangesetValue(nCol, change, stage),
        GetDateTimeInfo(propertyMap));
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
// All system property columns must be present in the changeset — no DB fallback is used.
// Virtual class-id columns must NOT be passed here; use CreateFixedClassId() instead.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateSystem(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage) {

    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());

    // Dispatch to the dedicated class-id column exposed by TableView.
    if (extType == ExtendedTypeHelper::ExtendedType::ClassId && tbl.GetClassIdCol() >= 0) {
        return std::make_unique<ChangesetPrimitiveValue>(
            columnInfo,
            GetChangesetValue(tbl.GetClassIdCol(), change, stage));
    }
    if (extType == ExtendedTypeHelper::ExtendedType::SourceClassId && tbl.GetSourceClassIdCol() >= 0) {
        return std::make_unique<ChangesetPrimitiveValue>(
            columnInfo,
            GetChangesetValue(tbl.GetSourceClassIdCol(), change, stage));
    }
    if (extType == ExtendedTypeHelper::ExtendedType::TargetClassId && tbl.GetTargetClassIdCol() >= 0) {
        return std::make_unique<ChangesetPrimitiveValue>(
            columnInfo,
            GetChangesetValue(tbl.GetTargetClassIdCol(), change, stage));
    }

    // General system property — single physical column, always present in the changeset.
    const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
    const auto dataMap = sysMap.GetDataPropertyMaps().front();
    if (dataMap->GetColumn().IsVirtual()) {
        LOG.errorv("Virtual system property '%s' passed to CreateSystem — cannot read from changeset.",
                   propertyMap.GetProperty().GetName().c_str());
        return nullptr;
    }
    const int nCol = tbl.GetColumnIndexOf(dataMap->GetColumn());
    return std::make_unique<ChangesetPrimitiveValue>(
        columnInfo,
        GetChangesetValue(nCol, change, stage));
}

//---------------------------------------------------------------------------------------
// Creates a fixed-value IECSqlValue for a class-id property whose value is determined
// statically at mapping time (e.g. virtual RelECClassId on a single-class relationship).
// @p id must be a valid ECClassId.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateFixedClassId(
    ECDbCR conn, PropertyMap const& propertyMap, ECN::ECClassId id) {

    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreatePrimitiveTypeDescriptor(PRIMITIVETYPE_Long),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    return std::make_unique<ChangesetFixedInt64Value>(columnInfo, id);
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a navigation property.
//
// Navigation properties are compound: [id, relClassId].  Each sub-component may be
// absent from the changeset in a partial update:
//   • id     — uses GetValueWithDbFallback directly (scalar; CreatePrimitive would only
//              fall back to DB for Point2d/3d, not for scalar ids).
//   • relClassId (physical) — uses GetValueWithDbFallback.
//   • relClassId (virtual) — emitted as a fixed value; no DB read needed.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateNav(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage, ECInstanceId instanceId) {

    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();

    // Id sub-component — the id column may be absent from the changeset when only the
    // relClassId was updated.  Use GetValueWithDbFallback so the live DB is consulted
    // when the column is absent.  (CreatePrimitive only falls back to DB for Point2d/3d,
    // not for scalar ids.)
    const auto& idPropMap = navMap.GetIdPropertyMap();
    const auto& primMap = idPropMap.GetAs<SingleColumnDataPropertyMap>();
    const int nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    const DbValue idDbVal = GetValueWithDbFallback(nCol, change, stage, tbl, instanceId);
    if (!idDbVal.IsValid()) {
        LOG.errorv("Failed to read navigation Id from live DB for property '%s'.",
                   propertyMap.GetProperty().GetName().c_str());
        return nullptr;
    }
    auto idVal = std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(idPropMap), idDbVal);

    // RelClassId sub-component.
    std::unique_ptr<IECSqlValue> relClassIdVal;
    const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    if (relClassIdMap.GetColumn().IsVirtual()) {
        // Single-class relationship — the class-id is fixed at mapping time.
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        relClassIdVal = CreateFixedClassId(conn, relClassIdMap, navProp->GetRelationshipClass()->GetId());
    } else {
        // Physical column — may be absent from the changeset in a partial update.
        const int relCol = tbl.GetColumnIndexOf(relClassIdMap.GetColumn());
        const DbValue relVal = GetValueWithDbFallback(relCol, change, stage, tbl, instanceId);
        if (!relVal.IsValid()) {
            LOG.errorv("Failed to read navigation RelECClassId from live DB for property '%s'.",
                       propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
        relClassIdVal = std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(relClassIdMap), relVal);
    }

    return std::make_unique<ChangesetNavValue>(
        MakeNavColumnInfo(propertyMap),
        std::move(idVal),
        std::move(relClassIdVal));
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for an array (primitive-array or struct-array) property.
// Arrays are stored as a single JSON blob column — read directly from the changeset.
// No DB fallback: if the column is not in the changeset the property is skipped upstream.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateArray(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage) {

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    const int nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    // Column is guaranteed present: caller verified PropertyHasChangesetData().
    return std::make_unique<ChangesetArrayValue>(
        MakeArrayColumnInfo(propertyMap),
        GetChangesetValue(nCol, change, stage),
        conn);
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a struct property.
// Only struct members whose backing column(s) are present in the changeset are included;
// unchanged members are omitted entirely (partial struct update semantics).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateStruct(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage, ECInstanceId instanceId) {

    auto structVal = std::make_unique<ChangesetStructValue>(MakeStructColumnInfo(propertyMap));
    for (auto& memberMap : propertyMap.GetAs<StructPropertyMap>()) {
        // Omit unchanged members — only include what is actually in the changeset.
        if (!PropertyHasChangesetData(*memberMap, tbl, change, stage))
            continue;

        auto memberVal = CreateValueInternal(conn, *memberMap, tbl, change, stage, instanceId);
        if (memberVal == nullptr)
            return nullptr;  // Propagate DB-read failure from member.
        structVal->AppendMember(memberMap->GetProperty().GetName(), std::move(memberVal));
    }
    return structVal;
}

//---------------------------------------------------------------------------------------
// Dispatches to the appropriate typed Create* method.  Does NOT check for changeset
// presence — callers are responsible for calling PropertyHasChangesetData() first.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateValueInternal(
    ECDbCR conn, PropertyMap const& propertyMap, TableView const& tbl,
    Changes::Change const& change, Stage stage, ECInstanceId instanceId) {

    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem())
        return CreateSystem(conn, propertyMap, tbl, change, stage);
    if (prop.GetIsPrimitive())
        return CreatePrimitive(conn, propertyMap, tbl, change, stage, instanceId);
    if (prop.GetIsStruct())
        return CreateStruct(conn, propertyMap, tbl, change, stage, instanceId);
    if (prop.GetIsNavigation())
        return CreateNav(conn, propertyMap, tbl, change, stage, instanceId);
    if (prop.GetIsArray())
        return CreateArray(conn, propertyMap, tbl, change, stage);

    BeAssert(false && "Unknown property type in ChangesetFieldFactory::CreateValueInternal");
    return nullptr;
}

//=============================================================================
// ChangesetFieldFactory — high-level resolution helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// Attempts to resolve an ECClassId and its ClassMap from the changeset.
// The TableView's class-id column index carries the concrete class for polymorphic rows.
// Returns true when a valid ClassId and ClassMap are found; @p classMapOut and
// @p classIdOut are populated accordingly.  Returns false when the column is absent,
// null, zero, or the class is unknown.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryResolveClassMapFromChangeset(
    TableView const& tbl, Changes::Change const& change, Stage stage,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    const int classIdColIdx = tbl.GetClassIdCol();
    if (classIdColIdx < 0)
        return false;

    DbValue classIdVal = change.GetValue(classIdColIdx, stage);
    if (!classIdVal.IsValid() || classIdVal.IsNull())
        return false;

    ECClassId candidate(classIdVal.GetValueUInt64());
    if (!candidate.IsValid())
        return false;

    const ECClass* cls = conn.Schemas().Main().GetClass(candidate);
    if (cls == nullptr)
        return false;

    const ClassMap* cm = conn.Schemas().Main().GetClassMap(*cls);
    if (cm == nullptr)
        return false;

    classMapOut = cm;
    classIdOut  = candidate;
    return true;
}

//---------------------------------------------------------------------------------------
// Attempts to resolve an ECClassId and its ClassMap by:
//   1. Extracting the first primary-key column value from the changeset as ECInstanceId.
//   2. Seeking the live database row via @p tbl.
//   3. Reading the ECClassId column from the sought row.
// This path is taken when the class-id column is absent from the changeset (e.g. entity
// tables where ECClassId is a virtual column not captured in changeset data).
// Returns true when a valid class and ClassMap are resolved; populates @p classMapOut
// and @p classIdOut.  Returns false silently on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryResolveClassMapFromDbSeek(
    TableView const& tbl, Changes::Change const& change, Stage stage,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    // The DB seek is only meaningful when the TableView exposes a class-id column.
    if (tbl.GetClassIdCol() < 0)
        return false;

    // Find the first primary-key column and read its value from the changeset.
    int pkCol = -1;
    for (int i = 0; i < change.GetColumnCount(); ++i) {
        if (change.IsPrimaryKeyColumn(i)) {
            pkCol = i;
            break;
        }
    }
    if (pkCol < 0)
        return false;

    DbValue pkVal = change.GetValue(pkCol, stage);
    if (!pkVal.IsValid() || pkVal.IsNull())
        return false;

    ECInstanceId instanceId(pkVal.GetValueUInt64());
    if (!instanceId.IsValid())
        return false;

    // Seek the live database and read the class-id from the row.
    ECN::ECClassId classId;
    if (!tbl.Seek(instanceId, &classId))
        return false;

    if (!classId.IsValid())
        return false;

    const ECClass* cls = conn.Schemas().Main().GetClass(classId);
    if (cls == nullptr)
        return false;

    const ClassMap* cm = conn.Schemas().Main().GetClassMap(*cls);
    if (cm == nullptr)
        return false;

    classMapOut = cm;
    classIdOut  = classId;
    return true;
}

//---------------------------------------------------------------------------------------
// Finds the ECInstanceId property in @p classMap, reads its value exclusively from the
// changeset, validates it, and emits the corresponding IECSqlValue field.
//
// @p tbl is the primary TableView for the change row; a per-property TableView is
// obtained via GetTableView when the ECInstanceId column lives in a different physical
// table (e.g. overflow table).
//
// Returns true on success; @p instanceIdOut and @p fieldOut are populated.
// Returns false and logs an error on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::ResolveInstanceId(
    ClassMap const& classMap, TableView const& primaryTbl,
    Changes::Change const& change, Stage stage,
    ECDbCR conn, DbTable const& primaryDbTable,
    ECInstanceId& instanceIdOut, std::unique_ptr<IECSqlValue>& fieldOut) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        if (!propertyMap->IsSystem())
            continue;
        const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
        if (prim == nullptr)
            continue;
        const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extType != ExtendedTypeHelper::ExtendedType::Id)
            continue;
        if (!propertyMap->GetProperty().GetName().EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            continue;

        // ECInstanceId is the primary key — it must always be present in the changeset.
        // Always look up the index in primaryTbl: the changeset column indices belong
        // exclusively to the table that was passed to Create().
        const auto& sysMap = propertyMap->GetAs<SystemPropertyMap>();
        const int colIdx   = primaryTbl.GetColumnIndexOf(sysMap.GetDataPropertyMaps().front()->GetColumn());
        DbValue rawId      = change.GetValue(colIdx, stage);
        if (!rawId.IsValid() || rawId.IsNull()) {
            LOG.errorv("ECInstanceId is absent or null in changeset for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return false;
        }

        ECInstanceId instanceId(rawId.GetValueUInt64());
        if (!instanceId.IsValid()) {
            LOG.errorv("ECInstanceId resolved to an invalid (zero) id for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return false;
        }

        instanceIdOut = instanceId;
        fieldOut      = CreateSystem(conn, *propertyMap, primaryTbl, change, stage);
        LOG.debugv("Table '%s': resolved ECInstanceId %" PRIu64 " from changeset.",
                   primaryDbTable.GetName().c_str(), instanceId.GetValueUnchecked());
        return true;
    }

    LOG.errorv("ECInstanceId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return false;
}

//---------------------------------------------------------------------------------------
// Builds the ECClassId field for the result set as a fixed value from the already-resolved
// @p resolvedClassId.  The class-id is always known before this function is called
// (resolved in step 2 of Create), so there is no need to re-read it from the changeset.
//
// Returns nullptr and logs an error when the ECClassId property is not found.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::ResolveClassIdField(
    ClassMap const& classMap,
    ECClassId resolvedClassId,
    ECDbCR conn, DbTable const& primaryDbTable) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        if (!propertyMap->IsSystem())
            continue;
        const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
        if (prim == nullptr)
            continue;
        const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extType != ExtendedTypeHelper::ExtendedType::ClassId)
            continue;
        if (!propertyMap->GetProperty().GetName().EqualsIAscii(ECDBSYS_PROP_ECClassId))
            continue;

        return CreateFixedClassId(conn, *propertyMap, resolvedClassId);
    }

    LOG.errorv("ECClassId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return nullptr;
}

//---------------------------------------------------------------------------------------
// Iterates all properties in @p classMap and appends an IECSqlValue to @p fieldsOut for
// each property that has at least one column present in the changeset.
//
// ECInstanceId and ECClassId are always skipped here — they are emitted separately by
// the caller as slots [0] and [1].
//
// For compound properties (Point2d/3d, Nav): absent sub-columns are filled from the
// live DB via GetValueWithDbFallback (inside CreatePrimitive / CreateNav).
// For scalar, array, and struct properties: changeset-only reads are used.
//
// Returns true on success.  Returns false (with a logged error) only if a TableView
// cannot be obtained for a property's physical table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::BuildPropertyFields(
    ClassMap const& classMap, TableView const& primaryTbl,
    Changes::Change const& change, Stage stage,
    ECInstanceId instanceId, ECDbCR conn,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {

        // ECInstanceId and ECClassId are emitted as fixed slots [0] and [1] by the caller.
        if (propertyMap->IsSystem()) {
            const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
            if (prim != nullptr) {
                const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
                const Utf8StringCR propName = propertyMap->GetProperty().GetName();
                if (extType == ExtendedTypeHelper::ExtendedType::Id &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
                    continue;
                if (extType == ExtendedTypeHelper::ExtendedType::ClassId &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
                    continue;
            }
        }

        // Always use primaryTbl for all changeset index lookups — the changeset column
        // indices belong exclusively to the one table that was passed to Create().
        // Properties that live in a different physical table (e.g. overflow) will not
        // appear in this changeset entry and will be skipped by PropertyHasChangesetData.
        if (!PropertyHasChangesetData(*propertyMap, primaryTbl, change, stage))
            continue;

        auto val = CreateValueInternal(conn, *propertyMap, primaryTbl, change, stage, instanceId);
        if (val == nullptr)
            return false;
        fieldsOut.emplace_back(std::move(val));
    }

    return true;
}

//=============================================================================
// ChangesetFieldFactory::Create — public entry point
//
// Produces an ordered list of IECSqlValue fields representing one changeset row:
//   [0]  ECInstanceId  — always first; read exclusively from the changeset.
//   [1]  ECClassId     — always second; preferred from the changeset, then from
//                        a live DB seek, then from GetRootClassMap.
//   [2+] User / system properties that have at least one column in the changeset.
//
// Strategy for ECClassId resolution (in priority order):
//   1. Read from the changeset via TryResolveClassMapFromChangeset.
//   2. Seek the live DB using the first PK column as ECInstanceId via
//      TryResolveClassMapFromDbSeek (handles entity tables where ECClassId is a
//      virtual column absent from the changeset).
//   3. Fall back to GetRootClassMap when neither of the above yields a usable
//      ClassMap (overflow tables, single-class entity tables).
//   4. Return an error when no path yields a usable ClassMap.
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::Create(
    ECDbCR conn, DbTable const& tbl, Changes::Change const& change, Stage const& stage,
    std::vector<std::unique_ptr<IECSqlValue>>& fields) {

    // -----------------------------------------------------------------------
    // Step 1: Create a TableView so we can map DbColumn objects to changeset
    // column indices and seek live DB rows for partial-update fallback reads.
    // -----------------------------------------------------------------------
    auto tableViewOwner = CreateTableView(conn, tbl);
    if (tableViewOwner == nullptr) {
        LOG.errorv("Failed to create TableView for table '%s'.", tbl.GetName().c_str());
        return BE_SQLITE_ERROR;
    }
    TableView const& tableView = *tableViewOwner;

    // -----------------------------------------------------------------------
    // Step 2: Resolve ClassMap — try changeset, then DB seek, fall back to root map.
    // -----------------------------------------------------------------------
    const ClassMap*  classMap          = nullptr;
    ECClassId        resolvedClassId;
    if (TryResolveClassMapFromChangeset(tableView, change, stage, conn, classMap, resolvedClassId)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " from changeset.",
                   tbl.GetName().c_str(), resolvedClassId.GetValueUnchecked());
    } else if (TryResolveClassMapFromDbSeek(tableView, change, stage, conn, classMap, resolvedClassId)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via DB seek.",
                   tbl.GetName().c_str(), resolvedClassId.GetValueUnchecked());
    } else {
        classMap = GetRootClassMap(tbl, conn);
        if (classMap != nullptr) {
            resolvedClassId = classMap->GetClass().GetId();
            LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via GetRootClassMap.",
                       tbl.GetName().c_str(), resolvedClassId.GetValueUnchecked());
        }
    }

    if (classMap == nullptr || !resolvedClassId.IsValid()) {
        LOG.errorv("Could not resolve ECClassId or ClassMap for table '%s'.", tbl.GetName().c_str());
        return BE_SQLITE_ERROR;
    }

    // -----------------------------------------------------------------------
    // Step 3: Resolve ECInstanceId (slot [0]).
    // -----------------------------------------------------------------------
    ECInstanceId instanceId;
    std::unique_ptr<IECSqlValue>     instanceIdField;
    if (!ResolveInstanceId(*classMap, tableView, change, stage, conn, tbl, instanceId, instanceIdField))
        return BE_SQLITE_ERROR;

    // -----------------------------------------------------------------------
    // Step 4: Build ECClassId field (slot [1]).
    // -----------------------------------------------------------------------
    std::unique_ptr<IECSqlValue> classIdField = ResolveClassIdField(*classMap, resolvedClassId, conn, tbl);
    if (classIdField == nullptr)
        return BE_SQLITE_ERROR;

    // -----------------------------------------------------------------------
    // Step 5: Build remaining property fields (slots [2+]).
    // -----------------------------------------------------------------------
    fields.emplace_back(std::move(instanceIdField));
    fields.emplace_back(std::move(classIdField));

    if (!BuildPropertyFields(*classMap, tableView, change, stage, instanceId, conn, fields)) {
        fields.clear();
        return BE_SQLITE_ERROR;
    }

    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
