/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//*********************** DateTimeInfo *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
const DateTime::Kind DateTimeInfo::DEFAULT_KIND = DateTime::Kind::Unspecified;
const DateTime::Component DateTimeInfo::DEFAULT_COMPONENT = DateTime::Component::DateAndTime;
const DateTime::Info DateTimeInfo::s_default = DateTime::Info (DateTimeInfo::DEFAULT_KIND, DateTimeInfo::DEFAULT_COMPONENT);

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTimeInfo::DateTimeInfo ()
    : m_isKindNull (true), m_isComponentNull (true)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTimeInfo::DateTimeInfo (DateTime::Info const& metadata)
    : m_isKindNull (false), m_isComponentNull (false), m_info (metadata)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTimeInfo::DateTimeInfo (bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component)
    : m_isKindNull (isKindNull), m_isComponentNull (isComponentNull), m_info (kind, component)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 08/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::operator== (DateTimeInfo const& rhs) const
    {
    return (
            ((m_isKindNull && rhs.m_isKindNull) || (!m_isKindNull && !rhs.m_isKindNull && m_info.GetKind () == rhs.m_info.GetKind ())) &&
            ((m_isComponentNull && rhs.m_isComponentNull) || (!m_isComponentNull && !rhs.m_isComponentNull && m_info.GetComponent () == rhs.m_info.GetComponent ()))
            );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 08/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::operator!= (DateTimeInfo const& rhs) const
    {
    return !(*this == rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info DateTimeInfo::GetInfo (bool useDefaultIfUnset) const
    {
    if (!useDefaultIfUnset)
        {
        return m_info;
        }

    const DateTime::Kind kind = IsKindNull () ? DEFAULT_KIND : m_info.GetKind ();
    const DateTime::Component component = IsComponentNull () ? DEFAULT_COMPONENT : m_info.GetComponent ();

    return DateTime::Info (kind, component);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::IsNull () const
    {
    return IsKindNull () && IsComponentNull ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::IsKindNull () const
    {
    return m_isKindNull;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::IsComponentNull () const
    {
    return m_isComponentNull;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info const& DateTimeInfo::GetInfo () const
    {
    return m_info;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
DateTime::Info const& DateTimeInfo::GetDefault ()
    {
    return s_default;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DateTimeInfo::IsMatchedBy (DateTime::Info const& rhs) const
    {
    DateTime::Info const& lhsInfo = GetInfo ();
    //If one of the members
    //of this object is null, the RHS counterpart is ignored and the
    //members are considered matching.
    return (IsKindNull () || lhsInfo.GetKind () == rhs.GetKind ()) &&
        (IsComponentNull () || lhsInfo.GetComponent () == rhs.GetComponent ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
WString DateTimeInfo::ToString () const
    {
    DateTime::Info const& info = GetInfo ();

    WString str;
    //reserve for the maximum length
    str.reserve (36);

    const bool isKindNull = IsKindNull ();
    if (!isKindNull)
        {
        str.append (L"Kind: ");
        str.append (DateTime::Info::KindToString (info.GetKind ()));
        }

    if (!IsComponentNull ())
        {
        if (!isKindNull)
            {
            str.append (L" ");
            }

        str.append (L"Component: ");
        str.append (DateTime::Info::ComponentToString (info.GetComponent ()));
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
    bmap<WString, StandaloneECEnablerPtr> m_enablers;

    static StandardCustomAttributesSchemaHolderPtr s_schemaHolder;

    StandardCustomAttributesSchemaHolder();

    ECSchemaPtr _GetSchema();

    IECInstancePtr _CreateCustomAttributeInstance(WCharCP attribute);

public:

/*__PUBLISH_SECTION_START__*/
    static StandardCustomAttributesSchemaHolderPtr GetHolder();

    static ECSchemaPtr GetSchema();

    static IECInstancePtr CreateCustomAttributeInstance(WCharCP attribute);
};

StandardCustomAttributesSchemaHolderPtr StandardCustomAttributesSchemaHolder::s_schemaHolder;

static WCharCP s_supplementalMetaDataAccessor = L"SupplementalSchemaMetaData";
static WCharCP s_supplementalProvenanceAccessor = L"SupplementalProvenance";
static const uint32_t s_bscaVersionMajor = 1;
static const uint32_t s_bscaVersionMinor = 8;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandardCustomAttributesSchemaHolder::StandardCustomAttributesSchemaHolder()
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey key (L"Bentley_Standard_CustomAttributes", s_bscaVersionMajor, s_bscaVersionMinor);

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
IECInstancePtr StandardCustomAttributesSchemaHolder::_CreateCustomAttributeInstance(WCharCP attribute)
    {
    if (!m_schema.IsValid())
        _GetSchema();

    auto enablerIterator = m_enablers.find(attribute);
    if (enablerIterator == m_enablers.end())
        {
        BeDataAssert (false && "Unknown supplemental schema custom attribute class name. Currently only SupplementalSchemaMetaData and SupplementalProvenance are supported.");
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
IECInstancePtr StandardCustomAttributesSchemaHolder::CreateCustomAttributeInstance(WCharCP attribute)
    {
    return GetHolder()->_CreateCustomAttributeInstance(attribute);
    }

//*********************** StandardCustomAttributeHelper *************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
WCharCP const StandardCustomAttributeHelper::SYSTEMSCHEMA_CA_NAME = L"SystemSchema";
//static
WCharCP const StandardCustomAttributeHelper::DYNAMICSCHEMA_CA_NAME = L"DynamicSchema";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus StandardCustomAttributeHelper::GetDateTimeInfo (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::GetFrom (dateTimeInfo, dateTimeProperty);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsSystemSchema (ECSchemaCR schema)
    {
    return schema.IsDefined (SYSTEMSCHEMA_CA_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsDynamicSchema (ECSchemaCR schema)
    {
    return schema.IsDefined (DYNAMICSCHEMA_CA_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan    02/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus StandardCustomAttributeHelper::SetIsDynamicSchema (ECSchemaR schema, bool isDynamic)
    {
    const bool isDynamicExistingValue = IsDynamicSchema (schema);
    if (isDynamic)
        {
        if (isDynamicExistingValue)
            return ECOBJECTS_STATUS_Success;

        SchemaNameClassNamePair dynamicSchemaClassId (L"Bentley_Standard_CustomAttributes", DYNAMICSCHEMA_CA_NAME);
        ECClassP dynamicSchemaClass = schema.GetReferencedSchemas().FindClassP (dynamicSchemaClassId);
        //BeAssert (dynamicSchemaClass != NULL && "It seem BSCA schema is not referenced or current reference has version less then 1.6");
        if (dynamicSchemaClass == NULL)
            return ECOBJECTS_STATUS_DynamicSchemaCustomAttributeWasNotFound;

        IECInstancePtr dynamicSchemaInstance = dynamicSchemaClass->GetDefaultStandaloneEnabler()->CreateInstance();
        return schema.SetCustomAttribute (*dynamicSchemaInstance);
        }

    if (!isDynamicExistingValue)
        return ECOBJECTS_STATUS_Success;

    if (schema.RemoveCustomAttribute (DYNAMICSCHEMA_CA_NAME))
        return ECOBJECTS_STATUS_Success;

    return ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP StandardCustomAttributeHelper::GetCustomAttributeClass(WCharCP attributeName)
    {
    return StandardCustomAttributesSchemaHolder::GetSchema()->GetClassCP(attributeName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributeHelper::CreateCustomAttributeInstance(WCharCP attributeName)
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
    static IECInstanceCP Read(IECCustomAttributeContainer const& caContainer, WCharCP customAttributeSchemaName, WCharCP customAttributeName);

    static ECObjectsStatus TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyAccessString);
    static ECObjectsStatus TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex);
    static ECObjectsStatus TryGetBooleanValue(bool& val, IECInstanceCR ca, WCharCP ecPropertyAccessString);
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
    IECInstanceCP ca = CustomAttributeReader::Read(schema, L"ECDbMap", L"SchemaMap");
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
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, L"ECDbMap", L"ClassMap");
    if (ca == nullptr)
        return false;

    classMap = ECDbClassMap(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetPropertyMap(ECDbPropertyMap& propertyMap, ECPropertyCR ecProperty)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecProperty, L"ECDbMap", L"PropertyMap");
    if (ca == nullptr)
        return false;

    propertyMap = ECDbPropertyMap(ecProperty, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap (ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, L"ECDbMap", L"LinkTableRelationshipMap");
    if (ca == nullptr)
        return false;

    linkTableRelationshipMap = ECDbLinkTableRelationshipMap(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap (ECDbForeignKeyRelationshipMap& foreignKeyTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, L"ECDbMap", L"ForeignKeyRelationshipMap");
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
        return ECOBJECTS_STATUS_Error;

    ECObjectsStatus stat = CustomAttributeReader::TryGetTrimmedValue(tablePrefix, *m_ca, L"TablePrefix");
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    if (tablePrefix.empty() || ECNameValidation::Validate(WString(tablePrefix.c_str(), BentleyCharEncoding::Utf8).c_str()) == ECNameValidation::RESULT_Valid)
        return ECOBJECTS_STATUS_Success;

    LOG.errorv("Custom attribute '%s' on the ECSchema '%s' has an invalid value for the property 'TablePrefix': %s. "
               "The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.",
               Utf8String(m_ca->GetClass().GetName()).c_str(),
               Utf8String(m_schema->GetName()).c_str(), tablePrefix.c_str());
    return ECOBJECTS_STATUS_Error;
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
ECObjectsStatus ECDbClassMap::TryGetMapStrategy(Utf8StringR mapStrategy, Utf8StringR mapStrategyOptions) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    ECObjectsStatus stat = CustomAttributeReader::TryGetTrimmedValue(mapStrategy, *m_ca, L"MapStrategy");
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    return CustomAttributeReader::TryGetTrimmedValue(mapStrategyOptions, *m_ca, L"MapStrategyOptions");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetTableName(Utf8String& tableName) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetTrimmedValue(tableName, *m_ca, L"TableName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, *m_ca, L"ECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbClassMap::TryGetIndexes(bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    uint32_t propIx;
    ECObjectsStatus stat = m_ca->GetEnablerR().GetPropertyIndex(propIx, L"Indexes");
    if (ECOBJECTS_STATUS_Success != stat)
        {
        LOG.errorv(L"Failed to get property index for property 'Indexes' of custom attribute '%ls' on ECClass '%ls'.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return stat;
        }

    ECValue indexesVal;
    stat = m_ca->GetValue(indexesVal, propIx);
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    indices.clear();
    const uint32_t indexCount = indexesVal.IsNull () ? 0 : indexesVal.GetArrayInfo().GetCount();
    if (indexCount == 0)
        return ECOBJECTS_STATUS_Success;

    for (uint32_t i = 0; i < indexCount; i++)
        {
        ECValue indexVal;
        stat = m_ca->GetValue(indexVal, propIx, i);
        if (ECOBJECTS_STATUS_Success != stat)
            {
            LOG.errorv(L"DbIndex #%d in custom attribute %ls on ECClass %ls could not be retrieved.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return stat;
            }

        IECInstancePtr dbIndexCA = indexVal.GetStruct();
        if (dbIndexCA == nullptr)
            continue;

        //optional
        Utf8String indexName;
        stat = CustomAttributeReader::TryGetTrimmedValue(indexName, *dbIndexCA, L"Name");
        if (ECOBJECTS_STATUS_Success != stat)
            return stat;

        Utf8String whereClause;
        stat = CustomAttributeReader::TryGetTrimmedValue(whereClause, *dbIndexCA, L"Where");
        if (ECOBJECTS_STATUS_Success != stat)
            return stat;

        bool isUnique = false;
        stat = CustomAttributeReader::TryGetBooleanValue(isUnique, *dbIndexCA, L"IsUnique");
        if (ECOBJECTS_STATUS_Success != stat)
            return stat;

        //Properties are mandatory for the index to be valid, so fail if there are none
        uint32_t propertiesPropIdx;
        stat = dbIndexCA->GetEnablerR().GetPropertyIndex(propertiesPropIdx, L"Properties");
        if (ECOBJECTS_STATUS_Success != stat)
            {
            LOG.errorv(L"Failed to get property index for property 'Indexes.Properties' of custom attribute '%ls' on ECClass '%ls'.",
                       m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return stat;
            }

        ECValue propertiesVal;
        uint32_t propertiesCount = 0;
        stat = dbIndexCA->GetValue(propertiesVal, propertiesPropIdx);
        if (ECOBJECTS_STATUS_Success != stat)
            return stat;
           
        if(propertiesVal.IsNull() || (propertiesCount = propertiesVal.GetArrayInfo().GetCount()) == 0)
            {
            LOG.errorv(L"DbIndex #%d in custom attribute %ls on ECClass %ls is invalid. At least one property must be specified.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ECOBJECTS_STATUS_Error;
            }

        DbIndex dbIndex(indexName.c_str(), isUnique, whereClause.c_str());
        for (uint32_t j = 0; j < propertiesCount; j++)
            {
            Utf8String propName;
            if (ECOBJECTS_STATUS_Success != CustomAttributeReader::TryGetTrimmedValue(propName, *dbIndexCA, propertiesPropIdx, j) ||
                propName.empty())
                {
                LOG.errorv(L"DbIndex #%d in custom attribute %ls on ECClass %ls is invalid. Array element of the Properties property is an empty string.",
                           i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
                return ECOBJECTS_STATUS_Error;
                }

            BeAssert(!propName.empty());
            dbIndex.AddProperty(propName);
            }

        indices.push_back(std::move(dbIndex));
        }

    BeAssert(indices.size() == indexCount);
    return ECOBJECTS_STATUS_Success;
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
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetTrimmedValue(columnName, *m_ca, L"ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetIsNullable(bool& isNullable) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetBooleanValue(isNullable, *m_ca, L"IsNullable");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetIsUnique(bool& isUnique) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetBooleanValue(isUnique, *m_ca, L"IsUnique");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbPropertyMap::TryGetCollation(Utf8StringR collation) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetTrimmedValue(collation, *m_ca, L"Collation");
    }


//*****************************************************************
//ECDbRelationshipConstraintMapHelper
//*****************************************************************
struct ECDbRelationshipConstraintMapHelper
    {
private:
    ECDbRelationshipConstraintMapHelper();
    ~ECDbRelationshipConstraintMapHelper();

public:
    static ECObjectsStatus TryRead(ECDbRelationshipConstraintMap& constraintMap, IECInstanceCR ca, WCharCP propName);
    };

//*****************************************************************
//ECDbLinkTableRelationshipMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbLinkTableRelationshipMap::ECDbLinkTableRelationshipMap(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca) {}
    


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetSource(ECDbRelationshipConstraintMap& sourceConstraint) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return ECDbRelationshipConstraintMapHelper::TryRead(sourceConstraint, *m_ca, L"Source");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetTarget(ECDbRelationshipConstraintMap& targetConstraint) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return ECDbRelationshipConstraintMapHelper::TryRead(targetConstraint, *m_ca, L"Target");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbLinkTableRelationshipMap::TryGetAllowDuplicateRelationships(bool& allowDuplicateRelationshipsFlag) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return CustomAttributeReader::TryGetBooleanValue(allowDuplicateRelationshipsFlag, *m_ca, L"AllowDuplicateRelationships");
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
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetForeignKeyEnd(ECRelationshipEnd& foreignKeyEnd) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    Utf8String foreignKeyEndStr;
    if (!CustomAttributeReader::TryGetTrimmedValue(foreignKeyEndStr, *m_ca, L"ForeignKeyEnd"))
        return ECOBJECTS_STATUS_Success;

    if (foreignKeyEndStr.EqualsI("source"))
        foreignKeyEnd = ECRelationshipEnd_Source;
    else if (foreignKeyEndStr.EqualsI("target"))
        foreignKeyEnd = ECRelationshipEnd_Target;
    else
        return ECOBJECTS_STATUS_Error;

    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECDbForeignKeyRelationshipMap::TryGetForeignKey(ECDbRelationshipConstraintMap& foreignKeyConstraint) const
    {
    if (m_ca == nullptr)
        return ECOBJECTS_STATUS_Error;

    return ECDbRelationshipConstraintMapHelper::TryRead(foreignKeyConstraint, *m_ca, L"ForeignKey");
    }


//*****************************************************************
//ECDbRelationshipConstraintMapHelper
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus ECDbRelationshipConstraintMapHelper::TryRead(ECDbRelationshipConstraintMap& constraintMap, IECInstanceCR ca, WCharCP propName)
    {
    if (WString::IsNullOrEmpty(propName))
        return ECOBJECTS_STATUS_Error;

    WString accessString(propName);
    accessString.append(L".ECInstanceIdColumn");
    Utf8String ecInstanceIdColumnName;
    ECObjectsStatus stat = CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    accessString = propName;
    accessString.append(L".ECClassIdColumn");
    Utf8String ecClassIdColumnName;
    stat = CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    accessString = propName;
    accessString.append(L".CreateDefaultIndex");
    bool createDefaultIndex = true; //default is true
    stat = CustomAttributeReader::TryGetBooleanValue(createDefaultIndex, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    accessString = propName;
    accessString.append(L".EnforceReferentialIntegrity");
    bool enforceReferentialIntegrity = false;
    stat = CustomAttributeReader::TryGetBooleanValue(enforceReferentialIntegrity, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    accessString = propName;
    accessString.append(L".OnDeleteAction");
    Utf8String onDeleteAction;
    stat = CustomAttributeReader::TryGetTrimmedValue(onDeleteAction, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    accessString = propName;
    accessString.append(L".OnUpdateAction");
    Utf8String onUpdateAction;
    stat = CustomAttributeReader::TryGetTrimmedValue(onUpdateAction, ca, accessString.c_str());
    if (ECOBJECTS_STATUS_Success != stat)
        return stat;

    constraintMap = ECDbRelationshipConstraintMap(ecInstanceIdColumnName.c_str(), ecClassIdColumnName.c_str(), createDefaultIndex, enforceReferentialIntegrity, onDeleteAction.c_str(), onUpdateAction.c_str());
    return ECOBJECTS_STATUS_Success;
    }

//*****************************************************************
//CustomAttributeReader
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstanceCP CustomAttributeReader::Read(IECCustomAttributeContainer const& caContainer, WCharCP customAttributeSchemaName, WCharCP customAttributeName)
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
ECObjectsStatus CustomAttributeReader::TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECOBJECTS_STATUS_Success != stat)
        {
        LOG.errorv(L"Could not retrieve value of ECProperty '%ls' of the '%ls' custom attribute ECInstance.",
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
    if (ECOBJECTS_STATUS_Success != stat)
        {
        LOG.errorv(L"Could not retrieve array element #%d value of ECProperty with property index %d of the '%ls' custom attribute ECInstance.",
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

    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
ECObjectsStatus CustomAttributeReader::TryGetBooleanValue(bool& val, IECInstanceCR ca, WCharCP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECOBJECTS_STATUS_Success != stat)
        {
        LOG.errorv(L"Could not retrieve value of ECProperty '%ls' of the '%ls' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return stat;
        }

    if (!v.IsNull())
        val = v.GetBoolean();

    return ECOBJECTS_STATUS_Success;
    }


END_BENTLEY_ECOBJECT_NAMESPACE
