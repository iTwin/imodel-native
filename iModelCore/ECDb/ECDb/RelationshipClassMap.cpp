/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <array>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************ RelationshipClassMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
RelationshipClassMap::RelationshipClassMap(ECDb const& ecdb, Type type, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : ClassMap(ecdb, type, ecRelClass, mapStrategy), m_sourceConstraintMap( ecRelClass.GetRelationshipClassCP()->GetSource()), m_targetConstraintMap( ecRelClass.GetRelationshipClassCP()->GetTarget())
    {
    BeAssert(ecRelClass.IsRelationshipClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& RelationshipClassMap::GetConstraintMap(ECN::ECRelationshipEnd constraintEnd) const
    {
    return constraintEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap& RelationshipClassMap::GetConstraintMapR(ECN::ECRelationshipEnd constraintEnd)
    {
    return constraintEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ConstraintECInstanceIdPropertyMap const* RelationshipClassMap::GetConstraintECInstanceIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECInstanceIdPropMap();
    else
        return GetTargetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ConstraintECClassIdPropertyMap const* RelationshipClassMap::GetConstraintECClassIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECClassIdPropMap();
    else
        return GetTargetECClassIdPropMap();
    }




//************************ RelationshipClassEndTableMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap const* RelationshipClassEndTableMap::GetBaseClassMap(SchemaImportContext* ctx) const
    {
    if (!GetClass().HasBaseClasses())
        return nullptr;

    BeAssert(GetClass().GetBaseClasses().size() == 1);
    ECRelationshipClassCP relationshipClass = static_cast<ECRelationshipClassCP>(GetClass().GetBaseClasses().front());
    if (ClassMapCP classMap = GetDbMap().GetClassMap(*relationshipClass))
        return &classMap->GetAs<RelationshipClassEndTableMap>();

    BeAssert(ctx != nullptr);
    if (GetDbMap().MapRelationshipClass(*ctx, *relationshipClass) == ClassMappingStatus::Success)
        return &GetDbMap().GetClassMap(*relationshipClass)->GetAs<RelationshipClassEndTableMap>();

    BeAssert(false);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetReferencedEnd() const
    {
    return GetForeignEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetForeignEnd() const
    {
    return GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::MapSubClass(RelationshipClassEndTableMap const& baseRelClassMap)
    {
    //ECInstanceId property map
    SystemPropertyMap const* basePropMap = baseRelClassMap.GetECInstanceIdPropertyMap();
    if (basePropMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*basePropMap, *this)) != SUCCESS)
        return ClassMappingStatus::Error;

    //ECClassId property map
    SystemPropertyMap const* classIdPropertyMap = baseRelClassMap.GetECClassIdPropertyMap();
    if (classIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*classIdPropertyMap, *this)) != SUCCESS)
        return ClassMappingStatus::Error;


    //Source
    RelationshipConstraintMap const& baseSourceConstraintMap = baseRelClassMap.GetConstraintMap(ECRelationshipEnd::ECRelationshipEnd_Source);
    if (baseSourceConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseSourceConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& sourceConstraintMap = GetConstraintMapR(ECRelationshipEnd::ECRelationshipEnd_Source);

    //Source ECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseSourceConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

    sourceConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    GetTablesPropertyMapVisitor tableDisp;
    clonedConstraintInstanceId->AcceptVisitor(tableDisp);
    for (DbTable const* table : tableDisp.GetTables())
        {
        AddTable(*const_cast<DbTable *>(table));
        }

    //Source ECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseSourceConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    sourceConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    //Target
    RelationshipConstraintMap const& baseTargetEndConstraintMap = baseRelClassMap.GetConstraintMap(ECRelationshipEnd::ECRelationshipEnd_Target);
    if (baseTargetEndConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseTargetEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& targetEndConstraintMap = GetConstraintMapR(ECRelationshipEnd::ECRelationshipEnd_Target);

    //Target ECInstanceId prop map
    clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseTargetEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

    targetEndConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    //Target ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseTargetEndConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ClassMappingStatus::Error;
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    targetEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    return ClassMappingStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::_Map(ClassMappingContext& ctx)
    {
    if (RelationshipClassEndTableMap const* baseClassMap = GetBaseClassMap(&ctx.GetImportCtx()))
        return MapSubClass(*baseClassMap);

    //End table relationship is always mapped to virtual table.
    //we use nav properties to generate views dynamically
    if (SUCCESS != DbMappingManager::Tables::CreateVirtualTableForFkRelationship(ctx.GetImportCtx(), *this, ctx.GetClassMappingInfo()))
        return ClassMappingStatus::Error;

    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;
    
    DbTable* vtable = GetTables().front();
    {////////SourceECInstanceId
    DbColumn* sourceECInstanceId = vtable->CreateColumn(ECDBSYS_PROP_SourceECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceId});
    if (propMap == nullptr || sourceECInstanceId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap SourceECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 2) != SUCCESS)
        return ClassMappingStatus::Error;
    std::vector <DbTable const*> list;
    GetConstraintMapR(ECRelationshipEnd_Source).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////SourceECClassId
    DbColumn* sourceECClassId = vtable->CreateColumn(ECDBSYS_PROP_SourceECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassId});
    if (propMap == nullptr || sourceECClassId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap SourceECClassId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 3) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Source).SetECClassIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECInstanceId
    DbColumn* targetECInstanceId = vtable->CreateColumn(ECDBSYS_PROP_TargetECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceId});
    if (propMap == nullptr || targetECInstanceId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap TargetECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 4) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Target).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECClassId
    DbColumn* targetECClassId = vtable->CreateColumn(ECDBSYS_PROP_TargetECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECClassId});
    if (propMap == nullptr || targetECClassId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap TargetECClassId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 5) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Target).SetECClassIdPropMap(propMap.get());
    }/////////////////////////

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (SUCCESS != ClassMap::_Load(ctx, mapInfo))
        return ERROR;

    //SourceECInstanceId
    std::vector<DbColumn const*> const* mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_SourceECInstanceId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap> sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //SourceECClassId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_SourceECClassId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> sourceClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceClassIdPropMap.get());

    //TargetECInstanceId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_TargetECInstanceId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap> targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    //TargetECClassId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_TargetECClassId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> targetClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetClassIdPropMap.get());
    return SUCCESS;
    }


