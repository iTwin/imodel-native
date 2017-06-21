/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
ClassMappingCACache const* SchemaImportContext::GetClassMappingCACache(ECClassCR ecclass) const
    {
    return GetClassMappingCACacheP(ecclass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan       06/2017
//--------------------------------------------------------------------------------------
SchemaImportContext::SchemaImportContext(SchemaManager::SchemaImportOptions options) :m_relCol(new EndTableMappingContextCollection(*this)), m_options(options) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
ClassMappingCACache* SchemaImportContext::GetClassMappingCACacheP(ECClassCR ecclass) const
    {
    auto it = m_classMappingCACache.find(&ecclass);
    if (it != m_classMappingCACache.end())
        return it->second.get();

    std::unique_ptr<ClassMappingCACache> cache = std::unique_ptr<ClassMappingCACache>(new ClassMappingCACache());
    if (SUCCESS != cache->Initialize(ecclass))
        return nullptr; // error

    ClassMappingCACache* cacheP = cache.get();
    m_classMappingCACache[&ecclass] = std::move(cache);
    return cacheP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan       06/2017
//---------------------------------------------------------------------------------------
ClassMappingStatus SchemaImportContext::MapNavigationProperty(NavigationPropertyMap& navPropMap) { return m_relCol->Map(navPropMap); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan       06/2017
//---------------------------------------------------------------------------------------
ClassMappingStatus SchemaImportContext::FinishEndTableMapping() { return m_relCol->FinishMapping(); }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   08/2015
//---------------------------------------------------------------------------------------
void SchemaImportContext::CacheClassMapInfo(ClassMap const& classMap, std::unique_ptr<ClassMappingInfo>& info)
    {
    ClassMappingInfo* pInfo = info.get();
    m_classMappingInfoCache[&classMap] = std::move(info);

    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        m_relCol->RegisterContext(classMap.GetAs<RelationshipClassEndTableMap>(), static_cast<RelationshipMappingInfo const&> (*pInfo));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2017
//---------------------------------------------------------------------------------------
bool SchemaImportContext::CacheFkConstraintCA(ECN::NavigationECPropertyCR navProp)
    {
    ForeignKeyConstraintCustomAttribute ca;
    if (!ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(ca, navProp))
        return false;

    BeAssert(navProp.GetRelationshipClass() != nullptr);
    ECClassId relClassId = navProp.GetRelationshipClass()->GetId();
    BeAssert(relClassId.IsValid() && "Navigation property's relationship class is expected to have been persisted already by this time");
    BeAssert(m_fkConstraintCACache.find(relClassId) == m_fkConstraintCACache.end());
    m_fkConstraintCACache[relClassId] = ca;
    return true;
    }

//****************************************************************************************** 
//ECSchemaCompareContext
//****************************************************************************************** 

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaCompareContext::ReloadContextECSchemas(SchemaManager const& schemaManager)
    {
    //save names
    std::vector<Utf8String> existingSchemaNames, importingSchemaNames;
    for (ECSchemaCP schema : m_existingSchemas)
        existingSchemaNames.push_back(schema->GetName());

    for (ECSchemaCP schema : m_schemasToImport)
        importingSchemaNames.push_back(schema->GetName());

    m_existingSchemas.clear();
    m_schemasToImport.clear();
    schemaManager.GetECDb().ClearECDbCache();

    for (Utf8StringCR name : existingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            LOG.errorv("Schema import failed. Failed to read imported schema %s from ECDb.", name.c_str());
            return ERROR;
            }

        m_existingSchemas.push_back(schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            LOG.errorv("Schema import failed. Failed to read imported schema %s from ECDb.", name.c_str());
            return ERROR;
            }

        m_schemasToImport.push_back(schema);
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaCompareContext::Prepare(SchemaManager const& schemaManager, bvector<ECSchemaCP> const& dependencyOrderedPrimarySchemas)
    {
    if (dependencyOrderedPrimarySchemas.empty())
        {
        BeAssert(false);
        return ERROR;
        }

    m_existingSchemas.clear();
    m_schemasToImport.clear();
    std::set<Utf8String> doneList;
    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        if (doneList.find(schema->GetFullSchemaName()) != doneList.end())
            continue;

        doneList.insert(schema->GetFullSchemaName());
        if (ECSchemaCP existingSchema = schemaManager.GetSchema(schema->GetName().c_str(), true))
            {
            if (existingSchema == schema)
                continue;

            m_existingSchemas.push_back(existingSchema);
            }

        m_schemasToImport.push_back(schema);
        }

    if (!m_existingSchemas.empty())
        {
        SchemaComparer comparer;
        //We do not require detail if schema is added or deleted the name and version suffice
        SchemaComparer::Options options = SchemaComparer::Options(SchemaComparer::AppendDetailLevel::Partial, SchemaComparer::AppendDetailLevel::Partial);
        if (comparer.Compare(m_changes, m_existingSchemas, m_schemasToImport, options) != SUCCESS)
            return ERROR;

        std::set<Utf8CP, CompareIUtf8Ascii> schemaOfInterest;
        if (m_changes.IsValid())
            {
            for (size_t i = 0; i < m_changes.Count(); i++)
                {
                schemaOfInterest.insert(m_changes.At(i).GetId());
                }
            }
        //Remove any none interesting schemas
        auto importItor = m_schemasToImport.begin();
        while (importItor != m_schemasToImport.end())
            {
            if (schemaOfInterest.find((*importItor)->GetName().c_str()) == schemaOfInterest.end())
                importItor = m_schemasToImport.erase(importItor);
            else
                ++importItor;
            }

        //Remove any none interesting schemas
        auto existingItor = m_existingSchemas.begin();
        while (existingItor != m_existingSchemas.end())
            {
            if (schemaOfInterest.find((*existingItor)->GetName().c_str()) == schemaOfInterest.end())
                existingItor = m_existingSchemas.erase(existingItor);
            else
                ++existingItor;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaCompareContext::FindExistingSchema(Utf8CP schemaName) const
    {
    for (ECSchemaCP schema : m_existingSchemas)
        {
        if (schema->GetName().Equals(schemaName))
            return schema;
        }

    return nullptr;
    }

//*********************************************************************************
// SchemaPolicies
//*********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPolicies::ReadPolicies(ECDbCR ecdb)
    {
    std::vector<ECN::ECSchemaId> systemSchemaExceptions = GetSystemSchemaExceptions(ecdb);
    for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
        {
        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints, systemSchemaExceptions))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalLinkTables, systemSchemaExceptions))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalRootEntityClasses, systemSchemaExceptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPolicies::ReadPolicy(ECDbCR ecdb, ECN::ECSchemaCR schema, SchemaPolicy::Type policyType, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    IECInstancePtr policyCA = schema.GetCustomAttributeLocal("ECDbSchemaPolicies", SchemaPolicy::TypeToString(policyType));
    if (policyCA == nullptr)
        return SUCCESS;

    auto it = m_optedInPolicies.find(policyType);
    if (it != m_optedInPolicies.end())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import schemas. Schema '%s' opts in policy '%s' although it is already opted in by schema '%s'. A schema policy can only be opted in by one schema.",
                                                      schema.GetName().c_str(), SchemaPolicy::TypeToString(policyType), ecdb.Schemas().GetReader().GetSchemaName(it->second->GetOptingInSchemaId()).c_str());
        return ERROR;
        }


    std::unique_ptr<SchemaPolicy> policy = nullptr;
    switch (policyType)
        {
            case SchemaPolicy::Type::NoAdditionalForeignKeyConstraints:
                policy = NoAdditionalForeignKeyConstraintsPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            case SchemaPolicy::Type::NoAdditionalLinkTables:
                policy = NoAdditionalLinkTablesPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            case SchemaPolicy::Type::NoAdditionalRootEntityClasses:
                policy = NoAdditionalRootEntityClassesPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            default:
                BeAssert(false && "New SchemaPolicy type was added. Adjust this method!");
                return ERROR;
        }

    if (policy == nullptr)
        return ERROR;

    m_optedInPolicies[policyType] = std::move(policy);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
bool SchemaPolicies::IsOptedIn(SchemaPolicy const*& policy, SchemaPolicy::Type policyType) const
    {
    auto it = m_optedInPolicies.find(policyType);
    if (it == m_optedInPolicies.end())
        return false;

    policy = it->second.get();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
std::vector<ECSchemaId> SchemaPolicies::GetSystemSchemaExceptions(ECDbCR ecdb)
    {
    std::vector<Utf8CP> ecdbSchemaNames = ecdb.Schemas().GetReader().GetECDbSchemaNames();
    std::set<Utf8CP, CompareIUtf8Ascii> systemSchemaExceptions(ecdbSchemaNames.begin(), ecdbSchemaNames.end());
    return SchemaPersistenceHelper::GetSchemaIds(ecdb, Utf8StringVirtualSet([&systemSchemaExceptions] (Utf8CP name) { return systemSchemaExceptions.find(name) != systemSchemaExceptions.end(); }));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
bool SchemaPolicy::IsException(ECN::ECClassCR candidateClass) const
    {
    if (m_schemaExceptions.find(candidateClass.GetSchema().GetId()) != m_schemaExceptions.end())
        return true;

    return m_classExceptions.find(candidateClass.GetId()) != m_classExceptions.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus SchemaPolicy::RetrieveExceptions(bvector<bvector<Utf8String>>& tokenizedExceptions, ECN::IECInstanceCR policyCA, Utf8CP exceptionsPropName)
    {
    ECValue exceptionsVal;
    if (ECObjectsStatus::Success != policyCA.GetValue(exceptionsVal, exceptionsPropName))
        return ERROR;

    if (exceptionsVal.IsNull())
        return SUCCESS;

    const uint32_t arraySize = exceptionsVal.GetArrayInfo().GetCount();
    for (uint32_t i = 0; i < arraySize; i++)
        {
        ECValue exceptionVal;
        if (ECObjectsStatus::Success != policyCA.GetValue(exceptionVal, exceptionsPropName, i))
            return ERROR;

        if (exceptionVal.IsNull() || Utf8String::IsNullOrEmpty(exceptionVal.GetUtf8CP()))
            continue;

        tokenizedExceptions.push_back(bvector<Utf8String>());
        BeStringUtilities::Split(exceptionVal.GetUtf8CP(), ":.", tokenizedExceptions.back());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP SchemaPolicy::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::NoAdditionalForeignKeyConstraints:
                return "NoAdditionalForeignKeyConstraints";
            case Type::NoAdditionalLinkTables:
                return "NoAdditionalLinkTables";
            case Type::NoAdditionalRootEntityClasses:
                return "NoAdditionalRootEntityClasses";

            default:
                BeAssert(false && "SchemaPolicy::Type enum has a new value. This method needs to be adjusted");
                return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalRootEntityClassesPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalRootEntityClassesPolicy> policy(new NoAdditionalRootEntityClassesPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(optingInSchemaId); // the opting-in schema is always an exception
    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                      policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(optingInSchemaId).c_str());
        return nullptr;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 2)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                          policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(optingInSchemaId).c_str());
            return nullptr;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = ecdb.Schemas().GetReader().GetSchemaId(tokenizedException[0], SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                policy->m_schemaExceptions.insert(exceptionSchemaId);
            }
        else
            {
            ECClassId exceptionClassId = ecdb.Schemas().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);
            
            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                policy->m_classExceptions.insert(exceptionClassId);
            }
        }

    return std::move(policy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalRootEntityClassesPolicy::Evaluate(ECDbCR ecdb, ECN::ECClassCR ecClass) const
    {
    if (ecClass.HasBaseClasses() || !ecClass.IsEntityClass() || ecClass.GetEntityClassCP()->IsMixin() || IsException(ecClass))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECClass '%s'. It violates against the 'No additional root entity classes' policy which means that all entity classes must subclass from classes defined in the ECSchema %s",
                                                  ecClass.GetFullName(), ecdb.Schemas().GetReader().GetSchemaName(GetOptingInSchemaId()).c_str());

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalLinkTablesPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalLinkTablesPolicy> policy(new NoAdditionalLinkTablesPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(optingInSchemaId); // the opting-in schema is always an exception
    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                      policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(optingInSchemaId).c_str());
        return nullptr;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 2)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                          policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(optingInSchemaId).c_str());
            return nullptr;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = ecdb.Schemas().GetReader().GetSchemaId(tokenizedException[0], SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                policy->m_schemaExceptions.insert(exceptionSchemaId);
            }
        else
            {
            ECClassId exceptionClassId = ecdb.Schemas().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);

            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                policy->m_classExceptions.insert(exceptionClassId);
            }
        }

    return std::move(policy);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalLinkTablesPolicy::Evaluate(ECDbCR ecdb, ECN::ECRelationshipClassCR relClass) const
    {
    if (relClass.HasBaseClasses() || IsException(relClass))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECRelationshipClass '%s'. It violates against the 'No additional link tables' policy which means that relationship classes with 'Link table' mapping must subclass from relationship classes defined in the ECSchema %s",
                    relClass.GetFullName(), ecdb.Schemas().GetReader().GetSchemaName(GetOptingInSchemaId()).c_str());

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalForeignKeyConstraintsPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalForeignKeyConstraintsPolicy> policy(new NoAdditionalForeignKeyConstraintsPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    if (SUCCESS != policy->ReadExceptionsFromCA(ecdb, policyCA))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                      policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(optingInSchemaId).c_str());
        return nullptr;
        }

    return std::move(policy);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalForeignKeyConstraintsPolicy::ReadExceptionsFromCA(ECDbCR ecdb, ECN::IECInstanceCR policyCA)
    {
    m_schemaExceptions.insert(m_optingInSchemaId); // the opting-in schema is always an exception
   
    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                      policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(m_optingInSchemaId).c_str());
        return ERROR;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 3)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                                                          policyCA.GetClass().GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(m_optingInSchemaId).c_str());
            return ERROR;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = ecdb.Schemas().GetReader().GetSchemaId(tokenizedException[0], SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                m_schemaExceptions.insert(exceptionSchemaId);
            }
        else if (tokenCount == 2 || tokenizedException[2].EqualsIAscii("*"))
            {
            ECClassId exceptionClassId = ecdb.Schemas().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);

            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                m_classExceptions.insert(exceptionClassId);
            }
        else
            {
            ECPropertyId exceptionPropId = ecdb.Schemas().GetReader().GetPropertyId(tokenizedException[0], tokenizedException[1], tokenizedException[2], SchemaLookupMode::AutoDetect);
            // If property id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionPropId.IsValid())
                m_propertyExceptions.insert(exceptionPropId);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalForeignKeyConstraintsPolicy::Evaluate(ECDbCR ecdb, ECN::NavigationECPropertyCR navPropWithFkConstraintCA) const
    {
    if (IsException(navPropWithFkConstraintCA))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECClass '%s'. Its navigation property '%s' violates against the 'No additional foreign key constraints' policy which means that navigation properties may not define the 'ForeignKeyConstraint' custom attribute other than in the ECSchema %s",
                                        navPropWithFkConstraintCA.GetClass().GetFullName(), navPropWithFkConstraintCA.GetName().c_str(), ecdb.Schemas().GetReader().GetSchemaName(GetOptingInSchemaId()).c_str());

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
bool NoAdditionalForeignKeyConstraintsPolicy::IsException(ECN::NavigationECPropertyCR navProp) const
    {
    if (IsException(navProp.GetClass()))
        return true;

    return m_propertyExceptions.find(navProp.GetId()) != m_propertyExceptions.end();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
