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
        DbTable const& parentTbl = tbl.GetLinkNode().GetParent()->GetTable();
        LOG.infov("Resolving root class for overflow table '%s' via parent table '%s'.", tbl.GetName().c_str(), parentTbl.GetName().c_str());       
        rootClassId = parentTbl.GetExclusiveRootECClassId();
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
// Returns the double value stored in @p val, or quiet_NaN if @p val is null.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValueFactory::CheckNullAndGetDoubleValueFromDbValue(DbValue const& val) {
    if(!val.IsValid() || val.IsNull())
        return std::numeric_limits<double>::quiet_NaN();
    return val.GetValueDouble();
}

//---------------------------------------------------------------------------------------
// Returns an Id with the uint64_t value stored in @p val, or an Id with 0 if @p val is null.
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
    
    ClearStatement(stmt); // ensure statement is reset and has no bindings before we bind parameters

    int bindIdx = 1;
    for (DbColumn const* pkCol : pk->GetColumns())
        stmt->BindUInt64(bindIdx++, columnValues.find(pkCol->GetName())->second.GetValueUInt64());

    return stmt;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetValueFactory::ClearStatement(CachedStatementPtr stmt) {
    if (stmt != nullptr) {
        stmt->Reset();
        stmt->ClearBindings();
    }
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
    DbResult rc = stmt->Step();
    if (rc != BE_SQLITE_ROW) {
        ClearStatement(stmt); // ensure statement is reset and has no bindings after stepping even on failure to prepare or step
        return false;
    }
    outVal = stmt->IsColumnNull(0) ? std::numeric_limits<double>::quiet_NaN() : stmt->GetValueDouble(0);
    ClearStatement(stmt); // ensure statement is reset and has no bindings after successful fetch

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
    DbResult rc = stmt->Step();
    if (rc != BE_SQLITE_ROW) {
        ClearStatement(stmt); // ensure statement is reset and has no bindings after stepping even on failure to prepare or step
        return false;
    }
    outVal = stmt->IsColumnNull(0) ? BeInt64Id(0) : BeInt64Id(stmt->GetValueUInt64(0));
    ClearStatement(stmt); // ensure statement is reset and has no bindings after successful fetch
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
// Convention: every Create* returns SUCCESS and populates @p out on success.
//   SUCCESS with an empty fieldsOut means "no changeset data for this property — skip it".
//   ERROR is a hard failure; the caller must propagate it immediately.
//   Exception: CreateFixedId always succeeds and returns void.
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreatePoint2d(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
    const DbColumn& xCol = pt2dMap.GetX().GetColumn();
    const DbColumn& yCol = pt2dMap.GetY().GetColumn();

    const bool xInCurrentTableAndChangeset = xCol.GetTable() == ctx.dbTable && IsInMap(xCol.GetName(), ctx.columnValues);
    const bool yInCurrentTableAndChangeset = yCol.GetTable() == ctx.dbTable && IsInMap(yCol.GetName(), ctx.columnValues);

    if (!xInCurrentTableAndChangeset && !yInCurrentTableAndChangeset) {
        LOG.infov("Point2d property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    double x = 0.0, y = 0.0;

    if (xInCurrentTableAndChangeset) {
        x = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(xCol.GetName(), ctx.columnValues));
    } else {
        if (!TryFetchDoubleFromDb(x, ctx.conn, xCol, ctx.columnValues)) {
            LOG.errorv("Point2d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
    }

    if (yInCurrentTableAndChangeset) {
        y = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(yCol.GetName(), ctx.columnValues));
    } else {
        if (!TryFetchDoubleFromDb(y, ctx.conn, yCol, ctx.columnValues)) {
            LOG.errorv("Point2d property '%s': Y coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetPoint2dValue>(MakePrimitiveColumnInfo(propertyMap), x, y));
    Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (xInCurrentTableAndChangeset && yInCurrentTableAndChangeset) {
        ctx.changedProps.emplace_back(propName);
    } else {
        if (xInCurrentTableAndChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt2dMap.GetX().GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s)); }
        if (yInCurrentTableAndChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt2dMap.GetY().GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s)); }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreatePoint3d(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
    const DbColumn& xCol = pt3dMap.GetX().GetColumn();
    const DbColumn& yCol = pt3dMap.GetY().GetColumn();
    const DbColumn& zCol = pt3dMap.GetZ().GetColumn();

    const bool xInCurrentTableAndChangeset = xCol.GetTable() == ctx.dbTable && IsInMap(xCol.GetName(), ctx.columnValues);
    const bool yInCurrentTableAndChangeset = yCol.GetTable() == ctx.dbTable && IsInMap(yCol.GetName(), ctx.columnValues);
    const bool zInCurrentTableAndChangeset = zCol.GetTable() == ctx.dbTable && IsInMap(zCol.GetName(), ctx.columnValues);

    if (!xInCurrentTableAndChangeset && !yInCurrentTableAndChangeset && !zInCurrentTableAndChangeset) {
        LOG.infov("Point3d property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    double x = 0.0, y = 0.0, z = 0.0;

    if (xInCurrentTableAndChangeset) {
        x = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(xCol.GetName(), ctx.columnValues));
    } else {
        if (!TryFetchDoubleFromDb(x, ctx.conn, xCol, ctx.columnValues)) {
            LOG.errorv("Point3d property '%s': X coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
    }

    if (yInCurrentTableAndChangeset) {
        y = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(yCol.GetName(), ctx.columnValues));
    } else {
        if (!TryFetchDoubleFromDb(y, ctx.conn, yCol, ctx.columnValues)) {
            LOG.errorv("Point3d property '%s': Y coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
    }

    if (zInCurrentTableAndChangeset) {
        z = CheckNullAndGetDoubleValueFromDbValue(GetFromMap(zCol.GetName(), ctx.columnValues));
    } else {
        if (!TryFetchDoubleFromDb(z, ctx.conn, zCol, ctx.columnValues)) {
            LOG.errorv("Point3d property '%s': Z coordinate absent and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetPoint3dValue>(MakePrimitiveColumnInfo(propertyMap), x, y, z));
    Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (xInCurrentTableAndChangeset && yInCurrentTableAndChangeset && zInCurrentTableAndChangeset) {
        ctx.changedProps.emplace_back(propName);
    } else {
        if (xInCurrentTableAndChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetX().GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s)); }
        if (yInCurrentTableAndChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetY().GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s)); }
        if (zInCurrentTableAndChangeset) { Utf8String s; s.Sprintf("%s.%s", propName.c_str(), pt3dMap.GetZ().GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s)); }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreatePrimitive(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();

    if (prim->GetType() == PRIMITIVETYPE_Point2d)
        return CreatePoint2d(propertyMap, ctx);

    if (prim->GetType() == PRIMITIVETYPE_Point3d)
        return CreatePoint3d(propertyMap, ctx);

    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    if(primMap.GetColumn().GetTable() != ctx.dbTable) {
        LOG.infov("Primitive property '%s': column '%s' belongs to a different table than the one being processed — skipping.",
                  propertyMap.GetProperty().GetName().c_str(), primMap.GetColumn().GetName().c_str());
        return SUCCESS;
    }
    if (!IsInMap(primMap.GetColumn().GetName(), ctx.columnValues)) {
        LOG.infov("Primitive property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetPrimitiveValue>(
        MakePrimitiveColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), ctx.columnValues),
        GetDateTimeInfo(propertyMap)));
    ctx.changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreateSystem(PropertyMap const& propertyMap, BuildCtx& ctx) {
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
    const auto* dataMap = sysMap.FindDataPropertyMap(ctx.dbTable);
    if (dataMap == nullptr) {
        LOG.infov("No data property map found for system property '%s' in table '%s'.",
                   propertyMap.GetProperty().GetName().c_str(), ctx.dbTable.GetName().c_str());
        return SUCCESS;
    }

    if (!IsInMap(dataMap->GetColumn().GetName(), ctx.columnValues)) {
        LOG.infov("System property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetPrimitiveValue>(columnInfo, GetFromMap(dataMap->GetColumn().GetName(), ctx.columnValues)));
    ctx.changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetValueFactory::CreateFixedId(
    PropertyMap const& propertyMap, BeInt64Id id, BuildCtx& ctx, ChangesetValue*& out) {

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

    out = ctx.alloc.New<ChangesetFixedInt64Value>(columnInfo, id);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreateNav(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    const auto& idPropMap = navMap.GetIdPropertyMap();
    const DbColumn& idCol = idPropMap.GetAs<SingleColumnDataPropertyMap>().GetColumn();
    const auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();

    const bool hasIdInCurrentTableAndChangeset = idCol.GetTable() == ctx.dbTable && IsInMap(idCol.GetName(), ctx.columnValues);
    const bool relClassIdIsVirtual = relClassIdMap.GetColumn().IsVirtual();
    const bool hasPhysicalRelClassIdInCurrentTableAndChangeset = relClassIdMap.GetColumn().GetTable() == ctx.dbTable && IsInMap(relClassIdMap.GetColumn().GetName(), ctx.columnValues);

    if (!hasIdInCurrentTableAndChangeset && !hasPhysicalRelClassIdInCurrentTableAndChangeset) {
        LOG.infov("Nav property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    // --- Resolve id sub-component ---
    ChangesetValue* idVal = nullptr;
    if (hasIdInCurrentTableAndChangeset) {
        CreateFixedId(idPropMap, CheckNullAndGetBeInt64IdValueFromDbValue(GetFromMap(idCol.GetName(), ctx.columnValues)), ctx, idVal);
    } else {
        BeInt64Id fetchedId;
        if (!TryFetchBeInt64IdFromDb(fetchedId, ctx.conn, idCol, ctx.columnValues)) {
            LOG.errorv("Nav property '%s': id absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
        CreateFixedId(idPropMap, BeInt64Id(fetchedId), ctx, idVal);
    }

    LOG.infov("Nav property '%s'-> virtual: %s", propertyMap.GetProperty().GetName().c_str(), relClassIdIsVirtual ? "true" : "false");
    // --- Resolve relClassId sub-component ---
    ChangesetValue* relClassIdVal = nullptr;
    if (hasPhysicalRelClassIdInCurrentTableAndChangeset) {
        CreateFixedId(relClassIdMap, CheckNullAndGetBeInt64IdValueFromDbValue(GetFromMap(relClassIdMap.GetColumn().GetName(), ctx.columnValues)), ctx, relClassIdVal);
    } else if (relClassIdIsVirtual) {
        const auto navProp = propertyMap.GetProperty().GetAsNavigationProperty();
        LOG.infov("Nav property '%s': relClassId is virtual; using default value based on relationship class '%s'.",
                   propertyMap.GetProperty().GetName().c_str(), navProp->GetRelationshipClass()->GetFullName());
        CreateFixedId(relClassIdMap, navProp->GetRelationshipClass()->GetId(), ctx, relClassIdVal);
    } else {
        BeInt64Id fetchedRelClassId;
        if (!TryFetchBeInt64IdFromDb(fetchedRelClassId, ctx.conn, relClassIdMap.GetColumn(), ctx.columnValues)) {
            LOG.errorv("Nav property '%s': relClassId absent from changeset and could not be fetched from DB.",
                       propertyMap.GetProperty().GetName().c_str());
            return ERROR;
        }
        CreateFixedId(relClassIdMap, fetchedRelClassId, ctx, relClassIdVal);
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetNavValue>(MakeNavColumnInfo(propertyMap), idVal, relClassIdVal));
    Utf8StringCR propName = propertyMap.GetProperty().GetName();
    if (hasIdInCurrentTableAndChangeset &&  hasPhysicalRelClassIdInCurrentTableAndChangeset) {
        ctx.changedProps.emplace_back(propName);
    } else if (hasIdInCurrentTableAndChangeset) {
        Utf8String s; s.Sprintf("%s.%s", propName.c_str(), idPropMap.GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s));
    } else if (hasPhysicalRelClassIdInCurrentTableAndChangeset) {
        Utf8String s; s.Sprintf("%s.%s", propName.c_str(), relClassIdMap.GetProperty().GetName().c_str()); ctx.changedProps.emplace_back(std::move(s));
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreateArray(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();

    if(primMap.GetColumn().GetTable() != ctx.dbTable) {
        LOG.infov("Primitive property '%s': column '%s' belongs to a different table than the one being processed — skipping.",
                  propertyMap.GetProperty().GetName().c_str(), primMap.GetColumn().GetName().c_str());
        return SUCCESS;
    }

    if (!IsInMap(primMap.GetColumn().GetName(), ctx.columnValues)) {
        LOG.infov("Array property '%s': no data in changeset — skipping.",
                  propertyMap.GetProperty().GetName().c_str());
        return SUCCESS;
    }

    ctx.fields.push_back(ctx.alloc.New<ChangesetArrayValue>(
        MakeArrayColumnInfo(propertyMap),
        GetFromMap(primMap.GetColumn().GetName(), ctx.columnValues),
        ctx.conn));
    ctx.changedProps.emplace_back(propertyMap.GetProperty().GetName());
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreateStruct(PropertyMap const& propertyMap, BuildCtx& ctx) {
    ChangesetStructValue* structVal = ctx.alloc.New<ChangesetStructValue>(MakeStructColumnInfo(propertyMap));
    Utf8StringCR structName = propertyMap.GetProperty().GetName();
    bool anyMember = false;
    for (auto& memberMap : propertyMap.GetAs<StructPropertyMap>()) {
        std::vector<ChangesetValue*> memberTemp;
        std::vector<Utf8String> memberChangedProps;
        BuildCtx memberCtx { ctx.conn, ctx.columnValues, ctx.dbTable, ctx.alloc, memberTemp, memberChangedProps };
        BentleyStatus status = CreateValueForProperty(*memberMap, memberCtx);
        if (status != SUCCESS)
            return status;
        if (memberTemp.empty())
            continue; // not in changeset
        anyMember = true;
        structVal->AppendMember(memberMap->GetProperty().GetName(), memberTemp.front());
        for (auto& name : memberChangedProps) {
            Utf8String path;
            path.Sprintf("%s.%s", structName.c_str(), name.c_str());
            ctx.changedProps.emplace_back(std::move(path));
        }
    }

    if (!anyMember)
        return SUCCESS;

    ctx.fields.push_back(structVal);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::CreateValueForProperty(PropertyMap const& propertyMap, BuildCtx& ctx) {
    const auto& prop = propertyMap.GetProperty();

    if (propertyMap.IsSystem())
        return CreateSystem(propertyMap, ctx);
    if (prop.GetIsPrimitive())
        return CreatePrimitive(propertyMap, ctx);
    if (prop.GetIsNavigation())
        return CreateNav(propertyMap, ctx);
    if (prop.GetIsStruct())
        return CreateStruct(propertyMap, ctx);
    if (prop.GetIsArray())
        return CreateArray(propertyMap, ctx);

    BeAssert(false && "Unknown property type in ChangesetValueFactory::CreateValueForProperty");
    return ERROR;
}

//=============================================================================
// ChangesetValueFactory — high-level resolution helpers
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassIdFromChangeset(
    DbTable const& dbTable, ColumnValueMap const& columnValues,
    ECDbCR conn, ECClassId& classIdOut) {

    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();

    auto it = columnValues.find(classIdCol.GetName());
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    ECClassId candidate(it->second.GetValueUInt64());
    if (!candidate.IsValid())
        return false;

    classIdOut  = candidate;
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassIdFromDbSeek(
    DbTable const& dbTable,
    ColumnValueMap const& columnValues,
    ECDbCR conn, ECClassId& classIdOut) {

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

    classIdOut  = classId;
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::ResolveInstanceId(
    ClassMap const& classMap, BuildCtx& ctx,
    ECInstanceId& instanceIdOut, ChangesetValue*& fieldOut) {

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
        const SystemPropertyMap::PerTableIdPropertyMap* dataMap = sysMap.FindDataPropertyMap(ctx.dbTable);
        if(dataMap == nullptr && ctx.dbTable.GetType() == DbTable::Type::Overflow) {
            DbTable const& parentTable = ctx.dbTable.GetLinkNode().GetParent()->GetTable();
            LOG.infov("ECInstanceId property map not found for overflow table '%s'; trying parent table '%s'.",
                      ctx.dbTable.GetName().c_str(), parentTable.GetName().c_str());
            dataMap = sysMap.FindDataPropertyMap(parentTable);
        }
        if (dataMap == nullptr) {
            LOG.errorv("No ECInstanceId data property map found for table '%s'.",
                       ctx.dbTable.GetName().c_str());
            return ERROR;
        }
        Utf8StringCR colName = dataMap->GetColumn().GetName();
        auto it = ctx.columnValues.find(colName);
        if (it == ctx.columnValues.end() || !it->second.IsValid() || it->second.IsNull()) {
            LOG.errorv("ECInstanceId is absent or null in changeset for table '%s'.",
                       ctx.dbTable.GetName().c_str());
            return ERROR;
        }

        ECInstanceId instanceId(it->second.GetValueUInt64());
        if (!instanceId.IsValid()) {
            LOG.errorv("ECInstanceId resolved to an invalid (zero) id for table '%s'.",
                       ctx.dbTable.GetName().c_str());
            return ERROR;
        }

        ChangesetValue* sysVal = nullptr;
        CreateFixedId(*propertyMap, instanceId, ctx, sysVal);

        instanceIdOut = instanceId;
        fieldOut      = sysVal;
        LOG.debugv("Table '%s': resolved ECInstanceId %" PRIu64 " from changeset.",
                   ctx.dbTable.GetName().c_str(), instanceId.GetValueUnchecked());
        return SUCCESS;
    }

    LOG.errorv("ECInstanceId property not found in ClassMap for table '%s'.",
               ctx.dbTable.GetName().c_str());
    return ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::ResolveClassIdField(
    ClassMap const& classMap, ECClassId resolvedClassId,
    BuildCtx& ctx, ChangesetValue*& out) {

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

        CreateFixedId(*propertyMap, resolvedClassId, ctx, out);
        return SUCCESS;
    }

    LOG.errorv("ECClassId property not found in ClassMap for table '%s'.",
               ctx.dbTable.GetName().c_str());
    return ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::BuildPropertyFields(ClassMap const& classMap, BuildCtx& ctx) {
    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        // ECInstanceId and ECClassId are emitted as fixed slots [0] and [1] by the caller.
        if (propertyMap->IsSystem()) {
            const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
            if (prim != nullptr) {
                const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
                Utf8StringCR propName = propertyMap->GetProperty().GetName();
                if (extType == ExtendedTypeHelper::ExtendedType::Id &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
                    continue;
                if (extType == ExtendedTypeHelper::ExtendedType::ClassId &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
                    continue;
            }
        }
        BentleyStatus status = CreateValueForProperty(*propertyMap, ctx);
        if (status != SUCCESS)
            return status;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::IsDerivedFromBisElement(ECClassId classId, ECDbCR conn) {
    const ECClass* cls = conn.Schemas().Main().GetClass(classId);
    if (cls == nullptr)
        return false;

    const ECClass* bisElementClass = conn.Schemas().Main().GetClass("BisCore", "Element", SchemaLookupMode::AutoDetect);
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
BentleyStatus ChangesetValueFactory::ResolveClassId(
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues, ECClassId& resolvedClassIdOut, bool& classIdFromChangesetOut) {
    resolvedClassIdOut.Invalidate();
    classIdFromChangesetOut = false;

    if (TryResolveClassIdFromChangeset(tbl, columnValues, conn, resolvedClassIdOut)) {
        classIdFromChangesetOut = true;
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " from changeset.",
                   tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
    } else if (TryResolveClassIdFromDbSeek(tbl, columnValues, conn, resolvedClassIdOut)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via DB seek.",
                   tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
    } else {
        const ClassMap* classMapOut = GetRootClassMap(tbl, conn);
        if (classMapOut != nullptr) {
            resolvedClassIdOut = classMapOut->GetClass().GetId();
            LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via GetRootClassMap.",
                       tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
        }
    }

    if (!resolvedClassIdOut.IsValid()) {
        LOG.errorv("Could not resolve ECClassId for table '%s'.", tbl.GetName().c_str());
        return ERROR;
    }

    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::Create(
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues, ECN::ECClassId resolvedClassId, bool classIdFromChangeset,
    std::vector<ChangesetValue*>& fields, PmrObjectAllocator<ChangesetValue>& alloc, ChangesetReader::PropertyFilter propertyFilter, std::vector<Utf8String>& changedProps) {

    const ECClass* cls = conn.Schemas().Main().GetClass(resolvedClassId);
    if (cls == nullptr) {
        LOG.errorv("Resolved ECClassId %" PRIu64 " could not be found in the schema.", resolvedClassId.GetValueUnchecked());
        return ERROR;
    }

    const ClassMap* classMap  = conn.Schemas().Main().GetClassMap(*cls);
    if (classMap == nullptr) {
        LOG.errorv("ClassMap for ECClassId %" PRIu64 " could not be found in the schema.", resolvedClassId.GetValueUnchecked());
        return ERROR;
    }

    // ECInstanceId and ECClassId name collection — handled here since they are
    // emitted as fixed slots and skipped by the BuildPropertyFields loop.
    changedProps.emplace_back(ECDBSYS_PROP_ECInstanceId);
    if (classIdFromChangeset)
        changedProps.emplace_back(ECDBSYS_PROP_ECClassId);

    BuildCtx ctx { conn, columnValues, tbl, alloc, fields, changedProps };

    // -----------------------------------------------------------------------
    // Step 2: Resolve ECInstanceId (slot [0]).
    // -----------------------------------------------------------------------
    ECInstanceId instanceId;
    ChangesetValue* instanceIdField = nullptr;
    BentleyStatus status = ResolveInstanceId(*classMap, ctx, instanceId, instanceIdField);
    if (status != SUCCESS)
        return status;

    // -----------------------------------------------------------------------
    // Step 3: Build ECClassId field (slot [1]).
    // -----------------------------------------------------------------------
    ChangesetValue* classIdField = nullptr;
    status = ResolveClassIdField(*classMap, resolvedClassId, ctx, classIdField);
    if (status != SUCCESS)
        return status;

    // -----------------------------------------------------------------------
    // Step 4: Build remaining property fields (slots [2+]).
    // -----------------------------------------------------------------------
    fields.push_back(instanceIdField);
    fields.push_back(classIdField);

    if (propertyFilter == ChangesetReader::PropertyFilter::InstanceKey)
        return SUCCESS; // caller only needs the instance key — skip user properties

    if(propertyFilter == ChangesetReader::PropertyFilter::BisCoreElement && IsDerivedFromBisElement(resolvedClassId, conn) && !tbl.GetName().EqualsIAscii("bis_Element"))
        return SUCCESS; // caller only needs bis_element properties — skip rest   

    status = BuildPropertyFields(*classMap, ctx);

    if (status != SUCCESS) {
        fields.clear();
        return status;
    }

    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
