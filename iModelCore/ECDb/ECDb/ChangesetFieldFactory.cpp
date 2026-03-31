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
// ColumnValueMap accessors
//=============================================================================

//---------------------------------------------------------------------------------------
// Returns true when @p colName is present in @p columnValues with a valid DbValue.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::IsInMap(Utf8StringCR colName, ColumnValueMap const& columnValues) {
    auto it = columnValues.find(colName);
    return it != columnValues.end() && it->second.IsValid();
}

//---------------------------------------------------------------------------------------
// Reads @p colName from @p columnValues.  Must only be called after IsInMap() has
// returned true — asserts in debug if the key is absent.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbValue ChangesetFieldFactory::GetFromMap(Utf8StringCR colName, ColumnValueMap const& columnValues) {
    auto it = columnValues.find(colName);
    BeAssert(it != columnValues.end() && "GetFromMap called but column absent from map");
    return it != columnValues.end() ? it->second : DbValue(nullptr);
}

//---------------------------------------------------------------------------------------
// Returns a DbValue for a single column.
//   • Fast path: column present in columnValues — return it directly.
//   • Slow path: column absent — seek the live database row via @p tableView and return
//     the persisted value.
// This fallback is ONLY for partial-update compound properties (Point2d/3d, Nav) where
// one sub-column may be modified while the other is not.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//---------------------------------------------------------------------------------------
// Returns true when at least one SQLite column backing @p propertyMap is present in
// @p columnValues.
//
// Rules per property kind:
//   System  — checks the specific data column.
//   Primitive — single column; Point2d/3d require at least one coordinate column.
//   Navigation — at least one of: id column, relClassId column (if physical).
//   Struct  — at least one member satisfies this predicate recursively.
//   Array   — the single JSON blob column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::PropertyHasChangesetData(PropertyMap const& propertyMap,
                                                     ColumnValueMap const& columnValues) {
    const auto& prop = propertyMap.GetProperty();

    if (propertyMap.IsSystem()) {
        const auto prim = prop.GetAsPrimitiveProperty();
        if (prim == nullptr)
            return false;
        const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
        const auto dataMap = sysMap.GetDataPropertyMaps().front();
        if (dataMap->GetColumn().IsVirtual())
            return false;
        return IsInMap(dataMap->GetColumn().GetName(), columnValues);
    }

    if (prop.GetIsPrimitive()) {
        const auto prim = prop.GetAsPrimitiveProperty();
        if (prim->GetType() == PRIMITIVETYPE_Point2d) {
            const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
            return IsInMap(pt2dMap.GetX().GetColumn().GetName(), columnValues) ||
                   IsInMap(pt2dMap.GetY().GetColumn().GetName(), columnValues);
        }
        if (prim->GetType() == PRIMITIVETYPE_Point3d) {
            const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
            return IsInMap(pt3dMap.GetX().GetColumn().GetName(), columnValues) ||
                   IsInMap(pt3dMap.GetY().GetColumn().GetName(), columnValues) ||
                   IsInMap(pt3dMap.GetZ().GetColumn().GetName(), columnValues);
        }
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        return IsInMap(primMap.GetColumn().GetName(), columnValues);
    }

    if (prop.GetIsNavigation()) {
        const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
        if (PropertyHasChangesetData(navMap.GetIdPropertyMap(), columnValues))
            return true;
        const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
        if (relClassIdMap.GetColumn().IsVirtual())
            return false;
        return IsInMap(relClassIdMap.GetColumn().GetName(), columnValues);
    }

    if (prop.GetIsStruct()) {
        const auto& structMap = propertyMap.GetAs<StructPropertyMap>();
        for (const auto& memberMap : structMap)
            if (PropertyHasChangesetData(*memberMap, columnValues))
                return true;
        return false;
    }

    if (prop.GetIsArray()) {
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        return IsInMap(primMap.GetColumn().GetName(), columnValues);
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
//   Read directly from the column values map.
//
// Point2d / Point3d (compound types):
//   All coordinates present  → returns a full compound value.
//   Only some coordinates present → returns a ChangesetPrimitiveValue (Double) for the
//   first present coordinate.  No DB fallback is performed.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreatePrimitive(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();

    if (prim->GetType() == ECN::PRIMITIVETYPE_Point2d) {
        const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
        const bool hasX = IsInMap(pt2dMap.GetX().GetColumn().GetName(), columnValues);
        const bool hasY = IsInMap(pt2dMap.GetY().GetColumn().GetName(), columnValues);
        if (hasX && hasY)
            return std::make_unique<ChangesetPoint2dValue>(MakePrimitiveColumnInfo(propertyMap),
                GetFromMap(pt2dMap.GetX().GetColumn().GetName(), columnValues),
                GetFromMap(pt2dMap.GetY().GetColumn().GetName(), columnValues));
        // Partial — emit the present coordinate as a raw double value.
        if(hasX)
        {
            return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(pt2dMap.GetX()), GetFromMap(pt2dMap.GetX().GetColumn().GetName(), columnValues), GetDateTimeInfo(pt2dMap.GetX()));
        }
        if(hasY)
        {
            return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(pt2dMap.GetY()), GetFromMap(pt2dMap.GetY().GetColumn().GetName(), columnValues), GetDateTimeInfo(pt2dMap.GetY()));
        }
    }

    if (prim->GetType() == PRIMITIVETYPE_Point3d) {
        const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
        const bool hasX = IsInMap(pt3dMap.GetX().GetColumn().GetName(), columnValues);
        const bool hasY = IsInMap(pt3dMap.GetY().GetColumn().GetName(), columnValues);
        const bool hasZ = IsInMap(pt3dMap.GetZ().GetColumn().GetName(), columnValues);
        if (hasX && hasY && hasZ)
            return std::make_unique<ChangesetPoint3dValue>(MakePrimitiveColumnInfo(propertyMap),
                GetFromMap(pt3dMap.GetX().GetColumn().GetName(), columnValues),
                GetFromMap(pt3dMap.GetY().GetColumn().GetName(), columnValues),
                GetFromMap(pt3dMap.GetZ().GetColumn().GetName(), columnValues));
        // Partial — emit the first present coordinate as a raw double value.
        if(hasX)
        {
            return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(pt3dMap.GetX()), GetFromMap(pt3dMap.GetX().GetColumn().GetName(), columnValues), GetDateTimeInfo(pt3dMap.GetX()));
        }
        if(hasY)
        {
            return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(pt3dMap.GetY()), GetFromMap(pt3dMap.GetY().GetColumn().GetName(), columnValues), GetDateTimeInfo(pt3dMap.GetY()));
        }
        if(hasZ)
        {
            return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(pt3dMap.GetZ()), GetFromMap(pt3dMap.GetZ().GetColumn().GetName(), columnValues), GetDateTimeInfo(pt3dMap.GetZ()));
        }
    }

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    return std::make_unique<ChangesetPrimitiveValue>(
        MakePrimitiveColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        GetDateTimeInfo(propertyMap));
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
// All system property columns must be present in the changeset — no DB fallback is used.
// Virtual class-id columns must NOT be passed here; use CreateFixedClassId() instead.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateSystem(
    ECDbCR conn, PropertyMap const& propertyMap,
    ColumnValueMap const& columnValues) {

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

    const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
    const auto dataMap = sysMap.GetDataPropertyMaps().front();
    if (dataMap->GetColumn().IsVirtual()) {
        LOG.errorv("Virtual system property '%s' passed to CreateSystem — cannot read from changeset.",
                   propertyMap.GetProperty().GetName().c_str());
        return nullptr;
    }
    Utf8StringCR colName = dataMap->GetColumn().GetName();
    auto it = columnValues.find(colName);
    if (it == columnValues.end()) {
        LOG.errorv("System property column '%s' not found in column values map.", colName.c_str());
        return nullptr;
    }
    return std::make_unique<ChangesetPrimitiveValue>(columnInfo, it->second);
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
// All values are read exclusively from the map — no DB fallback.
//   • Both id and relClassId resolvable (physical+in-map or virtual) → ChangesetNavValue.
//   • Only id available → ChangesetPrimitiveValue for id.
//   • Only relClassId physical available → ChangesetPrimitiveValue for relClassId.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateNav(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();

    // Id sub-component — read from map only.
    const auto& idPropMap = navMap.GetIdPropertyMap();
    const auto& idColName = idPropMap.GetAs<SingleColumnDataPropertyMap>().GetColumn().GetName();
    const bool hasId = IsInMap(idColName, columnValues);

    // RelClassId sub-component — virtual (fixed) or physical (from map).
    const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    std::unique_ptr<IECSqlValue> relClassIdVal = nullptr;
    if (relClassIdMap.GetColumn().IsVirtual()) {
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        relClassIdVal = CreateFixedClassId(conn, relClassIdMap, navProp->GetRelationshipClass()->GetId());
    } else if (IsInMap(relClassIdMap.GetColumn().GetName(), columnValues)) {
        relClassIdVal = std::make_unique<ChangesetPrimitiveValue>(
            MakePrimitiveColumnInfo(relClassIdMap),
            GetFromMap(relClassIdMap.GetColumn().GetName(), columnValues));
    }

    if (hasId && relClassIdVal != nullptr) {
        return std::make_unique<ChangesetNavValue>(
            MakeNavColumnInfo(propertyMap),
            std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(idPropMap),
                                                     GetFromMap(idColName, columnValues)),
            std::move(relClassIdVal));
    }

    // Partial — emit whichever sub-component is available as a plain primitive.
    if (hasId)
        return std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(idPropMap),
                                                        GetFromMap(idColName, columnValues));
    return relClassIdVal;  // may be nullptr if neither sub-component is in the map
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for an array (primitive-array or struct-array) property.
// Arrays are stored as a single JSON blob column — read directly from the changeset.
// No DB fallback: if the column is not in the changeset the property is skipped upstream.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateArray(
    ECDbCR conn, PropertyMap const& propertyMap,
    ColumnValueMap const& columnValues) {

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    // Column is guaranteed present: caller verified PropertyHasChangesetData().
    return std::make_unique<ChangesetArrayValue>(
        MakeArrayColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        conn);
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a struct property.
// Only struct members whose backing column(s) are present in the changeset are included;
// unchanged members are omitted entirely (partial struct update semantics).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateStruct(
    ECDbCR conn, PropertyMap const& propertyMap,
    ColumnValueMap const& columnValues) {

    auto structVal = std::make_unique<ChangesetStructValue>(MakeStructColumnInfo(propertyMap));
    for (auto& memberMap : propertyMap.GetAs<StructPropertyMap>()) {
        if (!PropertyHasChangesetData(*memberMap, columnValues))
            continue;

        auto memberVal = CreateValueInternal(conn, *memberMap, columnValues);
        if (memberVal == nullptr)
            return nullptr;
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
    ECDbCR conn, PropertyMap const& propertyMap,
    ColumnValueMap const& columnValues) {

    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem())
        return CreateSystem(conn, propertyMap, columnValues);
    if (prop.GetIsPrimitive())
        return CreatePrimitive(conn, propertyMap, columnValues);
    if (prop.GetIsStruct())
        return CreateStruct(conn, propertyMap, columnValues);
    if (prop.GetIsNavigation())
        return CreateNav(conn, propertyMap, columnValues);
    if (prop.GetIsArray())
        return CreateArray(conn, propertyMap, columnValues);

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
    DbTable const& dbTable, ColumnValueMap const& columnValues,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();
    if (classIdCol.IsVirtual())
        return false;

    auto it = columnValues.find(classIdCol.GetName());
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    ECClassId candidate(it->second.GetValueUInt64());
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
    TableView const& tbl, DbTable const& dbTable,
    ColumnValueMap const& columnValues,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    // The DB seek is only meaningful when the TableView exposes a class-id column.
    if (tbl.GetClassIdCol() < 0)
        return false;

    // Find the first primary-key column via the schema and read its value from the map.
    PrimaryKeyDbConstraint const* pkConstraint = dbTable.GetPrimaryKeyConstraint();
    if (pkConstraint == nullptr || pkConstraint->GetColumns().empty())
        return false;

    DbColumn const& firstPkCol = *pkConstraint->GetColumns().front();
    auto it = columnValues.find(firstPkCol.GetName());
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    ECInstanceId instanceId(it->second.GetValueUInt64());
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
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
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
        const auto& sysMap = propertyMap->GetAs<SystemPropertyMap>();
        Utf8StringCR colName = sysMap.GetDataPropertyMaps().front()->GetColumn().GetName();
        auto it = columnValues.find(colName);
        if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull()) {
            LOG.errorv("ECInstanceId is absent or null in changeset for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return false;
        }

        ECInstanceId instanceId(it->second.GetValueUInt64());
        if (!instanceId.IsValid()) {
            LOG.errorv("ECInstanceId resolved to an invalid (zero) id for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return false;
        }

        instanceIdOut = instanceId;
        fieldOut      = CreateSystem(conn, *propertyMap, columnValues);
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
// each property that has at least one column present in the map.
//
// ECInstanceId and ECClassId are always skipped here — they are emitted separately by
// the caller as slots [0] and [1].
//
// For compound properties (Point2d/3d, Nav): all sub-columns read exclusively from the
// map — no DB fallback.  Partial compound values are emitted as primitives.
// For scalar, array, and struct properties: read directly from the map.
//
// Returns true on success.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::BuildPropertyFields(
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
    ECDbCR conn,
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

        if (!PropertyHasChangesetData(*propertyMap, columnValues))
            continue;

        auto val = CreateValueInternal(conn, *propertyMap, columnValues);
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
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fields) {

    // -----------------------------------------------------------------------
    // Step 1: Create a TableView so we can seek live DB rows for partial-update
    // fallback reads (Point2d/3d, Nav properties).
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
    if (TryResolveClassMapFromChangeset(tbl, columnValues, conn, classMap, resolvedClassId)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " from changeset.",
                   tbl.GetName().c_str(), resolvedClassId.GetValueUnchecked());
    } else if (TryResolveClassMapFromDbSeek(tableView, tbl, columnValues, conn, classMap, resolvedClassId)) {
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
    if (!ResolveInstanceId(*classMap, columnValues, conn, tbl, instanceId, instanceIdField))
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

    if (!BuildPropertyFields(*classMap, columnValues, conn, fields)) {
        fields.clear();
        return BE_SQLITE_ERROR;
    }

    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
