/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaImportContext.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus SchemaImportContext::QueryMixIns(ECDbCR ecdb, DbTable const& table, std::vector<ECEntityClassCP>& mixIns)
	{
	if (!table.GetExclusiveRootECClassId().IsValid())
		{
		BeAssert(false && "This only apply to TPH");
		return ERROR;
		}

	if (!mixIns.empty())
		mixIns.clear();

	Utf8CP mixInSql =
		"SELECT  CHBC.BaseClassId from " TABLE_ClassHierarchyCache " CCH "
		"	INNER JOIN " TABLE_ClassHasBaseClasses " CHBC ON CHBC.ClassId = CCH.ClassId "
		"WHERE CCH.BaseClassId = ?  AND CHBC.BaseClassId IN ( "
		"	SELECT CA.ContainerId FROM " TABLE_Class " C "
		"		INNER JOIN " TABLE_Schema " S ON S.Id = C.SchemaId "
		"		INNER JOIN " TABLE_CustomAttribute " CA ON CA.ClassId = C.Id "
		"	WHERE C.Name = 'IsMixin' AND S.Name='CoreCustomAttributes') "
		"GROUP BY CHBC.BaseClassId ";

	CachedStatementPtr stmt = ecdb.GetCachedStatement(mixInSql);
	stmt->BindId(1, table.GetExclusiveRootECClassId());
	while (stmt->Step() == BE_SQLITE_ROW)
		{
		ECClassId mixInId = stmt->GetValueId<ECClassId>(0);
		ECClassCP classCP = ecdb.Schemas().GetECClass(mixInId);
		if (!classCP->IsEntityClass())
			{
			mixIns.clear();
			BeAssert(false && "SQL query has issue. Something changed about Mixin CA");
			return ERROR;
			}
		ECEntityClassCP entityClass = static_cast<ECEntityClassCP>(classCP);
		if (!entityClass->IsMixin())
			{
			mixIns.clear();
			BeAssert(false && "SQL query has issue. Expecting MixIn");
			return ERROR;
			}

		mixIns.push_back(entityClass);
		//Load its derived classes as well.
		}

	return SUCCESS;
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2017
//---------------------------------------------------------------------------------------
std::vector<ECN::ECEntityClassCP> const* SchemaImportContext::QueryMixIns(ECDbCR ecdb, DbTable const& table) const
	{
	auto itor = m_mixInCachePerTable.find(&table);
	if (itor != m_mixInCachePerTable.end())
		{
		BeAssert(false && "API should be classed for TPH");
		return itor->second.get();
		}

	std::vector<ECEntityClassCP>* mixIns = new std::vector<ECEntityClassCP>();
	if (QueryMixIns(ecdb, table, *mixIns) == ERROR)
		{
		delete mixIns;
		mixIns = nullptr;
		BeAssert(false && "API should be classed for TPH");
		}

	m_mixInCachePerTable.insert(std::make_pair(&table, std::unique_ptr<std::vector<ECEntityClassCP>>(mixIns)));
	return mixIns;
	}
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

//****************************************************************************************** 
//SchemaImportContext::ClassMapSaveContext
//****************************************************************************************** 

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    06/2016
//------------------------------------------------------------------------------------------
bool SchemaImportContext::ClassMapSaveContext::NeedsSaving(ClassMap const& classMap)
    {
    if (m_alreadySavedClassMaps.find(&classMap) != m_alreadySavedClassMaps.end())
        return false;

    m_alreadySavedClassMaps.insert(&classMap);
    return true;
    }

//****************************************************************************************** 
//ECSchemaCompareContext
//****************************************************************************************** 

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaCompareContext::ReloadContextECSchemas(ECDbSchemaManager const& schemaManager)
    {
    //We need to reload context schemas
    //if (HasNoSchemasToImport() || !RequiresUpdate())
    //    return SUCCESS;

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
BentleyStatus ECSchemaCompareContext::Prepare(ECDbSchemaManager const& schemaManager, bvector<ECSchemaCP> const& dependencyOrderedPrimarySchemas)
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
