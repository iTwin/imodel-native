/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

const Utf8CP kBentleyStandardCustomAttributes = "Bentley_Standard_CustomAttributes";
const Utf8CP kSupplementalMetaDataAccessor = "SupplementalSchemaMetaData";
const Utf8CP kSupplementalProvenanceAccessor = "SupplementalProvenance";
const uint32_t kBscaVersionRead = 1u;
const uint32_t kBscaVersionMinor = 8u;

//*********************** StandardCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr StandardCustomAttributeHelper::_GetSchema(ECSchemaReadContextPtr schemaContext)
    {
    SchemaKey key(kBentleyStandardCustomAttributes, kBscaVersionRead, kBscaVersionMinor);
    ECSchemaPtr schema = ECSchema::LocateSchema(key, *schemaContext);
    return schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus StandardCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP StandardCustomAttributeHelper::GetCustomAttributeClass(ECSchemaReadContextPtr schemaContext, Utf8CP attributeName)
    {
    ECSchemaPtr schema = _GetSchema(schemaContext);
    return schema->GetClassCP(attributeName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributeHelper::CreateCustomAttributeInstance(ECSchemaReadContextPtr schemaContext, Utf8CP attributeName)
    {
    ECSchemaPtr schema = _GetSchema(schemaContext);
    if (!schema.IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kBentleyStandardCustomAttributes);
        return nullptr;
        }

    if (0 != strcmp(attributeName, kSupplementalMetaDataAccessor) &&
        0 != strcmp(attributeName, kSupplementalProvenanceAccessor))
        {
        BeDataAssert(false && "Unknown supplemental schema custom attribute class name. Currently only SupplementalSchemaMetaData and SupplementalProvenance are supported.");
        return nullptr;
        }

    ECClassP ecClass = schema->GetClassP(attributeName);
    StandaloneECEnablerPtr enabler;
    if (nullptr != ecClass)
        enabler = ecClass->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

static  Utf8CP kCoreCustomAttributes = "CoreCustomAttributes";
//*********************** CoreCustomAttributesSchemaHolder *************************************

//=======================================================================================
//! Helper class to hold the CoreCustomAttributes schema
//! @bsiclass
//=======================================================================================
struct CoreCustomAttributesSchemaHolder
    {

    const Utf8CP kSupplementalAccessor = "SupplementalSchema";
    const Utf8CP kSupplementalProvenanceAccessor = "SupplementalProvenance";
    const Utf8CP kIsMixinAccessor = "IsMixin";
    const Utf8CP kDynamicSchema = "DynamicSchema";
    const uint32_t kCoreCAVersionRead = 1u;
    const uint32_t kCoreCAVersionWrite = 0u;
    const uint32_t kCoreCAVersionMinor = 0u;   

    private:
        ECSchemaPtr m_schema;
        bmap<Utf8String, StandaloneECEnablerPtr> m_enablers;

        CoreCustomAttributesSchemaHolder();
        ECSchemaPtr _GetSchema();
        IECInstancePtr _CreateCustomAttributeInstance(Utf8CP attribute);

    public:

        static CoreCustomAttributesSchemaHolder* GetHolder();
        static ECSchemaPtr GetSchema();
        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CoreCustomAttributesSchemaHolder::CoreCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(kCoreCustomAttributes, kCoreCAVersionRead, kCoreCAVersionWrite, kCoreCAVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);

    if (!m_schema.IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        }
    else
        {
        ECClassP metaDataClass = m_schema->GetClassP(kSupplementalAccessor);
        StandaloneECEnablerPtr enabler;
        if (nullptr != metaDataClass)
            enabler = metaDataClass->GetDefaultStandaloneEnabler();

        m_enablers.Insert(kSupplementalAccessor, enabler);

        ECClassP provenanceClass = m_schema->GetClassP(kSupplementalProvenanceAccessor);
        StandaloneECEnablerPtr provenanceEnabler;
        if (nullptr != provenanceClass)
            provenanceEnabler = provenanceClass->GetDefaultStandaloneEnabler();
        m_enablers.Insert(kSupplementalProvenanceAccessor, provenanceEnabler);

        ECClassP mixinClass = m_schema->GetClassP(kIsMixinAccessor);
        StandaloneECEnablerPtr mixinEnabler;
        if (nullptr != mixinClass)
            mixinEnabler = mixinClass->GetDefaultStandaloneEnabler();
        m_enablers.Insert(kIsMixinAccessor, mixinEnabler);

        ECClassP dynamicClass = m_schema->GetClassP(kDynamicSchema);
        if (nullptr != dynamicClass)
            {
            StandaloneECEnablerPtr dynamicEnabler;
            dynamicEnabler = dynamicClass->GetDefaultStandaloneEnabler();
            m_enablers.Insert(kDynamicSchema, dynamicEnabler);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CoreCustomAttributesSchemaHolder* CoreCustomAttributesSchemaHolder::GetHolder()
    {
    static auto s_schemaHolder = new CoreCustomAttributesSchemaHolder();
    return s_schemaHolder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr CoreCustomAttributesSchemaHolder::_GetSchema()
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr CoreCustomAttributesSchemaHolder::GetSchema()
    {
    return GetHolder()->_GetSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CoreCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(Utf8CP attribute)
    {
    if (!_GetSchema().IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
        }

    auto enablerIterator = m_enablers.find(attribute);
    if (enablerIterator == m_enablers.end())
        {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only SupplementalSchemaMetaData, SupplementalProvenance, and IsMixin are supported.");
        return nullptr;
        }

    StandaloneECEnablerPtr enabler = enablerIterator->second;
    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CoreCustomAttributesSchemaHolder::CreateCustomAttributeInstance(Utf8CP attribute)
    {
    return GetHolder()->_CreateCustomAttributeInstance(attribute);
    }

//*********************** CoreCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus CoreCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus CoreCustomAttributeHelper::GetCurrentTimeStampProperty(PrimitiveECPropertyCP& currentTimeStampProp, ECClassCR ecClass)
    {
    currentTimeStampProp = nullptr;
    IECInstancePtr ca = ecClass.GetCustomAttributeLocal(kCoreCustomAttributes, "ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return SUCCESS;

    ECValue v;
    if (ECObjectsStatus::Success != ca->GetValue(v, "PropertyName"))
        {
        LOG.errorv("Failed to retrieve 'ClassHasCurrentTimeStampProperty' custom attribute on ECClass '%s'. It is malformed.",
                        ecClass.GetFullName());
        return ERROR;
        }

    if (!v.IsNull() && v.IsString())
        {
        ECPropertyCP foundProp = nullptr;
        if (v.IsUtf8())
            foundProp = ecClass.GetPropertyP(v.GetUtf8CP(), true);
        else
            foundProp = ecClass.GetPropertyP(v.GetWCharCP(), true);

        if (foundProp != nullptr)
            {
            PrimitiveECPropertyCP foundPrimProp = foundProp->GetAsPrimitiveProperty();
            if (foundPrimProp != nullptr && foundPrimProp->GetType() == PRIMITIVETYPE_DateTime)
                {
                currentTimeStampProp = foundPrimProp;
                return SUCCESS;
                }
            }
        }

    LOG.errorv("Failed to retrieve 'ClassHasCurrentTimeStampProperty' custom attribute on ECClass '%s'. The property 'PropertyName' must be set to the name of a primitive property of type DateTime.",
               ecClass.GetFullName());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECCustomAttributeClassCP CoreCustomAttributeHelper::GetCustomAttributeClass(Utf8CP attributeName)
    {
    ECSchemaPtr coreCaSchema = CoreCustomAttributesSchemaHolder::GetSchema();
    if (!coreCaSchema.IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
        }

    ECClassCP ecClass = coreCaSchema->GetClassCP(attributeName);
    if (nullptr == ecClass)
        return nullptr;
    return ecClass->GetCustomAttributeClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECClassCP CoreCustomAttributeHelper::GetClass(Utf8CP attributeName)
    {
    ECSchemaPtr coreCaSchema = CoreCustomAttributesSchemaHolder::GetSchema();
    if (!coreCaSchema.IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
        }

    return coreCaSchema->GetClassCP(attributeName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr CoreCustomAttributeHelper::GetSchema()
    {
    return CoreCustomAttributesSchemaHolder::GetSchema();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CoreCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    return CoreCustomAttributesSchemaHolder::CreateCustomAttributeInstance(attributeName);
    }

const Utf8CP s_convSchemaName = "ECv3ConversionAttributes";
const Utf8CP s_renamedAccessor = "RenamedPropertiesMapping";
const Utf8CP s_oldUnitAccessor = "OldPersistenceUnit";
const Utf8CP s_oldDerivedClasses = "OldDerivedClasses";
const Utf8CP s_isFlattened = "IsFlattened";

const uint32_t s_convVersionRead = 1;
const uint32_t s_convVersionMinor = 0;

ECSchemaPtr ConversionCustomAttributeHelper::m_schema = nullptr;
//*********************** ConversionCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
//static
bool ConversionCustomAttributeHelper::Initialize()
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetCalculateChecksum(true);
    SchemaKey key(s_convSchemaName, s_convVersionRead, s_convVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);
    if (!m_schema.IsValid())
        {
        LOG.errorv("Could not load the standard schema '%s'", s_convSchemaName);
        return false;
    }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr ConversionCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    if (!m_schema.IsValid() && !Initialize())
        {
        LOG.errorv("Could not load standard schema '%s'", s_convSchemaName);
        return nullptr;
        }

    if (0 != strcmp(attributeName, s_renamedAccessor) &&
        0 != strcmp(attributeName, s_oldUnitAccessor) &&
        0 != strcmp(attributeName, s_oldDerivedClasses) &&
        0 != strcmp(attributeName, s_isFlattened))
        {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only RenamedPropertiesMapping, OldPersistenceUnit, OldDerivedClasses, and IsFlattened are supported.");
        return nullptr;
        }

    ECClassP ecClass = m_schema->GetClassP(attributeName);
    StandaloneECEnablerPtr enabler;
    if (nullptr != ecClass)
        enabler = ecClass->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
//static
void ConversionCustomAttributeHelper::Reset()
    {
    m_schema = nullptr;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
