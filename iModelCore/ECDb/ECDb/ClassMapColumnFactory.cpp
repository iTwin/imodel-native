/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapColumnFactory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       12 / 2016
//------------------------------------------------------------------------------------------
ClassMapColumnFactory::ClassMapColumnFactory(ClassMap const& classMap) 
    : m_classMap(classMap), m_usesSharedColumnStrategy(classMap.GetMapStrategy().GetTphInfo().IsValid() && classMap.GetMapStrategy().GetTphInfo().GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes)
    {
    Initialize();
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::Initialize()
    {
    bmap<ECN::ECClassCP, ClassMap const*> relevantClassMaps;
    if (ComputeRelevantClassMaps(relevantClassMaps) != SUCCESS)
        return;

    for (bpair<ECClassCP, ClassMap const*> const& kp : relevantClassMaps)
        {
        ECClassCP interfaceClass = kp.first;
        const ClassMap* implClassMap = kp.second;
        std::set<Utf8CP, CompareIUtf8Ascii> relevantProperties;
        const bool isMixIn = interfaceClass->GetId() != implClassMap->GetClass().GetId() && !interfaceClass->IsRelationshipClass();
        if (isMixIn)
            {
            for (ECPropertyCP property : interfaceClass->GetProperties(true))
                relevantProperties.insert(property->GetName().c_str());
            }

		if (interfaceClass->IsRelationshipClass())
			{
			RelationshipClassEndTableMap const* relClassEndTableMap = static_cast<RelationshipClassEndTableMap const*>(implClassMap);
			RelationshipConstraintMap const& persistedEnd = relClassEndTableMap->GetConstraintMap(relClassEndTableMap->GetForeignEnd());
			if (auto const* ecInstanceId = persistedEnd.GetECInstanceIdPropMap()->FindDataPropertyMap(m_classMap.GetJoinedTable()))
				{
				AddColumnToCache(ecInstanceId->GetColumn(), relClassEndTableMap->BuildQualifiedAccessString(ecInstanceId->GetAccessString()));
				}

			if (auto const* relECClassId = relClassEndTableMap->GetECClassIdPropertyMap()->FindDataPropertyMap(m_classMap.GetJoinedTable()))
				{
				AddColumnToCache(relECClassId->GetColumn(), relClassEndTableMap->BuildQualifiedAccessString(relECClassId->GetAccessString()));
				}
			}
		else 
			{
			SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
			implClassMap->GetPropertyMaps().AcceptVisitor(visitor);
			for (PropertyMap const* propertyMap : visitor.Results())
				{
				if (!propertyMap->IsMappedToTable(m_classMap.GetJoinedTable()))
					continue;

				if (isMixIn)
					{
					PropertyMap const* cur = propertyMap;
					while (cur->GetParent())
						cur = cur->GetParent();

					//ignore other properties in case of mixin
					if (relevantProperties.find(cur->GetName().c_str()) == relevantProperties.end())
						continue;
					}

				SingleColumnDataPropertyMap const* singleColDataPropertyMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
				auto columnItor = m_usedColumnMap.find(singleColDataPropertyMap->GetAccessString());
				if (columnItor != m_usedColumnMap.end())
					{
					if (columnItor->second.find(&singleColDataPropertyMap->GetColumn()) != columnItor->second.end())
						continue;
					}

				AddColumnToCache(singleColDataPropertyMap->GetColumn(), singleColDataPropertyMap->GetAccessString());
				}
			}
        }
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::CreateColumn(ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        outColumn = ApplySharedColumnStrategy(ecProp, colType, params);
    else
        outColumn = ApplyDefaultStrategy(ecProp, colType, params, accessString);

    if (outColumn == nullptr)
        return nullptr;

    //AddColumnToCache(*outColumn, accessString);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::SetupCompoundFilter(bset<const ClassMap*> const* additionalFilter) const
	{
	if (m_compoundFilter.size() != 1 || additionalFilter != nullptr)
		{
		m_compoundFilter.clear();
		m_compoundFilter.push_back(&GetClassMap());
		}

	if (additionalFilter)
		for (const ClassMap* additionalClassMap : *additionalFilter)
			{
			if (additionalClassMap == &GetClassMap())
				continue;

			if (additionalClassMap->GetJoinedTable().GetId() != GetTable().GetId())
				{
				BeAssert(false);
				continue;
				}

			m_compoundFilter.push_back(additionalClassMap);
			}
	}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2017
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::RemoveCompoundFilter () const
	{
	if (m_compoundFilter.size() != 1)
		m_compoundFilter.clear();
	}
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2017
//------------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bset<const ClassMap*> const* additionalFilter) const
	{
	SetupCompoundFilter(additionalFilter);
	bool foundColumn = false;
	DbColumn* outColumn = nullptr;
	//First try to find exisitng map
	std::map<Utf8String, std::set<DbColumn const*>, CompareIUtf8Ascii>::iterator itor;
	for (const ClassMap* classMapFilter : m_compoundFilter)
		{
		itor = classMapFilter->GetColumnFactory().m_usedColumnMap.find(accessString);
		if (itor != m_usedColumnMap.end())
			{
			foundColumn = true;
			break;
			}
		}

	if (foundColumn)
		{
		//Find a column that is suitable
		const std::set<DbColumn const*>& mappedColumns = itor->second;
		for (DbColumn const* mappedColumn : mappedColumns)
			{
			//set allocate column to mapped column if it fits
			if (IsCompatible(*mappedColumn, type, param))
				{
				outColumn = const_cast<DbColumn*>(mappedColumn);
				break;
				}
			}

		if (outColumn == nullptr)
			outColumn = CreateColumn(property, type, param, accessString);
		}
	else
		{
		outColumn = CreateColumn(property, type, param, accessString);
		}

	//Register column
	if (outColumn)
		{
		for (const ClassMap* classMapFilter : m_compoundFilter)
			classMapFilter->GetColumnFactory().AddColumnToCache(*outColumn, accessString);
		}

	RemoveCompoundFilter();
	return outColumn;
	}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsColumnInUseByClassMap(DbColumn const& column) const
	{
	for (const ClassMap* classMap : m_compoundFilter)
		{
		if (classMap->GetColumnFactory().m_usedColumnSet.find(&column) != classMap->GetColumnFactory().m_usedColumnSet.end())
			return true;
		}

	return false;
	}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::ApplyDefaultStrategy(ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
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

        GetECDb().GetECDbImplR().GetIssueReporter().Report("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                                                        " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                                                        existingColumn->GetName().c_str(), GetTable().GetName().c_str());
        return nullptr;
        }


    BeAssert(!params.GetColumnName().empty() && "Column name must not be null for default strategy");
    bool effectiveNotNullConstraint = params.AddNotNullConstraint();
    if (params.AddNotNullConstraint() && (GetTable().HasExclusiveRootECClass() && GetTable().GetExclusiveRootECClassId() != m_classMap.GetClass().GetId()))
        {
        LOG.warningv("For the ECProperty '%s' on ECClass '%s' a NOT NULL constraint is defined. The constraint cannot be enforced though because "
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

    DbColumn* newColumn = GetTable().CreateColumn(resolvedColumnName, colType, DbColumn::Kind::DataColumn, PersistenceType::Physical);
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
DbColumn* ClassMapColumnFactory::ApplySharedColumnStrategy(ECN::ECPropertyCR prop, DbColumn::Type colType, DbColumn::CreateParams const& params) const
    {
    //Defining a col name for a shared column is a DB thing and DB CAs are taken strictly.
    if (params.IsColumnNameFromPropertyMapCA())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a value for 'ColumnName'. "
                                                        "'ColumnName' must not be specified for this ECProperty because it is mapped to a column shared with other ECProperties.",
                                                        prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //Defining a collation which is not doable is an error because this is a DB thing and DB CAs are taken strictly.
    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a Collation constraint "
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
        LOG.warningv("For the ECProperty '%s' on ECClass '%s' either a NOT NULL or a UNIQUE constraint is defined. The constraint cannot be enforced though because "
                     "the ECProperty is mapped to a column shared with other ECProperties.",
                     prop.GetName().c_str(), prop.GetClass().GetFullName());

        addNotNullConstraint = false;
        addUniqueConstraint = false;
        }

    DbColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<DbColumn*>(reusableColumn);

    DbColumn* col = GetTable().CreateSharedColumn(colType);
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
BentleyStatus ClassMapColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId classId, int retryCount) const
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
ECClassId ClassMapColumnFactory::GetPersistenceClassId(ECN::ECPropertyCR ecProp, Utf8StringCR propAccessString) const
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
bool ClassMapColumnFactory::TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const
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
void ClassMapColumnFactory::AddColumnToCache(DbColumn const& column, Utf8StringCR accessString) const
    {
    m_usedColumnMap[accessString].insert(&column);
    m_usedColumnSet.insert(&column);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsCompatible(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& params) const
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
BentleyStatus ClassMapColumnFactory::ComputeRelevantClassMaps(bmap<ECN::ECClassCP, ClassMap const*>& contextGraph) const
    {
    contextGraph.clear();
    if (!m_classMap.GetMapStrategy().GetTphInfo().IsValid())
        return SUCCESS;

    if (!GetTable().HasExclusiveRootECClass())
        {
        BeAssert(false);
        return ERROR;
        }

    ECClassCP exclusiveRootClass = GetECDb().Schemas().GetECClass(GetTable().GetExclusiveRootECClassId());
    if (exclusiveRootClass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    bset<ClassMap const*> mixins;
    bset<ECN::ECClassId> doneList;
    bset<ECN::ECClassId> primaryHierarchyClassIds;
    CachedStatementPtr stmt = GetECDb().GetCachedStatement("SELECT ClassId FROM " TABLE_ClassHierarchyCache " WHERE BaseClassId = ?1 AND ClassId != ?1 UNION ALL "
                                                      "SELECT BaseClassId FROM " TABLE_ClassHierarchyCache " WHERE ClassId = ?1 ");
    stmt->BindId(1, m_classMap.GetClass().GetId());
    while (stmt->Step() == BE_SQLITE_ROW)
        primaryHierarchyClassIds.insert(stmt->GetValueId<ECClassId>(0));

    std::function<const ClassMap*(ECClassCR)> findFirstImplementationOfMixin = [&] (ECN::ECClassCR mixInClass)
        {
        if (mixInClass.GetId() != m_classMap.GetClass().GetId())
            {
            auto itor = contextGraph.find(&mixInClass);
            if (itor != contextGraph.end())
                return itor->second;

            if (ClassMap const* mixInClassMap = m_classMap.GetDbMap().GetClassMap(mixInClass))
                {
                if (mixInClassMap->GetJoinedTable().GetId() == GetTable().GetId())
                    return mixInClassMap;
                }
            }

        for (ECClassCP derivedClass : GetECDb().Schemas().GetDerivedECClasses(mixInClass))
            {
            if (const ClassMap* foundImpl = findFirstImplementationOfMixin(*derivedClass))
                {
                if (foundImpl->GetJoinedTable().GetId() == GetTable().GetId())
                    return foundImpl;
                }
            }

        return (const ClassMap*) nullptr;
        };

    std::function<void(ECClassCR)> traverseDerivedClasses = [&] (ECN::ECClassCR contextClass)
        {
        if (doneList.find(contextClass.GetId()) != doneList.end())
            return;

        const bool isPartOfPrimaryHierarchy = (primaryHierarchyClassIds.find(contextClass.GetId()) != primaryHierarchyClassIds.end());
        doneList.insert(contextClass.GetId());
        if (isPartOfPrimaryHierarchy)
            {
            ClassMap const* contextClassMap = m_classMap.GetDbMap().GetClassMap(contextClass);
            if (contextClassMap != nullptr)
                {
                if (contextClassMap->GetJoinedTable().GetId() == GetTable().GetId())
                    contextGraph.insert(bpair<ECClassCP, ClassMap const*>(&contextClass, contextClassMap));
                else
                    {
                    //If a class is in another table then we do not care and also do not care about its derived classes or its interfaces
                    return;
                    }
                }
            }

        //! Find mixins if any
        for (ECClassCP baseClass : contextClass.GetBaseClasses())
            {
            ClassMap const* mixInClassMap = m_classMap.GetDbMap().GetClassMap(*baseClass);
            if (mixInClassMap == nullptr)
                continue;

            if (mixInClassMap->GetJoinedTable().GetPersistenceType() == PersistenceType::Virtual &&
                mixins.find(mixInClassMap) != mixins.end())
                {
                mixins.insert(mixInClassMap);
                if (contextGraph.find(&mixInClassMap->GetClass()) == contextGraph.end())
                    {
                    const ClassMap* impl = findFirstImplementationOfMixin(mixInClassMap->GetClass());
                    if (impl != nullptr && contextGraph.find(&impl->GetClass()) == contextGraph.end())
                        contextGraph[&mixInClassMap->GetClass()] = impl;
                    }
                }
            }

        //! traverse derive hierarchy as long as possible while staying in same table
        const ssize_t n = contextGraph.size();
        for (ECClassCP derivedClass : GetECDb().Schemas().GetDerivedECClasses(contextClass))
            traverseDerivedClasses(*derivedClass);

        //! record only deepest class in primary hierarchy
        if (isPartOfPrimaryHierarchy && contextGraph.size() > n)
            {
            auto contextItemItor = contextGraph.find(&contextClass);
            if (contextItemItor != contextGraph.end())
                contextGraph.erase(contextItemItor);
            }
        };

	for (bpair<ECN::ECClassId, LightweightCache::RelationshipEnd> const& relKey : m_classMap.GetDbMap().GetLightweightCache().GetRelationshipClasssForConstraintClass(m_classMap.GetClass().GetId()))
		{
		//!We are interested in relationship that are end table and are persisted in m_classMap.GetJoinedTable()		
		ECClassCP relClass = m_classMap.GetDbMap().GetECDb().Schemas().GetECClass(relKey.first);
		BeAssert(relClass != nullptr);
		ClassMap const* relMap = m_classMap.GetDbMap().GetClassMap(*relClass);
		if (relMap == nullptr || relMap->GetTables().empty())
			continue;

		if (relMap->GetType() != ClassMap::Type::RelationshipEndTable)
			continue;

		const RelationshipClassEndTableMap* endTableMap = static_cast<const RelationshipClassEndTableMap*>(relMap);		
		RelationshipConstraintMap const& persistedEnd = endTableMap->GetConstraintMap(endTableMap->GetForeignEnd());
		if (!persistedEnd.GetECInstanceIdPropMap()->IsMappedToTable(m_classMap.GetJoinedTable()))
			continue;

		contextGraph[relClass] = endTableMap;
		}

    traverseDerivedClasses(*exclusiveRootClass);
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbTable& ClassMapColumnFactory::GetTable() const  { return m_classMap.GetJoinedTable();  }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbCR ClassMapColumnFactory::GetECDb() const { return m_classMap.GetDbMap().GetECDb(); }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::Debug() const
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