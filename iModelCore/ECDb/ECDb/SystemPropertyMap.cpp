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
SystemPropertyMap::SystemPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
    : PropertyMap(kind, classMap, ecProperty)
    {
    BeAssert(ecProperty.GetType() == ECN::PrimitiveType::PRIMITIVETYPE_Long);
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
BentleyStatus SystemPropertyMap::Init(std::vector<DbColumn const*> const& columns)
    {
    if (!InEditMode())
        return ERROR;

    if (columns.empty())
        {
        BeAssert(false && "Columns cannot be empty");
        return ERROR;
        }

    bset<DbTable const*> doneList;
    for (DbColumn const* column : columns)
        {
        if (column->GetType() != DbColumn::Type::Integer || doneList.find(&column->GetTable()) != doneList.end())
            {
            BeAssert(false && "System column must have type integer and column must be unique per table");
            m_dataPropMaps.clear();
            m_tables.clear();
            m_dataPropMapList.clear();
            return ERROR;
            }

        doneList.insert(&column->GetTable());
        auto prop = PrimitivePropertyMap::CreateInstance(*GetProperty().GetAsPrimitiveProperty(),*this, *column);
        if (prop == nullptr)
            {
            BeAssert(false && "Failed to create property Map");
            m_dataPropMaps.clear();
            m_tables.clear();
            m_dataPropMapList.clear();
            return ERROR;
            }

        PropertyMap::OverrideAccessString(*prop, GetAccessString());
        m_dataPropMaps[prop->GetTable().GetName().c_str()] = prop;
        m_tables.push_back(&prop->GetTable());
        m_dataPropMapList.push_back(prop.get());
        }
    
    FinishEditing();
    return SUCCESS;
    }

//************************************ECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ECInstanceIdPropertyMap> ECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), ECSqlSystemProperty::ECInstanceId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ECInstanceIdPropertyMap> systemPropertyMap =  new ECInstanceIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty());
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }

//************************************ECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ECClassIdPropertyMap> ECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), ECSqlSystemProperty::ECClassId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ECClassIdPropertyMap> systemPropertyMap = new ECClassIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty(), defaultEClassId);
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }

//************************************ConstraintECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ConstraintECClassIdPropertyMap> ConstraintECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> systemPropertyMap = new ConstraintECClassIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty(), defaultEClassId, constraintType);
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }

//************************************ConstraintECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<ConstraintECInstanceIdPropertyMap> ConstraintECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap> systemPropertyMap = new ConstraintECInstanceIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty(), constraintType);
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
