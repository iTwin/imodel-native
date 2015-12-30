/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
    BeMutexHolder mutex(m_criticalSection);
    ECSchemaId ecSchemaId = schema.GetId();
    if (m_ecSchemaCache.find(ecSchemaId) != m_ecSchemaCache.end())
        return;

    unique_ptr<DbECSchemaEntry> schemaEntry = unique_ptr<DbECSchemaEntry>(new DbECSchemaEntry(schema));
    DbECSchemaEntry* schemaEntryP = schemaEntry.get();
    m_ecSchemaCache[ecSchemaId] = move(schemaEntry);

    for (ECClassCP ecClass : schemaEntryP->m_cachedECSchema->GetClasses())
        {
        if (!ecClass->HasId())
            const_cast<ECClassP>(ecClass)->SetId(
                ECDbSchemaPersistence::GetECClassId(m_db,
                                                    ecClass->GetSchema().GetName().c_str(),
                                                    ecClass->GetName().c_str(), ResolveSchema::BySchemaName));

        unique_ptr<DbECClassEntry> classEntry = unique_ptr<DbECClassEntry>(new DbECClassEntry(ecSchemaId, *ecClass));
        m_ecClassCache[classEntry->m_ecClassId] = std::move(classEntry);
        schemaEntryP->m_nTypesInSchema++;
        }

    for (ECEnumerationCP ecEnum : schemaEntryP->m_cachedECSchema->GetEnumerations())
        {
        unique_ptr<DbECEnumEntry> enumEntry = unique_ptr<DbECEnumEntry>(new DbECEnumEntry(ecSchemaId, *ecEnum));
        DbECEnumEntry* enumEntryP = enumEntry.get();
        m_ecEnumCache[enumEntryP->m_enumName] = std::move(enumEntry);
        schemaEntryP->m_nTypesInSchema++;
        }

    schemaEntryP->m_nTypesLoaded = schemaEntryP->m_nTypesInSchema;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECClass(ECClassP& ecClass, ECClassId ecClassId) const
    {
    if (ecClassId == ECClass::UNSET_ECCLASSID)
        {
        ecClass = nullptr;
        return ERROR;
        }

    BeMutexHolder lock (m_criticalSection);

    DbECClassEntryMap::const_iterator classKeyIterator = m_ecClassCache.find (ecClassId);
    if (classKeyIterator != m_ecClassCache.end())
        {
        ecClass = classKeyIterator->second->m_cachedECClass;
        return SUCCESS;
        }

    const int schemaIdColIx = 0;
    const int nameColIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int modifierColIx = 5;
    const int relStrengthColIx = 6;
    const int relStrengthDirColIx = 7;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT SchemaId, Name,DisplayLabel,Description,Type,Modifier,RelationStrength,RelationStrengthDirection FROM ec_Class WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    ECSchemaId schemaId = stmt->GetValueInt64(schemaIdColIx);
    Utf8CP className = stmt->GetValueText(nameColIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    ECClassType classType = Enum::FromInt<ECClassType>(stmt->GetValueInt(typeColIx));
    ECClassModifier classModifier = Enum::FromInt<ECClassModifier>(stmt->GetValueInt(modifierColIx));

    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, schemaId, false))
        return ERROR;

    ECSchemaR schema = *schemaKey->m_cachedECSchema;
    switch (classType)
        {
            case ECClassType::CustomAttribute:
            {
            ECCustomAttributeClassP newClass = nullptr;
            if (schema.CreateCustomAttributeClass(newClass, className) != ECObjectsStatus::Success)
                return ERROR;

            ecClass = newClass;
            break;
            }

            case ECClassType::Entity:
            {
            ECEntityClassP newClass = nullptr;
            if (schema.CreateEntityClass(newClass, className) != ECObjectsStatus::Success)
                return ERROR;

            ecClass = newClass;
            break;
            }

            case ECClassType::Struct:
            {
            ECStructClassP newClass = nullptr;
            if (schema.CreateStructClass(newClass, className) != ECObjectsStatus::Success)
                return ERROR;

            ecClass = newClass;
            break;
            }

            case ECClassType::Relationship:
            {
            ECRelationshipClassP newClass = nullptr;
            if (schema.CreateRelationshipClass(newClass, className) != ECObjectsStatus::Success)
                return ERROR;

            BeAssert(!stmt->IsColumnNull(relStrengthColIx) && !stmt->IsColumnNull(relStrengthDirColIx));
            newClass->SetStrength(Enum::FromInt<StrengthType>(stmt->GetValueInt(relStrengthColIx)));
            newClass->SetStrengthDirection(Enum::FromInt<ECRelatedInstanceDirection>(stmt->GetValueInt(relStrengthDirColIx)));
            ecClass = newClass;
            break;
            }

            default:
                BeAssert(false);
                return ERROR;
        }

    ecClass->SetId(ecClassId);
    ecClass->SetClassModifier(classModifier);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        ecClass->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        ecClass->SetDescription(description);

    BeAssert(stmt->Step() == BE_SQLITE_DONE);
    stmt = nullptr; //to release it, so that it can be reused without repreparation

    if (SUCCESS != LoadBaseClassesFromDb(ecClass, ecClassId))
        return ERROR;

    if (SUCCESS != LoadECPropertiesFromDb(ecClass, ecClassId))
        return ERROR;

    if (SUCCESS != LoadCAFromDb(*ecClass, ecClassId, ECContainerType::Class))
        return ERROR;

    ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
    if (relClass != nullptr)
        {
        if (SUCCESS != LoadECRelationshipConstraintFromDb(relClass, ecClassId, ECRelationshipEnd_Source))
            return ERROR;

        if (SUCCESS != LoadECRelationshipConstraintFromDb(relClass, ecClassId, ECRelationshipEnd_Target))
            return ERROR;
        }

    schemaKey->m_nTypesLoaded++;

    //cache the class
    m_ecClassCache[ecClassId] = unique_ptr<DbECClassEntry>(new DbECClassEntry(schemaId, *ecClass));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Krischan.Eberle    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECEnumeration(ECEnumerationP& ecEnum, ECN::ECSchemaId schemaId, Utf8CP enumName) const
    {
    if (Utf8String::IsNullOrEmpty(enumName))
        {
        ecEnum = nullptr;
        return ERROR;
        }

    BeMutexHolder lock(m_criticalSection);

    auto enumEntryIt = m_ecEnumCache.find(enumName);
    if (enumEntryIt != m_ecEnumCache.end())
        {
        ecEnum = enumEntryIt->second->m_cachedECEnum;
        return SUCCESS;
        }

    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, schemaId, false))
        return ERROR;

    const int displayLabelColIx = 0;
    const int descriptionColIx = 1;
    const int typeColIx = 2;
    const int isStrictColIx = 3;
    const int valuesColIx = 4;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration WHERE SchemaId=? AND Name=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, schemaId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, enumName, Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    PrimitiveType underlyingType = (PrimitiveType) stmt->GetValueInt(typeColIx);
    bool isStrict = stmt->GetValueInt(isStrictColIx) != 0;
    Utf8CP enumValuesJsonStr = stmt->GetValueText(valuesColIx);

    ecEnum = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedECSchema->CreateEnumeration(ecEnum, enumName, underlyingType))
        return ERROR;

    if (displayLabel != nullptr)
        ecEnum->SetDisplayLabel(displayLabel);

    ecEnum->SetDescription(description);
    ecEnum->SetIsStrict(isStrict);

    if (Utf8String::IsNullOrEmpty(enumValuesJsonStr))
        {
        BeAssert(false);
        return ERROR;
        }

    rapidjson::Document enumValuesJson;
    if (enumValuesJson.Parse<0>(enumValuesJsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse ECEnumeration values JSON string.");
        return ERROR;
        }

    BeAssert(enumValuesJson.IsArray());
    for (auto it = enumValuesJson.Begin(); it != enumValuesJson.End(); ++it)
        {
        rapidjson::Value const& enumValueJson = *it;
        BeAssert(enumValueJson.IsArray());
        const rapidjson::SizeType enumValueMemberCount = enumValueJson.Size();
        BeAssert(enumValueMemberCount == 1 || enumValueMemberCount == 2);

        rapidjson::Value const& val = enumValueJson[(rapidjson::SizeType) 0];

        ECEnumeratorP enumValue = nullptr;
        
        if (val.IsInt())
            {
            if (ECObjectsStatus::Success != ecEnum->CreateEnumerator(enumValue, val.GetInt()))
                return ERROR;
            }
        else if (val.IsString())
            {
            if (ECObjectsStatus::Success != ecEnum->CreateEnumerator(enumValue, val.GetString()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported underlying ECEnumeration type");
            return ERROR;
            }

        if (enumValueMemberCount == 2)
            {
            BeAssert(enumValueJson[1].IsString());
            enumValue->SetDisplayLabel(enumValueJson[1].GetString());
            }
        }


    //cache the enum
    auto enumEntry = unique_ptr<DbECEnumEntry>(new DbECEnumEntry(schemaId, *ecEnum));
    DbECEnumEntry* enumEntryP = enumEntry.get();
    m_ecEnumCache[enumEntryP->m_enumName] = move(enumEntry);

    schemaKey->m_nTypesLoaded++;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaDefinition(DbECSchemaEntry*& outECSchemaKey, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ecSchemaId) const
    {
    auto schemaIterator = m_ecSchemaCache.find(ecSchemaId);
    if (schemaIterator != m_ecSchemaCache.end())
        {
        BeAssert(schemaIterator->second->m_cachedECSchema != nullptr);
        outECSchemaKey = schemaIterator->second.get();
        return SUCCESS;
        }

    unique_ptr<DbECSchemaEntry> schemaEntry = unique_ptr<DbECSchemaEntry>(new DbECSchemaEntry());
    if (SUCCESS != ECDbSchemaPersistence::ResolveECSchemaId(*schemaEntry, ecSchemaId, m_db))
        return ERROR;

    DbECSchemaEntry* schemaEntryP = schemaEntry.get();
    m_ecSchemaCache[ecSchemaId] = move(schemaEntry);

    if (SUCCESS != LoadECSchemaFromDb(schemaEntryP->m_cachedECSchema, ecSchemaId))
        return ERROR;

    bvector<ECSchemaId> referencedSchemaIds;
    if (SUCCESS != ECDbSchemaPersistence::GetReferencedSchemas(referencedSchemaIds, m_db, ecSchemaId))
        return ERROR;

    newlyLoadedSchemas.push_back(schemaEntryP);
    for (ECSchemaId referencedSchemaId : referencedSchemaIds)
        {
        DbECSchemaEntry* referenceSchemaKey = nullptr;
        if (SUCCESS != LoadECSchemaDefinition(referenceSchemaKey, newlyLoadedSchemas, referencedSchemaId))
            return ERROR;

        ECObjectsStatus s = schemaEntryP->m_cachedECSchema->AddReferencedSchema(*referenceSchemaKey->m_cachedECSchema);
        if (s != ECObjectsStatus::Success)
            return ERROR;
        }

    outECSchemaKey = schemaEntryP;
    BeAssert(schemaEntryP->m_cachedECSchema != nullptr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECSchema(DbECSchemaEntry*& outECSchemaKey, ECSchemaId ctxECSchemaId, bool ensureAllClassesLoaded) const
    {
    BeMutexHolder lock (m_criticalSection);
    bvector<DbECSchemaEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadECSchemaDefinition (outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId))
        return ERROR;

    for (DbECSchemaEntry* newlyLoadedSchema : newlyLoadedSchemas)
        {
        if (SUCCESS != LoadCAFromDb(*(newlyLoadedSchema->m_cachedECSchema), newlyLoadedSchema->m_ecSchemaId, ECContainerType::Schema))
            return ERROR;
        }

    if (ensureAllClassesLoaded && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<DbECSchemaEntry*> fullyLoadedSchemas;
        if (SUCCESS != LoadClassesAndEnumsFromDb(outECSchemaKey, fullyLoadedSchemas))
            return ERROR;
        
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::GetECSchema(ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool ensureAllClassesLoaded) const
    {
    DbECSchemaEntry* outECSchemaKey;
    if (SUCCESS != ReadECSchema(outECSchemaKey, ecSchemaId, ensureAllClassesLoaded))
        return ERROR;

    ecSchemaOut = outECSchemaKey->m_cachedECSchema.get();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadClassesAndEnumsFromDb(DbECSchemaEntry* ecSchemaKey, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const
    {
    BeAssert(ecSchemaKey != nullptr);
    if (!ecSchemaKey)
        return ERROR;

    if (fullyLoadedSchemas.find(ecSchemaKey) != fullyLoadedSchemas.end())
        return SUCCESS;

    //Enure all reference schemas also loaded
    for (auto& refSchemaKey : ecSchemaKey->m_cachedECSchema->GetReferencedSchemas())
        {
        DbECSchemaEntry* key = nullptr;
        ECSchemaId referenceECSchemaId = refSchemaKey.second->GetId();
        DbECSchemaMap::const_iterator schemaIterator = m_ecSchemaCache.find(referenceECSchemaId);
        if (schemaIterator != m_ecSchemaCache.end())
            key = schemaIterator->second.get();

        if (SUCCESS != LoadClassesAndEnumsFromDb(key, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded())
        return SUCCESS;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT Id FROM ec_Class WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaKey->m_ecSchemaId))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassP ecClass = nullptr;
        if (SUCCESS != ReadECClass(ecClass, (ECClassId) stmt->GetValueInt64(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = m_db.GetCachedStatement("SELECT Name FROM ec_Enumeration WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaKey->m_ecSchemaId))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECEnumerationP ecEnum = nullptr;
        if (SUCCESS != ReadECEnumeration(ecEnum, ecSchemaKey->m_ecSchemaId, stmt->GetValueText(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaFromDb(ECSchemaPtr& ecSchemaOut, ECSchemaId ecSchemaId) const
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
        != ECObjectsStatus::Success )
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
BentleyStatus ECDbSchemaReader::LoadECPropertiesFromDb(ECClassP& ecClass, ECClassId ecClassId) const
    {
    const int kindIx = 0;
    const int idIx = 1;
    const int nameIx = 2;
    const int displayLabelIx = 3;
    const int descrIx = 4;
    const int isReadonlyIx = 5;
    const int primTypeIx = 6;
    const int nonPrimTypeIx = 7;
    const int minOccursIx = 8;
    const int maxOccursIx = 9;
    const int navPropDirectionIx = 10;

    struct PropReaderHelper
        {
        static BentleyStatus PrepareStatement(CachedStatementPtr& stmt, ECDbCR ecdb, ECClassId classId)
            {
            if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT Kind,Id,Name,DisplayLabel,Description,IsReadonly,PrimitiveType,NonPrimitiveType,ArrayMinOccurs,ArrayMaxOccurs,NavigationPropertyDirection FROM ec_Property WHERE ClassId=? ORDER BY Ordinal"))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt64(1, classId))
                return ERROR;

            return SUCCESS;
            }

        static BentleyStatus TryReadPrimitiveType(PrimitiveType& primType, CachedStatement& stmt)
            {
            if (stmt.IsColumnNull(primTypeIx))
                return ERROR;

            primType = (PrimitiveType) stmt.GetValueInt(primTypeIx);
            return SUCCESS;
            }

        static BentleyStatus TryReadNonPrimitiveType(ECClassP& nonPrimType, ECDbSchemaReader const& schemaReader, CachedStatement& stmt)
            {
            if (stmt.IsColumnNull(nonPrimTypeIx))
                return ERROR;

            const ECClassId nonPrimTypeId = (ECClassId) stmt.GetValueInt64(nonPrimTypeIx);
            BeAssert(nonPrimTypeId != ECClass::UNSET_ECCLASSID);
            return schemaReader.ReadECClass(nonPrimType, nonPrimTypeId);
            }

        static BentleyStatus TryReadArrayConstraints(uint32_t& minOccurs, uint32_t& maxOccurs, CachedStatement& stmt)
            {
            if (stmt.IsColumnNull(minOccursIx) || stmt.IsColumnNull(maxOccursIx))
                return ERROR;

            minOccurs = (uint32_t) stmt.GetValueInt(minOccursIx);
            maxOccurs = (uint32_t) stmt.GetValueInt(maxOccursIx);
            return SUCCESS;
            }
        };

    CachedStatementPtr stmt = nullptr;
    if (SUCCESS != PropReaderHelper::PrepareStatement(stmt, m_db, ecClassId))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const ECPropertyKind kind = Enum::FromInt<ECPropertyKind>(stmt->GetValueInt(kindIx));
        const ECPropertyId id = (ECPropertyId) stmt->GetValueInt64(idIx);
        Utf8CP propName = stmt->GetValueText(nameIx);

        Utf8CP displayLabel = stmt->IsColumnNull(displayLabelIx) ? nullptr : stmt->GetValueText(displayLabelIx);
        Utf8CP description = stmt->IsColumnNull(descrIx) ? nullptr : stmt->GetValueText(descrIx);
        const bool isReadonly = stmt->IsColumnNull(isReadonlyIx) ? false : stmt->GetValueInt(isReadonlyIx) != 0;

        ECPropertyP prop = nullptr;
        switch (kind)
            {
                case ECPropertyKind::Primitive:
                {
                PrimitiveType primType;
                if (SUCCESS != PropReaderHelper::TryReadPrimitiveType(primType, *stmt))
                    {
                    BeAssert(false && "PrimitiveType column is not expected to be NULL for primitive property");
                    return ERROR;
                    }

                PrimitiveECPropertyP primProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreatePrimitiveProperty(primProp, propName, primType))
                    return ERROR;

                prop = primProp;
                break;
                }

                case ECPropertyKind::Struct:
                {
                ECClassP structClassRaw = nullptr;
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(structClassRaw, *this, *stmt))
                    {
                    BeAssert(false && "NonPrimitiveType column is not expected to be NULL for struct property");
                    return ERROR;
                    }

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructECPropertyP structProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateStructProperty(structProp, propName, *structClass))
                    return ERROR;

                prop = structProp;
                break;
                }

                case ECPropertyKind::PrimitiveArray:
                {
                PrimitiveType primType;
                if (SUCCESS != PropReaderHelper::TryReadPrimitiveType(primType, *stmt))
                    {
                    BeAssert(false && "PrimitiveType column is not expected to be NULL for primitive array property");
                    return ERROR;
                    }

                ArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateArrayProperty(arrayProp, propName, primType))
                    return ERROR;

                uint32_t minOccurs, maxOccurs;
                if (SUCCESS != PropReaderHelper::TryReadArrayConstraints(minOccurs, maxOccurs, *stmt))
                    {
                    BeAssert(false && "MinOccurs and MaxOccurs columns are not expected to be NULL for array property");
                    return ERROR;
                    }

                arrayProp->SetMinOccurs(minOccurs);
                arrayProp->SetMaxOccurs(maxOccurs);

                prop = arrayProp;
                break;
                }

                case ECPropertyKind::StructArray:
                {
                ECClassP structClassRaw = nullptr;
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(structClassRaw, *this, *stmt))
                    {
                    BeAssert(false && "NonPrimitiveType column is not expected to be NULL for struct array property");
                    return ERROR;
                    }

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateStructArrayProperty(arrayProp, propName, structClass))
                    return ERROR;

                uint32_t minOccurs, maxOccurs;
                if (SUCCESS != PropReaderHelper::TryReadArrayConstraints(minOccurs, maxOccurs, *stmt))
                    {
                    BeAssert(false && "MinOccurs and MaxOccurs columns are not expected to be NULL for array property");
                    return ERROR;
                    }

                arrayProp->SetMinOccurs(minOccurs);
                arrayProp->SetMaxOccurs(maxOccurs);

                prop = arrayProp;
                break;
                }
                
                case ECPropertyKind::Navigation:
                {
                BeAssert(ecClass->IsEntityClass());

                ECClassP relClassRaw = nullptr;
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(relClassRaw, *this, *stmt))
                    {
                    BeAssert(false && "NonPrimitiveType column is not expected to be NULL for navigation property");
                    return ERROR;
                    }

                BeAssert(relClassRaw->IsRelationshipClass());
                ECRelatedInstanceDirection direction = stmt->IsColumnNull(navPropDirectionIx) ? ECRelatedInstanceDirection::Forward : (ECRelatedInstanceDirection) stmt->GetValueInt(navPropDirectionIx);
                NavigationECPropertyP navProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->GetEntityClassP()->CreateNavigationProperty(navProp, propName, *relClassRaw->GetRelationshipClassCP(), direction))
                    return ERROR;
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }

        BeAssert(prop != nullptr);
        prop->SetId(id);
        prop->SetIsReadOnly(isReadonly);
        prop->SetDescription(description);
        if (displayLabel != nullptr)
            prop->SetDisplayLabel(displayLabel);

        if (SUCCESS != LoadCAFromDb(*prop, id, ECContainerType::Property))
            return ERROR;
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, ECClassId ecClassId) const
    {
    ECDbSchemaPersistence::ECClassIdList baseClassIds;
    if (SUCCESS != ECDbSchemaPersistence::GetBaseECClasses(baseClassIds, ecClassId, m_db))
        return ERROR;

    ECClassP baseClass;
    for (ECClassId baseClassId : baseClassIds)
        {
        if (SUCCESS != ReadECClass(baseClass, baseClassId))
            return ERROR;

        if (ECObjectsStatus::Success != ecClass->AddBaseClass(*baseClass))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType) const
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
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintFromDb(ECRelationshipClassP& ecRelationship, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd) const
    {
    DbECRelationshipConstraintInfo info;
    info.ColsWhere =
        DbECRelationshipConstraintInfo::COL_RelationshipClassId   |
        DbECRelationshipConstraintInfo::COL_RelationshipEnd;

    info.ColsSelect =
        DbECRelationshipConstraintInfo::COL_MultiplicityLowerLimit |
        DbECRelationshipConstraintInfo::COL_MultiplicityUpperLimit |
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
    constraint.SetCardinality(RelationshipCardinality(info.m_multiplicityLowerLimit, info.m_multiplicityUpperLimit));
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
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR constraint, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd) const
    {
    CachedStatementPtr statement = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(statement, "SELECT ClassId, KeyProperties FROM ec_RelationshipConstraintClass WHERE RelationshipClassId=? AND RelationshipEnd=?"))
        return ERROR;

    if (BE_SQLITE_OK != statement->BindInt64(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != statement->BindInt(2, (int) relationshipEnd))
        return ERROR;

    while (statement->Step() == BE_SQLITE_ROW)
        {
        const ECClassId constraintClassId = statement->GetValueInt64(0);
        Utf8CP keyProperties = statement->IsColumnNull(1) ? nullptr : statement->GetValueText(1);

        ECClassP constraintClass = nullptr;
        if (SUCCESS != ReadECClass(constraintClass, constraintClassId))
            return ERROR;

        ECEntityClassP constraintAsEntity = constraintClass->GetEntityClassP();
        if (nullptr == constraintAsEntity)
            {
            BeAssert(false && "Relationship constraint classes are expected to be entity classes.");
            return ERROR;
            }

        ECRelationshipConstraintClassP constraintClassObj = nullptr;
        if (ECObjectsStatus::Success != constraint.AddConstraintClass(constraintClassObj, *constraintAsEntity))
            return ERROR;

        if (keyProperties != nullptr)
            {
            rapidjson::Document d;
            if (d.Parse<0>(keyProperties).HasParseError())
                {
                BeAssert(false && "Could not parse KeyProperty JSON string.");
                return ERROR;
                }

            BeAssert(d.IsArray());
            const rapidjson::SizeType count = d.Size();
            for (rapidjson::SizeType i = 0; i < count; i++)
                {
                BeAssert(d[i].IsString());
                constraintClassObj->AddKey(d[i].GetString());
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECDbSchemaReader::GetECClass(ECClassId ecClassId) const
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaReader::GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId(m_db, schemaName);
    if (schemaId == INT64_C(0))
        return nullptr;

    ECEnumerationP ecEnum = nullptr;
    if (SUCCESS != ReadECEnumeration(ecEnum, schemaId, enumName))
        return nullptr;

    return ecEnum;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::ClearCache ()
    {
    BeMutexHolder lock (m_criticalSection);

    m_ecEnumCache.clear();
    m_ecClassCache.clear ();
    m_ecSchemaCache.clear ();
    m_cache.Clear ();
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
