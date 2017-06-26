/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define BSCA_SCHEMA_NAME "Bentley_Standard_CustomAttributes"
#define CORECA_SCHEMA_NAME "CoreCustomAttributes"

//*********************** StandardCustomAttributesSchemaHolder *************************************

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
        bmap<Utf8String, StandaloneECEnablerPtr> m_enablers;
        static StandardCustomAttributesSchemaHolderPtr s_schemaHolder;

        StandardCustomAttributesSchemaHolder();
        ECSchemaPtr _GetSchema();
        IECInstancePtr _CreateCustomAttributeInstance(Utf8CP attribute);

    public:

        static StandardCustomAttributesSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema();
        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute);
    };

StandardCustomAttributesSchemaHolderPtr StandardCustomAttributesSchemaHolder::s_schemaHolder;

static Utf8CP s_supplementalMetaDataAccessor = "SupplementalSchemaMetaData";
static Utf8CP s_supplementalProvenanceAccessor = "SupplementalProvenance";
static const uint32_t s_bscaVersionRead = 1;
static const uint32_t s_bscaVersionMinor = 8;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCustomAttributesSchemaHolder::StandardCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(BSCA_SCHEMA_NAME, s_bscaVersionRead, s_bscaVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);

    ECClassP metaDataClass = m_schema->GetClassP(s_supplementalMetaDataAccessor);
    StandaloneECEnablerPtr enabler;
    if (nullptr != metaDataClass)
        enabler = metaDataClass->GetDefaultStandaloneEnabler();

    m_enablers.Insert(s_supplementalMetaDataAccessor, enabler);

    ECClassP provenanceClass = m_schema->GetClassP(s_supplementalProvenanceAccessor);
    StandaloneECEnablerPtr provenanceEnabler;
    if (nullptr != provenanceClass)
        provenanceEnabler = provenanceClass->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_supplementalProvenanceAccessor, provenanceEnabler);

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
IECInstancePtr StandardCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(Utf8CP attribute)
    {
    if (!m_schema.IsValid())
        _GetSchema();

    auto enablerIterator = m_enablers.find(attribute);
    if (enablerIterator == m_enablers.end())
        {
        BeDataAssert(false && "Unknown supplemental schema custom attribute class name. Currently only SupplementalSchemaMetaData and SupplementalProvenance are supported.");
        return nullptr;
        }

    StandaloneECEnablerPtr enabler = enablerIterator->second;
    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributesSchemaHolder::CreateCustomAttributeInstance(Utf8CP attribute)
    {
    return GetHolder()->_CreateCustomAttributeInstance(attribute);
    }

//*********************** StandardCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus StandardCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP StandardCustomAttributeHelper::GetCustomAttributeClass(Utf8CP attributeName)
    {
    return StandardCustomAttributesSchemaHolder::GetSchema()->GetClassCP(attributeName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    return StandardCustomAttributesSchemaHolder::CreateCustomAttributeInstance(attributeName);
    }

//*********************** CoreCustomAttributesSchemaHolder *************************************

//=======================================================================================    
//! Helper class to hold the CoreCustomAttributes schema
//! @bsiclass
//=======================================================================================    
struct CoreCustomAttributesSchemaHolder;
typedef RefCountedPtr<CoreCustomAttributesSchemaHolder> CoreCustomAttributesSchemaHolderPtr;

struct CoreCustomAttributesSchemaHolder : RefCountedBase
    {
    private:
        ECSchemaPtr            m_schema;
        bmap<Utf8String, StandaloneECEnablerPtr> m_enablers;

        static CoreCustomAttributesSchemaHolderPtr s_schemaHolder;
        CoreCustomAttributesSchemaHolder();
        ECSchemaPtr _GetSchema();
        IECInstancePtr _CreateCustomAttributeInstance(Utf8CP attribute);

    public:

        static CoreCustomAttributesSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema();
        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute);
    };

CoreCustomAttributesSchemaHolderPtr CoreCustomAttributesSchemaHolder::s_schemaHolder;

static Utf8CP s_supplementalAccessor = "SupplementalSchema";
static Utf8CP s_isMixinAccessor = "IsMixin";
static const uint32_t s_coreCAVersionRead = 1;
static const uint32_t s_coreCAVersionWrite = 0;
static const uint32_t s_coreCAVersionMinor = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CoreCustomAttributesSchemaHolder::CoreCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(CORECA_SCHEMA_NAME, s_coreCAVersionRead, s_coreCAVersionWrite, s_coreCAVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);

    ECClassP metaDataClass = m_schema->GetClassP(s_supplementalAccessor);
    StandaloneECEnablerPtr enabler;
    if (nullptr != metaDataClass)
        enabler = metaDataClass->GetDefaultStandaloneEnabler();

    m_enablers.Insert(s_supplementalAccessor, enabler);

    ECClassP provenanceClass = m_schema->GetClassP(s_supplementalProvenanceAccessor);
    StandaloneECEnablerPtr provenanceEnabler;
    if (nullptr != provenanceClass)
        provenanceEnabler = provenanceClass->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_supplementalProvenanceAccessor, provenanceEnabler);

    ECClassP mixinClass = m_schema->GetClassP(s_isMixinAccessor);
    StandaloneECEnablerPtr mixinEnabler;
    if (nullptr != mixinClass)
        mixinEnabler = mixinClass->GetDefaultStandaloneEnabler();
    m_enablers.Insert(s_isMixinAccessor, mixinEnabler);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CoreCustomAttributesSchemaHolderPtr CoreCustomAttributesSchemaHolder::GetHolder()
    {
    if (s_schemaHolder.IsNull())
        s_schemaHolder = new CoreCustomAttributesSchemaHolder();

    return s_schemaHolder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr CoreCustomAttributesSchemaHolder::_GetSchema()
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr CoreCustomAttributesSchemaHolder::GetSchema()
    {
    return GetHolder()->_GetSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CoreCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(Utf8CP attribute)
    {
    if (!m_schema.IsValid())
        _GetSchema();

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
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CoreCustomAttributesSchemaHolder::CreateCustomAttributeInstance(Utf8CP attribute)
    {
    return GetHolder()->_CreateCustomAttributeInstance(attribute);
    }

//*********************** CoreCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus CoreCustomAttributeHelper::GetDateTimeInfo(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECCustomAttributeClassCP CoreCustomAttributeHelper::GetCustomAttributeClass(Utf8CP attributeName)
    {
    ECClassCP ecClass = CoreCustomAttributesSchemaHolder::GetSchema()->GetClassCP(attributeName);
    if (nullptr == ecClass)
        return nullptr;
    return ecClass->GetCustomAttributeClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECClassCP CoreCustomAttributeHelper::GetClass(Utf8CP attributeName)
    {
    ECClassCP ecClass = CoreCustomAttributesSchemaHolder::GetSchema()->GetClassCP(attributeName);
    if (nullptr == ecClass)
        return nullptr;
    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CoreCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    return CoreCustomAttributesSchemaHolder::CreateCustomAttributeInstance(attributeName);
    }

//*********************** ConversionCustomAttributesSchemaHolder *************************************

//=======================================================================================    
//! Helper class to hold the ECv3ConversionAttributes schema
//! The primary use-case is to facilitate adding the PropertyRenamed CA to an ECProperty 
//! for Instance transformation.
//! @bsiclass
//=======================================================================================    

struct ConversionCustomAttributesSchemaHolder;
typedef RefCountedPtr<ConversionCustomAttributesSchemaHolder> ConversionCustomAttributesSchemaHolderPtr;

static Utf8CP s_convSchemaName = "ECv3ConversionAttributes";
static Utf8CP s_renamedAccessor = "RenamedPropertiesMapping";
static Utf8CP s_oldUnitAccessor = "OldPersistenceUnit";
static const uint32_t s_convVersionRead = 1;
static const uint32_t s_convVersionMinor = 0;

struct ConversionCustomAttributesSchemaHolder : RefCountedBase
    {
    private:
        ECSchemaPtr            m_schema;
        bmap<Utf8String, StandaloneECEnablerPtr> m_enablers;

        static ConversionCustomAttributesSchemaHolderPtr s_schemaHolder;

        ConversionCustomAttributesSchemaHolder();
        ECSchemaPtr _GetSchema() {return m_schema;}
        IECInstancePtr _CreateCustomAttributeInstance(Utf8CP attribute);

    public:
        static ConversionCustomAttributesSchemaHolderPtr GetHolder();
        static ECSchemaPtr GetSchema() {return GetHolder()->_GetSchema();}
        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute) {return GetHolder()->_CreateCustomAttributeInstance(attribute);}
    };

ConversionCustomAttributesSchemaHolderPtr ConversionCustomAttributesSchemaHolder::s_schemaHolder;

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
ConversionCustomAttributesSchemaHolder::ConversionCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(s_convSchemaName, s_convVersionRead, s_convVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);

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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
ConversionCustomAttributesSchemaHolderPtr ConversionCustomAttributesSchemaHolder::GetHolder()
    {
    if (s_schemaHolder.IsNull())
        s_schemaHolder = new ConversionCustomAttributesSchemaHolder();

    return s_schemaHolder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr ConversionCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(Utf8CP attribute)
    {
    if (!m_schema.IsValid())
        _GetSchema();

    auto enablerIterator = m_enablers.find(attribute);
    if (enablerIterator == m_enablers.end())
        {
        BeDataAssert(false && "Unknown custom attribute class name. Currently only RenamedPropertiesMapping is supported.");
        return nullptr;
        }

    StandaloneECEnablerPtr enabler = enablerIterator->second;
    if (!enabler.IsValid())
        return nullptr;

    return enabler->CreateInstance().get();
    }

//*********************** ConversionCustomAttributeHelper *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr ConversionCustomAttributeHelper::CreateCustomAttributeInstance(Utf8CP attributeName)
    {
    return ConversionCustomAttributesSchemaHolder::CreateCustomAttributeInstance(attributeName);
    }



END_BENTLEY_ECOBJECT_NAMESPACE
