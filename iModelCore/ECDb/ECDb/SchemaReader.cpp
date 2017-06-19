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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaReader::GetSchema(ECSchemaId ecSchemaId, bool loadSchemaEntities) const
    {
    SchemaReader::Context ctx;
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
        BeAssert(ecSchema.GetId() == SchemaPersistenceHelper::GetSchemaId(m_ecdb, ecSchema.GetName().c_str()));
        return ecSchema.GetId();
        }

    const ECSchemaId schemaId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, ecSchema.GetName().c_str());
    if (schemaId.IsValid())
        {
        //it is possible that the schema was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECSchemaR>(ecSchema).SetId(schemaId);
        }

    return schemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaReader::GetEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    ECEnumerationId enumId = SchemaPersistenceHelper::GetEnumerationId(m_ecdb, schemaName, enumName);
    if (!enumId.IsValid())
        return nullptr;

    SchemaReader::Context ctx;

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
        BeAssert(ecEnum.GetId() == SchemaPersistenceHelper::GetEnumerationId(m_ecdb, ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str()));
        return ecEnum.GetId();
        }

    const ECEnumerationId id = SchemaPersistenceHelper::GetEnumerationId(m_ecdb, ecEnum.GetSchema().GetName().c_str(), ecEnum.GetName().c_str());
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
KindOfQuantityCP SchemaReader::GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const
    {
    KindOfQuantityId koqId = SchemaPersistenceHelper::GetKindOfQuantityId(m_ecdb, schemaName, koqName);
    if (!koqId.IsValid())
        return nullptr;

    SchemaReader::Context ctx;

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
        BeAssert(koq.GetId() == SchemaPersistenceHelper::GetKindOfQuantityId(m_ecdb, koq.GetSchema().GetName().c_str(), koq.GetName().c_str()));
        return koq.GetId();
        }

    const KindOfQuantityId id = SchemaPersistenceHelper::GetKindOfQuantityId(m_ecdb, koq.GetSchema().GetName().c_str(), koq.GetName().c_str());
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
PropertyCategoryCP SchemaReader::GetPropertyCategory(Utf8CP schemaName, Utf8CP catName) const
    {
    PropertyCategoryId catId = SchemaPersistenceHelper::GetPropertyCategoryId(m_ecdb, schemaName, catName);
    if (!catId.IsValid())
        return nullptr;

    SchemaReader::Context ctx;

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
        BeAssert(cat.GetId() == SchemaPersistenceHelper::GetPropertyCategoryId(m_ecdb, cat.GetSchema().GetName().c_str(), cat.GetName().c_str()));
        return cat.GetId();
        }

    const PropertyCategoryId id = SchemaPersistenceHelper::GetPropertyCategoryId(m_ecdb, cat.GetSchema().GetName().c_str(), cat.GetName().c_str());
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
        BeAssert(prop.GetId() == SchemaPersistenceHelper::GetPropertyId(m_ecdb, prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str()));
        return prop.GetId();
        }

    ECPropertyId propId;
    if (prop.GetClass().HasId())
        {
        //If the ECClass already has an id, we can run a faster SQL to get the property id
        BeAssert(prop.GetClass().GetId() == SchemaPersistenceHelper::GetClassId(m_ecdb, prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), SchemaLookupMode::ByName));

        propId = SchemaPersistenceHelper::GetPropertyId(m_ecdb, prop.GetClass().GetId(), prop.GetName().c_str());
        }
    else
        propId = SchemaPersistenceHelper::GetPropertyId(m_ecdb, prop.GetClass().GetSchema().GetName().c_str(), prop.GetClass().GetName().c_str(), prop.GetName().c_str());

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
    auto itor = m_classCache.find(ecClassId);
    if (itor != m_classCache.end())
        {
        if (itor->second->m_ensureDerivedClassesExist)
            return SUCCESS;

        //Just mark is loaded as code that ensure has very rare chance of ever failing
        itor->second->m_ensureDerivedClassesExist = true;
        }

    SchemaReader::Context ctx;
    if (SUCCESS != EnsureDerivedClassesExist(ctx, ecClassId))
        return ERROR;

    return ctx.Postprocess(*this);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus SchemaReader::EnsureDerivedClassesExist(Context& ctx, ECClassId ecClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT ClassId FROM ec_ClassHasBaseClasses WHERE BaseClassId=?"))
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaReader::GetClass(ECClassId ecClassId) const
    {
    SchemaReader::Context ctx;
    ECClassCP ecclass = GetClass(ctx, ecClassId);
    if (ecclass == nullptr)
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return ecclass;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP SchemaReader::GetClass(Context& ctx, ECClassId ecClassId) const
    {
    if (!ecClassId.IsValid())
        return nullptr;

    ECDbExpressionSymbolContext symbolsContext(m_ecdb);

    ECClassP classFromCache = nullptr;
    if (TryGetClassFromCache(classFromCache, ecClassId))
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

    BeSQLite::CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType FROM ec_Class WHERE Id=?");
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

    SchemaDbEntry* schemaKey = nullptr;
    if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
        return nullptr;

    ECSchemaR schema = *schemaKey->m_cachedSchema;
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
            if (schema.CreateRelationshipClass(newClass, className, false) != ECObjectsStatus::Success)
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
    m_classCache[ecClassId] = std::make_unique<ClassDbEntry>(*ecClass);

    if (SUCCESS != LoadCAFromDb(*ecClass, ctx, ECContainerId(ecClassId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class))
        return nullptr;

    if (SUCCESS != LoadMixinAppliesToClass(ctx, *ecClass))
        return nullptr;

    if (SUCCESS != LoadBaseClassesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    if (SUCCESS != LoadPropertiesFromDb(ecClass, ctx, ecClassId))
        return nullptr;

    ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
    if (relClass != nullptr)
        {
        if (SUCCESS != LoadRelationshipConstraintFromDb(relClass, ctx, ecClassId, ECRelationshipEnd_Source))
            return nullptr;

        if (SUCCESS != LoadRelationshipConstraintFromDb(relClass, ctx, ecClassId, ECRelationshipEnd_Target))
            return nullptr;

        if (!relClass->Verify())
            return nullptr;
        }

    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        05/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaReader::TryGetClassFromCache(ECClassP& ecClass, ECClassId ecClassId) const
    {
    if (!ecClassId.IsValid())
        return false;

    auto classKeyIterator = m_classCache.find(ecClassId);
    if (classKeyIterator != m_classCache.end())
        {
        ecClass = classKeyIterator->second->m_cachedClass;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadEnumeration(ECEnumerationP& ecEnum, Context& ctx, ECEnumerationId enumId) const
    {
    auto enumEntryIt = m_enumCache.find(enumId);
    if (enumEntryIt != m_enumCache.end())
        {
        ecEnum = enumEntryIt->second->m_cachedEnum;
        return SUCCESS;
        }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int isStrictColIx = 5;
    const int valuesColIx = 6;

    BeSQLite::CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration WHERE Id=?");
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
    m_enumCache[enumId] = std::unique_ptr<EnumDbEntry>(new EnumDbEntry(enumId, *ecEnum));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadKindOfQuantity(KindOfQuantityP& koq, Context& ctx, KindOfQuantityId koqId) const
    {
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
    const int relErrorColIx = 5;
    const int presUnitColIx = 6;

    BeSQLite::CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits FROM ec_KindOfQuantity WHERE Id=?");
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
    koq->SetPersistenceUnit(Formatting::FormatUnitSet(persUnitStr));
    BeAssert(!koq->GetPersistenceUnit().HasProblem() && "KOQ Persistence Unit could not be deserialized correctly. It has an invalid format");
    koq->SetRelativeError(relError);

    if (!Utf8String::IsNullOrEmpty(presUnitsStr))
        {
        if (SUCCESS != SchemaPersistenceHelper::DeserializeKoqPresentationUnits(*koq, presUnitsStr))
            return ERROR;
        }

    //cache the koq
    m_koqCache[koqId] = std::unique_ptr<KindOfQuantityDbEntry>(new KindOfQuantityDbEntry(koqId, *koq));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadPropertyCategory(PropertyCategoryP& cat, Context& ctx, PropertyCategoryId catId) const
    {
    auto catEntryIt = m_propCategoryCache.find(catId);
    if (catEntryIt != m_propCategoryCache.end())
        {
        cat = catEntryIt->second->m_cachedCategory;
        return SUCCESS;
        }

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int priorityColIx = 4;

    BeSQLite::CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT SchemaId,Name,DisplayLabel,Description,Priority FROM ec_PropertyCategory WHERE Id=?");
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
    m_propCategoryCache[catId] = std::unique_ptr<PropertyCategoryDbEntry>(new PropertyCategoryDbEntry(catId, *cat));

    schemaKey->m_loadedTypeCount++;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::LoadSchemaDefinition(SchemaDbEntry*& schemaEntry, bvector<SchemaDbEntry*>& newlyLoadedSchemas, ECSchemaId ecSchemaId) const
    {
    auto schemaIterator = m_schemaCache.find(ecSchemaId);
    if (schemaIterator != m_schemaCache.end())
        {
        BeAssert(schemaIterator->second->m_cachedSchema != nullptr);
        schemaEntry = schemaIterator->second.get();
        return SUCCESS;
        }

    if (SUCCESS != LoadSchemaFromDb(schemaEntry, ecSchemaId))
        return ERROR;

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT ReferencedSchemaId FROM ec_SchemaReference WHERE SchemaId=?"))
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
BentleyStatus SchemaReader::ReadSchema(SchemaDbEntry*& outECSchemaKey, Context& ctx, ECSchemaId ctxECSchemaId, bool loadSchemaEntities) const
    {
    bvector<SchemaDbEntry*> newlyLoadedSchemas;
    if (SUCCESS != LoadSchemaDefinition(outECSchemaKey, newlyLoadedSchemas, ctxECSchemaId))
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
        SchemaDbEntry* key = nullptr;
        ECSchemaId referenceECSchemaId = refSchemaKey.second->GetId();
        auto schemaIterator = m_schemaCache.find(referenceECSchemaId);
        if (schemaIterator != m_schemaCache.end())
            key = schemaIterator->second.get();

        if (SUCCESS != LoadSchemaEntitiesFromDb(key, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(ecSchemaKey);
    if (ecSchemaKey->IsFullyLoaded())
        return SUCCESS;

    BeSQLite::CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_Class WHERE SchemaId=?");
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
    stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_Enumeration WHERE SchemaId=?");
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
    stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_KindOfQuantity WHERE SchemaId=?");
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
    stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_PropertyCategory WHERE SchemaId=?");
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT S.Name, S.DisplayLabel,S.Description,S.Alias,S.VersionDigit1,S.VersionDigit2,S.VersionDigit3, "
                                                        "(SELECT COUNT(*) FROM ec_Class C WHERE S.Id = C.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM ec_Enumeration e WHERE S.Id = e.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM ec_KindOfQuantity koq WHERE S.Id = koq.SchemaId) + "
                                                        "(SELECT COUNT(*) FROM ec_PropertyCategory cat WHERE S.Id = cat.SchemaId) "
                                                        "FROM ec_Schema S WHERE S.Id=?");
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

    std::unique_ptr<SchemaDbEntry> schemaEntryPtr = std::unique_ptr<SchemaDbEntry>(new SchemaDbEntry(schema, typesInSchema));
    schemaEntry = schemaEntryPtr.get();
    m_schemaCache[ecSchemaId] = std::move(schemaEntryPtr);
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
BentleyStatus SchemaReader::LoadPropertiesFromDb(ECClassP& ecClass, Context& ctx, ECClassId ecClassId) const
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
            Nullable<int> m_primType;
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

        static BentleyStatus ReadRows(std::vector<RowInfo>& rows, ECDbCR ecdb, ECClassId classId)
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
            const int catIdIx = 11;
            const int minOccursIx = 12;
            const int maxOccursIx = 13;
            const int navRelationshipClassId = 14;
            const int navPropDirectionIx = 15;

            CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Id,Kind,Name,DisplayLabel,Description,IsReadonly,"
                                                              "PrimitiveType,EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,"
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
                const PropertyKind kind = Enum::FromInt<PropertyKind>(stmt->GetValueInt(kindIx));
                rowInfo.m_kind = kind;
                rowInfo.m_name.assign(stmt->GetValueText(nameIx));

                if (!stmt->IsColumnNull(displayLabelIx))
                    rowInfo.m_displayLabel.assign(stmt->GetValueText(displayLabelIx));

                if (!stmt->IsColumnNull(descrIx))
                    rowInfo.m_description.assign(stmt->GetValueText(descrIx));

                if (!stmt->IsColumnNull(isReadonlyIx))
                    rowInfo.m_isReadonly = stmt->GetValueBoolean(isReadonlyIx);

                if (!stmt->IsColumnNull(primTypeIx))
                    rowInfo.m_primType = stmt->GetValueInt(primTypeIx);

                if (!stmt->IsColumnNull(enumIdIx))
                    rowInfo.m_enumId = stmt->GetValueId<ECEnumerationId>(enumIdIx);

                if (kind == PropertyKind::Primitive && rowInfo.m_primType.IsNull() && !rowInfo.m_enumId.IsValid())
                    {
                    BeAssert(false && "Either PrimitiveType or EnumerationId column must not be NULL for primitive property");
                    return ERROR;
                    }

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
    if (SUCCESS != PropReaderHelper::ReadRows(rowInfos, m_ecdb, ecClassId))
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

                    if (ECObjectsStatus::Success != ecClass->CreateEnumerationProperty(primProp, rowInfo.m_name, *ecenum))
                        return ERROR;
                    }
                else
                    {
                    if (ECObjectsStatus::Success != ecClass->CreatePrimitiveProperty(primProp, rowInfo.m_name, (PrimitiveType) rowInfo.m_primType.Value()))
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
                if (ECObjectsStatus::Success != ecClass->CreateStructProperty(structProp, rowInfo.m_name, *structClass))
                    return ERROR;

                prop = structProp;
                break;
                }

                case PropertyKind::PrimitiveArray:
                {
                BeAssert(!rowInfo.m_primType.IsNull());

                PrimitiveType primType = (PrimitiveType) rowInfo.m_primType.Value();

                PrimitiveArrayECPropertyP arrayProp = nullptr;
                if (ECObjectsStatus::Success != ecClass->CreatePrimitiveArrayProperty(arrayProp, rowInfo.m_name, primType))
                    return ERROR;

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
                if (ECObjectsStatus::Success != ecClass->CreateStructArrayProperty(arrayProp, rowInfo.m_name, *structClass))
                    return ERROR;

                if (SUCCESS != PropReaderHelper::AssignArrayBounds(*arrayProp, rowInfo))
                    return ERROR;

                prop = arrayProp;
                break;
                }

                case PropertyKind::Navigation:
                {
                BeAssert(ecClass->IsEntityClass());

                BeAssert(rowInfo.m_navPropRelClassId.IsValid() && !rowInfo.m_navPropDirection.IsNull());
                ECClassCP relClassRaw = GetClass(ctx, rowInfo.m_navPropRelClassId);
                if (relClassRaw == nullptr)
                    return ERROR;

                BeAssert(relClassRaw->IsRelationshipClass());
                ECRelatedInstanceDirection direction = (ECRelatedInstanceDirection) rowInfo.m_navPropDirection.Value();
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

            prop->SetPropertyCategory(cat);
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
BentleyStatus SchemaReader::LoadBaseClassesFromDb(ECClassP& ecClass, Context& ctx, ECClassId ecClassId) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT BaseClassId FROM ec_ClassHasBaseClasses WHERE ClassId=? ORDER BY Ordinal"))
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
        ECClassCP baseClass = GetClass(ctx, baseClassId);
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
BentleyStatus SchemaReader::LoadCAFromDb(ECN::IECCustomAttributeContainerR caConstainer, Context& ctx, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT ClassId,Instance FROM ec_CustomAttribute WHERE ContainerId=? AND ContainerType=? ORDER BY Ordinal"))
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
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT Id,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId FROM ec_RelationshipConstraint WHERE RelationshipClassId=? AND RelationshipEnd=?"))
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
        if (abstractConstraintClass == nullptr || !abstractConstraintClass->IsEntityClass())
            {
            BeAssert(false && "Could not load abstract constraint class or it is not an entity class");
            return ERROR;
            }

        if (ECObjectsStatus::Success != constraint.SetAbstractConstraint(*abstractConstraintClass->GetEntityClassCP()))
            return ERROR;
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
    CachedStatementPtr statement = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(statement, "SELECT ClassId FROM ec_RelationshipConstraintClass WHERE ConstraintId=?"))
        return ERROR;

    if (BE_SQLITE_OK != statement->BindId(1, constraintId))
        return ERROR;

    while (statement->Step() == BE_SQLITE_ROW)
        {
        const ECClassId constraintClassId = statement->GetValueId<ECClassId>(0);
        ECClassCP constraintClass = GetClass(ctx, constraintClassId);
        if (constraintClass == nullptr)
            return ERROR;

        ECEntityClassCP constraintAsEntity = constraintClass->GetEntityClassCP();
        if (nullptr == constraintAsEntity)
            {
            BeAssert(false && "Relationship constraint classes are expected to be entity classes.");
            return ERROR;
            }

        if (ECObjectsStatus::Success != constraint.AddClass(*constraintAsEntity))
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
        BeAssert(ecClass.GetId() == GetClassId(ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), SchemaLookupMode::ByName));
        return ecClass.GetId();
        }

    ECClassId classId;
    if (ecClass.GetSchema().HasId())
        {
        //If the ECSchema already has an id, we can run a faster SQL to get the class id
        BeAssert(ecClass.GetSchema().GetId() == SchemaPersistenceHelper::GetSchemaId(m_ecdb, ecClass.GetSchema().GetName().c_str()));
        classId = SchemaPersistenceHelper::GetClassId(m_ecdb, ecClass.GetSchema().GetId(), ecClass.GetName().c_str());
        }
    else
        classId = GetClassId(ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), SchemaLookupMode::ByName);

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
    auto outerIt = m_classIdCache.find(schemaName);
    if (outerIt != m_classIdCache.end())
        {
        bmap<Utf8String, ECClassId, CompareIUtf8Ascii> const& classIdMap = outerIt->second;
        auto innerIt = classIdMap.find(className);
        if (innerIt != classIdMap.end())
            return innerIt->second;
        }

    ECClassId ecClassId = SchemaPersistenceHelper::GetClassId(m_ecdb, schemaName.c_str(), className.c_str(), lookupMode);
    
    //add id to cache (only if valid class id to avoid overflow of the cache)
    if (ecClassId.IsValid())
        m_classIdCache[schemaName][className] = ecClassId;

    return ecClassId;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaReader::ClearCache() const
    {
    m_classIdCache.clear();
    m_enumCache.clear();
    m_koqCache.clear();
    m_propCategoryCache.clear();
    m_classCache.clear();
    m_schemaCache.clear();
    m_systemSchemaHelper.ClearCache();
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

END_BENTLEY_SQLITE_EC_NAMESPACE
