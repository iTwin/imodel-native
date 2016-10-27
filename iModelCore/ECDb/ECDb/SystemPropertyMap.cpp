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
    BeAssert(!m_vmapsPerTable.empty());
    if (m_vmapsPerTable.empty())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const* SystemPropertyMap::FindVerticalPropertyMap(Utf8CP tableName) const
    {
    auto itor = m_vmapsPerTable.find(tableName);
    if (itor != m_vmapsPerTable.end())
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
        {
        return ERROR;
        }

    std::vector<RefCountedPtr<PrimitivePropertyMap>> propertyMaps;
    PrimitiveECPropertyCP primtiveECProp = GetProperty().GetAsPrimitiveProperty();

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
            m_vmapsPerTable.clear();
            m_tables.clear();
            m_vmaps.clear();
            return ERROR;
            }

        if (doneList.find(&column->GetTable()) != doneList.end())
            {
            BeAssert(false && "Column must unique per table");
            m_vmapsPerTable.clear();
            m_tables.clear();
            m_vmaps.clear();
            return ERROR;
            }

        doneList.insert(&column->GetTable());
        auto prop = PrimitivePropertyMap::CreateInstance(*primtiveECProp,*this, *column);
        if (prop == nullptr)
            {
            BeAssert(false && "Failed to create property Map");
            m_vmapsPerTable.clear();
            m_tables.clear();
            m_vmaps.clear();
            return ERROR;
            }

        m_vmapsPerTable[prop->GetTable().GetName().c_str()] = prop;
        m_tables.push_back(&prop->GetTable());
        m_vmaps.push_back(prop.get());
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

    auto systemPropertyMap =  new ECInstanceIdPropertyMap(Kind::ECInstanceId, classMap, *systemProperty->GetAsPrimitiveProperty());
    if (systemPropertyMap->Init(columns) == ERROR)
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

    auto systemPropertyMap = new ECClassIdPropertyMap(Kind::ECClassId, classMap, *systemProperty->GetAsPrimitiveProperty(), defaultEClassId);
    if (systemPropertyMap->Init(columns) == ERROR)
        return nullptr;

    return systemPropertyMap;
    }

//************************************ConstraintECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ConstraintECClassIdPropertyMap> ConstraintECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    auto systemPropertyMap = new ConstraintECClassIdPropertyMap(Kind::ConstraintECClassId, classMap, *systemProperty->GetAsPrimitiveProperty(), defaultEClassId, constraintType);
    if (systemPropertyMap->Init(columns) == ERROR)
        return nullptr;

    return systemPropertyMap;
    }

//************************************ConstraintECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<ConstraintECInstanceIdPropertyMap> ConstraintECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = ECDbSystemSchemaHelper::GetSystemProperty(classMap.GetDbMap().GetECDb().Schemas(), constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId);
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    auto systemPropertyMap = new ConstraintECInstanceIdPropertyMap(Kind::ConstraintECInstanceId, classMap, *systemProperty->GetAsPrimitiveProperty(), constraintType);
    if (systemPropertyMap->Init(columns) == ERROR)
        return nullptr;

    return systemPropertyMap;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
