/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SupplementalSchema.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

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
    ECSchemaCachePtr    m_schemaOwner;
    ECSchemaP           m_schema;
    ECClassP            m_supplementalSchemaMetaDataClass;
    StandaloneECEnablerPtr m_enabler;

    static StandardCustomAttributesSchemaHolderPtr s_schemaHolder;

    StandardCustomAttributesSchemaHolder();

    ECSchemaP _GetSchema();

    IECInstancePtr _CreateSupplementalSchemaMetaDataInstance();

public:

/*__PUBLISH_SECTION_START__*/
    static StandardCustomAttributesSchemaHolderPtr GetHolder();

    static ECSchemaP GetSchema();

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
    m_schemaOwner = ECSchemaCache::Create();

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext(*m_schemaOwner);
    schemaContext->HideSchemasFromLeakDetection();
    m_schema = ECSchema::LocateSchema(L"Bentley_Standard_CustomAttributes", s_bscaVersionMajor, s_bscaVersionMinor, *schemaContext);
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
ECSchemaP StandardCustomAttributesSchemaHolder::_GetSchema()
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP StandardCustomAttributesSchemaHolder::GetSchema()
    {
    return GetHolder()->_GetSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributesSchemaHolder::_CreateSupplementalSchemaMetaDataInstance()
    {
    if (NULL == m_schema)
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
    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaNamePropertyAccessor()))
        m_primarySchemaName = propertyValue.GetString();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMajorVersionPropertyAccessor()))
        m_primarySchemaMajorVersion = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrimarySchemaMinorVersionPropertyAccessor()))
        m_primarySchemaMinorVersion = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPrecedencePropertyAccessor()))
        m_supplementalSchemaPrecedence = propertyValue.GetInteger();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetPurposePropertyAccessor()))
        m_supplementalSchemaPurpose = propertyValue.GetString();

    if (SUCCESS == supplementalSchemaMetaDataCustomAttribute.GetValue(propertyValue, GetIsUserSpecificPropertyAccessor()))
        m_isUserSpecific = propertyValue.GetBoolean();
    }

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
WStringCR primarySchemaName, 
UInt32 primarySchemaMajorVersion, 
UInt32 primarySchemaMinorVersion, 
SchemaMatchType matchType
) const
    {
    return ECSchema::SchemasMatch(matchType, primarySchemaName.c_str(), primarySchemaMajorVersion, primarySchemaMinorVersion,
        m_primarySchemaName.c_str(), m_primarySchemaMajorVersion, m_primarySchemaMinorVersion);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementedSchemaStatus SupplementedSchemaBuilder::UpdateSchema
(
ECSchemaR primarySchema, 
const bvector<ECSchemaR>& supplementalSchemaList
)
    {
    bmap<UInt32, ECSchemaP> schemasByPrecedence;
    bvector<ECSchemaP> localizationSchemas;
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;

    OrderSupplementalSchemas(primarySchema, supplementalSchemaList, schemasByPrecedence, localizationSchemas);

    return status;
    }

SupplementedSchemaStatus SupplementedSchemaBuilder::OrderSupplementalSchemas
(
ECSchemaR primarySchema, 
const bvector<ECSchemaR>& supplementalSchemaList, 
bmap<UInt32, ECSchemaP> schemasByPrecedence, 
bvector<ECSchemaP> localizationSchemas 
)
    {
    SupplementedSchemaStatus status = SUPPLEMENTED_SCHEMA_STATUS_Success;

    return status;
    }

END_BENTLEY_EC_NAMESPACE