//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : RelationshipClassMap(ecdb, Type::RelationshipLinkTable, ecRelClass, mapStrategy)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
ClassMappingStatus RelationshipClassLinkTableMap::_Map(ClassMappingContext& ctx)
    {
    BeAssert(!MapStrategyExtendedInfo::IsForeignKeyMapping(GetMapStrategy()) &&
             "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");

    ClassMappingStatus stat = DoMapPart1(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    if (GetRelationshipClass().HasBaseClasses())
        return MapSubClass(ctx);

    ECRelationshipConstraintCR sourceConstraint = GetRelationshipClass().GetSource();
    ECRelationshipConstraintCR targetConstraint = GetRelationshipClass().GetTarget();

    LinkTableRelationshipMapCustomAttribute ca;
    ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(ca, GetRelationshipClass());

    //*** root rel class
    //Table retrieval is only needed for the root rel class. Subclasses will use the tables of its base class
    //TODO: How should we handle this properly?
    std::set<DbTable const*> sourceTables = GetDbMap().GetRelationshipConstraintPrimaryTables(ctx.GetImportCtx(), sourceConstraint);
    std::set<DbTable const*> targetTables = GetDbMap().GetRelationshipConstraintPrimaryTables(ctx.GetImportCtx(), targetConstraint);

    if (sourceTables.empty() || targetTables.empty())
        {
        Issues().Report("Failed to map ECRelationshipClass '%s'. Source or target constraint classes are abstract without subclasses. Consider applying the MapStrategy 'TablePerHierarchy' to the abstract constraint class.",
                        GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    const size_t sourceTableCount = sourceTables.size();
    const size_t targetTableCount = targetTables.size();
    if (sourceTableCount > 1 || targetTableCount > 1)
        {
        Utf8CP constraintStr = nullptr;
        if (sourceTableCount > 1 && targetTableCount > 1)
            constraintStr = "source and target constraints are";
        else if (sourceTableCount > 1)
            constraintStr = "source constraint is";
        else
            constraintStr = "target constraint is";

        Issues().Report("Failed to map ECRelationshipClass '%s'. It is mapped to a link table, but the %s mapped to more than one table, which is not supported for link tables.",
                        GetClass().GetFullName(), constraintStr);

        return ClassMappingStatus::Error;
        }

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addSourceECClassIdColumnToTable, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addTargetECClassIdColumnToTable, targetConstraint);
    stat = CreateConstraintPropMaps(ctx.GetImportCtx(), ca, addSourceECClassIdColumnToTable, addTargetECClassIdColumnToTable);
    if (stat != ClassMappingStatus::Success)
        return stat;

    stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;


    //only create constraints on TPH root or if not TPH and not existing table
    if (GetPrimaryTable().GetType() != DbTable::Type::Existing && (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId()))
        {
        Nullable<bool> createFkConstraints;
        ca.TryGetCreateForeignKeyConstraints(createFkConstraints);
        if (createFkConstraints.IsNull() || //if CreateFkConstraint was not specified, it defaults to true
            createFkConstraints.Value())
            {
            //Create FK from Source-Primary to LinkTable
            DbTable const* sourceTable = *sourceTables.begin();
            DbColumn const* fkColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
            DbColumn const* referencedColumn = sourceTable->FindFirst(DbColumn::Kind::ECInstanceId);
            GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

            //Create FK from Target-Primary to LinkTable
            DbTable const* targetTable = *targetTables.begin();
            fkColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
            referencedColumn = targetTable->FindFirst(DbColumn::Kind::ECInstanceId);
            GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);
            }
        }

    Nullable<bool> allowDuplicateRelationships;
    ca.TryGetAllowDuplicateRelationships(allowDuplicateRelationships);
    AddIndices(ctx.GetImportCtx(), GetAllowDuplicateRelationshipsFlag(allowDuplicateRelationships));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::MapSubClass(ClassMappingContext& ctx)
    {
    if (GetClass().GetBaseClasses().size() != 1)
        {
        BeAssert(false && "Multi-inheritance of ECRelationshipclasses should have been caught before already");
        return ClassMappingStatus::Error;
        }

    ECClassCP baseClass = GetClass().GetBaseClasses()[0];
    ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
    if (baseClassMap == nullptr || baseClassMap->GetType() != ClassMap::Type::RelationshipLinkTable)
        {
        BeAssert(false && "Could not find class map of base ECRelationship class or is not of right type");
        return ClassMappingStatus::Error;
        }

    RelationshipClassLinkTableMap const& baseRelClassMap = baseClassMap->GetAs<RelationshipClassLinkTableMap>();

    //SourceECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(&clonedSourceECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //SourceECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(&clonedSourceECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    //TargetECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(&clonedTargetECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //TargetECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(&clonedTargetECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    ClassMappingStatus stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    AddIndices(ctx.GetImportCtx(), DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClass->GetRelationshipClassCP()));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(SchemaImportContext& ctx, LinkTableRelationshipMapCustomAttribute const& ca, ECRelationshipEnd relationshipEnd)
    {
    DbColumn* endECClassIdColumn = nullptr;
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? GetRelationshipClass().GetSource() : GetRelationshipClass().GetTarget();
    ECClass const* foreignEndClass = foreignEndConstraint.GetConstraintClasses()[0];
    ClassMap const* foreignEndClassMap = GetDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetDbMap().GetRelationshipConstraintTableCount(ctx, foreignEndConstraint);

    Utf8String columnName = DetermineConstraintECClassIdColumnName(ca, relationshipEnd);
    if (GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr &&
        GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        //Following error occurs in Upgrading ECSchema but is not fatal.
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return nullptr;
        }

    if (foreignEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
        BeAssert(endECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        endECClassIdColumn = const_cast<DbColumn*>(foreignEndClassMap->GetPrimaryTable().FindFirst(DbColumn::Kind::ECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Virtual);
        }

    return endECClassIdColumn;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(SchemaImportContext& ctx, LinkTableRelationshipMapCustomAttribute const& ca, bool addSourceECClassIdColumnToTable,
    bool addTargetECClassIdColumnToTable)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = DetermineConstraintECInstanceIdColumnName(ca, ECRelationshipEnd_Source);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        ctx.Issues().Report("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_SourceECInstanceId " column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
    if (sourceECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceIdColumn});
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //**** SourceECClassId prop map
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(ctx, ca, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = DetermineConstraintECInstanceIdColumnName(ca, ECRelationshipEnd_Target);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_TargetECInstanceId " column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
    if (targetECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceIdColumn});
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());


    //**** TargetECClassId prop map
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(ctx, ca, ECRelationshipEnd_Target);
    auto targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this , ECRelationshipEnd_Target, {targetECClassIdColumn});
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());
    return ClassMappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndices(SchemaImportContext& ctx, bool allowDuplicateRelationships)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;

    // Add indices on the source and target based on cardinality
    //(the many side can be unique, but the one side must never be unique)
    const bool sourceIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1);
    const bool targetIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1);

    AddIndex(ctx, RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndex(ctx, RelationshipIndexSpec::Target, targetIsUnique);

    if (!allowDuplicateRelationships)
        AddIndex(ctx, RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndex(SchemaImportContext& schemaImportContext, RelationshipIndexSpec spec, bool isUniqueIndex)
    {
    // Setup name of the index
    Utf8String name(isUniqueIndex ? "uix_" : "ix_");
    name.append(GetClass().GetSchema().GetAlias()).append("_").append(GetClass().GetName()).append("_");

    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                name.append("source");
                break;
            case RelationshipIndexSpec::Target:
                name.append("target");
                break;
            case RelationshipIndexSpec::SourceAndTarget:
                name.append("sourcetarget");
                break;
            default:
                BeAssert(false);
                break;
        }

    auto sourceECInstanceIdColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;
    auto targetECInstanceIdColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;

    std::vector<DbColumn const*> columns;
    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, nullptr, nullptr);
                break;
            case RelationshipIndexSpec::Target:
                GenerateIndexColumnList(columns, targetECInstanceIdColumn, targetECClassIdColumn, nullptr, nullptr);
                break;

            case RelationshipIndexSpec::SourceAndTarget:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn);
                break;

            default:
                BeAssert(false);
                break;
        }

    if (SUCCESS != DbMappingManager::Tables::CreateIndex(schemaImportContext, GetPrimaryTable(), name, isUniqueIndex, columns, false, true, GetClass().GetId(),
                                                         //if a partial index is created, it must only apply to this class,
                                                         //not to subclasses, as constraints are not inherited by relationships
                                                         false))
        {
        BeAssert(false && "Failed to create index for link table relationship");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::GenerateIndexColumnList(std::vector<DbColumn const*>& columns, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4)
    {
    if (nullptr != col1 && col1->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col1);

    if (nullptr != col2 && col2->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col2);

    if (nullptr != col3 && col3->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col3);

    if (nullptr != col4 && col4->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col4);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECInstanceIdColumnName(LinkTableRelationshipMapCustomAttribute const& ca, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            Nullable<Utf8String> sourceIdColName;
            ca.TryGetSourceECInstanceIdColumn(sourceIdColName);
            if (sourceIdColName.IsNull())
                colName.assign(COL_DEFAULTNAME_SourceId);
            else
                colName.assign(sourceIdColName.Value());

            break;
            }
            case ECRelationshipEnd_Target:
            {
            Nullable<Utf8String> targetIdColName;
            ca.TryGetTargetECInstanceIdColumn(targetIdColName);
            if (targetIdColName.IsNull())
                colName.assign(COL_DEFAULTNAME_TargetId);
            else
                colName.assign(targetIdColName.Value());

            break;
            }

            default:
                BeAssert(false);
                break;
        }

    BeAssert(!colName.empty());
    return colName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECClassIdColumnName(LinkTableRelationshipMapCustomAttribute const& ca, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    Nullable<Utf8String> idColName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            ca.TryGetSourceECInstanceIdColumn(idColName);

            if (idColName.IsNull())
                colName.assign(COL_SourceECClassId);

            break;
            }

            case ECRelationshipEnd_Target:
            {
            ca.TryGetTargetECInstanceIdColumn(idColName);

            if (idColName.IsNull())
                colName.assign(COL_TargetECClassId);
            
            break;
            }

            default:
                BeAssert(false);
                break;
        }

    if (!idColName.IsNull())
        {
        if (!idColName.Value().EndsWithIAscii("id"))
            colName.assign(idColName.Value());
        else
            colName.assign(idColName.Value().substr(0, idColName.Value().size() - 2));

        colName.append("ClassId");
        }

    BeAssert(!colName.empty());
    return colName;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                10/2015
