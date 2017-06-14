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
void SchemaImportContext::CacheFkConstraintCA(ECN::NavigationECPropertyCR navProp)
    {
    ForeignKeyConstraintCustomAttribute ca;
    if (!ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(ca, navProp))
        return;

    BeAssert(navProp.GetRelationshipClass() != nullptr);
    ECClassId relClassId = navProp.GetRelationshipClass()->GetId();
    BeAssert(relClassId.IsValid() && "Navigation property's relationship class is expected to have been persisted already by this time");
    BeAssert(m_fkConstraintCACache.find(relClassId) == m_fkConstraintCACache.end());
    m_fkConstraintCACache[relClassId] = ca;
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

END_BENTLEY_SQLITE_EC_NAMESPACE
