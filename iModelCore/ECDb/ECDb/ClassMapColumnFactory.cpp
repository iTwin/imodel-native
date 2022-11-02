/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
bool ColumnMaps::IsColumnInUse(DbColumn const& column) const { return m_columns.find(&column) != m_columns.end(); }
//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
void ColumnMaps::Insert(SingleColumnDataPropertyMap const& propertyMap) { Insert(propertyMap.GetAccessString(), propertyMap.GetColumn()); }


//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
void ColumnMaps::Insert(Utf8StringCR accessString, DbColumn const& column, bool newlyMappedColumn)
    {
    m_maps.insert(make_bpair(accessString, &column));
    m_columns.insert(&column);
    if (newlyMappedColumn)
        m_newMappedColumns.insert(accessString);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
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

BentleyStatus ColumnMapContext::QueryDerivedColumnMapsForClass(ECClassCR ecClass, ColumnMaps& columnMaps, ClassMap const& contextClassMap)
    {
    TableSpaceSchemaManager const& schemaManager = contextClassMap.GetSchemaManager(); //class hierarchy is always in a single table space
    if (ClassMap const* derivedClassMap = schemaManager.GetClassMap(ecClass))
        {
        DbTable const& primTable = derivedClassMap->GetPrimaryTable();
        if (primTable.GetType() == DbTable::Type::Virtual)
            return SUCCESS;

        if (primTable != contextClassMap.GetPrimaryTable())
            return SUCCESS;

        if (Query(columnMaps, *derivedClassMap, Filter::DerivedAndLocal, &contextClassMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::QueryDerivedColumnMaps(ColumnMaps& columnMaps, ClassMap const& contextClassMap)
    {
    TableSpaceSchemaManager const& schemaManager = contextClassMap.GetSchemaManager(); //class hierarchy is always in a single table space

    ECDerivedClassesList const* subClasses = schemaManager.GetDerivedClasses(contextClassMap.GetClass());
    if (subClasses == nullptr)
        return ERROR;

    for (ECN::ECClassCP derivedClass : *subClasses)
        {
        if (QueryDerivedColumnMapsForClass(*derivedClass, columnMaps, contextClassMap) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::Query(ColumnMaps& columnMaps, ClassMap const& classMap, Filter filter, ClassMap const* base)
    {
    if (filter == Filter::InheritedAndLocal)
        {
        if (QueryLocalColumnMaps(columnMaps, classMap) != SUCCESS)
            return ERROR;

        if (base == nullptr)
            {
            const size_t nProperties = classMap.GetClass().GetPropertyCount() + 2;
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
            const size_t nProperties = classMap.GetClass().GetPropertyCount() + 2;
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
//@bsimethod
//-----------------------------------------------------------------------------------------
BentleyStatus ColumnMapContext::Query(ColumnMaps& columnMaps, ClassMap const& classMap, Filter filter)
    {
    return Query(columnMaps, classMap, filter, nullptr);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//-----------------------------------------------------------------------------------------
ClassMapColumnFactory::ClassMapColumnFactory(ClassMap const& classMap) : m_classMap(classMap), m_primaryOrJoinedTable(&m_classMap.GetJoinedOrPrimaryTable())
    {
    m_useSharedColumnStrategy = (classMap.GetMapStrategy().GetTphInfo().IsValid() && classMap.GetMapStrategy().GetTphInfo().GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes);
    if (m_useSharedColumnStrategy && m_classMap.GetMapStrategy().GetTphInfo().GetMaxSharedColumnsBeforeOverflow().IsValid())
        m_maxSharedColumnCount = m_classMap.GetMapStrategy().GetTphInfo().GetMaxSharedColumnsBeforeOverflow();
    }

//------------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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

        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Column %s in table %s is used by multiple property maps where property name and data type matches,"
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
//@bsimethod
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::AllocateSharedColumn(SchemaImportContext& ctx, ECN::ECPropertyCR prop, DbColumn::CreateParams const& params, Utf8StringCR accessString) const
    {
    //Defining a col name for a shared column is a DB thing and DB CAs are taken strictly.
    if (params.IsColumnNameFromPropertyMapCA())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a value for 'ColumnName'. "
                                                           "'ColumnName' must not be specified for this ECProperty because it is mapped to a column shared with other ECProperties.",
                                                           prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //Defining a collation which is not doable is an error because this is a DB thing and DB CAs are taken strictly.
    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a Collation constraint "
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

    auto* column = ReuseOrCreateSharedColumn(ctx);
    return RegisterColumnMap(accessString, column);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
void ClassMapColumnFactory::EvaluateIfPropertyGoesToOverflow(Utf8StringCR propertyName, SchemaImportContext& ctx) const
    {
    BeAssert(!propertyName.empty());
    ECN::ECPropertyCP property = m_classMap.GetClass().GetPropertyP(propertyName);
    if (property == nullptr)
            {
            BeAssert(false && "Property must exist in associated class map");
            return;
            }

    const uint32_t columnsRequired = MaxColumnsRequiredToPersistProperty(*property);
    EvaluateIfPropertyGoesToOverflow(columnsRequired, ctx);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
void ClassMapColumnFactory::EvaluateIfPropertyGoesToOverflow(uint32_t columnsRequired, SchemaImportContext& ctx) const
    {
    if (m_putCurrentPropertyToOverflow)
        {
        BeAssert(false);
        return;
        }

    if (!m_useSharedColumnStrategy)
        {
        BeAssert(false && "Shared Column must be enabled for this allocation to work");
        return;
        }

      const uint32_t maxColumnInBaseTable = 63;
      if(columnsRequired > maxColumnInBaseTable)
        {
        m_putCurrentPropertyToOverflow = true; //in this case we can directly choose overflow
        return;
        }

      const std::vector<DbColumn const*> physicalColumns = m_primaryOrJoinedTable->FindAll(PersistenceType::Physical);
      const uint32_t nAvaliablePhysicalColumns = maxColumnInBaseTable - (uint32_t) physicalColumns.size();

      const std::vector<DbColumn const*> sharedColumns = m_primaryOrJoinedTable->FindAll(DbColumn::Kind::SharedData);
      const uint32_t nSharedColumns = (uint32_t) sharedColumns.size();

      //Determine how many shared columns can be created
      uint32_t sharedColumnThatCanBeCreated = 0;
      if (m_maxSharedColumnCount.IsNull())
          {
          sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns;
          }
      else
          {
          if (nSharedColumns > m_maxSharedColumnCount.Value())
              {
              BeAssert(false && "SharedColumnCount bypassed the limit set in CA");
              return;
              }

          sharedColumnThatCanBeCreated = m_maxSharedColumnCount.Value() - (uint32_t) sharedColumns.size();
          if (sharedColumnThatCanBeCreated > nAvaliablePhysicalColumns)
              sharedColumnThatCanBeCreated = nAvaliablePhysicalColumns; //restrict available shared columns to available physical columns
          }

    if (sharedColumnThatCanBeCreated >= columnsRequired)
        return; //we can just exit here, we definitely never have to go to overflow in this case

    uint32_t requiredRemainingColumns = columnsRequired - sharedColumnThatCanBeCreated;
    if (requiredRemainingColumns > nSharedColumns)
        { //no need to check, we know there won't be enough columns
        m_putCurrentPropertyToOverflow = true;
        return;
        }

    for (DbColumn const* sharedColumn : sharedColumns)
        {
        if (!IsColumnInUse(*sharedColumn) && !IsColumnUsedByAnyDerivedClass(*sharedColumn, ctx))
            requiredRemainingColumns--; //column can be reused

        if(requiredRemainingColumns <= 0)
            return;
        }
    
    m_putCurrentPropertyToOverflow = true; // TODO: this flag is mutable and the current method is marked as const. Use return value instead?
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const
    {
    GetColumnMaps()->Insert(accessString, *column, true);
    return column;
    }

//------------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::Allocate(SchemaImportContext& ctx, ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bool forcePhysicalColum) const
    {
    if (DbColumn* column = GetColumnMaps()->FindP(accessString.c_str()))
        {
        if (IsCompatible(*column, type, param))
            return HandleOverflowColumn(column);
        }

    if (m_useSharedColumnStrategy && !forcePhysicalColum)
        return AllocateSharedColumn(ctx, property, param, accessString);

    return AllocateColumn(ctx, property, type, param, accessString);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
DbTable* ClassMapColumnFactory::GetEffectiveTable(SchemaImportContext& ctx) const
    {
    if (m_putCurrentPropertyToOverflow)
        return GetOrCreateOverflowTable(ctx);

    return m_primaryOrJoinedTable;
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
bool ClassMapColumnFactory::IsColumnInUse(DbColumn const& column) const { return GetColumnMaps()->IsColumnInUse(column); }
//------------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//-----------------------------------------------------------------------------------------
ColumnMaps* ClassMapColumnFactory::GetColumnMaps() const
    {
    BeAssert(m_columnResolutionScope != nullptr);
    if (m_columnResolutionScope == nullptr)
        return nullptr;

    return &m_columnResolutionScope->GetColumnMaps();
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
DbColumn* ClassMapColumnFactory::ReuseOrCreateSharedColumn(SchemaImportContext& ctx) const
    {
    for (DbColumn const* column : GetEffectiveTable(ctx)->GetColumns())
        {
        if (column->IsShared() && !GetColumnMaps()->IsColumnInUse(*column))
            {
            if(!IsColumnUsedByAnyDerivedClass(*column, ctx))
                return const_cast<DbColumn*>(column);
            }
        }

    return GetEffectiveTable(ctx)->AddSharedColumn();
    }

//------------------------------------------------------------------------------------------
//@bsimethod
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

bool ClassMapColumnFactory::IsColumnUsedByAnyDerivedClass(DbColumn const& column, SchemaImportContext& ctx) const
    {
    // Additional check. When adding classes or properties to an existing schema, we can miss mapped columns and assign them multiple times
    // this ensures there is no other class down the hierarchy that occupies a column.
    if (!column.HasId()) // Not-yet-persisted columns cannot be used by subclasses
        return false;
    
    ECClassCR ecClass = m_classMap.GetClass();
    if (!ecClass.HasId())
        { // Not-yet-persisted class
        BeAssert(false);
        LOG.errorv("Class %s does not have an id, when trying to check if a column is used by one of its base classes.", ecClass.GetFullName());
        return false;     // it should not be possible at this point to have no ID on a class.
        }

    BeInt64Id columnId = column.GetId();
    BeInt64Id classId = ecClass.GetId();
    ECDbCR ecdb = ctx.GetECDb();
    
    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(
        "SELECT EXISTS (SELECT 1 FROM main.ec_PropertyMap pm "
        "JOIN main.ec_cache_ClassHierarchy ch ON ch.ClassId = pm.ClassId "
        "WHERE pm.ColumnId = ? AND ch.BaseClassId = ? limit 1)");
    BeAssert(stmt.IsValid());
    
    stmt->BindId(1, columnId);
    stmt->BindId(2, classId);

    if (stmt->Step() != BE_SQLITE_ROW)
        {
        LOG.warningv("Query to evaluate if any subclass of class %s is using column with id %d failed.",
                    classId.GetValue(), columnId.GetValue(), ecClass.GetFullName());
        return false;
        }

    bool result = stmt->GetValueBoolean(0);
    return result;
    }

//***************************************************************************************
// ClassMapColumnFactory::ColumnResolutionScope
//***************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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
//@bsimethod
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
//@bsimethod
//-----------------------------------------------------------------------------------------
void ImportColumnResolutionScope::_Fill(ColumnMaps& columnMaps)
    {
    ColumnMapContext::Query(columnMaps, m_classMap, ColumnMapContext::Filter::InheritedAndLocal);
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//-----------------------------------------------------------------------------------------
void UpdateColumnResolutionScope::_Fill(ColumnMaps& columnMaps)
    {
    ColumnMapContext::Query(columnMaps, m_classMap, ColumnMapContext::Filter::Full);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
