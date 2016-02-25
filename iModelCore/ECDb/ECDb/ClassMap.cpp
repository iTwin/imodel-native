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
// @bsimethod                                 Affan.Khan                    12/2015
//---------------------------------------------------------------------------------------
const Utf8String IClassMap::GetPersistedViewName() const
    {
    return Utf8String("_" + GetClass().GetSchema().GetNamespacePrefix() + "_" + GetClass().GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    12/2015
//---------------------------------------------------------------------------------------
bool IClassMap::HasPersistedView() const
    {
    return GetECDbMap().GetECDb().TableExists(GetPersistedViewName().c_str());
    }

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
        if (propMap->GetAsPropertyMapStructArray() != nullptr)
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
bool IClassMap::MapsToStructArrayTable() const
    {
    return MapsToStructArrayTable(GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::MapsToStructArrayTable(ECN::ECClassCR ecClass)
    {
    return ecClass.IsStructClass();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
bool IClassMap::HasJoinedTable() const
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
IClassMap const* IClassMap::GetParentOfJoinedTable() const
    {
    std::vector<IClassMap const*> path;
    if (GetPathToParentOfJoinedTable(path) != SUCCESS)
        return nullptr;

    return path.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus IClassMap::GetPathToParentOfJoinedTable(std::vector<IClassMap const*>& path) const
    {
    path.clear();
    IClassMap const* current = this;
    if (!current->HasJoinedTable() && !current->IsParentOfJoinedTable())
        return ERROR;

    path.insert(path.begin(), current);
    do
        {
        ECClassId nextParentId = current->GetParentMapClassId();
        if (nextParentId == ECClass::UNSET_ECCLASSID)
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
        }
    while (current != nullptr);

    path.clear();
    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  10/2015
//---------------------------------------------------------------------------------------
IClassMap const* IClassMap::FindClassMapOfParentOfJoinedTable() const
    {
    IClassMap const* current = this;
    if (!current->HasJoinedTable())
        return nullptr;

    do
        {
        if (current->IsParentOfJoinedTable())
            return current;

        auto nextParentId = current->GetParentMapClassId();
        if (nextParentId == ECClass::UNSET_ECCLASSID)
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
IClassMap const* IClassMap::FindSharedTableRootClassMap() const
    {
    ECDbMapStrategy mapStrategy = GetMapStrategy();
    if (mapStrategy.GetStrategy() != ECDbMapStrategy::Strategy::SharedTable && !mapStrategy.AppliesToSubclasses())
        return nullptr;

    ECClassId parentId = GetParentMapClassId();
    if (parentId == ECClass::UNSET_ECCLASSID)
        return this;

    ClassMap const* parent = GetECDbMap().GetClassMap(parentId);
    if (parent == nullptr)
        {
        BeAssert(false && "Failed to find parent classmap. This should not happen");
        return nullptr;
        }

    return parent->FindSharedTableRootClassMap();
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
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus IClassMap::DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR ecclass)
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


//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
    : IClassMap(), m_ecDbMap(ecDbMap), m_ecClass(ecClass), m_mapStrategy(mapStrategy),
    m_parentMapClassId(ECClass::UNSET_ECCLASSID), m_dbView(nullptr), m_isDirty(setIsDirty), m_columnFactory(*this), m_id(0ULL)
    {
    if (SUCCESS != InitializeDisableECInstanceIdAutogeneration())
        {
        BeAssert(false && "InitializeDisableECInstanceIdAutogeneration failed");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::Map(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo)
    {
    ECDbMapStrategy const& mapStrategy = GetMapStrategy();
    IClassMap const* effectiveParentClassMap = (mapStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && mapStrategy.AppliesToSubclasses()) ? mapInfo.GetParentClassMap() : nullptr;

    auto stat = _MapPart1(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;
    
    stat = _MapPart2(schemaImportContext, mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    return _OnInitialized();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_MapPart1(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView(*this));
    ECDbSqlTable const* baseTable = nullptr;
    TableType tableType = TableType::Primary;
    if (Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable))
        {
        tableType = TableType::Joined;
        baseTable = &parentClassMap->GetPrimaryTable();
        }
    else if (IClassMap::MapsToStructArrayTable(m_ecClass))
        tableType = TableType::StructArray;
    else if (mapInfo.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable)
        tableType = TableType::Existing;

    auto findOrCreateTable = [&] ()
        {
        if (TableType::Joined == tableType)
            {
            BeAssert(baseTable != nullptr);
            }

        ECDbSqlTable* table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable(&schemaImportContext, mapInfo.GetTableName(), tableType,
            mapInfo.IsMapToVirtualTable(), mapInfo.GetECInstanceIdColumnName(), baseTable);

        if (!EXPECTED_CONDITION(table != nullptr))
            return MapStatus::Error;

        SetTable(*table, true /*append*/);
        return MapStatus::Success;
        };

    BeAssert(tableType != TableType::Joined || parentClassMap != nullptr);
    if (parentClassMap != nullptr)
        {
        PRECONDITION(!parentClassMap->GetMapStrategy().IsNotMapped(), MapStatus::Error);
        m_parentMapClassId = parentClassMap->GetClass().GetId();
        SetTable(parentClassMap->GetPrimaryTable());

        if (tableType == TableType::Joined)
            {
            if (findOrCreateTable() == MapStatus::Error)
                return MapStatus::Error;
            }
        }
    else
        {
        if (findOrCreateTable() == MapStatus::Error)
            return MapStatus::Error;
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
MapStatus ClassMap::_MapPart2(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    MapStatus stat = AddPropertyMaps(schemaImportContext.GetClassMapLoadContext(), parentClassMap, nullptr, &mapInfo);
    if (stat != MapStatus::Success)
        return stat;

    ECPropertyCP currentTimeStampProp = mapInfo.GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return MapStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = Enum::Contains(mapInfo.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    if (isJoinedTable)
        {
        PRECONDITION(parentClassMap != nullptr, MapStatus::Error);
        if (&parentClassMap->GetJoinedTable() != &GetJoinedTable())
            {
            auto primaryKeyColumn = parentClassMap->GetJoinedTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
            auto foreignKeyColumn = GetJoinedTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
            PRECONDITION(primaryKeyColumn != nullptr, MapStatus::Error);
            PRECONDITION(foreignKeyColumn != nullptr, MapStatus::Error);
            bool createFKConstraint = true;
            for (auto constraint : GetJoinedTable().GetConstraints())
                {
                if (constraint->GetType() == ECDbSqlConstraint::Type::ForeignKey)
                    {
                    auto fk = static_cast<ECDbSqlForeignKeyConstraint const*>(constraint);
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
                auto fkConstraint = GetJoinedTable().CreateForeignKeyConstraint(parentClassMap->GetJoinedTable());
                fkConstraint->Add(foreignKeyColumn->GetName().c_str(), primaryKeyColumn->GetName().c_str());
                fkConstraint->SetOnDeleteAction(ForeignKeyActionType::Cascade);
                }
            }
        }

    return MapStatus::Success;
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

    ECDbSqlColumn* currentTimeStampColumn = const_cast<ECDbSqlColumn*>(propertyMap->GetSingleColumn());
    if (currentTimeStampColumn == nullptr)
        {
        BeAssert(currentTimeStampColumn != nullptr && "TimeStamp column cannot be null");
        return ERROR;
        }

    if (currentTimeStampColumn->GetType() == ECDbSqlColumn::Type::Any)
        {
        m_ecDbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                   "ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
                   "ECDb therefore does not create the current timestamp trigger for this property.",
                           currentTimeStampProp.GetName().c_str(), currentTimeStampProp.GetClass().GetFullName());

        return SUCCESS;
        }

    BeAssert(currentTimeStampColumn->GetType() == ECDbSqlColumn::Type::TimeStamp);
    currentTimeStampColumn->GetConstraintR().SetDefaultExpression(CURRENTIMESTAMP_SQLEXP);
    currentTimeStampColumn->GetConstraintR().SetIsNotNull(true);

    PropertyMapCP idPropMap = GetECInstanceIdPropertyMap();
    if (idPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ECDbSqlTable& table = currentTimeStampColumn->GetTableR();
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

    return table.CreateTrigger(triggerName.c_str(), whenCondition.c_str(), body.c_str(), TriggerType::Create, TriggerSubType::After);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan   02/2016
//---------------------------------------------------------------------------------------
void ClassMap::SetTable(ECDbSqlTable& newTable, bool append /*= false*/)
    {
    if (!append)
        m_tables.clear();

    m_tables.push_back(&newTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::AddPropertyMaps(ClassMapLoadContext& ctx, IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo,ClassMapInfo const* classMapInfo)
    {
    const bool isJoinedTableMapping = Enum::Contains(GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::JoinedTable);
    const bool isImportingSchemas = classMapInfo != nullptr && loadInfo == nullptr;
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
            GetPropertyMapsR().AddPropertyMap(PropertyMap::Clone(m_ecDbMap, *propMapInBaseClass, GetClass(), nullptr));
        else
            propertiesToCreatePropMapsFor.push_back(property);
        }

    if (isImportingSchemas)
        GetColumnFactoryR().Update();

    for (ECPropertyCP property : propertiesToCreatePropMapsFor)
        {
        Utf8CP propertyAccessString = property->GetName().c_str();
        PropertyMapPtr propMap = PropertyMap::CreateAndEvaluateMapping(ctx, m_ecDbMap.GetECDb(), *property, m_ecClass, propertyAccessString, nullptr);
        if (propMap == nullptr)
            return MapStatus::Error;

        if (GetPropertyMap(propertyAccessString) != nullptr)
            {
            BeAssert(GetPropertyMap(propertyAccessString) == nullptr && " it should not be there");
            return MapStatus::Error;
            }

        if (isImportingSchemas)
            {
            if (SUCCESS != propMap->FindOrCreateColumnsInTable(*this))
                {
                BeAssert(false);
                return MapStatus::Error;
                }

            GetPropertyMapsR().AddPropertyMap(propMap);
            }
        else
            {
            if (SUCCESS != propMap->Load(*loadInfo))
                {
                BeAssert(false);
                return MapStatus::Error;
                }

            GetPropertyMapsR().AddPropertyMap(propMap);
            }
        }

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::CreateUserProvidedIndexes(SchemaImportContext& schemaImportContext, bvector<ClassIndexInfoPtr> const& indexInfoList) const
    {
    int i = 0;
    IssueReporter const& issues = m_ecDbMap.GetECDbR().GetECDbImplR().GetIssueReporter();
    for (ClassIndexInfoPtr indexInfo : indexInfoList)
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

            std::vector<ECDbSqlColumn const*> columns;
            propertyMap->GetColumns(columns);
            if (0 == columns.size())
                {
                BeAssert(false && "Reject user defined index on %s. Fail to find column property map for property. Something wrong with mapping");
                return ERROR;
                }

            for (ECDbSqlColumn const* column : columns)
                {
                if (column->GetTable().GetPersistenceType() == PersistenceType::Persisted && column->GetPersistenceType() == PersistenceType::Virtual)
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

        ECDbSqlTable* involvedTable =  const_cast<ECDbSqlTable*>(*involvedTables.begin());
        if (nullptr == schemaImportContext.GetECDbMapDb().CreateIndex(m_ecDbMap.GetECDbR(), *involvedTable, indexInfo->GetName(), indexInfo->GetIsUnique(),
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

        
        auto mapInfo = m_ecDbMap.GetSQLManager().GetMapStorage().CreateClassMap(GetClass().GetId(), m_mapStrategy, baseClassMap == nullptr ? ECClass::UNSET_ECCLASSID : baseClassMap->GetId());
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
BentleyStatus ClassMap::_Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView>(new ClassDbView(*this));
    if (parentClassMap)
        m_parentMapClassId = parentClassMap->GetClass().GetId();

    auto& allPropertyInfos = mapInfo.GetPropertyMaps(false);

    std::set<Utf8CP, CompareUtf8> localPropSet;
    for (auto property : GetClass().GetProperties(false))
        {
        localPropSet.insert(property->GetName().c_str());
        }

    localPropSet.insert(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME);
    localPropSet.insert(ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME);
    localPropSet.insert(ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME);
    localPropSet.insert(ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME);
    localPropSet.insert(ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME);
    std::set<ECDbSqlTable*> tables;
    std::set<ECDbSqlTable*> joinedTables;

    if (allPropertyInfos.empty())
        {
        SetTable(*const_cast<ECDbSqlTable*>(GetECDbMap().GetSQLManager().GetNullTable()));
        }
    else
        {

        for (auto propertyInfo : allPropertyInfos)
            {
            if (propertyInfo->GetColumns().front()->GetKind() == ColumnKind::ECClassId)
                continue;
           
            for (auto column : propertyInfo->GetColumns())
                if (column->GetTable().GetTableType() == TableType::Joined)
                    joinedTables.insert(&column->GetTableR());
                else
                    tables.insert(&column->GetTableR());
            }

        for (auto table : tables)
            SetTable(*table, true);

        for (auto table : joinedTables)
            SetTable(*table, true);

        BeAssert(!GetTables().empty());
        }

    if (GetECInstanceIdPropertyMap() != nullptr)
        return BentleyStatus::ERROR;
    
    if (auto propInfo = mapInfo.FindPropertyMapByAccessString(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME))
        {
        PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create(Schemas(), *this, propInfo->GetColumns());
        if (ecInstanceIdPropertyMap == nullptr)
            return BentleyStatus::ERROR;

        GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropertyMap);
        }
    else
        {
        BeAssert(false && "Failed to deserialize ECInstanceId");
        return ERROR;
        }
    return AddPropertyMaps(ctx, parentClassMap, &mapInfo, nullptr) == MapStatus::Success ? SUCCESS : ERROR;
    }

//=========================================================================================
//ColumnFactory
//=========================================================================================

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::CreateColumn(PropertyMapCR propMap, Utf8CP requestedColumnName, ECDbSqlColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, ECDbSqlColumn::Constraint::Collation collation) const
    {
    ECDbSqlColumn* outColumn = nullptr;
    if (Enum::Contains(m_classMap.GetMapStrategy().GetOptions(), ECDbMapStrategy::Options::SharedColumns))
        {
        BeAssert(m_classMap.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable);
        // Shared column does not support NOT NULL constraint -> omit NOT NULL and issue warning
        if (addNotNullConstraint || addUniqueConstraint || collation != ECDbSqlColumn::Constraint::Collation::Default)
            {
            m_classMap.GetECDbMap().GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, "For the ECProperty '%s' on ECClass '%s' either a 'not null', unique or collation constraint is defined. It is mapped "
                                                          "to a column though shared with other ECProperties. Therefore ECDb cannot enforce any of these constraints. "
                                                          "The column is created without constraints.",
                                                          propMap.GetProperty().GetName().c_str(), propMap.GetProperty().GetClass().GetFullName());

            }

        outColumn = ApplySharedColumnStrategy();
        }
    else
        {
        BeAssert(!Utf8String::IsNullOrEmpty(requestedColumnName) && "requestedColumnName must not be null for Strategy::CreateOrReuse");
        outColumn = ApplyDefaultStrategy(requestedColumnName, propMap, colType, addNotNullConstraint, addUniqueConstraint, collation);
        }

    if (outColumn == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    RegisterColumnInUse(*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyDefaultStrategy(Utf8CP requestedColumnName, PropertyMapCR propMap, ECDbSqlColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, ECDbSqlColumn::Constraint::Collation collation) const
    {
    BeAssert(!Utf8String::IsNullOrEmpty(requestedColumnName) && "Column name must not be null for CreateOrReuseStrategy");

    ECDbSqlColumn* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && !IsColumnInUse(*existingColumn) &&
        ECDbSqlColumn::IsCompatible(existingColumn->GetType(), colType))
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
    if (classId == ECClass::UNSET_ECCLASSID)
        return nullptr;

    //column already exists but doesn't match, therefore create a new one
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

    const bool canEdit = GetTable().GetEditHandle().CanEdit();
    if (!canEdit)
        GetTable().GetEditHandleR().BeginEdit();

    ECDbSqlColumn* newColumn = GetTable().CreateColumn(resolvedColumnName.c_str(), colType, ColumnKind::DataColumn, PersistenceType::Persisted);
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    newColumn->GetConstraintR().SetIsNotNull(addNotNullConstraint);
    newColumn->GetConstraintR().SetIsUnique(addUniqueConstraint);
    newColumn->GetConstraintR().SetCollation(collation);

    if (!canEdit)
        GetTable().GetEditHandleR().EndEdit();

    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplySharedColumnStrategy() const
    {
    ECDbSqlColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<ECDbSqlColumn*>(reusableColumn);

    return GetTable().CreateColumn(nullptr, ECDbSqlColumn::Type::Any, ColumnKind::DataColumn, PersistenceType::Persisted);
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

    ECDbSqlColumn const* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && IsColumnInUse(*existingColumn))
        resolvedColumName.Sprintf("c%lld_%s", classId, requestedColumnName);
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
        return ECClass::UNSET_ECCLASSID;
        }

    return property->GetClass().GetId();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::TryFindReusableSharedDataColumn(ECDbSqlColumn const*& reusableColumn) const
    {
    reusableColumn = nullptr;
    std::vector<ECDbSqlColumn const*> reusableColumns;
    for (auto column : GetTable().GetColumns())
        {
        if (Enum::Contains(column->GetKind(), ColumnKind::DataColumn) && column->GetType() == ECDbSqlColumn::Type::Any)
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
void ColumnFactory::RegisterColumnInUse(ECDbSqlColumn const& column) const
    {
    m_columnsInUseSet.insert(column.GetFullName());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse(Utf8CP columnFullName) const
    {
    return m_columnsInUseSet.find(columnFullName) != m_columnsInUseSet.end();
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
    m_columnsInUseSet.clear();
    std::vector<ECDbSqlColumn const*> columnsInUse;
    m_classMap.GetPropertyMaps().Traverse(
        [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (propMap->GetAsNavigationPropertyMap() == nullptr)
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
ECDbSqlTable& ColumnFactory::GetTable() const
    {
    return m_classMap.GetJoinedTable();
    }


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
    auto const& table = classMap.GetJoinedTable();

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
    BeAssert (classMap.GetECDbMap ().GetSQLManager ().IsNullTable (classMap.GetJoinedTable()) == false);
    Ptr propertySet = Ptr (new PropertyMapSet (classMap));
    ECValue defaultValue;
    AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECInstanceId, defaultValue);
    AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECClassId, ECValue (classMap.GetClass ().GetId ()));
    if (classMap.GetClass ().IsStructClass())
        {
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ParentECInstanceId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECPropertyPathId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ColumnKind::ECArrayIndex, defaultValue);
        }

    if (classMap.IsRelationshipClassMap ())
        {
        RelationshipClassMapCR relationshipMap = static_cast<RelationshipClassMapCR>(classMap);
        auto const& sourceConstraints = relationshipMap.GetRelationshipClass ().GetSource ().GetClasses ();
        auto const& targetConstraints = relationshipMap.GetRelationshipClass ().GetTarget ().GetClasses ();
        auto sourceECInstanceIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetSingleColumn ();
        auto sourceECClassIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetSingleColumn ();
        auto targetECInstanceIdColumn = relationshipMap.GetTargetECInstanceIdPropMap ()->GetSingleColumn ();
        auto targetECClassIdColumn = relationshipMap.GetTargetECClassIdPropMap ()->GetSingleColumn ();


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
        else if (nullptr != propMap->GetAsPropertyMapStructArray() || nullptr != propMap->GetAsNavigationPropertyMap())
            {
            feedback = TraversalFeedback::NextSibling;
            }
        else if (!propMap->IsSystemPropertyMap () && !propMap->GetProperty().GetAsStructProperty())
            {
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint (propMap->GetPropertyAccessString (), *propMap->GetSingleColumn (), ECValue ())));
            }
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto const& ep : propertySet->m_orderedEndPoints)
        {
        propertySet->m_endPointByAccessString[ep->GetAccessString ().c_str ()] = ep.get ();
        }
    return propertySet;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    01/2016
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapLoadContext::Postprocess(ECDbMapCR ecdbMap) const
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

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
