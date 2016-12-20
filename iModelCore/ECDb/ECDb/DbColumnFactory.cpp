/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbColumnFactory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       12 / 2016
//------------------------------------------------------------------------------------------
DbColumnFactory::DbColumnFactory(ClassMap const& classMap) 
    : m_classMap(classMap), m_usesSharedColumnStrategy(classMap.GetMapStrategy().GetTphInfo().IsValid() && classMap.GetMapStrategy().GetTphInfo().GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes)
    {
    Initialize();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       12 / 2016
//------------------------------------------------------------------------------------------
void DbColumnFactory::Initialize()
    {
    const std::set<ClassMap const*> deepestClassMapsInTph = GetDeepestClassMapsInTph(m_classMap);
    //!The function return deepest mapped classes from memory or disk and does not include class for which its called.
    for (ClassMap const* classMap : deepestClassMapsInTph)
        {
        if (classMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            RelationshipClassEndTableMap const* relationshipEndTableMap = static_cast<RelationshipClassEndTableMap const*>(classMap);
            SystemPropertyMap::PerTablePrimitivePropertyMap const* ecInstanceIdPropMap = relationshipEndTableMap->GetForeignEndECInstanceIdPropMap()->FindDataPropertyMap(m_classMap.GetJoinedTable());
            BeAssert(ecInstanceIdPropMap != nullptr);
            Utf8String columnMapKey;
            columnMapKey.Sprintf("%s.%s", classMap->GetClass().GetFullName(), ecInstanceIdPropMap->GetAccessString().c_str());
            AddColumnToCache(ecInstanceIdPropMap->GetColumn(), columnMapKey);

            SystemPropertyMap::PerTablePrimitivePropertyMap const* ecClassIdPropMap = relationshipEndTableMap->GetForeignEndECInstanceIdPropMap()->FindDataPropertyMap(m_classMap.GetJoinedTable());   
            BeAssert(ecClassIdPropMap != nullptr);
            columnMapKey.Sprintf("%s.%s", classMap->GetClass().GetFullName(), ecClassIdPropMap->GetAccessString().c_str());
            AddColumnToCache(ecClassIdPropMap->GetColumn(), columnMapKey);
            }
        else
            {
            SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
            classMap->GetPropertyMaps().AcceptVisitor(visitor);
            for (PropertyMap const* propertyMap : visitor.Results())
                {
                if (!propertyMap->IsMappedToTable(m_classMap.GetJoinedTable()))
                    continue;

                SingleColumnDataPropertyMap const* dataPropertyMap = static_cast<SingleColumnDataPropertyMap const*>(propertyMap);
                auto columnItor = m_usedColumnMap.find(propertyMap->GetAccessString());
                if (columnItor != m_usedColumnMap.end())
                    {
                    if (columnItor->second.find(&dataPropertyMap->GetColumn()) != columnItor->second.end())
                        continue;
                    }

                AddColumnToCache(dataPropertyMap->GetColumn(), dataPropertyMap->GetAccessString());
                }
            }
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::CreateColumn(ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        outColumn = ApplySharedColumnStrategy(ecProp, colType, params);
    else
        outColumn = ApplyDefaultStrategy(ecProp, colType, params, accessString);

    if (outColumn == nullptr)
        return nullptr;

    AddColumnToCache(*outColumn, accessString);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString) const
    {
    auto itor = m_usedColumnMap.find(accessString);
    if (itor == m_usedColumnMap.end())
        return CreateColumn(property, type, param, accessString);

    //Find a column that is suitable
    const std::set<DbColumn const*>& mappedColumns = itor->second;
    for (DbColumn const* mappedColumn : mappedColumns)
        {
        //set allocate column to mapped column if it fits
        if (IsCompatible(*mappedColumn, type, param))
            return const_cast<DbColumn*>(mappedColumn);
        }

    return CreateColumn(property, type, param, accessString);
    }



//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplyDefaultStrategy(ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    DbColumn* existingColumn = GetTable().FindColumnP(params.GetColumnName().c_str());
    if (existingColumn != nullptr && !IsColumnInUseByClassMap(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (!GetTable().IsOwnedByECDb() || (existingColumn->GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                                            existingColumn->GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                                            existingColumn->GetConstraints().GetCollation() == params.GetCollation()))
            {
            return existingColumn;
            }

        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Column %s in table %s is used by multiple property maps where property name and data type matches,"
                                                        " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                                                        existingColumn->GetName().c_str(), GetTable().GetName().c_str());
        return nullptr;
        }


    BeAssert(!params.GetColumnName().empty() && "Column name must not be null for default strategy");
    bool effectiveNotNullConstraint = params.AddNotNullConstraint();
    if (params.AddNotNullConstraint() && (GetTable().HasExclusiveRootECClass() && GetTable().GetExclusiveRootECClassId() != m_classMap.GetClass().GetId()))
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                        "For the ECProperty '%s' on ECClass '%s' a NOT NULL constraint is defined. The constraint cannot be enforced though because "
                                                        "the ECProperty has base ECClasses mapped to the same table.",
                                                        ecProp.GetName().c_str(), ecProp.GetClass().GetFullName());

        effectiveNotNullConstraint = false;
        }

    const ECClassId classId = GetPersistenceClassId(ecProp, accessString);
    if (!classId.IsValid())
        return nullptr;

    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (SUCCESS != ResolveColumnName(tmp, params.GetColumnName(), classId, retryCount))
        return nullptr;

    resolvedColumnName = tmp;
    while (GetTable().FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (SUCCESS != ResolveColumnName(resolvedColumnName, params.GetColumnName(), classId, retryCount))
            return nullptr;
        }

    DbColumn* newColumn = GetTable().CreateColumn(resolvedColumnName, colType, DbColumn::Kind::DataColumn, PersistenceType::Persisted);
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    if (effectiveNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (params.AddUniqueConstraint())
        newColumn->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        newColumn->GetConstraintsR().SetCollation(params.GetCollation());

    AddColumnToCache(*newColumn, accessString);
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplySharedColumnStrategy(ECN::ECPropertyCR prop, DbColumn::Type colType, DbColumn::CreateParams const& params) const
    {
    //Defining a col name for a shared column is a DB thing and DB CAs are taken strictly.
    if (params.IsColumnNameFromPropertyMapCA())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a value for 'ColumnName'. "
                                                        "'ColumnName' must not be specified for this ECProperty because it is mapped to a column shared with other ECProperties.",
                                                        prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //Defining a collation which is not doable is an error because this is a DB thing and DB CAs are taken strictly.
    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a Collation constraint "
                                                        "which cannot be created because the ECProperty is mapped to a column shared with other ECProperties.",
                                                        prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //NOT NULL and UNIQUE will soon become ECSchema level things. They are not an error, and can only be taken as hints because
    //the ECSchema level doesn't say which layer (DB or API) has to enforce it
    bool addNotNullConstraint = params.AddNotNullConstraint();
    bool addUniqueConstraint = params.AddUniqueConstraint();
    if (params.AddNotNullConstraint() || params.AddUniqueConstraint())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                        "For the ECProperty '%s' on ECClass '%s' either a NOT NULL or a UNIQUE constraint is defined. The constraint cannot be enforced though because "
                                                        "the ECProperty is mapped to a column shared with other ECProperties.",
                                                        prop.GetName().c_str(), prop.GetClass().GetFullName());

        addNotNullConstraint = false;
        addUniqueConstraint = false;
        }

    DbColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<DbColumn*>(reusableColumn);

    DbColumn* col = GetTable().CreateOverflowSlaveColumn(colType);
    if (col == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    if (addNotNullConstraint)
        col->GetConstraintsR().SetNotNullConstraint();

    if (addUniqueConstraint)
        col->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() == DbColumn::Constraints::Collation::Unset)
        col->GetConstraintsR().SetCollation(params.GetCollation());

    return col;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus DbColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId classId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert(!resolvedColumName.empty());
        resolvedColumName += SqlPrintfString("%d", retryCount);
        return SUCCESS;
        }

    if (requestedColumnName.empty())
        {
        //use name generator
        resolvedColumName.clear();
        return SUCCESS;
        }

    DbColumn const* existingColumn = GetTable().FindColumnP(requestedColumnName.c_str());
    if (existingColumn != nullptr && IsColumnInUseByClassMap(*existingColumn))
        {
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName.c_str());
        }
    else
        resolvedColumName.assign(requestedColumnName);

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId DbColumnFactory::GetPersistenceClassId(ECN::ECPropertyCR ecProp, Utf8StringCR propAccessString) const
    {
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
bool DbColumnFactory::TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const
    {
    reusableColumn = nullptr;
    for (DbColumn const* column : GetTable().GetColumns())
        {
        if (column->IsShared() && !IsColumnInUseByClassMap(*column))
            {
            reusableColumn = column;
            return true;
            }
        }

    return false;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void DbColumnFactory::AddColumnToCache(DbColumn const& column, Utf8StringCR accessString) const
    {
    m_usedColumnMap[accessString].insert(&column);
    m_usedColumnSet.insert(&column);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool DbColumnFactory::IsCompatible(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& params) const
    {
    if (DbColumn::IsCompatible(avaliableColumn.GetType(), type))
        {
        if (!GetTable().IsOwnedByECDb() || (avaliableColumn.GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                                            avaliableColumn.GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                                            avaliableColumn.GetConstraints().GetCollation() == params.GetCollation()))
            {
            return true;
            }
        }

    return false;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
//static 
std::set<ClassMap const*> DbColumnFactory::GetDeepestClassMapsInTph(ClassMap const& classMap)
    {
    if (!classMap.GetMapStrategy().GetTphInfo().IsValid())
        return std::set<ClassMap const*>();

    DbTable const& contextTable = classMap.GetJoinedTable();
    ECDbMap const& dbMap = classMap.GetDbMap();

    std::set<ECClassCP> doneList;
    std::set<ClassMap const*> deepestMappedClassSet;
    std::set<RelationshipClassEndTableMap const*> relationshipMapSet;
    std::function<void(ECClassCP)> findDeepestMappedClass = [&] (ECClassCP contextClass)
        {
        if (doneList.find(contextClass) != doneList.end())
            return;

        doneList.insert(contextClass);
        ClassMap const* contextClassMap = dbMap.GetClassMap(*contextClass);
        if (contextClassMap == nullptr)
            return;

        if (contextClassMap->GetJoinedTable().GetId() == contextTable.GetId())
            deepestMappedClassSet.insert(contextClassMap);

        const size_t n = deepestMappedClassSet.size();
        for (ECClassCP derivedClass : dbMap.GetECDb().Schemas().GetDerivedECClasses(contextClassMap->GetClass()))
            findDeepestMappedClass(derivedClass);

        //Figure out Relationship
        for (bpair<ECClassId, LightweightCache::RelationshipEnd> const& key : dbMap.GetLightweightCache().GetRelationshipClasssForConstraintClass(contextClassMap->GetClass().GetId()))
            {
            ECClassId relationshipClassId = key.first;
            ECRelationshipClassCP relationshipClass = static_cast<ECRelationshipClassCP>(dbMap.GetECDb().Schemas().GetECClass(relationshipClassId));
            if (relationshipClass == nullptr)
                continue;

            if (doneList.find(relationshipClass) != doneList.end())
                continue;

            doneList.insert(relationshipClass);
            ClassMap const* contextRelationshipMap = dbMap.GetClassMap(*relationshipClass);
            if (contextRelationshipMap == nullptr)
                continue;

            if (contextRelationshipMap->GetType() != ClassMap::Type::RelationshipEndTable)
                continue;

            RelationshipClassEndTableMap const* contextRelationshipEndTableMap = static_cast<RelationshipClassEndTableMap const*>(contextRelationshipMap);
            if (contextRelationshipEndTableMap->GetForeignEndECInstanceIdPropMap()->IsMappedToTable(contextTable))
                {
                relationshipMapSet.insert(contextRelationshipEndTableMap);
                }
            }
        //
        //We are only interested in deepest mapped leave nodes and not all classes
        //We will remove the current class if we find that it has valid leaf nodes
        auto contextItemItor = deepestMappedClassSet.find(contextClassMap);
        if (contextItemItor != deepestMappedClassSet.end() && deepestMappedClassSet.size() > n)
            {
            deepestMappedClassSet.erase(contextItemItor);
            }
        };

    for (ECClassCP rootClass : GetRootClasses(classMap.GetClass()))
        findDeepestMappedClass(rootClass);

    deepestMappedClassSet.insert(relationshipMapSet.begin(), relationshipMapSet.end());
    return deepestMappedClassSet;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
//static 
std::set<ECN::ECClassCP> DbColumnFactory::GetRootClasses(ECN::ECClassCR ecClass)
    {
    std::set<ECN::ECClassCP> rootClasses;
    std::function<void(ECClassCP)> traverse = [&] (ECClassCP contextClass)
        {
        if (!contextClass->HasBaseClasses())
            rootClasses.insert(contextClass);
        else
            for (ECClassCP baseClass : contextClass->GetBaseClasses())
                traverse(baseClass);
        };

    traverse(&ecClass);
    return rootClasses;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbTable& DbColumnFactory::GetTable() const  { return m_classMap.GetJoinedTable();  }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbCR DbColumnFactory::GetECDb() const { return GetClassMap().GetDbMap().GetECDb(); }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void DbColumnFactory::Debug() const
    {
    NativeSqlBuilder sql;
    sql.Append("ClassMap : ").AppendLine(GetClassMap().GetClass().GetFullName());

    for (auto const& pair : m_usedColumnMap)
        {
        for (DbColumn const* column : pair.second)
            sql.Append(pair.first.c_str()).Append(" -> ").Append(column->GetTable().GetName().c_str()).AppendDot().Append(column->GetName().c_str()).AppendLine("");
        }
    sql.AppendLine("------------------------------------------------");

    printf("%s\n", sql.ToString());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE