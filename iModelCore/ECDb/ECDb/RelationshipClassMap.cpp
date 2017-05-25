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
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
RelationshipClassMap::RelationshipClassMap(ECDb const& ecdb, Type type, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const& updatableViewInfo)
    : ClassMap(ecdb, type, ecRelClass, mapStrategy, updatableViewInfo),
    m_sourceConstraintMap(ecRelClass.GetRelationshipClassCP()->GetSource()),
    m_targetConstraintMap(ecRelClass.GetRelationshipClassCP()->GetTarget())
    {
    BeAssert(ecRelClass.IsRelationshipClass());
    }

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



//************************ RelationshipClassEndTableMap **********************************
//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP RelationshipClassEndTableMap::DEFAULT_FK_COL_PREFIX = "FK_";
//static
Utf8CP RelationshipClassEndTableMap::RELECCLASSID_COLNAME_TOKEN = "RelECClassId";

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap::RelationshipClassEndTableMap(ECDb const& ecdb, ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy) {}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap::RelationshipClassEndTableMap(ECDb const& ecdb, ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const& updatableViewInfo)
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy, updatableViewInfo)
    {}

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

    RefCountedPtr<ECClassIdPropertyMap> ecClassIdPropMap = ECClassIdPropertyMap::CreateInstance(*this, columns.GetFkRelECClassIdColumns());
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
    RefCountedPtr<ConstraintECClassIdPropertyMap> foreignEndClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, GetForeignEnd(), columns.GetECClassIdColumns());
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
    RefCountedPtr<ConstraintECClassIdPropertyMap> referenceClassIdPropertyMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, GetReferencedEnd(), columns.GetFkECClassIdColumns());
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

    AddIndexToRelationshipEnd(relClassMappingInfo);
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns(ColumnLists& columns, RelationshipMappingInfo const& classMappingInfo)
    {
    BeAssert(!GetClass().HasBaseClasses() && "RelationshipClassEndTableMap::DetermineKeyAndConstraintColumns is expected to only be called for root rel classes.");

    ForeignKeyColumnInfo fkColInfo;
    if (SUCCESS != DetermineFkColumns(columns, fkColInfo, classMappingInfo))
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
    DbColumn const* referencedTablePKCol = referencedTable->FindFirst(DbColumn::Kind::ECInstanceId);
    if (referencedTablePKCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbColumn const* referencedTableClassIdCol = referencedTable->FindFirst(DbColumn::Kind::ECClassId);
    for (DbColumn const* fkCol : columns.GetFkECInstanceIdColumns())
        {
        DbTable& fkTable = fkCol->GetTableR();

        DbColumn* relClassIdCol = CreateRelECClassIdColumn(columns.GetColumnFactory(), fkTable, fkColInfo, *fkCol);
        if (relClassIdCol == nullptr)
            {
            BeAssert(false && "Could not create RelClassId col");
            return ERROR;
            }

        columns.AddFkRelECClassIdColumn(*relClassIdCol);

        DbColumn const* fkTableClassIdCol = fkTable.FindFirst(DbColumn::Kind::ECClassId);
        //If ForeignEndClassId column is missing create a virtual one
        if (fkTableClassIdCol == nullptr)
            {
            Utf8CP colName = GetForeignEnd() == ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
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

        columns.AddECInstanceIdColumn(*fkTable.FindFirst(DbColumn::Kind::ECInstanceId));
        columns.AddECClassIdColumn(*fkTableClassIdCol);

        if (referencedTableClassIdCol != nullptr)
            columns.AddFkECClassIdColumn(*referencedTableClassIdCol);
        else
            {
            //referenced table doesn't have a class id col --> create a virtual one in the foreign end table
            Utf8CP colName = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
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
        if (fkTable.GetLinkNode().IsChildTable())
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

        if (fkTable.GetType() == DbTable::Type::Existing || fkTable.GetType() == DbTable::Type::Virtual || 
            referencedTable->GetType() == DbTable::Type::Virtual || 
            fkCol->IsShared() || !classMappingInfo.GetFkMappingInfo()->IsPhysicalFk())
            continue;

        fkTable.CreateForeignKeyConstraint(*fkCol, *referencedTablePKCol, onDelete, onUpdate);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       12/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::DetermineFkColumns(ColumnLists& columns, ForeignKeyColumnInfo& fkColInfo, RelationshipMappingInfo const& classMappingInfo)
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
            DbColumn const* pkColumn = foreignEndTable->FindFirst(DbColumn::Kind::ECInstanceId);
            BeAssert(pkColumn != nullptr);

            DbColumn* pkColumnP = const_cast<DbColumn*> (pkColumn);
            pkColumnP->AddKind(foreignKeyColumnKind);

            columns.AddFkECInstanceIdColumn(*pkColumn);
            }

        return SUCCESS;
        }

    const bool multiplicityImpliesNotNullOnFkCol = referencedEndConstraint.GetMultiplicity().GetLowerLimit() > 0;

    if (SUCCESS != TryGetForeignKeyColumnInfoFromNavigationProperty(fkColInfo, foreignEndConstraint, relClass, GetForeignEnd()))
        return ERROR;

    Utf8String fkColName;
    if (fkColInfo.CanImplyFromNavigationProperty() && !fkColInfo.GetImpliedFkColumnName().empty())
        fkColName.assign(fkColInfo.GetImpliedFkColumnName());
    else
        {
        //default name: FK_<schema alias>_<rel class name>
        fkColName.assign(DEFAULT_FK_COL_PREFIX).append(relClass.GetSchema().GetAlias()).append("_").append(relClass.GetName());
        }

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
        if (foreignEndTable->GetType() == DbTable::Type::Existing)
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

        DbTable& fkTableR = *const_cast<DbTable*>(foreignEndTable);
        DbColumn* newFkCol = columns.GetColumnFactory().AllocateForeignKeyECInstanceId(fkTableR, fkColName, fkColPosition);
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
DbColumn* RelationshipClassEndTableMap::CreateRelECClassIdColumn(ColumnFactory& colfactory, DbTable& table, ForeignKeyColumnInfo const& fkColInfo, DbColumn const& fkCol) const
    {
    BeAssert(!GetClass().HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");

    const bool makeRelClassIdColNotNull = fkCol.DoNotAllowDbNull();

    PersistenceType persType = PersistenceType::Physical;
    if (table.GetType() == DbTable::Type::Virtual || table.GetType() == DbTable::Type::Existing || GetClass().GetClassModifier() == ECClassModifier::Sealed)
        persType = PersistenceType::Virtual;

    Utf8String relECClassIdColName;
    if (fkColInfo.CanImplyFromNavigationProperty())
        relECClassIdColName.assign(fkColInfo.GetImpliedRelClassIdColumnName());
    else if (fkCol.GetName().EndsWithIAscii("id"))
        {
        relECClassIdColName = fkCol.GetName().substr(0, fkCol.GetName().size() - 2);
        relECClassIdColName.append(RELECCLASSID_COLNAME_TOKEN);
        }
    else if (!fkCol.GetName().StartsWithIAscii(DEFAULT_FK_COL_PREFIX) && !fkCol.IsShared())
        relECClassIdColName.assign(fkCol.GetName()).append(RELECCLASSID_COLNAME_TOKEN);
    else
        {
        //default name: RelECClassId_<schema alias>_<rel class name>
        relECClassIdColName.assign(RELECCLASSID_COLNAME_TOKEN).append("_").append(GetRelationshipClass().GetSchema().GetAlias()).append("_").append(GetRelationshipClass().GetName());
        }

    DbColumn* relClassIdCol = table.FindColumnP(relECClassIdColName.c_str());
    if (relClassIdCol != nullptr)
        {
        BeAssert(Enum::Contains(relClassIdCol->GetKind(), DbColumn::Kind::RelECClassId));
        if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
            {
            relClassIdCol->GetConstraintsR().SetNotNullConstraint();
            BeAssert(relClassIdCol->GetId().IsValid());
            }

        return relClassIdCol;
        }

    const bool canEdit = table.GetEditHandleR().CanEdit();
    if (!canEdit)
        table.GetEditHandleR().BeginEdit();

    relClassIdCol = colfactory.AllocateForeignKeyRelECClassId(table, relECClassIdColName, persType, fkCol.DeterminePosition() + 1);
    if (relClassIdCol == nullptr)
        return nullptr;

    if (makeRelClassIdColNotNull && !relClassIdCol->DoNotAllowDbNull())
        {
        relClassIdCol->GetConstraintsR().SetNotNullConstraint();
        BeAssert(relClassIdCol->GetId().IsValid());
        }

    if (persType != PersistenceType::Virtual)
        {
        Nullable<Utf8String> indexName("ix_");
        indexName.ValueR().append(table.GetName()).append("_").append(relClassIdCol->GetName());
        DbIndex* index = GetDbMap().GetDbSchemaR().CreateIndex(table, indexName, false, {relClassIdCol}, true, true, GetClass().GetId());
        if (index == nullptr)
            {
            LOG.errorv("Failed to create index on " ECDBSYS_PROP_NavPropRelECClassId " column %s on table %s.", relClassIdCol->GetName().c_str(), table.GetName().c_str());
            return nullptr;
            }
        }

    if (!canEdit)
        table.GetEditHandleR().EndEdit();

    return relClassIdCol;
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

    RelationshipClassEndTableMap const& baseRelClassMap = baseClassMap->GetAs<RelationshipClassEndTableMap>();
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
    if (clonedConstraintInstanceId == nullptr)
        return ERROR;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    GetTablesPropertyMapVisitor tableDisp;
    clonedConstraintInstanceId->AcceptVisitor(tableDisp);
    for (DbTable const* table : tableDisp.GetTables())
        {
        AddTable(*const_cast<DbTable *>(table));
        }

    //Foreign ECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseForeignEndConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ERROR;

    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ERROR;

    foreignEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

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
    if (clonedConstraintInstanceId == nullptr)
        return ERROR;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    //Referenced ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ERROR;
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ERROR;

    referencedEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    return SUCCESS;
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
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd(RelationshipMappingInfo const& relClassMappingInfo)
    {
    //0:0 or 1:1 cardinalities imply unique index
    const bool isUniqueIndex = GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1 &&
                               GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1;

    if (!relClassMappingInfo.GetFkMappingInfo()->IsPhysicalFk())
        return;

    BeAssert(GetReferencedEndECInstanceIdPropMap() != nullptr);
    for (SystemPropertyMap::PerTableIdPropertyMap const* vmap : GetReferencedEndECInstanceIdPropMap()->GetDataPropertyMaps())
        {
        DbTable& persistenceEndTable = const_cast<DbTable&>(vmap->GetColumn().GetTable());
        if (persistenceEndTable.GetType() == DbTable::Type::Existing || vmap->GetColumn().IsShared())
            continue;

        // name of the index
        Nullable<Utf8String> name(isUniqueIndex ? "uix_" : "ix_");
        name.ValueR().append(persistenceEndTable.GetName()).append("_fk_").append(GetClass().GetSchema().GetAlias() + "_" + GetClass().GetName());
        if (GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable)
            name.ValueR().append("_source");
        else
            name.ValueR().append("_target");

        GetDbMap().GetDbSchemaR().CreateIndex(persistenceEndTable, name, isUniqueIndex, {&vmap->GetColumn()}, true, true, GetClass().GetId());
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
            if (navProp == singleNavProperty)
                continue; //this case can occur if multiple constraint class inherit the same nav prop 

            if (navProp != nullptr && navProp->GetRelationshipClass() == &relClass && navProp->GetDirection() == expectedDirection)
                {
                if (singleNavProperty == nullptr)
                    singleNavProperty = navProp;
                else
                    {
                    BeAssert(false && "This should not be hit anymore since we disallowed multiple nav props to same relationship");
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
    Utf8String defaultFkColName, defaultRelClassIdColName;
    if (navPropName.EndsWithIAscii("id"))
        {
        defaultFkColName.assign(navPropName);
        defaultRelClassIdColName = navPropName.substr(0, navPropName.size() - 2);
        }
    else
        {
        defaultFkColName.assign(navPropName).append("Id");
        defaultRelClassIdColName.assign(navPropName);
        }

    defaultRelClassIdColName.append(RELECCLASSID_COLNAME_TOKEN);

    ClassMap const* classMap = GetDbMap().GetClassMap(singleNavProperty->GetClass());
    TablePerHierarchyInfo const& tphInfo = classMap->GetMapStrategy().GetTphInfo();
    if (tphInfo.IsValid() && tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        //table uses shared columns, so FK col position cannot depend on NavigationProperty position
        fkColInfo.Assign(defaultFkColName, defaultRelClassIdColName, true, nullptr, nullptr);
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

    fkColInfo.Assign(defaultFkColName, defaultRelClassIdColName, false, precedingPropMap, succeedingPropMap);
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
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : RelationshipClassMap(ecdb, Type::RelationshipLinkTable, ecRelClass, mapStrategy)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const& updatableViewInfo)
    : RelationshipClassMap(ecdb, Type::RelationshipLinkTable, ecRelClass, mapStrategy, updatableViewInfo)
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
    DetermineConstraintClassIdColumnHandling(addSourceECClassIdColumnToTable, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addTargetECClassIdColumnToTable, targetConstraint);
    stat = CreateConstraintPropMaps(ctx, relationClassMapInfo, addSourceECClassIdColumnToTable, addTargetECClassIdColumnToTable);
    if (stat != ClassMappingStatus::Success)
        return stat;

    stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    //only create constraints on TPH root or if not TPH and not existing table
    if (GetPrimaryTable().GetType() != DbTable::Type::Existing && relationClassMapInfo.GetLinkTableMappingInfo()->GetCreateForeignKeyConstraintsFlag() &&
        (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId())) 
        {
        //Create FK from Source-Primary to LinkTable
        DbTable const* sourceTable = *relationClassMapInfo.GetSourceTables().begin();
        DbColumn const* fkColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        DbColumn const* referencedColumn = sourceTable->FindFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

        //Create FK from Target-Primary to LinkTable
        DbTable const* targetTable = *relationClassMapInfo.GetTargetTables().begin();
        fkColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        referencedColumn = targetTable->FindFirst(DbColumn::Kind::ECInstanceId);
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

    AddIndices(ctx, DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClass->GetRelationshipClassCP()));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(ClassMappingContext& ctx, RelationshipMappingInfo const& mapInfo, ECRelationshipEnd relationshipEnd)
    {
    DbColumn* endECClassIdColumn = nullptr;
    ECRelationshipClassCP relationship = mapInfo.GetClass().GetRelationshipClassCP();
    BeAssert(relationship != nullptr);
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? relationship->GetSource() : relationship->GetTarget();
    ECClass const* foreignEndClass = foreignEndConstraint.GetConstraintClasses()[0];
    ClassMap const* foreignEndClassMap = GetDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetDbMap().GetTableCountOnRelationshipEnd(ctx.GetImportCtx(), foreignEndConstraint);

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
        endECClassIdColumn = const_cast<DbColumn*>(foreignEndClassMap->GetPrimaryTable().FindFirst(DbColumn::Kind::ECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), columnId, PersistenceType::Virtual);
        }

    return endECClassIdColumn;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(ClassMappingContext& ctx, RelationshipMappingInfo const& mapInfo, bool addSourceECClassIdColumnToTable,
    bool addTargetECClassIdColumnToTable)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = DetermineConstraintECInstanceIdColumnName(*mapInfo.GetLinkTableMappingInfo(), ECRelationshipEnd_Source);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_SourceECInstanceId " column named '%s'.",
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
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(ctx, mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = DetermineConstraintECInstanceIdColumnName(*mapInfo.GetLinkTableMappingInfo(), ECRelationshipEnd_Target);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_TargetECInstanceId " column named '%s'.",
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
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(ctx, mapInfo, ECRelationshipEnd_Target);
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
    Nullable<Utf8String> name(isUniqueIndex ? "uix_" : "ix_");
    name.ValueR().append(GetClass().GetSchema().GetAlias()).append("_").append(GetClass().GetName()).append("_");

    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                name.ValueR().append("source");
                break;
            case RelationshipIndexSpec::Target:
                name.ValueR().append("target");
                break;
            case RelationshipIndexSpec::SourceAndTarget:
                name.ValueR().append("sourcetarget");
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

    GetDbMap().GetDbSchemaR().CreateIndex(GetPrimaryTable(), name, isUniqueIndex, columns, false,
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
            if (linkTableInfo.GetSourceIdColumnName().IsNull())
                colName.assign(COL_DEFAULTNAME_SourceId);
            else
                colName.assign(linkTableInfo.GetSourceIdColumnName().Value());

            break;
            }
            case ECRelationshipEnd_Target:
            {
            if (linkTableInfo.GetTargetIdColumnName().IsNull())
                colName.assign(COL_DEFAULTNAME_TargetId);
            else
                colName.assign(linkTableInfo.GetTargetIdColumnName().Value());

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
            if (linkTableInfo.GetSourceIdColumnName().IsNull())
                colName.assign(COL_SourceECClassId);
            else
                idColName = &linkTableInfo.GetSourceIdColumnName().Value();
            
            break;
            }

            case ECRelationshipEnd_Target:
            {
            if (linkTableInfo.GetTargetIdColumnName().IsNull())
                colName.assign(COL_TargetECClassId);
            else
                idColName = &linkTableInfo.GetTargetIdColumnName().Value();
            
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
        //default for AllowDuplicateRelationships: false
        Nullable<bool> allowDuplicateRels;
        linkRelMap.TryGetAllowDuplicateRelationships(allowDuplicateRels);
        if (!allowDuplicateRels.IsNull() && allowDuplicateRels.Value())
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
DbColumn* RelationshipClassLinkTableMap::CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind columnKind, PersistenceType persType)
    {
    DbTable& table = GetPrimaryTable();
    const bool wasEditMode = table.GetEditHandle().CanEdit();
    if (!wasEditMode)
        table.GetEditHandleR().BeginEdit();

    DbColumn* column = table.FindColumnP(columnName);
    if (column != nullptr)
        {
        if (!Enum::Intersects(column->GetKind(), columnKind))
            column->AddKind(columnKind);

        return column;
        }

    persType = table.GetType() != DbTable::Type::Existing ? persType : PersistenceType::Virtual;
    //Following protect creating virtual id/fk columns in persisted tables.
    if (table.GetType() != DbTable::Type::Virtual && persType == PersistenceType::Virtual)
        {
        if (columnKind == DbColumn::Kind::SourceECInstanceId || columnKind == DbColumn::Kind::TargetECInstanceId)
            {
            LOG.errorv("Failed to map ECRelationshipClass '%s': No columns found for " ECDBSYS_PROP_SourceECInstanceId " or " ECDBSYS_PROP_TargetECInstanceId " in table '%s'. Consider applying the LinkTableRelationshipMap custom attribute to the ECRelationshipClass.", 
                       GetClass().GetFullName(), table.GetName().c_str());
            return nullptr;
            }
        }
        
    column = table.CreateColumn(Utf8String(columnName), DbColumn::Type::Integer, columnKind, persType);

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
    }

//************************RelationshipClassEndTableMap::ColumnFactory********************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
RelationshipClassEndTableMap::ColumnFactory::ColumnFactory(RelationshipClassEndTableMap const& relMap, RelationshipMappingInfo const& relInfo)
    :m_relMap(relMap), m_relInfo(relInfo)
    {
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassEndTableMap::ColumnFactory::AllocateForeignKeyECInstanceId(DbTable& table, Utf8StringCR colName, int position)
    {
    const DbColumn::Kind colKind = m_relMap.GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;
    const DbColumn::Type colType = DbColumn::Type::Integer;
    if (m_relInfo.GetFkMappingInfo()->IsPhysicalFk())
        return table.CreateColumn(colName, colType, position, colKind, PersistenceType::Physical);

    DbTable* resolvedTable = &table;
    if (table.GetType() == DbTable::Type::Overflow)
        resolvedTable = &resolvedTable->GetLinkNode().GetParent()->GetTableR();

    auto itor = m_scopes.find(resolvedTable);
    if (itor == m_scopes.end())
        {
        BeAssert(false);
        return nullptr;
        }

    ClassMapCP rootClassMap = &itor->second->GetClassMap();
    //ECDB_RULE: If IsPhysicalFK is false and we do not have shared column support go a head and create a column
    if (!rootClassMap->GetMapStrategy().IsTablePerHierarchy() ||
        rootClassMap->GetMapStrategy().GetTphInfo().GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        return table.CreateColumn(colName, colType, position, colKind, PersistenceType::Physical);
        };
    
    //Create shared column
    ECSqlSystemPropertyInfo const& constraintECInstanceIdType = m_relMap.GetReferencedEnd() == ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECInstanceId() : ECSqlSystemPropertyInfo::TargetECInstanceId();
    ECDbSystemSchemaHelper const& systemSchemaHelper = m_relMap.GetDbMap().GetECDb().Schemas().GetReader().GetSystemSchemaHelper();
    ECPropertyCP  constraintECInstanceIdProp = systemSchemaHelper.GetSystemProperty(constraintECInstanceIdType);
    if (rootClassMap->GetColumnFactory().UsesSharedColumnStrategy())
        {
        auto itor = m_sharedBlock.find(rootClassMap);
        if (itor == m_sharedBlock.end())
            {
            rootClassMap->GetColumnFactory().ReserveSharedColumns(2);
            m_sharedBlock.insert(rootClassMap);
            itor = m_sharedBlock.end();
            }

        DbColumn* col = rootClassMap->GetColumnFactory().Allocate(*constraintECInstanceIdProp, colType, DbColumn::CreateParams(colName.c_str()), m_relMap.GetAcccessStringForId());
        if (itor != m_sharedBlock.end())
            {
            rootClassMap->GetColumnFactory().ReleaseSharedColumnReservation();
            m_sharedBlock.erase(itor);
            }

        return col;
        }

    return rootClassMap->GetColumnFactory().Allocate(*constraintECInstanceIdProp, colType, DbColumn::CreateParams(colName.c_str()), m_relMap.GetAcccessStringForId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassEndTableMap::ColumnFactory::AllocateForeignKeyRelECClassId(DbTable& table, Utf8StringCR colName, PersistenceType persType, int position)
    {
    const DbColumn::Type colType = DbColumn::Type::Integer;
    const DbColumn::Kind colKind = DbColumn::Kind::RelECClassId;
    if (m_relInfo.GetFkMappingInfo()->IsPhysicalFk() || persType == PersistenceType::Virtual)
        return table.CreateColumn(colName, colType, position, colKind, persType);

    DbTable* resolvedTable = &table;
    if (table.GetType() == DbTable::Type::Overflow)
        resolvedTable = &resolvedTable->GetLinkNode().GetParent()->GetTableR();

    auto itor = m_scopes.find(resolvedTable);
    if (itor == m_scopes.end())
        {
        BeAssert(false);
        return nullptr;
        }

    ClassMapCP rootClassMap = &itor->second->GetClassMap();
    //ECDB_RULE: If IsPhysicalFK is false and we do not have shared column support go a head and create a column
    if (!rootClassMap->GetMapStrategy().IsTablePerHierarchy() ||
        rootClassMap->GetMapStrategy().GetTphInfo().GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        return table.CreateColumn(colName, colType, position, colKind, persType);
        }

    ECDbSystemSchemaHelper const& systemSchemaHelper = m_relMap.GetDbMap().GetECDb().Schemas().GetReader().GetSystemSchemaHelper();
    //ECDB_RULE: Note we are using ECClassId here its not a mistake.
    ECPropertyCP relECClassIdProp = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::ECClassId());
    if (rootClassMap->GetColumnFactory().UsesSharedColumnStrategy())
        {
        auto itor = m_sharedBlock.find(rootClassMap);
        if (itor == m_sharedBlock.end())
            {
            rootClassMap->GetColumnFactory().ReserveSharedColumns(2);
            m_sharedBlock.insert(rootClassMap);
            itor = m_sharedBlock.end();
            }

        DbColumn* col = rootClassMap->GetColumnFactory().Allocate(*relECClassIdProp, colType, DbColumn::CreateParams(colName.c_str()), m_relMap.GetAcccessStringForRelClassId());
        if (itor != m_sharedBlock.end())
            {
            rootClassMap->GetColumnFactory().ReleaseSharedColumnReservation();
            m_sharedBlock.erase(itor);
            }

        return col;
        }

    return rootClassMap->GetColumnFactory().Allocate(*relECClassIdProp, colType, DbColumn::CreateParams(colName.c_str()), m_relMap.GetAcccessStringForRelClassId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         01/2017
//---------------------------------------------------------------------------------------
void RelationshipClassEndTableMap::ColumnFactory::Initialize()
    {
    DbMap const& ecdbMap = m_relMap.GetDbMap();
    ECN::ECRelationshipConstraintCR constraint = m_relMap.GetForeignEnd() == ECN::ECRelationshipEnd_Source ? m_relMap.GetRelationshipClass().GetSource() : m_relMap.GetRelationshipClass().GetTarget();
    std::function<void(DbMap const&, ECClassCR, ECN::ECRelationshipConstraintCR)> traverseConstraintClass;
    bmap<DbTable const*, bset<ClassMapCP>> constraintClassMaps;
    traverseConstraintClass = [&traverseConstraintClass,&constraintClassMaps] (DbMap const& ecdbMap, ECClassCR constraintClass, ECN::ECRelationshipConstraintCR constraint)
        {
        ClassMapCP classMap = ecdbMap.GetClassMap(constraintClass);
        if (classMap != nullptr)
            {
            constraintClassMaps[&classMap->GetJoinedOrPrimaryTable()].insert(classMap);
            if (classMap->GetMapStrategy().IsTablePerHierarchy())
                return;
            }

        if (!constraint.GetIsPolymorphic())
            return;

        for (ECClassCP derivedClass : ecdbMap.GetECDb().Schemas().GetDerivedClasses(constraintClass))
            traverseConstraintClass(ecdbMap, *derivedClass, constraint);
        };

    for (ECN::ECClassCP constraintClass : constraint.GetConstraintClasses())
        traverseConstraintClass(ecdbMap, *constraintClass, constraint);

    for (auto const& entry : constraintClassMaps)
        {
        std::vector<ClassMap const*> vect = std::vector<ClassMap const*>(entry.second.begin(), entry.second.end());
        m_scopes.insert(std::make_pair(entry.first, std::unique_ptr<EndTableRelationshipColumnResolutionScope>(new EndTableRelationshipColumnResolutionScope(*vect.front(), vect))));
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
