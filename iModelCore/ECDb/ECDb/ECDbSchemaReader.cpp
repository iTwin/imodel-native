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
ECDbSchemaReader::~ECDbSchemaReader()
    {
    ClearCache();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaReaderPtr ECDbSchemaReader::Create(ECDbCR db)
    {
    return new ECDbSchemaReader(db);
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::AddECSchemaToCache (ECSchemaCR schema)
    {
    AddECSchemaToCacheInternal (schema);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::AddECSchemaToCacheInternal (ECSchemaCR schema)
    {
    BeMutexHolder aGuard (m_criticalSection);
    ECSchemaId ecSchemaId = schema.GetId();
    DbECSchemaEntry* schemaKey = FindDbECSchemaEntry (ecSchemaId);
    if (schemaKey == nullptr)
        {
        // build key lookups
        schemaKey                        = new DbECSchemaEntry();
        schemaKey->m_ecSchemaId          = ecSchemaId;
        schemaKey->m_resolvedECSchema    = const_cast<ECSchemaP>(&schema);
        schemaKey->m_schemaName          = schema.GetName().c_str();
        schemaKey->m_versionMajor        = schema.GetVersionMajor();
        schemaKey->m_versionMinor        = schema.GetVersionMinor();
        schemaKey->m_nClassesInSchema = schemaKey->m_nClassesLoaded = 0;
        m_ecSchemaByECSchemaIdLookup[ecSchemaId] = schemaKey;
        //m_ecSchemaByNameLookup[schema.GetName().c_str()] = schemaKey;
        for (ECClassCP ecClass : schemaKey->m_resolvedECSchema->GetClasses())
            {
            DbECClassEntry* classKey    = new DbECClassEntry();
            if (!ecClass->HasId())
                const_cast<ECClassP>(ecClass)->SetId (
                    ECDbSchemaPersistence::GetECClassId(m_db, 
                        ecClass->GetSchema().GetName().c_str(),
                        ecClass->GetName().c_str(), ResolveSchema::BySchemaName));
            classKey->m_ecClassId       = ecClass->GetId();
            classKey->m_ecSchemaId      = ecSchemaId;
            classKey->m_className       = ecClass->GetName().c_str();

            classKey->m_resolvedECClass = const_cast<ECClassP>(ecClass);
            m_ecClassKeyByECClassIdLookup[classKey->m_ecClassId] = classKey;
            schemaKey->m_nClassesInSchema++;
            }
        schemaKey->m_nClassesLoaded = schemaKey->m_nClassesInSchema;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECClass(ECClassP& ecClass, ECClassId ecClassId)
    {
    if (ecClassId == ECClass::UNSET_ECCLASSID)
        {
        ecClass = nullptr;
        return ERROR;
        }

    BeMutexHolder lock (m_criticalSection);

    DbECClassEntry* key = nullptr;
    DbECClassEntryMap::const_iterator  classKeyIterator = m_ecClassKeyByECClassIdLookup.find (ecClassId);
    if (classKeyIterator != m_ecClassKeyByECClassIdLookup.end())
        key = classKeyIterator->second; 
    else
        {
        key = new DbECClassEntry();
        key->m_resolvedECClass = nullptr;
        if (SUCCESS != ECDbSchemaPersistence::ResolveECClassId (*key, ecClassId, m_db))
            return ERROR;

        m_ecClassKeyByECClassIdLookup[key->m_ecClassId] = key;

        DbECSchemaEntry* outECSchemaKey;
        if (SUCCESS != ReadECSchema(outECSchemaKey, key->m_ecSchemaId, false))
            return ERROR;

        if (SUCCESS != LoadECClassFromDb(key->m_resolvedECClass, ecClassId, *outECSchemaKey->m_resolvedECSchema))
            return ERROR;
        

        outECSchemaKey->m_nClassesLoaded++;
        }

    if (key->m_resolvedECClass == nullptr)
        return ERROR;

    ecClass = key->m_resolvedECClass;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbECClassEntry* ECDbSchemaReader::FindDbECClassEntry(ECClassId ecClassId)
    {
    DbECClassEntryMap::const_iterator  classKeyIterator = m_ecClassKeyByECClassIdLookup.find (ecClassId);
    if (classKeyIterator != m_ecClassKeyByECClassIdLookup.end())
        return classKeyIterator->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbECSchemaEntry* ECDbSchemaReader::FindDbECSchemaEntry(ECSchemaId ecSchemaId)
    {
    DbECSchemaMap::const_iterator  schemaIterator = m_ecSchemaByECSchemaIdLookup.find (ecSchemaId);
    if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end())
        return schemaIterator->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaDefinition(DbECSchemaEntry*& outECSchemaKey, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ecSchemaId)
    {
    DbECSchemaEntry* key = nullptr;
    DbECSchemaMap::const_iterator  schemaIterator = m_ecSchemaByECSchemaIdLookup.find (ecSchemaId);
    if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end())
        key =  schemaIterator->second;
    else
        {
        key = new DbECSchemaEntry();
        if (SUCCESS != ECDbSchemaPersistence::ResolveECSchemaId (*key, ecSchemaId, m_db))
            return ERROR;

        m_ecSchemaByECSchemaIdLookup[key->m_ecSchemaId] = key;
        if (SUCCESS != LoadECSchemaFromDb(key->m_resolvedECSchema, ecSchemaId))
            return ERROR;

        bvector<ECSchemaId> referencedSchemaIds;
        if (SUCCESS != ECDbSchemaPersistence::GetReferencedSchemas(referencedSchemaIds, m_db, ecSchemaId))
            return ERROR;

        newlyLoadedSchemas.push_back(key);
        for (ECSchemaId referencedSchemaId : referencedSchemaIds)
            {
            DbECSchemaEntry* referenceSchemaKey = nullptr;
            if (SUCCESS != LoadECSchemaDefinition(referenceSchemaKey, newlyLoadedSchemas, referencedSchemaId))
                return ERROR;

            ECObjectsStatus s = key->m_resolvedECSchema->AddReferencedSchema(*referenceSchemaKey->m_resolvedECSchema);
            if (s != ECOBJECTS_STATUS_Success)
                return ERROR;
            }
        }

    BeAssert(key->m_resolvedECSchema != nullptr);
    outECSchemaKey = key;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECSchema(DbECSchemaEntry*& outECSchemaKey, ECSchemaId ctxECSchemaId, bool ensureAllClassesLoaded)
    {
    BeMutexHolder lock (m_criticalSection);
    bvector<DbECSchemaEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadECSchemaDefinition (outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId))
        return ERROR;

    for (DbECSchemaEntry* newlyLoadedSchema : newlyLoadedSchemas)
        {
        if (SUCCESS != LoadCAFromDb(*(newlyLoadedSchema->m_resolvedECSchema), newlyLoadedSchema->m_ecSchemaId, ECContainerType::Schema))
            return ERROR;
        }

    if (ensureAllClassesLoaded && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<DbECSchemaEntry*> fullyLoadedSchemas;
        if (SUCCESS != LoadECSchemaClassesFromDb(outECSchemaKey, fullyLoadedSchemas))
            return ERROR;
        
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::GetECSchema(ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool ensureAllClassesLoaded)
    {
    DbECSchemaEntry* outECSchemaKey;
    if (SUCCESS != ReadECSchema(outECSchemaKey, ecSchemaId, ensureAllClassesLoaded))
        return ERROR;

    ecSchemaOut = outECSchemaKey->m_resolvedECSchema.get();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaReader::FindECSchemaIdInDb(ECSchemaId& ecSchemaId, Utf8CP schemaName) const
    {
    BeAssert(schemaName);
    ecSchemaId = 0;
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT SchemaId FROM ec_Schema WHERE Name=?"))
        {
        BeAssert(false);
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(1, schemaName, Statement::MakeCopy::No))
        {
        BeAssert(false);
        return ERROR;
        }

    if (stmt->Step() == BE_SQLITE_ROW)
        ecSchemaId = stmt->GetValueInt64(0);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaClassesFromDb(DbECSchemaEntry* ecSchemaKey, std::set<DbECSchemaEntry*>& fullyLoadedSchemas)
    {
    BeAssert (ecSchemaKey != nullptr);
    if (!ecSchemaKey)
        return ERROR;

    if (fullyLoadedSchemas.find (ecSchemaKey) != fullyLoadedSchemas.end ())
        return SUCCESS;

    //Enure all reference schemas also loaded
    for (auto& refSchemaKey : ecSchemaKey->m_resolvedECSchema->GetReferencedSchemas ())
        {
        DbECSchemaEntry* key = nullptr;
        ECSchemaId referenceECSchemaId = refSchemaKey.second->GetId ();
        DbECSchemaMap::const_iterator schemaIterator = m_ecSchemaByECSchemaIdLookup.find (referenceECSchemaId);
        if (schemaIterator != m_ecSchemaByECSchemaIdLookup.end ())
            key = schemaIterator->second;

        if (SUCCESS != LoadECSchemaClassesFromDb(key, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert (ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded ())
        return SUCCESS;

    DbECClassInfo info;
    info.ColsWhere = DbECClassInfo::COL_SchemaId;
    info.ColsSelect = DbECClassInfo::COL_Id;
    info.m_ecSchemaId = ecSchemaKey->m_ecSchemaId;
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (SUCCESS != ECDbSchemaPersistence::FindECClass(stmt, m_db, info))
        return ERROR;
      
    while (ECDbSchemaPersistence::Step(info, *stmt) == BE_SQLITE_ROW)
        {
        ECClassP ecClass = nullptr;
        if (SUCCESS != ReadECClass (ecClass, info.m_ecClassId))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaFromDb(ECSchemaPtr& ecSchemaOut, ECSchemaId ecSchemaId)
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

    CachedStatementPtr stmt = nullptr;
    if (SUCCESS != ECDbSchemaPersistence::FindECSchema(stmt, m_db, info))
        return ERROR;

    if (BE_SQLITE_ROW != ECDbSchemaPersistence::Step(info, *stmt))
        return ERROR;

    if (ECSchema::CreateSchema(ecSchemaOut, info.m_name.c_str(), info.m_versionMajor, info.m_versionMinor) 
        != ECOBJECTS_STATUS_Success )
        return ERROR;

    ecSchemaOut->SetId(ecSchemaId);
    m_cache.AddSchema(*ecSchemaOut); 
    ecSchemaOut->SetNamespacePrefix(info.m_namespacePrefix.c_str());
    if (!(info.ColsNull & DbECSchemaInfo::COL_DisplayLabel))
        ecSchemaOut->SetDisplayLabel(info.m_displayLabel.c_str());
    if (!(info.ColsNull & DbECSchemaInfo::COL_Description))
        ecSchemaOut->SetDescription(info.m_description.c_str());      

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECClassFromDb(ECClassP& ecClassOut, ECClassId ecClassId, ECSchemaR ecSchemaIn)
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
    if (SUCCESS != ECDbSchemaPersistence::FindECClass (stmt, m_db, info))
        return ERROR;

    if (BE_SQLITE_ROW != ECDbSchemaPersistence::Step(info, *stmt))
        return ERROR;

    ECRelationshipClassP ecRelationshipClass = nullptr;
    if (info.m_isRelationship)
        {
        if ( ecSchemaIn.CreateRelationshipClass (ecRelationshipClass, info.m_name.c_str()) != ECOBJECTS_STATUS_Success )
            return ERROR;
        ecClassOut = ecRelationshipClass;
        ecRelationshipClass->SetStrength          (info.m_relationStrength);
        ecRelationshipClass->SetStrengthDirection (info.m_relationStrengthDirection);
        }
    else
        {
        if ( ecSchemaIn.CreateClass (ecClassOut, info.m_name.c_str()) != ECOBJECTS_STATUS_Success )
            return ERROR;
        }

    if (!(info.ColsNull & DbECClassInfo::COL_DisplayLabel))
        ecClassOut->SetDisplayLabel(info.m_displayLabel.c_str());

    ecClassOut->SetId (ecClassId);
    ecClassOut->SetDescription(info.m_description.c_str());
    ecClassOut->SetIsStruct(info.m_isStruct);
    ecClassOut->SetIsCustomAttributeClass(info.m_isCustomAttribute);
    ecClassOut->SetIsDomainClass(info.m_isDomainClass);

    if (SUCCESS != LoadBaseClassesFromDb(ecClassOut, ecClassId))
        return ERROR;

    if (SUCCESS != LoadECPropertiesFromDb(ecClassOut, ecClassId))
        return ERROR;

    if (SUCCESS != LoadCAFromDb(*ecClassOut, ecClassId, ECContainerType::Class))
        return ERROR;

    if (ecRelationshipClass != nullptr)
        {
        if (SUCCESS != LoadECRelationshipConstraintFromDb(ecRelationshipClass, ecClassId, ECRelationshipEnd_Source))
            return ERROR;

        if (SUCCESS != LoadECRelationshipConstraintFromDb(ecRelationshipClass, ecClassId, ECRelationshipEnd_Target))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECPropertiesFromDb(ECClassP& ecClass, ECClassId ecClassId)
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
        DbECPropertyInfo::COL_IsReadonly |
        DbECPropertyInfo::COL_MinOccurs |
        DbECPropertyInfo::COL_MaxOccurs;

    BeSQLite::CachedStatementPtr stmt = nullptr;
    info.m_ecClassId = ecClassId;

    if (SUCCESS != ECDbSchemaPersistence::FindECProperty (stmt, m_db, info))
        return ERROR;

    PrimitiveECPropertyP ecPrimitiveProperty = nullptr;
    ArrayECPropertyP ecArrayProperty = nullptr;
    StructECPropertyP ecStructProperty = nullptr;
    info.m_minOccurs = 0;
    info.m_maxOccurs = UINT32_MAX;

    while (ECDbSchemaPersistence::Step(info, *stmt) == BE_SQLITE_ROW)
        {
        ECPropertyP ecProperty = nullptr;
        if (info.m_isArray)
            {
            if (~info.ColsNull & DbECPropertyInfo::COL_PrimitiveType)
                {
                if (ECOBJECTS_STATUS_Success != ecClass->CreateArrayProperty (ecArrayProperty, info.m_name, info.m_primitiveType))
                    return ERROR;
                }
            else if (~info.ColsNull & DbECPropertyInfo::COL_StructType)
                {
                ECClassP structType;
                if (SUCCESS != ReadECClass (structType, info.m_structType))
                    return ERROR;

                if (ECOBJECTS_STATUS_Success != ecClass->CreateArrayProperty (ecArrayProperty, info.m_name, structType))
                    return ERROR;
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
                if (ECOBJECTS_STATUS_Success != ecClass->CreatePrimitiveProperty(ecPrimitiveProperty, info.m_name, info.m_primitiveType))
                    return ERROR;

                ecProperty = ecPrimitiveProperty;
                }
            else if (~info.ColsNull & DbECPropertyInfo::COL_StructType)
                {
                ECClassP structType;
                if (SUCCESS != ReadECClass(structType, info.m_structType))
                    return ERROR;

                if (ECOBJECTS_STATUS_Success != ecClass->CreateStructProperty(ecStructProperty, info.m_name, *structType))
                    return ERROR;

                ecProperty = ecStructProperty;
                }
            }
        BeAssert(ecProperty != nullptr);
        ecProperty->SetId(info.m_ecPropertyId); // WIP_FNV
        ecProperty->SetIsReadOnly(info.m_isReadOnly);
        ecProperty->SetDescription (info.m_description.c_str());

        if (!(info.ColsNull & DbECPropertyInfo::COL_DisplayLabel))
            ecProperty->SetDisplayLabel (info.m_displayLabel.c_str());

        if (SUCCESS != LoadCAFromDb (*ecProperty, info.m_ecPropertyId, ECContainerType::Property))
            return ERROR;
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, ECClassId ecClassId)
    {
    ECDbSchemaPersistence::ECClassIdList baseClassIds;
    if (SUCCESS != ECDbSchemaPersistence::GetBaseECClasses(baseClassIds, ecClassId, m_db))
        return ERROR;

    ECClassP baseClass;
    for (ECClassId baseClassId : baseClassIds)
        {
        if (SUCCESS != ReadECClass(baseClass, baseClassId))
            return ERROR;

        if (ECOBJECTS_STATUS_Success != ecClass->AddBaseClass(*baseClass))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType)
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

    CachedStatementPtr stmt = nullptr;
    if (SUCCESS != ECDbSchemaPersistence::FindCustomAttribute(stmt, m_db, readerInfo))
        return ERROR;

    readerInfo.Clear();
    while (ECDbSchemaPersistence::Step(readerInfo, *stmt) == BE_SQLITE_ROW)
        {
        ECClassP caClass = nullptr;
        if (SUCCESS != ReadECClass(caClass, readerInfo.m_ecClassId))
            return ERROR;

        IECInstancePtr inst;
        if (!Utf8String::IsNullOrEmpty(readerInfo.GetCaInstanceXml()) && !(DbCustomAttributeInfo::COL_Instance & readerInfo.ColsNull))
            {
            if (SUCCESS != readerInfo.DeserializeCaInstance (inst, caClass->GetSchema ()))
                {
                LOG.error("Deserializing custom attribute instance from XML failed.");
                return ERROR;
                }
            }
        else
            {
            LOG.error("Custom attribute defined but its content is missing. It doesn't have a ECInstanceId or corresponding xml.");
            return ERROR;
            }

        if (!inst.IsNull())
            caConstainer.SetCustomAttribute(*inst);
        else
            {
            LOG.error("Error getting Custom attribute for a container");
            return ERROR;
            }
        readerInfo.Clear();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintFromDb(ECRelationshipClassP& ecRelationship, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd)
    {
    DbECRelationshipConstraintInfo info;
    info.ColsWhere =
        DbECRelationshipConstraintInfo::COL_RelationshipClassId   |
        DbECRelationshipConstraintInfo::COL_RelationshipEnd;

    info.ColsSelect =
        DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit |
        DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit |
        DbECRelationshipConstraintInfo::COL_IsPolymorphic         |
        DbECRelationshipConstraintInfo::COL_RoleLabel ;

    info.ColsNull = 0;

    info.m_relationshipClassId = relationshipClassId;
    info.m_ecRelationshipEnd = relationshipEnd;

    CachedStatementPtr stmt = nullptr;
    if (SUCCESS != ECDbSchemaPersistence::FindECRelationshipConstraint(stmt, m_db, info))
        return ERROR;

    if (BE_SQLITE_ROW != ECDbSchemaPersistence::Step (info, *stmt))
        return ERROR;

    ECRelationshipConstraintR constraint = (relationshipEnd == ECRelationshipEnd_Target) ? ecRelationship->GetTarget() : ecRelationship->GetSource();
    constraint.SetCardinality(RelationshipCardinality(info.m_cardinalityLowerLimit, info.m_cardinalityUpperLimit));
    constraint.SetIsPolymorphic(info.m_isPolymorphic);

    if (!(info.ColsNull & DbECRelationshipConstraintInfo::COL_RoleLabel))
        constraint.SetRoleLabel(info.m_roleLabel);

    if (SUCCESS != LoadECRelationshipConstraintClassesFromDb(constraint, relationshipClassId, relationshipEnd))
        return ERROR;

    ECContainerType containerType = 
        relationshipEnd == ECRelationshipEnd_Target ? ECContainerType::RelationshipConstraintTarget : ECContainerType::RelationshipConstraintSource;

    return LoadCAFromDb(constraint, relationshipClassId, containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR constraint, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd)
    {
    DbECRelationshipConstraintClassInfo info;
    info.ColsWhere =
        DbECRelationshipConstraintClassInfo::COL_RelationshipClassId |
        DbECRelationshipConstraintClassInfo::COL_RelationshipEnd;

    info.ColsSelect = DbECRelationshipConstraintClassInfo::COL_ConstraintClassId;

    info.ColsNull = 0;

    info.m_relationshipClassId = relationshipClassId;
    info.m_ecRelationshipEnd = relationshipEnd;

    CachedStatementPtr stmt = nullptr;
    if (SUCCESS != ECDbSchemaPersistence::FindECRelationshipConstraintClass(stmt, m_db, info))
        return ERROR;

    while (ECDbSchemaPersistence::Step(info, *stmt) == BE_SQLITE_ROW)
        {
        const ECClassId constraintClassId = info.m_constraintClassId;
        ECClassP constraintClass = nullptr;
        if (SUCCESS != ReadECClass(constraintClass, constraintClassId))
            return ERROR;

        ECRelationshipConstraintClassP constraintClassObj = nullptr;
        constraint.AddConstraintClass(constraintClassObj, *constraintClass);
        if (constraintClassObj != nullptr)
            {
            CachedStatementPtr statement;
            Utf8CP sql = "SELECT KeyPropertyName FROM ec_RelationshipConstraintClassKeyProperty WHERE RelationshipClassId = ? AND ConstraintClassId = ? AND RelationshipEnd = ?";
            m_db.GetCachedStatement(statement, sql);
            statement->BindInt64(1, relationshipClassId);
            statement->BindInt64(2, constraintClassId);
            statement->BindInt(3, relationshipEnd);
            while (statement->Step() == BE_SQLITE_ROW)
                {
                constraintClassObj->AddKey(statement->GetValueText(0));
                }
            }
        }

    return SUCCESS;
    }



/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECDbSchemaReader::GetECClass(ECClassId ecClassId)
    {
    ECClassP ecClass;
    if (ReadECClass (ecClass, ecClassId) == SUCCESS)
        return ecClass;

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaReader::TryGetECClassId(ECClassId& id, Utf8CP schemaName, Utf8CP className, ResolveSchema resolveSchema) const
    {
    ECClassId ecClassId = ECDbSchemaPersistence::GetECClassId(m_db, schemaName, className, resolveSchema); // needswork: if this is a performance issue, try to look it up in-memory, first
    if (ecClassId == ECClass::UNSET_ECCLASSID)
        return false;

    id = ecClassId;
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::ClearCache ()
    {
    BeMutexHolder lock (m_criticalSection);
    for (DbECClassEntryMap::reference pair : m_ecClassKeyByECClassIdLookup)
        delete pair.second;

    m_ecClassKeyByECClassIdLookup.clear ();

    for (DbECSchemaMap::reference pair : m_ecSchemaByECSchemaIdLookup)
        delete pair.second;

    m_ecSchemaByECSchemaIdLookup.clear ();

    m_cache.Clear ();
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
