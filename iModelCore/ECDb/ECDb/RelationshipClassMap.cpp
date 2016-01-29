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
//static
Utf8CP const RelationshipClassMap::DEFAULT_SOURCEECINSTANCEID_COLUMNNAME = "SourceECInstanceId";
//static
Utf8CP const RelationshipClassMap::DEFAULT_SOURCEECCLASSID_COLUMNNAME = "SourceECClassId";
//static
Utf8CP const RelationshipClassMap::DEFAULT_TARGETECINSTANCEID_COLUMNNAME = "TargetECInstanceId";
//static
Utf8CP const RelationshipClassMap::DEFAULT_TARGETECCLASSID_COLUMNNAME = "TargetECClassId";

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
// @bsimethod                                                    Krischan.Eberle  06/2015
//---------------------------------------------------------------------------------------
//static
RelationshipEndColumns const& RelationshipClassMap::GetEndColumnsMapping(RelationshipMapInfo const& info, ECN::ECRelationshipEnd end)
    {
    return end == ECRelationshipEnd_Source ? info.GetSourceColumnsMapping() : info.GetTargetColumnsMapping();
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
// @bsimethod                                               Affan.Khan       02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSqlColumn* RelationshipClassEndTableMap::ConfigureForeignECClassIdKey(RelationshipMapInfo const& mapInfo, ECRelationshipConstraintCR primaryEndConstraint, ECDbSqlTable const& otheEndTable, size_t primaryEndTableCount)
    {
    RelationshipEndColumns const& constraintColumnsMapping = GetEndColumnsMapping(mapInfo);
    Utf8String classIdColName(constraintColumnsMapping.GetECClassIdColumnName());
    if (classIdColName.empty() &&
        !GetPrimaryEndECClassIdColumnName(classIdColName, GetJoinedTable(), true))
        return nullptr;

    ColumnKind columnId = GetForeignEnd () == ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnKind::TargetECClassId : ColumnKind::SourceECClassId;

    ECDbSqlColumn* primaryEndECClassIdColumn = nullptr;
    if (ConstraintIncludesAnyClass(primaryEndConstraint.GetClasses()) || primaryEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        primaryEndECClassIdColumn = CreateConstraintColumn (classIdColName.c_str (), columnId, PersistenceType::Persisted);
        BeAssert (primaryEndECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        primaryEndECClassIdColumn = const_cast<ECDbSqlColumn*>(otheEndTable.GetFilteredColumnFirst (ColumnKind::ECClassId));
        if (primaryEndECClassIdColumn == nullptr)
            primaryEndECClassIdColumn = CreateConstraintColumn (classIdColName.c_str (), columnId, PersistenceType::Virtual);
        }

    return primaryEndECClassIdColumn;
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         1 / 16
//---------------------------------------------------------------------------------------
DataIntegrityEnforcementMethod RelationshipClassEndTableMap::GetDataIntegrityEnforcementMethod() const
    {
    if (GetPrimaryTable().GetTableType() == TableType::Existing || GetRelationshipClass().GetClassModifier() == ECClassModifier::Abstract
        || GetRelationshipClass().GetSource().GetClasses().empty() || GetRelationshipClass().GetTarget().GetClasses().empty() )
        return DataIntegrityEnforcementMethod::None;

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing || GetRelationshipClass().GetStrength() == StrengthType::Embedding)
        {
        return DataIntegrityEnforcementMethod::ForeignKey;
        }

    return DataIntegrityEnforcementMethod::Trigger;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_MapPart1 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    //Don't call base class method as end table map requires its own handling
    BeAssert (GetMapStrategy ().IsForeignKeyMapping());
    m_dbView = CreateClassDbView ();

    RelationshipMapInfo const& relationshipClassMapInfo = dynamic_cast<RelationshipMapInfo const&> (classMapInfo);
    BeAssert (m_ecClass.GetRelationshipClassCP () != nullptr && classMapInfo.GetMapStrategy ().IsForeignKeyMapping());

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    ECRelationshipConstraintCR sourceConstraint = relationshipClass.GetSource ();
    ECRelationshipConstraintCR targetConstraint = relationshipClass.GetTarget ();

    const ECRelationshipEnd foreignEnd = GetForeignEnd ();
    const ECRelationshipEnd primaryEnd = foreignEnd == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    ECRelationshipConstraintCR primaryEndConstraint = foreignEnd == ECRelationshipEnd_Source ? targetConstraint : sourceConstraint;
    ECRelationshipConstraintCR foreignEndConstraint = foreignEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;

    ECEntityClass const* foreignEndClass = foreignEndConstraint.GetClasses()[0];
    ClassMap const* foreignEndClassMap = GetECDbMap ().GetClassMap (*foreignEndClass);
    ECDbSqlTable* foreignEndTable = const_cast<ECDbSqlTable*>(GetECDbMap().GetFirstTableFromRelationshipEnd(foreignEndConstraint));

    ECEntityClass const* primaryEndClass = primaryEndConstraint.GetClasses ()[0];
    size_t primaryEndTableCount = GetECDbMap().GetTableCountOnRelationshipEnd(primaryEndConstraint);
    ECDbSqlTable const* primaryEndTable = GetECDbMap().GetFirstTableFromRelationshipEnd(primaryEndConstraint);
    BeAssert(primaryEndTable != nullptr);

    //SetTable for EndTable case.
    if (foreignEndClassMap->HasJoinedTable())
        {
        std::set<ECDbSqlColumn const*> thisKeyPropCols, otherKeyPropCols;

        if (SUCCESS != TryGetKeyPropertyColumn(thisKeyPropCols, foreignEndConstraint, *relationshipClassMapInfo.GetECClass().GetRelationshipClassCP(), foreignEnd))
            return MapStatus::Error;

        if (SUCCESS != TryGetKeyPropertyColumn(otherKeyPropCols, primaryEndConstraint, *relationshipClassMapInfo.GetECClass().GetRelationshipClassCP(), primaryEnd))
            return MapStatus::Error;

        if (!otherKeyPropCols.empty() )
            {
            if ((*otherKeyPropCols.begin())->GetKind() != ColumnKind::ECInstanceId)
                {
                GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(
                    ECDbIssueSeverity::Error,
                    "KeyProperty specified but not used by ECDb. It should be either set to ECInstanceId or removed. %s Constraint %s",
                    relationshipClassMapInfo.GetECClass().GetFullName(), 
                    primaryEnd == ECRelationshipEnd_Source ? "Source": "Target"
                    );

                return MapStatus::Error;
                }
            }
        /*
        How to set Table for LinkTable in joined Table case?
        1. Persiste relationship in table which contain class.
        2. If relationship is on baseClass then it should baseClass where it should be stored.
        3. If relationship is on childClass then it should be childClass where it should be stored.
        4. For keyProperty it should be stored in table that has key property.
        */
        if (thisKeyPropCols.empty())
            {
            SetTable(*foreignEndTable);
            }
        else
            {
            //KeypropCol is either base or child table of joined table case.
            //Check to make sure its once of the table.
            BeAssert(foreignEndClassMap->IsMappedTo((*thisKeyPropCols.begin())->GetTable()));
            SetTable(const_cast<ECDbSqlColumn*>((*thisKeyPropCols.begin()))->GetTableR());
            }
        }
    else  //Normal case.
        {
        SetTable(*foreignEndTable);
        }

    //Create ECinstanceId for this classMap. This must map to current table for this class evaluate above and set through SetTable();
    PropertyMapPtr ecInstanceIdPropMap = PropertyMapECInstanceId::Create(GetECDbMap().GetECDbR().Schemas(), *this);
    if (ecInstanceIdPropMap == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMapECInstanceId");
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropMap);

    //if no class id column on this end is required, store the class id directly so that it can be used as literal in the native SQL
    const ECClassId defaultForeignEndECClassId = foreignEndClass->GetId ();

    //**** Other End
    ECDbSqlColumn* foreignKeyClassIdColumn = ConfigureForeignECClassIdKey (relationshipClassMapInfo, primaryEndConstraint, *primaryEndTable, primaryEndTableCount);
    if (foreignKeyClassIdColumn == nullptr)
        {
        BeAssert (false && "Failed to create foreign ECClassId column for relationship");
        return MapStatus::Error;
        }

    ECDbSqlColumn* foreignKeyIdColumn = nullptr;
    auto stat = CreateConstraintColumns(foreignKeyIdColumn, relationshipClassMapInfo, foreignEnd, foreignEndConstraint);
    if (stat != MapStatus::Success)
        return stat;

    //**** Prop Maps

    stat = CreateConstraintPropMaps (foreignEnd, defaultForeignEndECClassId, foreignKeyIdColumn, foreignKeyClassIdColumn, primaryEndClass->GetId());
    if (stat != MapStatus::Success)
        return stat;

    if (GetDataIntegrityEnforcementMethod() == DataIntegrityEnforcementMethod::ForeignKey)
        {
        auto const& primaryEndConstraint = foreignEnd != ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
        auto const& primaryEndConstraintMap = foreignEnd != ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
        auto foreignColumnName = foreignEnd != ECRelationshipEnd_Source ? GetSourceECInstanceIdPropMap()->GetFirstColumn()->GetName().c_str() : GetTargetECInstanceIdPropMap()->GetFirstColumn()->GetName().c_str();

        const std::set<ECDbSqlTable const*> foreignTables = GetECDbMap().GetTablesFromRelationshipEndWithColumn(foreignEndConstraint, foreignColumnName);
        if (GetECDbMap().GetTableCountOnRelationshipEnd(primaryEndConstraint) != 1)
            {
            GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(
                ECDbIssueSeverity::Error,
                "Relationship %s is evaluated to more then one table for primary (%s) side. ECDb expect only one table on each side.",
                relationshipClassMapInfo.GetECClass().GetFullName(),
                primaryEnd == ECRelationshipEnd_Source ? "Source" : "Target"
                );

            BeAssert(false);
            return MapStatus::Error;
            }

        auto primaryClassMap = GetECDbMap().GetClassMap(*primaryEndConstraintMap.GetRelationshipConstraint().GetClasses()[0]);
        BeAssert(primaryClassMap != nullptr && "Primary Class map is null");
        auto primaryKeyColumn = primaryClassMap->GetPrimaryTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        auto& primaryTable = primaryKeyColumn->GetTable();

        auto userRequestedDeleteAction = relationshipClassMapInfo.CreateForeignKeyConstraint()? relationshipClassMapInfo.GetOnDeleteAction() : ForeignKeyActionType::NotSpecified;
        if (primaryTable.GetPersistenceType() == PersistenceType::Persisted)
            {
            BeAssert(primaryKeyColumn != nullptr);

            for (ECDbSqlTable const* foreignTable : foreignTables)
                {
                if (foreignTable->GetPersistenceType() == PersistenceType::Virtual)
                    continue;

                auto foreignKeyColumn = foreignTable->FindColumnCP(foreignColumnName);
                BeAssert(foreignKeyColumn != nullptr);

                if (foreignKeyColumn == nullptr || primaryKeyColumn == nullptr)
                    return MapStatus::Error;

                //Create foreign key constraint
                auto foreignKey = const_cast<ECDbSqlTable*>(foreignTable)->CreateForeignKeyConstraint(primaryTable);
                foreignKey->Add(foreignKeyColumn->GetName().c_str(), primaryKeyColumn->GetName().c_str());
                if (userRequestedDeleteAction!= ForeignKeyActionType::NotSpecified)
                    foreignKey->SetOnDeleteAction(userRequestedDeleteAction);
                else
                    {
                    if (GetRelationshipClass().GetStrength() == StrengthType::Embedding)
                        foreignKey->SetOnDeleteAction(ForeignKeyActionType::Cascade);
                    else
                        foreignKey->SetOnDeleteAction(ForeignKeyActionType::SetNull);
                    }
                foreignKey->RemoveIfDuplicate();
                }
            }
        }
    return stat;
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
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();
    auto foreignEnd = GetForeignEnd ();
    auto const& foreignEndConstraint = foreignEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
    auto const& primaryEndConstraint = foreignEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;

    ECClassId defaultSourceECClassId, defaultTargetECClassId;

    if (foreignEnd == ECRelationshipEnd_Source)
        {
        defaultSourceECClassId = foreignEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : foreignEndConstraint.GetClasses().front()->GetId();
        defaultTargetECClassId = primaryEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : primaryEndConstraint.GetClasses().front()->GetId();
        }
    else
        {
        defaultTargetECClassId = foreignEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : foreignEndConstraint.GetClasses().front()->GetId();
        defaultSourceECClassId = primaryEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : primaryEndConstraint.GetClasses().front()->GetId();
        }

    auto sourceECInstanceIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECInstanceId);
    auto pm = mapInfo.FindPropertyMap (sourceECInstanceIdProperty->GetId (), ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), DEFAULT_SOURCEECINSTANCEID_COLUMNNAME);
    PRECONDITION (sourceECInstanceIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());


    auto sourceECClassIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECClassId);
    pm = mapInfo.FindPropertyMap (sourceECClassIdProperty->GetId (), ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }


    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultSourceECClassId, *this, DEFAULT_SOURCEECCLASSID_COLUMNNAME);
    PRECONDITION (sourceECClassIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());

    auto targetECInstanceIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::TargetECInstanceId);
    pm = mapInfo.FindPropertyMap (targetECInstanceIdProperty->GetId (), ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), DEFAULT_TARGETECINSTANCEID_COLUMNNAME);
    PRECONDITION (targetECInstanceIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());

    auto targetECClassIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::TargetECClassId);
    pm = mapInfo.FindPropertyMap (targetECClassIdProperty->GetId (), ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultTargetECClassId, *this, DEFAULT_TARGETECCLASSID_COLUMNNAME);
    PRECONDITION (targetECClassIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintColumns(ECDbSqlColumn*& fkIdColumn, RelationshipMapInfo const& mapInfo, ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint)
    {
    fkIdColumn = nullptr;

    ECRelationshipClassCR relClass = *mapInfo.GetECClass().GetRelationshipClassCP();
    std::set<ECDbSqlColumn const*> keyPropertyColumns;
    if (SUCCESS != TryGetKeyPropertyColumn(keyPropertyColumns, constraint, relClass, constraintEnd))
        return MapStatus::Error;

    const ColumnKind fkColumnId = GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnKind::TargetECInstanceId : ColumnKind::SourceECInstanceId;

    m_autogenerateForeignKeyColumns = false;

    if (!keyPropertyColumns.empty())
        fkIdColumn = const_cast<ECDbSqlColumn*>((*keyPropertyColumns.begin()));
    else
        {
        //First look at CA for an ECInstanceIdColumnName. 
        //If not specified, look whether there is a single NavigationProperty and take its name
        //If this doesn't exist, generate a default name

        RelationshipEndColumns const& constraintColumnMapping = GetEndColumnsMapping(mapInfo);
        Utf8String idColName (constraintColumnMapping.GetECInstanceIdColumnName());
        if (idColName.empty())
            {
            if (SUCCESS != TryGetConstraintIdColumnNameFromNavigationProperty(idColName, constraint, relClass, constraintEnd))
                return MapStatus::Error;
            }

        if (!idColName.empty())
            fkIdColumn = GetPrimaryTable().FindColumnP(idColName.c_str());

        if (fkIdColumn == nullptr)
            {
            if (idColName.empty())
                {
                if (!GetPrimaryEndKeyColumnName(idColName, GetPrimaryTable(), true))
                    return MapStatus::Error;
                }

            fkIdColumn = CreateConstraintColumn(idColName.c_str(), fkColumnId, PersistenceType::Persisted);
            m_autogenerateForeignKeyColumns = true;
            }
        }

    BeAssert(fkIdColumn != nullptr);
    if (!m_autogenerateForeignKeyColumns)
        {
        bool canEdit = fkIdColumn->GetTableR().GetEditHandle().CanEdit();
        if (!canEdit)
            fkIdColumn->GetTableR().GetEditHandleR().BeginEdit();

        if (!Enum::Intersects(fkIdColumn->GetKind(), fkColumnId))
            fkIdColumn->AddKind(fkColumnId);

        if (!canEdit)
            fkIdColumn->GetTableR().GetEditHandleR().EndEdit();
        }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintPropMaps 
(
ECRelationshipEnd foreignEnd,
ECClassId defaultForeignEndClassId,
ECDbSqlColumn* const& primaryEndECInstanceIdColumn, 
ECDbSqlColumn* const& primaryEndECClassIdColumn,
ECClassId defaultPrimaryEndClassId
)
    {
    //Now add prop maps for source/target ecinstance id and ecclass id prop maps
    //Existing this end instance id and class id columns will be reused
    auto& persistenceEndTable = GetPrimaryTable();
    std::vector<ECDbSqlColumn const*> systemColumns;
    if (persistenceEndTable.GetFilteredColumnList (systemColumns, ColumnKind::ECInstanceId) == BentleyStatus::ERROR)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return MapStatus::Error;
        }

    auto foreignEndECInstanceIdColumn = const_cast<ECDbSqlColumn*>(systemColumns.front ());
    auto foreignEndECClassIdColumn = const_cast<ECDbSqlColumn*>(GetPrimaryTable().GetFilteredColumnFirst(ColumnKind::ECClassId));

    ECDbSqlColumn* sourceECInstanceIdColumn = nullptr;
    Utf8CP sourceECInstanceIdViewColumnAlias = DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
    ECDbSqlColumn* sourceECClassIdColumn = nullptr;
    Utf8CP sourceECClassIdViewColumnAlias = DEFAULT_SOURCEECCLASSID_COLUMNNAME;
    ECClassId defaultSourceECClassId = ECClass::UNSET_ECCLASSID;
    ECDbSqlColumn* targetECInstanceIdColumn = nullptr;
    Utf8CP targetECInstanceIdViewColumnAlias = DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
    ECDbSqlColumn* targetECClassIdColumn = nullptr;
    Utf8CP targetECClassIdViewColumnAlias = DEFAULT_TARGETECCLASSID_COLUMNNAME;
    ECClassId defaultTargetECClassId = ECClass::UNSET_ECCLASSID;

    //class id columns in end table are delay generated. Respective property map needs to know that so that
    //it can register for a hook that notifies the prop map when the class id column was generated.
    //This only affects the class id prop map pointing to this end's class id column. The other end class id column
    //is not affected.

    bool sourceECClassIdColIsDelayGenerated = false;
    bool targetECClassIdColIsDelayGenerated = false;
    if (foreignEnd == ECRelationshipEnd_Source)
        {
        sourceECInstanceIdColumn = foreignEndECInstanceIdColumn;
        if (foreignEndECClassIdColumn != nullptr)
            sourceECClassIdColumn = foreignEndECClassIdColumn;
        else
            {
            sourceECClassIdColumn = CreateConstraintColumn (sourceECClassIdViewColumnAlias, ColumnKind::SourceECClassId, PersistenceType::Virtual);
            sourceECClassIdColIsDelayGenerated = true;
            defaultSourceECClassId = defaultForeignEndClassId;
            }

        targetECInstanceIdColumn = primaryEndECInstanceIdColumn;
        targetECClassIdColumn = primaryEndECClassIdColumn;
        defaultTargetECClassId = defaultPrimaryEndClassId;
        }
    else
        {
        sourceECInstanceIdColumn = primaryEndECInstanceIdColumn;
        sourceECClassIdColumn = primaryEndECClassIdColumn;
        defaultSourceECClassId = defaultPrimaryEndClassId;

        targetECInstanceIdColumn = foreignEndECInstanceIdColumn;
        if (foreignEndECClassIdColumn != nullptr)
            targetECClassIdColumn = foreignEndECClassIdColumn;
        else
            {
            targetECClassIdColumn = CreateConstraintColumn (targetECClassIdViewColumnAlias, ColumnKind::TargetECClassId, PersistenceType::Virtual);
            targetECClassIdColIsDelayGenerated = true;
            defaultTargetECClassId = defaultForeignEndClassId;
            }
        }

    BeAssert (sourceECInstanceIdColumn != nullptr);
    BeAssert (sourceECClassIdColumn != nullptr);
    BeAssert (targetECInstanceIdColumn != nullptr);
    BeAssert (targetECClassIdColumn != nullptr);

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas(), sourceECInstanceIdColumn, sourceECInstanceIdViewColumnAlias);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), sourceECClassIdColumn, defaultSourceECClassId, *this, sourceECClassIdViewColumnAlias, sourceECClassIdColIsDelayGenerated);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());
 
    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), targetECInstanceIdColumn, targetECInstanceIdViewColumnAlias);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), targetECClassIdColumn, defaultTargetECClassId, *this, targetECClassIdViewColumnAlias, targetECClassIdColIsDelayGenerated);
    PRECONDITION(targetECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return MapStatus::Success;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd(SchemaImportContext& schemaImportContext, ClassMapInfo const& mapInfo)
    {
    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&mapInfo) != nullptr);
    RelationshipMapInfo const& relMapInfo = static_cast<RelationshipMapInfo const&> (mapInfo);
    const bool isUniqueIndex = relMapInfo.GetCardinality() == RelationshipMapInfo::Cardinality::OneToOne;
    ECDbSqlTable& persistenceEndTable = GetPrimaryTable();

    if (!relMapInfo.CreateIndexOnForeignKey() || persistenceEndTable.GetTableType() == TableType::Existing || 
        (!isUniqueIndex && !m_autogenerateForeignKeyColumns))
        return;

    BeAssert(GetPrimaryEndECInstanceIdPropMap() != nullptr && GetPrimaryEndECInstanceIdPropMap()->GetFirstColumn() != nullptr);
    ECDbSqlColumn const* primaryEndIdColumn = GetPrimaryEndECInstanceIdPropMap()->GetFirstColumn();

    // name of the index
    Utf8String name(isUniqueIndex ? "uix_" : "ix_");
    name.append(persistenceEndTable.GetName()).append ("_fk_").append(m_ecClass.GetSchema().GetNamespacePrefix() + "_" + m_ecClass.GetName());
    if (GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable)
        name.append("_source");
    else
        name.append("_target");
    
    NativeSqlBuilder whereClause;
    if (!primaryEndIdColumn->GetConstraint().IsNotNull())
        {
        whereClause.AppendEscaped(primaryEndIdColumn->GetName().c_str()).AppendSpace();
        whereClause.Append(BooleanSqlOperator::IsNot, true).Append("NULL");
        }

    schemaImportContext.GetECDbMapDb().CreateIndex(GetECDbMap().GetECDbR(), persistenceEndTable, name.c_str(), isUniqueIndex, {primaryEndIdColumn}, whereClause.ToString(), true, GetClass().GetId());
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
bool RelationshipClassEndTableMap::GetPrimaryEndKeyColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "ForeignECInstanceId_", mappingInProgress);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetPrimaryEndECClassIdColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "ForeignECClassId_", mappingInProgress);
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
PropertyMapRelationshipConstraintClassId const* RelationshipClassEndTableMap::GetForeignEndECClassIdPropMap () const
    {
    return GetConstraintMap (GetForeignEnd ()).GetECClassIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetPrimaryEndECInstanceIdPropMap () const
    {
    return GetConstraintMap (GetPrimaryEnd ()).GetECInstanceIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapRelationshipConstraintClassId const* RelationshipClassEndTableMap::GetPrimaryEndECClassIdPropMap () const
    {
    return GetConstraintMap (GetPrimaryEnd ()).GetECClassIdPropMap ();
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
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetPrimaryEnd () const
    {
    return GetForeignEnd () == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2015
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipEndColumns const& RelationshipClassEndTableMap::GetEndColumnsMapping(RelationshipMapInfo const& info) const
    {
    BeAssert(GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? info.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable : info.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);
    return RelationshipClassMap::GetEndColumnsMapping(info, GetForeignEnd());
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
        if (constraintMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

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

    if (GetRelationshipClass().GetStrength() == StrengthType::Referencing)
        {
        if (nSourceTables == 1 && nTargetTables == 1)
            {
            return DataIntegrityEnforcementMethod::ForeignKey;
            }
        }

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

    if (GetDataIntegrityEnforcementMethod() == DataIntegrityEnforcementMethod::ForeignKey)
        {
        if (GetRelationshipClass().GetStrength() == StrengthType::Embedding)
            {
            GetECDbMap().GetECDbR().GetECDbImplR().GetIssueReporter().Report(
                ECDbIssueSeverity::Error,
                "Relationship %s is of type embedding and desgniated to be mapped as LinkTable which is not supported in ECDb",
                GetRelationshipClass().GetFullName()
                );

            BeAssert(false);
            return MapStatus::Error;
            }

        std::set<ECDbSqlTable const*> sourceTables = GetECDbMap().GetTablesFromRelationshipEnd(GetRelationshipClass().GetSource());
        std::set<ECDbSqlTable const*> targetTables = GetECDbMap().GetTablesFromRelationshipEnd(GetRelationshipClass().GetTarget());
        
        //Create FK from Source-Primary to LinkTable
        ECDbSqlTable * sourceTable = const_cast<ECDbSqlTable*>(*sourceTables.begin());
        ECDbSqlForeignKeyConstraint* sourceFK = GetPrimaryTable().CreateForeignKeyConstraint(*sourceTable);
        ECDbSqlColumn const* souceColumn = sourceTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        sourceFK->Add(GetSourceECInstanceIdPropMap()->GetFirstColumn()->GetName().c_str(), souceColumn->GetName().c_str());
        sourceFK->SetOnDeleteAction(ForeignKeyActionType::Cascade);
        sourceFK->RemoveIfDuplicate();
        sourceFK = nullptr;


        //Create FK from Target-Primary to LinkTable
        ECDbSqlTable * targetTable = const_cast<ECDbSqlTable*>(*targetTables.begin());
        ECDbSqlForeignKeyConstraint* targetFK = GetPrimaryTable().CreateForeignKeyConstraint(*targetTable);
        ECDbSqlColumn const* targetColumn = targetTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        targetFK->Add(GetTargetECInstanceIdPropMap()->GetFirstColumn()->GetName().c_str(), targetColumn->GetName().c_str());
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
    Utf8String columnName (mapInfo.GetSourceColumnsMapping().GetECInstanceIdColumnName ());
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Source, GetPrimaryTable()))
            return MapStatus::Error;
        }

    auto sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (),ColumnKind::SourceECInstanceId, PersistenceType::Persisted);
    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), sourceECInstanceIdColumn);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    sourceECInstanceIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    //**** SourceECClassId prop map
    auto sourceECClassIdColumn = ConfigureForeignECClassIdKey (mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdColumnAlias = sourceECClassIdColumn->GetName ().EqualsI (DEFAULT_SOURCEECCLASSID_COLUMNNAME) == true ? nullptr : DEFAULT_SOURCEECCLASSID_COLUMNNAME;
    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), sourceECClassIdColumn, defaultSourceECClassId, *this, sourceECClassIdColumnAlias);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    sourceECClassIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());


    //**** TargetECInstanceId prop map 
    columnName = mapInfo.GetTargetColumnsMapping().GetECInstanceIdColumnName();
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Target, GetPrimaryTable()))
            return MapStatus::Error;
        }

    auto targetECInstanceIdColumn = CreateConstraintColumn (columnName.c_str (), ColumnKind::TargetECInstanceId, PersistenceType::Persisted);

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), targetECInstanceIdColumn);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    targetECInstanceIdPropMap->FindOrCreateColumnsInTable(*this, &mapInfo);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());


    //**** TargetECClassId prop map
    auto targetECClassIdColumn = ConfigureForeignECClassIdKey (mapInfo, ECRelationshipEnd_Target);
    auto targetECClassIdColumnAlias = targetECClassIdColumn->GetName ().EqualsI (DEFAULT_TARGETECCLASSID_COLUMNNAME) == true ? nullptr : DEFAULT_TARGETECCLASSID_COLUMNNAME;
    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), targetECClassIdColumn, defaultTargetECClassId, *this, targetECClassIdColumnAlias);
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

    auto sourceECInstanceIdColumn = GetSourceECInstanceIdPropMap()->GetFirstColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->IsMappedToClassMapTables() ? GetSourceECClassIdPropMap()->GetFirstColumn() : nullptr;
    auto targetECInstanceIdColumn = GetTargetECInstanceIdPropMap()->GetFirstColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->IsMappedToClassMapTables() ? GetTargetECClassIdPropMap()->GetFirstColumn() : nullptr;

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
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? DEFAULT_SOURCEECINSTANCEID_COLUMNNAME : DEFAULT_TARGETECINSTANCEID_COLUMNNAME;

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
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? DEFAULT_SOURCEECCLASSID_COLUMNNAME : DEFAULT_TARGETECCLASSID_COLUMNNAME;

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

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), DEFAULT_SOURCEECINSTANCEID_COLUMNNAME);
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


    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultSourceECClassId, *this, DEFAULT_SOURCEECCLASSID_COLUMNNAME);
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

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), DEFAULT_TARGETECINSTANCEID_COLUMNNAME);
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

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultTargetECClassId, *this, DEFAULT_TARGETECCLASSID_COLUMNNAME);
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
