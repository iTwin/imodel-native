/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECDbSchemaReader::GetECClass(Context& ctx, ECClassId ecClassId) const
    {
    if (ecClassId == ECClass::UNSET_ECCLASSID)
        return nullptr;

    BeMutexHolder lock (m_criticalSection);

    DbECClassEntryMap::const_iterator classKeyIterator = m_ecClassCache.find (ecClassId);
    if (classKeyIterator != m_ecClassCache.end())
        return classKeyIterator->second->m_cachedECClass;

    const int schemaIdColIx = 0;
    const int nameColIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int modifierColIx = 5;
    const int relStrengthColIx = 6;
    const int relStrengthDirColIx = 7;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationStrength,RelationStrengthDirection FROM ec_Class WHERE Id=?");
    if (stmt == nullptr)
        return nullptr;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecClassId))
        return nullptr;

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    ECSchemaId schemaId = stmt->GetValueInt64(schemaIdColIx);
    Utf8CP className = stmt->GetValueText(nameColIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    ECClassType classType = Enum::FromInt<ECClassType>(stmt->GetValueInt(typeColIx));
    ECClassModifier classModifier = Enum::FromInt<ECClassModifier>(stmt->GetValueInt(modifierColIx));

    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, ctx, schemaId, false))
        return nullptr;

    ECSchemaR schema = *schemaKey->m_cachedECSchema;
    ECClassP ecClass = nullptr;
    switch (classType)
        {
            case ECClassType::CustomAttribute:
            {
            ECCustomAttributeClassP newClass = nullptr;
            if (schema.CreateCustomAttributeClass(newClass, className) != ECObjectsStatus::Success)
                return nullptr;

            ecClass = newClass;
            break;
            }

            case ECClassType::Entity:
            {
            ECEntityClassP newClass = nullptr;
            if (schema.CreateEntityClass(newClass, className) != ECObjectsStatus::Success)
                return nullptr;

            ecClass = newClass;
            break;
            }

            case ECClassType::Struct:
            {
            ECStructClassP newClass = nullptr;
            if (schema.CreateStructClass(newClass, className) != ECObjectsStatus::Success)
                return nullptr;

            ecClass = newClass;
            break;
            }

            case ECClassType::Relationship:
            {
            ECRelationshipClassP newClass = nullptr;
            if (schema.CreateRelationshipClass(newClass, className) != ECObjectsStatus::Success)
                return nullptr;

            BeAssert(!stmt->IsColumnNull(relStrengthColIx) && !stmt->IsColumnNull(relStrengthDirColIx));
            newClass->SetStrength(Enum::FromInt<StrengthType>(stmt->GetValueInt(relStrengthColIx)));
            newClass->SetStrengthDirection(Enum::FromInt<ECRelatedInstanceDirection>(stmt->GetValueInt(relStrengthDirColIx)));
            ecClass = newClass;
            break;
            }

            default:
                BeAssert(false);
                return nullptr;
        }

    ecClass->SetId(ecClassId);
    ecClass->SetClassModifier(classModifier);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        ecClass->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        ecClass->SetDescription(description);

    BeAssert(stmt->Step() == BE_SQLITE_DONE);
    stmt = nullptr; //to release it, so that it can be reused without repreparation

    //cache the class, before loading properties and base classes, because the class can be referenced by other classes (e.g. via nav props)
    schemaKey->m_loadedTypeCount++;
    m_ecClassCache[ecClassId] = unique_ptr<DbECClassEntry>(new DbECClassEntry(*ecClass));

    if (SUCCESS != LoadBaseClassesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    if (SUCCESS != LoadECPropertiesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    if (SUCCESS != LoadCAFromDb(*ecClass, ctx, ecClassId, ECContainerType::Class))
        return nullptr;

    ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
    if (relClass != nullptr)
        {
        if (SUCCESS != LoadECRelationshipConstraintFromDb(relClass, ctx, ecClassId, ECRelationshipEnd_Source))
            return nullptr;

        if (SUCCESS != LoadECRelationshipConstraintFromDb(relClass, ctx, ecClassId, ECRelationshipEnd_Target))
            return nullptr;
        }

    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaReader::ReadECEnumeration(ECEnumerationP& ecEnum, Context& ctx, uint64_t enumId) const
    {
    BeMutexHolder lock(m_criticalSection);

    auto enumEntryIt = m_ecEnumCache.find(enumId);
    if (enumEntryIt != m_ecEnumCache.end())
        {
        ecEnum = enumEntryIt->second->m_cachedECEnum;
        return SUCCESS;
        }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int isStrictColIx = 5;
    const int valuesColIx = 6;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, enumId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueInt64(schemaIdIx);
    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP enumName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    const PrimitiveType underlyingType = (PrimitiveType) stmt->GetValueInt(typeColIx);
    const bool isStrict = stmt->GetValueInt(isStrictColIx) != 0;
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

    if (SUCCESS != ECDbSchemaPersistenceHelper::DeserializeECEnumerationValues(*ecEnum, enumValuesJsonStr))
        return ERROR;

    //cache the enum
    m_ecEnumCache[enumId] = unique_ptr<DbECEnumEntry>(new DbECEnumEntry(enumId, *ecEnum));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaDefinition(DbECSchemaEntry*& schemaEntry, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ecSchemaId) const
    {
    auto schemaIterator = m_ecSchemaCache.find(ecSchemaId);
    if (schemaIterator != m_ecSchemaCache.end())
        {
        BeAssert(schemaIterator->second->m_cachedECSchema != nullptr);
        schemaEntry = schemaIterator->second.get();
        return SUCCESS;
        }

    if (SUCCESS != LoadECSchemaFromDb(schemaEntry, ecSchemaId))
        return ERROR;

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT ReferencedSchemaId FROM ec_SchemaReference WHERE SchemaId=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaId))
        return ERROR;

    newlyLoadedSchemas.push_back(schemaEntry);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        BeAssert(!stmt->IsColumnNull(0));
        ECSchemaId referencedSchemaId = (ECSchemaId) stmt->GetValueInt64(0);
        DbECSchemaEntry* referenceSchemaKey = nullptr;
        if (SUCCESS != LoadECSchemaDefinition(referenceSchemaKey, newlyLoadedSchemas, referencedSchemaId))
            return ERROR;

        ECObjectsStatus s = schemaEntry->m_cachedECSchema->AddReferencedSchema(*referenceSchemaKey->m_cachedECSchema);
        if (s != ECObjectsStatus::Success)
            return ERROR;
        }

    BeAssert(schemaEntry->m_cachedECSchema != nullptr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::ReadECSchema(DbECSchemaEntry*& outECSchemaKey, Context& ctx, ECSchemaId ctxECSchemaId, bool ensureAllClassesLoaded) const
    {
    BeMutexHolder lock (m_criticalSection);
    bvector<DbECSchemaEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadECSchemaDefinition (outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId))
        return ERROR;

    for (DbECSchemaEntry* newlyLoadedSchema : newlyLoadedSchemas)
        {
        ECSchemaR schema = *newlyLoadedSchema->m_cachedECSchema;
        if (SUCCESS != LoadCAFromDb(schema, ctx, schema.GetId(), ECContainerType::Schema))
            return ERROR;
        }

    if (ensureAllClassesLoaded && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<DbECSchemaEntry*> fullyLoadedSchemas;
        if (SUCCESS != LoadClassesAndEnumsFromDb(outECSchemaKey, ctx, fullyLoadedSchemas))
            return ERROR;
        
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaReader::GetECSchema(Context& ctx, ECSchemaId ecSchemaId, bool ensureAllClassesLoaded) const
    {
    DbECSchemaEntry* outECSchemaKey = nullptr;
    if (SUCCESS != ReadECSchema(outECSchemaKey,ctx,  ecSchemaId, ensureAllClassesLoaded))
        return nullptr;

    return outECSchemaKey->m_cachedECSchema.get();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadClassesAndEnumsFromDb(DbECSchemaEntry* ecSchemaKey, Context& ctx, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const
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

        if (SUCCESS != LoadClassesAndEnumsFromDb(key, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded())
        return SUCCESS;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT Id FROM ec_Class WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (nullptr == GetECClass(ctx, (ECClassId) stmt->GetValueInt64(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = m_db.GetCachedStatement("SELECT Id FROM ec_Enumeration WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECEnumerationP ecEnum = nullptr;
        if (SUCCESS != ReadECEnumeration(ecEnum, ctx, (uint64_t) stmt->GetValueInt64(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaFromDb(DbECSchemaEntry*& schemaEntry, ECSchemaId ecSchemaId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT S.Name, S.DisplayLabel,S.Description,S.NamespacePrefix,S.VersionMajor,S.VersionMinor, "
        "(SELECT COUNT(*) FROM ec_Class C WHERE S.Id = C.SchemaID) + (SELECT COUNT(*) FROM ec_Enumeration e WHERE S.Id = e.SchemaID) "
        "FROM ec_Schema S WHERE S.Id = ?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecSchemaId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    Utf8CP schemaName = stmt->GetValueText(0);
    Utf8CP displayLabel = stmt->IsColumnNull(1) ? nullptr : stmt->GetValueText(1);
    Utf8CP description = stmt->GetValueText(2);
    Utf8CP nsprefix = stmt->GetValueText(3);
    uint32_t versionMajor = (uint32_t) stmt->GetValueInt(4);
    uint32_t versionMinor = (uint32_t) stmt->GetValueInt(5);
    const int typesInSchema = stmt->GetValueInt(6);

    ECSchemaPtr schema = nullptr;
    if (ECSchema::CreateSchema(schema, schemaName, versionMajor, versionMinor) != ECObjectsStatus::Success)
        return ERROR;

    schema->SetId(ecSchemaId);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        schema->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        schema->SetDescription(description);

    if (!Utf8String::IsNullOrEmpty(nsprefix))
        schema->SetNamespacePrefix(nsprefix);

    unique_ptr<DbECSchemaEntry> schemaEntryPtr = unique_ptr<DbECSchemaEntry>(new DbECSchemaEntry(schema, typesInSchema));
    schemaEntry = schemaEntryPtr.get();
    m_ecSchemaCache[ecSchemaId] = move(schemaEntryPtr);
    m_cache.AddSchema(*schemaEntry->m_cachedECSchema);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECPropertiesFromDb(ECClassP& ecClass, Context& ctx, ECClassId ecClassId) const
    {
    const int kindIx = 0;
    const int idIx = 1;
    const int nameIx = 2;
    const int displayLabelIx = 3;
    const int descrIx = 4;
    const int isReadonlyIx = 5;
    const int navPropDirectionIx = 11;

    struct PropReaderHelper
        {
        static BentleyStatus PrepareStatement(CachedStatementPtr& stmt, ECDbCR ecdb, ECClassId classId)
            {
            if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "SELECT Kind,Id,Name,DisplayLabel,Description,IsReadonly,PrimitiveType,NonPrimitiveType,Enumeration,ArrayMinOccurs,ArrayMaxOccurs,NavigationPropertyDirection FROM ec_Property WHERE ClassId=? ORDER BY Ordinal"))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt64(1, classId))
                return ERROR;

            return SUCCESS;
            }

        static BentleyStatus TryReadPrimitiveType(PrimitiveType& primType, CachedStatement& stmt)
            {
            const int primTypeIx = 6;
            if (stmt.IsColumnNull(primTypeIx))
                return ERROR;

            primType = (PrimitiveType) stmt.GetValueInt(primTypeIx);
            return SUCCESS;
            }

        static BentleyStatus TryReadNonPrimitiveType(ECClassP& nonPrimType, ECDbSchemaReader const& schemaReader, Context& ctx, CachedStatement& stmt)
            {
            const int nonPrimTypeIx = 7;
            if (stmt.IsColumnNull(nonPrimTypeIx))
                return ERROR;

            const ECClassId nonPrimTypeId = (ECClassId) stmt.GetValueInt64(nonPrimTypeIx);
            BeAssert(nonPrimTypeId != ECClass::UNSET_ECCLASSID);
            nonPrimType = schemaReader.GetECClass(ctx, nonPrimTypeId);
            return nonPrimType != nullptr ? SUCCESS : ERROR;
            }

        static BentleyStatus TryReadEnumeration(ECEnumerationP& enumeration, ECDbSchemaReader const& schemaReader, Context& ctx, CachedStatement& stmt)
            {
            const int ix = 8;
            if (stmt.IsColumnNull(ix))
                return ERROR;

            const int64_t enumTypeId = stmt.GetValueInt64(ix);
            BeAssert(enumTypeId > 0);
            return schemaReader.ReadECEnumeration(enumeration, ctx, enumTypeId);;
            }

        static BentleyStatus TryReadArrayConstraints(uint32_t& minOccurs, uint32_t& maxOccurs, CachedStatement& stmt)
            {
            const int minOccursIx = 9;
            const int maxOccursIx = 10;
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

                case ECPropertyKind::Enumeration:
                {
                ECEnumerationP ecenum = nullptr;
                if (SUCCESS != PropReaderHelper::TryReadEnumeration(ecenum, *this, ctx, *stmt))
                    {
                    BeAssert(false && "Enumeration column is not expected to be NULL for property using an ECEnumeration");
                    return ERROR;
                    }

                PrimitiveECPropertyP primProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateEnumerationProperty(primProp, propName, *ecenum))
                    return ERROR;

                prop = primProp;
                break;
                }

                case ECPropertyKind::Struct:
                {
                ECClassP structClassRaw = nullptr;
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(structClassRaw, *this, ctx, *stmt))
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
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(structClassRaw, *this, ctx, *stmt))
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
                if (SUCCESS != PropReaderHelper::TryReadNonPrimitiveType(relClassRaw, *this, ctx, *stmt))
                    {
                    BeAssert(false && "NonPrimitiveType column is not expected to be NULL for navigation property");
                    return ERROR;
                    }

                BeAssert(relClassRaw->IsRelationshipClass());
                ECRelatedInstanceDirection direction = stmt->IsColumnNull(navPropDirectionIx) ? ECRelatedInstanceDirection::Forward : (ECRelatedInstanceDirection) stmt->GetValueInt(navPropDirectionIx);
                NavigationECPropertyP navProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->GetEntityClassP()->CreateNavigationProperty(navProp, propName, *relClassRaw->GetRelationshipClassCP(), direction))
                    return ERROR;

                //keep track of nav prop as we need to validate them when everything else is loaded
                ctx.AddNavigationProperty(*navProp);
                prop = navProp;
                break;
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

        if (SUCCESS != LoadCAFromDb(*prop, ctx, id, ECContainerType::Property))
            return ERROR;
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, Context& ctx, ECClassId ecClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT BaseClassId FROM ec_BaseClass WHERE ClassId=? ORDER BY Ordinal"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecClassId))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId baseClassId = stmt->GetValueInt64(0);
        ECClassCP baseClass = GetECClass(ctx, baseClassId);
        if (baseClass == nullptr)
            return ERROR;

        if (ECObjectsStatus::Success != ecClass->AddBaseClass(*baseClass))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, Context& ctx, ECContainerId containerId, ECContainerType containerType) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT ClassId,Instance FROM ec_CustomAttribute WHERE ContainerId=? AND ContainerType=? ORDER BY Ordinal"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId caClassId = stmt->GetValueInt64(0);
        ECClassCP caClass = GetECClass(ctx, caClassId);
        if (caClass == nullptr)
            return ERROR;

        Utf8CP caXml = stmt->GetValueText(1);
        ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext(caClass->GetSchema());
        IECInstancePtr deserializedCa = nullptr;
        if (InstanceReadStatus::Success != IECInstance::ReadFromXmlString(deserializedCa, caXml, *readContext))
            return ERROR;
        BeAssert(deserializedCa != nullptr);
        caConstainer.SetCustomAttribute(*deserializedCa);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintFromDb(ECRelationshipClassP& ecRelationship, Context& ctx, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel FROM ec_RelationshipConstraint WHERE RelationshipClassId=? AND RelationshipEnd=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, relationshipEnd))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;
   
    ECRelationshipConstraintR constraint = (relationshipEnd == ECRelationshipEnd_Target) ? ecRelationship->GetTarget() : ecRelationship->GetSource();

    constraint.SetCardinality(RelationshipCardinality(stmt->GetValueInt(0), stmt->GetValueInt(1)));
    constraint.SetIsPolymorphic(stmt->GetValueInt(2) != 0);

    if (!stmt->IsColumnNull(3))
        constraint.SetRoleLabel(stmt->GetValueText(3));

    if (SUCCESS != LoadECRelationshipConstraintClassesFromDb(constraint, ctx, relationshipClassId, relationshipEnd))
        return ERROR;

    ECContainerType containerType = 
        relationshipEnd == ECRelationshipEnd_Target ? ECContainerType::RelationshipConstraintTarget : ECContainerType::RelationshipConstraintSource;

    return LoadCAFromDb(constraint, ctx, relationshipClassId, containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR constraint, Context& ctx, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd) const
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

        ECClassCP constraintClass = GetECClass(ctx, constraintClassId);
        if (constraintClass == nullptr)
            return ERROR;

        ECEntityClassCP constraintAsEntity = constraintClass->GetEntityClassCP();
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
            if (SUCCESS != ECDbSchemaPersistenceHelper::DeserializeRelationshipKeyProperties(*constraintClassObj, keyProperties))
                return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaReader::TryGetECClassId(ECClassId& id, Utf8CP schemaName, Utf8CP className, ResolveSchema resolveSchema) const
    {
    ECClassId ecClassId = ECDbSchemaPersistenceHelper::GetECClassId(m_db, schemaName, className, resolveSchema); // needswork: if this is a performance issue, try to look it up in-memory, first
    if (ecClassId == ECClass::UNSET_ECCLASSID)
        return false;

    id = ecClassId;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaReader::GetECEnumeration(Context& ctx, Utf8CP schemaName, Utf8CP enumName) const
    {
    uint64_t enumId = ECDbSchemaPersistenceHelper::GetECEnumerationId(m_db, schemaName, enumName);
    if (enumId == INT64_C(0))
        return nullptr;

    ECEnumerationP ecEnum = nullptr;
    if (SUCCESS != ReadECEnumeration(ecEnum, ctx, enumId))
        return nullptr;

    return ecEnum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaReader::EnsureDerivedClassesExist(Context& ctx, ECClassId baseClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT ClassId FROM ec_BaseClass WHERE BaseClassId = ?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, baseClassId))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        if (GetECClass(ctx, (ECClassId) stmt->GetValueInt64(0)) == nullptr)
            return ERROR;
        }

    return SUCCESS;
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
