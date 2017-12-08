/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"
#include <Formatting/FormattingApi.h>
#include <limits>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

SchemaReader::SchemaReader(TableSpaceSchemaManager const& manager) : m_schemaManager(manager), m_cache(manager.GetECDb()) 
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<BeMutexHolder> SchemaReader::LockECDb() const
    {
    return std::unique_ptr<BeMutexHolder>(new BeMutexHolder(GetECDb().GetImpl().GetMutex()));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<BeSqliteDbMutex> SchemaReader::LockDb() const
    {
    return std::unique_ptr<BeSqliteDbMutex>(new BeSqliteDbMutex(const_cast<ECDb&>(GetECDb())));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 BentleyStatus SchemaReader::GetSchemas(bvector<ECN::ECSchemaCP>& schemas, bool loadSchemaEntities) const
    {
    CachedStatementPtr stmt = nullptr;
    if (GetTableSpace().IsMain())
        stmt = GetCachedStatement("SELECT Id FROM main." TABLE_Schema);
    else 
        stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Schema, GetTableSpace().GetName().c_str()).c_str());

    if (stmt == nullptr)
        return ERROR;

    std::vector<ECSchemaId> schemaIds;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        schemaIds.push_back(stmt->GetValueId<ECSchemaId>(0));
        }

    stmt = nullptr; // in case the child call needs to reuse this statement

    for (ECSchemaId schemaId : schemaIds)
        {
        ECSchemaCP schema = GetSchema(schemaId, loadSchemaEntities);
        if (schema == nullptr)
            return ERROR;

        schemas.push_back(schema);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaReader::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode) const
    {
    const ECSchemaId schemaId = SchemaPersistenceHelper::GetSchemaId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), mode);
    if (!schemaId.IsValid())
        return nullptr;

    return GetSchema(schemaId, loadSchemaEntities);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaReader::GetSchema(ECSchemaId ecSchemaId, bool loadSchemaEntities) const
    {
    Context ctx;
    ECSchemaCP schema = GetSchema(ctx, ecSchemaId, loadSchemaEntities);
    if (schema == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return schema;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaReader::GetSchema(Context& ctx, ECSchemaId ecSchemaId, bool loadSchemaEntities) const
    {
    SchemaDbEntry* outECSchemaKey = nullptr;
    if (SUCCESS != ReadSchema(outECSchemaKey, ctx, ecSchemaId, loadSchemaEntities))
        return nullptr;

    return outECSchemaKey->m_cachedSchema.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaId SchemaReader::GetSchemaId(ECSchemaCR ecSchema) const
    {
    if (ecSchema.HasId())
        {
        BeAssert(ecSchema.GetId() == SchemaPersistenceHelper::GetSchemaId(GetECDb(), GetTableSpace(), ecSchema.GetName().c_str(), SchemaLookupMode::ByName));
        return ecSchema.GetId();
        }

    const ECSchemaId schemaId = SchemaPersistenceHelper::GetSchemaId(GetECDb(), GetTableSpace(), ecSchema.GetName().c_str(), SchemaLookupMode::ByName);
    if (schemaId.IsValid())
        {
        //it is possible that the schema was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECSchemaR>(ecSchema).SetId(schemaId);
        }

    return schemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP SchemaReader::GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode) const
    {
    const ECClassId id = GetClassId(schemaNameOrAlias, className, mode);
    if (!id.IsValid())
        return nullptr;

    return GetClass(id);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaReader::GetClass(ECClassId ecClassId) const
    {
    Context ctx;
    ECClassCP ecclass = GetClass(ctx, ecClassId);
    if (ecclass != nullptr)
        {
        if (SUCCESS == ctx.Postprocess(*this))
            return ecclass;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP SchemaReader::GetClass(Context& ctx, ECClassId ecClassId) const
    {
    if (!ecClassId.IsValid())
        return nullptr;

    {
    auto lockECDb = LockECDb();
    ClassDbEntry* cacheEntry = m_cache.Find(ecClassId);
    if (cacheEntry == nullptr)
        {
        //ECDb allows nullptr as entry in the cache to indicate that this is a class which was attempted
        //to be loaded before but failed. Subsequent calls don't have to attempt a load anymore, so nullptr
        //can be returned right away.
        if (m_cache.HasClassEntry(ecClassId))
            return nullptr;
        }
    else
        return cacheEntry->m_cachedClass;
    }

    auto lockDb = LockDb();
    auto lockECDb = LockECDb();
    
    if (ClassDbEntry* cacheEntry = m_cache.Find(ecClassId))
        return cacheEntry->m_cachedClass;

    ECDbExpressionSymbolContext symbolsContext(GetECDb());
    const int schemaIdColIx = 0;
    const int nameColIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int modifierColIx = 5;
    const int relStrengthColIx = 6;
    const int relStrengthDirColIx = 7;  
    const int caContainerTypeIx = 8;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType FROM [%s]." TABLE_Class " WHERE Id=?", GetTableSpace().GetName().c_str()).c_str());
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
    Nullable<ECClassType> classType = SchemaPersistenceHelper::ToClassType(stmt->GetValueInt(typeColIx));
    Nullable<ECClassModifier> classModifier = SchemaPersistenceHelper::ToClassModifier(stmt->GetValueInt(modifierColIx));

    SchemaDbEntry* schemaKey = nullptr;
    if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
        return nullptr;

    BeAssert(schemaKey != nullptr);
    BeAssert(schemaKey->m_cachedSchema.IsValid());

    ECSchemaR schema = *schemaKey->m_cachedSchema;

    if (classType.IsNull() || classModifier.IsNull())
        {
        LOG.errorv("Failed to load ECClass %s.%s. Its ECClassType or ECClassModifier is unsupported. The file might have been used with newer versions of the software.",
                   schema.GetName().c_str(), className);
        return nullptr;
        }

    ECClassP ecClass = nullptr;
    switch (classType.Value())
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
            if (schema.CreateRelationshipClass(newClass, className, false) != ECObjectsStatus::Success)
                return nullptr;

            BeAssert(!stmt->IsColumnNull(relStrengthColIx) && !stmt->IsColumnNull(relStrengthDirColIx));

            Nullable<StrengthType> strengthType = SchemaPersistenceHelper::ToStrengthType(stmt->GetValueInt(relStrengthColIx));
            Nullable<ECRelatedInstanceDirection> strengthDir = SchemaPersistenceHelper::ToECRelatedInstanceDirection(stmt->GetValueInt(relStrengthDirColIx));
            if (strengthType.IsNull() || strengthDir.IsNull())
                {
                LOG.errorv("Failed to load ECRelationshipClass %s.%s. Its StrengthType or ECRelatedInstanceDirection is unsupported. The file might have been used with newer versions of the software.",
                           schema.GetName().c_str(), className);
                return nullptr;
                }

            newClass->SetStrength(strengthType.Value());
            newClass->SetStrengthDirection(strengthDir.Value());
            ecClass = newClass;
            break;
            }

            default:
                BeAssert(false);
                return nullptr;
        }

    ecClass->SetId(ecClassId);
    ecClass->SetClassModifier(classModifier.Value());

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        ecClass->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        ecClass->SetDescription(description);

    BeAssert(stmt->Step() == BE_SQLITE_DONE);
    stmt = nullptr; //to release it, so that it can be reused without repreparation

    //cache the class, before loading properties and base classes, because the class can be referenced by other classes (e.g. via nav props)
    m_cache.Insert(std::unique_ptr<ClassDbEntry>(new ClassDbEntry(*ecClass)));
    if (SUCCESS != LoadClassComponentsFromDb(ctx, *ecClass))
        {
        //set the cache entry to nullptr if the class could not be loaded so that future calls will return nullptr without querying into the DB
        //(It is not expected that a future call will succeed, so returning nullptr is correct)
        m_cache.SetClassEntryToNull(ecClassId);
        return nullptr;
        }

    schemaKey->m_loadedTypeCount++;
    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::LoadClassComponentsFromDb(Context& ctx, ECN::ECClassR ecClass) const
    {
    if (SUCCESS != LoadCAFromDb(ecClass, ctx, ECContainerId(ecClass.GetId()), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class))
        return ERROR;

    if (SUCCESS != LoadMixinAppliesToClass(ctx, ecClass))
        return ERROR;

    if (SUCCESS != LoadBaseClassesFromDb(ctx, ecClass))
        return ERROR;

    if (SUCCESS != LoadPropertiesFromDb(ctx, ecClass))
        return ERROR;

    ECRelationshipClassP relClass = ecClass.GetRelationshipClassP();
    if (relClass != nullptr)
        {
        if (SUCCESS != LoadRelationshipConstraintFromDb(relClass, ctx, relClass->GetId(), ECRelationshipEnd_Source))
            return ERROR;

        if (SUCCESS != LoadRelationshipConstraintFromDb(relClass, ctx, relClass->GetId(), ECRelationshipEnd_Target))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId SchemaReader::GetClassId(ECClassCR ecClass) const
    {
    if (ecClass.HasId()) //This is unsafe but since we do not delete ecclass any class that hasId does exist in db
        {
        BeAssert(ecClass.GetId() == GetClassId(ecClass.GetSchema().GetName(), ecClass.GetName(), SchemaLookupMode::ByName));
        return ecClass.GetId();
        }

    ECClassId classId;
    if (ecClass.GetSchema().HasId())
        {
        //If the ECSchema already has an id, we can run a faster SQL to get the class id
        BeAssert(ecClass.GetSchema().GetId() == SchemaPersistenceHelper::GetSchemaId(GetECDb(), GetTableSpace(), ecClass.GetSchema().GetName().c_str(), SchemaLookupMode::ByName));
        classId = SchemaPersistenceHelper::GetClassId(GetECDb(), GetTableSpace(), ecClass.GetSchema().GetId(), ecClass.GetName().c_str());
        }
    else
        classId = GetClassId(ecClass.GetSchema().GetName(), ecClass.GetName(), SchemaLookupMode::ByName);

    if (classId.IsValid())
        {
        //it is possible that the ECClass was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECClassR>(ecClass).SetId(classId);
        }

    return classId;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId SchemaReader::GetClassId(Utf8StringCR schemaName, Utf8StringCR className, SchemaLookupMode lookupMode) const
    {
    //Always looking up the ECClassId from the DB seems too slow. Therefore cache the requested ids.
    ECClassId ecClassId = m_cache.Find(schemaName, className);
    if (!ecClassId.IsValid())
        {
        ecClassId = SchemaPersistenceHelper::GetClassId(GetECDb(), GetTableSpace(), schemaName.c_str(), className.c_str(), lookupMode);

        //add id to cache (only if valid class id to avoid overflow of the cache)
        if (ecClassId.IsValid())
            m_cache.Insert(schemaName, className, ecClassId);
        }

    return ecClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaReader::GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode schemaLookupMode) const
    {
    ECEnumerationId enumId = SchemaPersistenceHelper::GetEnumerationId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), enumName.c_str(), schemaLookupMode);
    if (!enumId.IsValid())
        return nullptr;

    Context ctx;

    ECEnumerationP ecEnum = nullptr;
    if (SUCCESS != ReadEnumeration(ecEnum, ctx, enumId))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return ecEnum;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//---------------------------------------------------------------------------------------
ECEnumerationId SchemaReader::GetEnumerationId(ECEnumerationCR ecEnum) const
    {
    if (ecEnum.HasId()) //This is unsafe but since we do not delete ecenum any class that hasId does exist in db
        {
        BeAssert(ecEnum.GetId() == SchemaPersistenceHelper::GetEnumerationId(GetECDb(), GetTableSpace(), ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str(), SchemaLookupMode::ByName));
        return ecEnum.GetId();
        }

    const ECEnumerationId id = SchemaPersistenceHelper::GetEnumerationId(GetECDb(), GetTableSpace(), ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the ECEnumeration was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECEnumerationR>(ecEnum).SetId(id);
        }

    return id;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaReader::GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode schemaLookupMode) const
    {
    KindOfQuantityId koqId = SchemaPersistenceHelper::GetKindOfQuantityId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), koqName.c_str(), schemaLookupMode);
    if (!koqId.IsValid())
        return nullptr;

    Context ctx;

    KindOfQuantityP koq = nullptr;
    if (SUCCESS != ReadKindOfQuantity(koq, ctx, koqId))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return koq;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/2016
//---------------------------------------------------------------------------------------
KindOfQuantityId SchemaReader::GetKindOfQuantityId(KindOfQuantityCR koq) const
    {
    if (koq.HasId()) //This is unsafe but since we do not delete KOQ any class that hasId does exist in db
        {
        BeAssert(koq.GetId() == SchemaPersistenceHelper::GetKindOfQuantityId(GetECDb(), GetTableSpace(), koq.GetSchema().GetName().c_str(), koq.GetName().c_str(), SchemaLookupMode::ByName));
        return koq.GetId();
        }

    const KindOfQuantityId id = SchemaPersistenceHelper::GetKindOfQuantityId(GetECDb(), GetTableSpace(), koq.GetSchema().GetName().c_str(), koq.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the KOQ was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<KindOfQuantityR>(koq).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP SchemaReader::GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR catName, SchemaLookupMode schemaLookupMode) const
    {
    PropertyCategoryId catId = SchemaPersistenceHelper::GetPropertyCategoryId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), catName.c_str(), schemaLookupMode);
    if (!catId.IsValid())
        return nullptr;

    Context ctx;

    PropertyCategoryP cat = nullptr;
    if (SUCCESS != ReadPropertyCategory(cat, ctx, catId))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return cat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/2017
//---------------------------------------------------------------------------------------
PropertyCategoryId SchemaReader::GetPropertyCategoryId(PropertyCategoryCR cat) const
    {
    if (cat.HasId()) //This is unsafe but since we do not delete PropertyCategory any class that hasId does exist in db
        {
        BeAssert(cat.GetId() == SchemaPersistenceHelper::GetPropertyCategoryId(GetECDb(), GetTableSpace(), cat.GetSchema().GetName().c_str(), cat.GetName().c_str(), SchemaLookupMode::ByName));
        return cat.GetId();
        }

    const PropertyCategoryId id = SchemaPersistenceHelper::GetPropertyCategoryId(GetECDb(), GetTableSpace(), cat.GetSchema().GetName().c_str(), cat.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the PropertyCategory was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<PropertyCategoryR>(cat).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/2016
//---------------------------------------------------------------------------------------
ECPropertyId SchemaReader::GetPropertyId(ECPropertyCR prop) const
    {
    if (prop.HasId()) //This is unsafe but since we do not delete KOQ any class that hasId does exist in db
        {
        BeAssert(prop.GetId() == SchemaPersistenceHelper::GetPropertyId(GetECDb(), GetTableSpace(), prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str(), SchemaLookupMode::ByName));
        return prop.GetId();
        }

    ECPropertyId propId;
    if (prop.GetClass().HasId())
        {
        //If the ECClass already has an id, we can run a faster SQL to get the property id
        BeAssert(prop.GetClass().GetId() == SchemaPersistenceHelper::GetClassId(GetECDb(), GetTableSpace(), prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), SchemaLookupMode::ByName));

        propId = SchemaPersistenceHelper::GetPropertyId(GetECDb(), GetTableSpace(), prop.GetClass().GetId(), prop.GetName().c_str());
        }
    else
        propId = SchemaPersistenceHelper::GetPropertyId(GetECDb(), GetTableSpace(), prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str(), SchemaLookupMode::ByName);

    if (propId.IsValid())
        {
        //it is possible that the property was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECPropertyR>(prop).SetId(propId);
        }

    return propId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus SchemaReader::EnsureDerivedClassesExist(ECClassId ecClassId) const
    {
    {
    auto lockECDb = LockECDb();
    if (ClassDbEntry* entry = m_cache.Find(ecClassId))
        {
        if (entry->m_ensureDerivedClassesExist)
            return SUCCESS;

        }
    }

    auto dbLock = LockDb();
    auto lockECDb = LockECDb();
    if (ClassDbEntry* entry = m_cache.Find(ecClassId))
        {
        if (entry->m_ensureDerivedClassesExist)
            return SUCCESS;

        entry->m_ensureDerivedClassesExist = true;
        }

    Context ctx;
    if (SUCCESS != EnsureDerivedClassesExist(ctx, ecClassId))
        return ERROR;

    return ctx.Postprocess(*this);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus SchemaReader::EnsureDerivedClassesExist(Context& ctx, ECClassId ecClassId) const
    {
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT ClassId FROM [%s]." TABLE_ClassHasBaseClasses " WHERE BaseClassId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        if (GetClass(ctx, stmt->GetValueId<ECClassId>(0)) == nullptr)
            return ERROR;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadEnumeration(ECEnumerationP& ecEnum, Context& ctx, ECEnumerationId enumId) const
    {
    {
    auto lockECDb = LockECDb();
    if (EnumDbEntry* entry = m_cache.Find(enumId))
        {
        ecEnum = entry->m_cachedEnum;
        return SUCCESS;
        }
    }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int isStrictColIx = 5;
    const int valuesColIx = 6;

    auto dbLock = LockDb();
    auto lockECDb = LockECDb();
    if (EnumDbEntry* entry = m_cache.Find(enumId))
        {
        ecEnum = entry->m_cachedEnum;
        return SUCCESS;
        }

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM [%s]." TABLE_Enumeration " WHERE Id=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
    SchemaDbEntry* schemaKey = nullptr;
    if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP enumName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    const PrimitiveType underlyingType = (PrimitiveType) stmt->GetValueInt(typeColIx);
    const bool isStrict = stmt->GetValueBoolean(isStrictColIx);
    Utf8CP enumValuesJsonStr = stmt->GetValueText(valuesColIx);

    ecEnum = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreateEnumeration(ecEnum, enumName, underlyingType))
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

    if (SUCCESS != SchemaPersistenceHelper::DeserializeEnumerationValues(*ecEnum, enumValuesJsonStr))
        return ERROR;

    //cache the enum
    m_cache.Insert(std::unique_ptr<EnumDbEntry>(new EnumDbEntry(enumId, *ecEnum)));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadKindOfQuantity(KindOfQuantityP& koq, Context& ctx, KindOfQuantityId koqId) const
    {
    {
    auto lockECDb = LockECDb();
    if (KindOfQuantityDbEntry* entry = m_cache.Find(koqId))
        {
        koq = entry->m_cachedKoq;
        return SUCCESS;
        }
    }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int persUnitColIx = 4;
    const int relErrorColIx = 5;
    const int presUnitColIx = 6;

    auto dbLock = LockDb();
    auto lockECDb = LockECDb();
    if (KindOfQuantityDbEntry* entry = m_cache.Find(koqId))
        {
        koq = entry->m_cachedKoq;
        return SUCCESS;
        }

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT SchemaId,Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits FROM [%s]." TABLE_KindOfQuantity " WHERE Id=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, koqId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
    SchemaDbEntry* schemaKey = nullptr;
    if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP koqName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    BeAssert(!stmt->IsColumnNull(persUnitColIx));
    Utf8CP persUnitStr = stmt->GetValueText(persUnitColIx);
    const double relError = stmt->GetValueDouble(relErrorColIx);
    Utf8CP presUnitsStr = stmt->IsColumnNull(presUnitColIx) ? nullptr : stmt->GetValueText(presUnitColIx);

    koq = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreateKindOfQuantity(koq, koqName))
        return ERROR;

    koq->SetId(koqId);

    if (displayLabel != nullptr)
        koq->SetDisplayLabel(displayLabel);

    koq->SetDescription(description);

    BeAssert(!Utf8String::IsNullOrEmpty(persUnitStr));
    if (!koq->SetPersistenceUnit(Formatting::FormatUnitSet(persUnitStr)))
        {
        BeAssert(!koq->GetPersistenceUnit().HasProblem() && "KOQ Persistence Unit could not be deserialized correctly. It has an invalid format");
        return ERROR;
        }

    koq->SetRelativeError(relError);

    if (!Utf8String::IsNullOrEmpty(presUnitsStr))
        {
        if (SUCCESS != SchemaPersistenceHelper::DeserializeKoqPresentationUnits(*koq, presUnitsStr))
            return ERROR;
        }

    //cache the koq
    m_cache.Insert(std::unique_ptr<KindOfQuantityDbEntry>(new KindOfQuantityDbEntry(koqId, *koq)));
    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadPropertyCategory(PropertyCategoryP& cat, Context& ctx, PropertyCategoryId catId) const
    {
    {
    auto lockECDb = LockECDb();
    if (PropertyCategoryDbEntry* entry = m_cache.Find(catId))
        {
        cat = entry->m_cachedCategory;
        return SUCCESS;
        }
    }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int priorityColIx = 4;

    auto dbLock = LockDb();
    auto lockECDb = LockECDb();
    if (PropertyCategoryDbEntry* entry = m_cache.Find(catId))
        {
        cat = entry->m_cachedCategory;
        return SUCCESS;
        }

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT SchemaId,Name,DisplayLabel,Description,Priority FROM [%s]." TABLE_PropertyCategory " WHERE Id=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, catId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
    SchemaDbEntry* schemaKey = nullptr;
    if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
        return ERROR;

    Utf8CP catName = stmt->GetValueText(nameIx);
    Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
    Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);
    Nullable<uint32_t> prio;
    if (!stmt->IsColumnNull(priorityColIx))
        {
        //uint32_t is persisted as int64 to not lose unsigned-ness
        prio = (uint32_t) stmt->GetValueInt64(priorityColIx);
        }

    cat = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreatePropertyCategory(cat, catName))
        return ERROR;

    cat->SetId(catId);

    if (displayLabel != nullptr)
        cat->SetDisplayLabel(displayLabel);

    cat->SetDescription(description);

    if (!prio.IsNull())
        cat->SetPriority(prio.Value());

    //cache the category
    m_cache.Insert(std::unique_ptr<PropertyCategoryDbEntry>(new PropertyCategoryDbEntry(catId, *cat)));
    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadSchemaDefinition(SchemaDbEntry*& schemaEntry, bvector<SchemaDbEntry*>& newlyLoadedSchemas, ECSchemaId ecSchemaId) const
    {
    {
    auto lockECDb = LockECDb();
    if (schemaEntry = m_cache.Find(ecSchemaId))
        {
        BeAssert(schemaEntry->m_cachedSchema != nullptr);
        return SUCCESS;
        }
    }

    auto dbLock = LockDb();
    auto lockECDb = LockECDb();
    if (schemaEntry = m_cache.Find(ecSchemaId))
        {
        BeAssert(schemaEntry->m_cachedSchema != nullptr);
        return SUCCESS;
        }

    //Following method is not by itself thread safe as it write to cache but 
    //this is the only call to it which is thread safe.
    if (SUCCESS != LoadSchemaFromDb(schemaEntry, ecSchemaId))
        return ERROR;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT ReferencedSchemaId FROM [%s]." TABLE_SchemaReference " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
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
        SchemaDbEntry* referenceSchemaKey = nullptr;
        if (SUCCESS != LoadSchemaDefinition(referenceSchemaKey, newlyLoadedSchemas, referencedSchemaId))
            return ERROR;

        ECObjectsStatus s = schemaEntry->m_cachedSchema->AddReferencedSchema(*referenceSchemaKey->m_cachedSchema);
        if (s != ECObjectsStatus::Success)
            return ERROR;
        }

    BeAssert(schemaEntry->m_cachedSchema != nullptr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::ReadSchema(SchemaDbEntry*& outECSchemaKey, Context& ctx, ECSchemaId schemaId, bool loadSchemaEntities) const
    {
    bvector<SchemaDbEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadSchemaDefinition(outECSchemaKey, newlyLoadedSchemas, schemaId))
        return ERROR;

    for (SchemaDbEntry* newlyLoadedSchema : newlyLoadedSchemas)
        {
        ECSchemaR schema = *newlyLoadedSchema->m_cachedSchema;
        ctx.AddSchemaToLoadCAInstanceFor(schema);
        }

    if (loadSchemaEntities && !outECSchemaKey->IsFullyLoaded())
        {
        std::set<SchemaDbEntry*> fullyLoadedSchemas;
        if (SUCCESS != LoadSchemaEntitiesFromDb(outECSchemaKey, ctx, fullyLoadedSchemas))
            return ERROR;

        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadSchemaEntitiesFromDb(SchemaDbEntry* ecSchemaKey, Context& ctx, std::set<SchemaDbEntry*>& fullyLoadedSchemas) const
    {
    BeAssert(ecSchemaKey != nullptr);
    if (!ecSchemaKey)
        return ERROR;

    if (fullyLoadedSchemas.find(ecSchemaKey) != fullyLoadedSchemas.end())
        return SUCCESS;

    //Enure all reference schemas also loaded
    for (auto& refSchemaKey : ecSchemaKey->m_cachedSchema->GetReferencedSchemas())
        {
        ECSchemaId referenceECSchemaId = refSchemaKey.second->GetId();
        SchemaDbEntry* key = m_cache.Find(referenceECSchemaId);
        if (SUCCESS != LoadSchemaEntitiesFromDb(key, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded())
        return SUCCESS;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Class " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const ECClassId classId = stmt->GetValueId<ECClassId>(0);
        if (nullptr == GetClass(ctx, classId))
            {
            LOG.errorv("Could not load ECClass with id %" PRIu64 " from schema %s", classId.GetValue(), ecSchemaKey->m_cachedSchema->GetName().c_str());
            return ERROR;
            }

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Enumeration " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECEnumerationP ecEnum = nullptr;
        if (SUCCESS != ReadEnumeration(ecEnum, ctx, stmt->GetValueId<ECEnumerationId>(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_KindOfQuantity " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
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

    stmt = nullptr;
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_PropertyCategory " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaKey->GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        PropertyCategoryP cat = nullptr;
        if (SUCCESS != ReadPropertyCategory(cat, ctx, stmt->GetValueId<PropertyCategoryId>(0)))
            return ERROR;

        if (ecSchemaKey->IsFullyLoaded())
            return SUCCESS;
        }
       
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadSchemaFromDb(SchemaDbEntry*& schemaEntry, ECSchemaId ecSchemaId) const
    {
    Utf8CP tableSpace = GetTableSpace().GetName().c_str();
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT S.Name, S.DisplayLabel,S.Description,S.Alias,S.VersionDigit1,S.VersionDigit2,S.VersionDigit3, "
                                                        "(SELECT COUNT(*) FROM [%s]." TABLE_Class "  C WHERE S.Id = C.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM [%s]." TABLE_Enumeration " e WHERE S.Id = e.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM [%s]." TABLE_KindOfQuantity " koq WHERE S.Id = koq.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM [%s]." TABLE_PropertyCategory " cat WHERE S.Id = cat.SchemaId) "
                                                        "FROM [%s]." TABLE_Schema " S WHERE S.Id=?", tableSpace, tableSpace, tableSpace, tableSpace, tableSpace).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    Utf8CP schemaName = stmt->GetValueText(0);
    Utf8CP displayLabel = stmt->IsColumnNull(1) ? nullptr : stmt->GetValueText(1);
    Utf8CP description = stmt->IsColumnNull(2) ? nullptr : stmt->GetValueText(2);
    Utf8CP alias = stmt->GetValueText(3);
    //uint32_t is persisted as int64 to not lose unsigned-ness
    uint32_t versionMajor = (uint32_t) stmt->GetValueInt64(4);
    uint32_t versionWrite = (uint32_t) stmt->GetValueInt64(5);
    uint32_t versionMinor = (uint32_t) stmt->GetValueInt64(6);
    const int typesInSchema = stmt->GetValueInt(7);

    ECSchemaPtr schema = nullptr;
    if (ECSchema::CreateSchema(schema, schemaName, alias, versionMajor, versionWrite, versionMinor) != ECObjectsStatus::Success)
        return ERROR;

    schema->SetId(ecSchemaId);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        schema->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        schema->SetDescription(description);

    schemaEntry = new SchemaDbEntry(schema, typesInSchema);
    m_cache.Insert(std::unique_ptr<SchemaDbEntry>(schemaEntry));   
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     03/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::LoadMixinAppliesToClass(Context& ctx, ECN::ECClassCR mixinClass) const
    {
    IECInstancePtr mixinCA = mixinClass.GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
    if (mixinCA == nullptr)
        return SUCCESS;

    ECValue appliesToValue;
    if (ECObjectsStatus::Success != mixinCA->GetValue(appliesToValue, "AppliesToEntityClass"))
        {
        LOG.errorv("Could not load Mixin ECClass %s. Could not read the IsMixin Custom Attribute's property 'AppliesToEntityClass'.", mixinClass.GetFullName());
        return ERROR;
        }

    Utf8CP val = nullptr;
    if (appliesToValue.IsNull() || !appliesToValue.IsString() || Utf8String::IsNullOrEmpty((val = appliesToValue.GetUtf8CP())))
        {
        LOG.errorv("Could not load Mixin ECClass %s. The IsMixin Custom Attribute's property 'AppliesToEntityClass' is unset or has an invalid value.", mixinClass.GetFullName());
        return ERROR;
        }

    Utf8String appliesToSchemaAlias;
    Utf8String appliesToClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(appliesToSchemaAlias, appliesToClassName, val))
        {
        LOG.errorv("Could not load Mixin ECClass %s. The IsMixin Custom Attribute has an invalid value for 'AppliesToEntityClass': %s", mixinClass.GetFullName(), val);
        return ERROR;
        }

    SchemaLookupMode schemaLookupMode = SchemaLookupMode::AutoDetect;
    Utf8StringCP effectiveSchemaName = &appliesToSchemaAlias;
    if (appliesToSchemaAlias.empty())
        {
        effectiveSchemaName = &mixinClass.GetSchema().GetName();
        schemaLookupMode = SchemaLookupMode::ByName;
        }

    BeAssert(effectiveSchemaName != nullptr);

    ECClassId appliesToClassId = GetClassId(*effectiveSchemaName, appliesToClassName, schemaLookupMode);
    if (!appliesToClassId.IsValid() || 
        //this is the important step and the mere purpose of the routine. We need to load the applies to class into memory
        //so that ECObjects can validate the mixin.
        GetClass(ctx, appliesToClassId) == nullptr) 
        {
        LOG.errorv("Could not load Mixin ECClass %s. The 'applies to' class '%s.%s' does not exist.", mixinClass.GetFullName(), appliesToSchemaAlias.c_str(), appliesToClassName.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadPropertiesFromDb(Context& ctx, ECClassR ecClass) const
    {
    struct PropReaderHelper
        {
        struct RowInfo
            {
            ECPropertyId m_id;
            PropertyKind m_kind;
            Utf8String m_name;
            Utf8String m_displayLabel;
            Utf8String m_description;
            bool m_isReadonly = false;
            Nullable<int64_t> m_priority;
            Nullable<int> m_primType;
            Nullable<int64_t> m_primTypeMinLength;
            Nullable<int64_t> m_primTypeMaxLength;
            ECValue m_primTypeMinValue;
            ECValue m_primTypeMaxValue;
            ECClassId m_structClassId;
            Utf8String m_extendedTypeName;
            ECEnumerationId m_enumId;
            KindOfQuantityId m_koqId;
            PropertyCategoryId m_catId;
            int64_t m_arrayMinOccurs = INT64_C(0);
            Nullable<int64_t> m_arrayMaxOccurs;
            ECClassId m_navPropRelClassId;
            Nullable<int> m_navPropDirection;
            };

        static BentleyStatus ReadRows(std::vector<RowInfo>& rows, ECDbCR ecdb, DbTableSpace const& tableSpace, ECClassCR ecClass)
            {
            const int idIx = 0;
            const int kindIx = 1;
            const int nameIx = 2;
            const int displayLabelIx = 3;
            const int descrIx = 4;
            const int isReadonlyIx = 5;
            const int priorityIx = 6;
            const int primTypeIx = 7;
            const int primTypeMinLengthIx = 8;
            const int primTypeMaxLengthIx = 9;
            const int primTypeMinValueIx = 10;
            const int primTypeMaxValueIx = 11;
            const int enumIdIx = 12;
            const int structClassIdIx = 13;
            const int extendedTypeIx = 14;
            const int koqIdIx = 15;
            const int catIdIx = 16;
            const int minOccursIx = 17;
            const int maxOccursIx = 18;
            const int navRelationshipClassId = 19;
            const int navPropDirectionIx = 20;

            CachedStatementPtr stmt = nullptr;
            if (tableSpace.IsMain())
                stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id,Kind,Name,DisplayLabel,Description,IsReadonly,Priority,"
                                                              "PrimitiveType,PrimitiveTypeMinLength,PrimitiveTypeMaxLength,PrimitiveTypeMinValue,PrimitiveTypeMaxValue,"
                                                              "EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,"
                                                              "ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection "
                                                              "FROM main." TABLE_Property " WHERE ClassId=? ORDER BY Ordinal");
            else
                stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Id,Kind,Name,DisplayLabel,Description,IsReadonly,Priority,"
                                                               "PrimitiveType,PrimitiveTypeMinLength,PrimitiveTypeMaxLength,PrimitiveTypeMinValue,PrimitiveTypeMaxValue,"
                                                               "EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,"
                                                               "ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection "
                                                               "FROM [%s]." TABLE_Property " WHERE ClassId=? ORDER BY Ordinal", tableSpace.GetName().c_str()).c_str());

            if (stmt == nullptr)
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(1, ecClass.GetId()))
                return ERROR;

            while (BE_SQLITE_ROW == stmt->Step())
                {
                RowInfo rowInfo;
                rowInfo.m_id = stmt->GetValueId<ECPropertyId>(idIx);
                rowInfo.m_name.assign(stmt->GetValueText(nameIx));

                Nullable<PropertyKind> kind = SchemaPersistenceHelper::ToPropertyKind(stmt->GetValueInt(kindIx));
                if (kind.IsNull())
                    {
                    LOG.errorv("Failed to load ECProperty %s.%s. It is an unsupported type of ECProperty. The file might have been used with newer versions of the software.",
                               ecClass.GetFullName(), rowInfo.m_name.c_str());
                    return ERROR;
                    }

                rowInfo.m_kind = kind.Value();

                if (!stmt->IsColumnNull(displayLabelIx))
                    rowInfo.m_displayLabel.assign(stmt->GetValueText(displayLabelIx));

                if (!stmt->IsColumnNull(descrIx))
                    rowInfo.m_description.assign(stmt->GetValueText(descrIx));

                if (!stmt->IsColumnNull(isReadonlyIx))
                    rowInfo.m_isReadonly = stmt->GetValueBoolean(isReadonlyIx);

                //uint32_t are persisted as int64 to not lose unsigned-ness
                if (!stmt->IsColumnNull(priorityIx))
                    rowInfo.m_priority = stmt->GetValueInt64(priorityIx);

                if (!stmt->IsColumnNull(primTypeIx))
                    {
                    rowInfo.m_primType = stmt->GetValueInt(primTypeIx);
                    if (SchemaPersistenceHelper::ToPrimitiveType(rowInfo.m_primType.Value()).IsNull())
                        {
                        LOG.errorv("Failed to load ECProperty %s.%s. It has an unsupported primitive data type. The file might have been used with newer versions of the software.",
                                   ecClass.GetFullName(), rowInfo.m_name.c_str());
                        return ERROR;
                        }
                    }

                //MinLength/MaxLength is persisted as int64 to not lose unsigned-ness
                if (!stmt->IsColumnNull(primTypeMinLengthIx))
                    rowInfo.m_primTypeMinLength = stmt->GetValueInt64(primTypeMinLengthIx);

                if (!stmt->IsColumnNull(primTypeMaxLengthIx))
                    rowInfo.m_primTypeMaxLength = stmt->GetValueInt64(primTypeMaxLengthIx);

                if (SUCCESS != ReadMinMaxValue(rowInfo.m_primTypeMinValue, rowInfo, *stmt, primTypeMinValueIx))
                    return ERROR;

                if (SUCCESS != ReadMinMaxValue(rowInfo.m_primTypeMaxValue, rowInfo, *stmt, primTypeMaxValueIx))
                    return ERROR;

                if (!stmt->IsColumnNull(enumIdIx))
                    rowInfo.m_enumId = stmt->GetValueId<ECEnumerationId>(enumIdIx);

                if (stmt->IsColumnNull(structClassIdIx))
                    {
                    if (kind == PropertyKind::Struct || kind == PropertyKind::StructArray)
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

                if (rowInfo.m_koqId.IsValid() && kind == PropertyKind::Navigation)
                    {
                    BeAssert(false && "KindOfQuantityId must only be set for primitive or primitive array props");
                    return ERROR;
                    }

                if (!stmt->IsColumnNull(catIdIx))
                    rowInfo.m_catId = stmt->GetValueId<PropertyCategoryId>(catIdIx);

                if (stmt->IsColumnNull(minOccursIx))
                    {
                    if (kind == PropertyKind::PrimitiveArray || kind == PropertyKind::StructArray)
                        {
                        BeAssert(false && "ArrayMinOccurs column must not be NULL for array property");
                        return ERROR;
                        }
                    }
                else
                    {
                    //uint32_t are persisted as int64 to not lose the unsigned-ness.
                    rowInfo.m_arrayMinOccurs = stmt->GetValueInt64(minOccursIx);
                    //unbound maxOccurs is persisted as DB NULL.
                    if (!stmt->IsColumnNull(maxOccursIx))
                        rowInfo.m_arrayMaxOccurs = stmt->GetValueInt64(maxOccursIx);
                    }

                if (stmt->IsColumnNull(navRelationshipClassId))
                    {
                    if (kind == PropertyKind::Navigation)
                        {
                        BeAssert(false && "NavigationRelationshipClassId column must not be NULL for navigation property");
                        return ERROR;
                        }
                    }
                else
                    rowInfo.m_navPropRelClassId = stmt->GetValueId<ECClassId>(navRelationshipClassId);

                if (stmt->IsColumnNull(navPropDirectionIx))
                    {
                    if (kind == PropertyKind::Navigation)
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

        static BentleyStatus ReadMinMaxValue(ECN::ECValueR val, RowInfo const& rowInfo, Statement& stmt, int colIx)
            {
            if (stmt.IsColumnNull(colIx))
                return SUCCESS;

            if (rowInfo.m_primType.IsNull())
                {
                BeAssert(false && "PrimitiveTypeMinValue or PrimitiveTypeMaxValue must not be set if PrimitiveType is NULL");
                return ERROR;
                }

            switch (rowInfo.m_primType.Value())
                {
                    case PrimitiveType::PRIMITIVETYPE_DateTime:
                    {
                    double jd = stmt.GetValueDouble(colIx);
                    const uint64_t jdMsec = DateTime::RationalDayToMsec(jd);
                    val.SetDateTimeTicks(DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000);
                    break;
                    }
                    case PrimitiveType::PRIMITIVETYPE_Double:
                        val.SetDouble(stmt.GetValueDouble(colIx));
                        break;

                    case PrimitiveType::PRIMITIVETYPE_Integer:
                        val.SetInteger(stmt.GetValueInt(colIx));
                        break;

                    case PrimitiveType::PRIMITIVETYPE_Long:
                        val.SetLong(stmt.GetValueInt64(colIx));
                        break;

                    case PrimitiveType::PRIMITIVETYPE_String:
                        val.SetUtf8CP(stmt.GetValueText(colIx), true);
                        break;

                    default:
                        BeAssert(false && "ECProperty MinimumValue/MaximumValue is of unexpected type");
                        return ERROR;
                }

            return SUCCESS;
            }

        static BentleyStatus AssignArrayBounds(ArrayECProperty& prop, RowInfo const& rowInfo)
            {
            //uint32_t was persisted as int64 to not lose unsigned-ness
            if (ECObjectsStatus::Success != prop.SetMinOccurs((uint32_t) rowInfo.m_arrayMinOccurs))
                return ERROR;

            if (!rowInfo.m_arrayMaxOccurs.IsNull())
                {
                //if maxoccurs is DB NULL, it means unbound. This is the default in ArrayECProperty
                if (ECObjectsStatus::Success != prop.SetMaxOccurs((uint32_t) rowInfo.m_arrayMaxOccurs.Value()))
                    return ERROR;
                }
            else
                {
                BeAssert(prop.IsStoredMaxOccursUnbounded());
                }

            return SUCCESS;
            }
        };

    std::vector<PropReaderHelper::RowInfo> rowInfos;
    if (SUCCESS != PropReaderHelper::ReadRows(rowInfos, GetECDb(), GetTableSpace(), ecClass))
        return ERROR;

    for (PropReaderHelper::RowInfo const& rowInfo : rowInfos)
        {
        ECPropertyP prop = nullptr;
        switch (rowInfo.m_kind)
            {
                case PropertyKind::Primitive:
                {
                BeAssert(!rowInfo.m_primType.IsNull() || rowInfo.m_enumId.IsValid());

                PrimitiveECPropertyP primProp = nullptr;

                if (rowInfo.m_enumId.IsValid())
                    {
                    ECEnumerationP ecenum = nullptr;
                    if (SUCCESS != ReadEnumeration(ecenum, ctx, rowInfo.m_enumId))
                        return ERROR;

                    if (ECObjectsStatus::Success != ecClass.CreateEnumerationProperty(primProp, rowInfo.m_name, *ecenum))
                        return ERROR;
                    }
                else
                    {
                    if (ECObjectsStatus::Success != ecClass.CreatePrimitiveProperty(primProp, rowInfo.m_name, (PrimitiveType) rowInfo.m_primType.Value()))
                        return ERROR;
                    }

                if (!rowInfo.m_extendedTypeName.empty())
                    {
                    if (ECObjectsStatus::Success != primProp->SetExtendedTypeName(rowInfo.m_extendedTypeName.c_str()))
                        return ERROR;
                    }

                prop = primProp;
                break;
                }

                case PropertyKind::Struct:
                {
                BeAssert(rowInfo.m_structClassId.IsValid());

                ECClassCP structClassRaw = GetClass(ctx, rowInfo.m_structClassId);
                if (structClassRaw == nullptr)
                    return ERROR;

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructECPropertyP structProp = nullptr;
                if (ECObjectsStatus::Success != ecClass.CreateStructProperty(structProp, rowInfo.m_name, *structClass))
                    return ERROR;

                prop = structProp;
                break;
                }

                case PropertyKind::PrimitiveArray:
                {
                BeAssert(!rowInfo.m_primType.IsNull() || rowInfo.m_enumId.IsValid());

                PrimitiveArrayECPropertyP arrayProp = nullptr;

                if (rowInfo.m_enumId.IsValid())
                    {
                    ECEnumerationP ecenum = nullptr;
                    if (SUCCESS != ReadEnumeration(ecenum, ctx, rowInfo.m_enumId))
                        return ERROR;

                    if (ECObjectsStatus::Success != ecClass.CreatePrimitiveArrayProperty(arrayProp, rowInfo.m_name, *ecenum))
                        return ERROR;
                    }
                else
                    {
                    BeAssert(!rowInfo.m_primType.IsNull());
                    if (ECObjectsStatus::Success != ecClass.CreatePrimitiveArrayProperty(arrayProp, rowInfo.m_name, (PrimitiveType) rowInfo.m_primType.Value()))
                        return ERROR;
                    }

                if (!rowInfo.m_extendedTypeName.empty())
                    {
                    if (ECObjectsStatus::Success != arrayProp->SetExtendedTypeName(rowInfo.m_extendedTypeName.c_str()))
                        return ERROR;
                    }

                if (SUCCESS != PropReaderHelper::AssignArrayBounds(*arrayProp, rowInfo))
                    return ERROR;

                prop = arrayProp;
                break;
                }

                case PropertyKind::StructArray:
                {
                BeAssert(rowInfo.m_structClassId.IsValid());
                ECClassCP structClassRaw = GetClass(ctx, rowInfo.m_structClassId);
                if (structClassRaw == nullptr)
                    return ERROR;

                ECStructClassCP structClass = structClassRaw->GetStructClassCP();
                if (nullptr == structClass)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                StructArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass.CreateStructArrayProperty(arrayProp, rowInfo.m_name, *structClass))
                    return ERROR;

                if (SUCCESS != PropReaderHelper::AssignArrayBounds(*arrayProp, rowInfo))
                    return ERROR;

                prop = arrayProp;
                break;
                }

                case PropertyKind::Navigation:
                {
                BeAssert(ecClass.IsEntityClass() || ecClass.IsRelationshipClass());
                BeAssert(rowInfo.m_navPropRelClassId.IsValid() && !rowInfo.m_navPropDirection.IsNull());

                Nullable<ECRelatedInstanceDirection> direction = SchemaPersistenceHelper::ToECRelatedInstanceDirection(rowInfo.m_navPropDirection.Value());
                if (direction.IsNull())
                    {
                    LOG.errorv("Failed to load NavigationECProperty %s.%s. Its relationship direction is unsupported. The file might have been used with newer versions of the software.",
                               ecClass.GetFullName(), rowInfo.m_name.c_str());
                    return ERROR;
                    }

                ECClassCP relClassRaw = GetClass(ctx, rowInfo.m_navPropRelClassId);
                if (relClassRaw == nullptr)
                    return ERROR;

                BeAssert(relClassRaw->IsRelationshipClass());
                NavigationECPropertyP navProp = nullptr;
                if (ecClass.IsEntityClass())
                    {
                    if (ECObjectsStatus::Success != ecClass.GetEntityClassP()->CreateNavigationProperty(navProp, rowInfo.m_name, *relClassRaw->GetRelationshipClassCP(), direction.Value(), false))
                        return ERROR;
                    }
                else if (ecClass.IsRelationshipClass())
                    {
                    if (ECObjectsStatus::Success != ecClass.GetRelationshipClassP()->CreateNavigationProperty(navProp, rowInfo.m_name, *relClassRaw->GetRelationshipClassCP(), direction.Value(), false))
                        return ERROR;
                    }
                else
                    {
                    BeAssert(false && "Can only create nav props for entity and relationship classes. Should have been caught at import time");
                    return ERROR;
                    }

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

        if (!rowInfo.m_priority.IsNull())
            prop->SetPriority((int32_t) rowInfo.m_priority.Value());

        if (!rowInfo.m_description.empty())
            prop->SetDescription(rowInfo.m_description);

        if (!rowInfo.m_displayLabel.empty())
            prop->SetDisplayLabel(rowInfo.m_displayLabel);

        if (!rowInfo.m_primTypeMinLength.IsNull())
            prop->SetMinimumLength((uint32_t) rowInfo.m_primTypeMinLength.Value());

        if (!rowInfo.m_primTypeMaxLength.IsNull())
            prop->SetMaximumLength((uint32_t) rowInfo.m_primTypeMaxLength.Value());

        if (!rowInfo.m_primTypeMinValue.IsNull())
            prop->SetMinimumValue(rowInfo.m_primTypeMinValue);

        if (!rowInfo.m_primTypeMaxValue.IsNull())
            prop->SetMaximumValue(rowInfo.m_primTypeMaxValue);

        if (rowInfo.m_koqId.IsValid())
            {
            KindOfQuantityP koq = nullptr;
            if (SUCCESS != ReadKindOfQuantity(koq, ctx, rowInfo.m_koqId))
                return ERROR;

            prop->SetKindOfQuantity(koq);
            }

        if (rowInfo.m_catId.IsValid())
            {
            PropertyCategoryP cat = nullptr;
            if (SUCCESS != ReadPropertyCategory(cat, ctx, rowInfo.m_catId))
                return ERROR;

            prop->SetCategory(cat);
            }

        if (SUCCESS != LoadCAFromDb(*prop, ctx, ECContainerId(rowInfo.m_id), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property))
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
BentleyStatus SchemaReader::LoadBaseClassesFromDb(Context& ctx, ECClassR ecClass) const
    {
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT BaseClassId FROM [%s]." TABLE_ClassHasBaseClasses " s WHERE ClassId=? ORDER BY Ordinal", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClass.GetId()))
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
        ECClassCP baseClass = GetClass(ctx, baseClassId);
        if (baseClass == nullptr)
            return ERROR;

        if (ECObjectsStatus::Success != ecClass.AddBaseClass(*baseClass, false, false, false))
            return ERROR;
        }

    return SUCCESS;
    }



/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR caConstainer, Context& ctx, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType) const
    {
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT ClassId,Instance FROM [%s]." TABLE_CustomAttribute " WHERE ContainerId=? AND ContainerType=? ORDER BY Ordinal", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId caClassId = stmt->GetValueId<ECClassId>(0);
        ECClassCP caClass = GetClass(ctx, caClassId);
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
BentleyStatus SchemaReader::LoadRelationshipConstraintFromDb(ECRelationshipClassP& ecRelationship, Context& ctx, ECClassId relationshipClassId, ECRelationshipEnd relationshipEnd) const
    {
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId FROM [%s]." TABLE_RelationshipConstraint " WHERE RelationshipClassId=? AND RelationshipEnd=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, relationshipEnd))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    const int constraintIdIx = 0;
    const int lowerLimitIx = 1;
    const int upperLimitIx = 2;
    const int isPolymorphicIx = 3;
    const int roleLabelIx = 4;
    const int abstractConstraintClassIdIx = 5;

    ECRelationshipConstraintId constraintId = stmt->GetValueId<ECRelationshipConstraintId>(constraintIdIx);

    ECRelationshipConstraintR constraint = (relationshipEnd == ECRelationshipEnd_Target) ? ecRelationship->GetTarget() : ecRelationship->GetSource();

    //uint32_t was persisted as int64 to not lose signed-ness. 
    const uint32_t multiplicityLowerLimit = (uint32_t) stmt->GetValueInt64(lowerLimitIx);
    if (stmt->IsColumnNull(upperLimitIx))
        {
        //If upper limit  DB NULL, this means it is unbounded -> Use ctor that only takes lower limit
        constraint.SetMultiplicity(RelationshipMultiplicity(multiplicityLowerLimit));
        BeAssert(constraint.GetMultiplicity().IsUpperLimitUnbounded());
        }
    else
        constraint.SetMultiplicity(RelationshipMultiplicity(multiplicityLowerLimit, (uint32_t) stmt->GetValueInt64(upperLimitIx)));

    constraint.SetIsPolymorphic(stmt->GetValueBoolean(isPolymorphicIx));

    if (!stmt->IsColumnNull(roleLabelIx))
        constraint.SetRoleLabel(stmt->GetValueText(roleLabelIx));

    ECClassId abstractConstraintClassId;
    if (!stmt->IsColumnNull(abstractConstraintClassIdIx))
        abstractConstraintClassId = stmt->GetValueId<ECClassId>(abstractConstraintClassIdIx);

    //release statement as we have read all information and so that child calls can reuse the same statement without repreparation.
    stmt = nullptr;

    if (abstractConstraintClassId.IsValid())
        {
        ECClassCP abstractConstraintClass = GetClass(ctx, abstractConstraintClassId);
        if (abstractConstraintClass == nullptr)
            {
            BeAssert(false && "Could not load abstract constraint class or it is neither an entity class nor a relationship class");
            return ERROR;
            }

        if (abstractConstraintClass->IsEntityClass())
            {
            if (ECObjectsStatus::Success != constraint.SetAbstractConstraint(*abstractConstraintClass->GetEntityClassCP()))
                return ERROR;
            }
        else if (abstractConstraintClass->IsRelationshipClass())
            {
            if (ECObjectsStatus::Success != constraint.SetAbstractConstraint(*abstractConstraintClass->GetRelationshipClassCP()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Should have been caught before");
            return ERROR;
            }
        }

    if (SUCCESS != LoadRelationshipConstraintClassesFromDb(constraint, ctx, constraintId))
        return ERROR;

    SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType =
        relationshipEnd == ECRelationshipEnd_Target ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint;

    return LoadCAFromDb(constraint, ctx, ECContainerId(constraintId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadRelationshipConstraintClassesFromDb(ECRelationshipConstraintR constraint, Context& ctx, ECRelationshipConstraintId constraintId) const
    {
    CachedStatementPtr statement = GetCachedStatement(Utf8PrintfString("SELECT ClassId FROM [%s]." TABLE_RelationshipConstraintClass " WHERE ConstraintId=?", GetTableSpace().GetName().c_str()).c_str());
    if (statement == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != statement->BindId(1, constraintId))
        return ERROR;

    while (statement->Step() == BE_SQLITE_ROW)
        {
        const ECClassId constraintClassId = statement->GetValueId<ECClassId>(0);
        ECClassCP constraintClass = GetClass(ctx, constraintClassId);
        if (constraintClass == nullptr)
            return ERROR;

        if (!constraintClass->IsEntityClass() && !constraintClass->IsRelationshipClass())
            {
            BeAssert(false && "Relationship constraint classes are expected to be entity classes.");
            return ERROR;
            }

        if (constraintClass->IsEntityClass())
            {
            if (ECObjectsStatus::Success != constraint.AddClass(*constraintClass->GetEntityClassCP()))
                return ERROR;
            }
        else if (constraintClass->IsRelationshipClass())
            {
            if (ECObjectsStatus::Success != constraint.AddClass(*constraintClass->GetRelationshipClassCP()))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Relationship constraint classes are expected to be entity classes or relationships.");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
ECDbCR SchemaReader::GetECDb() const { return m_schemaManager.GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
DbTableSpace const& SchemaReader::GetTableSpace() const { return m_schemaManager.GetTableSpace(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
CachedStatementPtr SchemaReader::GetCachedStatement(Utf8CP sql) const { return GetECDb().GetImpl().GetCachedSqliteStatement(sql); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaReader::ClearCache() const
    {
    m_cache.Clear();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaReader::Context::Postprocess(SchemaReader const& reader) const
    {
    for (ECN::NavigationECProperty* navProp : m_navProps)
        {
        if (!navProp->Verify())
            return ERROR;

        if (!const_cast<ECN::ECRelationshipClassP>(navProp->GetRelationshipClass())->GetIsVerified())
            {
            if (!navProp->GetRelationshipClass()->Verify())
                return ERROR;
            }
        }

    if (m_schemasToLoadCAInstancesFor.empty())
        return SUCCESS;

    //load CAs for gatherer schemas now and in a separate context.
    //delaying CA loading on schemas is necessary for the case where a schema defines a CA class
    //and at the same time carries an instance of it.
    Context ctx;
    for (ECN::ECSchema* schema : m_schemasToLoadCAInstancesFor)
        {
        if (SUCCESS != reader.LoadCAFromDb(*schema, ctx, ECContainerId(schema->GetId()), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
            return ERROR;
        }

    return ctx.Postprocess(reader);
    }
//////////////////////////////SchemaReader::ReaderCache////////////////////////////////////
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
void SchemaReader::ReaderCache::Clear() const
    {
    BeMutexHolder lockECDb(m_ecdb.GetImpl().GetMutex());
    m_classIdCache.clear();
    m_enumCache.clear();
    m_koqCache.clear();
    m_propCategoryCache.clear();
    m_classCache.clear();
    m_schemaCache.clear();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
SchemaDbEntry* SchemaReader::ReaderCache::Find(ECN::ECSchemaId id) const
    {
    auto itor = m_schemaCache.find(id);
    if (itor != m_schemaCache.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(std::unique_ptr<SchemaDbEntry> entry) const
    {
    BeAssert(entry != nullptr);
    const ECN::ECSchemaId id = entry->GetId();
    BeAssert(id.IsValid());
    auto itor = m_schemaCache.insert(std::make_pair(id, std::move(entry)));
    return itor.second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
ClassDbEntry* SchemaReader::ReaderCache::Find(ECN::ECClassId id) const
    {
    auto itor = m_classCache.find(id);
    if (itor != m_classCache.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(std::unique_ptr<ClassDbEntry> entry) const
    {
    BeAssert(entry != nullptr);
    const ECN::ECClassId id = entry->m_classId;
    BeAssert(id.IsValid());
    auto itor = m_classCache.insert(std::make_pair(id, std::move(entry)));
    return itor.second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
EnumDbEntry* SchemaReader::ReaderCache::Find(ECN::ECEnumerationId id) const
    {
    auto itor = m_enumCache.find(id);
    if (itor != m_enumCache.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(std::unique_ptr<EnumDbEntry> entry) const
    {
    BeAssert(entry != nullptr);
    const ECN::ECEnumerationId id = entry->m_enumId;
    BeAssert(id.IsValid());

    auto itor = m_enumCache.insert(std::make_pair(id, std::move(entry)));
    return itor.second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
KindOfQuantityDbEntry* SchemaReader::ReaderCache::Find(ECN::KindOfQuantityId id) const
    {
    auto itor = m_koqCache.find(id);
    if (itor != m_koqCache.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(std::unique_ptr<KindOfQuantityDbEntry> entry) const
    {
    BeAssert(entry != nullptr);
    const ECN::KindOfQuantityId id = entry->m_koqId;
    BeAssert(id.IsValid());

    auto itor = m_koqCache.insert(std::make_pair(id, std::move(entry)));
    return itor.second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyCategoryDbEntry* SchemaReader::ReaderCache::Find(ECN::PropertyCategoryId id) const
    {
    auto itor = m_propCategoryCache.find(id);
    if (itor != m_propCategoryCache.end())
        return itor->second.get();

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(std::unique_ptr<PropertyCategoryDbEntry> entry) const
    {
    BeAssert(entry != nullptr);
    const ECN::PropertyCategoryId id = entry->m_categoryId;
    BeAssert(id.IsValid());

    auto itor = m_propCategoryCache.insert(std::make_pair(id, std::move(entry)));
    return itor.second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
ECN::ECClassId SchemaReader::ReaderCache::Find(Utf8StringCR schemaName, Utf8StringCR className) const
    {
    auto itorA = m_classIdCache.find(schemaName);
    if (itorA == m_classIdCache.end())
        return ECN::ECClassId();

    bmap<Utf8String, ECN::ECClassId, CompareIUtf8Ascii>& classes = itorA->second;
    auto itorB = classes.find(className);
    if (itorB == classes.end())
        return ECN::ECClassId();


    return itorB->second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::Insert(Utf8StringCR schemaName, Utf8StringCR className, ECN::ECClassId id) const
    {
    BeAssert(id.IsValid());
    BeAssert(!schemaName.empty());
    BeAssert(!className.empty());
    m_classIdCache[schemaName][className] = id;
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
bool SchemaReader::ReaderCache::HasClassEntry(ECN::ECClassId id) const
    {
    BeAssert(id.IsValid());
    return m_classCache.find(id) != m_classCache.end();
    }
    
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
void SchemaReader::ReaderCache::SetClassEntryToNull(ECN::ECClassId id) const
    {
    BeAssert(id.IsValid());
    m_classCache[id] = nullptr;
    
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
