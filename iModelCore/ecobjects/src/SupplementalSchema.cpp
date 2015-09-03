/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SupplementalSchema.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaData::SupplementalSchemaMetaData
(
Utf8String primarySchemaName, 
uint32_t primarySchemaMajorVersion, 
uint32_t primarySchemaMinorVersion, 
uint32_t supplementalSchemaPrecedence, 
Utf8String supplementalSchemaPurpose, 
bool isUserSpecific 
)
    {
    m_primarySchemaName            = primarySchemaName;
    m_primarySchemaMajorVersion    = primarySchemaMajorVersion;
    m_primarySchemaMinorVersion    = primarySchemaMinorVersion;
    m_supplementalSchemaPrecedence = supplementalSchemaPrecedence;
    m_supplementalSchemaPurpose    = supplementalSchemaPurpose;
    m_isUserSpecific               = isUserSpecific;
    }

Utf8CP SupplementalSchemaMetaData::s_customAttributeAccessor = "SupplementalSchemaMetaData";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaData::SupplementalSchemaMetaData
(
IECInstanceCR supplementalSchemaMetaDataCustomAttribute
)
    {
    assert (0 == supplementalSchemaMetaDataCustomAttribute.GetClass().GetName().compare(GetCustomAttributeAccessor()));

    ECValue propertyValue;
    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaNamePropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaName = propertyValue.GetUtf8CP();

    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMajorVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaMajorVersion = propertyValue.GetInteger();

    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMinorVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaMinorVersion = propertyValue.GetInteger();

    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrecedencePropertyAccessor()) && !propertyValue.IsNull())
        m_supplementalSchemaPrecedence = propertyValue.GetInteger();

    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPurposePropertyAccessor()) && !propertyValue.IsNull())
        m_supplementalSchemaPurpose = propertyValue.GetUtf8CP();

    if (ECOBJECTS_STATUS_Success == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetIsUserSpecificPropertyAccessor()) && !propertyValue.IsNull())
        m_isUserSpecific = propertyValue.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaDataPtr SupplementalSchemaMetaData::Create
( 
Utf8String primarySchemaName, 
uint32_t primarySchemaMajorVersion, 
uint32_t primarySchemaMinorVersion, 
uint32_t supplementalSchemaPrecedence, 
Utf8String supplementalSchemaPurpose, 
bool isUserSpecific
)
    {
    return new SupplementalSchemaMetaData(primarySchemaName, primarySchemaMajorVersion, primarySchemaMinorVersion, supplementalSchemaPrecedence, supplementalSchemaPurpose, isUserSpecific);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaDataPtr SupplementalSchemaMetaData::Create(IECInstanceCR supplementalSchemaMetaDataCustomAttribute)
    {
    return new SupplementalSchemaMetaData(supplementalSchemaMetaDataCustomAttribute);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetCustomAttributeAccessor()
    {
    return s_customAttributeAccessor;
    }

static Utf8CP s_primarySchemaNameAccessor = "PrimarySchemaName";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaNamePropertyAccessor()
    {
    return s_primarySchemaNameAccessor;
    }

static Utf8CP s_primarySchemaVersionMajorAccessor = "PrimarySchemaMajorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaMajorVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMajorAccessor;
    }

static Utf8CP s_primarySchemaVersionMinorAccessor = "PrimarySchemaMinorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetPrimarySchemaMinorVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMinorAccessor;
    }

static Utf8CP s_precedenceAccessor = "Precedence";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetPrecedencePropertyAccessor()
    {
    return s_precedenceAccessor;
    }

static Utf8CP s_purposeAccessor = "Purpose";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetPurposePropertyAccessor()
    {
    return s_purposeAccessor;
    }

