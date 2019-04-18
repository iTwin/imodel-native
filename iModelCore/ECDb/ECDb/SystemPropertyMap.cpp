/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************SystemPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
SystemPropertyMap::SystemPropertyMap(Type kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
    : PropertyMap(kind, classMap, ecProperty)
    {
    BeAssert(ecProperty.GetType() == ECN::PrimitiveType::PRIMITIVETYPE_Long);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
SystemPropertyMap::PerTableIdPropertyMap const* SystemPropertyMap::FindDataPropertyMap(Utf8CP tableName) const
    {
    auto itor = m_dataPropMaps.find(tableName);
    if (itor != m_dataPropMaps.end())
        return itor->second.get();

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
SystemPropertyMap::PerTableIdPropertyMap const* SystemPropertyMap::FindDataPropertyMap(ClassMap const& classMap) const
    {
    PerTableIdPropertyMap const* last = nullptr;
    for (DbTable const* table : classMap.GetTables())
        {
        if (PerTableIdPropertyMap const* current = FindDataPropertyMap(*table))
            {
            if (last)
                {
                BeAssert(false && "DataProperty must belong to one of the table else its a programmer error");
                return nullptr;
                }
            else
                last = current;
            }
        }

    return last;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
BentleyStatus SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(SystemPropertyMap& propertyMap, DbColumn const& column)
    {
    return propertyMap.Init({&column}, true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SystemPropertyMap::Init(std::vector<DbColumn const*> const& columns, bool appendMode)
    {
    if (columns.empty())
    {
    BeAssert(false && "Columns cannot be empty");
    return ERROR;
    }

    bset<DbTable const*> doneList;
    if (appendMode)
        doneList.insert(begin(m_tables), end(m_tables));

    for (DbColumn const* column : columns)
        {
        if ((column->GetType() != DbColumn::Type::Integer && column->GetType() != DbColumn::Type::Any) || doneList.find(&column->GetTable()) != doneList.end())
            {
            BeAssert(false && "System column must have type integer or 'Any' and column must be unique per table");
            m_dataPropMaps.clear();
            m_tables.clear();
            m_dataPropMapList.clear();
            return ERROR;
            }

        doneList.insert(&column->GetTable());
        RefCountedPtr<PerTableIdPropertyMap> propMap = _CreatePerTablePropertyMap(*this, *GetProperty().GetAsPrimitiveProperty(), *column);
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create property Map");
            m_dataPropMaps.clear();
            m_tables.clear();
            m_dataPropMapList.clear();
            return ERROR;
            }

        m_dataPropMaps[propMap->GetTable().GetName().c_str()] = propMap;
        m_tables.push_back(&propMap->GetTable());
        m_dataPropMapList.push_back(propMap.get());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      10/16
//---------------------------------------------------------------------------------------
bool SystemPropertyMap::_IsMappedToTable(DbTable const& table) const
    {
    for (DbTable const* t : m_tables)
        if (t == &table)
            return true;

    return false;
    }


//************************************SystemPropertyMap::PerTablePrimitivePropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      10/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<SystemPropertyMap::PerTableIdPropertyMap> SystemPropertyMap::PerTableIdPropertyMap::CreateInstance(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    if (ecProperty.GetType() != PRIMITIVETYPE_Long)
        {
        BeAssert(false && "SystemPropertyMap's child prop map must always be of data type PRIMITIVETYPE_Long");
        return nullptr;
        }

    return new PerTableIdPropertyMap(parentPropertyMap, ecProperty, column);
    }

//************************************SystemPropertyMap::PerTableClassIdPropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      10/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<SystemPropertyMap::PerTableClassIdPropertyMap> SystemPropertyMap::PerTableClassIdPropertyMap::CreateInstance(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
    {
    if (ecProperty.GetType() != PRIMITIVETYPE_Long)
        {
        BeAssert(false && "SystemPropertyMap's child prop map must always be of data type PRIMITIVETYPE_Long");
        return nullptr;
        }

    return new PerTableClassIdPropertyMap(parentPropertyMap, ecProperty, column, defaultClassId);
    }

//************************************ECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ECInstanceIdPropertyMap> ECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = classMap.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::ECInstanceId());
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
RefCountedPtr<ECClassIdPropertyMap> ECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = classMap.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::ECClassId());
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ECClassIdPropertyMap> systemPropertyMap = new ECClassIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty());
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle      03/17
//---------------------------------------------------------------------------------------
RefCountedPtr<SystemPropertyMap::PerTableIdPropertyMap> ECClassIdPropertyMap::_CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR prop, DbColumn const& col) const
    {
    ECClassId defaultClassId;
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        ClassMap const& classMap = parentPropMap.GetClassMap();
        //For end table relationships the exclusive root class of a table is not the relationship class, but the constraint class
        //of the relationship. In that case we can safely use the rel class' id as the table will not contain subclasses of it
        //(because otherwise the column wouldn't be virtual)
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable || !col.GetTable().HasExclusiveRootECClass())
            defaultClassId = classMap.GetClass().GetId();
        else
            defaultClassId = col.GetTable().GetExclusiveRootECClassId();
        }

    return PerTableClassIdPropertyMap::CreateInstance(parentPropMap, prop, col, defaultClassId);
    }

//************************************ConstraintECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<ConstraintECClassIdPropertyMap> ConstraintECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = classMap.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECClassId() : ECSqlSystemPropertyInfo::TargetECClassId());
    if (systemProperty == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> systemPropertyMap = new ConstraintECClassIdPropertyMap(classMap, *systemProperty->GetAsPrimitiveProperty(), constraintType);
    if (SUCCESS != systemPropertyMap->Init(columns))
        return nullptr;

    return systemPropertyMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle      03/17
//---------------------------------------------------------------------------------------
RefCountedPtr<SystemPropertyMap::PerTableIdPropertyMap> ConstraintECClassIdPropertyMap::_CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR prop, DbColumn const& col) const
    {
    ECClassId defaultClassId;
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(col.GetTable().HasExclusiveRootECClass());
        defaultClassId = col.GetTable().GetExclusiveRootECClassId();
        }

    return PerTableClassIdPropertyMap::CreateInstance(parentPropMap, prop, col, defaultClassId);
    }

//************************************ConstraintECInstanceIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<ConstraintECInstanceIdPropertyMap> ConstraintECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns)
    {
    ECPropertyCP systemProperty = classMap.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(constraintType == ECRelationshipEnd::ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECInstanceId() : ECSqlSystemPropertyInfo::TargetECInstanceId());
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
