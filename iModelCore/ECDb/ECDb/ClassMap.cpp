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
MappingStatus ClassMap::_Map(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo)
    {
    MappingStatus stat = DoMapPart1(schemaImportContext, mapInfo);
    if (stat != MappingStatus::Success)
        return stat;

    return DoMapPart2(schemaImportContext, mapInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::DoMapPart1(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mappingInfo)
    {
    DbTable::Type tableType = DbTable::Type::Primary;
    const bool isTph = mappingInfo.GetMapStrategy().IsTablePerHierarchy();
    TablePerHierarchyInfo const& tphInfo = mappingInfo.GetMapStrategy().GetTphInfo();
    ClassMap const* tphBaseClassMap = isTph ? mappingInfo.GetTphBaseClassMap() : nullptr;
    if (isTph && tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
        {
        tableType = DbTable::Type::Joined;
        if (tphBaseClassMap == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
            }
        }
    else if (mappingInfo.GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
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
        const bool isExclusiveRootClassOfTable = DetermineIsExclusiveRootClassOfTable(mappingInfo);
        DbTable* table = const_cast<ECDbMap&>(GetDbMap()).FindOrCreateTable(&schemaImportContext, mappingInfo.GetTableName(), tableType,
                                                                           mappingInfo.MapsToVirtualTable(), mappingInfo.GetECInstanceIdColumnName(),
                                                                           isExclusiveRootClassOfTable ? mappingInfo.GetECClass().GetId() : ECClassId(),
                                                                           primaryTable);
        if (table == nullptr)
            return MappingStatus::Error;

        AddTable(*table);
        }

    if (isTph && tphInfo.UseSharedColumns() && tphInfo.GetSharedColumnCount() > 0)
        {
        if (SUCCESS != GetJoinedTable().SetMinimumSharedColumnCount(tphInfo.GetSharedColumnCount()))
            {
            Issues().Report(ECDbIssueSeverity::Error,
                            "Only one ECClass per table can specify a shared column count. Found duplicate definition on ECClass '%s'.",
                            m_ecClass.GetFullName());
            return MappingStatus::Error;
            }
        }

    return MapSystemColumns();
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
MappingStatus ClassMap::DoMapPart2(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mappingInfo)
    {
    MappingStatus stat = MapProperties(schemaImportContext);
    if (stat != MappingStatus::Success)
        return stat;

    ECPropertyCP currentTimeStampProp = mappingInfo.GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return MappingStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = mappingInfo.GetMapStrategy().GetTphInfo().IsValid() && mappingInfo.GetMapStrategy().GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
    if (isJoinedTable)
        {
        ClassMap const* tphBaseClassMap = mappingInfo.GetTphBaseClassMap();
        if (tphBaseClassMap == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
            }
        
        DbTable const& baseClassMapJoinedTable = tphBaseClassMap->GetJoinedTable();
        if (&baseClassMapJoinedTable != &GetJoinedTable())
            {
            DbColumn const* primaryKeyColumn = baseClassMapJoinedTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            DbColumn const* foreignKeyColumn = GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            PRECONDITION(primaryKeyColumn != nullptr, MappingStatus::Error);
            PRECONDITION(foreignKeyColumn != nullptr, MappingStatus::Error);
            bool createFKConstraint = true;
            for (DbConstraint const* constraint : GetJoinedTable().GetConstraints())
                {
                if (constraint->GetType() == DbConstraint::Type::ForeignKey)
                    {
                    ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
                    if (&fk->GetReferencedTable() == &baseClassMapJoinedTable)
                        {
                        if (fk->GetFkColumns().front() == foreignKeyColumn && fk->GetReferencedTableColumns().front() == primaryKeyColumn)
                            {
                            createFKConstraint = false;
                            break;
                            }
                        }
                    }
                }

            if (createFKConstraint)
                {
                if (GetJoinedTable().CreateForeignKeyConstraint(*foreignKeyColumn, *primaryKeyColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified) == nullptr)
                    return MappingStatus::Error;
                }
            }
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
MappingStatus ClassMap::MapProperties(SchemaImportContext& ctx)
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
            ctx.GetClassMapLoadContext().AddNavigationPropertyMap(*navPropertyMap);
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
            NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
            if (!prop.GetIsPrimitive() && (navProp == nullptr || navProp->IsMultiple()))
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
        propertyMap->AcceptVisitor(saveVisitor);
        if (saveVisitor.GetStatus() != SUCCESS)
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

    if (GetPropertyMapsR().Insert(ecInstanceIdPropertyMap, 0LL) != SUCCESS)
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

    if (GetPropertyMapsR().Insert(ecClassIdPropertyMap, 1LL) != SUCCESS)
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

MappingStatus ClassMap::MapSystemColumns()
    {
    if (GetECInstanceIdPropertyMap() != nullptr || GetECClassIdPropertyMap() != nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    std::vector<DbColumn const*> ecInstanceIdColumns, ecClassIdColumns;

    for (DbTable const* table : GetTables())
        {
        DbColumn const* ecInstanceIdColumn = table->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        if (ecInstanceIdColumn == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
            }

        DbColumn const* ecClassIdColumn = table->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
        if (ecClassIdColumn == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
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
        return MappingStatus::Error;
        }

    auto ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, ecInstanceIdColumns);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    auto ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, GetClass().GetId(), ecClassIdColumns);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(ecInstanceIdPropertyMap, 0LL) != SUCCESS)
        return MappingStatus::Error;

    if (GetPropertyMapsR().Insert(ecClassIdPropertyMap, 1LL) != SUCCESS)
        return MappingStatus::Error;

    return MappingStatus::Success;
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

//=========================================================================================
//ColumnFactory
//=========================================================================================

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::ColumnFactory(ECDbCR ecdb, ClassMapCR classMap) : m_ecdb(ecdb), m_classMap(classMap), m_usesSharedColumnStrategy(false)
    {
    TablePerHierarchyInfo const& tphInfo = m_classMap.GetMapStrategy().GetTphInfo();
    m_usesSharedColumnStrategy = tphInfo.IsValid() && tphInfo.UseSharedColumns();
    BeAssert(!m_usesSharedColumnStrategy || m_classMap.GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
    Update(false);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* ColumnFactory::CreateColumn(ECN::ECPropertyCR ecProp, Utf8CP accessString, Utf8CP requestedColumnName, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        {
        // Shared column does not support NOT NULL constraint -> omit NOT NULL and issue warning
        if (addNotNullConstraint || addUniqueConstraint || collation != DbColumn::Constraints::Collation::Default)
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, "For the ECProperty '%s' on ECClass '%s' either a 'not null', unique or collation constraint is defined. It is mapped "
                                                          "to a column though shared with other ECProperties. Therefore ECDb cannot enforce any of these constraints. "
                                                          "The column is created without constraints.",
                                                          ecProp.GetName().c_str(), ecProp.GetClass().GetFullName());

            }

        outColumn = ApplySharedColumnStrategy();
        }
    else
        outColumn = ApplyDefaultStrategy(requestedColumnName, ecProp, accessString, colType, addNotNullConstraint, addUniqueConstraint, collation);

    if (outColumn == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    CacheUsedColumn(*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
DbColumn* ColumnFactory::ApplyDefaultStrategy(Utf8CP requestedColumnName, ECN::ECPropertyCR ecProp, Utf8CP accessString, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    BeAssert(!Utf8String::IsNullOrEmpty(requestedColumnName) && "Column name must not be null for default strategy");

    DbColumn* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && !IsColumnInUseByClassMap(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (!GetTable().IsOwnedByECDb() || (existingColumn->GetConstraints().HasNotNullConstraint() == addNotNullConstraint &&
                                            existingColumn->GetConstraints().HasUniqueConstraint() == addUniqueConstraint &&
                                            existingColumn->GetConstraints().GetCollation() == collation))
            {
            return existingColumn;
            }

        LOG.warningv("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                     " but where 'Nullable', 'Unique', or 'Collation' differs, and which will therefore be ignored for some of the properties.",
                     existingColumn->GetName().c_str(), GetTable().GetName().c_str());
        }

    const ECClassId classId = GetPersistenceClassId(ecProp, accessString);
    if (!classId.IsValid())
        return nullptr;

    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (SUCCESS != ResolveColumnName(tmp, requestedColumnName, classId, retryCount))
        return nullptr;

    resolvedColumnName = tmp;
    while (GetTable().FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (SUCCESS != ResolveColumnName(resolvedColumnName, requestedColumnName, classId, retryCount))
            return nullptr;
        }

    DbColumn* newColumn = GetTable().CreateColumn(resolvedColumnName.c_str(), colType, DbColumn::Kind::DataColumn, PersistenceType::Persisted);
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    if (addNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (addUniqueConstraint)
        newColumn->GetConstraintsR().SetUniqueConstraint();

    newColumn->GetConstraintsR().SetCollation(collation);
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* ColumnFactory::ApplySharedColumnStrategy() const
    {
    DbColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<DbColumn*>(reusableColumn);

    return GetTable().CreateSharedColumn();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus ColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, Utf8CP requestedColumnName, ECN::ECClassId classId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert(!resolvedColumName.empty());
        resolvedColumName += SqlPrintfString("%d", retryCount);
        return SUCCESS;
        }

    if (Utf8String::IsNullOrEmpty(requestedColumnName))
        {
        //use name generator
        resolvedColumName.clear();
        return SUCCESS;
        }

    DbColumn const* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && IsColumnInUseByClassMap(*existingColumn))
        {
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName);
        }
    else
        resolvedColumName.assign(requestedColumnName);

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId ColumnFactory::GetPersistenceClassId(ECN::ECPropertyCR ecProp, Utf8CP accessString) const
    {
    Utf8String propAccessString(accessString);
    const size_t dotPosition = propAccessString.find(".");
    ECPropertyCP property = nullptr;
    if (dotPosition != Utf8String::npos)
        {
        //! Get root property in given accessString.
        property = m_classMap.GetClass().GetPropertyP(propAccessString.substr(0, dotPosition).c_str());
        }
    else
        property = m_classMap.GetClass().GetPropertyP(propAccessString.c_str());


    if (property == nullptr)
        {
        BeAssert(false && "Failed to find root property");
        return ECClassId();
        }

    return property->GetClass().GetId();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const
    {
    reusableColumn = nullptr;
    std::vector<DbColumn const*> reusableColumns;
    for (DbColumn const* column : GetTable().GetColumns())
        {
        if (column->IsShared() && !IsColumnInUseByClassMap(*column))
            reusableColumns.push_back(column);
        }

    if (reusableColumns.empty())
        return false;

    reusableColumn = reusableColumns.front();
    return true;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::CacheUsedColumn(DbColumn const& column) const
    {
    m_idsOfColumnsInUseByClassMap.insert(column.GetId());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUseByClassMap(DbColumn const& column) const
    {
    bool isUsed = m_idsOfColumnsInUseByClassMap.find(column.GetId()) != m_idsOfColumnsInUseByClassMap.end();
    return isUsed;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Update(bool includeDerivedClasses) const
    {
    m_idsOfColumnsInUseByClassMap.clear();
    GetColumnsPropertyMapVisitor sharedColumnVisitor(PropertyMap::Type::Data);
    m_classMap.GetPropertyMaps().AcceptVisitor(sharedColumnVisitor);
    for (DbColumn const* columnInUse : sharedColumnVisitor.GetColumns())
        {
        if (columnInUse->IsShared() && &columnInUse->GetTable() == &GetTable())
            CacheUsedColumn(*columnInUse);
        }

    if (includeDerivedClasses)
        {
        std::vector<DbColumn const*> columns;
        GetDerivedColumnList(columns);
        for (DbColumn const* columnInUse : columns)
            {
            //WIP Why can the column ever be nullptr at all??
            if (columnInUse != nullptr)
                CacheUsedColumn(*columnInUse);
            }
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       10 / 2016
//------------------------------------------------------------------------------------------
BentleyStatus ColumnFactory::GetDerivedColumnList(std::vector<DbColumn const*>& columns) const
 {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT c.Name FROM ec_Column c "
        "              JOIN ec_PropertyMap pm ON c.Id = pm.ColumnId "
        "              JOIN ec_ClassMap cm ON cm.ClassId = pm.ClassId "
        "              JOIN " TABLE_ClassHierarchyCache " ch ON ch.ClassId = cm.ClassId "
        "              JOIN ec_Table t on t.Id = c.TableId "
        "WHERE ch.BaseClassId=? AND t.Name=? and c.ColumnKind & " SQLVAL_DbColumn_Kind_SharedDataColumn " <> 0 "
        "GROUP BY c.Name");

    if (stmt == nullptr)
        return ERROR;
    stmt->BindId(1, m_classMap.GetClass().GetId());
    stmt->BindText(2, GetTable().GetName().c_str(), Statement::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        columns.push_back(GetTable().FindColumn(stmt->GetValueText(0)));
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbTable& ColumnFactory::GetTable() const
    {
    return m_classMap.GetJoinedTable();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    01/2016
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapLoadContext::Postprocess(ECDbMap const& ecdbMap)
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


//************************** UnmappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MappingStatus NotMappedClassMap::_Map(SchemaImportContext&, ClassMappingInfo const& classMapInfo)
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
