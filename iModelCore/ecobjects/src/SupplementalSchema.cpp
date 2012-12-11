/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SupplementalSchema.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
* Helper class to hold the schema
*                                     Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct StandardCustomAttributesSchemaHolder;
typedef RefCountedPtr<StandardCustomAttributesSchemaHolder> StandardCustomAttributesSchemaHolderPtr;

struct StandardCustomAttributesSchemaHolder : RefCountedBase
{
private:
    ECSchemaPtr            m_schema;
    ECClassP               m_supplementalSchemaMetaDataClass;
    StandaloneECEnablerPtr m_enabler;

    static StandardCustomAttributesSchemaHolderPtr s_schemaHolder;

    StandardCustomAttributesSchemaHolder();

    ECSchemaPtr _GetSchema();

    IECInstancePtr _CreateSupplementalSchemaMetaDataInstance();

public:

/*__PUBLISH_SECTION_START__*/
    static StandardCustomAttributesSchemaHolderPtr GetHolder();

    static ECSchemaPtr GetSchema();

    static IECInstancePtr CreateSupplementalSchemaMetaDataInstance();
};

StandardCustomAttributesSchemaHolderPtr StandardCustomAttributesSchemaHolder::s_schemaHolder;

static WCharCP s_customAttributeAccessor = L"SupplementalSchemaMetaData";
static UInt32 s_bscaVersionMajor = 1;
static UInt32 s_bscaVersionMinor = 4;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCustomAttributesSchemaHolder::StandardCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key (L"Bentley_Standard_CustomAttributes", s_bscaVersionMajor, s_bscaVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);
    m_supplementalSchemaMetaDataClass = m_schema->GetClassP(s_customAttributeAccessor);

    m_enabler = m_supplementalSchemaMetaDataClass->GetDefaultStandaloneEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCustomAttributesSchemaHolderPtr StandardCustomAttributesSchemaHolder::GetHolder()
    {
    if (s_schemaHolder.IsNull())
        s_schemaHolder = new StandardCustomAttributesSchemaHolder();

    return s_schemaHolder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr StandardCustomAttributesSchemaHolder::_GetSchema()
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr StandardCustomAttributesSchemaHolder::GetSchema()
    {
    return GetHolder()->_GetSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributesSchemaHolder::_CreateSupplementalSchemaMetaDataInstance()
    {
    if (!m_schema.IsValid())
        _GetSchema();

    StandaloneECInstancePtr standaloneInstance = m_enabler->CreateInstance();
    return IECInstancePtr(standaloneInstance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributesSchemaHolder::CreateSupplementalSchemaMetaDataInstance()
    {
    return GetHolder()->_CreateSupplementalSchemaMetaDataInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaData::SupplementalSchemaMetaData
(
WString primarySchemaName, 
UInt32 primarySchemaMajorVersion, 
UInt32 primarySchemaMinorVersion, 
UInt32 supplementalSchemaPrecedence, 
WString supplementalSchemaPurpose, 
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
    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaNamePropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaName = propertyValue.GetString();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMajorVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaMajorVersion = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMinorVersionPropertyAccessor()) && !propertyValue.IsNull())
        m_primarySchemaMinorVersion = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrecedencePropertyAccessor()) && !propertyValue.IsNull())
        m_supplementalSchemaPrecedence = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPurposePropertyAccessor()) && !propertyValue.IsNull())
        m_supplementalSchemaPurpose = propertyValue.GetString();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetIsUserSpecificPropertyAccessor()) && !propertyValue.IsNull())
        m_isUserSpecific = propertyValue.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaMetaDataPtr SupplementalSchemaMetaData::Create
