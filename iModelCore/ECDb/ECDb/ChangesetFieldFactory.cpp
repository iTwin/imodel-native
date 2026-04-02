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
// Validates all PK columns, builds a parameterised SELECT statement for @p selectColName,
// and binds all PK values so the caller only needs to call Step().
// Returns nullptr on any failure (missing/null PK value, prepare error, etc.).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CachedStatementPtr ChangesetFieldFactory::PreparePkStatement(ECDbCR conn, DbTable const& tbl,
                                                              Utf8StringCR selectColName,
                                                              ColumnValueMap const& columnValues) {
    PrimaryKeyDbConstraint const* pk = tbl.GetPrimaryKeyConstraint();
    if (pk == nullptr || pk->GetColumns().empty())
        return nullptr;

    // Validate every PK column is present, valid, and non-null.
    for (DbColumn const* pkCol : pk->GetColumns()) {
        auto it = columnValues.find(pkCol->GetName());
        if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
            return nullptr;
    }

    // Build WHERE clause: [pk1]=? AND [pk2]=? ...
    Utf8String whereClause;
    for (DbColumn const* pkCol : pk->GetColumns()) {
        if (!whereClause.empty())
            whereClause.append(" AND ");
        whereClause.append("[");
        whereClause.append(pkCol->GetName());
        whereClause.append("]=?");
    }

    CachedStatementPtr stmt = conn.GetCachedStatement(
        Utf8PrintfString("SELECT [%s] FROM [%s] WHERE %s",
            selectColName.c_str(), tbl.GetName().c_str(), whereClause.c_str()).c_str());
    if (stmt == nullptr)
        return nullptr;

    int bindIdx = 1;
    for (DbColumn const* pkCol : pk->GetColumns())
        stmt->BindUInt64(bindIdx++, columnValues.find(pkCol->GetName())->second.GetValueUInt64());

    return stmt;
}

//---------------------------------------------------------------------------------------
// Fetches a single real column value from the live DB for the row identified by
// all primary-key columns (read from @p columnValues).
// Returns false immediately if any PK column is absent, invalid, or null.
// Returns true and sets @p outVal on success; returns false on any other failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryFetchDoubleFromDb(double& outVal, ECDbCR conn,
                                                 DbColumn const& col,
                                                 ColumnValueMap const& columnValues) {
    CachedStatementPtr stmt = PreparePkStatement(conn, col.GetTable(), col.GetName(), columnValues);
    if (stmt == nullptr)
        return false;
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;
    outVal = stmt->GetValueDouble(0);
    return true;
}

//---------------------------------------------------------------------------------------
// Fetches a single integer column value from the live DB for the row identified by
// all primary-key columns (read from @p columnValues).
// Returns false immediately if any PK column is absent, invalid, or null.
// Returns true and sets @p outVal on success; returns false on any other failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::TryFetchInt64FromDb(int64_t& outVal, ECDbCR conn,
                                                DbColumn const& col,
                                                ColumnValueMap const& columnValues) {
    CachedStatementPtr stmt = PreparePkStatement(conn, col.GetTable(), col.GetName(), columnValues);
    if (stmt == nullptr)
        return false;
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;
    outVal = stmt->GetValueInt64(0);
    return true;
}

//=============================================================================
// ColumnInfo factory helpers
//=============================================================================

