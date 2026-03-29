/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using Stage = Changes::Change::Stage;

const ClassMap* ChangesetFieldFactory::GetRootClassMap(DbTable const& tbl, ECDbCR conn) {
    ECClassId rootClassId;
    if (tbl.GetType() == DbTable::Type::Overflow) {
        rootClassId = tbl.GetLinkNode().GetParent()->GetTable().GetExclusiveRootECClassId();
    } else {
        rootClassId = tbl.GetExclusiveRootECClassId();
    }

    const auto rootClass = conn.Schemas().Main().GetClass(rootClassId);
    if (rootClass != nullptr) {
        return conn.Schemas().Main().GetClassMap(*rootClass);
    }
    return nullptr;
}

std::vector<ChangesetFieldFactory::FieldPtr> ChangesetFieldFactory::Create(ECDbCR conn, DbTable const& tbl, Changes::Change const& change, Stage const& stage) {
    auto classMap = GetRootClassMap(tbl, conn);
    if(classMap == nullptr) {
        return std::vector<FieldPtr>();
    }
    std::vector<FieldPtr> queryProps;
    for (auto& propertyMap : classMap->GetPropertyMaps()) {
        GetTablesPropertyMapVisitor visitor(PropertyMap::Type::All);
        propertyMap->AcceptVisitor(visitor);
        DbTable const* table = (*visitor.GetTables().begin());
        if (propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId) {
            if (!propertyMap->IsMappedToClassMapTables()) {
                table = classMap->GetTables().front();
            }
        }
        const auto queryTable = TableView::Create(conn, *table);
        if (queryTable == nullptr) {
            return std::vector<FieldPtr>();
        }
        queryProps.emplace_back(
            CreateField(conn, *propertyMap, *(queryTable.get()), change, stage));
    }
    return queryProps;
}

ECSqlPropertyPath ChangesetFieldFactory::GetPropertyPath(PropertyMap const& propertyMap) {
    ECSqlPropertyPath propertyPath;
    for (auto& part : propertyMap.GetPath()) {
        propertyPath.AddEntry(part->GetProperty());
    }
    return propertyPath;
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreatePrimitiveField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        GetDateTimeInfo(propertyMap),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    if (prim->GetType() == ECN::PRIMITIVETYPE_Point2d) {
        const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt2dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt2dMap.GetY().GetColumn());
        return std::make_unique<PointECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, xCol, yCol);
    } else if (prim->GetType() == PRIMITIVETYPE_Point3d) {
        const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt3dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt3dMap.GetY().GetColumn());
        const auto zCol = tbl.GetColumnIndexOf(pt3dMap.GetZ().GetColumn());
        return std::make_unique<PointECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, xCol, yCol, zCol);
    } else {
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        const auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
        return std::make_unique<PrimitiveECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, nCol);
    }
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateSystemField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    const auto extendedType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
    if (extendedType == ExtendedTypeHelper::ExtendedType::ClassId && tbl.GetClassIdCol() >= 0) {
        return std::make_unique<PrimitiveECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, tbl.GetClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId && tbl.GetSourceClassIdCol() >= 0) {
        return std::make_unique<PrimitiveECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, tbl.GetSourceClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::TargetClassId && tbl.GetTargetClassIdCol() >= 0) {
        return std::make_unique<PrimitiveECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, tbl.GetTargetClassIdCol());
    }

    const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
    const auto dataMap = sysMap.GetDataPropertyMaps().front();
    if (dataMap->GetColumn().IsVirtual()) {
        BeAssert(false);
    }
    const auto nCol = tbl.GetColumnIndexOf(dataMap->GetColumn());
    return std::make_unique<PrimitiveECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, nCol);
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateStructField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto structProp = propertyMap.GetProperty().GetAsStructProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreateStructTypeDescriptor(),
        DateTime::Info(),
        &structProp->GetType(),
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    auto newStructField = std::make_unique<StructECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo);
    auto& structPropertyMap = propertyMap.GetAs<StructPropertyMap>();
    for (auto& memberMap : structPropertyMap) {
        newStructField->AppendField(CreateField(ecdb, *memberMap, tbl, change, stage));
    }
    return std::move(newStructField);
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateClassIdField(ECDbCR ecdb, PropertyMap const& propertyMap, ECN::ECClassId id, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreatePrimitiveTypeDescriptor(PRIMITIVETYPE_Long),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    return std::make_unique<ClassIdECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, id);
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateNavigationField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto prim = propertyMap.GetProperty().GetAsNavigationProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreateNavigationTypeDescriptor(prim->GetType(), prim->IsMultiple()),
        GetDateTimeInfo(propertyMap),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    auto idField = CreatePrimitiveField(ecdb, navMap.GetIdPropertyMap(), tbl, change, stage);

    std::unique_ptr<ECSqlField> relClassIdField;
    auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    if (relClassIdMap.GetColumn().IsVirtual()) {
        relClassIdField = CreateClassIdField(ecdb, relClassIdMap, prim->GetRelationshipClass()->GetId(), tbl, change, stage);
    } else {
        relClassIdField = CreatePrimitiveField(ecdb, navMap.GetRelECClassIdPropertyMap(), tbl, change, stage);
    }
    auto navField = std::make_unique<NavigationPropertyECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo);
    navField->SetMembers(std::move(idField), std::move(relClassIdField));
    return std::move(navField);
}

DateTime::Info ChangesetFieldFactory::GetDateTimeInfo(PropertyMap const& propertyMap) {
    DateTime::Info info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    if (propertyMap.GetType() != PropertyMap::Type::PrimitiveArray && propertyMap.GetType() != PropertyMap::Type::Primitive) {
        return info;
    }

    if (auto property = propertyMap.GetProperty().GetAsPrimitiveArrayProperty()) {
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }
    if (auto property = propertyMap.GetProperty().GetAsPrimitiveProperty()) {
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }

    return info;
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateArrayField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    ECN::ECTypeDescriptor desc;
    const auto& prop = propertyMap.GetProperty();
    if (prop.GetIsStructArray()) {
        desc = ECN::ECTypeDescriptor::CreateStructArrayTypeDescriptor();
    } else {
        auto primType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        desc = ECN::ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(primType);
    }
    ECSqlColumnInfo columnInfo(
        desc,
        GetDateTimeInfo(propertyMap),
        prop.GetIsStructArray() ? &prop.GetAsStructArrayProperty()->GetStructElementType() : nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    return std::make_unique<ArrayECSqlField>(ecdb, std::make_unique<ChangeSetDbRow>(change, stage), columnInfo, nCol);
}

std::unique_ptr<ECSqlField> ChangesetFieldFactory::CreateField(ECDbCR ecdb, PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem()) {
        return CreateSystemField(ecdb, propertyMap, tbl, change, stage);
    } else if (prop.GetIsPrimitive()) {
        return CreatePrimitiveField(ecdb, propertyMap, tbl, change, stage);
    } else if (prop.GetIsStruct()) {
        return CreateStructField(ecdb, propertyMap, tbl, change, stage);
    } else if (prop.GetIsNavigation()) {
        return CreateNavigationField(ecdb, propertyMap, tbl, change, stage);
    } else if (prop.GetIsArray()) {
        return CreateArrayField(ecdb, propertyMap, tbl, change, stage);
    }
    return nullptr;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
