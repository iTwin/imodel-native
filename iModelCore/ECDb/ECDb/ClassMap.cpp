/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define CURRENTIMESTAMP_SQLEXP "julianday('now')"

//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, Type type, ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy)
    : m_type(type), m_ecdb(ecdb), m_schemaManager(manager), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy), m_propertyMaps(*this), m_state(ObjectState::New)
    {
    if (m_mapStrategyExtInfo.IsTablePerHierarchy())
        m_tphHelper = std::make_unique<TablePerHierarchyHelper>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
DbTable& ClassMap::GetPrimaryTable() const
    {
    if (GetType() == Type::RelationshipEndTable)
        return *m_tables.front();

    for (DbTable* table : GetTables())
        {
        DbTable::Type tableType = table->GetType();
        if (tableType == DbTable::Type::Primary || tableType == DbTable::Type::Existing || tableType == DbTable::Type::Virtual)
            return *table;
        }

    BeAssert(false);
    DbTable* nulltable = nullptr;
    return *nulltable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
DbTable& ClassMap::GetJoinedOrPrimaryTable() const
    {
    DbTable* joinedTable = nullptr;
    DbTable* primaryTable = nullptr;
    for (DbTable* table : m_tables)
        {
        DbTable::Type type = table->GetType();

        if (type == DbTable::Type::Joined)
            joinedTable = table;
        else if (type == DbTable::Type::Primary || type == DbTable::Type::Existing || type == DbTable::Type::Virtual)
            primaryTable = table;

        if (joinedTable != nullptr)
            return *joinedTable;
        }

    BeAssert(primaryTable != nullptr);
    return *primaryTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
DbTable* ClassMap::GetOverflowTable() const
    {
    for (DbTable* table : GetTables())
        {
        if (table->GetType() == DbTable::Type::Overflow)
            return table;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::_Map(ClassMappingContext& ctx)
    {
    ClassMappingStatus stat = DoMapPart1(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    return DoMapPart2(ctx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::DoMapPart1(ClassMappingContext& ctx)
    {
    if (SUCCESS != DbMappingManager::Tables::MapToTable(ctx.GetImportCtx(), *this, ctx.GetClassMappingInfo()))
        return ClassMappingStatus::Error;
    
    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::DoMapPart2(ClassMappingContext& ctx)
    {
    ImportColumnResolutionScope columnResolutionScope(*this);
    ClassMappingStatus stat = MapProperties(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    PrimitiveECPropertyCP currentTimeStampProp = ctx.GetClassMappingInfo().GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return ClassMappingStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = m_mapStrategyExtInfo.GetTphInfo().IsValid() && m_mapStrategyExtInfo.GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
    if (!isJoinedTable)
        return ClassMappingStatus::Success;

    ClassMap const* tphBaseClassMap = ctx.GetClassMappingInfo().GetTphBaseClassMap();
    if (tphBaseClassMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    DbTable const& baseClassMapJoinedTable = tphBaseClassMap->GetJoinedOrPrimaryTable();
    if (&baseClassMapJoinedTable == &GetJoinedOrPrimaryTable())
        return ClassMappingStatus::Success;

    DbColumn const* primaryKeyColumn = baseClassMapJoinedTable.FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* foreignKeyColumn = GetJoinedOrPrimaryTable().FindFirst(DbColumn::Kind::ECInstanceId);
    PRECONDITION(primaryKeyColumn != nullptr, ClassMappingStatus::Error);
    PRECONDITION(foreignKeyColumn != nullptr, ClassMappingStatus::Error);
    bool createFKConstraint = true;
    for (DbConstraint const* constraint : GetJoinedOrPrimaryTable().GetConstraints())
        {
        if (constraint->GetType() != DbConstraint::Type::ForeignKey)
            continue;

        ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
        if (&fk->GetReferencedTable() == &baseClassMapJoinedTable &&
            fk->GetFkColumns().front() == foreignKeyColumn && fk->GetReferencedTableColumns().front() == primaryKeyColumn)
            {
            createFKConstraint = false;
            break;
            }
        }

    if (createFKConstraint)
        {
        if (GetJoinedOrPrimaryTable().AddForeignKeyConstraint(*foreignKeyColumn, *primaryKeyColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified) == nullptr)
            return ClassMappingStatus::Error;
        }

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::CreateCurrentTimeStampTrigger(PrimitiveECPropertyCR currentTimeStampProp)
    {
    if (currentTimeStampProp.GetType() != PRIMITIVETYPE_DateTime)
        {
        BeAssert(false);
        return ERROR;
        }

    PropertyMap const* propMap = GetPropertyMaps().Find(currentTimeStampProp.GetName().c_str());
    if (propMap == nullptr)
        return SUCCESS;

    DbColumn& currentTimeStampColumn = const_cast<DbColumn&>(propMap->GetAs<PrimitivePropertyMap>().GetColumn());
    if (currentTimeStampColumn.IsShared())
        {
        LOG.warningv("ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
                   "ECDb therefore does not create the current timestamp trigger for this property.",
                           currentTimeStampProp.GetName().c_str(), currentTimeStampProp.GetClass().GetFullName());

        return SUCCESS;
        }

    BeAssert(currentTimeStampColumn.GetType() == DbColumn::Type::TimeStamp);
    currentTimeStampColumn.GetConstraintsR().SetDefaultValueExpression(CURRENTIMESTAMP_SQLEXP);
    currentTimeStampColumn.GetConstraintsR().SetNotNullConstraint();

    ECInstanceIdPropertyMap const* idPropMap = GetECInstanceIdPropertyMap();
    if (idPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable& table = currentTimeStampColumn.GetTableR();
    Utf8CP tableName = table.GetName().c_str();
    Utf8CP instanceIdColName = idPropMap->FindDataPropertyMap(tableName)->GetColumn().GetName().c_str();
    Utf8CP currentTimeStampColName = currentTimeStampColumn.GetName().c_str();
     
    Utf8String triggerName;
    //triggerName.Sprintf("%s_%s_SetCurrentTimeStamp", tableName, currentTimeStampColName);
    triggerName.Sprintf("%s_CurrentTimeStamp", tableName);
    Utf8String body;
    body.Sprintf("BEGIN UPDATE [%s] SET [%s]=" CURRENTIMESTAMP_SQLEXP " WHERE [%s]=new.[%s]; END", tableName, currentTimeStampColName, instanceIdColName, instanceIdColName);

    Utf8String whenCondition;
    whenCondition.Sprintf("old.[%s]=new.[%s] AND old.[%s]!=" CURRENTIMESTAMP_SQLEXP, currentTimeStampColName, currentTimeStampColName, currentTimeStampColName);

    return table.AddTrigger(triggerName, DbTrigger::Type::After, whenCondition.c_str(), body.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan           06/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::AddOrUpdateTableList(DataPropertyMap const& propertyThatIsNotYetAdded)
    {
    if (propertyThatIsNotYetAdded.GetType() == PropertyMap::Type::Navigation)
        {
        NavigationPropertyMap const& navPropertyMap = propertyThatIsNotYetAdded.GetAs<NavigationPropertyMap>();
        if (!navPropertyMap.IsComplete())
            return SUCCESS;
        }

    DbTable& propertyTable = const_cast< DbTable&>(propertyThatIsNotYetAdded.GetTable());
    if (std::find(m_tables.begin(), m_tables.end(), &propertyTable) == m_tables.end())
        {
        if (propertyTable.GetType() == DbTable::Type::Overflow)
            return SetOverflowTable(propertyTable);

        AddTable(propertyTable);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::MapProperties(ClassMappingContext& ctx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    DbMappingManager::Classes::PropertyMapInheritanceMode inheritanceMode = DbMappingManager::Classes::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetSchemaManager().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ClassMappingStatus::Error;
                }

            if (baseClassMap->GetMapStrategy().IsTablePerHierarchy())
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }

    std::vector<ECPropertyCP> propertiesToMap;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (&property->GetClass() == &m_ecClass ||
            inheritanceMode == DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
            {
            //Property map that should not be inherited -> must be mapped
            propertiesToMap.push_back(property);
            continue;
            }

        //look for the base class' property map as property is inherited
        DataPropertyMap const* baseClassPropMap = nullptr;
        for (ClassMap const* baseClassMap : tphBaseClassMaps)
            {
            PropertyMap const* baseClassPropMapRaw = baseClassMap->GetPropertyMaps().Find(property->GetName().c_str());
            if (baseClassPropMapRaw != nullptr && baseClassPropMapRaw->IsData())
                {
                baseClassPropMap = &baseClassPropMapRaw->GetAs<DataPropertyMap>();
                break;
                }
            }

        if (baseClassPropMap == nullptr)
            {
            //Property is inherited, but not from a TPH base class, so we have to map the property from scratch
            propertiesToMap.push_back(property);
            continue;
            }

        if (baseClassPropMap->IsData())
            {
            if (AddOrUpdateTableList(baseClassPropMap->GetAs<DataPropertyMap>()) != SUCCESS)
                return ClassMappingStatus::Error;
            }

        RefCountedPtr<DataPropertyMap> propertyMap = PropertyMapCopier::CreateCopy(*baseClassPropMap, *this);        
        if (propertyMap == nullptr || SUCCESS != m_propertyMaps.Insert(propertyMap))
            return ClassMappingStatus::Error;

        if (propertyMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus stat = DbMappingManager::Classes::MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (ClassMappingStatus::Success != stat)
                    return stat;
                }
            }
        }

    for (ECPropertyCP property : propertiesToMap)
        {
        PropertyMap* propMap = DbMappingManager::Classes::MapProperty(ctx.GetImportCtx(), *this, *property);
        if (propMap == nullptr)
            return ClassMappingStatus::Error;

        if (property->GetIsNavigation())
            {
            //WIP_CLEANUP code redundant to a few lines above.
            NavigationPropertyMap const& navPropertyMap = propMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus stat = DbMappingManager::Classes::MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (ClassMappingStatus::Success != stat)
                    return stat;
                }
            }
        }

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Save(SchemaImportContext& importCtx, DbMapSaveContext& ctx)
    {
    if (ctx.IsAlreadySaved(*this) || !importCtx.ClassMapNeedsSaving(m_ecClass.GetId()))
        return SUCCESS;

    ctx.BeginSaving(*this);
    if (GetClass().HasBaseClasses())
        {
        for (ECClassCP baseClass : GetClass().GetBaseClasses())
            {
            ClassMap* baseClassMap = const_cast<ClassMap*>(GetSchemaManager().GetClassMap(*baseClass));
            if (baseClassMap == nullptr)
                {
                BeAssert(false && "Failed to find baseClass map");
                return ERROR;
                }

            if (SUCCESS != baseClassMap->Save(importCtx, ctx))
                return ERROR;
            }
        }

    if (ctx.InsertClassMap(GetClass().GetId(), GetMapStrategy()) != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    DbClassMapSaveContext classMapSaveContext(ctx);
    SavePropertyMapVisitor saveVisitor(classMapSaveContext);
    for (PropertyMap const* propertyMap : GetPropertyMaps())
        {
        BeAssert(GetClass().GetId() == propertyMap->GetClassMap().GetClass().GetId());
        if (SUCCESS != propertyMap->AcceptVisitor(saveVisitor))
            return ERROR;
        }

    ctx.EndSaving(*this);
    m_state = ObjectState::Persisted;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx)
    {
    m_state = ObjectState::Persisted;
    std::set<DbTable*> primaryTables;
    std::set<DbTable*> joinedTables;
    std::set<DbTable*> overflowTables;
    if (!dbLoadCtx.HasMappedProperties())
        {
        SetTable(*const_cast<DbTable*>(GetSchemaManager().GetDbSchema().GetNullTable()));
        return SUCCESS;
        }

    for (std::pair<Utf8String, std::vector<DbColumn const*>> const& propMapping : dbLoadCtx.GetPropertyMaps())
        {
        std::vector<DbColumn const*> const& columns = propMapping.second;
        for (DbColumn const* column : columns)
            {
            if (column->GetTable().GetType() == DbTable::Type::Joined)
                joinedTables.insert(&column->GetTableR());
            else if (column->GetTable().GetType() == DbTable::Type::Overflow)
                overflowTables.insert(&column->GetTableR());
            else if (column->GetKind() != DbColumn::Kind::ECClassId)
                primaryTables.insert(&column->GetTableR());
            }
        }
    //Orderly add the tables
    for (DbTable* table : primaryTables)
        AddTable(*table);

    for (DbTable* table : joinedTables)
        AddTable(*table);

    for (DbTable* table : overflowTables)
        AddTable(*table);

    BeAssert(!GetTables().empty());

    if (GetECInstanceIdPropertyMap() != nullptr)
        return ERROR;

    std::vector<DbColumn const*> const* mapColumnsList = dbLoadCtx.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_ECInstanceId));
    if (mapColumnsList == nullptr)
        return ERROR;

    //Load ECInstanceId================================================
    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, *mapColumnsList);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
        return ERROR;

    mapColumnsList = dbLoadCtx.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_ECClassId));
    if (mapColumnsList == nullptr)
        return ERROR;

    //Load ECClassId   ================================================
    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, *mapColumnsList);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecClassIdPropertyMap, 1) != SUCCESS)
        return ERROR;

    return LoadPropertyMaps(ctx, dbLoadCtx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  09/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::LoadPropertyMaps(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbCtx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    DbMappingManager::Classes::PropertyMapInheritanceMode inheritanceMode = DbMappingManager::Classes::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetSchemaManager().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                return ERROR;

            if (baseClassMap->GetMapStrategy().IsTablePerHierarchy())
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }
   
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        DataPropertyMap const*  tphBaseClassPropMap = nullptr;
        if (&property->GetClass() != &m_ecClass && inheritanceMode == DbMappingManager::Classes::PropertyMapInheritanceMode::Clone)
            {
            for (ClassMap const* baseClassMap : tphBaseClassMaps)
                {
                PropertyMap const* propMap = baseClassMap->GetPropertyMaps().Find(property->GetName().c_str());
                if (propMap != nullptr && propMap->IsData())
                    {
                    tphBaseClassPropMap = &propMap->GetAs<DataPropertyMap>();
                    break;
                    }
                }
            }

        if (tphBaseClassPropMap == nullptr)
            {
            if (DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property) == nullptr)
                m_failedToLoadProperties.push_back(property);

            continue;
            }

        RefCountedPtr<PropertyMap> propMap = PropertyMapCopier::CreateCopy(*tphBaseClassPropMap, *this);
        if (propMap == nullptr)
            return ERROR;

        if (m_propertyMaps.Insert(propMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan  01/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Update(SchemaImportContext& ctx)
    {
    //Follow can change ECInstanceId, ECClassId by optionally adding 
    if (m_failedToLoadProperties.empty())
        return SUCCESS;

    BeAssert(m_state == ObjectState::Persisted);
    m_state = ObjectState::Modified;

    UpdateColumnResolutionScope columnResolutionScope(*this);
    for (ECPropertyCP property : m_failedToLoadProperties)
        {
        PropertyMap const* propMap = DbMappingManager::Classes::MapProperty(ctx, *this, *property);
        if (propMap == nullptr)
            return ERROR;

        if (!propMap->IsData())
            {
            BeAssert(false);
            return ERROR;
            }

        //Nav property maps cannot be saved here as they are not yet mapped.
        if (propMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap& navPropMap = const_cast<NavigationPropertyMap&>(propMap->GetAs<NavigationPropertyMap>());
            if (ClassMappingStatus::Success != DbMappingManager::Classes::MapNavigationProperty(ctx, navPropMap))
                return ERROR;
            }

        //! ECSchema update added new property for which we need to save property map
        DbMapSaveContext ctx(m_ecdb);
        if (GetSchemaManager().GetDbSchema().UpdateTable(propMap->GetAs<DataPropertyMap>().GetTable()) != SUCCESS)
            {
            BeAssert(false && "Failed to save table");
            return ERROR;
            }

        ctx.BeginSaving(*this);
        DbClassMapSaveContext classMapContext(ctx);
        SavePropertyMapVisitor saveVisitor(classMapContext);
        propMap->AcceptVisitor(saveVisitor);
        ctx.EndSaving(*this);
        }

    m_failedToLoadProperties.clear();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECInstanceIdPropertyMap const* ClassMap::GetECInstanceIdPropertyMap() const
    {
    PropertyMap const* propMap = GetPropertyMaps().Find(ECDBSYS_PROP_ECInstanceId);
    if (propMap == nullptr)
        return nullptr;

    return &propMap->GetAs<ECInstanceIdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECClassIdPropertyMap const* ClassMap::GetECClassIdPropertyMap() const
    {
    PropertyMap const* propMap = GetPropertyMaps().Find(ECDBSYS_PROP_ECClassId);
    if (propMap == nullptr)
        return nullptr;

    return &propMap->GetAs<ECClassIdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
IssueReporter const& ClassMap::Issues() const { return m_ecdb.GetImpl().Issues(); }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription const& ClassMap::GetStorageDescription() const { return GetSchemaManager().GetLightweightCache().GetStorageDescription(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
Nullable<std::vector<ClassMap const*>> ClassMap::GetDerivedClassMaps() const
    {
    ECDerivedClassesList const* derivedClasses = m_ecdb.Schemas().GetDerivedClassesInternal(GetClass());
    if (derivedClasses == nullptr)
        return nullptr;

    std::vector<ClassMap const*> derivedClassMaps;
    for (ECClassCP derivedClass : *derivedClasses)
        {
        if (ClassMap const* derivedClassMap = GetSchemaManager().GetClassMap(*derivedClass))
            derivedClassMaps.push_back(derivedClassMap);
        }

    return derivedClassMaps;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       04 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMap::SetOverflowTable(DbTable& overflowTable)
    {
    if (GetOverflowTable())
        return SUCCESS;

    if (!GetMapStrategy().IsTablePerHierarchy())
        {
        BeAssert(GetMapStrategy().IsTablePerHierarchy());
        return ERROR;
        }

    ECInstanceIdPropertyMap* ecInstanceIdPropertyMap = const_cast<ECInstanceIdPropertyMap*>(GetECInstanceIdPropertyMap());
    ECClassIdPropertyMap* ecClassIdPropertyMap = const_cast<ECClassIdPropertyMap*>(GetECClassIdPropertyMap());
    if (ecInstanceIdPropertyMap == nullptr || ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    
    //!Overflow is delay created which mean system property map are not upto day
    AddTable(overflowTable);

    DbColumn const* ecInstanceIdColumn = overflowTable.FindFirst(DbColumn::Kind::ECInstanceId);
    if (ecInstanceIdColumn == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbColumn const* ecClassIdColumn = overflowTable.FindFirst(DbColumn::Kind::ECClassId);
    if (ecClassIdColumn == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*ecInstanceIdPropertyMap, *ecInstanceIdColumn) == ERROR)
        return ERROR;

    if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*ecClassIdPropertyMap, *ecClassIdColumn) == ERROR)
        return ERROR;
    
    if (GetState() == ObjectState::Modified || GetState() == ObjectState::Persisted)
        {
        //We need to save to get id for overflow table when its created during schema update
        if (!overflowTable.HasId())
            m_ecdb.Schemas().GetDispatcher().Main().GetDbSchema().SaveOrUpdateTables();
        
        DbMapSaveContext ctx(m_ecdb);
        ctx.BeginSaving(*this);
        DbClassMapSaveContext classMapContext(ctx);
        SavePropertyMapVisitor saveVisitor(classMapContext, &overflowTable);
        ecInstanceIdPropertyMap->AcceptVisitor(saveVisitor);
        ecClassIdPropertyMap->AcceptVisitor(saveVisitor);
        ctx.EndSaving(*this);
        }

    Nullable<std::vector<ClassMap const*>> derivedClassMaps = GetDerivedClassMaps();
    if (derivedClassMaps == nullptr)
        return ERROR;

    for (ClassMap const* derivedClassMap : derivedClassMaps.Value())
        {
        if (derivedClassMap != nullptr)
            const_cast<ClassMap*>(derivedClassMap)->SetOverflowTable(overflowTable);
        }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       10 / 2016
//------------------------------------------------------------------------------------------
BentleyStatus ClassMap::MapSystemColumns()
    {
    if (GetECInstanceIdPropertyMap() != nullptr || GetECClassIdPropertyMap() != nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    std::vector<DbColumn const*> ecInstanceIdColumns, ecClassIdColumns;

    for (DbTable const* table : GetTables())
        {
        DbColumn const* ecInstanceIdColumn = table->FindFirst(DbColumn::Kind::ECInstanceId);
        if (ecInstanceIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        DbColumn const* ecClassIdColumn = table->FindFirst(DbColumn::Kind::ECClassId);
        if (ecClassIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ecInstanceIdColumns.insert(ecInstanceIdColumns.begin(), ecInstanceIdColumn);
        ecClassIdColumns.insert(ecClassIdColumns.begin(), ecClassIdColumn);
        }

    if (ecInstanceIdColumns.empty() || ecClassIdColumns.empty())
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, ecInstanceIdColumns);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, ecClassIdColumns);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
        return ERROR;

    return m_propertyMaps.Insert(ecClassIdPropertyMap, 1);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapColumnFactory const& ClassMap::GetColumnFactory() const
    {
    if (m_columnFactory == nullptr)
        m_columnFactory = std::make_unique<ClassMapColumnFactory>(*this);

    return *m_columnFactory;
    }

//************************** NotMappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ClassMappingStatus NotMappedClassMap::_Map(ClassMappingContext& ctx)
    {
    DbTable const* nullTable = GetSchemaManager().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus NotMappedClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    DbTable const* nullTable = GetSchemaManager().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
