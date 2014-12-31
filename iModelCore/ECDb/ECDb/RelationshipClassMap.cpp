/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
Utf8CP const RelationshipClassMap::DEFAULT_SOURCEECINSTANCEID_COLUMNNAME = "RK_Source";
//static
Utf8CP const RelationshipClassMap::DEFAULT_SOURCEECCLASSID_COLUMNNAME = "RC_Source";
//static
Utf8CP const RelationshipClassMap::DEFAULT_TARGETECINSTANCEID_COLUMNNAME = "RK_Target";
//static
Utf8CP const RelationshipClassMap::DEFAULT_TARGETECCLASSID_COLUMNNAME = "RC_Target";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassMap::RelationshipClassMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty)
    : ClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty), 
    m_sourceConstraintMap (ecDbMap.GetECDbR ().GetSchemaManager (), ecRelClass.GetSource ()),
    m_targetConstraintMap (ecDbMap.GetECDbR ().GetSchemaManager (), ecRelClass.GetTarget ())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
DbColumnPtr RelationshipClassMap::CreateConstraintColumn (Utf8CP columnName, bool addToTable)
    {
    DbColumnPtr column = nullptr;
    if (GetMapStrategy() == MapStrategy::SharedTableForThisClass && (column = GetTable ().GetColumns ().GetPtr (columnName)) != nullptr)
        return column;

    column = DbColumn::Create(columnName, PRIMITIVETYPE_DbKey, true, false);
    if (addToTable)
        {
        auto& dbTable = GetTable();
        if (!dbTable.GetColumnsR().Contains(column->GetName()))
            dbTable.GetColumnsR().Add (column);
        }
    else
        column->MakeVirtual ();

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
    //TODO: Need to check other multi-table strategies too?
    addConstraintClassIdColumnNeeded = GetMapStrategy () == MapStrategy::SharedTableForThisClass || 
                                constraintClasses.size () > 1 || ConstraintIncludesAnyClass (constraintClasses);
    //if constraint is polymorphic, and if addConstraintClassIdColumnNeeded is not true yet,
    //we also need to check if the constraint classes have subclasses. If there is at least one, addConstraintClassIdColumnNeeded
    //is set to true;
    if (!addConstraintClassIdColumnNeeded && constraint.GetIsPolymorphic ())
        {
        auto const& schemaManager = GetSchemaManager ();
        for (auto constraintClass : constraintClasses)
            {
            auto const& derivedClasses = schemaManager.GetDerivedECClasses (*constraintClass);
            if (derivedClasses.size () > 0)
                {
                addConstraintClassIdColumnNeeded = true;
                break;
                }
            }
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
        defaultConstraintClassId = UNSET_ECCLASSID;
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
RelationshipClassMap::ConstraintMap const& RelationshipClassMap::GetConstraintMap (ECN::ECRelationshipEnd constraintEnd) const
    {
    return constraintEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
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
PropertyMapCP RelationshipClassMap::GetConstraintECClassIdPropMap (ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECClassIdPropMap();
    else
        return GetTargetECClassIdPropMap();
    }


//************************ RelationshipClassMap::ConstraintMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
bool RelationshipClassMap::ConstraintMap::ClassIdMatchesConstraint (ECN::ECClassId candidateClassId) const
    {
    CacheClassIds ();

    if (m_anyClassMatches)
        return true;

    return m_ecClassIdCache.find (candidateClassId) != m_ecClassIdCache.end ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
bool RelationshipClassMap::ConstraintMap::TryGetSingleClassIdFromConstraint (ECClassId& constraintClassId) const
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
void RelationshipClassMap::ConstraintMap::CacheClassIds () const
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
void RelationshipClassMap::ConstraintMap::CacheClassIds (bvector<ECN::ECClassP> const& constraintClassList, bool constraintIsPolymorphic) const
    {
    //runs through all constraint classes, and if recursive is true through subclasses
    //and caches the class ids as this method here is expensive performance-wise.
    for (auto constraintClass : constraintClassList)
        {
        if (IsAnyClass (*constraintClass))
            {
            SetAnyClassMatches ();
            return;
            }

        const ECClassId classId = constraintClass->GetId ();
        CacheClassId (classId);

        if (constraintIsPolymorphic)
            {
            //call into schema manager to ensure that the derived classes are loaded if needed
            auto const& derivedClasses = m_schemaManager.GetDerivedECClasses (*constraintClass);
            CacheClassIds (derivedClasses, constraintIsPolymorphic);
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipClassMap::ConstraintMap::CacheClassId (ECN::ECClassId classId) const
    {
    m_ecClassIdCache.insert (classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2014
//---------------------------------------------------------------------------------------
void RelationshipClassMap::ConstraintMap::SetAnyClassMatches () const
    {
    m_anyClassMatches = true;
    //class id cache not needed if any class matches
    m_ecClassIdCache.clear ();
    }


//************************ RelationshipClassEndTableMap::NativeSqlConverterImpl ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
RelationshipClassEndTableMap::NativeSqlConverterImpl::NativeSqlConverterImpl (RelationshipClassEndTableMapCR classMap)
: ClassMap::NativeSqlConverterImpl (classMap)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus RelationshipClassEndTableMap::NativeSqlConverterImpl::_GetWhereClause 
(
NativeSqlBuilder& whereClauseBuilder, 
ECSqlType ecsqlType, 
bool isPolymorphicClassExp, 
Utf8CP tableAlias
) const
    {
    //Don't call base class method, as for end table mappings the table never has a column for the relationship's class id
    //and the base class method would take the table's class id column and filter it by the relationship's class id which will not
    //work as the table class id contains this end's constraint class id and not the relationship's class id.
    if (!whereClauseBuilder.IsEmpty ())
        whereClauseBuilder.Append (" AND ");

    //Existence of an end table relationships is told by the fact whether the other end ecinstance id column is set or not.
    //The following where expression tests for that.
    BeAssert (dynamic_cast<RelationshipClassEndTableMapCP> (&GetClassMap ()) != nullptr);
    auto const& relClassMap = static_cast<RelationshipClassEndTableMapCR> (GetClassMap ());
    auto otherEndECInstanceIdColSqlSnippets = relClassMap.GetOtherEndECInstanceIdPropMap ()->ToNativeSql (tableAlias, ecsqlType);
    BeAssert (!otherEndECInstanceIdColSqlSnippets.empty ());
    whereClauseBuilder.AppendParenLeft ();
    bool isFirstItem = true;
    for (auto const& sqlSnippet : otherEndECInstanceIdColSqlSnippets)
        {
        if (!isFirstItem)
            whereClauseBuilder.Append (" AND ");

        whereClauseBuilder.Append (sqlSnippet).Append (" IS NOT NULL");

        isFirstItem = false;
        }

    whereClauseBuilder.AppendParenRight ();

    return ECSqlStatus::Success;
    }


//************************ RelationshipClassEndTableMap **********************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassEndTableMap::RelationshipClassEndTableMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty)
: RelationshipClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_InitializePart1 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    //Don't call base class method as end table map requires its own handling
    BeAssert (GetMapStrategy () == MapStrategy::RelationshipSourceTable || GetMapStrategy () == MapStrategy::RelationshipTargetTable);
    m_dbView = CreateClassDbView ();

    auto const& relationshipClassMapInfo = dynamic_cast<RelationshipClassMapInfoCR> (classMapInfo);
    BeAssert (m_ecClass.GetRelationshipClassCP () != nullptr &&
        (classMapInfo.GetMapStrategy () == MapStrategy::RelationshipSourceTable || classMapInfo.GetMapStrategy () == MapStrategy::RelationshipTargetTable));

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

    auto thisEnd = GetThisEnd ();

    //**** This End
    auto const& thisEndConstraint = thisEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
    auto firstThisEndClass = thisEndConstraint.GetClasses()[0];
    auto firstThisEndClassMap = GetECDbMap ().GetClassMapCP (*firstThisEndClass, true);

    //*** persistence end table
    SetTable (&firstThisEndClassMap->GetTable ());

    //if no class id column on this end is required, store the class id directly so that it can be used as literal in the native SQL
    const ECClassId defaultThisEndECClassId = firstThisEndClass->GetId ();

    //**** Other End
    auto const& otherEndConstraint = thisEnd == ECRelationshipEnd_Source ? targetConstraint : sourceConstraint;

    bool addOtherEndECClassIdColumnToTable = false;
    ECClassId defaultOtherEndECClassId = UNSET_ECCLASSID;
    DetermineConstraintClassIdColumnHandling (addOtherEndECClassIdColumnToTable, defaultOtherEndECClassId, otherEndConstraint);

    //if the class id column is not needed, we still create one, but mark it virtual
    //other end requires a class id if the constraint consists of more than one classes or of AnyClass 
    DbColumnPtr otherEndECInstanceIdColumn = nullptr;
    DbColumnPtr otherEndECClassIdColumn = nullptr;
    auto stat = CreateConstraintColumns (otherEndECInstanceIdColumn, otherEndECClassIdColumn, relationshipClassMapInfo, thisEnd, addOtherEndECClassIdColumnToTable);
    if (stat != MapStatus::Success)
        return stat;

    //**** Prop Maps
    //Reuse ECInstanceId prop map from persistence end class map
    PropertyMapPtr ecInstanceIdPropMap = nullptr;
    if (!firstThisEndClassMap->TryGetECInstanceIdPropertyMap (ecInstanceIdPropMap))
        {
        BeAssert (false);
        return MapStatus::Error;
        }
    GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropMap);
    stat = CreateConstraintPropMaps (thisEnd, defaultThisEndECClassId, otherEndECInstanceIdColumn, otherEndECClassIdColumn, defaultOtherEndECClassId);
    if (stat != MapStatus::Success)
        return stat;

    //! Add referencial integerity if user requested it.
    auto const& thisEndConstraintInfo = thisEnd == ECRelationshipEnd_Source ? relationshipClassMapInfo.GetSourceInfo() : relationshipClassMapInfo.GetTargetInfo ();
    if (thisEndConstraintInfo.DoEnforceIntegrityCheck () && relationshipClass.GetStrength () != StrengthType::STRENGTHTYPE_Holding && !thisEndConstraintInfo.IsEmpty())
        {
        //auto const& thisEndConstraint = thisEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
        auto const& thisEndConstraintMap = thisEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
        auto const& otherEndConstraint = thisEnd != ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
        auto const& otherEndConstraintMap = thisEnd != ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;

        bset<DbTableP> otherEndTables;
        GetECDbMap ().GetTablesFromRelationshipEnd (&otherEndTables, otherEndConstraint);

        BeAssert (otherEndTables.size () == 1 && "In EndTableMap we expect one table on each side");
        if (otherEndTables.size () != 1)
            return MapStatus::Error;

        auto forignKeyColumn = otherEndConstraintMap.GetECInstanceIdPropMap ()->GetFirstColumn ();
        auto& forignTable = GetTable ();
        auto primaryKeyColumn = thisEndConstraintMap.GetECInstanceIdPropMap ()->GetFirstColumn ();
        auto& primaryTable = **(otherEndTables.begin());
        
        BeAssert (forignKeyColumn != nullptr);
        BeAssert (primaryKeyColumn != nullptr);
        if (forignKeyColumn == nullptr || primaryKeyColumn == nullptr)
            return MapStatus::Error;
            
        //Create forign key constraint
        auto forignKey = DbForignKeyConstraint::Create ();
        forignKey->AddColumn (*forignKeyColumn);
        forignKey->SetTargetTable (primaryTable);
        forignKey->AddTargetColumn (*primaryKeyColumn);
        forignKey->SetOnDelete (thisEndConstraintInfo.GetOnDeleteAction());
        forignKey->SetOnUpdate (thisEndConstraintInfo.GetOnUpdateAction());
        forignKey->SetMatchType(thisEndConstraintInfo.GetMatchType ());
        forignTable.AddConstraint (*forignKey);
        }
    
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_InitializePart2 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    //add non-system property maps
    AddPropertyMaps (classMapInfo, parentClassMap);

    //**** Add indices
    AddIndices (classMapInfo);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintColumns (DbColumnPtr& otherEndECInstanceIdColumn, DbColumnPtr& otherEndECClassIdColumn,
                                                                 RelationshipClassMapInfoCR mapInfo, ECRelationshipEnd thisEnd, bool addOtherEndClassIdColumnToTable)
    {
    //** Other End ECInstanceId column
    Utf8String columnName = thisEnd == ECRelationshipEnd_Source ? 
                  mapInfo.GetTargetECInstanceIdColumn() : mapInfo.GetSourceECInstanceIdColumn();

    if(columnName.empty())
        if (!GetOtherEndKeyColumnName (columnName, GetTable (), true))
            return MapStatus::Error;

    otherEndECInstanceIdColumn = CreateConstraintColumn (columnName.c_str(), true);
    BeAssert (otherEndECInstanceIdColumn.IsValid());

    //** Other End ECClassId column
    columnName.clear ();
    Utf8String defaultECClassIdColumnName;
    GetOtherEndECClassIdColumnName (defaultECClassIdColumnName, GetTable (), false);
       
    columnName = thisEnd == ECRelationshipEnd_Source ?
        mapInfo.GetTargetECClassIdColumn () : mapInfo.GetSourceECClassIdColumn ();

    if (!columnName.empty())
        {
        //CustomAttribute was used to specify ECClassId for existing class
        addOtherEndClassIdColumnToTable = true;
        }

    if (columnName.empty() && !GetOtherEndECClassIdColumnName (columnName, GetTable (), true))
        return MapStatus::Error;

    otherEndECClassIdColumn = CreateConstraintColumn (columnName.c_str (), addOtherEndClassIdColumnToTable);
    BeAssert (otherEndECClassIdColumn.IsValid());
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintPropMaps 
(
ECRelationshipEnd thisEnd,
ECClassId defaultThisEndClassId,
DbColumnPtr const& otherEndECInstanceIdColumn, 
DbColumnPtr const& otherEndECClassIdColumn,
ECClassId defaultOtherEndClassId
)
    {
    //Now add prop maps for source/target ecinstance id and ecclass id prop maps
    //Existing this end instance id and class id columns will be reused
    auto& persistenceEndTable = GetTable();
    auto const& pkColumns = persistenceEndTable.GetSystemKeys ();
    auto thisEndECInstanceIdColumn = pkColumns.front ();
    auto thisEndECClassIdColumn = GetTable ().GetClassIdColumnPtr ();

    DbColumnPtr sourceECInstanceIdColumn = nullptr;
    Utf8CP sourceECInstanceIdViewColumnAlias = nullptr;
    DbColumnPtr sourceECClassIdColumn = nullptr;
    Utf8CP sourceECClassIdViewColumnAlias = nullptr;
    ECClassId defaultSourceECClassId = UNSET_ECCLASSID;
    DbColumnPtr targetECInstanceIdColumn = nullptr;
    Utf8CP targetECInstanceIdViewColumnAlias = nullptr;
    DbColumnPtr targetECClassIdColumn = nullptr;
    Utf8CP targetECClassIdViewColumnAlias = nullptr;
    ECClassId defaultTargetECClassId = UNSET_ECCLASSID;

    //class id columns in end table are delay generated. Respective property map needs to know that so that
    //it can register for a hook that notifies the prop map when the class id column was generated.
    //This only affects the class id prop map pointing to this end's class id column. The other end class id column
    //is not affected.
    bool sourceECClassIdColIsDelayGenerated = false;
    bool targetECClassIdColIsDelayGenerated = false;
    if (thisEnd == ECRelationshipEnd_Source)
        {
        sourceECInstanceIdViewColumnAlias = DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
        sourceECInstanceIdColumn = thisEndECInstanceIdColumn;

        sourceECClassIdViewColumnAlias = DEFAULT_SOURCEECCLASSID_COLUMNNAME;
        if (thisEndECClassIdColumn != nullptr)
            sourceECClassIdColumn = thisEndECClassIdColumn;
        else
            {
            //the virtual columns will not be added to the DbTable. They will only be held by the respective prop map
            sourceECClassIdColumn = CreateConstraintColumn (sourceECClassIdViewColumnAlias, false); //false: don't add to table -> make virtual
            sourceECClassIdColIsDelayGenerated = true;
            defaultSourceECClassId = defaultThisEndClassId;
            }

        targetECInstanceIdColumn = otherEndECInstanceIdColumn;
        targetECClassIdColumn = otherEndECClassIdColumn;
        defaultTargetECClassId = defaultOtherEndClassId;
        }
    else
        {
        sourceECInstanceIdColumn = otherEndECInstanceIdColumn;
        sourceECClassIdColumn = otherEndECClassIdColumn;
        defaultSourceECClassId = defaultOtherEndClassId;

        targetECInstanceIdViewColumnAlias = DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
        targetECInstanceIdColumn = thisEndECInstanceIdColumn;

        targetECClassIdViewColumnAlias = DEFAULT_TARGETECCLASSID_COLUMNNAME;
        if (thisEndECClassIdColumn != nullptr)
            targetECClassIdColumn = thisEndECClassIdColumn;
        else
            {
            targetECClassIdColumn = CreateConstraintColumn (targetECClassIdViewColumnAlias, false); //false: don't add to table -> make virtual
            targetECClassIdColIsDelayGenerated = true;
            defaultTargetECClassId = defaultThisEndClassId;
            }
        }

    BeAssert (sourceECInstanceIdColumn != nullptr);
    BeAssert (sourceECClassIdColumn != nullptr);
    BeAssert (targetECInstanceIdColumn != nullptr);
    BeAssert (targetECClassIdColumn != nullptr);

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, GetSchemaManager(), sourceECInstanceIdColumn, sourceECInstanceIdViewColumnAlias);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    auto persistenceEndTableP = sourceECClassIdColIsDelayGenerated ? &persistenceEndTable : nullptr;
    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, GetSchemaManager (), sourceECClassIdColumn, defaultSourceECClassId, sourceECClassIdViewColumnAlias, persistenceEndTableP);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());
 
    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, GetSchemaManager (), targetECInstanceIdColumn, targetECInstanceIdViewColumnAlias);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());

    persistenceEndTableP = targetECClassIdColIsDelayGenerated ? &persistenceEndTable : nullptr;
    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, GetSchemaManager (), targetECClassIdColumn, defaultTargetECClassId, targetECClassIdViewColumnAlias, persistenceEndTableP);
    PRECONDITION(targetECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndices (ClassMapInfoCR mapInfo)
    {
    BeAssert (dynamic_cast<RelationshipClassMapInfoCP> (&mapInfo) != nullptr);

    if (mapInfo.GetMapStrategy () == MapStrategy::RelationshipSourceTable)
    if (!static_cast<RelationshipClassMapInfoCR> (mapInfo).GetSourceInfo ().GenerateDefaultIndex ())
        return;

    if (mapInfo.GetMapStrategy () == MapStrategy::RelationshipTargetTable)
    if (!static_cast<RelationshipClassMapInfoCR> (mapInfo).GetTargetInfo ().GenerateDefaultIndex ())
        return;

    bool enforceUniqueness = false;
    // TODO: We need to enforce uniqueness of constraints, but cannot at the moment since we get these 
    // i-models that don't honor these constraints. See comments @ RelationshipClassLinkTableMap::AddIndices()
    RelationshipClassMapInfo::CardinalityType cardinality = static_cast<RelationshipClassMapInfoCR> (mapInfo).GetCardinality ();

    switch(cardinality)
        {
            case RelationshipClassMapInfo::CardinalityType::OneToOne:
                AddIndexToRelationshipEnd (enforceUniqueness);
                break;
            case RelationshipClassMapInfo::CardinalityType::OneToMany:
            case RelationshipClassMapInfo::CardinalityType::ManyToOne:
                AddIndexToRelationshipEnd (false);
                break;
            case RelationshipClassMapInfo::CardinalityType::ManyToMany:
                //not supported in EndTableMap
                break;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndexToRelationshipEnd (bool isUniqueIndex)
    {
    auto& persistenceEndTable = GetTable ();

    // Setup name of the index
    Utf8String name = "idx_ECRel_";
    if (MapStrategy::RelationshipSourceTable == GetMapStrategy ())
        name.append("Source_");
    else
        name.append("Target_");
    if (isUniqueIndex)
        name.append ("Unique_");
    name.append (Utf8String (m_ecClass.GetSchema().GetNamespacePrefix() + L"_" + m_ecClass.GetName()));
    DbIndexPtr index = DbIndex::Create (name.c_str (), persistenceEndTable, isUniqueIndex);
    BeAssert (GetOtherEndECInstanceIdPropMap() != nullptr && GetOtherEndECInstanceIdPropMap()->GetFirstColumn() != nullptr);
    index->AddColumn (*GetOtherEndECInstanceIdPropMap()->GetFirstColumn());

    BeAssert (GetOtherEndECClassIdPropMap() != nullptr);

    auto otherEndECClassIdColumn = GetOtherEndECClassIdPropMap()->GetFirstColumn();
    BeAssert (otherEndECClassIdColumn != nullptr);
    if (!otherEndECClassIdColumn->IsVirtual())
        index->AddColumn (*otherEndECClassIdColumn);

    persistenceEndTable.AddIndex (*index);
    }

   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetRelationshipColumnName (Utf8StringR columnName, DbTableCR table, Utf8CP prefix, bool mappingInProgress) const
    {
    Utf8String classNameUtf8 (m_ecClass.GetName());
    columnName = prefix;
    columnName.append (classNameUtf8);
    
    if (!table.GetColumns().Contains (columnName.c_str()))
        return true;

    //! ECSchema Upgrade creates a second column for ECRelationshipClass because of following.
    //! Although following must have been added due to issue. A longterm solution is to store the
    //! name of column in db. Otherwise we cannot tell if column that exist in db is for this this 
    //! relation of some other relation. The information is currently not stored in database. Its
    //! only create of ECDbHints or produced by default values.
    //! We can qualify all relation with schema name to avoid this but it will be breaking changes
    //! as older db will still construct the name by default without schema name. Thus we need the
    //! the persistence of column name for ECRelationship Keys.
#if 0
    Utf8String schemaNameUtf8 (m_ecClass.GetSchema().GetName());
    columnName = prefix;
    columnName.append (schemaNameUtf8);
    columnName.append ("_");
    columnName.append (classNameUtf8);
    if (!table.GetColumn (columnName.c_str()))
        return true;
#endif
    if (mappingInProgress)
        {
        LOG.errorv ("Table %s already contains column named %s. ECRelationship %s has failed to map.", 
            table.GetName(), columnName.c_str(), Utf8String (m_ecClass.GetFullName()).c_str ());
        return false;
        }
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetOtherEndKeyColumnName (Utf8StringR columnName, DbTableCR table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "RK_", mappingInProgress);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetOtherEndECClassIdColumnName (Utf8StringR columnName, DbTableCR table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "RC_", mappingInProgress);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetThisEndECInstanceIdPropMap () const
    {
    return GetConstraintMap (GetThisEnd ()).GetECInstanceIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetThisEndECClassIdPropMap () const
    {
    return GetConstraintMap (GetThisEnd ()).GetECClassIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetOtherEndECInstanceIdPropMap () const
    {
    return GetConstraintMap (GetOtherEnd ()).GetECInstanceIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMapCP RelationshipClassEndTableMap::GetOtherEndECClassIdPropMap () const
    {
    return GetConstraintMap (GetOtherEnd ()).GetECClassIdPropMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetThisEnd() const
    {
    return GetMapStrategy () == MapStrategy::RelationshipSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetOtherEnd () const
    {
    return GetThisEnd () == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
ClassMap::NativeSqlConverter const& RelationshipClassEndTableMap::_GetNativeSqlConverter () const
    {
    if (m_nativeSqlConverter == nullptr)
        m_nativeSqlConverter = std::unique_ptr<NativeSqlConverter> (new NativeSqlConverterImpl (*this));

    return *m_nativeSqlConverter;
    }


//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty)
: RelationshipClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty) 
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MapStatus RelationshipClassLinkTableMap::_InitializePart1 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    BeAssert (GetMapStrategy () == MapStrategy::TablePerHierarchy ||
        GetMapStrategy () == MapStrategy::TableForThisClass ||
        GetMapStrategy () == MapStrategy::TablePerClass ||
        GetMapStrategy () == MapStrategy::SharedTableForThisClass ||
        GetMapStrategy () == MapStrategy::InParentTable &&
        "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");

    auto stat = RelationshipClassMap::_InitializePart1 (classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    //**** Class View for SELECT statement
    m_dbView = CreateClassDbView ();

    BeAssert (dynamic_cast<RelationshipClassMapInfoCP> (&classMapInfo) != nullptr);
    auto const& relationClassMapInfo = static_cast<RelationshipClassMapInfoCR> (classMapInfo);

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    ECClassId defaultSourceECClassId = UNSET_ECCLASSID;
    DetermineConstraintClassIdColumnHandling (addSourceECClassIdColumnToTable, defaultSourceECClassId, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    ECClassId defaultTargetECClassId = UNSET_ECCLASSID;
    DetermineConstraintClassIdColumnHandling (addTargetECClassIdColumnToTable, defaultTargetECClassId, targetConstraint);
    return CreateConstraintPropMaps (relationClassMapInfo, addSourceECClassIdColumnToTable, defaultSourceECClassId, addTargetECClassIdColumnToTable, defaultTargetECClassId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MapStatus RelationshipClassLinkTableMap::_InitializePart2 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    auto stat = RelationshipClassMap::_InitializePart2 (classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    BeAssert (dynamic_cast<RelationshipClassMapInfoCP> (&classMapInfo) != nullptr);
    auto const& relationClassMapInfo = static_cast<RelationshipClassMapInfoCR> (classMapInfo);

    //**** Add relationship indices
    AddIndices (relationClassMapInfo);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps 
(
RelationshipClassMapInfoCR mapInfo,
bool addSourceECClassIdColumnToTable,
ECClassId defaultSourceECClassId, 
bool addTargetECClassIdColumnToTable, 
ECClassId defaultTargetECClassId
)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = mapInfo.GetSourceECInstanceIdColumn();
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Source, GetTable()))
            return MapStatus::Error;
        }

    auto sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (), true);
    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, GetSchemaManager (), sourceECInstanceIdColumn);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    sourceECInstanceIdPropMap->FindOrCreateColumnsInTable (*this);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    //**** SourceECClassId prop map
    columnName = mapInfo.GetSourceECClassIdColumn ();
    if (columnName.empty ())
        {
        if (!GetConstraintECClassIdColumnName (columnName, ECRelationshipEnd_Source, GetTable ()))
            return MapStatus::Error;
        }

    auto sourceECClassIdColumn = CreateConstraintColumn (columnName.c_str (), addSourceECClassIdColumnToTable);
    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, GetSchemaManager (), sourceECClassIdColumn, defaultSourceECClassId);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    sourceECClassIdPropMap->FindOrCreateColumnsInTable (*this);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());


    //**** TargetECInstanceId prop map 
    columnName = mapInfo.GetTargetECInstanceIdColumn();
    if (columnName.empty ())
        {
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Target, GetTable()))
            return MapStatus::Error;
        }

    auto targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (), true);

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, GetSchemaManager (), targetECInstanceIdColumn);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    targetECInstanceIdPropMap->FindOrCreateColumnsInTable (*this);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());


    //**** TargetECClassId prop map
    columnName = mapInfo.GetTargetECClassIdColumn();
    if (columnName.empty ())
        {
        if (!GetConstraintECClassIdColumnName (columnName, ECRelationshipEnd_Target, GetTable ()))
            return MapStatus::Error;
        }

    auto targetECClassIdColumn = CreateConstraintColumn (columnName.c_str (), addTargetECClassIdColumnToTable);
    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, GetSchemaManager (), targetECClassIdColumn, defaultTargetECClassId);
    if (targetECClassIdPropMap == nullptr)
        { 
        BeAssert (targetECClassIdPropMap != nullptr);
        return MapStatus::Error;
        }

    targetECClassIdPropMap->FindOrCreateColumnsInTable (*this);
    GetPropertyMapsR ().AddPropertyMap(targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndices (ClassMapInfoCR mapInfo)
    {
    BeAssert (dynamic_cast<RelationshipClassMapInfoCP> (&mapInfo) != nullptr);
    auto const& relationshipClassMapInfo = static_cast<RelationshipClassMapInfoCR> (mapInfo);

    RelationshipClassMapInfo::CardinalityType cardinality = relationshipClassMapInfo.GetCardinality ();

    bool enforceUniqueness = false;
    // TODO: We need to enforce uniqueness of constraints, but cannot at the moment since we get these 
    // i-models that don't honor these constraints. For example, ConstructSim has a ComponentHasComponent
    // 1-M relationship, but ends up having many instances on the source instead of one. There are
    // SystemComponent-s that include GroupingComponent-s and PartComponent-s, where the GroupingComponent-s
    // themselves include the SystemComponent-s again. 

    bool createOnSource = relationshipClassMapInfo.GetSourceInfo ().GenerateDefaultIndex ();
    bool createOnTarget = relationshipClassMapInfo.GetTargetInfo ().GenerateDefaultIndex ();

    // Add indices on the source and target based on cardinality
    switch (cardinality)
        {
        case RelationshipClassMapInfo::CardinalityType::OneToOne:
            {
            if (createOnSource)
                AddIndicesToRelationshipEnds (RIDX_SourceOnly, enforceUniqueness);

            if (createOnTarget)
                AddIndicesToRelationshipEnds (RIDX_TargetOnly, enforceUniqueness);
            break;
            }
        case RelationshipClassMapInfo::CardinalityType::OneToMany:
            {
            if (createOnSource)
                AddIndicesToRelationshipEnds (RIDX_SourceOnly, false);

            if (createOnTarget)
                AddIndicesToRelationshipEnds (RIDX_TargetOnly, enforceUniqueness);
            break;
            }
        case RelationshipClassMapInfo::CardinalityType::ManyToOne:
            {
            if (createOnSource)
                AddIndicesToRelationshipEnds (RIDX_SourceOnly, enforceUniqueness);

            if (createOnTarget)
                AddIndicesToRelationshipEnds (RIDX_TargetOnly, false);
            break;
            }
        case RelationshipClassMapInfo::CardinalityType::ManyToMany:
            {
            if (createOnSource)
                AddIndicesToRelationshipEnds (RIDX_SourceOnly, false);

            if (createOnTarget)
                AddIndicesToRelationshipEnds (RIDX_TargetOnly, false);

            /*
            * Make the entire relationship instance unique unless the user wants otherwise
            * Note: We make the relationship unique by default to support the usual case.
            * An example of an unlikely case where the user would want duplicate relationships is a
            * a "queue of tasks". The same task can figure in a queue more than once!
            */
            RelationshipClassMapInfo::PreferredDirection preferredDirection = relationshipClassMapInfo.GetUserPreferredDirection ();
            if (preferredDirection == RelationshipClassMapInfo::PreferredDirection::TargetToSource && relationshipClassMapInfo.GetAllowDuplicateRelationships () != RelationshipClassMapInfo::TriState::True)
                {
                if (createOnSource)
                    AddIndicesToRelationshipEnds (RIDX_TargetToSource, true); // unique index starting with target
                }
            else
                {
                if (createOnTarget)
                    AddIndicesToRelationshipEnds (RIDX_SourceToTarget, true); // unique index starting with target
                }

            break;
            }
        default:
            break;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbIndexPtr RelationshipClassLinkTableMap::CreateIndex (RelationshipIndexSpec spec, bool isUniqueIndex)
    {
    // Setup name of the index
    Utf8String name = "idx_ECRel_";
    switch(spec)
        {
        case RIDX_SourceOnly:
            name.append ("Source_");
            break;
        case RIDX_TargetOnly:
            name.append ("Target_");
            break;
        case RIDX_SourceToTarget:
            name.append ("SourceToTarget_");
            break;
        case RIDX_TargetToSource:
            name.append ("TargetToSource_");
            break;
        default:
            break;
        }
    if (isUniqueIndex)
        name.append ("Unique_");
    
    name.append (GetTable ().GetName());

    return DbIndex::Create (name.c_str (), GetTable (), isUniqueIndex);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddColumnsToIndex (DbIndexR index, DbColumnCP col1, DbColumnCP col2, DbColumnCP col3, DbColumnCP col4)
    {
    if (nullptr != col1 && !col1->IsVirtual ())
        index.AddColumn (*col1);

    if (nullptr != col2 && !col2->IsVirtual ())
        index.AddColumn (*col2);

    if (nullptr != col3 && !col3->IsVirtual ())
        index.AddColumn (*col3);

    if (nullptr != col4 && !col4->IsVirtual ())
        index.AddColumn (*col4);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndicesToRelationshipEnds (RelationshipIndexSpec spec, bool addUniqueIndex)
    {
    auto sourceECInstanceIdColumn = GetSourceECInstanceIdPropMap()->GetFirstColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->GetFirstColumn();
    auto targetECInstanceIdColumn = GetTargetECInstanceIdPropMap()->GetFirstColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->GetFirstColumn();
    DbIndexPtr index = CreateIndex (spec, addUniqueIndex);
    switch(spec)
        {
        case RIDX_SourceOnly:
            {
            // CREATE [UNIQUE] INDEX <Name> ON <TableName> (RK_Source, [RC_Source])
            AddColumnsToIndex (*index, sourceECInstanceIdColumn, sourceECClassIdColumn, nullptr, nullptr);
            break;
            }
        case RIDX_TargetOnly:
            {
            // CREATE [UNIQUE] INDEX <Name> ON <TableName> (RK_Target, [RC_Target])
            AddColumnsToIndex (*index, targetECInstanceIdColumn, targetECClassIdColumn, nullptr, nullptr);
            break;
            }
        case RIDX_SourceToTarget:
            {
            // CREATE [UNIQUE] INDEX <Name> ON <TableName> (RK_Source, [RC_Source], RK_Target, [RC_Target])
            AddColumnsToIndex (*index, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn);
            break;
            }
        case RIDX_TargetToSource:
            {
            // CREATE [UNIQUE] INDEX <Name> ON <TableName> (RK_Target, [RC_Target], RK_Source, [RC_Source])
            AddColumnsToIndex (*index, targetECInstanceIdColumn, targetECClassIdColumn, sourceECInstanceIdColumn, sourceECClassIdColumn);
            break;
            }
        }

    if (!index.IsNull())
        GetTable ().AddIndex (*index);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECInstanceIdColumnName (Utf8StringR columnName, ECRelationshipEnd relationshipEnd, DbTableCR table) const
    {
    if (columnName.empty ())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? DEFAULT_SOURCEECINSTANCEID_COLUMNNAME : DEFAULT_TARGETECINSTANCEID_COLUMNNAME;

    if (!table.GetColumns ().Contains (columnName.c_str()))
        return true;

    if (GetMapStrategy() == MapStrategy::SharedTableForThisClass)
        return true;
    //Following error occure in Upgrading ECSchema but is not fatal.
    LOG.errorv ("Table %s already contains column named %s. ECRelationship %s has failed to map.", 
        table.GetName(), columnName.c_str (), Utf8String (m_ecClass.GetFullName()).c_str ());
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassLinkTableMap::GetConstraintECClassIdColumnName (Utf8StringR columnName, ECRelationshipEnd relationshipEnd, DbTableCR table) const
    {
    if (columnName.empty ())
        columnName = (relationshipEnd == ECRelationshipEnd_Source) ? DEFAULT_SOURCEECCLASSID_COLUMNNAME : DEFAULT_TARGETECCLASSID_COLUMNNAME;

    if (!table.GetColumns ().Contains (columnName.c_str()))
        return true;
    
    if (GetMapStrategy() == MapStrategy::SharedTableForThisClass)
        return true;

    //Following error occure in Upgrading ECSchema but is not fatal.
    LOG.errorv (L"Table %s already contains column named %s. ECRelationship %s has failed to map.", 
        table.GetName(), columnName.c_str (), Utf8String (m_ecClass.GetFullName()).c_str ());
    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
