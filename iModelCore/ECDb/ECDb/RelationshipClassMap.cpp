/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************ RelationshipClassMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
RelationshipClassMap::RelationshipClassMap(ECDb const& ecdb, Type type, ECRelationshipClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty)
    : ClassMap(ecdb, type, ecRelClass, mapStrategy, setIsDirty),
    m_sourceConstraintMap(ecdb, ecRelClass.GetId(), ECRelationshipEnd_Source, ecRelClass.GetSource()),
    m_targetConstraintMap(ecdb, ecRelClass.GetId(), ECRelationshipEnd_Target, ecRelClass.GetTarget())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassMap::CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind columnId, PersistenceType persType)
    {
    DbTable& table = GetPrimaryTable();
    const bool wasEditMode = table.GetEditHandle().CanEdit();
    if (!wasEditMode)
        table.GetEditHandleR().BeginEdit();

    DbColumn* column = table.FindColumnP(columnName);
    if (column != nullptr)
        {
        if (!Enum::Intersects(column->GetKind(), columnId))
            column->AddKind(columnId);

        return column;
        }

    persType = table.IsOwnedByECDb() ? persType : PersistenceType::Virtual;
    //Following protect creating virtual id/fk columns in persisted tables.
    if (table.GetPersistenceType() == PersistenceType::Persisted)
        {
        if (persType == PersistenceType::Virtual)
            {
            if (columnId == DbColumn::Kind::SourceECInstanceId || columnId == DbColumn::Kind::TargetECInstanceId)
                {
                BeAssert(false);
                return nullptr;
                }
            }
        }
    column = table.CreateColumn(columnName, DbColumn::Type::Integer, columnId, persType);

    if (!wasEditMode)
        table.GetEditHandleR().EndEdit();

    return column;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                        12/13
//---------------------------------------------------------------------------------------
//static
bool RelationshipClassMap::ConstraintIncludesAnyClass(ECN::ECRelationshipConstraintClassList const& constraintClasses)
    {
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        if (IsAnyClass(constraintClass->GetClass()))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
void RelationshipClassMap::DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECRelationshipConstraintCR constraint) const
    {
    //A constraint class id column is needed if 
    // * the map strategy implies that multiple classes are stored in the same table or
    // * the constraint includes the AnyClass or 
    // * it has more than one classes including subclasses in case of a polymorphic constraint. 
    //So we first determine whether a constraint class id column is needed
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    addConstraintClassIdColumnNeeded = constraintClasses.size() > 1 || ConstraintIncludesAnyClass(constraintClasses);
    //if constraint is polymorphic, and if addConstraintClassIdColumnNeeded is not true yet,
    //we also need to check if the constraint classes have subclasses. If there is at least one, addConstraintClassIdColumnNeeded
    //is set to true;
    if (!addConstraintClassIdColumnNeeded && constraint.GetIsPolymorphic())
        addConstraintClassIdColumnNeeded = true;

    //if no class id column on the end is required, store the class id directly so that it can be used as literal in the native SQL
    if (!addConstraintClassIdColumnNeeded)
        {
        BeAssert(constraintClasses.size() == 1);
        ECRelationshipConstraintClassCP constraintClass = constraintClasses[0];
        BeAssert(constraintClass->GetClass().HasId());
        defaultConstraintClassId = constraintClass->GetClass().GetId();
        }
    else
        defaultConstraintClassId = ECClassId();
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
// @bsimethod                                                    Krischan.Eberle  06/2015
//---------------------------------------------------------------------------------------
//static
RelationshipEndColumns const& RelationshipClassMap::GetEndColumnsMapping(RelationshipMappingInfo const& info, ECN::ECRelationshipEnd end)
    {
    return info.GetColumnsMapping(end);
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


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    02/2016
//---------------------------------------------------------------------------------------
bool RelationshipClassMap::_RequiresJoin(ECN::ECRelationshipEnd endPoint) const
    {
    ConstraintECClassIdPropertyMap const* referencedEndClassIdPropertyMap = endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECClassIdPropMap() : GetTargetECClassIdPropMap();
    if (referencedEndClassIdPropertyMap->GetDataPropertyMaps().size() != 1LL)
        {
        BeAssert(false && "Expecting exactly one property map");
        return false;
        }

    SingleColumnDataPropertyMap const* vm = static_cast<SingleColumnDataPropertyMap const*>(referencedEndClassIdPropertyMap->GetDataPropertyMaps().front());
    return vm->GetColumn().GetPersistenceType() != PersistenceType::Virtual && !vm->IsMappedToClassMapTables();
    }

//************************ RelationshipConstraintMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
RelationshipConstraintMap::RelationshipConstraintMap(ECDb const& ecdb, ECN::ECClassId relClassId, ECN::ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint) : m_ecdb(ecdb), m_relClassId(relClassId), m_constraintEnd(constraintEnd), m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr)
    {
    m_anyClassMatches = RelationshipClassMap::ConstraintIncludesAnyClass(m_constraint.GetConstraintClasses());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
bool RelationshipConstraintMap::ClassIdMatchesConstraint(ECN::ECClassId candidateClassId) const
    {
    if (m_anyClassMatches)
        return true;

    bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_ecdb.Schemas().GetDbMap().GetLightweightCache().GetConstraintClassesForRelationshipClass(m_relClassId);
    auto it = constraintClassIds.find(candidateClassId);
    if (it == constraintClassIds.end())
        return false;

    const LightweightCache::RelationshipEnd requiredEnd = m_constraintEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
    const LightweightCache::RelationshipEnd actualEnd = it->second;
    return actualEnd == LightweightCache::RelationshipEnd::Both || actualEnd == requiredEnd;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2016
//---------------------------------------------------------------------------------------
bool RelationshipConstraintMap::TryGetSingleClassIdFromConstraint(ECClassId& constraintClassId) const
    {
    if (m_anyClassMatches)
        return false;

    bmap<ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_ecdb.Schemas().GetDbMap().GetLightweightCache().GetConstraintClassesForRelationshipClass(m_relClassId);

    const LightweightCache::RelationshipEnd requiredEnd = m_constraintEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
    ECClassId singleConstraintClassId;
    for (bpair<ECClassId, LightweightCache::RelationshipEnd> const& kvPair : constraintClassIds)
        {
        const LightweightCache::RelationshipEnd actualEnd = kvPair.second;
        if (LightweightCache::RelationshipEnd::Both == actualEnd || requiredEnd == actualEnd)
            {
            //If IsValid, then single constraint class id is already set, and therefore the constraint has more than one constraint classes
            if (singleConstraintClassId.IsValid())
                return false;

            singleConstraintClassId = kvPair.first;
            }
        }

    return true;
    }

//************************ RelationshipClassEndTableMap **********************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassEndTableMap::RelationshipClassEndTableMap(ECDb const& ecdb, ECRelationshipClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty)
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy, setIsDirty), m_hasKeyPropertyFk(false)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
RelationshipClassMap::ReferentialIntegrityMethod RelationshipClassEndTableMap::_GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing || GetRelationshipClass().GetClassModifier() == ECClassModifier::Abstract
        || GetRelationshipClass().GetSource().GetClasses().empty() || GetRelationshipClass().GetTarget().GetClasses().empty())
        return ReferentialIntegrityMethod::None;

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing || GetRelationshipClass().GetStrength() == StrengthType::Embedding || GetRelationshipClass().GetStrength() == StrengthType::Holding)
        {
        return ReferentialIntegrityMethod::ForeignKey;
        }

    BeAssert(false && "Trigger are not supported");
    return ReferentialIntegrityMethod::Trigger;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
bool RelationshipClassEndTableMap::_RequiresJoin(ECN::ECRelationshipEnd endPoint) const
    {
    //We need to join if ECClassId is both SourceECClassId and TargetECClassId. This case of selfJoin where we must join.
    if (endPoint == GetForeignEnd())
        return false;

    auto referencedEndClassIdPropertyMap = endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECClassIdPropMap() : GetTargetECClassIdPropMap();
    if (!referencedEndClassIdPropertyMap->IsMappedToSingleTable())
        {
        BeAssert(false);
        return false;
        }

    SingleColumnDataPropertyMap const* vm = static_cast<SingleColumnDataPropertyMap const*>(referencedEndClassIdPropertyMap->GetDataPropertyMaps().front());
    if (vm->GetColumn().GetPersistenceType() != PersistenceType::Virtual && !vm->IsMappedToClassMapTables())
        return true;

    //SELF JOIN case
    if (GetSourceECClassIdPropMap()->IsMappedToSingleTable() && GetTargetECClassIdPropMap()->IsMappedToSingleTable())
        {
        auto source = GetSourceECClassIdPropMap()->GetDataPropertyMaps().front();
        auto target = GetTargetECClassIdPropMap()->GetDataPropertyMaps().front();

        return  &source->GetColumn() == &target->GetColumn()
            && source->GetColumn().GetPersistenceType() == PersistenceType::Persisted;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus RelationshipClassEndTableMap::_Map(SchemaImportContext& ctx, ClassMappingInfo const& classMapInfo)
    {   
    //Don't call base class method as end table map requires its own handling
    BeAssert(GetClass().GetRelationshipClassCP() != nullptr && MapStrategyExtendedInfo::IsForeignKeyMapping(classMapInfo.GetMapStrategy()));
    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&classMapInfo) != nullptr);
    RelationshipMappingInfo const& relClassMappingInfo = static_cast<RelationshipMappingInfo const&> (classMapInfo);

    if (GetClass().HasBaseClasses())
        return MapSubClass(relClassMappingInfo) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;

    //root class (no base class) mapping

    ColumnLists columns;
    if (SUCCESS != DetermineKeyAndConstraintColumns(columns, relClassMappingInfo))
        return MappingStatus::Error;

    //Set tables
    for (DbColumn const* fkTablePkCol : columns.m_ecInstanceIdColumnsPerFkTable)
        {
        AddTable(fkTablePkCol->GetTableR());
        }

    //Create ECInstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropMap = ECInstanceIdPropertyMap::CreateInstance(*this, columns.m_ecInstanceIdColumnsPerFkTable);
    if (ecInstanceIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMapECInstanceId");
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(ecInstanceIdPropMap, 0LL) != SUCCESS)
        return MappingStatus::Error;

    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropMap = ECClassIdPropertyMap::CreateInstance(*this, GetClass().GetId(), columns.m_relECClassIdColumnsPerFkTable);
    if (ecClassIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create ECClassIdPropertyMap");
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(ecClassIdPropMap, 1LL) != SUCCESS)
        return MappingStatus::Error;

    //ForeignEnd ECInstanceId PropMap
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> foreignEndIdPropertyMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetForeignEnd(), columns.m_ecInstanceIdColumnsPerFkTable);
    if (foreignEndIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(foreignEndIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECInstanceIdPropMap(foreignEndIdPropertyMap.get());

    //Create ForeignEnd ClassId propertyMap
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    RefCountedPtr<ConstraintECClassIdPropertyMap> foreignEndClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, referencedEndConstraint.GetClasses()[0]->GetId(), GetForeignEnd(), columns.m_classIdColumnsPerFkTable);
    if (foreignEndClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(foreignEndClassIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECClassIdPropMap(foreignEndClassIdPropertyMap.get());

    //FK PropMap (aka referenced end id prop map)
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> fkPropertyMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetReferencedEnd(), columns.m_fkColumnsPerFkTable);
    if (fkPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(fkPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECInstanceIdPropMap(fkPropertyMap.get());


    //FK ClassId PropMap (aka referenced end classid prop map)
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    RefCountedPtr<ConstraintECClassIdPropertyMap> fkClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, foreignEndConstraint.GetClasses()[0]->GetId(), GetReferencedEnd(), columns.m_referencedEndECClassIdColumns);
    if (fkClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(fkClassIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECClassIdPropMap(fkClassIdPropertyMap.get());

    //map non-system properties
    if (MappingStatus::Error == MapProperties(ctx))
        return MappingStatus::Error;

    AddIndexToRelationshipEnd(ctx, classMapInfo);
    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns(ColumnLists& columns, RelationshipMappingInfo const& classMappingInfo)
    {
    BeAssert(!GetClass().HasBaseClasses() && "RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns is expected to only be called for root rel classes.");
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    std::set<DbTable const*> foreignEndTables = GetForeignEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables() : classMappingInfo.GetTargetTables();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    //! table must meet following constraint though these are already validated at MapStrategy evaluation time.
    BeAssert(foreignEndTables.size() >= 1 && "ForeignEnd Tables must be >= 1");
    BeAssert(GetReferencedEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables().size() == 1 : classMappingInfo.GetTargetTables().size() == 1 && "ReferencedEnd Tables must be == 1");

    //needed to determine NOT NULL of FK and RelClassId cols
    bset<ECClassId> foreignEndConstraintClassIds;
    for (ECRelationshipConstraintClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
        {
        BeAssert(constraintClass->GetClass().HasId());
        foreignEndConstraintClassIds.insert(constraintClass->GetClass().GetId());
        }


    //! Determine FK column name and map to it or create a column and then map to it.
    //!--------------------------------------------------------------------------------------
    //! 1. Provided as RelationshipKey property
    //!     a. Only one key property should be defined.
    //!     b. DataType must match referencedEndTable PK.
    //!     c. All class in constraint must have the property.
    //! 2. Provided as part of CustomAttribute
    //!     a. Column name specified must match or it would be a error.
    //!     b. If column cannot be created in one of the foreign end table its a error.
    //! 3. Generate foreign key column.
    //!     a. Use Nav property column name if exist
    //!     b. Generate a name and create a column that name.


    //! 1. Provided as RelationshipKey property
    //! ---------------------------------------

    m_hasKeyPropertyFk = false;

    std::set<DbColumn const*> fkTableFkColSet;
    if (SUCCESS != TryGetKeyPropertyColumn(fkTableFkColSet, foreignEndConstraint, relClass, GetForeignEnd()))
        return ERROR;

        {
        std::set<DbColumn const*> referencedTablePrimaryKeyColSet;
        if (SUCCESS != TryGetKeyPropertyColumn(referencedTablePrimaryKeyColSet, referencedEndConstraint, relClass, GetReferencedEnd()))
            return ERROR;

        if (!referencedTablePrimaryKeyColSet.empty())
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s' because a KeyProperty is defined on the %s constraint. "
                            "A KeyProperty can only be specified on the foreign key end constraint of the ECRelationshipClass (here: %s constraint)",
                            relClass.GetFullName(),
                            GetReferencedEnd() == ECRelationshipEnd_Source ? "source" : "target",
                            GetForeignEnd() == ECRelationshipEnd_Source ? "source" : "target");
            return ERROR;
            }
        }

    //+++ Determine FK column(s) +++
    //Note: The FK column is the column that refers to the referenced end. Therefore the ECRelationshipEnd of the referenced end has to be taken!
    DbColumn::Kind foreignKeyColumnKind = GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;
    const bool multiplicityImpliesNotNullOnFkCol = referencedEndConstraint.GetMultiplicity().GetLowerLimit() > 0;

    Utf8String fkColName;
    if (!fkTableFkColSet.empty())
        {
        m_hasKeyPropertyFk = true;
        for (DbColumn const* fkCol : fkTableFkColSet)
            {
            if (SUCCESS != ValidateForeignKeyColumn(*fkCol, multiplicityImpliesNotNullOnFkCol, foreignKeyColumnKind))
                return ERROR;

            ColumnLists::push_back(columns.m_fkColumnsPerFkTable, fkCol);
            if (fkColName.empty())
                fkColName.assign(fkCol->GetName());
            else
                {
                if (!fkColName.EqualsIAscii(fkCol->GetName().c_str()))
                    {
                    Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s' because a KeyProperty is defined on the %s constraint which "
                                    "maps to columns in different tables and the columns don't have the same name.",
                                    relClass.GetFullName(), GetReferencedEnd() == ECRelationshipEnd_Source ? "source" : "target");

                    return ERROR;
                    }
                }
            }
        }
    else
        {
        ForeignKeyColumnInfo fkColInfo;
        if (SUCCESS != TryGetForeignKeyColumnInfoFromNavigationProperty(fkColInfo, foreignEndConstraint, relClass, GetForeignEnd()))
            return ERROR;

        fkColName = DetermineFkColumnName(classMappingInfo, fkColInfo);

        for (DbTable const* foreignEndTable : foreignEndTables)
            {
            DbColumn const* fkCol = foreignEndTable->FindColumn(fkColName.c_str());
            if (!foreignEndTable->IsOwnedByECDb())
                {
                //for existing tables, the FK column must exist otherwise we fail schema import
                if (fkCol != nullptr)
                    {
                    if (SUCCESS != ValidateForeignKeyColumn(*fkCol, multiplicityImpliesNotNullOnFkCol, foreignKeyColumnKind))
                        return ERROR;

                    ColumnLists::push_back(columns.m_fkColumnsPerFkTable, fkCol);
                    continue;
                    }

                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. It is mapped to the existing table '%s' not owned by ECDb, but doesn't have a foreign key column called '%s'. Consider adding a ForeignKeyRelationshipMap CustomAttribute to the relationship class and specify the foreign key column.",
                                relClass.GetFullName(), foreignEndTable->GetName().c_str(), fkColName.c_str());
                return ERROR;
                }

            //table owned by ECDb
            if (fkCol != nullptr)
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. ForeignKey column name '%s' is already used by another column in the table '%s'.",
                                relClass.GetFullName(), fkColName.c_str(), foreignEndTable->GetName().c_str());
                return ERROR;
                }

            int fkColPosition = -1;
            if (SUCCESS != TryDetermineForeignKeyColumnPosition(fkColPosition, *foreignEndTable, fkColInfo))
                return ERROR;

            const PersistenceType columnPersistenceType = foreignEndTable->GetPersistenceType() == PersistenceType::Persisted ? PersistenceType::Persisted : PersistenceType::Virtual;
            DbColumn* newFkCol = const_cast<DbTable*>(foreignEndTable)->CreateColumn(fkColName.c_str(), DbColumn::Type::Integer, fkColPosition, foreignKeyColumnKind, columnPersistenceType);
            if (newFkCol == nullptr)
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                                relClass.GetFullName(), fkColName.c_str(), foreignEndTable->GetName().c_str());
                BeAssert(false && "Could not create FK column for end table mapping");
                return ERROR;
                }

            const bool makeFkColNotNull = multiplicityImpliesNotNullOnFkCol && foreignEndTable->HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(foreignEndTable->GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
            if (makeFkColNotNull)
                newFkCol->GetConstraintsR().SetNotNullConstraint();

            ColumnLists::push_back(columns.m_fkColumnsPerFkTable, newFkCol);
            }
        }

    BeAssert(columns.m_fkColumnsPerFkTable.size() == foreignEndTables.size());

    ForeignKeyDbConstraint::ActionType userRequestedDeleteAction = classMappingInfo.GetOnDeleteAction();
    ForeignKeyDbConstraint::ActionType userRequestedUpdateAction = classMappingInfo.GetOnUpdateAction();

    ForeignKeyDbConstraint::ActionType onDelete = ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType onUpdate = ForeignKeyDbConstraint::ActionType::NotSpecified;

    if (userRequestedDeleteAction != ForeignKeyDbConstraint::ActionType::NotSpecified)
        onDelete = userRequestedDeleteAction;
    else
        {
        if (relClass.GetStrength() == StrengthType::Embedding)
            onDelete = ForeignKeyDbConstraint::ActionType::Cascade;
        else
            onDelete = ForeignKeyDbConstraint::ActionType::SetNull;
        }

    if (userRequestedUpdateAction != ForeignKeyDbConstraint::ActionType::NotSpecified)
        onUpdate = userRequestedUpdateAction;

    //+++ Other column(s) +++

    std::set<DbTable const*> referencedEndTables = GetReferencedEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables() : classMappingInfo.GetTargetTables();
    BeAssert(referencedEndTables.size() == 1);
    DbTable const* referencedTable = *referencedEndTables.begin();
    DbColumn const* referencedTablePKCol = referencedTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    if (referencedTablePKCol == nullptr)
        {
        BeAssert(referencedTablePKCol != nullptr);
        return ERROR;
        }

    Utf8String relECClassIdColName = DetermineRelECClassIdColumnName(relClass, fkColName);
    DbColumn const* referencedTableClassIdCol = referencedTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    for (DbColumn const* fkCol : columns.m_fkColumnsPerFkTable)
        {
        DbTable& fkTable = fkCol->GetTableR();
        const bool makeRelClassIdColNotNull = multiplicityImpliesNotNullOnFkCol && fkTable.IsOwnedByECDb() && fkTable.HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(fkTable.GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
        DbColumn* relClassIdCol = CreateRelECClassIdColumn(fkTable, relECClassIdColName.c_str(), makeRelClassIdColNotNull);
        if (relClassIdCol == nullptr)
            {
            BeAssert(false && "Could not create RelClassId col");
            return ERROR;
            }

        ColumnLists::push_back(columns.m_relECClassIdColumnsPerFkTable, relClassIdCol);
        DbColumn const* fkTableClassIdCol = fkTable.GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
        //If ForeignEndClassId column is missing create a virtual one
        if (fkTableClassIdCol == nullptr)
            {
            Utf8CP colName = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
            DbColumn::Kind kind = GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId;

            fkTableClassIdCol = fkTable.FindColumn(colName);
            if (fkTableClassIdCol == nullptr)
                {
                const bool readonly = !fkTable.GetEditHandle().CanEdit();
                if (readonly)
                    fkTable.GetEditHandleR().BeginEdit();

                fkTableClassIdCol = fkTable.CreateColumn(colName, DbColumn::Type::Integer, kind, PersistenceType::Virtual);

                if (readonly)
                    fkTable.GetEditHandleR().EndEdit();
                }
            else
                {
                if (fkTableClassIdCol->GetKind() != kind || fkTableClassIdCol->GetPersistenceType() != PersistenceType::Virtual)
                    {
                    BeAssert(false && "Expecting virtual column");
                    return ERROR;
                    }
                }
            }

        ColumnLists::push_back(columns.m_ecInstanceIdColumnsPerFkTable, fkTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId));
        ColumnLists::push_back(columns.m_classIdColumnsPerFkTable, fkTableClassIdCol);

        if (referencedTableClassIdCol != nullptr)
            ColumnLists::push_back(columns.m_referencedEndECClassIdColumns, referencedTableClassIdCol);
        else
            {
            //referenced table doesn't have a class id col --> create a virtual one in the foreign end table
            Utf8CP colName = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
            DbColumn::Kind kind = GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId;

            DbColumn const* fkTableReferencedEndClassIdCol = fkTable.FindColumn(colName);
            if (fkTableReferencedEndClassIdCol == nullptr)
                {
                const bool readonly = !fkTable.GetEditHandle().CanEdit();
                if (readonly)
                    fkTable.GetEditHandleR().BeginEdit();

                fkTableReferencedEndClassIdCol = fkTable.CreateColumn(colName, DbColumn::Type::Integer, kind, PersistenceType::Virtual);

                if (readonly)
                    fkTable.GetEditHandleR().EndEdit();
                }
            else
                {
                if (fkTableReferencedEndClassIdCol->GetKind() != kind || fkTableReferencedEndClassIdCol->GetPersistenceType() != PersistenceType::Virtual)
                    {
                    BeAssert(false && "Expecting virtual column");
                    return ERROR;
                    }
                }

            ColumnLists::push_back(columns.m_referencedEndECClassIdColumns, fkTableReferencedEndClassIdCol);
            }

        //if FK table is a joined table, CASCADE is not allowed as it would leave orphaned rows in the parent of joined table.
        if (fkTable.GetParentOfJoinedTable() != nullptr)
            {
            if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade ||
                (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified && relClass.GetStrength() == StrengthType::Embedding))
                {
                if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade)
                    Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its ForeignKeyRelationshipMap custom attribute specifies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());
                else
                    Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its strength is 'Embedding' which implies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());

                return ERROR;
                }
            }

        if (!fkTable.IsOwnedByECDb() || fkTable.GetPersistenceType() == PersistenceType::Virtual ||
            referencedTable->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        if (fkCol->IsShared())
            {
            Issues().Report(ECDbIssueSeverity::Warning, "The ECRelationshipClass '%s' implies a foreign key constraint. ECDb cannot create it though for the "
                            "column '%s' in table '%s' because the column is used by other properties. Consider disabling column sharing "
                            "(via ClassMap custom attribute) or redesigning the ECRelationshipClass without using a Key property.",
                            relClass.GetFullName(), fkCol->GetName().c_str(), fkTable.GetName().c_str());
            continue;
            }

        fkTable.CreateForeignKeyConstraint(*fkCol, *referencedTablePKCol, onDelete, onUpdate);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* RelationshipClassEndTableMap::CreateRelECClassIdColumn(DbTable& table, Utf8CP relClassIdColName, bool makeNotNull) const
    {
    BeAssert(!GetClass().HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");
    PersistenceType persType = PersistenceType::Persisted;
    if (table.GetPersistenceType() == PersistenceType::Virtual || !table.IsOwnedByECDb() || GetClass().GetClassModifier() != ECClassModifier::Abstract)
        persType = PersistenceType::Virtual;

    DbColumn* relClassIdCol = table.FindColumnP(relClassIdColName);
    if (relClassIdCol != nullptr)
        {
        BeAssert(Enum::Contains(relClassIdCol->GetKind(), DbColumn::Kind::RelECClassId));
        if (makeNotNull && !relClassIdCol->DoNotAllowDbNull())
            {
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();
            BeAssert(relClassIdCol->GetId().IsValid());
            }

        return relClassIdCol;
        }

    const bool canEdit = table.GetEditHandleR().CanEdit();
    if (!canEdit)
        table.GetEditHandleR().BeginEdit();

    relClassIdCol = table.CreateColumn(relClassIdColName, DbColumn::Type::Integer, DbColumn::Kind::RelECClassId, persType);
    if (relClassIdCol == nullptr)
        return nullptr;

    if (makeNotNull && !relClassIdCol->DoNotAllowDbNull())
        {
        relClassIdCol->GetConstraintsR().SetNotNullConstraint();
        BeAssert(relClassIdCol->GetId().IsValid());
        }

    if (persType != PersistenceType::Virtual)
        {
        Utf8String indexName("ix_");
        indexName.append(table.GetName()).append("_").append(relClassIdCol->GetName());
        DbIndex* index = GetDbMap().GetDbSchemaR().CreateIndex(table, indexName.c_str(), false, {relClassIdCol}, true, true, GetClass().GetId());
        if (index == nullptr)
            {
            LOG.errorv("Failed to create index on RelECClassId column %s on table %s.", relClassIdCol->GetName().c_str(), table.GetName().c_str());
            return nullptr;
            }
        }

    if (!canEdit)
        table.GetEditHandleR().EndEdit();

    return relClassIdCol;
    }


#define DEFAULT_FKCOLUMNNAME_PREFIX "ForeignECInstanceId_"
#define RELCLASSIDCOLUMNNAME_TERM "RelECClassId"

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String RelationshipClassEndTableMap::DetermineFkColumnName(RelationshipMappingInfo const& classMappingInfo, ForeignKeyColumnInfo const& fkColInfo) const
    {
    Utf8String fkColumnName;

    ECClassCR relClass = classMappingInfo.GetECClass();

    Utf8CP userProvidedFkColumnName = classMappingInfo.GetColumnsMapping(GetForeignEnd()).GetECInstanceIdColumnName();

    if (!Utf8String::IsNullOrEmpty(userProvidedFkColumnName))
        fkColumnName.assign(userProvidedFkColumnName);
    else if (fkColInfo.CanImplyFromNavigationProperty() && !fkColInfo.GetImpliedColumnName().empty())
        fkColumnName.assign(fkColInfo.GetImpliedColumnName());
    else
        {
        //default name: prefix_<schema alias>_<rel class name>
        fkColumnName.assign(DEFAULT_FKCOLUMNNAME_PREFIX).append(relClass.GetSchema().GetAlias()).append("_").append(relClass.GetName());
        }

    BeAssert(!fkColumnName.empty());
    return fkColumnName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String RelationshipClassEndTableMap::DetermineRelECClassIdColumnName(ECRelationshipClassCR relClass, Utf8StringCR fkColumnName)
    {
    Utf8String relECClassIdColName;
    if (fkColumnName.EndsWithIAscii("id"))
        {
        relECClassIdColName = fkColumnName.substr(0, fkColumnName.size() - 2);
        relECClassIdColName.append(RELCLASSIDCOLUMNNAME_TERM);
        }
    else if (fkColumnName.StartsWithIAscii(DEFAULT_FKCOLUMNNAME_PREFIX))
        relECClassIdColName.assign(RELCLASSIDCOLUMNNAME_TERM "_").append(relClass.GetSchema().GetAlias()).append("_").append(relClass.GetName());
    else
        relECClassIdColName.assign(fkColumnName).append(RELCLASSIDCOLUMNNAME_TERM);

    BeAssert(!relECClassIdColName.empty());
    return relECClassIdColName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::MapSubClass(RelationshipMappingInfo const& classMappingInfo)
    {
    if (GetClass().GetBaseClasses().size() != 1)
        {
        BeAssert(false && "Multi-inheritance of ECRelationshipclasses should have been caught before already");
        return ERROR;
        }

    ECClassCP baseClass = GetClass().GetBaseClasses()[0];
    ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
    if (baseClassMap == nullptr || baseClassMap->GetType() != ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "Could not find class map of base ECRelationship class or is not of right type");
        return ERROR;
        }

    RelationshipClassEndTableMap const& baseRelClassMap = static_cast<RelationshipClassEndTableMap const&>(*baseClassMap);
    //ECInstanceId property map
    SystemPropertyMap const* basePropMap = baseRelClassMap.GetECInstanceIdPropertyMap();
    if (basePropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

     //WIP Following is come from merge. classLoadCtx was passed to PropertyMapFactory::ClonePropertyMap. 
    //WIPClassMapLoadContext& classLoadCtx = GetDbMap().GetSchemaImportContext()->GetClassMapLoadContext();
    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*basePropMap, *this)) != SUCCESS)
        return ERROR;

    //ECClassId property map
    SystemPropertyMap const* classIdPropertyMap = baseRelClassMap.GetECClassIdPropertyMap();
    if (classIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*classIdPropertyMap,*this)) != SUCCESS)
        return ERROR;


    //ForeignEnd
    RelationshipConstraintMap const& baseForeignEndConstraintMap = baseRelClassMap.GetConstraintMap(GetForeignEnd());
    if (baseForeignEndConstraintMap.GetECInstanceIdPropMap() == nullptr || 
        baseForeignEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RelationshipConstraintMap& foreignEndConstraintMap = GetConstraintMapR(GetForeignEnd());
    
    //Foreign ECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseForeignEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECInstanceIdPropMap(static_cast<ConstraintECInstanceIdPropertyMap const*>(clonedConstraintInstanceId.get()));

    GetTablesPropertyMapVisitor tableDisp;
    clonedConstraintInstanceId->AcceptVisitor(tableDisp);
    for (DbTable const* table : tableDisp.GetTables())
        {
        AddTable(*const_cast<DbTable *>(table));
        }

    //Foreign ECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseForeignEndConstraintMap.GetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECClassIdPropMap(static_cast<ConstraintECClassIdPropertyMap const*>(clonedConstraintClassId.get()));

    //ReferencedEnd
    RelationshipConstraintMap const& baseReferencedEndConstraintMap = baseRelClassMap.GetConstraintMap(GetReferencedEnd());
    if (baseReferencedEndConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseReferencedEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RelationshipConstraintMap& referencedEndConstraintMap = GetConstraintMapR(GetReferencedEnd());

    //Referenced ECInstanceId prop map
    clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECInstanceIdPropMap(static_cast<ConstraintECInstanceIdPropertyMap const*>(clonedConstraintInstanceId.get()));

    //Referenced ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECClassIdPropMap(static_cast<ConstraintECClassIdPropertyMap const*>(clonedConstraintClassId.get()));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (ClassMap::_Load(ctx, mapInfo) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    ECRelationshipClassCR relationshipClass = GetRelationshipClass();
    ECClassId defaultSourceECClassId, defaultTargetECClassId;
    if (GetReferencedEnd() == ECRelationshipEnd_Source)
        {
        defaultSourceECClassId = relationshipClass.GetSource().GetClasses().empty() ? ECClassId() : relationshipClass.GetSource().GetClasses().front()->GetId();
        defaultTargetECClassId = relationshipClass.GetTarget().GetClasses().empty() ? ECClassId() : relationshipClass.GetTarget().GetClasses().front()->GetId();
        }
    else
        {
        defaultSourceECClassId = relationshipClass.GetTarget().GetClasses().empty() ? ECClassId() : relationshipClass.GetTarget().GetClasses().front()->GetId();
        defaultTargetECClassId = relationshipClass.GetSource().GetClasses().empty() ? ECClassId() : relationshipClass.GetSource().GetClasses().front()->GetId();
        }

    //SourceECInstanceId
    std::vector<DbColumn const*> const* mapColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize SourceECInstanceId property map");
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
    mapColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize SourceECClassId property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> sourceClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultSourceECClassId, ECRelationshipEnd_Source, *mapColumns);
    if (sourceClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceClassIdPropMap.get());

    //TargetECInstanceId
    mapColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize TargetECInstanceId property map");
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
    mapColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize TargetECClassId property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> targetClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultTargetECClassId, ECRelationshipEnd_Target, *mapColumns);
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

