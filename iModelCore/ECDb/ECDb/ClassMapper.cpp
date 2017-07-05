/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************ClassMapper*****************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap* ClassMapper::MapProperty(ClassMap& classMap, ECN::ECPropertyCR ecProperty)
    {
    ClassMapper mapper(classMap);
    return mapper.ProcessProperty(ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
PropertyMap* ClassMapper::LoadPropertyMap(ClassMap& classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext)
    {
    ClassMapper mapper(classMap, loadContext);
    return mapper.ProcessProperty(ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap* ClassMapper::ProcessProperty(ECPropertyCR property)
    {
    if (m_classMap.GetPropertyMaps().Find(property.GetName().c_str()))
        {
        BeAssert(false && "PropertyMap already exist. This should have been caught before");
        return nullptr;
        }

    const bool useColumnReservation = (m_classMap.GetColumnFactory().UsesSharedColumnStrategy() && !m_loadContext);
    if (useColumnReservation)
        m_classMap.GetColumnFactory().ReserveSharedColumns(property.GetName());

    RefCountedPtr<PropertyMap> propertyMap = nullptr;
    if (auto typedProperty = property.GetAsPrimitiveProperty())
        propertyMap = MapPrimitiveProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsPrimitiveArrayProperty())
        propertyMap = MapPrimitiveArrayProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructArrayProperty())
        propertyMap = MapStructArrayProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructProperty())
        propertyMap = MapStructProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsNavigationProperty())
        propertyMap = MapNavigationProperty(*typedProperty);
    else
        {
        BeAssert(false && "Unhandled case");
        return nullptr;
        }

    if (propertyMap == nullptr)
        return nullptr;

    if (useColumnReservation)
        m_classMap.GetColumnFactory().ReleaseSharedColumnReservation();

    if (m_classMap.GetPropertyMapsR().Insert(propertyMap) != SUCCESS)
        {
        BeAssert(false && "Failed to insert property map");
        return nullptr;
        }

    return propertyMap.get();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& ClassMapper::GetConstraintMap(ECN::NavigationECPropertyCR navProp, RelationshipClassMapCR relClassMap, NavigationPropertyMap::NavigationEnd end)
    {
    return relClassMap.GetConstraintMap(NavigationPropertyMap::GetRelationshipEnd(navProp, end));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ClassMapper::DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams& params, ClassMap const& classMap, PrimitiveECPropertyCR ecProp, Utf8StringCR accessString)
    {
    //return information whether the col name originates from the PropertyMap CA or whether a default name was used
    Utf8String columnName;
    bool colNameIsFromPropertyMapCA = false;
    bool isNullable = true;
    bool isUnique = false;
    DbColumn::Constraints::Collation collation = DbColumn::Constraints::Collation::Unset;

    IssueReporter const& issues = classMap.GetDbMap().GetECDb().GetImpl().Issues();
    PropertyMapCustomAttribute customPropMap;
    if (ECDbMapCustomAttributeHelper::TryGetPropertyMap(customPropMap, ecProp))
        {
        if (classMap.GetClass().IsEntityClass() && classMap.GetClass().GetEntityClassCP()->IsMixin())
            {
            issues.Report("Failed to map Mixin ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap which is not allowed for mixins.",
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
            issues.Report("Failed to map ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap with a value for 'ColumnName'. Only ECClasses with map strategy 'ExistingTable' may specify a column name.",
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
                issues.Report("Failed to map ECClass '%s': Its ECProperty '%s' has the Custom Attribute PropertyMap with an invalid value for 'Collation': %s",
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



//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<DataPropertyMap> ClassMapper::MapPrimitiveProperty(ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);

    DbColumn::CreateParams createParams;
    if (m_loadContext == nullptr)
        {
        if (SUCCESS != DetermineColumnInfoForPrimitiveProperty(createParams, m_classMap, property, accessString))
            return nullptr;
        }

    if (property.GetType() == PRIMITIVETYPE_Point2d)
        return MapPoint2dProperty(property, compoundPropMap, accessString, createParams);

    if (property.GetType() == PRIMITIVETYPE_Point3d)
        return MapPoint3dProperty(property, compoundPropMap, accessString, createParams);

    DbColumn const* column = nullptr;
    if (m_loadContext != nullptr)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            return nullptr;

        column = columns->front();
        }
    else
        {
        const DbColumn::Type colType = PrimitivePropertyMap::DetermineColumnDataType(property.GetType());
        column = m_classMap.GetColumnFactory().Allocate(property, colType, createParams, accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }

    return PrimitivePropertyMap::CreateInstance(m_classMap, compoundPropMap, property, *column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point2dPropertyMap> ClassMapper::MapPoint2dProperty(ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap, Utf8StringCR accessString, DbColumn::CreateParams const& colCreateParams)
    {
    if (property.GetType() != PRIMITIVETYPE_Point2d)
        return nullptr;

    const DbColumn* x = nullptr, *y = nullptr;
    if (m_loadContext != nullptr)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointX));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointY));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        }
    else
        {
        DbColumn::CreateParams coordColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointX, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        x = m_classMap.GetColumnFactory().Allocate(property, Point2dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointX).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }


        DbColumn::CreateParams yColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointY, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        y = m_classMap.GetColumnFactory().Allocate(property, Point2dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointY).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    return Point2dPropertyMap::CreateInstance(m_classMap, compoundPropMap, property, *x, *y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<Point3dPropertyMap> ClassMapper::MapPoint3dProperty(ECN::PrimitiveECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap, Utf8StringCR accessString, DbColumn::CreateParams const& colCreateParams)
    {
    if (property.GetType() != PRIMITIVETYPE_Point3d)
        return nullptr;

    const DbColumn *x = nullptr, *y = nullptr, *z = nullptr;
    if (m_loadContext != nullptr)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointX));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointY));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + "." ECDBSYS_PROP_PointZ));
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        z = columns->front();
        }
    else
        {
        DbColumn::CreateParams coordColParams;
        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointX, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        x = m_classMap.GetColumnFactory().Allocate(property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointX).c_str());
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointY, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        y = m_classMap.GetColumnFactory().Allocate(property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointY).c_str());
        if (y == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        coordColParams.Assign(colCreateParams.GetColumnName() + "_" ECDBSYS_PROP_PointZ, colCreateParams.IsColumnNameFromPropertyMapCA(), colCreateParams.AddNotNullConstraint(), colCreateParams.AddUniqueConstraint(), colCreateParams.GetCollation());
        z = m_classMap.GetColumnFactory().Allocate(property, Point3dPropertyMap::COORDINATE_COLUMN_DATATYPE, coordColParams, (accessString + "." ECDBSYS_PROP_PointZ).c_str());
        if (z == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    BeAssert(x != nullptr && y != nullptr && z != nullptr);
    return Point3dPropertyMap::CreateInstance(m_classMap, compoundPropMap, property, *x, *y, *z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<PrimitiveArrayPropertyMap> ClassMapper::MapPrimitiveArrayProperty(ECN::PrimitiveArrayECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);
    DbColumn const* column = nullptr;
    if (m_loadContext != nullptr)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }
    else
        {
        Utf8String colName = DbColumn::CreateParams::ColumnNameFromAccessString(accessString);
        column = m_classMap.GetColumnFactory().Allocate(property, PrimitiveArrayPropertyMap::COLUMN_DATATYPE, DbColumn::CreateParams(colName), accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }

    return PrimitiveArrayPropertyMap::CreateInstance(m_classMap, compoundPropMap, property, *column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<StructArrayPropertyMap> ClassMapper::MapStructArrayProperty(ECN::StructArrayECPropertyCR property, CompoundDataPropertyMap const* compoundPropMap)
    {
    //TODO: Create column or map to existing column
    Utf8String accessString = ComputeAccessString(property, compoundPropMap);
    DbColumn const* column = nullptr;
    if (m_loadContext != nullptr)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString);
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }
    else
        {
        Utf8String colName = DbColumn::CreateParams::ColumnNameFromAccessString(accessString);
        column = m_classMap.GetColumnFactory().Allocate(property, StructArrayPropertyMap::COLUMN_DATATYPE, DbColumn::CreateParams(colName), accessString.c_str());
        if (column == nullptr)
            return nullptr;
        }

    return StructArrayPropertyMap::CreateInstance(m_classMap, compoundPropMap, property, *column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<NavigationPropertyMap> ClassMapper::MapNavigationProperty(ECN::NavigationECPropertyCR property)
    {
    RefCountedPtr<NavigationPropertyMap> propertyMap = NavigationPropertyMap::CreateInstance(m_classMap, property);

    //if during mapping, the incomplete prop map is returned and completed later (when corresponding rel class was mapped)
    if (m_loadContext == nullptr)
        return propertyMap;

    const DbColumn *id = nullptr, *relClassId = nullptr;
    std::vector<DbColumn const*> const* columns;
    columns = m_loadContext->FindColumnByAccessString((property.GetName() + "." + ECDBSYS_PROP_NavPropId));
    if (columns == nullptr || columns->size() != 1)
        return nullptr;

    id = columns->front();
    columns = m_loadContext->FindColumnByAccessString((property.GetName() + "." + ECDBSYS_PROP_NavPropRelECClassId));
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
//static 
RefCountedPtr<StructPropertyMap> ClassMapper::MapStructProperty(ECN::StructECPropertyCR property, StructPropertyMap const* parentPropMap)
    {
    StructPropertyMapBuilder builder(m_classMap, parentPropMap, property);
    for (ECN::ECPropertyCP property : property.GetType().GetProperties())
        {
        RefCountedPtr<DataPropertyMap> propertyMap;
        if (auto typedProperty = property->GetAsPrimitiveProperty())
            propertyMap = MapPrimitiveProperty(*typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsPrimitiveArrayProperty())
            propertyMap = MapPrimitiveArrayProperty(*typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsStructProperty())
            propertyMap = MapStructProperty(*typedProperty, &builder.GetPropertyMapUnderConstruction());
        else if (auto typedProperty = property->GetAsStructArrayProperty())
            propertyMap = MapStructArrayProperty(*typedProperty, &builder.GetPropertyMapUnderConstruction());
        else
            {
            BeAssert(false && "Unhandled case");
            return nullptr;
            }

        if (propertyMap == nullptr)
            {
            BeAssert(false && "Failed to created property map");
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
BentleyStatus ClassMapper::SetupNavigationPropertyMap(NavigationPropertyMap& propertyMap)
    {
    if (propertyMap.IsComplete())
        {
        BeAssert(false && "propertyMap must be 'under construction' when passed to this method");
        return ERROR;
        }

    DbMap const& ecdbMap = propertyMap.GetClassMap().GetDbMap();
    ECN::NavigationECPropertyCP navigationProperty = propertyMap.GetProperty().GetAsNavigationProperty();
    ClassMap const* relClassMap = ecdbMap.GetClassMap(*navigationProperty->GetRelationshipClass());
    if (relClassMap == nullptr)
        {
        BeAssert(false && "RelationshipClassMap should not be nullptr when finishing the NavigationPropMap");
        return ERROR;
        }

    if (relClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        ecdbMap.GetECDb().GetImpl().Issues().Report("Failed to map ECClass '%s'. Its NavigationECProperty '%s' refers to a relationship that has the 'NotMapped' strategy. Therefore its dependencies must have that strategy as well.",
                                                                   navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str());
        return ERROR;
        }

    if (relClassMap->GetType() == ClassMap::Type::RelationshipLinkTable)
        {
        ecdbMap.GetECDb().GetImpl().Issues().Report("Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
                                                                   navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str());
        return ERROR;
        }

    //nav prop only supported if going from foreign end (where FK column is persisted) to referenced end
    RelationshipClassEndTableMap const& endTableRelClassMap = relClassMap->GetAs<RelationshipClassEndTableMap>();
    const ECRelationshipEnd foreignEnd = endTableRelClassMap.GetForeignEnd();
    const ECRelatedInstanceDirection navDirection = navigationProperty->GetDirection();
    if ((foreignEnd == ECRelationshipEnd_Source && navDirection == ECRelatedInstanceDirection::Backward) ||
        (foreignEnd == ECRelationshipEnd_Target && navDirection == ECRelatedInstanceDirection::Forward))
        {
        Utf8CP constraintEndName = foreignEnd == ECRelationshipEnd_Source ? "Source" : "Target";
        ecdbMap.GetECDb().GetImpl().Issues().Report("Failed to map Navigation property '%s.%s'. "
                                                                   "Navigation properties can only be defined on the %s constraint ECClass of the respective ECRelationshipClass '%s'. Reason: "
                                                                   "The Foreign Key is mapped to the %s end of this ECRelationshipClass.",
                                                                   navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str(), constraintEndName,
                                                                   endTableRelClassMap.GetClass().GetFullName(), constraintEndName);
        return ERROR;
        }

    ClassMap const& classMap = propertyMap.GetClassMap();

    SingleColumnDataPropertyMap const* idProp = GetConstraintMap(*navigationProperty, endTableRelClassMap, NavigationPropertyMap::NavigationEnd::To).GetECInstanceIdPropMap()->FindDataPropertyMap(classMap);
    SingleColumnDataPropertyMap const* relECClassIdProp = endTableRelClassMap.GetECClassIdPropertyMap()->FindDataPropertyMap(classMap);

    if (idProp == nullptr || relECClassIdProp == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (idProp->GetTable().GetId() !=relECClassIdProp->GetTable().GetId())
        {
        BeAssert(false);
        return ERROR;
        }

    return propertyMap.SetMembers(idProp->GetColumn(), relECClassIdProp->GetColumn(), endTableRelClassMap.GetClass().GetId());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
Utf8String ClassMapper::ComputeAccessString(ECN::ECPropertyCR ecProperty, CompoundDataPropertyMap const* parentPropMap)
    {
    if (parentPropMap == nullptr)
        return ecProperty.GetName();

    Utf8String accessString(parentPropMap->GetAccessString());
    accessString.append(".").append(ecProperty.GetName());
    return accessString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   10/2016
//---------------------------------------------------------------------------------------
//static
ClassMapper::PropertyMapInheritanceMode ClassMapper::GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const& mapStrategyExtInfo)
    {
    if (mapStrategyExtInfo.GetStrategy() != MapStrategy::TablePerHierarchy)
        return PropertyMapInheritanceMode::NotInherited;

    return PropertyMapInheritanceMode::Clone;
    }


//*************************************************************************************
//ClassMapper::TableMapper
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::TableMapper::MapToTable(ClassMap& classMap, ClassMappingInfo const& info)
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

            for (DbTable const* table : classMap.GetTables())
                {
                DbTable::LinkNode const* overflowTableNode = table->GetLinkNode().FindOverflowTable();
                if (overflowTableNode != nullptr)
                    {
                    classMap.AddTable(overflowTableNode->GetTableR());
                    break;
                    }
                }
            }
        }

    if (needsToCreateTable)
        {
        DbTable* table = FindOrCreateTable(classMap, info, tableType, primaryTable);
        if (table == nullptr)
            return ERROR;

        classMap.AddTable(*table);
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static
DbTable* ClassMapper::TableMapper::FindOrCreateTable(ClassMap const& classMap, ClassMappingInfo const& info, DbTable::Type tableType, DbTable const* primaryTable)
    {
    BeAssert(!info.GetECInstanceIdColumnName().empty() && "should always be set (either to user value or default value) by this time");
    const ECClassId exclusiveRootClassId = IsExclusiveRootClassOfTable(info) ? info.GetClass().GetId() : ECClassId();;
    DbTable* table = classMap.GetDbMap().GetDbSchema().FindTableP(info.GetTableName().c_str());
    if (table != nullptr)
        {
        if (table->GetType() != tableType)
            {
            classMap.GetDbMap().Issues().Report("Table %s already used by another ECClass for a different mapping type.", table->GetName().c_str());
            return nullptr;
            }

        if (table->HasExclusiveRootECClass())
            {
            BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
            classMap.GetDbMap().Issues().Report("Table %s is exclusively used by the ECClass with Id %s and therefore "
                                                                        "cannot be used by other ECClasses which are no subclass of the mentioned ECClass.",
                                                table->GetName().c_str(), table->GetExclusiveRootECClassId().ToString().c_str());
            return nullptr;
            }

        if (exclusiveRootClassId.IsValid())
            {
            BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
            classMap.GetDbMap().Issues().Report("The ECClass with Id %s requests exclusive use of the table %s, "
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
        return CreateTableForOtherStrategies(classMap, info.GetTableName(), tableType, info.GetECInstanceIdColumnName(), classIdColPersistenceType, exclusiveRootClassId, primaryTable);

    return CreateTableForExistingTableStrategy(classMap, info.GetTableName(), info.GetECInstanceIdColumnName(), classIdColPersistenceType, exclusiveRootClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle       11/2016
//---------------------------------------------------------------------------------------
//static
DbTable* ClassMapper::TableMapper::CreateTableForOtherStrategies(ClassMap const& classMap, Utf8StringCR tableName, DbTable::Type tableType, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable)
    {
    if (classMap.GetDbMap().GetECDb().TableExists(tableName.c_str()))
        {
        classMap.GetDbMap().Issues().Report("Failed to map ECClass '%s'. It would be mapped to the table '%s' which already exists. Consider applying the 'ExistingTable' MapStrategy to the ECClass.",
                                            classMap.GetClass().GetFullName(), tableName.c_str());
        return nullptr;
        }

    DbTable* table = classMap.GetDbMap().GetDbSchemaR().CreateTable(tableName.c_str(), tableType, exclusiveRootClassId, primaryTable);

    DbColumn* pkColumn = table->CreateColumn(primaryKeyColumnName, DbColumn::Type::Integer, DbColumn::Kind::ECInstanceId, PersistenceType::Physical);
    if (table->GetType() != DbTable::Type::Virtual)
        {
        std::vector<DbColumn*> pkColumns {pkColumn};
        if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns))
            return nullptr;
        }

    if (SUCCESS != CreateClassIdColumn(classMap.GetDbMap().GetDbSchemaR(), *table, classIdColPersistenceType))
        return nullptr;

    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
DbTable* ClassMapper::TableMapper::CreateTableForExistingTableStrategy(ClassMap const& classMap, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECClassId exclusiveRootClassId)
    {
    BeAssert(!existingTableName.empty());

    if (!classMap.GetDbMap().GetECDb().TableExists(existingTableName.c_str()))
        {
        classMap.GetDbMap().Issues().Report("Failed to map ECClass '%s'. The table '%s' specified in ClassMap custom attribute must exist if MapStrategy is ExistingTable.",
                                            classMap.GetClass().GetFullName(), existingTableName.c_str());
        return nullptr;
        }

    DbTable* table = classMap.GetDbMap().GetDbSchemaR().CreateTable(existingTableName, DbTable::Type::Existing, exclusiveRootClassId, nullptr);
    if (table == nullptr)
        return nullptr;

    bvector<SqliteColumnInfo> existingColumnInfos;
    if (SUCCESS != DbSchemaPersistenceManager::RunPragmaTableInfo(existingColumnInfos, classMap.GetDbMap().GetECDb(), existingTableName))
        {
        BeAssert(false && "Failed to get column informations");
        return nullptr;
        }

    if (!table->GetEditHandle().CanEdit())
        table->GetEditHandleR().BeginEdit();

    DbColumn* idColumn = nullptr;
    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;
    for (SqliteColumnInfo const& colInfo : existingColumnInfos)
        {
        DbColumn* column = table->CreateColumn(colInfo.GetName(), colInfo.GetType(), DbColumn::Kind::DataColumn, PersistenceType::Physical);
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
            classMap.GetDbMap().Issues().Report("Failed to map ECClass '%s'. Multi-column PK not supported for MapStrategy 'ExistingTable'. Table: %s",
                                                classMap.GetClass().GetFullName(), existingTableName.c_str());
            return nullptr;
            }

        if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return nullptr;
        }

    if (idColumn != nullptr)
        idColumn->SetKind(DbColumn::Kind::ECInstanceId);
    else
        {
        classMap.GetDbMap().Issues().Report("Failed to map ECClass '%s'. " ECDBSYS_PROP_ECInstanceId " column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy.",
                                            classMap.GetClass().GetFullName(), primaryKeyColName.c_str(), existingTableName.c_str());
        return nullptr;
        }

    if (SUCCESS != CreateClassIdColumn(classMap.GetDbMap().GetDbSchemaR(), *table, classIdColPersistenceType))
        return nullptr;

    table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;
    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle       05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::TableMapper::CreateClassIdColumn(DbSchema& dbSchema, DbTable& table, PersistenceType persType)
    {
    DbColumn* classIdColumn = table.CreateColumn(Utf8String(COL_ECClassId), DbColumn::Type::Integer, 1, DbColumn::Kind::ECClassId, persType);
    if (classIdColumn == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    classIdColumn->GetConstraintsR().SetNotNullConstraint();

    //create index on ECClassId col if it is not virtual
    if (persType == PersistenceType::Virtual)
        return SUCCESS;

    Nullable<Utf8String> indexName("ix_");
    indexName.ValueR().append(table.GetName()).append("_ecclassid");
    return dbSchema.CreateIndex(table, indexName, false, {classIdColumn}, false, true, ECClassId()) != nullptr ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
//static
bool ClassMapper::TableMapper::IsExclusiveRootClassOfTable(ClassMappingInfo const& mappingInfo)
    {
    MapStrategy strategy = mappingInfo.GetMapStrategy().GetStrategy();
    switch (strategy)
        {
            case MapStrategy::ExistingTable:
                //for existing table we also assume an exclusive root as ECDb only supports mapping a single ECClass to an existing table
                return true;

                //OwnedTable obviously always has an exclusive root because only a single class is mapped to the table.
            case MapStrategy::OwnTable:
                return true;

            case MapStrategy::ForeignKeyRelationshipInSourceTable:
            case MapStrategy::ForeignKeyRelationshipInTargetTable:
                return true;
            default:
                break;
        }

    //For subclasses in a TablePerHierarchy, true must be returned for joined table root classes
    ClassMap const* tphBaseClassMap = mappingInfo.GetTphBaseClassMap();
    if (tphBaseClassMap == nullptr) //this is the root of the TablePerHierarchy class hierarchy
        return true;

    //if base class is the direct parent of the joined table, this class is the
    //starting point of the joined table, so also the exclusive root (of the joined table)
    BeAssert(tphBaseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
    return tphBaseClassMap->GetTphHelper()->IsParentOfJoinedTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::TableMapper::DetermineTableName(Utf8StringR tableName, ECN::ECClassCR ecclass, Utf8CP tablePrefix)
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
//static
BentleyStatus ClassMapper::TableMapper::DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR ecclass)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP RelationshipClassEndTableMappingContext::RELECCLASSID_COLNAME_TOKEN = "RelECClassId";


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* RelationshipClassEndTableMappingContext::CreateRelECClassIdColumn(DbMap const& dbMap, ECClassCR ecClass, DbTable& fkTable, ForeignKeyColumnInfo const& fkColInfo, DbColumn const& fkCol, NavigationPropertyMap const& navPropMap)
    {
    BeAssert(!ecClass.HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");

    const bool makeRelClassIdColNotNull = m_relInfo.GetMappingType().GetType() == RelationshipMappingType::Type::PhysicalForeignKey && fkCol.DoNotAllowDbNull();

    PersistenceType persType = PersistenceType::Physical;
    if (fkTable.GetType() == DbTable::Type::Virtual || fkTable.GetType() == DbTable::Type::Existing || ecClass.GetClassModifier() == ECClassModifier::Sealed)
        persType = PersistenceType::Virtual;

    DbColumn* relClassIdCol = fkTable.FindColumnP(fkColInfo.GetRelClassIdColumnName().c_str());
    if (relClassIdCol != nullptr)
        {
        BeAssert(Enum::Contains(relClassIdCol->GetKind(), DbColumn::Kind::RelECClassId));
        if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
            {
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();
            BeAssert(relClassIdCol->GetId().IsValid());
            }

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
        relClassIdCol = navPropMap.GetClassMap().GetColumnFactory().Allocate(navPropMap.GetProperty(), DbColumn::Type::Integer, params, accessString, navPropMap.HasForeignKeyConstraint());
        }
    else
        {
        //WIP_CLEANUP: virtual columns should also be created by the factory
        relClassIdCol = fkTable.CreateColumn(fkColInfo.GetRelClassIdColumnName(), DbColumn::Type::Integer, DbColumn::Kind::DataColumn, persType);

        if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
            {
            BeAssert(relClassIdCol->GetId().IsValid());
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();
            }
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
DbColumn* RelationshipClassEndTableMappingContext::CreateForeignKeyColumn(DbTable&  fkTable, NavigationPropertyMap const& navPropMap, ForeignKeyColumnInfo& fkColInfo)
    {
    ECRelationshipClassCR relClass = *m_relationshipMap.GetClass().GetRelationshipClassCP();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    DbColumn::Kind foreignKeyColumnKind = GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;

    fkColInfo = ForeignKeyColumnInfo::FromNavigationProperty(*navPropMap.GetProperty().GetAsNavigationProperty());
    const bool multiplicityImpliesNotNullOnFkCol = navPropMap.CardinalityImpliesNotNull();

    DbColumn* fkColumn = const_cast<DbColumn*>(fkTable.FindColumn(fkColInfo.GetFkColumnName().c_str()));
    if (fkTable.GetType() == DbTable::Type::Existing)
        {
        //for existing tables, the FK column must exist otherwise we fail schema import
        if (fkColumn != nullptr)
            {
            if (SUCCESS != ValidateForeignKeyColumn(*fkColumn, multiplicityImpliesNotNullOnFkCol, foreignKeyColumnKind))
                return nullptr;

            return fkColumn;
            }

        Issues().Report("Failed to map ECRelationshipClass '%s'. It is mapped to the existing table '%s' not owned by ECDb, but doesn't have a foreign key column called '%s'.",
                        relClass.GetFullName(), fkTable.GetName().c_str(), fkColInfo.GetFkColumnName().c_str());

        return nullptr;
        }

    //table owned by ECDb
    if (fkColumn != nullptr)
        {
        Issues().Report("Failed to map ECRelationshipClass '%s'. ForeignKey column name '%s' is already used by another column in the table '%s'.",
                        relClass.GetFullName(), fkColInfo.GetFkColumnName().c_str(), fkTable.GetName().c_str());
        return nullptr;
        }

    bool makeFkColNotNull = false;
    if (m_relInfo.GetMappingType().GetType() == RelationshipMappingType::Type::PhysicalForeignKey)
        {
        bset<ECClassId> foreignEndConstraintClassIds;
        for (ECClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
            foreignEndConstraintClassIds.insert(constraintClass->GetId());

        makeFkColNotNull = multiplicityImpliesNotNullOnFkCol && fkTable.HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(fkTable.GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
        }

    DbColumn::CreateParams colCreateParams;
    colCreateParams.Assign(fkColInfo.GetFkColumnName(), false, makeFkColNotNull, false, DbColumn::Constraints::Collation::Unset);
    Utf8String accessString = navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropId;
    fkColumn = navPropMap.GetClassMap().GetColumnFactory().Allocate(navPropMap.GetProperty(), DbColumn::Type::Integer, colCreateParams, accessString, navPropMap.HasForeignKeyConstraint());
    if (fkColumn == nullptr)
        {
        Issues().Report("Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                        relClass.GetFullName(), fkColInfo.GetFkColumnName().c_str(), fkTable.GetName().c_str());
        BeAssert(false && "Could not create FK column for end table mapping");
        return nullptr;
        }


    return fkColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMappingContext::CreateForeignKeyConstraint(DbTable const& referencedTable)
    {
    if (m_relInfo.GetMappingType().GetType() != RelationshipMappingType::Type::PhysicalForeignKey)
        return ClassMappingStatus::Success; // logical key don't get fk constraints (by definition)

    ECRelationshipClassCR relClass = *m_relationshipMap.GetClass().GetRelationshipClassCP();
    ForeignKeyDbConstraint::ActionType onDelete = ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType onUpdate = ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType userRequestedDeleteAction = m_relInfo.GetMappingType().GetAs<PhysicalForeignKeyMappingType>().GetOnDeleteAction();
    ForeignKeyDbConstraint::ActionType userRequestedUpdateAction = m_relInfo.GetMappingType().GetAs<PhysicalForeignKeyMappingType>().GetOnUpdateAction();

    if (userRequestedDeleteAction != ForeignKeyDbConstraint::ActionType::NotSpecified)
        onDelete = userRequestedDeleteAction;
    else
        {
        if (relClass.GetStrength() == StrengthType::Embedding)
            onDelete = ForeignKeyDbConstraint::ActionType::Cascade;
        else
            onDelete = ForeignKeyDbConstraint::ActionType::SetNull;
        }

    if (userRequestedUpdateAction != ForeignKeyDbConstraint::ActionType::NotSpecified)
        onUpdate = userRequestedUpdateAction;


    for (DbColumn const* fkCol : GetPartitionColumns(PartitionInfo::ConstraintECInstanceId(GetReferencedEnd())))
        {
        DbTable& fkTable = const_cast<DbTable&>(fkCol->GetTable());
        if (fkTable.GetLinkNode().IsChildTable())
            {
            if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade ||
                (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified && relClass.GetStrength() == StrengthType::Embedding))
                {
                if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade)
                    Issues().Report("Failed to map ECRelationshipClass %s. Its ForeignKeyConstraint custom attribute specifies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());
                else
                    Issues().Report("Failed to map ECRelationshipClass %s. Its strength is 'Embedding' which implies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());

                return ClassMappingStatus::Error;
                }
            }

        if (fkTable.GetType() == DbTable::Type::Existing || fkTable.GetType() == DbTable::Type::Virtual ||
            referencedTable.GetType() == DbTable::Type::Virtual ||
            fkCol->IsShared())
            continue;

        DbColumn const* referencedColumnId = referencedTable.FindFirst(DbColumn::Kind::ECInstanceId);
        fkTable.CreateForeignKeyConstraint(*fkCol, *referencedColumnId, onDelete, onUpdate);
        }

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMappingContext::FinishMapping()
    {
    if (m_relationshipMap.IsRelationshipSubclass())
        return ClassMappingStatus::Success;

    ECRelationshipConstraintCR refConstraint = GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? m_relationshipMap.GetRelationshipClass().GetSource() : m_relationshipMap.GetRelationshipClass().GetTarget();
    std::set<DbTable const*> tables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(m_relationshipMap.GetDbMap(), m_schemaContext, refConstraint, true);
    for (ECClassCP constraintClass : refConstraint.GetConstraintClasses())
        {
        ClassMapCP constraintClassMap = m_relationshipMap.GetDbMap().GetClassMap(*constraintClass);
        if (constraintClassMap == nullptr)
            return ClassMappingStatus::Error;

        if (constraintClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            {
            GetECDb().GetImpl().Issues().Report("Failed to map ECRelationship '%s'. Its has constraint EClass '%s' which has the 'NotMapped' strategy.",
                                                               m_relationshipMap.GetClass().GetFullName(), constraintClassMap->GetClass().GetFullName());
            return ClassMappingStatus::Error;
            }
        }

    if (tables.size() > 1)
        {
        GetECDb().GetImpl().Issues().Report("Failed to map ECRelationship '%s'. Its referenced end maps to more then one table.",
                                                           m_relationshipMap.GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    DbTable const* primaryTable = tables.empty() ? nullptr : *std::begin(tables);
    if (m_relInfo.GetMappingType().GetType() == RelationshipMappingType::Type::PhysicalForeignKey)
        {
        if (PersistedEndHasNonVirtualForeignKeyColumn())
            {
            if (tables.empty())
                {
                GetECDb().GetImpl().Issues().Report("Failed to map ECRelationship '%s'. The relationship specify PhysicalForeignKey constraint but one of its side does not have physical table.",
                                                                   m_relationshipMap.GetClass().GetFullName());
                return ClassMappingStatus::Error;
                }
            }

        PRECONDITION(primaryTable != nullptr, ClassMappingStatus::Error);
        if (CreateForeignKeyConstraint(*primaryTable) != ClassMappingStatus::Success)
            return ClassMappingStatus::Error;
        }

    if (primaryTable != nullptr)
        {
        DbColumn* columnRefClassId = const_cast<DbColumn*>(primaryTable->FindFirst(DbColumn::Kind::ECClassId));
        if (columnRefClassId == nullptr)
            {
            BeAssert(false);
            return ClassMappingStatus::Error;
            }

        const bool primaryTableWasAlreadyInEditState = columnRefClassId->GetTableR().GetEditHandle().CanEdit();
        if (!primaryTableWasAlreadyInEditState)
            columnRefClassId->GetTableR().GetEditHandleR().BeginEdit();

        if (!columnRefClassId->IsShared())
            columnRefClassId->AddKind(GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId);

        for (auto & kp : m_partitions)
            for (auto & entry : kp.second)
                {
                if (!entry.IsPersisted())
                    entry.Set(PartitionInfo::ConstraintECClassId(GetReferencedEnd()), columnRefClassId);
                }
        if (!primaryTableWasAlreadyInEditState)
            columnRefClassId->GetTableR().GetEditHandleR().EndEdit();
        }

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMappingContext::UpdatePersistedEnd(NavigationPropertyMap& navPropMap)
    {
    BeAssert(!navPropMap.IsComplete());
    if (navPropMap.GetClassMap().GetClass().GetFullName () == Utf8String("BridgePhysical:Bridge"))
        {
        printf("");
        }
    //nav prop only supported if going from foreign end (where FK column is persisted) to referenced end
    NavigationECPropertyCP navigationProperty = navPropMap.GetProperty().GetAsNavigationProperty();
    if (m_relationshipMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        GetECDb().GetImpl().Issues().Report("Failed to map ECClass '%s'. Its NavigationECProperty '%s' refers to a relationship that has the 'NotMapped' strategy. Therefore its dependencies must have that strategy as well.",
                                                           navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str());
        return ClassMappingStatus::Error;
        }

    if (navPropMap.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        GetECDb().GetImpl().Issues().Report("Failed to map ECRelationship '%s'. Its NavigationECProperty '%s' come from a ECClass %s which has the 'NotMapped' strategy.",
                                                           m_relationshipMap.GetClass().GetFullName(), navigationProperty->GetName().c_str(), navigationProperty->GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    const ECRelatedInstanceDirection navDirection = navigationProperty->GetDirection();
    if ((GetForeignEnd() == ECRelationshipEnd_Source && navDirection == ECRelatedInstanceDirection::Backward) ||
        (GetForeignEnd() == ECRelationshipEnd_Target && navDirection == ECRelatedInstanceDirection::Forward))
        {
        Utf8CP constraintEndName = GetForeignEnd() == ECRelationshipEnd_Source ? "Source" : "Target";
        GetECDb().GetImpl().Issues().Report("Failed to map Navigation property '%s.%s'. "
                                                           "Navigation properties can only be defined on the %s constraint ECClass of the respective ECRelationshipClass '%s'. Reason: "
                                                           "The Foreign Key is mapped to the %s end of this ECRelationshipClass.",
                                                           navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str(), constraintEndName,
                                                           m_relationshipMap.GetClass().GetFullName(), constraintEndName);
        return ClassMappingStatus::Error;
        }
    ///////////////////////////////////////////////////////////  
    std::vector<PartitionInfo*> partitions;
    if (TryGetPartition(navPropMap.GetClassMap(), partitions))
        {
        ClassMapColumnFactory const& columnFactory = navPropMap.GetClassMap().GetColumnFactory();
        for (PartitionInfo* partition : partitions)
            {
            DbColumn const* id = partition->Get(PartitionInfo::ConstraintECInstanceId(GetReferencedEnd()));
            DbColumn const* classId = partition->Get(PartitionInfo::ColumnId::ECClassId);
            PRECONDITION(id != nullptr && classId != nullptr, ClassMappingStatus::Error);
            if (!columnFactory.IsColumnInUse(*id) && !columnFactory.IsColumnInUse(*classId))
                {
                if (navPropMap.SetMembers(*id, *classId, m_relationshipMap.GetClass().GetId()) != SUCCESS)
                    return ClassMappingStatus::Error;
                
                if (!columnFactory.MarkNavPropertyMapColumnUsed(navPropMap))
                    return ClassMappingStatus::Error;

                return ClassMappingStatus::Success;
                }
            }
        }
    ////////////////////////////////////////////////////////////

    ForeignKeyColumnInfo fkColInfo;
    DbColumn* columnRefId = CreateForeignKeyColumn(const_cast<DbTable&>(navPropMap.GetClassMap().GetJoinedOrPrimaryTable()), navPropMap, fkColInfo);
    if (columnRefId == nullptr)
        return ClassMappingStatus::Error;

    const bool fkTableWasAlreadyInEditState = columnRefId->GetTableR().GetEditHandle().CanEdit();
    if (!fkTableWasAlreadyInEditState)
        columnRefId->GetTableR().GetEditHandleR().BeginEdit();

    if (!columnRefId->IsShared())
        columnRefId->AddKind(GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId);

    DbColumn* columnId = const_cast<DbColumn*>(columnRefId->GetTableR().FindFirst(DbColumn::Kind::ECInstanceId));
    if (columnId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn* columnClassId = CreateRelECClassIdColumn(m_relationshipMap.GetDbMap(), m_relationshipMap.GetClass(), columnRefId->GetTableR(), fkColInfo, *columnRefId, navPropMap);
    if (columnClassId == nullptr)
        return ClassMappingStatus::Error;

    if (!columnClassId->IsShared())
        columnClassId->AddKind(DbColumn::Kind::RelECClassId);

    DbColumn* columnForeignClassId = const_cast<DbColumn*>(columnRefId->GetTable().FindFirst(DbColumn::Kind::ECClassId));
    if (columnForeignClassId == nullptr)
        return ClassMappingStatus::Error;

    if (!columnForeignClassId->IsShared())
        columnForeignClassId->AddKind(GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId);

    DbColumn* columnForeignId = const_cast<DbColumn*>(columnRefId->GetTableR().FindFirst(DbColumn::Kind::ECInstanceId));
    if (columnForeignId == nullptr)
        return ClassMappingStatus::Error;

    if (!columnForeignId->IsShared())
        columnForeignId->AddKind(GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId);

    PRECONDITION(columnRefId != nullptr, ClassMappingStatus::Error);
    PRECONDITION(columnId != nullptr, ClassMappingStatus::Error);
    PRECONDITION(columnClassId != nullptr, ClassMappingStatus::Error);
    PRECONDITION(columnForeignClassId != nullptr, ClassMappingStatus::Error);
    PRECONDITION(columnForeignId != nullptr, ClassMappingStatus::Error);

    ////////////////////////////////////////////////////////////
    PartitionInfo* newPartition = CreatePartition(columnRefId->GetTableR().GetId());
    PRECONDITION(newPartition != nullptr, ClassMappingStatus::Error);
    newPartition->Set(PartitionInfo::ColumnId::ECInstanceId, columnId);
    newPartition->Set(PartitionInfo::ColumnId::ECClassId, columnClassId);
    newPartition->Set(PartitionInfo::ConstraintECInstanceId(GetReferencedEnd()), columnRefId);
    newPartition->Set(PartitionInfo::ConstraintECClassId(GetReferencedEnd()), nullptr); //will be set in finish 
    newPartition->Set(PartitionInfo::ConstraintECInstanceId(GetForeignEnd()), columnForeignId);
    newPartition->Set(PartitionInfo::ConstraintECClassId(GetForeignEnd()), columnForeignClassId);
    if (AddIndexToRelationshipEnd(*newPartition) == ERROR)
        return  ClassMappingStatus::Error;

    const_cast<RelationshipClassEndTableMap&>(m_relationshipMap).Modified();
    return navPropMap.SetMembers(*columnRefId, *columnClassId, m_relationshipMap.GetClass().GetId()) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }

    //--------------------------------------------------------------------------------------
    //@bsimethod                                 Krischan.Eberle                   04/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus RelationshipClassEndTableMappingContext::ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind fkColKind)
        {
        DbTable& fkTable = fkColumn.GetTableR();

        if (fkColumn.DoNotAllowDbNull() != cardinalityImpliesNotNullOnFkCol)
            {
            Utf8CP error = nullptr;
            if (cardinalityImpliesNotNullOnFkCol)
                error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is nullable "
                "although the relationship's cardinality implies that the column is not nullable. Either modify the cardinality or mark the property specified that maps to the foreign key column as not nullable.";
            else
                error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is not nullable "
                "although the relationship's cardinality implies that the column is nullable. Please modify the cardinality accordingly.";

            Issues().Report(error, m_relationshipMap.GetRelationshipClass().GetFullName());
            return ERROR;
            }

        const bool tableIsReadonly = !fkTable.GetEditHandle().CanEdit();
        if (tableIsReadonly)
            fkTable.GetEditHandleR().BeginEdit();

        //Kind of existing columns must be modified so that they also have the constraint ecinstanceid kind
        const_cast<DbColumn&>(fkColumn).AddKind(fkColKind);

        if (tableIsReadonly)
            fkTable.GetEditHandleR().EndEdit();

        return SUCCESS;
        }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipClassEndTableMappingContext::AddIndexToRelationshipEnd(RelationshipClassEndTableMappingContext::PartitionInfo const& info)
    {
    if (m_relInfo.GetMappingType().GetType() != RelationshipMappingType::Type::PhysicalForeignKey)
        return SUCCESS; //indexes only for physical fks - even if they would be enforcing cardinality (via a unique index)

    //0:0 or 1:1 cardinalities imply unique index
    const bool isUniqueIndex = m_relationshipMap.GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1 &&
        m_relationshipMap.GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1;

    DbColumn const* refId = info.Get(PartitionInfo::ConstraintECInstanceId(GetReferencedEnd()));
    DbTable& persistenceEndTable = const_cast<DbTable&>(refId->GetTable());
    if (persistenceEndTable.GetType() == DbTable::Type::Existing || refId->IsShared())
        return SUCCESS;

    DbColumn const* refClassId = info.Get(PartitionInfo::ColumnId::ECClassId);
    if (!refClassId->IsVirtual())
        {
        Nullable<Utf8String> indexName("ix_");
        indexName.ValueR().append(persistenceEndTable.GetName()).append("_").append(refClassId->GetName());
        DbIndex* index = m_relationshipMap.GetDbMap().GetDbSchemaR().CreateIndex(persistenceEndTable, indexName, false, {refClassId}, true, true, m_relationshipMap.GetClass().GetId());
        if (index == nullptr)
            {
            LOG.errorv("Failed to create index on " ECDBSYS_PROP_NavPropRelECClassId " column %s on Table %s.", refClassId->GetName().c_str(), persistenceEndTable.GetName().c_str());
            return ERROR;
            }
        }


    // name of the index
    Nullable<Utf8String> name(isUniqueIndex ? "uix_" : "ix_");
    name.ValueR().append(persistenceEndTable.GetName()).append("_fk_").append(m_relationshipMap.GetClass().GetSchema().GetAlias() + "_" + m_relInfo.GetClass().GetName());
    if (m_relationshipMap.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable)
        name.ValueR().append("_source");
    else
        name.ValueR().append("_target");

    m_relationshipMap.GetDbMap().GetDbSchemaR().CreateIndex(persistenceEndTable, name, isUniqueIndex, {refId}, true, true, m_relInfo.GetClass().GetId());
    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMappingContext::GetForeignEnd() const
    {
    return m_relationshipMap.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMappingContext::GetReferencedEnd() const
    {
    return GetForeignEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }


//************************** RelationshipClassEndTableMap::ForeignKeyColumnInfo *****************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
//static
RelationshipClassEndTableMappingContext::ForeignKeyColumnInfo RelationshipClassEndTableMappingContext::ForeignKeyColumnInfo::FromNavigationProperty(ECN::NavigationECPropertyCR navProp)
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
//static
Utf8String RelationshipClassEndTableMappingContext::ForeignKeyColumnInfo::DetermineRelClassIdColumnName(Utf8StringCR fkColName)
    {
    Utf8String relClassIdColName;
    if (fkColName.EndsWithIAscii("id"))
        relClassIdColName.assign(fkColName.substr(0, fkColName.size() - 2));
    else
        relClassIdColName.assign(fkColName);

    relClassIdColName.append(RELECCLASSID_COLNAME_TOKEN);
    return relClassIdColName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
const std::vector<DbColumn const*> RelationshipClassEndTableMappingContext::GetPartitionColumns(RelationshipClassEndTableMappingContext::PartitionInfo::ColumnId id) const
    {
    std::vector<DbColumn const*> list;
    for (auto & kp : m_partitions)
        {
        for (auto & entry : kp.second)
            {
            list.push_back(entry.Get(id));
            }
        }

    return list;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
bool RelationshipClassEndTableMappingContext::PersistedEndHasNonVirtualForeignKeyColumn() const
    {
    for (DbColumn const* refId : GetPartitionColumns(PartitionInfo::ConstraintECInstanceId(GetReferencedEnd())))
        if (refId->GetPersistenceType() == PersistenceType::Physical)
            return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
bool RelationshipClassEndTableMappingContext::TryGetPartition(ClassMapCR classMap, std::vector<RelationshipClassEndTableMappingContext::PartitionInfo*>& partitions)
    {
    auto itor = m_partitions.find(classMap.GetJoinedOrPrimaryTable().GetId());
    if (itor != m_partitions.end())
        {
        for (auto& entry : itor->second)
            partitions.push_back(&entry);
        }

    if (DbTable const* overFlowTable = classMap.GetOverflowTable())
        {
        auto itor = m_partitions.find(overFlowTable->GetId());
        if (itor != m_partitions.end())
            {
            for (auto& entry : itor->second)
                partitions.push_back(&entry);
            }
        }

    return !partitions.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
std::unique_ptr<RelationshipClassEndTableMappingContext> RelationshipClassEndTableMappingContext::Create(SchemaImportContext& schemaContext, RelationshipClassEndTableMap const& relationshipMap, RelationshipMappingInfo const& relinfo)
    {
    return std::unique_ptr<RelationshipClassEndTableMappingContext>(new RelationshipClassEndTableMappingContext(schemaContext, relationshipMap, relinfo));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
ECDbCR RelationshipClassEndTableMappingContext::GetECDb() const { return m_relationshipMap.GetDbMap().GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
IssueReporter const& RelationshipClassEndTableMappingContext::Issues() const { return m_relationshipMap.GetDbMap().GetECDb().GetImpl().Issues(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
RelationshipClassEndTableMappingContext::PartitionInfo* RelationshipClassEndTableMappingContext::CreatePartition(DbTableId id)
    {
    std::vector<PartitionInfo>& parts = m_partitions[id];
    auto itor = parts.insert(parts.end(), PartitionInfo());
    return &(*itor);
    }

RelationshipClassEndTableMappingContext::RelationshipClassEndTableMappingContext(SchemaImportContext& schemaContext, RelationshipClassEndTableMap const& relationshipMap, RelationshipMappingInfo const& relinfo)
    :m_relInfo(relinfo), m_relationshipMap(relationshipMap), m_schemaContext(schemaContext)
    {
    for (RelationshipClassEndTableMap::Partition const* partition : relationshipMap.GetPartitionView().GetPartitions(/*skipVirutalTable =*/ false))
        m_partitions[partition->GetTable().GetId()].push_back(PartitionInfo(*partition));
    }
//////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
void EndTableMappingContextCollection::RegisterContext(RelationshipClassEndTableMap const& relationshipMap, RelationshipMappingInfo const& relinfo)
    {
    ECN::ECClassCP effectiveClass = GetRoot(relationshipMap.GetClass());
    auto itor = m_contentMap.find(effectiveClass->GetId());
    if (itor != m_contentMap.end())
        return;

    auto newVal = m_contextList.insert(m_contextList.end(), RelationshipClassEndTableMappingContext::Create(m_schemaImportContext, relationshipMap, relinfo));
    m_contentMap[effectiveClass->GetId()] = (*newVal).get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
ClassMappingStatus EndTableMappingContextCollection::FinishMapping()
    {
    for (auto& entry : m_contextList)
        {
        if (entry->FinishMapping() != ClassMappingStatus::Success)
            return ClassMappingStatus::Error;
        }

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
ClassMappingStatus EndTableMappingContextCollection::Map(NavigationPropertyMap& navPropMap)
    {
    NavigationECPropertyCR navProp = static_cast<NavigationECPropertyCR>(navPropMap.GetProperty());
    ECRelationshipClassCP navRel = navProp.GetRelationshipClass();
    DbMap const& dbMap = navPropMap.GetClassMap().GetDbMap();
    if (navRel == nullptr)
        return ClassMappingStatus::Error;

    ClassMapCP relMap = dbMap.GetClassMap(*navRel);
    if (relMap == nullptr)
        {
        ClassMappingStatus r = dbMap.MapRelationshipClass(m_schemaImportContext, *navRel);
        if (r != ClassMappingStatus::Success)
            return r;

        relMap = dbMap.GetClassMap(*navRel);
        if (relMap == nullptr)
            return ClassMappingStatus::Error;
        }

    if (relMap->GetType() != ClassMap::Type::RelationshipEndTable)
        {
        dbMap.GetECDb().GetImpl().Issues().Report("Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
                                                                 navProp.GetClass().GetFullName(), navProp.GetName().c_str());
        return ClassMappingStatus::Error;
        }

    ECN::ECClassCP effectiveClass = GetRoot(*navRel);
    auto itor = m_contentMap.find(effectiveClass->GetId());
    if (itor == m_contentMap.end())
        {
        if (relMap->GetState() == ObjectState::Persisted)
            {
            ClassMappingStatus status = ClassMappingStatus::Success;
            std::unique_ptr<ClassMappingInfo> classMapInfo = ClassMappingInfoFactory::Create(status, m_schemaImportContext, relMap->GetDbMap().GetECDb(), relMap->GetClass());
            if ((status == ClassMappingStatus::Error))
                return status;

            m_schemaImportContext.CacheClassMapInfo(*relMap, classMapInfo);
            itor = m_contentMap.find(effectiveClass->GetId());
            }
        else
            return ClassMappingStatus::Error;
        }

    RelationshipClassEndTableMappingContext* ctx = itor->second;
    const bool useColumnReservation = navPropMap.GetClassMap().GetColumnFactory().UsesSharedColumnStrategy();
    if (useColumnReservation)
        navPropMap.GetClassMap().GetColumnFactory().ReserveSharedColumns(navPropMap.GetName());

    ClassMappingStatus status = ctx->UpdatePersistedEnd(navPropMap);
    if (useColumnReservation)
        navPropMap.GetClassMap().GetColumnFactory().ReleaseSharedColumnReservation();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
EndTableMappingContextCollection::EndTableMappingContextCollection(SchemaImportContext& ctx)
    :m_schemaImportContext(ctx)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    06/2017
//---------------------------------------------------------------------------------------
 ECN::ECClassCP EndTableMappingContextCollection::GetRoot(ECN::ECClassCR ecClass)
    {
    ECN::ECClassCP cursor = &ecClass;
    while (cursor->HasBaseClasses())
        {
        BeAssert(cursor->GetBaseClasses().size() == 1);
        cursor = cursor->GetBaseClasses().front();
        }
    return cursor;
    }

 //////////////////////////////////////////////////////
 //---------------------------------------------------------------------------------------
 // @bsimethod                                 Affan.Khan                    06/2017
 //---------------------------------------------------------------------------------------
 RelationshipClassEndTableMappingContext::PartitionInfo::PartitionInfo(RelationshipClassEndTableMap::Partition const& partition) 
     :m_isPersisted(false)
     {
     Set(ColumnId::ECInstanceId, &partition.GetECInstanceId());
     Set(ColumnId::ECClassId, &partition.GetECClassId());
     Set(ColumnId::SourceECInstanceId, &partition.GetSourceECInstanceId());
     Set(ColumnId::SourceEClassId, &partition.GetSourceECClassId());
     Set(ColumnId::TargetECInstanceId, &partition.GetTargetECInstanceId());
     Set(ColumnId::TargetECClassId, &partition.GetTargetECClassId());
     m_isPersisted = true;
     EXPECTED_CONDITION(IsValid());
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                 Affan.Khan                    06/2017
 //---------------------------------------------------------------------------------------
 void  RelationshipClassEndTableMappingContext::PartitionInfo::Set(ColumnId id, DbColumn const* column)
     {
     if (m_isPersisted)
         {
         BeAssert(false);
         return;
         }

     m_cols[Enum::ToInt(id)] = column;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                 Affan.Khan                    06/2017
 //---------------------------------------------------------------------------------------
 void RelationshipClassEndTableMappingContext::PartitionInfo::Clear()
     {
     for (auto i = 0; i < 6; i++)
         m_cols[i] = nullptr;
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                 Affan.Khan                    06/2017
 //---------------------------------------------------------------------------------------
 bool RelationshipClassEndTableMappingContext::PartitionInfo::IsValid() const
     {
     for (auto i = 0; i < 6; i++)
         if (m_cols[i] == nullptr)
             return false;

     return true;
     }
END_BENTLEY_SQLITE_EC_NAMESPACE
