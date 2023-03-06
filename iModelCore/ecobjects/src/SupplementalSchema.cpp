/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

Utf8CP SupplementalSchemaMetaData::s_customAttributeAccessor = "SupplementalSchema";
Utf8CP SupplementalSchemaMetaData::s_customAttributeSchemaName = "CoreCustomAttributes";

static Utf8CP s_bscaCustomAttributeAccessor = "SupplementalSchemaMetaData";
static Utf8CP s_bscaCustomAttributeSchemaName = "Bentley_Standard_CustomAttributes";
static Utf8CP s_bsca_primarySchemaNameAccessor = "PrimarySchemaName";
static Utf8CP s_bsca_primarySchemaMajorVersionAccessor = "PrimarySchemaMajorVersion";
static Utf8CP s_bsca_primarySchemaMinorVersionAccessor = "PrimarySchemaMinorVersion";
static Utf8CP s_bsca_isUserSpecificAccessor = "IsUserSpecific";

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaData::SupplementalSchemaMetaData(IECInstanceCR supplementalSchemaMetaDataCustomAttribute)
    {
    if (!supplementalSchemaMetaDataCustomAttribute.GetClass().GetName().EqualsIAscii(GetCustomAttributeAccessor()))
        {
        InitializeFromOldCustomAttribute(supplementalSchemaMetaDataCustomAttribute);
        return;
        }

    ECValue propertyValue;
    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaNamePropertyAccessor()) && !propertyValue.IsNull())
        m_schemaName = propertyValue.GetUtf8CP();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaReadVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_readVersion = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaWriteVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_writeVersion = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMinorVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_minorVersion = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrecedencePropertyAccessor()) && !propertyValue.IsNull())
        m_precedence = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPurposePropertyAccessor()) && !propertyValue.IsNull())
        m_purpose = propertyValue.GetUtf8CP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void SupplementalSchemaMetaData::InitializeFromOldCustomAttribute(IECInstanceCR supplementalSchemaMetaDataCustomAttribute)
    {
    assert(0 == supplementalSchemaMetaDataCustomAttribute.GetClass().GetName().compare(s_bscaCustomAttributeAccessor));

    ECValue propertyValue;
    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, s_bsca_primarySchemaNameAccessor) && !propertyValue.IsNull())
        m_schemaName = propertyValue.GetUtf8CP();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, s_bsca_primarySchemaMajorVersionAccessor) && !propertyValue.IsNull())
        m_readVersion = propertyValue.GetInteger();

    m_writeVersion = 0;

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, s_bsca_primarySchemaMinorVersionAccessor) && !propertyValue.IsNull())
        m_minorVersion = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrecedencePropertyAccessor()) && !propertyValue.IsNull())
        m_precedence = propertyValue.GetInteger();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPurposePropertyAccessor()) && !propertyValue.IsNull())
        m_purpose = propertyValue.GetUtf8CP();

    if (ECObjectsStatus::Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, s_bsca_isUserSpecificAccessor) && !propertyValue.IsNull())
        m_isUserSpecific = propertyValue.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetCustomAttributeAccessor()
    {
    return s_customAttributeAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetCustomAttributeSchemaName()
    {
    return s_customAttributeSchemaName;
    }

static Utf8CP s_primarySchemaNameAccessor = "PrimarySchemaReference.SchemaName";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaNamePropertyAccessor()
    {
    return s_primarySchemaNameAccessor;
    }

static Utf8CP s_primarySchemaVersionMajorAccessor = "PrimarySchemaReference.MajorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaReadVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMajorAccessor;
    }

static Utf8CP s_primarySchemaVersionWriteAccessor = "PrimarySchemaReference.WriteVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaWriteVersionPropertyAccessor()
    {
    return s_primarySchemaVersionWriteAccessor;
    }

static Utf8CP s_primarySchemaVersionMinorAccessor = "PrimarySchemaReference.MinorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaMinorVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMinorAccessor;
    }

static Utf8CP s_precedenceAccessor = "Precedence";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPrecedencePropertyAccessor()
    {
    return s_precedenceAccessor;
    }

