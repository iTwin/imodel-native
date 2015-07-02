/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
std::shared_ptr<ClassMapInfo> ClassMapInfoFactory::Create(MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecDbMap)
    {
    auto info = Create(mapStatus, ecClass, ecDbMap);
    if (info == nullptr)
        return nullptr;

    if (!info->GetMapStrategy ().IsNotMapped())
        {
        auto validationResult = schemaImportContext.GetSchemaValidationResult (ecClass.GetSchema ());
        if (validationResult != nullptr)
            {
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
                                info->GetMapStrategyR ().Assign(MapStrategy::NotMapped, false);
                                schemaImportContext.GetIssueListener ().Report (ECDbSchemaManager::IImportIssueListener::Severity::Warning,
                                                                                "Did not map ECRelationshipClass '%s': ECRelationshipClasses for which names only differ by case are not supported by ECDb.", Utf8String (ecClass.GetFullName ()).c_str ());
                                }
                            }
                        }
                    }
                }
            }
        else
            {
            LOG.fatalv (L"Programmer error: No schema validation result found for ECSchema %ls", ecClass.GetSchema ().GetName ().c_str ());
            }
        }

    return info;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::shared_ptr<ClassMapInfo> ClassMapInfoFactory::Create(MapStatus& stat, ECClassCR ecClass, ECDbMapCR ecDbMap)
    {
    std::shared_ptr<ClassMapInfo> info = nullptr;
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP ();
    if (ecRelationshipClass != nullptr)
        info = std::make_shared<RelationshipMapInfo>(*ecRelationshipClass, ecDbMap);
    else
        info = std::make_shared<ClassMapInfo>(ecClass, ecDbMap);

    if (info == nullptr || (stat = info->_Initialize()) != MapStatus::Success)
        return nullptr;

    return info;
    }



