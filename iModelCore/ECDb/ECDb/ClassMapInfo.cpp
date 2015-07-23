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
std::unique_ptr<ClassMapInfo> ClassMapInfoFactory::Create(MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecDbMap)
    {
    std::unique_ptr<ClassMapInfo> info = nullptr;
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    if (ecRelationshipClass != nullptr)
        info = std::unique_ptr<ClassMapInfo>(new RelationshipMapInfo(*ecRelationshipClass, ecDbMap));
    else
        info = std::unique_ptr<ClassMapInfo>(new ClassMapInfo(ecClass, ecDbMap));

    if (info == nullptr || (mapStatus = info->Initialize()) != MapStatus::Success)
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
MapStatus ClassMapInfo::Initialize()
    {
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
    if (m_ecClass.GetIsCustomAttributeClass() && !m_ecClass.GetIsDomainClass() && !m_ecClass.GetIsStruct())
        {
        LogClassNotMapped(NativeLogging::LOG_DEBUG, m_ecClass, "ECClass is a custom attribute which is never mapped to a table in ECDb.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MapStatus::Success;
        }

    if (IClassMap::IsAnyClass(m_ecClass) || (m_ecClass.GetSchema().IsStandardSchema() && m_ecClass.GetName().CompareTo("InstanceCount") == 0))
        {
        LogClassNotMapped(NativeLogging::LOG_INFO, m_ecClass, "ECClass is a standard class not supported by ECDb.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MapStatus::Success;
        }

    BeAssert(m_ecdbMap.GetSchemaImportContext() != nullptr);
    UserECDbMapStrategy* userStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategyP(m_ecClass);
    if (userStrategy == nullptr)
        {
        BeAssert(false);
        return MapStatus::Error;
        }

    if (userStrategy->GetStrategy() == UserECDbMapStrategy::Strategy::NotMapped)
        {
        m_resolvedStrategy.Assign(*userStrategy);
        return MapStatus::Success;
        }

    MapStatus stat = DoEvaluateMapStrategy(*userStrategy);
    if (stat != MapStatus::Success)
        return stat;

    BeAssert(m_resolvedStrategy.IsResolved());

    //! We override m_isMapToVirtualTable if SharedTable was used.
    if (m_isMapToVirtualTable)
        m_isMapToVirtualTable = m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::SharedTable;

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ClassMapInfo::DoEvaluateMapStrategy(UserECDbMapStrategy& userStrategy)
    {
    bvector<IClassMap const*> baseClassMaps;
    bvector<IClassMap const*> polymorphicSharedTableClassMaps; // SharedTable-Polymorphic have the highest priority, but there can be only one
    bvector<IClassMap const*> polymorphicOwnTableClassMaps; // OwnTable-Polymorphic have second priority
    bvector<IClassMap const*> polymorphicNotMappedClassMaps; // NotMapped-Polymorphic has priority only over NoHint or NotMapped-NonPolymorphic

    bool areBaseClassesMapped = GatherBaseClassMaps(baseClassMaps, polymorphicSharedTableClassMaps, polymorphicOwnTableClassMaps, polymorphicNotMappedClassMaps, m_ecClass);
    if (!areBaseClassesMapped)
        return MapStatus::BaseClassesNotMapped;

    if (baseClassMaps.empty())
        {
        BeAssert(polymorphicSharedTableClassMaps.empty() && polymorphicOwnTableClassMaps.empty() && polymorphicNotMappedClassMaps.empty());
        return SUCCESS == m_resolvedStrategy.Assign(userStrategy) ? MapStatus::Success : MapStatus::Error;
        }

    // ClassMappingRule: No more than one ancestor of a class can use SharedTable-Polymorphic strategy. Mapping fails if this is violated
    if (polymorphicSharedTableClassMaps.size() > 1)
        return ReportError_OneClassMappedByTableInHierarchyFromTwoDifferentAncestors(m_ecClass, polymorphicSharedTableClassMaps);

    IClassMap const* parentClassMap = nullptr;
    if (!polymorphicSharedTableClassMaps.empty())
        parentClassMap = polymorphicSharedTableClassMaps[0];
    else if (!polymorphicOwnTableClassMaps.empty())
        parentClassMap = polymorphicOwnTableClassMaps[0];
    else if (!polymorphicNotMappedClassMaps.empty())
        parentClassMap = polymorphicNotMappedClassMaps[0];
    else
        parentClassMap = baseClassMaps[0];

    if (!IsValidChildStrategy(parentClassMap->GetMapStrategy (), userStrategy))
        {
        LOG.errorv("MapStrategies of base ECClass %ls and derived ECClass %ls mismatch.", parentClassMap->GetClass().GetFullName(),
                   m_ecClass.GetFullName());
        return MapStatus::Error;
        }

    // ClassMappingRule: If exactly 1 ancestor ECClass is using SharedTable-Polymorphic, use this
    if (polymorphicSharedTableClassMaps.size() == 1)
        {
        m_parentClassMap = parentClassMap;
        ECClassCR parentClass = parentClassMap->GetClass();
        BeAssert(GetECClass().GetIsStruct() == parentClass.GetIsStruct() && "This should have been caught by the schema validation already");

        UserECDbMapStrategy const* parentUserStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategy(parentClass);
        if (parentUserStrategy == nullptr)
            {
            BeAssert(false);
            return MapStatus::Error;
            }

        UserECDbMapStrategy const& rootUserStrategy = userStrategy.AssignRoot(*parentUserStrategy);

        BeAssert(parentClassMap->GetMapStrategy().IsPolymorphicSharedTable());

        ECDbMapStrategy::Option option = ECDbMapStrategy::Option::None;
        if (userStrategy.GetOption() != UserECDbMapStrategy::Option::DisableSharedColumns && 
                (rootUserStrategy.GetOption() == UserECDbMapStrategy::Option::SharedColumns || 
                 rootUserStrategy.GetOption() == UserECDbMapStrategy::Option::SharedColumnsForSubclasses))
            option = ECDbMapStrategy::Option::SharedColumns;

        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::SharedTable, option, true) == SUCCESS ? MapStatus::Success : MapStatus::Error;
        }

    // ClassMappingRule: If one or more parent is using OwnClass-polymorphic, use OwnClass-polymorphic mapping
    if (polymorphicOwnTableClassMaps.size() > 0)
        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::OwnTable, true) == SUCCESS ? MapStatus::Success : MapStatus::Error;

    // ClassMappingRule: If one or more parent is using NotMapped-polymorphic, use NotMapped-polymorphic
    if (polymorphicNotMappedClassMaps.size() > 0)
        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, true) == SUCCESS ? MapStatus::Success : MapStatus::Error;

    if (SUCCESS != m_resolvedStrategy.Assign(userStrategy))
        return MapStatus::Error;

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassMapInfo::IsValidChildStrategy(ECDbMapStrategy const& parentResolvedStrategy, UserECDbMapStrategy const& childStrategy)
    {
    if (childStrategy.GetStrategy() == UserECDbMapStrategy::Strategy::None && childStrategy.GetOption() == UserECDbMapStrategy::Option::None &&
        !childStrategy.IsPolymorphic())
        return true;

    //if (parentResolvedStrategy.IsPolymorphic() && !childStrategy.IsPolymorphic())
    //    return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMapInfo::_InitializeFromSchema ()
    {
    if (SUCCESS != InitializeFromClassMapCA() || SUCCESS != InitializeFromClassHasCurrentTimeStampProperty())
        return ERROR;

    // Add indices for important identifiers
    if (SUCCESS != ProcessStandardKeys(m_ecClass, "BusinessKeySpecification") ||
        SUCCESS != ProcessStandardKeys(m_ecClass, "GlobalIdSpecification") ||
        SUCCESS != ProcessStandardKeys(m_ecClass, "SyncIDSpecification"))
        return ERROR;
    
    //TODO: VerifyThatTableNameIsNotReservedName, e.g. dgn element table, etc.
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 muhammad.zaighum                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapInfo::InitializeFromClassHasCurrentTimeStampProperty()
    {
    IECInstancePtr classHint = m_ecClass.GetCustomAttributeLocal("ClassHasCurrentTimeStampProperty");

    if (classHint == nullptr)
        return SUCCESS;
   
    Utf8String propertyName;
    ECValue v;
    if (classHint->GetValue(v, "PropertyName") == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        propertyName = v.GetUtf8CP();
        ECPropertyCP dateTimeProperty = m_ecClass.GetPropertyP(propertyName);
        if (nullptr == dateTimeProperty)
            {
            BeAssert(false && "ClassHasCurrentTimeStamp Property Not Found in ECClass");
            return ERROR;
            }
        if (dateTimeProperty->GetTypeName().Equals("dateTime"))
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

    UserECDbMapStrategy const* userStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategy(m_ecClass, &customClassMap);
    if (userStrategy == nullptr || !userStrategy->IsValid())
        return ERROR;

    ECObjectsStatus ecstat = customClassMap.TryGetTableName(m_tableName);
    if (ECOBJECTS_STATUS_Success != ecstat)
        return ERROR;

    if ((userStrategy->GetStrategy() == UserECDbMapStrategy::Strategy::ExistingTable ||
        (userStrategy->GetStrategy() == UserECDbMapStrategy::Strategy::SharedTable && !userStrategy->IsPolymorphic())))
        {
        if (m_tableName.empty())
            {
            LOG.errorv("TableName must not be empty in ClassMap custom attribute on ECClass %s if MapStrategy is 'SharedTable, polymorphic' or if MapStrategy is 'ExistingTable'.",
                       m_ecClass.GetFullName());
            return ERROR;
            }
        }
    else
        {
        if (!m_tableName.empty())
            {
            LOG.errorv("TableName must only be set in ClassMap custom attribute on ECClass %s if MapStrategy is 'SharedTable, polymorphic' or 'ExistingTable'.",
                       m_ecClass.GetFullName());
            return ERROR;
            }
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
BentleyStatus ClassMapInfo::ProcessStandardKeys(ECClassCR ecClass, Utf8CP customAttributeName)
    {
    StandardKeySpecification::Type keyType = StandardKeySpecification::GetTypeFromString(customAttributeName);
    if (keyType == StandardKeySpecification::Type::None)
        return SUCCESS;

    IECInstancePtr ca = ecClass.GetCustomAttribute(customAttributeName);
    if (ca == nullptr)
        return SUCCESS;

    ECValue v;
    switch (keyType)
        {
            case StandardKeySpecification::Type::BusinessKeySpecification:
            case StandardKeySpecification::Type::GlobalIdSpecification:
                ca->GetValue(v, "PropertyName"); break;
            case StandardKeySpecification::Type::SyncIDSpecification:
                ca->GetValue(v, "Property"); break;

            default:
                BeAssert(false);
                return ERROR;
        }

    if (v.IsNull())
        return SUCCESS;

    //Create unique not null index on provided property
    Utf8CP keyPropName = v.GetUtf8CP();
    ECPropertyP keyProp = ecClass.GetPropertyP(keyPropName);
    if (nullptr == keyProp)
        {
        LOG.errorv("Invalid %s on class '%s'. The specified property '%s' does not exist in the class.",
                   customAttributeName, ecClass.GetFullName(), keyPropName);
        return ERROR;
        }

    if (keyProp->GetAsPrimitiveProperty() != nullptr)
        {
        const PrimitiveType primType = keyProp->GetAsPrimitiveProperty()->GetType();
        if (primType == PRIMITIVETYPE_Binary ||
            primType == PRIMITIVETYPE_Boolean ||
            primType == PRIMITIVETYPE_DateTime ||
            primType == PRIMITIVETYPE_Double ||
            primType == PRIMITIVETYPE_Integer ||
            primType == PRIMITIVETYPE_Long ||
            primType == PRIMITIVETYPE_String)
            {
            StandardKeySpecificationPtr spec = StandardKeySpecification::Create(keyType);
            spec->GetKeyProperties().push_back(keyPropName);
            m_standardKeys.push_back(spec);
            return SUCCESS;
            }
        }

    LOG.errorv("Invalid %s on class '%s'. The data type of the specified property '%s' is not supported. Supported types: Binary, Boolean, DateTime, Double, Integer, Long and String.",
               customAttributeName, ecClass.GetFullName(), keyPropName);
    return ERROR;
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
                case ECDbMapStrategy::Strategy::SharedTable:
                    {
                    bool add = true;
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

                case ECDbMapStrategy::Strategy::OwnTable:
                    tpcMaps.push_back(baseClassMap);
                    break;

                case ECDbMapStrategy::Strategy::NotMapped:
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

    Utf8StringCR namespacePrefix = schema.GetNamespacePrefix ();
    if (namespacePrefix.empty ())
        return schema.GetName ();
    
    return namespacePrefix;
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
    Utf8String message;
    message.Sprintf ("Two or more base ECClasses (or their base classes) of %s.%s are using the MapStrategy 'SharedTable (polymorphic)'. We cannot determine which to honor. The base ECClasses are: ",
        ecClass.GetSchema ().GetName ().c_str (), ecClass.GetName ().c_str ());
    for (auto tphMap : tphMaps)
        {
        message.append (tphMap->GetClass ().GetName ().c_str ());
        message.append (" ");
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
        LOG.errorv("ECRelationshipClass '%s' can only have either the ForeignKeyRelationshipMap or the LinkTableRelationshipMap custom attribute but not both.",
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
                    LOG.errorv("ForeignKeyRelationshipMap custom attribute on ECRelationshipClass '%s' must not have a value for ForeignKeyProperty as there are Key properties defined in the ECRelationshipConstraint on the foreign key end.",
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
    MapStatus stat = ClassMapInfo::_EvaluateMapStrategy();
    if (stat != MapStatus::Success)
        return stat;

    if (m_resolvedStrategy.IsNotMapped())
        return MapStatus::Success;

    ECRelationshipClassCP relationshipClass = GetECClass().GetRelationshipClassCP();
    ECRelationshipConstraintR source = relationshipClass->GetSource();
    ECRelationshipConstraintR target = relationshipClass->GetTarget();

    DetermineCardinality(source, target);

    const size_t sourceTableCount = m_ecdbMap.GetTableCountOnRelationshipEnd(source);
    const size_t targetTableCount = m_ecdbMap.GetTableCountOnRelationshipEnd(target);

    if (sourceTableCount == 0 || targetTableCount == 0)
        {
        LogClassNotMapped(NativeLogging::LOG_WARNING, *relationshipClass, "Source or target constraint classes are not mapped.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MapStatus::Success;
        }

    const bool userStrategyIsForeignKeyMapping = m_customMapType == CustomMapType::ForeignKeyOnSource || m_customMapType == CustomMapType::ForeignKeyOnTarget;

    if (m_resolvedStrategy.IsPolymorphicSharedTable())
        {
        if (userStrategyIsForeignKeyMapping)
            {
            LOG.errorv("The ECRelationshipClass '%s' implies a link table relationship with (MapStrategy: SharedTable, polymorphic), but it has a ForeignKeyRelationshipMap custom attribute.",
                       GetECClass().GetFullName());
            return MapStatus::Error;
            }

        return MapStatus::Success;
        }

    if (m_customMapType == CustomMapType::LinkTable ||
        m_cardinality == Cardinality::ManyToMany ||
        GetECClass().GetPropertyCount() > 0)
        {
        if (userStrategyIsForeignKeyMapping)
            {
            LOG.errorv("The ECRelationshipClass '%s' implies a link table relationship with because of its cardinality or because it has ECProperties. Therefore it must not have a ForeignKeyRelationshipMap custom attribute.",
                       GetECClass().GetFullName());
            return MapStatus::Error;
            }

        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::OwnTable, false) == SUCCESS ? MapStatus::Success : MapStatus::Error;
        }

    BeAssert(!m_allowDuplicateRelationships && "This can only be true if CustomMapType is LinkTable. That condition was already handled before though.");

    ECDbMapStrategy::Strategy resolvedStrategy;
    switch (m_cardinality)
        {
            case Cardinality::OneToOne:
                {
                // Don't persist at an end that has more than one table
                if (sourceTableCount > 1 || targetTableCount > 1)
                    {
                    if (userStrategyIsForeignKeyMapping)
                        {
                        Utf8CP constraintStr = nullptr;
                        if (sourceTableCount > 1 && targetTableCount > 1)
                            constraintStr = "source and target constraints are";
                        else if (sourceTableCount > 1)
                            constraintStr = "source constraint is";
                        else
                            constraintStr = "target constraint is";

                        LOG.errorv("ECRelationshipClass %s implies a link table relationship as the %s mapped to more than one end table. Therefore it must not have a ForeignKeyRelationshipMap custom attribute.",
                                   GetECClass().GetFullName(), constraintStr);
                        return MapStatus::Error;
                        }

                    resolvedStrategy = ECDbMapStrategy::Strategy::OwnTable;
                    }
                else if (m_customMapType == CustomMapType::ForeignKeyOnSource)
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable;
                else
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable;

                break;
                }

            case Cardinality::OneToMany:
                {
                if (m_customMapType == CustomMapType::ForeignKeyOnSource)
                    {
                    LOG.errorv("ECRelationshipClass %s implies a foreign key relationship on the target's table. Therefore the 'End' property in the ForeignKeyRelationshipMap custom attribute must not be set to 'Source'.",
                               GetECClass().GetFullName());
                    return MapStatus::Error;
                    }

                if (targetTableCount > 1)
                    {
                    if (userStrategyIsForeignKeyMapping)
                        {
                        LOG.errorv("ECRelationshipClass %s implies a link table relationship as the target constraint is mapped to more than one end table. Therefore it must not have a ForeignKeyRelationshipMap custom attribute.",
                                   GetECClass().GetFullName());
                        return MapStatus::Error;
                        }

                    resolvedStrategy = ECDbMapStrategy::Strategy::OwnTable;
                    }
                else
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable;

                break;
                }

            case Cardinality::ManyToOne:
                {
                if (m_customMapType == CustomMapType::ForeignKeyOnTarget)
                    {
                    LOG.errorv("ECRelationshipClass %s implies a foreign key relationship on the source's table. Therefore the 'End' property in the ForeignKeyRelationshipMap custom attribute must not be set to 'Target'.",
                               GetECClass().GetFullName());
                    return MapStatus::Error;
                    }

                if (sourceTableCount > 1)
                    {
                    if (userStrategyIsForeignKeyMapping)
                        {
                        LOG.errorv("ECRelationshipClass %s implies a link table relationship as the source constraint is mapped to more than one end table.. Therefore it must not have a ForeignKeyRelationshipMap custom attribute.",
                                   GetECClass().GetFullName());
                        return MapStatus::Error;
                        }

                    resolvedStrategy = ECDbMapStrategy::Strategy::OwnTable;
                    }
                else
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable;

                break;
                }

            default:
                BeAssert(false && "ManyToMany case should have been handled already.");
                return MapStatus::Error;
        }

    return m_resolvedStrategy.Assign(resolvedStrategy, false) == SUCCESS ? MapStatus::Success : MapStatus::Error;

    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                06/2014
//+---------------+---------------+---------------+---------------+---------------+------
void RelationshipMapInfo::DetermineCardinality(ECRelationshipConstraintCR source, ECRelationshipConstraintCR target)
    {
    const bool sourceIsM = source.GetCardinality().GetUpperLimit() > 1;
    const bool targetIsM = target.GetCardinality().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        m_cardinality = Cardinality::ManyToMany;
    else if (!sourceIsM && targetIsM)
        m_cardinality = Cardinality::OneToMany;
    else if (sourceIsM && !targetIsM)
        m_cardinality = Cardinality::ManyToOne;
    else
        m_cardinality = Cardinality::OneToOne;
    }
  

END_BENTLEY_SQLITE_EC_NAMESPACE