//--------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                   04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind fkColKind) const
    {
    DbTable& fkTable = fkColumn.GetTableR();

    if (fkColumn.DoNotAllowDbNull() != cardinalityImpliesNotNullOnFkCol)
        {
        Utf8CP error = nullptr;
        if (cardinalityImpliesNotNullOnFkCol)
            error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is nullable "
            "although the relationship's cardinality implies that the column is not nullable. Either modify the cardinality or mark the property specified that maps to the foreign key column as not nullable.";
        else
            error = "Failed to map ECRelationshipClass '%s'. It is mapped to an existing foreign key column which is not nullable "
            "although the relationship's cardinality implies that the column is nullable. Please modify the cardinality accordingly.";

        Issues().Report(ECDbIssueSeverity::Error, error, GetRelationshipClass().GetFullName());
        return ERROR;
        }

    const bool tableIsReadonly = !fkTable.GetEditHandle().CanEdit();
    if (tableIsReadonly)
        fkTable.GetEditHandleR().BeginEdit();

    //Kind of existing columns must be modified so that they also have the constraint ecinstanceid kind
    const_cast<DbColumn&>(fkColumn).AddKind(fkColKind);

    if (tableIsReadonly)
        fkTable.GetEditHandleR().EndEdit();

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo)
    {
    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&mapInfo) != nullptr);
    RelationshipMappingInfo const& relMapInfo = static_cast<RelationshipMappingInfo const&> (mapInfo);
    const bool isUniqueIndex = relMapInfo.GetCardinality() == RelationshipMappingInfo::Cardinality::OneToOne;

    if (!isUniqueIndex && m_hasKeyPropertyFk)
        return;

    BeAssert(GetReferencedEndECInstanceIdPropMap() != nullptr);
    std::vector<DbColumn const*> referencedEndIdColumns;

    for (SingleColumnDataPropertyMap const* vmap : GetReferencedEndECInstanceIdPropMap()->GetDataPropertyMaps())
        {
        DbTable& persistenceEndTable = const_cast<DbTable&>(vmap->GetColumn().GetTable());
        if (persistenceEndTable.GetType() == DbTable::Type::Existing)
            continue;

        // name of the index
        Utf8String name(isUniqueIndex ? "uix_" : "ix_");
        name.append(persistenceEndTable.GetName()).append("_fk_").append(GetClass().GetSchema().GetAlias() + "_" + GetClass().GetName());
        if (GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable)
            name.append("_source");
        else
            name.append("_target");

        GetDbMap().GetDbSchemaR().CreateIndex(persistenceEndTable, name.c_str(), isUniqueIndex, {&vmap->GetColumn()}, true, true, GetClass().GetId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ConstraintECInstanceIdPropertyMap const* RelationshipClassEndTableMap::GetForeignEndECInstanceIdPropMap() const
    {
    return GetConstraintMap(GetForeignEnd()).GetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ConstraintECInstanceIdPropertyMap const* RelationshipClassEndTableMap::GetReferencedEndECInstanceIdPropMap() const
    {
    return GetConstraintMap(GetReferencedEnd()).GetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ConstraintECClassIdPropertyMap const* RelationshipClassEndTableMap::GetReferencedEndECClassIdPropMap() const
    {
    return GetConstraintMap(GetReferencedEnd()).GetECClassIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetForeignEnd() const
    {
    return GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetReferencedEnd() const
    {
    return GetForeignEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          06/2015
//+---------------+---------------+---------------+---------------+---------------+------
void LogKeyPropertyRetrievalError(IssueReporter const& issueReporter, Utf8CP errorDetails, ECRelationshipClassCR relClass, ECRelationshipEnd constraintEnd)
    {
    issueReporter.Report(ECDbIssueSeverity::Error, "Invalid Key property on %s constraint in ECRelationshipClass '%s': %s",
                         constraintEnd == ECRelationshipEnd_Source ? "source" : "target", relClass.GetFullName(), errorDetails);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          06/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::TryGetKeyPropertyColumn(std::set<DbColumn const*>& keyPropertyColumns, ECRelationshipConstraintCR constraint, ECRelationshipClassCR relClass, ECRelationshipEnd constraintEnd) const
    {
    keyPropertyColumns.clear();
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    if (constraintClasses.size() == 0)
        return SUCCESS;

    Utf8String keyPropertyName;
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        bvector<Utf8String> const& keys = constraintClass->GetKeys();
        const size_t keyCount = keys.size();

        if (keyCount == 0)
            {
            if (keyPropertyName.empty())
                continue;

            LogKeyPropertyRetrievalError(Issues(), "ECRelationshipConstraint Key properties must be specified on all classes of the constraint or on none.",
                                         relClass, constraintEnd);
            return ERROR;
            }

        if (keyCount > 1 || keys[0].empty())
            {
            LogKeyPropertyRetrievalError(Issues(), "ECDb does not support ECRelationshipConstraint Keys that are empty or made up of multiple properties.",
                                         relClass, constraintEnd);
            return ERROR;
            }

        if (keyPropertyName.empty())
            keyPropertyName = keys.front().c_str();
        else
            {
            if (keyPropertyName != keys.front())
                {
                LogKeyPropertyRetrievalError(Issues(), "ECDb does not support ECRelationshipConstraint Keys with different accessStrings. All Key properties in constraint must have same name",
                                             relClass, constraintEnd);
                return ERROR;
                }
            }
        }

    if (keyPropertyName.empty())
        return SUCCESS;

    std::set<ClassMap const*> constraintMaps = GetDbMap().GetClassMapsFromRelationshipEnd(constraint, nullptr);
    for (auto constraintMap : constraintMaps)
        {
        Utf8CP keyPropAccessString = keyPropertyName.c_str();
        PropertyMap const* keyPropertyMap = constraintMap->GetPropertyMaps().Find(keyPropAccessString);
        if (keyPropertyMap == nullptr)
            {
            Utf8String error;
            error.Sprintf("Key property '%s' does not exist or is not mapped.", keyPropAccessString);
            LogKeyPropertyRetrievalError(Issues(), error.c_str(), relClass, constraintEnd);
            return ERROR;
            }

        ECSqlTypeInfo typeInfo(keyPropertyMap->GetProperty());
        if (!typeInfo.IsExactNumeric() && !typeInfo.IsString())
            {
            Utf8String error;
            error.Sprintf("Unsupported data type of Key property '%s'. ECDb only supports Key properties that have an integral or string data type.", keyPropAccessString);
            LogKeyPropertyRetrievalError(Issues(), error.c_str(), relClass, constraintEnd);
            return ERROR;
            }

        std::vector<DbColumn const*> columns;
        GetColumnsPropertyMapVisitor columnVisitor(constraintMap->GetJoinedTable());
        keyPropertyMap->AcceptVisitor(columnVisitor);
        if (columnVisitor.GetColumns().size() != 1)
            {
            BeAssert(false && "Key property map is expected to map to a single column.");
            return ERROR;
            }

        keyPropertyColumns.insert(columnVisitor.GetColumns().front());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          01/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::TryGetForeignKeyColumnInfoFromNavigationProperty(ForeignKeyColumnInfo& fkColInfo, ECN::ECRelationshipConstraintCR constraint, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd constraintEnd) const
    {
    fkColInfo.Clear();

    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    if (constraintClasses.size() == 0)
        return SUCCESS;

    const ECRelatedInstanceDirection expectedDirection = constraintEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? ECRelatedInstanceDirection::Forward : ECRelatedInstanceDirection::Backward;
    NavigationECPropertyCP singleNavProperty = nullptr;
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        for (ECPropertyCP prop : constraintClass->GetClass().GetProperties())
            {
            NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
            if (navProp != nullptr && navProp->GetRelationshipClass() == &relClass && navProp->GetDirection() == expectedDirection)
                {
                if (singleNavProperty == nullptr)
                    singleNavProperty = navProp;
                else
                    {
                    LOG.infov("More than one NavigationECProperty found on the %s constraint classes of the ECRelationship %s. Therefore the constraint column name cannot be implied from a navigation property. A default name will be picked.",
                              constraintEnd == ECRelationshipEnd_Source ? "source" : "target", relClass.GetFullName());
                    return SUCCESS;
                    }
                }
            }
        }

    //no nav prop found
    if (singleNavProperty == nullptr)
        return SUCCESS;

    bool isNullable, isUnique; //unused
    DbColumn::Constraints::Collation collation;//unused
    Utf8String columnName;
    if (SUCCESS != ClassMapper::DetermineColumnInfo(columnName, isNullable, isUnique, collation, GetECDb(), *singleNavProperty, singleNavProperty->GetName().c_str()))
        return ERROR;

    ClassMap const* classMap = GetDbMap().GetClassMap(singleNavProperty->GetClass());
    TablePerHierarchyInfo const& tphInfo = classMap->GetMapStrategy().GetTphInfo();
    if (tphInfo.IsValid() && tphInfo.UseSharedColumns())
        {
        //table uses shared columns, so FK col position cannot depend on NavigationProperty position
        fkColInfo.Assign(columnName.c_str(), true, nullptr, nullptr);
        return SUCCESS;
        }

    //now determine the property map defined just before the nav prop in the ECClass, that is mapped to
    PropertyMap const* precedingPropMap = nullptr;
    PropertyMap const* succeedingPropMap = nullptr;
    bool foundNavProp = false;
    SearchPropertyMapVisitor isSystemOrNavPropertyMapVisitor(Enum::Or(PropertyMap::Type::System, PropertyMap::Type::Navigation));
    for (PropertyMap const* propMap : classMap->GetPropertyMaps())
        {
        if (&propMap->GetProperty() == singleNavProperty)
            {
            foundNavProp = true;
            if (precedingPropMap != nullptr)
                break;

            //no preceding prop map exists, continue until suceeding prop map is found
            continue;
            }

        isSystemOrNavPropertyMapVisitor.Reset();
        propMap->AcceptVisitor(isSystemOrNavPropertyMapVisitor); //Skip system properties and navigation properties
        if (!isSystemOrNavPropertyMapVisitor.ResultSet().empty())
            continue;

        if (!foundNavProp)
            precedingPropMap = propMap;
        else
            {
            succeedingPropMap = propMap;
            break;
            }
        }

    fkColInfo.Assign(columnName.c_str(), false, precedingPropMap, succeedingPropMap);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::TryDetermineForeignKeyColumnPosition(int& position, DbTable const& table, ForeignKeyColumnInfo const& fkColInfo) const
    {
    position = -1;
    if (!fkColInfo.CanImplyFromNavigationProperty() || fkColInfo.AppendToEnd())
        return SUCCESS;

    GetColumnsPropertyMapVisitor precedingColsVisitor(table);
    PropertyMap const* precedingPropMap = fkColInfo.GetPropertyMapBeforeNavProp();
    if (precedingPropMap != nullptr)
        precedingPropMap->AcceptVisitor(precedingColsVisitor);

    bool isNeighborColumnPreceeding = true;
    DbColumn const* neighborColumn = nullptr;
    if (!precedingColsVisitor.GetColumns().empty())
        neighborColumn = precedingColsVisitor.GetColumns().back();
    else
        {

        PropertyMap const* succeedingPropMap = fkColInfo.GetPropertyMapAfterNavProp();
        GetColumnsPropertyMapVisitor succeedingColsVisitor(table);
        if (succeedingPropMap != nullptr)
            succeedingPropMap->AcceptVisitor(succeedingColsVisitor);

        if (!succeedingColsVisitor.GetColumns().empty())
            {
            neighborColumn = succeedingColsVisitor.GetColumns()[0];
            isNeighborColumnPreceeding = false;
            }
        }

    if (neighborColumn == nullptr)
        {
        DbColumn const* unused = nullptr;
        position = table.TryGetECClassIdColumn(unused) ? 2 : 1;
        return SUCCESS;
        }

    int i = 0;
    for (DbColumn const* col : table.GetColumns())
        {
        if (col == neighborColumn)
            break;

        i++;
        }

    if (isNeighborColumnPreceeding)
        position = i + 1;
    else
        position = i;

    return SUCCESS;
    }

//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, ECRelationshipClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty)
    : RelationshipClassMap(ecdb, Type::RelationshipLinkTable, ecRelClass, mapStrategy, setIsDirty)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MappingStatus RelationshipClassLinkTableMap::_Map(SchemaImportContext& context, ClassMappingInfo const& classMapInfo)
    {
    BeAssert(!MapStrategyExtendedInfo::IsForeignKeyMapping(GetMapStrategy()) &&
             "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");
    BeAssert(GetRelationshipClass().GetStrength() != StrengthType::Embedding && "Should have been caught already in ClassMapInfo");

    MappingStatus stat = DoMapPart1(context, classMapInfo);
    if (stat != MappingStatus::Success)
        return stat;

    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&classMapInfo) != nullptr);
    RelationshipMappingInfo const& relationClassMapInfo = static_cast<RelationshipMappingInfo const&> (classMapInfo);

    ECRelationshipClassCR relationshipClass = GetRelationshipClass();
    ECRelationshipConstraintCR sourceConstraint = relationshipClass.GetSource();
    ECRelationshipConstraintCR targetConstraint = relationshipClass.GetTarget();

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    ECClassId defaultSourceECClassId;
    DetermineConstraintClassIdColumnHandling(addSourceECClassIdColumnToTable, defaultSourceECClassId, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    ECClassId defaultTargetECClassId;
    DetermineConstraintClassIdColumnHandling(addTargetECClassIdColumnToTable, defaultTargetECClassId, targetConstraint);
    stat = CreateConstraintPropMaps(relationClassMapInfo, addSourceECClassIdColumnToTable, defaultSourceECClassId, addTargetECClassIdColumnToTable, defaultTargetECClassId);
    if (stat != MappingStatus::Success)
        return stat;

    stat = DoMapPart2(context, classMapInfo);
    if (stat != MappingStatus::Success)
        return stat;

    //only create constraints on TPH root or if not TPH and not existing table
    if (GetPrimaryTable().GetType() != DbTable::Type::Existing &&
        (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId())) 
        {
        RelationshipMappingInfo const& relationClassMapInfo = static_cast<RelationshipMappingInfo const&> (classMapInfo);

        //Create FK from Source-Primary to LinkTable
        DbTable const* sourceTable = *relationClassMapInfo.GetSourceTables().begin();
        DbColumn const* fkColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        DbColumn const* referencedColumn = sourceTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

        //Create FK from Target-Primary to LinkTable
        DbTable const* targetTable = *relationClassMapInfo.GetTargetTables().begin();
        fkColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        referencedColumn = targetTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);
        }


    AddIndices(context, classMapInfo);
    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(RelationshipMappingInfo const& mapInfo, ECRelationshipEnd relationshipEnd)
    {
    DbColumn* endECClassIdColumn = nullptr;
    ECRelationshipClassCP relationship = mapInfo.GetECClass().GetRelationshipClassCP();
    BeAssert(relationship != nullptr);
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? relationship->GetSource() : relationship->GetTarget();
    ECEntityClass const* foreignEndClass = foreignEndConstraint.GetClasses()[0];
    ClassMap const* foreignEndClassMap = GetDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetDbMap().GetTableCountOnRelationshipEnd(foreignEndConstraint);

    DbColumn::Kind columnId = relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId;
    RelationshipEndColumns const& constraintColumnsMapping = GetEndColumnsMapping(mapInfo, relationshipEnd);
    Utf8String columnName(constraintColumnsMapping.GetECClassIdColumnName());
    if (columnName.empty())
        {
        if (!GetConstraintECClassIdColumnName(columnName, relationshipEnd, GetPrimaryTable()))
            return nullptr;
        }

    if (ConstraintIncludesAnyClass(foreignEndConstraint.GetConstraintClasses()) || foreignEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), columnId, PersistenceType::Persisted);
        BeAssert(endECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        endECClassIdColumn = const_cast<DbColumn*>(foreignEndClassMap->GetPrimaryTable().GetFilteredColumnFirst(DbColumn::Kind::ECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), columnId, PersistenceType::Virtual);
        }

    return endECClassIdColumn;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
RelationshipClassMap::ReferentialIntegrityMethod RelationshipClassLinkTableMap::_GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return ReferentialIntegrityMethod::None;

    size_t nSourceTables = GetDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetSource());
    size_t nTargetTables = GetDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetTarget());

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing ||
        GetRelationshipClass().GetStrength() == StrengthType::Holding)
        {
        if (nSourceTables == 1 && nTargetTables == 1)
            {
            return ReferentialIntegrityMethod::ForeignKey;
            }
        }

    BeAssert(false && "Trigger not supported");
    return ReferentialIntegrityMethod::Trigger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(RelationshipMappingInfo const& mapInfo, bool addSourceECClassIdColumnToTable, ECClassId defaultSourceECClassId,
    bool addTargetECClassIdColumnToTable, ECClassId defaultTargetECClassId)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName(mapInfo.GetColumnsMapping(ECRelationshipEnd_Source).GetECInstanceIdColumnName());
    if (columnName.empty())
        {
        if (!GetConstraintECInstanceIdColumnName(columnName, ECRelationshipEnd_Source, GetPrimaryTable()))
            return MappingStatus::Error;
        }

    auto sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), DbColumn::Kind::SourceECInstanceId, PersistenceType::Persisted);
    if (sourceECInstanceIdColumn == nullptr)
        return MappingStatus::Error;

    auto sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceIdColumn});
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //**** SourceECClassId prop map
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultSourceECClassId, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = mapInfo.GetColumnsMapping(ECRelationshipEnd_Target).GetECInstanceIdColumnName();
    if (columnName.empty())
        {
        if (!GetConstraintECInstanceIdColumnName(columnName, ECRelationshipEnd_Target, GetPrimaryTable()))
            return MappingStatus::Error;
        }

    auto targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), DbColumn::Kind::TargetECInstanceId, PersistenceType::Persisted);
    if (targetECInstanceIdColumn == nullptr)
        return MappingStatus::Error;

    auto targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceIdColumn});
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MappingStatus::Error);
    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());


    //**** TargetECClassId prop map
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Target);
    auto targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultTargetECClassId , ECRelationshipEnd_Target, {targetECClassIdColumn});
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(targetECClassIdPropMap != nullptr);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());
    return MappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndices(SchemaImportContext& schemaImportContext, ClassMappingInfo const& mapInfo)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;

    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&mapInfo) != nullptr);
    RelationshipMappingInfo const& relationshipClassMapInfo = static_cast<RelationshipMappingInfo const&> (mapInfo);

    RelationshipMappingInfo::Cardinality cardinality = relationshipClassMapInfo.GetCardinality();
    const bool enforceUniqueness = !relationshipClassMapInfo.AllowDuplicateRelationships();

    // Add indices on the source and target based on cardinality
    bool sourceIsUnique = enforceUniqueness;
    bool targetIsUnique = enforceUniqueness;

    switch (cardinality)
        {
        //the many side can be unique, but the one side must never be unique
            case RelationshipMappingInfo::Cardinality::OneToMany:
                sourceIsUnique = false;
                break;
            case RelationshipMappingInfo::Cardinality::ManyToOne:
                targetIsUnique = false;
                break;

            case RelationshipMappingInfo::Cardinality::ManyToMany:
                sourceIsUnique = false;
                targetIsUnique = false;
                break;
            default:
                break;
        }

    AddIndex(schemaImportContext, RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndex(schemaImportContext, RelationshipIndexSpec::Target, targetIsUnique);

    if (enforceUniqueness)
        AddIndex(schemaImportContext, RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndex(SchemaImportContext& schemaImportContext, RelationshipIndexSpec spec, bool isUniqueIndex)
    {
    // Setup name of the index
    Utf8String name;
    if (isUniqueIndex)
        name.append("uix_");
    else
        name.append("ix_");

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
    auto sourceECClassIdColumn =  GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;
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

    GetDbMap().GetDbSchemaR().CreateIndex(GetPrimaryTable(), name.c_str(), isUniqueIndex, columns, false,
                                            true, GetClass().GetId(),
                                            //if a partial index is created, it must only apply to this class,
                                            //not to subclasses, as constraints are not inherited by relationships
                                            false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::GenerateIndexColumnList(std::vector<DbColumn const*>& columns, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4)
    {
    if (nullptr != col1 && col1->GetPersistenceType() == PersistenceType::Persisted)
        columns.push_back(col1);

    if (nullptr != col2 && col2->GetPersistenceType() == PersistenceType::Persisted)
        columns.push_back(col2);

    if (nullptr != col3 && col3->GetPersistenceType() == PersistenceType::Persisted)
        columns.push_back(col3);

    if (nullptr != col4 && col4->GetPersistenceType() == PersistenceType::Persisted)
        columns.push_back(col4);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECInstanceIdColumnName(Utf8StringR columnName, ECRelationshipEnd relationshipEnd, DbTable const& table) const
    {
    if (columnName.empty())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME : ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;

    if (table.FindColumn(columnName.c_str()) == nullptr)
        return true;

    if (GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy || GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        return true;

    //Following error occure in Upgrading ECSchema but is not fatal.
    LOG.errorv("Table %s already contains column named %s. ECRelationship %s has failed to map.",
               table.GetName().c_str(), columnName.c_str(), GetClass().GetFullName());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECClassIdColumnName(Utf8StringR columnName, ECRelationshipEnd relationshipEnd, DbTable const& table) const
    {
    if (columnName.empty())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;

    if (table.FindColumn(columnName.c_str()) == nullptr)
        return true;

    if (GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy || GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        return true;

    //Following error occurs in Upgrading ECSchema but is not fatal.
    LOG.errorv("Table %s already contains column named %s. ECRelationship %s has failed to map.",
               table.GetName().c_str(), columnName.c_str(), GetClass().GetFullName());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipClassLinkTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (ClassMap::_Load(ctx, mapInfo) != BentleyStatus::SUCCESS)
        return ERROR;

    ECRelationshipClassCR relationshipClass = GetRelationshipClass();
    auto const& sourceConstraint = relationshipClass.GetSource();
    auto const& targetConstraint = relationshipClass.GetTarget();

    ECClassId defaultSourceECClassId = sourceConstraint.GetClasses().empty() ? ECClassId() : sourceConstraint.GetClasses().front()->GetId();
    ECClassId defaultTargetECClassId = targetConstraint.GetClasses().empty() ? ECClassId() : targetConstraint.GetClasses().front()->GetId();

    auto mapColumns = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());


    mapColumns = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultSourceECClassId, ECRelationshipEnd_Source, *mapColumns);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ERROR);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultTargetECClassId, ECRelationshipEnd_Target, *mapColumns);
    PRECONDITION(targetECClassIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());

    return BentleyStatus::SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
