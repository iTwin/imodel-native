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
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy), m_mapping(true) {}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap::RelationshipClassEndTableMap(ECDb const& ecdb, ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, UpdatableViewInfo const& updatableViewInfo)
    : RelationshipClassMap(ecdb, Type::RelationshipEndTable, ecRelClass, mapStrategy, updatableViewInfo), m_mapping(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* RelationshipClassEndTableMap::CreateRelECClassIdColumn(DbTable& fkTable, ForeignKeyColumnInfo const& fkColInfo, DbColumn const& fkCol,NavigationPropertyMap const& navPropMap) const
    {
    BeAssert(!GetClass().HasBaseClasses() && "CreateRelECClassIdColumn is expected to only be called for root rel classes");
    const bool makeRelClassIdColNotNull = fkCol.DoNotAllowDbNull();

    PersistenceType persType = PersistenceType::Physical;
    if (fkTable.GetType() == DbTable::Type::Virtual || fkTable.GetType() == DbTable::Type::Existing || GetClass().GetClassModifier() == ECClassModifier::Sealed)
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

    DbColumn* relClassIdCol = fkTable.FindColumnP(relECClassIdColName.c_str());
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

    const bool canEdit = fkTable.GetEditHandleR().CanEdit();
    if (!canEdit)
        fkTable.GetEditHandleR().BeginEdit();
    
    if (persType == PersistenceType::Physical)
        {
        Utf8String accessString = navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropRelECClassId;
        relClassIdCol = navPropMap.GetClassMap().GetColumnFactory().Allocate(navPropMap.GetProperty(), DbColumn::Type::Integer, DbColumn::CreateParams(relECClassIdColName), accessString, navPropMap.HasForeignKeyConstraint());
        }
    else
        {
        relClassIdCol = fkTable.CreateColumn(relECClassIdColName, DbColumn::Type::Integer, DbColumn::Kind::DataColumn, persType);
        }

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
        indexName.ValueR().append(fkTable.GetName()).append("_").append(relClassIdCol->GetName());
        DbIndex* index = GetDbMap().GetDbSchemaR().CreateIndex(fkTable, indexName, false, {relClassIdCol}, true, true, GetClass().GetId());
        if (index == nullptr)
            {
            LOG.errorv("Failed to create index on " ECDBSYS_PROP_NavPropRelECClassId " column %s on Table %s.", relClassIdCol->GetName().c_str(), fkTable.GetName().c_str());
            return nullptr;
            }
        }

    if (!canEdit)
        fkTable.GetEditHandleR().EndEdit();

    return relClassIdCol;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn* RelationshipClassEndTableMap::CreateForeignColumn(RelationshipMappingInfo const& classMappingInfo, DbTable&  fkTable, NavigationPropertyMap const& navPropMap, ForeignKeyColumnInfo& fkColInfo)
    {
    ECRelationshipClassCR relClass = *GetClass().GetRelationshipClassCP();
    ECRelationshipConstraintCR foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    ECRelationshipConstraintCR referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    DbColumn::Kind foreignKeyColumnKind = GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId;
    DbColumn* fkColumn = nullptr;
    if (classMappingInfo.GetFkMappingInfo()->UseECInstanceIdAsFk())
        {
        DbColumn const* pkColumn = fkTable.FindFirst(DbColumn::Kind::ECInstanceId);
        DbColumn* pkColumnP = const_cast<DbColumn*> (pkColumn);
        pkColumnP->AddKind(foreignKeyColumnKind);
        fkColumn = const_cast<DbColumn*>(pkColumn);
        }
    else
        {
        GetForeignKeyColumnInfo(fkColInfo, navPropMap);
        const bool multiplicityImpliesNotNullOnFkCol = referencedEndConstraint.GetMultiplicity().GetLowerLimit() > 0;
        Utf8String fkColName;
        if (fkColInfo.CanImplyFromNavigationProperty() && !fkColInfo.GetImpliedFkColumnName().empty())
            fkColName.assign(fkColInfo.GetImpliedFkColumnName());
        else
            {
            //default name: FK_<schema alias>_<rel class name>
            fkColName.assign(DEFAULT_FK_COL_PREFIX).append(relClass.GetSchema().GetAlias()).append("_").append(relClass.GetName());
            }

        DbColumn* fkCol = const_cast<DbColumn*>(fkTable.FindColumn(fkColName.c_str()));
        if (fkTable.GetType() == DbTable::Type::Existing)
            {
            //for existing tables, the FK column must exist otherwise we fail schema import
            if (fkCol != nullptr)
                {
                if (SUCCESS != ValidateForeignKeyColumn(*fkCol, multiplicityImpliesNotNullOnFkCol, foreignKeyColumnKind))
                    return nullptr;

                return fkCol;
                }

            Issues().Report("Failed to map ECRelationshipClass '%s'. It is mapped to the existing table '%s' not owned by ECDb, but doesn't have a foreign key column called '%s'.",
                            relClass.GetFullName(), fkTable.GetName().c_str(), fkColName.c_str());

            return nullptr;
            }

        //table owned by ECDb
        if (fkCol != nullptr)
            {
            Issues().Report("Failed to map ECRelationshipClass '%s'. ForeignKey column name '%s' is already used by another column in the table '%s'.",
                            relClass.GetFullName(), fkColName.c_str(), fkTable.GetName().c_str());
            return nullptr;
            }


        Utf8String accessString = navPropMap.GetAccessString() + "." + ECDBSYS_PROP_NavPropId;        
        fkColumn= navPropMap.GetClassMap().GetColumnFactory().Allocate(navPropMap.GetProperty(), DbColumn::Type::Integer, DbColumn::CreateParams(fkColName), accessString, navPropMap.HasForeignKeyConstraint());
        if (fkColumn == nullptr)
            {
            Issues().Report("Failed to map ECRelationshipClass '%s'. Could not create foreign key column '%s' in table '%s'.",
                            relClass.GetFullName(), fkColName.c_str(), fkTable.GetName().c_str());
            BeAssert(false && "Could not create FK column for end table mapping");
            return nullptr;
            }

        bset<ECClassId> foreignEndConstraintClassIds;
        for (ECClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
            foreignEndConstraintClassIds.insert(constraintClass->GetId());

        const bool makeFkColNotNull = multiplicityImpliesNotNullOnFkCol && fkTable.HasExclusiveRootECClass() && foreignEndConstraintClassIds.find(fkTable.GetExclusiveRootECClassId()) != foreignEndConstraintClassIds.end();
        if (makeFkColNotNull)
            fkColumn->GetConstraintsR().SetNotNullConstraint();
        }

    return fkColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbColumn * RelationshipClassEndTableMap::CreateReferencedClassIdColumn(DbTable & fkTable) const
    {
    DbColumn* fkClassIdColumn = const_cast<DbColumn*>(fkTable.FindFirst(DbColumn::Kind::ECClassId));
    if (fkClassIdColumn != nullptr)
        return fkClassIdColumn;

    Utf8CP colName = GetForeignEnd() == ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
    DbColumn::Kind kind = GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId;

    fkClassIdColumn = const_cast<DbColumn*>(fkTable.FindColumn(colName));
    if (fkClassIdColumn == nullptr)
        {
        const bool readonly = !fkTable.GetEditHandle().CanEdit();
        if (readonly)
            fkTable.GetEditHandleR().BeginEdit();

        fkClassIdColumn = fkTable.CreateColumn(Utf8String(colName), DbColumn::Type::Integer, kind, PersistenceType::Virtual);

        if (readonly)
            fkTable.GetEditHandleR().EndEdit();
        }
    else
        {
        if (fkClassIdColumn->GetKind() != kind || fkClassIdColumn->GetPersistenceType() != PersistenceType::Virtual)
            {
            BeAssert(false && "Expecting virtual column");
            return nullptr;
            }
        }

    return fkClassIdColumn;
    }
ClassMappingStatus RelationshipClassEndTableMap::CreateForiegnKeyConstraint(DbTable const& referencedTable, RelationshipMappingInfo const& classMappingInfo)
    {
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


    for (SingleColumnDataPropertyMap const* fkCol : GetConstraintECInstanceIdPropMap(GetReferencedEnd())->GetDataPropertyMaps())
        {
        DbTable& fkTable = const_cast<DbTable&>(fkCol->GetColumn().GetTable());
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

                return ClassMappingStatus::Error;
                }
            }

        if (fkTable.GetType() == DbTable::Type::Existing || fkTable.GetType() == DbTable::Type::Virtual ||
            referencedTable.GetType() == DbTable::Type::Virtual ||
            fkCol->GetColumn().IsShared() || !classMappingInfo.GetFkMappingInfo()->IsPhysicalFk())
            continue;

        DbColumn const* referencedColumnId = referencedTable.FindFirst(DbColumn::Kind::ECInstanceId);
        fkTable.CreateForeignKeyConstraint(fkCol->GetColumn(), *referencedColumnId, onDelete, onUpdate);
        }

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::FinishMappingForChild(SchemaImportContext& ctx)
    {
    if (GetClass().GetBaseClasses().size() != 1)
        {
        BeAssert(false && "Multi-inheritance of ECRelationshipclasses should have been caught before already");
        return ClassMappingStatus::Error;
        }

    RelationshipClassEndTableMap* baseClassMap = GetRootRelationshipMap(ctx);
    if (baseClassMap->FinishMapping(ctx) != ClassMappingStatus::Success)
        return ClassMappingStatus::Error;

    if (baseClassMap == nullptr || baseClassMap->GetType() != ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "Could not find class map of base ECRelationship class or is not of right type");
        return ClassMappingStatus::Error;
        }

    RelationshipClassEndTableMap const& baseRelClassMap = baseClassMap->GetAs<RelationshipClassEndTableMap>();
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


    //ForeignEnd
    RelationshipConstraintMap const& baseForeignEndConstraintMap = baseRelClassMap.GetConstraintMap(GetForeignEnd());
    if (baseForeignEndConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseForeignEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& foreignEndConstraintMap = GetConstraintMapR(GetForeignEnd());

    //Foreign ECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseForeignEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

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
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    foreignEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    //ReferencedEnd
    RelationshipConstraintMap const& baseReferencedEndConstraintMap = baseRelClassMap.GetConstraintMap(GetReferencedEnd());
    if (baseReferencedEndConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseReferencedEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& referencedEndConstraintMap = GetConstraintMapR(GetReferencedEnd());

    //Referenced ECInstanceId prop map
    clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

    referencedEndConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    //Referenced ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseReferencedEndConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ClassMappingStatus::Error;
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    referencedEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::FinishMapping(SchemaImportContext& ctx)
    {
    if (!m_mapping)
        return ClassMappingStatus::Success;

    if (GetClass().HasBaseClasses())
        {
        return FinishMappingForChild(ctx);
        }

    if (GetState() == ObjectState::Persisted)
        return ClassMappingStatus::Success;

    auto itor = ctx.GetClassMappingInfoCache().find(this);
    if (itor == ctx.GetClassMappingInfoCache().end())
        return ClassMappingStatus::Error;

    RelationshipMappingInfo const& classMappingInfo = static_cast<RelationshipMappingInfo const&> (*itor->second);
    ECRelationshipConstraintCR refConstraint = GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? GetRelationshipClass().GetSource() : GetRelationshipClass().GetTarget();
    
    std::set<DbTable const*> tables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(GetDbMap(), ctx, refConstraint, true);

    for (ECClassCP constraintClass : refConstraint.GetConstraintClasses())
        {
        ClassMapCP constraintClassMap = GetDbMap().GetClassMap(*constraintClass);
        if (constraintClassMap == nullptr)
            return ClassMappingStatus::Error;
    
        if (constraintClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            {
            GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECRelationship '%s'. Its has constraint EClass '%s' which has the 'NotMapped' strategy.",
                                                               GetClass().GetFullName(), constraintClassMap->GetClass().GetFullName());
            return ClassMappingStatus::Error;
            }
        }

    if (tables.size() > 1)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECRelationship '%s'. Its referenced end maps to more then one table.",
                                                           GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    if (tables.empty())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECRelationship '%s'. Its referenced end does not have any physical table.",
                                                           GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }


    DbTable const* primaryTable = *std::begin(tables);
    //DbColumn const* columnId = primaryTable->FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn* columnForeignClassId = const_cast<DbColumn*>(primaryTable->FindFirst(DbColumn::Kind::ECClassId));
    if (columnForeignClassId == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    DbColumn::UpdateKind(*columnForeignClassId, GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId);
    Utf8CP refEndClassId = GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
    if (ConstraintECClassIdPropertyMap* propertyMap = static_cast<ConstraintECClassIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(refEndClassId))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnForeignClassId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, GetReferencedEnd(), {columnForeignClassId});
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECClassId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 3) != SUCCESS)
            return ClassMappingStatus::Error;


        GetConstraintMapR(GetReferencedEnd()).SetECClassIdPropMap(propMap.get());
        }

    if (GetPropertyMapsR().Size() != 6)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECRelationshipClass '%s'. ",
                                                           GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    if (CreateForiegnKeyConstraint(*primaryTable, classMappingInfo) != ClassMappingStatus::Success)
        return ClassMappingStatus::Error;

    AddIndexToRelationshipEnd(classMappingInfo);
    m_mapping = false;
    return ClassMappingStatus::Success;
    }     

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap* RelationshipClassEndTableMap::GetRootRelationshipMap(SchemaImportContext& ctx)
    {
    ECClassCP c = &GetClass();
    while (c->HasBaseClasses())
        {
        if (c->GetBaseClasses().size() > 1 || !c->IsRelationshipClass())
            {
            BeAssert(false);
            return nullptr;
            }

        c = c->GetBaseClasses().front();
        }

    ECRelationshipClassCP relationshipClass = static_cast<ECRelationshipClassCP>(c);
    ClassMapCP classMap = GetDbMap().GetClassMap(*relationshipClass);
    if (classMap == nullptr)
        {
        if (GetDbMap().MapRelationshipClass(ctx, *relationshipClass) != ClassMappingStatus::Success)
            return nullptr;
        }

    return  static_cast<RelationshipClassEndTableMap*>(const_cast<ClassMap*>(GetDbMap().GetClassMap(*relationshipClass)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::UpdatePersistedEndForChild(SchemaImportContext & ctx, NavigationPropertyMap& navPropMap)
    {
    RelationshipClassEndTableMap* relationshipMap = GetRootRelationshipMap(ctx);
    if (relationshipMap == nullptr)
        return ClassMappingStatus::Error;

    return relationshipMap->UpdatePersistedEnd(ctx, navPropMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan            05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::UpdatePersistedEnd(SchemaImportContext& ctx, NavigationPropertyMap& navPropMap)
    {
    BeAssert(!navPropMap.IsComplete());
    if (GetClass().HasBaseClasses())
        return UpdatePersistedEndForChild(ctx, navPropMap);

    //nav prop only supported if going from foreign end (where FK column is persisted) to referenced end
    NavigationECPropertyCP navigationProperty = navPropMap.GetProperty().GetAsNavigationProperty();
    if (GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECClass '%s'. Its NavigationECProperty '%s' refers to a relationship that has the 'NotMapped' strategy. Therefore its dependencies must have that strategy as well.",
                                                           navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str());
        return ClassMappingStatus::Error;
        }

    if (navPropMap.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map ECRelationship '%s'. Its NavigationECProperty '%s' come from a ECClass %s which has the 'NotMapped' strategy.",
                                                           GetClass().GetFullName(), navigationProperty->GetName().c_str(), navigationProperty->GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    const ECRelatedInstanceDirection navDirection = navigationProperty->GetDirection();
    if ((GetForeignEnd() == ECRelationshipEnd_Source && navDirection == ECRelatedInstanceDirection::Backward) ||
        (GetForeignEnd() == ECRelationshipEnd_Target && navDirection == ECRelatedInstanceDirection::Forward))
        {
        Utf8CP constraintEndName = GetForeignEnd() == ECRelationshipEnd_Source ? "Source" : "Target";
        GetDbMap().GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to map Navigation property '%s.%s'. "
                                                                      "Navigation properties can only be defined on the %s constraint ECClass of the respective ECRelationshipClass '%s'. Reason: "
                                                                      "The Foreign Key is mapped to the %s end of this ECRelationshipClass.",
                                                                      navigationProperty->GetClass().GetFullName(), navigationProperty->GetName().c_str(), constraintEndName,
                                                                      GetClass().GetFullName(), constraintEndName);
        return ClassMappingStatus::Error;
        }

    SingleColumnDataPropertyMap const* idPropMap = nullptr;
    SingleColumnDataPropertyMap const* relECClassIdPropMap = nullptr;

    if (ConstraintECInstanceIdPropertyMap const* idPropertyMap = GetConstraintECInstanceIdPropMap(GetReferencedEnd()))
        idPropMap = idPropertyMap->FindDataPropertyMap(navPropMap.GetClassMap());

    if (ECClassIdPropertyMap const*  classIdPropertyMap = GetECClassIdPropertyMap())
        relECClassIdPropMap = classIdPropertyMap->FindDataPropertyMap(navPropMap.GetClassMap());

    if (idPropMap != nullptr && relECClassIdPropMap != nullptr)
        {
        if (navPropMap.SetMembers(idPropMap->GetColumn(), relECClassIdPropMap->GetColumn(), GetClass().GetId()) != SUCCESS)
            return ClassMappingStatus::Error;

        return ClassMappingStatus::Success;
        }

    if (idPropMap != nullptr || relECClassIdPropMap != nullptr)
        {
        BeAssert(false && "programmer error");
        return ClassMappingStatus::Error;
        }


    auto itor = ctx.GetClassMappingInfoCache().find(this);
    if (itor == ctx.GetClassMappingInfoCache().end())
        return ClassMappingStatus::Error;

    RelationshipMappingInfo const& classMappingInfo = static_cast<RelationshipMappingInfo const&> (*itor->second);
    DbTable& fkTable = const_cast<DbTable&>(navPropMap.GetClassMap().GetJoinedOrPrimaryTable());


    /*
                                        Foo------->FooHasGoo-------->Goo
                                Source (Reference)        Target (Foriegn)

    Foo [ts_Foo]
    ----------------------------------------------------------------------------------------------------------------
    ECInstanceId  |  ECClassId   | ...
    ----------------------------------------------------------------------------------------------------------------
        Id          ECClassId

    Goo [ts_Goo]
    ----------------------------------------------------------------------------------------------------------------
    ECInstanceId  |  ECClassId  |  NavProp.Id  |  NavProp.RelECClassId | ...
    ----------------------------------------------------------------------------------------------------------------
        Id           ECClassId       FK_Id          FK_RelECClassId     ...

    FooHasGoo [ts_Goo]
    ----------------------------------------------------------------------------------------------------------------
    ECInstanceId  |  ECClassId  |  SourceECInstanceId  |  SourceECClassId  |  TargetECInstanceId  |  TargetECClassId
    ----------------------------------------------------------------------------------------------------------------
    ts_Goo           ts_Goo           ts_Goo                 ts_Foo              ts_Goo                ts_Goo
    Id         FK_RelECClassId       FK_Id                ECClassId              Id                 ECClassId
    |                |                 |                      |                    |                     |
    1                2                 3                      4                    5                     6
    M                M                 M                      X                    M                     M

    1. columnId
    2. columnClassId
    3. columnForeignId
    4. columnForeignClassId
    5. columnRefId
    6. columnRefClassId

    M  -> Map in this methiod
    X -> Map in Finish ()
    */

    ForeignKeyColumnInfo fkColInfo;
    DbColumn* columnRefId = CreateForeignColumn(classMappingInfo, fkTable, navPropMap, fkColInfo);
    if (columnRefId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn::UpdateKind(*columnRefId, GetReferencedEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId);

    AddTable(columnRefId->GetTableR());
    DbColumn* columnId = const_cast<DbColumn*>(fkTable.FindFirst(DbColumn::Kind::ECInstanceId));
    if (columnId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn* columnClassId = CreateRelECClassIdColumn(fkTable, fkColInfo, *columnRefId, navPropMap);
    if (columnClassId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn::UpdateKind(*columnClassId, DbColumn::Kind::RelECClassId);

    DbColumn* columnForeignClassId = CreateReferencedClassIdColumn(fkTable);
    if (columnForeignClassId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn::UpdateKind(*columnForeignClassId, GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECClassId : DbColumn::Kind::TargetECClassId);

    DbColumn* columnForeignId = const_cast<DbColumn*>(fkTable.FindFirst(DbColumn::Kind::ECInstanceId));
    if (columnForeignId == nullptr)
        return ClassMappingStatus::Error;

    DbColumn::UpdateKind(*columnForeignId, GetForeignEnd() == ECRelationshipEnd_Source ? DbColumn::Kind::SourceECInstanceId : DbColumn::Kind::TargetECInstanceId);


    //[+++ECInstanceId-----------------------------------------------------------------------------------------------------------------------------------]
    if (ECInstanceIdPropertyMap* propertyMap = static_cast<ECInstanceIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(ECDBSYS_PROP_ECInstanceId))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ECInstanceIdPropertyMap> propMap = ECInstanceIdPropertyMap::CreateInstance(*this, {columnId});

        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECInstanceId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 0) != SUCCESS)
            return ClassMappingStatus::Error;
        }

    //[+++ECClassId-----------------------------------------------------------------------------------------------------------------------------------]
    if (ECClassIdPropertyMap* propertyMap = static_cast<ECClassIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(ECDBSYS_PROP_ECClassId))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnClassId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ECClassIdPropertyMap> propMap = ECClassIdPropertyMap::CreateInstance(*this, {columnClassId});
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECClassId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 1) != SUCCESS)
            return ClassMappingStatus::Error;
        }

    Utf8CP refEnd = GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECInstanceId : ECDBSYS_PROP_TargetECInstanceId;
    Utf8CP foreignEnd = GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECInstanceId : ECDBSYS_PROP_TargetECInstanceId;
    Utf8CP foreignEndClassId = GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
    //[-----------------------------------------------------------------------------------------------------------------------------------]
    if (ConstraintECInstanceIdPropertyMap* propertyMap = static_cast<ConstraintECInstanceIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(refEnd))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnRefId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetReferencedEnd(), {columnRefId});
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECClassId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 2) != SUCCESS)
            return ClassMappingStatus::Error;

        GetConstraintMapR(GetReferencedEnd()).SetECInstanceIdPropMap(propMap.get());
        }

    //[-----------------------------------------------------------------------------------------------------------------------------------]
    if (ConstraintECInstanceIdPropertyMap* propertyMap = static_cast<ConstraintECInstanceIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(foreignEnd))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnForeignId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, GetForeignEnd(), {columnForeignId});
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECClassId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 4) != SUCCESS)
            return ClassMappingStatus::Error;

        GetConstraintMapR(GetForeignEnd()).SetECInstanceIdPropMap(propMap.get());
        }

    //[-----------------------------------------------------------------------------------------------------------------------------------]
    if (ConstraintECClassIdPropertyMap* propertyMap = static_cast<ConstraintECClassIdPropertyMap*>(const_cast<PropertyMap*>(GetPropertyMaps().Find(foreignEndClassId))))
        {
        if (SystemPropertyMap::AppendSystemColumnFromNewlyAddedDataTable(*propertyMap, *columnForeignClassId) != SUCCESS)
            return ClassMappingStatus::Error;
        }
    else
        {
        RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, GetForeignEnd(), {columnForeignClassId});
        if (propMap == nullptr)
            {
            BeAssert(false && "Failed to create PropertyMap ECClassId");
            return ClassMappingStatus::Error;
            }

        if (GetPropertyMapsR().Insert(propMap, 5) != SUCCESS)
            return ClassMappingStatus::Error;

        GetConstraintMapR(GetForeignEnd()).SetECClassIdPropMap(propMap.get());
        }

    //[-----------------------------------------------------------------------------------------------------------------------------------]
    if (navPropMap.SetMembers(*columnRefId, *columnClassId, GetClass().GetId()) != SUCCESS)
        return ClassMappingStatus::Error;

    Modified();
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::_Map(ClassMappingContext& ctx)
    {   
    // This call is noop as the relationship get mapped through
    // 1. UpdatePersistedEnd()
    // 2. Finish() - is called
    //----------------------------------------------------------
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
ConstraintECInstanceIdPropertyMap const* RelationshipClassEndTableMap::GetReferencedEndECInstanceIdPropMap() const
    {
    return GetConstraintMap(GetReferencedEnd()).GetECInstanceIdPropMap();
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
void RelationshipClassEndTableMap::GetForeignKeyColumnInfo(ForeignKeyColumnInfo& fkColInfo, NavigationPropertyMap const& navProp) const
    {
    //if not overridden with a PropertyMap CA, the FK column name is implied as <nav prop name>Id.
    //if nav prop name ends with "Id" already, it is not appended again.
    NavigationECPropertyCP singleNavProperty = static_cast<NavigationECPropertyCP>(&navProp.GetProperty());
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
    TablePerHierarchyInfo const& tphInfo = navProp.GetClassMap().GetMapStrategy().GetTphInfo();
    if (tphInfo.IsValid() && tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes)
        {
        //table uses shared columns, so FK col position cannot depend on NavigationProperty position
        fkColInfo.Assign(defaultFkColName, defaultRelClassIdColName);
        return;
        }

    fkColInfo.Assign(defaultFkColName, defaultRelClassIdColName);
    return;
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
    LinkTableRelationshipMapCustomAttribute linkRelMap;
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



END_BENTLEY_SQLITE_EC_NAMESPACE
