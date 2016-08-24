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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassMap::RelationshipClassMap(Type type, ECRelationshipClassCR ecRelClass, ECDbMap const& ecDbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty)
    : ClassMap(type, ecRelClass, ecDbMap, mapStrategy, setIsDirty),
    m_sourceConstraintMap(ecDbMap, ecRelClass.GetId(), ECRelationshipEnd_Source, ecRelClass.GetSource()),
    m_targetConstraintMap(ecDbMap, ecRelClass.GetId(), ECRelationshipEnd_Target, ecRelClass.GetTarget())
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
PropertyMapCP RelationshipClassMap::GetConstraintECInstanceIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECInstanceIdPropMap();
    else
        return GetTargetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
RelConstraintECClassIdPropertyMap const* RelationshipClassMap::GetConstraintECClassIdPropMap(ECRelationshipEnd constraintEnd) const
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
    RelConstraintECClassIdPropertyMap const* referencedEndClassIdPropertyMap = endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECClassIdPropMap() : GetTargetECClassIdPropMap();
    return !referencedEndClassIdPropertyMap->IsVirtual() && !referencedEndClassIdPropertyMap->IsMappedToClassMapTables();
    }

//************************ RelationshipConstraintMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
RelationshipConstraintMap::RelationshipConstraintMap(ECDbMap const& ecdbMap, ECN::ECClassId const& relClassId, ECN::ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint) : m_ecdbMap(ecdbMap), m_relClassId(relClassId), m_constraintEnd(constraintEnd), m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr)
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

    bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_ecdbMap.GetLightweightCache().GetConstraintClassesForRelationshipClass(m_relClassId);
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

    bmap<ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_ecdbMap.GetLightweightCache().GetConstraintClassesForRelationshipClass(m_relClassId);

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
RelationshipClassEndTableMap::RelationshipClassEndTableMap(ECRelationshipClassCR ecRelClass, ECDbMap const& ecDbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty)
    : RelationshipClassMap(Type::RelationshipEndTable, ecRelClass, ecDbMap, mapStrategy, setIsDirty), m_hasKeyPropertyFk(false)
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
    if (!referencedEndClassIdPropertyMap->IsVirtual() && !referencedEndClassIdPropertyMap->IsMappedToClassMapTables())
        return true;

    std::vector<DbColumn const*> sourceColumns, targetColumns;
    GetSourceECClassIdPropMap()->GetColumns(sourceColumns);
    GetTargetECClassIdPropMap()->GetColumns(targetColumns);

    //SELF JOIN case
    if (sourceColumns.size() == 1 && targetColumns.size() == 1)
        {
        return  sourceColumns.front() == targetColumns.front()
            && sourceColumns.front()->GetPersistenceType() == PersistenceType::Persisted
            && targetColumns.front()->GetPersistenceType() == PersistenceType::Persisted;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus RelationshipClassEndTableMap::_Map(SchemaImportContext& ctx, ClassMappingInfo const& classMapInfo)
    {   
    //Don't call base class method as end table map requires its own handling
    BeAssert(GetClass().GetRelationshipClassCP() != nullptr && classMapInfo.GetMapStrategy().IsForeignKeyMapping());
    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&classMapInfo) != nullptr);
    RelationshipMappingInfo const& relClassMappingInfo = static_cast<RelationshipMappingInfo const&> (classMapInfo);

    ClassMap const* baseClassMap = classMapInfo.GetBaseClassMap();
    if (baseClassMap != nullptr)
        return MapSubClass(relClassMappingInfo, *baseClassMap) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;

    //root class (no base class) mapping

    ColumnLists columns;
    if (SUCCESS != DetermineKeyAndConstraintColumns(columns, relClassMappingInfo))
        return MappingStatus::Error;

    //Create ECInstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    PropertyMapPtr ecInstanceIdPropMap = ECInstanceIdPropertyMap::Create(Schemas(), *this, columns.m_ecInstanceIdColumnsPerFkTable);
    if (ecInstanceIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMapECInstanceId");
        return MappingStatus::Error;
        }

    //Create ECInstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    PropertyMapPtr ecClassIdPropMap = ECClassIdPropertyMap::Create(Schemas(), *this, columns.m_relECClassIdColumnsPerFkTable);
    if (ecClassIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create ECClassIdPropertyMap");
        return MappingStatus::Error;
        }

    //Set tables
    for (DbColumn const* fkTablePkCol : columns.m_ecInstanceIdColumnsPerFkTable)
        {
        SetTable(fkTablePkCol->GetTableR(), true);
        }

    //Add primary key property map
    if (GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    //Add primary key property map
    if (GetPropertyMapsR().AddPropertyMap(ecClassIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    //ForeignEnd ECInstanceId PropMap
    Utf8CP fkTableColAlias = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME : ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;
    PropertyMapPtr foreignEndIdPropertyMap = RelationshipConstraintECInstanceIdPropertyMap::Create(GetForeignEnd(), Schemas(), columns.m_ecInstanceIdColumnsPerFkTable, fkTableColAlias);
    if (foreignEndIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().AddPropertyMap(foreignEndIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECInstanceIdPropMap(foreignEndIdPropertyMap.get());


    //Create ForeignEnd ClassId propertyMap
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();

    fkTableColAlias = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    RefCountedPtr<RelConstraintECClassIdPropertyMap> foreignEndClassIdPropertyMap = RelConstraintECClassIdPropertyMap::Create(GetForeignEnd(), Schemas(), columns.m_classIdColumnsPerFkTable,
                                                                                                                                                referencedEndConstraint.GetClasses()[0]->GetId(), *this, fkTableColAlias);

    if (foreignEndClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().AddPropertyMap(foreignEndClassIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECClassIdPropMap(foreignEndClassIdPropertyMap.get());

    //FK PropMap (aka referenced end id prop map)
    PropertyMapPtr fkPropertyMap = RelationshipConstraintECInstanceIdPropertyMap::Create(GetReferencedEnd(), Schemas(), columns.m_fkColumnsPerFkTable);
    if (fkPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().AddPropertyMap(fkPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECInstanceIdPropMap(fkPropertyMap.get());


    //FK ClassId PropMap (aka referenced end classid prop map)
    fkTableColAlias = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    RefCountedPtr<RelConstraintECClassIdPropertyMap> fkClassIdPropertyMap = RelConstraintECClassIdPropertyMap::Create(GetReferencedEnd(), Schemas(), columns.m_referencedEndECClassIdColumns,
                                                                                                                                        foreignEndConstraint.GetClasses()[0]->GetId(), *this, fkTableColAlias);
    if (fkClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    if (GetPropertyMapsR().AddPropertyMap(fkClassIdPropertyMap) != SUCCESS)
        return MappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECClassIdPropMap(fkClassIdPropertyMap.get());

    //add non-system property maps
    AddPropertyMaps(ctx.GetClassMapLoadContext(), baseClassMap, nullptr, &classMapInfo);
    AddIndexToRelationshipEnd(ctx, classMapInfo);
    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns(ColumnLists& columns, RelationshipMappingInfo const& classMappingInfo)
    {
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    std::set<DbTable const*> foreignEndTables = GetForeignEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables() : classMappingInfo.GetTargetTables();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();

    //! table must meet following constraint though these are already validated at MapStrategy evaluation time.
    BeAssert(foreignEndTables.size() >= 1 && "ForeignEnd Tables must be >= 1");
    BeAssert(GetReferencedEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables().size() == 1 : classMappingInfo.GetTargetTables().size() == 1 && "ReferencedEnd Tables must be == 1");


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
    const bool cardinalityImpliesNotNullOnFkCol = referencedEndConstraint.GetCardinality().GetLowerLimit() > 0;

    Utf8String fkColName;
    if (!fkTableFkColSet.empty())
        {
        m_hasKeyPropertyFk = true;
        for (DbColumn const* fkCol : fkTableFkColSet)
            {
            if (SUCCESS != ValidateForeignKeyColumn(*fkCol, cardinalityImpliesNotNullOnFkCol, foreignKeyColumnKind))
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
                    if (SUCCESS != ValidateForeignKeyColumn(*fkCol, cardinalityImpliesNotNullOnFkCol, foreignKeyColumnKind))
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

            const PersistenceType columnPersistenceType = foreignEndTable->IsOwnedByECDb() && foreignEndTable->GetPersistenceType() == PersistenceType::Persisted ? PersistenceType::Persisted : PersistenceType::Virtual;
            fkCol = const_cast<DbTable*>(foreignEndTable)->CreateColumn(fkColName.c_str(), DbColumn::Type::Integer, fkColPosition, foreignKeyColumnKind, columnPersistenceType);
            if (fkCol == nullptr)
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                                relClass.GetFullName(), fkColName.c_str(), foreignEndTable->GetName().c_str());
                BeAssert(false && "Could not create FK column for end table mapping");
                return ERROR;
                }

            ColumnLists::push_back(columns.m_fkColumnsPerFkTable, fkCol);
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
    const PersistenceType relECClassIdColPersType = relClass.GetDerivedClasses().empty() ? PersistenceType::Virtual : PersistenceType::Persisted;

    DbColumn const* referencedTableClassIdCol = referencedTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    for (DbColumn const* fkCol : columns.m_fkColumnsPerFkTable)
        {
        DbTable& fkTable = fkCol->GetTableR();

        DbColumn* relClassIdCol = CreateRelECClassIdColumn(fkTable, relECClassIdColName.c_str(), relECClassIdColPersType);
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
DbColumn* RelationshipClassEndTableMap::CreateRelECClassIdColumn(DbTable& table, Utf8CP relClassIdColName, PersistenceType persType) const
    {
    if (table.GetPersistenceType() == PersistenceType::Virtual || !table.IsOwnedByECDb())
        persType = PersistenceType::Virtual;

    DbColumn* relClassIdCol = table.FindColumnP(relClassIdColName);
    if (relClassIdCol != nullptr)
        {
        BeAssert(Enum::Contains(relClassIdCol->GetKind(), DbColumn::Kind::RelECClassId));
        return relClassIdCol;
        }

    const bool canEdit = table.GetEditHandleR().CanEdit();
    if (!canEdit)
        table.GetEditHandleR().BeginEdit();

    relClassIdCol = table.CreateColumn(relClassIdColName, DbColumn::Type::Integer, DbColumn::Kind::RelECClassId, persType);
    if (relClassIdCol == nullptr)
        return nullptr;

    if (persType != PersistenceType::Virtual)
        {
        Utf8String indexName("ix_");
        indexName.append(table.GetName()).append("_").append(relClassIdCol->GetName());
        DbIndex* index = GetECDbMap().GetDbSchemaR().CreateIndex(table, indexName.c_str(), false, {relClassIdCol}, true, true, GetClass().GetId());
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
        //default name: prefix_<schema namespace prefix>_<rel class name>
        fkColumnName.assign(DEFAULT_FKCOLUMNNAME_PREFIX).append(relClass.GetSchema().GetNamespacePrefix()).append("_").append(relClass.GetName());
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
        relECClassIdColName.assign(RELCLASSIDCOLUMNNAME_TERM "_").append(relClass.GetSchema().GetNamespacePrefix()).append("_").append(relClass.GetName());
    else
        relECClassIdColName.assign(fkColumnName).append(RELCLASSIDCOLUMNNAME_TERM);

    BeAssert(!relECClassIdColName.empty());
    return relECClassIdColName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::MapSubClass(RelationshipMappingInfo const& classMappingInfo, ClassMap const& baseClassMap)
    {
    BeAssert(dynamic_cast<RelationshipClassEndTableMap const*>(&baseClassMap) != nullptr);
    RelationshipClassEndTableMap const& baseRelClassMap = static_cast<RelationshipClassEndTableMap const&>(baseClassMap);

    //ECInstanceId property map
    PropertyMapCP basePropMap = baseRelClassMap.GetECInstanceIdPropertyMap();
    if (basePropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *basePropMap, GetClass(), nullptr)) != SUCCESS)
        return ERROR;

    //ECClassId property map
    PropertyMapCP classIdPropertyMap = baseRelClassMap.GetECClassIdPropertyMap();
    if (classIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *classIdPropertyMap, GetClass(), nullptr)) != SUCCESS)
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
    PropertyMapPtr clonedPropMap = PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *baseForeignEndConstraintMap.GetECInstanceIdPropMap(), GetClass(), nullptr);
    if (GetPropertyMapsR().AddPropertyMap(clonedPropMap) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECInstanceIdPropMap(clonedPropMap.get());

    for (DbTable const* table : clonedPropMap->GetTables())
        {
        SetTable(*const_cast<DbTable*>(table), true);
        }

    //Foreign ECClassId prop map
    clonedPropMap = PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *baseForeignEndConstraintMap.GetECClassIdPropMap(), GetClass(), nullptr);
    if (GetPropertyMapsR().AddPropertyMap(clonedPropMap) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECClassIdPropMap(static_cast<RelConstraintECClassIdPropertyMap const*> (clonedPropMap.get()));

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
    clonedPropMap = PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *baseReferencedEndConstraintMap.GetECInstanceIdPropMap(), GetClass(), nullptr);
    if (GetPropertyMapsR().AddPropertyMap(clonedPropMap) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECInstanceIdPropMap(clonedPropMap.get());

    //Referenced ECClassId prop map
    clonedPropMap = PropertyMapFactory::ClonePropertyMap(GetECDbMap(), *baseReferencedEndConstraintMap.GetECClassIdPropMap(), GetClass(), nullptr);
    if (GetPropertyMapsR().AddPropertyMap(clonedPropMap) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECClassIdPropMap(static_cast<RelConstraintECClassIdPropertyMap const*> (clonedPropMap.get()));

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
    std::vector<DbColumn const*> const* propertyinfo = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize SourceECInstanceId property map");
        return ERROR;
        }

    PropertyMapPtr sourceECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), *propertyinfo,
                                                                                                     ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (sourceECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //SourceECClassId
    propertyinfo = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize SourceECClassId property map");
        return ERROR;
        }

    auto sourceClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), *propertyinfo, defaultSourceECClassId, *this, ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (sourceClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(sourceClassIdPropMap) != SUCCESS)
        return ERROR;
    m_sourceConstraintMap.SetECClassIdPropMap(sourceClassIdPropMap.get());

    //TargetECInstanceId
    propertyinfo = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize TargetECInstanceId property map");
        return ERROR;
        }

    auto targetECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), *propertyinfo,
                                                                                           ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (targetECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    //TargetECClassId
    propertyinfo = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize TargetECClassId property map");
        return ERROR;
        }

    auto targetClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), *propertyinfo, defaultTargetECClassId, *this, ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (targetClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().AddPropertyMap(targetClassIdPropMap) != SUCCESS)
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

    if (!relMapInfo.CreateIndexOnForeignKey() ||
        (!isUniqueIndex && m_hasKeyPropertyFk))
        return;

    BeAssert(GetReferencedEndECInstanceIdPropMap() != nullptr);
    std::vector<DbColumn const*> referencedEndIdColumns;
    GetReferencedEndECInstanceIdPropMap()->GetColumns(referencedEndIdColumns);
    for (DbColumn const* referencedEndIdColumn : referencedEndIdColumns)
        {
        DbTable& persistenceEndTable = const_cast<DbTable&>(referencedEndIdColumn->GetTable());
        if (persistenceEndTable.GetType() == DbTable::Type::Existing)
            continue;

        // name of the index
        Utf8String name(isUniqueIndex ? "uix_" : "ix_");
        name.append(persistenceEndTable.GetName()).append("_fk_").append(GetClass().GetSchema().GetNamespacePrefix() + "_" + GetClass().GetName());
        if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable)
            name.append("_source");
        else
            name.append("_target");

        GetECDbMap().GetDbSchemaR().CreateIndex(persistenceEndTable, name.c_str(), isUniqueIndex, {referencedEndIdColumn}, true, true, GetClass().GetId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetForeignEndECInstanceIdPropMap() const
    {
    return GetConstraintMap(GetForeignEnd()).GetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetReferencedEndECInstanceIdPropMap() const
    {
    return GetConstraintMap(GetReferencedEnd()).GetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
RelConstraintECClassIdPropertyMap const* RelationshipClassEndTableMap::GetReferencedEndECClassIdPropMap() const
    {
    return GetConstraintMap(GetReferencedEnd()).GetECClassIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetForeignEnd() const
    {
    return GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
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

    std::set<ClassMap const*> constraintMaps = GetECDbMap().GetClassMapsFromRelationshipEnd(constraint, nullptr);
    for (auto constraintMap : constraintMaps)
        {
        Utf8CP keyPropAccessString = keyPropertyName.c_str();
        PropertyMap const* keyPropertyMap = constraintMap->GetPropertyMap(keyPropAccessString);
        if (keyPropertyMap == nullptr || keyPropertyMap->IsVirtual())
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
        keyPropertyMap->GetColumns(columns);
        if (columns.size() != 1)
            {
            BeAssert(false && "Key property map is expected to map to a single column.");
            return ERROR;
            }

        keyPropertyColumns.insert(columns[0]);
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
    if (SUCCESS != PropertyMap::DetermineColumnInfo(columnName, isNullable, isUnique, collation, GetECDbMap().GetECDb(), *singleNavProperty, singleNavProperty->GetName().c_str()))
        return ERROR;

    ClassMap const* classMap = GetECDbMap().GetClassMap(singleNavProperty->GetClass());
    if (classMap->GetColumnFactory().UsesSharedColumnStrategy())
        {
        //table uses shared columns, so FK col position cannot depend on NavigationProperty position
        fkColInfo.Assign(columnName.c_str(), true, nullptr, nullptr);
        return SUCCESS;
        }

    //now determine the property map defined just before the nav prop in the ECClass, that is mapped to
    PropertyMapCP precedingPropMap = nullptr;
    PropertyMapCP succeedingPropMap = nullptr;
    bool foundNavProp = false;
    for (PropertyMapCP propMap : classMap->GetPropertyMaps())
        {
        if (&propMap->GetProperty() == singleNavProperty)
            {
            foundNavProp = true;
            if (precedingPropMap != nullptr)
                break;

            //no preceding prop map exists, continue until suceeding prop map is found
            continue;
            }

        if (propMap->IsSystemPropertyMap() || propMap->GetType() == PropertyMap::Type::Navigation)
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

    PropertyMapCP precedingPropMap = fkColInfo.GetPropertyMapBeforeNavProp();
    std::vector<DbColumn const*> precedingCols;
    if (precedingPropMap != nullptr)
        precedingPropMap->GetColumns(precedingCols, table);

    bool isNeighborColumnPreceeding = true;
    DbColumn const* neighborColumn = nullptr;
    if (!precedingCols.empty())
        neighborColumn = precedingCols.back();
    else
        {
        PropertyMapCP succeedingPropMap = fkColInfo.GetPropertyMapAfterNavProp();
        std::vector<DbColumn const*> succeedingCols;
        if (succeedingPropMap != nullptr)
            succeedingPropMap->GetColumns(succeedingCols, table);

        if (!succeedingCols.empty())
            {
            neighborColumn = succeedingCols[0];
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
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECRelationshipClassCR ecRelClass, ECDbMap const& ecDbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty)
    : RelationshipClassMap(Type::RelationshipLinkTable, ecRelClass, ecDbMap, mapStrategy, setIsDirty)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MappingStatus RelationshipClassLinkTableMap::_Map(SchemaImportContext& context, ClassMappingInfo const& classMapInfo)
    {
    BeAssert(!GetMapStrategy().IsForeignKeyMapping() &&
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

    if (GetPrimaryTable().GetType() != DbTable::Type::Existing &&
        classMapInfo.GetBaseClassMap() == nullptr) //if subclass we must not create any FK anymore, as the base class mapping did that already
        {
        RelationshipMappingInfo const& relationClassMapInfo = static_cast<RelationshipMappingInfo const&> (classMapInfo);

        //Create FK from Source-Primary to LinkTable
        DbTable const* sourceTable = *relationClassMapInfo.GetSourceTables().begin();
        DbColumn const* fkColumn = GetSourceECInstanceIdPropMap()->GetSingleColumn();
        DbColumn const* referencedColumn = sourceTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

        //Create FK from Target-Primary to LinkTable
        DbTable const* targetTable = *relationClassMapInfo.GetTargetTables().begin();
        fkColumn = GetTargetECInstanceIdPropMap()->GetSingleColumn();
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
    ClassMap const* foreignEndClassMap = GetECDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetECDbMap().GetTableCountOnRelationshipEnd(foreignEndConstraint);

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

    size_t nSourceTables = GetECDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetSource());
    size_t nTargetTables = GetECDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetTarget());

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

    auto sourceECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), {sourceECInstanceIdColumn});
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MappingStatus::Error);
    sourceECInstanceIdPropMap->FindOrCreateColumnsInTable(*this);
    if (GetPropertyMapsR().AddPropertyMap(sourceECInstanceIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //**** SourceECClassId prop map
    auto sourceECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdColumnAlias = sourceECClassIdColumn->GetName().EqualsI(ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME) == true ? nullptr : ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME;
    auto sourceECClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), {sourceECClassIdColumn}, defaultSourceECClassId, *this, sourceECClassIdColumnAlias);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MappingStatus::Error);
    sourceECClassIdPropMap->FindOrCreateColumnsInTable(*this);
    if (GetPropertyMapsR().AddPropertyMap(sourceECClassIdPropMap) != SUCCESS)
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

    auto targetECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), {targetECInstanceIdColumn});
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MappingStatus::Error);
    targetECInstanceIdPropMap->FindOrCreateColumnsInTable(*this);
    if (GetPropertyMapsR().AddPropertyMap(targetECInstanceIdPropMap) != SUCCESS)
        return MappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());


    //**** TargetECClassId prop map
    auto targetECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Target);
    auto targetECClassIdColumnAlias = targetECClassIdColumn->GetName().EqualsI(ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME) == true ? nullptr : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    auto targetECClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), {targetECClassIdColumn}, defaultTargetECClassId, *this, targetECClassIdColumnAlias);
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(targetECClassIdPropMap != nullptr);
        return MappingStatus::Error;
        }

    targetECClassIdPropMap->FindOrCreateColumnsInTable(*this);
    if (GetPropertyMapsR().AddPropertyMap(targetECClassIdPropMap) != SUCCESS)
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

    name.append(GetClass().GetSchema().GetNamespacePrefix()).append("_").append(GetClass().GetName()).append("_");

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

    auto sourceECInstanceIdColumn = GetSourceECInstanceIdPropMap()->GetSingleColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->IsMappedToClassMapTables() ? GetSourceECClassIdPropMap()->GetSingleColumn() : nullptr;
    auto targetECInstanceIdColumn = GetTargetECInstanceIdPropMap()->GetSingleColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->IsMappedToClassMapTables() ? GetTargetECClassIdPropMap()->GetSingleColumn() : nullptr;

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

    GetECDbMap().GetDbSchemaR().CreateIndex(GetPrimaryTable(), name.c_str(), isUniqueIndex, columns, false,
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

    if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable || GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable)
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

    if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable || GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable)
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

    auto pm = mapInfo.FindColumnByAccessString(ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), *pm);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().AddPropertyMap(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());


    pm = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Source, Schemas(), *pm, defaultSourceECClassId, *this, ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ERROR);
    if (GetPropertyMapsR().AddPropertyMap(sourceECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());

    pm = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECInstanceIdPropMap = RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), *pm);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().AddPropertyMap(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    pm = mapInfo.FindColumnByAccessString( ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECClassIdPropMap = RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd_Target, Schemas(), *pm, defaultTargetECClassId, *this, ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    PRECONDITION(targetECClassIdPropMap.IsValid(), BentleyStatus::ERROR);
    if (GetPropertyMapsR().AddPropertyMap(targetECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());

    return BentleyStatus::SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