static Utf8CP s_isUserSpecificAccessor = "IsUserSpecific";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaMetaData::GetIsUserSpecificPropertyAccessor()
    {
    return s_isUserSpecificAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::TryGetFromSchema
(
SupplementalSchemaMetaDataPtr& supplementalSchemaMetadata, 
ECSchemaCR supplementalSchema
)
    {
    IECInstancePtr supplementalSchemaMetaDataCustomAttribute = supplementalSchema.GetCustomAttribute(GetCustomAttributeAccessor());
    if (!supplementalSchemaMetaDataCustomAttribute.IsValid())
        return false;

    supplementalSchemaMetadata = Create(*supplementalSchemaMetaDataCustomAttribute);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetMetadata
(
ECSchemaR supplementalSchema, 
SupplementalSchemaMetaDataR supplementalSchemaData
)
    {
    IECInstancePtr instance = supplementalSchemaData.CreateCustomAttribute();
    supplementalSchema.SetCustomAttribute(*instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::IsSupplemental
(
ECSchemaP supplementalSchema
)
    {
    if (NULL == supplementalSchema)
        return false;

    return supplementalSchema->IsDefined(GetCustomAttributeAccessor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr SupplementalSchemaMetaData::CreateCustomAttribute()
    {
    IECInstancePtr instance = StandardCustomAttributeHelper::CreateCustomAttributeInstance(SupplementalSchemaMetaData::GetCustomAttributeAccessor());
    instance->SetValue(GetPrimarySchemaNamePropertyAccessor(), ECValue(GetPrimarySchemaName().c_str()));
    instance->SetValue(GetPrimarySchemaMajorVersionPropertyAccessor(), ECValue((::int32_t)GetPrimarySchemaMajorVersion()));
    instance->SetValue(GetPrimarySchemaMinorVersionPropertyAccessor(), ECValue((::int32_t)GetPrimarySchemaMinorVersion()));
    instance->SetValue(GetPrecedencePropertyAccessor(), ECValue((::int32_t)GetSupplementalSchemaPrecedence()));
    instance->SetValue(GetPurposePropertyAccessor(), ECValue(GetSupplementalSchemaPurpose().c_str()));
    instance->SetValue(GetIsUserSpecificPropertyAccessor(), ECValue(IsUserSpecific()));

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SupplementalSchemaMetaData::GetPrimarySchemaName() const
    {
    return m_primarySchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaName
(
Utf8StringCR name
)
    {
    m_primarySchemaName = name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SupplementalSchemaMetaData::GetPrimarySchemaMajorVersion() const
    {
    return m_primarySchemaMajorVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaMajorVersion
(
uint32_t major
)
    {
    m_primarySchemaMajorVersion = major;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SupplementalSchemaMetaData::GetPrimarySchemaMinorVersion() const
    {
    return m_primarySchemaMinorVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaMinorVersion
(
uint32_t minor
)
    {
    m_primarySchemaMinorVersion = minor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SupplementalSchemaMetaData::GetSupplementalSchemaPrecedence() const
    {
    return m_supplementalSchemaPrecedence;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetSupplementalSchemaPrecedence
(
uint32_t precedence
)
    {
    m_supplementalSchemaPrecedence = precedence;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SupplementalSchemaMetaData::GetSupplementalSchemaPurpose() const
    {
    return m_supplementalSchemaPurpose;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetSupplementalSchemaPurpose
(
Utf8StringCR purpose
)
    {
    m_supplementalSchemaPurpose = purpose;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::IsUserSpecific() const
    {
    return m_isUserSpecific;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetUserSpecific
(
bool userSpecific
)
    {
    m_isUserSpecific = userSpecific;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaMetaData::IsForPrimarySchema
(
Utf8StringCR querySchemaName, 
uint32_t querySchemaMajorVersion, 
uint32_t querySchemaMinorVersion, 
SchemaMatchType matchType
) const
    {
    SchemaKey primaryKey(m_primarySchemaName.c_str(), m_primarySchemaMajorVersion, m_primarySchemaMinorVersion);
    SchemaKey queryKey(querySchemaName.c_str(), querySchemaMajorVersion, querySchemaMinorVersion);
    return primaryKey.Matches(queryKey, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::UpdateSchema
(
ECSchemaR primarySchema, 
bvector<ECSchemaP>& supplementalSchemaList,
bool createCopyOfSupplementalCustomAttribute
)
    {
    return UpdateSchema(primarySchema, supplementalSchemaList, "", createCopyOfSupplementalCustomAttribute);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::UpdateSchema
(
ECSchemaR           primarySchema,
bvector<ECSchemaP>& supplementalSchemaList,
Utf8CP locale,
bool createCopyOfSupplementalCustomAttribute
)
    {
    m_createCopyOfSupplementalCustomAttribute = createCopyOfSupplementalCustomAttribute;
    StopWatch timer (L"", true);
    bmap<uint32_t, ECSchemaP> schemasByPrecedence;
    bvector<ECSchemaP> localizationSchemas;
    SupplementedSchemaStatus status = OrderSupplementalSchemas(schemasByPrecedence, primarySchema, supplementalSchemaList, localizationSchemas);
    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
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
    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        return status;

    status = MergeSchemasIntoSupplementedSchema(primarySchema, schemasByPrecedence);
    primarySchema.SetIsSupplemented(true);
    SupplementalSchemaInfoPtr info = SupplementalSchemaInfo::Create(primarySchema.GetFullSchemaName().c_str(), m_supplementalSchemaNamesAndPurposes);
    primarySchema.SetSupplementalSchemaInfo(info.get());

    ApplyLocalizationSupplementals(primarySchema, locale, localizationSchemas);
    
    timer.Stop();
    Utf8String primarySchemaName = primarySchema.GetFullSchemaName();
    LOG.infov ("Supplemented (in %.4f seconds) %s with %d supplemental ECSchemas", timer.GetElapsedSeconds(), 
        primarySchemaName.c_str(), supplementalSchemaList.size());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementedSchemaBuilder::ApplyLocalizationSupplementals
(
ECSchemaR primarySchema,
Utf8CP locale,
bvector<ECSchemaP>& localizationSchemas
)
    {
    ECSchemaP locSchema = nullptr;
    // If not locale specified use the first localization supplemental found.
    if (0 == strcmp("", locale))
        {
        if (localizationSchemas.begin() != localizationSchemas.end ())
            {
            LOG.debugv("No locale set choosing first localization supplemental found");
            locSchema = *(localizationSchemas.begin());
            }
        }
    else // Find the localization supplemental with the matching locale
        {
        for (ECSchemaP supplemental : localizationSchemas)
            {
            if (0 == strcmp(locale, SchemaLocalizedStrings::GetLocaleFromSupplementalSchema(supplemental).c_str()))
                {
                locSchema = supplemental;
                }
            }
        }
    if (nullptr == locSchema)
        return;

    LOG.debugv("Applying localizations from %s to %s", locSchema->GetName().c_str(), primarySchema.GetName().c_str());

    primarySchema.m_localizedStrings = SchemaLocalizedStrings(locSchema, primarySchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::OrderSupplementalSchemas
(
bmap<uint32_t, ECSchemaP>& schemasByPrecedence, 
ECSchemaR primarySchema, 
const bvector<ECSchemaP>& supplementalSchemaList, 
bvector<ECSchemaP>& localizationSchemas 
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    for (ECSchemaP supplemental : supplementalSchemaList)
        {
        if (SchemaLocalizedStrings::IsLocalizationSupplementalSchema(supplemental))
            {
            localizationSchemas.push_back(supplemental);
            continue;
            }

        SupplementalSchemaMetaDataPtr metaData;
        if (!SupplementalSchemaMetaData::TryGetFromSchema(metaData, *supplemental))
            return SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing;
        if (!metaData.IsValid())
            return SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing;

        m_supplementalSchemaNamesAndPurposes[supplemental->GetFullSchemaName()] = metaData->GetSupplementalSchemaPurpose();
        uint32_t precedence = metaData->GetSupplementalSchemaPrecedence();

        bmap<uint32_t, ECSchemaP>::const_iterator precedenceIterator = schemasByPrecedence.find(precedence);

        // If multiple schemas have the same precedence, they need to be merged into a single schema.  
        if (precedenceIterator != schemasByPrecedence.end())
            {
            ECSchemaP schema1 = precedenceIterator->second;
            status = CreateMergedSchemaFromSchemasWithEqualPrecedence(schema1, supplemental);
            if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
                return status;
            SchemaKey key(schema1->GetName().c_str(), schema1->GetVersionMajor(), schema1->GetVersionMinor());
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
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::CreateMergedSchemaFromSchemasWithEqualPrecedence
(
ECSchemaP schema1, 
ECSchemaP schema2
)
    {
    ECSchemaPtr mergedSchema;
    schema1->CopySchema(mergedSchema);  //Do we need to copy it?

    Utf8String supplementalSchemaFullName = schema2->GetFullSchemaName();
    Utf8String mergedSchemaFullName = schema1->GetFullSchemaName();
    LOG.infov ("Merging %s into %s", supplementalSchemaFullName.c_str(), mergedSchemaFullName.c_str());
    MergeCustomAttributeClasses(*mergedSchema, schema2->GetPrimaryCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);

    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    for(ECClassP supplementalClass: schema2->GetClasses())
        {
        status = MergeClassesWithEqualPrecedence(mergedSchema.get(), supplementalClass, supplementalSchemaFullName, mergedSchemaFullName);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            return status;
        }
    m_schemaCache->AddSchema(*mergedSchema);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeClassesWithEqualPrecedence
(
ECSchemaP mergedSchema, 
ECClassP supplementalClass, 
Utf8StringCR supplementalSchemaFullName, 
Utf8StringCR mergedSchemaFullName
)
    {
    ECClassP mergedClass = mergedSchema->GetClassP(supplementalClass->GetName().c_str());
    // The class doesn't already exist, we need to create a new one
    if (NULL == mergedClass)
        {
        if (ECOBJECTS_STATUS_Success != mergedSchema->CopyClass(mergedClass, *supplementalClass))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        return SUPPLEMENTED_SCHEMA_STATUS_Success;
        }

    // The class does exist, we need to do a merge
    ECRelationshipClassP supplementalRelationship = supplementalClass->GetRelationshipClassP();
    if (NULL != supplementalRelationship)
        MergeRelationshipClassConstraints(mergedClass, supplementalRelationship, SCHEMA_PRECEDENCE_Equal);

    for(ECPropertyP supplementalProperty: supplementalClass->GetProperties(false))
        {
        ECPropertyP mergedProperty = mergedClass->GetPropertyP(supplementalProperty->GetName(), false);
        // Class exists but this property does not
        if (NULL == mergedProperty)
            {
            mergedClass->CopyPropertyForSupplementation(mergedProperty, supplementalProperty, true);
            }
        // Class and property exist, merge property custom attributes
        else
            {
            SupplementedSchemaStatus status = MergeCustomAttributeClasses(*mergedProperty, supplementalProperty->GetCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);
            if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
                return status;
            }
        }

    // Merge class custom attributes
    return MergeCustomAttributeClasses(*mergedClass, supplementalClass->GetCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeSchemasIntoSupplementedSchema
(
ECSchemaR primarySchema,
bmap<uint32_t, ECSchemaP> schemasByPrecedence
)
    {
    bvector<ECSchemaP> lowPrecedenceSchemas;

    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;

    for ( bmap<uint32_t, ECSchemaP>::iterator schemaWithPrecedence = schemasByPrecedence.begin(); 
          schemaWithPrecedence != schemasByPrecedence.end(); schemaWithPrecedence++)
        {
        int precedence = schemaWithPrecedence->first;
        ECSchemaP schema = schemaWithPrecedence->second;
        if (precedence <= PRECEDENCE_THRESHOLD)
            lowPrecedenceSchemas.insert(lowPrecedenceSchemas.begin(), schema);
        else
            {
            status = MergeIntoSupplementedSchema(primarySchema, schema, SCHEMA_PRECEDENCE_Greater);
            if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
                return status;
            }
        }

    for (bvector<ECSchemaP>::iterator lowPrecedenceSchema = lowPrecedenceSchemas.begin();
        lowPrecedenceSchema != lowPrecedenceSchemas.end(); lowPrecedenceSchema++)
        {
        ECSchemaP schema = *lowPrecedenceSchema;
        status = MergeIntoSupplementedSchema(primarySchema, schema, SCHEMA_PRECEDENCE_Lower);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeIntoSupplementedSchema
(
ECSchemaR primarySchema,
ECSchemaP supplementalSchema,
SchemaPrecedence precedence
)
    {
    ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalSchema->GetCustomAttributes(false);
    Utf8String supplementalSchemaFullName = supplementalSchema->GetFullSchemaName();

    SupplementedSchemaStatus status = MergeCustomAttributeClasses(primarySchema.GetCustomAttributeContainer(), supplementalCustomAttributes, precedence, &supplementalSchemaFullName, NULL);
    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        {
        LOG.errorv ("Failed to merge the custom attributes from the supplemental schema '%s' into the supplemented schema '%s'", supplementalSchemaFullName.c_str(), primarySchema.GetFullSchemaName().c_str());
        return status;
        }

    for (ECClassP ecClass: supplementalSchema->GetClasses())
        {
        status = SupplementClass(primarySchema, supplementalSchema, ecClass, precedence, &supplementalSchemaFullName);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            {
            LOG.errorv("Failed to merge the custom attributes from the supplemental class '%s' into the supplemented class '%s:%s'",
                                           ecClass->GetFullName(),  primarySchema.GetFullSchemaName().c_str(), ecClass->GetName().c_str());
            return status;
            }
        }

    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::SupplementedSchemaStatus SupplementedSchemaBuilder::MergeCustomAttributeClasses
( 
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
ECCustomAttributeInstanceIterable supplementalCustomAttributes, 
SchemaPrecedence precedence, 
Utf8StringCP supplementalSchemaFullName, 
Utf8StringCP consolidatedSchemaFullName 
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    for (IECInstancePtr const & customAttribute: supplementalCustomAttributes)
        {
        Utf8String className = customAttribute->GetClass().GetName();
        if (0 == strcmp(SupplementalSchemaMetaData::GetCustomAttributeAccessor(), className.c_str()))
            continue;

        IECInstancePtr supplementalCustomAttribute = m_createCopyOfSupplementalCustomAttribute ? customAttribute->CreateCopyThroughSerialization() : customAttribute;
        IECInstancePtr localCustomAttribute = consolidatedCustomAttributeContainer.GetCustomAttributeLocal(customAttribute->GetClass());
        IECInstancePtr consolidatedCustomAttribute;
        
        if (localCustomAttribute.IsNull())
            {
            if (SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *customAttribute, precedence) != ECN::ECOBJECTS_STATUS_Success)
                return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;

            continue;
            }

        if (m_createCopyOfSupplementalCustomAttribute)
            consolidatedCustomAttribute = localCustomAttribute->CreateCopyThroughSerialization();
        else
            consolidatedCustomAttribute = localCustomAttribute;

        // We don't use merging delegates like in the managed world, but Units custom attributes still need to be treated specially
        if (customAttribute->GetClass().GetSchema().GetName().EqualsI("Unit_Attributes"))  // changed from "Unit_Attributes.01.00" - ECSchema::GetName() does not include the version numbers...
            {
            if (customAttribute->GetClass().GetName().EqualsI("UnitSpecification"))
                status = MergeUnitSpecificationCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            else if (customAttribute->GetClass().GetName().EqualsI("UnitSpecifications"))
                status = MergeUnitSpecificationsCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            else
                status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);
            }
        else
            status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute, consolidatedCustomAttribute.get(), precedence);

        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            return status;
        }
    for (IECInstancePtr const & customAttribute : consolidatedCustomAttributeContainer.GetPrimaryCustomAttributes(false))
        {
        ECClassCR classDefinition = customAttribute->GetClass();
        bool found = false;
        for (IECInstancePtr const & customAttribute : consolidatedCustomAttributeContainer.GetCustomAttributes(false))
            {
            ECClassCR currentClass = customAttribute->GetClass();
            if (ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
               {
               found = true;
               }
            }
        if (!found)
            {
            consolidatedCustomAttributeContainer.SetSupplementedCustomAttribute(*customAttribute);
            }
        }



    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementedSchemaBuilder::SetMergedCustomAttribute(IECCustomAttributeContainerR container, IECInstanceR customAttributeInstance, SchemaPrecedence precedence)
    {
    ECSchemaR customAttributeSchema = const_cast<ECSchemaR>(customAttributeInstance.GetClass().GetSchema());
    ECSchemaP containerSchema = container.GetContainerSchema();
    if (containerSchema != &(customAttributeSchema))
        {
        if (!ECSchema::IsSchemaReferenced(*containerSchema, customAttributeSchema))
            {
            ECObjectsStatus status = containerSchema->AddReferencedSchema(customAttributeSchema);
            if (status != ECOBJECTS_STATUS_Success)
                return status;
            }
        }

    if (precedence == SCHEMA_PRECEDENCE_Equal)
        return container.SetPrimaryCustomAttribute(customAttributeInstance);
    else
        return container.SetSupplementedCustomAttribute(customAttributeInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::SupplementClass
(
ECSchemaR primarySchema,
ECSchemaP supplementalSchema,
ECClassP supplementalECClass,
SchemaPrecedence precedence,
Utf8StringCP supplementalSchemaFullName
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    ECClassP consolidatedECClass = primarySchema.GetClassP(supplementalECClass->GetName().c_str());
    if (NULL == consolidatedECClass)
        return SUPPLEMENTED_SCHEMA_STATUS_Success;

    if (supplementalECClass->HasBaseClasses())
        {
        LOG.errorv("The class '%s' from the Supplemental Schema '%s' has one or more base classes.  This is not allowed.",
            supplementalECClass->GetName().c_str(), supplementalSchemaFullName->c_str());
        return SUPPLEMENTED_SCHEMA_STATUS_SupplementalClassHasBaseClass;
        }

    ECRelationshipClassP relationship = supplementalECClass->GetRelationshipClassP();
    // If this is a relationship class merge custom attributes on the source and target constraints
    if (NULL != relationship)
        status = MergeRelationshipClassConstraints(consolidatedECClass, relationship, precedence);

    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        return status;

    // Merge the custom attributes on the class
    ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalECClass->GetCustomAttributes(false);
    status = MergeCustomAttributeClasses(*consolidatedECClass, supplementalCustomAttributes, precedence, supplementalSchemaFullName, NULL);

    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        return status;

    // work on Custom Attributes applied to each property
    status = MergePropertyCustomAttributes(consolidatedECClass, supplementalECClass, precedence);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeRelationshipClassConstraints
(
ECClassP consolidatedECClass, 
ECRelationshipClassP supplementalECRelationshipClass, 
SchemaPrecedence precedence
)
    {
    Utf8String supplementalSchemaFullName = supplementalECRelationshipClass->GetSchema().GetFullSchemaName();

    ECRelationshipClassP consolidatedECRelationshipClass = consolidatedECClass ? consolidatedECClass->GetRelationshipClassP() : NULL;
    if (NULL == consolidatedECRelationshipClass)
        {
        LOG.errorv("The supplemental class is an ECRelationshipClass but the primary class is not.  Class name: '%s.%s'",
            supplementalSchemaFullName.c_str(), supplementalECRelationshipClass->GetName().c_str());
        return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }

    Utf8String consolidatedSchemaFullName = consolidatedECRelationshipClass->GetSchema().GetFullSchemaName();

    SupplementedSchemaStatus status;
    status = MergeCustomAttributeClasses(consolidatedECRelationshipClass->GetTarget(), supplementalECRelationshipClass->GetTarget().GetCustomAttributes(false),
        precedence, &supplementalSchemaFullName, &consolidatedSchemaFullName);

    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        return status;

    return MergeCustomAttributeClasses(consolidatedECRelationshipClass->GetSource(), supplementalECRelationshipClass->GetSource().GetCustomAttributes(false),
        precedence, &supplementalSchemaFullName, &consolidatedSchemaFullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergePropertyCustomAttributes
(
ECClassP consolidatedECClass, 
ECClassP supplementalECClass, 
SchemaPrecedence precedence
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
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

            ECObjectsStatus status = consolidatedECClass->CopyPropertyForSupplementation(consolidatedECProperty, inheritedECProperty, false);
            if (ECOBJECTS_STATUS_Success != status)
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
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeStandardCustomAttribute
(
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
IECInstanceR supplementalCustomAttribute, 
IECInstanceP consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
    {
    ECClassCR customAttributeClass = supplementalCustomAttribute.GetClass();
    if (SCHEMA_PRECEDENCE_Greater == precedence)
        {
        if (ECOBJECTS_STATUS_Success != SetMergedCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }
    // This case is ONLY for dealing with two supplemental ECSchemas that have the same precedence.
    // A supplemental schema CAN NOT be a primary schema and hence can NOT be supplemented,
    // because of that we must work on the actual primary custom attributes.
    else if (SCHEMA_PRECEDENCE_Equal == precedence)
        {
        IECInstancePtr primaryCustomAttribute = consolidatedCustomAttributeContainer.GetPrimaryCustomAttribute(customAttributeClass);
        if (primaryCustomAttribute.IsValid())
            {
            LOG.errorv("The ECCustomAttribute: %s:%s exists in the same place in two ECSchemas that have the same precedence",
                customAttributeClass.GetSchema().GetFullSchemaName().c_str(), customAttributeClass.GetName().c_str());
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
            }

        if (ECOBJECTS_STATUS_Success != SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }
    else if (NULL == consolidatedCustomAttribute)
        if (ECOBJECTS_STATUS_Success != SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;

    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static SupplementedSchemaStatus mergeAttributeProperty (IECInstanceR to, IECInstanceCR from, Utf8CP accessor, bool checkForConflict)
    {
    ECValue vFrom;
    from.GetValue (vFrom, accessor);

    bool toIsNull;
    if (ECOBJECTS_STATUS_Success != to.IsPropertyNull (toIsNull, accessor))
        return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;

    if (toIsNull)
        to.SetValue (accessor, vFrom);
    else if (checkForConflict)
        {
        ECValue vTo;
        to.GetValue (vTo, accessor);
        if (!vTo.Equals (vFrom))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }
    
    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool allowableUnitsContains (Utf8CP search, IECInstanceCR instance, uint32_t size)
    {
    ECValue v;
    for (uint32_t i = 0; i < size; i++)
        {
        if (ECOBJECTS_STATUS_Success == instance.GetValue (v, "AllowableUnits", i) && !v.IsNull() && 0 == strcmp (search, v.GetUtf8CP()))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static SupplementedSchemaStatus mergeUnitSpecification (IECInstanceR to, IECInstanceCR from, bool detectConflicts)
    {
    Utf8CP propertyNames[] = { "DimensionName", "KindOfQuantityName", "UnitName" };
    for (size_t i = 0; i < _countof(propertyNames); i++)
        {
        SupplementedSchemaStatus mergeStatus = mergeAttributeProperty (to, from, propertyNames[i], detectConflicts);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != mergeStatus)
            return mergeStatus;
        }

    if (detectConflicts)
        {
        // Native code doesn't use AllowableUnits, but we have to compare them for managed (though managed doesn't really seem to use them either...)
        ECValue listTo, listFrom;
        to.GetValue (listTo, "AllowableUnits");
        from.GetValue (listFrom, "AllowableUnits");
        if (listTo.IsNull() != listFrom.IsNull())
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException; // they differ
        else if (!listTo.IsNull())  // both are non-null, compare them
            {
            ArrayInfo infoTo    = listTo.GetArrayInfo(),
                      infoFrom  = listFrom.GetArrayInfo();
            if (infoTo.GetCount() != infoFrom.GetCount())
                return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException; // they differ

            // compare contents.
            uint32_t size = infoTo.GetCount();
            for (uint32_t i = 0; i < size; i++)
                {
                ECValue v;
                if (ECOBJECTS_STATUS_Success == to.GetValue (v, "AllowableUnits", i) && !v.IsNull() && !allowableUnitsContains (v.GetUtf8CP(), from, size))
                    return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException; // unit in one list not found in another
                }
            }
        }

    return SUPPLEMENTED_SCHEMA_STATUS_Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationCustomAttribute
(
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
IECInstanceR supplementalCustomAttribute, 
IECInstanceP consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
    {
    ECObjectsStatus setStatus = ECOBJECTS_STATUS_Success;
    if (NULL != consolidatedCustomAttribute)
        {
        IECInstanceP to = SCHEMA_PRECEDENCE_Greater == precedence ? &supplementalCustomAttribute : consolidatedCustomAttribute;
        IECInstanceCP from = to == &supplementalCustomAttribute ? consolidatedCustomAttribute : &supplementalCustomAttribute;
        bool detectConflicts = SCHEMA_PRECEDENCE_Equal == precedence;
        
        SupplementedSchemaStatus mergeStatus = mergeUnitSpecification (*to, *from, detectConflicts);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != mergeStatus)
            return mergeStatus;

        setStatus = SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *to, precedence);
        }
    else
        setStatus = SetMergedCustomAttribute (consolidatedCustomAttributeContainer, supplementalCustomAttribute, precedence);

    return setStatus == ECOBJECTS_STATUS_Success ? SUPPLEMENTED_SCHEMA_STATUS_Success : SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void buildUnitSpecificationKey (Utf8StringR key, IECInstanceCR spec)
    {
    ECValue v;
    if (ECOBJECTS_STATUS_Success == spec.GetValue (v, "KindOfQuantityName") && !v.IsNull())
        key = v.GetUtf8CP();
    else if (ECOBJECTS_STATUS_Success == spec.GetValue (v, "DimensionName") && !v.IsNull())
        key = v.GetUtf8CP();
    else
        key = "@#$";   // a Dimension or KOQ name must be a valid ECClass name, so any invalid string suffices to represent "KindOfQuantity and Dimension not present"
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationsCustomAttribute
(
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
IECInstanceR supplementalCustomAttribute, 
IECInstanceP consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
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
            if (ECOBJECTS_STATUS_Success == to->GetValue (spec, "UnitSpecificationList", i) && !spec.IsNull())
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
            if (ECOBJECTS_STATUS_Success == from->GetValue (spec, "UnitSpecificationList", i) && !spec.IsNull())
                {
                buildUnitSpecificationKey (unitSpecKey, *spec.GetStruct());
                bmap<Utf8String, IECInstancePtr>::const_iterator found = toSpecs.find (unitSpecKey);
                if (toSpecs.end() != found)
                    {
                    SupplementedSchemaStatus mergeStatus = mergeUnitSpecification (*(found->second), *spec.GetStruct(), detectConflicts);
                    if (SUPPLEMENTED_SCHEMA_STATUS_Success != mergeStatus)
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

    return ECOBJECTS_STATUS_Success == SetMergedCustomAttribute (consolidatedCustomAttributeContainer, *attributeToStore, precedence) ? SUPPLEMENTED_SCHEMA_STATUS_Success : SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
    }

Utf8CP SupplementalSchemaInfo::s_customAttributeAccessor = "SupplementalProvenance";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfo::SupplementalSchemaInfo
(
Utf8StringCR primarySchemaFullName, 
SchemaNamePurposeMap& schemaFullNameToPurposeMapping
) : m_primarySchemaFullName(primarySchemaFullName)
    {
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = schemaFullNameToPurposeMapping.begin(); iter != schemaFullNameToPurposeMapping.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        m_supplementalSchemaNamesAndPurpose[Utf8String(entry.first)] = Utf8String(entry.second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfoPtr SupplementalSchemaInfo::Create
(
Utf8StringCR primarySchemaFullName, 
SchemaNamePurposeMap& schemaFullNameToPurposeMapping
)
    {
    return new SupplementalSchemaInfo(primarySchemaFullName, schemaFullNameToPurposeMapping);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementalSchemaInfo::GetSupplementalSchemaNames
(
bvector<Utf8String>& supplementalSchemaNames
) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECOBJECTS_STATUS_SchemaNotSupplemented;

    // make sure the list starts out empty
    supplementalSchemaNames.clear();
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<Utf8String, Utf8String>const& entry = *iter;
        supplementalSchemaNames.push_back(entry.first);
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP SupplementalSchemaInfo::GetPurposeOfSupplementalSchema
(
Utf8StringCR fullSchemaName
) const
    {
    SchemaNamePurposeMap::const_iterator iter;
    iter = m_supplementalSchemaNamesAndPurpose.find(fullSchemaName);
    if (m_supplementalSchemaNamesAndPurpose.end() == iter)
        return NULL;
    return &(iter->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementalSchemaInfo::GetSupplementalSchemasWithPurpose
(
bvector<Utf8String>& supplementalSchemaNames, 
Utf8StringCR purpose
) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECOBJECTS_STATUS_SchemaNotSupplemented;

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

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupplementalSchemaInfo::HasSameSupplementalSchemasForPurpose
(
ECSchemaCR secondSchema, 
Utf8StringCR purpose
) const
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
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SupplementalSchemaInfo::GetCustomAttributeAccessor()
    {
    return s_customAttributeAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr SupplementalSchemaInfo::CreateCustomAttribute()
    {
    IECInstancePtr instance = StandardCustomAttributeHelper::CreateCustomAttributeInstance(GetCustomAttributeAccessor());
    if (!instance.IsValid())
        return instance;

    ECClassCP schemaNameAndPurpose = StandardCustomAttributeHelper::GetCustomAttributeClass("SchemaNameAndPurpose");
    if (NULL == schemaNameAndPurpose)
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
        instance->SetValue ("SupplementalSchemaNamesAndPurposes", v, arrayIndex++);

        }

    return instance;
    }
END_BENTLEY_ECOBJECT_NAMESPACE

