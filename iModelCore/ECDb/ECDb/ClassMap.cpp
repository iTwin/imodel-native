/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define CURRENTIMESTAMP_SQLEXP "julianday('now')"

//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMap::ClassMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, Type type, ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy)
    : m_type(type), m_ecdb(ecdb), m_schemaManager(manager), m_ecClass(ecClass), m_mapStrategyExtInfo(mapStrategy), m_propertyMaps(*this), m_state(ObjectState::New)
    {
    if (m_mapStrategyExtInfo.IsTablePerHierarchy())
        m_tphHelper = std::make_unique<TablePerHierarchyHelper>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbTable& ClassMap::GetPrimaryTable() const
    {
    if (GetType() == Type::RelationshipEndTable)
        return *m_tables.front();

    for (DbTable* table : GetTables())
        {
        DbTable::Type tableType = table->GetType();
        if (tableType == DbTable::Type::Primary || tableType == DbTable::Type::Existing || tableType == DbTable::Type::Virtual)
            return *table;
        }

    BeAssert(false);
    DbTable* nulltable = nullptr;
    return *nulltable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbTable& ClassMap::GetJoinedOrPrimaryTable() const
    {
    DbTable* joinedTable = nullptr;
    DbTable* primaryTable = nullptr;
    for (DbTable* table : m_tables)
        {
        DbTable::Type type = table->GetType();

        if (type == DbTable::Type::Joined)
            joinedTable = table;
        else if (type == DbTable::Type::Primary || type == DbTable::Type::Existing || type == DbTable::Type::Virtual)
            primaryTable = table;

        if (joinedTable != nullptr)
            return *joinedTable;
        }

    BeAssert(primaryTable != nullptr);
    return *primaryTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbTable* ClassMap::GetOverflowTable() const
    {
    for (DbTable* table : GetTables())
        {
        if (table->GetType() == DbTable::Type::Overflow)
            return table;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::_Map(ClassMappingContext& ctx)
    {
    ClassMappingStatus stat = DoMapPart1(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    return DoMapPart2(ctx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::DoMapPart1(ClassMappingContext& ctx)
    {
    if (SUCCESS != DbMappingManager::Tables::MapToTable(ctx.GetImportCtx(), *this, ctx.GetClassMappingInfo()))
        return ClassMappingStatus::Error;

    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::DoMapPart2(ClassMappingContext& ctx)
    {
    ImportColumnResolutionScope columnResolutionScope(*this);
    ClassMappingStatus stat = MapProperties(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    PrimitiveECPropertyCP currentTimeStampProp = ctx.GetClassMappingInfo().GetClassHasCurrentTimeStampProperty();
    if (currentTimeStampProp != nullptr)
        {
        if (SUCCESS != CreateCurrentTimeStampTrigger(*currentTimeStampProp))
            return ClassMappingStatus::Error;
        }

    //Add cascade delete for joinedTable;
    bool isJoinedTable = m_mapStrategyExtInfo.GetTphInfo().IsValid() && m_mapStrategyExtInfo.GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::JoinedTable;
    if (!isJoinedTable)
        return ClassMappingStatus::Success;

    ClassMap const* tphBaseClassMap = ctx.GetClassMappingInfo().GetTphBaseClassMap();
    if (tphBaseClassMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    DbTable const& baseClassMapJoinedTable = tphBaseClassMap->GetJoinedOrPrimaryTable();
    if (&baseClassMapJoinedTable == &GetJoinedOrPrimaryTable())
        return ClassMappingStatus::Success;

    DbColumn const* primaryKeyColumn = baseClassMapJoinedTable.FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* foreignKeyColumn = GetJoinedOrPrimaryTable().FindFirst(DbColumn::Kind::ECInstanceId);
    PRECONDITION(primaryKeyColumn != nullptr, ClassMappingStatus::Error);
    PRECONDITION(foreignKeyColumn != nullptr, ClassMappingStatus::Error);
    bool createFKConstraint = true;
    for (DbConstraint const* constraint : GetJoinedOrPrimaryTable().GetConstraints())
        {
        if (constraint->GetType() != DbConstraint::Type::ForeignKey)
            continue;

        ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
        if (&fk->GetReferencedTable() == &baseClassMapJoinedTable &&
            fk->GetFkColumns().front() == foreignKeyColumn && fk->GetReferencedTableColumns().front() == primaryKeyColumn)
            {
            createFKConstraint = false;
            break;
            }
        }

    if (createFKConstraint)
        {
        if (GetJoinedOrPrimaryTable().AddForeignKeyConstraint(*foreignKeyColumn, *primaryKeyColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified) == nullptr)
            return ClassMappingStatus::Error;
        }

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::CreateCurrentTimeStampTrigger(PrimitiveECPropertyCR currentTimeStampProp)
    {
    if (currentTimeStampProp.GetType() != PRIMITIVETYPE_DateTime)
        {
        BeAssert(false);
        return ERROR;
        }

    PropertyMap const* propMap = GetPropertyMaps().Find(currentTimeStampProp.GetName().c_str());
    if (propMap == nullptr)
        return SUCCESS;

    DbColumn& currentTimeStampColumn = const_cast<DbColumn&>(propMap->GetAs<PrimitivePropertyMap>().GetColumn());
    if (currentTimeStampColumn.IsShared())
        {
        LOG.warningv("ECProperty '%s' in ECClass '%s' has the ClassHasCurrentTimeStampProperty custom attribute but is mapped to a shared column. "
                   "ECDb therefore does not create the current timestamp trigger for this property.",
                           currentTimeStampProp.GetName().c_str(), currentTimeStampProp.GetClass().GetFullName());

        return SUCCESS;
        }

    BeAssert(currentTimeStampColumn.GetType() == DbColumn::Type::TimeStamp);
    currentTimeStampColumn.GetConstraintsR().SetDefaultValueExpression(CURRENTIMESTAMP_SQLEXP);
    currentTimeStampColumn.GetConstraintsR().SetNotNullConstraint();

    ECInstanceIdPropertyMap const* idPropMap = GetECInstanceIdPropertyMap();
    if (idPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable& table = currentTimeStampColumn.GetTableR();
    Utf8CP tableName = table.GetName().c_str();
    Utf8CP instanceIdColName = idPropMap->FindDataPropertyMap(tableName)->GetColumn().GetName().c_str();
    Utf8CP currentTimeStampColName = currentTimeStampColumn.GetName().c_str();

    Utf8String triggerName;
    //triggerName.Sprintf("%s_%s_SetCurrentTimeStamp", tableName, currentTimeStampColName);
    triggerName.Sprintf("%s_CurrentTimeStamp", tableName);
    Utf8String body;
    body.Sprintf("BEGIN UPDATE [%s] SET [%s]=" CURRENTIMESTAMP_SQLEXP " WHERE [%s]=new.[%s]; END", tableName, currentTimeStampColName, instanceIdColName, instanceIdColName);

    Utf8String whenCondition;
    whenCondition.Sprintf("old.[%s]=new.[%s] AND old.[%s]!=" CURRENTIMESTAMP_SQLEXP, currentTimeStampColName, currentTimeStampColName, currentTimeStampColName);

    return table.AddTrigger(triggerName, DbTrigger::Type::After, whenCondition.c_str(), body.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::AddOrUpdateTableList(DataPropertyMap const& propertyThatIsNotYetAdded)
    {
    if (propertyThatIsNotYetAdded.GetType() == PropertyMap::Type::Navigation)
        {
        NavigationPropertyMap const& navPropertyMap = propertyThatIsNotYetAdded.GetAs<NavigationPropertyMap>();
        if (!navPropertyMap.IsComplete())
            return SUCCESS;
        }

    DbTable& propertyTable = const_cast< DbTable&>(propertyThatIsNotYetAdded.GetTable());
    if (std::find(m_tables.begin(), m_tables.end(), &propertyTable) == m_tables.end())
        {
        if (propertyTable.GetType() == DbTable::Type::Overflow)
            return SetOverflowTable(propertyTable);

        AddTable(propertyTable);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMappingStatus ClassMap::MapProperties(ClassMappingContext& ctx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    DbMappingManager::Classes::PropertyMapInheritanceMode inheritanceMode = DbMappingManager::Classes::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetSchemaManager().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                {
                BeAssert(false);
                return ClassMappingStatus::Error;
                }

            if (baseClassMap->GetMapStrategy().IsTablePerHierarchy())
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }

    std::vector<ECPropertyCP> propertiesToMap;
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if ((&property->GetClass() == &m_ecClass && !property->GetBaseProperty()) ||
            inheritanceMode == DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
            {
            //Property map that should not be inherited -> must be mapped
            propertiesToMap.push_back(property);
            continue;
            }

        //look for the base class' property map as property is inherited
        DataPropertyMap const* baseClassPropMap = nullptr;
        for (ClassMap const* baseClassMap : tphBaseClassMaps)
            {
            PropertyMap const* baseClassPropMapRaw = baseClassMap->GetPropertyMaps().Find(property->GetName().c_str());
            if (baseClassPropMapRaw != nullptr && baseClassPropMapRaw->IsData())
                {
                baseClassPropMap = &baseClassPropMapRaw->GetAs<DataPropertyMap>();
                break;
                }
            }

        if (baseClassPropMap == nullptr)
            {
            //Property is inherited, but not from a TPH base class, so we have to map the property from scratch
            propertiesToMap.push_back(property);
            continue;
            }

        if (baseClassPropMap->IsData())
            {
            if (AddOrUpdateTableList(baseClassPropMap->GetAs<DataPropertyMap>()) != SUCCESS)
                return ClassMappingStatus::Error;
            }

        RefCountedPtr<DataPropertyMap> propertyMap = PropertyMapCopier::CreateCopy(*baseClassPropMap, *this);
        if (propertyMap == nullptr || SUCCESS != m_propertyMaps.Insert(propertyMap))
            return ClassMappingStatus::Error;

        if (propertyMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus stat = DbMappingManager::Classes::MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (ClassMappingStatus::Success != stat)
                    return stat;
                }
            }
        }

    for (ECPropertyCP property : propertiesToMap)
        {
        PropertyMap* propMap = DbMappingManager::Classes::MapProperty(ctx.GetImportCtx(), *this, *property);
        if (propMap == nullptr)
            return ClassMappingStatus::Error;

        if (property->GetIsNavigation())
            {
            //WIP_CLEANUP code redundant to a few lines above.
            NavigationPropertyMap const& navPropertyMap = propMap->GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                {
                ClassMappingStatus stat = DbMappingManager::Classes::MapNavigationProperty(ctx.GetImportCtx(), const_cast<NavigationPropertyMap&>(navPropertyMap));
                if (ClassMappingStatus::Success != stat)
                    return stat;
                }
            }
        }

    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Save(SchemaImportContext& importCtx, DbMapSaveContext& ctx)
    {
    if (ctx.IsAlreadySaved(*this) || !importCtx.ClassMapNeedsSaving(m_ecClass.GetId()))
        return SUCCESS;

    ctx.BeginSaving(*this);
    if (GetClass().HasBaseClasses())
        {
        for (ECClassCP baseClass : GetClass().GetBaseClasses())
            {
            ClassMap* baseClassMap = const_cast<ClassMap*>(GetSchemaManager().GetClassMap(*baseClass));
            if (baseClassMap == nullptr)
                {
                BeAssert(false && "Failed to find baseClass map");
                return ERROR;
                }

            if (SUCCESS != baseClassMap->Save(importCtx, ctx))
                return ERROR;
            }
        }

    if (ctx.InsertClassMap(GetClass().GetId(), GetMapStrategy()) != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    DbClassMapSaveContext classMapSaveContext(ctx);
    SavePropertyMapVisitor saveVisitor(classMapSaveContext);
    for (PropertyMap const* propertyMap : GetPropertyMaps())
        {
        BeAssert(GetClass().GetId() == propertyMap->GetClassMap().GetClass().GetId());
        if (SUCCESS != propertyMap->AcceptVisitor(saveVisitor))
            return ERROR;
        }

    ctx.EndSaving(*this);
    m_state = ObjectState::Persisted;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbLoadCtx)
    {
    m_state = ObjectState::Persisted;
    std::set<DbTable*> primaryTables;
    std::set<DbTable*> joinedTables;
    std::set<DbTable*> overflowTables;
    if (!dbLoadCtx.HasMappedProperties())
        {
        SetTable(*const_cast<DbTable*>(GetSchemaManager().GetDbSchema().GetNullTable()));
        return SUCCESS;
        }

    for (auto const& propMapping : dbLoadCtx.GetPropertyMaps())
        {
        std::vector<DbColumn const*> const& columns = propMapping.second;
        for (DbColumn const* column : columns)
            {
            if (column->GetTable().GetType() == DbTable::Type::Joined)
                joinedTables.insert(&column->GetTableR());
            else if (column->GetTable().GetType() == DbTable::Type::Overflow)
                overflowTables.insert(&column->GetTableR());
            else if (column->GetKind() != DbColumn::Kind::ECClassId)
                primaryTables.insert(&column->GetTableR());
            }
        }
    //Orderly add the tables
    for (DbTable* table : primaryTables)
        AddTable(*table);

    for (DbTable* table : joinedTables)
        AddTable(*table);

    for (DbTable* table : overflowTables)
        AddTable(*table);

    BeAssert(!GetTables().empty());

    if (GetECInstanceIdPropertyMap() != nullptr)
        return ERROR;

    std::vector<DbColumn const*> const* mapColumnsList = dbLoadCtx.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_ECInstanceId));
    if (mapColumnsList == nullptr)
        return ERROR;

    //Load ECInstanceId================================================
    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, *mapColumnsList);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
        return ERROR;

    mapColumnsList = dbLoadCtx.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_ECClassId));
    if (mapColumnsList == nullptr)
        return ERROR;

    //Load ECClassId   ================================================
    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, *mapColumnsList);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false && "Failed to create property map");
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecClassIdPropertyMap, 1) != SUCCESS)
        return ERROR;

    if (SUCCESS != LoadPropertyMaps(ctx, dbLoadCtx))
        return ERROR;

    /*
     * POC-No-Remap
     * Apply per-class column overrides from ec_PropertyMap_Overrides when a property is promoted to a base class without physically moving data.
     * The canonical ec_PropertyMap now points to a fresh base-class column, but instances of this class still have their data at the original column.
     * Remapping each property map to the override column makes the in-memory ClassMap reflect the actual storage layout, so HasDivergentColumnAssignments can detect the divergence at query time.
     */
    if (m_ecdb.TableExists(TABLE_PropertyMap_Overrides))
        {
        const ECClassId classId = GetSchemaManager().GetClassId(m_ecClass);
        CachedStatementPtr overrideStmt = m_ecdb.GetCachedStatement(
            "SELECT o.AccessString, col.Name, tab.Name "
            "FROM " TABLE_PropertyMap_Overrides " o "
            "JOIN ec_Column col ON col.Id = o.ColumnId "
            "JOIN ec_Table  tab ON tab.Id = col.TableId "
            "WHERE o.ClassId = ?");

        if (overrideStmt != nullptr && overrideStmt->BindId(1, classId) == BE_SQLITE_OK)
            {
            std::map<Utf8String, DbColumn*, CompareIUtf8Ascii> overrides;
            while (overrideStmt->Step() == BE_SQLITE_ROW)
                {
                Utf8CP accessString = overrideStmt->GetValueText(0);
                Utf8CP colName      = overrideStmt->GetValueText(1);
                Utf8CP tableName    = overrideStmt->GetValueText(2);
                if (const DbTable* table = GetSchemaManager().GetDbSchema().FindTable(tableName))
                    {
                    if (DbColumn* col = table->FindColumnP(colName))
                        overrides.emplace(Utf8String(accessString), col);
                    }
                }

            if (!overrides.empty())
                {
                SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
                m_propertyMaps.AcceptVisitor(visitor);
                for (const PropertyMap* pm : visitor.Results())
                    {
                    auto it = overrides.find(pm->GetAccessString().c_str());
                    if (it != overrides.end())
                        pm->GetAs<SingleColumnDataPropertyMap>().Remap(*it->second);
                    }
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::LoadPropertyMaps(ClassMapLoadContext& ctx, DbClassMapLoadContext const& dbCtx)
    {
    bvector<ClassMap const*> tphBaseClassMaps;
    DbMappingManager::Classes::PropertyMapInheritanceMode inheritanceMode = DbMappingManager::Classes::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited)
        {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetSchemaManager().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                return ERROR;

            if (baseClassMap->GetMapStrategy().IsTablePerHierarchy())
                tphBaseClassMaps.push_back(baseClassMap);
            }
        }

    /*
     * POC-No-Remap
     * First pass: Load all properties that the class has into m_propertyMaps while skipping the inherited properties.
     */
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (dbCtx.IsPropertyIgnored(property->GetName()) && m_ecClass.GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            continue;

        if (&property->GetClass() != &m_ecClass)
            continue; // skip inherited properties for now

        if (DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property) == nullptr)
            m_failedToLoadProperties.push_back(property);
        }

    /*
     * POC-No-Remap
     * Second pass: Load every INHERITED property from ec_PropertyMap for this class which already have a row in the table.
     * Use case: SchemaRemapTestFixture.RevitStoryScenario
     * When a class's base changes (e.g. Story: CompositeElement -> FacilityPart), the derived concrete class (Level) inherits a new property
     *  (Description) whose canonical shared column (ps2) was already assigned in an earlier schema version, to a mixin property (RevitId) of the same concrete class.
     * GetProperties(true) may return Description before RevitId, so the Pass 3 conflict check would not yet see RevitId -> ps2 in m_propertyMaps, allowing Description to be cloned onto ps2 and creating a conflict
     * By pre-loading all canonically-mapped inherited properties in Pass 2, we ensure that the complete set of occupied columns is visible to the conflict check in Pass 3.
     */
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (dbCtx.IsPropertyIgnored(property->GetName()) && m_ecClass.GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            continue;

        if (&property->GetClass() == &m_ecClass)
            continue; // own property which is already handled in Pass 1

        DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property);
        }

    /*
     * POC-No-Remap
     * Third pass: Process inherited properties not yet in m_propertyMaps.
     * At this point m_propertyMaps contains ALL own-class columns (Pass 1) plus ALL canonically-mapped inherited columns (Pass 2), giving the conflict check a complete view of occupied columns.
     */
    for (ECPropertyCP property : m_ecClass.GetProperties(true))
        {
        if (dbCtx.IsPropertyIgnored(property->GetName()) && m_ecClass.GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            continue;

        if (&property->GetClass() == &m_ecClass)
            continue; // already handled in Pass 1

        // Skip properties already inserted into m_propertyMaps during Pass 2.
        if (m_propertyMaps.Find(property->GetName().c_str()) != nullptr)
            continue;

        if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::Clone)
            {
            if (DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property) == nullptr)
                m_failedToLoadProperties.push_back(property);
            continue;
            }

        DataPropertyMap const* tphBaseClassPropMap = nullptr;
        for (ClassMap const* baseClassMap : tphBaseClassMaps)
            {
            PropertyMap const* propMap = baseClassMap->GetPropertyMaps().Find(property->GetName().c_str());
            if (propMap != nullptr && propMap->IsData())
                {
                tphBaseClassPropMap = &propMap->GetAs<DataPropertyMap>();
                break;
                }
            }

        if (tphBaseClassPropMap == nullptr)
            {
            if (DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property) == nullptr)
                m_failedToLoadProperties.push_back(property);
            continue;
            }

        /*
         * POC-No-Remap
         * Unlikely but Defensive early-out: if this property somehow has an ec_PropertyMap entry that Pass 2 missed, load it now and skip the conflict check. 
         */
        if (DbMappingManager::Classes::LoadPropertyMap(dbCtx, *this, *property) != nullptr)
            continue;

        /*
         * POC-No-Remap
         * Column Conflict check
         * 
         * Before cloning the inherited property's column assignment, check whether any column it uses is already occupied by any property already in m_propertyMaps.
         * Own properties from Pass 1 OR canonically-mapped inherited properties from Pass 2 such as mixin properties.
         *
         * Encountered in tests:
         * 1. SchemaRemapTestFixture.PutSiblingsIntoHierarchy / SchemaRemapTestFixture.InjectBaseClassInBaseSchema: 
         *     The class's direct base changes to a former sibling that already owns a property mapped to the same shared column as an OWN property of this class.
         *
         * 2. SchemaRemapTestFixture.RevitStoryScenario / SchemaRemapTestFixture.RevitStoryScenarioWithSiblingAndMixins:
         *     A mixin property (e.g. RevitId) was mapped in a previous schema version to a shared column (ps2) that is now also the canonical column for a newly-inherited base-class property (e.g. Description).
         *     Because RevitId and Description belong to different class branches that were unrelated in the previous hierarchy, their ps2 assignments were both valid then; but after the base-class change they now collide.
         *
         * In both cases CleanModifiedMappings would normally resolve the conflict using remapping.
         * We now defer resolution to ClassMap::Update (which allocates a fresh column), which is safe because the conflicting property retains its original ec_PropertyMap entry, while the inherited property gets a new canonical column.
         */
        bool inheritedColumnConflicts = false;
        {
        SearchPropertyMapVisitor baseColumnVisitor(PropertyMap::Type::SingleColumnData);
        tphBaseClassPropMap->AcceptVisitor(baseColumnVisitor);
        for (const PropertyMap* basePm : baseColumnVisitor.Results())
            {
            const DbColumn& baseCol = basePm->GetAs<SingleColumnDataPropertyMap>().GetColumn();
            SearchPropertyMapVisitor ownVisitor(PropertyMap::Type::SingleColumnData);
            m_propertyMaps.AcceptVisitor(ownVisitor);
            for (const PropertyMap* ownPm : ownVisitor.Results())
                {
                if (&ownPm->GetAs<SingleColumnDataPropertyMap>().GetColumn() == &baseCol)
                    {
                    inheritedColumnConflicts = true;
                    break;
                    }
                }
            if (inheritedColumnConflicts)
                break;
            }
        }

        if (inheritedColumnConflicts)
            {
            m_failedToLoadProperties.push_back(property);
            continue;
            }

        RefCountedPtr<PropertyMap> propMap = PropertyMapCopier::CreateCopy(*tphBaseClassPropMap, *this);
        if (propMap == nullptr)
            return ERROR;

        if (m_propertyMaps.Insert(propMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::CopyModifiedBasePropertyMaps(SchemaImportContext& ctx){
    if (m_state != ObjectState::Persisted)
        return SUCCESS;

    bvector<ClassMap const*> tphBaseClassMaps;
    DbMappingManager::Classes::PropertyMapInheritanceMode inheritanceMode = DbMappingManager::Classes::GetPropertyMapInheritanceMode(m_mapStrategyExtInfo);
    if (inheritanceMode != DbMappingManager::Classes::PropertyMapInheritanceMode::NotInherited) {
        for (ECClassCP baseClass : m_ecClass.GetBaseClasses()) {
            ClassMap const* baseClassMap = GetSchemaManager().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                return ERROR;

            if (baseClassMap->GetMapStrategy().IsTablePerHierarchy())
                tphBaseClassMaps.push_back(baseClassMap);
        }
    }

    for (ECPropertyCP property : m_ecClass.GetProperties(true)) {
        auto localPropertyMap = GetPropertyMaps().Find(property->GetName().c_str());
        if (localPropertyMap == nullptr || !localPropertyMap->IsData()) {
            continue;
        }

        DataPropertyMap const*  tphBaseClassPropMap = nullptr;
        if (&property->GetClass() != &m_ecClass && inheritanceMode == DbMappingManager::Classes::PropertyMapInheritanceMode::Clone) {
            for (ClassMap const* baseClassMap : tphBaseClassMaps) {
                PropertyMap const* propMap = baseClassMap->GetPropertyMaps().Find(property->GetName().c_str());
                if (propMap != nullptr && propMap->IsData()) {
                    tphBaseClassPropMap = &propMap->GetAs<DataPropertyMap>();
                    break;
                }
            }
        }

        if (tphBaseClassPropMap == nullptr) {
            continue;
        }

        ComputeHashPropertyMapVisitor basePropertyHash;
        tphBaseClassPropMap->AcceptVisitor(basePropertyHash);

        ComputeHashPropertyMapVisitor localPropertyMapHash;
        localPropertyMap->AcceptVisitor(localPropertyMapHash);

        if (localPropertyMapHash.GetHashCode() != basePropertyHash.GetHashCode()) {
            /*
             * POC-No-Remap
             * If this class has an override entry in ec_PropertyMap_Overrides for this property, the override mechanism in ClassMap::_Load will wire the correct old column at query time.
             * We must NOT copy the base class's new column here.
             */
            if (Enum::Contains(ctx.GetOptions(), SchemaManager::SchemaImportOptions::SkipRemapping) && m_ecdb.TableExists(TABLE_PropertyMap_Overrides))
                {
                /*
                 * POC-No-Remap
                 * Encountered in test SchemaRemapTestFixture.PutSiblingsIntoHierarchy (A property is a newly-inherited property that was deferred in _Load due to a conflict with an own
                 *   property, then assigned a fresh shared column).
                 * 
                 * If this property was assigned a fresh column by a prior Update() call in this import, do NOT copy the base class's column.
                 * The fresh assignment is intentionally different from the base and must be preserved to avoid a "column used by multiple properties" conflict.
                 */
                if (m_newlyMappedPropertyNames.count(property->GetName()) > 0)
                    continue;

                ECClassId classId = m_ecClass.GetId();
                Utf8StringCR propName = property->GetName();
                CachedStatementPtr skipStmt = m_ecdb.GetImpl().GetCachedSqliteStatement(
                    "SELECT 1 FROM main." TABLE_PropertyMap_Overrides
                    " WHERE ClassId = ? AND (AccessString = ? OR AccessString LIKE ? || '.%') LIMIT 1");
                if (skipStmt.IsValid()
                    && skipStmt->BindId(1, classId) == BE_SQLITE_OK
                    && skipStmt->BindText(2, propName.c_str(), Statement::MakeCopy::No) == BE_SQLITE_OK
                    && skipStmt->BindText(3, propName.c_str(), Statement::MakeCopy::No) == BE_SQLITE_OK
                    && skipStmt->Step() == BE_SQLITE_ROW)
                    continue;
                }

            // Base property have modified
            auto idx = m_propertyMaps.IndexOf(*localPropertyMap);
            m_propertyMaps.Remove(localPropertyMap->GetName().c_str());
            RefCountedPtr<PropertyMap> propMap = PropertyMapCopier::CreateCopy(*tphBaseClassPropMap, *this);
            if (propMap == nullptr)
                return ERROR;

            if (m_propertyMaps.Insert(propMap, idx) != SUCCESS)
                return ERROR;

            DbMapSaveContext saveCtx(m_ecdb);
            saveCtx.BeginSaving(*this);
            DbClassMapSaveContext classMapContext(saveCtx);
            SavePropertyMapVisitor saveVisitor(classMapContext, SavePropertyMapVisitor::Action::SkipIfExists);
            propMap->AcceptVisitor(saveVisitor);
            saveCtx.EndSaving(*this);
        }
     }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::Update(SchemaImportContext& ctx)
    {
    if (CopyModifiedBasePropertyMaps(ctx) != SUCCESS){
        BeAssert(false && "Unable to copy modified base properties");
        return ERROR;
    }

    //Follow can change ECInstanceId, ECClassId by optionally adding
    if (m_failedToLoadProperties.empty())
        return DbMappingManager::Classes::MapIndexes(ctx, *this, true);

    BeAssert(m_state == ObjectState::Persisted);
    m_state = ObjectState::Modified;

    UpdateColumnResolutionScope columnResolutionScope(*this);
    for (ECPropertyCP property : m_failedToLoadProperties)
        {
        PropertyMap const* propMap = DbMappingManager::Classes::MapProperty(ctx, *this, *property);
        if (propMap == nullptr)
            return ERROR;

        if (!propMap->IsData())
            {
            BeAssert(false);
            return ERROR;
            }

        /*
         * POC-No-Remap
         * 
         * Track this property so CopyModifiedBasePropertyMaps won't overwrite the freshly assigned column when this class is encountered a second time as a transitive derived class.
         */
        m_newlyMappedPropertyNames.insert(property->GetName());

        //Nav property maps cannot be saved here as they are not yet mapped.
        if (propMap->GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap& navPropMap = const_cast<NavigationPropertyMap&>(propMap->GetAs<NavigationPropertyMap>());
            if (ClassMappingStatus::Success != DbMappingManager::Classes::MapNavigationProperty(ctx, navPropMap))
                return ERROR;
            }

        //! ECSchema update added new property for which we need to save property map
        DbMapSaveContext saveCtx(m_ecdb);
        if (GetSchemaManager().GetDbSchema().UpdateTable(propMap->GetAs<DataPropertyMap>().GetTable()) != SUCCESS)
            {
            BeAssert(false && "Failed to save table");
            return ERROR;
            }

        saveCtx.BeginSaving(*this);
        DbClassMapSaveContext classMapContext(saveCtx);
        SavePropertyMapVisitor saveVisitor(classMapContext, SavePropertyMapVisitor::Action::SkipIfExists);
        propMap->AcceptVisitor(saveVisitor);
        saveCtx.EndSaving(*this);
        }

    m_failedToLoadProperties.clear();
    return DbMappingManager::Classes::MapIndexes(ctx, *this, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceIdPropertyMap const* ClassMap::GetECInstanceIdPropertyMap() const
    {
    PropertyMap const* propMap = GetPropertyMaps().Find(ECDBSYS_PROP_ECInstanceId);
    if (propMap == nullptr)
        return nullptr;

    return &propMap->GetAs<ECInstanceIdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECClassIdPropertyMap const* ClassMap::GetECClassIdPropertyMap() const
    {
    PropertyMap const* propMap = GetPropertyMaps().Find(ECDBSYS_PROP_ECClassId);
    if (propMap == nullptr)
        return nullptr;

    return &propMap->GetAs<ECClassIdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IssueDataSource const& ClassMap::Issues() const { return m_ecdb.GetImpl().Issues(); }


//------------------------------------------------------------------------------------------
//@bsimethod
//------------------------------------------------------------------------------------------
StorageDescription const& ClassMap::GetStorageDescription() const { return GetSchemaManager().GetLightweightCache().GetStorageDescription(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Nullable<std::vector<ClassMap const*>> ClassMap::GetDerivedClassMaps() const
    {
    ECDerivedClassesList const* derivedClasses = m_ecdb.Schemas().GetDerivedClassesInternal(GetClass());
    if (derivedClasses == nullptr)
        return nullptr;

    std::vector<ClassMap const*> derivedClassMaps;
    for (ECClassCP derivedClass : *derivedClasses)
        {
        if (ClassMap const* derivedClassMap = GetSchemaManager().GetClassMap(*derivedClass))
            derivedClassMaps.push_back(derivedClassMap);
        }

    return derivedClassMaps;
    }

//------------------------------------------------------------------------------------------
//@bsimethod
//------------------------------------------------------------------------------------------
BentleyStatus ClassMap::SetOverflowTable(DbTable& overflowTable)
    {
    if (GetOverflowTable())
        return SUCCESS;

    if (!GetMapStrategy().IsTablePerHierarchy())
        {
        BeAssert(GetMapStrategy().IsTablePerHierarchy());
        return ERROR;
        }

    ECInstanceIdPropertyMap* ecInstanceIdPropertyMap = const_cast<ECInstanceIdPropertyMap*>(GetECInstanceIdPropertyMap());
    ECClassIdPropertyMap* ecClassIdPropertyMap = const_cast<ECClassIdPropertyMap*>(GetECClassIdPropertyMap());
    if (ecInstanceIdPropertyMap == nullptr || ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }


    //!Overflow is delay created which mean system property map are not upto day
    AddTable(overflowTable);

    DbColumn const* ecInstanceIdColumn = overflowTable.FindFirst(DbColumn::Kind::ECInstanceId);
    if (ecInstanceIdColumn == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbColumn const* ecClassIdColumn = overflowTable.FindFirst(DbColumn::Kind::ECClassId);
    if (ecClassIdColumn == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*ecInstanceIdPropertyMap, *ecInstanceIdColumn) == ERROR)
        return ERROR;

    if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*ecClassIdPropertyMap, *ecClassIdColumn) == ERROR)
        return ERROR;

    if (GetState() == ObjectState::Modified || GetState() == ObjectState::Persisted)
        {
        //We need to save to get id for overflow table when its created during schema update
        if (!overflowTable.HasId())
            m_ecdb.Schemas().GetDispatcher().Main().GetDbSchema().SaveOrUpdateTables();

        DbMapSaveContext ctx(m_ecdb);
        ctx.BeginSaving(*this);
        DbClassMapSaveContext classMapContext(ctx);
        SavePropertyMapVisitor saveVisitor(classMapContext, SavePropertyMapVisitor::Action::SkipIfExists, &overflowTable);
        ecInstanceIdPropertyMap->AcceptVisitor(saveVisitor);
        ecClassIdPropertyMap->AcceptVisitor(saveVisitor);
        ctx.EndSaving(*this);
        }

    Nullable<std::vector<ClassMap const*>> derivedClassMaps = GetDerivedClassMaps();
    if (derivedClassMaps == nullptr)
        return ERROR;

    for (ClassMap const* derivedClassMap : derivedClassMaps.Value())
        {
        if (derivedClassMap != nullptr)
            const_cast<ClassMap*>(derivedClassMap)->SetOverflowTable(overflowTable);
        }

    return SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod
//------------------------------------------------------------------------------------------
BentleyStatus ClassMap::MapSystemColumns()
    {
    if (GetECInstanceIdPropertyMap() != nullptr || GetECClassIdPropertyMap() != nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    std::vector<DbColumn const*> ecInstanceIdColumns, ecClassIdColumns;

    for (DbTable const* table : GetTables())
        {
        DbColumn const* ecInstanceIdColumn = table->FindFirst(DbColumn::Kind::ECInstanceId);
        if (ecInstanceIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        DbColumn const* ecClassIdColumn = table->FindFirst(DbColumn::Kind::ECClassId);
        if (ecClassIdColumn == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ecInstanceIdColumns.insert(ecInstanceIdColumns.begin(), ecInstanceIdColumn);
        ecClassIdColumns.insert(ecClassIdColumns.begin(), ecClassIdColumn);
        }

    if (ecInstanceIdColumns.empty() || ecClassIdColumns.empty())
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropertyMap = ECInstanceIdPropertyMap::CreateInstance(*this, ecInstanceIdColumns);
    if (ecInstanceIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropertyMap = ECClassIdPropertyMap::CreateInstance(*this, ecClassIdColumns);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_propertyMaps.Insert(ecInstanceIdPropertyMap, 0) != SUCCESS)
        return ERROR;

    return m_propertyMaps.Insert(ecClassIdPropertyMap, 1);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapColumnFactory const& ClassMap::GetColumnFactory() const
    {
    if (m_columnFactory == nullptr)
        m_columnFactory = std::make_unique<ClassMapColumnFactory>(*this);

    return *m_columnFactory;
    }

//************************** NotMappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ClassMappingStatus NotMappedClassMap::_Map(ClassMappingContext& ctx)
    {
    DbTable const* nullTable = GetSchemaManager().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus NotMappedClassMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    DbTable const* nullTable = GetSchemaManager().GetDbSchema().GetNullTable();
    SetTable(*const_cast<DbTable*> (nullTable));
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
