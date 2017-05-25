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
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECDb const& ecdb, Type type, ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const& updatableViewInfo)
    : m_type(type), m_ecdb(ecdb), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy), m_updatableViewInfo(updatableViewInfo), m_propertyMaps(*this), m_state(ObjectState::New)
    {
    if (m_mapStrategyExtInfo.IsTablePerHierarchy())
        m_tphHelper = std::make_unique<TablePerHierarchyHelper>(*this);
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
    DbTable::Type tableType = DbTable::Type::Primary;
    const bool isTph = ctx.GetClassMappingInfo().GetMapStrategy().IsTablePerHierarchy();
    TablePerHierarchyInfo const& tphInfo = ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo();
    ClassMap const* tphBaseClassMap = isTph ? ctx.GetClassMappingInfo().GetTphBaseClassMap() : nullptr;
    if (isTph && tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
        {
        tableType = DbTable::Type::Joined;
        if (tphBaseClassMap == nullptr)
            {
            BeAssert(false);
            return ClassMappingStatus::Error;
            }
        }
    else if (ctx.GetClassMappingInfo().GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        tableType = DbTable::Type::Existing;

    DbTable const* primaryTable = nullptr;
    bool needsToCreateTable = !isTph || tphBaseClassMap == nullptr;
    if (tphBaseClassMap != nullptr)
        {
        SetTable(tphBaseClassMap->GetPrimaryTable());
        if (tableType == DbTable::Type::Joined)
            {
            if (tphBaseClassMap->GetTphHelper()->IsParentOfJoinedTable())
                {
                primaryTable = &tphBaseClassMap->GetPrimaryTable();
                needsToCreateTable = true;
                }
            else
                AddTable(tphBaseClassMap->GetJoinedOrPrimaryTable());
            }

        for (DbTable const* table : GetTables())
            {
            DbTable::LinkNode const* overflowTableNode = table->GetLinkNode().FindOverflowTable();
            if (overflowTableNode != nullptr)
                {
                AddTable(overflowTableNode->GetTableR());
                break;
                }
            }
        }

    if (needsToCreateTable)
        {
        const bool isExclusiveRootClassOfTable = DetermineIsExclusiveRootClassOfTable(ctx.GetClassMappingInfo());
        DbTable* table = TableMapper::FindOrCreateTable(GetDbMap().GetDbSchemaR(), ctx.GetClassMappingInfo().GetTableName(), tableType, ctx.GetClassMappingInfo().GetMapStrategy(),
                                                        ctx.GetClassMappingInfo().MapsToVirtualTable(), ctx.GetClassMappingInfo().GetECInstanceIdColumnName(),
                                                        isExclusiveRootClassOfTable ? ctx.GetClassMappingInfo().GetClass().GetId() : ECClassId(),
                                                        primaryTable);
        if (table == nullptr)
            return ClassMappingStatus::Error;

        AddTable(*table);
        }

    
    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
bool ClassMap::DetermineIsExclusiveRootClassOfTable(ClassMappingInfo const& mappingInfo) const
    {
    if (GetType() == Type::RelationshipEndTable)
        {
        BeAssert(false && "Should not be called for end table rel class maps");
        return false;
        }

    MapStrategy strategy = mappingInfo.GetMapStrategy().GetStrategy();
    switch (strategy)
        {
            case MapStrategy::ExistingTable:
                //for existing table we also assume an exclusive root as ECDb only supports mapping a single ECClass to an existing table
                return true;

                //OwnedTable obviously always has an exclusive root because only a single class is mapped to the table.
            case MapStrategy::OwnTable:
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
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::MapProperties(ClassMappingContext& ctx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    PropertyMapInheritanceMode inheritanceMode = GetPropertyMapInheritanceMode();
    if (inheritanceMode != PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ClassMappingStatus::Error;
                }

            if (baseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Physical &&
                baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }

    std::vector<ECPropertyCP> propertiesToMap;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (&property->GetClass() == &m_ecClass ||
            inheritanceMode == PropertyMapInheritanceMode::NotInherited)
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
        if (propertyMap == nullptr || SUCCESS != GetPropertyMapsR().Insert(propertyMap))
            return ClassMappingStatus::Error;

        if (propertyMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                ctx.GetImportCtx().GetClassMapLoadContext().AddNavigationPropertyMap(const_cast<NavigationPropertyMap&>(navPropertyMap));
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
                ctx.GetImportCtx().GetClassMapLoadContext().AddNavigationPropertyMap(const_cast<NavigationPropertyMap&>(navPropertyMap));
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
            if (table.GetPersistenceType() == PersistenceType::Physical && columnVisitor.GetVirtualColumnCount() > 0)
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
            ClassMap* baseClassMap = (ClassMap*) GetDbMap().GetClassMap(*baseClass);
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

    if (GetPropertyMapsR().Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
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

    if (GetPropertyMapsR().Insert(ecClassIdPropertyMap, 1) != SUCCESS)
        return ERROR;

    return LoadPropertyMaps(ctx, dbLoadCtx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  09/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::LoadPropertyMaps(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbCtx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    PropertyMapInheritanceMode inheritanceMode = GetPropertyMapInheritanceMode();
    if (inheritanceMode != PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            if (baseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Physical &&
                baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }
   
    //bvector<ECPropertyCP> failedToLoadProperties;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        DataPropertyMap const*  tphBaseClassPropMap = nullptr;
        if (&property->GetClass() != &m_ecClass && inheritanceMode == PropertyMapInheritanceMode::Clone)
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

        if (GetPropertyMapsR().Insert(propMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan  01/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Update()
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
                continue;

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
        ClassMap const* derivedClassMap = GetDbMap().GetClassMap(*derivedClass);
        derivedClassMaps.push_back(derivedClassMap);
        }

    return derivedClassMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMap::DetermineTableName(Utf8StringR tableName, ECN::ECClassCR ecclass, Utf8CP tablePrefix)
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

    if (GetPropertyMapsR().Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
        return ERROR;

    return GetPropertyMapsR().Insert(ecClassIdPropertyMap, 1);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMap::DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR ecclass)
    {
    ECSchemaCR schema = ecclass.GetSchema();
    ECDbSchemaMap customSchemaMap;

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

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapColumnFactory const& ClassMap::GetColumnFactory() const
    {
    if (m_columnFactory == nullptr)
        m_columnFactory = std::make_unique<ClassMapColumnFactory>(*this);

    return *m_columnFactory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  06/2016
//---------------------------------------------------------------------------------------
DbTable const* ClassMap::ExpectingSingleTable() const
    {
    BeAssert(GetTables().size() == 1);
    if (GetTables().size() != 1)
        return nullptr;

    return &GetJoinedOrPrimaryTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   10/2016
//---------------------------------------------------------------------------------------
//static
ClassMap::PropertyMapInheritanceMode ClassMap::GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const& mapStrategyExtInfo)
    {
    if (mapStrategyExtInfo.GetStrategy() != MapStrategy::TablePerHierarchy)
        return PropertyMapInheritanceMode::NotInherited;

    return PropertyMapInheritanceMode::Clone;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
//static
Utf8CP ClassMap::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::Class:
                return "Class";
            case Type::NotMapped:
                return "NotMapped";
            case Type::RelationshipEndTable:
                return "RelationshipEndTable";
            case Type::RelationshipLinkTable:
                return "RelationshipLinkTable";

            default:
                BeAssert(false && "ClassMap::TypeToString must be extended to handle new ClassMap::Type value");
                return "";
        }
    }

//=========================================================================================
//ClassMap::TablePerHierarchyHelper
//=========================================================================================
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    09/2016
//------------------------------------------------------------------------------------------
ECClassId ClassMap::TablePerHierarchyHelper::DetermineParentOfJoinedTableECClassId() const
    {
    if (!HasJoinedTable())
        {
        BeAssert(false && "TablePerHierarchyHelper::DetermineParentOfJoinedTableECClassId can only be called for class maps that have a joined table.");
        return ECClassId();
        }

    if (!m_parentOfJoinedTableECClassId.IsValid())
        {
        CachedStatementPtr stmt = m_classMap.GetECDb().GetCachedStatement("SELECT ch.BaseClassId FROM " TABLE_ClassHierarchyCache " ch "
                                                                          "JOIN ec_ClassMap cm ON cm.ClassId = ch.BaseClassId AND cm.JoinedTableInfo=" SQLVAL_JoinedTableInfo_ParentOfJoinedTable
                                                                          " WHERE ch.ClassId=?");
        if (stmt == nullptr ||
            BE_SQLITE_OK != stmt->BindId(1, m_classMap.GetClass().GetId()) ||
            BE_SQLITE_ROW != stmt->Step())
            {
            BeAssert(false && "Failed to retrieve parent of joined table ECClassId");
            return ECClassId();
            }

        m_parentOfJoinedTableECClassId = stmt->GetValueId<ECClassId>(0);
        BeAssert(m_parentOfJoinedTableECClassId.IsValid() && "Retrieval failed");
        }

    return m_parentOfJoinedTableECClassId; 
    }
//************************** ClassMapLoadContext ***************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    01/2016
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapLoadContext::Postprocess(DbMap const& ecdbMap)
    {
    for (ECN::ECClassCP constraintClass : m_constraintClasses)
        {
        if (ecdbMap.GetClassMap(*constraintClass) == nullptr)
            {
            LOG.errorv("Finishing creation of ECRelationship class map because class map for Constraint ECClass '%s' could not be found.",
                       constraintClass->GetFullName());
            return ERROR;
            }
        }

    for (NavigationPropertyMap* propMap : m_navPropMaps)
        {
        if (SUCCESS != ClassMapper::SetupNavigationPropertyMap(*propMap))
            return ERROR;
        }

    m_constraintClasses.clear();
    m_navPropMaps.clear();
    return SUCCESS;
    }
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
