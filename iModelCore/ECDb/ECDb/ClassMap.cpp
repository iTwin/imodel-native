/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
ClassMap::ClassMap(ECDb const& ecdb, Type type, ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty)
    : m_ecdb(ecdb), m_type(type), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy),
    m_isDirty(setIsDirty), m_columnFactory(ecdb, *this), m_isECInstanceIdAutogenerationDisabled(false), m_tphHelper(nullptr), m_propertyMaps(*this)
    {
    if (m_mapStrategyExtInfo.IsTablePerHierarchy())
        m_tphHelper = std::unique_ptr<TablePerHierarchyHelper>(new TablePerHierarchyHelper(*this));

    if (SUCCESS != InitializeDisableECInstanceIdAutogeneration())
        {
        BeAssert(false && "InitializeDisableECInstanceIdAutogeneration failed");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::_Map(ClassMappingContext& ctx)
    {
    MappingStatus stat = DoMapPart1(ctx);
    if (stat != MappingStatus::Success)
        return stat;

    return DoMapPart2(ctx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::DoMapPart1(ClassMappingContext& ctx)
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
            return MappingStatus::Error;
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
                AddTable(tphBaseClassMap->GetJoinedTable());
            }
        }

    if (needsToCreateTable)
        {
        const bool isExclusiveRootClassOfTable = DetermineIsExclusiveRootClassOfTable(ctx.GetClassMappingInfo());
        DbTable* table = TableMapper::FindOrCreateTable(GetDbMap().GetDbSchemaR(), ctx.GetClassMappingInfo().GetTableName(), tableType,
                                                        ctx.GetClassMappingInfo().MapsToVirtualTable(), ctx.GetClassMappingInfo().GetECInstanceIdColumnName(),
                                                        isExclusiveRootClassOfTable ? ctx.GetClassMappingInfo().GetECClass().GetId() : ECClassId(),
                                                        primaryTable);
        if (table == nullptr)
            return MappingStatus::Error;

        AddTable(*table);
        }

    if (SUCCESS != MapSystemColumns())
        return MappingStatus::Error;

    //shared columns evaluation
    //Shared columns are created if ApplyToSubclassesOnly is true and if the subclasses are mapped to this table.
    //Reason: For a given table under shared columns regime the DB layout of the table must not change in later schema imports.
    if (!isTph || tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::No ||
        (tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly && tphInfo.GetJoinedTableInfo() == JoinedTableInfo::ParentOfJoinedTable))
        return MappingStatus::Success;

    if (tphBaseClassMap != nullptr)
        {
        TablePerHierarchyInfo const& baseTphInfo = tphBaseClassMap->GetMapStrategy().GetTphInfo();
        //if shared columns mode already enabled in base class but the base class is not the parent of a joined table
        //shared columns are already created in this table (s. above)
        if (baseTphInfo.GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::No &&
            baseTphInfo.GetJoinedTableInfo() != JoinedTableInfo::ParentOfJoinedTable)
            return MappingStatus::Success;
        }

    if (tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly)
        {
        //Shared cols are only used by subclasses. So all props of this class should get columns before the shared columns
        ctx.SetCreateSharedColumnsAfterMappingProperties();
        }
    else
        {
        if (SUCCESS != GetJoinedTable().CreateSharedColumns(tphInfo))
            {
            Issues().Report(ECDbIssueSeverity::Error, "Could not create shared columns for ECClass '%s'.", m_ecClass.GetFullName());
            return MappingStatus::Error;
            }
        }

    return MappingStatus::Success;
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
            case MapStrategy::SharedTable:
                return false;

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
MappingStatus ClassMap::DoMapPart2(ClassMappingContext& ctx)
    {
    MappingStatus stat = MapProperties(ctx);
    if (stat != MappingStatus::Success)
        return stat;

    ECPropertyCP currentTimeStampProp = ctx.GetClassMappingInfo().GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return MappingStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo().IsValid() && ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
    if (!isJoinedTable)
        return MappingStatus::Success;

    ClassMap const* tphBaseClassMap = ctx.GetClassMappingInfo().GetTphBaseClassMap();
    if (tphBaseClassMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    DbTable const& baseClassMapJoinedTable = tphBaseClassMap->GetJoinedTable();
    if (&baseClassMapJoinedTable == &GetJoinedTable())
        return MappingStatus::Success;

    DbColumn const* primaryKeyColumn = baseClassMapJoinedTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* foreignKeyColumn = GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    PRECONDITION(primaryKeyColumn != nullptr, MappingStatus::Error);
    PRECONDITION(foreignKeyColumn != nullptr, MappingStatus::Error);
    bool createFKConstraint = true;
    for (DbConstraint const* constraint : GetJoinedTable().GetConstraints())
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
        if (GetJoinedTable().CreateForeignKeyConstraint(*foreignKeyColumn, *primaryKeyColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified) == nullptr)
            return MappingStatus::Error;
        }

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::CreateCurrentTimeStampTrigger(ECPropertyCR currentTimeStampProp)
    {
    PrimitivePropertyMap const* propertyMap = dynamic_cast<PrimitivePropertyMap const*>(GetPropertyMaps().Find(currentTimeStampProp.GetName().c_str()));
    if (propertyMap == nullptr)
        return SUCCESS;


    DbColumn& currentTimeStampColumn = const_cast<DbColumn&>(propertyMap->GetColumn());
    if (currentTimeStampColumn.IsShared())
        {
        Issues().Report(ECDbIssueSeverity::Warning,
                   "ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
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
MappingStatus ClassMap::MapProperties(ClassMappingContext& ctx)
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
                return MappingStatus::Error;
                }

            if (baseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Persisted &&
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
            if (baseClassPropMap = dynamic_cast<DataPropertyMap const*>(baseClassMap->GetPropertyMaps().Find(property->GetName().c_str())))
                break;
            }

        if (baseClassPropMap == nullptr)
            {
            //Property is inherited, but not from a TPH base class, so we have to map the property from scratch
            propertiesToMap.push_back(property);
            continue;
            }

        RefCountedPtr<DataPropertyMap> propertyMap = PropertyMapCopier::CreateCopy(*baseClassPropMap, *this);        
        if (propertyMap == nullptr || SUCCESS != GetPropertyMapsR().Insert(propertyMap))
            return MappingStatus::Error;

        if (propertyMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap& navPropertyMap = static_cast<NavigationPropertyMap&>(*propertyMap);
            if (!navPropertyMap.IsComplete())
                ctx.GetImportCtx().GetClassMapLoadContext().AddNavigationPropertyMap(navPropertyMap);
            }
        }

    GetColumnFactory().Update(false);

    for (ECPropertyCP property : propertiesToMap)
        {
        PropertyMap* propMap = ClassMapper::MapProperty(*this, *property);
        if (propMap == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
            }

        if (property->GetIsNavigation())
            {
            NavigationPropertyMap* navPropertyMap = static_cast<NavigationPropertyMap*>(propMap);
            if (!navPropertyMap->IsComplete())
                ctx.GetImportCtx().GetClassMapLoadContext().AddNavigationPropertyMap(*navPropertyMap);
            }
        }

    //create shared columns if it was delayed to after the mapping of props
    if (ctx.IsCreateSharedColumnsAfterMappingProperties())
        {
        BeAssert(ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo().IsValid());
        if (SUCCESS != GetJoinedTable().CreateSharedColumns(ctx.GetClassMappingInfo().GetMapStrategy().GetTphInfo()))
            {
            Issues().Report(ECDbIssueSeverity::Error, "Could not create shared columns for ECClass '%s'.", m_ecClass.GetFullName());
            return MappingStatus::Error;
            }
        }

    return MappingStatus::Success;
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

        bset<DbTable const*> involvedTables;
        for (Utf8StringCR propertyAccessString : indexInfo->GetProperties())
            {
            PropertyMap const* propertyMap = GetPropertyMaps().Find(propertyAccessString.c_str());
            if (propertyMap == nullptr)
                {
                Issues().Report(ECDbIssueSeverity::Error,
                   "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                   "The specified ECProperty '%s' does not exist or is not mapped.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            ECPropertyCR prop = propertyMap->GetProperty();
            if (!prop.GetIsPrimitive())
                {
                Issues().Report(ECDbIssueSeverity::Error,
                              "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                              "The specified ECProperty '%s' is not of a primitive type.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            GetColumnsPropertyMapVisitor columnVisitor(GetJoinedTable());
            propertyMap->AcceptVisitor(columnVisitor);
            std::vector<DbColumn const*> const& columns = columnVisitor.GetColumns();
            if (0 == columns.size())
                {
                BeAssert(false && "Reject user defined index on %s. Fail to find column property map for property. Something wrong with mapping");
                return ERROR;
                }

            for (DbColumn const* column : columns)
                {
                if (column->IsOverflowSlave())
                    {
                    Issues().Report(ECDbIssueSeverity::Error,
                                    "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                                    "The specified ECProperty '%s' is mapped to an overflow column. Indexes on overflow columns are not supported.",
                                    i, GetClass().GetFullName(), propertyAccessString.c_str());
                    return ERROR;
                    }

                if (column->GetTable().GetPersistenceType() == PersistenceType::Persisted && column->GetPersistenceType() == PersistenceType::Virtual)
                    {
                    Issues().Report(ECDbIssueSeverity::Error,
                                  "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                                  "The specified ECProperty '%s' is mapped to a virtual column.",
                                  i, GetClass().GetFullName(), propertyAccessString.c_str());
                    return ERROR;
                    }

                DbTable const& table = column->GetTable();
                if (!involvedTables.empty() && involvedTables.find(&table) == involvedTables.end())
                    {
                    if (m_mapStrategyExtInfo.GetTphInfo().IsValid() && m_mapStrategyExtInfo.GetTphInfo().GetJoinedTableInfo() != JoinedTableInfo::None)
                        {
                        Issues().Report(ECDbIssueSeverity::Error,
                                      "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid. "
                                      "The properties that make up the index are mapped to different tables because the 'JoinedTablePerDirectSubclass' custom attribute "
                                      "is applied to this class hierarchy.",
                                      i, GetClass().GetFullName());
                        }
                    else
                        {
                        Issues().Report(ECDbIssueSeverity::Error,
                                      "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid. "
                                      "The properties that make up the index are mapped to different tables.",
                                      i, GetClass().GetFullName());

                        BeAssert(false && "Properties of DbIndex are mapped to different tables although JoinedTable option is not applied.");
                        }

                    return ERROR;
                    }

                involvedTables.insert(&table);
                totalColumns.push_back(column);
                }
            }

        DbTable* involvedTable =  const_cast<DbTable*>(*involvedTables.begin());
        if (nullptr == GetDbMap().GetDbSchemaR().CreateIndex(*involvedTable, indexInfo->GetName(), indexInfo->GetIsUnique(),
                                                                      totalColumns, indexInfo->IsAddPropsAreNotNullWhereExp(), false, GetClass().GetId()))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                09/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassHasDisableECInstanceIdAutogenerationCA(bool* appliesToSubclasses, ECClassCR ecclass)
    {
    if (appliesToSubclasses != nullptr)
        *appliesToSubclasses = false;

    IECInstancePtr disableECInstanceIdAutoGenerationCA = ecclass.GetCustomAttributeLocal("DisableECInstanceIdAutogeneration");
    if (disableECInstanceIdAutoGenerationCA != nullptr && appliesToSubclasses != nullptr)
        {
        ECValue v;
        if (ECObjectsStatus::Success != disableECInstanceIdAutoGenerationCA->GetValue(v, "AppliesToSubclasses"))
            {
            BeAssert(false && "CA DisableECInstanceIdAutogeneration is expected to have a property AppliesToSubclasses");
            return false;
            }

        *appliesToSubclasses = v.IsNull() || v.GetBoolean();
        }

    return disableECInstanceIdAutoGenerationCA != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                09/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMap::InitializeDisableECInstanceIdAutogeneration()
    {
    if (ClassHasDisableECInstanceIdAutogenerationCA(nullptr, m_ecClass))
        {
        m_isECInstanceIdAutogenerationDisabled = true;
        return SUCCESS;
        }

    for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
        {
        bool appliesToSubclasses = false;
        if (ClassHasDisableECInstanceIdAutogenerationCA(&appliesToSubclasses, *baseClass))
            {
            if (appliesToSubclasses)
                {
                m_isECInstanceIdAutogenerationDisabled = true;
                return SUCCESS;
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Save(DbMapSaveContext& ctx)
    {
    if (!m_isDirty || ctx.IsAlreadySaved(*this))
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

            if (SUCCESS != baseClassMap->Save(ctx))
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

    m_isDirty = false;
    ctx.EndSaving(*this);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx)
    {
    std::set<DbTable*> tables;
    std::set<DbTable*> joinedTables;

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
            else if (!Enum::Contains(column->GetKind(), DbColumn::Kind::ECClassId))
                {
                tables.insert(&column->GetTableR());
                }
            }
        }

    for (DbTable* table : tables)
        AddTable(*table);

    for (DbTable* table : joinedTables)
        AddTable(*table);

    BeAssert(!GetTables().empty());

    if (GetECInstanceIdPropertyMap() != nullptr)
        return ERROR;

    std::vector<DbColumn const*> const* mapColumnsList = dbLoadCtx.FindColumnByAccessString(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME);
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

    mapColumnsList = dbLoadCtx.FindColumnByAccessString(ECDbSystemSchemaHelper::ECCLASSID_PROPNAME);
    if (mapColumnsList == nullptr)
        return ERROR;

    //Load ECClassId   ================================================
    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this,GetClass().GetId(), *mapColumnsList);
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

            if (baseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Persisted &&
                baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }
   
    bvector<ECPropertyCP> failedToLoadProperties;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        DataPropertyMap const*  tphBaseClassPropMap = nullptr;
        if (&property->GetClass() != &m_ecClass && inheritanceMode == PropertyMapInheritanceMode::Clone)
            {
            for (ClassMap const* baseClassMap : tphBaseClassMaps)
                {
                if (tphBaseClassPropMap = dynamic_cast<DataPropertyMap const*> (baseClassMap->GetPropertyMaps().Find(property->GetName().c_str())))
                    break;
                }
            }

        if (tphBaseClassPropMap == nullptr)
            {
            if (ClassMapper::LoadPropertyMap(*this, *property, dbCtx) == nullptr)
                {
                failedToLoadProperties.push_back(property);
                }
            }
        else
            {
            RefCountedPtr<PropertyMap> propMap = PropertyMapCopier::CreateCopy(*tphBaseClassPropMap,*this);
            if (propMap == nullptr)
                return ERROR;

            if (GetPropertyMapsR().Insert(propMap) != SUCCESS)
                return ERROR;
            }
        }

    if (!failedToLoadProperties.empty())
        {
        GetColumnFactory().Update(true);
        for (ECPropertyCP property : failedToLoadProperties)
            {
            PropertyMap* propMap = ClassMapper::MapProperty(*this, *property);
            if (propMap == nullptr)
            	{
                BeAssert(false);
                return ERROR;
                }

            if (!propMap->IsData())
                {
                BeAssert(false);
                return ERROR;
                }
            //Nav properties cannot be safed here as they are not yet mapped.
            if (propMap->GetType() != PropertyMap::Type::Navigation)
                {
                DataPropertyMap const* dataPropMap = static_cast<DataPropertyMap*>(propMap);
                //! ECSchema update added new property for which we need to save property map
                DbMapSaveContext ctx(m_ecdb);
                //First make sure table is updated on disk. The table must already exist for this operation to work.
                if (GetDbMap().GetDbSchema().UpdateTableOnDisk(dataPropMap->GetTable()) != SUCCESS)
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
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECInstanceIdPropertyMap const* ClassMap::GetECInstanceIdPropertyMap() const
    {
    return static_cast<ECInstanceIdPropertyMap const*> (GetPropertyMaps().Find(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECClassIdPropertyMap const* ClassMap::GetECClassIdPropertyMap() const
    {
    return static_cast<ECClassIdPropertyMap const*>(GetPropertyMaps().Find(ECDbSystemSchemaHelper::ECCLASSID_PROPNAME));
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
    ECDerivedClassesList const& derivedClasses = m_ecdb.Schemas().GetDerivedECClasses(GetClass());
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
        DbColumn const* ecInstanceIdColumn = table->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        if (ecInstanceIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        DbColumn const* ecClassIdColumn = table->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
        if (ecClassIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        //WIP: If we push it at back it will break some code that presume that first table is the correct one.
        // The order should not be importable
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

    auto ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, ecInstanceIdColumns);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    auto ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, GetClass().GetId(), ecClassIdColumns);
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
    tablePrefix.clear();

    ECSchemaCR schema = ecclass.GetSchema();
    ECDbSchemaMap customSchemaMap;

    if (ECDbMapCustomAttributeHelper::TryGetSchemaMap(customSchemaMap, schema))
        {
        if (customSchemaMap.TryGetTablePrefix(tablePrefix) != ECObjectsStatus::Success)
            return ERROR;
        }

    if (tablePrefix.empty())
        {
        Utf8StringCR alias = schema.GetAlias();
        if (!alias.empty())
            tablePrefix = alias;
        else
            tablePrefix = schema.GetName();
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    12/2015
//---------------------------------------------------------------------------------------
Utf8String ClassMap::GetUpdatableViewName() const
    {
    Utf8String name;
    name.Sprintf("_%s_%s", GetClass().GetSchema().GetAlias().c_str(), GetClass().GetName().c_str());
    return name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   03/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::GenerateSelectViewSql(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const
    {
    return ViewGenerator::GenerateSelectViewSql(viewSql, m_ecdb, *this, isPolymorphic, prepareContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  06/2016
//---------------------------------------------------------------------------------------
DbTable const* ClassMap::ExpectingSingleTable() const
    {
    BeAssert(GetTables().size() == 1);
    if (GetTables().size() != 1)
        return nullptr;

    return &GetJoinedTable();
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

//************************** UnmappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MappingStatus NotMappedClassMap::_Map(ClassMappingContext&)
    {
    DbTable const* nullTable = GetDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));

    return MappingStatus::Success;
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
