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
//static
DateTime::Info const& DateTimeInfo::GetDefault ()
    {
    return s_default;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StandardCustomAttributeHelper::TryGetDateTimeInfo (DateTimeInfo& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    return DateTimeInfoAccessor::TryGetFrom (dateTimeInfo, dateTimeProperty);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