( 
WString primarySchemaName, 
UInt32 primarySchemaMajorVersion, 
UInt32 primarySchemaMinorVersion, 
UInt32 supplementalSchemaPrecedence, 
WString supplementalSchemaPurpose, 
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
WCharCP SupplementalSchemaMetaData::GetCustomAttributeAccessor()
    {
    return s_customAttributeAccessor;
    }

static WCharCP s_primarySchemaNameAccessor = L"PrimarySchemaName";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetPrimarySchemaNamePropertyAccessor()
    {
    return s_primarySchemaNameAccessor;
    }

static WCharCP s_primarySchemaVersionMajorAccessor = L"PrimarySchemaMajorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetPrimarySchemaMajorVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMajorAccessor;
    }

static WCharCP s_primarySchemaVersionMinorAccessor = L"PrimarySchemaMinorVersion";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetPrimarySchemaMinorVersionPropertyAccessor()
    {
    return s_primarySchemaVersionMinorAccessor;
    }

static WCharCP s_precedenceAccessor = L"Precedence";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetPrecedencePropertyAccessor()
    {
    return s_precedenceAccessor;
    }

static WCharCP s_purposeAccessor = L"Purpose";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetPurposePropertyAccessor()
    {
    return s_purposeAccessor;
    }

static WCharCP s_isUserSpecificAccessor = L"IsUserSpecific";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP SupplementalSchemaMetaData::GetIsUserSpecificPropertyAccessor()
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
    IECInstancePtr instance = StandardCustomAttributesSchemaHolder::CreateSupplementalSchemaMetaDataInstance();
    instance->SetValue(GetPrimarySchemaNamePropertyAccessor(), ECValue(GetPrimarySchemaName().c_str()));
    instance->SetValue(GetPrimarySchemaMajorVersionPropertyAccessor(), ECValue((::Int32)GetPrimarySchemaMajorVersion()));
    instance->SetValue(GetPrimarySchemaMinorVersionPropertyAccessor(), ECValue((::Int32)GetPrimarySchemaMinorVersion()));
    instance->SetValue(GetPrecedencePropertyAccessor(), ECValue((::Int32)GetSupplementalSchemaPrecedence()));
    instance->SetValue(GetPurposePropertyAccessor(), ECValue(GetSupplementalSchemaPurpose().c_str()));
    instance->SetValue(GetIsUserSpecificPropertyAccessor(), ECValue(IsUserSpecific()));

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SupplementalSchemaMetaData::GetPrimarySchemaName() const
    {
    return m_primarySchemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaName
(
WStringCR name
)
    {
    m_primarySchemaName = name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 SupplementalSchemaMetaData::GetPrimarySchemaMajorVersion() const
    {
    return m_primarySchemaMajorVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaMajorVersion
(
UInt32 major
)
    {
    m_primarySchemaMajorVersion = major;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 SupplementalSchemaMetaData::GetPrimarySchemaMinorVersion() const
    {
    return m_primarySchemaMinorVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetPrimarySchemaMinorVersion
(
UInt32 minor
)
    {
    m_primarySchemaMinorVersion = minor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 SupplementalSchemaMetaData::GetSupplementalSchemaPrecedence() const
    {
    return m_supplementalSchemaPrecedence;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetSupplementalSchemaPrecedence
(
UInt32 precedence
)
    {
    m_supplementalSchemaPrecedence = precedence;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SupplementalSchemaMetaData::GetSupplementalSchemaPurpose() const
    {
    return m_supplementalSchemaPurpose;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SupplementalSchemaMetaData::SetSupplementalSchemaPurpose
(
WStringCR purpose
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
WStringCR querySchemaName, 
UInt32 querySchemaMajorVersion, 
UInt32 querySchemaMinorVersion, 
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
    m_createCopyOfSupplementalCustomAttribute = createCopyOfSupplementalCustomAttribute;
    bmap<UInt32, ECSchemaP> schemasByPrecedence;
    bvector<ECSchemaP> localizationSchemas;
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;

    status = OrderSupplementalSchemas(schemasByPrecedence, primarySchema, supplementalSchemaList, localizationSchemas);
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
    
    status = MergeSchemasIntoSupplementedSchema(primarySchema, schemasByPrecedence);
    primarySchema.SetIsSupplemented(true);
    primarySchema.SetSupplementalSchemaInfo(SupplementalSchemaInfo::Create(primarySchema.GetFullSchemaName().c_str(), m_supplementalSchemaNamesAndPurposes).get());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::OrderSupplementalSchemas
(
bmap<UInt32, ECSchemaP>& schemasByPrecedence, 
ECSchemaR primarySchema, 
const bvector<ECSchemaP>& supplementalSchemaList, 
bvector<ECSchemaP> localizationSchemas 
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    for (bvector<ECSchemaP>::const_iterator iter = supplementalSchemaList.begin(); iter != supplementalSchemaList.end(); iter++)
        {
        SupplementalSchemaMetaDataPtr metaData;
        ECSchemaP supplemental = *iter;
        if (!SupplementalSchemaMetaData::TryGetFromSchema(metaData, *supplemental))
            return SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing;
        if (!metaData.IsValid())
            return SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing;

        m_supplementalSchemaNamesAndPurposes[supplemental->GetFullSchemaName()] = metaData->GetSupplementalSchemaPurpose();
        UInt32 precedence = metaData->GetSupplementalSchemaPrecedence();

        // Not supporting localization schemas

        bmap<UInt32, ECSchemaP>::const_iterator precedenceIterator = schemasByPrecedence.find(precedence);

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
    schema1->CopySchema(mergedSchema);

    WString supplementalSchemaFullName = schema2->GetFullSchemaName();
    WString mergedSchemaFullName = schema1->GetFullSchemaName();
    MergeCustomAttributeClasses(*mergedSchema, schema2->GetPrimaryCustomAttributes(false), SCHEMA_PRECEDENCE_Equal, &supplementalSchemaFullName, &mergedSchemaFullName);

    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    FOR_EACH(ECClassP supplementalClass, schema2->GetClasses())
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
WStringCR supplementalSchemaFullName, 
WStringCR mergedSchemaFullName
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
    ECRelationshipClassP supplementalRelationship = dynamic_cast<ECRelationshipClassP>(supplementalClass);
    if (NULL != supplementalRelationship)
        MergeRelationshipClassConstraints(mergedClass, supplementalRelationship, SCHEMA_PRECEDENCE_Equal);

    FOR_EACH(ECPropertyP supplementalProperty, supplementalClass->GetProperties(false))
        {
        ECPropertyP mergedProperty = mergedClass->GetPropertyP(supplementalProperty->GetName(), false);
        // Class exists but this property does not
        if (NULL == mergedProperty)
            {
            mergedClass->CopyProperty(mergedProperty, supplementalProperty);
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
bmap<UInt32, ECSchemaP> schemasByPrecedence
)
    {
    bvector<ECSchemaP> lowPrecedenceSchemas;

    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;

    for ( bmap<UInt32, ECSchemaP>::iterator schemaWithPrecedence = schemasByPrecedence.begin(); 
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
    WString supplementalSchemaFullName = supplementalSchema->GetFullSchemaName();

    SupplementedSchemaStatus status = MergeCustomAttributeClasses(primarySchema.GetCustomAttributeContainer(), supplementalCustomAttributes, precedence, &supplementalSchemaFullName, NULL);
    if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to merge the custom attributes from the supplemental schema '%ls' into the supplemented schema '%ls'", supplementalSchemaFullName.c_str(), primarySchema.GetFullSchemaName().c_str());
        return status;
        }

    FOR_EACH (ECClassP ecClass, supplementalSchema->GetClasses())
        {
        status = SupplementClass(primarySchema, supplementalSchema, ecClass, precedence, &supplementalSchemaFullName);
        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            {
            ECObjectsLogger::Log()->errorv(L"Failed to merge the custom attributes from the supplemental class '%ls' into the supplemented class '%ls:%ls'",
                                           ecClass->GetFullName(),  primarySchema.GetFullSchemaName().c_str(), ecClass->GetName().c_str());
            return status;
            }
        }

    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::ECN::SupplementedSchemaStatus SupplementedSchemaBuilder::MergeCustomAttributeClasses
( 
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
ECCustomAttributeInstanceIterable supplementalCustomAttributes, 
SchemaPrecedence precedence, 
WStringCP supplementalSchemaFullName, 
WStringCP consolidatedSchemaFullName 
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    FOR_EACH (IECInstancePtr customAttribute, supplementalCustomAttributes)
        {
        WString className = customAttribute->GetClass().GetName();
        if (0 == wcscmp(SupplementalSchemaMetaData::GetCustomAttributeAccessor(), className.c_str()))
            continue;

        IECInstancePtr supplementalCustomAttribute = m_createCopyOfSupplementalCustomAttribute ? customAttribute->CreateCopyThroughSerialization() : customAttribute;
        IECInstancePtr localCustomAttribute = consolidatedCustomAttributeContainer.GetCustomAttributeLocal(customAttribute->GetClass());
        IECInstancePtr consolidatedCustomAttribute;
        
        if (localCustomAttribute.IsValid())
            if (m_createCopyOfSupplementalCustomAttribute)
                consolidatedCustomAttribute = localCustomAttribute->CreateCopyThroughSerialization();
            else
                consolidatedCustomAttribute = localCustomAttribute;

        // We don't use merging delegates like in the managed world, but Units custom attributes still need to be treated specially
        if (customAttribute->GetClass().GetSchema().GetName().EqualsI(L"Unit_Attributes.01.00"))
            {
            if (customAttribute->GetClass().GetName().EqualsI(L"UnitSpecification"))
                status = MergeUnitSpecificationCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, consolidatedCustomAttribute, precedence);
            else if (customAttribute->GetClass().GetName().EqualsI(L"UnitSpecifications"))
                status = MergeUnitSpecificationsCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, consolidatedCustomAttribute, precedence);
            else
                status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, consolidatedCustomAttribute, precedence);
            }
        else
            status = MergeStandardCustomAttribute(consolidatedCustomAttributeContainer, supplementalCustomAttribute, consolidatedCustomAttribute, precedence);

        if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SupplementedSchemaBuilder::SetConsolidatedCustomAttribute(IECCustomAttributeContainerR container, IECInstanceR customAttributeInstance)
    {
    ECSchemaR customAttributeSchema = const_cast<ECSchemaR>(customAttributeInstance.GetClass().GetSchema());
    ECSchemaP containerSchema = container.GetContainerSchema();
    if (containerSchema != &(customAttributeSchema))
        {
        if (!ECSchema::IsSchemaReferenced (*containerSchema, customAttributeSchema))
            {
            ECObjectsStatus status = containerSchema->AddReferencedSchema (customAttributeSchema);
            if (status != ECOBJECTS_STATUS_Success)
                return status;
            }
        }
    return container.SetConsolidatedCustomAttribute (customAttributeInstance);

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
WStringCP supplementalSchemaFullName
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;
    ECClassP consolidatedECClass = primarySchema.GetClassP(supplementalECClass->GetName().c_str());
    if (NULL == consolidatedECClass)
        return SUPPLEMENTED_SCHEMA_STATUS_Success;

    if (supplementalECClass->HasBaseClasses())
        {
        ECObjectsLogger::Log()->errorv(L"The class '%ls' from the Supplemental Schema '%ls' has one or more base classes.  This is not allowed.",
            supplementalECClass->GetName().c_str(), supplementalSchemaFullName->c_str());
        return SUPPLEMENTED_SCHEMA_STATUS_SupplementalClassHasBaseClass;
        }

    ECRelationshipClassP relationship = dynamic_cast<ECRelationshipClassP>(supplementalECClass);
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
    WString supplementalSchemaFullName = supplementalECRelationshipClass->GetSchema().GetFullSchemaName();

    ECRelationshipClassP consolidatedECRelationshipClass = dynamic_cast<ECRelationshipClassP>(consolidatedECClass);
    if (NULL == consolidatedECRelationshipClass)
        {
        ECObjectsLogger::Log()->errorv(L"The supplemental class is an ECRelationshipClass but the primary class is not.  Class name: '%ls.%ls'",
            supplementalSchemaFullName.c_str(), supplementalECRelationshipClass->GetName().c_str());
        return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }

    WString consolidatedSchemaFullName = consolidatedECRelationshipClass->GetSchema().GetFullSchemaName();

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
    FOR_EACH(ECPropertyP supplementalECProperty, supplementalECClass->GetProperties(false))
        {
        ECCustomAttributeInstanceIterable supplementalCustomAttributes = supplementalECProperty->GetCustomAttributes(false);

        ECPropertyP consolidatedECProperty = consolidatedECClass->GetPropertyP(supplementalECProperty->GetName(), false);
        if (NULL == consolidatedECProperty)
            {
            ECPropertyP inheritedECProperty = consolidatedECClass->GetPropertyP(supplementalECProperty->GetName(), true);
            BeAssert(inheritedECProperty);
            if (NULL == inheritedECProperty)
                continue;

            ECObjectsStatus status = consolidatedECClass->CopyProperty(consolidatedECProperty, inheritedECProperty);
            if (ECOBJECTS_STATUS_Success != status)
                continue;

            // By adding this property override it is possible that classes derived from this one that override this property
            // will need to have the BaseProperty updated to the newly added temp property.
            FOR_EACH(ECClassP derivedClass, consolidatedECClass->GetDerivedClasses())
                {
                ECPropertyP derivedECProperty = derivedClass->GetPropertyP(supplementalECProperty->GetName(), false);
                if (NULL != derivedECProperty)
                    derivedECProperty->SetBaseProperty(consolidatedECProperty);
                }
            }
        WString schemaName = supplementalECClass->GetSchema().GetFullSchemaName();
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
IECInstancePtr supplementalCustomAttribute, 
IECInstancePtr consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
    {
    ECClassCR customAttributeClass = supplementalCustomAttribute->GetClass();
    if (SCHEMA_PRECEDENCE_Greater == precedence)
        {
        if (ECOBJECTS_STATUS_Success != SetConsolidatedCustomAttribute(consolidatedCustomAttributeContainer, *supplementalCustomAttribute))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }
    // This case is ONLY for dealing with two supplemental schemas that have the same precedence.
    // A supplemental schema CAN NOT be a primary schema and hence can NOT be supplemented,
    // because of that we must work on the actual custom attributes.
    else if (SCHEMA_PRECEDENCE_Equal == precedence)
        {
        IECInstancePtr primaryCustomAttribute = consolidatedCustomAttributeContainer.GetPrimaryCustomAttribute(customAttributeClass);
        if (primaryCustomAttribute.IsValid())
            {
            ECObjectsLogger::Log()->errorv(L"The CustomAttribute: %ls:%ls exists in the same place in two schemas which have the same precedence",
                customAttributeClass.GetSchema().GetFullSchemaName().c_str(), customAttributeClass.GetName().c_str());
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
            }

        if (ECOBJECTS_STATUS_Success != consolidatedCustomAttributeContainer.SetPrimaryCustomAttribute(*supplementalCustomAttribute))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
        }
    else if (!consolidatedCustomAttribute.IsValid())
        if (ECOBJECTS_STATUS_Success != SetConsolidatedCustomAttribute (consolidatedCustomAttributeContainer, *supplementalCustomAttribute))
            return SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;

    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationCustomAttribute
(
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
IECInstancePtr supplementalCustomAttribute, 
IECInstancePtr consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
    {
    ECObjectsStatus setStatus = ECOBJECTS_STATUS_Success;
    if (consolidatedCustomAttribute.IsValid())
        {
        // WIP - need native Units support before we can do this
        //UnitSpecification consolidatedUnitSpec = UnitsSchemaReader.GetUnitSpecificationFromValueContainer (consolidatedCustomAttribute);
        //UnitSpecification supplementalUnitSpec = UnitsSchemaReader.GetUnitSpecificationFromValueContainer (supplementalCustomAttribute);

        //UnitSpecification mergedUnitSpec = ECSchemaUnitsManager.MergeUnitSpecificationUsingPrecedence (consolidatedUnitSpec, supplementalUnitSpec, supplementalPrecedenceCompairedToConsolidated);
        //IECValueContainer customAttributeValueContainer = null;
        //UnitsSchemaCreator.BuildCustomAttributeFromUnitSpecification (mergedUnitSpec, ref customAttributeValueContainer);

        //SetCustomAttribute (consolidatedCustomAttributeContainer, customAttributeValueContainer as IECInstance);

        }
    else
        setStatus = SetConsolidatedCustomAttribute (consolidatedCustomAttributeContainer, *supplementalCustomAttribute);
    return setStatus == ECOBJECTS_STATUS_Success ? SUPPLEMENTED_SCHEMA_STATUS_Success : SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::MergeUnitSpecificationsCustomAttribute
(
IECCustomAttributeContainerR consolidatedCustomAttributeContainer, 
IECInstancePtr supplementalCustomAttribute, 
IECInstancePtr consolidatedCustomAttribute, 
SchemaPrecedence precedence
)
    {

    // WIP - Need native Units before this can be implemented

    //if (UnitsConstants.UnitSpecifications != supplementalCustomAttribute.ClassDefinition.Name)
    //    throw ThrowingPolicy.Apply (new ProgrammerException ("The supplemental custom attribute passed in is not of the correct type.\n"
    //    + "Expected: " + UnitsConstants.UnitSpecifications
    //    + "\nActual:   " + supplementalCustomAttribute.ClassDefinition.Name));

    //IECInstance unitSpecList = supplementalCustomAttribute.ClassDefinition.CreateInstance ();
    //IECArrayValue unitSpecArray = unitSpecList[UnitsConstants.UnitSpecificationList] as IECArrayValue;

    //Dictionary<string, UnitSpecification> supplementalUnitSpecDictionary = new Dictionary<string, UnitSpecification> ();
    //ECSchemaUnitsManager.BuildUnitSpecDictionaryFromUnitSpecCustomAttribute (supplementalCustomAttribute, ref supplementalUnitSpecDictionary, false);
    //Dictionary<string, UnitSpecification> consolidatedUnitSpecDictionary = new Dictionary<string, UnitSpecification> ();
    //ECSchemaUnitsManager.BuildUnitSpecDictionaryFromUnitSpecCustomAttribute (consolidatedCustomAttribute, ref consolidatedUnitSpecDictionary, false);

    //if (consolidatedUnitSpecDictionary.Count == 0)
    //    {
    //    string caOrigin;
    //    if (CustomAttributeOriginAccessor.TryGetOrigin (supplementalCustomAttribute, out caOrigin))
    //        CustomAttributeOriginAccessor.SetOrigin (unitSpecList, caOrigin);
    //    }

    //List<UnitSpecification> finalUnitSpecificationList = new List<UnitSpecification> ();
    //foreach (string unitSpecKey in supplementalUnitSpecDictionary.Keys)
    //    {
    //    UnitSpecification consolidatedUnitSpec = null;
    //    if (consolidatedUnitSpecDictionary.TryGetValue (unitSpecKey, out consolidatedUnitSpec))
    //        {
    //        UnitSpecification mergedUnitSpec = ECSchemaUnitsManager.MergeUnitSpecificationUsingPrecedence (consolidatedUnitSpec, supplementalUnitSpecDictionary[unitSpecKey], supplementalPrecedenceCompairedToConsolidated);
    //        consolidatedUnitSpecDictionary.Remove (unitSpecKey);
    //        finalUnitSpecificationList.Add (mergedUnitSpec);
    //        }
    //    else
    //        finalUnitSpecificationList.Add (supplementalUnitSpecDictionary[unitSpecKey]);
    //    }

    //foreach (UnitSpecification unitSpec in consolidatedUnitSpecDictionary.Values)
    //    finalUnitSpecificationList.Add (unitSpec);

    //for (int unitSpecIndex = 0; unitSpecIndex < finalUnitSpecificationList.Count; ++unitSpecIndex)
    //    {
    //    UnitSpecification unitSpecification = finalUnitSpecificationList[unitSpecIndex];

    //    IECValueContainer unitSpec = (IECValueContainer)unitSpecArray[unitSpecIndex];
    //    UnitsSchemaCreator.BuildCustomAttributeFromUnitSpecification (unitSpecification, ref unitSpec);
    //    }
    //SetCustomAttribute (consolidatedCustomAttributeContainer, unitSpecList);

    return SUPPLEMENTED_SCHEMA_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfo::SupplementalSchemaInfo
(
WStringCR primarySchemaFullName, 
SchemaNamePurposeMap& schemaFullNameToPurposeMapping
) : m_primarySchemaFullName(primarySchemaFullName)
    {
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = schemaFullNameToPurposeMapping.begin(); iter != schemaFullNameToPurposeMapping.end(); iter++)
        {
        bpair<WString, WString>const& entry = *iter;
        m_supplementalSchemaNamesAndPurpose[WString(entry.first)] = WString(entry.second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfoPtr SupplementalSchemaInfo::Create
(
WStringCR primarySchemaFullName, 
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
bvector<WString>& supplementalSchemaNames
) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECOBJECTS_STATUS_SchemaNotSupplemented;

    // make sure the list starts out empty
    supplementalSchemaNames.clear();
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<WString, WString>const& entry = *iter;
        supplementalSchemaNames.push_back(entry.first);
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCP SupplementalSchemaInfo::GetPurposeOfSupplementalSchema
(
WStringCR fullSchemaName
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
bvector<WString>& supplementalSchemaNames, 
WStringCR purpose
) const
    {
    if (m_supplementalSchemaNamesAndPurpose.size() < 1)
        return ECOBJECTS_STATUS_SchemaNotSupplemented;

    // make sure the list starts out empty
    supplementalSchemaNames.clear();
    SchemaNamePurposeMap::const_iterator iter;
    for (iter = m_supplementalSchemaNamesAndPurpose.begin(); iter != m_supplementalSchemaNamesAndPurpose.end(); iter++)
        {
        bpair<WString, WString>const& entry = *iter;
        WString storedPurpose = entry.second;
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
WStringCR purpose
) const
    {
    bvector<WString> supplementalSchemas;
    bvector<WString> secondSupplementalSchemas;
    GetSupplementalSchemasWithPurpose(supplementalSchemas, purpose);
    SupplementalSchemaInfoPtr schemaInfo = secondSchema.GetSupplementalInfo();
    if (!schemaInfo.IsValid())
        return false;
    schemaInfo->GetSupplementalSchemasWithPurpose(secondSupplementalSchemas, purpose);

    if (supplementalSchemas.size() == 0 && secondSupplementalSchemas.size() == 0)
        return true;

    if (supplementalSchemas.size() != secondSupplementalSchemas.size())
        return false;

    bvector<WString>::const_iterator namesInSchema;
    for (namesInSchema = supplementalSchemas.begin(); namesInSchema != supplementalSchemas.end(); ++namesInSchema)
        {
        WString name = *namesInSchema;
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
END_BENTLEY_ECOBJECT_NAMESPACE