static Utf8CP s_purposeAccessor = "Purpose";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaMetaData::GetPurposePropertyAccessor()
    {
    return s_purposeAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
bool SupplementalSchemaMetaData::TryGetFromSchema(SupplementalSchemaMetaDataPtr& supplementalSchemaMetadata, ECSchemaCR supplementalSchema)
    {
    IECInstancePtr supplementalSchemaMetaDataCustomAttribute = supplementalSchema.GetCustomAttribute(GetCustomAttributeSchemaName(), GetCustomAttributeAccessor());
    if (!supplementalSchemaMetaDataCustomAttribute.IsValid())
        supplementalSchemaMetaDataCustomAttribute = supplementalSchema.GetCustomAttribute(s_bscaCustomAttributeSchemaName, s_bscaCustomAttributeAccessor);
        
    if (!supplementalSchemaMetaDataCustomAttribute.IsValid())
        return false;

    supplementalSchemaMetadata = Create(*supplementalSchemaMetaDataCustomAttribute);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
void SupplementalSchemaMetaData::SetMetadata(ECSchemaR supplementalSchema, SupplementalSchemaMetaDataR supplementalSchemaData, ECSchemaReadContextR schemaContext)
    {
    IECInstancePtr instance = supplementalSchemaData.CreateCustomAttribute(schemaContext);
    supplementalSchema.SetCustomAttribute(*instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::IsSupplemental(ECSchemaP supplementalSchema) const
    {
    if (nullptr == supplementalSchema)
        return false;

    return supplementalSchema->IsDefined(GetCustomAttributeSchemaName(), GetCustomAttributeAccessor()) || supplementalSchema->IsDefined(s_bscaCustomAttributeSchemaName, s_bscaCustomAttributeAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr SupplementalSchemaMetaData::CreateCustomAttribute(ECSchemaReadContextR schemaContext)
    {
    IECInstancePtr instance = CoreCustomAttributeHelper::CreateCustomAttributeInstance(schemaContext, SupplementalSchemaMetaData::GetCustomAttributeAccessor());
    instance->SetValue(GetPrimarySchemaNamePropertyAccessor(), ECValue(GetPrimarySchemaName().c_str()));
    instance->SetValue(GetPrimarySchemaReadVersionPropertyAccessor(), ECValue((::int32_t)GetPrimarySchemaReadVersion()));
    instance->SetValue(GetPrimarySchemaWriteVersionPropertyAccessor(), ECValue((::int32_t)GetPrimarySchemaWriteVersion()));
    instance->SetValue(GetPrimarySchemaMinorVersionPropertyAccessor(), ECValue((::int32_t)GetPrimarySchemaMinorVersion()));
    instance->SetValue(GetPrecedencePropertyAccessor(), ECValue((::int32_t)GetSupplementalSchemaPrecedence()));
    instance->SetValue(GetPurposePropertyAccessor(), ECValue(GetSupplementalSchemaPurpose().c_str()));

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::IsForPrimarySchema(Utf8StringCR querySchemaName, uint32_t querySchemaReadVersion, uint32_t querySchemaWriteVersion, uint32_t querySchemaMinorVersion, SchemaMatchType matchType) const
    {
    SchemaKey primaryKey(m_schemaName.c_str(), m_readVersion, m_writeVersion, m_minorVersion);
    SchemaKey queryKey(querySchemaName.c_str(), querySchemaReadVersion, querySchemaWriteVersion, querySchemaMinorVersion);
    return primaryKey.Matches(queryKey, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::UpdateSchema(ECSchemaR primarySchema, bvector<ECSchemaP>& supplementalSchemaList, ECSchemaReadContextR schemaContext, bool createCopyOfSupplementalCustomAttribute)
    {
    return UpdateSchema(primarySchema, supplementalSchemaList, schemaContext, "", createCopyOfSupplementalCustomAttribute);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::UpdateSchema(ECSchemaR primarySchema, bvector<ECSchemaP>& supplementalSchemaList, ECSchemaReadContextR schemaContext, Utf8CP locale, bool createCopyOfSupplementalCustomAttribute)
    {
    m_createCopyOfSupplementalCustomAttribute = createCopyOfSupplementalCustomAttribute;
    StopWatch timer(true);
    bmap<uint32_t, ECSchemaP> schemasByPrecedence;
    SupplementedSchemaStatus status = OrderSupplementalSchemas(schemasByPrecedence, primarySchema, supplementalSchemaList);
    if (SupplementedSchemaStatus::Success != status)
        return status;

    // If it is already supplemented, need to unsupplement it first
    if (primarySchema.IsSupplemented())
        {
        //UnsupplementSchemaContainer ();
        //if (NULL != m_classesToUpdate)
        //    UnsupplementClasses (m_classesToUpdate);
        //else
        //    UnsupplementClasses (m_supplementedSchema.GetClasses ());
        }
    if (SupplementedSchemaStatus::Success != status)
        return status;

    if (1 > schemasByPrecedence.size())
        return status;

    SupplementalSchemaInfoPtr info = SupplementalSchemaInfo::Create(primarySchema.GetFullSchemaName().c_str(), m_supplementalSchemaNamesAndPurposes);
    primarySchema.SetSupplementalSchemaInfo(info.get(), schemaContext);
    status = MergeSchemasIntoSupplementedSchema(primarySchema, schemasByPrecedence);
    if (SupplementedSchemaStatus::Success != status)
        primarySchema.SetSupplementalSchemaInfo(nullptr, schemaContext); // NEEDS_WORK: Should probably attempt to rollback partial changes

    primarySchema.SetIsSupplemented(true);
    
    timer.Stop();
    Utf8String primarySchemaName = primarySchema.GetFullSchemaName();
    LOG.infov ("Supplemented (in %.4f seconds) %s with %d supplemental ECSchemas", timer.GetElapsedSeconds(), 
        primarySchemaName.c_str(), supplementalSchemaList.size());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::OrderSupplementalSchemas(bmap<uint32_t, ECSchemaP>& schemasByPrecedence, ECSchemaR primarySchema, const bvector<ECSchemaP>& supplementalSchemaList)
    {
    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;
    for (ECSchemaP supplemental : supplementalSchemaList)
        {
        SupplementalSchemaMetaDataPtr metaData;
        if (!SupplementalSchemaMetaData::TryGetFromSchema(metaData, *supplemental))
            return SupplementedSchemaStatus::Metadata_Missing;
        if (!metaData.IsValid())
            return SupplementedSchemaStatus::Metadata_Missing;
        if (!metaData->IsForPrimarySchema(primarySchema.GetName(), primarySchema.GetVersionRead(), primarySchema.GetVersionWrite(), primarySchema.GetVersionMinor(), SchemaMatchType::LatestWriteCompatible))
            continue;

        m_supplementalSchemaNamesAndPurposes[supplemental->GetFullSchemaName()] = metaData->GetSupplementalSchemaPurpose();
        uint32_t precedence = metaData->GetSupplementalSchemaPrecedence();

        bmap<uint32_t, ECSchemaP>::const_iterator precedenceIterator = schemasByPrecedence.find(precedence);

        // If multiple schemas have the same precedence, they need to be merged into a single schema.  
        if (precedenceIterator != schemasByPrecedence.end())
            {
            ECSchemaP schema1 = precedenceIterator->second;
            status = CreateMergedSchemaFromSchemasWithEqualPrecedence(schema1, supplemental);
            if (SupplementedSchemaStatus::Success != status)
                return status;
            SchemaKey key(schema1->GetName().c_str(), schema1->GetVersionRead(), schema1->GetVersionWrite(), schema1->GetVersionMinor());
            schemasByPrecedence[precedence] = m_schemaCache->GetSchema(key);
            }
        else
            {
            schemasByPrecedence[precedence] = supplemental;
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::CreateMergedSchemaFromSchemasWithEqualPrecedence(ECSchemaP schema1, ECSchemaP schema2)
    {
    ECSchemaPtr mergedSchema;
    schema1->CopySchema(mergedSchema);  //Do we need to copy it?

    Utf8String supplementalSchemaFullName = schema2->GetFullSchemaName();
    Utf8String mergedSchemaFullName = schema1->GetFullSchemaName();
    LOG.infov ("Merging %s into %s", supplementalSchemaFullName.c_str(), mergedSchemaFullName.c_str());
    MergeCustomAttributeClasses(*mergedSchema, schema2->GetCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);

    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;
    bvector<ECClassP> orderedClasses;
    GetOrderedClasses(orderedClasses, schema2);
    for(ECClassP supplementalClass: orderedClasses)
        {
        status = MergeClassesWithEqualPrecedence(mergedSchema.get(), supplementalClass, supplementalSchemaFullName, mergedSchemaFullName);
        if (SupplementedSchemaStatus::Success != status)
            return status;
        }
    m_schemaCache->AddSchema(*mergedSchema);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void SupplementedSchemaBuilder::GetOrderedClasses(bvector<ECClassP>& orderedClasses, ECSchemaP schema)
    {
    for (ECClassP ecClass : schema->GetClasses())
        {
        for (IECInstancePtr container : ecClass->GetCustomAttributes(false))
            {
            ECClassP caClass = const_cast<ECClassP>(&(container->GetClass()));
            if (caClass->GetSchemaR().IsSupplementalSchema())
                {
                if (std::find(orderedClasses.begin(), orderedClasses.end(), caClass) == orderedClasses.end())
                    {
                    orderedClasses.push_back(caClass);
                    }
                }
            }
        PropertyList props;
        ecClass->GetProperties(false, &props);
        for (ECPropertyP prop : props)
            {
            for (IECInstancePtr container : prop->GetCustomAttributes(false))
                {
                ECClassP caClass = const_cast<ECClassP>(&(container->GetClass()));
                if (caClass->GetSchemaR().IsSupplementalSchema())
                    {
                    if (std::find(orderedClasses.begin(), orderedClasses.end(), caClass) == orderedClasses.end())
                        {
                        orderedClasses.push_back(caClass);
                        }
                    }
                }
            }
        if (std::find(orderedClasses.begin(), orderedClasses.end(), ecClass) == orderedClasses.end())
            {
            orderedClasses.push_back(ecClass);
            }

        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeClassesWithEqualPrecedence(ECSchemaP mergedSchema, ECClassP supplementalClass, Utf8StringCR supplementalSchemaFullName, Utf8StringCR mergedSchemaFullName)
    {
    ECClassP mergedClass = mergedSchema->GetClassP(supplementalClass->GetName().c_str());
    // The class doesn't already exist, we need to create a new one
    if (nullptr == mergedClass)
        {
        if (ECObjectsStatus::Success != mergedSchema->CopyClass(mergedClass, *supplementalClass, true))
            return SupplementedSchemaStatus::SchemaMergeException;
        return SupplementedSchemaStatus::Success;
        }

    // The class does exist, we need to do a merge
    ECRelationshipClassP supplementalRelationship = supplementalClass->GetRelationshipClassP();
    if (nullptr != supplementalRelationship)
        MergeRelationshipClassConstraints(mergedClass, supplementalRelationship, SCHEMA_PRECEDENCE_Equal);

    for(ECPropertyP supplementalProperty: supplementalClass->GetProperties(false))
        {
        ECPropertyP mergedProperty = mergedClass->GetPropertyP(supplementalProperty->GetName(), false);
        // Class exists but this property does not
        if (nullptr == mergedProperty)
            {
            mergedClass->CopyPropertyForSupplementation(mergedProperty, supplementalProperty, true);
            }
        // Class and property exist, merge property custom attributes
        else
            {
            SupplementedSchemaStatus status = MergeCustomAttributeClasses(*mergedProperty, supplementalProperty->GetCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);
            if (SupplementedSchemaStatus::Success != status)
                return status;
            }
        }

    // Merge class custom attributes
    return MergeCustomAttributeClasses(*mergedClass, supplementalClass->GetCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeSchemasIntoSupplementedSchema(ECSchemaR primarySchema, bmap<uint32_t, ECSchemaP> schemasByPrecedence)
    {
    bvector<ECSchemaP> lowPrecedenceSchemas;
    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;

    for (bmap<uint32_t, ECSchemaP>::iterator schemaWithPrecedence = schemasByPrecedence.begin(); 
          schemaWithPrecedence != schemasByPrecedence.end(); schemaWithPrecedence++)
        {
        int precedence = schemaWithPrecedence->first;
        ECSchemaP schema = schemaWithPrecedence->second;
        if (precedence <= PRECEDENCE_THRESHOLD)
            lowPrecedenceSchemas.insert(lowPrecedenceSchemas.begin(), schema);
        else
            {
            status = MergeIntoSupplementedSchema(primarySchema, schema, SCHEMA_PRECEDENCE_Greater);
            if (SupplementedSchemaStatus::Success != status)
                return status;
            }
        }

    for (bvector<ECSchemaP>::iterator lowPrecedenceSchema = lowPrecedenceSchemas.begin();
        lowPrecedenceSchema != lowPrecedenceSchemas.end(); lowPrecedenceSchema++)
        {
        ECSchemaP schema = *lowPrecedenceSchema;
        status = MergeIntoSupplementedSchema(primarySchema, schema, SCHEMA_PRECEDENCE_Lower);
        if (SupplementedSchemaStatus::Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeIntoSupplementedSchema(ECSchemaR primarySchema, ECSchemaP supplementalSchema, SchemaPrecedence precedence)
    {
    ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalSchema->GetCustomAttributes(false);
    Utf8String supplementalSchemaFullName = supplementalSchema->GetFullSchemaName();

    SupplementedSchemaStatus status = MergeCustomAttributeClasses(primarySchema.GetCustomAttributeContainer(), supplementalCustomAttributes, precedence, &supplementalSchemaFullName, NULL);
    if (SupplementedSchemaStatus::Success != status)
        {
        LOG.errorv ("Failed to merge the custom attributes from the supplemental schema '%s' into the supplemented schema '%s'", supplementalSchemaFullName.c_str(), primarySchema.GetFullSchemaName().c_str());
        return status;
        }

    bvector<ECClassP> orderedClasses;
    GetOrderedClasses(orderedClasses, supplementalSchema);
    for (ECClassP ecClass: orderedClasses)
        {
        status = SupplementClass(primarySchema, supplementalSchema, ecClass, precedence, &supplementalSchemaFullName);
        if (SupplementedSchemaStatus::Success != status)
            {
            LOG.errorv("Failed to merge the custom attributes from the supplemental class '%s' into the supplemented class '%s:%s'",
                                           ecClass->GetFullName(),  primarySchema.GetFullSchemaName().c_str(), ecClass->GetName().c_str());
            return status;
            }
        }

    return SupplementedSchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeCustomAttributeClasses(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, ECCustomAttributeInstanceIterable supplementalCustomAttributes, SchemaPrecedence precedence, 
                                                                                        Utf8StringCP supplementalSchemaFullName, Utf8StringCP consolidatedSchemaFullName)
    {
    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;
    for (IECInstancePtr const & customAttribute: supplementalCustomAttributes)
        {
        Utf8String className = customAttribute->GetClass().GetName();
        if (0 == strcmp(SupplementalSchemaMetaData::GetCustomAttributeAccessor(), className.c_str()) || 0 == strcmp(s_bscaCustomAttributeAccessor, className.c_str()))
            continue;

        IECInstancePtr supplementalCustomAttribute = m_createCopyOfSupplementalCustomAttribute ? customAttribute->CreateCopyThroughSerialization() : customAttribute;
        IECInstancePtr localCustomAttribute = consolidatedCustomAttributeContainer.GetCustomAttributeLocal(customAttribute->GetClass());
        IECInstancePtr consolidatedCustomAttribute;
        
        if (customAttribute->GetClass().GetSchema().IsSupplementalSchema() && 
            nullptr != consolidatedCustomAttributeContainer.GetContainerSchema()->GetClassCP(customAttribute->GetClass().GetName().c_str()))
            supplementalCustomAttribute = customAttribute->CreateCopyThroughSerialization(*consolidatedCustomAttributeContainer.GetContainerSchema());

        if (localCustomAttribute.IsNull())
            {
            if (SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *supplementalCustomAttribute, precedence) != ECN::ECObjectsStatus::Success)
                return SupplementedSchemaStatus::SchemaMergeException;

            continue;
            }

        if (m_createCopyOfSupplementalCustomAttribute)
            consolidatedCustomAttribute = localCustomAttribute->CreateCopyThroughSerialization();
        else
            consolidatedCustomAttribute = localCustomAttribute;

        // We don't use merging delegates like in the managed world, but Units custom attributes still need to be treated specially
        if (customAttribute->GetClass().GetSchema().GetName().EqualsI("Unit_Attributes"))  // changed from "Unit_Attributes.01.00" - ECSchema::GetName() does not include the version numbers...
            {
            if (customAttribute->GetClass().GetName().EqualsI("UnitSpecificationAttr")) //UnitSpecificationAttr is the new name for the attribute to make it not clash with UnitSpecification which is only a struct now.
                status = MergeUnitSpecificationCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            else if (customAttribute->GetClass().GetName().EqualsI("UnitSpecifications"))
                status = MergeUnitSpecificationsCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            else
                status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            }
        else
            status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);

        if (SupplementedSchemaStatus::Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementedSchemaBuilder::SetMergedCustomAttribute(IECCustomAttributeContainerR container, IECInstanceR customAttributeInstance, SchemaPrecedence precedence)
    {
    ECSchemaR customAttributeSchema = const_cast<ECSchemaR>(customAttributeInstance.GetClass().GetSchema());
    ECSchemaP containerSchema = container.GetContainerSchema();
    IECInstancePtr copiedInstance = nullptr;
    if (containerSchema != &(customAttributeSchema))
        {
        // Custom attributes should never be defined in a supplemental schema, but there are schemas out there that do this.  So we need to work around that
        // by copying the class over to the target schema
        if (customAttributeSchema.IsSupplementalSchema())
            {
            ECClassP containerCA = nullptr;
            containerSchema->CopyClass(containerCA, customAttributeInstance.GetClass(), true);
            copiedInstance = customAttributeInstance.CreateCopyThroughSerialization(*containerSchema);
            }
        else if (!ECSchema::IsSchemaReferenced(*containerSchema, customAttributeSchema))
            {
            ECObjectsStatus status = containerSchema->AddReferencedSchema(customAttributeSchema);
            if (status != ECObjectsStatus::Success)
                return status;
            }
        }

    return container.SetCustomAttribute(copiedInstance.IsValid() ? *copiedInstance : customAttributeInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::SupplementClass(ECSchemaR primarySchema, ECSchemaP supplementalSchema, ECClassP supplementalECClass, SchemaPrecedence precedence, Utf8StringCP supplementalSchemaFullName)
    {
    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;
    if (supplementalECClass->HasBaseClasses())
        {
        LOG.errorv("The class '%s' from the Supplemental Schema '%s' has one or more base classes.  This is not allowed.",
                   supplementalECClass->GetName().c_str(), supplementalSchemaFullName->c_str());
        return SupplementedSchemaStatus::SupplementalClassHasBaseClass;
        }

    ECClassP consolidatedECClass = primarySchema.GetClassP(supplementalECClass->GetName().c_str());
    if (NULL == consolidatedECClass)
        {
        if (supplementalECClass->IsCustomAttributeClass())
            primarySchema.CopyClass(consolidatedECClass, *supplementalECClass, true);
        return SupplementedSchemaStatus::Success;
        }


    ECRelationshipClassP relationship = supplementalECClass->GetRelationshipClassP();
    // If this is a relationship class merge custom attributes on the source and target constraints
    if (NULL != relationship)
        status = MergeRelationshipClassConstraints(consolidatedECClass, relationship, precedence);

    if (SupplementedSchemaStatus::Success != status)
        return status;

    // Merge the custom attributes on the class
    ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalECClass->GetCustomAttributes(false);
    status = MergeCustomAttributeClasses(*consolidatedECClass, supplementalCustomAttributes, precedence, supplementalSchemaFullName, NULL);

    if (SupplementedSchemaStatus::Success != status)
        return status;

    // work on Custom Attributes applied to each property
    status = MergePropertyCustomAttributes(consolidatedECClass, supplementalECClass, precedence);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeRelationshipClassConstraints(ECClassP consolidatedECClass, ECRelationshipClassP supplementalECRelationshipClass, SchemaPrecedence precedence)
    {
    Utf8String supplementalSchemaFullName = supplementalECRelationshipClass->GetSchema().GetFullSchemaName();

    ECRelationshipClassP consolidatedECRelationshipClass = consolidatedECClass ? consolidatedECClass->GetRelationshipClassP() : NULL;
    if (NULL == consolidatedECRelationshipClass)
        {
        LOG.errorv("The supplemental class is an ECRelationshipClass but the primary class is not.  Class name: '%s.%s'",
            supplementalSchemaFullName.c_str(), supplementalECRelationshipClass->GetName().c_str());
        return SupplementedSchemaStatus::SchemaMergeException;
        }

    Utf8String consolidatedSchemaFullName = consolidatedECRelationshipClass->GetSchema().GetFullSchemaName();

    SupplementedSchemaStatus status;
    status = MergeCustomAttributeClasses(consolidatedECRelationshipClass->GetTarget(), supplementalECRelationshipClass->GetTarget().GetCustomAttributes(false),
        precedence, &supplementalSchemaFullName, &consolidatedSchemaFullName);

    if (SupplementedSchemaStatus::Success != status)
        return status;

    return MergeCustomAttributeClasses(consolidatedECRelationshipClass->GetSource(), supplementalECRelationshipClass->GetSource().GetCustomAttributes(false),
        precedence, &supplementalSchemaFullName, &consolidatedSchemaFullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergePropertyCustomAttributes(ECClassP consolidatedECClass, ECClassP supplementalECClass, SchemaPrecedence precedence)
    {
    SupplementedSchemaStatus status = SupplementedSchemaStatus::Success;
    for(ECPropertyP supplementalECProperty: supplementalECClass->GetProperties(false))
        {
        ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalECProperty->GetCustomAttributes(false);

        if (supplementalCustomAttributes.begin() == supplementalCustomAttributes.end())
            continue;

       ECPropertyP consolidatedECProperty = consolidatedECClass->GetPropertyP(supplementalECProperty->GetName(), false);
        if (NULL == consolidatedECProperty)
            {
            ECPropertyP inheritedECProperty = consolidatedECClass->GetPropertyP(supplementalECProperty->GetName(), true);
            if (NULL == inheritedECProperty)
                {
                LOG.debugv("%s supplements non-existent ECProperty %s.%s",
                    supplementalECClass->GetFullName(), consolidatedECClass->GetFullName(), supplementalECProperty->GetName().c_str());
                continue;
                }

            ECObjectsStatus copyStatus = consolidatedECClass->CopyPropertyForSupplementation(consolidatedECProperty, inheritedECProperty, false);
            if (ECObjectsStatus::Success != copyStatus)
                continue;

            consolidatedECProperty->SetBaseProperty (inheritedECProperty);
            // By adding this property override it is possible that classes derived from this one that override this property
            // will need to have the BaseProperty updated to the newly added temp property.
            for(ECClassP derivedClass: consolidatedECClass->GetDerivedClasses())
                {
                ECPropertyP derivedECProperty = derivedClass->GetPropertyP(supplementalECProperty->GetName(), false);
                if (NULL != derivedECProperty)
                    derivedECProperty->SetBaseProperty(consolidatedECProperty);
                }
            }

        Utf8String schemaName = supplementalECClass->GetSchema().GetFullSchemaName();
        status = MergeCustomAttributeClasses(*consolidatedECProperty, supplementalCustomAttributes, precedence, &schemaName, NULL);
        if (SupplementedSchemaStatus::Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeStandardCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence)
    {
    ECClassCR customAttributeClass = supplementalCustomAttribute.GetClass();
    if (SCHEMA_PRECEDENCE_Greater == precedence)
        {
        if (ECObjectsStatus::Success != SetMergedCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SupplementedSchemaStatus::SchemaMergeException;
        }
    // This case is ONLY for dealing with two supplemental ECSchemas that have the same precedence.
    // A supplemental schema CAN NOT be a primary schema and hence can NOT be supplemented,
    // because of that we must work on the actual primary custom attributes.
    else if (SCHEMA_PRECEDENCE_Equal == precedence)
        {
        IECInstancePtr primaryCustomAttribute = consolidatedCustomAttributeContainer.GetCustomAttribute(customAttributeClass);
        if (primaryCustomAttribute.IsValid())
            {
            LOG.errorv("The ECCustomAttribute: %s:%s exists in the same place in two ECSchemas that have the same precedence",
                customAttributeClass.GetSchema().GetFullSchemaName().c_str(), customAttributeClass.GetName().c_str());
            return SupplementedSchemaStatus::SchemaMergeException;
            }

        if (ECObjectsStatus::Success != SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SupplementedSchemaStatus::SchemaMergeException;
        }
    else if (NULL == consolidatedCustomAttribute)
        if (ECObjectsStatus::Success != SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SupplementedSchemaStatus::SchemaMergeException;

    return SupplementedSchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SupplementedSchemaStatus mergeAttributeProperty (IECInstanceR to, IECInstanceCR from, Utf8CP accessor, bool checkForConflict)
    {
    ECValue vFrom;
    from.GetValue (vFrom, accessor);

    bool toIsNull;
    if (ECObjectsStatus::Success != to.IsPropertyNull (toIsNull, accessor))
        return SupplementedSchemaStatus::SchemaMergeException;

    if (toIsNull)
        to.SetValue (accessor, vFrom);
    else if (checkForConflict)
        {
        ECValue vTo;
        to.GetValue (vTo, accessor);
        if (vTo.IsString() && Utf8String::IsNullOrEmpty(vTo.GetUtf8CP()))
            to.SetValue(accessor, vFrom);
        else if (!vTo.Equals (vFrom))
            return SupplementedSchemaStatus::SchemaMergeException;
        }
    
    return SupplementedSchemaStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool allowableUnitsContains (Utf8CP search, IECInstanceCR instance, uint32_t size)
    {
    ECValue v;
    for (uint32_t i = 0; i < size; i++)
        {
        if (ECObjectsStatus::Success == instance.GetValue (v, "AllowableUnits", i) && !v.IsNull() && 0 == strcmp (search, v.GetUtf8CP()))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SupplementedSchemaStatus mergeUnitSpecification (IECInstanceR to, IECInstanceCR from, bool detectConflicts)
    {
    Utf8CP propertyNames[] = { "DimensionName", "KindOfQuantityName", "UnitName" };
    for (size_t i = 0; i < _countof(propertyNames); i++)
        {
        SupplementedSchemaStatus mergeStatus = mergeAttributeProperty (to, from, propertyNames[i], detectConflicts);
        if (SupplementedSchemaStatus::Success != mergeStatus)
            return mergeStatus;
        }

    if (detectConflicts)
        {
        // Native code doesn't use AllowableUnits, but we have to compare them for managed (though managed doesn't really seem to use them either...)
        ECValue listTo, listFrom;
        to.GetValue (listTo, "AllowableUnits");
        from.GetValue (listFrom, "AllowableUnits");
        if (listTo.IsNull() != listFrom.IsNull())
            return SupplementedSchemaStatus::SchemaMergeException; // they differ
        else if (!listTo.IsNull())  // both are non-null, compare them
            {
            ArrayInfo infoTo    = listTo.GetArrayInfo(),
                      infoFrom  = listFrom.GetArrayInfo();
            if (infoTo.GetCount() != infoFrom.GetCount())
                return SupplementedSchemaStatus::SchemaMergeException; // they differ

            // compare contents.
            uint32_t size = infoTo.GetCount();
            for (uint32_t i = 0; i < size; i++)
                {
                ECValue v;
                if (ECObjectsStatus::Success == to.GetValue (v, "AllowableUnits", i) && !v.IsNull() && !allowableUnitsContains (v.GetUtf8CP(), from, size))
                    return SupplementedSchemaStatus::SchemaMergeException; // unit in one list not found in another
                }
            }
        }

    return SupplementedSchemaStatus::Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence)
    {
    ECObjectsStatus setStatus = ECObjectsStatus::Success;
    if (NULL != consolidatedCustomAttribute)
        {
        IECInstanceP to = SCHEMA_PRECEDENCE_Greater == precedence ? &supplementalCustomAttribute : consolidatedCustomAttribute;
        IECInstanceCP from = to == &supplementalCustomAttribute ? consolidatedCustomAttribute : &supplementalCustomAttribute;
        bool detectConflicts = SCHEMA_PRECEDENCE_Equal == precedence;
        
        SupplementedSchemaStatus mergeStatus = mergeUnitSpecification (*to, *from, detectConflicts);
        if (SupplementedSchemaStatus::Success != mergeStatus)
            return mergeStatus;

        setStatus = SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *to, precedence);
        }
    else
        setStatus = SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence);

    return setStatus == ECObjectsStatus::Success ? SupplementedSchemaStatus::Success : SupplementedSchemaStatus::SchemaMergeException;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void buildUnitSpecificationKey (Utf8StringR key, IECInstanceCR spec)
    {
    ECValue v;
    if (ECObjectsStatus::Success == spec.GetValue (v, "KindOfQuantityName") && !v.IsNull())
        key = v.GetUtf8CP();
    else if (ECObjectsStatus::Success == spec.GetValue (v, "DimensionName") && !v.IsNull())
        key = v.GetUtf8CP();
    else
        key = "@#$";   // a Dimension or KOQ name must be a valid ECClass name, so any invalid string suffices to represent "KindOfQuantity and Dimension not present"
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationsCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence)
    {
    IECInstanceP attributeToStore = &supplementalCustomAttribute;
    if (NULL != consolidatedCustomAttribute)
        {
        // Each attribute contains a list of UnitSpecification instances
        // We want to combine both lists into one
        // Where both lists contain an entry with the same "key", we select or merge based on precedence
        IECInstanceP to = SCHEMA_PRECEDENCE_Greater == precedence ? &supplementalCustomAttribute : consolidatedCustomAttribute;
        IECInstanceCP from = to == &supplementalCustomAttribute ? consolidatedCustomAttribute : &supplementalCustomAttribute;
        bool detectConflicts = SCHEMA_PRECEDENCE_Equal == precedence;

        ECValue toList, fromList;
        to->GetValue (toList, "UnitSpecificationList");
        from->GetValue (fromList, "UnitSpecificationList");

        ArrayInfo toInfo    = toList.GetArrayInfo();
        ArrayInfo fromInfo  = fromList.GetArrayInfo();

        // build the set UnitSpecification instances in destination list
        bmap<Utf8String, IECInstancePtr> toSpecs;
        Utf8String unitSpecKey;
        for (uint32_t i = 0; i < toInfo.GetCount(); i++)
            {
            ECValue spec;
            if (ECObjectsStatus::Success == to->GetValue (spec, "UnitSpecificationList", i) && !spec.IsNull())
                {
                buildUnitSpecificationKey (unitSpecKey, *spec.GetStruct());
                toSpecs[unitSpecKey] = spec.GetStruct();
                }
            }

        // merge each UnitSpecification instance from source list
        uint32_t specCount = toInfo.GetCount();
        for (uint32_t i = 0; i < fromInfo.GetCount(); i++)
            {
            ECValue spec;
            if (ECObjectsStatus::Success == from->GetValue (spec, "UnitSpecificationList", i) && !spec.IsNull())
                {
                buildUnitSpecificationKey (unitSpecKey, *spec.GetStruct());
                bmap<Utf8String, IECInstancePtr>::const_iterator found = toSpecs.find (unitSpecKey);
                if (toSpecs.end() != found)
                    {
                    SupplementedSchemaStatus mergeStatus = mergeUnitSpecification (*(found->second), *spec.GetStruct(), detectConflicts);
                    if (SupplementedSchemaStatus::Success != mergeStatus)
                        return mergeStatus;
                    }
                else
                    {
                    // add the spec
                    to->AddArrayElements ("UnitSpecificationList", 1);
                    ++specCount;
                    ECValue newSpec;
                    newSpec.SetStruct (spec.GetStruct().get());
                    to->SetValue ("UnitSpecificationList", newSpec, specCount - 1);
                    }
                }
            }

        attributeToStore = to;
        }

    return ECObjectsStatus::Success == SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *attributeToStore, precedence) ? SupplementedSchemaStatus::Success : SupplementedSchemaStatus::SchemaMergeException;
    }

Utf8CP SupplementalSchemaInfo::s_customAttributeAccessor = "SupplementalProvenance";
Utf8CP SupplementalSchemaInfo::s_customAttributeSchemaName = "CoreCustomAttributes";

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfo::SupplementalSchemaInfo(Utf8StringCR primarySchemaFullName, SchemaNamePurposeMap& schemaFullNameToPurposeMapping) : m_primarySchemaFullName(primarySchemaFullName)
    {
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = schemaFullNameToPurposeMapping.begin(); iter != schemaFullNameToPurposeMapping.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        m_supplementalSchemaNamesAndPurpose[Utf8String(entry.first)] = Utf8String(entry.second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementalSchemaInfo::GetSupplementalSchemaNames(bvector<Utf8String>& supplementalSchemaNames) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECObjectsStatus::SchemaNotSupplemented;

    // make sure the list starts out empty
    supplementalSchemaNames.clear();
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        supplementalSchemaNames.push_back(entry.first);
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP SupplementalSchemaInfo::GetPurposeOfSupplementalSchema(Utf8StringCR fullSchemaName) const
    {
    SchemaNamePurposeMap::const_iterator iter;
    iter = m_supplementalSchemaNamesAndPurpose.find(fullSchemaName);
    if (m_supplementalSchemaNamesAndPurpose.end() == iter)
        return NULL;
    return &(iter->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementalSchemaInfo::GetSupplementalSchemasWithPurpose(bvector<Utf8String>& supplementalSchemaNames, Utf8StringCR purpose) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECObjectsStatus::SchemaNotSupplemented;

    // make sure the list starts out empty
    supplementalSchemaNames.clear();
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        Utf8String storedPurpose = entry.second;
        if (0 == storedPurpose.CompareTo(purpose))
            supplementalSchemaNames.push_back(entry.first);
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaInfo::HasSameSupplementalSchemasForPurpose(ECSchemaCR secondSchema, Utf8StringCR purpose) const
    {
    bvector<Utf8String> supplementalSchemas;
    bvector<Utf8String> secondSupplementalSchemas;
    GetSupplementalSchemasWithPurpose(supplementalSchemas, purpose);
    SupplementalSchemaInfoPtr schemaInfo = secondSchema.GetSupplementalInfo();
    if (!schemaInfo.IsValid())
        return false;
    schemaInfo->GetSupplementalSchemasWithPurpose(secondSupplementalSchemas, purpose);

    if (supplementalSchemas.size() == 0 && secondSupplementalSchemas.size() == 0)
        return true;

    if (supplementalSchemas.size() != secondSupplementalSchemas.size())
        return false;

    bvector<Utf8String>::const_iterator namesInSchema;
    for (namesInSchema = supplementalSchemas.begin(); namesInSchema != supplementalSchemas.end(); ++namesInSchema)
        {
        Utf8String name = *namesInSchema;
        bool foundIt = false;
        for (size_t i = 0; i < secondSupplementalSchemas.size(); i++)
            {
            if (0 == name.compare(secondSupplementalSchemas[i]))
                {
                foundIt = true;
                break;
                }
            }
        if (!foundIt)
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaInfo::GetCustomAttributeAccessor()
    {
    return s_customAttributeAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SupplementalSchemaInfo::GetCustomAttributeSchemaName()
    {
    return s_customAttributeSchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr SupplementalSchemaInfo::CreateCustomAttribute(ECSchemaReadContextR schemaContext)
    {
    IECInstancePtr instance = CoreCustomAttributeHelper::CreateCustomAttributeInstance(schemaContext, GetCustomAttributeAccessor());
    if (!instance.IsValid())
        return instance;

    ECClassCP schemaNameAndPurpose = CoreCustomAttributeHelper::GetClass(schemaContext, "SchemaNameAndPurpose");
    if (nullptr == schemaNameAndPurpose)
        return instance;

    StandaloneECEnablerPtr classEnabler = schemaNameAndPurpose->GetDefaultStandaloneEnabler();
    StandaloneECEnablerPtr memberEnabler = classEnabler->GetEnablerForStructArrayMember (schemaNameAndPurpose->GetSchema().GetSchemaKey(), schemaNameAndPurpose->GetName().c_str()); 

    SchemaNamePurposeMap::const_iterator iter;
    uint32_t arrayIndex = 0;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        Utf8String storedPurpose = entry.second;
        StandaloneECInstancePtr memberInst = memberEnabler->CreateInstance().get();

        ECValue v1(entry.first.c_str());
        memberInst->SetValue("SchemaName", v1);
        ECValue v2(entry.second.c_str());
        memberInst->SetValue("Purpose", v2);

        ECValue v;
        v.SetStruct (memberInst.get());
        instance->AddArrayElements("SupplementalSchemaNamesAndPurposes", 1);
        instance->SetValue ("SupplementalSchemaNamesAndPurposes", v, arrayIndex++);
        }

    return instance;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
