/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportContext::Initialize(DbSchema& dbSchema, ECDbCR ecdb)
    {
    return DbSchemaPersistenceManager::Load(dbSchema, ecdb, DbSchema::LoadState::ForSchemaImport);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy const* SchemaImportContext::GetUserStrategy(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    return GetUserStrategyP(ecclass, classMapCA);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass) const
    {
    return GetUserStrategyP(ecclass, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    auto it = m_userStrategyCache.find(&ecclass);
    if (it != m_userStrategyCache.end())
        return it->second.get();

    bool hasClassMapCA = true;
    ECDbClassMap classMap;
    if (classMapCA == nullptr)
        {
        hasClassMapCA = ECDbMapCustomAttributeHelper::TryGetClassMap(classMap, ecclass);
        classMapCA = &classMap;
        }

    std::unique_ptr<UserECDbMapStrategy> userStrategy = std::unique_ptr<UserECDbMapStrategy>(new UserECDbMapStrategy());

    if (hasClassMapCA)
        {
        ECDbClassMap::MapStrategy strategy;
        if (ECObjectsStatus::Success != classMapCA->TryGetMapStrategy(strategy))
            return nullptr; // error

        if (SUCCESS != UserECDbMapStrategy::TryParse(*userStrategy, strategy) || !userStrategy->IsValid())
            return nullptr; // error
        }

    UserECDbMapStrategy* userStrategyP = userStrategy.get();
    m_userStrategyCache[&ecclass] = std::move(userStrategy);
    return userStrategyP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   08/2015
//---------------------------------------------------------------------------------------
void SchemaImportContext::CacheClassMapInfo(ClassMap const& classMap, std::unique_ptr<ClassMappingInfo>& info)
    {
    m_classMapInfoCache[&classMap] = std::move(info);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaCompareContext::ReloadECSchemaIfRequired(ECDbSchemaManager const& schemaManager)
    {
    if (HasNoSchemasToImport() || !RequiresUpdate())
        return SUCCESS;

    //save names
    std::vector<Utf8String> existingSchemaNames, importingSchemaNames;
    for (ECSchemaCP schema : m_existingSchemas)
        existingSchemaNames.push_back(schema->GetName());

    for (ECSchemaCP schema : m_importingSchemas)
        importingSchemaNames.push_back(schema->GetName());

    m_existingSchemas.clear();
    m_importingSchemas.clear();
    schemaManager.GetECDb().ClearECDbCache();

    for (Utf8StringCR name : existingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_existingSchemas.push_back(schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_importingSchemas.push_back(schema);
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaCompareContext::AssertIfNotPrepared() const
    {
    if (m_prepared)
        return false;

    BeAssert(m_prepared && "Context is not prepared");
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchemaCompareContext::FindExistingSchema(Utf8CP schemaName) const
    {
    if (AssertIfNotPrepared())
        return nullptr;

    for (auto schema : m_existingSchemas)
        if (schema->GetName() == schemaName)
            return schema;

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaCompareContext::RequiresUpdate() const
    {
    AssertIfNotPrepared();
    return !m_existingSchemas.empty();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaCompareContext::Prepare(ECDbSchemaManager const& schemaManager, bvector<ECSchemaP> const& dependencyOrderedPrimarySchemas)
    {
    if (m_prepared)
        {
        BeAssert(false && "Already prepared");
        return ERROR;
        }

    m_existingSchemas.clear();
    m_importingSchemas.clear();

    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        if (ECSchemaCP existingSchema = schemaManager.GetECSchema(schema->GetName().c_str(), true))
            {
            if (existingSchema == schema)
                continue;

            m_existingSchemas.push_back(existingSchema);
            }

        m_importingSchemas.push_back(schema);
        }

    if (!m_existingSchemas.empty())
        {
        ECSchemaComparer comparer;
        if (comparer.Compare(m_changes, m_existingSchemas, m_importingSchemas) != SUCCESS)
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
        auto importItor = m_importingSchemas.begin();
        while (importItor != m_importingSchemas.end())
            {
            if (schemaOfInterest.find((*importItor)->GetName().c_str()) == schemaOfInterest.end())
                importItor = m_importingSchemas.erase(importItor);
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

    m_prepared = true;
    return SUCCESS;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
