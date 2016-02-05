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
RelationshipClassMap::RelationshipClassMap(ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
    : ClassMap(ecRelClass, ecDbMap, mapStrategy, setIsDirty),
    m_sourceConstraintMap(ecDbMap.GetECDbR().Schemas(), ecRelClass.GetSource()),
    m_targetConstraintMap(ecDbMap.GetECDbR().Schemas(), ecRelClass.GetTarget())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn* RelationshipClassMap::CreateConstraintColumn(Utf8CP columnName, ColumnKind columnId, PersistenceType persType)
    {
    ECDbSqlColumn* column = GetPrimaryTable().FindColumnP(columnName);
    if (column != nullptr)
        {
        if (!Enum::Intersects(column->GetKind(), columnId))
            column->AddKind(columnId);

        return column;
        }

    if (GetPrimaryTable().IsOwnedByECDb())
        {
        column = GetPrimaryTable().CreateColumn(columnName, ECDbSqlColumn::Type::Long, columnId, persType);
        }
    else
        {
        GetPrimaryTable().GetEditHandleR().BeginEdit();
        column = GetPrimaryTable().CreateColumn(columnName, ECDbSqlColumn::Type::Long, columnId, PersistenceType::Virtual);
        GetPrimaryTable().GetEditHandleR().EndEdit();
        }

    return column;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn* RelationshipClassMap::CreateConstraintColumn(ECDbSqlTable& table, Utf8CP columnName, ColumnKind columnId, PersistenceType persType)
    {
    ECDbSqlColumn* column = table.FindColumnP(columnName);
    if (column != nullptr)
        {
        if (!Enum::Intersects(column->GetKind(), columnId))
            column->AddKind(columnId);

        return column;
        }

    if (table.IsOwnedByECDb())
        {
        column = table.CreateColumn(columnName, ECDbSqlColumn::Type::Long, columnId, persType);
        }

    return column;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                        12/13
//---------------------------------------------------------------------------------------
//static
bool RelationshipClassMap::ConstraintIncludesAnyClass (ECConstraintClassesList const& constraintClasses)
    {
    for (auto& ecclass : constraintClasses)
        {
        if (IsAnyClass (*ecclass))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
void RelationshipClassMap::DetermineConstraintClassIdColumnHandling (bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECRelationshipConstraintCR constraint) const
    {
    //A constraint class id column is needed if 
    // * the map strategy implies that multiple classes are stored in the same table or
    // * the constraint includes the AnyClass or 
    // * it has more than one classes including subclasses in case of a polymorphic constraint. 
    //So we first determine whether a constraint class id column is needed
    auto const& constraintClasses = constraint.GetClasses ();
    addConstraintClassIdColumnNeeded = constraintClasses.size () > 1 || ConstraintIncludesAnyClass (constraintClasses);
    //if constraint is polymorphic, and if addConstraintClassIdColumnNeeded is not true yet,
    //we also need to check if the constraint classes have subclasses. If there is at least one, addConstraintClassIdColumnNeeded
    //is set to true;
    if (!addConstraintClassIdColumnNeeded && constraint.GetIsPolymorphic ())
        {
        addConstraintClassIdColumnNeeded = true;
        }

    //if no class id column on the end is required, store the class id directly so that it can be used as literal in the native SQL
    if (!addConstraintClassIdColumnNeeded)
        {
        BeAssert (constraintClasses.size () == 1);
        auto constraintClass = constraintClasses[0];
        BeAssert (constraintClass->HasId ());
        defaultConstraintClassId = constraintClass->GetId ();
        }
    else
        defaultConstraintClassId = ECClass::UNSET_ECCLASSID;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ClassDbView> RelationshipClassMap::CreateClassDbView()
    {
    return std::unique_ptr<ClassDbView> (new ClassDbView (*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& RelationshipClassMap::GetConstraintMap (ECN::ECRelationshipEnd constraintEnd) const
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
RelationshipEndColumns const& RelationshipClassMap::GetEndColumnsMapping(RelationshipMapInfo const& info, ECN::ECRelationshipEnd end)
    {
    return info.GetColumnsMapping(end);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
PropertyMapCP RelationshipClassMap::GetConstraintECInstanceIdPropMap (ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECInstanceIdPropMap();
    else
        return GetTargetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
PropertyMapRelationshipConstraintClassId const* RelationshipClassMap::GetConstraintECClassIdPropMap (ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECClassIdPropMap();
    else
        return GetTargetECClassIdPropMap();
    }


//************************ RelationshipConstraintMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
bool RelationshipConstraintMap::ClassIdMatchesConstraint (ECN::ECClassId candidateClassId) const
    {
    CacheClassIds ();

    if (m_anyClassMatches)
        return true;

    return m_ecClassIdCache.find (candidateClassId) != m_ecClassIdCache.end ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                               Muhammad.Zaighum                        04/13
//---------------------------------------------------------------------------------------
ECN::ECRelationshipConstraintCR RelationshipConstraintMap::GetRelationshipConstraint()const
    {
    return m_constraint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
bool RelationshipConstraintMap::TryGetSingleClassIdFromConstraint (ECClassId& constraintClassId) const
    {
    CacheClassIds ();

    if (m_anyClassMatches || m_ecClassIdCache.size () != 1)
        return false;

    BeAssert (m_ecClassIdCache.size () == 1);
    constraintClassId = *m_ecClassIdCache.begin ();
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipConstraintMap::CacheClassIds () const
    {
    if (!m_isCacheSetup)
        {
        CacheClassIds (m_constraint.GetClasses (), m_constraint.GetIsPolymorphic ());
        m_isCacheSetup = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipConstraintMap::CacheClassIds(ECConstraintClassesList const& constraintClassList, bool constraintIsPolymorphic) const
    {
    //runs through all constraint classes, and if recursive is true through subclasses
    //and caches the class ids as this method here is expensive performance-wise.
    for (ECClassCP constraintClass : constraintClassList)
        {
        if (IClassMap::IsAnyClass(*constraintClass))
            {
            SetAnyClassMatches();
            return;
            }

        const ECClassId classId = constraintClass->GetId();
        CacheClassId(classId);

        if (constraintIsPolymorphic)
            {
            //call into schema manager to ensure that the derived classes are loaded if needed
            ECConstraintClassesList derivedClasses;
            for (auto derivedClass : m_schemaManager.GetDerivedECClasses(*constraintClass))
                derivedClasses.push_back(derivedClass->GetEntityClassP());
            CacheClassIds(derivedClasses, constraintIsPolymorphic);
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipConstraintMap::CacheClassId (ECN::ECClassId classId) const
    {
    m_ecClassIdCache.insert (classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipConstraintMap::SetAnyClassMatches () const
    {
    m_anyClassMatches = true;
    //class id cache not needed if any class matches
    m_ecClassIdCache.clear ();
    }

//************************ RelationshipClassEndTableMap **********************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassEndTableMap::RelationshipClassEndTableMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: RelationshipClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty), m_autogenerateForeignKeyColumns(true)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
DataIntegrityEnforcementMethod RelationshipClassEndTableMap::GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetTableType() == TableType::Existing || GetRelationshipClass().GetClassModifier() == ECClassModifier::Abstract
        || GetRelationshipClass().GetSource().GetClasses().empty() || GetRelationshipClass().GetTarget().GetClasses().empty() )
        return DataIntegrityEnforcementMethod::None;

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing || GetRelationshipClass().GetStrength() == StrengthType::Embedding || GetRelationshipClass().GetStrength() == StrengthType::Holding)
        {
        return DataIntegrityEnforcementMethod::ForeignKey;
        }

    BeAssert(false && "Trigger are not supported");
    return DataIntegrityEnforcementMethod::Trigger;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_MapPart1(SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    //Don't call base class method as end table map requires its own handling
    BeAssert(GetMapStrategy().IsForeignKeyMapping());
    m_dbView = CreateClassDbView();
    RelationshipMapInfo const& relationshipClassMapInfo = dynamic_cast<RelationshipMapInfo const&> (classMapInfo);
    BeAssert(m_ecClass.GetRelationshipClassCP() != nullptr && classMapInfo.GetMapStrategy().IsForeignKeyMapping());
    ECRelationshipClassCR relationshipClass = GetRelationshipClass();


    std::set<ECDbSqlTable const*> foreignEndTables = GetForeignEnd() == ECRelationshipEnd_Source ? relationshipClassMapInfo.GetSourceTables() : relationshipClassMapInfo.GetTargetTables();
    ECRelationshipConstraintCR const& foreignEndConstraint = GetForeignEnd() == ECRelationshipEnd_Source ? relationshipClass.GetSource() : relationshipClass.GetTarget();
    ECRelationshipConstraintCR const& referencedEndConstraint = GetReferencedEnd() == ECRelationshipEnd_Source ? relationshipClass.GetSource() : relationshipClass.GetTarget();

    //! table must meet following constraint though these are already validated at MapStrategy evaluation time.
    BeAssert(foreignEndTables.size() >= 1 && "ForeignEnd Tables must be >= 1");
    BeAssert(GetReferencedEnd() == ECRelationshipEnd_Source ? relationshipClassMapInfo.GetSourceTables().size() == 1 : relationshipClassMapInfo.GetTargetTables().size() == 1 && "ReferencedEnd Tables must be == 1");


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


    m_autogenerateForeignKeyColumns = true;

    std::set<ECDbSqlColumn const*> fkTableFkCols, referencedTablePrimaryKeyCols;
    if (SUCCESS != TryGetKeyPropertyColumn(fkTableFkCols, foreignEndConstraint, relationshipClass, GetForeignEnd()))
        return MapStatus::Error;

    if (SUCCESS != TryGetKeyPropertyColumn(referencedTablePrimaryKeyCols, referencedEndConstraint, relationshipClass, GetReferencedEnd()))
        return MapStatus::Error;


    if (!fkTableFkCols.empty())
        m_autogenerateForeignKeyColumns = false;
    else
        {
        Utf8String fkColumnName(relationshipClassMapInfo.GetColumnsMapping(GetForeignEnd()).GetECInstanceIdColumnName());
        if (fkColumnName.empty())
            {
            if (SUCCESS != TryGetConstraintIdColumnNameFromNavigationProperty(fkColumnName, foreignEndConstraint, relationshipClass, GetForeignEnd()))
                return MapStatus::Error;
            }

        if (fkColumnName.empty())
            fkColumnName.append("ForeignECInstanceId_").append(relationshipClass.GetName());

        //Note: The FK column is the column that refers to the referenced end. Therefore the ECRelationshipEnd of the referenced end has to be taken!
        ColumnKind foreignKeyColumnKind = GetReferencedEnd() == ECRelationshipEnd_Source ? ColumnKind::SourceECInstanceId : ColumnKind::TargetECInstanceId;

        for (ECDbSqlTable const* foreignEndTable : foreignEndTables)
            {
            PersistenceType columnPersistenceType = foreignEndTable->IsOwnedByECDb() && foreignEndTable->GetPersistenceType() == PersistenceType::Persisted ? PersistenceType::Persisted : PersistenceType::Virtual;
            ECDbSqlColumn const* column = CreateConstraintColumn(*const_cast<ECDbSqlTable*>(foreignEndTable), fkColumnName.c_str(), foreignKeyColumnKind, columnPersistenceType);
            if (column != nullptr)
                fkTableFkCols.insert(column);
            }
        }

    if (!referencedTablePrimaryKeyCols.empty())
        {
        if (referencedTablePrimaryKeyCols.size() != 1)
            {
            BeAssert(false && "Expecting exactly one column in referencedTablePrimaryKeyCols");
            return MapStatus::Error;
            }

        if ((*referencedTablePrimaryKeyCols.begin())->GetKind() != ColumnKind::ECInstanceId)
            {
            GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(
                ECDbIssueSeverity::Error,
                "KeyProperty specified but not used by ECDb. It should be either set to ECInstanceId or removed. %s Constraint %s",
                relationshipClass.GetFullName(),
                GetReferencedEnd() == ECRelationshipEnd_Source ? "Source" : "Target"
                );

            return MapStatus::Error;
            }
        }
    else
        {
        std::set<ECDbSqlTable const*> referencedEndTables = GetReferencedEnd() == ECRelationshipEnd_Source ? relationshipClassMapInfo.GetSourceTables() : relationshipClassMapInfo.GetTargetTables();
        ECDbSqlTable const* referencedTable = *referencedEndTables.begin();
        BeAssert(referencedTable != nullptr);
        ECDbSqlColumn const* ecInstanceId = referencedTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        if (ecInstanceId == nullptr)
            {
            BeAssert(ecInstanceId != nullptr);
            return MapStatus::Error;
            }

        referencedTablePrimaryKeyCols.insert(ecInstanceId);
        }

    ECDbSqlColumn const* referencedTablePKCol = *referencedTablePrimaryKeyCols.begin();
    ECDbSqlTable const& referencedTable = referencedTablePKCol->GetTable();

    //all sets are columns in the FK table
    std::set<ECDbSqlColumn const*> fkTablePkCols;
    std::set<ECDbSqlColumn const*> fkTableClassIdCols;
    std::set<ECDbSqlColumn const*> fkTableReferencedEndClassIdCols;
    //Create property maps and foreign key constraints
    for (ECDbSqlColumn const* fkCol : fkTableFkCols)
        {
        ECDbSqlTable& fkTable = const_cast<ECDbSqlTable &>(fkCol->GetTable());
        ECDbSqlColumn const* fkTableClassIdCol = fkTable.GetFilteredColumnFirst(ColumnKind::ECClassId);
        //If ForeignEndClassId column is missing create a virtual one
        if (fkTableClassIdCol == nullptr)
            {
            Utf8CP colName = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
            ColumnKind kind = GetForeignEnd() == ECRelationshipEnd_Source ? ColumnKind::SourceECClassId : ColumnKind::TargetECClassId;

            fkTableClassIdCol = fkTable.FindColumnCP(colName);
            if (fkTableClassIdCol == nullptr)
                {
                const bool readonly = !fkTable.GetEditHandle().CanEdit();
                if (readonly) 
                    fkTable.GetEditHandleR().BeginEdit();

                fkTableClassIdCol = fkTable.CreateColumn(colName, ECDbSqlColumn::Type::Long, kind, PersistenceType::Virtual);

                if (readonly) 
                    fkTable.GetEditHandleR().EndEdit();
                }
            else
                {
                if (fkTableClassIdCol->GetKind() != kind || fkTableClassIdCol->GetPersistenceType() != PersistenceType::Virtual)
                    {
                    BeAssert(false && "Expecting virtual column");
                    return MapStatus::Error;
                    }
                }
            }

        fkTablePkCols.insert(fkTable.GetFilteredColumnFirst(ColumnKind::ECInstanceId));
        fkTableClassIdCols.insert(fkTableClassIdCol);

        ECDbSqlColumn const* referencedTableClassIdCol = referencedTable.GetFilteredColumnFirst(ColumnKind::ECClassId);

        //if referenced table doesn't have a class id col, create a virtual one in the foreign end table
        if (referencedTableClassIdCol == nullptr)
            {
            Utf8CP colName = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
            ColumnKind kind = GetReferencedEnd() == ECRelationshipEnd_Source ? ColumnKind::SourceECClassId : ColumnKind::TargetECClassId;

            ECDbSqlColumn const* referencedEndClassIdCol = fkTable.FindColumnCP(colName);
            if (referencedEndClassIdCol == nullptr)
                {
                const bool readonly = !fkTable.GetEditHandle().CanEdit();
                if (readonly) 
                    fkTable.GetEditHandleR().BeginEdit();
                
                referencedEndClassIdCol = fkTable.CreateColumn(colName, ECDbSqlColumn::Type::Long, kind, PersistenceType::Virtual);
                
                if (readonly) 
                    fkTable.GetEditHandleR().EndEdit();
                }
            else
                {
                if (referencedEndClassIdCol->GetKind() != kind || referencedEndClassIdCol->GetPersistenceType() != PersistenceType::Virtual)
                    {
                    BeAssert(false && "Expecting virtual column");
                    return MapStatus::Error;
                    }
                }

            fkTableReferencedEndClassIdCols.insert(referencedEndClassIdCol);
            }

        ForeignKeyActionType userRequestedDeleteAction = relationshipClassMapInfo.CreateForeignKeyConstraint() ? relationshipClassMapInfo.GetOnDeleteAction() : ForeignKeyActionType::NotSpecified;

        //! Create Foreign Key constraint only if FK is not a virtual or existing table.
        if (fkTable.IsOwnedByECDb() && fkTable.GetPersistenceType() == PersistenceType::Persisted
            && referencedTable.GetPersistenceType() == PersistenceType::Persisted)
            {
            ECDbSqlForeignKeyConstraint* foreignKeyConstraint = fkTable.CreateForeignKeyConstraint(referencedTable);
            foreignKeyConstraint->Add(fkCol->GetName().c_str(), referencedTablePKCol->GetName().c_str());
            if (userRequestedDeleteAction != ForeignKeyActionType::NotSpecified)
                foreignKeyConstraint->SetOnDeleteAction(userRequestedDeleteAction);
            else
                {
                if (GetRelationshipClass().GetStrength() == StrengthType::Embedding)
                    foreignKeyConstraint->SetOnDeleteAction(ForeignKeyActionType::Cascade);
                else
                    foreignKeyConstraint->SetOnDeleteAction(ForeignKeyActionType::SetNull);
                }

            //! remove the fk constraint if already exist due to another relationship on same column
            foreignKeyConstraint->RemoveIfDuplicate();
            }
        }

    //SourceECInstanceId 
    //SourcePrimaryECInstanceId

    //Create ECinstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    std::vector<ECDbSqlColumn const*> fkTablePkColsVector(fkTablePkCols.begin(), fkTablePkCols.end());
    PropertyMapPtr ecInstanceIdPropMap = PropertyMapECInstanceId::Create(GetECDbMap().GetECDbR().Schemas(), *this, fkTablePkColsVector);
    if (ecInstanceIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMapECInstanceId");
        return MapStatus::Error;
        }

    //Set tables
    for (ECDbSqlColumn const* fkTablePkCol : fkTablePkCols)
        {
        SetTable(fkTablePkCol->GetTableR(), true);
        }

    //Add primary key property map
    GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropMap);


    { 
    //ForeignEnd ECInstanceId PropMap
    Utf8CP fkTableColAlias = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME : ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;
    PropertyMapPtr propertyMap = PropertyMapRelationshipConstraintECInstanceId::Create(GetForeignEnd(), Schemas(), fkTablePkColsVector, fkTableColAlias);
    if (propertyMap == nullptr)
        {
        BeAssert(false);
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(propertyMap);
    GetConstraintMapR(GetForeignEnd()).SetECInstanceIdPropMap(propertyMap.get());
    }

    //Create ForeignEnd ClassId propertyMap
    {
    Utf8CP fkTableColAlias = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;

    RefCountedPtr<PropertyMapRelationshipConstraintClassId> propertyMap = PropertyMapRelationshipConstraintClassId::Create(GetForeignEnd(), Schemas(), std::vector<ECDbSqlColumn const*>(fkTableClassIdCols.begin(), fkTableClassIdCols.end()),
        foreignEndConstraint.GetClasses().front()->GetId(), *this, fkTableColAlias);

    if (propertyMap == nullptr)
        {
        BeAssert(false);
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(propertyMap);
    GetConstraintMapR(GetForeignEnd()).SetECClassIdPropMap(propertyMap.get());
    }

    { //Setup 
    PropertyMapPtr propertyMap = PropertyMapRelationshipConstraintECInstanceId::Create(GetReferencedEnd(), Schemas(),
        std::vector<ECDbSqlColumn const*>(fkTableFkCols.begin(), fkTableFkCols.end()));

    if (propertyMap == nullptr)
        {
        BeAssert(false);
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(propertyMap);
    GetConstraintMapR(GetReferencedEnd()).SetECInstanceIdPropMap(propertyMap.get());
    }

    {
    Utf8CP fkTableColAlias = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;

    RefCountedPtr<PropertyMapRelationshipConstraintClassId> propertyMap = PropertyMapRelationshipConstraintClassId::Create(GetReferencedEnd(), Schemas(),
        std::vector<ECDbSqlColumn const*>(fkTableReferencedEndClassIdCols.begin(), fkTableReferencedEndClassIdCols.end()),
        referencedEndConstraint.GetClasses().front()->GetId(), *this, fkTableColAlias);

    if (propertyMap == nullptr)
        {
        BeAssert(false);
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(propertyMap);
    GetConstraintMapR(GetReferencedEnd()).SetECClassIdPropMap(propertyMap.get());
    }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_MapPart2(SchemaImportContext& schemaImportContext, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    //add non-system property maps
    AddPropertyMaps(schemaImportContext.GetClassMapLoadContext(), parentClassMap, nullptr, &classMapInfo);
    
    AddIndexToRelationshipEnd(schemaImportContext, classMapInfo);
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::_Load (std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    if (ClassMap::_Load (loadGraph, ctx, mapInfo, parentClassMap) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    m_dbView = CreateClassDbView ();   
    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();


    ECClassId defaultSourceECClassId, defaultTargetECClassId;

    const Utf8CP referencedEndIdentECClassId = GetReferencedEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    const Utf8CP foreignEndIdentECClassId = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    const Utf8CP foreignEndEndIdentECId = GetForeignEnd() == ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME : ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;


    if (GetForeignEnd() == ECRelationshipEnd_Source)
        {
        defaultSourceECClassId = relationshipClass.GetSource().GetClasses().empty() ? ECClass::UNSET_ECCLASSID : relationshipClass.GetSource().GetClasses().front()->GetId();
        defaultTargetECClassId = relationshipClass.GetTarget().GetClasses().empty() ? ECClass::UNSET_ECCLASSID : relationshipClass.GetTarget().GetClasses().front()->GetId();
        }
    else
        {
        defaultSourceECClassId = relationshipClass.GetTarget().GetClasses().empty() ? ECClass::UNSET_ECCLASSID : relationshipClass.GetTarget().GetClasses().front()->GetId();
        defaultTargetECClassId = relationshipClass.GetSource().GetClasses().empty() ? ECClass::UNSET_ECCLASSID : relationshipClass.GetSource().GetClasses().front()->GetId();
        }

    //SourceECInstanceId
    auto id = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECInstanceId)->GetId();
    auto propertyinfo = mapInfo.FindPropertyMap (id, ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }
    else
        {
        auto alias = (GetForeignEnd() == ECRelationshipEnd_Source ? foreignEndEndIdentECId : nullptr);
        auto map = PropertyMapRelationshipConstraintECInstanceId::Create(ECRelationshipEnd_Source, Schemas(), propertyinfo->GetColumns(), alias);
        PRECONDITION(map.IsValid(), BentleyStatus::ERROR);
        GetPropertyMapsR().AddPropertyMap(map);
        m_sourceConstraintMap.SetECInstanceIdPropMap(map.get());
        }

    //TargetECInstanceId
    id = ECDbSystemSchemaHelper::GetSystemProperty(Schemas(), ECSqlSystemProperty::SourceECClassId)->GetId();
    propertyinfo = mapInfo.FindPropertyMap(id, ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }
    else
        {
        auto alias = (GetForeignEnd() == ECRelationshipEnd_Source ? foreignEndIdentECClassId : referencedEndIdentECClassId);
        auto map = PropertyMapRelationshipConstraintClassId::Create(ECRelationshipEnd_Source, Schemas(), propertyinfo->GetColumns(), defaultSourceECClassId,*this, alias);
        PRECONDITION(map.IsValid(), BentleyStatus::ERROR);
        GetPropertyMapsR().AddPropertyMap(map);
        m_sourceConstraintMap.SetECClassIdPropMap(map.get());
        }


    //TargetECInstanceId
    id = ECDbSystemSchemaHelper::GetSystemProperty(Schemas(), ECSqlSystemProperty::TargetECInstanceId)->GetId();
    propertyinfo = mapInfo.FindPropertyMap(id, ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }
    else
        {
        auto alias = (GetForeignEnd() == ECRelationshipEnd_Target ? foreignEndEndIdentECId : nullptr);
        auto map = PropertyMapRelationshipConstraintECInstanceId::Create(ECRelationshipEnd_Target, Schemas(), propertyinfo->GetColumns(), alias);
        PRECONDITION(map.IsValid(), BentleyStatus::ERROR);
        GetPropertyMapsR().AddPropertyMap(map);
        m_targetConstraintMap.SetECInstanceIdPropMap(map.get());
        }


    //TargetECClassId
    id = ECDbSystemSchemaHelper::GetSystemProperty(Schemas(), ECSqlSystemProperty::TargetECClassId)->GetId();
    propertyinfo = mapInfo.FindPropertyMap(id, ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (propertyinfo == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }
    else
        {
        auto alias = (GetForeignEnd() == ECRelationshipEnd_Target ? foreignEndIdentECClassId : referencedEndIdentECClassId);
        auto map = PropertyMapRelationshipConstraintClassId::Create(ECRelationshipEnd_Target, Schemas(), propertyinfo->GetColumns(), defaultTargetECClassId, *this, alias);
        PRECONDITION(map.IsValid(), BentleyStatus::ERROR);
        GetPropertyMapsR().AddPropertyMap(map);
        m_targetConstraintMap.SetECClassIdPropMap(map.get());
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo)
    {
    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&mapInfo) != nullptr);
    RelationshipMapInfo const& relMapInfo = static_cast<RelationshipMapInfo const&> (mapInfo);
    const bool isUniqueIndex = relMapInfo.GetCardinality() == RelationshipMapInfo::Cardinality::OneToOne;

    if (!relMapInfo.CreateIndexOnForeignKey() ||
        (!isUniqueIndex && !m_autogenerateForeignKeyColumns))
        return;

    BeAssert(GetReferencedEndECInstanceIdPropMap() != nullptr);
    std::vector<ECDbSqlColumn const*> referencedEndIdColumns;
    GetReferencedEndECInstanceIdPropMap()->GetColumns(referencedEndIdColumns);
    for (ECDbSqlColumn const* referencedEndIdColumn : referencedEndIdColumns)
        {
        ECDbSqlTable& persistenceEndTable = const_cast<ECDbSqlTable&>(referencedEndIdColumn->GetTable());
        if (persistenceEndTable.GetTableType() == TableType::Existing)
            continue;

        // name of the index
        Utf8String name(isUniqueIndex ? "uix_" : "ix_");
        name.append(persistenceEndTable.GetName()).append("_fk_").append(m_ecClass.GetSchema().GetNamespacePrefix() + "_" + m_ecClass.GetName());
        if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable)
            name.append("_source");
        else
            name.append("_target");

        NativeSqlBuilder whereClause;
        if (!referencedEndIdColumn->GetConstraint().IsNotNull())
            {
            whereClause.AppendEscaped(referencedEndIdColumn->GetName().c_str()).AppendSpace();
            whereClause.Append(BooleanSqlOperator::IsNot, true).Append("NULL");
            }

        schemaImportContext.GetECDbMapDb().CreateIndex(GetECDbMap().GetECDbR(), persistenceEndTable, name.c_str(), isUniqueIndex, {referencedEndIdColumn}, whereClause.ToString(), true, GetClass().GetId());
        }
    }

   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetRelationshipColumnName (Utf8StringR columnName, ECDbSqlTable const& table, Utf8CP prefix, bool mappingInProgress) const
    {
    columnName.assign(prefix);
    columnName.append(m_ecClass.GetName());
    
    if (table.FindColumnCP (columnName.c_str()) == nullptr)
        return true;

    //! ECSchema Upgrade creates a second column for ECRelationshipClass because of following.
    //! Although following must have been added due to issue. A longterm solution is to store the
    //! name of column in db. Otherwise we cannot tell if column that exist in db is for this this 
    //! relation of some other relation. The information is currently not stored in database. Its
    //! only create of ECDbHints or produced by default values.
    //! We can qualify all relation with schema name to avoid this but it will be breaking changes
    //! as older db will still construct the name by default without schema name. Thus we need the
    //! the persistence of column name for ECRelationship Keys.

    if (mappingInProgress)
        {
        LOG.errorv ("Table %s already contains column named %s. ECRelationship %s has failed to map.", 
            table.GetName().c_str(), columnName.c_str(), m_ecClass.GetFullName());
        return false;
        }
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetReferencedEndKeyColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "ForeignECInstanceId_", mappingInProgress);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetForeignEndECInstanceIdPropMap () const
    {
    return GetConstraintMap (GetForeignEnd ()).GetECInstanceIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetReferencedEndECInstanceIdPropMap () const
    {
    return GetConstraintMap (GetReferencedEnd ()).GetECInstanceIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapRelationshipConstraintClassId const* RelationshipClassEndTableMap::GetReferencedEndECClassIdPropMap () const
    {
    return GetConstraintMap (GetReferencedEnd ()).GetECClassIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetForeignEnd() const
    {
    return GetMapStrategy ().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetReferencedEnd () const
    {
    return GetForeignEnd () == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
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
BentleyStatus RelationshipClassEndTableMap::TryGetKeyPropertyColumn(std::set<ECDbSqlColumn const*>& keyPropertyColumns, ECRelationshipConstraintCR constraint, ECRelationshipClassCR relClass, ECRelationshipEnd constraintEnd) const
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

            LogKeyPropertyRetrievalError(GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter(), "ECRelationshipConstraint Key properties must be specified on all classes of the constraint or on none.",
                relClass, constraintEnd);
            return ERROR;
            }

        if (keyCount > 1 || keys[0].empty())
            {
            LogKeyPropertyRetrievalError(GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter(), "ECDb does not support ECRelationshipConstraint Keys that are empty or made up of multiple properties.",
                relClass, constraintEnd);
            return ERROR;
            }

        if (keyPropertyName.empty())
            keyPropertyName = keys.front().c_str();
        else
            {
            if (keyPropertyName != keys.front())
                {
                LogKeyPropertyRetrievalError(GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter(), "ECDb does not support ECRelationshipConstraint Keys with different accessStrings. All Key properties in constraint must have same name",
                    relClass, constraintEnd);
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
            LogKeyPropertyRetrievalError(GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter(), error.c_str(), relClass, constraintEnd);
            return ERROR;
            }

        ECSqlTypeInfo typeInfo(keyPropertyMap->GetProperty());
        if (!typeInfo.IsExactNumeric() && !typeInfo.IsString())
            {
            Utf8String error;
            error.Sprintf("Unsupported data type of Key property '%s'. ECDb only supports Key properties that have an integral or string data type.", keyPropAccessString);
            LogKeyPropertyRetrievalError(GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter(), error.c_str(), relClass, constraintEnd);
            return ERROR;
            }

        std::vector<ECDbSqlColumn const*> columns;
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
BentleyStatus RelationshipClassEndTableMap::TryGetConstraintIdColumnNameFromNavigationProperty(Utf8StringR columnName, ECN::ECRelationshipConstraintCR constraint, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd constraintEnd) const
    {
    columnName.clear();
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
    ECDbSqlColumn::Constraint::Collation collation;//unused
    return PropertyMap::DetermineColumnInfo(columnName, isNullable, isUnique, collation, *singleNavProperty, singleNavProperty->GetName().c_str());
    }

//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: RelationshipClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty) 
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MapStatus RelationshipClassLinkTableMap::_MapPart1 (SchemaImportContext& context, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    BeAssert (!GetMapStrategy ().IsForeignKeyMapping() &&
        "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");

    auto stat = RelationshipClassMap::_MapPart1 (context, classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    //**** Class View for SELECT statement
    m_dbView = CreateClassDbView ();

    BeAssert (dynamic_cast<RelationshipMapInfo const*> (&classMapInfo) != nullptr);
    RelationshipMapInfo const& relationClassMapInfo = static_cast<RelationshipMapInfo const&> (classMapInfo);

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

    if (HasKeyProperties(sourceConstraint) || HasKeyProperties(targetConstraint))
        {
        GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "The ECRelationshipClass '%s' is mapped to a link table. One of its constraints has Key properties which is only supported for foreign key type relationships.",
                                                                         relationshipClass.GetFullName());
        return MapStatus::Error;
        }

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    ECClassId defaultSourceECClassId = ECClass::UNSET_ECCLASSID;
    DetermineConstraintClassIdColumnHandling (addSourceECClassIdColumnToTable, defaultSourceECClassId, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    ECClassId defaultTargetECClassId = ECClass::UNSET_ECCLASSID;
    DetermineConstraintClassIdColumnHandling (addTargetECClassIdColumnToTable, defaultTargetECClassId, targetConstraint);
    return CreateConstraintPropMaps (relationClassMapInfo, addSourceECClassIdColumnToTable, defaultSourceECClassId, addTargetECClassIdColumnToTable, defaultTargetECClassId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
ECDbSqlColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(RelationshipMapInfo const& mapInfo, ECRelationshipEnd relationshipEnd)
    {
    ECDbSqlColumn* endECClassIdColumn = nullptr;
    ECRelationshipClassCP relationship = mapInfo.GetECClass ().GetRelationshipClassCP ();
    BeAssert (relationship != nullptr);
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? relationship->GetSource () : relationship->GetTarget ();
    ECEntityClass const* foreignEndClass = foreignEndConstraint.GetClasses ()[0];
    ClassMap const* foreignEndClassMap = GetECDbMap ().GetClassMap (*foreignEndClass);
    size_t foreignEndTableCount = GetECDbMap ().GetTableCountOnRelationshipEnd (foreignEndConstraint);

    ColumnKind columnId = relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnKind::SourceECClassId : ColumnKind::TargetECClassId;
    RelationshipEndColumns const& constraintColumnsMapping = GetEndColumnsMapping(mapInfo, relationshipEnd);
    Utf8String columnName(constraintColumnsMapping.GetECClassIdColumnName());
    if (columnName.empty())
        {
        if (!GetConstraintECClassIdColumnName (columnName, relationshipEnd, GetPrimaryTable ()))
            return nullptr;
        }

    if (ConstraintIncludesAnyClass (foreignEndConstraint.GetClasses ()) || foreignEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn (columnName.c_str (), columnId, PersistenceType::Persisted);
        BeAssert (endECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        endECClassIdColumn = const_cast<ECDbSqlColumn*>(foreignEndClassMap->GetPrimaryTable ().GetFilteredColumnFirst (ColumnKind::ECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn (columnName.c_str (), columnId, PersistenceType::Virtual);
        }

    return endECClassIdColumn;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
DataIntegrityEnforcementMethod RelationshipClassLinkTableMap::GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetTableType() == TableType::Existing)
        return DataIntegrityEnforcementMethod::None;

    size_t nSourceTables = GetECDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetSource());
    size_t nTargetTables = GetECDbMap().GetTableCountOnRelationshipEnd(GetRelationshipClass().GetTarget());

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing ||
        GetRelationshipClass().GetStrength() == StrengthType::Holding)
        {
        if (nSourceTables == 1 && nTargetTables == 1)
            {
            return DataIntegrityEnforcementMethod::ForeignKey;
            }
        }

    BeAssert(false && "Trigger not supported");
    return DataIntegrityEnforcementMethod::Trigger;
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MapStatus RelationshipClassLinkTableMap::_MapPart2 (SchemaImportContext& context, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    MapStatus stat = RelationshipClassMap::_MapPart2 (context, classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    RelationshipMapInfo const& relationClassMapInfo = static_cast<RelationshipMapInfo const&> (classMapInfo);


    std::set<ECDbSqlTable const*> sourceTables = relationClassMapInfo.GetSourceTables();
    std::set<ECDbSqlTable const*> targetTables = relationClassMapInfo.GetTargetTables();
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

        GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
            "The ECRelationshipClass '%s' is mapped to a link table, but the %s mapped to more than one table, which is not supported for link tables.",
            GetRelationshipClass().GetFullName(), constraintStr);

        return MapStatus::Error;
        }

    BeAssert(GetRelationshipClass().GetStrength() != StrengthType::Embedding && "Should have caught already in ClassMapInfo");

    if (GetPrimaryTable().GetTableType() != TableType::Existing)
        {
        //Create FK from Source-Primary to LinkTable
        ECDbSqlTable * sourceTable = const_cast<ECDbSqlTable*>(*sourceTables.begin());
        ECDbSqlForeignKeyConstraint* sourceFK = GetPrimaryTable().CreateForeignKeyConstraint(*sourceTable);
        ECDbSqlColumn const* souceColumn = sourceTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        sourceFK->Add(GetSourceECInstanceIdPropMap()->GetSingleColumn()->GetName().c_str(), souceColumn->GetName().c_str());
        sourceFK->SetOnDeleteAction(ForeignKeyActionType::Cascade);
        sourceFK->RemoveIfDuplicate();
        sourceFK = nullptr;


        //Create FK from Target-Primary to LinkTable
        ECDbSqlTable * targetTable = const_cast<ECDbSqlTable*>(*targetTables.begin());
        ECDbSqlForeignKeyConstraint* targetFK = GetPrimaryTable().CreateForeignKeyConstraint(*targetTable);
        ECDbSqlColumn const* targetColumn = targetTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        targetFK->Add(GetTargetECInstanceIdPropMap()->GetSingleColumn()->GetName().c_str(), targetColumn->GetName().c_str());
        targetFK->SetOnDeleteAction(ForeignKeyActionType::Cascade);
        targetFK->RemoveIfDuplicate();
        targetFK = nullptr;
        }

    AddIndices (context, classMapInfo);
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps 
(
RelationshipMapInfo const& mapInfo,
bool addSourceECClassIdColumnToTable,
ECClassId defaultSourceECClassId, 
bool addTargetECClassIdColumnToTable, 
ECClassId defaultTargetECClassId
)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName (mapInfo.GetColumnsMapping(ECRelationshipEnd_Source).GetECInstanceIdColumnName ());
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Source, GetPrimaryTable()))
            return MapStatus::Error;
        }

    auto sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (),ColumnKind::SourceECInstanceId, PersistenceType::Persisted);
    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), PropertyMapSystem::ToVector(sourceECInstanceIdColumn));
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    sourceECInstanceIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    //**** SourceECClassId prop map
    auto sourceECClassIdColumn = ConfigureForeignECClassIdKey (mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdColumnAlias = sourceECClassIdColumn->GetName ().EqualsI (ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME) == true ? nullptr : ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME;
    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), PropertyMapSystem::ToVector(sourceECClassIdColumn), defaultSourceECClassId, *this, sourceECClassIdColumnAlias);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    sourceECClassIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());


    //**** TargetECInstanceId prop map 
    columnName = mapInfo.GetColumnsMapping(ECRelationshipEnd_Target).GetECInstanceIdColumnName();
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Target, GetPrimaryTable()))
            return MapStatus::Error;
        }

    auto targetECInstanceIdColumn = CreateConstraintColumn (columnName.c_str (), ColumnKind::TargetECInstanceId, PersistenceType::Persisted);

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), PropertyMapSystem::ToVector(targetECInstanceIdColumn));
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    targetECInstanceIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());


    //**** TargetECClassId prop map
    auto targetECClassIdColumn = ConfigureForeignECClassIdKey (mapInfo, ECRelationshipEnd_Target);
    auto targetECClassIdColumnAlias = targetECClassIdColumn->GetName ().EqualsI (ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME) == true ? nullptr : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), PropertyMapSystem::ToVector(targetECClassIdColumn), defaultTargetECClassId, *this,targetECClassIdColumnAlias);
    if (targetECClassIdPropMap == nullptr)
        { 
        BeAssert (targetECClassIdPropMap != nullptr);
        return MapStatus::Error;
        }

    targetECClassIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndices (SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo)
    {
    if (GetPrimaryTable ().GetTableType () == TableType::Existing)
        return;

    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&mapInfo) != nullptr);
    RelationshipMapInfo const& relationshipClassMapInfo = static_cast<RelationshipMapInfo const&> (mapInfo);

    RelationshipMapInfo::Cardinality cardinality = relationshipClassMapInfo.GetCardinality();
    const bool enforceUniqueness = !relationshipClassMapInfo.AllowDuplicateRelationships();

    // Add indices on the source and target based on cardinality
    bool sourceIsUnique = enforceUniqueness;
    bool targetIsUnique = enforceUniqueness;

    switch (cardinality)
        {
        //the many side can be unique, but the one side must never be unique
            case RelationshipMapInfo::Cardinality::OneToMany:
                sourceIsUnique = false;
                break;
            case RelationshipMapInfo::Cardinality::ManyToOne:
                targetIsUnique = false;
                break;
              
            case RelationshipMapInfo::Cardinality::ManyToMany:
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

    std::vector<ECDbSqlColumn const*> columns;
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

    schemaImportContext.GetECDbMapDb().CreateIndex(GetECDbMap().GetECDbR(), GetPrimaryTable(), name.c_str(), isUniqueIndex, columns, nullptr,
                                                   true, GetClass().GetId(), 
                                                   //if a partial index is created, it must only apply to this class,
                                                   //not to subclasses, as constraints are not inherited by relationships
                                                   false);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::GenerateIndexColumnList(std::vector<ECDbSqlColumn const*>& columns, ECDbSqlColumn const* col1, ECDbSqlColumn const* col2, ECDbSqlColumn const* col3, ECDbSqlColumn const* col4)
    {
    if (nullptr != col1 && col1->GetPersistenceType () == PersistenceType::Persisted)
        columns.push_back(col1);

    if (nullptr != col2 && col2->GetPersistenceType () == PersistenceType::Persisted)
        columns.push_back(col2);

    if (nullptr != col3 && col3->GetPersistenceType () == PersistenceType::Persisted)
        columns.push_back(col3);

    if (nullptr != col4 && col4->GetPersistenceType () == PersistenceType::Persisted)
        columns.push_back(col4);
    }
    

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECInstanceIdColumnName(Utf8StringR columnName, ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const
    {
    if (columnName.empty())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME : ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME;

    if (table.FindColumnCP(columnName.c_str()) == nullptr)
        return true;

    if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable)
        return true;

    //Following error occure in Upgrading ECSchema but is not fatal.
    LOG.errorv("Table %s already contains column named %s. ECRelationship %s has failed to map.",
               table.GetName().c_str(), columnName.c_str(), m_ecClass.GetFullName());
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECClassIdColumnName (Utf8StringR columnName, ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const
    {
    if (columnName.empty ())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;

    if (table.FindColumnCP (columnName.c_str ()) == nullptr)
        return true;

    if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable)
        return true;

    //Following error occure in Upgrading ECSchema but is not fatal.
    LOG.errorv("Table %s already contains column named %s. ECRelationship %s has failed to map.",
               table.GetName().c_str(), columnName.c_str(), m_ecClass.GetFullName());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipClassLinkTableMap::_Load (std::set<ClassMap const*>& loadGraph, ClassMapLoadContext& ctx, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    if (ClassMap::_Load (loadGraph, ctx, mapInfo, parentClassMap) != BentleyStatus::SUCCESS)
        return ERROR;

    m_dbView = CreateClassDbView ();
    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

    ECClassId defaultSourceECClassId = sourceConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : sourceConstraint.GetClasses().front()->GetId();
    ECClassId defaultTargetECClassId = targetConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : targetConstraint.GetClasses().front()->GetId();

    auto sourceECInstanceIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECInstanceId);
    auto pm = mapInfo.FindPropertyMap (sourceECInstanceIdProperty->GetId (), ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), pm->GetColumns());
    PRECONDITION (sourceECInstanceIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());


    auto sourceECClassIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECClassId);
    pm = mapInfo.FindPropertyMap (sourceECClassIdProperty->GetId (), ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return ERROR;
        }


    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), pm->GetColumns(), defaultSourceECClassId, *this, ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    PRECONDITION (sourceECClassIdPropMap.IsValid (), ERROR);
    GetPropertyMapsR ().AddPropertyMap (sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());

    auto targetECInstanceIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::TargetECInstanceId);
    pm = mapInfo.FindPropertyMap (targetECInstanceIdProperty->GetId (), ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), pm->GetColumns());
    PRECONDITION (targetECInstanceIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());

    auto targetECClassIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::TargetECClassId);
    pm = mapInfo.FindPropertyMap (targetECClassIdProperty->GetId (), ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return ERROR;
        }

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), pm->GetColumns(), defaultTargetECClassId, *this, ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    PRECONDITION (targetECClassIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return BentleyStatus::SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                10/2015
//+---------------+---------------+---------------+---------------+---------------+-
//static
bool RelationshipClassLinkTableMap::HasKeyProperties(ECN::ECRelationshipConstraint const& constraint)
    {
    for (ECRelationshipConstraintClassCP constraintClass : constraint.GetConstraintClasses())
        {
        if (!constraintClass->GetKeys().empty())
            return true;
        }

    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