//**********************************************************************************************
// ClassMapInfo
//**********************************************************************************************
//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapInfo::ClassMapInfo (ECClassCR ecClass, ECDbMapCR ecDbMap)
    : m_ecdbMap(ecDbMap), m_ecClass(ecClass), m_isMapToVirtualTable(IClassMap::IsAbstractECClass(ecClass)), m_classHasCurrentTimeStampProperty(nullptr), m_parentClassMap(nullptr)
    {}

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus ClassMapInfo::_Initialize()
    {
    //  if (Utf8String::IsNullOrEmpty (tableName))
    if (SUCCESS != _InitializeFromSchema())
        return MapStatus::Error;

    //Default values for table name and primary key column name
    if (m_tableName.empty())
        {
        // ClassMappingRule: if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        m_tableName = ResolveTablePrefix(m_ecClass);
        if (!IClassMap::IsMapToSecondaryTableStrategy(m_ecClass))  // ClassMappingRule: assumes that structs are always used in arrays
            m_tableName.append("_");
        else
            m_tableName.append("_ArrayOf");

        m_tableName.append(Utf8String(m_ecClass.GetName()));
        }

    if (m_ecInstanceIdColumnName.empty())
        {
        // ClassMappingRule: if hint does not supply an ECInstanceId (primary key) column name, use ECDB_COL_ECInstanceId
        m_ecInstanceIdColumnName = ECDB_COL_ECInstanceId;
        }

    return _EvaluateMapStrategy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::_EvaluateMapStrategy()
    {
    ECDbMapStrategy& mapStrategy = GetMapStrategyR();
    MapStrategy strategy = mapStrategy.GetStrategy();

    if (mapStrategy.IsNotMapped())
        return MapStatus::Success;

    if (strategy == MapStrategy::None)
        mapStrategy.Assign(MapStrategy::Default, false);

    ECClassCR ecClass = GetECClass ();
    if (ecClass.GetIsCustomAttributeClass () && !ecClass.GetIsDomainClass () && !ecClass.GetIsStruct ())
        {
        LogClassNotMapped (NativeLogging::LOG_DEBUG, ecClass, "ECClass is a custom attribute which is never mapped to a table in ECDb.");
        mapStrategy.Assign(MapStrategy::NotMapped, false);
        return MapStatus::Success;
        }

    if (IClassMap::IsAnyClass (ecClass) || (ecClass.GetSchema ().IsStandardSchema () && ecClass.GetName ().CompareTo (L"InstanceCount") == 0))
        {
        LogClassNotMapped (NativeLogging::LOG_INFO, ecClass, "ECClass is a standard class not supported by ECDb.");
        mapStrategy.Assign(MapStrategy::NotMapped, false);
        return MapStatus::Success;
        }

    // ClassMappingRule: Classes within the same hierarchy should all be relationships or non-relationships
    bool isValid = ValidateBaseClasses ();
    if (!isValid)
        {
        mapStrategy.Assign(MapStrategy::NotMapped, false);
        return MapStatus::Success;
        }

    //! We override m_isMapToVirtualTable if tablePerHierarchy Or shareTableStrategy was used.
    if (m_isMapToVirtualTable)
        {
        m_isMapToVirtualTable = strategy != MapStrategy::SharedTable;
        }

    return EvaluateInheritedMapStrategy ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::EvaluateInheritedMapStrategy ()
    {
    bvector<IClassMap const*> baseClassMaps;
    bvector<IClassMap const*> tphMaps; // SharedTable-Polymorphic have the highest priority, but there can be only one
    bvector<IClassMap const*> tpcMaps; // OwnTable have second priority
    bvector<IClassMap const*> nmhMaps; // NotMapped-Polymorphic has priority only over NoHint or NotMapped-NonPolymorphic

    bool areBaseClassesMapped = GatherBaseClassMaps (baseClassMaps, tphMaps, tpcMaps, nmhMaps, m_ecClass);
    if (!areBaseClassesMapped)
        return MapStatus::BaseClassesNotMapped;

    if (baseClassMaps.empty())
        {
        BeAssert(tphMaps.empty() && tpcMaps.empty() && nmhMaps.empty());
        return MapStatus::Success;
        }

    // ClassMappingRule: No more than one ancestor of a class can use SharedTable-Polymorphic strategy. Mapping fails if this is violated
    if (tphMaps.size () > 1)
        return ReportError_OneClassMappedByTableInHierarchyFromTwoDifferentAncestors (m_ecClass, tphMaps);

    // ClassMappingRule: If exactly 1 ancestor ECClass is using SharedTable-Polymorphic, use this
    if (tphMaps.size() == 1)
        {
        m_parentClassMap = tphMaps[0];
        ECDbMapStrategy const& parentMapStrategy = m_parentClassMap->GetMapStrategy();
        BeAssert(parentMapStrategy.IsPolymorphicSharedTable());

        MapStrategyOptions options = MapStrategyOptions::None;
        if (GetMapStrategy().HasOption(MapStrategyOptions::DisableSharedColumns))
            options = MapStrategyOptions::DisableSharedColumns;
        else if (parentMapStrategy.HasOption(MapStrategyOptions::SharedColumns) || parentMapStrategy.HasOption(MapStrategyOptions::SharedColumnsForSubclasses))
            options = MapStrategyOptions::SharedColumns;

        if (SUCCESS != GetMapStrategyR().Assign(MapStrategy::SharedTable, options, true))
            return MapStatus::Error;

        if (GetECClass ().GetIsStruct () && !m_parentClassMap->GetClass().GetIsStruct())
            {
            WString msg;
            msg.append (L"Struct classes cannot be included in 'SharedTable' that has a non-struct class as its root. Struct Class [").append (GetECClass ().GetFullName ()).append (L"] will be mapped to its own table.");
            LOG.warning (msg.c_str ());
            m_strategy.Assign (MapStrategy::OwnTable, false);
            m_parentClassMap = nullptr;
            }
        else if (!GetECClass ().GetIsStruct () && m_parentClassMap->GetClass ().GetIsStruct ())
            {
            WString msg;
            msg.append (L"Regular classes cannot be included in 'SharedTable' that has a struct class as its root. Class [").append (GetECClass ().GetFullName ()).append (L"] will be mapped to its own table.");
            LOG.warning (msg.c_str ());
            m_strategy.Assign(MapStrategy::OwnTable, false);
            m_parentClassMap = nullptr;
            }
        }

    if (GetMapStrategy().IsPolymorphicSharedTable())
        return MapStatus::Success;

    // ClassMappingRule: If one or more parent is using OwnClass-polymorphic, use OwnClass-polymorphic mapping
    else if (tpcMaps.size () > 0)
        GetMapStrategyR ().Assign(MapStrategy::OwnTable, true);
    // ClassMappingRule: If one or more parent is using NotMapped-polymorphic, use NotMapped-polymorphic
    else if (nmhMaps.size () > 0)
        GetMapStrategyR ().Assign(MapStrategy::NotMapped, true);

    return MapStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMapInfo::_InitializeFromSchema ()
    {
    if (SUCCESS != InitializeFromClassMapCA() || SUCCESS != InitializeFromClassHasCurrentTimeStampProperty())
        return ERROR;

    // Add indices for important identifiers
    if (SUCCESS != ProcessStandardKeys(m_ecClass, L"BusinessKeySpecification") ||
        SUCCESS != ProcessStandardKeys(m_ecClass, L"GlobalIdSpecification") ||
        SUCCESS != ProcessStandardKeys(m_ecClass, L"SyncIDSpecification"))
        return ERROR;
    
    //TODO: VerifyThatTableNameIsNotReservedName, e.g. dgn element table, etc.
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 muhammad.zaighum                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapInfo::InitializeFromClassHasCurrentTimeStampProperty()
    {
    IECInstancePtr classHint = m_ecClass.GetCustomAttributeLocal(L"ClassHasCurrentTimeStampProperty");

    if (classHint == nullptr)
        return SUCCESS;
   
    WString propertyName;
    ECValue v;
    if (classHint->GetValue(v, L"PropertyName") == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        propertyName = v.GetString();
        ECPropertyCP dateTimeProperty = m_ecClass.GetPropertyP(propertyName);
        if (nullptr == dateTimeProperty)
            {
            BeAssert(false && "ClassHasCurrentTimeStamp Property Not Found in ECClass");
            return ERROR;
            }
        if (dateTimeProperty->GetTypeName().Equals(L"dateTime"))
            {
            m_classHasCurrentTimeStampProperty = dateTimeProperty;
            }
        else
            {
            BeAssert(false && "Property is not of type dateTime");
            return ERROR;
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                03/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapInfo::InitializeFromClassMapCA()
    {
    ECDbClassMap customClassMap;
    if (!ECDbMapCustomAttributeHelper::TryGetClassMap(customClassMap, m_ecClass))
        return SUCCESS;

    ECDbClassMap::MapStrategy mapStrategyCA;
    ECObjectsStatus ecstat = customClassMap.TryGetMapStrategy(mapStrategyCA);
    if (ECOBJECTS_STATUS_Success != ecstat)
        return ERROR;

    if (ECDbMapStrategy::TryParse(m_strategy, mapStrategyCA) != SUCCESS)
        {
        LOG.errorv(L"Invalid MapStrategy and/or MapStrategyOptions specified in ClassMap custom attribute on ECClass %ls.",
                    m_ecClass.GetFullName());
        return ERROR;
        }

    if (m_strategy.GetStrategy() == MapStrategy::SharedTable)
        m_isMapToVirtualTable = false;

    ecstat = customClassMap.TryGetTableName(m_tableName);
    if (ECOBJECTS_STATUS_Success != ecstat)
        return ERROR;

    if (m_tableName.empty() && (m_strategy.GetStrategy() == MapStrategy::ExistingTable || 
                                (m_strategy.GetStrategy() == MapStrategy::SharedTable && !m_strategy.IsPolymorphic())))
        {
        LOG.errorv(L"TableName must not be empty in ClassMap custom attribute on ECClass %ls if MapStrategy is 'SharedTable' and IsMapStrategyPolymorphic is false or if MapStrategy is 'ExistingTable'.",
                   m_ecClass.GetFullName());
        return ERROR;
        }

    ecstat = customClassMap.TryGetECInstanceIdColumn(m_ecInstanceIdColumnName);
    if (ECOBJECTS_STATUS_Success != ecstat)
        return ERROR;

    bvector<ECDbClassMap::DbIndex> indices;
    ecstat = customClassMap.TryGetIndexes(indices);
    if (ECOBJECTS_STATUS_Success != ecstat)
        return ERROR;

    for (ECDbClassMap::DbIndex const& index : indices)
        {
        ClassIndexInfoPtr indexInfo = ClassIndexInfo::Create(index);
        if (indexInfo == nullptr)
            return ERROR;

        m_dbIndexes.push_back(indexInfo);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMapInfo::ProcessStandardKeys (ECClassCR ecClass, WCharCP customAttributeName)
    {
    StandardKeySpecification::Type keyType = StandardKeySpecification::GetTypeFromString(customAttributeName);
    if (keyType == StandardKeySpecification::Type::None)
        return SUCCESS;

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
            if (nullptr == (key = ecClass.GetPropertyP(v.GetString())))
                {
                LOG.errorv(L"Rejecting user specified %ls on class %ls because property specified in it '%ls' doesn't exist in class", customAttributeName, ecClass.GetFullName(), v.GetString());
                return ERROR;
                }
            else
                {
                if (key->GetAsPrimitiveProperty() == nullptr)
                    {
                    LOG.errorv(L"Rejecting user specified %ls on class %ls because specified property is not primitive.", customAttributeName, ecClass.GetFullName());
                    return ERROR;
                    }
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
                        {
                        LOG.errorv(L"Rejecting user specified %ls on class %ls because specified property type not supported. Supported types are Binary, Boolean, DateTime, Double, Integer, Long and String.", customAttributeName, ecClass.GetFullName());
                        return ERROR;
                        }
                    }
                }
            } 
        }

    return SUCCESS;
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
        auto baseClassMap = m_ecdbMap.GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            return false;

        ECDbMapStrategy const& baseMapStrategy = baseClassMap->GetMapStrategy();
        if (!baseMapStrategy.IsPolymorphic())
            {
            // ClassMappingRule: non-polymorphic MapStrategies used in base classes have no effect on child classes
            return true;
            }

        switch (baseMapStrategy.GetStrategy())
            {
                case MapStrategy::SharedTable:
                    {

                    auto add = true;
                    for (auto classMap : tphMaps)
                        {
                        if (classMap->GetTable().GetId() == baseClassMap->GetTable().GetId())
                            {
                            add = false;
                            break;
                            }
                        }

                    if (add)
                        tphMaps.push_back(baseClassMap);
                    break;
                    }

                case MapStrategy::OwnTable:
                    tpcMaps.push_back(baseClassMap);
                    break;

                case MapStrategy::NotMapped:
                    nmhMaps.push_back(baseClassMap);
                    break;

                default:
                    BeAssert(false);
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
    ECDbSchemaMap customSchemaMap;
    if (ECDbMapCustomAttributeHelper::TryGetSchemaMap (customSchemaMap, schema))
        {
        Utf8String tablePrefix;
        if (customSchemaMap.TryGetTablePrefix(tablePrefix) == ECOBJECTS_STATUS_Success && !tablePrefix.empty ())
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus RelationshipMapInfo::_Initialize()
    {
    MapStatus stat = ClassMapInfo::_Initialize();
    if (stat != MapStatus::Success)
        return stat;

    DetermineCardinality();
    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipMapInfo::_InitializeFromSchema()
    {  
    if (SUCCESS != ClassMapInfo::_InitializeFromSchema())
        return ERROR;

    auto relClass = GetECClass ().GetRelationshipClassCP ();
    BeAssert (relClass != nullptr);

    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    const bool hasForeignKeyRelMap = ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap(foreignKeyRelMap, *relClass);
    ECDbLinkTableRelationshipMap linkTableRelationMap;
    const bool hasLinkTableRelMap = ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationMap, *relClass);

    if (hasForeignKeyRelMap && hasLinkTableRelMap)
        {
        LOG.errorv(L"ECRelationshipClass '%ls' can only have either the ForeignKeyRelationshipMap or the LinkTableRelationshipMap custom attribute but not both.",
                   GetECClass().GetFullName());
        return ERROR;
        }

    if (hasForeignKeyRelMap)
        {
        ECRelationshipEnd foreignKeyEnd = ECRelationshipEnd_Target;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetEnd(foreignKeyEnd))
            return ERROR;

        RelationshipEndColumns* foreignKeyColumnsMapping = nullptr;
        ECRelationshipConstraintCP foreignKeyConstraint = nullptr;
        if (foreignKeyEnd == ECRelationshipEnd_Target)
            {
            foreignKeyConstraint = &relClass->GetTarget();
            foreignKeyColumnsMapping = &m_targetColumnsMapping;
            m_sourceColumnsMappingIsNull = true;
            m_targetColumnsMappingIsNull = false;
            m_customMapType = RelationshipMapInfo::CustomMapType::ForeignKeyOnTarget;
            }
        else
            {
            foreignKeyConstraint = &relClass->GetSource();
            foreignKeyColumnsMapping = &m_sourceColumnsMapping;
            m_sourceColumnsMappingIsNull = false;
            m_targetColumnsMappingIsNull = true;
            m_customMapType = RelationshipMapInfo::CustomMapType::ForeignKeyOnSource;
            }

        Utf8String foreignKeyColName;
        Utf8String foreignKeyClassIdColName;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetForeignKeyColumn(foreignKeyColName))
            return ERROR;

        if (!foreignKeyColName.empty())
            {
            for (ECRelationshipConstraintClassCP constraintClass : foreignKeyConstraint->GetConstraintClasses())
                {
                if (!constraintClass->GetKeys().empty())
                    {
                    LOG.errorv(L"ForeignKeyRelationshipMap custom attribute on ECRelationshipClass '%ls' must not have a value for ForeignKeyProperty as there are Key properties defined in the ECRelationshipConstraint on the foreign key end.",
                               GetECClass().GetFullName());
                    return ERROR;
                    }
                }
            }

        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetForeignKeyClassIdColumn(foreignKeyClassIdColName))
            return ERROR;

        *foreignKeyColumnsMapping = RelationshipEndColumns(foreignKeyColName.c_str(), foreignKeyClassIdColName.c_str());

        bool createConstraint = false;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetCreateConstraint(createConstraint))
            return ERROR;

        if (createConstraint)
            {
            Utf8String onDeleteActionStr;
            if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnDeleteAction(onDeleteActionStr))
                return ERROR;

            Utf8String onUpdateActionStr;
            if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnUpdateAction(onUpdateActionStr))
                return ERROR;

            m_onDeleteAction = ECDbSqlForeignKeyConstraint::ToActionType(onDeleteActionStr.c_str());
            m_onUpdateAction = ECDbSqlForeignKeyConstraint::ToActionType(onUpdateActionStr.c_str());
            }

        return SUCCESS;
        }
    
    if (hasLinkTableRelMap)
        {
        if (ECOBJECTS_STATUS_Success != linkTableRelationMap.TryGetAllowDuplicateRelationships(m_allowDuplicateRelationships))
            return ERROR;

        Utf8String sourceIdColName;
        if (ECOBJECTS_STATUS_Success != linkTableRelationMap.TryGetSourceECInstanceIdColumn(sourceIdColName))
            return ERROR;

        Utf8String sourceClassIdColName;
        if (ECOBJECTS_STATUS_Success != linkTableRelationMap.TryGetSourceECClassIdColumn(sourceClassIdColName))
            return ERROR;

        Utf8String targetIdColName;
        if (ECOBJECTS_STATUS_Success != linkTableRelationMap.TryGetTargetECInstanceIdColumn(targetIdColName))
            return ERROR;

        Utf8String targetClassIdColName;
        if (ECOBJECTS_STATUS_Success != linkTableRelationMap.TryGetTargetECClassIdColumn(targetClassIdColName))
            return ERROR;

        m_sourceColumnsMappingIsNull = false;
        m_sourceColumnsMapping = RelationshipEndColumns(sourceIdColName.c_str(), sourceClassIdColName.c_str());
        m_targetColumnsMappingIsNull = false;
        m_targetColumnsMapping = RelationshipEndColumns(targetIdColName.c_str(), targetClassIdColName.c_str());
        m_customMapType = RelationshipMapInfo::CustomMapType::LinkTable;
        }

    return SUCCESS;
    }
    
//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipMapInfo::_EvaluateMapStrategy()
    {
    bool alreadyEvaluated = false;
    MapStatus stat = DetermineImpliedMapStrategy(alreadyEvaluated);
    if (stat != MapStatus::Success)
        return stat;

    return EvaluateCustomMapping(alreadyEvaluated);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipMapInfo::DetermineImpliedMapStrategy(bool& mapStrategyAlreadyEvaluated)
    {
    //early return means evaluation was successful. Other cases set the value explicitly
    mapStrategyAlreadyEvaluated = true;

    auto stat = ClassMapInfo::_EvaluateMapStrategy();
    if (stat != MapStatus::Success)
        return stat;

    BeAssert(GetMapStrategy().GetStrategy() != MapStrategy::None);

    if (GetMapStrategy().IsNotMapped())
        return MapStatus::Success;

    DetermineCardinality();

    const bool isValid = VerifyRelatedClasses();
    if (!isValid)
        {
        GetMapStrategyR().Assign(MapStrategy::NotMapped, false);
        return MapStatus::Success;
        }

    if (GetMapStrategy().IsPolymorphicSharedTable())
        return MapStatus::Success;

    if (auto parentClassMap = GetParentClassMap())
        {
        if (parentClassMap->GetMapStrategy().IsPolymorphicSharedTable())
            {
            GetMapStrategyR().Assign(MapStrategy::SharedTable, true);
            return MapStatus::Success;
            }
        }

    //Note: At this point, MapStrategy can only be one of -
    //MapStrategy::RelationshipSourceTable, MapStrategy::RelationshipTargetTable
    ECRelationshipClassCP relationshipClass = GetECClass().GetRelationshipClassCP();
    ECRelationshipConstraintR source = relationshipClass->GetSource();
    ECRelationshipConstraintR target = relationshipClass->GetTarget();

    if (m_cardinality == Cardinality::ManyToMany ||
        relationshipClass->GetPropertyCount() > 0 ||
        // Follow override other. Since if relation map to Source/Target table it cannot have duplicate values there for
        // we forces it to use a link table for this relationship so duplicate relationships can be allowed.
        m_allowDuplicateRelationships)
        {
        if (SUCCESS != GetMapStrategyR().Assign(MapStrategy::OwnTable, false))
            return MapStatus::Error;
        }
    else if (m_cardinality == Cardinality::OneToMany || m_cardinality == Cardinality::ManyToOne)
        {
        MapStrategy strategy;
        if (TryDetermine1MRelationshipMapStrategy(strategy, source, target, *relationshipClass))
            {
            if (SUCCESS != GetMapStrategyR().Assign(strategy, false))
                return MapStatus::Error;
            }
        else
            mapStrategyAlreadyEvaluated = false;
        }
    else
        {
        BeAssert(m_cardinality == Cardinality::OneToOne);
        MapStrategy strategy;
        if (TryDetermine11RelationshipMapStrategy(strategy, source, target, *relationshipClass))
            {
            if (SUCCESS != GetMapStrategyR().Assign(strategy, false))
                return MapStatus::Error;
            }
        else
            mapStrategyAlreadyEvaluated = false;
        }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               06/2015
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus RelationshipMapInfo::EvaluateCustomMapping(bool mapStrategyAlreadyEvaluated)
    {
    if (!mapStrategyAlreadyEvaluated)
        {
        if (m_cardinality != Cardinality::OneToOne)
            {
            BeAssert(m_cardinality == Cardinality::OneToOne && "This is only the case for cardinality 1:1");
            return MapStatus::Error;
            }

        ECDbMapStrategy& mapStrategy = GetMapStrategyR();
        switch (m_customMapType)
            {
                case CustomMapType::ForeignKeyOnSource:
                    mapStrategy.Assign(MapStrategy::ForeignKeyRelationshipInSourceTable, false);
                    break;

                case CustomMapType::ForeignKeyOnTarget:
                    mapStrategy.Assign(MapStrategy::ForeignKeyRelationshipInTargetTable, false);
                    break;

                case CustomMapType::LinkTable:
                    mapStrategy.Assign(MapStrategy::OwnTable, false);
                    break;

                default:
                case CustomMapType::None:
                    mapStrategy.Assign(MapStrategy::ForeignKeyRelationshipInTargetTable, false);
                    break;
            }

        return MapStatus::Success;
        }

    if (m_customMapType == CustomMapType::None)
        return MapStatus::Success;

    //now verify that implied MapStrategy matches the custom mapping
    if (GetMapStrategy().GetStrategy () == MapStrategy::ForeignKeyRelationshipInSourceTable)
        {
        if (m_customMapType != CustomMapType::ForeignKeyOnSource)
            {
            LOG.errorv(L"The ECRelationshipClass '%ls' implies a foreign key relationship with the foreign key on the source side. The class however has a mismatching RelationshipMap custom attribute.",
                       GetECClass().GetFullName());
            return MapStatus::Error;
            }

        return MapStatus::Success;
        }

    if (GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable)
        {
        if (m_customMapType != CustomMapType::ForeignKeyOnTarget)
            {
            LOG.errorv(L"The ECRelationshipClass '%ls' implies a foreign key relationship with the foreign key on the target side. The class however has a mismatching RelationshipMap custom attribute.",
                       GetECClass().GetFullName());
            return MapStatus::Error;
            }

        return MapStatus::Success;
        }

    //any other map strategy requires a link table, so both 
    if (m_customMapType != CustomMapType::LinkTable)
        {
        LOG.errorv(L"The ECRelationshipClass '%ls' implies a link table relationship. The class however has a ForeignKeyRelationshipMap custom attribute.",
                   GetECClass().GetFullName());
        return MapStatus::Error;
        }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                06/2014
//+---------------+---------------+---------------+---------------+---------------+------
void RelationshipMapInfo::DetermineCardinality ()
    {
    ECRelationshipClassCP relationshipClass = GetECClass ().GetRelationshipClassCP ();
    ECRelationshipConstraintR source = relationshipClass->GetSource ();
    ECRelationshipConstraintR target = relationshipClass->GetTarget ();

    bool sourceIsM = source.GetCardinality().GetUpperLimit() > 1;
    bool targetIsM = target.GetCardinality().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        m_cardinality = Cardinality::ManyToMany;
    else if (!sourceIsM && targetIsM)
        m_cardinality = Cardinality::OneToMany;
    else if (sourceIsM && !targetIsM)
        m_cardinality = Cardinality::ManyToOne;
    else
        m_cardinality = Cardinality::OneToOne;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                08 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
bool RelationshipMapInfo::VerifyRelatedClasses() const
    {
    BeAssert(GetECClass().GetRelationshipClassCP() != nullptr);
    ECRelationshipClassCR relationshipClass = *GetECClass().GetRelationshipClassCP();
    ECRelationshipConstraintR source = relationshipClass.GetSource ();
    ECRelationshipConstraintR target = relationshipClass.GetTarget ();

    // ClassMappingRule: a relationship with either or both ends having no ECClass is an abstract relationship, and will not be mapped
    auto sourceEndClasses = m_ecdbMap.GetClassesFromRelationshipEnd(source);
    auto targetEndClasses = m_ecdbMap.GetClassesFromRelationshipEnd(target);

    // ClassMappingRule: cannot map relationships that relate other relationships
    if (ContainsRelationshipClass (sourceEndClasses) || ContainsRelationshipClass (targetEndClasses))
        {
        LogClassNotMapped (NativeLogging::LOG_WARNING, GetECClass (), 
            "The source or target constraint contain a relationship class which is not supported.");
        return false;
        }

    // ClassMappingRule: If the ends of the relationship are not mapped, don't map the relationship
    size_t nSourceTables = m_ecdbMap.GetTablesFromRelationshipEnd(nullptr, source);
    size_t nTargetTables = m_ecdbMap.GetTablesFromRelationshipEnd(nullptr, target);
    if (0 == nSourceTables || 0 == nTargetTables)
        {
        LogClassNotMapped(NativeLogging::LOG_WARNING, relationshipClass,
            "Source or target constraint classes are not mapped.");

        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                08 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipMapInfo::ContainsRelationshipClass(std::vector<ECClassCP> const& endClasses)
    {
    for (ECClassCP endClass : endClasses)
        {
        if (nullptr != endClass->GetRelationshipClassCP ())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipMapInfo::TryDetermine11RelationshipMapStrategy
(
MapStrategy& strategy,
ECRelationshipConstraintR source,
ECRelationshipConstraintR target,
ECRelationshipClassCR relationshipClass
) const
    {
    /* Note: At this point of the algorithm, map strategy can only be MapStrategy::RelationshipSourceTable,
     * or MapStrategy::RelationshipTargetTable */

    // Check if we need a link table to persist the relationship
    size_t nSourceTables = m_ecdbMap.GetTablesFromRelationshipEnd (nullptr, source);
    size_t nTargetTables = m_ecdbMap.GetTablesFromRelationshipEnd (nullptr, target);

    BeAssert (0 != nSourceTables && "Condition should have been caught earlier, and strategy set to DoNotMap");
    BeAssert (0 != nTargetTables && "Condition should have been caught earlier, and strategy set to DoNotMap");

    // Don't persist at an end that has more than one table
    if (!(nSourceTables == 1 && nTargetTables == 1))
        {
        // We don't persist at an end that has more than one table
        LOG.infov(L"ECRelationshipClass %ls is mapped to link table as more than one end tables exists on either end.",
                     relationshipClass.GetFullName());
        strategy = MapStrategy::OwnTable;
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipMapInfo::TryDetermine1MRelationshipMapStrategy
(
MapStrategy& strategy,
ECRelationshipConstraintR source, 
ECRelationshipConstraintR target, 
ECRelationshipClassCR relationshipClass
) const
    {
    // Pick the M(any) end or the middle depending on the number of tables
    size_t nManyEndTables;
    if (m_cardinality == Cardinality::ManyToOne)
        {
        nManyEndTables = m_ecdbMap.GetTablesFromRelationshipEnd(nullptr, source);
        if (nManyEndTables > 1)
            strategy = MapStrategy::OwnTable;
        else
            strategy = MapStrategy::ForeignKeyRelationshipInSourceTable;
        }
    else
        {
        nManyEndTables = m_ecdbMap.GetTablesFromRelationshipEnd (nullptr, target);
        BeAssert (target.GetCardinality().GetUpperLimit() > 1);
        if (nManyEndTables > 1)
            strategy = MapStrategy::OwnTable;
        else
            strategy = MapStrategy::ForeignKeyRelationshipInTargetTable;
        }

    return true;
    }
   

END_BENTLEY_SQLITE_EC_NAMESPACE
