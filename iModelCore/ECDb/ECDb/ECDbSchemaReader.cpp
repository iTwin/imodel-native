/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"
#include <limits>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaReader::GetECSchema(ECSchemaId const& ecSchemaId, bool loadSchemaEntities) const
    {
    ECDbSchemaReader::Context ctx;
    ECSchemaCP schema = GetECSchema(ctx, ecSchemaId, loadSchemaEntities);
    if (schema == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return schema;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaReader::GetECSchema(Context& ctx, ECSchemaId const& ecSchemaId, bool loadSchemaEntities) const
    {
    DbECSchemaEntry* outECSchemaKey = nullptr;
    if (SUCCESS != ReadECSchema(outECSchemaKey, ctx, ecSchemaId, loadSchemaEntities))
        return nullptr;

    return outECSchemaKey->m_cachedECSchema.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaReader::GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    ECDbSchemaReader::Context ctx;
    ECEnumerationCP ecenum = GetECEnumeration(ctx, schemaName, enumName);
    if (ecenum == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return ecenum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP ECDbSchemaReader::GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const
    {
    ECDbSchemaReader::Context ctx;
    KindOfQuantityCP koq = GetKindOfQuantity(ctx, schemaName, koqName);
    if (koq == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return koq;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaReader::EnsureDerivedClassesExist(ECClassId const& ecClassId) const
    {
    ECDbSchemaReader::Context ctx;
    if (SUCCESS != EnsureDerivedClassesExist(ctx, ecClassId))
        return ERROR;

    return ctx.Postprocess(*this);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaReader::EnsureDerivedClassesExist(Context& ctx, ECClassId const& ecClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT ClassId FROM ec_ClassHasBaseClasses WHERE BaseClassId=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        if (GetECClass(ctx, stmt->GetValueId<ECClassId>(0)) == nullptr)
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaReader::GetECClass(ECClassId const& ecClassId) const
    {
    ECDbSchemaReader::Context ctx;
    ECClassCP ecclass = GetECClass(ctx, ecClassId);
    if (ecclass == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return ecclass;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECDbSchemaReader::GetECClass(Context& ctx, ECClassId const& ecClassId) const
    {
    if (!ecClassId.IsValid())
        return nullptr;

    BeMutexHolder lock(m_criticalSection);
    ECDbExpressionSymbolContext symbolsContext(m_db);

    ECClassP classFromCache = nullptr;
    if (TryGetECClassFromCache(classFromCache, ecClassId))
        return classFromCache;

    const int schemaIdColIx = 0;
    const int nameColIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int modifierColIx = 5;
    const int relStrengthColIx = 6;
    const int relStrengthDirColIx = 7;
    const int caContainerTypeIx = 8;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType FROM ec_Class WHERE Id=?");
    if (stmt == nullptr)
        return nullptr;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return nullptr;

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdColIx);
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

            if (!stmt->IsColumnNull(caContainerTypeIx))
                newClass->SetContainerType(Enum::FromInt<CustomAttributeContainerType>(stmt->GetValueInt(caContainerTypeIx)));
            else
                newClass->SetContainerType(CustomAttributeContainerType::Any);

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
    m_ecClassCache[ecClassId] = std::unique_ptr<DbECClassEntry>(new DbECClassEntry(*ecClass));

    if (SUCCESS != LoadBaseClassesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    if (SUCCESS != LoadECPropertiesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    if (SUCCESS != LoadCAFromDb(*ecClass, ctx, ECContainerId(ecClassId), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class))
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
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbSchemaReader::TryGetECClassFromCache(ECClassP& ecClass, ECClassId const& ecClassId) const
    {
    if (!ecClassId.IsValid())
        return false;

    DbECClassEntryMap::const_iterator classKeyIterator = m_ecClassCache.find(ecClassId);
    if (classKeyIterator != m_ecClassCache.end())
        {
        ecClass = classKeyIterator->second->m_cachedECClass;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaReader::GetECEnumeration(Context& ctx, Utf8CP schemaName, Utf8CP enumName) const
    {
    ECEnumerationId enumId = ECDbSchemaPersistenceHelper::GetECEnumerationId(m_db, schemaName, enumName);
    if (!enumId.IsValid())
        return nullptr;

    ECEnumerationP ecEnum = nullptr;
    if (SUCCESS != ReadECEnumeration(ecEnum, ctx, enumId))
        return nullptr;

    return ecEnum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaReader::ReadECEnumeration(ECEnumerationP& ecEnum, Context& ctx, ECEnumerationId const& enumId) const
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

    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP enumName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    const PrimitiveType underlyingType = (PrimitiveType) stmt->GetValueInt(typeColIx);
    const bool isStrict = DbSchemaPersistenceManager::IsTrue(stmt->GetValueInt(isStrictColIx));
    Utf8CP enumValuesJsonStr = stmt->GetValueText(valuesColIx);

    ecEnum = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedECSchema->CreateEnumeration(ecEnum, enumName, underlyingType))
        return ERROR;

    ecEnum->SetId(enumId);

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
    m_ecEnumCache[enumId] = std::unique_ptr<DbECEnumEntry>(new DbECEnumEntry(enumId, *ecEnum));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP ECDbSchemaReader::GetKindOfQuantity(Context& ctx, Utf8CP schemaName, Utf8CP koqName) const
    {
    KindOfQuantityId koqId = ECDbSchemaPersistenceHelper::GetKindOfQuantityId(m_db, schemaName, koqName);
    if (!koqId.IsValid())
        return nullptr;

    KindOfQuantityP koq = nullptr;
    if (SUCCESS != ReadKindOfQuantity(koq, ctx, koqId))
        return nullptr;

    return koq;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaReader::ReadKindOfQuantity(KindOfQuantityP& koq, Context& ctx, KindOfQuantityId const& koqId) const
    {
    BeMutexHolder lock(m_criticalSection);

    auto koqEntryIt = m_koqCache.find(koqId);
    if (koqEntryIt != m_koqCache.end())
        {
        koq = koqEntryIt->second->m_cachedKoq;
        return SUCCESS;
        }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int persUnitColIx = 4;
    const int persPrecColIx = 5;
    const int defaultPresUnitColIx = 6;
    const int altPresUnitColIx = 7;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,PersistenceUnit,PersistencePrecision,DefaultPresentationUnit,AlternativePresentationUnits FROM ec_KindOfQuantity WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, koqId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
    DbECSchemaEntry* schemaKey = nullptr;
    if (SUCCESS != ReadECSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP koqName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    BeAssert(!stmt->IsColumnNull(persUnitColIx));
    Utf8CP persUnit = stmt->GetValueText(persUnitColIx);
    uint32_t persPrec = (uint32_t) stmt->GetValueInt(persPrecColIx);
    Utf8CP defaultPresUnit = stmt->IsColumnNull(defaultPresUnitColIx) ? nullptr : stmt->GetValueText(defaultPresUnitColIx);
    Utf8CP altPresUnits = stmt->IsColumnNull(altPresUnitColIx) ? nullptr : stmt->GetValueText(altPresUnitColIx);
    
    koq = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedECSchema->CreateKindOfQuantity(koq, koqName))
        return ERROR;

    koq->SetId(koqId);

    if (displayLabel != nullptr)
        koq->SetDisplayLabel(displayLabel);

    koq->SetDescription(description);
    koq->SetPersistenceUnit(persUnit);
    koq->SetPrecision(persPrec);

    if (!Utf8String::IsNullOrEmpty(defaultPresUnit))
        koq->SetDefaultPresentationUnit(defaultPresUnit);

    if (!Utf8String::IsNullOrEmpty(altPresUnits))
        {
        if (SUCCESS != ECDbSchemaPersistenceHelper::DeserializeKoqAlternativePresentationUnits(*koq, altPresUnits))
            return ERROR;
        }

    //cache the koq
    m_koqCache[koqId] = std::unique_ptr<DbKindOfQuantityEntry>(new DbKindOfQuantityEntry(koqId, *koq));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaDefinition(DbECSchemaEntry*& schemaEntry, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId const& ecSchemaId) const
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

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaId))
        return ERROR;

    newlyLoadedSchemas.push_back(schemaEntry);

    //cache schema ids before loading reference schemas, so that statement can be reused.
    std::vector<ECSchemaId> referencedSchemaIds;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        BeAssert(!stmt->IsColumnNull(0));
        ECSchemaId referencedSchemaId = stmt->GetValueId<ECSchemaId>(0);
        BeAssert(referencedSchemaId.IsValid());
        referencedSchemaIds.push_back(referencedSchemaId);
        }

    //release stmt so that it can be reused when loading schema reference
    stmt = nullptr;

    for (ECSchemaId referencedSchemaId : referencedSchemaIds)
        {
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
BentleyStatus ECDbSchemaReader::ReadECSchema(DbECSchemaEntry*& outECSchemaKey, Context& ctx, ECSchemaId const& ctxECSchemaId, bool loadSchemaEntities) const
    {
    BeMutexHolder lock(m_criticalSection);
    bvector<DbECSchemaEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadECSchemaDefinition(outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId))
        return ERROR;

    for (DbECSchemaEntry* newlyLoadedSchema : newlyLoadedSchemas)
        {
        ECSchemaR schema = *newlyLoadedSchema->m_cachedECSchema;
        ctx.AddSchemaToLoadCAInstanceFor(schema);
        }

    if (loadSchemaEntities && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<DbECSchemaEntry*> fullyLoadedSchemas;
        if (SUCCESS != LoadSchemaEntitiesFromDb(outECSchemaKey, ctx, fullyLoadedSchemas))
            return ERROR;

        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadSchemaEntitiesFromDb(DbECSchemaEntry* ecSchemaKey, Context& ctx, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const
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

        if (SUCCESS != LoadSchemaEntitiesFromDb(key, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded())
        return SUCCESS;

    BeSQLite::CachedStatementPtr stmt = m_db.GetCachedStatement("SELECT Id FROM ec_Class WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (nullptr == GetECClass(ctx, stmt->GetValueId<ECClassId>(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = m_db.GetCachedStatement("SELECT Id FROM ec_Enumeration WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECEnumerationP ecEnum = nullptr;
        if (SUCCESS != ReadECEnumeration(ecEnum, ctx, stmt->GetValueId<ECEnumerationId>(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = m_db.GetCachedStatement("SELECT Id FROM ec_KindOfQuantity WHERE SchemaId=?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        KindOfQuantityP koq = nullptr;
        if (SUCCESS != ReadKindOfQuantity(koq, ctx, stmt->GetValueId<KindOfQuantityId>(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECSchemaFromDb(DbECSchemaEntry*& schemaEntry, ECSchemaId const& ecSchemaId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt,
        "SELECT S.Name, S.DisplayLabel,S.Description,S.NamespacePrefix,S.VersionDigit1,S.VersionDigit2,S.VersionDigit3, "
        "(SELECT COUNT(*) FROM ec_Class C WHERE S.Id = C.SchemaId) + "
        "(SELECT COUNT(*) FROM ec_Enumeration e WHERE S.Id = e.SchemaId) + "
        "(SELECT COUNT(*) FROM ec_KindOfQuantity koq WHERE S.Id = koq.SchemaId)"
        "FROM ec_Schema S WHERE S.Id=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    Utf8CP schemaName = stmt->GetValueText(0);
    Utf8CP displayLabel = stmt->IsColumnNull(1) ? nullptr : stmt->GetValueText(1);
    Utf8CP description = stmt->IsColumnNull(2) ? nullptr : stmt->GetValueText(2);
    Utf8CP nsprefix = stmt->GetValueText(3);
    uint32_t versionMajor = (uint32_t) stmt->GetValueInt(4);
    uint32_t versionWrite = (uint32_t) stmt->GetValueInt(5);
    uint32_t versionMinor = (uint32_t) stmt->GetValueInt(6);
    const int typesInSchema = stmt->GetValueInt(7);

    ECSchemaPtr schema = nullptr;
    if (ECSchema::CreateSchema(schema, schemaName, nsprefix, versionMajor, versionWrite, versionMinor) != ECObjectsStatus::Success)
        return ERROR;

    schema->SetId(ecSchemaId);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        schema->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        schema->SetDescription(description);

    std::unique_ptr<DbECSchemaEntry> schemaEntryPtr = std::unique_ptr<DbECSchemaEntry>(new DbECSchemaEntry(schema, typesInSchema));
    schemaEntry = schemaEntryPtr.get();
    m_ecSchemaCache[ecSchemaId] = std::move(schemaEntryPtr);
    m_cache.AddSchema(*schemaEntry->m_cachedECSchema);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECPropertiesFromDb(ECClassP& ecClass, Context& ctx, ECClassId const& ecClassId) const
    {
    struct PropReaderHelper
        {
        struct RowInfo
            {
            ECPropertyId m_id;
            ECPropertyKind m_kind;
            Utf8String m_name;
            Utf8String m_displayLabel;
            Utf8String m_description;
            bool m_isReadonly = false;
            int m_primType = -1;
            ECClassId m_structClassId;
            Utf8String m_extendedTypeName;
            ECEnumerationId m_enumId;
            KindOfQuantityId m_koqId;
            uint32_t m_arrayMinOccurs = 0;
            uint32_t m_arrayMaxOccurs = std::numeric_limits<uint32_t>::max();
            ECClassId m_navPropRelClassId;
            int m_navPropDirection = -1;
            };

        static BentleyStatus ReadRows(std::vector<RowInfo>& rows, ECDbCR ecdb, ECClassId const& classId)
            {
            const int idIx = 0;
            const int kindIx = 1;
            const int nameIx = 2;
            const int displayLabelIx = 3;
            const int descrIx = 4;
            const int isReadonlyIx = 5;
            const int primTypeIx = 6;
            const int enumIdIx = 7;
            const int structClassIdIx = 8;
            const int extendedTypeIx = 9;
            const int koqIdIx = 10;
            const int minOccursIx = 11;
            const int maxOccursIx = 12;
            const int navRelationshipClassId = 13;
            const int navPropDirectionIx = 14;

            CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Id,Kind,Name,DisplayLabel,Description,IsReadonly,"
                                                              "PrimitiveType,EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,"
                                                              "ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection "
                                                              "FROM ec_Property WHERE ClassId=? ORDER BY Ordinal");
            if (stmt == nullptr)
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(1, classId))
                return ERROR;

            while (BE_SQLITE_ROW == stmt->Step())
                {
                RowInfo rowInfo;
                rowInfo.m_id = stmt->GetValueId<ECPropertyId>(idIx);
                const ECPropertyKind kind = Enum::FromInt<ECPropertyKind>(stmt->GetValueInt(kindIx));
                rowInfo.m_kind = kind;
                rowInfo.m_name.assign(stmt->GetValueText(nameIx));
                if (!stmt->IsColumnNull(displayLabelIx))
                    rowInfo.m_displayLabel.assign(stmt->GetValueText(displayLabelIx));

                if (!stmt->IsColumnNull(descrIx))
                    rowInfo.m_description.assign(stmt->GetValueText(descrIx));

                if (!stmt->IsColumnNull(isReadonlyIx))
                    rowInfo.m_isReadonly = DbSchemaPersistenceManager::IsTrue(stmt->GetValueInt(isReadonlyIx));

                bool primTypeIsNull = false;
                if (stmt->IsColumnNull(primTypeIx))
                    primTypeIsNull = true;
                else
                    rowInfo.m_primType = stmt->GetValueInt(primTypeIx);

                if (!stmt->IsColumnNull(enumIdIx))
                    rowInfo.m_enumId = stmt->GetValueId<ECEnumerationId>(enumIdIx);

                if ((kind == ECPropertyKind::Primitive || kind == ECPropertyKind::PrimitiveArray) && primTypeIsNull && !rowInfo.m_enumId.IsValid())
                    {
                    BeAssert(false && "Either PrimitiveType or EnumerationId column must not be NULL for primitive and prim array property");
                    return ERROR;
                    }

                if (stmt->IsColumnNull(structClassIdIx))
                    {
                    if (kind == ECPropertyKind::Struct || kind == ECPropertyKind::StructArray)
                        {
                        BeAssert(false && "StructClassId column must not be NULL for struct or struct array property");
                        return ERROR;
                        }
                    }
                else
                    rowInfo.m_structClassId = stmt->GetValueId<ECClassId>(structClassIdIx);

                if (!stmt->IsColumnNull(extendedTypeIx))
                    rowInfo.m_extendedTypeName.assign(stmt->GetValueText(extendedTypeIx));

                if (!stmt->IsColumnNull(koqIdIx))
                    rowInfo.m_koqId = stmt->GetValueId<KindOfQuantityId>(koqIdIx);

                if (rowInfo.m_koqId.IsValid() && kind != ECPropertyKind::Primitive && kind != ECPropertyKind::PrimitiveArray)
                    {
                    BeAssert(false && "KindOfQuantityId must only be set for primitive or primitive array props");
                    return ERROR;
                    }

                if (stmt->IsColumnNull(minOccursIx) || stmt->IsColumnNull(maxOccursIx))
                    {
                    if (kind == ECPropertyKind::PrimitiveArray || kind == ECPropertyKind::StructArray)
                        {
                        BeAssert(false && "ArrayMinOccurs and ArrayMaxOccurs columns must not be NULL for array property");
                        return ERROR;
                        }
                    }
                else
                    {
                    rowInfo.m_arrayMinOccurs = (uint32_t) stmt->GetValueInt(minOccursIx);
                    rowInfo.m_arrayMaxOccurs = (uint32_t) stmt->GetValueInt(maxOccursIx);
                    }

                if (stmt->IsColumnNull(navRelationshipClassId))
                    {
                    if (kind == ECPropertyKind::Navigation)
                        {
                        BeAssert(false && "NavigationRelationshipClassId column must not be NULL for navigation property");
                        return ERROR;
                        }
                    }
                else
                    rowInfo.m_navPropRelClassId = stmt->GetValueId<ECClassId>(navRelationshipClassId);

                if (stmt->IsColumnNull(navPropDirectionIx))
                    {
                    if (kind == ECPropertyKind::Navigation)
                        {
                        BeAssert(false && "NavigationDirection column must not be NULL for navigation property");
                        return ERROR;
                        }
                    }
                else
                    rowInfo.m_navPropDirection = stmt->GetValueInt(navPropDirectionIx);


                rows.push_back(rowInfo);
                }

            return SUCCESS;
            }

        };

    std::vector<PropReaderHelper::RowInfo> rowInfos;
    if (SUCCESS != PropReaderHelper::ReadRows(rowInfos, m_db, ecClassId))
        return ERROR;

    for (PropReaderHelper::RowInfo const& rowInfo : rowInfos)
        {
        ECPropertyP prop = nullptr;
        switch (rowInfo.m_kind)
            {
                case ECPropertyKind::Primitive:
                {
                BeAssert(rowInfo.m_primType >= 0 || rowInfo.m_enumId.IsValid());

                PrimitiveECPropertyP primProp = nullptr;

                if (rowInfo.m_enumId.IsValid())
                    {
                    ECEnumerationP ecenum = nullptr;
                    if (SUCCESS != ReadECEnumeration(ecenum, ctx, rowInfo.m_enumId))
                        return ERROR;

                    if (ECObjectsStatus::Success != ecClass->CreateEnumerationProperty(primProp, rowInfo.m_name, *ecenum))
                        return ERROR;
                    }
                else
                    {
                    if (ECObjectsStatus::Success != ecClass->CreatePrimitiveProperty(primProp, rowInfo.m_name, (PrimitiveType) rowInfo.m_primType))
                        return ERROR;
                    }

                if (!rowInfo.m_extendedTypeName.empty())
                    {
                    if (ECObjectsStatus::Success != primProp->SetExtendedTypeName(rowInfo.m_extendedTypeName.c_str()))
                        return ERROR;
                    }

                if (rowInfo.m_koqId.IsValid())
                    {
                    KindOfQuantityP koq = nullptr;
                    if (SUCCESS != ReadKindOfQuantity(koq, ctx, rowInfo.m_koqId))
                        return ERROR;

                    primProp->SetKindOfQuantity(koq);
                    }

                prop = primProp;
                break;
                }

                case ECPropertyKind::Struct:
                {
                BeAssert(rowInfo.m_structClassId.IsValid());

                ECClassCP structClassRaw = GetECClass(ctx, rowInfo.m_structClassId);
                if (structClassRaw == nullptr)
                    return ERROR;

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructECPropertyP structProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateStructProperty(structProp, rowInfo.m_name, *structClass))
                    return ERROR;

                prop = structProp;
                break;
                }

                case ECPropertyKind::PrimitiveArray:
                {
                BeAssert(rowInfo.m_primType >= 0);

                PrimitiveType primType = (PrimitiveType) rowInfo.m_primType;

                ArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateArrayProperty(arrayProp, rowInfo.m_name, primType))
                    return ERROR;

                if (!rowInfo.m_extendedTypeName.empty())
                    {
                    if (ECObjectsStatus::Success != arrayProp->SetExtendedTypeName(rowInfo.m_extendedTypeName.c_str()))
                        return ERROR;
                    }

                if (rowInfo.m_koqId.IsValid())
                    {
                    KindOfQuantityP koq = nullptr;
                    if (SUCCESS != ReadKindOfQuantity(koq, ctx, rowInfo.m_koqId))
                        return ERROR;

                    arrayProp->SetKindOfQuantity(koq);
                    }

                arrayProp->SetMinOccurs(rowInfo.m_arrayMinOccurs);
                arrayProp->SetMaxOccurs(rowInfo.m_arrayMaxOccurs);

                prop = arrayProp;
                break;
                }

                case ECPropertyKind::StructArray:
                {
                BeAssert(rowInfo.m_structClassId.IsValid());
                ECClassCP structClassRaw = GetECClass(ctx, rowInfo.m_structClassId);
                if (structClassRaw == nullptr)
                    return ERROR;

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreateStructArrayProperty(arrayProp, rowInfo.m_name, structClass))
                    return ERROR;

                arrayProp->SetMinOccurs(rowInfo.m_arrayMinOccurs);
                arrayProp->SetMaxOccurs(rowInfo.m_arrayMaxOccurs);

                prop = arrayProp;
                break;
                }

                case ECPropertyKind::Navigation:
                {
                BeAssert(ecClass->IsEntityClass());

                BeAssert(rowInfo.m_navPropRelClassId.IsValid());
                ECClassCP relClassRaw = GetECClass(ctx, rowInfo.m_navPropRelClassId);
                if (relClassRaw == nullptr)
                    return ERROR;

                BeAssert(relClassRaw->IsRelationshipClass());
                ECRelatedInstanceDirection direction = rowInfo.m_navPropDirection < 0 ? ECRelatedInstanceDirection::Forward : (ECRelatedInstanceDirection) rowInfo.m_navPropDirection;
                NavigationECPropertyP navProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->GetEntityClassP()->CreateNavigationProperty(navProp, rowInfo.m_name, *relClassRaw->GetRelationshipClassCP(), direction, PrimitiveType::PRIMITIVETYPE_Long, false))
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
        prop->SetId(rowInfo.m_id);
        prop->SetIsReadOnly(rowInfo.m_isReadonly);
        if (!rowInfo.m_description.empty())
            prop->SetDescription(rowInfo.m_description);

        if (!rowInfo.m_displayLabel.empty())
            prop->SetDisplayLabel(rowInfo.m_displayLabel);

        if (SUCCESS != LoadCAFromDb(*prop, ctx, ECContainerId(rowInfo.m_id), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property))
            return ERROR;

        // For ECDb symbol provider ensure the calculated property specification is created (must do that after
        // custom attributes are loaded)
        if (prop->IsCalculated())
            prop->GetCalculatedPropertySpecification();

        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, Context& ctx, ECClassId const& ecClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT BaseClassId FROM ec_ClassHasBaseClasses WHERE ClassId=? ORDER BY Ordinal"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    //cache base class ids before loading base classes so that statement can be reused for fetching base class ids
    std::vector<ECClassId> baseClassIds;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId baseClassId = stmt->GetValueId<ECClassId>(0);
        BeAssert(baseClassId.IsValid());
        baseClassIds.push_back(baseClassId);
        }

    //release stmt so that it can be reused for loading base classes
    stmt = nullptr;

    for (ECClassId baseClassId : baseClassIds)
        {
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
BentleyStatus ECDbSchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR caConstainer, Context& ctx, ECContainerId const& containerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT ClassId,Instance FROM ec_CustomAttribute WHERE ContainerId=? AND ContainerType=? ORDER BY Ordinal"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId caClassId = stmt->GetValueId<ECClassId>(0);
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
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintFromDb(ECRelationshipClassP& ecRelationship, Context& ctx, ECClassId const& relationshipClassId, ECRelationshipEnd relationshipEnd) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(stmt, "SELECT Id,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel FROM ec_RelationshipConstraint WHERE RelationshipClassId=? AND RelationshipEnd=?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, relationshipEnd))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    ECRelationshipConstraintId constraintId = stmt->GetValueId<ECRelationshipConstraintId>(0);

    ECRelationshipConstraintR constraint = (relationshipEnd == ECRelationshipEnd_Target) ? ecRelationship->GetTarget() : ecRelationship->GetSource();

    constraint.SetCardinality(RelationshipCardinality(stmt->GetValueInt(1), stmt->GetValueInt(2)));
    constraint.SetIsPolymorphic(DbSchemaPersistenceManager::IsTrue(stmt->GetValueInt(3)));

    if (!stmt->IsColumnNull(4))
        constraint.SetRoleLabel(stmt->GetValueText(4));

    if (SUCCESS != LoadECRelationshipConstraintClassesFromDb(constraint, ctx, constraintId))
        return ERROR;

    ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType =
        relationshipEnd == ECRelationshipEnd_Target ? ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint : ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint;

    return LoadCAFromDb(constraint, ctx, ECContainerId(constraintId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaReader::LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR constraint, Context& ctx, ECRelationshipConstraintId const& constraintId) const
    {
    CachedStatementPtr statement = nullptr;
    if (BE_SQLITE_OK != m_db.GetCachedStatement(statement, "SELECT ClassId, KeyProperties FROM ec_RelationshipConstraintClass WHERE ConstraintId=?"))
        return ERROR;

    if (BE_SQLITE_OK != statement->BindId(1, constraintId))
        return ERROR;

    while (statement->Step() == BE_SQLITE_ROW)
        {
        const ECClassId constraintClassId = statement->GetValueId<ECClassId>(0);
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
    if (!ecClassId.IsValid())
        return false;

    id = ecClassId;
    return true;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaReader::ClearCache()
    {
    BeMutexHolder lock(m_criticalSection);

    m_ecEnumCache.clear();
    m_koqCache.clear();
    m_ecClassCache.clear();
    m_ecSchemaCache.clear();
    m_cache.Clear();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaReader::Context::Postprocess(ECDbSchemaReader const& reader) const
    {
    for (ECN::NavigationECProperty* navProp : m_navProps)
        {
        if (!navProp->Verify())
            return ERROR;
        }

    if (m_schemasToLoadCAInstancesFor.empty())
        return SUCCESS;

    //load CAs for gatherer schemas now and in a separate context.
    //delaying CA loading on schemas is necessary for the case where a schema defines a CA class
    //and at the same time carries an instance of it.
    Context ctx;
    for (ECN::ECSchema* schema : m_schemasToLoadCAInstancesFor)
        {
        if (SUCCESS != reader.LoadCAFromDb(*schema, ctx, ECContainerId(schema->GetId()), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
            return ERROR;
        }

    return ctx.Postprocess(reader);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