//+---------------+---------------+---------------+---------------+---------------+-
//static
bool RelationshipClassLinkTableMap::DetermineAllowDuplicateRelationshipsFlagFromRoot(ECRelationshipClassCR baseRelClass)
    {
    LinkTableRelationshipMapCustomAttribute linkRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkRelMap, baseRelClass))
        {
        //default for AllowDuplicateRelationships: false
        Nullable<bool> allowDuplicateRels;
        linkRelMap.TryGetAllowDuplicateRelationships(allowDuplicateRels);
        if (GetAllowDuplicateRelationshipsFlag(allowDuplicateRels))
            return true;
        }

    if (!baseRelClass.HasBaseClasses())
        return false;

    BeAssert(baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP() != nullptr);
    return DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipClassLinkTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (ClassMap::_Load(ctx, mapInfo) != SUCCESS)
        return ERROR;

    std::vector<DbColumn const*> const* mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_SourceECInstanceId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    RefCountedPtr<ConstraintECInstanceIdPropertyMap> sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_SourceECClassId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    RefCountedPtr<ConstraintECClassIdPropertyMap> sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_TargetECInstanceId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap>  targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_TargetECClassId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }    
    
    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::CreateConstraintColumn(Utf8CP columnName, PersistenceType persType)
    {
    DbTable& table = GetPrimaryTable();
    const bool wasEditMode = table.GetEditHandle().CanEdit();
    if (!wasEditMode)
        table.GetEditHandleR().BeginEdit();

    DbColumn* column = table.FindColumnP(columnName);
    if (column != nullptr)
        return column;

    persType = table.GetType() != DbTable::Type::Existing ? persType : PersistenceType::Virtual;
    //Following protect creating virtual id/fk columns in persisted tables.
    if (table.GetType() != DbTable::Type::Virtual && persType == PersistenceType::Virtual)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': No columns found for " ECDBSYS_PROP_SourceECInstanceId " or " ECDBSYS_PROP_TargetECInstanceId " in table '%s'. Consider applying the LinkTableRelationshipMap custom attribute to the ECRelationshipClass.",
                   GetClass().GetFullName(), table.GetName().c_str());
        return nullptr;
        }
        
    column = table.CreateColumn(Utf8String(columnName), DbColumn::Type::Integer, DbColumn::Kind::Default, persType);

    if (!wasEditMode)
        table.GetEditHandleR().EndEdit();

    return column;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
void RelationshipClassLinkTableMap::DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECRelationshipConstraintCR constraint) const
    {
    //A constraint class id column is needed if 
    // * the map strategy implies that multiple classes are stored in the same table or
    // * it has more than one classes including subclasses in case of a polymorphic constraint. 
    //So we first determine whether a constraint class id column is needed
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    addConstraintClassIdColumnNeeded = constraintClasses.size() > 1 ;
    //if constraint is polymorphic, and if addConstraintClassIdColumnNeeded is not true yet,
    //we also need to check if the constraint classes have subclasses. If there is at least one, addConstraintClassIdColumnNeeded
    //is set to true;
    if (!addConstraintClassIdColumnNeeded && constraint.GetIsPolymorphic())
        addConstraintClassIdColumnNeeded = true;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