//---------------------------------------------------------------------------------------
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
//
// Convention: every Create* returns BE_SQLITE_OK and populates @p out on success.
//   BE_SQLITE_OK with out == nullptr means "no changeset data for this property — skip it".
//   Any other DbResult is a hard error; the caller must propagate it immediately.
//=============================================================================

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a Point2d property.
// Returns BE_SQLITE_OK with out==nullptr when neither coordinate is in the changeset.
// Returns BE_SQLITE_ERROR when one coordinate is present but the other cannot be fetched
// from the live DB.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreatePoint2d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
    const DbColumn& xCol = pt2dMap.GetX().GetColumn();
    const DbColumn& yCol = pt2dMap.GetY().GetColumn();

    const bool xInChangeset = IsInMap(xCol.GetName(), columnValues);
    const bool yInChangeset = IsInMap(yCol.GetName(), columnValues);

    if (!xInChangeset && !yInChangeset)
        return BE_SQLITE_OK; // nothing in changeset — skip

    double x = 0.0, y = 0.0;

    if (xInChangeset) {
        x = GetFromMap(xCol.GetName(), columnValues).GetValueDouble();
    } else {
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.errorv("Point2d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (yInChangeset) {
        y = GetFromMap(yCol.GetName(), columnValues).GetValueDouble();
    } else {
        if (!TryFetchDoubleFromDb(y, conn, yCol, columnValues)) {
            LOG.errorv("Point2d property '%s': Y coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    fieldsOut.emplace_back(std::make_unique<ChangesetPoint2dValue>(MakePrimitiveColumnInfo(propertyMap), x, y));
    const Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (xInChangeset && yInChangeset) {
        changedProps.insert(propName);
    } else {
        if (xInChangeset) { Utf8String s; s.Sprintf("%s.X", propName.c_str()); changedProps.insert(s); }
        if (yInChangeset) { Utf8String s; s.Sprintf("%s.Y", propName.c_str()); changedProps.insert(s); }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a Point3d property.
// Returns BE_SQLITE_OK with out==nullptr when no coordinate is in the changeset.
// Returns BE_SQLITE_ERROR when at least one coordinate is present but another cannot be
// fetched from the live DB.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreatePoint3d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
    const DbColumn& xCol = pt3dMap.GetX().GetColumn();
    const DbColumn& yCol = pt3dMap.GetY().GetColumn();
    const DbColumn& zCol = pt3dMap.GetZ().GetColumn();

    const bool xInChangeset = IsInMap(xCol.GetName(), columnValues);
    const bool yInChangeset = IsInMap(yCol.GetName(), columnValues);
    const bool zInChangeset = IsInMap(zCol.GetName(), columnValues);

    if (!xInChangeset && !yInChangeset && !zInChangeset)
        return BE_SQLITE_OK; // nothing in changeset — skip

    double x = 0.0, y = 0.0, z = 0.0;

    if (xInChangeset) {
        x = GetFromMap(xCol.GetName(), columnValues).GetValueDouble();
    } else {
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.errorv("Point3d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (yInChangeset) {
        y = GetFromMap(yCol.GetName(), columnValues).GetValueDouble();
    } else {
        if (!TryFetchDoubleFromDb(y, conn, yCol, columnValues)) {
            LOG.errorv("Point3d property '%s': Y coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (zInChangeset) {
        z = GetFromMap(zCol.GetName(), columnValues).GetValueDouble();
    } else {
        if (!TryFetchDoubleFromDb(z, conn, zCol, columnValues)) {
            LOG.errorv("Point3d property '%s': Z coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    fieldsOut.emplace_back(std::make_unique<ChangesetPoint3dValue>(MakePrimitiveColumnInfo(propertyMap), x, y, z));
    const Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (xInChangeset && yInChangeset && zInChangeset) {
        changedProps.insert(propName);
    } else {
        if (xInChangeset) { Utf8String s; s.Sprintf("%s.X", propName.c_str()); changedProps.insert(s); }
        if (yInChangeset) { Utf8String s; s.Sprintf("%s.Y", propName.c_str()); changedProps.insert(s); }
        if (zInChangeset) { Utf8String s; s.Sprintf("%s.Z", propName.c_str()); changedProps.insert(s); }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a scalar primitive property.
// Returns BE_SQLITE_OK with out==nullptr when the column is absent from the changeset.
// Delegates Point2d/3d to CreatePoint2d/3d().
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreatePrimitive(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();

    if (prim->GetType() == PRIMITIVETYPE_Point2d)
        return CreatePoint2d(conn, propertyMap, columnValues, fieldsOut, changedProps);

    if (prim->GetType() == PRIMITIVETYPE_Point3d)
        return CreatePoint3d(conn, propertyMap, columnValues, fieldsOut, changedProps);

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    if (!IsInMap(primMap.GetColumn().GetName(), columnValues))
        return BE_SQLITE_OK; // not in changeset — skip

    fieldsOut.emplace_back(std::make_unique<ChangesetPrimitiveValue>(
        MakePrimitiveColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        GetDateTimeInfo(propertyMap)));
    changedProps.insert(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a system property (ECInstanceId, ECClassId, etc.).
// Returns BE_SQLITE_OK with out==nullptr when the column is absent from the changeset or virtual
// with no fixed default.
// For ClassId / SourceClassId / TargetClassId properties with a virtual column and a known
// default class id (single-class tables / link tables), emits a fixed value matching
// InstanceReader behaviour.
// Returns BE_SQLITE_ERROR on internal mapping failures.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateSystem(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

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
    const auto* dataMap = sysMap.FindDataPropertyMap(dbTable);
    if (dataMap == nullptr) {
        LOG.infov("No data property map found for system property '%s' in table '%s'.",
                   propertyMap.GetProperty().GetName().c_str(), dbTable.GetName().c_str());
        return BE_SQLITE_OK;
    }

    if (!IsInMap(dataMap->GetColumn().GetName(), columnValues))
        return BE_SQLITE_OK; // not in changeset — skip

    fieldsOut.emplace_back(std::make_unique<ChangesetPrimitiveValue>(columnInfo, GetFromMap(dataMap->GetColumn().GetName(), columnValues)));
    changedProps.insert(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates a fixed-value IECSqlValue whose integer value is statically known.
// Always succeeds — never skipped and never consults the changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateFixedId(
    ECDbCR conn, PropertyMap const& propertyMap, BeInt64Id id,
    std::unique_ptr<IECSqlValue>& out) {

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

    out = std::make_unique<ChangesetFixedInt64Value>(columnInfo, id);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates a full ChangesetNavValue for a navigation property.
// Returns BE_SQLITE_OK with out==nullptr when neither physical component is in the changeset.
// Returns BE_SQLITE_ERROR when a component is partially present but the DB fetch fails.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateNav(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    const auto& idPropMap = navMap.GetIdPropertyMap();
    const DbColumn& idCol = idPropMap.GetAs<SingleColumnDataPropertyMap>().GetColumn();
    const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();

    const bool hasIdInChangeset = IsInMap(idCol.GetName(), columnValues);
    const bool relClassIdIsVirtual = relClassIdMap.GetColumn().IsVirtual();
    const bool hasPhysicalRelClassIdInChangeset =
        !relClassIdIsVirtual && IsInMap(relClassIdMap.GetColumn().GetName(), columnValues);

    if (!hasIdInChangeset && !hasPhysicalRelClassIdInChangeset)
        return BE_SQLITE_OK; // nothing in changeset — skip

    // --- Resolve id sub-component ---
    std::unique_ptr<IECSqlValue> idVal;
    if (hasIdInChangeset) {
        if(CreateFixedId(conn, idPropMap, BeInt64Id(GetFromMap(idCol.GetName(), columnValues).GetValueUInt64()), idVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed id value from changeset data.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else {
        int64_t fetchedId;
        if (!TryFetchInt64FromDb(fetchedId, conn, idCol, columnValues)) {
            LOG.errorv("Nav property '%s': id absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
        if(CreateFixedId(conn, idPropMap, BeInt64Id(static_cast<uint64_t>(fetchedId)), idVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed id value from data fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    // --- Resolve relClassId sub-component ---
    std::unique_ptr<IECSqlValue> relClassIdVal;
    if (relClassIdIsVirtual) {
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        if(CreateFixedId(conn, relClassIdMap, navProp->GetRelationshipClass()->GetId(), relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from virtual column.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else if (hasPhysicalRelClassIdInChangeset) {
        if(CreateFixedId(conn, relClassIdMap, BeInt64Id(GetFromMap(relClassIdMap.GetColumn().GetName(), columnValues).GetValueUInt64()), relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from changeset data.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else {
        int64_t fetchedRelClassId;
        if (!TryFetchInt64FromDb(fetchedRelClassId, conn, relClassIdMap.GetColumn(), columnValues)) {
            LOG.errorv("Nav property '%s': relClassId absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
        if(CreateFixedId(conn, relClassIdMap, ECN::ECClassId(static_cast<uint64_t>(fetchedRelClassId)), relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from data fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    fieldsOut.emplace_back(std::make_unique<ChangesetNavValue>(MakeNavColumnInfo(propertyMap),
                                                               std::move(idVal), std::move(relClassIdVal)));
    const Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (hasIdInChangeset &&  hasPhysicalRelClassIdInChangeset) {
        changedProps.insert(propName);
    } else if (hasIdInChangeset) {
        Utf8String s; s.Sprintf("%s.Id", propName.c_str()); changedProps.insert(s);
    } else if (hasPhysicalRelClassIdInChangeset) {
        Utf8String s; s.Sprintf("%s.RelECClassId", propName.c_str()); changedProps.insert(s);
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for an array property.
// Returns BE_SQLITE_OK with out==nullptr when the column is absent from the changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateArray(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    if (!IsInMap(primMap.GetColumn().GetName(), columnValues))
        return BE_SQLITE_OK; // not in changeset — skip

    fieldsOut.emplace_back(std::make_unique<ChangesetArrayValue>(
        MakeArrayColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        conn));
    changedProps.insert(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Creates an IECSqlValue for a struct property.
// Only members whose backing column(s) are in the changeset are included.
// Returns BE_SQLITE_OK with out==nullptr when no member has changeset data.
// Returns BE_SQLITE_ERROR if any member fails to be created.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateStruct(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    auto structVal = std::make_unique<ChangesetStructValue>(MakeStructColumnInfo(propertyMap));
    const Utf8StringCR structName = propertyMap.GetProperty().GetName();
    bool anyMember = false;
    for (auto& memberMap : propertyMap.GetAs<StructPropertyMap>()) {
        std::vector<std::unique_ptr<IECSqlValue>> memberTemp;
        std::unordered_set<Utf8String> memberChangedProps;
        DbResult status = CreateValueForProperty(conn, *memberMap, columnValues, dbTable, memberTemp, memberChangedProps);
        if (status != BE_SQLITE_OK)
            return status;
        if (memberTemp.empty())
            continue; // not in changeset
        anyMember = true;
        structVal->AppendMember(memberMap->GetProperty().GetName(), std::move(memberTemp.front()));
        for (auto& name : memberChangedProps) {
            Utf8String path;
            path.Sprintf("%s.%s", structName.c_str(), name.c_str());
            changedProps.insert(std::move(path));
        }
    }

    if (!anyMember)
        return BE_SQLITE_OK;

    fieldsOut.emplace_back(std::move(structVal));
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Single-pass dispatch: checks changeset presence and constructs the value in one step.
// Returns BE_SQLITE_OK with out==nullptr when the property has no changeset data.
// Returns BE_SQLITE_ERROR on a hard failure (e.g. live DB fetch fails).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::CreateValueForProperty(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::unordered_set<Utf8String>& changedProps) {

    const auto& prop = propertyMap.GetProperty();

    if (propertyMap.IsSystem())
        return CreateSystem(conn, propertyMap, columnValues, dbTable, fieldsOut, changedProps);
    if (prop.GetIsPrimitive())
        return CreatePrimitive(conn, propertyMap, columnValues, fieldsOut, changedProps);
    if (prop.GetIsNavigation())
        return CreateNav(conn, propertyMap, columnValues, fieldsOut, changedProps);
    if (prop.GetIsStruct())
        return CreateStruct(conn, propertyMap, columnValues, dbTable, fieldsOut, changedProps);
    if (prop.GetIsArray())
        return CreateArray(conn, propertyMap, columnValues, fieldsOut, changedProps);

    BeAssert(false && "Unknown property type in ChangesetFieldFactory::CreateValueForProperty");
    return BE_SQLITE_ERROR;
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
//   2. Extracting all primary-key column values from the changeset (fails immediately if any is absent/invalid/null).
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

    // Validate all PK columns, build and bind the statement via shared helper.
    int64_t fetchedClassId = 0;
    if(!TryFetchInt64FromDb(fetchedClassId, conn, classIdCol, columnValues))
        return false;

    ECClassId classId(static_cast<uint64_t>(fetchedClassId));
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
// Finds the ECInstanceId property in @p classMap, reads its value from the changeset,
// validates it, and emits the IECSqlValue field.
// Returns BE_SQLITE_OK and populates @p instanceIdOut / @p fieldOut on success.
// Returns BE_SQLITE_ERROR on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::ResolveInstanceId(
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

        const auto& sysMap = propertyMap->GetAs<SystemPropertyMap>();
        const auto* dataMap = sysMap.FindDataPropertyMap(primaryDbTable);
        BeAssert(dataMap != nullptr && "ECInstanceId SystemPropertyMap has no data property map for the given table");
        if (dataMap == nullptr) {
            LOG.errorv("No ECInstanceId data property map found for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return BE_SQLITE_ERROR;
        }
        Utf8StringCR colName = dataMap->GetColumn().GetName();
        auto it = columnValues.find(colName);
        if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull()) {
            LOG.errorv("ECInstanceId is absent or null in changeset for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return BE_SQLITE_ERROR;
        }

        ECInstanceId instanceId(it->second.GetValueUInt64());
        if (!instanceId.IsValid()) {
            LOG.errorv("ECInstanceId resolved to an invalid (zero) id for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return BE_SQLITE_ERROR;
        }

        std::unique_ptr<IECSqlValue> sysVal;
        DbResult status = CreateFixedId(conn, *propertyMap, instanceId, sysVal);
        if (status != BE_SQLITE_OK)
            return status;
        if (sysVal == nullptr) {
            LOG.errorv("ECInstanceId field could not be created for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return BE_SQLITE_ERROR;
        }

        instanceIdOut = instanceId;
        fieldOut      = std::move(sysVal);
        LOG.debugv("Table '%s': resolved ECInstanceId %" PRIu64 " from changeset.",
                   primaryDbTable.GetName().c_str(), instanceId.GetValueUnchecked());
        return BE_SQLITE_OK;
    }

    LOG.errorv("ECInstanceId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// Creates the ECClassId field as a fixed value from the already-resolved @p resolvedClassId.
// Returns BE_SQLITE_OK and populates @p out on success.
// Returns BE_SQLITE_ERROR when the ECClassId property is not found in the ClassMap.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::ResolveClassIdField(
    ClassMap const& classMap,
    ECClassId resolvedClassId,
    ECDbCR conn, DbTable const& primaryDbTable,
    std::unique_ptr<IECSqlValue>& out) {

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

        return CreateFixedId(conn, *propertyMap, resolvedClassId, out);
    }

    LOG.errorv("ECClassId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return BE_SQLITE_ERROR;
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
// Returns BE_SQLITE_OK on success; BE_SQLITE_ERROR immediately on any failure.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetFieldFactory::BuildPropertyFields(
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
    ECDbCR conn,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
    std::unordered_set<Utf8String>& changedProps) {

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
        DbResult status = CreateValueForProperty(conn, *propertyMap, columnValues, dbTable, fieldsOut, changedProps);
        if (status != BE_SQLITE_OK)
            return status;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Returns true when @p classId identifies BisCore::Element or any class derived from it.
// Returns false when the class cannot be resolved or is not in the BisCore.Element hierarchy.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFieldFactory::isChildClassOfBisCore(ECClassId classId, ECDbCR conn) {
    const ECClass* cls = conn.Schemas().Main().GetClass(classId);
    if (cls == nullptr)
        return false;

    const ECClass* bisElementClass = conn.Schemas().Main().GetClass("BisCore", "Element");
    if (bisElementClass == nullptr)
        return false;

    // If it's exactly BisCore.Element, return false because we just want children of biscore element class not the biscore elemnt class itself
    if(ECClass::ClassesAreEqualByName(cls, bisElementClass)) 
        return false;

    return cls->Is(bisElementClass);
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
    std::vector<std::unique_ptr<IECSqlValue>>& fields, ECChangesetReader::Mode mode,
    bool includeInstanceId, std::unordered_set<Utf8String>& changedProps) {

    // -----------------------------------------------------------------------
    // Step 1: Resolve ClassMap — try changeset, then DB seek, fall back to root map.
    // -----------------------------------------------------------------------
    const ClassMap*  classMap          = nullptr;
    ECClassId        resolvedClassId;
    bool             classIdFromChangeset = false;
    if (TryResolveClassMapFromChangeset(tbl, columnValues, conn, classMap, resolvedClassId)) {
        classIdFromChangeset = true;
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

    // ECInstanceId and ECClassId name collection — handled here since they are
    // emitted as fixed slots and skipped by the BuildPropertyFields loop.
    if (includeInstanceId)
        changedProps.insert(ECDBSYS_PROP_ECInstanceId);
    if (classIdFromChangeset)
        changedProps.insert(ECDBSYS_PROP_ECClassId);

    // -----------------------------------------------------------------------
    // Step 2: Resolve ECInstanceId (slot [0]).
    // -----------------------------------------------------------------------
    ECInstanceId instanceId;
    std::unique_ptr<IECSqlValue> instanceIdField;
    DbResult status = ResolveInstanceId(*classMap, columnValues, conn, tbl, instanceId, instanceIdField);
    if (status != BE_SQLITE_OK)
        return status;

    // -----------------------------------------------------------------------
    // Step 3: Build ECClassId field (slot [1]).
    // -----------------------------------------------------------------------
    std::unique_ptr<IECSqlValue> classIdField;
    status = ResolveClassIdField(*classMap, resolvedClassId, conn, tbl, classIdField);
    if (status != BE_SQLITE_OK)
        return status;

    // -----------------------------------------------------------------------
    // Step 4: Build remaining property fields (slots [2+]).
    // -----------------------------------------------------------------------
    fields.emplace_back(std::move(instanceIdField));
    fields.emplace_back(std::move(classIdField));

    if (mode == ECChangesetReader::Mode::Instance_Key)
        return BE_SQLITE_OK; // caller only needs the instance key — skip user properties

    if(mode == ECChangesetReader::Mode::Bis_Element_Properties && isChildClassOfBisCore(resolvedClassId, conn))
        return BE_SQLITE_OK; // caller only needs element properties — skip user properties)    

    status = BuildPropertyFields(*classMap, columnValues, conn, tbl, fields, changedProps);

    if (status != BE_SQLITE_OK) {
        fields.clear();
        return status;
    }

    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
