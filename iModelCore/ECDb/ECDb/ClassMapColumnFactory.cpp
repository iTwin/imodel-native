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
void ClassMapColumnFactory::Initialize()const
    {
    UsedColumnFinder::ColumnMap columnMap;
    if (UsedColumnFinder::Find(columnMap, m_classMap) == SUCCESS)
        {
        for (auto const& entry : columnMap)
            {
            AddColumnToCache(*entry.second, entry.first);
            }

        return;
        }

    BeAssert(false && "UsedColumnFinder::Find(columnMap, m_classMap) return ERROR");
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

//**************************ClassMapColumnFactory::UsedColumnFinder*************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
ClassMap const* ClassMapColumnFactory::UsedColumnFinder::GetClassMap(ECN::ECClassCR entityClass) const
    {
    return m_classMap.GetDbMap().GetClassMap(entityClass);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
bool ClassMapColumnFactory::UsedColumnFinder::IsMappedIntoContextClassMapTables(ClassMap const& classMap) const
    {
    for (DbTable const* table : classMap.GetTables())
        {
        if (m_contextMapTableSet.find(table) != m_contextMapTableSet.end())
            return true;
        }

    return false;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
bool ClassMapColumnFactory::UsedColumnFinder::IsMappedIntoContextClassMapTables(PropertyMap const& propertyMap) const
    {
    GetTablesPropertyMapVisitor visitor;
    propertyMap.AcceptVisitor(visitor);
    for (DbTable const* table : visitor.GetTables())
        {
        if (m_contextMapTableSet.find(table) != m_contextMapTableSet.end())
            return true;
        }

    return false;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::ResolveMixins()
    {
    for (ClassMapCP deepestClassMapped : m_deepestClassMapped)
        {
        for (ECClassCP baseClass : deepestClassMapped->GetClass().GetBaseClasses())
            {
            if (baseClass->IsEntityClass())
                {
                ECEntityClassCP entityClassMap = static_cast<ECEntityClassCP>(baseClass);
                if (entityClassMap->IsMixin())
                    {
                    auto itor = m_mixIns.find(entityClassMap);
                    if (itor == m_mixIns.end())
                        {
                        //current class interface. We had not added them before 
                        //so add them now.
                        m_mixIns.insert(entityClassMap);
                        }
                    else 
                        {
                        //Remove the mixin as we already have implementation
                        m_mixIns.erase(itor);
                        }
                    }
                }
            }
        }

    //this mixin has not be resolved. We need to find it implementation if avaliable.
    if (!m_mixIns.empty())
        {
        std::vector<ECEntityClassCP> collection(m_mixIns.begin(), m_mixIns.end());
        for (auto unresolvedMixIn : collection)
            {
            if (ResolveMixins(*unresolvedMixIn) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::ResolveMixins(ECN::ECClassCR currentClass)
    {
    if (m_primaryHierarchy.find(&currentClass) != m_primaryHierarchy.end())
        return SUCCESS;

    ClassMap const* currentClassMap = GetClassMap(currentClass);
    if (currentClassMap && !currentClassMap->GetTables().empty())
        {
        if (currentClass.GetId() != m_classMap.GetClass().GetId())
            {
            if (IsMappedIntoContextClassMapTables(*currentClassMap))
                {
                for (ECClassCP baseClass : currentClass.GetBaseClasses())
                    {
                    if (baseClass->IsEntityClass())
                        {
                        ECEntityClassCP entityClassMap = static_cast<ECEntityClassCP>(baseClass);
                        if (entityClassMap->IsMixin())
                            {
                            auto itor = m_mixIns.find(entityClassMap);
                            if (itor != m_mixIns.end())
                                {
                                m_mixIns.erase(itor);
                                m_mixInsImpl[entityClassMap] = currentClassMap;
                                }
                            }
                        }
                    }
                }
            }
        }

    for (ECClassCP derivedClass : m_classMap.GetDbMap().GetECDb().Schemas().GetDerivedECClasses(currentClass))
        {
        if (m_mixIns.empty()) return SUCCESS;
        if (ResolveMixins(*derivedClass) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::TraverseClassHierarchy(ECN::ECClassCR currentClass, ClassMap const* parentClassMap)
    {
    if (m_primaryHierarchy.find(&currentClass) != m_primaryHierarchy.end())
        return SUCCESS;

    m_primaryHierarchy.insert(&currentClass);
    ClassMap const* currentClassMap = GetClassMap(currentClass);
    if (currentClass.GetId() == m_classMap.GetClass().GetId())
        {
        for (ECClassCP baseClass : currentClass.GetBaseClasses())
            {
            ECEntityClassCP entityClass = baseClass->GetEntityClassCP();
            if (entityClass && !entityClass->IsMixin())
                {
                if (ClassMap const* baseClassMap = GetClassMap(*baseClass))
                    m_deepestClassMapped.insert(baseClassMap);
                }
            }
        }

    if (currentClassMap && !currentClassMap->GetTables().empty())
        {
        if (currentClass.GetId() != m_classMap.GetClass().GetId())
            if (IsMappedIntoContextClassMapTables(*currentClassMap))
                parentClassMap = currentClassMap;

        ECDerivedClassesList const& derivedClasses = m_classMap.GetDbMap().GetECDb().Schemas().GetDerivedECClasses(currentClass);
        if (derivedClasses.empty())
            {
            if (parentClassMap != nullptr)
                m_deepestClassMapped.insert(parentClassMap);
            }
        else
            {
            for (ECClassCP derivedClass : derivedClasses)
                if (TraverseClassHierarchy(*derivedClass, parentClassMap) != SUCCESS)
                    return ERROR;
            }
        }
    else
        {
        if (parentClassMap != nullptr)
            m_deepestClassMapped.insert(parentClassMap);
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::FindRelationshipEndTableMaps()
    {
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
        if (!IsMappedIntoContextClassMapTables(*persistedEnd.GetECInstanceIdPropMap()))
            continue;

        m_endTableRelationship.insert(endTableMap);
        }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
ClassMapColumnFactory::UsedColumnFinder::UsedColumnFinder(ClassMap const& classMap)
    :m_classMap(classMap)
    {
    m_contextMapTableSet.insert(classMap.GetTables().begin(), classMap.GetTables().end());
    }
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::QueryRelevantMixIns()
    {
    Utf8CP mixInSql =
        "SELECT  CHBC.BaseClassId from " TABLE_ClassHierarchyCache " CCH "
        "   INNER JOIN " TABLE_ClassHasBaseClasses " CHBC ON CHBC.ClassId = CCH.ClassId "
        "WHERE CCH.BaseClassId = ?  AND CHBC.BaseClassId IN ( "
        "   SELECT CA.ContainerId FROM " TABLE_Class " C "
        "       INNER JOIN " TABLE_Schema " S ON S.Id = C.SchemaId "
        "       INNER JOIN " TABLE_CustomAttribute " CA ON CA.ClassId = C.Id "
        "   WHERE C.Name = 'IsMixin' AND S.Name='CoreCustomAttributes') "
        "GROUP BY CHBC.BaseClassId ";

    ECDbCR ecdb = m_classMap.GetDbMap().GetECDb();
    CachedStatementPtr stmt = ecdb.GetCachedStatement(mixInSql);
    stmt->BindId(1, m_classMap.GetClass().GetId());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId mixInId = stmt->GetValueId<ECClassId>(0);
        ECClassCP classCP = ecdb.Schemas().GetECClass(mixInId);
        if (!classCP->IsEntityClass())
            {
            BeAssert(false && "SQL query has issue. Something changed about Mixin CA");
            return ERROR;
            }
        ECEntityClassCP entityClass = static_cast<ECEntityClassCP>(classCP);
        if (!entityClass->IsMixin())
            {
            BeAssert(false && "SQL query has issue. Expecting MixIn");
            return ERROR;
            }

        m_mixIns.insert(entityClass);
        }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::Execute(ColumnMap& columnMap)
    {
    //Traverse derived class hierarchy and find deepest mapped classes. 
    //It also identify any mixin that it encounter
    if (TraverseClassHierarchy(m_classMap.GetClass(), nullptr) != SUCCESS)
        return ERROR;

    if (QueryRelevantMixIns() != SUCCESS)
        return ERROR;

    //Find implementation for mixins and also remove the mixin from queue that has implementaiton as part of deepest mapped classes
    if (ResolveMixins() != SUCCESS)
        return ERROR;

    //Find relationship that is relevent to current class so to adds its column to used column list
    if (FindRelationshipEndTableMaps() != SUCCESS)
        return ERROR;

    //Append current map property maps
    if (!m_classMap.GetPropertyMaps().empty())
        {
        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        m_classMap.GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            columnMap.insert(std::make_pair(propertyMap->GetAccessString(), &propertyMap->GetAs<SingleColumnDataPropertyMap>()->GetColumn()));
        }

    for (ClassMapCP deepestClassMapped : m_deepestClassMapped)
        {
        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        deepestClassMapped->GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            {
            if (columnMap.find(propertyMap->GetAccessString().c_str()) != columnMap.end())
                continue;

            if (IsMappedIntoContextClassMapTables(*propertyMap))
                {
                SingleColumnDataPropertyMap const* singleColDataPropertyMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
                columnMap.insert(std::make_pair(singleColDataPropertyMap->GetAccessString(), &singleColDataPropertyMap->GetColumn()));
                }
            }
        }

    for (std::pair<ECEntityClassCP, ClassMapCP > const& entry : m_mixInsImpl)
        {
        std::set<Utf8CP, CompareIUtf8Ascii> mixInPropertySet;
        for (ECPropertyCP property : entry.first->GetProperties(true))
            mixInPropertySet.insert(property->GetName().c_str());

        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        entry.second->GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            {
            if (columnMap.find(propertyMap->GetAccessString().c_str()) != columnMap.end())
                continue;

            PropertyMap const* cur = propertyMap;
            while (cur->GetParent())
                cur = cur->GetParent();

            //ignore other properties in case of mixin
            if (mixInPropertySet.find(cur->GetName().c_str()) == mixInPropertySet.end())
                continue;

            if (IsMappedIntoContextClassMapTables(*propertyMap))
                {
                SingleColumnDataPropertyMap const* singleColDataPropertyMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
                columnMap.insert(std::make_pair(singleColDataPropertyMap->GetAccessString(), &singleColDataPropertyMap->GetColumn()));
                }
            }
        }

    for (RelationshipClassEndTableMap const* relClassEndTableMap : m_endTableRelationship)
        {
        RelationshipConstraintMap const& persistedEnd = relClassEndTableMap->GetConstraintMap(relClassEndTableMap->GetForeignEnd());
        for (DbTable const* mappedTable : m_classMap.GetTables())
            {
            if (auto const* ecInstanceId = persistedEnd.GetECInstanceIdPropMap()->FindDataPropertyMap(*mappedTable))
                columnMap.insert(std::make_pair(relClassEndTableMap->BuildQualifiedAccessString(ecInstanceId->GetAccessString()), &ecInstanceId->GetColumn()));

            if (auto const* relECClassId = relClassEndTableMap->GetECClassIdPropertyMap()->FindDataPropertyMap(*mappedTable))
                columnMap.insert(std::make_pair(relClassEndTableMap->BuildQualifiedAccessString(relECClassId->GetAccessString()), &relECClassId->GetColumn()));
            }
        }

 
    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::Find(ClassMapColumnFactory::UsedColumnFinder::ColumnMap& columnMap, ClassMap const& classMap)
    {
    UsedColumnFinder calc(classMap);
    if (calc.Execute(columnMap) != SUCCESS)
        return ERROR;

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE