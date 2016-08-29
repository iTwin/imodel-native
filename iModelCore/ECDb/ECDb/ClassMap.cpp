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
ClassMap::ClassMap(Type type, ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
    : m_type(type), m_ecDbMap(ecDbMap), m_ecClass(ecClass), m_mapStrategy(mapStrategy), 
    m_isDirty(setIsDirty), m_columnFactory(*this), m_isECInstanceIdAutogenerationDisabled(false)
    {
    if (SUCCESS != InitializeDisableECInstanceIdAutogeneration())
        {
        BeAssert(false && "InitializeDisableECInstanceIdAutogeneration failed");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                      04/2016
//---------------------------------------------------------------------------------------
void ClassMap::_WriteDebugInfo(DebugWriter& writer) const
    {
    Utf8String typeStr;
    switch (m_type)
        {
            case Type::Class: typeStr = "Class"; break;
            case Type::RelationshipEndTable:typeStr = "RelationshipEndTable"; break;
            case Type::RelationshipLinkTable:typeStr = "RelationshipLinkTable"; break;
            case Type::Unmapped:typeStr = "Unmapped"; break;
        }

    writer.AppendLine("%s : %s, [Id=%" PRId64 "], [MapId=%" PRId64 "], [Type=%s], [AutoGenECId=%s], [Dirty=%s]",
                      GetClass().IsRelationshipClass() ? "ECRelationshipClass" : "ECClass",
                      GetClass().GetFullName(),
                      GetClass().GetId().GetValue(),
                      m_id.GetValue(),
                      typeStr.c_str(),
                      m_isECInstanceIdAutogenerationDisabled ? "true" : "false",
                      m_isDirty ? "true" : "false"
                      );
    if (auto b0 = writer.CreateIndentBlock())
        {
        writer.AppendLine("MapStrategy : %s", m_mapStrategy.ToString().c_str());
        writer.AppendLine("Tables [Count=%d]", m_tables.size());
        if (auto b1 = writer.CreateIndentBlock())
            {
            for (auto table : m_tables)
                {
                writer.AppendLine("Table : %s, [Id=%" PRId64 "], [Virtual=%s], [Kind=%s], [BaseTable=%s]",
                                  table->GetName().c_str(),
                                  table->GetId().GetValue(),
                                  table->GetPersistenceType() == PersistenceType::Virtual ? "true" : "false",
                                  (table->GetType() == DbTable::Type::Existing ? "Existing" : (table->GetType() == DbTable::Type::Joined ? "Joined" : "Primary")),
                                  table->GetParentOfJoinedTable() != nullptr ? table->GetParentOfJoinedTable()->GetName().c_str() : "null"
                                  );
                }
            } //b1


        writer.AppendLine("PropertyMaps [Count=%d]", GetPropertyMaps().Size());
        if (auto b1 = writer.CreateIndentBlock())
            {
            for (auto& propertyMap : GetPropertyMaps())
                propertyMap->WriteDebugInfo(writer);
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::Map(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo)
    {
    ECDbMapStrategy const& mapStrategy = GetMapStrategy();
    ClassMap const* effectiveParentClassMap = (mapStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && mapStrategy.AppliesToSubclasses()) ? mapInfo.GetParentClassMap() : nullptr;

    auto stat = _MapPart1(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MappingStatus::Success)
        return stat;
    
    stat = _MapPart2(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MappingStatus::Success)
        return stat;

    return _OnInitialized();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::_MapPart1(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo, ClassMap const* parentClassMap)
    {
    DbTable const* primaryTable = nullptr;
    DbTable::Type tableType = DbTable::Type::Primary;
    if (Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable))
        {
        tableType = DbTable::Type::Joined;
        primaryTable = &parentClassMap->GetPrimaryTable();
        }
    else if (mapInfo.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable)
        tableType = DbTable::Type::Existing;

    auto findOrCreateTable = [&] ()
        {
        if (DbTable::Type::Joined == tableType)
            {
            BeAssert(primaryTable != nullptr);
            }

        DbTable* table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable(&schemaImportContext, mapInfo.GetTableName(), tableType,
            mapInfo.IsMapToVirtualTable(), mapInfo.GetECInstanceIdColumnName(), primaryTable);

        if (table == nullptr)
            return MappingStatus::Error;

        SetTable(*table, true /*append*/);
        return MappingStatus::Success;
        };

    BeAssert(tableType != DbTable::Type::Joined || parentClassMap != nullptr);
    if (parentClassMap != nullptr)
        {
        if (parentClassMap->GetMapStrategy().IsNotMapped())
            return MappingStatus::Error;

        m_parentMapClassId = parentClassMap->GetClass().GetId();
        SetTable(parentClassMap->GetPrimaryTable());

        if (tableType == DbTable::Type::Joined)
            {
            if (findOrCreateTable() == MappingStatus::Error)
                return MappingStatus::Error;
            }
        }
    else
        {
        if (findOrCreateTable() == MappingStatus::Error)
            return MappingStatus::Error;
        }

    if (mapInfo.GetMapStrategy().GetMinimumSharedColumnCount() != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        {
        if (SUCCESS != GetJoinedTable().SetMinimumSharedColumnCount(mapInfo.GetMapStrategy().GetMinimumSharedColumnCount()))
            {
            m_ecDbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                     "Only one ECClass per table can specify a minimum shared column count. Found duplicate definition on ECClass '%s'.",
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

    GetPropertyMapsR ().AddPropertyMap(ecInstanceIdPropertyMap);

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::_MapPart2(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo, ClassMap const* parentClassMap)
    {
    MappingStatus stat = AddPropertyMaps(schemaImportContext.GetClassMapLoadContext(), parentClassMap, nullptr, &mapInfo);
    if (stat != MappingStatus::Success)
        return stat;

    ECPropertyCP currentTimeStampProp = mapInfo.GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return MappingStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    if (isJoinedTable)
        {
        PRECONDITION(parentClassMap != nullptr, MappingStatus::Error);
        if (&parentClassMap->GetJoinedTable() != &GetJoinedTable())
            {
            DbColumn const* primaryKeyColumn = parentClassMap->GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            DbColumn const* foreignKeyColumn = GetJoinedTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            PRECONDITION(primaryKeyColumn != nullptr, MappingStatus::Error);
            PRECONDITION(foreignKeyColumn != nullptr, MappingStatus::Error);
            bool createFKConstraint = true;
            for (DbConstraint const* constraint : GetJoinedTable().GetConstraints())
                {
                if (constraint->GetType() == DbConstraint::Type::ForeignKey)
                    {
                    ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
                    if (&fk->GetReferencedTable() == &parentClassMap->GetJoinedTable())
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
        m_ecDbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                   "ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
                   "ECDb therefore does not create the current timestamp trigger for this property.",
                           currentTimeStampProp.GetName().c_str(), currentTimeStampProp.GetClass().GetFullName());

        return SUCCESS;
        }

    BeAssert(currentTimeStampColumn->GetType() == DbColumn::Type::TimeStamp);
    currentTimeStampColumn->GetConstraintR().SetDefaultExpression(CURRENTIMESTAMP_SQLEXP);
    currentTimeStampColumn->GetConstraintR().SetIsNotNull(true);

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
// @bsimethod                                                Affan.Khan   02/2016
//---------------------------------------------------------------------------------------
void ClassMap::SetTable(DbTable& newTable, bool append /*= false*/)
    {
    if (!append)
        m_tables.clear();

    m_tables.push_back(&newTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MappingStatus ClassMap::AddPropertyMaps(ClassMapLoadContext& ctx, ClassMap const* parentClassMap, DbClassMapLoadContext const* dbClassMapLoadContext, ClassMappingInfo const* classMapInfo)
    {
    const bool isJoinedTableMapping = Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    const bool isImportingSchemas = classMapInfo != nullptr && dbClassMapLoadContext == nullptr;
    if (dbClassMapLoadContext)
        parentClassMap = dbClassMapLoadContext->GetBaseClassMap();

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

        if (!isJoinedTableMapping && propMapInBaseClass->GetAsNavigationPropertyMap() == nullptr)
            {
            GetPropertyMapsR().AddPropertyMap(propMapInBaseClass);
            continue;
            }

        //nav prop maps and if the class is mapped to primary and joined table, create clones of property maps of the base class
        //as the context (table, containing ECClass) is different
        if (isImportingSchemas)
            GetPropertyMapsR().AddPropertyMap(PropertyMapFactory::ClonePropertyMap(m_ecDbMap, *propMapInBaseClass, GetClass(), nullptr));
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
            if (ERROR == propMap->Load(*dbClassMapLoadContext))
                {
                //ECSchema Upgrade
                GetColumnFactoryR().Update();
                if (SUCCESS != propMap->FindOrCreateColumnsInTable(*this))
                    {
                    BeAssert(false);
                    return MappingStatus::Error;
                    }
                //! ECSchema update added new property for which we need to save property map
                DbMapSaveContext ctx(GetECDbMap().GetECDb());
                //First make sure table is updated on disk. The table must already exist for this operation to work.
                if (GetECDbMap().GetDbSchema().UpdateTableOnDisk(*propMap->GetTable()) != SUCCESS)
                    {
                    BeAssert(false && "Failed to save table");
                    return MappingStatus::Error;
                    }

                ctx.BeginSaving(*this);
                DbClassMapSaveContext classMapContext(ctx);
                propMap->Save(classMapContext);
                ctx.EndSaving(*this);
                //!WIP save this property map
                //BeAssert(false);
        
                //we need to save this map
                }
            }

        GetPropertyMapsR().AddPropertyMap(propMap);
        }

    return MappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::CreateUserProvidedIndexes(SchemaImportContext& schemaImportContext, std::vector<IndexMappingInfoPtr> const& indexInfoList) const
    {
    int i = 0;
    IssueReporter const& issues = m_ecDbMap.GetECDb().GetECDbImplR().GetIssueReporter();
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
                issues.Report(ECDbIssueSeverity::Error, 
                   "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                   "The specified ECProperty '%s' does not exist or is not mapped.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            ECPropertyCR prop = propertyMap->GetProperty();
            NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
            if (!prop.GetIsPrimitive() && (navProp == nullptr || navProp->IsMultiple()))
                {
                issues.Report(ECDbIssueSeverity::Error,
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
                    issues.Report(ECDbIssueSeverity::Error,
                                  "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                                  "The specified ECProperty '%s' is mapped to a virtual column.",
                                  i, GetClass().GetFullName(), propertyAccessString.c_str());
                    return ERROR;
                    }

                DbTable const& table = column->GetTable();
                if (!involvedTables.empty() && involvedTables.find(&table) == involvedTables.end())
                    {
                    if (Enum::Intersects(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable | ECDbMapStrategy::Options::ParentOfJoinedTable))
                        {
                        issues.Report(ECDbIssueSeverity::Error,
                                      "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid. "
                                      "The properties that make up the index are mapped to different tables because the MapStrategy option '" USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS 
                                      "' is applied to this class hierarchy.",
                                      i, GetClass().GetFullName());
                        }
                    else
                        {
                        issues.Report(ECDbIssueSeverity::Error,
                                      "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid. "
                                      "The properties that make up the index are mapped to different tables.",
                                      i, GetClass().GetFullName());

                        BeAssert(false && "Properties of DbIndex are mapped to different tables although JoinedTable MapStrategy option is not applied.");
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
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
PropertyMapCP ClassMap::GetECInstanceIdPropertyMap() const
    {
    PropertyMapPtr propMap = nullptr;
    if (TryGetECInstanceIdPropertyMap(propMap))
        return propMap.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
bool ClassMap::TryGetECInstanceIdPropertyMap(PropertyMapPtr& ecInstanceIdPropertyMap) const
    {
    return GetPropertyMaps().TryGetPropertyMap(ecInstanceIdPropertyMap, ECInstanceIdPropertyMap::PROPERTYACCESSSTRING);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManagerCR ClassMap::Schemas() const
    {
    return GetECDbMap().GetECDb ().Schemas();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Save(DbMapSaveContext& ctx)
    {
    if (GetId().IsValid())
        return SUCCESS;

    if (ctx.IsAlreadySaved(*this))
        return SUCCESS;

    ECDbCR ecdb = GetECDbMap().GetECDb();
    ClassMapId baseClassMapid;
    std::set<PropertyMapCP> baseProperties;
    ctx.BeginSaving(*this);
    if (GetParentMapClassId().IsValid())
        {
        auto baseClass = ecdb.Schemas().GetECClass(GetParentMapClassId());
        if (baseClass == nullptr)
            {
            BeAssert(false && "Failed to find baseclass");
            return ERROR;
            }

        ClassMap* baseClassMap = (ClassMap*) GetECDbMap().GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            {
            BeAssert(false && "Failed to find baseClass map");
            return ERROR;
            }
        
        if (!baseClassMap->GetId().IsValid())
            {
            if (SUCCESS != baseClassMap->Save(ctx))
                return ERROR;
            }

        for (PropertyMapCP propertyMap : baseClassMap->GetPropertyMaps())
            baseProperties.insert(propertyMap);

        baseClassMapid = baseClassMap->GetId();
        }

    if (ctx.InsertClassMap(m_id, GetClass().GetId(), GetMapStrategy(), baseClassMapid) != SUCCESS)
        {
        m_id = ClassMapId();
        BeAssert(false);
        return ERROR;
        }

    DbClassMapSaveContext classMapSaveContext(ctx);
    for (PropertyMapCP propertyMap : GetPropertyMaps())
        {
        if (baseProperties.find(propertyMap) != baseProperties.end())
            continue;

        if (SUCCESS != propertyMap->Save(classMapSaveContext))
            return ERROR;
        }

    m_isDirty = false;
    ctx.EndSaving(*this);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    std::set<DbTable*> tables;
    std::set<DbTable*> joinedTables;

    if (!mapInfo.HasMappedProperties())
        {
        SetTable(*const_cast<DbTable*>(GetECDbMap().GetDbSchema().GetNullTable()));
        return SUCCESS;
        }
    else
        {
        for (auto const& propMapping : mapInfo.GetPropertyMaps())
            {
            std::vector<DbColumn const*> const& columns = propMapping.second;
            if (columns.front()->GetKind() == DbColumn::Kind::ECClassId)
                continue;

            for (DbColumn const* column : columns)
                {
                if (column->GetTable().GetType() == DbTable::Type::Joined)
                    joinedTables.insert(&column->GetTableR());
                else
                    tables.insert(&column->GetTableR());
                }
            }

        for (DbTable* table : tables)
            SetTable(*table, true);

        for (DbTable* table : joinedTables)
            SetTable(*table, true);

        BeAssert(!GetTables().empty());
        }

    if (GetECInstanceIdPropertyMap() != nullptr)
        return ERROR;

    std::vector<DbColumn const*> const* ecInstanceIdColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME);
    if (ecInstanceIdColumns == nullptr)
        return ERROR;

    PropertyMapPtr ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::Create(Schemas(), *this, *ecInstanceIdColumns);
    if (ecInstanceIdPropertyMap == nullptr)
        return ERROR;

    GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropertyMap);

    return AddPropertyMaps(ctx, nullptr, &mapInfo, nullptr) == MappingStatus::Success ? SUCCESS : ERROR;
    }

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
bool ClassMap::HasJoinedTable() const
    {
    return Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
bool ClassMap::IsParentOfJoinedTable() const
    {
    return Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::ParentOfJoinedTable);
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
        ECClassId nextParentId = current->GetParentMapClassId();
        if (!nextParentId.IsValid())
            return SUCCESS;

        current = GetECDbMap().GetClassMap(nextParentId);
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

        auto nextParentId = current->GetParentMapClassId();
        if (!nextParentId.IsValid())
            return nullptr;

        current = GetECDbMap().GetClassMap(nextParentId);
        if (current == nullptr)
            {
            BeAssert(current != nullptr && "Failed to find parent classmap. This should not happen");
            return nullptr;
            }

        } while (current != nullptr);

        return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2016
//---------------------------------------------------------------------------------------
ClassMap const* ClassMap::FindSharedTableRootClassMap() const
    {
    ECDbMapStrategy mapStrategy = GetMapStrategy();
    if (mapStrategy.GetStrategy() != ECDbMapStrategy::Strategy::SharedTable && !mapStrategy.AppliesToSubclasses())
        return nullptr;

    ECClassId parentId = GetParentMapClassId();
    if (!parentId.IsValid())
        return this;

    ClassMap const* parent = GetECDbMap().GetClassMap(parentId);
    if (parent == nullptr)
        {
        BeAssert(false && "Failed to find parent classmap. This should not happen");
        return nullptr;
        }

    return parent->FindSharedTableRootClassMap();
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
Utf8String ClassMap::GetPersistedViewName() const
    {
    Utf8String name;
    name.Sprintf("_%s_%s", GetClass().GetSchema().GetNamespacePrefix().c_str(), GetClass().GetName().c_str());
    return name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    12/2015
//---------------------------------------------------------------------------------------
bool ClassMap::HasPersistedView() const
    {
    return GetECDbMap().GetECDb().TableExists(GetPersistedViewName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                   03/2016
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::GenerateSelectView(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& prepareContext) const
    {
    return ViewGenerator::CreateView(viewSql, GetECDbMap(), *this, isPolymorphic, prepareContext, true /*optimizeByIncludingOnlyRealTables*/);
    }

//=========================================================================================
//ColumnFactory
//=========================================================================================

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::ColumnFactory(ClassMapCR classMap) : m_classMap(classMap), m_usesSharedColumnStrategy(false)
    {
    m_usesSharedColumnStrategy = Enum::Contains(m_classMap.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::SharedColumns);
    BeAssert(!m_usesSharedColumnStrategy || m_classMap.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable);

    Update();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* ColumnFactory::CreateColumn(PropertyMapCR propMap, Utf8CP requestedColumnName, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraint::Collation collation) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        {
        // Shared column does not support NOT NULL constraint -> omit NOT NULL and issue warning
        if (addNotNullConstraint || addUniqueConstraint || collation != DbColumn::Constraint::Collation::Default)
            {
            m_classMap.GetECDbMap().GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, "For the ECProperty '%s' on ECClass '%s' either a 'not null', unique or collation constraint is defined. It is mapped "
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
DbColumn* ColumnFactory::ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR propMap, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraint::Collation collation) const
    {
    BeAssert(!Utf8String::IsNullOrEmpty(requestedColumnName) && "Column name must not be null for default strategy");

    DbColumn* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && !IsColumnInUseByClassMap(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (!GetTable().IsOwnedByECDb() || (existingColumn->GetConstraint().IsNotNull() == addNotNullConstraint &&
                                            existingColumn->GetConstraint().IsUnique() == addUniqueConstraint &&
                                            existingColumn->GetConstraint().GetCollation() == collation))
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

    newColumn->GetConstraintR().SetIsNotNull(addNotNullConstraint);
    newColumn->GetConstraintR().SetIsUnique(addUniqueConstraint);
    newColumn->GetConstraintR().SetCollation(collation);

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
        if (propMap->GetAsNavigationPropertyMap() == nullptr)
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
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
const PropertyMapSet::EndPoints  PropertyMapSet::GetEndPoints () const
    {
    EndPoints endPoints;
    for (auto const& endPoint : m_orderedEndPoints)
        endPoints.push_back (endPoint.get ());

    return endPoints;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus PropertyMapSet::AddSystemEndPoint(PropertyMapSet& propertySet, ClassMap const& classMap, DbColumn::Kind kind, ECValueCR value, DbColumn const* column)
    {
    DbTable const& table = classMap.GetJoinedTable();

    if (column == nullptr)
        column = table.GetFilteredColumnFirst(kind);

    Utf8CP accessString = DbColumn::KindToString(kind);
    if (value.IsNull())
        {
        BeAssert(column != nullptr);
        if (column == nullptr)
            return ERROR;
        }

    BeAssert(accessString != nullptr);
    if (accessString == nullptr)
        return ERROR;

    if (column == nullptr)
        propertySet.m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint(accessString, kind, value)));
    else
        propertySet.m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint(accessString, *column, value)));

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
PropertyMapSet::Ptr PropertyMapSet::Create(ClassMap const& classMap)
    {
    BeAssert(!classMap.GetJoinedTable().IsNullTable());
    Ptr propertySet = Ptr(new PropertyMapSet());
    ECValue defaultValue;
    AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::ECInstanceId, defaultValue);
    AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::ECClassId, ECValue(classMap.GetClass().GetId().GetValue()));

    if (classMap.IsRelationshipClassMap())
        {
        RelationshipClassMapCR relationshipMap = static_cast<RelationshipClassMapCR>(classMap);
        auto const& sourceConstraints = relationshipMap.GetRelationshipClass().GetSource().GetClasses();
        auto const& targetConstraints = relationshipMap.GetRelationshipClass().GetTarget().GetClasses();
        auto sourceECInstanceIdColumn = relationshipMap.GetSourceECInstanceIdPropMap()->GetSingleColumn();
        auto sourceECClassIdColumn = relationshipMap.GetSourceECInstanceIdPropMap()->GetSingleColumn();
        auto targetECInstanceIdColumn = relationshipMap.GetTargetECInstanceIdPropMap()->GetSingleColumn();
        auto targetECClassIdColumn = relationshipMap.GetTargetECClassIdPropMap()->GetSingleColumn();


        AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::SourceECInstanceId, defaultValue, sourceECInstanceIdColumn);
        auto sourceConstraintClass = sourceConstraints.at(0);
        if (!ClassMap::IsAnyClass(*sourceConstraintClass) && sourceConstraints.size() == 1)
            AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::SourceECClassId, ECValue(sourceConstraintClass->GetId().GetValue()), sourceECClassIdColumn);
        else
            AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::SourceECClassId, defaultValue, sourceECClassIdColumn);

        AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::TargetECInstanceId, defaultValue, targetECInstanceIdColumn);
        auto targetConstraintClass = targetConstraints.at(0);
        if (!ClassMap::IsAnyClass(*targetConstraintClass) && targetConstraints.size() == 1)
            AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::TargetECClassId, ECValue(targetConstraintClass->GetId().GetValue()), targetECClassIdColumn);
        else
            AddSystemEndPoint(*propertySet, classMap, DbColumn::Kind::SourceECClassId, defaultValue, targetECClassIdColumn);
        }

    classMap.GetPropertyMaps().Traverse([&propertySet] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (auto pm = dynamic_cast<PointPropertyMap const*> (propMap))
            {
            std::vector<DbColumn const*> columns;
            pm->GetColumns(columns);
            Utf8String  baseAccessString = pm->GetPropertyAccessString();
            propertySet->m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint((baseAccessString + ".X").c_str(), *columns[0], ECValue())));
            propertySet->m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint((baseAccessString + ".Y").c_str(), *columns[1], ECValue())));
            if (pm->Is3d())
                propertySet->m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint((baseAccessString + ".Z").c_str(), *columns[2], ECValue())));
            }
        else if (nullptr != propMap->GetAsNavigationPropertyMap())
            {
            feedback = TraversalFeedback::NextSibling;
            }
        else if (!propMap->IsSystemPropertyMap() && !propMap->GetProperty().GetAsStructProperty())
            {
            propertySet->m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint(propMap->GetPropertyAccessString(), *propMap->GetSingleColumn(), ECValue())));
            }
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto const& ep : propertySet->m_orderedEndPoints)
        {
        propertySet->m_endPointByAccessString[ep->GetAccessString().c_str()] = ep.get();
        }

    return propertySet;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    01/2016
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapLoadContext::Postprocess(ECDbMapCR ecdbMap) 
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
MappingStatus UnmappedClassMap::_MapPart1(SchemaImportContext&, ClassMappingInfo const& classMapInfo, ClassMap const* parentClassMap)
    {
    if (parentClassMap != nullptr)
        m_parentMapClassId = parentClassMap->GetParentMapClassId();

    DbTable const* nullTable = GetECDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus UnmappedClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    DbTable const* nullTable = GetECDbMap().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
