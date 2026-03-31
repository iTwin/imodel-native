/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
// Fetches a single real column value from the live DB for the row identified by
// the table's primary-key column (read from @p columnValues).
// Returns true and sets @p outVal on success; returns false on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryFetchDoubleFromDb(double& outVal, ECDbCR conn,
                                                 DbColumn const& col,
                                                 ColumnValueMap const& columnValues) {
    DbTable const& tbl = col.GetTable();
    PrimaryKeyDbConstraint const* pk = tbl.GetPrimaryKeyConstraint();
    if (pk == nullptr || pk->GetColumns().empty())
        return false;

    Utf8StringCR pkColName = pk->GetColumns().front()->GetName();
    auto it = columnValues.find(pkColName);
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    CachedStatementPtr stmt = conn.GetCachedStatement(
        Utf8PrintfString("SELECT [%s] FROM [%s] WHERE [%s]=?",
            col.GetName().c_str(), tbl.GetName().c_str(), pkColName.c_str()).c_str());
    if (stmt == nullptr)
        return false;

    stmt->BindUInt64(1, it->second.GetValueUInt64());
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    outVal = stmt->GetValueDouble(0);
    return true;
}

//---------------------------------------------------------------------------------------
// Fetches a single integer column value from the live DB for the row identified by
// the table's primary-key column (read from @p columnValues).
// Returns true and sets @p outVal on success; returns false on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryFetchInt64FromDb(int64_t& outVal, ECDbCR conn,
                                                DbColumn const& col,
                                                ColumnValueMap const& columnValues) {
    DbTable const& tbl = col.GetTable();
    PrimaryKeyDbConstraint const* pk = tbl.GetPrimaryKeyConstraint();
    if (pk == nullptr || pk->GetColumns().empty())
        return false;

    Utf8StringCR pkColName = pk->GetColumns().front()->GetName();
    auto it = columnValues.find(pkColName);
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    CachedStatementPtr stmt = conn.GetCachedStatement(
        Utf8PrintfString("SELECT [%s] FROM [%s] WHERE [%s]=?",
            col.GetName().c_str(), tbl.GetName().c_str(), pkColName.c_str()).c_str());
    if (stmt == nullptr)
        return false;

    stmt->BindUInt64(1, it->second.GetValueUInt64());
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    outVal = stmt->GetValueInt64(0);
    return true;
}

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
// Creates an IECSqlValue for a Point2d property.
// If neither coordinate is in the changeset the property is skipped.
// A missing coordinate is fetched from the live DB only when the other IS in the changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreatePoint2d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
    const DbColumn& xCol = pt2dMap.GetX().GetColumn();
    const DbColumn& yCol = pt2dMap.GetY().GetColumn();

    const bool xInChangeset = IsInMap(xCol.GetName(), columnValues);
    const bool yInChangeset = IsInMap(yCol.GetName(), columnValues);

    // Neither coordinate in changeset — skip entirely, no DB lookup.
    if (!xInChangeset && !yInChangeset)
        return nullptr;

    double x = 0.0, y = 0.0;

    if (xInChangeset) {
        x = GetFromMap(xCol.GetName(), columnValues).GetValueDouble();
    } else {
        // Y is in changeset; fetch X from DB to complete the point.
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.warningv("Point2d property '%s': X coordinate absent and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
    }

    if (yInChangeset) {
        y = GetFromMap(yCol.GetName(), columnValues).GetValueDouble();
    } else {
        // X is in changeset; fetch Y from DB to complete the point.
        if (!TryFetchDoubleFromDb(y, conn, yCol, columnValues)) {
            LOG.warningv("Point2d property '%s': Y coordinate absent and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
    }

    return std::make_unique<ChangesetPoint2dValue>(MakePrimitiveColumnInfo(propertyMap), x, y);
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a Point3d property.
// If no coordinates are in the changeset the property is skipped.
// A missing coordinate is fetched from the live DB only when at least one other IS in the changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreatePoint3d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
    const DbColumn& xCol = pt3dMap.GetX().GetColumn();
    const DbColumn& yCol = pt3dMap.GetY().GetColumn();
    const DbColumn& zCol = pt3dMap.GetZ().GetColumn();

    const bool xInChangeset = IsInMap(xCol.GetName(), columnValues);
    const bool yInChangeset = IsInMap(yCol.GetName(), columnValues);
    const bool zInChangeset = IsInMap(zCol.GetName(), columnValues);

    // No coordinates in changeset — skip entirely, no DB lookup.
    if (!xInChangeset && !yInChangeset && !zInChangeset)
        return nullptr;

    double x = 0.0, y = 0.0, z = 0.0;

    if (xInChangeset) {
        x = GetFromMap(xCol.GetName(), columnValues).GetValueDouble();
    } else {
        // At least one other coordinate is in changeset; fetch X from DB.
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.warningv("Point3d property '%s': X coordinate absent and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
    }

    if (yInChangeset) {
        y = GetFromMap(yCol.GetName(), columnValues).GetValueDouble();
    } else {
        // At least one other coordinate is in changeset; fetch Y from DB.
        if (!TryFetchDoubleFromDb(y, conn, yCol, columnValues)) {
            LOG.warningv("Point3d property '%s': Y coordinate absent and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
    }

    if (zInChangeset) {
        z = GetFromMap(zCol.GetName(), columnValues).GetValueDouble();
    } else {
        // At least one other coordinate is in changeset; fetch Z from DB.
        if (!TryFetchDoubleFromDb(z, conn, zCol, columnValues)) {
            LOG.warningv("Point3d property '%s': Z coordinate absent and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
    }

    return std::make_unique<ChangesetPoint3dValue>(MakePrimitiveColumnInfo(propertyMap), x, y, z);
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a scalar primitive property.
// Delegates Point2d to CreatePoint2d() and Point3d to CreatePoint3d().
// All other types are read directly from the changeset column values map.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreatePrimitive(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();

    if (prim->GetType() == PRIMITIVETYPE_Point2d)
        return CreatePoint2d(conn, propertyMap, columnValues);

    if (prim->GetType() == PRIMITIVETYPE_Point3d)
        return CreatePoint3d(conn, propertyMap, columnValues);

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    return std::make_unique<ChangesetPrimitiveValue>(
        MakePrimitiveColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        GetDateTimeInfo(propertyMap));
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
// All system property columns must be present in the changeset — no DB fallback is used.
// Virtual class-id columns must NOT be passed here; use CreateFixedId() instead.
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
// Creates a fixed-value IECSqlValue whose integer value is known statically or
// has been fetched from the live DB.  @p id may be any BeInt64Id-derived type.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateFixedId(
    ECDbCR conn, PropertyMap const& propertyMap, BeInt64Id id) {

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
// Creates a full ChangesetNavValue for a navigation property.
//
// Skip rules (no DB query fired):
//   • Neither id nor physical relClassId is in the changeset → return nullptr.
//
// DB-fallback rules (only when one side IS in the changeset):
//   • id absent, physical relClassId present → fetch id from DB.
//   • physical relClassId absent, id present → fetch relClassId from DB.
//   • virtual relClassId → always resolved from schema (no changeset / DB needed).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IECSqlValue> ChangesetFieldFactory::CreateNav(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues) {

    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    const auto& idPropMap = navMap.GetIdPropertyMap();
    const DbColumn& idCol = idPropMap.GetAs<SingleColumnDataPropertyMap>().GetColumn();
    const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();

    const bool hasIdInChangeset = IsInMap(idCol.GetName(), columnValues);
    const bool relClassIdIsVirtual = relClassIdMap.GetColumn().IsVirtual();
    const bool hasPhysicalRelClassIdInChangeset =
        !relClassIdIsVirtual && IsInMap(relClassIdMap.GetColumn().GetName(), columnValues);

    // Skip if neither physical component is in the changeset.
    if (!hasIdInChangeset && !hasPhysicalRelClassIdInChangeset)
        return nullptr;

    // --- Resolve id sub-component ---
    std::unique_ptr<IECSqlValue> idVal;
    if (hasIdInChangeset) {
        idVal = std::make_unique<ChangesetPrimitiveValue>(MakePrimitiveColumnInfo(idPropMap),
                                                         GetFromMap(idCol.GetName(), columnValues));
    } else {
        // Physical relClassId is in changeset; fetch id from DB.
        int64_t fetchedId;
        if (!TryFetchInt64FromDb(fetchedId, conn, idCol, columnValues)) {
            LOG.warningv("Nav property '%s': id absent from changeset and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
        idVal = std::make_unique<ChangesetFixedInt64Value>(MakePrimitiveColumnInfo(idPropMap),
                                                           BeInt64Id(static_cast<uint64_t>(fetchedId)));
    }

    // --- Resolve relClassId sub-component ---
    std::unique_ptr<IECSqlValue> relClassIdVal;
    if (relClassIdIsVirtual) {
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        relClassIdVal = CreateFixedId(conn, relClassIdMap, navProp->GetRelationshipClass()->GetId());
    } else if (hasPhysicalRelClassIdInChangeset) {
        relClassIdVal = std::make_unique<ChangesetPrimitiveValue>(
            MakePrimitiveColumnInfo(relClassIdMap),
            GetFromMap(relClassIdMap.GetColumn().GetName(), columnValues));
    } else {
        // Id is in changeset; fetch relClassId from DB.
        int64_t fetchedRelClassId;
        if (!TryFetchInt64FromDb(fetchedRelClassId, conn, relClassIdMap.GetColumn(), columnValues)) {
            LOG.warningv("Nav property '%s': relClassId absent from changeset and could not be fetched from DB.",
                         propertyMap.GetProperty().GetName().c_str());
            return nullptr;
        }
        relClassIdVal = std::make_unique<ChangesetFixedInt64Value>(
            MakePrimitiveColumnInfo(relClassIdMap),
            ECN::ECClassId(static_cast<uint64_t>(fetchedRelClassId)));
    }

    return std::make_unique<ChangesetNavValue>(MakeNavColumnInfo(propertyMap),
                                               std::move(idVal), std::move(relClassIdVal));
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
//   1. Checking that the ECClassId column is physical (not virtual).
//   2. Extracting the first primary-key column value from the changeset.
//   3. Firing a direct SQL query to read the ECClassId from the live DB row.
// This path is taken when the class-id column is absent from the changeset (e.g. entity
// tables where the ECClassId is a physical column but was not changed in this row).
// Returns false immediately when the ECClassId column is virtual (no physical row to query).
// Returns true when a valid class and ClassMap are resolved; populates @p classMapOut
// and @p classIdOut.  Returns false silently on any other failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryResolveClassMapFromDbSeek(
    DbTable const& dbTable,
    ColumnValueMap const& columnValues,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    // A virtual class-id column has no physical storage — nothing to read from the DB.
    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();
    if (classIdCol.IsVirtual())
        return false;

    // Find the first primary-key column and read its value from the changeset.
    PrimaryKeyDbConstraint const* pkConstraint = dbTable.GetPrimaryKeyConstraint();
    if (pkConstraint == nullptr || pkConstraint->GetColumns().empty())
        return false;

    DbColumn const& firstPkCol = *pkConstraint->GetColumns().front();
    auto it = columnValues.find(firstPkCol.GetName());
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    // Fire a direct SQL query to read the ECClassId from the live DB.
    CachedStatementPtr stmt = conn.GetCachedStatement(
        Utf8PrintfString("SELECT [%s] FROM [%s] WHERE [%s]=?",
            classIdCol.GetName().c_str(), dbTable.GetName().c_str(),
            firstPkCol.GetName().c_str()).c_str());
    if (stmt == nullptr)
        return false;

    stmt->BindUInt64(1, it->second.GetValueUInt64());
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    ECClassId classId(static_cast<uint64_t>(stmt->GetValueInt64(0)));
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

        return CreateFixedId(conn, *propertyMap, resolvedClassId);
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
// For compound properties (Point2d/3d, Nav): changeset data gates inclusion;
// missing sub-columns are fetched from the live DB when at least one component is present.
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
    // Step 1: Resolve ClassMap — try changeset, then DB seek, fall back to root map.
    // -----------------------------------------------------------------------
    const ClassMap*  classMap          = nullptr;
    ECClassId        resolvedClassId;
    if (TryResolveClassMapFromChangeset(tbl, columnValues, conn, classMap, resolvedClassId)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " from changeset.",
                   tbl.GetName().c_str(), resolvedClassId.GetValueUnchecked());
    } else if (TryResolveClassMapFromDbSeek(tbl, columnValues, conn, classMap, resolvedClassId)) {
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
    // Step 2: Resolve ECInstanceId (slot [0]).
    // -----------------------------------------------------------------------
    ECInstanceId instanceId;
    std::unique_ptr<IECSqlValue>     instanceIdField;
    if (!ResolveInstanceId(*classMap, columnValues, conn, tbl, instanceId, instanceIdField))
        return BE_SQLITE_ERROR;

    // -----------------------------------------------------------------------
    // Step 3: Build ECClassId field (slot [1]).
    // -----------------------------------------------------------------------
    std::unique_ptr<IECSqlValue> classIdField = ResolveClassIdField(*classMap, resolvedClassId, conn, tbl);
    if (classIdField == nullptr)
        return BE_SQLITE_ERROR;

    // -----------------------------------------------------------------------
    // Step 4: Build remaining property fields (slots [2+]).
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
