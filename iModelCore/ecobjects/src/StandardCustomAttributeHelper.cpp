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

public:
    static IECInstanceCP Read(IECCustomAttributeContainer const& caContainer, WCharCP customAttributeSchemaName, WCharCP customAttributeName);
    static bool TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyName);
    static bool TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex);
    static bool TryGetBooleanValue(bool& val, IECInstanceCR ca, WCharCP ecPropertyName);
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
bool ECDbSchemaMap::TryGetTablePrefix(Utf8String& tablePrefix) const
    {
    if (m_ca == nullptr)
        return false;

    if (CustomAttributeReader::TryGetTrimmedValue(tablePrefix, *m_ca, L"TablePrefix"))
        {
        if (ECNameValidation::Validate(WString(tablePrefix.c_str(), BentleyCharEncoding::Utf8).c_str()) == ECNameValidation::RESULT_Valid)
            return true;

        LOG.errorv("Custom attribute '%s' on the ECSchema '%s' has an invalid value for the property 'TablePrefix': %s. "
                   "The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.",
                   Utf8String(m_ca->GetClass().GetName()).c_str(),
                   Utf8String(m_schema->GetName()).c_str(), tablePrefix.c_str());

        tablePrefix.clear();
        }

    return false;
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
bool ECDbClassMap::TryGetMapStrategy(Utf8StringR mapStrategy, Utf8StringR mapStrategyOptions) const
    {
    if (m_ca == nullptr)
        return false;

    const bool foundMapStrategy = CustomAttributeReader::TryGetTrimmedValue(mapStrategy, *m_ca, L"MapStrategy");
    const bool foundMapStrategyOptions = CustomAttributeReader::TryGetTrimmedValue(mapStrategyOptions, *m_ca, L"MapStrategyOptions");
    return foundMapStrategy || foundMapStrategyOptions;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbClassMap::TryGetTableName (Utf8String& tableName) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetTrimmedValue(tableName, *m_ca, L"TableName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbClassMap::TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, *m_ca, L"ECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbClassMap::TryGetIndexes (bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return false;

    indices.clear();

    uint32_t propIx;
    if (m_ca->GetEnablerR().GetPropertyIndex(propIx, L"Indexes") != ECOBJECTS_STATUS_Success)
        {
        LOG.errorv(L"Failed to get property index for property 'Indexes' of custom attribute '%ls' on ECClass '%ls'.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return false;
        }

    ECValue indexesVal;
    if (m_ca->GetValue(indexesVal, propIx) != ECOBJECTS_STATUS_Success || indexesVal.IsNull())
        return false;

    const uint32_t indexCount = indexesVal.GetArrayInfo().GetCount();
    if (indexCount == 0)
        return false;

    for (uint32_t i = 0; i < indexCount; i++)
        {
        ECValue indexVal;
        if (m_ca->GetValue(indexVal, propIx, i) != ECOBJECTS_STATUS_Success)
            {
            LOG.errorv(L"DbIndex #%d in custom attribute %ls on ECClass %ls could not be retrieved.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return false;
            }

        IECInstancePtr dbIndexCA = indexVal.GetStruct();
        if (dbIndexCA == nullptr)
            continue;

        //optional
        Utf8String indexName;
        CustomAttributeReader::TryGetTrimmedValue(indexName, *dbIndexCA, L"Name");

        Utf8String whereClause;
        CustomAttributeReader::TryGetTrimmedValue(whereClause, *dbIndexCA, L"Where");

        bool isUnique = false;
        CustomAttributeReader::TryGetBooleanValue(isUnique, *dbIndexCA, L"IsUnique");

        //Properties are mandatory for the index to be valid, so fail if there are none
        uint32_t propertiesPropIdx;
        if (dbIndexCA->GetEnablerR().GetPropertyIndex(propertiesPropIdx, L"Properties") != ECOBJECTS_STATUS_Success)
            {
            LOG.errorv(L"Failed to get property index for property 'Indexes.Properties' of custom attribute '%ls' on ECClass '%ls'.",
                       m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return false;
            }

        ECValue propertiesVal;
        uint32_t propertiesCount = 0;
        if (dbIndexCA->GetValue(propertiesVal, propertiesPropIdx) != ECOBJECTS_STATUS_Success || propertiesVal.IsNull() ||
            (propertiesCount = propertiesVal.GetArrayInfo().GetCount()) == 0)
            {
            LOG.errorv(L"DbIndex #%d in custom attribute %ls on ECClass %ls is invalid. At least one property must be specified.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return false;
            }

        DbIndex dbIndex(indexName.c_str(), isUnique, whereClause.c_str());
        for (uint32_t j = 0; j < propertiesCount; j++)
            {
            Utf8String propName;
            if (CustomAttributeReader::TryGetTrimmedValue(propName, *dbIndexCA, propertiesPropIdx, j))
                dbIndex.AddProperty(propName);
            }

        indices.push_back(std::move(dbIndex));
        }

    BeAssert(indices.size() == indexCount);
    return true;
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
bool ECDbPropertyMap::TryGetColumnName(Utf8StringR columnName) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetTrimmedValue(columnName, *m_ca, L"ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbPropertyMap::TryGetIsNullable(bool& isNullable) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetBooleanValue(isNullable, *m_ca, L"IsNullable");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbPropertyMap::TryGetIsUnique(bool& isUnique) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetBooleanValue(isUnique, *m_ca, L"IsUnique");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbPropertyMap::TryGetCollation(Utf8StringR collation) const
    {
    if (m_ca == nullptr)
        return false;

    return CustomAttributeReader::TryGetTrimmedValue(collation, *m_ca, L"Collation");
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
bool CustomAttributeReader::TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyName)
    {
    ECValue v;

    if (ca.GetValue(v, ecPropertyName) != ECOBJECTS_STATUS_Success || v.IsNull())
        return false;

    val = v.GetUtf8CP();
    val.Trim();
    return true;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bool CustomAttributeReader::TryGetTrimmedValue(Utf8StringR val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex)
    {
    ECValue v;
    if (ca.GetValue(v, propIndex, arrayIndex) != ECOBJECTS_STATUS_Success || v.IsNull())
        return false;

    val = v.GetUtf8CP();
    val.Trim();
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
bool CustomAttributeReader::TryGetBooleanValue(bool& val, IECInstanceCR ca, WCharCP ecPropertyName)
    {
    ECValue v;
    if (ca.GetValue(v, ecPropertyName) == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        val = v.GetBoolean();
        return true;
        }

    return false;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
