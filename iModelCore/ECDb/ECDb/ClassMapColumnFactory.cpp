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
    if (m_usesSharedColumnStrategy)
        {
        if (m_classMap.GetMapStrategy().GetTphInfo().GetSharedColumnCount().IsValid())
            m_sharedColumnCount = m_classMap.GetMapStrategy().GetTphInfo().GetSharedColumnCount().Value();
        }

    Initialize();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       04/2017
//------------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetOverflowTable() const
    {
    if (GetTable().GetLinkNode().GetChildren().empty())
        {
        DbTable* overflowTable = m_classMap.GetDbMap().GetDbSchemaR().CreateOverflowTable(GetTable());
        const_cast<ClassMap&>(m_classMap).SetOverflowTable(*overflowTable);
        return overflowTable;
        }

    if (GetTable().GetLinkNode().GetChildren().size() == 1)
        {
        DbTable::LinkNode const* overflowTable = GetTable().GetLinkNode().GetChildren()[0];
        if (overflowTable->GetType() == DbTable::Type::Overflow)
            return &overflowTable->GetTableR();
        }

    BeAssert(false && "Cannot create overflow table");
    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       04/2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::BeginSharedColumnBlock(Utf8CP propertyName, bset<const ClassMap*> const* additionalFilter, int columnsRequired) const
    {
    if (m_columnReservationInfo != nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (!m_usesSharedColumnStrategy)
        {
        BeAssert(false && "Shared Column must be enabled for this allocation to work");
        return ERROR;
        }
    if (propertyName == nullptr)
        {
        BeAssert(columnsRequired > 0);
        }
    else
        {
        BeAssert(columnsRequired == 0);
        ECN::ECPropertyCP property = m_classMap.GetClass().GetPropertyP(propertyName);
        if (property == nullptr)
            {
            BeAssert(false && "Property must exist in associated class map");
            return ERROR;
            }

        columnsRequired = ColumnReservationInfo::MaxColumnsRequiredToPersistProperty(*property);
        }

    SetupCompoundFilter(additionalFilter);
    int sharedColumnThatCanBeCreated;
    int sharedColumnThatCanBeReused;
    if (TryGetAvailableColumns(sharedColumnThatCanBeCreated, sharedColumnThatCanBeReused) != SUCCESS)
        {
        RemoveCompoundFilter();
        return ERROR;
        }
    
    if (columnsRequired <= (sharedColumnThatCanBeReused + sharedColumnThatCanBeCreated))
        {
        m_columnReservationInfo = std::unique_ptr<ColumnReservationInfo>(new ColumnReservationInfo(sharedColumnThatCanBeReused, sharedColumnThatCanBeCreated));
        }
    else
        {
        DbTable* overflowTable = GetOverflowTable();
        if (overflowTable == nullptr)
            {
            RemoveCompoundFilter();
            return ERROR;
            }

        m_columnReservationInfo = std::unique_ptr<ColumnReservationInfo>(new ColumnReservationInfo(*overflowTable));
        }

    RemoveCompoundFilter();
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       04/2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::EndSharedColumnBlock() const
    {
    if (m_columnReservationInfo == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    m_columnReservationInfo = nullptr;
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::Initialize() const
    {
    bmap<Utf8String, DbColumn const*> columnMap;
    if (SUCCESS != UsedColumnFinder::Find(columnMap, m_classMap))
        {
        BeAssert(false && "UsedColumnFinder::Find(columnMap, m_classMap) return ERROR");
        return;
        }

    for (bpair<Utf8String, DbColumn const*> const& entry : columnMap)
        {
        AddColumnToCache(*entry.second, entry.first);
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
        m_compoundFilter.push_back(&m_classMap);
        }

    if (!additionalFilter)
        return;

    for (ClassMap const* additionalClassMap : *additionalFilter)
        {
        if (additionalClassMap == &m_classMap)
            continue;

        if (additionalClassMap->GetJoinedOrPrimaryTable().GetId() != GetTable().GetId())
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
    //First try to find existing map
    std::map<Utf8String, std::set<DbColumn const*>, CompareIUtf8Ascii>::iterator itor;
    for (ClassMap const* classMapFilter : m_compoundFilter)
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
        for (ClassMap const* classMapFilter : m_compoundFilter)
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
    for (ClassMap const* classMap : m_compoundFilter)
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

    //col rename needed for TPH and shared tables without column sharing if sibling classes have same property names
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
        return nullptr;

    if (effectiveNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (params.AddUniqueConstraint())
        newColumn->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        newColumn->GetConstraintsR().SetCollation(params.GetCollation());

    AddColumnToCache(*newColumn, accessString);
    return newColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      04/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::TryGetAvailableColumns(int& sharedColumnThatCanBeCreated, int& sharedColumnThatCanBeReused) const
    {
    const int maxColumnInBaseTable = 63;
    const std::vector<DbColumn const*> physicalColumns = GetTable().FindAll(PersistenceType::Physical);
    const std::vector<DbColumn const*> sharedColumns = GetTable().FindAll(DbColumn::Kind::SharedDataColumn);
    const int nAvaliablePhysicalColumns = maxColumnInBaseTable - (int) physicalColumns.size();
    sharedColumnThatCanBeReused = 0;
    for (DbColumn const* sharedColumn : sharedColumns)
        if (!IsColumnInUseByClassMap(*sharedColumn))
            sharedColumnThatCanBeReused++;

    if (m_sharedColumnCount >= 0)
        {
        if (sharedColumns.size() > m_sharedColumnCount)
            {
            BeAssert(false && "SharedColumnCount bypassed the limit set in CA");
            return ERROR;
            }

        sharedColumnThatCanBeCreated = m_sharedColumnCount - (int)sharedColumns.size();
        if (sharedColumnThatCanBeCreated > nAvaliablePhysicalColumns)
            sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns; //restrict avaliable shared columns to avaliable physical columsn
        }
    else
        {
        sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns;
        }


    return SUCCESS;
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
        {
        if (m_columnReservationInfo && !m_columnReservationInfo->GetOverflowTable())
            m_columnReservationInfo->AllocateExisting();

        return const_cast<DbColumn*>(reusableColumn);
        }
    
    DbColumn* col;
    if (m_columnReservationInfo && m_columnReservationInfo->GetOverflowTable())
        col = m_columnReservationInfo->GetOverflowTable()->CreateSharedColumn(colType);
    else
        col = GetTable().CreateSharedColumn(colType);

    if (m_columnReservationInfo && !m_columnReservationInfo->GetOverflowTable())
        m_columnReservationInfo->AllocateNew();

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
DbTable& ClassMapColumnFactory::GetTable() const  { return m_classMap.GetJoinedOrPrimaryTable();  }

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
    sql.Append("ClassMap : ").AppendLine(m_classMap.GetClass().GetFullName());

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
        std::vector<ECEntityClassCP> unresolvedMixins(m_mixins.begin(), m_mixins.end());
        for (ECEntityClassCP mixIn : unresolvedMixins)
            {
            if (deepestClassMapped->GetClass().Is(mixIn))
                {
                m_mixins.erase(mixIn);
                }
            }
        }

    //this mixin has not be resolved. We need to find it implementation if avaliable.
    if (!m_mixins.empty())
        {
        //std::vector<ECEntityClassCP> unresolvedMixins(m_mixins.begin(), m_mixins.end());
        for (ECEntityClassCP unresolvedMixin : m_mixins)
            {
            ClassMap const* impl = ResolveMixin(*unresolvedMixin);
            if (impl != nullptr)
                {
                m_mixinImpls[unresolvedMixin] = impl;
                }
            else
                {
                //Find base mixin and find there impl
                ResolveBaseMixin(*unresolvedMixin);
                }
            }
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
void ClassMapColumnFactory::UsedColumnFinder::ResolveBaseMixin(ECN::ECClassCR currentClass)
    {
    for (ECClassCP baseClass : currentClass.GetBaseClasses())
        {
        ECEntityClassCP baseEntityClass = baseClass->GetEntityClassCP();
        ClassMap const* impl = ResolveMixin(*baseEntityClass);
        if (impl != nullptr)
            {
            if (m_mixinImpls.find(baseEntityClass) == m_mixinImpls.end())
                m_mixinImpls[baseEntityClass] = impl;
            }
        else
            {
            ResolveBaseMixin(*baseEntityClass);
            }
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
ClassMap const* ClassMapColumnFactory::UsedColumnFinder::ResolveMixin(ECN::ECClassCR currentClass)
    {
    ClassMap const* currentClassMap = GetClassMap(currentClass);
    if (currentClassMap && !currentClassMap->GetTables().empty() &&
        currentClass.GetId() != m_classMap.GetClass().GetId() && IsMappedIntoContextClassMapTables(*currentClassMap))
        {
        return currentClassMap;
        }

    for (ECClassCP derivedClass : m_classMap.GetDbMap().GetECDb().Schemas().GetDerivedClasses(currentClass))
        {
        if (currentClassMap = ResolveMixin(*derivedClass))
            return currentClassMap;
        }

    return nullptr;
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

    if (currentClassMap == nullptr || currentClassMap->GetTables().empty())
        {
        if (parentClassMap != nullptr)
            m_deepestClassMapped.insert(parentClassMap);

        return SUCCESS;
        }

    if (currentClass.GetId() != m_classMap.GetClass().GetId() && IsMappedIntoContextClassMapTables(*currentClassMap))
        parentClassMap = currentClassMap;

    ECDerivedClassesList const& derivedClasses = m_classMap.GetDbMap().GetECDb().Schemas().GetDerivedClasses(currentClass);
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

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::FindRelationshipEndTableMaps(ECN::ECClassId classId)
    {
    for (bpair<ECN::ECClassId, LightweightCache::RelationshipEnd> const& relKey : m_classMap.GetDbMap().GetLightweightCache().GetRelationshipClasssForConstraintClass(classId))
        {
        if (m_endTableRelationship.find(relKey.first) != m_endTableRelationship.end())
            continue;

        m_endTableRelationship[relKey.first] = nullptr;
        //!We are interested in relationship that are end table and are persisted in m_classMap.GetJoinedOrPrimaryTable()
        ECClassCP relClass = m_classMap.GetDbMap().GetECDb().Schemas().GetClass(relKey.first);
        BeAssert(relClass != nullptr);
        ClassMap const* relMap = m_classMap.GetDbMap().GetClassMap(*relClass);
        if (relMap == nullptr || relMap->GetTables().empty())
            continue;

        if (relMap->GetType() != ClassMap::Type::RelationshipEndTable)
            continue;

        RelationshipClassEndTableMap const& endTableMap = relMap->GetAs<RelationshipClassEndTableMap>();
        RelationshipConstraintMap const& persistedEnd = endTableMap.GetConstraintMap(endTableMap.GetForeignEnd());
        if (!IsMappedIntoContextClassMapTables(*persistedEnd.GetECInstanceIdPropMap()))
            continue;

        m_endTableRelationship[endTableMap.GetClass().GetId()] = &endTableMap;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2017
//---------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::QueryRelevantMixins()
    {
    ECDbCR ecdb = m_classMap.GetDbMap().GetECDb();
    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        "SELECT  CHBC.BaseClassId from " TABLE_ClassHierarchyCache " CCH "
        "INNER JOIN " TABLE_ClassHasBaseClasses " CHBC ON CHBC.ClassId = CCH.ClassId "
        "WHERE CCH.BaseClassId=? AND CHBC.BaseClassId IN ("
        "SELECT CA.ContainerId FROM " TABLE_Class " C"
        "                      INNER JOIN " TABLE_Schema " S ON S.Id=C.SchemaId"
        "                      INNER JOIN " TABLE_CustomAttribute " CA ON CA.ClassId = C.Id "
        "                      WHERE C.Name = 'IsMixin' AND S.Name='CoreCustomAttributes') "
        "GROUP BY CHBC.BaseClassId");

    if (BE_SQLITE_OK != stmt->BindId(1, m_classMap.GetClass().GetId()))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId mixinId = stmt->GetValueId<ECClassId>(0);
        ECClassCP classCP = ecdb.Schemas().GetClass(mixinId);
        if (!classCP->IsEntityClass() || !classCP->GetEntityClassCP()->IsMixin())
            {
            BeAssert(false && "SQL query has issue. Something changed about Mixin CA");
            return ERROR;
            }

        m_mixins.insert(classCP->GetEntityClassCP());
        }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::Execute(bmap<Utf8String, DbColumn const*>& columnMap)
    {
    //Traverse derived class hierarchy and find deepest mapped classes. 
    //It also identify any mixin that it encounter
    if (TraverseClassHierarchy(m_classMap.GetClass(), nullptr) != SUCCESS)
        return ERROR;

    if (QueryRelevantMixins() != SUCCESS)
        return ERROR;

    //Find implementation for mixins and also remove the mixin from queue that has implementation as part of deepest mapped classes
    if (ResolveMixins() != SUCCESS)
        return ERROR;

    //Find relationship that is relevant to current class so to adds its column to used column list
    for(ECClassCP ecClass: m_primaryHierarchy)
        if (FindRelationshipEndTableMaps(m_classMap.GetClass().GetId()) != SUCCESS)
            return ERROR;

    //Append current map property maps
    if (!m_classMap.GetPropertyMaps().empty())
        {
        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        m_classMap.GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            columnMap.insert(bpair<Utf8String, DbColumn const*>(propertyMap->GetAccessString(), &propertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn()));
        }

    for (ClassMapCP deepestClassMapped : m_deepestClassMapped)
        {
        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        deepestClassMapped->GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            {
            if (columnMap.find(propertyMap->GetAccessString()) != columnMap.end())
                continue;

            if (IsMappedIntoContextClassMapTables(*propertyMap))
                {
                SingleColumnDataPropertyMap const& singleColDataPropertyMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
                columnMap.insert(bpair<Utf8String, DbColumn const*>(singleColDataPropertyMap.GetAccessString(), &singleColDataPropertyMap.GetColumn()));
                }
            }
        }

    for (std::pair<ECEntityClassCP, ClassMapCP > const& entry : m_mixinImpls)
        {
        std::set<Utf8CP, CompareIUtf8Ascii> mixInPropertySet;
        for (ECPropertyCP property : entry.first->GetProperties(true))
            mixInPropertySet.insert(property->GetName().c_str());

        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        entry.second->GetPropertyMaps().AcceptVisitor(visitor);
        for (PropertyMap const* propertyMap : visitor.Results())
            {
            if (columnMap.find(propertyMap->GetAccessString()) != columnMap.end())
                continue;

            PropertyMap const* cur = propertyMap;
            while (cur->GetParent())
                cur = cur->GetParent();

            //ignore other properties in case of mixin
            if (mixInPropertySet.find(cur->GetName().c_str()) == mixInPropertySet.end())
                continue;

            if (IsMappedIntoContextClassMapTables(*propertyMap))
                {
                SingleColumnDataPropertyMap const& singleColDataPropertyMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
                columnMap.insert(bpair<Utf8String, DbColumn const*>(singleColDataPropertyMap.GetAccessString(), &singleColDataPropertyMap.GetColumn()));
                }
            }
        }

    for (auto const& row : m_endTableRelationship)
        {
        if (!row.second) 
            continue;

        RelationshipClassEndTableMap const* relClassEndTableMap = row.second;
        RelationshipConstraintMap const& persistedEnd = relClassEndTableMap->GetConstraintMap(relClassEndTableMap->GetReferencedEnd());
        SystemPropertyMap::PerTableIdPropertyMap const* relECClassIdPropMap = nullptr;
        SystemPropertyMap::PerTableIdPropertyMap const* ecInstanceIdPropMap = nullptr;
        for (DbTable const* mappedTable : m_classMap.GetTables())
            {
            if (!ecInstanceIdPropMap)
                {
                ecInstanceIdPropMap = persistedEnd.GetECInstanceIdPropMap()->FindDataPropertyMap(*mappedTable);
                if (ecInstanceIdPropMap != nullptr)
                    columnMap.insert(bpair<Utf8String, DbColumn const*>(relClassEndTableMap->BuildQualifiedAccessString(ecInstanceIdPropMap->GetAccessString()), &ecInstanceIdPropMap->GetColumn()));
                }

            if (!relECClassIdPropMap) 
                {
                relECClassIdPropMap = relClassEndTableMap->GetECClassIdPropertyMap()->FindDataPropertyMap(*mappedTable);
                if (relECClassIdPropMap != nullptr)
                    columnMap.insert(bpair<Utf8String, DbColumn const*>(relClassEndTableMap->BuildQualifiedAccessString(relECClassIdPropMap->GetAccessString()), &relECClassIdPropMap->GetColumn()));
                }
            }
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapColumnFactory::UsedColumnFinder::Find(bmap<Utf8String, DbColumn const*>& columnMap, ClassMap const& classMap)
    {
    UsedColumnFinder calc(classMap);
    return calc.Execute(columnMap);
    }



//**************************ClassMapColumnFactory::ColumnReservationInfo*************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       02 / 2017
//------------------------------------------------------------------------------------------
//static 
int ClassMapColumnFactory::ColumnReservationInfo::MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR ecProperty)
    {
    int columnsRequired = 0;
    if (ecProperty.GetIsNavigation())
        {
        columnsRequired = 2;
        }
    else if (PrimitiveECPropertyCP primitive = ecProperty.GetAsPrimitiveProperty())
        {
        if (primitive->GetType() == PrimitiveType::PRIMITIVETYPE_Point3d)
            columnsRequired = 3;
        else if (primitive->GetType() == PrimitiveType::PRIMITIVETYPE_Point2d)
            columnsRequired = 2;
        else
            columnsRequired = 1;
        }
    else if (ecProperty.GetIsArray())
        {
        columnsRequired = 1;
        }
    else if (StructECPropertyCP structProperty = ecProperty.GetAsStructProperty())
        {
        for (ECN::ECPropertyCP prop : structProperty->GetType().GetProperties(true))
            columnsRequired += MaxColumnsRequiredToPersistProperty(*prop);
        }
    else
        {
        columnsRequired = std::numeric_limits<int>().max();
        BeAssert(false && "Unknow type of ECProperty");
        }

    return columnsRequired;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE