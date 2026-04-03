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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
const ClassMap* ChangesetValueFactory::GetRootClassMap(DbTable const& tbl, ECDbCR conn) {
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
ECSqlPropertyPath ChangesetValueFactory::GetPropertyPath(PropertyMap const& propertyMap) {
    ECSqlPropertyPath path;
    for (auto& part : propertyMap.GetPath())
        path.AddEntry(part->GetProperty());
    return path;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info ChangesetValueFactory::GetDateTimeInfo(PropertyMap const& propertyMap) {
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
bool ChangesetValueFactory::IsInMap(Utf8StringCR colName, ColumnValueMap const& columnValues) {
    auto it = columnValues.find(colName);
    return it != columnValues.end() && it->second.IsValid();
}

//---------------------------------------------------------------------------------------
// Reads @p colName from @p columnValues.  Must only be called after IsInMap() has
// returned true — asserts in debug if the key is absent.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbValue ChangesetValueFactory::GetFromMap(Utf8StringCR colName, ColumnValueMap const& columnValues) {
    auto it = columnValues.find(colName);
    BeAssert(it != columnValues.end() && "GetFromMap called but column absent from map");
    return it != columnValues.end() ? it->second : DbValue(nullptr);
}

//---------------------------------------------------------------------------------------
// Returns the double value stored in @p val, or 0.0 if @p val is null.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValueFactory::CheckNullAndGetDoubleValueFromDbValue(DbValue const& val) {
    if(!val.IsValid() || val.IsNull())
        return std::numeric_limits<double>::quiet_NaN();
    return val.GetValueDouble();
}

//---------------------------------------------------------------------------------------
// Returns the uint64_t value stored in @p val, or 0 if @p val is null.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeInt64Id ChangesetValueFactory::CheckNullAndGetBeInt64IdValueFromDbValue(DbValue const& val) {
    if(!val.IsValid() || val.IsNull())
        return BeInt64Id(0);
    return BeInt64Id(val.GetValueUInt64());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CachedStatementPtr ChangesetValueFactory::PreparePkStatement(ECDbCR conn, DbTable const& tbl,
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryFetchDoubleFromDb(double& outVal, ECDbCR conn,
                                                 DbColumn const& col,
                                                 ColumnValueMap const& columnValues) {
    CachedStatementPtr stmt = PreparePkStatement(conn, col.GetTable(), col.GetName(), columnValues);
    if (stmt == nullptr)
        return false;
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;
    outVal = stmt->IsColumnNull(0) ? std::numeric_limits<double>::quiet_NaN() : stmt->GetValueDouble(0);
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryFetchBeInt64IdFromDb(BeInt64Id& outVal, ECDbCR conn,
                                                DbColumn const& col,
                                                ColumnValueMap const& columnValues) {
    CachedStatementPtr stmt = PreparePkStatement(conn, col.GetTable(), col.GetName(), columnValues);
    if (stmt == nullptr)
        return false;
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;
    outVal = stmt->IsColumnNull(0) ? BeInt64Id(0) : BeInt64Id(stmt->GetValueUInt64(0));
    return true;
}

//=============================================================================
// ColumnInfo factory helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo ChangesetValueFactory::MakePrimitiveColumnInfo(PropertyMap const& propertyMap) {
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
ECSqlColumnInfo ChangesetValueFactory::MakeStructColumnInfo(PropertyMap const& propertyMap) {
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
ECSqlColumnInfo ChangesetValueFactory::MakeNavColumnInfo(PropertyMap const& propertyMap) {
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
ECSqlColumnInfo ChangesetValueFactory::MakeArrayColumnInfo(PropertyMap const& propertyMap) {
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreatePoint2d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

    const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
    const DbColumn& xCol = pt2dMap.GetX().GetColumn();
    const DbColumn& yCol = pt2dMap.GetY().GetColumn();

    const bool xInChangeset = IsInMap(xCol.GetName(), columnValues);
    const bool yInChangeset = IsInMap(yCol.GetName(), columnValues);

    if (!xInChangeset && !yInChangeset)
        return BE_SQLITE_OK; // nothing in changeset — skip

    double x = 0.0, y = 0.0;

    if (xInChangeset) {
        x = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(xCol.GetName(), columnValues));
    } else {
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.errorv("Point2d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (yInChangeset) {
        y = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(yCol.GetName(), columnValues));
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
        changedProps.emplace_back(propName);
    } else {
        if (xInChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt2dMap.GetX().GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s)); }
        if (yInChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt2dMap.GetY().GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s)); }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreatePoint3d(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

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
        x = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(xCol.GetName(), columnValues));
    } else {
        if (!TryFetchDoubleFromDb(x, conn, xCol, columnValues)) {
            LOG.errorv("Point3d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (yInChangeset) {
        y = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(yCol.GetName(), columnValues));
    } else {
        if (!TryFetchDoubleFromDb(y, conn, yCol, columnValues)) {
            LOG.errorv("Point3d property '%s': Y coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    if (zInChangeset) {
        z = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(zCol.GetName(), columnValues));
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
        changedProps.emplace_back(propName);
    } else {
        if (xInChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetX().GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s)); }
        if (yInChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetY().GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s)); }
        if (zInChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetZ().GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s)); }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreatePrimitive(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

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
    changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateSystem(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

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
    changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateFixedId(
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateNav(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

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
        if(CreateFixedId(conn, idPropMap, CheckNullAndGetBeInt64IdValueFromDbValue(GetFromMap(idCol.GetName(), columnValues)), idVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed id value from changeset data.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else {
        BeInt64Id fetchedId;
        if (!TryFetchBeInt64IdFromDb(fetchedId, conn, idCol, columnValues)) {
            LOG.errorv("Nav property '%s': id absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
        if(CreateFixedId(conn, idPropMap, BeInt64Id(fetchedId), idVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed id value from data fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    // --- Resolve relClassId sub-component ---
    std::unique_ptr<IECSqlValue> relClassIdVal;
    if (relClassIdIsVirtual) {
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        LOG.infov("Nav property '%s': relClassId is virtual; using default value based on relationship class '%s'.",
                   propertyMap.GetProperty().GetName().c_str(), navProp->GetRelationshipClass()->GetFullName());
        if(CreateFixedId(conn, relClassIdMap, navProp->GetRelationshipClass()->GetId(), relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from virtual column.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else if (hasPhysicalRelClassIdInChangeset) {
        if(CreateFixedId(conn, relClassIdMap, CheckNullAndGetBeInt64IdValueFromDbValue(GetFromMap(relClassIdMap.GetColumn().GetName(), columnValues)), relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from changeset data.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    } else {
        BeInt64Id fetchedRelClassId;
        if (!TryFetchBeInt64IdFromDb(fetchedRelClassId, conn, relClassIdMap.GetColumn(), columnValues)) {
            LOG.errorv("Nav property '%s': relClassId absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
        if(CreateFixedId(conn, relClassIdMap, fetchedRelClassId, relClassIdVal) != BE_SQLITE_OK) {
            LOG.errorv("Nav property '%s': failed to create fixed relClassId value from data fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return BE_SQLITE_ERROR;
        }
    }

    fieldsOut.emplace_back(std::make_unique<ChangesetNavValue>(MakeNavColumnInfo(propertyMap),
                                                               std::move(idVal), std::move(relClassIdVal)));
    const Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (hasIdInChangeset &&  hasPhysicalRelClassIdInChangeset) {
        changedProps.emplace_back(propName);
    } else if (hasIdInChangeset) {
        Utf8String s; s.Sprintf("%s.%s", propName.c_str(), idPropMap.GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s));
    } else if (hasPhysicalRelClassIdInChangeset) {
        Utf8String s; s.Sprintf("%s.%s", propName.c_str(), relClassIdMap.GetProperty().GetName().c_str()); changedProps.emplace_back(std::move(s));
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateArray(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    if (!IsInMap(primMap.GetColumn().GetName(), columnValues))
        return BE_SQLITE_OK; // not in changeset — skip

    fieldsOut.emplace_back(std::make_unique<ChangesetArrayValue>(
        MakeArrayColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), columnValues),
        conn));
    changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateStruct(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

    auto structVal = std::make_unique<ChangesetStructValue>(MakeStructColumnInfo(propertyMap));
    const Utf8StringCR structName = propertyMap.GetProperty().GetName();
    bool anyMember = false;
    for (auto& memberMap : propertyMap.GetAs<StructPropertyMap>()) {
        std::vector<std::unique_ptr<IECSqlValue>> memberTemp;
        std::vector<Utf8String> memberChangedProps;
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
            changedProps.emplace_back(std::move(path));
        }
    }

    if (!anyMember)
        return BE_SQLITE_OK;

    fieldsOut.emplace_back(std::move(structVal));
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::CreateValueForProperty(
    ECDbCR conn, PropertyMap const& propertyMap, ColumnValueMap const& columnValues,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut, std::vector<Utf8String>& changedProps) {

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

    BeAssert(false && "Unknown property type in ChangesetValueFactory::CreateValueForProperty");
    return BE_SQLITE_ERROR;
}

//=============================================================================
// ChangesetValueFactory — high-level resolution helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassMapFromChangeset(
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassMapFromDbSeek(
    DbTable const& dbTable,
    ColumnValueMap const& columnValues,
    ECDbCR conn, const ClassMap*& classMapOut, ECClassId& classIdOut) {

    // A virtual class-id column has no physical storage — nothing to read from the DB.
    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();
    if (classIdCol.IsVirtual())
        return false;

    // Validate all PK columns, build and bind the statement via shared helper.
    ECClassId classId;
    if(!TryFetchBeInt64IdFromDb(classId, conn, classIdCol, columnValues))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::ResolveInstanceId(
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::ResolveClassIdField(
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::BuildPropertyFields(
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
    ECDbCR conn,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
    std::vector<Utf8String>& changedProps) {

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::isChildClassOfBisCore(ECClassId classId, ECDbCR conn) {
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetValueFactory::Create(
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues,
    std::vector<std::unique_ptr<IECSqlValue>>& fields, ECChangesetReader::Mode mode, std::vector<Utf8String>& changedProps) {

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
    changedProps.emplace_back(ECDBSYS_PROP_ECInstanceId);
    if (classIdFromChangeset)
        changedProps.emplace_back(ECDBSYS_PROP_ECClassId);

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

    if(mode == ECChangesetReader::Mode::Bis_Element_Properties && isChildClassOfBisCore(resolvedClassId, conn) && !tbl.GetName().EqualsIAscii("bis_Element"))
        return BE_SQLITE_OK; // caller only needs bis_element properties — skip rest   

    status = BuildPropertyFields(*classMap, columnValues, conn, tbl, fields, changedProps);

    if (status != BE_SQLITE_OK) {
        fields.clear();
        return status;
    }

    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
