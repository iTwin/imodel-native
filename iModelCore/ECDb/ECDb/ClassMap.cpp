/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMap.h"
#include <algorithm>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define CURRENTIMESTAMP_SQLEXP "julianday('now')"

//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECDb const& ecdb, Type type, ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy)
    : m_type(type), m_ecdb(ecdb), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy), m_propertyMaps(*this), m_state(ObjectState::New)
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
        if (table->GetType() == DbTable::Type::Primary || table->GetType() == DbTable::Type::Existing || table->GetType() == DbTable::Type::Virtual)
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
        if (table->GetType() == DbTable::Type::Joined)
            joinedTable = table;
        else if (table->GetType() == DbTable::Type::Primary || table->GetType() == DbTable::Type::Existing || table->GetType() == DbTable::Type::Virtual)
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
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
bool ClassMap::IsMixin() const
    {
    if (auto entity = GetClass().GetEntityClassCP())
        {
        return entity->IsMixin();
        }

    return false;
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
    if (SUCCESS != ClassMapper::TableMapper::MapToTable(*this, ctx.GetClassMappingInfo()))
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
    bool isJoinedTable = ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo().IsValid() && ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
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
        if (GetJoinedOrPrimaryTable().CreateForeignKeyConstraint(*foreignKeyColumn, *primaryKeyColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified) == nullptr)
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
    body.Sprintf("BEGIN UPDATE %s SET %s=" CURRENTIMESTAMP_SQLEXP " WHERE %s=new.%s; END", tableName, currentTimeStampColName, instanceIdColName, instanceIdColName);

    Utf8String whenCondition;
    whenCondition.Sprintf("old.%s=new.%s AND old.%s!=" CURRENTIMESTAMP_SQLEXP, currentTimeStampColName, currentTimeStampColName, currentTimeStampColName);

    return table.CreateTrigger(triggerName.c_str(), DbTrigger::Type::After, whenCondition.c_str(), body.c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan           06/2015
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::MapNavigationProperty(SchemaImportContext& ctx, NavigationPropertyMap& navPropMap)
    {
    return ctx.MapNavigationProperty(navPropMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::MapProperties(ClassMappingContext& ctx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    ClassMapper::PropertyMapInheritanceMode inheritanceMode = ClassMapper::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != ClassMapper::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ClassMappingStatus::Error;
                }

            if (baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }

    std::vector<ECPropertyCP> propertiesToMap;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (property->GetIsNavigation() && &property->GetClass() == &m_ecClass)
            {
            //WIP_RELMAP_REFACTOR extract ForeignKeyConstraint on nav prop for later use during mapping the relationship
            //WIP this can be changed once relationship mapping is refactored
            if (ctx.GetImportCtx().CacheFkConstraintCA(*property->GetAsNavigationProperty()))
                {
                SchemaPolicy const* noAdditionalForeignKeyConstraintsPolicy = nullptr;
                if (ctx.GetImportCtx().GetSchemaPolicies().IsOptedIn(noAdditionalForeignKeyConstraintsPolicy, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints))
                    {
                    if (SUCCESS != noAdditionalForeignKeyConstraintsPolicy->GetAs<NoAdditionalForeignKeyConstraintsPolicy>().Evaluate(m_ecdb, *property->GetAsNavigationProperty()))
                        return ClassMappingStatus::Error;
                    }

                }
            }

        if (&property->GetClass() == &m_ecClass ||
            inheritanceMode == ClassMapper::PropertyMapInheritanceMode::NotInherited)
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

        RefCountedPtr<DataPropertyMap> propertyMap = PropertyMapCopier::CreateCopy(*baseClassPropMap, *this);        
        if (propertyMap == nullptr || SUCCESS != m_propertyMaps.Insert(propertyMap))
            return ClassMappingStatus::Error;

        if (propertyMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus navMapStatus = MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (navMapStatus != ClassMappingStatus::Success)
                    return navMapStatus;
                }
            }
        }

    for (ECPropertyCP property : propertiesToMap)
        {
        PropertyMap* propMap = ClassMapper::MapProperty(*this, *property);
        if (propMap == nullptr)
            return ClassMappingStatus::Error;

        if (property->GetIsNavigation())
            {
            NavigationPropertyMap const& navPropertyMap = propMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus navMapStatus = MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (navMapStatus != ClassMappingStatus::Success)
                    return navMapStatus;
                }
            }
        }

    return ClassMappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::CreateUserProvidedIndexes(SchemaImportContext& schemaImportContext, std::vector<IndexMappingInfoPtr> const& indexInfoList) const
    {
    int i = 0;
    for (IndexMappingInfoPtr const& indexInfo : indexInfoList)
        {
        i++;

        std::vector<DbColumn const*> totalColumns;
        NativeSqlBuilder whereExpression;

        for (Utf8StringCR propertyAccessString : indexInfo->GetProperties())
            {
            PropertyMap const* propertyMap = GetPropertyMaps().Find(propertyAccessString.c_str());
            if (propertyMap == nullptr)
                {
                Issues().Report("DbIndex custom attribute #%d on ECClass '%s' is invalid: "
                   "The specified ECProperty '%s' does not exist or is not mapped.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            ECPropertyCR prop = propertyMap->GetProperty();
            if (!prop.GetIsPrimitive())
                {
                Issues().Report("DbIndex custom attribute #%d on ECClass '%s' is invalid: "
                              "The specified ECProperty '%s' is not of a primitive type.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            DbTable const& table = GetJoinedOrPrimaryTable();
            GetColumnsPropertyMapVisitor columnVisitor(table);
            propertyMap->AcceptVisitor(columnVisitor);
            if (columnVisitor.GetVirtualColumnCount() > 0)
                {
                Issues().Report("DbIndex custom attribute #%d on ECClass '%s' is invalid: "
                                "The specified ECProperty '%s' is mapped to a virtual column.",
                                i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            if (columnVisitor.GetColumnCount() == 0)
                {
                if (m_mapStrategyExtInfo.GetTphInfo().IsValid() && m_mapStrategyExtInfo.GetTphInfo().GetJoinedTableInfo() != JoinedTableInfo::None)
                    {
                    Issues().Report("DbIndex custom attribute #%d on ECClass '%s' is invalid. "
                                    "The properties that make up the index are mapped to different tables because the 'JoinedTablePerDirectSubclass' custom attribute "
                                    "is applied to this class hierarchy.",
                                    i, GetClass().GetFullName());
                    }
                else
                    {
                    Issues().Report("DbIndex custom attribute #%d on ECClass '%s' is invalid. "
                                    "The properties that make up the index are mapped to different tables.",
                                    i, GetClass().GetFullName());

                    BeAssert(false && "Properties of DbIndex are mapped to different tables although JoinedTable option is not applied.");
                    }

                return ERROR;
                }

            totalColumns.insert(totalColumns.end(), columnVisitor.GetColumns().begin(), columnVisitor.GetColumns().end());
            }

        if (nullptr == GetDbMap().GetDbSchemaR().CreateIndex(GetJoinedOrPrimaryTable(), indexInfo->GetName(), indexInfo->GetIsUnique(),
                                                                      totalColumns, indexInfo->IsAddPropsAreNotNullWhereExp(), false, GetClass().GetId()))
            {
            return ERROR;
            }
        }

    return SUCCESS;
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
            ClassMap* baseClassMap = const_cast<ClassMap*>(GetDbMap().GetClassMap(*baseClass));
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
        SetTable(*const_cast<DbTable*>(GetDbMap().GetDbSchema().GetNullTable()));
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
            else if (!Enum::Contains(column->GetKind(), DbColumn::Kind::ECClassId))
                {
                primaryTables.insert(&column->GetTableR());
                }
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
    ClassMapper::PropertyMapInheritanceMode inheritanceMode = ClassMapper::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != ClassMapper::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            if (baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }
   
    //bvector<ECPropertyCP> failedToLoadProperties;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        DataPropertyMap const*  tphBaseClassPropMap = nullptr;
        if (&property->GetClass() != &m_ecClass && inheritanceMode == ClassMapper::PropertyMapInheritanceMode::Clone)
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
            if (ClassMapper::LoadPropertyMap(*this, *property, dbCtx) == nullptr)
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
    if (!m_failedToLoadProperties.empty())
        {
        BeAssert(m_state == ObjectState::Persisted);
        m_state = ObjectState::Modified;

        UpdateColumnResolutionScope columnResolutionScope(*this);
        for (ECPropertyCP property : m_failedToLoadProperties)
            {
            PropertyMap const* propMap = ClassMapper::MapProperty(*this, *property);
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
                NavigationPropertyMap & navPropMap = const_cast<NavigationPropertyMap&>(propMap->GetAs<NavigationPropertyMap>());
                if (MapNavigationProperty(ctx, navPropMap) != ClassMappingStatus::Success)
                    return ERROR;
                }

            //! ECSchema update added new property for which we need to save property map
            DbMapSaveContext ctx(m_ecdb);
            //First make sure table is updated on disk. The table must already exist for this operation to work.
            if (GetDbMap().GetDbSchema().UpdateTableOnDisk(propMap->GetAs<DataPropertyMap>().GetTable()) != SUCCESS)
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
        }

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
IssueReporter const& ClassMap::Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription const& ClassMap::GetStorageDescription() const
    {
    return GetDbMap().GetLightweightCache().GetStorageDescription(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
std::vector<ClassMap const*> ClassMap::GetDerivedClassMaps() const
    {
    ECDerivedClassesList const& derivedClasses = m_ecdb.Schemas().GetDerivedClasses(GetClass());
    std::vector<ClassMap const*> derivedClassMaps;
    for (ECClassCP derivedClass : derivedClasses)
        {
        if (ClassMap const* derivedClassMap = GetDbMap().GetClassMap(*derivedClass))
            derivedClassMaps.push_back(derivedClassMap);
        }

    return derivedClassMaps;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       04 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMap::SetOverflowTable(DbTable& overflowTable)
    {
    if (GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy)
        {
        BeAssert(GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
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
    
    for (ClassMapCP derviedClassMap : GetDerivedClassMaps())
        if (derviedClassMap)
            const_cast<ClassMap*>(derviedClassMap)->SetOverflowTable(overflowTable);

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

        //WIP: If we push it at back it will break some code that presume that first table is the correct one.
        // The order should not be important
#ifndef NOT_A_GOOD_SOLUTION
        ecInstanceIdColumns.insert(ecInstanceIdColumns.begin(), ecInstanceIdColumn);
        ecClassIdColumns.insert(ecClassIdColumns.begin(), ecClassIdColumn);
#else //This is the right way because order should not be important and anycode that depend on order should be corrected.
        ecInstanceIdColumns.push_back(ecInstanceIdColumn);
        ecClassIdColumns.push_back(ecClassIdColumn);
#endif
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

//************************** ClassMapLoadContext ***************************************************
//************************** NotMappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ClassMappingStatus NotMappedClassMap::_Map(ClassMappingContext&)
    {
    DbTable const* nullTable = GetDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus NotMappedClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    DbTable const* nullTable = GetDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
