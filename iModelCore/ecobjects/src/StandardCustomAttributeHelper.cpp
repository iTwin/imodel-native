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
    return m_isKindNull == rhs.m_isKindNull && m_isComponentNull == rhs.m_isComponentNull && m_info == rhs.m_info;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
WCharCP const StandardCustomAttributeHelper::SYSTEMSCHEMA_CA_NAME = L"SystemSchema";
//static
WCharCP const StandardCustomAttributeHelper::DYNAMICSCHEMA_CA_NAME = L"DynamicSchema";
//static
WCharCP const StandardCustomAttributeHelper::SYSTEMSCHEMA_CA_SCHEMA = L"Bentley_Standard_CustomAttributes";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::TryGetDateTimeInfo (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::TryGetFrom (dateTimeInfo, dateTimeProperty);
    }

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
    bmap<WCharCP, StandaloneECEnablerPtr> m_enablers;

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

    bmap<WCharCP, StandaloneECEnablerPtr>::const_iterator enablerIterator = m_enablers.find(attribute);
    StandaloneECEnablerPtr enabler = enablerIterator->second;
    IECInstancePtr customAttributeInstance;
    if (!enabler.IsValid())
        return customAttributeInstance;

    StandaloneECInstancePtr standaloneInstance = enabler->CreateInstance();
    return IECInstancePtr(standaloneInstance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr StandardCustomAttributesSchemaHolder::CreateCustomAttributeInstance(WCharCP attribute)
    {
    return GetHolder()->_CreateCustomAttributeInstance(attribute);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsSystemSchema (ECSchemaCR schema)
    {
    return schema.IsDefined (SYSTEMSCHEMA_CA_SCHEMA, SYSTEMSCHEMA_CA_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsDynamicSchema (ECSchemaCR schema)
    {
    return schema.IsDefined (SYSTEMSCHEMA_CA_SCHEMA, DYNAMICSCHEMA_CA_NAME);
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
            return /* ECOBJECTS_STATUS_DynamicSchemaCustomAttributeWasNotFound; */ ECOBJECTS_STATUS_Error;

        IECInstancePtr dynamicSchemaInstance = dynamicSchemaClass->GetDefaultStandaloneEnabler()->CreateInstance();
        return schema.SetCustomAttribute (*dynamicSchemaInstance);
        }

    if (!isDynamicExistingValue)
        return ECOBJECTS_STATUS_Success;

    if (schema.RemoveCustomAttribute (SYSTEMSCHEMA_CA_SCHEMA, DYNAMICSCHEMA_CA_NAME))
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

END_BENTLEY_ECOBJECT_NAMESPACE
