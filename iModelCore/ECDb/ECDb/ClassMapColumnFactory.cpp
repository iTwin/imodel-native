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

bool ColumnMaps::IsColumnInUsed(DbColumn const& column) const
    {
    return n_columns.find(&column) != n_columns.end();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ColumnMaps::Insert(SingleColumnDataPropertyMap const& propertyMap)
    {
    Insert(propertyMap.GetAccessString().c_str(), propertyMap.GetColumn());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
Utf8StringCR ColumnMaps::Copy(Utf8StringCR str)
    {
    auto itor = m_strings.find(str);
    if (itor != m_strings.end())
        return *itor;

    return *(m_strings.insert(str).first);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ColumnMaps::Insert(Utf8StringCR accessString, DbColumn const& column, bool newlyMappedColumn)
    {
    Utf8StringCR copiedAccessString = Copy(accessString);
    m_maps.insert(make_bpair(copiedAccessString.c_str(), &column));
    n_columns.insert(&column);
    if (newlyMappedColumn)
        m_newMappedColumns.insert(copiedAccessString.c_str());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryInheritedColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap)
    {
    ECDbCR ecdb = classMap.GetDbMap().GetECDb();
    ECClassCR  contextClass = classMap.GetClass();
    if (!contextClass.HasBaseClasses())
        return SUCCESS;

    std::vector<ECN::ECClassCP> mixins;
    std::vector<ClassMapCP> baseClasses;

    for (ECClassCP baseClass : contextClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = ecdb.Schemas().GetDbMap().GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            {
            BeAssert(false && "Expecting class map for primary base class to exist and never null");
            return ERROR;
            }

        if (baseClass->IsEntityClass() && baseClass->GetEntityClassCP()->IsMixin())
            {
            mixins.push_back(baseClass);
            continue;
            }

        if (baseClassMap->GetPrimaryTable().GetId() != classMap.GetPrimaryTable().GetId())
            continue;

        baseClasses.push_back(baseClassMap);
        }


    if (baseClasses.size() > 1)
        {
        BeAssert(false && "Expecting zero or one base class");
        return ERROR;
        }

    if (!mixins.empty())
        {
        if (QueryMixinColumnMaps(columnMaps, classMap, &mixins) != SUCCESS)
            return ERROR;
        }

    for (ClassMap const* baseClassMap : baseClasses)
        if (Query(columnMaps, *baseClassMap, Filter::InheritedAndLocal, nullptr) != SUCCESS)
            return ERROR;

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryDerivedColumnMaps(ColumnMaps& columnMaps, ClassMap const& contextClassMap)
    {
    ECDbCR ecdb = contextClassMap.GetDbMap().GetECDb();
    ECDerivedClassesList const& derivedClasses = ecdb.Schemas().GetDerivedClasses(contextClassMap.GetClass());
    DbMap const& dbMap = ecdb.Schemas().GetDbMap();
    for (ECN::ECClassCP derivedClass : derivedClasses)
        {
        if (ClassMapCP derivedClassMap = dbMap.GetClassMap(*derivedClass))
            {
            DbTable const& primTable = derivedClassMap->GetPrimaryTable();
            if (primTable.GetType() == DbTable::Type::Virtual)
                continue;

            if (primTable.GetId() != contextClassMap.GetPrimaryTable().GetId())
                continue;

            if (Query(columnMaps, *derivedClassMap, Filter::DerivedAndLocal, &contextClassMap) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ColumnMapContext::AppendRelationshipColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap, ECN::ECClassId relationshipClassId)
    {
    std::vector<DbTable*> const& tables = classMap.GetTables();
    ECDbCR ecdb = classMap.GetDbMap().GetECDb();
    ECClassCP relClass = ecdb.Schemas().GetClass(relationshipClassId);
    BeAssert(relClass != nullptr);
    ClassMap const* relMap = ecdb.Schemas().GetDbMap().GetClassMap(*relClass);
    if (relMap == nullptr || relMap->GetTables().empty())
        return;

    if (relMap->GetType() != ClassMap::Type::RelationshipEndTable)
        return;

    RelationshipClassEndTableMap const& endTableMap = relMap->GetAs<RelationshipClassEndTableMap>();
    RelationshipConstraintMap const& persistedEnd = endTableMap.GetConstraintMap(endTableMap.GetReferencedEnd());
    const SingleColumnDataPropertyMap* id = nullptr;
    const SingleColumnDataPropertyMap* relClassId = nullptr;
    for (DbTable const* table : tables)
        {
        if (id == nullptr)
            id = persistedEnd.GetECInstanceIdPropMap()->FindDataPropertyMap(*table);

        if (relClassId == nullptr)
            relClassId = relMap->GetECClassIdPropertyMap()->FindDataPropertyMap(*table);

        if (id != nullptr && relClassId != nullptr)
            break;
        }

    if (relClassId != nullptr || id != nullptr)
        {
        columnMaps.Insert(endTableMap.GetAccessStringForId(), id->GetColumn());
        columnMaps.Insert(endTableMap.GetAccessStringForRelClassId(), relClassId->GetColumn());
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryEndTableRelationshipMaps(ColumnMaps& columnMaps, ClassMap const& classMap, RelationshpFilter filter)
    {
    LightweightCache const& lwc = classMap.GetDbMap().GetECDb().Schemas().GetDbMap().GetLightweightCache();
    //This should include relationship that the mixin have added to current class
    if (filter == RelationshpFilter::Direct)
        {
        for (ECN::ECClassId relClassId : lwc.GetDirectRelationshipClasssForConstraintClass(classMap.GetClass().GetId()))
            AppendRelationshipColumnMaps(columnMaps, classMap, relClassId);
        }
    else
        {
        for (auto const& entry : lwc.GetRelationshipClassesForConstraintClass(classMap.GetClass().GetId()))
            AppendRelationshipColumnMaps(columnMaps, classMap, entry.first);
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryMixinColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap, std::vector<ECN::ECClassCP> const* seedMixins)
    {
    ECDbCR ecdb = classMap.GetDbMap().GetECDb();
    std::vector<ECN::ECClassCP> mixins;
    if (seedMixins)
        mixins = *seedMixins;
    else
        if (FindMixins(mixins, ecdb, classMap.GetClass().GetId()) != SUCCESS)
            return ERROR;

    bmap<ECN::ECClassCP, ClassMapCP> resolved;
    std::deque<ECClassCP> q;
    for (ECClassCP mixin : mixins)
        {
        if (resolved.find(mixin) != resolved.end())
            continue;

        q.push_back(mixin);
        while (!q.empty())
            {
            if (ClassMapCP impl = FindMixinImplementation(ecdb, *q.front(), classMap.GetJoinedOrPrimaryTable().GetId(), classMap.GetClass().GetId()))
                {
                if (resolved.find(mixin) == resolved.end())
                    resolved[q.front()] = impl;

                q.clear();
                }
            else
                {
                for (ECClassCP b : q.front()->GetBaseClasses())
                    q.push_back(b);

                q.pop_front();
                }
            }
        }

    for (auto const& r : resolved)
        {
        ECClassCP mixin = r.first;
        ClassMapCP impl = r.second;
        for (ECPropertyCP property : mixin->GetProperties())
            {
            if (property->GetIsNavigation())
                continue;

            PropertyMap const* propertyMap = impl->GetPropertyMaps().Find(property->GetName().c_str());
            if (propertyMap == nullptr)
                continue;

            SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
            propertyMap->AcceptVisitor(visitor);
            for (PropertyMap const* p : visitor.Results())
                columnMaps.Insert(p->GetAs<SingleColumnDataPropertyMap>());
            }
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::FindMixins(std::vector<ECN::ECClassCP>& mixins, ECDbCR ecdb, ECN::ECClassId contextClassId)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        "SELECT  CHBC.BaseClassId from " TABLE_ClassHierarchyCache " CCH "
        "INNER JOIN " TABLE_ClassHasBaseClasses " CHBC ON CHBC.ClassId = CCH.ClassId "
        "WHERE CCH.BaseClassId=? AND CHBC.BaseClassId IN ("
        "SELECT CA.ContainerId FROM " TABLE_Class " C"
        "                      INNER JOIN " TABLE_Schema " S ON S.Id=C.SchemaId"
        "                      INNER JOIN " TABLE_CustomAttribute " CA ON CA.ClassId = C.Id "
        "                      WHERE C.Name = 'IsMixin' AND S.Name='CoreCustomAttributes') "
        "GROUP BY CHBC.BaseClassId");

    if (BE_SQLITE_OK != stmt->BindId(1, contextClassId))
        {
        BeAssert(false);
        return ERROR;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId mixinId = stmt->GetValueId<ECClassId>(0);
        ECClassCP classCP = ecdb.Schemas().GetClass(mixinId);
        if (!classCP->IsEntityClass() || !classCP->GetEntityClassCP()->IsMixin())
            {
            BeAssert(false && "SQL query has issue. Something changed about Mixin CA");
            return ERROR;
            }

        mixins.push_back(classCP);
        }

    auto a = begin(mixins);
    for (; a != end(mixins); ++a)
        for (auto b = a + 1; b != end(mixins); ++b)
            {
            if ((*a)->Is((*b)))
                {
                a = mixins.erase(a);
                break;
                }
            }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ClassMap const* ColumnMapContext::FindMixinImplementation(ECDbCR ecdb, ECN::ECClassCR contextClass, DbTableId primaryTable, ECN::ECClassId skipId)
    {
    ECDerivedClassesList const& derivedClasses = ecdb.Schemas().GetDerivedClasses(contextClass);
    DbMap const& dbMap = ecdb.Schemas().GetDbMap();
    for (ECN::ECClassCP derivedClass : derivedClasses)
        {
        if (skipId == derivedClass->GetId())
            continue;

        if (ClassMapCP dervicedClassMap = dbMap.GetClassMap(*derivedClass))
            {
            if (dervicedClassMap->GetJoinedOrPrimaryTable().GetId() == primaryTable)
                return dervicedClassMap;
            else
                return nullptr;
            }
        }

    for (ECN::ECClassCP derivedClass : derivedClasses)
        {
        if (skipId == derivedClass->GetId())
            continue;

        if (ClassMapCP dervicedClassMap = FindMixinImplementation(ecdb, *derivedClass, primaryTable, skipId))
            return dervicedClassMap;
        }

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::Query(ColumnMaps& columnMaps, ClassMap const& classMap, Filter filter, ClassMap const* base)
    {
    //Following is need for multisession import where base class already persisted.
    RelationshpFilter relationshipFilter = RelationshpFilter::All;
    bool isNewClass = classMap.GetState() == ObjectState::New;
    if (!isNewClass)
        relationshipFilter = RelationshpFilter::Direct;

    if (filter == Filter::InheritedAndLocal)
        {
        if (QueryLocalColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;

        if (QueryMixinColumnMaps(columnMaps, classMap, nullptr) != SUCCESS)
            return ERROR;

        if (QueryEndTableRelationshipMaps(columnMaps, classMap, relationshipFilter) != SUCCESS)
            return ERROR;

        if (base == nullptr)
            {
            const size_t unmapped = (classMap.GetClass().GetPropertyCount(true) + 2) - classMap.GetPropertyMaps().Size();
            if (unmapped > 0)
                {
                if (QueryInheritedColumnMaps(columnMaps, classMap) != SUCCESS)
                    return ERROR;
                }
            }
        }

    if (filter == Filter::DerivedAndLocal)
        {
        if (QueryLocalColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;

        if (QueryEndTableRelationshipMaps(columnMaps, classMap, relationshipFilter) != SUCCESS)
            return ERROR;

        if (QueryDerivedColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;
        }

    if (filter == Filter::Full)
        {
        if (QueryLocalColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;

        if (base == nullptr)
            {
            size_t unmapped = (classMap.GetClass().GetPropertyCount(true) + 2) - classMap.GetPropertyMaps().Size();
            if (unmapped > 0)
                {
                if (QueryInheritedColumnMaps(columnMaps, classMap) != SUCCESS)
                    return ERROR;
                }
            }

        if (QueryEndTableRelationshipMaps(columnMaps, classMap, relationshipFilter) != SUCCESS)
            return ERROR;

        if (QueryMixinColumnMaps(columnMaps, classMap, nullptr) != SUCCESS)
            return ERROR;

        if (QueryDerivedColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::Query(ColumnMaps& columnMaps, ClassMap const& classMap, Filter filter)
    {
    static double fullTime;
    StopWatch stopwatch(true);
    BentleyStatus r = Query(columnMaps, classMap, filter, nullptr);
    stopwatch.Stop();
    fullTime += stopwatch.GetElapsedSeconds();
    LOG.debugv("ColumnMapContext::Query(%s) (%.4f seconds). [total=%.4f]", classMap.GetClass().GetFullName(), stopwatch.GetElapsedSeconds(), fullTime);
    return r;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryLocalColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap)
    {
    for (ECPropertyCP property : classMap.GetClass().GetProperties(true))
        {
        if (property->GetIsNavigation())
            continue;

        PropertyMap const* propertyMap = classMap.GetPropertyMaps().Find(property->GetName().c_str());
        if (propertyMap == nullptr)
            continue;

        SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
        propertyMap->AcceptVisitor(visitor);
        for (PropertyMap const* p : visitor.Results())
            columnMaps.Insert(p->GetAs<SingleColumnDataPropertyMap>());
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
//static
uint32_t ClassMapColumnFactory::MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsNavigation())
        return 2;

    if (PrimitiveECPropertyCP primitive = ecProperty.GetAsPrimitiveProperty())
        {
        if (primitive->GetType() == PrimitiveType::PRIMITIVETYPE_Point3d)
            return 3;
        
        if (primitive->GetType() == PrimitiveType::PRIMITIVETYPE_Point2d)
            return 2;

        return 1;
        }

    if (ecProperty.GetIsArray())
        return 1;

    if (StructECPropertyCP structProperty = ecProperty.GetAsStructProperty())
        {
        uint32_t columnsRequired = 0;
        for (ECN::ECPropertyCP prop : structProperty->GetType().GetProperties(true))
            {
            columnsRequired += MaxColumnsRequiredToPersistProperty(*prop);
            }

        return columnsRequired;
        }

    BeAssert("Unhandled ECProperty type in ClassMapColumnFactory::MaxColumnsRequiredToPersistProperty");
    return 0;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ECDbCR ClassMapColumnFactory::GetECDb() const { return m_classMap.GetDbMap().GetECDb(); }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::AllocateColumn(ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    std::function<ECN::ECClassId(ECN::ECPropertyCR, Utf8StringCR)> getPersistenceClassId = [&] (ECN::ECPropertyCR ecProp, Utf8StringCR propAccessString)
        {
        const size_t dotPosition = propAccessString.find(".");
        ECN::ECPropertyCP property = nullptr;
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
            return ECN::ECClassId();
            }

        return property->GetClass().GetId();
        };

    std::function<void(Utf8StringR, Utf8StringCR, ECN::ECClassId, int)> resolveColumnName = [&] (Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId classId, int retryCount)
        {
        if (retryCount > 0)
            {
            BeAssert(!resolvedColumName.empty());
            resolvedColumName += SqlPrintfString("%d", retryCount);
            return;
            }

        if (requestedColumnName.empty())
            {
            //use name generator
            resolvedColumName.clear();
            return;
            }

        DbColumn const* existingColumn = GetEffectiveTable()->FindColumnP(requestedColumnName.c_str());
        if (existingColumn != nullptr && IsColumnInUse(*existingColumn))
            {
            Utf8Char classIdStr[ECN::ECClassId::ID_STRINGBUFFER_LENGTH];
            classId.ToString(classIdStr);
            resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName.c_str());
            }
        else
            resolvedColumName.assign(requestedColumnName);
        };


    DbColumn* existingColumn = GetEffectiveTable()->FindColumnP(params.GetColumnName().c_str());
    if (existingColumn != nullptr && !IsColumnInUse(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (GetEffectiveTable()->GetType() == DbTable::Type::Existing || 
            (existingColumn->GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                                                      existingColumn->GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                                                      existingColumn->GetConstraints().GetCollation() == params.GetCollation()))
            {
            return existingColumn;
            }

        GetECDb().GetECDbImplR().GetIssueReporter().Report("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                                                           " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                                                           existingColumn->GetName().c_str(), GetEffectiveTable()->GetName().c_str());
        return nullptr;
        }


    BeAssert(!params.GetColumnName().empty() && "Column name must not be null for default strategy");
    bool effectiveNotNullConstraint = params.AddNotNullConstraint();
    if (params.AddNotNullConstraint() && (GetEffectiveTable()->HasExclusiveRootECClass() && GetEffectiveTable()->GetExclusiveRootECClassId() != m_classMap.GetClass().GetId()))
        {
        LOG.warningv("For the ECProperty '%s' on ECClass '%s' a NOT NULL constraint is defined. The constraint cannot be enforced though because "
                     "the ECProperty has base ECClasses mapped to the same table.",
                     ecProp.GetName().c_str(), ecProp.GetClass().GetFullName());

        effectiveNotNullConstraint = false;
        }

    //col rename needed for TPH and shared tables without column sharing if sibling classes have same property names
    const ECN::ECClassId classId = getPersistenceClassId(ecProp, accessString);
    if (!classId.IsValid())
        return nullptr;

    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    resolveColumnName(tmp, params.GetColumnName(), classId, retryCount);

    resolvedColumnName = tmp;
    while (GetEffectiveTable()->FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        resolveColumnName(resolvedColumnName, params.GetColumnName(), classId, retryCount);
        }

    DbColumn* newColumn = GetEffectiveTable()->CreateColumn(resolvedColumnName, colType, DbColumn::Kind::DataColumn, PersistenceType::Physical);
    if (newColumn == nullptr)
        return nullptr;

    if (effectiveNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (params.AddUniqueConstraint())
        newColumn->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        newColumn->GetConstraintsR().SetCollation(params.GetCollation());

    return RegisterColumnMap(accessString, newColumn);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::AllocatedSharedColumn(ECN::ECPropertyCR prop, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
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
    if (params.AddNotNullConstraint() || params.AddUniqueConstraint())
        {
        LOG.warningv("For the ECProperty '%s' on ECClass '%s' either a NOT NULL or a UNIQUE constraint is defined. The constraint cannot be enforced though because "
                     "the ECProperty is mapped to a column shared with other ECProperties.",
                     prop.GetName().c_str(), prop.GetClass().GetFullName());

        }

    return RegisterColumnMap(accessString, ReuseOrCreateSharedColumn());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ClassMapColumnFactory::ReserveSharedColumns(Utf8StringCR propertyName) const
    {
    BeAssert(!propertyName.empty());
    ECN::ECPropertyCP property = m_classMap.GetClass().GetPropertyP(propertyName);
    if (property == nullptr)
            {
            BeAssert(false && "Property must exist in associated class map");
            return;
            }

    const uint32_t columnsRequired = MaxColumnsRequiredToPersistProperty(*property);
    ReserveSharedColumns(columnsRequired);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ClassMapColumnFactory::ReserveSharedColumns(uint32_t columnsRequired) const
    {
    if (m_areSharedColumnsReserved)
        {
        BeAssert(false);
        return;
        }

    if (!m_useSharedColumnStrategy)
        {
        BeAssert(false && "Shared Column must be enabled for this allocation to work");
        return;
        }

    std::function<bool(uint32_t&, uint32_t&)> findAvailableColumns = [this] (uint32_t& sharedColumnThatCanBeCreated, uint32_t& sharedColumnThatCanBeReused)
        {
        const uint32_t maxColumnInBaseTable = 63;
        const std::vector<DbColumn const*> physicalColumns = m_primaryOrJoinedTable->FindAll(PersistenceType::Physical);
        const std::vector<DbColumn const*> sharedColumns = m_primaryOrJoinedTable->FindAll(DbColumn::Kind::SharedDataColumn);
        const uint32_t nAvaliablePhysicalColumns = maxColumnInBaseTable - (uint32_t) physicalColumns.size();
        sharedColumnThatCanBeReused = 0;
        for (DbColumn const* sharedColumn : sharedColumns)
            {
            if (!IsColumnInUse(*sharedColumn))
                sharedColumnThatCanBeReused++;
            }

        if (!m_maxSharedColumnCount.IsNull())
            {
            if (((uint32_t) sharedColumns.size()) > m_maxSharedColumnCount.Value())
                {
                BeAssert(false && "SharedColumnCount bypassed the limit set in CA");
                return false;
                }

            sharedColumnThatCanBeCreated = m_maxSharedColumnCount.Value() - (uint32_t) sharedColumns.size();
            if (sharedColumnThatCanBeCreated > nAvaliablePhysicalColumns)
                sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns; //restrict available shared columns to available physical columns
            }
        else
            sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns;

        return true;
        };

    uint32_t sharedColumnThatCanBeCreated = 0, sharedColumnThatCanBeReused = 0;
    if (!findAvailableColumns(sharedColumnThatCanBeCreated, sharedColumnThatCanBeReused))
        return;

    if (columnsRequired > (sharedColumnThatCanBeReused + sharedColumnThatCanBeCreated))
        m_areSharedColumnsReserved = true;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetEffectiveTable() const
    {
    if (m_areSharedColumnsReserved)
        return GetOrCreateOverflowTable();

    return m_primaryOrJoinedTable;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsColumnInUse(DbColumn const& column) const
    {
    return GetColumnMaps()->IsColumnInUsed(column);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const
    {
    GetColumnMaps()->Insert(accessString, *column, true);
    return column;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::Allocate(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString) const
    {
    if (DbColumn* column = GetColumnMaps()->FindP(accessString.c_str()))
        {
        if (IsCompatible(*column, type, param))
            return column;
        }

    if (m_useSharedColumnStrategy)
        return AllocatedSharedColumn(property, param, accessString);

    return AllocateColumn(property, type, param, accessString);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetOrCreateOverflowTable() const
    {
    if (m_overflowTable != nullptr)
        return m_overflowTable;

    if (m_primaryOrJoinedTable->GetLinkNode().GetChildren().empty())
        {
        DbTable* overflowTable = m_classMap.GetDbMap().GetDbSchemaR().CreateOverflowTable(*m_primaryOrJoinedTable);
        const_cast<ClassMap&>(m_classMap).SetOverflowTable(*overflowTable);
        return m_overflowTable = overflowTable;

        }
    else if (m_primaryOrJoinedTable->GetLinkNode().GetChildren().size() == 1)
        {
        DbTable::LinkNode const* overflowTable = m_primaryOrJoinedTable->GetLinkNode().GetChildren()[0];
        if (overflowTable->GetType() == DbTable::Type::Overflow)
            return  m_overflowTable = &overflowTable->GetTableR();
        }

    BeAssert(false && "Cannot create overflow table");
    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ClassMapColumnFactory::ClassMapColumnFactory(ClassMap const& classMap) : m_classMap(classMap), m_primaryOrJoinedTable(&m_classMap.GetJoinedOrPrimaryTable())
    {
    m_useSharedColumnStrategy = (classMap.GetMapStrategy().GetTphInfo().IsValid() && classMap.GetMapStrategy().GetTphInfo().GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes);
    if (m_useSharedColumnStrategy && m_classMap.GetMapStrategy().GetTphInfo().GetMaxSharedColumnsBeforeOverflow().IsValid())
        m_maxSharedColumnCount = m_classMap.GetMapStrategy().GetTphInfo().GetMaxSharedColumnsBeforeOverflow();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ColumnMaps* ClassMapColumnFactory::GetColumnMaps() const
    {
    BeAssert(m_columnResolutionScope != nullptr);
    if (m_columnResolutionScope == nullptr)
        return nullptr;

    return &m_columnResolutionScope->GetColumnMaps();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::ReuseOrCreateSharedColumn() const
    {
    for (DbColumn const* column : GetEffectiveTable()->GetColumns())
        {
        if (column->IsShared() && !GetColumnMaps()->IsColumnInUsed(*column))
            return const_cast<DbColumn*>(column);
        }

    return GetEffectiveTable()->CreateSharedColumn();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsCompatible(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& params) const
    {
    if (DbColumn::IsCompatible(avaliableColumn.GetType(), type))
        {
        if (m_primaryOrJoinedTable->GetType() == DbTable::Type::Existing
            || (avaliableColumn.GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                avaliableColumn.GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                avaliableColumn.GetConstraints().GetCollation() == params.GetCollation()))
            {
            return true;
            }
        }

    return false;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ClassMapColumnFactory::ColumnResolutionScope::ColumnResolutionScope(ClassMap const& classMap) : m_classMap(classMap)
    {
    if (m_classMap.GetColumnFactory().m_columnResolutionScope != nullptr)
        {
        BeAssert(m_classMap.GetColumnFactory().m_columnResolutionScope == nullptr);
        return;
        }

    m_classMap.GetColumnFactory().m_columnResolutionScope = this;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ColumnMaps& ClassMapColumnFactory::ColumnResolutionScope::GetColumnMaps()
    {
    if (!m_init)
        {
        _Fill(m_columnMaps);
        m_init = true;
        }

    return m_columnMaps;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
ClassMapColumnFactory::ColumnResolutionScope::~ColumnResolutionScope()
    {
#if 0
    if (m_init)
        {
        LOG.debugv("<<<<<<<<<<<<<<<<<<<< (%s <<<<<<<<<<<<<<<<<<<<)", m_classMap.GetClass().GetName().c_str());
        for (auto const& key : m_columnMaps.GetEntries())
            {
            DbColumn const* column = key.second;
            Utf8CP accessString = key.first;
            bool isMapped = m_columnMaps.IsNew(accessString);
            LOG.debugv("----->>> %s [%s] map to [%s].[%s]", isMapped ? "NEW" : "OLD", accessString, column->GetTable().GetName().c_str(), column->GetName().c_str());
            }

        LOG.debugv(">>>>>>>>>>>>>>>>>>>> (%s) >>>>>>>>>>>>>>>>>>>>", m_classMap.GetClass().GetName().c_str());
        }
#endif
    m_classMap.GetColumnFactory().m_columnResolutionScope = nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ImportColumnResolutionScope::_Fill(ColumnMaps& columnMaps)
    {
    ColumnMapContext::Query(columnMaps, m_classMap, ColumnMapContext::Filter::InheritedAndLocal);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void UpdateColumnResolutionScope::_Fill(ColumnMaps& columnMaps)
    {
    ColumnMapContext::Query(columnMaps, m_classMap, ColumnMapContext::Filter::Full);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void EndTableRelationshipColumnResolutionScope::_Fill(ColumnMaps& columnMaps)
    {
    for (ClassMapCP classMap : m_relevantMaps)
        {
        ColumnMapContext::Query(columnMaps, *classMap, ColumnMapContext::Filter::Full);
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE