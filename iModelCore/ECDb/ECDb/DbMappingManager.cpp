/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//******************************************************************************************************
// DbMappingManager::Classes
//******************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap* DbMappingManager::Classes::ProcessProperty(Context& ctx, ECPropertyCR property)
    {
    if (ctx.m_classMap.GetPropertyMaps().Find(property.GetName().c_str()))
        {
        BeAssert(false && "PropertyMap already exist. This should have been caught before");
        return nullptr;
        }

    const bool useColumnReservation = (ctx.m_classMap.GetColumnFactory().UsesSharedColumnStrategy() && !ctx.m_loadCtx);
    if (useColumnReservation)
        ctx.m_classMap.GetColumnFactory().ReserveSharedColumns(property.GetName());

    RefCountedPtr<PropertyMap> propertyMap = nullptr;
    if (auto typedProperty = property.GetAsPrimitiveProperty())
        propertyMap = MapPrimitiveProperty(ctx, *typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsPrimitiveArrayProperty())
        propertyMap = MapPrimitiveArrayProperty(ctx, *typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructArrayProperty())
        propertyMap = MapStructArrayProperty(ctx, *typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructProperty())
        propertyMap = MapStructProperty(ctx, *typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsNavigationProperty())
        propertyMap = MapNavigationProperty(ctx, *typedProperty);
    else
        {
        BeAssert(false && "Unhandled case");
        return nullptr;
        }

    if (propertyMap == nullptr)
        return nullptr;

    if (useColumnReservation)
        ctx.m_classMap.GetColumnFactory().ReleaseSharedColumnReservation();

    if (ctx.m_classMap.GetPropertyMapsR().Insert(propertyMap) != SUCCESS)
        {
        BeAssert(false && "Failed to insert property map");
        return nullptr;
        }

    return propertyMap.get();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<DataPropertyMap> DbMappingManager::Classes::MapPrimitiveProperty(Context& ctx, ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);

    DbColumn::CreateParams createParams;
    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        if (SUCCESS != DetermineColumnInfoForPrimitiveProperty(createParams, ctx.m_classMap, property, accessString))
            return nullptr;
        }

    if (property.GetType() == PRIMITIVETYPE_Point2d)
        return MapPoint2dProperty(ctx, property, compoundPropMap, accessString, createParams);

    if (property.GetType() == PRIMITIVETYPE_Point3d)
        return MapPoint3dProperty(ctx, property, compoundPropMap, accessString, createParams);

    DbColumn const* column = nullptr;
    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        const DbColumn::Type colType = PrimitivePropertyMap::DetermineColumnDataType(property.GetType());
        column = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, colType, createParams, accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }
    else
        {
        BeAssert(ctx.GetMode() == Context::Mode::Loading);
        std::vector<DbColumn const*> const* columns;
        columns = ctx.m_loadCtx->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            return nullptr;

        column = columns->front();
        }

    return PrimitivePropertyMap::CreateInstance(ctx.m_classMap, compoundPropMap, property, *column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
RefCountedPtr<Point2dPropertyMap>  DbMappingManager::Classes::MapPoint2dProperty(Context& ctx, ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap, Utf8StringCR accessString, DbColumn::CreateParams const& colCreateParams)
    {
    if (property.GetType() != PRIMITIVETYPE_Point2d)
        return nullptr;

    const DbColumn* x = nullptr, *y = nullptr;
    
    if( ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        DbColumn::CreateParams coordColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointX, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        x = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, Point2dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointX).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }


        DbColumn::CreateParams yColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointY, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        y = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, Point2dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointY).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }
    else
        {
        BeAssert(ctx.GetMode() == Context::Mode::Loading);
        std::vector<DbColumn const*> const* columns;
        columns = ctx.m_loadCtx->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointX));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = ctx.m_loadCtx->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointY));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        }

    return Point2dPropertyMap::CreateInstance(ctx.m_classMap, compoundPropMap, property, *x, *y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<Point3dPropertyMap>  DbMappingManager::Classes::MapPoint3dProperty(Context& ctx, ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap, Utf8StringCR accessString, DbColumn::CreateParams const& colCreateParams)
    {
    if (property.GetType() != PRIMITIVETYPE_Point3d)
        return nullptr;

    const DbColumn *x = nullptr, *y = nullptr, *z = nullptr;

    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        DbColumn::CreateParams coordColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointX, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        x = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointX).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointY, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        y = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointY).c_str());
        if (y == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointZ, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        z = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointZ).c_str());
        if (z == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }
    else
        {
        BeAssert(ctx.GetMode() == Context::Mode::Loading);
        std::vector<DbColumn const*> const* columns;
        columns = ctx.m_loadCtx->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointX));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = ctx.m_loadCtx->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointY));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        columns = ctx.m_loadCtx->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointZ));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        z = columns->front();
        }

    BeAssert(x != nullptr && y != nullptr && z != nullptr);
    return Point3dPropertyMap::CreateInstance(ctx.m_classMap, compoundPropMap, property, *x, *y, *z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<PrimitiveArrayPropertyMap> DbMappingManager::Classes::MapPrimitiveArrayProperty(Context& ctx, ECN::PrimitiveArrayECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);
    DbColumn const* column = nullptr;

    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        Utf8String colName = DbColumn::CreateParams::ColumnNameFromAccessString(accessString);
        column = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, PrimitiveArrayPropertyMap::COLUMN_DATATYPE, DbColumn::CreateParams(colName), accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }
    else
        {
        BeAssert(ctx.GetMode() == Context::Mode::Loading);
        std::vector<DbColumn const*> const* columns;
        columns = ctx.m_loadCtx->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }

    return PrimitiveArrayPropertyMap::CreateInstance(ctx.m_classMap, compoundPropMap, property, *column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructArrayPropertyMap> DbMappingManager::Classes::MapStructArrayProperty(Context& ctx, ECN::StructArrayECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    //TODO: Create column or map to existing column
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);
    DbColumn const* column = nullptr;
    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        {
        Utf8String colName = DbColumn::CreateParams::ColumnNameFromAccessString(accessString);
        column = ctx.m_classMap.GetColumnFactory().Allocate(*ctx.m_importCtx, property, StructArrayPropertyMap::COLUMN_DATATYPE, DbColumn::CreateParams(colName), accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }
    else
        {
        BeAssert(ctx.GetMode() == Context::Mode::Loading);
        std::vector<DbColumn const*> const* columns;
        columns = ctx.m_loadCtx->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }

    return StructArrayPropertyMap::CreateInstance(ctx.m_classMap, compoundPropMap, property, *column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<NavigationPropertyMap> DbMappingManager::Classes::MapNavigationProperty(Context& ctx, ECN::NavigationECPropertyCR property)
    {
    RefCountedPtr<NavigationPropertyMap> propertyMap = NavigationPropertyMap::CreateInstance(ctx.m_classMap, property);

    //if during mapping, the incomplete prop map is returned and completed later (when corresponding rel class was mapped)
    if (ctx.GetMode() == Context::Mode::ImportingSchema)
        return propertyMap;

    const DbColumn *id = nullptr, *relClassId = nullptr;
    std::vector<DbColumn const*> const* columns;
    columns = ctx.m_loadCtx->FindColumnByAccessString((property.GetName() + "." + ECDBSYS_PROP_NavPropId));
    if (columns == nullptr || columns->size() != 1)
        return nullptr;

    id = columns->front();
    columns = ctx.m_loadCtx->FindColumnByAccessString((property.GetName() + "." + ECDBSYS_PROP_NavPropRelECClassId));
    if (columns == nullptr || columns->size() != 1)
        return nullptr;

    relClassId = columns->front();

    if (SUCCESS != propertyMap->SetMembers(*id, *relClassId, property.GetRelationshipClass()->GetId()))
        return nullptr;

    return propertyMap;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructPropertyMap> DbMappingManager::Classes::MapStructProperty(Context& ctx, ECN::StructECPropertyCR property, StructPropertyMap const* parentPropMap)
    {
    StructPropertyMapBuilder builder(ctx.m_classMap, parentPropMap, property);
    for (ECN::ECPropertyCP property : property.GetType().GetProperties())
        {
        RefCountedPtr<DataPropertyMap> propertyMap;
        if (auto typedProperty = property->GetAsPrimitiveProperty())
            propertyMap = MapPrimitiveProperty(ctx, *typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsPrimitiveArrayProperty())
            propertyMap = MapPrimitiveArrayProperty(ctx, *typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsStructProperty())
            propertyMap = MapStructProperty(ctx, *typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsStructArrayProperty())
            propertyMap = MapStructArrayProperty(ctx, *typedProperty, &builder.GetPropertyMapUnderConstruction());
        else
            {
            BeAssert(false && "Unhandled case");
            return nullptr;
            }

        if (propertyMap == nullptr)
            {
            if (ctx.GetMode() != Context::Mode::Loading)
                {
                BeAssert(false && "Failed to created property map");
                }

            return nullptr;
            }

        if (builder.AddMember(propertyMap) != SUCCESS)
            {
            BeAssert(false && "Failed to insert property map");
            return nullptr;
            }
        }

    if (SUCCESS != builder.Finish())
        return nullptr;

    return builder.GetPropertyMap();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
ClassMappingStatus DbMappingManager::Classes::MapNavigationProperty(SchemaImportContext& ctx, NavigationPropertyMap& navPropMap)
    {
    NavigationECPropertyCR navProp = static_cast<NavigationECPropertyCR>(navPropMap.GetProperty());
    ECRelationshipClassCP navRel = navProp.GetRelationshipClass();
    if (navRel == nullptr)
        return ClassMappingStatus::Error;

    BeAssert(!navRel->HasBaseClasses() && "Nav props can only be on root rel classes. Should have been caught before in schema validator");


    FkRelationshipMappingInfo* fkRelInfo = nullptr;
    if (!ctx.GetFkRelationshipMappingInfos().TryGet(fkRelInfo, navRel->GetId()))
        {
        if (ClassMappingStatus::Success != ctx.GetSchemaManager().MapClass(ctx, *navRel))
                return ClassMappingStatus::Error;

         ClassMap const* relClassMap = ctx.GetSchemaManager().GetClassMap(*navRel);
         if (relClassMap == nullptr)
             {
             BeAssert(false);
             return ClassMappingStatus::Error;
             }

        MapStrategyExtendedInfo const& relMapStrategy = relClassMap->GetMapStrategy();

        if (relMapStrategy.GetStrategy() == MapStrategy::NotMapped)
            {
            ctx.Issues().ReportV("Failed to map navigation property '%s.%s'. Its relationship class '%s' has the map strategy 'NotMapped'. Therefore the navigation property's class '%s' must also have the strategy 'NotMapped'.",
                                navProp.GetClass().GetFullName(), navProp.GetName().c_str(), navRel->GetFullName(), navProp.GetClass().GetName().c_str());
            return ClassMappingStatus::Error;
            }

        if (!MapStrategyExtendedInfo::IsForeignKeyMapping(relMapStrategy))
            {
            //Schema upgrade case. Rel was mapped previously w/o nav prop and amounted to link table. That cannot be changed now
            ctx.Issues().ReportV("Failed to map navigation property '%s.%s'. Its ECRelationshipClass '%s' is mapped as link table relationship.",
                                navProp.GetClass().GetFullName(), navProp.GetName().c_str(), navRel->GetFullName());
            return ClassMappingStatus::Error;
            }

        fkRelInfo = &ctx.GetFkRelationshipMappingInfos().Add(ctx.GetECDb(), *navRel, relMapStrategy.GetStrategy());
        }

    BeAssert(fkRelInfo != nullptr && fkRelInfo->GetPartitionView().GetMapStrategy() != MapStrategy::NotMapped);

    const bool useColumnReservation = navPropMap.GetClassMap().GetColumnFactory().UsesSharedColumnStrategy();
    if (useColumnReservation)
        navPropMap.GetClassMap().GetColumnFactory().ReserveSharedColumns(navPropMap.GetName());

    const BentleyStatus status = FkRelationships::UpdatePersistedEnd(ctx, *fkRelInfo, navPropMap);
    if (useColumnReservation)
        navPropMap.GetClassMap().GetColumnFactory().ReleaseSharedColumnReservation();

    return SUCCESS == status ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   07/2017
//---------------------------------------------------------------------------------------
BentleyStatus DbMappingManager::Classes::MapUserDefinedIndexes(SchemaImportContext& importCtx, ClassMap const& classMap)
    {
    DbIndexListCustomAttribute dbIndexListCA;
    if (!ECDbMapCustomAttributeHelper::TryGetDbIndexList(dbIndexListCA, classMap.GetClass()))
        return SUCCESS;

    bvector<DbIndexListCustomAttribute::DbIndex> indexCAs;
    if (SUCCESS != dbIndexListCA.GetIndexes(indexCAs))
        return ERROR;

    if (indexCAs.empty())
        return SUCCESS;

    if (classMap.GetClass().IsEntityClass() && classMap.GetClass().GetEntityClassCP()->IsMixin())
        {
        importCtx.Issues().ReportV("Failed to map mixin ECClass %s. Mixins cannot have user-defined indexes.", classMap.GetClass().GetFullName());
        return ERROR;
        }

    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        importCtx.Issues().ReportV("Failed to map ECRelationshipClass %s. Foreign key type relationships cannot have user-defined indexes.", classMap.GetClass().GetFullName());
        return ERROR;
        }

    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable || (!classMap.GetMapStrategy().IsTablePerHierarchy() && classMap.GetClass().GetClassModifier() != ECClassModifier::Sealed))
        {
        importCtx.Issues().ReportV("Failed to map ECClass %s. A user-defined index can only be defined on classes with MapStrategy 'TablePerHierarchy' or on sealed classes that don't have the MapStrategy 'ExistingTable'.", classMap.GetClass().GetFullName());
        return ERROR;
        }

    for (DbIndexListCustomAttribute::DbIndex const& indexCA : indexCAs)
        {
        if (SUCCESS != MapUserDefinedIndex(importCtx, classMap, indexCA))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   07/2017
//---------------------------------------------------------------------------------------
BentleyStatus DbMappingManager::Classes::MapUserDefinedIndex(SchemaImportContext& importCtx, ClassMap const& classMap, DbIndexListCustomAttribute::DbIndex const& indexCA)
    {
    ECClassCR ecClass = classMap.GetClass();

    if (!indexCA.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        {
        importCtx.Issues().ReportV("Failed to map ECClass %s. It is mapped using the 'ExistingTable' strategy but also has the DbIndexList custom attribute. Indexes cannot be created in this case.", ecClass.GetFullName());
        return ERROR;
        }

    bool addPropsAreNotNullWhereExp = false;
    if (!indexCA.GetWhereClause().IsNull())
        {
        if (indexCA.GetWhereClause().Value().EqualsIAscii("IndexedColumnsAreNotNull"))
            addPropsAreNotNullWhereExp = true;
        else
            {
            importCtx.Issues().ReportV("Failed to map ECClass %s. Invalid where clause in DbIndexList::DbIndex: %s. Only 'IndexedColumnsAreNotNull' is supported by ECDb.", ecClass.GetFullName(), indexCA.GetWhereClause().Value().c_str());
            return ERROR;
            }
        }

    std::vector<DbColumn const*> totalColumns;
    for (Utf8StringCR propertyAccessString : indexCA.GetProperties())
        {
        PropertyMap const* propertyMap = classMap.GetPropertyMaps().Find(propertyAccessString.c_str());
        if (propertyMap == nullptr)
            {
            importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECClass '%s' is invalid: "
                          "The specified ECProperty '%s' does not exist or is not mapped.",
                          indexCA.GetName().c_str(), ecClass.GetFullName(), propertyAccessString.c_str());
            return ERROR;
            }

        ECPropertyCR prop = propertyMap->GetProperty();
        if (!prop.GetIsPrimitive())
            {
            importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECClass '%s' is invalid: "
                          "The specified ECProperty '%s' is not of a primitive type.",
                          indexCA.GetName().c_str(), ecClass.GetFullName(), propertyAccessString.c_str());
            return ERROR;
            }


        if (propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId)
            {
            BeAssert(classMap.GetType() == ClassMap::Type::RelationshipLinkTable);
            importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECRelationshipClass '%s' is invalid. Cannot define index on the "
                          "system properties " ECDBSYS_PROP_SourceECClassId " or " ECDBSYS_PROP_TargetECClassId ".",
                          indexCA.GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }

        DbTable const& table = classMap.GetJoinedOrPrimaryTable();
        GetColumnsPropertyMapVisitor columnVisitor(table);
        propertyMap->AcceptVisitor(columnVisitor);
        if (table.GetType() == DbTable::Type::Virtual || columnVisitor.GetVirtualColumnCount() > 0)
            {
            importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECClass '%s' is invalid: "
                          "The specified ECProperty '%s' cannot be used in the index as it is not mapped to a column (aka virtual column).",
                          indexCA.GetName().c_str(), ecClass.GetFullName(), propertyAccessString.c_str());
            return ERROR;
            }

        if (columnVisitor.GetColumnCount() == 0)
            {
            if (classMap.GetMapStrategy().GetTphInfo().IsValid() && classMap.GetMapStrategy().GetTphInfo().GetJoinedTableInfo() != JoinedTableInfo::None)
                {
                importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECClass '%s' is invalid. "
                              "The properties that make up the index are mapped to different tables because the 'JoinedTablePerDirectSubclass' custom attribute "
                              "is applied to this class hierarchy.",
                              indexCA.GetName().c_str(), ecClass.GetFullName());
                }
            else
                {
                importCtx.Issues().ReportV("DbIndex custom attribute '%s' on ECClass '%s' is invalid. "
                              "The properties that make up the index are mapped to different tables.",
                              indexCA.GetName().c_str(), ecClass.GetFullName());

                BeAssert(false && "Properties of DbIndex are mapped to different tables although JoinedTable option is not applied.");
                }

            return ERROR;
            }

        totalColumns.insert(totalColumns.end(), columnVisitor.GetColumns().begin(), columnVisitor.GetColumns().end());
        }

    return Tables::CreateIndex(importCtx, classMap.GetJoinedOrPrimaryTable(), indexCA.GetName(), indexCA.IsUnique(), totalColumns, addPropsAreNotNullWhereExp, false, ecClass.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus DbMappingManager::Classes::DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams& params, ClassMap const& classMap, PrimitiveECPropertyCR ecProp, Utf8StringCR accessString)
    {
    //return information whether the col name originates from the PropertyMap CA or whether a default name was used
    Utf8String columnName;
    bool colNameIsFromPropertyMapCA = false;
    bool isNullable = true;
    bool isUnique = false;
    DbColumn::Constraints::Collation collation = DbColumn::Constraints::Collation::Unset;

    IssueReporter const& issues = classMap.GetSchemaManager().Issues();
    PropertyMapCustomAttribute customPropMap;
    if (ECDbMapCustomAttributeHelper::TryGetPropertyMap(customPropMap, ecProp))
        {
        if (classMap.GetClass().IsEntityClass() && classMap.GetClass().GetEntityClassCP()->IsMixin())
            {
            issues.ReportV("Failed to map Mixin ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap which is not allowed for mixins.",
                          ecProp.GetClass().GetFullName(), ecProp.GetName().c_str());
            return ERROR;
            }

        Nullable<Utf8String> colNameFromCA;
        if (SUCCESS != customPropMap.TryGetColumnName(colNameFromCA))
            {
            BeAssert(false);
            return ERROR;
            }

        colNameIsFromPropertyMapCA = !colNameFromCA.IsNull();
        if (!colNameFromCA.IsNull() && classMap.GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
            {
            BeAssert(!colNameFromCA.Value().empty());
            issues.ReportV("Failed to map ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap with a value for 'ColumnName'. Only ECClasses with map strategy 'ExistingTable' may specify a column name.",
                          ecProp.GetClass().GetFullName(), ecProp.GetName().c_str());
            return ERROR;
            }

        if (!colNameFromCA.IsNull())
            columnName.assign(colNameFromCA.Value());

        Nullable<bool> isNullableFromCA;
        if (SUCCESS != customPropMap.TryGetIsNullable(isNullableFromCA))
            return ERROR;

        if (!isNullableFromCA.IsNull())
            isNullable = isNullableFromCA.Value();

        Nullable<bool> isUniqueFromCA;
        if (SUCCESS != customPropMap.TryGetIsUnique(isUniqueFromCA))
            return ERROR;

        if (!isUniqueFromCA.IsNull())
            isUnique = isUniqueFromCA.Value();

        Nullable<Utf8String> collationStr;
        if (SUCCESS != customPropMap.TryGetCollation(collationStr))
            return ERROR;

        if (!collationStr.IsNull())
            {
            if (!DbColumn::Constraints::TryParseCollationString(collation, collationStr.Value()))
                {
                issues.ReportV("Failed to map ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap with an invalid value for 'Collation': %s",
                              ecProp.GetClass().GetFullName(), ecProp.GetName().c_str(), collationStr.Value().c_str());
                return ERROR;
                }
            }
        }


    if (!colNameIsFromPropertyMapCA)
        columnName.assign(DbColumn::CreateParams::ColumnNameFromAccessString(accessString));

    params.Assign(columnName, colNameIsFromPropertyMapCA, !isNullable, isUnique, collation);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const&  DbMappingManager::Classes::GetConstraintMap(ECN::NavigationECPropertyCR navProp, RelationshipClassMapCR relClassMap, NavigationPropertyMap::NavigationEnd end)
    {
    return relClassMap.GetConstraintMap(NavigationPropertyMap::GetRelationshipEnd(navProp, end));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::Classes::TryDetermineRelationshipMappingType(RelationshipMappingType& mappingType, SchemaImportContext const& ctx, ECRelationshipClassCR relClass)
    {
    LinkTableRelationshipMapCustomAttribute linkTableRelationshipMapCA;
    ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationshipMapCA, relClass);

    const bool isRootClass = !relClass.HasBaseClasses();
    if (!isRootClass)
        {
        if (linkTableRelationshipMapCA.IsValid())
            {
            ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. It has a base class and therefore must not have the 'LinkTableRelationshipMap' custom attribute. Only the root relationship class of a hierarchy can have this custom attribute.",
                                           relClass.GetFullName());
            return ERROR;
            }
        }

    if (linkTableRelationshipMapCA.IsValid() || (relClass.GetSource().GetMultiplicity().GetUpperLimit() > 1 && relClass.GetTarget().GetMultiplicity().GetUpperLimit() > 1) ||
        relClass.GetPropertyCount() > 0)
        {
        CachedStatementPtr stmt = ctx.GetECDb().GetImpl().GetCachedSqliteStatement("SELECT 1 FROM main." TABLE_Property " WHERE NavigationRelationshipClassId=?");
        if (stmt == nullptr || BE_SQLITE_OK != stmt->BindId(1, relClass.GetId()))
            {
            BeAssert(false);
            return ERROR;
            }

        if (BE_SQLITE_ROW == stmt->Step())
            {
            ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. It implies to be mapped as link table. However, a navigation property is defined for the relationship which is not supported for link table relationship classes.",
                                relClass.GetFullName());
            return ERROR;
            }

        mappingType = RelationshipMappingType::LinkTable;
        return SUCCESS;
        }

    //Now it would be a FK relationship, but this would require a nav prop. So check for nav prop now
    ECRelationshipEnd fkEnd;
    if (SUCCESS != DbMappingManager::FkRelationships::TryDetermineFkEnd(fkEnd, relClass, ctx.Issues()))
        return ERROR;

    const ECRelatedInstanceDirection navPropDir = fkEnd == ECRelationshipEnd::ECRelationshipEnd_Target ? ECRelatedInstanceDirection::Backward : ECRelatedInstanceDirection::Forward;

    //finally check whether the relationship requires a nav prop. If it does but doesn't have one, we also fall back to a link table
    CachedStatementPtr stmt = ctx.GetECDb().GetImpl().GetCachedSqliteStatement("SELECT ClassId, Name FROM main.ec_Property WHERE NavigationRelationshipClassId=? AND NavigationDirection=? ORDER BY ClassId");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relClass.GetId()) ||
        BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(navPropDir)))
        {
        BeAssert(false);
        return ERROR;
        }

    std::vector<ECClassId> actualConstraintClassIds;
    Utf8String expectedNavPropName;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (!isRootClass)
            {
            ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. A navigation property is defined on its %s constraint although it has a base class. Navigation properties may only be defined for root relationship classes.",
                                           relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
            return ERROR;
            }

        ECClassId constraintClassId = stmt->GetValueId<ECClassId>(0);
        if (!actualConstraintClassIds.empty() && actualConstraintClassIds.back() == constraintClassId)
            {
            ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. More than one navigation property for the same relationship is defined on %s constraint class %s.",
                                           relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                           ctx.GetECDb().Schemas().GetClass(constraintClassId)->GetFullName());
            return ERROR;
            }

        actualConstraintClassIds.push_back(constraintClassId);

        Utf8CP actualNavPropName = stmt->GetValueText(1);
        if (expectedNavPropName.empty())
            expectedNavPropName.assign(actualNavPropName);
        else if (!expectedNavPropName.EqualsIAscii(actualNavPropName))
            {
            ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. The navigation properties must have the same name in all %s constraint classes. "
                                           "Violating names: '%s' versus '%s'",
                                           relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                           expectedNavPropName.c_str(), actualNavPropName);
            return ERROR;
            }
        }

    if (isRootClass)
        {
        if (actualConstraintClassIds.empty())
            {
            LOG.debugv("ECRelationshipClass '%s' is mapped to a link table because none of the constraint classes on the %s end define a navigation property for this relationship class.",
                       relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");


            mappingType = RelationshipMappingType::LinkTable;
            return SUCCESS;
            }

        ECRelationshipConstraintCR fkConstraint = fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
        std::vector<ECClassCP> expectedConstraintClasses(fkConstraint.GetConstraintClasses().begin(), fkConstraint.GetConstraintClasses().end());
        std::sort(expectedConstraintClasses.begin(), expectedConstraintClasses.end(), [] (ECClassCP lhs, ECClassCP rhs) { return lhs->GetId() < rhs->GetId(); });
        const size_t expectedConstraintClassCount = expectedConstraintClasses.size();
        if (actualConstraintClassIds.size() != expectedConstraintClassCount)
            {
            if (actualConstraintClassIds.size() < expectedConstraintClassCount)
                {
                ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. Not all %s constraint classes define a navigation property for this relationship class.",
                                               relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                }
            else
                {
                ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. More navigation properties found for this relationship class than expected: every %s constraint classes must define exactly one navigation property for this relationship class.",
                                               relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                }

            return ERROR;
            }

        for (size_t i = 0; i < expectedConstraintClassCount; i++)
            {
            ECClassCP expectedConstraintClass = expectedConstraintClasses[i];
            if (expectedConstraintClass->GetId() != actualConstraintClassIds[i])
                {
                if (expectedConstraintClass->GetId() > actualConstraintClassIds[i])
                    {
                    ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. No navigation property found for %s constraint class %s.",
                                                   relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                                   expectedConstraintClass->GetFullName());
                    }
                else
                    {
                    ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. Every %s constraint class must define exactly one navigation property for the relationship class.",
                                                   relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                    }

                return ERROR;
                }
            }
        }

    mappingType = fkEnd == ECRelationshipEnd_Source ? RelationshipMappingType::ForeignKeyOnSource : RelationshipMappingType::ForeignKeyOnTarget;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   10/2016
//---------------------------------------------------------------------------------------
DbMappingManager::Classes::PropertyMapInheritanceMode DbMappingManager::Classes::GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const& mapStrategyExtInfo)
    {
    if (!mapStrategyExtInfo.IsTablePerHierarchy())
        return PropertyMapInheritanceMode::NotInherited;

    return PropertyMapInheritanceMode::Clone;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
Utf8String DbMappingManager::Classes::ComputeAccessString(ECN::ECPropertyCR ecProperty, CompoundDataPropertyMap const* parentPropMap)
    {
    if (parentPropMap == nullptr)
        return ecProperty.GetName();

    Utf8String accessString(parentPropMap->GetAccessString());
    accessString.append(".").append(ecProperty.GetName());
    return accessString;
    }


//******************************************************************************************************
// DbMappingManager::FkRelationships
//******************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::UpdatePersistedEnd(SchemaImportContext& ctx, FkRelationshipMappingInfo& fkRelMappingInfo, NavigationPropertyMap& navPropMap)
    {
    BeAssert(!navPropMap.IsComplete());
    NavigationECPropertyCR navProp = *navPropMap.GetProperty().GetAsNavigationProperty();
    if (!fkRelMappingInfo.IsForeignKeyConstraintCustomAttributeRead())
        fkRelMappingInfo.ReadFkConstraintCA(ctx, navProp);
    else
        {
        ForeignKeyConstraintCustomAttribute newCA;
        ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(newCA, navProp);
        if (fkRelMappingInfo.GetFkConstraintCA() != newCA)
            {
            ctx.Issues().ReportV("Failed to map ECClass %s. Its navigation property %s has a different ForeignKeyConstraint custom attribute than the navigation properties of the other constraint classes of the relationship '%s'.",
                                  navProp.GetClass().GetFullName(), navProp.GetName().c_str(), fkRelMappingInfo.GetRelationshipClass().GetFullName());

            return ERROR;
            }
        }

    if (fkRelMappingInfo.GetFkConstraintCA().IsValid())
        {
        if (navPropMap.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
            {
            ctx.Issues().ReportV("Failed to map ECClass %s. Its navigation property %s has the ForeignKeyConstraint custom attribute which cannot be applied for MapStrategy 'ExistingTable'.",
                                  navPropMap.GetClassMap().GetClass().GetFullName(), navProp.GetName().c_str());

            return ERROR;
            }

        SchemaPolicy const* noAdditionalForeignKeyConstraintsPolicy = nullptr;
        if (ctx.GetSchemaPolicies().IsOptedIn(noAdditionalForeignKeyConstraintsPolicy, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints))
            {
            if (SUCCESS != noAdditionalForeignKeyConstraintsPolicy->GetAs<NoAdditionalForeignKeyConstraintsPolicy>().Evaluate(ctx.GetECDb(), navProp))
                return ERROR;
            }
        }

    if (navPropMap.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        ctx.Issues().ReportV("Failed to map ECRelationship '%s'. Its NavigationECProperty '%s' come from a ECClass %s which has the 'NotMapped' strategy.",
                             fkRelMappingInfo.GetRelationshipClass().GetFullName(), navProp.GetName().c_str(), navProp.GetClass().GetFullName());
        return ERROR;
        }

    //nav prop only supported if going from foreign end (where FK column is persisted) to referenced end
    const ECRelationshipEnd fkEnd = fkRelMappingInfo.GetForeignKeyEnd();
    //const ECRelationshipEnd referencedEnd = fkRelMappingInfo.GetReferencedEnd();
    const ECRelatedInstanceDirection navDirection = navProp.GetDirection();
    if ((fkEnd == ECRelationshipEnd_Source && navDirection == ECRelatedInstanceDirection::Backward) ||
        (fkEnd == ECRelationshipEnd_Target && navDirection == ECRelatedInstanceDirection::Forward))
        {
        Utf8CP constraintEndName = fkEnd == ECRelationshipEnd_Source ? "source" : "target";
        ctx.Issues().ReportV("Failed to map Navigation property '%s.%s'. "
                              "Navigation properties can only be defined on the %s constraint ECClass of the respective ECRelationshipClass '%s'. Reason: "
                              "The Foreign Key is mapped to the %s end of this ECRelationshipClass.",
                              navProp.GetClass().GetFullName(), navProp.GetName().c_str(), constraintEndName,
                            fkRelMappingInfo.GetRelationshipClass().GetFullName(), constraintEndName);
        return ERROR;
        }

    if (auto partition = fkRelMappingInfo.GetPartitionView().FindCompatiblePartiton(navPropMap))
        {
        auto navColumns = partition->GetNavigationColumns();
        if (navPropMap.SetMembers(navColumns.GetIdColumn(), navColumns.GetRelECClassIdColumn(), fkRelMappingInfo.GetRelationshipClass().GetId()) != SUCCESS)
            return ERROR;

        if (!navPropMap.GetClassMap().GetColumnFactory().MarkNavPropertyMapColumnUsed(navPropMap))
            return ERROR;

        return SUCCESS;
        }

    DbColumn const* idColumn = navPropMap.GetClassMap().GetColumnFactory().FindColumn((navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropId).c_str());
    DbColumn const* relECClassIdColumn = navPropMap.GetClassMap().GetColumnFactory().FindColumn((navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropRelECClassId).c_str());
    if (idColumn != nullptr&& relECClassIdColumn != nullptr)
        {
        if (navPropMap.SetMembers(*idColumn, *relECClassIdColumn, fkRelMappingInfo.GetRelationshipClass().GetId()) != SUCCESS)
            return ERROR;

        return SUCCESS;
        }
    
    FkRelationshipMappingInfo::ForeignKeyColumnInfo fkColInfo;
    DbColumn* columnRefId = CreateForeignKeyColumn(fkColInfo, ctx, fkRelMappingInfo, const_cast<DbTable&>(navPropMap.GetClassMap().GetJoinedOrPrimaryTable()), navPropMap);
    if (columnRefId == nullptr)
        return ERROR;

    const bool fkTableWasAlreadyInEditState = columnRefId->GetTableR().GetEditHandle().CanEdit();
    if (!fkTableWasAlreadyInEditState)
        columnRefId->GetTableR().GetEditHandleR().BeginEdit();

    DbColumn* columnClassId = CreateRelECClassIdColumn(ctx, fkRelMappingInfo, fkColInfo, columnRefId->GetTableR(), *columnRefId, navPropMap);
    if (columnClassId == nullptr)
        return ERROR;

    if (!fkTableWasAlreadyInEditState)
        columnRefId->GetTableR().GetEditHandleR().EndEdit();

    std::unique_ptr< ForeignKeyPartitionView::Partition> newPartition = ForeignKeyPartitionView::Partition::Create(fkRelMappingInfo.GetPartitionView(), *columnRefId, *columnClassId);
    if (newPartition == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != AddIndexToRelationshipEnd(ctx, fkRelMappingInfo, *newPartition))
        return ERROR;

    if (fkRelMappingInfo.GetPartitionViewR().Insert(std::move(newPartition)) != SUCCESS)
        return ERROR;

    return navPropMap.SetMembers(*columnRefId, *columnClassId, fkRelMappingInfo.GetRelationshipClass().GetId());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::FinishMapping(SchemaImportContext& ctx)
    {
    for (FkRelationshipMappingInfo const* fkRelMappingInfo : ctx.GetFkRelationshipMappingInfos().Get())
        {
        if (SUCCESS != FinishMapping(ctx, *fkRelMappingInfo))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::FinishMapping(SchemaImportContext& ctx, FkRelationshipMappingInfo const& mappingInfo)
    {
    ECRelationshipClassCR relClass = mappingInfo.GetRelationshipClass();
    if (relClass.HasBaseClasses())
        return SUCCESS;

    const ECRelationshipEnd referencedEnd = mappingInfo.GetReferencedEnd();
    ECRelationshipConstraintCR referencedEndConstraint = referencedEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();

    std::set<DbTable const*> referencedEndTables = ctx.GetSchemaManager().GetRelationshipConstraintPrimaryTables(ctx, referencedEndConstraint);
    if (referencedEndTables.size() > 1)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. The referenced end maps to more than one table.", relClass.GetFullName());
        return ERROR;
        }

    if (!mappingInfo.IsPhysicalForeignKey())
        return SUCCESS;

    DbTable const* primaryTable = referencedEndTables.empty() ? nullptr : *std::begin(referencedEndTables);
    if (primaryTable == nullptr)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. It implies a foreign key constraint but its referenced end is not mapped to a physical table.", relClass.GetFullName());
        return ERROR;
        }

    return CreateForeignKeyConstraint(ctx, mappingInfo, *primaryTable);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* DbMappingManager::FkRelationships::CreateRelECClassIdColumn(SchemaImportContext& ctx, FkRelationshipMappingInfo const& mappingInfo, FkRelationshipMappingInfo::ForeignKeyColumnInfo const& fkColInfo, DbTable& fkTable, DbColumn const& fkCol, NavigationPropertyMap const& navPropMap)
    {
    BeAssert(!mappingInfo.GetRelationshipClass().HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");

    const bool makeRelClassIdColNotNull = mappingInfo.IsPhysicalForeignKey() && fkCol.DoNotAllowDbNull();

    PersistenceType persType = PersistenceType::Physical;
    DbTable::Type fkTableType = fkTable.GetType();
    if (fkTableType == DbTable::Type::Virtual || fkTableType == DbTable::Type::Existing || mappingInfo.GetRelationshipClass().GetClassModifier() == ECClassModifier::Sealed)
        persType = PersistenceType::Virtual;

    DbColumn* relClassIdCol = fkTable.FindColumnP(fkColInfo.GetRelClassIdColumnName().c_str());
    if (relClassIdCol != nullptr)
        {
        if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();

        return relClassIdCol;
        }

    const bool canEdit = fkTable.GetEditHandleR().CanEdit();
    if (!canEdit)
        fkTable.GetEditHandleR().BeginEdit();

    if (persType == PersistenceType::Physical)
        {
        Utf8String accessString = navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropRelECClassId;
        DbColumn::CreateParams params;
        params.Assign(fkColInfo.GetRelClassIdColumnName(), false, makeRelClassIdColNotNull, false, DbColumn::Constraints::Collation::Unset);
        relClassIdCol = navPropMap.GetClassMap().GetColumnFactory().Allocate(ctx, navPropMap.GetProperty(), DbColumn::Type::Integer, params, accessString, navPropMap.HasForeignKeyConstraint());
        }
    else
        {
        //WIP_CLEANUP: virtual columns should also be created by the factory
        relClassIdCol = fkTable.AddColumn(fkColInfo.GetRelClassIdColumnName(), DbColumn::Type::Integer, DbColumn::Kind::Default, persType);

        if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();
        }

    if (relClassIdCol == nullptr)
        return nullptr;

    if (!canEdit)
        fkTable.GetEditHandleR().EndEdit();

    return relClassIdCol;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* DbMappingManager::FkRelationships::CreateForeignKeyColumn(FkRelationshipMappingInfo::ForeignKeyColumnInfo& fkColInfo, SchemaImportContext& ctx, FkRelationshipMappingInfo const& mappingInfo, DbTable& fkTable, NavigationPropertyMap const& navPropMap)
    {
    ECRelationshipConstraintCR foreignEndConstraint = mappingInfo.GetForeignKeyEnd() == ECRelationshipEnd_Source ? mappingInfo.GetRelationshipClass().GetSource() : mappingInfo.GetRelationshipClass().GetTarget();
    fkColInfo = FkRelationshipMappingInfo::ForeignKeyColumnInfo::FromNavigationProperty(*navPropMap.GetProperty().GetAsNavigationProperty());
    const bool multiplicityImpliesNotNullOnFkCol = navPropMap.CardinalityImpliesNotNull();
    DbColumn* fkColumn = const_cast<DbColumn*>(fkTable.FindColumn(fkColInfo.GetFkColumnName().c_str()));
    if (fkTable.GetType() == DbTable::Type::Existing)
        {
        //for existing tables, the FK column must exist otherwise we fail schema import
        if (fkColumn != nullptr)
            {
            if (SUCCESS != ValidateForeignKeyColumn(ctx, mappingInfo, *fkColumn, multiplicityImpliesNotNullOnFkCol))
                return nullptr;

            return fkColumn;
            }

        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. It is mapped to the existing table '%s' not owned by ECDb, but doesn't have a foreign key column called '%s'.",
                            mappingInfo.GetRelationshipClass().GetFullName(), fkTable.GetName().c_str(), fkColInfo.GetFkColumnName().c_str());

        return nullptr;
        }

    //table owned by ECDb
    if (fkColumn != nullptr)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. ForeignKey column name '%s' is already used by another column in the table '%s'.",
                              mappingInfo.GetRelationshipClass().GetFullName(), fkColInfo.GetFkColumnName().c_str(), fkTable.GetName().c_str());
        return nullptr;
        }

    bool makeFkColNotNull = false;
    if (mappingInfo.IsPhysicalForeignKey())
        {
        bset<ECClassId> foreignEndConstraintClassIds;
        for (ECClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
            foreignEndConstraintClassIds.insert(constraintClass->GetId());

        makeFkColNotNull = multiplicityImpliesNotNullOnFkCol && fkTable.HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(fkTable.GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
        }

    DbColumn::CreateParams colCreateParams;
    colCreateParams.Assign(fkColInfo.GetFkColumnName(), false, makeFkColNotNull, false, DbColumn::Constraints::Collation::Unset);
    fkColumn = navPropMap.GetClassMap().GetColumnFactory().Allocate(ctx, navPropMap.GetProperty(), DbColumn::Type::Integer, colCreateParams, navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropId, navPropMap.HasForeignKeyConstraint());
    if (fkColumn == nullptr)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                            mappingInfo.GetRelationshipClass().GetFullName(), fkColInfo.GetFkColumnName().c_str(), fkTable.GetName().c_str());
        BeAssert(false && "Could not create FK column for end table mapping");
        return nullptr;
        }

    return fkColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::CreateForeignKeyConstraint(SchemaImportContext& ctx, FkRelationshipMappingInfo const& mappingInfo, DbTable const& referencedTable)
    {
    if (!mappingInfo.IsPhysicalForeignKey())
        return ERROR; // logical key don't get fk constraints (by definition)

    ECRelationshipClassCR relClass = mappingInfo.GetRelationshipClass();

    Nullable<Utf8String> onDeleteActionStr;
    if (SUCCESS != mappingInfo.GetFkConstraintCA().TryGetOnDeleteAction(onDeleteActionStr))
        return ERROR;

    ForeignKeyDbConstraint::ActionType onDeleteAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onDeleteAction, onDeleteActionStr))
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. Its navigation property's ForeignKeyConstraint custom attribute defines an invalid value for OnDeleteAction. See API documentation for valid values.",
                            relClass.GetFullName());
        return ERROR;
        }

    if (onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade && relClass.GetStrength() != StrengthType::Embedding)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. Its navigation property's ForeignKeyConstraint custom attribute can only define 'Cascade' as OnDeleteAction if the relationship strength is 'Embedding'.",
                            relClass.GetFullName());
        return ERROR;
        }

    if (onDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified)
        {
        if (relClass.GetStrength() == StrengthType::Embedding)
            onDeleteAction = ForeignKeyDbConstraint::ActionType::Cascade;
        else
            onDeleteAction = ForeignKeyDbConstraint::ActionType::SetNull;
        }

    Nullable<Utf8String> onUpdateActionStr;
    if (SUCCESS != mappingInfo.GetFkConstraintCA().TryGetOnUpdateAction(onUpdateActionStr))
        return ERROR;

    ForeignKeyDbConstraint::ActionType onUpdateAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onUpdateAction, onUpdateActionStr))
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s'. Its navigation property's ForeignKeyConstraint custom attribute defines an invalid value for OnUpdateAction. See API documentation for valid values.",
                            relClass.GetFullName());
        return ERROR;
        }

    for (ForeignKeyPartitionView::Partition const* partition : mappingInfo.GetPartitionView().GetPartitions())
        {
        DbColumn const& fkCol = partition->GetFromECInstanceIdColumn();
        DbTable& fkTable = const_cast<DbTable&>(fkCol.GetTable());
        if (fkTable.GetLinkNode().IsChildTable())
            {
            if (onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade ||
                (onDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified && mappingInfo.GetRelationshipClass().GetStrength() == StrengthType::Embedding))
                {
                if (onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade)
                    ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. Its ForeignKeyConstraint custom attribute specifies the OnDelete action 'Cascade'. "
                                          "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                        relClass.GetFullName());
                else
                    ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. Its strength is 'Embedding' which implies the OnDelete action 'Cascade'. "
                                          "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                        relClass.GetFullName());

                return ERROR;
                }
            }

        if (fkTable.GetType() == DbTable::Type::Existing || fkTable.GetType() == DbTable::Type::Virtual ||
            referencedTable.GetType() == DbTable::Type::Virtual || fkCol.IsShared())
            continue;

        DbColumn const* referencedColumnId = referencedTable.FindFirst(DbColumn::Kind::ECInstanceId);
        fkTable.AddForeignKeyConstraint(fkCol, *referencedColumnId, onDeleteAction, onUpdateAction);
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                   04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::ValidateForeignKeyColumn(SchemaImportContext& ctx, FkRelationshipMappingInfo const& fkRelMappingInfo, DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol)
    {
    if (fkColumn.DoNotAllowDbNull() != cardinalityImpliesNotNullOnFkCol)
        {
        Utf8CP error = nullptr;
        if (cardinalityImpliesNotNullOnFkCol)
            error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is nullable "
            "although the relationship's cardinality implies that the column is not nullable. Either modify the cardinality or mark the property specified that maps to the foreign key column as not nullable.";
        else
            error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is not nullable "
            "although the relationship's cardinality implies that the column is nullable. Please modify the cardinality accordingly.";

        ctx.Issues().ReportV(error, fkRelMappingInfo.GetRelationshipClass().GetFullName());
        return ERROR;
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbMappingManager::FkRelationships::AddIndexToRelationshipEnd(SchemaImportContext& ctx, FkRelationshipMappingInfo const& mappingInfo, ForeignKeyPartitionView::Partition const& partition)
    {
    if (!mappingInfo.IsPhysicalForeignKey())
        return SUCCESS; //indexes only for physical fks - even if they would be enforcing cardinality (via a unique index)

    ECRelationshipClassCR relClass = mappingInfo.GetRelationshipClass();
    //0:0 or 1:1 cardinalities imply unique index
    const bool isUniqueIndex = relClass.GetSource().GetMultiplicity().GetUpperLimit() <= 1 &&
        relClass.GetTarget().GetMultiplicity().GetUpperLimit() <= 1;

    DbColumn const& refId = partition.GetFromECInstanceIdColumn();
    DbTable& persistenceEndTable = const_cast<DbTable&>(refId.GetTable());
    if (persistenceEndTable.GetType() == DbTable::Type::Existing || refId.IsShared())
        return SUCCESS;

    // name of the index
    Utf8String name(isUniqueIndex ? "uix_" : "ix_");
    name.append(persistenceEndTable.GetName()).append("_fk_").append(relClass.GetSchema().GetAlias() + "_" + relClass.GetName());
    
    if (mappingInfo.GetPartitionView().GetPersistedEnd() == ForeignKeyPartitionView::PersistedEnd::SourceTable)
        name.append("_source");
    else
        name.append("_target");

    if (SUCCESS != Tables::CreateIndex(ctx, persistenceEndTable, name, isUniqueIndex, {&refId}, true, true, relClass.GetId()))
        return ERROR;

    //RelECClassId index
    DbColumn const& refClassId = partition.GetECClassIdColumn();
    if (refClassId.IsVirtual())
        return SUCCESS;

    Utf8String indexName("ix_");
    indexName.append(persistenceEndTable.GetName()).append("_").append(refClassId.GetName());
    return Tables::CreateIndex(ctx, persistenceEndTable, indexName, false, {&refClassId}, true, true, relClass.GetId());
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::FkRelationships::TryDetermineFkEnd(ECN::ECRelationshipEnd& fkEnd, ECN::ECRelationshipClassCR relClass, IssueReporter const& issues)
    {
    const StrengthType strength = relClass.GetStrength();
    const ECRelatedInstanceDirection strengthDirection = relClass.GetStrengthDirection();

    const bool sourceIsM = relClass.GetSource().GetMultiplicity().GetUpperLimit() > 1;
    const bool targetIsM = relClass.GetTarget().GetMultiplicity().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        {
        BeAssert(false && "Must not be called for M:N cardinalities");
        return ERROR;
        }

    if (!sourceIsM && targetIsM)
        {
        if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Backward)
            {
            issues.ReportV("Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Forward'.",
                          relClass.GetFullName(), relClass.GetSource().GetMultiplicity().ToString().c_str(), relClass.GetTarget().GetMultiplicity().ToString().c_str());
            return ERROR;
            }

        fkEnd = ECRelationshipEnd_Target;
        return SUCCESS;
        }

    if (sourceIsM && !targetIsM)
        {
        if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Forward)
            {
            issues.ReportV("Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Backward'.",
                          relClass.GetFullName(), relClass.GetSource().GetMultiplicity().ToString().c_str(), relClass.GetTarget().GetMultiplicity().ToString().c_str());
            return ERROR;
            }

        fkEnd = ECRelationshipEnd_Source;
        return SUCCESS;
        }

    BeAssert(!sourceIsM && !targetIsM);
    if (strengthDirection == ECRelatedInstanceDirection::Forward)
        fkEnd = ECRelationshipEnd_Target;
    else
        fkEnd = ECRelationshipEnd_Source;

    return SUCCESS;
    }


//******************************************************************************************************
// FkRelationshipMappingInfo
//******************************************************************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP FkRelationshipMappingInfo::RELECCLASSID_COLNAME_TOKEN = "RelECClassId";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
FkRelationshipMappingInfo::FkRelationshipMappingInfo(ECDbCR ecdb, ECN::ECRelationshipClassCR relClass, MapStrategy mapStrategy) : m_relClass(relClass)
    {
    //mapping only done in main table space
    m_fkPartitionView = ForeignKeyPartitionView::Create(ecdb.Schemas().Main(), m_relClass, mapStrategy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                  07/2017
//---------------------------------------------------------------------------------------
void FkRelationshipMappingInfo::ReadFkConstraintCA(SchemaImportContext& ctx, ECN::NavigationECPropertyCR navProp)
    {
    BeAssert(!m_fkConstraintCAIsRead);
    ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(m_fkConstraintCA, navProp);
    m_fkConstraintCAIsRead = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd FkRelationshipMappingInfo::GetForeignKeyEnd() const
    {
    return m_fkPartitionView->GetPersistedEnd() == ForeignKeyPartitionView::PersistedEnd::SourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd FkRelationshipMappingInfo::GetReferencedEnd() const
    {
    return GetForeignKeyEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }


//******************************************************************************************************
// FkRelationshipMappingInfo::ForeignKeyColumnInfo
//******************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
//static
FkRelationshipMappingInfo::ForeignKeyColumnInfo FkRelationshipMappingInfo::ForeignKeyColumnInfo::FromNavigationProperty(ECN::NavigationECPropertyCR navProp)
    {
    //the FK column name is implied as <nav prop name>Id.
    //if nav prop name ends with "Id" already, it is not appended again.
    ForeignKeyColumnInfo info(navProp.GetName());

    if (!navProp.GetName().EndsWithIAscii("id"))
        info.m_fkColName.assign(navProp.GetName()).append("Id");

    info.m_relClassIdColName = DetermineRelClassIdColumnName(info.m_fkColName);
    return info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
Utf8String FkRelationshipMappingInfo::ForeignKeyColumnInfo::DetermineRelClassIdColumnName(Utf8StringCR fkColName)
    {
    Utf8String relClassIdColName;
    if (fkColName.EndsWithIAscii("id"))
        relClassIdColName.assign(fkColName.substr(0, fkColName.size() - 2));
    else
        relClassIdColName.assign(fkColName);

    relClassIdColName.append(RELECCLASSID_COLNAME_TOKEN);
    return relClassIdColName;
    }


//*************************************************************************************
//DbMappingManager::Tables
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbMappingManager::Tables::MapToTable(SchemaImportContext& ctx, ClassMap& classMap, ClassMappingInfo const& info)
     {
     const bool isTph = info.GetMapStrategy().IsTablePerHierarchy();
     TablePerHierarchyInfo const& tphInfo = info.GetMapStrategy().GetTphInfo();
     const bool isFk = info.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable || info.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable;

     DbTable::Type tableType = (classMap.GetClass().GetClassModifier() != ECClassModifier::Abstract ||
                                isTph) && !isFk ? DbTable::Type::Primary : DbTable::Type::Virtual;

     bool needsToCreateTable = true;
     DbTable const* primaryTable = nullptr;
     if (!isFk)
         {
         ClassMap const* tphBaseClassMap = isTph ? info.GetTphBaseClassMap() : nullptr;
         if (isTph && tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
             {
             tableType = DbTable::Type::Joined;
             if (tphBaseClassMap == nullptr)
                 {
                 BeAssert(false);
                 return ERROR;
                 }
             }
         else if (info.GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
             tableType = DbTable::Type::Existing;



         needsToCreateTable = !isTph || tphBaseClassMap == nullptr;
         if (tphBaseClassMap != nullptr)
             {
             classMap.SetTable(tphBaseClassMap->GetPrimaryTable());
             if (tableType == DbTable::Type::Joined)
                 {
                 if (tphBaseClassMap->GetTphHelper()->IsParentOfJoinedTable())
                     {
                     primaryTable = &tphBaseClassMap->GetPrimaryTable();
                     needsToCreateTable = true;
                     }
                 else
                     classMap.AddTable(tphBaseClassMap->GetJoinedOrPrimaryTable());
                 }
             }
         }

     if (needsToCreateTable)
         {
         DbTable* table = FindOrCreateTable(ctx, classMap, info, tableType, primaryTable);
         if (table == nullptr)
             return ERROR;

         classMap.AddTable(*table);
         }

     return SUCCESS;
     }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      07/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMappingManager::Tables::CreateVirtualTableForFkRelationship(SchemaImportContext& ctx, RelationshipClassEndTableMap& classMap, ClassMappingInfo const& mappingInfo)
    {
    DbTable* table = FindOrCreateTable(ctx, classMap, mappingInfo, DbTable::Type::Virtual, nullptr);
    if (table == nullptr)
        return ERROR;

    classMap.AddTable(*table);
    return SUCCESS;
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                    casey.mullen      11/2011
 //+---------------+---------------+---------------+---------------+---------------+------
 DbTable* DbMappingManager::Tables::FindOrCreateTable(SchemaImportContext& ctx, ClassMap const& classMap, ClassMappingInfo const& info, DbTable::Type tableType, DbTable const* primaryTable)
     {
     BeAssert(!info.GetECInstanceIdColumnName().empty() && "should always be set (either to user value or default value) by this time");
     const ECClassId exclusiveRootClassId = IsExclusiveRootClassOfTable(info) ? classMap.GetClass().GetId() : ECClassId();

     DbTable* table = ctx.GetSchemaManager().GetDbSchema().FindTableP(info.GetTableName());
     if (table != nullptr)
         {
         if (table->GetType() != tableType)
             {
             ctx.Issues().ReportV("Table %s is already used by another ECClass for a different mapping type.", table->GetName().c_str());
             return nullptr;
             }

         
         if (table->HasExclusiveRootECClass())
             {
             BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
             ctx.Issues().ReportV("Table %s is exclusively used by the ECClass with Id %s and therefore cannot be used by other ECClasses which are no subclass of the mentioned ECClass.",
                                 table->GetName().c_str(), table->GetExclusiveRootECClassId().ToString().c_str());
             return nullptr;
             }

         if (exclusiveRootClassId.IsValid())
             {
             BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
             ctx.Issues().ReportV("The ECClass with Id %s requests exclusive use of the table %s, "
                                                 "but it is already used by some other ECClass.",
                                                 exclusiveRootClassId.ToString().c_str(), table->GetName().c_str());
             return nullptr;
             }

         return table;
         }

     PersistenceType classIdColPersistenceType;
     switch (info.GetMapStrategy().GetStrategy())
         {
             case MapStrategy::OwnTable:
             case MapStrategy::ExistingTable:
             case MapStrategy::ForeignKeyRelationshipInSourceTable:
             case MapStrategy::ForeignKeyRelationshipInTargetTable:
                 classIdColPersistenceType = PersistenceType::Virtual;
                 break;

             case MapStrategy::TablePerHierarchy:
                 classIdColPersistenceType = PersistenceType::Physical;
                 break;

             default:
                 BeAssert(false && "Should have been handled before");
                 return nullptr;
         }

     
     if (tableType != DbTable::Type::Existing)
         return CreateTableForOtherStrategies(ctx, classMap, info.GetTableName(), tableType, info.GetECInstanceIdColumnName(), classIdColPersistenceType, exclusiveRootClassId, primaryTable);

     return CreateTableForExistingTableStrategy(ctx, classMap, info.GetTableName(), info.GetECInstanceIdColumnName(), classIdColPersistenceType, exclusiveRootClassId);
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                Krischan.Eberle       11/2016
 //---------------------------------------------------------------------------------------
  DbTable* DbMappingManager::Tables::CreateTableForOtherStrategies(SchemaImportContext& ctx, ClassMap const& classMap, Utf8StringCR tableName, DbTable::Type tableType, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable)
     {
      if (DbUtilities::TableExists(ctx.GetECDb(), tableName.c_str(), TABLESPACE_Main))
          {
          ctx.Issues().ReportV("Failed to map ECClass '%s'. It would be mapped to table '%s' which exists already and was not created by ECDb. Mapping classes to pre-existing tables requires the MapStrategy 'ExistingTable'.",
                              classMap.GetClass().GetFullName(), tableName.c_str());
          return nullptr;
          }

     DbTable* table = nullptr;
     if (primaryTable != nullptr)
        table = ctx.GetSchemaManager().GetDbSchemaR().AddTable(tableName, tableType, exclusiveRootClassId, *primaryTable);
     else
        table = ctx.GetSchemaManager().GetDbSchemaR().AddTable(tableName, tableType, exclusiveRootClassId);

     DbColumn* pkColumn = table->AddColumn(primaryKeyColumnName, DbColumn::Type::Integer, DbColumn::Kind::ECInstanceId, PersistenceType::Physical);
     if (table->GetType() != DbTable::Type::Virtual)
         {
         std::vector<DbColumn*> pkColumns {pkColumn};
         if (SUCCESS != table->AddPrimaryKeyConstraint(pkColumns))
             return nullptr;
         }

     if (SUCCESS != CreateClassIdColumn(ctx, *table, classIdColPersistenceType))
         return nullptr;

     return table;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                    Affan.Khan        09/2014
 //---------------------------------------------------------------------------------------
 DbTable* DbMappingManager::Tables::CreateTableForExistingTableStrategy(SchemaImportContext& ctx, ClassMap const& classMap, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECClassId exclusiveRootClassId)
     {
     BeAssert(!existingTableName.empty());

     std::vector<SqliteColumnInfo> existingColumnInfos;
     if (SUCCESS != DbSchemaPersistenceManager::RunPragmaTableInfo(existingColumnInfos, ctx.GetECDb(), existingTableName.c_str()) ||
         existingColumnInfos.empty())
         {
         BeAssert(false && "Failed to get column informations");
         return nullptr;
         }

     if (existingColumnInfos.empty())
         {
         ctx.Issues().ReportV("Failed to map ECClass '%s'. The table '%s' specified in ClassMap custom attribute must exist if MapStrategy is ExistingTable.",
                             classMap.GetClass().GetFullName(), existingTableName.c_str());
         return nullptr;
         }

     DbTable* table = ctx.GetSchemaManager().GetDbSchemaR().AddTable(existingTableName, DbTable::Type::Existing, exclusiveRootClassId);
     if (table == nullptr)
         return nullptr;

     if (!table->GetEditHandle().CanEdit())
         table->GetEditHandleR().BeginEdit();

     DbColumn* idColumn = nullptr;
     std::vector<DbColumn*> pkColumns;
     std::vector<size_t> pkOrdinals;
     for (SqliteColumnInfo const& colInfo : existingColumnInfos)
         {
         DbColumn* column = table->AddColumn(colInfo.GetName(), colInfo.GetType(), DbColumn::Kind::Default, PersistenceType::Physical);
         if (column == nullptr)
             {
             BeAssert(false && "Failed to create column");
             return nullptr;
             }

         if (!colInfo.GetDefaultConstraint().empty())
             column->GetConstraintsR().SetDefaultValueExpression(colInfo.GetDefaultConstraint().c_str());

         if (colInfo.IsNotNull())
             column->GetConstraintsR().SetNotNullConstraint();

         if (colInfo.GetPrimaryKeyOrdinal() > 0)
             {
             pkColumns.push_back(column);
             pkOrdinals.push_back((size_t) (colInfo.GetPrimaryKeyOrdinal() - 1));
             }

         if (column->GetName().EqualsIAscii(primaryKeyColName))
             idColumn = column;
         }

     if (!pkColumns.empty())
         {
         if (pkColumns.size() > 1)
             {
             ctx.Issues().ReportV("Failed to map ECClass '%s'. Multi-column PK not supported for MapStrategy 'ExistingTable'. Table: %s",
                                                 classMap.GetClass().GetFullName(), existingTableName.c_str());
             return nullptr;
             }

         if (SUCCESS != table->AddPrimaryKeyConstraint(pkColumns, &pkOrdinals))
             return nullptr;
         }

     if (idColumn != nullptr)
         idColumn->SetKind(DbColumn::Kind::ECInstanceId);
     else
         {
         ctx.Issues().ReportV("Failed to map ECClass '%s'. " ECDBSYS_PROP_ECInstanceId " column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy.",
                                             classMap.GetClass().GetFullName(), primaryKeyColName.c_str(), existingTableName.c_str());
         return nullptr;
         }

     if (SUCCESS != CreateClassIdColumn(ctx, *table, classIdColPersistenceType))
         return nullptr;

     table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;
     return table;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                Krischan.Eberle       05/2017
 //---------------------------------------------------------------------------------------
 BentleyStatus DbMappingManager::Tables::CreateClassIdColumn(SchemaImportContext& ctx, DbTable& table, PersistenceType persType)
     {
     DbColumn* classIdColumn = table.AddColumn(Utf8String(COL_ECClassId), DbColumn::Type::Integer, 1, DbColumn::Kind::ECClassId, persType);
     if (classIdColumn == nullptr)
         {
         BeAssert(false);
         return ERROR;
         }

     classIdColumn->GetConstraintsR().SetNotNullConstraint();

     //create index on ECClassId col if it is not virtual
     if (persType == PersistenceType::Virtual)
         return SUCCESS;

     Utf8String indexName("ix_");
     indexName.append(table.GetName()).append("_ecclassid");
     return CreateIndex(ctx, table, indexName, false, {classIdColumn}, false, true, ECClassId());
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                    Affan.Khan        09/2014
 //---------------------------------------------------------------------------------------
 BentleyStatus DbMappingManager::Tables::CreateIndex(SchemaImportContext& ctx, DbTable& table, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
     {
     if (indexName.empty() || columns.empty())
         {
         BeAssert(false && "Index name and column list must not be empty. Should have been caught before");
         return ERROR;
         }

     if (SUCCESS != ctx.GetSchemaManager().GetDbSchema().LoadIndexDefs())
         return ERROR;

     table.AddIndexDef(std::make_unique<DbIndex>(table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial));
     return SUCCESS;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                    Affan.Khan        04/2017
 //---------------------------------------------------------------------------------------
 DbTable* DbMappingManager::Tables::CreateOverflowTable(SchemaImportContext& ctx, DbTable const& parentTable)
     {
     if (!(parentTable.GetType() == DbTable::Type::Primary ||
           parentTable.GetType() == DbTable::Type::Joined))
         {
         BeAssert(false && "Parent table must be primary or joined table");
         return nullptr;
         }

     if (parentTable.GetType() == DbTable::Type::Virtual)
         {
         BeAssert(false && "Parent table must not be virtual");
         return nullptr;
         }

     if (!parentTable.GetLinkNode().GetChildren().empty())
         {
         BeAssert(false && "Parent table must not have any secondary table at this time");
         return nullptr;
         }

     if (SUCCESS != parentTable.GetLinkNode().Validate())
         return nullptr;

     Utf8String name(parentTable.GetName());
     name.append("_Overflow");
     DbTable* overflowTable = ctx.GetSchemaManager().GetDbSchema().FindTableP(name);
     if (overflowTable != nullptr)
         return overflowTable;

     overflowTable = ctx.GetSchemaManager().GetDbSchemaR().AddTable(name, DbTable::Type::Overflow, ECClassId(), parentTable);
     if (overflowTable == nullptr)
         return nullptr;

     DbColumn const* pk = parentTable.FindFirst(DbColumn::Kind::ECInstanceId);
     DbColumn const* cl = parentTable.FindFirst(DbColumn::Kind::ECClassId);

     DbColumn * npk = overflowTable->AddColumn(pk->GetName(), pk->GetType(), DbColumn::Kind::ECInstanceId, pk->GetPersistenceType());
     DbColumn * ncl = overflowTable->AddColumn(cl->GetName(), cl->GetType(), DbColumn::Kind::ECClassId, pk->GetPersistenceType());
     ncl->GetConstraintsR().SetNotNullConstraint();

     overflowTable->AddPrimaryKeyConstraint({npk});
     overflowTable->AddForeignKeyConstraint(*npk, *pk, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NoAction);
     
     Utf8String indexName("ix_");
     indexName.append(overflowTable->GetName()).append("_ecclassid");
     if (SUCCESS != CreateIndex(ctx, *overflowTable, indexName, false, {ncl}, false, false, ECClassId()))
         return nullptr;

     return overflowTable;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                Krischan.Eberle      07/2016
 //---------------------------------------------------------------------------------------
 bool DbMappingManager::Tables::IsExclusiveRootClassOfTable(ClassMappingInfo const& mappingInfo)
     {
     //All but TPH maps to a single table for which the class is implicitly the exclusive root
     if (!mappingInfo.GetMapStrategy().IsTablePerHierarchy())
         return true;

     //For subclasses in a TablePerHierarchy, true must be returned for joined table root classes
     ClassMap const* tphBaseClassMap = mappingInfo.GetTphBaseClassMap();
     if (tphBaseClassMap == nullptr) //this is the root of the TablePerHierarchy class hierarchy
         return true;

     //if base class is the direct parent of the joined table, this class is the
     //starting point of the joined table, so also the exclusive root (of the joined table)
     BeAssert(tphBaseClassMap->GetMapStrategy().IsTablePerHierarchy());
     return tphBaseClassMap->GetTphHelper()->IsParentOfJoinedTable();
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                Krischan.Eberle      11/2015
 //---------------------------------------------------------------------------------------
  BentleyStatus DbMappingManager::Tables::DetermineTableName(Utf8StringR tableName, ECN::ECClassCR ecclass, Utf8CP tablePrefix)
     {
     if (!Utf8String::IsNullOrEmpty(tablePrefix))
         tableName.assign(tablePrefix);
     else
         {
         if (SUCCESS != DetermineTablePrefix(tableName, ecclass))
             return ERROR;
         }

     tableName.append("_").append(ecclass.GetName());
     return SUCCESS;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                Krischan.Eberle      11/2015
 //---------------------------------------------------------------------------------------
  BentleyStatus DbMappingManager::Tables::DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR ecclass)
     {
     ECSchemaCR schema = ecclass.GetSchema();
     SchemaMapCustomAttribute customSchemaMap;

     if (ECDbMapCustomAttributeHelper::TryGetSchemaMap(customSchemaMap, schema))
         {
         Nullable<Utf8String> tablePrefixFromCA;
         if (SUCCESS != customSchemaMap.TryGetTablePrefix(tablePrefixFromCA))
             return ERROR;

         if (!tablePrefixFromCA.IsNull())
             {
             tablePrefix.assign(tablePrefixFromCA.Value());
             BeAssert(!tablePrefix.empty() && "tablePrefixFromCA is null also if it contains an empty string");
             return SUCCESS;
             }
         }

     Utf8StringCR alias = schema.GetAlias();
     if (!alias.empty())
         tablePrefix = alias;
     else
         tablePrefix = schema.GetName();

     return SUCCESS;
     }


END_BENTLEY_SQLITE_EC_NAMESPACE
