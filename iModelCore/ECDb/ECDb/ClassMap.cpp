/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMap.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************* ClassDbView ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus ClassDbView::Generate(NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& preparedContext) const
    {
    if (m_classMap == nullptr)
        {
        BeAssert(false && "ClassDbView::Generate called but m_classMap is null");
        return ERROR;
        }

    if (m_classMap->GetMapStrategy().IsNotMapped())
        {
        BeAssert(false && "ClassDbView::Generate must not be called on unmapped class");
        return ERROR;
        }

    //ECSQL_TODO optimizeByIncludingOnlyRealTables result is optmize short queries but require some chatting with db to determine if table exist or not
    //           this feature need to be evaluated for performance before its enabled.
    return ViewGenerator::CreateView(viewSql, m_classMap->GetECDbMap(), *m_classMap, isPolymorphic, preparedContext, true /*optimizeByIncludingOnlyRealTables*/);
    }

//********************* IClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
IClassMap const& IClassMap::GetView(View classView) const
    {
    return _GetView(classView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection const& IClassMap::GetPropertyMaps() const
    {
    return _GetPropertyMaps();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP IClassMap::GetPropertyMap(Utf8CP propertyName) const
    {
    PropertyMapCP propMap = nullptr;
    if (GetPropertyMaps().TryGetPropertyMap(propMap, propertyName, true))
        return propMap;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool IClassMap::ContainsPropertyMapToTable() const
    {
    bool found = false;
    GetPropertyMaps().Traverse([&found] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert(propMap != nullptr);
        if (propMap->GetAsPropertyMapToTable() != nullptr)
            {
            found = true;
            feedback = TraversalFeedback::Cancel;
            }
        }, true);

    return found;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription const& IClassMap::GetStorageDescription() const
    {
    return GetECDbMap().GetLightweightCache().GetStorageDescription(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable& IClassMap::GetTable() const
    {
    return _GetTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
std::vector<IClassMap const*> IClassMap::GetDerivedClassMaps() const
    {
    auto const& ecdbMap = GetECDbMap();

    std::vector<IClassMap const*> derivedClassMaps;
    auto const& derivedClasses = ecdbMap.GetECDbR ().Schemas().GetDerivedECClasses(const_cast<ECClassR> (GetClass()));
    for (auto derivedClass : derivedClasses)
        {
        auto derivedClassMap = ecdbMap.GetClassMap(*derivedClass);
        derivedClassMaps.push_back(derivedClassMap);
        }

    return std::move(derivedClassMaps);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ECClassCR IClassMap::GetClass() const
    {
    return _GetClass();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ECClassId IClassMap::GetParentMapClassId() const
    {
    return _GetParentMapClassId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
ClassDbView const& IClassMap::GetDbView() const
    {
    return _GetDbView();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
IClassMap::Type IClassMap::GetClassMapType() const
    {
    return _GetClassMapType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbMapStrategy const& IClassMap::GetMapStrategy() const
    {
    ECDbMapStrategy const& strategy = _GetMapStrategy();
    BeAssert(strategy.IsValid() && "MapStrategy should have been resolved by the time it is hold in a class map.");
    return strategy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbMapCR IClassMap::GetECDbMap() const
    {
    return _GetECDbMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
bool IClassMap::IsRelationshipClassMap() const
    {
    const auto type = GetClassMapType();
    return type == Type::RelationshipEndTable || type == Type::RelationshipLinkTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
bool IClassMap::IsAbstractECClass() const
    {
    return IsAbstractECClass(GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsAbstractECClass(ECClassCR ecclass)
    {
    if (!ecclass.GetIsDomainClass() && !ecclass.GetIsStruct() && !ecclass.GetIsCustomAttributeClass())
        return true;

    //for relationship classes there is another criterion for abstractness: if one of the constraints doesn't have
    //any classes, then it is abstract. So check for that here now
    ECRelationshipClassCP relClass = ecclass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return false;
        
    return relClass->GetSource().GetClasses().empty() || relClass->GetTarget().GetClasses().empty();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsAnyClass(ECClassCR ecclass)
    {
    return ecclass.GetSchema().IsStandardSchema() && ecclass.GetName().Equals("AnyClass");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
bool IClassMap::IsMappedToSecondaryTable() const
    {
    return IsMapToSecondaryTableStrategy(GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsMapToSecondaryTableStrategy(ECN::ECClassCR ecClass)
    {
    return ecClass.GetIsStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
bool IClassMap::IsJoinedTable() const
    {
    return Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
bool IClassMap::IsParentOfJoinedTable() const
    {
    return Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::ParentOfJoinedTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
IClassMap const* IClassMap::FindParentOfJoinedTable() const
    {
    auto current = this;
    if (!current->IsJoinedTable())
        return nullptr;

    do
        {
        if (current->IsParentOfJoinedTable())
            return current;

        auto nextParentId = current->GetParentMapClassId();
        if (nextParentId == ECClass::UNSET_ECCLASSID)
            return nullptr;

        current = GetECDbMap().GetClassMapCP(nextParentId);
        BeAssert(current != nullptr && "Failed to find parent classmap. This should not happen");
        } while (current != nullptr);

        return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
const std::set<ECDbSqlTable const*> IClassMap::GetJoinedTables() const
    {
    std::set<ECDbSqlTable const*> secondaryTables;
    GetPropertyMaps().Traverse([&secondaryTables, this] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (!propMap->IsVirtual())
            {
            if (auto column = propMap->GetFirstColumn())
                {
                if (&column->GetTable() != &GetTable())
                    {
                    if (secondaryTables.find(&column->GetTable()) == secondaryTables.end())
                        secondaryTables.insert(&column->GetTable());
                    }
                }
            }
        feedback = TraversalFeedback::Next;
        }, true);

    return secondaryTables;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IClassMap::ToString() const
    {
    Utf8CP typeStr = nullptr;
    switch (GetClassMapType())
        {
        case IClassMap::Type::Class:
            typeStr = "Class";
            break;
        case IClassMap::Type::EmbeddedType:
            typeStr = "EmbeddedType";
            break;
        case IClassMap::Type::RelationshipEndTable:
            typeStr = "RelationshipEndTable";
            break;
        case IClassMap::Type::RelationshipLinkTable:
            typeStr = "RelationshipLinkTable";
            break;
        case IClassMap::Type::SecondaryTable:
            typeStr = "SecondaryTable";
            break;
        case IClassMap::Type::Unmapped:
            typeStr = "Unmapped";
            break;
        default:
            BeAssert(false && "Update ClassMap::ToString to handle new value in enum IClassMap::Type.");
            typeStr = "Unrecognized class map type";
            break;
        }

    Utf8String str;
    str.Sprintf("ClassMap '%s' - Type: %s - Map strategy: %s", GetClass().GetFullName(), typeStr, GetMapStrategy().ToString().c_str());

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus IClassMap::DetermineTableName(Utf8StringR tableName, ECN::ECClassCR ecclass, Utf8CP tablePrefix)
    {
    if (!Utf8String::IsNullOrEmpty(tablePrefix))
        tableName.assign(tablePrefix);
    else
        {
        if (SUCCESS != DetermineTablePrefix(tableName, ecclass))
            return ERROR;
        }

    tableName.append("_").append(ecclass.GetName());

    if (IsMapToSecondaryTableStrategy(ecclass))
        tableName.append(TABLESUFFIX_STRUCTARRAY);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus IClassMap::DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR ecclass)
    {
    ECSchemaCR schema = ecclass.GetSchema();
    ECDbSchemaMap customSchemaMap;

    if (ECDbMapCustomAttributeHelper::TryGetSchemaMap(customSchemaMap, schema))
        {
        if (customSchemaMap.TryGetTablePrefix(tablePrefix) != ECOBJECTS_STATUS_Success)
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


//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
    : IClassMap(), m_ecDbMap(ecDbMap), m_table(nullptr), m_ecClass(ecClass), m_mapStrategy(mapStrategy),
    m_parentMapClassId(ECClass::UNSET_ECCLASSID), m_dbView(nullptr), m_isDirty(setIsDirty), m_columnFactory(*this), m_id(0ULL)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::Initialize(SchemaImportContext* schemaImportContext, ClassMapInfo const& mapInfo)
    {
    m_isECInstanceIdAutogenerationDisabled = mapInfo.IsECInstanceIdAutogenerationDisabled();

    ECDbMapStrategy const& mapStrategy = GetMapStrategy();
    IClassMap const* effectiveParentClassMap = (mapStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && mapStrategy.AppliesToSubclasses()) ? mapInfo.GetParentClassMap() : nullptr;

    auto stat = _InitializePart1(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;
    
    stat = _InitializePart2(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    return _OnInitialized();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart1(SchemaImportContext* schemaImportContext, ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView(*this));
    bool isJoinedTable = Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    if (isJoinedTable)
        {
        PRECONDITION(parentClassMap != nullptr, MapStatus::Error);
        m_parentMapClassId = parentClassMap->GetClass().GetId();

        auto table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable(
            schemaImportContext,
            mapInfo.GetTableName(),
            mapInfo.IsMapToVirtualTable(),
            mapInfo.GetECInstanceIdColumnName(),
            IClassMap::IsMapToSecondaryTableStrategy(m_ecClass),
            mapInfo.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable);

        if (!EXPECTED_CONDITION(table != nullptr))
            return MapStatus::Error;

        m_table = table;
        }
    else
        {
        //if parent class map exists, its dbtable is reused.
        if (parentClassMap != nullptr)
            {
            PRECONDITION(!parentClassMap->GetMapStrategy().IsNotMapped(), MapStatus::Error);
            m_parentMapClassId = parentClassMap->GetClass().GetId();
            m_table = &parentClassMap->GetTable();
            }
        else
            {
            auto table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable(
                schemaImportContext,
                mapInfo.GetTableName(),
                mapInfo.IsMapToVirtualTable(),
                mapInfo.GetECInstanceIdColumnName(),
                IClassMap::IsMapToSecondaryTableStrategy(m_ecClass),
                mapInfo.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable);

            if (!EXPECTED_CONDITION(table != nullptr))
                return MapStatus::Error;

            m_table = table;
            }
        }
    //Add ECInstanceId property map
    //check if it already exists
    if (GetECInstanceIdPropertyMap() != nullptr)
        return MapStatus::Success;

    //does not exist yet
    PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create(Schemas(), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        //log and assert already done in child method
        return MapStatus::Error;

    GetPropertyMapsR ().AddPropertyMap(ecInstanceIdPropertyMap);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart2(SchemaImportContext* schemaImportContext, ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    MapStatus stat = AddPropertyMaps(schemaImportContext, parentClassMap, nullptr,&mapInfo);
    if (stat != MapStatus::Success)
        return stat;
    if (mapInfo.GetClassHasCurrentTimeStampProperty() != NULL)
        {
        PropertyMapCP propertyMap = GetPropertyMap(mapInfo.GetClassHasCurrentTimeStampProperty()->GetName().c_str());
        if (propertyMap != nullptr)
            {
            ECDbSqlColumn* column = const_cast<ECDbSqlColumn*>(propertyMap->GetFirstColumn());
            BeAssert(column != nullptr && "TimeStamp column cannot be null");
            if (column != nullptr)
                {
                //! TODO: Handle this case for shared column strategy;
                BeAssert(column->GetType() == ECDbSqlColumn::Type::DateTime);
                column->GetConstraintR().SetDefaultExpression("julianday('now')");
                column->GetConstraintR().SetIsNotNull(true);
                Utf8String whenCondtion;
                Utf8CP lastModName = column->GetName().c_str();
                whenCondtion.Sprintf("old.%s=new.%s AND old.%s!=julianday('now')", lastModName, lastModName, lastModName);
                Utf8String body;
                Utf8CP instanceId = GetPropertyMap("ECInstanceId")->GetFirstColumn()->GetName().c_str();
                body.Sprintf("BEGIN UPDATE %s SET %s=julianday('now') WHERE %s=new.%s; END", column->GetTableR().GetName().c_str(), lastModName, instanceId, instanceId);
                Utf8String triggerName;
                triggerName.Sprintf("%s_CurrentTimeStamp", column->GetTableR().GetName().c_str());
                column->GetTableR().CreateTrigger(triggerName.c_str(), column->GetTableR(), whenCondtion.c_str(), body.c_str(), TriggerType::Create, TriggerSubType::After);
                }
            }
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    if (isJoinedTable)
        {
        PRECONDITION(parentClassMap != nullptr, MapStatus::Error);
        if (&parentClassMap->GetTable() != &GetTable())
            {
            auto primaryKeyColumn = parentClassMap->GetTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
            auto foreignKeyColumn = GetTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
            PRECONDITION(primaryKeyColumn != nullptr, MapStatus::Error);
            PRECONDITION(foreignKeyColumn != nullptr, MapStatus::Error);
            bool createFKConstraint = true;
            for (auto constraint : GetTable().GetConstraints())
                {
                if (constraint->GetType() == ECDbSqlConstraint::Type::ForeignKey)
                    {
                    auto fk = static_cast<ECDbSqlForeignKeyConstraint const*>(constraint);
                    if (&fk->GetTargetTable() == &parentClassMap->GetTable())
                        {
                        if (fk->GetSourceColumns().front() == foreignKeyColumn && fk->GetTargetColumns().front() == primaryKeyColumn)
                            {
                            createFKConstraint = false;
                            break;
                            }
                        }
                    }
                }

            if (createFKConstraint)
                {
                auto fkConstraint = GetTable().CreateForeignKeyConstraint(parentClassMap->GetTable());
                fkConstraint->Add(foreignKeyColumn->GetName().c_str(), primaryKeyColumn->GetName().c_str());
                fkConstraint->SetOnDeleteAction(ForeignKeyActionType::Cascade);
                }
            }
        }
	if (schemaImportContext != nullptr)
        return ProcessStandardKeySpecifications(*schemaImportContext, mapInfo) == SUCCESS ? MapStatus::Success : MapStatus::Error;

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_OnInitialized()
    {
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::AddPropertyMaps(SchemaImportContext* schemaImportContext, IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo,ClassMapInfo const* classMapInfo)
    {

    bool isJoinedTable = isJoinedTable = Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    bool isMappingPhase = classMapInfo != nullptr && loadInfo == nullptr;
    if (!isMappingPhase && isJoinedTable)
        parentClassMap = nullptr;

    std::vector<ECPropertyP> propertiesToMap;
    PropertyMapPtr propMap = nullptr;
    for (auto property : m_ecClass.GetProperties(true))
        {
        Utf8CP propertyAccessString = property->GetName().c_str();
        propMap = nullptr;
        if (&property->GetClass() != &m_ecClass && parentClassMap != nullptr)
            parentClassMap->GetPropertyMaps().TryGetPropertyMap(propMap, propertyAccessString);

        if (propMap == nullptr)
            propertiesToMap.push_back(property);
        else
            {
            if (!isJoinedTable)
                GetPropertyMapsR().AddPropertyMap(propMap);
            else
                GetPropertyMapsR().AddPropertyMap(propMap->Clone(&GetTable()));
            }
        }

    if (loadInfo == nullptr)
        GetColumnFactoryR().Update();

    for (auto property : propertiesToMap)
        {
        Utf8CP propertyAccessString = property->GetName().c_str();
        propMap = PropertyMap::CreateAndEvaluateMapping(*property, m_ecDbMap, m_ecClass, propertyAccessString, &GetTable(), nullptr);
        if (propMap == nullptr)
            return MapStatus::Error;

        if (GetPropertyMap(propertyAccessString) != nullptr)
            {
            BeAssert(GetPropertyMap(propertyAccessString) == nullptr && " it should not be there");
            return MapStatus::Error;
            }

        if (loadInfo == nullptr)
            {
            if (SUCCESS == propMap->FindOrCreateColumnsInTable(schemaImportContext, *this, classMapInfo))
                GetPropertyMapsR().AddPropertyMap(propMap);
            }
        else
            {
            if (propMap->Load(*loadInfo) == SUCCESS)
                GetPropertyMapsR().AddPropertyMap(propMap);
            }
        }

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::ProcessStandardKeySpecifications(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo)
    {
    std::set<PropertyMapCP> doneList;
    std::set<Utf8String> specList;
    for (StandardKeySpecificationPtr spec : mapInfo.GetStandardKeys())
        {
        BeAssert(spec->GetKeyProperties().size() > 0);

        if (spec->GetKeyProperties().size() == 0)
            continue;

        Utf8String propertyName = spec->GetKeyProperties().front();
        Utf8String typeString = StandardKeySpecification::TypeToString(spec->GetType());
        if (specList.find(typeString) != specList.end())
            continue;

        specList.insert(typeString);
        auto propertyMap = GetPropertyMap(propertyName.c_str());
        if (propertyMap == nullptr)
            {
            LOG.warningv("Column index creation is ignoring %s on %s because map for ECProperty '%s' cannot be found", typeString.c_str(), GetClass().GetFullName(), propertyName.c_str());
            continue;
            }
        //We don't want to create multiple indexes on same column.
        if (doneList.find(propertyMap) != doneList.end())
            {
            LOG.warningv("Ignoring %s for property %s.%s. It is already part of another index.", typeString.c_str(), GetClass().GetFullName(), propertyName.c_str());
            continue;
            }
        doneList.insert(propertyMap);

        Utf8String indexName;
        indexName.Sprintf("ix_%s_%s_%s_%s", 
                          mapInfo.GetECClass().GetSchema().GetNamespacePrefix().c_str(),
                          mapInfo.GetECClass().GetName().c_str(), 
                          typeString.c_str(), 
                          propertyName.c_str());

        std::vector<ECDbSqlColumn const*> columns;
        propertyMap->GetColumns(columns);

        if (nullptr == schemaImportContext.GetECDbMapDb().CreateIndex(GetECDbMap().GetECDbR(), *m_table, indexName.c_str(), false, columns, nullptr,
                                            true, GetClass().GetId()))
            {
            BeAssert(false && "Index was not created correctly");
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::CreateUserProvidedIndices(SchemaImportContext& schemaImportContext, ClassMapInfo const& classMapInfo) const
    {
    int i = 0;
    IssueReporter const& issues = m_ecDbMap.GetECDbR().GetECDbImplR().GetIssueReporter();
    for (ClassIndexInfoPtr indexInfo : classMapInfo.GetIndexInfo())
        {
        i++;

        std::vector<ECDbSqlColumn const*> totalColumns;
        NativeSqlBuilder whereExpression;

        bset<ECDbSqlTable const*> involvedTables;
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

            if (!propertyMap->GetProperty().GetIsPrimitive())
                {
                issues.Report(ECDbIssueSeverity::Error,
                              "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                              "The specified ECProperty '%s' is not of a primitive type.",
                              i, GetClass().GetFullName(), propertyAccessString.c_str());
                return ERROR;
                }

            std::vector<ECDbSqlColumn const*> columns;
            propertyMap->GetColumns(columns);
            if (0 == columns.size())
                {
                BeAssert(false && "Reject user defined index on %s. Fail to find column property map for property. Something wrong with mapping");
                return ERROR;
                }

            for (ECDbSqlColumn const* column : columns)
                {
                if (column->GetPersistenceType() == PersistenceType::Virtual)
                    {
                    issues.Report(ECDbIssueSeverity::Error,
                                  "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid: "
                                  "The specified ECProperty '%s' is mapped to a virtual column.",
                                  i, GetClass().GetFullName(), propertyAccessString.c_str());
                    return ERROR;
                    }

                ECDbSqlTable const& table = column->GetTable();
                if (!involvedTables.empty() && involvedTables.find(&table) == involvedTables.end())
                    {
                    if (Enum::Intersects(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable | ECDbMapStrategy::Options::ParentOfJoinedTable))
                        {
                        issues.Report(ECDbIssueSeverity::Error,
                                      "DbIndex #%d defined in ClassMap custom attribute on ECClass '%s' is invalid. "
                                      "The properties that make up the index are mapped to different tables because the MapStrategy option '" USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS 
                                      "' or '" USERMAPSTRATEGY_OPTIONS_SINGLEJOINEDTABLEFORSUBCLASSES "'  is applied to this class hierarchy.",
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
                switch (indexInfo->GetWhere())
                    {
                        case EC::ClassIndexInfo::WhereConstraint::NotNull:
                        {
                        //if column is not nullable, no need to add IS NOT NULL expression to where clause of index
                        if (column->GetConstraint().IsNotNull())
                            break;

                        if (!whereExpression.IsEmpty())
                            whereExpression.AppendSpace().Append(BooleanSqlOperator::And, true);

                        whereExpression.AppendEscaped(column->GetName().c_str()).AppendSpace();
                        whereExpression.Append(BooleanSqlOperator::IsNot).Append("NULL");

                        break;
                        }

                        default:
                            break;
                    }
                }
            }

        if (nullptr == schemaImportContext.GetECDbMapDb().CreateIndex(m_ecDbMap.GetECDbR(), GetTable(), indexInfo->GetName(), indexInfo->GetIsUnique(),
                                                                      totalColumns, whereExpression.ToString(),
                                                                      false, GetClass().GetId()))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2011
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ClassMap::FindOrCreateColumnForProperty(SchemaImportContext* schemaImportContext, ClassMapCR classMap,ClassMapInfo const* classMapInfo, PropertyMapR propertyMap, Utf8CP requestedColumnName, PrimitiveType columnType, bool nullable, bool unique, ECDbSqlColumn::Constraint::Collation collation, Utf8CP accessStringPrefix)
    {
    ColumnFactory::Specification::Strategy strategy = ColumnFactory::Specification::Strategy::CreateOrReuse;
    ColumnFactory::Specification::GenerateColumnNameOptions generateColumnNameOpts = ColumnFactory::Specification::GenerateColumnNameOptions::NameBasedOnClassIdAndCaseSaveAccessString;
    ECDbSqlColumn::Type requestedColumnType = ECDbSqlHelper::PrimitiveTypeToColumnType(columnType);
 
    if (Enum::Contains(classMap.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::SharedColumns))
        {
        BeAssert(classMap.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable);
        strategy = ColumnFactory::Specification::Strategy::CreateOrReuseSharedColumn;
        requestedColumnType = ECDbSqlColumn::Type::Any; //If not set it will get set anyway
        generateColumnNameOpts = ColumnFactory::Specification::GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence;
        }
    
    auto spec = ColumnFactory::Specification 
        (        
        propertyMap, 
        strategy, 
        generateColumnNameOpts, 
        requestedColumnName, 
        requestedColumnType, 
        ColumnKind::DataColumn, 
        PersistenceType::Persisted, 
        accessStringPrefix, 
        !nullable, 
        unique, 
        collation
        );

    return GetColumnFactoryR().Configure(schemaImportContext, spec);
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
    return GetPropertyMaps().TryGetPropertyMap(ecInstanceIdPropertyMap, PropertyMapECInstanceId::PROPERTYACCESSSTRING);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection const& ClassMap::_GetPropertyMaps() const
    {
    return m_propertyMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection& ClassMap::GetPropertyMapsR ()
    {
    return m_propertyMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManagerCR ClassMap::Schemas() const
    {
    return GetECDbMap().GetECDbR ().Schemas();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
IClassMap::Type ClassMap::_GetClassMapType() const
    {
    return Type::Class;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Save(std::set<ClassMap const*>& savedGraph)
    {
    if (savedGraph.find(this) != savedGraph.end())
        return BentleyStatus::SUCCESS;

    savedGraph.insert(this);
    auto& mapStorage = const_cast<ECDbMapR>(m_ecDbMap).GetSQLManagerR ().GetMapStorageR ();
    std::set<PropertyMapCP> baseProperties;

    if (GetId() == 0ULL)
        {
        //auto baseClassMap = GetParentMapClassId () == 
        auto baseClass = GetECDbMap().GetECDbR ().Schemas().GetECClass(GetParentMapClassId());
        auto baseClassMap = baseClass == nullptr ? nullptr : (ClassMap*)GetECDbMap().GetClassMap(*baseClass);
        if (baseClassMap != nullptr)
            {
            auto r = baseClassMap->Save(savedGraph);
            if (r != BentleyStatus::SUCCESS)
                return r;

            for (auto propertyMap : baseClassMap->GetPropertyMaps())
                {
                baseProperties.insert(propertyMap);
                }
            }

        
        auto mapInfo = mapStorage.CreateClassMap(GetClass().GetId(), m_mapStrategy, baseClassMap == nullptr ? ECClass::UNSET_ECCLASSID : baseClassMap->GetId());
        for (auto propertyMap : GetPropertyMaps())
            {
            if (baseProperties.find(propertyMap) != baseProperties.end())
                continue;

            if (SUCCESS != propertyMap->Save(*mapInfo))
                return ERROR;
            }

        m_id = mapInfo->GetId();
        }

        m_isDirty = false;
        return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView(*this));
    if (parentClassMap)
        m_parentMapClassId = parentClassMap->GetClass().GetId();

    auto& propertyMapInfos = mapInfo.GetPropertyMaps(true);
    if (propertyMapInfos.empty())
        {
        SetTable(const_cast<ECDbSqlTable*>(GetECDbMap().GetSQLManager().GetNullTable()));
        }
    else
        {
        //! Following code determine table for a classMap which could be different from what orignally set. 
        //! We do not store primary table for a classmap.
        //! the best shot of getting it right is to look for ECInstanceId table.

        ECDbPropertyMapInfo const* ecInstanceIdPropertyMapInfo = nullptr;
        for (auto propertyMapInfo : propertyMapInfos)
            {
            if (propertyMapInfo->GetColumn().GetKind() == ColumnKind::ECInstanceId)
                {
                ecInstanceIdPropertyMapInfo = propertyMapInfo;
                break;
                }
            }

        if (ecInstanceIdPropertyMapInfo == nullptr)
            {
            SetTable(const_cast<ECDbSqlTable*>(&(propertyMapInfos.front()->GetColumn().GetTable())));
            }
        else
            SetTable(const_cast<ECDbSqlTable*>(&(ecInstanceIdPropertyMapInfo->GetColumn().GetTable())));
        }

    if (GetECInstanceIdPropertyMap() != nullptr)
        return BentleyStatus::ERROR;

    PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create(Schemas(), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        return BentleyStatus::ERROR;

    GetPropertyMapsR ().AddPropertyMap(ecInstanceIdPropertyMap);

    return AddPropertyMaps(nullptr, parentClassMap, &mapInfo, nullptr) == MapStatus::Success ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
ECPropertyCP ClassMap::GetECProperty(ECN::ECClassCR ecClass, Utf8CP propertyAccessString)
    {
    bvector<Utf8String> tokens;
    ECDbMap::ParsePropertyAccessString(tokens, propertyAccessString);

    //for recursive lambdas, iOS requires us to efine the lambda variable before assigning the actual function to it.
    std::function<ECPropertyCP (ECClassCR, bvector<Utf8String>&, int)> getECPropertyFromTokens;
    getECPropertyFromTokens = [&getECPropertyFromTokens] (ECClassCR ecClass, bvector<Utf8String>& tokens, int iCurrentToken) -> ECPropertyCP
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP (tokens[iCurrentToken].c_str(), true);
        if (!ecProperty)
            return nullptr;

        if (iCurrentToken == tokens.size() - 1)
            return ecProperty; // we are the last token

        // There are more tokens... delving into an embedded struct
        StructECPropertyCP structProperty = ecProperty->GetAsStructProperty();
        if (structProperty)
            return getECPropertyFromTokens(structProperty->GetType(), tokens, iCurrentToken + 1);

        BeAssert(false && "Any second-to-last ECProperty has to be a struct!");
        return nullptr;
        };

    return getECPropertyFromTokens(ecClass, tokens, 0);
    }

//************************** MappedTable ***************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTable::MappedTable(ECDbMapR ecDbMap, ClassMapCR classMap) : m_table(classMap.GetTable()), m_ecDbMap(ecDbMap), m_generatedClassIdColumn(false)
    {
    m_classMaps.push_back(&classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTablePtr MappedTable::Create(ECDbMapR ecDbMap, ClassMapCR classMap)
    {
    return new MappedTable(ecDbMap, classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MappedTable::FinishTableDefinition(ECDbCR ecdb, SchemaImportContext& schemaImportContext)
    {
    if (m_table.GetOwnerType() == OwnerType::ECDb)
        {
        int nOwners = 0;
        bool sharedTableWithAppliesToSubclasses = false;
        for (auto classMap : m_classMaps)
            {
            if (!classMap->GetMapStrategy().IsNotMapped() && classMap->GetClassMapType() != ClassMap::Type::RelationshipEndTable)
                {
                nOwners++;
                ECDbMapStrategy const& mapStrategy = classMap->GetMapStrategy();
                if (mapStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && mapStrategy.AppliesToSubclasses())
                    sharedTableWithAppliesToSubclasses = true;
                }
            }

        if (sharedTableWithAppliesToSubclasses || nOwners > 1)
            {
            if (!m_generatedClassIdColumn)
                {
                if (m_table.FindColumnCP(ECDB_COL_ECClassId) == nullptr)
                    {
                    const size_t insertPosition = 1;
                    ECDbSqlColumn const* ecClassIdColumn = m_table.CreateColumn(ECDB_COL_ECClassId, ECDbSqlColumn::Type::Long, insertPosition, ColumnKind::ECClassId, PersistenceType::Persisted);
                    if (ecClassIdColumn == nullptr)
                        return ERROR;

                    //whenever we create a class id column, we index it to speed up the frequent class id look ups
                    Utf8String indexName("ix_");
                    indexName.append(m_table.GetName()).append("_ecclassid");
                    schemaImportContext.GetECDbMapDb().CreateIndex(ecdb, m_table, indexName.c_str(), false, {ecClassIdColumn}, nullptr, true, ECClass::UNSET_ECCLASSID);
                    }

                m_generatedClassIdColumn = true;
                }

            else
                return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MappedTable::AddClassMap(ClassMapCR classMap)
    {
    if (&classMap.GetTable() != &m_table)
        {
        LOG.errorv("Attempted to add a ClassMap for table '%s' to MappedTable for table '%s'.", classMap.GetTable().GetName().c_str(), m_table.GetName().c_str());
        return ERROR;
        }

    if (std::find(m_classMaps.begin(), m_classMaps.end(), &classMap) != m_classMaps.end())
        return SUCCESS;

    m_classMaps.push_back(&classMap);
    return SUCCESS;
    }

//=========================================================================================
//ColumnFactory
//=========================================================================================
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::Specification::Specification(
    PropertyMapR propertyMap,
    Specification::Strategy strategy,
    Specification::GenerateColumnNameOptions generateColumnNameOptions,
    Utf8CP columnName,
    ECDbSqlColumn::Type columnType,
    ColumnKind kind,
    PersistenceType persistenceType,
    Utf8CP accessStringPrefix,
    bool isNotNull,
    bool isUnique,
    ECDbSqlColumn::Constraint::Collation collation)
    : m_propertyMap(propertyMap), m_requestedColumnName(columnName), m_columnType(columnType), m_isNotNull(isNotNull),
    m_isUnique(isUnique), m_collation(collation), m_generateColumnNameOptions(generateColumnNameOptions),
    m_columnKind(kind), m_persistenceType(persistenceType), m_strategy(strategy)
    {
    m_accessString = propertyMap.GetPropertyAccessString();
    if (!Utf8String::IsNullOrEmpty(accessStringPrefix))
        m_accessString.append(".").append(accessStringPrefix);

    if (GetStrategy() == Strategy::Create)
        {
        //Column name must be not null
        BeAssert(Utf8String::IsNullOrEmpty(columnName) == false && "Must supply a valid columnName if Strategy::Create is used");
        }
    else if (GetStrategy() == Strategy::CreateOrReuse)
        {
        //Column name must be not null
        BeAssert(Utf8String::IsNullOrEmpty(columnName) == false && "Must supply a valid columnName if Strategy::Create is used");
        }
    else if (GetStrategy() == Strategy::CreateOrReuseSharedColumn)
        {
        // Shared column does not support NOT NULL constraint
        BeAssert(isNotNull == false && "Shared column cannot enforce NOT NULL constraint.");
        BeAssert(persistenceType == PersistenceType::Persisted);
        BeAssert(Enum::Contains(kind, ColumnKind::DataColumn));
        m_generateColumnNameOptions = GenerateColumnNameOptions::NameBasedOnPropertyNameAndPropertyId;
        m_requestedColumnName.clear();
        m_columnType = ECDbSqlColumn::Type::Any;
        m_isNotNull = false;
        }

    if (persistenceType == PersistenceType::Virtual)
        {
        m_isUnique = false;
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::ColumnFactory(ClassMapCR classMap)
    :m_classMap(classMap)
    {
    Update();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::RegisterColumnInUse(ECDbSqlColumn const& column)
    {
    columnsInUseSet.insert(column.GetFullName());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Reset()
    {
    columnsInUseSet.clear();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse(Utf8CP columnFullName) const
    {
    return columnsInUseSet.find(columnFullName) != columnsInUseSet.end();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse(Utf8CP tableName, Utf8CP columnName) const
    {
    return IsColumnInUse(ECDbSqlColumn::BuildFullName(tableName, columnName).c_str());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse(ECDbSqlColumn const& column) const
    {
    return IsColumnInUse(column.GetFullName().c_str());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Update()
    {
    std::vector<ECDbSqlColumn const*> columnsInUse;
    Reset();
    m_classMap.GetPropertyMaps().Traverse(
        [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        propMap->GetColumns(columnsInUse);
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto columnInUse : columnsInUse)
        {
        if (columnInUse)
            RegisterColumnInUse(*columnInUse);
        }
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::TryFindReusableSharedDataColumn(ECDbSqlColumn const*& reusableColumn, ECDbSqlTable const& table, ECDbSqlColumn::Constraint::Collation collation) const
    {
    reusableColumn = nullptr;
    std::vector<ECDbSqlColumn const*> reusableColumns;
    for (auto column : table.GetColumns())
        {
        if (Enum::Contains(column->GetKind(), ColumnKind::DataColumn) && column->GetType() == ECDbSqlColumn::Type::Any && collation == column->GetConstraint().GetCollation())
            {
            if (!IsColumnInUse(*column))
                reusableColumns.push_back(column);
            }
        }

    if (reusableColumns.empty())
        return false;

    reusableColumn = reusableColumns.front();
    return true;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
const Utf8String ColumnFactory::Encode(Utf8StringCR acessString) const
    {
    Utf8String o;
    for (Utf8Char c : acessString)
        {
        if (c == '.')
            {
            o.append("_");
            }
        else if (islower(c))
            {
            o+= c;
            }
        else
            {
            o.append(SqlPrintfString("_%02x_", c));
            }
        }

    return o;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus ColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, ColumnFactory::Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert(!resolvedColumName.empty());
        resolvedColumName += SqlPrintfString("%d", retryCount);
        return BentleyStatus::SUCCESS;
        }

    auto existingColumn = specifications.GetColumnName().empty() ? nullptr : targetTable.FindColumnP (specifications.GetColumnName().c_str());
    if (existingColumn != nullptr && IsColumnInUse(*existingColumn))
        {
        switch (specifications.GetGenerateColumnNameOptions())
            {
            case Specification::GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence:
                BeAssert(false && "Not implemented. It used for shared column only");
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnPropertyNameAndPropertyId:

                resolvedColumName.Sprintf("%s_%lld", specifications.GetColumnName().c_str(), specifications.GetPropertyMap().GetProperty().GetId());
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnClassIdAndCaseSaveAccessString:
                {
                auto const& prefix = Encode(specifications.GetAccessString());
                resolvedColumName.Sprintf("c%lld_%s", propertyLocalToClassId, prefix.c_str());
                }
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnClassAndPropertyName:
                resolvedColumName = specifications.GetPropertyMap().GetProperty().GetClass().GetName().c_str();
                resolvedColumName.append("_").append(specifications.GetColumnName());
                break;
            case Specification::GenerateColumnNameOptions::NeverGenerate:
                return BentleyStatus::ERROR;
            }
        }
    else
        {
        resolvedColumName = specifications.GetColumnName();
        }

    return BentleyStatus::SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateStrategy(ColumnFactory::Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (ResolveColumnName(tmp, specifications, targetTable, propertyLocalToClassId, retryCount) == BentleyStatus::ERROR)
        {
        return nullptr;
        }

    resolvedColumnName = tmp;
    while (targetTable.FindColumnP (resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (ResolveColumnName(resolvedColumnName, specifications, targetTable, propertyLocalToClassId, retryCount) == BentleyStatus::ERROR)
            {
            return nullptr;
            }

        }

    auto canEdit = targetTable.GetEditHandle().CanEdit();
    if (!canEdit)
        targetTable.GetEditHandleR ().BeginEdit();

    auto newColumn = targetTable.CreateColumn(resolvedColumnName.c_str(), specifications.GetColumnType(), specifications.GetColumnKind(), specifications.GetColumnPersistenceType());
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    newColumn->GetConstraintR ().SetIsNotNull(specifications.IsNotNull());
    newColumn->GetConstraintR ().SetIsUnique(specifications.IsUnique());
    newColumn->GetConstraintR ().SetCollation(specifications.GetCollation());

    if (!canEdit)
        targetTable.GetEditHandleR ().EndEdit();

    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateOrReuseStrategy(Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    auto existingColumn = specifications.GetColumnName().empty() ? nullptr : targetTable.FindColumnP (specifications.GetColumnName().c_str());
    if (existingColumn != nullptr && !IsColumnInUse(*existingColumn))
        {
        if (ECDbSqlHelper::IsCompatible(existingColumn->GetType(), specifications.GetColumnType()))
            {
            if (GetTable().GetOwnerType() == OwnerType::ECDb)
                {
                if (existingColumn->GetConstraint().IsNotNull() != specifications.IsNotNull() || existingColumn->GetConstraint().IsUnique() != specifications.IsUnique() || existingColumn->GetConstraint().GetCollation() != specifications.GetCollation())
                    {
                    LOG.warningv("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                        " but where 'Nullable', 'Unique', or 'Collation' differs, and which will therefore be ignored for some of the properties.",
                        existingColumn->GetName().c_str(), GetTable().GetName().c_str());

                    BeAssert(false && "A column is used by multiple property maps where property name and data type matches, "
                        " but where 'Nullable', 'Unique', or 'Collation' differs.");

                    return ApplyCreateStrategy(specifications, targetTable, propertyLocalToClassId);
                    }
                }

            return existingColumn;
            }
        }

    return ApplyCreateStrategy(specifications, targetTable, propertyLocalToClassId);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateOrReuseSharedColumnStrategy(SchemaImportContext* schemaImportContext, Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    ECDbSqlColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn, targetTable, specifications.GetCollation()))
        return const_cast<ECDbSqlColumn*>(reusableColumn);

    ECDbSqlColumn* newColumn = targetTable.CreateColumn(nullptr, ECDbSqlColumn::Type::Any, specifications.GetColumnKind(), specifications.GetColumnPersistenceType());
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    newColumn->GetConstraintR ().SetCollation(specifications.GetCollation());
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId ColumnFactory::GetPersistenceClassId(Specification const& specifications) const
    {
    ECClassId propertyLocalToClassId;
    const auto n = specifications.GetAccessString().find(".");
    if (n != Utf8String::npos)
        {
        //! Get root property in given accessString.
        auto propertyP = m_classMap.GetClass().GetPropertyP (specifications.GetAccessString().substr(0, n).c_str());
        if (propertyP == nullptr)
            {
            BeAssert(false && "Failed to find root property");
            return 0;
            }

        propertyLocalToClassId = propertyP->GetClass().GetId();
        }
    else
        {
        auto propertyP = m_classMap.GetClass().GetPropertyP (specifications.GetAccessString().c_str());
        if (propertyP == nullptr)
            {
            BeAssert(false && "Failed to find root property");
            return 0;
            }

        propertyLocalToClassId = propertyP->GetClass().GetId();
        }

    return propertyLocalToClassId;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::Configure(SchemaImportContext* schemaImportContext, Specification const& specifications, ECDbSqlTable& targetTable)
    {
    ECClassId persistenceClassId = GetPersistenceClassId(specifications);
    if (persistenceClassId == 0)
        return nullptr;

    ECDbSqlColumn* outColumn = nullptr;
    switch (specifications.GetStrategy())
        {
        case Specification::Strategy::Create:
            outColumn = ApplyCreateStrategy(specifications, targetTable, persistenceClassId); break;
        case Specification::Strategy::CreateOrReuse:
            outColumn = ApplyCreateOrReuseStrategy(specifications, targetTable, persistenceClassId); break;
        case Specification::Strategy::CreateOrReuseSharedColumn:
            outColumn = ApplyCreateOrReuseSharedColumnStrategy(schemaImportContext, specifications, targetTable, persistenceClassId); break;
        }

    Utf8String const& className = m_classMap.GetClass().GetName();
    if (outColumn != nullptr)
        LOG.tracev("Property -> '%s:%s' mapped to '%s'", className.c_str(), specifications.GetAccessString().c_str(), outColumn->GetName().c_str());
    else
        LOG.tracev("Property -> '%s:%s' mapped to 'NULL'", className.c_str(), specifications.GetAccessString().c_str());

    RegisterColumnInUse(*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::Configure(SchemaImportContext* schemaImportContext, Specification const& specifications)
    {
    return Configure(schemaImportContext, specifications, m_classMap.GetTable());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlTable& ColumnFactory::GetTable()  { return m_classMap.GetTable(); }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
IClassMap const& PropertyMapSet::GetClassMap () const { return m_classMap; }

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
const PropertyMapSet::EndPoints PropertyMapSet::FindEndPoints (ColumnKind filter) const
    {
    EndPoints endPoints;
    for (auto const& endPoint : m_orderedEndPoints)
        {
        if (Enum::Contains(filter, endPoint->GetColumnKind()))
            endPoints.push_back(endPoint.get());
        }

    return endPoints;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus PropertyMapSet::AddSystemEndPoint(PropertyMapSet& propertySet, IClassMap const& classMap, ColumnKind kind, ECValueCR value, ECDbSqlColumn const* column)
    {
    auto const& table = classMap.GetTable();

    if (column == nullptr)
        column = table.GetFilteredColumnFirst(kind);

    auto const accessString = ECDbSqlColumn::KindToString(kind);
    if (value.IsNull())
        {
        BeAssert(column != nullptr);
        if (column == nullptr)
            return BentleyStatus::ERROR;
        }

    BeAssert(accessString != nullptr);
    if (accessString == nullptr)
        return BentleyStatus::ERROR;

    if (column == nullptr)
        propertySet.m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint(accessString, kind, value)));
    else
        propertySet.m_orderedEndPoints.push_back(std::unique_ptr<EndPoint>(new EndPoint(accessString, *column, value)));

    return BentleyStatus::SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
PropertyMapSet::Ptr PropertyMapSet::Create (IClassMap const& classMap)
    {
    BeAssert (classMap.GetECDbMap ().GetSQLManager ().IsNullTable (classMap.GetTable()) == false);
    Ptr propertySet = Ptr (new PropertyMapSet (classMap));
    ECValue defaultValue;
    AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECInstanceId, defaultValue);
    AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECClassId, ECValue (classMap.GetClass ().GetId ()));
    if (classMap.GetClass ().GetIsStruct ())
        {
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ParentECInstanceId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECPropertyPathId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECArrayIndex, defaultValue);
        }

    if (classMap.GetClassMapType () == IClassMap::Type::RelationshipLinkTable ||
        classMap.GetClassMapType () == IClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassMapCR relationshipMap = static_cast<RelationshipClassMapCR>(classMap);
        auto const& sourceConstraints = relationshipMap.GetRelationshipClass ().GetSource ().GetClasses ();
        auto const& targetConstraints = relationshipMap.GetRelationshipClass ().GetTarget ().GetClasses ();
        auto sourceECInstanceIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn ();
        auto sourceECClassIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn ();
        auto targetECInstanceIdColumn = relationshipMap.GetTargetECInstanceIdPropMap ()->GetFirstColumn ();
        auto targetECClassIdColumn = relationshipMap.GetTargetECClassIdPropMap ()->GetFirstColumn ();


        AddSystemEndPoint (*propertySet, classMap, ColumnKind::SourceECInstanceId, defaultValue, sourceECInstanceIdColumn);
        auto sourceConstraintClass = sourceConstraints.at (0);
        if (!IClassMap::IsAnyClass (*sourceConstraintClass) && sourceConstraints.size () == 1)
            AddSystemEndPoint (*propertySet, classMap, ColumnKind::SourceECClassId, ECValue (sourceConstraintClass->GetId ()), sourceECClassIdColumn);
        else
            AddSystemEndPoint (*propertySet, classMap, ColumnKind::SourceECClassId, defaultValue, sourceECClassIdColumn);

        AddSystemEndPoint (*propertySet, classMap, ColumnKind::TargetECInstanceId, defaultValue, targetECInstanceIdColumn);
        auto targetConstraintClass = targetConstraints.at (0);
        if (!IClassMap::IsAnyClass (*targetConstraintClass) && targetConstraints.size () == 1)
            AddSystemEndPoint (*propertySet, classMap, ColumnKind::TargetECClassId, ECValue (targetConstraintClass->GetId ()), targetECClassIdColumn);
        else
            AddSystemEndPoint (*propertySet, classMap, ColumnKind::SourceECClassId, defaultValue, targetECClassIdColumn);
        }

    classMap.GetPropertyMaps ().Traverse ([&propertySet] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (auto pm = dynamic_cast<PropertyMapPoint const*> (propMap))
            {
            std::vector<ECDbSqlColumn const*> columns;
            pm->GetColumns (columns);
            Utf8String  baseAccessString = pm->GetPropertyAccessString ();
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".X").c_str (), *columns[0], ECValue ())));
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".Y").c_str (), *columns[1], ECValue ())));
            if (pm->Is3d ())
                propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".Z").c_str (), *columns[2], ECValue ())));
            }
        else if (nullptr != dynamic_cast<PropertyMapToTable const*> (propMap))
            {
            feedback = TraversalFeedback::NextSibling;
            }
        else if (!propMap->IsSystemPropertyMap ())
            {
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint (propMap->GetPropertyAccessString (), *propMap->GetFirstColumn (), ECValue ())));
            }
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto const& ep : propertySet->m_orderedEndPoints)
        {
        propertySet->m_endPointByAccessString[ep->GetAccessString ().c_str ()] = ep.get ();
        }
    return propertySet;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
