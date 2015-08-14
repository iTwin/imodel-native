/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
RelationshipClassMap::RelationshipClassMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
    : ClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty), 
    m_sourceConstraintMap (ecDbMap.GetECDbR ().Schemas (), ecRelClass.GetSource ()),
    m_targetConstraintMap (ecDbMap.GetECDbR ().Schemas (), ecRelClass.GetTarget ())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn* RelationshipClassMap::CreateConstraintColumn (Utf8CP columnName, bool addToTable)
    {
    ECDbSqlColumn* column = GetTable ().FindColumnP (columnName);
    if (column != nullptr)
        return column;

    if (GetTable ().GetOwnerType () == OwnerType::ECDb)
        {
        column = GetTable ().CreateColumn (columnName, ECDbSqlColumn::Type::Long, ECDbDataColumn,
            addToTable ? PersistenceType::Persisted : PersistenceType::Virtual);
        }
    else
        {
        GetTable ().GetEditHandleR ().BeginEdit ();
        column = GetTable ().CreateColumn (columnName, ECDbSqlColumn::Type::Long, ECDbDataColumn, PersistenceType::Virtual);
        GetTable ().GetEditHandleR ().EndEdit ();
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
RelationshipClassMap::ConstraintMap const& RelationshipClassMap::GetConstraintMap (ECN::ECRelationshipEnd constraintEnd) const
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
// @bsimethod                               Muhammad.Zaighum                        04/13
//---------------------------------------------------------------------------------------
ECN::ECRelationshipConstraintCR RelationshipClassMap::ConstraintMap::GetRelationshipConstraint()const
    {
    return m_constraint;
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

//************************ RelationshipClassEndTableMap **********************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassEndTableMap::RelationshipClassEndTableMap (ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: RelationshipClassMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan       02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSqlColumn* RelationshipClassEndTableMap::ConfigureForeignECClassIdKey(RelationshipMapInfo const& mapInfo, ECRelationshipConstraintCR otherEndConstraint, IClassMap const& otheEndClassMap, size_t otherEndTableCount)
    {
    RelationshipEndColumns const& constraintColumnsMapping = GetEndColumnsMapping(mapInfo);
    Utf8String classIdColName(constraintColumnsMapping.GetECClassIdColumnName());
    if (classIdColName.empty() &&
        !GetOtherEndECClassIdColumnName(classIdColName, GetTable(), true))
        return nullptr;

    ECDbSqlColumn* otherEndECClassIdColumn = nullptr;
    if (ConstraintIncludesAnyClass(otherEndConstraint.GetClasses()) || otherEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        otherEndECClassIdColumn = CreateConstraintColumn(classIdColName.c_str(), true);
        BeAssert (otherEndECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        otherEndECClassIdColumn = const_cast<ECDbSqlColumn*>(otheEndClassMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECClassId));
        if (otherEndECClassIdColumn == nullptr)
            otherEndECClassIdColumn = CreateConstraintColumn(classIdColName.c_str(), false);
        }

    return otherEndECClassIdColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_InitializePart1 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    //Don't call base class method as end table map requires its own handling
    BeAssert (GetMapStrategy ().IsForeignKeyMapping());
    m_dbView = CreateClassDbView ();

    RelationshipMapInfo const& relationshipClassMapInfo = dynamic_cast<RelationshipMapInfo const&> (classMapInfo);
    BeAssert (m_ecClass.GetRelationshipClassCP () != nullptr && classMapInfo.GetMapStrategy ().IsForeignKeyMapping());

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

    auto thisEnd = GetThisEnd ();
    auto const& otherEndConstraint = thisEnd == ECRelationshipEnd_Source ? targetConstraint : sourceConstraint;
    auto const& thisEndConstraint = thisEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;

    auto thisEndClass = thisEndConstraint.GetClasses()[0];
    auto thisEndClassMap = GetECDbMap ().GetClassMapCP (*thisEndClass, true);
    size_t thisEndTableCount = GetECDbMap ().GetTableCountOnRelationshipEnd (thisEndConstraint);

    if (thisEndTableCount != 1)
        {
        BeAssert(thisEndTableCount == 1 && "ForeignKey end of relationship has more than one tables or has no table at all");
        return MapStatus::Error;
        }

    auto otherEndClass = otherEndConstraint.GetClasses ()[0];
    auto otheEndClassMap = GetECDbMap ().GetClassMapCP (*otherEndClass, true);
    size_t otherEndTableCount = GetECDbMap().GetTableCountOnRelationshipEnd(otherEndConstraint);

    //*** persistence end table
    SetTable (&thisEndClassMap->GetTable ());

    //if no class id column on this end is required, store the class id directly so that it can be used as literal in the native SQL
    const ECClassId defaultThisEndECClassId = thisEndClass->GetId ();

    //**** Other End
    ECDbSqlColumn* otherEndECClassIdColumn = ConfigureForeignECClassIdKey (relationshipClassMapInfo, otherEndConstraint, *otheEndClassMap, otherEndTableCount);
    if (otherEndECClassIdColumn == nullptr)
        {
        BeAssert (false && "Failed to create foreign ECClassId column for relationship");
        return MapStatus::Error;
        }

    ECDbSqlColumn* otherEndECInstanceIdColumn = nullptr;
    auto stat = CreateConstraintColumns(otherEndECInstanceIdColumn, relationshipClassMapInfo, thisEnd, thisEndConstraint);
    if (stat != MapStatus::Success)
        return stat;

    //**** Prop Maps
    //Reuse ECInstanceId prop map from persistence end class map
    PropertyMapPtr ecInstanceIdPropMap = nullptr;
    if (!thisEndClassMap->TryGetECInstanceIdPropertyMap (ecInstanceIdPropMap))
        {
        BeAssert (false);
        return MapStatus::Error;
        }

    GetPropertyMapsR().AddPropertyMap(ecInstanceIdPropMap);
    stat = CreateConstraintPropMaps (thisEnd, defaultThisEndECClassId, otherEndECInstanceIdColumn, otherEndECClassIdColumn, otherEndClass->GetId());
    if (stat != MapStatus::Success)
        return stat;

    //! Add referential integrity if user requested it.
    if (relationshipClassMapInfo.IsCreateForeignKeyConstraint () && relationshipClass.GetStrength() != StrengthType::STRENGTHTYPE_Holding)
        {
        auto const& otherEndConstraint = thisEnd != ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
        auto const& otherEndConstraintMap = thisEnd != ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;

        if (GetECDbMap().GetTableCountOnRelationshipEnd(otherEndConstraint) != 1)
            {
            BeAssert(false);
            return MapStatus::Error;
            }

        auto foreignKeyColumn = otherEndConstraintMap.GetECInstanceIdPropMap()->GetFirstColumn();
        auto& foreignTable = GetTable ();
        auto primaryClassMap = GetECDbMap().GetClassMap(*otherEndConstraintMap.GetRelationshipConstraint().GetClasses()[0]);
        BeAssert(primaryClassMap!=nullptr && "Primary Class map is null");
        auto primaryKeyColumn = primaryClassMap->GetTable().GetFilteredColumnFirst(ECDbSystemColumnECInstanceId);
        auto& primaryTable = primaryKeyColumn->GetTable();
        
        BeAssert (foreignKeyColumn != nullptr);
        BeAssert (primaryKeyColumn != nullptr);
        if (foreignKeyColumn == nullptr || primaryKeyColumn == nullptr)
            return MapStatus::Error;
            
        //Create foreign key constraint
        auto foreignKey = foreignTable.CreateForeignKeyConstraint (primaryTable);
        foreignKey->Add (foreignKeyColumn->GetName ().c_str (), primaryKeyColumn->GetName ().c_str ());
        
        foreignKey->SetOnDeleteAction(relationshipClassMapInfo.GetOnDeleteAction());
        foreignKey->SetOnUpdateAction(relationshipClassMapInfo.GetOnUpdateAction());
        }
    
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::_InitializePart2 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    //add non-system property maps
    AddPropertyMaps (parentClassMap, nullptr,&classMapInfo);

    //**** Add indices
    AddIndices (classMapInfo);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::_Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    if (ClassMap::_Load (loadGraph, mapInfo, parentClassMap) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    m_dbView = CreateClassDbView ();   
    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();
    auto thisEnd = GetThisEnd ();
    auto const& thisEndConstraint = thisEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;
    auto const& otherEndConstraint = thisEnd == ECRelationshipEnd_Source ? sourceConstraint : targetConstraint;

    ECClassId defaultSourceECClassId, defaultTargetECClassId;

    if (thisEnd == ECRelationshipEnd_Source)
        {
        defaultSourceECClassId = thisEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : thisEndConstraint.GetClasses().front()->GetId();
        defaultTargetECClassId = otherEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : otherEndConstraint.GetClasses().front()->GetId();
        }
    else
        {
        defaultTargetECClassId = thisEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : thisEndConstraint.GetClasses().front()->GetId();
        defaultSourceECClassId = otherEndConstraint.GetClasses().empty() ? ECClass::UNSET_ECCLASSID : otherEndConstraint.GetClasses().front()->GetId();
        }

    auto sourceECInstanceIdProperty = ECDbSystemSchemaHelper::GetSystemProperty (Schemas (), ECSqlSystemProperty::SourceECInstanceId);
    auto pm = mapInfo.FindPropertyMap (sourceECInstanceIdProperty->GetId (), ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to deserialize property map");
        return BentleyStatus::ERROR;
        }

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), *this, DEFAULT_SOURCEECINSTANCEID_COLUMNNAME);
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


    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultSourceECClassId, *this, DEFAULT_SOURCEECCLASSID_COLUMNNAME, nullptr);
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

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), *this, DEFAULT_TARGETECINSTANCEID_COLUMNNAME);
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

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultTargetECClassId, *this, DEFAULT_TARGETECCLASSID_COLUMNNAME, nullptr);
    PRECONDITION (targetECClassIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintColumns(ECDbSqlColumn*& otherEndECInstanceIdColumn, RelationshipMapInfo const& mapInfo, ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint)
    {
    ECDbSqlColumn const* keyPropertyCol = nullptr;
    if (SUCCESS != TryGetKeyPropertyColumn(keyPropertyCol, constraint, *mapInfo.GetECClass().GetRelationshipClassCP(), constraintEnd))
        return MapStatus::Error;

    if (keyPropertyCol != nullptr)
        otherEndECInstanceIdColumn = const_cast<ECDbSqlColumn*>(keyPropertyCol);
    else
        {
        //** Other End ECInstanceId column
        RelationshipEndColumns const& constraintColumnMapping = GetEndColumnsMapping(mapInfo);
        Utf8String otherEndECInstanceIdColumnName(constraintColumnMapping.GetECInstanceIdColumnName());
        if (otherEndECInstanceIdColumnName.empty())
            if (!GetOtherEndKeyColumnName(otherEndECInstanceIdColumnName, GetTable(), true))
                return MapStatus::Error;

        otherEndECInstanceIdColumn = CreateConstraintColumn(otherEndECInstanceIdColumnName.c_str(), true);
        BeAssert(otherEndECInstanceIdColumn != nullptr);
        }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassEndTableMap::CreateConstraintPropMaps 
(
ECRelationshipEnd thisEnd,
ECClassId defaultThisEndClassId,
ECDbSqlColumn* const& otherEndECInstanceIdColumn, 
ECDbSqlColumn* const& otherEndECClassIdColumn,
ECClassId defaultOtherEndClassId
)
    {
    //Now add prop maps for source/target ecinstance id and ecclass id prop maps
    //Existing this end instance id and class id columns will be reused
    auto& persistenceEndTable = GetTable();
    std::vector<ECDbSqlColumn const*> systemColumns;
    if (persistenceEndTable.GetFilteredColumnList (systemColumns, ECDbSystemColumnECInstanceId) == BentleyStatus::ERROR)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return MapStatus::Error;
        }

    auto thisEndECInstanceIdColumn = const_cast<ECDbSqlColumn*>(systemColumns.front ());
    auto thisEndECClassIdColumn = GetTable ().FindColumnP ("ECClassId");

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
    if (thisEnd == ECRelationshipEnd_Source)
        {

        sourceECInstanceIdColumn = thisEndECInstanceIdColumn;
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

        targetECInstanceIdColumn = thisEndECInstanceIdColumn;
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

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas(), sourceECInstanceIdColumn, *this, sourceECInstanceIdViewColumnAlias);
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECInstanceIdPropMap);
    m_sourceConstraintMap.SetECInstanceIdPropMap (sourceECInstanceIdPropMap.get ());

    auto persistenceEndTableP = sourceECClassIdColIsDelayGenerated ? &persistenceEndTable : nullptr;
    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), sourceECClassIdColumn, defaultSourceECClassId, *this, sourceECClassIdViewColumnAlias, persistenceEndTableP);
    PRECONDITION(sourceECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(sourceECClassIdPropMap);
    m_sourceConstraintMap.SetECClassIdPropMap (sourceECClassIdPropMap.get ());
 
    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), targetECInstanceIdColumn, *this, targetECInstanceIdViewColumnAlias);
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECInstanceIdPropMap);
    m_targetConstraintMap.SetECInstanceIdPropMap (targetECInstanceIdPropMap.get ());

    persistenceEndTableP = targetECClassIdColIsDelayGenerated ? &persistenceEndTable : nullptr;
    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), targetECClassIdColumn, defaultTargetECClassId, *this, targetECClassIdViewColumnAlias, persistenceEndTableP);
    PRECONDITION(targetECClassIdPropMap.IsValid(), MapStatus::Error);
    GetPropertyMapsR ().AddPropertyMap(targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return MapStatus::Success;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan         9/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassEndTableMap::AddIndices (ClassMapInfo const& mapInfo)
    {
    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&mapInfo) != nullptr);
    RelationshipMapInfo const& info = static_cast<RelationshipMapInfo const&> (mapInfo);
    switch (info.GetCardinality())
        {
            case RelationshipMapInfo::Cardinality::OneToOne:
                AddIndexToRelationshipEnd(true);
                break;
            case RelationshipMapInfo::Cardinality::OneToMany:
            case RelationshipMapInfo::Cardinality::ManyToOne:
                AddIndexToRelationshipEnd (false);
                break;
            case RelationshipMapInfo::Cardinality::ManyToMany:
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

    if (persistenceEndTable.GetOwnerType () == OwnerType::ExistingTable)
        return;

    // Setup name of the index
    Utf8String name = "idx_ECRel_";
    if (GetMapStrategy ().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable)
        name.append("Source_");
    else
        name.append("Target_");
    if (isUniqueIndex)
        name.append ("Unique_");
    name.append (m_ecClass.GetSchema().GetNamespacePrefix() + "_" + m_ecClass.GetName());
    
    auto existingIndex = persistenceEndTable.GetDbDef ().FindIndex (name.c_str ());
    if (existingIndex)
        {
        if (&existingIndex->GetTable () == &GetTable ())
            return;

        BeAssert (false && "Index with same name exist on a different table");
        return;
        }

    ECDbSqlIndex* index = persistenceEndTable.CreateIndex (name.c_str (), isUniqueIndex, GetClass().GetId());
    if (index == nullptr)
        {
        BeAssert (false && "Failed to create index");
        return;
        }

    BeAssert (GetOtherEndECInstanceIdPropMap() != nullptr && GetOtherEndECInstanceIdPropMap()->GetFirstColumn() != nullptr);
    index->Add (GetOtherEndECInstanceIdPropMap()->GetFirstColumn()->GetName().c_str());
    Utf8String whereExpr = "[" + GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ()->GetName () + "] IS NOT NULL";
    index->SetAdditionalWhereExpression (whereExpr.c_str ());
    }

   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetRelationshipColumnName (Utf8StringR columnName, ECDbSqlTable const& table, Utf8CP prefix, bool mappingInProgress) const
    {
    Utf8String classNameUtf8 (m_ecClass.GetName());
    columnName = prefix;
    columnName.append (classNameUtf8);
    
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
            table.GetName().c_str(), columnName.c_str(), Utf8String (m_ecClass.GetFullName()).c_str ());
        return false;
        }
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetOtherEndKeyColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "ForeignECInstanceId_", mappingInProgress);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipClassEndTableMap::GetOtherEndECClassIdColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const
    {
    return GetRelationshipColumnName (columnName, table, "ForeignECClassId_", mappingInProgress);
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
    return GetMapStrategy ().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetOtherEnd () const
    {
    return GetThisEnd () == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2015
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipEndColumns const& RelationshipClassEndTableMap::GetEndColumnsMapping(RelationshipMapInfo const& info) const
    {
    BeAssert(GetThisEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? info.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable : info.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);
    return RelationshipClassMap::GetEndColumnsMapping(info, GetThisEnd());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          06/2015
//+---------------+---------------+---------------+---------------+---------------+------
void LogKeyPropertyRetrievalError(Utf8CP errorDetails, ECRelationshipClassCR relClass, ECRelationshipEnd constraintEnd)
    {
    const NativeLogging::SEVERITY sev = NativeLogging::LOG_ERROR;
    if (LOG.isSeverityEnabled(sev))
        LOG.messagev(sev, "Invalid Key property on %s constraint in ECRelationshipClass '%s': %s",
               constraintEnd == ECRelationshipEnd_Source ? "source" : "target", relClass.GetFullName(), errorDetails);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                      Krischan.Eberle                          06/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::TryGetKeyPropertyColumn(ECDbSqlColumn const*& keyPropertyColumn, ECRelationshipConstraintCR constraint, ECRelationshipClassCR relClass, ECRelationshipEnd constraintEnd) const
    {
    keyPropertyColumn = nullptr;
    ECClassCP keyPropertyContainer = nullptr;
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    if (constraintClasses.size() == 0)
        return SUCCESS;

    Utf8StringCP foundPropName = nullptr;
    bool isMappable = constraintClasses.size() == 1;
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        bvector<Utf8String> const& keys = constraintClass->GetKeys();
        const size_t keyCount = keys.size();
        if (keyCount >= 1)
            {
            foundPropName = &keys[0];
            keyPropertyContainer = &constraintClass->GetClass();
            if (keyCount > 1 || foundPropName->empty())
                isMappable = false;

            break;
            }
        }

    if (foundPropName == nullptr)
        return SUCCESS;

    if (!isMappable)
        {
        LogKeyPropertyRetrievalError("ECDb only supports Key properties if the constraint consists of a single ECClass and only a single Key property is defined for that ECClass.",
                                     relClass, constraintEnd);
        return ERROR;
        }

    IClassMap const* classMap = GetECDbMap().GetClassMap(*keyPropertyContainer);
    if (classMap == nullptr || classMap->GetMapStrategy().IsNotMapped())
        {
        BeAssert(false && "Class on relationship end is not mapped. This should have been caught before.");
        return ERROR;
        }

    PropertyMap const* keyPropertyMap = classMap->GetPropertyMap(foundPropName->c_str());
    if (keyPropertyMap == nullptr || keyPropertyMap->IsUnmapped() || keyPropertyMap->IsVirtual())
        {
        Utf8String error;
        error.Sprintf("Key property '%s' does not exist or is not mapped.", foundPropName->c_str());
        LogKeyPropertyRetrievalError(error.c_str(), relClass, constraintEnd);
        return ERROR;
        }

    ECSqlTypeInfo typeInfo(keyPropertyMap->GetProperty());
    if (!typeInfo.IsExactNumeric() && !typeInfo.IsString())
        {
        Utf8String error;
        error.Sprintf("Unsupported data type of Key property '%s'. ECDb only supports integral or string Key properties.", foundPropName->c_str());
        LogKeyPropertyRetrievalError(error.c_str(), relClass, constraintEnd);
        return ERROR;
        }

    std::vector<ECDbSqlColumn const*> columns;
    keyPropertyMap->GetColumns(columns);
    if (columns.size() != 1)
        {
        BeAssert(false && "Key property map is expected to map to a single column.");
        return ERROR;
        }

    keyPropertyColumn = columns[0];
    return SUCCESS;
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
MapStatus RelationshipClassLinkTableMap::_InitializePart1 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    BeAssert (!GetMapStrategy ().IsForeignKeyMapping() &&
        "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");

    auto stat = RelationshipClassMap::_InitializePart1 (classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    //**** Class View for SELECT statement
    m_dbView = CreateClassDbView ();

    BeAssert (dynamic_cast<RelationshipMapInfo const*> (&classMapInfo) != nullptr);
    RelationshipMapInfo const& relationClassMapInfo = static_cast<RelationshipMapInfo const&> (classMapInfo);

    ECRelationshipClassCR relationshipClass = GetRelationshipClass ();
    auto const& sourceConstraint = relationshipClass.GetSource ();
    auto const& targetConstraint = relationshipClass.GetTarget ();

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
    auto relationship = mapInfo.GetECClass ().GetRelationshipClassCP ();
    BeAssert (relationship != nullptr);
    auto const& thisEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? relationship->GetSource () : relationship->GetTarget ();
    auto thisEndClass = thisEndConstraint.GetClasses ()[0];
    auto thisEndClassMap = GetECDbMap ().GetClassMapCP (*thisEndClass, true);
    size_t thisEndTableCount = GetECDbMap ().GetTableCountOnRelationshipEnd (thisEndConstraint);

    RelationshipEndColumns const& constraintColumnsMapping = GetEndColumnsMapping(mapInfo, relationshipEnd);
    Utf8String columnName(constraintColumnsMapping.GetECClassIdColumnName());
    if (columnName.empty())
        {
        if (!GetConstraintECClassIdColumnName (columnName, relationshipEnd, GetTable ()))
            return nullptr;
        }

    if (ConstraintIncludesAnyClass (thisEndConstraint.GetClasses ()) || thisEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn (columnName.c_str (), true);
        BeAssert (endECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        endECClassIdColumn = const_cast<ECDbSqlColumn*>(thisEndClassMap->GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn (columnName.c_str (), false);
        }

    return endECClassIdColumn;
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
MapStatus RelationshipClassLinkTableMap::_InitializePart2 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    auto stat = RelationshipClassMap::_InitializePart2 (classMapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&classMapInfo) != nullptr);
    RelationshipMapInfo const& relationClassMapInfo = static_cast<RelationshipMapInfo const&> (classMapInfo);

    //**** Add relationship indices
    AddIndices (relationClassMapInfo);

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
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Source, GetTable()))
            return MapStatus::Error;
        }

    auto sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (), true);
    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), sourceECInstanceIdColumn, *this);
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
        if (!GetConstraintECInstanceIdColumnName (columnName, ECRelationshipEnd_Target, GetTable()))
            return MapStatus::Error;
        }

    auto targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str (), true);

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), targetECInstanceIdColumn, *this);
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
void RelationshipClassLinkTableMap::AddIndices (ClassMapInfo const& mapInfo)
    {
    if (GetTable ().GetOwnerType () == OwnerType::ExistingTable)
        return;

    BeAssert(dynamic_cast<RelationshipMapInfo const*> (&mapInfo) != nullptr);
    RelationshipMapInfo const& relationshipClassMapInfo = static_cast<RelationshipMapInfo const&> (mapInfo);

    RelationshipMapInfo::Cardinality cardinality = relationshipClassMapInfo.GetCardinality();
    const bool enforceUniqueness = !relationshipClassMapInfo.AllowDuplicateRelationships();
    // TODO: We need to enforce uniqueness of constraints, but cannot at the moment since we get these 
    // i-models that don't honor these constraints. For example, ConstructSim has a ComponentHasComponent
    // 1-M relationship, but ends up having many instances on the source instead of one. There are
    // SystemComponent-s that include GroupingComponent-s and PartComponent-s, where the GroupingComponent-s
    // themselves include the SystemComponent-s again. 

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

    AddIndicesToRelationshipEnds(RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndicesToRelationshipEnds(RelationshipIndexSpec::Target, targetIsUnique);

    if (enforceUniqueness)
        AddIndicesToRelationshipEnds(RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSqlIndex* RelationshipClassLinkTableMap::CreateIndex (RelationshipIndexSpec spec, bool isUniqueIndex)
    {
    // Setup name of the index
    Utf8String name;
    if (isUniqueIndex)
        name.append("uix_");
    else
        name.append("ix_");

    if (isUniqueIndex)
        name.append("unique_");

    //add class full name as subclasses of the relationship might require different indices
    Utf8StringCR nsPrefix = GetClass().GetSchema().GetNamespacePrefix();
    name.append(!nsPrefix.empty() ? nsPrefix : GetClass().GetSchema().GetName()).append("_").append(GetClass().GetName()).append("_");
   
    switch(spec)
        {
        case RelationshipIndexSpec::Source:
            name.append ("Source");
            break;
        case RelationshipIndexSpec::Target:
            name.append ("Target");
            break;
        case RelationshipIndexSpec::SourceAndTarget:
            name.append("SourceTarget");  
            break;
        default:
            BeAssert(false);
            break;
        }
    
    ECDbSqlIndex* existingIndex = GetTable().GetDbDefR ().FindIndexP (name.c_str ());
    if (existingIndex != nullptr)
        {
        if (&existingIndex->GetTable () == &GetTable ())
            return existingIndex;
        else
            BeAssert (false && "Index with same name exist on a different table");

        return nullptr;
        }

    return GetTable().CreateIndex(name.c_str(), isUniqueIndex, GetClass().GetId());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddColumnsToIndex (ECDbSqlIndex& index, ECDbSqlColumn const* col1, ECDbSqlColumn const* col2, ECDbSqlColumn const* col3, ECDbSqlColumn const* col4)
    {
    if (nullptr != col1 && col1->GetPersistenceType () == PersistenceType::Persisted)
        index.Add (col1->GetName().c_str());

    if (nullptr != col2 && col2->GetPersistenceType () == PersistenceType::Persisted)
        index.Add (col2->GetName ().c_str ());

    if (nullptr != col3 && col3->GetPersistenceType () == PersistenceType::Persisted)
        index.Add (col3->GetName ().c_str ());

    if (nullptr != col4 && col4->GetPersistenceType () == PersistenceType::Persisted)
        index.Add (col4->GetName ().c_str ());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndicesToRelationshipEnds (RelationshipIndexSpec spec, bool addUniqueIndex)
    {
    auto sourceECInstanceIdColumn = GetSourceECInstanceIdPropMap()->GetFirstColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap ()->IsMappedToPrimaryTable() ? GetSourceECClassIdPropMap ()->GetFirstColumn () : nullptr;
    auto targetECInstanceIdColumn = GetTargetECInstanceIdPropMap()->GetFirstColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->IsMappedToPrimaryTable() ? GetTargetECClassIdPropMap ()->GetFirstColumn () : nullptr;
    auto index = CreateIndex (spec, addUniqueIndex);
    if (index == nullptr)
        return;
   
    switch(spec)
        {
        case RelationshipIndexSpec::Source:
            AddColumnsToIndex (*index, sourceECInstanceIdColumn, sourceECClassIdColumn, nullptr, nullptr);
            break;
        case RelationshipIndexSpec::Target:
            AddColumnsToIndex (*index, targetECInstanceIdColumn, targetECClassIdColumn, nullptr, nullptr);
            break;

        case RelationshipIndexSpec::SourceAndTarget:
            AddColumnsToIndex(*index, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn);
            break;

        default:
            BeAssert(false);
            break;
        }

    if (!index->IsValid())
        index->Drop();
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
BentleyStatus RelationshipClassLinkTableMap::_Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    if (ClassMap::_Load (loadGraph, mapInfo, parentClassMap) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

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
        return BentleyStatus::ERROR;
        }

    auto sourceECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), *this, DEFAULT_SOURCEECINSTANCEID_COLUMNNAME);
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


    auto sourceECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Source, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultSourceECClassId, *this, DEFAULT_SOURCEECCLASSID_COLUMNNAME, nullptr);
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

    auto targetECInstanceIdPropMap = PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), *this, DEFAULT_TARGETECINSTANCEID_COLUMNNAME);
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

    auto targetECClassIdPropMap = PropertyMapRelationshipConstraintClassId::Create (ECRelationshipEnd_Target, Schemas (), const_cast<ECDbSqlColumn*>(&pm->GetColumn ()), defaultTargetECClassId, *this, DEFAULT_TARGETECCLASSID_COLUMNNAME, nullptr);
    PRECONDITION (targetECClassIdPropMap.IsValid (), BentleyStatus::ERROR);
    GetPropertyMapsR ().AddPropertyMap (targetECClassIdPropMap);
    m_targetConstraintMap.SetECClassIdPropMap (targetECClassIdPropMap.get ());

    return BentleyStatus::SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
