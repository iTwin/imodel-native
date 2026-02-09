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
// static
ECSchemaPtr StandardCustomAttributeHelper::_GetSchema(ECSchemaReadContextR schemaContext) {
    SchemaKey key(kBentleyStandardCustomAttributes, kBscaVersionRead, kBscaVersionMinor);
    ECSchemaPtr schema = ECSchema::LocateSchema(key, schemaContext);
    return schema;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus StandardCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty) {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP StandardCustomAttributeHelper::GetCustomAttributeClass(ECSchemaReadContextR schemaContext, Utf8CP attributeName) {
    ECSchemaPtr schema = _GetSchema(schemaContext);
    return schema->GetClassCP(attributeName);
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributeHelper::CreateCustomAttributeInstance(ECSchemaReadContextR schemaContext, Utf8CP attributeName) {
    ECSchemaPtr schema = _GetSchema(schemaContext);
    if (!schema.IsValid()) {
        LOG.errorv("Cannot load standard schema '%s'", kBentleyStandardCustomAttributes);
        return nullptr;
    }

    if (0 != strcmp(attributeName, kSupplementalMetaDataAccessor) &&
        0 != strcmp(attributeName, kSupplementalProvenanceAccessor)) {
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

const Utf8CP kCoreCustomAttributes = "CoreCustomAttributes";
const Utf8CP kSupplementalAccessor = "SupplementalSchema";
const Utf8CP kIsMixinAccessor = "IsMixin";
const Utf8CP kDynamicSchema = "DynamicSchema";

const uint32_t kCoreCAVersionRead = 1u;
const uint32_t kCoreCAVersionWrite = 0u;
const uint32_t kCoreCAVersionMinor = 0u;

//*********************** CoreCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSchemaPtr CoreCustomAttributeHelper::_GetSchema(ECSchemaReadContextR schemaContext) {
    SchemaKey key(kCoreCustomAttributes, kCoreCAVersionRead, kCoreCAVersionWrite, kCoreCAVersionMinor);
    ECSchemaPtr schema = ECSchema::LocateSchema(key, schemaContext);
    return schema;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus CoreCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty) {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
BentleyStatus CoreCustomAttributeHelper::GetCurrentTimeStampProperty(PrimitiveECPropertyCP& currentTimeStampProp, ECClassCR ecClass) {
    currentTimeStampProp = nullptr;
    IECInstancePtr ca = ecClass.GetCustomAttributeLocal(kCoreCustomAttributes, "ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return SUCCESS;

    ECValue v;
    if (ECObjectsStatus::Success != ca->GetValue(v, "PropertyName")) {
        LOG.errorv("Failed to retrieve 'ClassHasCurrentTimeStampProperty' custom attribute on ECClass '%s'. It is malformed.",
                   ecClass.GetFullName());
        return ERROR;
    }

    if (!v.IsNull() && v.IsString()) {
        ECPropertyCP foundProp = nullptr;
        if (v.IsUtf8())
            foundProp = ecClass.GetPropertyP(v.GetUtf8CP(), true);
        else
            foundProp = ecClass.GetPropertyP(v.GetWCharCP(), true);

        if (foundProp != nullptr) {
            PrimitiveECPropertyCP foundPrimProp = foundProp->GetAsPrimitiveProperty();
            if (foundPrimProp != nullptr && foundPrimProp->GetType() == PRIMITIVETYPE_DateTime) {
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
// static
ECCustomAttributeClassCP CoreCustomAttributeHelper::GetCustomAttributeClass(ECSchemaReadContextR schemaContext, Utf8CP attributeName) {
    ECSchemaPtr coreCaSchema = GetSchema(schemaContext);
    if (!coreCaSchema.IsValid()) {
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
// static
ECClassCP CoreCustomAttributeHelper::GetClass(ECSchemaReadContextR schemaContext, Utf8CP attributeName) {
    ECSchemaPtr coreCaSchema = GetSchema(schemaContext);
    if (!coreCaSchema.IsValid()) {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
    }

    return coreCaSchema->GetClassCP(attributeName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSchemaPtr CoreCustomAttributeHelper::GetSchema(ECSchemaReadContextR schemaContext) {
    return _GetSchema(schemaContext);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
IECInstancePtr CoreCustomAttributeHelper::CreateCustomAttributeInstance(ECSchemaReadContextR schemaContext, Utf8CP attributeName) {
    ECSchemaPtr coreCaSchema = GetSchema(schemaContext);
    if (!coreCaSchema.IsValid()) {
        LOG.errorv("Cannot load standard schema '%s'", kCoreCustomAttributes);
        return nullptr;
    }

    if (0 != strcmp(attributeName, kSupplementalAccessor) &&
        0 != strcmp(attributeName, kSupplementalProvenanceAccessor) &&
        0 != strcmp(attributeName, kIsMixinAccessor) &&
        0 != strcmp(attributeName, kDynamicSchema)) {
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

//*********************** ConversionCustomAttributesSchemaHolder *************************************

//=======================================================================================
//! Helper class to hold the ECv3ConversionAttributes schema
//! The primary use-case is to facilitate adding the PropertyRenamed CA to an ECProperty
//! for Instance transformation.
//! @bsiclass
//=======================================================================================
struct ConversionCustomAttributesSchemaHolder {
    const Utf8CP s_convSchemaName = "ECv3ConversionAttributes";
    const Utf8CP s_renamedAccessor = "RenamedPropertiesMapping";
    const Utf8CP s_oldUnitAccessor = "OldPersistenceUnit";
    const Utf8CP s_oldDerivedClasses = "OldDerivedClasses";
    const Utf8CP s_isFlattened = "IsFlattened";

    const uint32_t s_convVersionRead = 1;
    const uint32_t s_convVersionMinor = 0;

   private:
    ECSchemaPtr m_schema;
    bmap<Utf8String, StandaloneECEnablerPtr> m_enablers;

    ConversionCustomAttributesSchemaHolder();
    ECSchemaPtr _GetSchema() { return m_schema; }
    IECInstancePtr _CreateCustomAttributeInstance(Utf8CP attribute);
    bool Initialize();

   public:
    static ConversionCustomAttributesSchemaHolder* GetHolder();
    static bool HasHolder() { return true; }
    static ECSchemaPtr GetSchema() { return GetHolder()->_GetSchema(); }
    static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute) { return GetHolder()->_CreateCustomAttributeInstance(attribute); }
    void Reset();
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ConversionCustomAttributesSchemaHolder::ConversionCustomAttributesSchemaHolder() {
    Initialize();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ConversionCustomAttributesSchemaHolder::Initialize() {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetCalculateChecksum(true);
    SchemaKey key(s_convSchemaName, s_convVersionRead, s_convVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);
    if (!m_schema.IsValid()) {
        LOG.errorv("Could not load the standard schema '%s'", s_convSchemaName);
        return false;
    }

    ECClassP metaDataClass = m_schema->GetClassP(s_renamedAccessor);
    StandaloneECEnablerPtr enabler;
    if (nullptr != metaDataClass)
        enabler = metaDataClass->GetDefaultStandaloneEnabler();

    m_enablers.Insert(s_renamedAccessor, enabler);

    ECClassP oldUnitClass = m_schema->GetClassP(s_oldUnitAccessor);
    StandaloneECEnablerPtr oldUnitEnabler;
    if (nullptr != oldUnitClass)
        oldUnitEnabler = oldUnitClass->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_oldUnitAccessor, oldUnitEnabler);

    ECClassP oldDerivedClass = m_schema->GetClassP(s_oldDerivedClasses);
    StandaloneECEnablerPtr oldDerivedEnabler;
    if (nullptr != oldDerivedClass)
        oldDerivedEnabler = oldDerivedClass->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_oldDerivedClasses, oldDerivedEnabler);

    ECClassP isFlattened = m_schema->GetClassP(s_isFlattened);
    StandaloneECEnablerPtr isFlattenedEnabler;
    if (nullptr != isFlattened)
        isFlattenedEnabler = isFlattened->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_isFlattened, isFlattenedEnabler);
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ConversionCustomAttributesSchemaHolder::Reset() {
    m_schema = nullptr;
    m_enablers.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ConversionCustomAttributesSchemaHolder* ConversionCustomAttributesSchemaHolder::GetHolder() {
    static auto s_schemaHolder = new ConversionCustomAttributesSchemaHolder();
    return s_schemaHolder;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr ConversionCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(Utf8CP attribute) {
    if (!m_schema.IsValid() && !Initialize()) {
        LOG.errorv("Could not load standard schema '%s'", s_convSchemaName);
        return nullptr;
    }

    auto enablerIterator = m_enablers.find(attribute);
    if (enablerIterator == m_enablers.end()) {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only RenamedPropertiesMapping is supported.");
        LOG.errorv("Could not find an enabler for Custom Attribute class '%s'", attribute);
        return nullptr;
    }

    StandaloneECEnablerPtr enabler = enablerIterator->second;
    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
}

//*********************** ConversionCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
IECInstancePtr ConversionCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName) {
    return ConversionCustomAttributesSchemaHolder::CreateCustomAttributeInstance(attributeName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ConversionCustomAttributeHelper::Reset() {
    ConversionCustomAttributesSchemaHolder::GetHolder()->Reset();
}

END_BENTLEY_ECOBJECT_NAMESPACE
