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

static Utf8CP kCoreCustomAttributes = "CoreCustomAttributes";
const Utf8CP kSupplementalAccessor = "SupplementalSchema";
const Utf8CP kIsMixinAccessor = "IsMixin";
const Utf8CP kDynamicSchema = "DynamicSchema";

const uint32_t kCoreCAVersionRead = 1u;
const uint32_t kCoreCAVersionWrite = 0u;
const uint32_t kCoreCAVersionMinor = 0u;

ECSchemaPtr CoreCustomAttributeHelper::m_schema = nullptr;
//*********************** CoreCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr CoreCustomAttributeHelper::_GetSchema()
    {
    if (m_schema == nullptr)
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        SchemaKey key(kCoreCustomAttributes, kCoreCAVersionRead, kCoreCAVersionWrite, kCoreCAVersionMinor);
        m_schema = ECSchema::LocateSchema(key, *schemaContext);
        }
    return m_schema;
    }

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
    ECSchemaPtr coreCaSchema = GetSchema();
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
    ECSchemaPtr coreCaSchema = GetSchema();
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
    return _GetSchema();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CoreCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    ECSchemaPtr coreCaSchema = GetSchema();
    if (!coreCaSchema.IsValid())
        {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
        }

    if (0 != strcmp(attributeName, kSupplementalAccessor) &&
        0 != strcmp(attributeName, kSupplementalProvenanceAccessor) &&
        0 != strcmp(attributeName, kIsMixinAccessor) &&
        0 != strcmp(attributeName, kDynamicSchema))
        {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only SupplementalSchemaMetaData, SupplementalProvenance, IsMixin, and DynamicSchema are supported.");
        return nullptr;
        }

    ECClassP ecClass = coreCaSchema->GetClassP(attributeName);
    StandaloneECEnablerPtr enabler;
    if (nullptr != ecClass)
        enabler = ecClass->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

const Utf8CP kConvSchemaName = "ECv3ConversionAttributes";
const Utf8CP kRenamedAccessor = "RenamedPropertiesMapping";
const Utf8CP kOldUnitAccessor = "OldPersistenceUnit";
const Utf8CP kOldDerivedClasses = "OldDerivedClasses";
const Utf8CP kIsFlattened = "IsFlattened";

const uint32_t kConvVersionRead = 1;
const uint32_t kConvVersionMinor = 0;

ECSchemaPtr ConversionCustomAttributeHelper::m_schema = nullptr;
//*********************** ConversionCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr ConversionCustomAttributeHelper::_GetSchema()
    {
    if (m_schema == nullptr)
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->SetCalculateChecksum(true);
        SchemaKey key(kConvSchemaName, kConvVersionRead, kConvVersionMinor);
        m_schema = ECSchema::LocateSchema(key, *schemaContext);
        }
    return m_schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr ConversionCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    ECSchemaPtr schema = _GetSchema();
    if (!schema.IsValid())
        {
        LOG.errorv("Could not load standard schema '%s'", kConvSchemaName);
        return nullptr;
        }

    if (0 != strcmp(attributeName, kRenamedAccessor) &&
        0 != strcmp(attributeName, kOldUnitAccessor) &&
        0 != strcmp(attributeName, kOldDerivedClasses) &&
        0 != strcmp(attributeName, kIsFlattened))
        {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only RenamedPropertiesMapping, OldPersistenceUnit, OldDerivedClasses, and IsFlattened are supported.");
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
//static
void ConversionCustomAttributeHelper::Reset()
    {
    m_schema = nullptr;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
