/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapInfo.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//**********************************************************************************************
// ClassMapInfoFactory
//**********************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassMapInfoPtr ClassMapInfoFactory::CreateFromHint (MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecDbMap)
    {
    mapStatus = MapStatus::Error;

    MapStrategy defaultStrategy = ClassMapInfo::GetDefaultMapStrategy ();
    //TODO: We will not redirect the strategy in 05 to avoid unforeseeable side-effects and backwards compatibility breaks.
    //Should be done though post 05
    //if (ECSchemaValidator::ContainsInvalidClass (schemaImportContext.GetInvalidClasses (), ecClass))
    //    defaultStrategy = MapStrategy::SharedTableForThisClass;

    auto info = CreateInstance (ecClass, ecDbMap, nullptr, nullptr, defaultStrategy);
    BeAssert (info != nullptr);
    if (info == nullptr)
        return nullptr;

    mapStatus = info->EvaluateMapStrategy ();
    if (mapStatus != MapStatus::Success)
        return nullptr;

    if (!ClassMap::IsDoNotMapStrategy (info->GetMapStrategy ()))
        {
        auto validationResult = schemaImportContext.GetSchemaValidationResult (ecClass.GetSchema ());
        for (auto& error : validationResult->GetErrors ())
            {
            if (error->GetRuleType () == ECSchemaValidationRule::Type::CaseInsensitiveClassNames)
                {
                auto casingError = static_cast<CaseInsensitiveClassNamesRule::Error const*> (error.get ());
                auto invalidClasses = casingError->TryGetInvalidClasses (ecClass);
                if (invalidClasses != nullptr)
                    {
                    for (auto invalidClass : *invalidClasses)
                        {
                        if (invalidClass == &ecClass) //don't check the class against itself.
                            continue;

                        auto classMap = ecDbMap.GetClassMap (*invalidClass, false);
                        if (classMap == nullptr)
                            continue;

                        //Relationship classes for which the name only differs by case are not supported by ECDb (in contrast to regular ECClasses)
                        if (ecClass.GetRelationshipClassCP () != nullptr && classMap->IsRelationshipClassMap ())
                            {
                            info->m_mapStrategy = MapStrategy::DoNotMap;
                            schemaImportContext.GetIssueListener ().Report (ECDbSchemaManager::IImportIssueListener::Severity::Warning,
                                "Did not map ECRelationshipClass '%s': ECRelationshipClasses for which names only differ by case are not supported by ECDb.", Utf8String (ecClass.GetFullName ()).c_str ());
                            }
                        }
                    }
                }
            }
        }

    return info;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassMapInfoPtr ClassMapInfoFactory::CreateInstance (ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, MapStrategy mapStrategy)
    {
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP ();
    if (ecRelationshipClass != nullptr)
        return RelationshipClassMapInfo::Create (*ecRelationshipClass, ecDbMap, tableName, primaryKeyColumnName, mapStrategy);
    else
        return ClassMapInfo::Create (ecClass, ecDbMap, tableName, primaryKeyColumnName, mapStrategy);
    }



//**********************************************************************************************
// ClassMapInfo
//**********************************************************************************************

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapInfoPtr ClassMapInfo::Create (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, MapStrategy mapStrategy)
    {
    return new ClassMapInfo (ecClass, ecDbMap, tableName, primaryKeyColumnName, mapStrategy);
    }

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapInfo::ClassMapInfo (ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, MapStrategy mapStrategy)
: m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecInstanceIdColumnName (primaryKeyColumnName), m_tableName (tableName), m_mapToExistingTable (false),
m_mapStrategy (mapStrategy), m_isMapToVirtualTable (IClassMap::IsAbstractECClass (ecClass)),m_replaceEmptyTableWithEmptyView(true)
    {
    if (Utf8String::IsNullOrEmpty (tableName))
        InitializeFromSchema ();

    //Default values for table name and primary key column name
    if (m_tableName.empty ())
        {
        // ClassMappingRule: if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        m_tableName = ResolveTablePrefix (ecClass);
        if (!IClassMap::IsMapToSecondaryTableStrategy (ecClass))  // ClassMappingRule: assumes that structs are always used in arrays
            m_tableName.append ("_");
        else
            m_tableName.append ("_ArrayOf");

        m_tableName.append (Utf8String (ecClass.GetName ()));
        }

    if (m_ecInstanceIdColumnName.empty ())
        {
        // ClassMappingRule: if hint does not supply an ECInstanceId (primary key) column name, use ECDB_COL_ECInstanceId
        m_ecInstanceIdColumnName =  ECDB_COL_ECInstanceId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::EvaluateMapStrategy ()
    {
    return _EvaluateMapStrategy ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::_EvaluateMapStrategy ()
    {
    if (m_mapStrategy == MapStrategy::NoHint)
        m_mapStrategy = GetDefaultMapStrategy ();

    if (m_mapStrategy == MapStrategy::DoNotMapHierarchy)
        return MapStatus::Success;

    ECClassCR ecClass = GetECClass ();
    if (ecClass.GetIsCustomAttributeClass () && !ecClass.GetIsDomainClass () && !ecClass.GetIsStruct ())
        {
        LogClassNotMapped (NativeLogging::LOG_DEBUG, ecClass, "ECClass is a custom attribute which is never mapped to a table in ECDb.");
        m_mapStrategy = MapStrategy::DoNotMap;
        return MapStatus::Success;
        }

    if (IClassMap::IsAnyClass (ecClass) || (ecClass.GetSchema ().IsStandardSchema () && ecClass.GetName ().CompareTo (L"InstanceCount") == 0))
        {
        LogClassNotMapped (NativeLogging::LOG_INFO, ecClass, "ECClass is a standard class not supported by ECDb.");
        m_mapStrategy = MapStrategy::DoNotMap;
        return MapStatus::Success;
        }

    // ClassMappingRule: Classes within the same hierarchy should all be relationships or non-relationships
    bool isValid = ValidateBaseClasses ();
    if (!isValid)
        {
        m_mapStrategy = MapStrategy::DoNotMap;
        return MapStatus::Success;
        }

    return EvaluateInheritedMapStrategy ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::EvaluateInheritedMapStrategy ()
    {
    bvector<IClassMap const*> baseClassMaps;
    bvector<IClassMap const*> tphMaps; // TablePerHierarchy or InParentTable have the highest priority, but there can be only one
    bvector<IClassMap const*> tpcMaps; // TablePerClass have second priority
    bvector<IClassMap const*> nmhMaps; // DoNotMapHierarchy has priority only over NoHint or DoNotMap
    bool areBaseClassesMapped = GatherBaseClassMaps (baseClassMaps, tphMaps, tpcMaps, nmhMaps, m_ecClass);
    if (!areBaseClassesMapped)
        return MapStatus::BaseClassesNotMapped;

    // ClassMappingRule: No more than one ancestor of a class can use TablePerHierarchy strategy. Mapping fails if this is violated
    if (tphMaps.size () > 1)
        return ReportError_OneClassMappedByTableInHierarchyFromTwoDifferentAncestors (m_ecClass, tphMaps);

    // ClassMappingRule: If exactly 1 ancestor ECClass is using TablePerHierarchy, use InParentTable mapping
    if (tphMaps.size () == 1)
        {
        m_mapStrategy = MapStrategy::InParentTable;
        m_parentClassMap = tphMaps[0];
        }
    // ClassMappingRule: If one or more parent is using TablePerClass, use TablePerClass mapping
    else if (tpcMaps.size () > 0)
        m_mapStrategy = MapStrategy::TablePerClass;
    // ClassMappingRule: If one or more parent is using DoNotMapHierarchy, use DoNotMapHierarchy
    else if (nmhMaps.size () > 0)
        m_mapStrategy = MapStrategy::DoNotMapHierarchy;

    return MapStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassMapInfo::InitializeFromSchema ()
    {
    InitializeFromClassHint ();

    // Add indices for important identifiers
    ProcessStandardKeys (m_ecClass, L"BusinessKeySpecification");
    ProcessStandardKeys (m_ecClass, L"GlobalIdSpecification");
    ProcessStandardKeys (m_ecClass, L"SyncIDSpecification");
    
    //TODO: VerifyThatTableNameIsNotReservedName, e.g. dgn element table, etc.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                03/2014
//+---------------+---------------+---------------+---------------+---------------+------
void ClassMapInfo::InitializeFromClassHint ()
    {
    auto classHint = ClassHintReader::ReadHint (m_ecClass);
    if (classHint == nullptr)
        return;

    MapStrategy mapStrategy = MapStrategy::DoNotMap;
    if (ClassHintReader::TryReadMapStrategy (mapStrategy, *classHint))
        m_mapStrategy = mapStrategy;

    Utf8String tableName;
    if (ClassHintReader::TryReadTableName (tableName, *classHint))
        m_tableName = tableName;

    Utf8String ecInstanceIdColumnName;
    if (ClassHintReader::TryReadECInstanceIdColumnName (ecInstanceIdColumnName, *classHint))
        m_ecInstanceIdColumnName = ecInstanceIdColumnName;

    ClassHintReader::TryReadMapToExistingTable (m_mapToExistingTable, *classHint);
    ClassHintReader::TryReadReplaceEmptyTableWithEmptyView (m_replaceEmptyTableWithEmptyView, *classHint);

    ClassHintReader::TryReadIndices (m_hintIndexes, *classHint, m_ecClass);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassMapInfo::ProcessStandardKeys (ECClassCR ecClass, WCharCP customAttributeName)
    {
    StandardKeySpecification::Type keyType = StandardKeySpecification::GetTypeFromString(customAttributeName);
    if (keyType == StandardKeySpecification::Type::None)
        return;

    IECInstancePtr ca = ecClass.GetCustomAttribute (customAttributeName);
    if (ca.IsValid())
        {
        ECValue v;
        switch(keyType)
            {
                case StandardKeySpecification::Type::BusinessKeySpecification:
                case StandardKeySpecification::Type::GlobalIdSpecification:
                ca->GetValue (v, L"PropertyName"); break;
                case StandardKeySpecification::Type::SyncIDSpecification:
                ca->GetValue (v, L"Property"); break;
            }

        if (!v.IsNull())
            {
            //Create unique not null index on provided property
            ECPropertyP key;
            if ( nullptr == (key = ecClass.GetPropertyP(v.GetString())))
                LOG.errorv(L"Rejecting user specified %ls on class %ls because property specified in it '%ls' doesn't exist in class", customAttributeName, ecClass.GetFullName(), v.GetString());
            else
                {
                if (key->GetAsPrimitiveProperty() == nullptr)
                    LOG.errorv(L"Rejecting user specified %ls on class %ls because specified property is not primitive.", customAttributeName, ecClass.GetFullName());
                else
                    {
                    PrimitiveType ectype = key->GetAsPrimitiveProperty()->GetType();
                    if (ectype == PRIMITIVETYPE_Binary   || 
                        ectype == PRIMITIVETYPE_Boolean  || 
                        ectype == PRIMITIVETYPE_DateTime || 
                        ectype == PRIMITIVETYPE_Double   || 
                        ectype == PRIMITIVETYPE_Integer  || 
                        ectype == PRIMITIVETYPE_Long     || 
                        ectype == PRIMITIVETYPE_String)
                        {
                        Utf8String keyPropertyName;
                        BeStringUtilities::WCharToUtf8 (keyPropertyName, v.GetString());
                        StandardKeySpecificationPtr spec = StandardKeySpecification::Create(keyType);
                        spec->GetKeyProperties().push_back(keyPropertyName);
                        m_standardKeys.push_back(spec);
                        }
                    else
                        LOG.errorv(L"Rejecting user specified %ls on class %ls because specified property type not supported. Supported types are Binary, Boolean, DateTime, Double, Integer, Long and String.", customAttributeName, ecClass.GetFullName());
                    }
                }
            } 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassMapInfo::ValidateBaseClasses () const
    {
    // It is not supported that a relationship class has non-relationship base classes. 
    // It is not supported that a non-relationship class has relationship base classes.
    const bool isRelationship = (nullptr != GetECClass ().GetRelationshipClassCP ());
    for (ECClassP baseClass : GetECClass ().GetBaseClasses ())
        {
        const bool isBaseRelationship = (nullptr != baseClass->GetRelationshipClassCP ());
        if (isRelationship != isBaseRelationship)
            {
            Utf8String message;
            message.Sprintf ("All classes in an inheritance hierarchy must either be non-relationship classes or relationship classes. Mismatching base class : %s",
                                Utf8String (baseClass->GetFullName ()).c_str ());
            LogClassNotMapped (NativeLogging::LOG_WARNING, GetECClass (), message.c_str ());
            return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if all base classes have been mapped.
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassMapInfo::GatherBaseClassMaps 
(
bvector<IClassMap const*>& baseClassMaps,
bvector<IClassMap const*>& tphMaps,
bvector<IClassMap const*>& tpcMaps,
bvector<IClassMap const*>& nmhMaps,
ECClassCR          ecClass
) const
    {
    for (ECClassP baseClass : ecClass.GetBaseClasses())
        {
        auto baseClassMap = m_ecDbMap.GetClassMap (*baseClass);
        if (baseClassMap == nullptr)
            return false;

        switch (baseClassMap->GetMapStrategy())
            {
            case MapStrategy::TablePerHierarchy:
            case MapStrategy::InParentTable:
                tphMaps.push_back (baseClassMap);
                break;

            case MapStrategy::TablePerClass:
                tpcMaps.push_back (baseClassMap);
                break;

            case MapStrategy::DoNotMapHierarchy:
                nmhMaps.push_back (baseClassMap);
                break;

            default:
                // ClassMappingRule: Certain MapStrategies used in base classes have no effect on child classes: MapStrategy::DoNotMap, 
                // MapStrategy::NoHint, MapStrategy::TableForThisClass
                break;
            }
        baseClassMaps.push_back (baseClassMap);
        }

    return true;
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                          03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String ClassMapInfo::ResolveTablePrefix (ECClassCR ecClass)
    {
    ECSchemaCR schema = ecClass.GetSchema ();
    auto hint = SchemaHintReader::ReadHint (schema);
    if (hint != nullptr)
        {
        Utf8String tablePrefix;
        if (SchemaHintReader::TryReadTablePrefix (tablePrefix, *hint))
            return tablePrefix;
        }

    WStringCR namespacePrefix = schema.GetNamespacePrefix ();
    if (namespacePrefix.empty ())
        return Utf8String (schema.GetName ());
    
    return Utf8String (namespacePrefix);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ClassMapInfo::LogClassNotMapped (NativeLogging::SEVERITY severity, ECClassCR ecClass, Utf8CP explanation)
    {
    Utf8CP classTypeStr = ecClass.GetRelationshipClassCP () != nullptr ? "ECRelationshipClass" : "ECClass";
    LOG.messagev (severity, "Did not map %s '%s': %s", classTypeStr, Utf8String (ecClass.GetFullName ()).c_str (), explanation);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::ReportError_OneClassMappedByTableInHierarchyFromTwoDifferentAncestors (ECClassCR ecClass, bvector<IClassMap const*> tphMaps) const
    {
    WString message;
    message.Sprintf (L"Two or more base ECClasses (or their base classes) of %ls.%ls are using TablePerHierarchy mapping. We cannot determine which to honor. The base ECClasses are: ",
        ecClass.GetSchema ().GetName ().c_str (), ecClass.GetName ().c_str ());
    for (auto tphMap : tphMaps)
        {
        message.append (tphMap->GetClass ().GetName ().c_str ());
        message.append (L" ");
        }

    LOG.error (message.c_str ());
    return MapStatus::Error;
    }

//****************************************************************************************************
//RelationshipClassMapInfo
//****************************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapInfoPtr RelationshipClassMapInfo::Create (ECRelationshipClassCR relationshipClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, MapStrategy mapStrategy)
    {
    return new RelationshipClassMapInfo (relationshipClass, ecDbMap, tableName, primaryKeyColumnName, mapStrategy);
    }

/*---------------------------------------------------------------------------------**//**
*@bsimethod                                 Casey.Mullen                            11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 RelationshipClassMapInfo::RelationshipClassMapInfo (ECRelationshipClassCR relationshipClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, MapStrategy mapStrategy)
     : ClassMapInfo (relationshipClass, ecDbMap, tableName, primaryKeyColumnName, mapStrategy),
        m_userPreferredDirection (PreferredDirection::Unspecified), 
        m_allowDuplicateRelationships(TriState::Default)
    {

    InitializeFromSchema (); //Necessary even when loading from db as part of the info is held in the hint CA stored in the db
    DetermineCardinality ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassMapInfo::InitializeFromSchema ()
    {  
    auto relClass = GetECClass ().GetRelationshipClassCP ();
    BeAssert (relClass != nullptr);
    auto relClassHint = RelationshipClassHintReader::ReadHint (*relClass);
    if (relClassHint.IsValid())
        {

        PreferredDirection preferredDirection = PreferredDirection::Unspecified;
        if (RelationshipClassHintReader::TryReadPreferredDirection (preferredDirection, *relClassHint))
            m_userPreferredDirection = preferredDirection;

        TriState allowDuplicateRelationships = TriState::Default;
        if (RelationshipClassHintReader::TryReadAllowDuplicateRelationships (allowDuplicateRelationships, *relClassHint))
            m_allowDuplicateRelationships = allowDuplicateRelationships;

        if (m_mapStrategy != MapStrategy::SharedTableForThisClass)
            {
            RelationshipClassHintReader::TryReadSourceECInstanceIdColumnName (m_sourceECInstanceIdColumn, *relClassHint);
            RelationshipClassHintReader::TryReadSourceECClassIdColumnName (m_sourceECClassIdColumn, *relClassHint);
            RelationshipClassHintReader::TryReadTargetECInstanceIdColumnName (m_targetECInstanceIdColumn, *relClassHint);
            RelationshipClassHintReader::TryReadTargetECClassIdColumnName (m_targetECClassIdColumn, *relClassHint);
            }
        }

    RelationshipConstraintInfo::ReadFromConstraint (m_sourceInfo, relClass->GetSource ());
    RelationshipConstraintInfo::ReadFromConstraint (m_targetInfo, relClass->GetTarget ());

    //! The goal is that source/target ECid/ECClassId column should be provided in ECDbRelationshipConstraintHint and not ECDbRelationshipHint
    if (!m_sourceInfo.GetECClassIdColumn ().empty ())
        m_sourceECClassIdColumn = m_sourceInfo.GetECClassIdColumn ();

    if (!m_sourceInfo.GetECInstanceIdColumn ().empty ())
        m_sourceECInstanceIdColumn = m_sourceInfo.GetECInstanceIdColumn ();

    if (!m_targetInfo.GetECClassIdColumn ().empty ())
        m_targetECClassIdColumn = m_targetInfo.GetECClassIdColumn ();

    if (!m_targetInfo.GetECInstanceIdColumn ().empty ())
        m_targetECInstanceIdColumn = m_targetInfo.GetECInstanceIdColumn ();
    }
    
//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassMapInfo::_EvaluateMapStrategy ()
    {
    auto stat = ClassMapInfo::_EvaluateMapStrategy ();
    if (stat != MapStatus::Success)
        return stat;

    BeAssert (m_mapStrategy != MapStrategy::NoHint);

    if (IClassMap::IsDoNotMapStrategy (m_mapStrategy))
        return MapStatus::Success;

    return DetermineRelationshipMapStrategy ();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipClassMapInfo::DetermineRelationshipMapStrategy ()
    {
    auto isValid = ValidateRelatedClasses ();
    if (!isValid)
        {
        m_mapStrategy = MapStrategy::DoNotMap;
        return MapStatus::Success;
        }

    if (m_mapStrategy == MapStrategy::SharedTableForThisClass)
        return MapStatus::Success;

    //Note: At this point, MapStrategy can only be one of -
    //MapStrategy::RelationshipSourceTable, MapStrategy::RelationshipTargetTable
    ECRelationshipClassCP relationshipClass = GetECClass ().GetRelationshipClassCP ();
    ECRelationshipConstraintR source = relationshipClass->GetSource ();
    ECRelationshipConstraintR target = relationshipClass->GetTarget ();

    DetermineCardinality (); 
    if (GetCardinality () == CardinalityType::ManyToMany || relationshipClass->GetPropertyCount () > 0)
        m_mapStrategy = MapStrategy::TableForThisClass;
    else if (GetCardinality () == CardinalityType::OneToMany || GetCardinality () == CardinalityType::ManyToOne)
        m_mapStrategy = Get1MRelationshipMapStrategy (source, target, *relationshipClass);
    else
        m_mapStrategy = Get11RelationshipMapStrategy (source, target, *relationshipClass);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                06/2014
//+---------------+---------------+---------------+---------------+---------------+------
void RelationshipClassMapInfo::DetermineCardinality ()
    {
    ECRelationshipClassCP relationshipClass = GetECClass ().GetRelationshipClassCP ();
    ECRelationshipConstraintR source = relationshipClass->GetSource ();
    ECRelationshipConstraintR target = relationshipClass->GetTarget ();

    m_relationshipCardinality = CalculateRelationshipCardinality (source, target);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                08 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
bool RelationshipClassMapInfo::ValidateRelatedClasses () const
    {
    ECRelationshipClassCP relationshipClass = GetECClass ().GetRelationshipClassCP ();
    ECRelationshipConstraintR source = relationshipClass->GetSource ();
    ECRelationshipConstraintR target = relationshipClass->GetTarget ();

    // ClassMappingRule: a relationship with either or both ends having no ECClass is an abstract relationship, and will not be mapped
    auto sourceEndClasses = m_ecDbMap.GetClassesFromRelationshipEnd (source);
    auto targetEndClasses = m_ecDbMap.GetClassesFromRelationshipEnd (target);
    if (sourceEndClasses.size () == 0 || targetEndClasses.size () == 0)
        {
        LogClassNotMapped (NativeLogging::LOG_INFO, GetECClass (), 
            "ECClass is an abstract ECRelationship class, i.e. one of the constraints is empty.");
        return false;
        }

    // ClassMappingRule: cannot map relationships that relate other relationships
    if (ContainsRelationshipClass (sourceEndClasses) || ContainsRelationshipClass (targetEndClasses))
        {
        LogClassNotMapped (NativeLogging::LOG_WARNING, GetECClass (), 
            "The source or target constraint contain a relationship class which is not supported.");
        return false;
        }

    // ClassMappingRule: If the ends of the relationship are not mapped, don't map the relationship
    size_t nSourceTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, source);
    size_t nTargetTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, target);
    if (0 == nSourceTables || 0 == nTargetTables)
        {
        LogClassNotMapped (NativeLogging::LOG_WARNING, GetECClass (),
            "Source or target constraint classes are not mapped.");

        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                08 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
bool RelationshipClassMapInfo::ContainsRelationshipClass (std::vector<ECClassCP> const& endClasses)
    {
    for (ECClassCP endClass : endClasses)
        {
        if (nullptr != endClass->GetRelationshipClassCP ())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
RelationshipClassMapInfo::CardinalityType RelationshipClassMapInfo::CalculateRelationshipCardinality (ECRelationshipConstraintCR source, ECRelationshipConstraintCR target)
    {
    bool sourceIsM = source.GetCardinality().GetUpperLimit() > 1;
    bool targetIsM = target.GetCardinality().GetUpperLimit() > 1;

    CardinalityType relationshipCardinality;
    if (sourceIsM && targetIsM)
        relationshipCardinality = CardinalityType::ManyToMany; 
    else if (!sourceIsM && targetIsM)
        relationshipCardinality = CardinalityType::OneToMany;
    else if (sourceIsM && !targetIsM)
        relationshipCardinality = CardinalityType::ManyToOne;
    else
        relationshipCardinality = CardinalityType::OneToOne;
   
    return relationshipCardinality;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStrategy RelationshipClassMapInfo::Get11RelationshipMapStrategy
(
ECRelationshipConstraintR source, 
ECRelationshipConstraintR target, 
ECRelationshipClassCR relationshipClass
) const
    {
    /* Note: At this point of the algorithm, map strategy can only be MapStrategy::RelationshipSourceTable, 
     * or MapStrategy::RelationshipTargetTable */

    // Check if we need a link table to persist the relationship
    size_t nSourceTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, source);
    size_t nTargetTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, target);

    BeAssert (0 != nSourceTables && "Condition should have been caught earlier, and strategy set to DoNotMap");
    BeAssert (0 != nTargetTables && "Condition should have been caught earlier, and strategy set to DoNotMap");

    if ((nSourceTables > 1 && m_mapStrategy == MapStrategy::RelationshipSourceTable) ||
        (nTargetTables > 1 && m_mapStrategy == MapStrategy::RelationshipTargetTable))
        {
        // We don't persist at an end that has more than one table
        LOG.warningv (L"Cannot map ECRelationshipClass %ls to end table as more than one end table exists on either end. Mapping to link table instead.", 
                relationshipClass.GetFullName());
        return MapStrategy::TableForThisClass;
        }

    // Don't persist at an end that has more than one table
    if (nSourceTables > 1 && nTargetTables > 1)
        {
        return MapStrategy::TableForThisClass;
        }

    if (nSourceTables == 1 && nTargetTables == 1)
        {
        /* Can persist in either end - use some heuristics */

        // Allow user his choice if he made one
        if (m_mapStrategy == MapStrategy::RelationshipSourceTable || m_mapStrategy == MapStrategy::RelationshipTargetTable)
            return m_mapStrategy;
      
        // PreferredDirection hint, indicates optimal traversal efficiency. 
        if (m_userPreferredDirection == PreferredDirection::SourceToTarget)
            return MapStrategy::RelationshipSourceTable;
        else if (m_userPreferredDirection == PreferredDirection::TargetToSource)
            return MapStrategy::RelationshipTargetTable;

        // Pick the End at the start of the strength direction
        if (relationshipClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward)
            return MapStrategy::RelationshipSourceTable;
        else
            return MapStrategy::RelationshipTargetTable;
        }

    // Persist at the end that has 1 table
    return (nSourceTables == 1) ? MapStrategy::RelationshipSourceTable : MapStrategy::RelationshipTargetTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStrategy RelationshipClassMapInfo::Get1MRelationshipMapStrategy
(
ECRelationshipConstraintR source, 
ECRelationshipConstraintR target, 
ECRelationshipClassCR relationshipClass
) const
    {
    // Follow override other. Since if relation map to Source/Target table it cannot have duplicate values there for
    // we forces it to use a link table for this relationship so duplicate relationships can be allowed.
    if (GetAllowDuplicateRelationships() == TriState::True)
        return MapStrategy::TableForThisClass;

    // Pick the M(any) end or the middle depending on the number of tables
    size_t nManyEndTables;
    if (m_relationshipCardinality == CardinalityType::ManyToOne)
        {
        nManyEndTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, source);
        if (nManyEndTables > 1)
            return MapStrategy::TableForThisClass;
        else
            return MapStrategy::RelationshipSourceTable;
        }
    else
        {
        nManyEndTables = m_ecDbMap.GetTablesFromRelationshipEnd (nullptr, target);
        BeAssert (target.GetCardinality().GetUpperLimit() > 1);
        if (nManyEndTables > 1)
            return MapStrategy::TableForThisClass;
        else
            return MapStrategy::RelationshipTargetTable;
        }
    }



//static
void RelationshipConstraintInfo::ReadFromConstraint (RelationshipConstraintInfo& info, ECRelationshipConstraintCR constraint)
    {
    info.m_ecClassIdColumn.clear ();
    info.m_ecInstanceIdColumn.clear ();
    info.m_enforceIntegrityCheck = false;
    info.m_onDeleteAction = ECDbSqlForiegnKeyConstraint::ActionType::NotSpecified;
    info.m_onUpdateAction = ECDbSqlForiegnKeyConstraint::ActionType::NotSpecified;
    info.m_matchType = ECDbSqlForiegnKeyConstraint::MatchType::NotSpecified;
    info.m_generateDefaultIndex = true;
    info.m_isEmpty = false;

    auto hint = constraint.GetCustomAttribute (L"ECDbRelationshipConstraintHint");
    if (hint.IsNull ())
        return;

    WString xml;
    hint->WriteToXmlString (xml, false, false);
    const WCharCP kECClassIdColumn = L"ECClassIdColumn";
    const WCharCP kECIdColumn = L"ECIdColumn";
    const WCharCP kGenerateDefaultIndex = L"GenerateDefaultIndex";
    const WCharCP kEnforceReferentialIntegrityCheck = L"ForeignKeyConstraint.EnforceReferentialIntegrityCheck";
    const WCharCP kOnDeleteAction = L"ForeignKeyConstraint.OnDeleteAction";
    const WCharCP kOnUpdateAction = L"ForeignKeyConstraint.OnUpdateAction";
    const WCharCP kMatchType = L"ForeignKeyConstraint.MatchType";


    ECValue v;
    if (hint->GetValue (v, kECClassIdColumn) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
        info.m_ecClassIdColumn = v.GetUtf8CP ();

    if (hint->GetValue (v, kECIdColumn) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
        info.m_ecInstanceIdColumn = v.GetUtf8CP ();

    if (hint->GetValue (v, kGenerateDefaultIndex) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
        info.m_generateDefaultIndex = v.GetBoolean();

    if (hint->GetValue (v, kEnforceReferentialIntegrityCheck) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
        info.m_enforceIntegrityCheck = v.GetBoolean ();

    if (info.DoEnforceIntegrityCheck ())
        {
        if (hint->GetValue (v, kOnDeleteAction) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
            info.m_onDeleteAction = ECDbSqlForiegnKeyConstraint::ParseActionType (v.GetString ());

        if (hint->GetValue (v, kOnUpdateAction) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
            info.m_onUpdateAction = ECDbSqlForiegnKeyConstraint::ParseActionType (v.GetString ());

        if (hint->GetValue (v, kMatchType) == ECObjectsStatus::ECOBJECTS_STATUS_Success && !v.IsNull ())
            info.m_matchType = ECDbSqlForiegnKeyConstraint::ParseMatchType (v.GetString ());
        }
    }
   
END_BENTLEY_SQLITE_EC_NAMESPACE
