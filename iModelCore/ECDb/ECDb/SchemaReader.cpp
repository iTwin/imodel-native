/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaReader.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"
#include <Formatting/FormattingApi.h>
#include <limits>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReader::SchemaReader(TableSpaceSchemaManager const& manager) : m_schemaManager(manager), m_cache(manager.GetECDb(), manager.GetTableSpace())
    {}

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


    // Legacy Units and Formats hack: ECDb deserializes the units and formats schemas from disk
    // if the legacy file contains KOQ 
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        if (SUCCESS != m_cache.GetLegacyUnitsHelper().Initialize())
            return ERROR;

        if (m_cache.GetLegacyUnitsHelper().GetUnitsSchema() != nullptr)
            schemas.push_back(m_cache.GetLegacyUnitsHelper().GetUnitsSchema());

        if (m_cache.GetLegacyUnitsHelper().GetFormatsSchema() != nullptr)
            schemas.push_back(m_cache.GetLegacyUnitsHelper().GetFormatsSchema());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaReader::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        const bool isUnitsSchema = m_cache.GetLegacyUnitsHelper().IsValidUnitsSchemaName(schemaNameOrAlias, mode);
        const bool isFormatsSchema = m_cache.GetLegacyUnitsHelper().IsValidFormatsSchemaName(schemaNameOrAlias, mode);
        if (isUnitsSchema || isFormatsSchema)
            {
            if (isUnitsSchema)
                return m_cache.GetLegacyUnitsHelper().GetUnitsSchema();

            return m_cache.GetLegacyUnitsHelper().GetFormatsSchema();
            }
        }

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
    if (!ecClassId.IsValid())
        return nullptr;

    BeMutexHolder ecdbLock(GetECDbMutex());
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
    ECClassId ecClassId;
    BeMutexHolder ecdbLock(GetECDbMutex());
    ecClassId = m_cache.Find(schemaName, className);
    if (ecClassId.IsValid())
        return ecClassId;


    ecClassId = SchemaPersistenceHelper::GetClassId(GetECDb(), GetTableSpace(), schemaName.c_str(), className.c_str(), lookupMode);
    //add id to cache (only if valid class id to avoid overflow of the cache)
    if (ecClassId.IsValid())
        m_cache.Insert(schemaName, className, ecClassId);

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

    ECEnumerationCP ecEnum = nullptr;
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

    KindOfQuantityCP koq = nullptr;
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

    PropertyCategoryCP cat = nullptr;
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
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
UnitSystemCP SchemaReader::GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR systemName, SchemaLookupMode schemaLookupMode) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        if (!m_cache.GetLegacyUnitsHelper().IsValidUnitsSchemaName(schemaNameOrAlias, schemaLookupMode))
            {
            LOG.errorv("Cannot read UnitSystem '%s.%s'. This version of the file only supports UnitSystems from the standard Units ECSchema.", schemaNameOrAlias.c_str(), systemName.c_str());
            return nullptr;
            }

        ECSchemaCP unitsSchema = m_cache.GetLegacyUnitsHelper().GetUnitsSchema();
        if (unitsSchema == nullptr)
            return nullptr;

        return unitsSchema->GetUnitSystemCP(systemName.c_str());
        }

    UnitSystemId id = SchemaPersistenceHelper::GetUnitSystemId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), systemName.c_str(), schemaLookupMode);
    if (!id.IsValid())
        return nullptr;

    Context ctx;
    if (SUCCESS != LoadUnitsAndFormats(ctx))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return m_cache.Find(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2018
//---------------------------------------------------------------------------------------
UnitSystemId SchemaReader::GetUnitSystemId(UnitSystemCR us) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        LOG.errorv("Cannot read UnitSystemId for UnitSystem '%s'. This version of the file does not support UnitSystems yet.", us.GetFullName().c_str());
        return UnitSystemId();
        }

    if (us.HasId()) //This is unsafe but since we do not delete UnitSystem any class that hasId does exist in db
        {
        BeAssert(us.GetId() == SchemaPersistenceHelper::GetUnitSystemId(GetECDb(), GetTableSpace(), us.GetSchema().GetName().c_str(), us.GetName().c_str(), SchemaLookupMode::ByName));
        return us.GetId();
        }

    const UnitSystemId id = SchemaPersistenceHelper::GetUnitSystemId(GetECDb(), GetTableSpace(), us.GetSchema().GetName().c_str(), us.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the UnitSystem was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<UnitSystemR>(us).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
PhenomenonCP SchemaReader::GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode schemaLookupMode) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        if (!m_cache.GetLegacyUnitsHelper().IsValidUnitsSchemaName(schemaNameOrAlias, schemaLookupMode))
            {
            LOG.errorv("Cannot read Phenomenon '%s.%s'. This version of the file only supports Phenomena from the standard Units ECSchema.", schemaNameOrAlias.c_str(), phenName.c_str());
            return nullptr;
            }

        ECSchemaCP unitsSchema = m_cache.GetLegacyUnitsHelper().GetUnitsSchema();
        if (unitsSchema == nullptr)
            return nullptr;

        return unitsSchema->GetPhenomenonCP(phenName.c_str());
        }

    PhenomenonId id = SchemaPersistenceHelper::GetPhenomenonId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), phenName.c_str(), schemaLookupMode);
    if (!id.IsValid())
        return nullptr;

    Context ctx;
    if (SUCCESS != LoadUnitsAndFormats(ctx))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return m_cache.Find(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2018
//---------------------------------------------------------------------------------------
PhenomenonId SchemaReader::GetPhenomenonId(PhenomenonCR ph) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        LOG.errorv("Cannot read PhenomenonId for Phenomenon '%s'. This version of the file does not support Phenomena yet.", ph.GetFullName().c_str());
        return PhenomenonId();
        }

    if (ph.HasId()) //This is unsafe but since we do not delete Phenomenon any class that hasId does exist in db
        {
        BeAssert(ph.GetId() == SchemaPersistenceHelper::GetPhenomenonId(GetECDb(), GetTableSpace(), ph.GetSchema().GetName().c_str(), ph.GetName().c_str(), SchemaLookupMode::ByName));
        return ph.GetId();
        }

    const PhenomenonId id = SchemaPersistenceHelper::GetPhenomenonId(GetECDb(), GetTableSpace(), ph.GetSchema().GetName().c_str(), ph.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the Phenomenon was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<PhenomenonR>(ph).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECUnitCP SchemaReader::GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode schemaLookupMode) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        if (!m_cache.GetLegacyUnitsHelper().IsValidUnitsSchemaName(schemaNameOrAlias, schemaLookupMode))
            {
            LOG.errorv("Cannot read Unit '%s.%s'. This version of the file only supports standard Units from the Units ECSchema.", schemaNameOrAlias.c_str(), unitName.c_str());
            return nullptr;
            }

        ECSchemaCP unitsSchema = m_cache.GetLegacyUnitsHelper().GetUnitsSchema();
        if (unitsSchema == nullptr)
            return nullptr;

        return unitsSchema->GetUnitCP(unitName.c_str());
        }

    UnitId unitId = SchemaPersistenceHelper::GetUnitId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), unitName.c_str(), schemaLookupMode);
    if (!unitId.IsValid())
        return nullptr;

    Context ctx;
    if (SUCCESS != LoadUnitsAndFormats(ctx))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return m_cache.Find(unitId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2018
//---------------------------------------------------------------------------------------
UnitId SchemaReader::GetUnitId(ECUnitCR unit) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        LOG.errorv("Cannot read UnitId for Unit '%s'. This version of the file does not support Units yet.", unit.GetFullName().c_str());
        return UnitId();
        }

    if (unit.HasId()) //This is unsafe but since we do not delete Unit any class that hasId does exist in db
        {
        BeAssert(unit.GetId() == SchemaPersistenceHelper::GetUnitId(GetECDb(), GetTableSpace(), unit.GetSchema().GetName().c_str(), unit.GetName().c_str(), SchemaLookupMode::ByName));
        return unit.GetId();
        }

    const UnitId id = SchemaPersistenceHelper::GetUnitId(GetECDb(), GetTableSpace(), unit.GetSchema().GetName().c_str(), unit.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the Unit was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECUnitR>(unit).SetId(id);
        }

    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECFormatCP SchemaReader::GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode schemaLookupMode) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        if (!m_cache.GetLegacyUnitsHelper().IsValidFormatsSchemaName(schemaNameOrAlias, schemaLookupMode))
            {
            LOG.errorv("Cannot read Format '%s.%s'. This version of the file only supports standard formats from the Formats ECSchema.", schemaNameOrAlias.c_str(), formatName.c_str());
            return nullptr;
            }

        ECSchemaCP formatsSchema = m_cache.GetLegacyUnitsHelper().GetFormatsSchema();
        if (formatsSchema == nullptr)
            return nullptr;

        return formatsSchema->GetFormatCP(formatName.c_str());
        }

    FormatId formatId = SchemaPersistenceHelper::GetFormatId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), formatName.c_str(), schemaLookupMode);
    if (!formatId.IsValid())
        return nullptr;

    Context ctx;
    if (SUCCESS != LoadUnitsAndFormats(ctx))
        return nullptr;

    if (SUCCESS != ctx.Postprocess(*this))
        return nullptr;

    return m_cache.Find(formatId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2018
//---------------------------------------------------------------------------------------
FormatId SchemaReader::GetFormatId(ECFormatCR format) const
    {
    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        LOG.errorv("Cannot read FormatId for Format '%s'. This version of the file does not support Formats yet.", format.GetFullName().c_str());
        return FormatId();
        }

    if (format.HasId()) //This is unsafe but since we do not delete Format any class that hasId does exist in db
        {
        BeAssert(format.GetId() == SchemaPersistenceHelper::GetFormatId(GetECDb(), GetTableSpace(), format.GetSchema().GetName().c_str(), format.GetName().c_str(), SchemaLookupMode::ByName));
        return format.GetId();
        }

    const FormatId id = SchemaPersistenceHelper::GetFormatId(GetECDb(), GetTableSpace(), format.GetSchema().GetName().c_str(), format.GetName().c_str(), SchemaLookupMode::ByName);
    if (id.IsValid())
        {
        //it is possible that the Format was already imported before, but the given C++ object comes from another source.
        //in that case we assign it here on the fly.
        const_cast<ECFormatR>(format).SetId(id);
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
    BeMutexHolder ecdbLock(GetECDbMutex());
    ClassDbEntry* entry = m_cache.Find(ecClassId);
    if (entry != nullptr && entry->m_derivedClassesAreLoaded)
        return SUCCESS;

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

    ClassDbEntry* entry = m_cache.Find(ecClassId);
    entry->m_derivedClassesAreLoaded = true;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadEnumeration(ECEnumerationCP& ecEnum, Context& ctx, ECEnumerationId enumId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    ecEnum = m_cache.Find(enumId);
    if (ecEnum != nullptr)
        return SUCCESS;

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int typeColIx = 4;
    const int isStrictColIx = 5;
    const int valuesColIx = 6;

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

    ECEnumerationP newEnum = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreateEnumeration(newEnum, enumName, underlyingType))
        return ERROR;

    newEnum->SetId(enumId);

    if (displayLabel != nullptr)
        newEnum->SetDisplayLabel(displayLabel);

    newEnum->SetDescription(description);
    newEnum->SetIsStrict(isStrict);

    if (Utf8String::IsNullOrEmpty(enumValuesJsonStr))
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != SchemaPersistenceHelper::DeserializeEnumerationValues(*newEnum, GetECDb(), enumValuesJsonStr))
        return ERROR;

    //cache the enum
    m_cache.Insert(*newEnum);
    schemaKey->m_loadedTypeCount++;
    ecEnum = newEnum;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadUnitSystems(Context& ctx) const
    {
    //no mutex holder as it is expected that this method is always called from another ReadXX method that does hold a mutex already

    BeAssert(FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) && "Should have been checked before");

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id,SchemaId,Name,DisplayLabel,Description FROM [%s]." TABLE_UnitSystem, GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const UnitSystemId id = stmt->GetValueId<UnitSystemId>(0);

        const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(1);
        SchemaDbEntry* schemaKey = nullptr;
        if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
            return ERROR;

        Utf8CP usName = stmt->GetValueText(2);
        Utf8CP displayLabel = stmt->IsColumnNull(3) ? nullptr : stmt->GetValueText(3);
        Utf8CP description = stmt->IsColumnNull(4) ? nullptr : stmt->GetValueText(4);

        UnitSystemP newSystem = nullptr;
        if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreateUnitSystem(newSystem, usName, displayLabel, description))
            return ERROR;

        newSystem->SetId(id);
        m_cache.Insert(*newSystem);
        schemaKey->m_loadedTypeCount++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadPhenomena(Context& ctx) const
    {
    //no mutex holder as it is expected that this method is always called from another ReadXX method that does hold a mutex already

    BeAssert(FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) && "Should have been checked before");

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id,SchemaId,Name,DisplayLabel,Description,Definition FROM [%s]." TABLE_Phenomenon, GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const PhenomenonId id = stmt->GetValueId<PhenomenonId>(0);

        const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(1);
        SchemaDbEntry* schemaKey = nullptr;
        if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
            return ERROR;

        Utf8CP name = stmt->GetValueText(2);
        Utf8CP displayLabel = stmt->IsColumnNull(3) ? nullptr : stmt->GetValueText(3);
        Utf8CP description = stmt->IsColumnNull(4) ? nullptr : stmt->GetValueText(4);
        Utf8CP definition = stmt->IsColumnNull(5) ? nullptr : stmt->GetValueText(5);

        PhenomenonP newPhen = nullptr;
        if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreatePhenomenon(newPhen, name, definition, displayLabel, description))
            return ERROR;

        newPhen->SetId(id);
        m_cache.Insert(*newPhen);
        schemaKey->m_loadedTypeCount++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadUnits(Context& ctx) const
    {
    //no mutex holder as it is expected that this method is always called from another ReadXX method that does hold a mutex already

    BeAssert(FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) && "Should have been checked before");

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id,SchemaId,Name,DisplayLabel,Description,PhenomenonId,UnitSystemId,Definition,Numerator,Denominator,Offset,IsConstant,InvertingUnitId FROM [%s]." TABLE_Unit, GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    struct UnitColumnIndexes final
        {
        int m_unitId = 0;
        int m_schemaId = 1;
        int m_name = 2;
        int m_displayLabel = 3;
        int m_description = 4;
        int m_phenId = 5;
        int m_unitSystemId = 6;
        int m_definition = 7;
        int m_numerator = 8;
        int m_denominator = 9;
        int m_offset = 10;
        int m_isConstant = 11;
        int m_invertingUnitId = 12;
        };

    UnitColumnIndexes colIndexes;

    struct InvertedUnitInfo
        {
        ECN::UnitId m_id;
        SchemaDbEntry* m_schema = nullptr;
        Utf8String m_name;
        ECN::UnitId m_invertingUnitId;
        Nullable<Utf8String> m_displayLabel;
        Nullable<Utf8String> m_description;
        ECN::UnitSystemCP m_unitSystem = nullptr;

        InvertedUnitInfo(ECN::UnitId id, SchemaDbEntry& schema, Utf8CP name, ECN::UnitId invertingUnitId, Utf8CP displayLabel, Utf8CP description, UnitSystemCR system) :
            m_id(id), m_schema(&schema), m_name(name), m_invertingUnitId(invertingUnitId), m_unitSystem(&system)
            {
            if (displayLabel != nullptr)
                m_displayLabel = Utf8String(displayLabel);

            if (description != nullptr)
                m_description = Utf8String(description);
            }

        Utf8CP GetDisplayLabel() const { return m_displayLabel.IsValid() ? m_displayLabel.Value().c_str() : nullptr; }
        Utf8CP GetDescription() const { return m_description.IsValid() ? m_description.Value().c_str() : nullptr; }
        };

    std::vector<InvertedUnitInfo> invertedUnitInfos;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        UnitId id = stmt->GetValueId<UnitId>(colIndexes.m_unitId);
        const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(colIndexes.m_schemaId);
        SchemaDbEntry* schema = nullptr;
        if (SUCCESS != ReadSchema(schema, ctx, schemaId, false))
            return ERROR;

        Utf8CP name = stmt->GetValueText(colIndexes.m_name);
        Utf8CP displayLabel = stmt->IsColumnNull(colIndexes.m_displayLabel) ? nullptr : stmt->GetValueText(colIndexes.m_displayLabel);
        Utf8CP description = stmt->IsColumnNull(colIndexes.m_description) ? nullptr : stmt->GetValueText(colIndexes.m_description);

        UnitSystemCP us = nullptr;
        if (!stmt->IsColumnNull(colIndexes.m_unitSystemId))
            {
            UnitSystemId usId = stmt->GetValueId<UnitSystemId>(colIndexes.m_unitSystemId);
            us = m_cache.Find(usId);
            if (us == nullptr)
                {
                BeAssert(false && "UnitSystems should have been loaded before Units");
                return ERROR;
                }
            }

        if (!stmt->IsColumnNull(colIndexes.m_invertingUnitId))
            {
            //cache inverted units as they need their inverting unit to be exist before. They are loaded once the regular units have been loaded.
            UnitId invertingUnitId = stmt->GetValueId<UnitId>(colIndexes.m_invertingUnitId);
            BeAssert(stmt->IsColumnNull(colIndexes.m_definition) && stmt->IsColumnNull(colIndexes.m_numerator) && stmt->IsColumnNull(colIndexes.m_denominator) && stmt->IsColumnNull(colIndexes.m_offset));
            invertedUnitInfos.push_back(InvertedUnitInfo(id, *schema, name, invertingUnitId, displayLabel, description, *us));
            continue;
            }

        const bool isConstant = stmt->GetValueBoolean(colIndexes.m_isConstant);
        const PhenomenonId phId = stmt->GetValueId<PhenomenonId>(colIndexes.m_phenId);
        PhenomenonCP ph = m_cache.Find(phId);
        if (ph == nullptr)
            {
            BeAssert(false && "Phenomena should have been loaded before Units");
            return ERROR;
            }

        BeAssert((isConstant && us == nullptr) || (!isConstant && us != nullptr));
        BeAssert(!stmt->IsColumnNull(colIndexes.m_definition));
        Utf8CP definition = stmt->IsColumnNull(colIndexes.m_definition) ? nullptr : stmt->GetValueText(colIndexes.m_definition);

        Nullable<double> numerator;
        if (!stmt->IsColumnNull(colIndexes.m_numerator))
            numerator = stmt->GetValueDouble(colIndexes.m_numerator);

        Nullable<double> denominator;
        if (!stmt->IsColumnNull(colIndexes.m_denominator))
            denominator = stmt->GetValueDouble(colIndexes.m_denominator);

        Nullable<double> offset;
        if (!stmt->IsColumnNull(colIndexes.m_offset))
            offset = stmt->GetValueDouble(colIndexes.m_offset);

        ECUnitP newUnit = nullptr;
        if (isConstant)
            {
            BeAssert(numerator != nullptr && "Constant unit expects numerator not to be null");
            if (ECObjectsStatus::Success != schema->m_cachedSchema->CreateConstant(newUnit, name, definition, *ph, numerator.Value(), denominator, displayLabel, description))
                return ERROR;
            }
        else
            {
            if (ECObjectsStatus::Success != schema->m_cachedSchema->CreateUnit(newUnit, name, definition, *ph, *us, numerator, denominator, offset, displayLabel, description))
                return ERROR;
            }

        newUnit->SetId(id);
        m_cache.Insert(*newUnit);
        schema->m_loadedTypeCount++;
        }

    //now load inverted units as their inverting units have been loaded now
    for (InvertedUnitInfo const& invertedUnitInfo : invertedUnitInfos)
        {
        SchemaDbEntry& schema = *invertedUnitInfo.m_schema;
        ECUnitCP invertingUnit = m_cache.Find(invertedUnitInfo.m_invertingUnitId);
        if (invertingUnit == nullptr)
            {
            BeAssert("Inverting unit for an inverted unit should have been loaded already");
            return ERROR;
            }

        ECUnitP newUnit = nullptr;
        if (ECObjectsStatus::Success != schema.m_cachedSchema->CreateInvertedUnit(newUnit, *invertingUnit, invertedUnitInfo.m_name.c_str(), *invertedUnitInfo.m_unitSystem,
                                                                                  invertedUnitInfo.GetDisplayLabel(), invertedUnitInfo.GetDescription()))
            {
            return ERROR;
            }

        newUnit->SetId(invertedUnitInfo.m_id);
        m_cache.Insert(*newUnit);
        schema.m_loadedTypeCount++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadFormats(Context& ctx) const
    {
    //no mutex holder as it is expected that this method is always called from another ReadXX method that does hold a mutex already

    BeAssert(FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) && "Should have been checked before");

    constexpr int idIx = 0;
    constexpr int schemaIdIx = 1;
    constexpr int nameIx = 2;
    constexpr int displayLabelColIx = 3;
    constexpr int descriptionColIx = 4;
    constexpr int numSpecColIx = 5;
    constexpr int compositeSpecColIx = 6;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id,SchemaId,Name,DisplayLabel,Description,NumericSpec,CompositeSpec FROM [%s]." TABLE_Format, GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const FormatId id = stmt->GetValueId<FormatId>(idIx);
        const ECSchemaId schemaId = stmt->GetValueId<ECSchemaId>(schemaIdIx);
        SchemaDbEntry* schemaKey = nullptr;
        if (SUCCESS != ReadSchema(schemaKey, ctx, schemaId, false))
            return ERROR;

        Utf8CP name = stmt->GetValueText(nameIx);
        Utf8CP displayLabel = stmt->IsColumnNull(displayLabelColIx) ? nullptr : stmt->GetValueText(displayLabelColIx);
        Utf8CP description = stmt->IsColumnNull(descriptionColIx) ? nullptr : stmt->GetValueText(descriptionColIx);

        Utf8CP numericSpecJsonStr = stmt->IsColumnNull(numSpecColIx) ? nullptr : stmt->GetValueText(numSpecColIx);
        const bool hasNumericSpec = !Utf8String::IsNullOrEmpty(numericSpecJsonStr);
        Formatting::NumericFormatSpec numSpec;
        if (hasNumericSpec)
            {
            Json::Value numericSpecJson;
            if (!Json::Reader::Parse(numericSpecJsonStr, numericSpecJson))
                return ERROR;

            if (!Formatting::NumericFormatSpec::FromJson(numSpec, numericSpecJson))
                return ERROR;
            }

        Utf8CP compSpecJsonStr = stmt->IsColumnNull(compositeSpecColIx) ? nullptr : stmt->GetValueText(compositeSpecColIx);

        ECSchemaR schema = *schemaKey->m_cachedSchema;
        ECFormatP newFormat = nullptr;
        if (ECObjectsStatus::Success != schema.CreateFormat(newFormat, name, displayLabel, description, hasNumericSpec ? &numSpec : nullptr))
            return ERROR;

        if (SUCCESS != ReadFormatComposite(ctx, *newFormat, id, compSpecJsonStr))
            return ERROR;

        if (newFormat->IsProblem())
            return ERROR;

        newFormat->SetId(id);
        m_cache.Insert(*newFormat);
        schemaKey->m_loadedTypeCount++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadFormatComposite(Context& ctx, ECFormat& format, FormatId formatId, Utf8CP compositeSpecJsonWithoutUnits) const
    {
    //no mutex holder as it is expected that this method is always called from another ReadXX method that does hold a mutex already

    BeAssert(FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) && "Should have been checked before");

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Label,UnitId FROM [%s]." TABLE_FormatCompositeUnit " WHERE FormatId=? ORDER BY Ordinal", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, formatId))
        return ERROR;

    bvector<Utf8String> labels;
    bvector<ECN::UnitId> unitIds;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP label = stmt->GetValueText(0);
        labels.push_back(label);

        unitIds.push_back(stmt->GetValueId<UnitId>(1));
        }
    stmt = nullptr;

    if (unitIds.empty()) // format doesn't have a composite spec
        {
        BeAssert(Utf8String::IsNullOrEmpty(compositeSpecJsonWithoutUnits));
        return SUCCESS;
        }

    const size_t labelCount = labels.size();
    BeAssert(labelCount <= 4 && unitIds.size() <= 4);

    bvector<Units::UnitCP> units;
    for (UnitId unitId : unitIds)
        {
        ECUnitCP unit = m_cache.Find(unitId);
        if (unit == nullptr)
            {
            BeAssert(false && "Units should have been loaded before creating the Format CompositeSpec");
            return ERROR;
            }
        units.push_back(unit);
        }
    Formatting::CompositeValueSpec spec;
	bool compStatus = Formatting::CompositeValueSpec::CreateCompositeSpec(spec, units);
	if (!compStatus)
		return ERROR;

    if (!Utf8String::IsNullOrEmpty(compositeSpecJsonWithoutUnits))
        {
        Json::Value compSpecJson;
        if (!Json::Reader::Parse(compositeSpecJsonWithoutUnits, compSpecJson))
            return ERROR;

        if (!Formatting::CompositeValueSpec::FromJson(spec, compSpecJson, units, labels))
            return ERROR;
        }

    if (spec.IsProblem())
        return ERROR;

    return format.SetCompositeSpec(spec) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadKindOfQuantity(KindOfQuantityCP& koq, Context& ctx, KindOfQuantityId koqId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    koq = m_cache.Find(koqId);
    if (koq != nullptr)
        return SUCCESS;

    if (SUCCESS != LoadUnitsAndFormats(ctx))
        return ERROR;

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int relErrorColIx = 4;
    const int persUnitColIx = 5;
    const int presFormatsColIx = 6;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT SchemaId,Name,DisplayLabel,Description,RelativeError,PersistenceUnit,PresentationUnits FROM [%s]." TABLE_KindOfQuantity " WHERE Id=?", GetTableSpace().GetName().c_str()).c_str());
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
    const double relError = stmt->GetValueDouble(relErrorColIx);
    Utf8CP persUnitStr = stmt->GetValueText(persUnitColIx);
    Utf8CP presFormatsStr = stmt->IsColumnNull(presFormatsColIx) ? nullptr : stmt->GetValueText(presFormatsColIx);

    KindOfQuantityP newKoq = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreateKindOfQuantity(newKoq, koqName))
        return ERROR;

    newKoq->SetId(koqId);

    if (displayLabel != nullptr)
        newKoq->SetDisplayLabel(displayLabel);

    newKoq->SetDescription(description);
    newKoq->SetRelativeError(relError);

    // Persistence Unit and Presentation Formats
    bvector<Utf8CP> presFormats; //can use Utf8CP as the string is owned by the rapidjson::Document
    rapidjson::Document presFormatsJson;
    if (!Utf8String::IsNullOrEmpty(presFormatsStr))
        {
        if (presFormatsJson.Parse<0>(presFormatsStr).HasParseError())
            {
            BeAssert(false && "Could not parse KindOfQuantity Presentation Formats values JSON string.");
            return ERROR;
            }

        BeAssert(presFormatsJson.IsArray());
        for (rapidjson::Value const& presFormatJson : presFormatsJson.GetArray())
            {
            BeAssert(presFormatJson.IsString() && presFormatJson.GetStringLength() > 0);
            presFormats.push_back(presFormatJson.GetString());
            }
        }

    if (FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        // The persistence unit is persisted as fully qualified name to preserve backwards compatibility
        BeAssert(!Utf8String::IsNullOrEmpty(persUnitStr));
        auto lookupUnit = [this] (Utf8StringCR unitAlias, Utf8StringCR unitName)
            {
            BeAssert(!unitAlias.empty());
            return m_schemaManager.GetUnit(unitAlias, unitName, SchemaLookupMode::ByAlias);
            };

        if (ECObjectsStatus::Success != newKoq->AddPersistenceUnitByName(persUnitStr, lookupUnit))
            {
            LOG.errorv("Failed to read KindOfQuantity '%s'. Its persistence unit '%s' could not be parsed.", newKoq->GetFullName().c_str(), persUnitStr);
            return ERROR;
            }

        //PresentationFormats
        if (!presFormats.empty())
            {
            auto lookupFormat = [this] (Utf8String alias, Utf8StringCR formatName)
                {
                return m_schemaManager.GetFormat(alias, formatName, SchemaLookupMode::ByAlias);
                };

            for (Utf8CP presFormat : presFormats)
                {
                if (ECObjectsStatus::Success != newKoq->AddPresentationFormatByString(presFormat, lookupFormat, lookupUnit))
                    {
                    LOG.errorv("Failed to read KindOfQuantity '%s'. Its presentation format '%s' could not be parsed.", newKoq->GetFullName().c_str(), presFormat);
                    return ERROR;
                    }
                }
            }
        }
    else
        {
        //Handle legacy KOQs
        if (SUCCESS != m_cache.GetLegacyUnitsHelper().AssignPersistenceUnitAndPresentationFormats(*newKoq, persUnitStr, presFormats))
            return ERROR;
        }

    //cache the koq
    m_cache.Insert(*newKoq);
    schemaKey->m_loadedTypeCount++;
    koq = newKoq;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::ReadPropertyCategory(PropertyCategoryCP& cat, Context& ctx, PropertyCategoryId catId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    cat = m_cache.Find(catId);
    if (cat != nullptr)
        return SUCCESS;

    const int schemaIdIx = 0;
    const int nameIx = 1;
    const int displayLabelColIx = 2;
    const int descriptionColIx = 3;
    const int priorityColIx = 4;

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

    PropertyCategoryP newCat = nullptr;
    if (ECObjectsStatus::Success != schemaKey->m_cachedSchema->CreatePropertyCategory(newCat, catName))
        return ERROR;

    newCat->SetId(catId);

    if (displayLabel != nullptr)
        newCat->SetDisplayLabel(displayLabel);

    newCat->SetDescription(description);

    if (!prio.IsNull())
        newCat->SetPriority(prio.Value());

    //cache the category
    m_cache.Insert(*newCat);
    schemaKey->m_loadedTypeCount++;
    cat = newCat;
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::ReadSchema(SchemaDbEntry*& schemaEntry, Context& ctx, ECSchemaId schemaId, bool loadSchemaEntities) const
    {
    if (SUCCESS != ReadSchemaStubAndReferences(schemaEntry, ctx, schemaId))
        return ERROR;

    BeAssert(schemaEntry != nullptr);
    if (loadSchemaEntities && !schemaEntry->IsFullyLoaded())
        {
        std::set<SchemaDbEntry*> fullyLoadedSchemas;
        if (SUCCESS != ReadSchemaElements(*schemaEntry, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::ReadSchemaStubAndReferences(SchemaDbEntry*& schemaEntry, Context& ctx, ECSchemaId schemaId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    if (schemaEntry = m_cache.Find(schemaId))
        {
        BeAssert(schemaEntry->m_cachedSchema != nullptr);
        return SUCCESS;
        }

    //Following method is not by itself thread safe as it write to cache but 
    //this is the only call to it which is thread safe.
    if (SUCCESS != ReadSchemaStub(schemaEntry, ctx, schemaId))
        return ERROR;

    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT ReferencedSchemaId FROM [%s]." TABLE_SchemaReference " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaId))
        return ERROR;

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
        if (SUCCESS != ReadSchemaStubAndReferences(referenceSchemaKey, ctx, referencedSchemaId))
            return ERROR;

        ECObjectsStatus s = schemaEntry->m_cachedSchema->AddReferencedSchema(*referenceSchemaKey->m_cachedSchema);
        if (s != ECObjectsStatus::Success)
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::ReadSchemaStub(SchemaDbEntry*& schemaEntry, Context& ctx, ECSchemaId ecSchemaId) const
    {
    Utf8CP tableSpace = GetTableSpace().GetName().c_str();
    CachedStatementPtr stmt = nullptr;
    const bool hasECVersionsAndUnits = FeatureManager::IsAvailable(GetECDb(), {Feature::ECVersions, Feature::UnitsAndFormats});
    if (hasECVersionsAndUnits)
        {
        stmt = GetCachedStatement(Utf8PrintfString("SELECT s.Name,s.DisplayLabel,s.Description,s.Alias,s.VersionDigit1,s.VersionDigit2,s.VersionDigit3,s.OriginalECXmlVersionMajor,s.OriginalECXmlVersionMinor,"
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Class "  c WHERE s.Id = c.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Enumeration " e WHERE s.Id = e.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_KindOfQuantity " koq WHERE s.Id = koq.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_PropertyCategory " cat WHERE s.Id = cat.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_UnitSystem " us WHERE s.Id = us.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Phenomenon " ph WHERE s.Id = ph.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Unit " u WHERE s.Id = u.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Format " f WHERE s.Id = f.SchemaId) "
                                                   "FROM [%s]." TABLE_Schema " s WHERE s.Id=?", tableSpace, tableSpace, tableSpace, tableSpace, tableSpace, tableSpace, tableSpace, tableSpace, tableSpace).c_str());
        }
    else
        {
        stmt = GetCachedStatement(Utf8PrintfString("SELECT s.Name, s.DisplayLabel,s.Description,s.Alias,s.VersionDigit1,s.VersionDigit2,s.VersionDigit3,"
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Class "  c WHERE s.Id = c.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_Enumeration " e WHERE s.Id = e.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_KindOfQuantity " koq WHERE s.Id = koq.SchemaId) + "
                                                   "(SELECT COUNT(*) FROM [%s]." TABLE_PropertyCategory " cat WHERE s.Id = cat.SchemaId) "
                                                   "FROM [%s]." TABLE_Schema " s WHERE s.Id=?", tableSpace, tableSpace, tableSpace, tableSpace, tableSpace).c_str());
        }

    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchemaId))
        return ERROR;

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    int colIx = 0;
    Utf8CP schemaName = stmt->GetValueText(colIx);

    colIx++;
    Utf8CP displayLabel = stmt->IsColumnNull(colIx) ? nullptr : stmt->GetValueText(colIx);

    colIx++;
    Utf8CP description = stmt->IsColumnNull(colIx) ? nullptr : stmt->GetValueText(colIx);

    colIx++;
    Utf8CP alias = stmt->GetValueText(colIx);

    //uint32_t is persisted as int64 to not lose unsigned-ness
    colIx++;
    uint32_t versionMajor = (uint32_t) stmt->GetValueInt64(colIx);

    colIx++;
    uint32_t versionWrite = (uint32_t) stmt->GetValueInt64(colIx);

    colIx++;
    uint32_t versionMinor = (uint32_t) stmt->GetValueInt64(colIx);

    Nullable<uint32_t> originalECVersionMajor, originalECVersionMinor;
    if (hasECVersionsAndUnits)
        {
        colIx++;
        if (!stmt->IsColumnNull(colIx))
            originalECVersionMajor = (uint32_t) stmt->GetValueInt64(colIx);

        colIx++;
        if (!stmt->IsColumnNull(colIx))
            originalECVersionMinor = (uint32_t) stmt->GetValueInt64(colIx);
        }

    colIx++;
    const int typesInSchema = stmt->GetValueInt(colIx);

    ECSchemaPtr schema = nullptr;
    if (ECSchema::CreateSchema(schema, schemaName, alias, versionMajor, versionWrite, versionMinor) != ECObjectsStatus::Success)
        return ERROR;

    if (hasECVersionsAndUnits && originalECVersionMajor != nullptr)
        schema->SetOriginalECXmlVersion(originalECVersionMajor.Value(), originalECVersionMinor.IsNull() ? 0 : originalECVersionMinor.Value());
    else
        schema->SetOriginalECXmlVersion(0, 0); // ECObjects set by default ECVersion as the original version. ECDb must not do that, as profile upgrade logic is based on the original version in case it was not set

    schema->SetId(ecSchemaId);

    if (!Utf8String::IsNullOrEmpty(displayLabel))
        schema->SetDisplayLabel(displayLabel);

    if (!Utf8String::IsNullOrEmpty(description))
        schema->SetDescription(description);

    schemaEntry = new SchemaDbEntry(schema, typesInSchema);
    m_cache.Insert(std::unique_ptr<SchemaDbEntry>(schemaEntry));
    ctx.AddSchemaToLoadCAInstanceFor(*schemaEntry->m_cachedSchema);
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaReader::ReadSchemaElements(SchemaDbEntry& schemaEntry, Context& ctx, std::set<SchemaDbEntry*>& fullyLoadedSchemas) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    if (fullyLoadedSchemas.find(&schemaEntry) != fullyLoadedSchemas.end())
        return SUCCESS;

    //Accessing cache object not safe. Parent function make sure its a thread safe call
    //Ensure all reference schemas also loaded
    for (auto& refSchemaKey : schemaEntry.m_cachedSchema->GetReferencedSchemas())
        {
        if (!refSchemaKey.second->HasId())
            {
            // schemas without id are only valid in a particular case: If this is a 4.0.0.1 file (pre EC3.2) and if the schema
            // is the units or formats schema - which in this case is deserialized by ECDb from disk (without assigning an id)
            if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats) &&
                (m_cache.GetLegacyUnitsHelper().IsValidUnitsSchemaName(refSchemaKey.first.GetName(), SchemaLookupMode::ByName) ||
                 m_cache.GetLegacyUnitsHelper().IsValidFormatsSchemaName(refSchemaKey.first.GetName(), SchemaLookupMode::ByName)))
                {
                continue;
                }

            BeAssert(false && "Referenced schema is expected to be in cache already at this point");
            return ERROR;
            }

        ECSchemaId referenceSchemaId = refSchemaKey.second->GetId();
        SchemaDbEntry* key = m_cache.Find(referenceSchemaId);
        if (key == nullptr)
            {
            BeAssert(false && "Referenced schema is expected to be in cache already at this point");
            return ERROR;
            }

        if (SUCCESS != ReadSchemaElements(*key, ctx, fullyLoadedSchemas))
            return ERROR;
        }

    //Ensure load all the classes in the schema
    fullyLoadedSchemas.insert(&schemaEntry);
    if (schemaEntry.IsFullyLoaded())
        return SUCCESS;

    // Load classes
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Class " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaEntry.GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const ECClassId classId = stmt->GetValueId<ECClassId>(0);
        if (nullptr == GetClass(ctx, classId))
            {
            LOG.errorv("Could not load ECClass with id %" PRIu64 " from schema %s", classId.GetValue(), schemaEntry.m_cachedSchema->GetName().c_str());
            return ERROR;
            }

        if (schemaEntry.IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;

    // Load enumerations
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Enumeration " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaEntry.GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECEnumerationCP ecEnum = nullptr;
        if (SUCCESS != ReadEnumeration(ecEnum, ctx, stmt->GetValueId<ECEnumerationId>(0)))
            return ERROR;

        if (schemaEntry.IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;

    // Load KOQs
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_KindOfQuantity " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaEntry.GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        KindOfQuantityCP koq = nullptr;
        if (SUCCESS != ReadKindOfQuantity(koq, ctx, stmt->GetValueId<KindOfQuantityId>(0)))
            return ERROR;

        if (schemaEntry.IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;

    // Load PropertyCategories
    stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_PropertyCategory " WHERE SchemaId=?", GetTableSpace().GetName().c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaEntry.GetId()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        PropertyCategoryCP cat = nullptr;
        if (SUCCESS != ReadPropertyCategory(cat, ctx, stmt->GetValueId<PropertyCategoryId>(0)))
            return ERROR;

        if (schemaEntry.IsFullyLoaded())
            return SUCCESS;
        }

    stmt = nullptr;

    if (FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        {
        // Load Units (are loaded at once for unit conversion API)
        if (!m_cache.m_areUnitsAndFormatsLoaded)
            {
            stmt = GetCachedStatement(Utf8PrintfString("SELECT Id FROM [%s]." TABLE_Unit " WHERE SchemaId=:schemaid UNION ALL "
                                                       "SELECT Id FROM [%s]." TABLE_UnitSystem " WHERE SchemaId=:schemaid UNION ALL "
                                                       "SELECT Id FROM [%s]." TABLE_Phenomenon " WHERE SchemaId=:schemaid UNION ALL "
                                                       "SELECT Id FROM [%s]." TABLE_Format " WHERE SchemaId=:schemaid", GetTableSpace().GetName().c_str(), GetTableSpace().GetName().c_str(), GetTableSpace().GetName().c_str(), GetTableSpace().GetName().c_str()).c_str());
            if (stmt == nullptr)
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(stmt->GetParameterIndex(":schemaid"), schemaEntry.GetId()))
                return ERROR;

            const bool schemaHasUnits = BE_SQLITE_ROW == stmt->Step();
            stmt = nullptr;

            if (schemaHasUnits)
                {
                if (SUCCESS != LoadUnitsAndFormats(ctx))
                    return ERROR;

                if (schemaEntry.IsFullyLoaded())
                    return SUCCESS;
                }
            }
        }

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
                    ECEnumerationCP ecenum = nullptr;
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
                    ECEnumerationCP ecenum = nullptr;
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
            KindOfQuantityCP koq = nullptr;
            if (SUCCESS != ReadKindOfQuantity(koq, ctx, rowInfo.m_koqId))
                return ERROR;

            prop->SetKindOfQuantity(koq);
            }

        if (rowInfo.m_catId.IsValid())
            {
            PropertyCategoryCP cat = nullptr;
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
// @bsimethod                                                    Krischan.Eberle    05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReader::LoadUnitsAndFormats(Context& ctx) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());

    if (!FeatureManager::IsAvailable(GetECDb(), Feature::UnitsAndFormats))
        return m_cache.GetLegacyUnitsHelper().Initialize();

    if (m_cache.m_areUnitsAndFormatsLoaded)
        return SUCCESS;

    m_cache.m_areUnitsAndFormatsLoaded = true;

    if (SUCCESS != ReadUnitSystems(ctx))
        return ERROR;

    if (SUCCESS != ReadPhenomena(ctx))
        return ERROR;

    if (SUCCESS != ReadUnits(ctx))
        return ERROR;

    return ReadFormats(ctx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
ECDbCR SchemaReader::GetECDb() const { return m_schemaManager.GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
ECDb& SchemaReader::GetECDbR() const { return const_cast<ECDb&>(m_schemaManager.GetECDb()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
BeMutex& SchemaReader::GetECDbMutex() const { return m_schemaManager.GetECDb().GetImpl().GetMutex(); }

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
void SchemaReader::ClearCache() const { m_cache.Clear(); }


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
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    m_classIdCache.clear();
    m_enumCache.clear();
    m_koqCache.clear();
    m_propCategoryCache.clear();
    m_classCache.clear();
    m_schemaCache.clear();
    m_unitSystemCache.clear();
    m_phenomenonCache.clear();
    m_unitCache.clear();
    m_formatCache.clear();
    m_areUnitsAndFormatsLoaded = false;
    m_legacyUnitsHelper.ClearCache();
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

//***********************************************************************************
// SchemaReader::LegacyUnitsHelper
//***********************************************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2018
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SchemaReader::LegacyUnitsHelper::AssignPersistenceUnitAndPresentationFormats(ECN::KindOfQuantityR koq, Utf8CP legacyPersUnit, bvector<Utf8CP> const& legacyPresUnits) const
    {
    BeAssert(m_isInitialized && m_unitsSchema != nullptr && m_formatsSchema != nullptr);

    if (SUCCESS != AddReferences(koq.GetSchema()))
        {
        LOG.errorv("Failed to read KindOfQuantity '%s'. It is a legacy KindOfQuantity which requires to upgrade the persistence and presentation units in memory. "
                   "Could not add Units and Formats schemas references to the KindOfQuantity's schema.", koq.GetFullName().c_str());
        return ERROR;
        }

    if (ECObjectsStatus::Success != koq.FromFUSDescriptors(legacyPersUnit, legacyPresUnits, *m_formatsSchema, *m_unitsSchema))
        {
        Utf8String legacyPresUnitsString;
        bool isFirstPresUnit = true;
        for (Utf8CP presUnit : legacyPresUnits)
            {
            if (!isFirstPresUnit)
                legacyPresUnitsString.append("; ");

            legacyPresUnitsString.append(presUnit);
            isFirstPresUnit = false;
            }

        LOG.errorv("Failed to read KindOfQuantity '%s'. It is a legacy KindOfQuantity which requires to upgrade the persistence and presentation units in memory. "
                   "Could not convert the legacy persistence '%s' and presentation units '%s'.", koq.GetFullName().c_str(), legacyPersUnit, legacyPresUnitsString.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2018
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SchemaReader::LegacyUnitsHelper::Initialize() const
    {
    if (m_isInitialized)
        return SUCCESS;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, Utf8PrintfString("SELECT 1 FROM %s." TABLE_KindOfQuantity " LIMIT 1", m_tableSpace.GetName().c_str()).c_str()))
        {
        LOG.error("Legacy Units support (pre EC3.1 units): Failed to determine whether the file contains KindOfQuantities.");
        return ERROR;
        }

    m_isInitialized = true;
    const DbResult stat = stmt.Step();
    // no KOQs in the file -> units and formats schemas are not deserialized
    if (BE_SQLITE_DONE == stat)
        return SUCCESS;

    if (BE_SQLITE_ROW != stat)
        {
        LOG.error("Legacy Units support (pre EC3.1 units): Failed to determine whether the file contains KindOfQuantities.");
        return ERROR;
        }

    ECSchemaReadContextPtr schemaReadCtx = ECSchemaReadContext::CreateContext();
    schemaReadCtx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey formatsSchemaKey("Formats", 1, 0, 0);
    m_formatsSchema = ECSchema::LocateSchema(formatsSchemaKey, *schemaReadCtx);
    SchemaKey unitsSchemaKey("Units", 1, 0, 0);
    m_unitsSchema = ECSchema::LocateSchema(unitsSchemaKey, *schemaReadCtx);
    if (m_formatsSchema != nullptr && m_unitsSchema != nullptr)
        return SUCCESS;

    LOG.error("Legacy Units support (pre EC3.1 units): Failed to deserialize the Units or Formats ECSchemas from disk.");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2018
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus SchemaReader::LegacyUnitsHelper::AddReferences(ECN::ECSchemaCR koqSchema) const
    {
    BeAssert(m_unitsSchema != nullptr && m_formatsSchema != nullptr);
    ECSchemaR koqSchemaR = const_cast<ECSchemaR>(koqSchema);
    ECObjectsStatus stat = koqSchemaR.AddReferencedSchema(*m_unitsSchema);
    //if it is referenced already, don't treat as error
    if (ECObjectsStatus::Success != stat && ECObjectsStatus::NamedItemAlreadyExists != stat)
        return ERROR;

    stat = koqSchemaR.AddReferencedSchema(*m_formatsSchema);
    //if it is referenced already, don't treat as error
    if (ECObjectsStatus::Success != stat && ECObjectsStatus::NamedItemAlreadyExists != stat)
        return ERROR;

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
