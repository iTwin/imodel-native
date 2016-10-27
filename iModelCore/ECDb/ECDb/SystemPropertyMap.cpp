/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************SystemPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
SystemPropertyMap::SystemPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<PrimitivePropertyMap>> const& maps)
    : PropertyMap(kind, classMap, ecProperty)
    {
    BeAssert(ecProperty.GetType() == ECN::PrimitiveType::PRIMITIVETYPE_Long);

    for (RefCountedPtr<PrimitivePropertyMap> const& map : maps)
        {
        if (m_dataPropMaps.find(map->GetTable().GetName().c_str()) != m_dataPropMaps.end())
            {
            BeAssert(false && "PropertyMap must be one per table");
            m_dataPropMaps.clear();
            m_dataPropMapList.clear();
            return;
            }

        m_dataPropMaps[map->GetTable().GetName().c_str()] = map;
        m_tables.push_back(&map->GetTable());
        m_dataPropMapList.push_back(map.get());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SystemPropertyMap::_Validate() const
    {
    BeAssert(!m_dataPropMaps.empty());
    if (m_dataPropMaps.empty())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const* SystemPropertyMap::FindDataPropertyMap(Utf8CP tableName) const
    {
    auto itor = m_dataPropMaps.find(tableName);
    if (itor != m_dataPropMaps.end())
        return itor->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
BentleyStatus SystemPropertyMap::TryCreateDataPropertyMaps(std::vector<RefCountedPtr<PrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    ECDbSchemaManagerCR schemaManger = classMap.GetDbMap().GetECDb().Schemas();
    ECPropertyCP ecProperty = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, systemProperty);
    if (ecProperty == nullptr)
        {
        BeAssert(false && "Failed to find system property");
        return ERROR;
        }
    PrimitiveECPropertyCP primtiveECProp = ecProperty->GetAsPrimitiveProperty();
    if (primtiveECProp == nullptr || primtiveECProp->GetType() != PRIMITIVETYPE_Long)
        {
        BeAssert(false && "System property must correspond to a primitive property of type long");
        return ERROR;
        }

    if (columns.empty())
        {
        BeAssert(false && "Columns cannot be empty");
        return ERROR;
        }

    bset<DbTable const*> doneList;
    for (DbColumn const* column : columns)
        {
        if (column->GetType() != DbColumn::Type::Integer)
            {
            BeAssert(false && "System column must have type integer");
            return ERROR;
            }

        if (doneList.find(&column->GetTable()) != doneList.end())
            {
            BeAssert(false && "Column must unique per table");
            return ERROR;
            }

        doneList.insert(&column->GetTable());
        auto prop = PrimitivePropertyMap::CreateInstance(classMap, *primtiveECProp, *column);
        if (prop == nullptr)
            {
            BeAssert(false && "Failed to create property Map");
            return ERROR;
            }

        propertyMaps.push_back(prop);
        }

    return SUCCESS;
    }

//************************************ECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ECInstanceIdPropertyMap> ECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<PrimitivePropertyMap>> propertyMaps;
    if (TryCreateDataPropertyMaps(propertyMaps, ECSqlSystemProperty::ECInstanceId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new ECInstanceIdPropertyMap(classMap, *systemProperty, propertyMaps);
    }

//************************************ECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ECClassIdPropertyMap> ECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<PrimitivePropertyMap>> propertyMaps;
    if (TryCreateDataPropertyMaps(propertyMaps, ECSqlSystemProperty::ECClassId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new ECClassIdPropertyMap(classMap, *systemProperty, propertyMaps, defaultEClassId);
    }

//************************************ConstraintECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ConstraintECClassIdPropertyMap> ConstraintECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<PrimitivePropertyMap>> propertyMaps;
    if (TryCreateDataPropertyMaps(propertyMaps, constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new ConstraintECClassIdPropertyMap(classMap, *systemProperty, propertyMaps, defaultEClassId, constraintType);
    }

//************************************ConstraintECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<ConstraintECInstanceIdPropertyMap> ConstraintECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<PrimitivePropertyMap>> propertyMaps;
    if (TryCreateDataPropertyMaps(propertyMaps, constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new ConstraintECInstanceIdPropertyMap(classMap, *systemProperty, propertyMaps, constraintType);
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
