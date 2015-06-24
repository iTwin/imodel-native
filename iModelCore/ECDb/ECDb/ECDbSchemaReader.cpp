/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::AddECSchemaToCache (ECSchemaCR schema)
    {
    AddECSchemaToCacheInternal (schema);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaReader::CanAnalyze (ECClassId classId) const
    {
    BeMutexHolder aGuard (m_criticalSection);
    return m_ecClassKeyByECClassIdLookup.find (classId) != m_ecClassKeyByECClassIdLookup.end();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::AddECSchemaToCacheInternal (ECSchemaCR schema)
    {
    BeMutexHolder aGuard (m_criticalSection);
    ECSchemaId ecSchemaId = schema.GetId();
    DbECSchemaEntryP schemaKey = FindDbECSchemaEntry (ecSchemaId);
    if (schemaKey == nullptr)
        {
        // build key lookups
        schemaKey                        = new DbECSchemaEntry();
        schemaKey->m_ecSchemaId          = ecSchemaId;
        schemaKey->m_resolvedECSchema    = const_cast<ECSchemaP>(&schema);
        schemaKey->m_schemaName.Assign (schema.GetName().c_str());
        schemaKey->m_versionMajor        = schema.GetVersionMajor();
        schemaKey->m_versionMinor        = schema.GetVersionMinor();
        schemaKey->m_nClassesInSchema = schemaKey->m_nClassesLoaded = 0;
        m_ecSchemaByECSchemaIdLookup[ecSchemaId] = schemaKey;
        //m_ecSchemaByNameLookup[schema.GetName().c_str()] = schemaKey;
        for (ECClassCP ecClass : schemaKey->m_resolvedECSchema->GetClasses())
            {
            DbECClassEntryP classKey    = new DbECClassEntry();
            if (!ecClass->HasId())
                const_cast<ECClassP>(ecClass)->SetId (
                    ECDbSchemaPersistence::GetECClassIdBySchemaName(m_db, 
                        Utf8String(ecClass->GetSchema().GetName()).c_str(),
                        Utf8String(ecClass->GetName()).c_str()));
            classKey->m_ecClassId       = ecClass->GetId();
            classKey->m_ecSchemaId      = ecSchemaId;
            classKey->m_className.Assign (ecClass->GetName().c_str());

            classKey->m_resolvedECClass = const_cast<ECClassP>(ecClass);
            m_ecClassKeyByECClassIdLookup[classKey->m_ecClassId] = classKey;
            schemaKey->m_nClassesInSchema++;
            }
        schemaKey->m_nClassesLoaded = schemaKey->m_nClassesInSchema;
        //WIP AddECSchemaToCacheInternal() is called for each schema that got imported. The references would get imported anyway so we donot need to traverse it.
        //ECSchemaReferenceListCR referencedSchemas = schemaKey->m_resolvedECSchema->GetReferencedSchemas();
        //for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        //    AddECSchemaToCacheInternal (*(iter->second), previousOwner);
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::ReadECClass(ECClassP& ecClass, ECClassId ecClassId)
    {
    BeMutexHolder aGuard (m_criticalSection);
    DbResult r;

    DbECClassEntryP key = nullptr;
    DbECClassEntryMap::const_iterator  classKeyIterator = m_ecClassKeyByECClassIdLookup.find (ecClassId);
    if (classKeyIterator != m_ecClassKeyByECClassIdLookup.end())
        key = classKeyIterator->second; 
    else
        {
        key = new DbECClassEntry();
        key->m_resolvedECClass = nullptr;
        r = ECDbSchemaPersistence::ResolveECClassId (*key, ecClassId, m_db);
        if (r != BE_SQLITE_ROW)
            return r;

        m_ecClassKeyByECClassIdLookup[key->m_ecClassId] = key;

        DbECSchemaEntryP outECSchemaKey;
        r = ReadECSchema (outECSchemaKey, key->m_ecSchemaId, false);
        if (r != BE_SQLITE_ROW)
            return r;

        r = LoadECClassFromDb(key->m_resolvedECClass, ecClassId, *outECSchemaKey->m_resolvedECSchema);
        if (r != BE_SQLITE_ROW)
            return r;
        else
            outECSchemaKey->m_nClassesLoaded++;
        }

    if (key->m_resolvedECClass == nullptr)
        return BE_SQLITE_ERROR;

    ecClass = key->m_resolvedECClass;
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbECClassEntryP ECDbSchemaReader::FindDbECClassEntry(ECClassId ecClassId)
    {
    DbECClassEntryMap::const_iterator  classKeyIterator = m_ecClassKeyByECClassIdLookup.find (ecClassId);
    if (classKeyIterator != m_ecClassKeyByECClassIdLookup.end())
        return classKeyIterator->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbECSchemaEntryP ECDbSchemaReader::FindDbECSchemaEntry(ECSchemaId ecSchemaId)
    {
    DbECSchemaMap::const_iterator  schemaIterator = m_ecSchemaByECSchemaIdLookup.find (ecSchemaId);
    if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end())
        return schemaIterator->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECSchemaDefinition(DbECSchemaEntryP& outECSchemaKey, bvector<DbECSchemaEntryP>& newlyLoadedSchemas, ECSchemaId ecSchemaId)
    {
    DbECSchemaEntryP                 key;
    DbECSchemaMap::const_iterator  schemaIterator = m_ecSchemaByECSchemaIdLookup.find (ecSchemaId);
    if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end())
        key =  schemaIterator->second;
    else
        {
        key = new DbECSchemaEntry();
        DbResult r = ECDbSchemaPersistence::ResolveECSchemaId (*key, ecSchemaId, m_db);
        if (r != BE_SQLITE_ROW)
            return r;

        m_ecSchemaByECSchemaIdLookup[key->m_ecSchemaId] = key;
        r = LoadECSchemaFromDb(key->m_resolvedECSchema, ecSchemaId);
        if (r != BE_SQLITE_ROW)
            return r;
        //add to cache as well
        //m_cache.AddSchema (*key->ResolvedECSchema);
        newlyLoadedSchemas.push_back (key);
        DbECSchemaReferenceInfo info;
        info.ColsSelect = DbECSchemaReferenceInfo::COL_ReferencedSchemaId;
        info.ColsWhere = DbECSchemaReferenceInfo::COL_SchemaId;
        info.m_ecSchemaId = ecSchemaId;

        BeSQLite::CachedStatementPtr stmt = nullptr;
        r = ECDbSchemaPersistence::FindECSchemaReferenceInfo (stmt, m_db, info);
        if (r != BE_SQLITE_OK)
            return r;

        DbECSchemaEntryP referenceSchemaKey;
        while ((r = ECDbSchemaPersistence::Step (info, *stmt)) == BE_SQLITE_ROW)
            {
            r = LoadECSchemaDefinition(referenceSchemaKey, newlyLoadedSchemas, info.m_referencedECSchemaId);     
            if (r != BE_SQLITE_ROW)
                return r;

            ECObjectsStatus s = key->m_resolvedECSchema->AddReferencedSchema(*referenceSchemaKey->m_resolvedECSchema); 
            if (s != ECOBJECTS_STATUS_Success)
                return BE_SQLITE_ERROR;
            }
        }

    BeAssert(key->m_resolvedECSchema != nullptr);
    outECSchemaKey = key;
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::ReadECSchema (DbECSchemaEntryP& outECSchemaKey, ECSchemaId ctxECSchemaId, bool ensureAllClassesLoaded)
    {
    BeMutexHolder aGuard (m_criticalSection);
    bvector<DbECSchemaEntryP> newlyLoadedSchemas;
    DbResult r = LoadECSchemaDefinition (outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId);
    if (r != BE_SQLITE_ROW)
        return r;

    for (DbECSchemaEntryP newlyLoadedSchema: newlyLoadedSchemas)
        {
        r = LoadCAFromDb (*(newlyLoadedSchema->m_resolvedECSchema), newlyLoadedSchema->m_ecSchemaId, ECContainerType::Schema);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    if (ensureAllClassesLoaded && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<DbECSchemaEntryP> fullyLoadedSchemas;
        r = LoadECSchemaClassesFromDb (outECSchemaKey, fullyLoadedSchemas);
        if (r != BE_SQLITE_DONE)
            {
            return r;
            }
        
        }
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::GetECSchema (ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool ensureAllClassesLoaded)
    {
    DbECSchemaEntryP outECSchemaKey;
    DbResult r = ReadECSchema (outECSchemaKey, ecSchemaId, ensureAllClassesLoaded);
    if (r != BE_SQLITE_ROW)
        return r;
    ecSchemaOut = outECSchemaKey->m_resolvedECSchema.get();
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::GetECSchema (ECSchemaP& ecSchema, Utf8CP schemaName, bool ensureAllClassesLoaded)
    {
    ecSchema = nullptr;
    ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId(m_db, schemaName); //WIP_FNV: could be more efficient if it first looked through those already cached in memory...
    if (0 == schemaId)
        return BE_SQLITE_DONE;

    return GetECSchema (ecSchema, schemaId, ensureAllClassesLoaded);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
DbResult ECDbSchemaReader::FindECSchemaIdInDb (ECSchemaId& ecSchemaId, Utf8CP schemaName) const
    {
    BeAssert(schemaName);
    ecSchemaId = 0;
    CachedStatementPtr stmt;
    DbResult r = m_db.GetCachedStatement(stmt, "SELECT SchemaId FROM ec_Schema WHERE Name=?");  BeAssert(BE_SQLITE_OK == r);
    r = stmt->BindText(1, schemaName, Statement::MakeCopy::No); BeAssert(BE_SQLITE_OK == r);
    r = stmt->Step();
    if (r == BE_SQLITE_ROW)
        ecSchemaId = stmt->GetValueInt64(0);

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECSchemaClassesFromDb (DbECSchemaEntryP ecSchemaKey, std::set<DbECSchemaEntryP>& fullyLoadedSchemas)
    {
    BeAssert (ecSchemaKey != nullptr);
    if (!ecSchemaKey)
        return BE_SQLITE_ERROR;

    if (fullyLoadedSchemas.find (ecSchemaKey) != fullyLoadedSchemas.end ())
        return BE_SQLITE_DONE;

    DbResult r;
    //Enure all reference schemas also loaded
    for (auto& refSchemaKey : ecSchemaKey->m_resolvedECSchema->GetReferencedSchemas ())
        {
        DbECSchemaEntryP key = nullptr;
        ECSchemaId referenceECSchemaId = refSchemaKey.second->GetId ();
        DbECSchemaMap::const_iterator schemaIterator = m_ecSchemaByECSchemaIdLookup.find (referenceECSchemaId);
        if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end ())
            key = schemaIterator->second;

        if ((r = LoadECSchemaClassesFromDb (key, fullyLoadedSchemas)) != BE_SQLITE_DONE)
            return r;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert (ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded ())
        {
        return  BE_SQLITE_DONE;
        }

    DbECClassInfo info;
    info.ColsWhere = DbECClassInfo::COL_SchemaId;
    info.ColsSelect = DbECClassInfo::COL_Id;
    info.m_ecSchemaId = ecSchemaKey->m_ecSchemaId;
    BeSQLite::CachedStatementPtr stmt = nullptr;
    r = ECDbSchemaPersistence::FindECClassInfo (stmt, m_db, info);
    if (r != BE_SQLITE_OK)
        return r;
      
    while ((r = ECDbSchemaPersistence::Step(info, *stmt)) == BE_SQLITE_ROW)
        {
        ECClassP ecClass = nullptr;
        r = ReadECClass (ecClass, info.m_ecClassId);
        if (r != BE_SQLITE_ROW)
            return r;

        if (ecSchemaKey->IsFullyLoaded())
            {
            r = BE_SQLITE_DONE;
            break;
            }
        }

    return r;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECSchemaFromDb(ECSchemaPtr& ecSchemaOut, ECSchemaId ecSchemaId)
    {
    DbECSchemaInfo info;
    info.ColsWhere = DbECSchemaInfo::COL_Id;
    info.ColsSelect =
        DbECSchemaInfo::COL_Name            |
        DbECSchemaInfo::COL_DisplayLabel    |
        DbECSchemaInfo::COL_Description     |
        DbECSchemaInfo::COL_NamespacePrefix |
        DbECSchemaInfo::COL_VersionMajor    |
        DbECSchemaInfo::COL_VersionMinor;
    info.ColsNull = 0;
    info.m_ecSchemaId = ecSchemaId;        

    BeSQLite::CachedStatementPtr stmt = nullptr;
    DbResult r = ECDbSchemaPersistence::FindECSchemaInfo(stmt, m_db,info);

    if (r != BE_SQLITE_OK)
        return r;

    r = ECDbSchemaPersistence::Step(info, *stmt);
    if (r != BE_SQLITE_ROW)
        return r;

    if (ECSchema::CreateSchema(ecSchemaOut, WString(info.m_name.c_str(), true), info.m_versionMajor, info.m_versionMinor) 
        != ECOBJECTS_STATUS_Success )
        return BE_SQLITE_ERROR;

    ecSchemaOut->SetId(ecSchemaId);
    m_cache.AddSchema(*ecSchemaOut); 
    ecSchemaOut->SetNamespacePrefix (WString (info.m_namespacePrefix.c_str(), true));
    if (!(info.ColsNull & DbECSchemaInfo::COL_DisplayLabel))
        ecSchemaOut->SetDisplayLabel    (WString (info.m_displayLabel.c_str(), true));
    if (!(info.ColsNull & DbECSchemaInfo::COL_Description))
        ecSchemaOut->SetDescription     (WString (info.m_description.c_str(), true));      

    return BE_SQLITE_ROW;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECClassFromDb(ECClassP& ecClassOut, ECClassId ecClassId, ECSchemaR ecSchemaIn)
    {
    DbECClassInfo info;
    info.ColsWhere = DbECClassInfo::COL_Id;
    info.ColsSelect =
        DbECClassInfo::COL_Name                      |
        DbECClassInfo::COL_DisplayLabel              |
        DbECClassInfo::COL_Description               |
        DbECClassInfo::COL_IsCustomAttribute         |
        DbECClassInfo::COL_IsStruct                  |
        DbECClassInfo::COL_IsDomainClass             |
        DbECClassInfo::COL_IsRelationship            |
        DbECClassInfo::COL_RelationStrength          |
        DbECClassInfo::COL_RelationStrengthDirection;

    info.m_ecClassId = ecClassId;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    DbResult r = ECDbSchemaPersistence::FindECClassInfo (stmt, m_db, info);
    if (r != BE_SQLITE_OK)
        return r;
    r = ECDbSchemaPersistence::Step(info, *stmt);
    if (r != BE_SQLITE_ROW)
        return r;

    ECRelationshipClassP ecRelationshipClass = nullptr;
    if (info.m_isRelationship)
        {
        if ( ecSchemaIn.CreateRelationshipClass (ecRelationshipClass, WString(info.m_name.c_str(), true)) != ECOBJECTS_STATUS_Success )
            return BE_SQLITE_ERROR;
        ecClassOut = ecRelationshipClass;
        ecRelationshipClass->SetStrength          (info.m_relationStrength);
        ecRelationshipClass->SetStrengthDirection (info.m_relationStrengthDirection);
        }
    else
        {
        if ( ecSchemaIn.CreateClass (ecClassOut, WString(info.m_name.c_str(), true)) != ECOBJECTS_STATUS_Success )
            return BE_SQLITE_ERROR;
        }

    if (!(info.ColsNull & DbECClassInfo::COL_DisplayLabel))
        ecClassOut->SetDisplayLabel       (WString (info.m_displayLabel.c_str(), true));

    ecClassOut->SetId (ecClassId);
    ecClassOut->SetDescription            (WString (info.m_description.c_str(), true));
    ecClassOut->SetIsStruct               (info.m_isStruct);
    ecClassOut->SetIsCustomAttributeClass (info.m_isCustomAttribute);
    ecClassOut->SetIsDomainClass          (info.m_isDomainClass);

    r = LoadBaseClassesFromDb(ecClassOut, ecClassId);
    if (r != BE_SQLITE_DONE)
        return r;

    r = LoadECPropertiesFromDb(ecClassOut, ecClassId);
    if (r != BE_SQLITE_DONE)
        return r;

    r = LoadCAFromDb(*ecClassOut, ecClassId, ECContainerType::Class);
    if (r != BE_SQLITE_DONE)
        return r;

    if (ecRelationshipClass != nullptr)
        {
        r = LoadECRelationConstraintFromDb (ecRelationshipClass, ecClassId, ECRelationshipEnd_Source);
        if (r != BE_SQLITE_ROW)
            return r;

        r = LoadECRelationConstraintFromDb (ecRelationshipClass, ecClassId, ECRelationshipEnd_Target);
        if (r != BE_SQLITE_ROW)
            return r;
        }
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECPropertiesFromDb(ECClassP& ecClass, ECClassId ecClassId)
    {
    DbECPropertyInfo info;
    info.ColsWhere = DbECPropertyInfo::COL_ClassId;
    info.ColsSelect =
        DbECPropertyInfo::COL_Name |
        DbECPropertyInfo::COL_Id |
        DbECPropertyInfo::COL_DisplayLabel |
        DbECPropertyInfo::COL_Description |
        DbECPropertyInfo::COL_IsArray |
        DbECPropertyInfo::COL_PrimitiveType |
        DbECPropertyInfo::COL_StructType |
        DbECPropertyInfo::COL_IsReadOnly |
        DbECPropertyInfo::COL_MinOccurs |
        DbECPropertyInfo::COL_MaxOccurs;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    info.m_ecClassId = ecClassId;

    DbResult r = ECDbSchemaPersistence::FindECPropertyInfo (stmt, m_db, info);
    if (r != BE_SQLITE_OK)
        return r;

    PrimitiveECPropertyP ecPrimitiveProperty = nullptr;
    ArrayECPropertyP ecArrayProperty = nullptr;
    StructECPropertyP ecStructProperty = nullptr;
    info.m_minOccurs = 0;
    info.m_maxOccurs = UINT32_MAX;

    while ((r = ECDbSchemaPersistence::Step(info, *stmt)) == BE_SQLITE_ROW)
        {
        ECPropertyP ecProperty = nullptr;
        WString name;
        BeStringUtilities::Utf8ToWChar(name, info.m_name.c_str());
        if (info.m_isArray)
            {
            if (~info.ColsNull & DbECPropertyInfo::COL_PrimitiveType)
                {
                if (ECOBJECTS_STATUS_Success != ecClass->CreateArrayProperty (ecArrayProperty, name, info.m_primitiveType))
                    return BE_SQLITE_ERROR;
                }
            else if (~info.ColsNull & DbECPropertyInfo::COL_StructType)
                {
                ECClassP structType;
                r = ReadECClass (structType, info.m_structType);
                if (r != BE_SQLITE_ROW)
                    return r;
                if (ECOBJECTS_STATUS_Success != ecClass->CreateArrayProperty (ecArrayProperty, name, structType))
                    return BE_SQLITE_ERROR;
                }

            if (~info.ColsNull & DbECPropertyInfo::COL_MinOccurs)
                ecArrayProperty->SetMinOccurs (info.m_minOccurs);

            if (~info.ColsNull & DbECPropertyInfo::COL_MaxOccurs)
                ecArrayProperty->SetMaxOccurs (info.m_maxOccurs);

            ecProperty = ecArrayProperty;
            }
        else
            {
            if (~info.ColsNull & DbECPropertyInfo::COL_PrimitiveType)
                {
                if (ECOBJECTS_STATUS_Success != ecClass->CreatePrimitiveProperty(ecPrimitiveProperty, name, info.m_primitiveType))
                    return BE_SQLITE_ERROR;
                ecProperty = ecPrimitiveProperty;
                }
            else if (~info.ColsNull & DbECPropertyInfo::COL_StructType)
                {
                ECClassP structType;
                r = ReadECClass (structType, info.m_structType);
                if (r != BE_SQLITE_ROW)
                    return r;
                if (ECOBJECTS_STATUS_Success != ecClass->CreateStructProperty(ecStructProperty, name, *structType))
                    return BE_SQLITE_ERROR;
                ecProperty = ecStructProperty;
                }
            }
        BeAssert(ecProperty != nullptr);
        ecProperty->SetId(info.m_ecPropertyId); // WIP_FNV
        ecProperty->SetIsReadOnly(info.m_isReadOnly);
        ecProperty->SetDescription (WString(info.m_description.c_str(), true));

        if (!(info.ColsNull & DbECPropertyInfo::COL_DisplayLabel))
            ecProperty->SetDisplayLabel (WString(info.m_displayLabel.c_str(), true));
        //! TODO: What is SetTypeName()????

        r = LoadCAFromDb (*ecProperty, info.m_ecPropertyId, ECContainerType::Property);
        if (r != BE_SQLITE_DONE)
            return r;
        }
    return r;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, ECClassId ecClassId)
    {
    ECDbSchemaPersistence::ECClassIdList baseClassIds;
    DbResult r = ECDbSchemaPersistence::GetBaseECClasses(baseClassIds, ecClassId, m_db);
    if (r != BE_SQLITE_DONE)
        return r;

    ECClassP baseClass;
    for (ECClassId baseClassId : baseClassIds)
        {
        r = ReadECClass (baseClass, baseClassId);
        if (r != BE_SQLITE_ROW)
            return r;
        ECObjectsStatus status =  ecClass->AddBaseClass(*baseClass);
        if (status != ECOBJECTS_STATUS_Success)
            return BE_SQLITE_ERROR;
        }
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType)
    {
    DbCustomAttributeInfo readerInfo;
    readerInfo.ColsWhere  =
        DbCustomAttributeInfo::COL_ContainerId |
        DbCustomAttributeInfo::COL_ContainerType;

    readerInfo.ColsSelect =
        DbCustomAttributeInfo::COL_ClassId |
        DbCustomAttributeInfo::COL_Instance;

    readerInfo.m_containerId = containerId;
    readerInfo.m_containerType = containerType;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    DbResult r = ECDbSchemaPersistence::FindCustomAttributeInfo(stmt, m_db, readerInfo);
    if (r != BE_SQLITE_OK)
        return r;

    readerInfo.Clear();
    while ((r = ECDbSchemaPersistence::Step(readerInfo, *stmt)) == BE_SQLITE_ROW)
        {
        ECClassP caClass = nullptr;
        r = ReadECClass (caClass, readerInfo.m_ecClassId);
        if (r != BE_SQLITE_ROW)
            return r;

        IECInstancePtr inst;
        if (!Utf8String::IsNullOrEmpty(readerInfo.GetCaInstanceXml()) && !(DbCustomAttributeInfo::COL_Instance & readerInfo.ColsNull))
            {
            BentleyStatus stat = readerInfo.DeserializeCaInstance (inst, caClass->GetSchema ());
            if (stat != SUCCESS)
                {
                LOG.errorv(L"Deserializing custom attribute instance from XML failed.");
                return BE_SQLITE_ERROR;
                }
            }
        else
            {
            LOG.errorv(L"Custom attribute defined but its content is missing. It doesn't have a ECInstanceId or corresponding xml.");
            return BE_SQLITE_ERROR;
            }

        if (!inst.IsNull())
            caConstainer.SetCustomAttribute(*inst);
        else
            {
            LOG.errorv(L"Error getting Custom attribute for a container");
            return BE_SQLITE_ERROR;
            }
        readerInfo.Clear();
        }

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECRelationConstraintFromDb (ECRelationshipClassP& ecRelationship, ECClassId ecClassId, ECRelationshipEnd relationshipEnd)
    {
    DbECRelationshipConstraintInfo info;
    info.ColsWhere =
        DbECRelationshipConstraintInfo::COL_ClassId             |
        DbECRelationshipConstraintInfo::COL_RelationshipEnd;

    info.ColsSelect =
        DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit |
        DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit |
        DbECRelationshipConstraintInfo::COL_IsPolymorphic         |
        DbECRelationshipConstraintInfo::COL_RoleLabel ;

    info.ColsNull = 0;

    info.m_ecClassId = ecClassId;
    info.m_ecRelationshipEnd = relationshipEnd;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    DbResult r = ECDbSchemaPersistence::FindECRelationshipConstraintInfo (stmt, m_db, info);
    if (r != BE_SQLITE_OK)
        return r;

    r = ECDbSchemaPersistence::Step (info, *stmt);
    if (r != BE_SQLITE_ROW)
        return r;

    ECRelationshipConstraintR constraint = (relationshipEnd == ECRelationshipEnd_Target) ? ecRelationship->GetTarget() : ecRelationship->GetSource();
    constraint.SetCardinality(RelationshipCardinality(info.m_cardinalityLowerLimit, info.m_cardinalityUpperLimit));
    constraint.SetIsPolymorphic(info.m_isPolymorphic);

    if (!(info.ColsNull & DbECRelationshipConstraintInfo::COL_RoleLabel))
        constraint.SetRoleLabel (WString(info.m_roleLabel.c_str(), true));

    r = LoadECRelationConstraintClassesFromDb(constraint, ecClassId, relationshipEnd);
    if (r != BE_SQLITE_DONE)
        return r;

    ECContainerType containerType = 
        relationshipEnd == ECRelationshipEnd_Target ? ECContainerType::RelationshipConstraintTarget : ECContainerType::RelationshipConstraintSource;

    r = LoadCAFromDb (constraint, ecClassId, containerType);
    if (r != BE_SQLITE_DONE)
        return r;

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaReader::LoadECRelationConstraintClassesFromDb(ECRelationshipConstraintR ecRelationship, ECClassId ecClassId, ECRelationshipEnd relationshipEnd)
    {
    DbECRelationshipConstraintClassInfo info;
    info.ColsWhere =
        DbECRelationshipConstraintClassInfo::COL_ClassId             |
        DbECRelationshipConstraintClassInfo::COL_RelationshipEnd;

    info.ColsSelect = DbECRelationshipConstraintClassInfo::COL_RelationClassId;

    info.ColsNull = 0;

    info.m_ecClassId = ecClassId;
    info.m_ecRelationshipEnd = relationshipEnd;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    DbResult r = ECDbSchemaPersistence::FindECRelationConstraintClassInfo (stmt, m_db, info);
    if (r != BE_SQLITE_OK)
        return r;
    ECClassP relationEndClass;
    while ((r = ECDbSchemaPersistence::Step(info, *stmt)) == BE_SQLITE_ROW)
        {
        r = ReadECClass(relationEndClass, info.m_relationECClassId);
        if (r != BE_SQLITE_ROW)
            return r;
        ECRelationshipConstraintClassP ecRelationShipconstraintClass;
        ecRelationship.AddConstraintClass(ecRelationShipconstraintClass,*relationEndClass);
        if (ecRelationShipconstraintClass != nullptr)
            {
            CachedStatementPtr statement;
            Utf8CP sql = "SELECT P.NAME FROM ec_RelationshipConstraintClassProperty I INNER JOIN ec_Property P on P.Id = I.RelationPropertyId WHERE I.ClassId = ? AND I.RelationshipEnd = ? AND I.RelationClassId = ? ";
            m_db.GetCachedStatement(statement, sql);
            statement->BindInt64(1, ecClassId);
            statement->BindInt(2, relationshipEnd);
            statement->BindInt64(3, relationEndClass->GetId());
            while ((r = statement->Step() )== BE_SQLITE_ROW)
                {
                ecRelationShipconstraintClass->AddKey(WString(statement->GetValueText(0),BentleyCharEncoding::Utf8).c_str());
                }
            }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaReader::ECDbSchemaReader (Db& db)
        :m_db(db), m_loadOnlyPrimaryCustomAttributes(false)
        {
        }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaReaderPtr ECDbSchemaReader::Create(Db& db)
    {
    return new ECDbSchemaReader(db);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECDbSchemaReader::GetECClass(ECClassId ecClassId)
    {
    ECClassP ecClass;
    if (ReadECClass (ecClass, ecClassId) == BE_SQLITE_ROW)
        return ecClass;
    return nullptr;
    }
ECClassP ECDbSchemaReader::GetECClass (Utf8CP schemaNameOrPrefix, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement(stmt, "SELECT c.Id FROM ec_Class c JOIN ec_Schema s WHERE c.SchemaId = s.Id AND (s.Name = ?1 OR s.NamespacePrefix = ?1) AND c.Name = ?2");
    stmt->BindText (1, schemaNameOrPrefix, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    DbResult r = stmt->Step();
    if (BE_SQLITE_ROW != r)
        return nullptr;
    return GetECClass(stmt->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaReader::GetECClass(ECClassP& ecClass, ECClassId ecClassId)
    {

    return ReadECClass (ecClass, ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::ClearCache ()
    {
    BeMutexHolder aGuard (m_criticalSection);
    for (DbECClassEntryMap::reference pair : m_ecClassKeyByECClassIdLookup)
        delete pair.second;

    m_ecClassKeyByECClassIdLookup.clear ();

    for (DbECSchemaMap::reference pair : m_ecSchemaByECSchemaIdLookup)
        delete pair.second;

    m_ecSchemaByECSchemaIdLookup.clear ();

    m_cache.Clear ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaReader::GetECClassBySchemaName (ECClassP& ecClass, Utf8CP schemaName, Utf8CP className)
    {
    ECClassId ecClassId = ECDbSchemaPersistence::GetECClassIdBySchemaName(m_db, schemaName, className); // needswork: if this is a performance issue, try to look it up in-memory, first
    return GetECClass (ecClass, ecClassId);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaReader::GetECClassBySchemaNameSpacePrefix(ECClassP& ecClass, Utf8CP schemaName, Utf8CP className)
    {
    ECClassId ecClassId = ECDbSchemaPersistence::GetECClassIdBySchemaNameSpacePrefix(m_db, schemaName, className); // needswork: if this is a performance issue, try to look it up in-memory, first
    return GetECClass(ecClass, ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaReader::~ECDbSchemaReader ()
    {
    ClearCache ();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
