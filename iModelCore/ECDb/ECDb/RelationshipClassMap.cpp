/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <array>
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
// @bsimethod                                 Affan.Khan                        12/13
//---------------------------------------------------------------------------------------
//static
bool RelationshipClassMap::ConstraintIncludesAnyClass(ECN::ECRelationshipConstraintClassList const& constraintClasses)
    {
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (IsAnyClass(*constraintClass))
            return true;
        }

    return false;
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


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                    02/2016
//---------------------------------------------------------------------------------------
bool RelationshipClassMap::_RequiresJoin(ECN::ECRelationshipEnd endPoint) const
    {
    ConstraintECClassIdPropertyMap const* referencedEndClassIdPropertyMap = endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECClassIdPropMap() : GetTargetECClassIdPropMap();
    if (referencedEndClassIdPropertyMap->GetDataPropertyMaps().size() != 1)
        {
        BeAssert(false && "Expecting exactly one property map");
        return false;
        }

    SingleColumnDataPropertyMap const* vm = referencedEndClassIdPropertyMap->GetDataPropertyMaps().front()->GetAs<SingleColumnDataPropertyMap>();
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
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy, setIsDirty) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
RelationshipClassMap::ReferentialIntegrityMethod RelationshipClassEndTableMap::_GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing || GetRelationshipClass().GetClassModifier() == ECClassModifier::Abstract
        || GetRelationshipClass().GetSource().GetConstraintClasses().empty() || GetRelationshipClass().GetTarget().GetConstraintClasses().empty())
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

    SingleColumnDataPropertyMap const* vm = referencedEndClassIdPropertyMap->GetDataPropertyMaps().front()->GetAs<SingleColumnDataPropertyMap>();
    if (vm->GetColumn().GetPersistenceType() != PersistenceType::Virtual && !vm->IsMappedToClassMapTables())
        return true;

    //SELF JOIN case
    if (GetSourceECClassIdPropMap()->IsMappedToSingleTable() && GetTargetECClassIdPropMap()->IsMappedToSingleTable())
        {
        auto source = GetSourceECClassIdPropMap()->GetDataPropertyMaps().front();
        auto target = GetTargetECClassIdPropMap()->GetDataPropertyMaps().front();

        return  &source->GetColumn() == &target->GetColumn()
            && source->GetColumn().GetPersistenceType() == PersistenceType::Physical;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::_Map(ClassMappingContext& ctx)
    {   
    //Don't call base class method as end table map requires its own handling
    BeAssert(GetClass().GetRelationshipClassCP() != nullptr && MapStrategyExtendedInfo::IsForeignKeyMapping(ctx.GetClassMappingInfo().GetMapStrategy()));
    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&ctx.GetClassMappingInfo()) != nullptr);
    RelationshipMappingInfo const& relClassMappingInfo = static_cast<RelationshipMappingInfo const&> (ctx.GetClassMappingInfo());

    if (GetClass().HasBaseClasses())
        return MapSubClass(relClassMappingInfo) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

    //root class (no base class)

    ColumnLists columns(*this, relClassMappingInfo);
    if (SUCCESS != DetermineKeyAndConstraintColumns(columns, relClassMappingInfo))
        return ClassMappingStatus::Error;

    //Set tables
    for (DbColumn const* fkTablePkCol : columns.GetECInstanceIdColumns())
        {
        AddTable(fkTablePkCol->GetTableR());
        }

    //Create ECInstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    RefCountedPtr<ECInstanceIdPropertyMap> ecInstanceIdPropMap = ECInstanceIdPropertyMap::CreateInstance(*this, columns.GetECInstanceIdColumns());
    if (ecInstanceIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMapECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(ecInstanceIdPropMap, 0) != SUCCESS)
        return ClassMappingStatus::Error;

    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropMap = ECClassIdPropertyMap::CreateInstance(*this, GetClass().GetId(), columns.GetFkRelECClassIdColumns());
    if (ecClassIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create ECClassIdPropertyMap");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(ecClassIdPropMap, 1) != SUCCESS)
        return ClassMappingStatus::Error;

    //ForeignEnd ECInstanceId PropMap
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> foreignEndIdPropertyMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetForeignEnd(), columns.GetECInstanceIdColumns());
    if (foreignEndIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(foreignEndIdPropertyMap) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECInstanceIdPropMap(foreignEndIdPropertyMap.get());

    //Create ForeignEnd ClassId propertyMap
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    RefCountedPtr<ConstraintECClassIdPropertyMap> foreignEndClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, foreignEndConstraint.GetConstraintClasses()[0]->GetId(), GetForeignEnd(), columns.GetECClassIdColumns());
    if (foreignEndClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(foreignEndClassIdPropertyMap) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(GetForeignEnd()).SetECClassIdPropMap(foreignEndClassIdPropertyMap.get());

    //FK PropMap (aka referenced end id prop map)
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> referencePropertyMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetReferencedEnd(), columns.GetFkECInstanceIdColumns());
    if (referencePropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(referencePropertyMap) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECInstanceIdPropMap(referencePropertyMap.get());


    //FK ClassId PropMap (aka referenced end classid prop map)
    ECRelationshipConstraintCR referenceEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    RefCountedPtr<ConstraintECClassIdPropertyMap> referenceClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, referenceEndConstraint.GetConstraintClasses()[0]->GetId(), GetReferencedEnd(), columns.GetFkECClassIdColumns());
    if (referenceClassIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(referenceClassIdPropertyMap) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(GetReferencedEnd()).SetECClassIdPropMap(referenceClassIdPropertyMap.get());

    //map non-system properties
    if (ClassMappingStatus::Error == MapProperties(ctx))
        return ClassMappingStatus::Error;

    AddIndexToRelationshipEnd();
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns(ColumnLists& columns, RelationshipMappingInfo const& classMappingInfo)
    {
    BeAssert(!GetClass().HasBaseClasses() && "RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns is expected to only be called for root rel classes.");

    if (SUCCESS != DetermineFkColumns(columns, classMappingInfo))
        return ERROR;

    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();

    ForeignKeyDbConstraint::ActionType onDelete = ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType onUpdate = ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType userRequestedDeleteAction = classMappingInfo.GetFkMappingInfo()->IsPhysicalFk() ? classMappingInfo.GetFkMappingInfo()->GetOnDeleteAction() : ForeignKeyDbConstraint::ActionType::NotSpecified;
    ForeignKeyDbConstraint::ActionType userRequestedUpdateAction = classMappingInfo.GetFkMappingInfo()->IsPhysicalFk() ? classMappingInfo.GetFkMappingInfo()->GetOnUpdateAction() : ForeignKeyDbConstraint::ActionType::NotSpecified;
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

    DbColumn const* referencedTableClassIdCol = referencedTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    for (DbColumn const* fkCol : columns.GetFkECInstanceIdColumns())
        {
        DbTable& fkTable = fkCol->GetTableR();
        const bool makeRelClassIdColNotNull = fkCol->DoNotAllowDbNull();
        DbColumn* relClassIdCol = CreateRelECClassIdColumn(columns.GetColumnFactory(), fkTable, DetermineRelECClassIdColumnName(relClass, fkCol->GetName()), makeRelClassIdColNotNull);
        if (relClassIdCol == nullptr)
            {
            BeAssert(false && "Could not create RelClassId col");
            return ERROR;
            }

        columns.AddFkRelECClassIdColumn(*relClassIdCol);
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

                fkTableClassIdCol = fkTable.CreateColumn(Utf8String(colName), DbColumn::Type::Integer, kind, PersistenceType::Virtual);

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

        columns.AddECInstanceIdColumn(*fkTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId));
        columns.AddECClassIdColumn(*fkTableClassIdCol);

        if (referencedTableClassIdCol != nullptr)
            columns.AddFkECClassIdColumn(*referencedTableClassIdCol);
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

                fkTableReferencedEndClassIdCol = fkTable.CreateColumn(Utf8String(colName), DbColumn::Type::Integer, kind, PersistenceType::Virtual);

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

            columns.AddFkECClassIdColumn(*fkTableReferencedEndClassIdCol);
            }

        //if FK table is a joined table, CASCADE is not allowed as it would leave orphaned rows in the parent of joined table.
        if (fkTable.GetParentOfJoinedTable() != nullptr)
            {
            if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade ||
                (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified && relClass.GetStrength() == StrengthType::Embedding))
                {
                if (userRequestedDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade)
                    Issues().Report("Failed to map ECRelationshipClass %s. Its ForeignKeyConstraint custom attribute specifies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());
                else
                    Issues().Report("Failed to map ECRelationshipClass %s. Its strength is 'Embedding' which implies the OnDelete action 'Cascade'. "
                                    "This is only allowed if the foreign key end of the ECRelationship is not mapped to a joined table.",
                                    relClass.GetFullName());

                return ERROR;
                }
            }

        if (!fkTable.IsOwnedByECDb() || fkTable.GetPersistenceType() == PersistenceType::Virtual ||
            referencedTable->GetPersistenceType() == PersistenceType::Virtual || fkCol->IsShared())
            continue;

        fkTable.CreateForeignKeyConstraint(*fkCol, *referencedTablePKCol, onDelete, onUpdate);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       12/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineFkColumns(ColumnLists& columns, RelationshipMappingInfo const& classMappingInfo)
    {
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    std::set<DbTable const*> foreignEndTables = GetForeignEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables() : classMappingInfo.GetTargetTables();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();

    // table must meet following constraint though these are already validated at MapStrategy evaluation time.
    BeAssert(foreignEndTables.size() >= 1 && "ForeignEnd Tables must be >= 1");
    BeAssert(GetReferencedEnd() == ECRelationshipEnd_Source ? classMappingInfo.GetSourceTables().size() == 1 : classMappingInfo.GetTargetTables().size() == 1 && "ReferencedEnd Tables must be == 1");

    //Note: The FK column is the column that refers to the referenced end. Therefore the ECRelationshipEnd of the referenced end has to be taken!
    DbColumn::Kind foreignKeyColumnKind = GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;

    if (classMappingInfo.GetFkMappingInfo()->UseECInstanceIdAsFk())
        {
        for (DbTable const* foreignEndTable : foreignEndTables)
            {
            DbColumn const* pkColumn = foreignEndTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            BeAssert(pkColumn != nullptr);

            DbColumn* pkColumnP = const_cast<DbColumn*> (pkColumn);
            pkColumnP->AddKind(foreignKeyColumnKind);

            columns.AddFkECInstanceIdColumn(*pkColumn);
            }

        return SUCCESS;
        }

    const bool multiplicityImpliesNotNullOnFkCol = referencedEndConstraint.GetMultiplicity().GetLowerLimit() > 0;

    ForeignKeyColumnInfo fkColInfo;
    if (SUCCESS != TryGetForeignKeyColumnInfoFromNavigationProperty(fkColInfo, foreignEndConstraint, relClass, GetForeignEnd()))
        return ERROR;

    Utf8String fkColName = DetermineFkColumnName(classMappingInfo, fkColInfo);

    //needed to determine NOT NULL of FK and RelClassId cols
    bset<ECClassId> foreignEndConstraintClassIds;
    for (ECClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
        {
        BeAssert(constraintClass->HasId());
        foreignEndConstraintClassIds.insert(constraintClass->GetId());
        }

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

                columns.AddFkECInstanceIdColumn(*fkCol);
                continue;
                }

            Issues().Report("Failed to map ECRelationshipClass '%s'. It is mapped to the existing table '%s' not owned by ECDb, but doesn't have a foreign key column called '%s'.",
                            relClass.GetFullName(), foreignEndTable->GetName().c_str(), fkColName.c_str());
            return ERROR;
            }

        //table owned by ECDb
        if (fkCol != nullptr)
            {
            Issues().Report("Failed to map ECRelationshipClass '%s'. ForeignKey column name '%s' is already used by another column in the table '%s'.",
                            relClass.GetFullName(), fkColName.c_str(), foreignEndTable->GetName().c_str());
            return ERROR;
            }

        int fkColPosition = -1;
        if (SUCCESS != TryDetermineForeignKeyColumnPosition(fkColPosition, *foreignEndTable, fkColInfo))
            return ERROR;

        const PersistenceType columnPersistenceType = foreignEndTable->GetPersistenceType() == PersistenceType::Physical ? PersistenceType::Physical : PersistenceType::Virtual;
        DbColumn* newFkCol = columns.GetColumnFactory().AllocateForeignKeyECInstanceId(*const_cast<DbTable*>(foreignEndTable), fkColName, columnPersistenceType, fkColPosition);
        if (newFkCol == nullptr)
            {
            Issues().Report("Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                            relClass.GetFullName(), fkColName.c_str(), foreignEndTable->GetName().c_str());
            BeAssert(false && "Could not create FK column for end table mapping");
            return ERROR;
            }

        const bool makeFkColNotNull = multiplicityImpliesNotNullOnFkCol && foreignEndTable->HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(foreignEndTable->GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
        if (makeFkColNotNull)
            newFkCol->GetConstraintsR().SetNotNullConstraint();

        columns.AddFkECInstanceIdColumn(*newFkCol);
        }

    BeAssert(columns.GetFkECInstanceIdColumns().size() == foreignEndTables.size());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* RelationshipClassEndTableMap::CreateRelECClassIdColumn(ColumnFactory& colfactory, DbTable& table, Utf8StringCR relClassIdColName, bool makeNotNull) const
    {
    BeAssert(!GetClass().HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");
    PersistenceType persType = PersistenceType::Physical;
    if (table.GetPersistenceType() == PersistenceType::Virtual || !table.IsOwnedByECDb() || GetClass().GetClassModifier() == ECClassModifier::Sealed)
        persType = PersistenceType::Virtual;

    DbColumn* relClassIdCol = table.FindColumnP(relClassIdColName.c_str());
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

    relClassIdCol = colfactory.AllocateForeignKeyRelECClassId(table, relClassIdColName, persType);
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
    if (fkColInfo.CanImplyFromNavigationProperty() && !fkColInfo.GetImpliedColumnName().empty())
        {
        BeAssert(!fkColInfo.GetImpliedColumnName().empty());
        return fkColInfo.GetImpliedColumnName();
        }

    //default name: prefix_<schema alias>_<rel class name>
    ECClassCR relClass = classMappingInfo.GetECClass();
    Utf8String fkColumnName(DEFAULT_FKCOLUMNNAME_PREFIX);
    fkColumnName.append(relClass.GetSchema().GetAlias()).append("_").append(relClass.GetName());
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

    foreignEndConstraintMap.SetECInstanceIdPropMap(clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

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

    foreignEndConstraintMap.SetECClassIdPropMap(clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

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

    referencedEndConstraintMap.SetECInstanceIdPropMap(clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    //Referenced ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECClassIdPropMap(clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

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
    ECClassId defaultSourceECClassId = relationshipClass.GetSource().GetConstraintClasses().empty() ? ECClassId() : relationshipClass.GetSource().GetConstraintClasses().front()->GetId();
    ECClassId defaultTargetECClassId = relationshipClass.GetTarget().GetConstraintClasses().empty() ? ECClassId() : relationshipClass.GetTarget().GetConstraintClasses().front()->GetId();

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

        Issues().Report(error, GetRelationshipClass().GetFullName());
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
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd()
    {
    //0:0 or 1:1 cardinalities imply unique index
    const bool isUniqueIndex = GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1 &&
                               GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1;

    BeAssert(GetReferencedEndECInstanceIdPropMap() != nullptr);
    std::vector<DbColumn const*> referencedEndIdColumns;

    for (SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap : GetReferencedEndECInstanceIdPropMap()->GetDataPropertyMaps())
        {
        DbTable& persistenceEndTable = const_cast<DbTable&>(vmap->GetColumn().GetTable());
        if (persistenceEndTable.GetType() == DbTable::Type::Existing || vmap->GetColumn().IsShared())
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
ConstraintECClassIdPropertyMap const* RelationshipClassEndTableMap::GetForeignEndECClassIdPropMap() const
    {
    return GetConstraintMap(GetForeignEnd()).GetECClassIdPropMap();
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
    for (ECClassCP constraintClass : constraintClasses)
        {
        for (ECPropertyCP prop : constraintClass->GetProperties())
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

    //if not overridden with a PropertyMap CA, the FK column name is implied as <nav prop name>Id.
    //if nav prop name ends with "Id" already, it is not appended again.
    Utf8StringCR navPropName = singleNavProperty->GetName();
    Utf8String defaultFkColName(navPropName);
    if (!navPropName.EndsWithIAscii("id"))
        defaultFkColName.append("Id");

    ClassMap const* classMap = GetDbMap().GetClassMap(singleNavProperty->GetClass());
    TablePerHierarchyInfo const& tphInfo = classMap->GetMapStrategy().GetTphInfo();
    if (tphInfo.IsValid() && tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        //table uses shared columns, so FK col position cannot depend on NavigationProperty position
        fkColInfo.Assign(defaultFkColName.c_str(), true, nullptr, nullptr);
        return SUCCESS;
        }

    //now determine the property map defined just before the nav prop in the ECClass, that is mapped to
    PropertyMap const* precedingPropMap = nullptr;
    PropertyMap const* succeedingPropMap = nullptr;
    bool foundNavProp = false;
    for (PropertyMap const* propMap : classMap->GetPropertyMaps())
        {
        if (&propMap->GetProperty() == singleNavProperty)
            {
            foundNavProp = true;
            if (precedingPropMap != nullptr)
                break;

            //no preceding prop map exists, continue until succeeding prop map is found
            continue;
            }

        //Skip system properties and navigation properties
        if (propMap->IsSystem() || propMap->GetType() == PropertyMap::Type::Navigation)
            continue;

        if (!foundNavProp)
            precedingPropMap = propMap;
        else
            {
            succeedingPropMap = propMap;
            break;
            }
        }

    fkColInfo.Assign(defaultFkColName.c_str(), false, precedingPropMap, succeedingPropMap);
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
        //after ECInstanceId and ECClassId DbColumns (the latter always exists (might be virtual though))
        position = 2;
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
ClassMappingStatus RelationshipClassLinkTableMap::_Map(ClassMappingContext& ctx)
    {
    BeAssert(!MapStrategyExtendedInfo::IsForeignKeyMapping(GetMapStrategy()) &&
             "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");
    BeAssert(GetRelationshipClass().GetStrength() != StrengthType::Embedding && "Should have been caught already in ClassMapInfo");

    ClassMappingStatus stat = DoMapPart1(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&ctx.GetClassMappingInfo()) != nullptr);
    RelationshipMappingInfo const& relationClassMapInfo = static_cast<RelationshipMappingInfo const&> (ctx.GetClassMappingInfo());

    if (GetRelationshipClass().HasBaseClasses())
        return MapSubClass(ctx, relationClassMapInfo);

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
    if (stat != ClassMappingStatus::Success)
        return stat;

    stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    //only create constraints on TPH root or if not TPH and not existing table
    if (GetPrimaryTable().GetType() != DbTable::Type::Existing &&
        (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId())) 
        {
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


    AddIndices(ctx, relationClassMapInfo.GetLinkTableMappingInfo()->AllowDuplicateRelationships());
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::MapSubClass(ClassMappingContext& ctx, RelationshipMappingInfo const& mappingInfo)
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

    RelationshipClassLinkTableMap const& baseRelClassMap = static_cast<RelationshipClassLinkTableMap const&>(*baseClassMap);

    //SourceECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(clonedSourceECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //SourceECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(clonedSourceECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    //TargetECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(clonedTargetECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //TargetECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(clonedTargetECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    ClassMappingStatus stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    AddIndices(ctx, DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClass->GetRelationshipClassCP()));
    return ClassMappingStatus::Success;
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
    ECClass const* foreignEndClass = foreignEndConstraint.GetConstraintClasses()[0];
    ClassMap const* foreignEndClassMap = GetDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetDbMap().GetTableCountOnRelationshipEnd(foreignEndConstraint);

    Utf8String columnName = DetermineConstraintECClassIdColumnName(*mapInfo.GetLinkTableMappingInfo(), relationshipEnd);
    if (GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr &&
        GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        //Following error occurs in Upgrading ECSchema but is not fatal.
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return nullptr;
        }

    const DbColumn::Kind columnId = relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId;

    if (ConstraintIncludesAnyClass(foreignEndConstraint.GetConstraintClasses()) || foreignEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), columnId, PersistenceType::Physical);
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
ClassMappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(RelationshipMappingInfo const& mapInfo, bool addSourceECClassIdColumnToTable, ECClassId defaultSourceECClassId,
    bool addTargetECClassIdColumnToTable, ECClassId defaultTargetECClassId)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = DetermineConstraintECInstanceIdColumnName(*mapInfo.GetLinkTableMappingInfo(), ECRelationshipEnd_Source);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains SourceECInstanceId column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), DbColumn::Kind::SourceECInstanceId, PersistenceType::Physical);
    if (sourceECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceIdColumn});
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //**** SourceECClassId prop map
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultSourceECClassId, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = DetermineConstraintECInstanceIdColumnName(*mapInfo.GetLinkTableMappingInfo(), ECRelationshipEnd_Target);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains TargetECInstanceId column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), DbColumn::Kind::TargetECInstanceId, PersistenceType::Physical);
    if (targetECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceIdColumn});
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());


    //**** TargetECClassId prop map
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(mapInfo, ECRelationshipEnd_Target);
    auto targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, defaultTargetECClassId , ECRelationshipEnd_Target, {targetECClassIdColumn});
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(targetECClassIdPropMap != nullptr);
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
void RelationshipClassLinkTableMap::AddIndices(ClassMappingContext& ctx, bool allowDuplicateRelationships)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;

    // Add indices on the source and target based on cardinality
    //(the many side can be unique, but the one side must never be unique)
    const bool sourceIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1);
    const bool targetIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1);

    AddIndex(ctx.GetImportCtx(), RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndex(ctx.GetImportCtx(), RelationshipIndexSpec::Target, targetIsUnique);

    if (!allowDuplicateRelationships)
        AddIndex(ctx.GetImportCtx(), RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
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
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECInstanceIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const& linkTableInfo, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            if (linkTableInfo.GetSourceIdColumnName().empty())
                colName.assign(ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
            else
                colName.assign(linkTableInfo.GetSourceIdColumnName());

            break;
            }
            case ECRelationshipEnd_Target:
            {
            if (linkTableInfo.GetTargetIdColumnName().empty())
                colName.assign(ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
            else
                colName.assign(linkTableInfo.GetTargetIdColumnName());

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
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECClassIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const& linkTableInfo, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    Utf8StringCP idColName = nullptr;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            if (linkTableInfo.GetSourceIdColumnName().empty())
                colName.assign(ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
            else
                idColName = &linkTableInfo.GetSourceIdColumnName();
            
            break;
            }

            case ECRelationshipEnd_Target:
            {
            if (linkTableInfo.GetTargetIdColumnName().empty())
                colName.assign(ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
            else
                idColName = &linkTableInfo.GetTargetIdColumnName();
            
            break;
            }

            default:
                BeAssert(false);
                break;
        }

    if (idColName != nullptr)
        {
        if (!idColName->EndsWithIAscii("id"))
            colName.assign(*idColName);
        else
            colName.assign(idColName->substr(0, idColName->size() - 2));

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
    ECDbLinkTableRelationshipMap linkRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkRelMap, baseRelClass))
        {
        bool allowDuplicateRels = false;
        linkRelMap.TryGetAllowDuplicateRelationships(allowDuplicateRels);
        if (allowDuplicateRels)
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
    if (ClassMap::_Load(ctx, mapInfo) != BentleyStatus::SUCCESS)
        return ERROR;

    ECRelationshipClassCR relationshipClass = GetRelationshipClass();
    auto const& sourceConstraint = relationshipClass.GetSource();
    auto const& targetConstraint = relationshipClass.GetTarget();

    ECClassId defaultSourceECClassId = sourceConstraint.GetConstraintClasses().empty() ? ECClassId() : sourceConstraint.GetConstraintClasses().front()->GetId();
    ECClassId defaultTargetECClassId = targetConstraint.GetConstraintClasses().empty() ? ECClassId() : targetConstraint.GetConstraintClasses().front()->GetId();

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

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind columnId, PersistenceType persType)
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
    if (table.GetPersistenceType() == PersistenceType::Physical)
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
    column = table.CreateColumn(Utf8String(columnName), DbColumn::Type::Integer, columnId, persType);

    if (!wasEditMode)
        table.GetEditHandleR().EndEdit();

    return column;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
void RelationshipClassLinkTableMap::DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECRelationshipConstraintCR constraint) const
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
        ECClassCP constraintClass = constraintClasses[0];
        BeAssert(constraintClass->HasId());
        defaultConstraintClassId = constraintClass->GetId();
        }
    else
        defaultConstraintClassId = ECClassId();
    }
//************************RelationshipClassEndTableMap::ColumnFactory********************
RelationshipClassEndTableMap::ColumnFactory::ColumnFactory(RelationshipClassEndTableMap const& relMap, RelationshipMappingInfo const& relInfo)
    :m_relMap(relMap), m_relInfo(relInfo) 
    {
        Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassEndTableMap::ColumnFactory::AllocateForeignKeyECInstanceId(DbTable& table, Utf8StringCR colName, PersistenceType persType, int position)
    {
    const DbColumn::Kind colKind = m_relMap.GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;
    const DbColumn::Type colType = DbColumn::Type::Integer;
    
    if (m_relInfo.GetFkMappingInfo()->IsPhysicalFk() || persType == PersistenceType::Virtual)
        return table.CreateColumn(colName, colType, position, colKind, persType);

    auto itor = m_constraintClassMaps.find(&table);
    if (itor == m_constraintClassMaps.end())
        {
        BeAssert(false);
        return nullptr;
        }

    ClassMapCP rootClassMap = itor->second;
    //ECDB_RULE: If IsPhysicalFK is false and we do not have shared column support go a head and create a column
    if (!rootClassMap->GetMapStrategy().IsTablePerHierarchy() ||
        rootClassMap->GetMapStrategy().GetTphInfo().GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        return table.CreateColumn(colName, colType, position, colKind, persType);
        }
    //ECDB_RULE: Create shared column
        
    ECSqlSystemPropertyInfo const& constraintECInstanceIdType = m_relMap.GetReferencedEnd() == ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECInstanceId() : ECSqlSystemPropertyInfo::TargetECInstanceId();
    ECDbSystemSchemaHelper const& systemSchemaHelper = m_relMap.GetDbMap().GetECDb().Schemas().GetReader().GetSystemSchemaHelper();
    ECPropertyCP  constraintECInstanceIdProp = systemSchemaHelper.GetSystemProperty(constraintECInstanceIdType);
    return rootClassMap->GetColumnFactory().AllocateDataColumn(
        *constraintECInstanceIdProp,
        colType,
        DbColumn::CreateParams(colName.c_str()),
        m_relMap.BuildQualifiedAccessString(constraintECInstanceIdProp->GetName()),
        nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassEndTableMap::ColumnFactory::AllocateForeignKeyRelECClassId(DbTable& table, Utf8StringCR colName, PersistenceType persType)
    {
    const DbColumn::Type colType = DbColumn::Type::Integer;
    const DbColumn::Kind colKind = DbColumn::Kind::RelECClassId;
    if (m_relInfo.GetFkMappingInfo()->IsPhysicalFk() || persType == PersistenceType::Virtual)
        return table.CreateColumn(colName, colType, colKind, persType);

    auto itor = m_constraintClassMaps.find(&table);
    if (itor == m_constraintClassMaps.end())
        {
        BeAssert(false);
        return nullptr;
        }

    ClassMapCP rootClassMap = itor->second;
    //ECDB_RULE: If IsPhysicalFK is false and we do not have shared column support go a head and create a column
    if (!rootClassMap->GetMapStrategy().IsTablePerHierarchy() ||
        rootClassMap->GetMapStrategy().GetTphInfo().GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        return table.CreateColumn(colName, colType, colKind, persType);
        }

    ECDbSystemSchemaHelper const& systemSchemaHelper = m_relMap.GetDbMap().GetECDb().Schemas().GetReader().GetSystemSchemaHelper();
    //ECDB_RULE: Note we are using ECClassId here its not a mistake.
    ECPropertyCP  relECClassIdProp = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::ECClassId());
    return rootClassMap->GetColumnFactory().AllocateDataColumn(
        *relECClassIdProp,
        colType,
        DbColumn::CreateParams(colName.c_str()),
        m_relMap.BuildQualifiedAccessString(relECClassIdProp->GetName()), nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
void RelationshipClassEndTableMap::ColumnFactory::Initialize()
    {
    ECDbMap const& ecdbMap = m_relMap.GetDbMap();
    ECN::ECRelationshipConstraintCR constraint = m_relMap.GetForeignEnd() == ECN::ECRelationshipEnd_Source ? m_relMap.GetRelationshipClass().GetSource() : m_relMap.GetRelationshipClass().GetTarget();

    std::function<void(ECClassCR)> traverseConstraintClass =
        [&](ECClassCR constraintClass)
        {
        if (ClassMapCP classMap = ecdbMap.GetClassMap(constraintClass))
            {
            m_constraintClassMaps[&classMap->GetJoinedTable()] = classMap;
            if (classMap->GetMapStrategy().IsTablePerHierarchy())
                return;
            }

        if (constraint.GetIsPolymorphic())
            for (ECClassCP derivedClass : constraintClass.GetDerivedClasses())
                traverseConstraintClass(*derivedClass);
        };

        for (ECN::ECClassCP constraintClass : constraint.GetConstraintClasses())
            {
            traverseConstraintClass(*constraintClass);
            }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
