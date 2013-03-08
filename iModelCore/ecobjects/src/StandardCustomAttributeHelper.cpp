/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandardCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//*********************** DateTimeInfo *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
const DateTime::Kind DateTimeInfo::DEFAULT_KIND = DateTime::DATETIMEKIND_Unspecified;
const DateTime::Component DateTimeInfo::DEFAULT_COMPONENT = DateTime::DATETIMECOMPONENT_DateTime;
const DateTime::Info DateTimeInfo::s_default = DateTime::Info (DateTimeInfo::DEFAULT_KIND, DateTimeInfo::DEFAULT_COMPONENT);

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTimeInfo::DateTimeInfo ()
    : m_isKindNull (true), m_isComponentNull (true)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::TryGetDateTimeInfo (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::TryGetFrom (dateTimeInfo, dateTimeProperty);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::IsSystemSchema (ECSchemaCR schema)
    {
    return schema.IsDefined (SYSTEMSCHEMA_CA_NAME);
    }


END_BENTLEY_ECOBJECT_NAMESPACE
