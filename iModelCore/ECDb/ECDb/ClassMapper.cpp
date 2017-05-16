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
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
PropertyMap* ClassMapper::MapProperty(ClassMap& classMap, ECN::ECPropertyCR ecProperty)
    {
    ClassMapper mapper(classMap);
    return mapper.ProcessProperty(ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static
PropertyMap* ClassMapper::LoadPropertyMap(ClassMap& classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext)
    {
    ClassMapper mapper(classMap, loadContext);
    return mapper.ProcessProperty(ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
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
//static
BentleyStatus ClassMapper::CreateECInstanceIdPropertyMap(ClassMap& classMap)
    {
    std::vector<DbColumn const*> ecInstanceIdColumns;
    DbColumn const* ecInstanceIdColumn = classMap.GetJoinedOrPrimaryTable().FindFirst(DbColumn::Kind::ECInstanceId);
    if (ecInstanceIdColumn == nullptr)
        {
        BeAssert(false && "ECInstanceId column does not exist in table");
        return ERROR;
        }

    ecInstanceIdColumns.push_back(ecInstanceIdColumn);
    RefCountedPtr<ECInstanceIdPropertyMap> newProperty = ECInstanceIdPropertyMap::CreateInstance(classMap, ecInstanceIdColumns);
    if (newProperty == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    return classMap.GetPropertyMapsR().Insert(newProperty, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::CreateECClassIdPropertyMap(ClassMap& classMap)
    {
    std::vector<DbColumn const*> ecClassIdColumns;
    DbColumn const* ecClassIdColumn = classMap.GetJoinedOrPrimaryTable().FindFirst(DbColumn::Kind::ECClassId);
    if (ecClassIdColumn == nullptr)
        {
        BeAssert(false && "ECInstanceId column does not exist in table");
        return ERROR;
        }

    ecClassIdColumns.push_back(ecClassIdColumn);
    RefCountedPtr<ECClassIdPropertyMap> newProperty = ECClassIdPropertyMap::CreateInstance(classMap, ecClassIdColumns);
    if (newProperty == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }


    return classMap.GetPropertyMapsR().Insert(newProperty, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipEnd ClassMapper::GetConstraintEnd(ECN::NavigationECPropertyCR prop, NavigationPropertyMap::NavigationEnd end)
    {
    const ECRelatedInstanceDirection navPropDir = prop.GetDirection();
    if (navPropDir == ECRelatedInstanceDirection::Forward && end == NavigationPropertyMap::NavigationEnd::From ||
        navPropDir == ECRelatedInstanceDirection::Backward && end == NavigationPropertyMap::NavigationEnd::To)
        return ECRelationshipEnd_Source;

    return ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& ClassMapper::GetConstraintMap(ECN::NavigationECPropertyCR navProp, RelationshipClassMapCR relClassMap, NavigationPropertyMap::NavigationEnd end)
    {
    return relClassMap.GetConstraintMap(GetConstraintEnd(navProp, end));
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

    IssueReporter const& issues = classMap.GetDbMap().GetECDb().GetECDbImplR().GetIssueReporter();
    ECDbPropertyMap customPropMap;
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

    if (relClassMap->GetType() == ClassMap::Type::RelationshipLinkTable)
        {
        ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
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
        ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map Navigation property '%s.%s'. "
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


//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
Utf8String ClassMapper::ComputeAccessString(ECN::ECPropertyCR ecProperty, CompoundDataPropertyMap const* parentPropMap)
    {
    if (parentPropMap == nullptr)
        return ecProperty.GetName();

    Utf8String accessString(parentPropMap->GetAccessString());
    accessString.append(".").append(ecProperty.GetName());
    return accessString;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
