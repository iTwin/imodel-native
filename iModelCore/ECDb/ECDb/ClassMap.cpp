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

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(Type type, ECClassCR ecClass, ECDbMap const& ecDbMap, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty)
    : m_type(type), m_ecDbMap(ecDbMap), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy),
    m_isDirty(setIsDirty), m_columnFactory(*this), m_isECInstanceIdAutogenerationDisabled(false)
    {
    if (SUCCESS != InitializeDisableECInstanceIdAutogeneration())
        {
        BeAssert(false && "InitializeDisableECInstanceIdAutogeneration failed");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::Map(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo)
    {
    BeAssert(mapInfo.GetBaseClassMap() == nullptr ||
             m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy ||
             m_mapStrategyExtInfo.GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ||
             m_mapStrategyExtInfo.GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable);

    return _Map(schemaImportContext, mapInfo);
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
    ClassMap const* baseClassMap = mappingInfo.GetBaseClassMap();
    if (baseClassMap != nullptr && baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return MappingStatus::Error;

    DbTable::Type tableType = DbTable::Type::Primary;
    TablePerHierarchyInfo const& tphInfo = mappingInfo.GetMapStrategy().GetTphInfo();
    if (tphInfo.IsValid() && tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
        {
        tableType = DbTable::Type::Joined;
        if (baseClassMap == nullptr)
            {
            BeAssert(false);
            return MappingStatus::Error;
            }
        }
    else if (mappingInfo.GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        tableType = DbTable::Type::Existing;

    DbTable const* primaryTable = nullptr;
    bool needsToCreateTable = false;
    if (baseClassMap == nullptr)
        needsToCreateTable = true;
    else
        {
        m_baseClassId = baseClassMap->GetClass().GetId();
        SetTable(baseClassMap->GetPrimaryTable());
        if (tableType == DbTable::Type::Joined)
            {
            if (baseClassMap->IsParentOfJoinedTable())
                {
                primaryTable = &baseClassMap->GetPrimaryTable();
                needsToCreateTable = true;
                }
            else
                AddTable(baseClassMap->GetJoinedTable());
            }
        }

    if (needsToCreateTable)
        {
        const bool isExclusiveRootClassOfTable = DetermineIsExclusiveRootClassOfTable(mappingInfo);
        DbTable* table = const_cast<ECDbMap&>(m_ecDbMap).FindOrCreateTable(&schemaImportContext, mappingInfo.GetTableName(), tableType,
                                                                           mappingInfo.MapsToVirtualTable(), mappingInfo.GetECInstanceIdColumnName(),
                                                                           isExclusiveRootClassOfTable ? mappingInfo.GetECClass().GetId() : ECClassId(),
                                                                           primaryTable);
        if (table == nullptr)
            return MappingStatus::Error;

        AddTable(*table);
        }

    if (tphInfo.IsValid() && tphInfo.UseSharedColumns() && tphInfo.GetSharedColumnCount() > 0)
        {
        if (SUCCESS != GetJoinedTable().SetMinimumSharedColumnCount(tphInfo.GetSharedColumnCount()))
            {
            Issues().Report(ECDbIssueSeverity::Error,
                            "Only one ECClass per table can specify a shared column count. Found duplicate definition on ECClass '%s'.",
                            m_ecClass.GetFullName());
            return MappingStatus::Error;
            }
        }

    //Add ECInstanceId property map
    //check if it already exists
    if (GetECInstanceIdPropertyMap() != nullptr)
        return MappingStatus::Success;

    //does not exist yet
    PropertyMapPtr ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::Create(Schemas(), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        //log and assert already done in child method
        return MappingStatus::Error;

    if (GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

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
    ClassMap const* baseClassMap = mappingInfo.GetBaseClassMap();
    if (baseClassMap == nullptr) //this is the root of the TablePerHierarchy class hierarchy
        return true;

    //if base class doesn't have TablePerHierarchy, this class is the starting point, and therefore the exclusive root.
    //if base class has shared table strategy and is the direct parent of the joined table, this class is the
    //starting point of the joined table, so also the exclusive root (of the joined table)
    return baseClassMap->GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy ||
        baseClassMap->IsParentOfJoinedTable();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::DoMapPart2(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mappingInfo)
    {
    ClassMap const* baseClassMap = mappingInfo.GetBaseClassMap();

    MappingStatus stat = AddPropertyMaps(schemaImportContext.GetClassMapLoadContext(), baseClassMap, nullptr, &mappingInfo);
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
        PRECONDITION(baseClassMap != nullptr, MappingStatus::Error);
        if (&baseClassMap->GetJoinedTable() != &GetJoinedTable())
            {
            DbColumn const* primaryKeyColumn = baseClassMap->GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            DbColumn const* foreignKeyColumn = GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            PRECONDITION(primaryKeyColumn != nullptr, MappingStatus::Error);
            PRECONDITION(foreignKeyColumn != nullptr, MappingStatus::Error);
            bool createFKConstraint = true;
            for (DbConstraint const* constraint : GetJoinedTable().GetConstraints())
                {
                if (constraint->GetType() == DbConstraint::Type::ForeignKey)
                    {
                    ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
                    if (&fk->GetReferencedTable() == &baseClassMap->GetJoinedTable())
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

#define CURRENTIMESTAMP_SQLEXP "julianday('now')"

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::CreateCurrentTimeStampTrigger(ECPropertyCR currentTimeStampProp)
    {
    PropertyMapCP propertyMap = GetPropertyMap(currentTimeStampProp.GetName().c_str());
    if (propertyMap == nullptr)
        return SUCCESS;

    DbColumn* currentTimeStampColumn = const_cast<DbColumn*>(propertyMap->GetSingleColumn());
    if (currentTimeStampColumn == nullptr)
        {
        BeAssert(currentTimeStampColumn != nullptr && "TimeStamp column cannot be null");
        return ERROR;
        }

    if (currentTimeStampColumn->IsShared())
        {
        Issues().Report(ECDbIssueSeverity::Warning,
                   "ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
                   "ECDb therefore does not create the current timestamp trigger for this property.",
                           currentTimeStampProp.GetName().c_str(), currentTimeStampProp.GetClass().GetFullName());

        return SUCCESS;
        }

    BeAssert(currentTimeStampColumn->GetType() == DbColumn::Type::TimeStamp);
    currentTimeStampColumn->GetConstraintsR().SetDefaultValueExpression(CURRENTIMESTAMP_SQLEXP);
    currentTimeStampColumn->GetConstraintsR().SetNotNullConstraint();

    PropertyMapCP idPropMap = GetECInstanceIdPropertyMap();
    if (idPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable& table = currentTimeStampColumn->GetTableR();
    Utf8CP tableName = table.GetName().c_str();
    Utf8CP instanceIdColName = idPropMap->GetSingleColumn()->GetName().c_str();
    Utf8CP currentTimeStampColName = currentTimeStampColumn->GetName().c_str();

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
// @bsimethod                                                       Affan.Khan   02/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::ConfigureECClassId(std::vector<DbColumn const*> const& columns, bool loadingFromDisk)
    {
    if (!GetECDbMap().IsImportingSchema() && !loadingFromDisk)
        {
        BeAssert(false && "Can only be called during schema import");
        return ERROR;
        }

    PropertyMapCP classIdPropertyMap = GetECClassIdPropertyMap();
    if (classIdPropertyMap == nullptr)
        {
        PropertyMapPtr ecclassIdPropertyMap = ECClassIdPropertyMap::Create(Schemas(), *this, columns);
        if (ecclassIdPropertyMap == nullptr)
            //log and assert already done in child method
            return ERROR;

        return GetPropertyMapsR().AddPropertyMap(ecclassIdPropertyMap, 1);
        }
    std::vector<DbColumn const*>  existingColumns;
    classIdPropertyMap->GetColumns(existingColumns);
    if (existingColumns.size() != columns.size())
        {
        BeAssert(false && "Invalid classMap");
        return ERROR;
        }

    for (DbColumn const* col : existingColumns)
        {
        if (std::find(columns.begin(), columns.end(), col) == columns.end())
            {
            BeAssert(false && "Invalid classMap");
            return ERROR;
            }
        }

    for (DbColumn const* col : columns)
        {
        if (std::find(existingColumns.begin(), existingColumns.end(), col) == existingColumns.end())
            {
            BeAssert(false && "Invalid classMap");
            return ERROR;
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                       Affan.Khan   02/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::ConfigureECClassId(DbColumn const& classIdColumn, bool loadingFromDisk)
    {
    std::vector<DbColumn const*> columns;
    columns.push_back(&classIdColumn);
    return ConfigureECClassId(columns, loadingFromDisk);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::AddPropertyMaps(ClassMapLoadContext& ctx, ClassMap const* parentClassMap, ClassDbMapping const* classMapping, ClassMappingInfo const* classMapInfo)
    {
    const bool isJoinedTableMapping = GetMapStrategy().GetTphInfo().IsValid() && GetMapStrategy().GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
    const bool isImportingSchemas = classMapInfo != nullptr && classMapping == nullptr;
    if (!isImportingSchemas && isJoinedTableMapping)
        parentClassMap = nullptr;

    std::vector<ECPropertyCP> propertiesToCreatePropMapsFor;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        PropertyMapPtr propMapInBaseClass = nullptr;
        if (&property->GetClass() != &m_ecClass && parentClassMap != nullptr)
            parentClassMap->GetPropertyMaps().TryGetPropertyMap(propMapInBaseClass, property->GetName().c_str());

        if (propMapInBaseClass == nullptr)
            {
            propertiesToCreatePropMapsFor.push_back(property);
            continue;
            }

        if (!isJoinedTableMapping && propMapInBaseClass->GetType() != PropertyMap::Type::Navigation)
            {
            if (GetPropertyMapsR().AddPropertyMap(propMapInBaseClass) != SUCCESS)
                return MappingStatus::Error;

            continue;
            }

        //nav prop maps and if the class is mapped to primary and joined table, create clones of property maps of the base class
        //as the context (table, containing ECClass) is different
        if (isImportingSchemas)
            {
            if (GetPropertyMapsR().AddPropertyMap(PropertyMapFactory::ClonePropertyMap(m_ecDbMap, *propMapInBaseClass, GetClass(), nullptr)) != SUCCESS)
                return MappingStatus::Error;
            }
        else
            propertiesToCreatePropMapsFor.push_back(property);
        }

    if (isImportingSchemas)
        GetColumnFactoryR().Update();

    for (ECPropertyCP property : propertiesToCreatePropMapsFor)
        {
        Utf8CP propertyAccessString = property->GetName().c_str();
        PropertyMapPtr propMap = PropertyMapFactory::CreatePropertyMap(ctx, m_ecDbMap.GetECDb(), GetClass(), *property, propertyAccessString, nullptr);
        if (propMap == nullptr)
            return MappingStatus::Error;

        if (isImportingSchemas)
            {
            if (SUCCESS != propMap->FindOrCreateColumnsInTable(*this))
                {
                BeAssert(false);
                return MappingStatus::Error;
                }
            }
        else
            {
            if (ERROR == propMap->Load(*classMapping))
                {
                //ECSchema Upgrade
                GetColumnFactoryR().Update();
                if (SUCCESS != propMap->FindOrCreateColumnsInTable(*this))
                    {
                    BeAssert(false);
                    return MappingStatus::Error;
                    }
                }
            }

        if (GetPropertyMapsR().AddPropertyMap(propMap) != SUCCESS)
            return MappingStatus::Error;
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
            PropertyMapCP propertyMap = GetPropertyMap(propertyAccessString.c_str());
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

            std::vector<DbColumn const*> columns;
            propertyMap->GetColumns(columns);
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
        if (nullptr == m_ecDbMap.GetDbSchemaR().CreateIndex(*involvedTable, indexInfo->GetName(), indexInfo->GetIsUnique(),
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
BentleyStatus ClassMap::Save(SchemaImportContext& ctx)
    {
    if (!m_isDirty || !ctx.GetClassMapSaveContext().NeedsSaving(*this))
        return SUCCESS;

    std::set<PropertyMapCP> baseProperties;

    if (!GetId().IsValid())
        {
        ECClassCP baseClass = Schemas().GetECClass(GetBaseClassId());
        ClassMap* baseClassMap = baseClass == nullptr ? nullptr : const_cast<ClassMap*> (GetECDbMap().GetClassMap(*baseClass));
        if (baseClassMap != nullptr)
            {
            if (SUCCESS != baseClassMap->Save(ctx))
                return ERROR;

            for (PropertyMapCP propertyMap : baseClassMap->GetPropertyMaps())
                {
                baseProperties.insert(propertyMap);
                }
            }

        ClassDbMapping* mapping = m_ecDbMap.GetDbSchemaR().GetDbMappingsR().CreateClassMapping(GetClass().GetId(), m_mapStrategyExtInfo, baseClassMap == nullptr ? ClassMapId() : baseClassMap->GetId());
        for (PropertyMapCP propertyMap : GetPropertyMaps())
            {
            if (baseProperties.find(propertyMap) != baseProperties.end())
                continue;

            if (SUCCESS != propertyMap->Save(*mapping))
                return ERROR;
            }

        m_id = mapping->GetId();
        }

    m_isDirty = false;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ClassDbMapping const& classMapping, ClassMap const* baseClassMap)
    {
    if (baseClassMap != nullptr)
        m_baseClassId = baseClassMap->GetClass().GetId();

    std::vector<PropertyDbMapping const*> allPropertyMappings;
    classMapping.GetPropertyMappings(allPropertyMappings, true);

    std::set<DbTable*> tables;
    std::set<DbTable*> joinedTables;

    if (GetClass().GetName().EqualsI("ManyGooHasManyRoo"))
        {
        printf("Break");
        }

    if (allPropertyMappings.empty())
        {
        SetTable(*const_cast<DbTable*>(GetECDbMap().GetDbSchema().GetNullTable()));
        return SUCCESS;
        }
    else
        {
        for (PropertyDbMapping const* propMapping : allPropertyMappings)
            {
            for (DbColumn const* column : propMapping->GetColumns())
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
        }

    if (GetECInstanceIdPropertyMap() != nullptr)
        return ERROR;

    PropertyDbMapping const* ecInstanceIdMapping = classMapping.FindPropertyMapping(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME);
    if (ecInstanceIdMapping == nullptr)
        return ERROR;

    PropertyMapPtr ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::Create(Schemas(), *this, ecInstanceIdMapping->GetColumns());
    if (ecInstanceIdPropertyMap == nullptr)
        return ERROR;

    if (GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropertyMap) != SUCCESS)
        return ERROR;

    PropertyDbMapping const* ecClassIdMapping = classMapping.FindPropertyMapping(ECDbSystemSchemaHelper::ECCLASSID_PROPNAME);
    if (ecClassIdMapping == nullptr)
        return ERROR;

    if (ConfigureECClassId(ecClassIdMapping->GetColumns(), true) != SUCCESS)
        return ERROR;

    return AddPropertyMaps(ctx, baseClassMap, &classMapping, nullptr) == MappingStatus::Success ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECInstanceIdPropertyMap const* ClassMap::GetECInstanceIdPropertyMap() const
    {
    PropertyMapPtr propMap = nullptr;
    if (GetPropertyMaps().TryGetPropertyMap(propMap, ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME))
        return static_cast<ECInstanceIdPropertyMap const*>(propMap.get());

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
ECClassIdPropertyMap const* ClassMap::GetECClassIdPropertyMap() const
    {
    PropertyMapPtr propMap = nullptr;
    if (GetPropertyMaps().TryGetPropertyMap(propMap, ECDbSystemSchemaHelper::ECCLASSID_PROPNAME))
        return static_cast<ECClassIdPropertyMap const*>(propMap.get());

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManagerCR ClassMap::Schemas() const { return m_ecDbMap.GetECDb().Schemas(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//---------------------------------------------------------------------------------------
IssueReporter const& ClassMap::Issues() const { return m_ecDbMap.Issues(); }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP ClassMap::GetPropertyMap(Utf8CP propertyName) const
    {
    PropertyMapCP propMap = nullptr;
    if (GetPropertyMaps().TryGetPropertyMap(propMap, propertyName, true))
        return propMap;

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription const& ClassMap::GetStorageDescription() const
    {
    return GetECDbMap().GetLightweightCache().GetStorageDescription(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
std::vector<ClassMap const*> ClassMap::GetDerivedClassMaps() const
    {
    ECDerivedClassesList const& derivedClasses = m_ecDbMap.GetECDb().Schemas().GetDerivedECClasses(GetClass());
    std::vector<ClassMap const*> derivedClassMaps;
    for (ECClassCP derivedClass : derivedClasses)
        {
        ClassMap const* derivedClassMap = m_ecDbMap.GetClassMap(*derivedClass);
        derivedClassMaps.push_back(derivedClassMap);
        }

    return derivedClassMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
ClassMap const* ClassMap::GetParentOfJoinedTable() const
    {
    std::vector<ClassMap const*> path;
    if (GetPathToParentOfJoinedTable(path) != SUCCESS)
        return nullptr;

    return path.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::GetPathToParentOfJoinedTable(std::vector<ClassMap const*>& path) const
    {
    path.clear();
    ClassMap const* current = this;
    if (!current->HasJoinedTable() && !current->IsParentOfJoinedTable())
        return ERROR;

    path.insert(path.begin(), current);
    do
        {
        ECClassId nextBaseId = current->GetBaseClassId();
        if (!nextBaseId.IsValid())
            return SUCCESS;

        current = GetECDbMap().GetClassMap(nextBaseId);
        if (current == nullptr)
            {
            BeAssert(current != nullptr && "Failed to find parent classmap. This should not happen");
            return ERROR;
            }

        if (current->HasJoinedTable() || current->IsParentOfJoinedTable())
            path.insert(path.begin(), current);
        else
            return SUCCESS;
        } while (current != nullptr);

        path.clear();
        return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
ClassMap const* ClassMap::FindClassMapOfParentOfJoinedTable() const
    {
    ClassMap const* current = this;
    if (!current->HasJoinedTable())
        return nullptr;

    do
        {
        if (current->IsParentOfJoinedTable())
            return current;

        auto nextBaseId = current->GetBaseClassId();
        if (!nextBaseId.IsValid())
            return nullptr;

        current = GetECDbMap().GetClassMap(nextBaseId);
        if (current == nullptr)
            {
            BeAssert(current != nullptr && "Failed to find parent classmap. This should not happen");
            return nullptr;
            }

        } while (current != nullptr);

        return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  01/2016
//---------------------------------------------------------------------------------------
ClassMap const* ClassMap::FindTablePerHierarchyRootClassMap() const
    {
    if (m_mapStrategyExtInfo.GetStrategy() != MapStrategy::TablePerHierarchy)
        return nullptr;

    ECClassId parentId = GetBaseClassId();
    if (!parentId.IsValid())
        return this;

    ClassMap const* parent = GetECDbMap().GetClassMap(parentId);
    if (parent == nullptr)
        {
        BeAssert(false && "Failed to find parent classmap. This should not happen");
        return nullptr;
        }

    return parent->FindTablePerHierarchyRootClassMap();
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
        Utf8StringCR namespacePrefix = schema.GetNamespacePrefix();
        if (!namespacePrefix.empty())
            tablePrefix = namespacePrefix;
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
    name.Sprintf("_%s_%s", GetClass().GetSchema().GetNamespacePrefix().c_str(), GetClass().GetName().c_str());
    return name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   03/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::GenerateSelectViewSql(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const
    {
    return ViewGenerator::GenerateSelectViewSql(viewSql, *this, isPolymorphic, prepareContext);
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
//ColumnFactory
//=========================================================================================

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::ColumnFactory(ClassMapCR classMap) : m_classMap(classMap), m_usesSharedColumnStrategy(false)
    {
    TablePerHierarchyInfo const& tphInfo = m_classMap.GetMapStrategy().GetTphInfo();
    m_usesSharedColumnStrategy = tphInfo.IsValid() && tphInfo.UseSharedColumns();
    BeAssert(!m_usesSharedColumnStrategy || m_classMap.GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
    Update();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* ColumnFactory::CreateColumn(PropertyMapCR propMap, Utf8CP requestedColumnName, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        {
        // Shared column does not support NOT NULL constraint -> omit NOT NULL and issue warning
        if (addNotNullConstraint || addUniqueConstraint || collation != DbColumn::Constraints::Collation::Default)
            {
            m_classMap.GetECDbMap().Issues().Report(ECDbIssueSeverity::Warning, "For the ECProperty '%s' on ECClass '%s' either a 'not null', unique or collation constraint is defined. It is mapped "
                                                          "to a column though shared with other ECProperties. Therefore ECDb cannot enforce any of these constraints. "
                                                          "The column is created without constraints.",
                                                          propMap.GetProperty().GetName().c_str(), propMap.GetProperty().GetClass().GetFullName());

            }

        outColumn = ApplySharedColumnStrategy();
        }
    else
        outColumn = ApplyDefaultStrategy(requestedColumnName, propMap, colType, addNotNullConstraint, addUniqueConstraint, collation);

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
DbColumn* ColumnFactory::ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR propMap, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
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

    const ECClassId classId = GetPersistenceClassId(propMap);
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
ECClassId ColumnFactory::GetPersistenceClassId(PropertyMapCR propMap) const
    {
    Utf8String propAccessString(propMap.GetPropertyAccessString());
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
    return m_idsOfColumnsInUseByClassMap.find(column.GetId()) != m_idsOfColumnsInUseByClassMap.end();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Update()
    {
    m_idsOfColumnsInUseByClassMap.clear();
    std::vector<DbColumn const*> columnsInUse;
    m_classMap.GetPropertyMaps().Traverse(
        [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (propMap->GetType() != PropertyMap::Type::Navigation)
            propMap->GetColumns(columnsInUse);

        feedback = TraversalFeedback::Next;
        }, true);

    for (DbColumn const* columnInUse : columnsInUse)
        {
        if (columnInUse != nullptr)
            CacheUsedColumn(*columnInUse);
        }
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
        if (SUCCESS != propMap->Postprocess(ecdbMap))
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
    if (classMapInfo.GetBaseClassMap() != nullptr)
        m_baseClassId = classMapInfo.GetBaseClassMap()->GetBaseClassId();

    DbTable const* nullTable = GetECDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus NotMappedClassMap::_Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ClassDbMapping const& mapInfo, ClassMap const* parentClassMap)
    {
    DbTable const* nullTable = GetECDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
