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
#define ECDBMAP_SCHEMA_NAME "ECDbMap"

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

        /*__PUBLISH_SECTION_START__*/
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

        /*__PUBLISH_SECTION_START__*/
        static CoreCustomAttributesSchemaHolderPtr GetHolder();

        static ECSchemaPtr GetSchema();

        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attribute);
    };

CoreCustomAttributesSchemaHolderPtr CoreCustomAttributesSchemaHolder::s_schemaHolder;

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
static Utf8CP s_renamedAccessor = "PropertyRenamed";
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
        /*__PUBLISH_SECTION_START__*/
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
        BeDataAssert(false && "Unknown custom attribute class name. Currently only PropertyRenamed is supported.");
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

//*****************************************************************
//ECDbMapHelper
//*****************************************************************
struct CustomAttributeReader
    {
    private:
        CustomAttributeReader();
        ~CustomAttributeReader();

        static ECObjectsStatus TryGetTrimmedValue(Utf8StringR strVal, ECValueCR val);

    public:
        static IECInstanceCP Read(IECCustomAttributeContainer const& caContainer, Utf8CP customAttributeSchemaName, Utf8CP customAttributeName);

        static ECObjectsStatus TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static ECObjectsStatus TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex);
        static ECObjectsStatus TryGetIntegerValue(int& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static ECObjectsStatus TryGetBooleanValue(bool& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
    };

//*****************************************************************
//ECDbMapCustomAttributeHelper
//*****************************************************************


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetSchemaMap(ECDbSchemaMap& schemaMap, ECSchemaCR schema)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(schema, ECDBMAP_SCHEMA_NAME, "SchemaMap");
    if (ca == nullptr)
        return false;

    schemaMap = ECDbSchemaMap(schema, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetClassMap(ECDbClassMap& classMap, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMA_NAME, "ClassMap");
    if (ca == nullptr)
        return false;

    classMap = ECDbClassMap(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetShareColumns(ShareColumns& shareColumns, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMA_NAME, "ShareColumns");
    if (ca == nullptr)
        return false;

    shareColumns = ShareColumns(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(ECEntityClassCR ecClass)
    {
    return ecClass.GetCustomAttributeLocal(ECDBMAP_SCHEMA_NAME, "JoinedTablePerDirectSubclass") != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetDbIndexList(DbIndexList& dbIndexList, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMA_NAME, "DbIndexList");
    if (ca == nullptr)
        return false;

    dbIndexList = DbIndexList(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetPropertyMap(ECDbPropertyMap& propertyMap, PrimitiveECPropertyCR ecProperty)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecProperty, ECDBMAP_SCHEMA_NAME, "PropertyMap");
    if (ca == nullptr)
        return false;

    propertyMap = ECDbPropertyMap(ecProperty, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMA_NAME, "LinkTableRelationshipMap");
    if (ca == nullptr)
        return false;

    linkTableRelationshipMap = ECDbLinkTableRelationshipMap(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(ECDbForeignKeyConstraint& foreignKeyTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMA_NAME, "ForeignKeyConstraint");
    if (ca == nullptr)
        return false;

    foreignKeyTableRelationshipMap = ECDbForeignKeyConstraint(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::HasUseECInstanceIdAsForeignKey(ECRelationshipClassCR relClass)
    {
    return relClass.GetCustomAttributeLocal(ECDBMAP_SCHEMA_NAME, "UseECInstanceIdAsForeignKey") != nullptr;
    }

//*****************************************************************
//ECDbSchemaMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaMap::ECDbSchemaMap(ECSchemaCR schema, IECInstanceCP ca) : m_schema(&schema), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbSchemaMap::TryGetTablePrefix(Utf8String& tablePrefix) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    ECObjectsStatus stat = CustomAttributeReader::TryGetTrimmedValue(tablePrefix, *m_ca, "TablePrefix");
    if (ECObjectsStatus::Success != stat)
        return stat;

    if (tablePrefix.empty() || ECNameValidation::Validate(tablePrefix.c_str()) == ECNameValidation::RESULT_Valid)
        return ECObjectsStatus::Success;

    LOG.errorv("Custom attribute '%s' on the ECSchema '%s' has an invalid value for the property 'TablePrefix': %s. "
               "The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.",
               m_ca->GetClass().GetName().c_str(),
               m_schema->GetName().c_str(), tablePrefix.c_str());
    return ECObjectsStatus::Error;
    }


//*****************************************************************
//ECDbClassMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbClassMap::ECDbClassMap(ECClassCR ecClass, IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetMapStrategy(Utf8String& mapStrategy) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(mapStrategy, *m_ca, "MapStrategy");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetTableName(Utf8String& tableName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(tableName, *m_ca, "TableName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, *m_ca, "ECInstanceIdColumn");
    }

//*****************************************************************
//DbIndexList
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
DbIndexList::DbIndexList(ECClassCR ecClass, IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus DbIndexList::GetIndexes(bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    uint32_t propIx;
    ECObjectsStatus stat = m_ca->GetEnablerR().GetPropertyIndex(propIx, "Indexes");
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Failed to get property index for property 'List' of custom attribute '%s' on ECClass '%s'.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return stat;
        }

    ECValue indexesVal;
    stat = m_ca->GetValue(indexesVal, propIx);
    if (ECObjectsStatus::Success != stat)
        return stat;

    indices.clear();
    const uint32_t indexCount = indexesVal.IsNull() ? 0 : indexesVal.GetArrayInfo().GetCount();
    if (indexCount == 0)
        {
        LOG.errorv("Failed to read %s custom attribute' on ECClass '%s'. Its property 'Indexes' must be defined and at contain at least one 'DbIndex' element.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return ECObjectsStatus::Error;
        }

    for (uint32_t i = 0; i < indexCount; i++)
        {
        ECValue indexVal;
        stat = m_ca->GetValue(indexVal, propIx, i);
        if (ECObjectsStatus::Success != stat)
            {
            LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s could not be retrieved.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return stat;
            }

        IECInstancePtr dbIndexCA = indexVal.GetStruct();
        if (dbIndexCA == nullptr)
            continue;

        //optional
        Utf8String indexName;
        stat = CustomAttributeReader::TryGetTrimmedValue(indexName, *dbIndexCA, "Name");
        if (ECObjectsStatus::Success != stat)
            return stat;

        Utf8String whereClause;
        stat = CustomAttributeReader::TryGetTrimmedValue(whereClause, *dbIndexCA, "Where");
        if (ECObjectsStatus::Success != stat)
            return stat;

        bool isUnique = false;
        stat = CustomAttributeReader::TryGetBooleanValue(isUnique, *dbIndexCA, "IsUnique");
        if (ECObjectsStatus::Success != stat)
            return stat;

        //Properties are mandatory for the index to be valid, so fail if there are none
        uint32_t propertiesPropIdx;
        stat = dbIndexCA->GetEnablerR().GetPropertyIndex(propertiesPropIdx, "Properties");
        if (ECObjectsStatus::Success != stat)
            {
            LOG.errorv("Failed to get property index for property 'Indexes.Properties' of custom attribute '%s' on ECClass '%s'.",
                       m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return stat;
            }

        ECValue propertiesVal;
        uint32_t propertiesCount = 0;
        stat = dbIndexCA->GetValue(propertiesVal, propertiesPropIdx);
        if (ECObjectsStatus::Success != stat)
            return stat;

        if (propertiesVal.IsNull() || (propertiesCount = propertiesVal.GetArrayInfo().GetCount()) == 0)
            {
            LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. At least one property must be specified.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ECObjectsStatus::Error;
            }

        DbIndex dbIndex(indexName.c_str(), isUnique, whereClause.c_str());
        for (uint32_t j = 0; j < propertiesCount; j++)
            {
            Utf8String propName;
            if (ECObjectsStatus::Success != CustomAttributeReader::TryGetTrimmedValue(propName, *dbIndexCA, propertiesPropIdx, j) ||
                propName.empty())
                {
                LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. Array element of the Properties property is an empty string.",
                           i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
                return ECObjectsStatus::Error;
                }

            BeAssert(!propName.empty());
            dbIndex.AddProperty(propName);
            }

        indices.push_back(std::move(dbIndex));
        }

    BeAssert(indices.size() == indexCount);
    return ECObjectsStatus::Success;
    }


//*****************************************************************
//ECDbPropertyMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbPropertyMap::ECDbPropertyMap(ECPropertyCR ecProperty, IECInstanceCP ca) : m_property(&ecProperty), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetColumnName(Utf8StringR columnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(columnName, *m_ca, "ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetIsNullable(bool& isNullable) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetBooleanValue(isNullable, *m_ca, "IsNullable");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetIsUnique(bool& isUnique) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetBooleanValue(isUnique, *m_ca, "IsUnique");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetCollation(Utf8StringR collation) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(collation, *m_ca, "Collation");
    }


//*****************************************************************
//ShareColumns
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ShareColumns::TryGetSharedColumnCount(int& sharedColumnCount) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetIntegerValue(sharedColumnCount, *m_ca, "SharedColumnCount");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ShareColumns::TryGetOverflowColumnName(Utf8String& excessColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(excessColumnName, *m_ca, "OverflowColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ShareColumns::TryGetApplyToSubclassesOnly(bool& applyToSubclassesOnly) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetBooleanValue(applyToSubclassesOnly, *m_ca, "ApplyToSubclassesOnly");
    }

//*****************************************************************
//ECDbLinkTableRelationshipMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbLinkTableRelationshipMap::ECDbLinkTableRelationshipMap(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetSourceECInstanceIdColumn(Utf8StringR sourceECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(sourceECInstanceIdColumnName, *m_ca, "SourceECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetTargetECInstanceIdColumn(Utf8StringR targetECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(targetECInstanceIdColumnName, *m_ca, "TargetECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetAllowDuplicateRelationships(bool& allowDuplicateRelationshipsFlag) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetBooleanValue(allowDuplicateRelationshipsFlag, *m_ca, "AllowDuplicateRelationships");
    }

//*****************************************************************
//ECDbForeignKeyConstraint
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbForeignKeyConstraint::ECDbForeignKeyConstraint(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyConstraint::TryGetOnDeleteAction(Utf8StringR onDeleteAction) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(onDeleteAction, *m_ca, "OnDeleteAction");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyConstraint::TryGetOnUpdateAction(Utf8StringR onUpdateAction) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(onUpdateAction, *m_ca, "OnUpdateAction");
    }


//*****************************************************************
//CustomAttributeReader
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstanceCP CustomAttributeReader::Read(IECCustomAttributeContainer const& caContainer, Utf8CP customAttributeSchemaName, Utf8CP customAttributeName)
    {
    for (IECInstancePtr const& ca : caContainer.GetCustomAttributes(false))
        {
        ECClassCR caClass = ca->GetClass();
        if (caClass.GetName().Equals(customAttributeName) && caClass.GetSchema().GetName().Equals(customAttributeSchemaName))
            return ca.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return stat;
        }

    return TryGetTrimmedValue(val, v);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, propIndex, arrayIndex);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve array element #%d value of ECProperty with property index %d of the '%s' custom attribute ECInstance.",
                   arrayIndex, propIndex, ca.GetClass().GetFullName());
        return stat;
        }

    return TryGetTrimmedValue(val, v);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetTrimmedValue(Utf8StringR strVal, ECValueCR val)
    {
    if (!val.IsNull())
        {
        strVal = val.GetUtf8CP();
        strVal.Trim();
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetIntegerValue(int& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return stat;
        }

    if (!v.IsNull())
        val = v.GetInteger();

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetBooleanValue(bool& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return stat;
        }

    if (!v.IsNull())
        val = v.GetBoolean();

    return ECObjectsStatus::Success;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
