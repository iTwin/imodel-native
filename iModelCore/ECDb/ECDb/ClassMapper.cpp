/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
WipPropertyMap* ClassMapper::MapProperty(ClassMapR classMap, ECN::ECPropertyCR ecProperty)
    {
    ClassMapper mapper(classMap);
    return mapper.ProcessProperty(ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static
WipPropertyMap* ClassMapper::LoadPropertyMap(ClassMapR classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext)
    {
    ClassMapper mapper(classMap, loadContext);
    return mapper.ProcessProperty(ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::CreateECInstanceIdPropertyMap(ClassMap& classMap)
    {
    std::vector<DbColumn const*> ecInstanceIdColumns;
    DbColumn const* ecInstanceIdColumn = classMap.GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    if (ecInstanceIdColumn != nullptr)
        {
        BeAssert(false && "ECInstanceId column does not exist in table");
        return ERROR;
        }

    ecInstanceIdColumns.push_back(ecInstanceIdColumn);
    RefCountedPtr<WipECInstanceIdPropertyMap> newProperty = WipPropertyMapFactory::CreateECInstanceIdPropertyMap(classMap, ecInstanceIdColumns);
    if (newProperty != nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    newProperty->FinishEditing();
    return classMap.GetPropertyMapsR().Insert(newProperty, 0LL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMapper::CreateECClassIdPropertyMap(ClassMap& classMap)
    {
    std::vector<DbColumn const*> ecClassIdColumns;
    DbColumn const* ecClassIdColumn = classMap.GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    if (ecClassIdColumn != nullptr)
        {
        BeAssert(false && "ECInstanceId column does not exist in table");
        return ERROR;
        }

    ecClassIdColumns.push_back(ecClassIdColumn);
    RefCountedPtr<WipECClassIdPropertyMap> newProperty = WipPropertyMapFactory::CreateECClassIdPropertyMap(classMap, classMap.GetClass().GetId(), ecClassIdColumns);
    if (newProperty != nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    newProperty->FinishEditing();
    return classMap.GetPropertyMapsR().Insert(newProperty, 1LL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipConstraintCR ClassMapper::GetConstraint(ECN::NavigationECPropertyCR navProp, WipNavigationPropertyMap::NavigationEnd end)
    {
    ECRelationshipClassCP relClass = navProp.GetRelationshipClass();
    return GetConstraintEnd(navProp, end) == ECRelationshipEnd_Source ? relClass->GetSource() : relClass->GetTarget();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipEnd ClassMapper::GetConstraintEnd(ECN::NavigationECPropertyCR prop, WipNavigationPropertyMap::NavigationEnd end)
    {
    const ECRelatedInstanceDirection navPropDir = prop.GetDirection();
    if (navPropDir == ECRelatedInstanceDirection::Forward && end == WipNavigationPropertyMap::NavigationEnd::From ||
        navPropDir == ECRelatedInstanceDirection::Backward && end == WipNavigationPropertyMap::NavigationEnd::To)
        return ECRelationshipEnd_Source;

    return ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& ClassMapper::GetConstraintMap(ECN::NavigationECPropertyCR navProp, RelationshipClassMapCR relClassMap, WipNavigationPropertyMap::NavigationEnd end)
    {
    return relClassMap.GetConstraintMap(GetConstraintEnd(navProp, end));
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus ClassMapper::SetupNavigationPropertyMap(WipNavigationPropertyMap& propertyMap)
    {
    if (!propertyMap.InEditMode())
        {
        BeAssert(false);
        return SUCCESS;
        }
    ECDbMap const& ecdbMap = propertyMap.GetClassMap().GetDbMap();
    ECN::NavigationECPropertyCP navigationProperty = propertyMap.GetProperty().GetAsNavigationProperty();
    RelationshipClassMap const* relClassMap = static_cast<RelationshipClassMap const*> (ecdbMap.GetClassMap(*navigationProperty->GetRelationshipClass()));
    if (relClassMap == nullptr)
        {
        BeAssert(false && "RelationshipClassMap should not be nullptr when finishing the NavigationPropMap");
        return ERROR;
        }

    if (relClassMap->GetType() == ClassMap::Type::RelationshipLinkTable)
        {
        ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                   "Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
                                                                   navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str());
        return ERROR;
        }

    ClassMap const& classMap = propertyMap.GetClassMap();

    WipColumnVerticalPropertyMap const* idProp = GetConstraintMap(*navigationProperty, *relClassMap, WipNavigationPropertyMap::NavigationEnd::To).GetECInstanceIdPropMap()->FindVerticalPropertyMap(classMap.GetPrimaryTable());
    WipColumnVerticalPropertyMap const* relECClassIdProp = relClassMap->GetECClassIdPropertyMap()->FindVerticalPropertyMap(classMap.GetPrimaryTable());

    if ((idProp == nullptr || relECClassIdProp == nullptr) && !classMap.IsMappedToSingleTable())
        {
        idProp = GetConstraintMap(*navigationProperty, *relClassMap, WipNavigationPropertyMap::NavigationEnd::To).GetECInstanceIdPropMap()->FindVerticalPropertyMap(classMap.GetJoinedTable());
        relECClassIdProp = relClassMap->GetECClassIdPropertyMap()->FindVerticalPropertyMap(classMap.GetJoinedTable());
        }

    if (idProp == nullptr || relECClassIdProp == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    return propertyMap.Setup(relECClassIdProp->GetColumn(), relClassMap->GetRelationshipClass().GetId(), idProp->GetColumn());
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
BentleyStatus ClassMapper::DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation& collation, ECDbCR ecdb, ECPropertyCR ecProp, Utf8CP propAccessString)
    {
    columnName.clear();
    isNullable = true;
    isUnique = false;
    collation = DbColumn::Constraints::Collation::Default;
    ECDbPropertyMap customPropMap;
    if (ECDbMapCustomAttributeHelper::TryGetPropertyMap(customPropMap, ecProp))
        {
        if (!ecProp.GetIsPrimitive())
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                          "Failed to map ECProperty '%s:%s': only primitive ECProperties can have the custom attribute PropertyMap.",
                                                          ecProp.GetClass().GetFullName(), ecProp.GetName().c_str());
            return ERROR;
            }

        if (ECObjectsStatus::Success != customPropMap.TryGetColumnName(columnName))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsNullable(isNullable))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsUnique(isUnique))
            return ERROR;

        Utf8String collationStr;
        if (ECObjectsStatus::Success != customPropMap.TryGetCollation(collationStr))
            return ERROR;

        if (!DbColumn::Constraints::TryParseCollationString(collation, collationStr.c_str()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                          "Failed to map ECProperty '%s:%s': Custom attribute PropertyMap has an invalid value for the property 'Collation': %s",
                                                          ecProp.GetClass().GetFullName(), ecProp.GetName().c_str(),
                                                          collationStr.c_str());
            return ERROR;
            }
        }

    // PropertyMappingRule: if custom attribute PropertyMap does not supply a column name for an ECProperty, 
    // we use the ECProperty's propertyAccessString (and replace . by _)
    if (columnName.empty())
        {
        columnName.assign(propAccessString);
        columnName.ReplaceAll(".", "_");
        }

    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
DbColumn* ClassMapper::DoFindOrCreateColumnsInTable(ECPropertyCR ecProperty, Utf8CP accessString, DbColumn::Type colType)
    {
    Utf8String colName;
    bool isNullable = true;
    bool isUnique = false;
    DbColumn::Constraints::Collation collation = DbColumn::Constraints::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(colName, isNullable, isUnique, collation, m_classMap.GetDbMap().GetECDb(), ecProperty, accessString))
        return nullptr;

    DbColumn* col = m_classMap.GetColumnFactory().CreateColumn(ecProperty, accessString, colName.c_str(), colType, !isNullable, isUnique, collation);
    if (col == nullptr)
        {
        BeAssert(col != nullptr);
        return nullptr;
        }

    return col;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint2dPropertyMap> ClassMapper::MapPoint2dProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    if (property.GetType() != PRIMITIVETYPE_Point2d)
        return nullptr;

    const DbColumn* x = nullptr, *y = nullptr;
    Utf8String accessString = ComputeAccessString(property, parent);
    if (m_loadContext)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString((accessString + ".X").c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + ".Y").c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        }
    else
        {
        Utf8String columnName;
        bool isNullable = true;
        bool isUnique = false;
        DbColumn::Constraints::Collation collation = DbColumn::Constraints::Collation::Default;
        const DbColumn::Type colType = DbColumn::Type::Real;
        if (SUCCESS != DetermineColumnInfo(columnName, isNullable, isUnique, collation, m_classMap.GetDbMap().GetECDb(), property, accessString.c_str()))
            return nullptr;

        x = m_classMap.GetColumnFactory().CreateColumn(property, accessString.c_str(), (columnName + "_X").c_str(), colType, !isNullable, isUnique, collation);
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }


        y = m_classMap.GetColumnFactory().CreateColumn(property, accessString.c_str(), (columnName + "_Y").c_str(), colType, !isNullable, isUnique, collation);
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    RefCountedPtr<WipPoint2dPropertyMap> ptr;
    if (parent)
        ptr = WipPropertyMapFactory::CreatePoint2dPropertyMap(property, *parent, *x, *y);
    else
        ptr = WipPropertyMapFactory::CreatePoint2dPropertyMap(m_classMap, property, *x, *y);

    ptr->FinishEditing();
    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint3dPropertyMap> ClassMapper::MapPoint3dProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    if (property.GetType() != PRIMITIVETYPE_Point3d)
        return nullptr;

    const DbColumn *x = nullptr, *y = nullptr, *z = nullptr;
    Utf8String accessString = ComputeAccessString(property, parent);
    if (m_loadContext)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString((accessString + ".X").c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        x = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + ".Y").c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        y = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + ".Z").c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        z = columns->front();
        }
    else
        {
        Utf8String columnName;
        bool isNullable = true;
        bool isUnique = false;
        DbColumn::Constraints::Collation collation = DbColumn::Constraints::Collation::Default;
        const DbColumn::Type colType = DbColumn::Type::Real;
        if (SUCCESS != DetermineColumnInfo(columnName, isNullable, isUnique, collation, m_classMap.GetDbMap().GetECDb(), property, accessString.c_str()))
            return nullptr;

        x = m_classMap.GetColumnFactory().CreateColumn(property, accessString.c_str(), (columnName + "_X").c_str(), colType, !isNullable, isUnique, collation);
        if (x == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }


        y = m_classMap.GetColumnFactory().CreateColumn(property, accessString.c_str(), (columnName + "_Y").c_str(), colType, !isNullable, isUnique, collation);
        if (y == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }


        z = m_classMap.GetColumnFactory().CreateColumn(property, accessString.c_str(), (columnName + "_Z").c_str(), colType, !isNullable, isUnique, collation);
        if (z == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }
    RefCountedPtr<WipPoint3dPropertyMap> ptr;
    if (parent)
        ptr = WipPropertyMapFactory::CreatePoint3dPropertyMap(property, *parent, *x, *y, *z);
    else
        ptr = WipPropertyMapFactory::CreatePoint3dPropertyMap(m_classMap, property, *x, *y, *z);

    ptr->FinishEditing();
    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
Utf8String ClassMapper::ComputeAccessString(ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const* parent)
    {
    Utf8String accessString;
    if (parent)
        accessString.append(parent->GetAccessString()).append(".");

    accessString.append(ecProperty.GetName());
    return accessString;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipVerticalPropertyMap> ClassMapper::MapPrimitiveProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    if (property.GetType() == PRIMITIVETYPE_Point2d)
        return MapPoint2dProperty(property, parent);

    if (property.GetType() == PRIMITIVETYPE_Point3d)
        return MapPoint3dProperty(property, parent);

    DbColumn const*  column = nullptr;
    Utf8String accessString = ComputeAccessString(property, parent);
    if (m_loadContext)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString.c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }
    else
        {
        const DbColumn::Type colType = DbColumn::PrimitiveTypeToColumnType(property.GetType());
        column = DoFindOrCreateColumnsInTable(property, accessString.c_str(), colType);
        if (column == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    RefCountedPtr<WipPrimitivePropertyMap> ptr;
    if (parent)
        ptr = WipPropertyMapFactory::CreatePrimitivePropertyMap(property, *parent, *column);
    else
        ptr = WipPropertyMapFactory::CreatePrimitivePropertyMap(m_classMap, property, *column);

    ptr->FinishEditing();
    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitiveArrayPropertyMap> ClassMapper::MapPrimitiveArrayProperty(ECN::ArrayECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    Utf8String accessString = ComputeAccessString(property, parent);
    DbColumn const* column = nullptr;
    if (m_loadContext)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString.c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }
    else
        {
        column = DoFindOrCreateColumnsInTable(property, accessString.c_str(), DbColumn::Type::Blob);
        if (column == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    RefCountedPtr<WipPrimitiveArrayPropertyMap> ptr;
    if (parent)
        ptr = WipPropertyMapFactory::CreatePrimitiveArrayPropertyMap(property, *parent, *column);
    else
        ptr = WipPropertyMapFactory::CreatePrimitiveArrayPropertyMap(m_classMap, property, *column);

    ptr->FinishEditing();
    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructArrayPropertyMap> ClassMapper::MapStructArrayProperty(ECN::StructArrayECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    //TODO: Create column or map to existing column
    Utf8String accessString = ComputeAccessString(property, parent);
    DbColumn const* column = nullptr;
    if (m_loadContext)
        {
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString(accessString.c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        column = columns->front();
        }
    else
        {
        column = DoFindOrCreateColumnsInTable(property, accessString.c_str(), DbColumn::Type::Blob);
        if (column == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    RefCountedPtr<WipStructArrayPropertyMap> ptr;
    if (parent)
        ptr = WipPropertyMapFactory::CreateStructArrayPropertyMap(property, *parent, *column);
    else
        ptr = WipPropertyMapFactory::CreateStructArrayPropertyMap(m_classMap, property, *column);

    ptr->FinishEditing();
    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipNavigationPropertyMap> ClassMapper::MapNavigationProperty(ECN::NavigationECPropertyCR property)
    {
    DbColumn const* column = nullptr;
    Utf8String accessString = property.GetName();
    RefCountedPtr<WipNavigationPropertyMap> propertyMap = WipPropertyMapFactory::CreateNavigationPropertyMap(m_classMap, property);
    if (m_loadContext)
        {
        const DbColumn *id = nullptr, *relClassId = nullptr;
        std::vector<DbColumn const*> const* columns;
        columns = m_loadContext->FindColumnByAccessString((accessString + "." + ECDbSystemSchemaHelper::ID_PROPNAME).c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        id = columns->front();
        columns = m_loadContext->FindColumnByAccessString((accessString + "." + ECDbSystemSchemaHelper::RELECCLASSID_PROPNAME).c_str());
        if (columns == nullptr || columns->size() != 1)
            {
            BeAssert(false);
            return nullptr;
            }

        relClassId = columns->front();

        if (propertyMap->Setup(*relClassId, property.GetRelationshipClass()->GetId(), *id) != SUCCESS)
            {
            BeAssert(false);
            return nullptr;
            }
        }

    return propertyMap;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructPropertyMap> ClassMapper::MapStructProperty(ECN::StructECPropertyCR property, WipVerticalPropertyMap const* parent)
    {
    RefCountedPtr<WipStructPropertyMap> structPropertyMap;
    if (parent)
        structPropertyMap = WipPropertyMapFactory::CreateStructPropertyMap(property, *parent);
    else
        structPropertyMap = WipPropertyMapFactory::CreateStructPropertyMap(m_classMap, property);

    for (ECN::ECPropertyCP property : property.GetType().GetProperties())
        {
        RefCountedPtr<WipVerticalPropertyMap> propertyMap;
        if (auto typedProperty = property->GetAsPrimitiveProperty())
            propertyMap = MapPrimitiveProperty(*typedProperty, structPropertyMap.get());
        else if (auto typedProperty = property->GetAsArrayProperty())
            propertyMap = MapPrimitiveArrayProperty(*typedProperty, structPropertyMap.get());
        else if (auto typedProperty = property->GetAsStructProperty())
            propertyMap = MapStructProperty(*typedProperty, structPropertyMap.get());
        else if (auto typedProperty = property->GetAsStructArrayProperty())
            propertyMap = MapStructArrayProperty(*typedProperty, structPropertyMap.get());
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

        if (propertyMap->InEditMode())
            {
            BeAssert(false && "Child property map must not be in edit mode");
            return nullptr;
            }

        if (structPropertyMap->Insert(propertyMap) != SUCCESS)
            {
            BeAssert(false && "Failed to insert property map");
            return nullptr;
            }
        }

    BeAssert(!structPropertyMap->empty());
    if (structPropertyMap->empty())
        {
        return nullptr;
        }

    structPropertyMap->FinishEditing();
    return structPropertyMap;
    }



//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
WipPropertyMap* ClassMapper::ProcessProperty(ECPropertyCR property)
    {
    RefCountedPtr<WipPropertyMap> propertyMap;
    if (m_classMap.GetPropertyMaps().Find(property.GetName().c_str()))
        {
        BeAssert(false && "PropertyMap already exist");
        return nullptr;
        }

    if (auto typedProperty = property.GetAsPrimitiveProperty())
        propertyMap = MapPrimitiveProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsArrayProperty())
        propertyMap = MapPrimitiveArrayProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructProperty())
        propertyMap = MapStructProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsStructArrayProperty())
        propertyMap = MapStructArrayProperty(*typedProperty, nullptr);
    else if (auto typedProperty = property.GetAsNavigationProperty())
        propertyMap = MapNavigationProperty(*typedProperty);
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


    if (m_classMap.GetPropertyMapsR().Insert(propertyMap) != SUCCESS)
        {
        BeAssert(false && "Failed to insert property map");
        return nullptr;
        }

    return propertyMap.get();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
