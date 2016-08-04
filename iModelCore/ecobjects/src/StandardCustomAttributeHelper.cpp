/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define BSCA_SCHEMA_NAME "Bentley_Standard_CustomAttributes"
#define SYSTEMSCHEMA_CA_NAME "SystemSchema"
#define DYNAMICSCHEMA_CA_NAME "DynamicSchema"

#define ECDBMAP_SCHEMA_NAME "ECDbMap"

//*********************** DateTimeInfo *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
const DateTime::Kind DateTimeInfo::DEFAULT_KIND = DateTime::Kind::Unspecified;
const DateTime::Component DateTimeInfo::DEFAULT_COMPONENT = DateTime::Component::DateAndTime;
const DateTime::Info DateTimeInfo::s_default = DateTime::Info(DateTimeInfo::DEFAULT_KIND, DateTimeInfo::DEFAULT_COMPONENT);

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTimeInfo::DateTimeInfo(bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component)
    : m_isKindNull(isKindNull), m_isComponentNull(isComponentNull), m_info(kind, component)
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 08/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::operator== (DateTimeInfo const& rhs) const
    {
    return ((m_isKindNull && rhs.m_isKindNull) || (!m_isKindNull && !rhs.m_isKindNull && m_info.GetKind() == rhs.m_info.GetKind())) &&
        ((m_isComponentNull && rhs.m_isComponentNull) || (!m_isComponentNull && !rhs.m_isComponentNull && m_info.GetComponent() == rhs.m_info.GetComponent()));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info DateTimeInfo::GetInfo(bool useDefaultIfUnset) const
    {
    if (!useDefaultIfUnset)
        {
        return m_info;
        }

    const DateTime::Kind kind = IsKindNull() ? DEFAULT_KIND : m_info.GetKind();
    const DateTime::Component component = IsComponentNull() ? DEFAULT_COMPONENT : m_info.GetComponent();

    return DateTime::Info(kind, component);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info const& DateTimeInfo::GetInfo() const { return m_info; }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime::Info const& DateTimeInfo::GetDefault() { return s_default; }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::IsMatchedBy(DateTime::Info const& rhs) const
    {
    DateTime::Info const& lhsInfo = GetInfo();
    //If one of the members
    //of this object is null, the RHS counterpart is ignored and the
    //members are considered matching.
    return (IsKindNull() || lhsInfo.GetKind() == rhs.GetKind()) &&
        (IsComponentNull() || lhsInfo.GetComponent() == rhs.GetComponent());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DateTimeInfo::ToString() const
    {
    DateTime::Info const& info = GetInfo();

    Utf8String str;
    //reserve for the maximum length
    str.reserve(36);

    const bool isKindNull = IsKindNull();
    if (!isKindNull)
        {
        str.append("Kind: ");
        str.append(DateTime::Info::KindToString(info.GetKind()));
        }

    if (!IsComponentNull())
        {
        if (!isKindNull)
            {
            str.append(" ");
            }

        str.append("Component: ");
        str.append(DateTime::Info::ComponentToString(info.GetComponent()));
        }

    return str;
    }


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
static const uint32_t s_bscaVersionMajor = 1;
static const uint32_t s_bscaVersionMinor = 8;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCustomAttributesSchemaHolder::StandardCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key(BSCA_SCHEMA_NAME, s_bscaVersionMajor, s_bscaVersionMinor);

    m_schema = ECSchema::LocateSchema(key, *schemaContext);

    ECClassP metaDataClass = m_schema->GetClassP(s_supplementalMetaDataAccessor);
    StandaloneECEnablerPtr enabler;
    if (NULL != metaDataClass)
        enabler = metaDataClass->GetDefaultStandaloneEnabler();

    m_enablers.Insert(s_supplementalMetaDataAccessor, enabler);

    ECClassP provenanceClass = m_schema->GetClassP(s_supplementalProvenanceAccessor);
    StandaloneECEnablerPtr provenanceEnabler;
    if (NULL != provenanceClass)
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
ECObjectsStatus StandardCustomAttributeHelper::GetDateTimeInfo(DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom(dateTimeInfo, dateTimeProperty);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsSystemSchema(ECSchemaCR schema)
    {
    return schema.IsDefined(BSCA_SCHEMA_NAME, SYSTEMSCHEMA_CA_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsDynamicSchema(ECSchemaCR schema)
    {
    return schema.IsDefined(BSCA_SCHEMA_NAME, DYNAMICSCHEMA_CA_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan    02/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus StandardCustomAttributeHelper::SetIsDynamicSchema(ECSchemaR schema, bool isDynamic)
    {
    const bool isDynamicExistingValue = IsDynamicSchema(schema);
    if (isDynamic)
        {
        if (isDynamicExistingValue)
            return ECObjectsStatus::Success;

        SchemaNameClassNamePair dynamicSchemaClassId(BSCA_SCHEMA_NAME, DYNAMICSCHEMA_CA_NAME);
        ECClassP dynamicSchemaClass = schema.GetReferencedSchemas().FindClassP(dynamicSchemaClassId);
        //BeAssert (dynamicSchemaClass != NULL && "It seem BSCA schema is not referenced or current reference has version less then 1.6");
        if (dynamicSchemaClass == nullptr)
            return ECObjectsStatus::DynamicSchemaCustomAttributeWasNotFound;

        IECInstancePtr dynamicSchemaInstance = dynamicSchemaClass->GetDefaultStandaloneEnabler()->CreateInstance();
        return schema.SetCustomAttribute(*dynamicSchemaInstance);
        }

    if (!isDynamicExistingValue)
        return ECObjectsStatus::Success;

    if (schema.RemoveCustomAttribute(BSCA_SCHEMA_NAME, DYNAMICSCHEMA_CA_NAME))
        return ECObjectsStatus::Success;

    return ECObjectsStatus::Error;
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
bool ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(ECClassCR ecClass)
    {
    return ecClass.GetCustomAttributeLocal(ECDBMAP_SCHEMA_NAME, "JoinedTablePerDirectSubclass") != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetPropertyMap(ECDbPropertyMap& propertyMap, ECPropertyCR ecProperty)
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
bool ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap(ECDbForeignKeyRelationshipMap& foreignKeyTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMA_NAME, "ForeignKeyRelationshipMap");
    if (ca == nullptr)
        return false;

    foreignKeyTableRelationshipMap = ECDbForeignKeyRelationshipMap(ecRelationship, ca);
    return true;
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

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetIndexes(bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    uint32_t propIx;
    ECObjectsStatus stat = m_ca->GetEnablerR().GetPropertyIndex(propIx, "Indexes");
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Failed to get property index for property 'Indexes' of custom attribute '%s' on ECClass '%s'.",
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
        return ECObjectsStatus::Success;

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
ECObjectsStatus ShareColumns::TryGetExcessColumnName(Utf8String& excessColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(excessColumnName, *m_ca, "ExcessColumnName");
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
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetSourceECClassIdColumn(Utf8StringR sourceECClassIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(sourceECClassIdColumnName, *m_ca, "SourceECClassIdColumn");
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
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetTargetECClassIdColumn(Utf8StringR targetECClassIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(targetECClassIdColumnName, *m_ca, "TargetECClassIdColumn");
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
//ECDbForeignKeyRelationshipMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbForeignKeyRelationshipMap::ECDbForeignKeyRelationshipMap(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca)
    {}


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetForeignKeyColumn(Utf8StringR foreignKeyColumnName) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(foreignKeyColumnName, *m_ca, "ForeignKeyColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetCreateIndex(bool& createIndexFlag) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetBooleanValue(createIndexFlag, *m_ca, "CreateIndex");
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetOnDeleteAction(Utf8StringR onDeleteAction) const
    {
    if (m_ca == nullptr)
        return ECObjectsStatus::Error;

    return CustomAttributeReader::TryGetTrimmedValue(onDeleteAction, *m_ca, "OnDeleteAction");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetOnUpdateAction(Utf8StringR onUpdateAction) const
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
