/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
bool ColumnMaps::IsColumnInUsed(DbColumn const& column) const { return n_columns.find(&column) != n_columns.end(); }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
void ColumnMaps::Insert(SingleColumnDataPropertyMap const& propertyMap) { Insert(propertyMap.GetAccessString().c_str(), propertyMap.GetColumn()); }

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
    ECClassCR  contextClass = classMap.GetClass();
    if (!contextClass.HasBaseClasses())
        return SUCCESS;

    TableSpaceSchemaManager const& schemaManager = classMap.GetSchemaManager(); //class hierarchy is always in a single table space
    std::vector<ClassMap const*> baseClasses;
    for (ECClassCP baseClass : contextClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = schemaManager.GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            {
            BeAssert(false && "Expecting class map for primary base class to exist and never null");
            return ERROR;
            }

        if (baseClassMap->GetPrimaryTable() != classMap.GetPrimaryTable())
            continue;

        baseClasses.push_back(baseClassMap);
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
    TableSpaceSchemaManager const& schemaManager = contextClassMap.GetSchemaManager(); //class hierarchy is always in a single table space

    ECDerivedClassesList const* subClasses = schemaManager.GetDerivedClasses(contextClassMap.GetClass());
    if (subClasses == nullptr)
        return ERROR;

    for (ECN::ECClassCP derivedClass : *subClasses)
        {
        if (ClassMap const* derivedClassMap = schemaManager.GetClassMap(*derivedClass))
            {
            DbTable const& primTable = derivedClassMap->GetPrimaryTable();
            if (primTable.GetType() == DbTable::Type::Virtual)
                continue;

            if (primTable != contextClassMap.GetPrimaryTable())
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

        if (base == nullptr)
            {
            ECPropertyIterableCR itor = classMap.GetClass().GetProperties(true);
            const size_t nProperties = std::distance(itor.begin(), itor.end()) + 2;
            const size_t unmapped = nProperties - classMap.GetPropertyMaps().Size();            
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

        if (QueryDerivedColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;
        }

    if (filter == Filter::Full)
        {
        if (QueryLocalColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;

        if (base == nullptr)
            {
            ECPropertyIterableCR itor = classMap.GetClass().GetProperties(true);
            const size_t nProperties = std::distance(itor.begin(), itor.end()) + 2;
            const size_t unmapped = nProperties - classMap.GetPropertyMaps().Size();
            if (unmapped > 0)
                {
                if (QueryInheritedColumnMaps(columnMaps, classMap) != SUCCESS)
                    return ERROR;
                }
            }

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
    return r;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryLocalColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap)
    {
    for (ECPropertyCP property : classMap.GetClass().GetProperties(true))
        {
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

//*****************************************************************************************
//ClassMapColumnFactory
//*****************************************************************************************
// ------------------------------------------------------------------------------------------
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
DbColumn* ClassMapColumnFactory::AllocateColumn(SchemaImportContext& ctx, ECN::ECPropertyCR ecProp, DbColumn::Type colType, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
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

        DbColumn const* existingColumn = GetEffectiveTable(ctx)->FindColumnP(requestedColumnName.c_str());
        if (existingColumn != nullptr && IsColumnInUse(*existingColumn))
            {
            Utf8Char classIdStr[ECN::ECClassId::ID_STRINGBUFFER_LENGTH];
            classId.ToString(classIdStr);
            resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName.c_str());
            }
        else
            resolvedColumName.assign(requestedColumnName);
        };


    DbTable* effectiveTableP = GetEffectiveTable(ctx);
    if (effectiveTableP == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }
    DbTable& effectiveTable = *effectiveTableP;

    DbColumn* existingColumn = effectiveTable.FindColumnP(params.GetColumnName().c_str());
    if (existingColumn != nullptr && !IsColumnInUse(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (effectiveTable.GetType() == DbTable::Type::Existing ||
            (existingColumn->GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                                                      existingColumn->GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                                                      existingColumn->GetConstraints().GetCollation() == params.GetCollation()))
            {
            return existingColumn;
            }

        ctx.Issues().ReportV("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                                                           " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                                                           existingColumn->GetName().c_str(), effectiveTable.GetName().c_str());
        return nullptr;
        }


    BeAssert(!params.GetColumnName().empty() && "Column name must not be null for default strategy");
    bool effectiveNotNullConstraint = params.AddNotNullConstraint();
    if (params.AddNotNullConstraint() && (effectiveTable.HasExclusiveRootECClass() && effectiveTable.GetExclusiveRootECClassId() != m_classMap.GetClass().GetId()))
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
    while (effectiveTable.FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        resolveColumnName(resolvedColumnName, params.GetColumnName(), classId, retryCount);
        }

    DbColumn* newColumn = effectiveTable.AddColumn(resolvedColumnName, colType, DbColumn::Kind::Default, PersistenceType::Physical);
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
DbColumn* ClassMapColumnFactory::AllocatedSharedColumn(SchemaImportContext& ctx, ECN::ECPropertyCR prop, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    //Defining a col name for a shared column is a DB thing and DB CAs are taken strictly.
    if (params.IsColumnNameFromPropertyMapCA())
        {
        ctx.Issues().ReportV("Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a value for 'ColumnName'. "
                                                           "'ColumnName' must not be specified for this ECProperty because it is mapped to a column shared with other ECProperties.",
                                                           prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //Defining a collation which is not doable is an error because this is a DB thing and DB CAs are taken strictly.
    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        ctx.Issues().ReportV("Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a Collation constraint "
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

    return RegisterColumnMap(accessString, ReuseOrCreateSharedColumn(ctx));
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
        const std::vector<DbColumn const*> sharedColumns = m_primaryOrJoinedTable->FindAll(DbColumn::Kind::SharedData);
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
DbColumn* ClassMapColumnFactory::RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const
    {
    GetColumnMaps()->Insert(accessString, *column, true);
    return column;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       10 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::HandleOverflowColumn(DbColumn* column) const
    {
    if (column->IsShared() && column->GetTable().GetType() == DbTable::Type::Overflow && !m_overflowTable)
        {
        m_overflowTable = &column->GetTableR();
        if (!m_classMap.GetOverflowTable())
            {
            const_cast<ClassMap&>(m_classMap).SetOverflowTable(*m_overflowTable);
            }
        else
            {
            if (m_overflowTable != m_classMap.GetOverflowTable())
                {
                BeAssert(false && "This would be a serious error");
                return nullptr;
                }
            }
        }

    return column;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::Allocate(SchemaImportContext& ctx, ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bool forcePhysicalColum) const
    {
    if (DbColumn* column = GetColumnMaps()->FindP(accessString.c_str()))
        {
        if (IsCompatible(*column, type, param))
            return HandleOverflowColumn(column);
        }

    if (m_useSharedColumnStrategy && !forcePhysicalColum)
        return AllocatedSharedColumn(ctx, property, param, accessString);

    return AllocateColumn(ctx, property, type, param, accessString);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetEffectiveTable(SchemaImportContext& ctx) const
    {
    if (m_areSharedColumnsReserved)
        return GetOrCreateOverflowTable(ctx);

    return m_primaryOrJoinedTable;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsColumnInUse(DbColumn const& column) const { return GetColumnMaps()->IsColumnInUsed(column); }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2017
//-----------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetOrCreateOverflowTable(SchemaImportContext& ctx) const
    {
    if (m_overflowTable != nullptr)
        return m_overflowTable;

    m_overflowTable  = m_classMap.GetOverflowTable();
    if (m_overflowTable != nullptr)
        return m_overflowTable;

    if (m_primaryOrJoinedTable->GetLinkNode().GetChildren().empty())
        {
        m_overflowTable = DbMappingManager::Tables::CreateOverflowTable(ctx, *m_primaryOrJoinedTable);
        }
    else if (m_primaryOrJoinedTable->GetLinkNode().GetChildren().size() == 1)
        {
        DbTable::LinkNode const* overflowTableNode = m_primaryOrJoinedTable->GetLinkNode().GetChildren()[0];
        if (overflowTableNode->GetTable().GetType() == DbTable::Type::Overflow)
            m_overflowTable = &overflowTableNode->GetTableR();
        }
    
    if (m_overflowTable == nullptr)
        {
        BeAssert(false && "Cannot create overflow table");
        return nullptr;
        }

    if (const_cast<ClassMap&>(m_classMap).SetOverflowTable(*m_overflowTable) != SUCCESS)
        {
        BeAssert(false && "SetOverflowTable failed");
        }

    return m_overflowTable;
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
DbColumn* ClassMapColumnFactory::ReuseOrCreateSharedColumn(SchemaImportContext& ctx) const
    {
    for (DbColumn const* column : GetEffectiveTable(ctx)->GetColumns())
        {
        if (column->IsShared() && !GetColumnMaps()->IsColumnInUsed(*column))
            return const_cast<DbColumn*>(column);
        }

    return GetEffectiveTable(ctx)->AddSharedColumn();
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


//***************************************************************************************
// ClassMapColumnFactory::ColumnResolutionScope
//***************************************************************************************
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

END_BENTLEY_SQLITE_EC_NAMESPACE
