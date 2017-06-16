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
// @bsimethod                                                    Krischan.Eberle   08/2015
//---------------------------------------------------------------------------------------
void SchemaImportContext::CacheClassMapInfo(ClassMap const& classMap, std::unique_ptr<ClassMappingInfo>& info)
    {
    m_classMappingInfoCache[&classMap] = std::move(info);
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
    for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
        {
        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalLinkTables))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalRootEntityClasses))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPolicies::ReadPolicy(ECDbCR ecdb, ECN::ECSchemaCR schema, SchemaPolicy::Type policyType)
    {
    IECInstancePtr policyCA = schema.GetCustomAttributeLocal("ECDbSchemaPolicies", SchemaPolicy::TypeToString(policyType));
    if (policyCA == nullptr)
        return SUCCESS;

    auto it = m_optedInPolicies.find(policyType);
    if (it != m_optedInPolicies.end())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import schemas. Schema '%s' opts in policy '%s' although it is already opted in by schema '%s'. A schema policy can only be opted in by one schema.",
                                                      schema.GetName().c_str(), SchemaPolicy::TypeToString(policyType), SchemaPersistenceHelper::GetSchemaName(ecdb, it->second->GetOptingInSchemaId()).c_str());
        return ERROR;
        }


    std::unique_ptr<SchemaPolicy> policy = SchemaPolicy::Create(policyType, schema.GetId());

    ECValue exceptionsVal;
    if (ECObjectsStatus::Success != policyCA->GetValue(exceptionsVal, "Exceptions"))
        return ERROR;

    //insert exceptions directly into the policy to avoid copying of set<Utf8String>s
    if (!exceptionsVal.IsNull())
        {
        const uint32_t arraySize = exceptionsVal.GetArrayInfo().GetCount();
        for (uint32_t i = 0; i < arraySize; i++)
            {
            ECValue exceptionVal;
            if (ECObjectsStatus::Success != policyCA->GetValue(exceptionVal, "Exceptions", i))
                return ERROR;

            if (exceptionVal.IsNull() || Utf8String::IsNullOrEmpty(exceptionVal.GetUtf8CP()))
                continue;

            policy->m_exceptions.insert(Utf8String(exceptionVal.GetUtf8CP()));
            }
        }

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
//static
std::unique_ptr<SchemaPolicy> SchemaPolicy::Create(Type type, ECN::ECSchemaId optingInSchemaId)
    {
    switch (type)
        {
            case Type::NoAdditionalForeignKeyConstraints:
                return std::make_unique<NoAdditionalForeignKeyConstraintsPolicy>(optingInSchemaId);

            case Type::NoAdditionalLinkTables:
                return std::make_unique<NoAdditionalLinkTablesPolicy>(optingInSchemaId);

            case Type::NoAdditionalRootEntityClasses:
                return std::make_unique<NoAdditionalRootEntityClassesPolicy>(optingInSchemaId);

            default:
                BeAssert(false && "New SchemaPolicy type. Adjust this method");
                return nullptr;
        }
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
BentleyStatus NoAdditionalRootEntityClassesPolicy::Evaluate(ECDbCR ecdb, ECN::ECClassCR ecClass) const
    {
    if (ecClass.GetSchema().GetId() == GetOptingInSchemaId() || 
        ecClass.HasBaseClasses() || !ecClass.IsEntityClass() || ecClass.GetEntityClassCP()->IsMixin() ||
        IsException(ecClass))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECClass '%s'. It violates against the policy that all entity classes must subclass from classes defined in the ECSchema %s",
                                                  ecClass.GetFullName(), SchemaPersistenceHelper::GetSchemaName(ecdb, GetOptingInSchemaId()).c_str());

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalLinkTablesPolicy::Evaluate(ECDbCR ecdb, ECN::ECRelationshipClassCR relClass) const
    {
    if (relClass.GetSchema().GetId() == GetOptingInSchemaId() ||
        relClass.HasBaseClasses() || IsException(relClass))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECRelationshipClass '%s'. It violates against the policy that relationship classes with 'Link table' mapping must subclass from relationship classes defined in the ECSchema %s",
                    relClass.GetFullName(), SchemaPersistenceHelper::GetSchemaName(ecdb, GetOptingInSchemaId()).c_str());

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle      06/2017
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalForeignKeyConstraintsPolicy::Evaluate(ECDbCR ecdb, ECN::NavigationECPropertyCR navPropWithFkConstraintCA) const
    {
    ECClassCR ecClass = navPropWithFkConstraintCA.GetClass();
    if (ecClass.GetSchema().GetId() == GetOptingInSchemaId() ||
         IsException(navPropWithFkConstraintCA))
        return SUCCESS;

    ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECClass '%s'. Its navigation property '%s' violates against the policy that navigation properties may not define the 'ForeignKeyConstraint' custom attribute other than in the ECSchema %s",
                                        navPropWithFkConstraintCA.GetClass().GetFullName(), navPropWithFkConstraintCA.GetName().c_str(), SchemaPersistenceHelper::GetSchemaName(ecdb, GetOptingInSchemaId()).c_str());

    return ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
