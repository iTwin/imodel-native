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
    for (ECSchemaCP schema : m_existingSchemaList)
        existingSchemaNames.push_back(schema->GetName());

    for (ECSchemaCP schema : m_importedSchemaList)
        importingSchemaNames.push_back(schema->GetName());

    m_existingSchemaList.clear();
    m_importedSchemaList.clear();
    schemaManager.GetECDb().ClearECDbCache();

    for (Utf8StringCR name : existingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_existingSchemaList.push_back(schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_importedSchemaList.push_back(schema);
        }

    return DbSchemaPersistenceManager::Load(
        schemaManager.GetECDb().GetECDbImplR().GetECDbMap().GetDbSchemaR(), 
        schemaManager.GetECDb(), DbSchema::LoadState::ForSchemaImport);
    
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

    for (auto schema : m_existingSchemaList)
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
    return !m_existingSchemaList.empty();
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

    m_existingSchemaList.clear();
    m_importedSchemaList.clear();
    std::set<Utf8String> doneList;
    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        if (doneList.find(schema->GetFullSchemaName()) != doneList.end())
            continue;

        doneList.insert(schema->GetFullSchemaName());
        if (ECSchemaCP existingSchema = schemaManager.GetECSchema(schema->GetName().c_str(), true))
            {
            if (existingSchema == schema)
                continue;

            m_existingSchemaList.push_back(existingSchema);
            }

        m_importedSchemaList.push_back(schema);
        }

    if (!m_existingSchemaList.empty())
        {
        ECSchemaComparer comparer;
        //We do not require detail if schema is added or deleted the name and version suffice
        ECSchemaComparer::Options options = ECSchemaComparer::Options(ECSchemaComparer::AppendDetailLevel::Partial, ECSchemaComparer::AppendDetailLevel::Partial);
        if (comparer.Compare(m_changes, m_existingSchemaList, m_importedSchemaList, options) != SUCCESS)
            return ERROR;
        
        /*
        Utf8String str;
        m_changes.WriteToString(str);
        printf("%s", str.c_str());
        */
        std::set<Utf8CP, CompareIUtf8Ascii> schemaOfInterest;
        if (m_changes.IsValid())
            {
            for (size_t i = 0; i < m_changes.Count(); i++)
                {
                schemaOfInterest.insert(m_changes.At(i).GetId());                
                }
            }
        //Remove any none interesting schemas
        auto importItor = m_importedSchemaList.begin();
        while (importItor != m_importedSchemaList.end())
            {
            if (schemaOfInterest.find((*importItor)->GetName().c_str()) == schemaOfInterest.end())
                importItor = m_importedSchemaList.erase(importItor);
            else
                ++importItor;
            }

        //Remove any none interesting schemas
        auto existingItor = m_existingSchemaList.begin();
        while (existingItor != m_existingSchemaList.end())
            {
            if (schemaOfInterest.find((*existingItor)->GetName().c_str()) == schemaOfInterest.end())
                existingItor = m_existingSchemaList.erase(existingItor);
            else
                ++existingItor;
            }
        }

    m_prepared = true;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
