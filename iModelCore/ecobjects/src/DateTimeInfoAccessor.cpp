/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DateTimeInfoAccessor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const DateTimeInfoAccessor::DATETIMEINFO_CLASSNAME = "DateTimeInfo";

//static
Utf8CP const DateTimeInfoAccessor::DATETIMEINFO_KIND_PROPERTYNAME = "DateTimeKind";
//static
Utf8CP const DateTimeInfoAccessor::DATETIMEINFO_COMPONENT_PROPERTYNAME = "DateTimeComponent";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const DateTimeInfoAccessor::DATETIMEKIND_UTC_STR = "Utc";
//static
WCharCP const DateTimeInfoAccessor::DATETIMEKIND_UTC_WSTR = L"Utc";
//static
Utf8CP const DateTimeInfoAccessor::DATETIMEKIND_UNSPECIFIED_STR = "Unspecified";
//static
WCharCP const DateTimeInfoAccessor::DATETIMEKIND_UNSPECIFIED_WSTR = L"Unspecified";
//static
Utf8CP const DateTimeInfoAccessor::DATETIMEKIND_LOCAL_STR = "Local";
//static
WCharCP const DateTimeInfoAccessor::DATETIMEKIND_LOCAL_WSTR = L"Local";
//static
Utf8CP const DateTimeInfoAccessor::DATETIMECOMPONENT_DATETIME_STR = "DateTime";
//static
WCharCP const DateTimeInfoAccessor::DATETIMECOMPONENT_DATETIME_WSTR = L"DateTime";
//static
Utf8CP const DateTimeInfoAccessor::DATETIMECOMPONENT_DATE_STR = "Date";
//static
WCharCP const DateTimeInfoAccessor::DATETIMECOMPONENT_DATE_WSTR = L"Date";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus DateTimeInfoAccessor::GetFrom (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    ArrayECPropertyCP arrayDateTimeProp = NULL;
    PRECONDITION ((dateTimeProperty.GetIsPrimitive () && dateTimeProperty.GetAsPrimitiveProperty ()->GetType () == PRIMITIVETYPE_DateTime) || 
                  ((arrayDateTimeProp = dateTimeProperty.GetAsArrayProperty ()) != NULL && arrayDateTimeProp->GetKind () == ARRAYKIND_Primitive && arrayDateTimeProp->GetPrimitiveElementType () == PRIMITIVETYPE_DateTime), ECOBJECTS_STATUS_DataTypeNotSupported);

    IECInstancePtr caInstance = dateTimeProperty.GetCustomAttribute (DATETIMEINFO_CLASSNAME);
    if (caInstance.IsNull())
        {
        //no CA found -> return a DateTimeInfo for which both Kind and Component are set to unset
        dateTimeInfo = DateTimeInfo ();
        return ECOBJECTS_STATUS_Success;
        }

    //Retrieve DateTimeKind
    ECValue caVal;
    ECObjectsStatus stat = caInstance->GetValue (caVal, DATETIMEINFO_KIND_PROPERTYNAME);
    if (stat != ECOBJECTS_STATUS_Success)
        {
        LogPropertyNotFoundError (DATETIMEINFO_KIND_PROPERTYNAME);
        return ECOBJECTS_STATUS_PropertyNotFound;
        }

    bool isKindNull = true;
    DateTime::Kind kind = DateTime::Kind::Unspecified;
    //parsing returns false in error case
    if (!TryParseKind (isKindNull, kind, caVal))
        {
        return ECOBJECTS_STATUS_ParseError;
        }

    //Retrieve DateTimeComponent
    caVal.Clear ();
    stat = caInstance->GetValue (caVal, DATETIMEINFO_COMPONENT_PROPERTYNAME);
    if (stat != ECOBJECTS_STATUS_Success)
        {
        LogPropertyNotFoundError (DATETIMEINFO_COMPONENT_PROPERTYNAME);
        return ECOBJECTS_STATUS_PropertyNotFound;
        }

    bool isComponentNull = true;
    DateTime::Component component = DateTime::Component::DateAndTime;
    //parsing returns false in error case
    if (!TryParseComponent (isComponentNull, component, caVal))
        {
        return ECOBJECTS_STATUS_ParseError;
        }
        
    dateTimeInfo = DateTimeInfo (isKindNull, kind, isComponentNull, component);
    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseKind (bool& isKindNull, DateTime::Kind& kind, ECValueCR ecValue)
    {
    PRECONDITION (ecValue.IsString (), false);

    if (ecValue.IsNull ())
        {
        isKindNull = true;
        return true;
        }

    if (ecValue.IsUtf8 ())
        {
        Utf8CP kindStr = ecValue.GetUtf8CP();
        return TryParseKind (isKindNull, kind, kindStr);
        }
    else
        {
        Utf16CP kindStr = ecValue.GetUtf16CP ();
        return TryParseKind (isKindNull, kind, kindStr);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseKind (bool& isKindNull, DateTime::Kind& kind, Utf8CP kindStr)
    {
    if (Utf8String::IsNullOrEmpty (kindStr))
        {
        isKindNull = true;
        return true;
        }

    isKindNull = false;
    if (BeStringUtilities::Stricmp (kindStr, DATETIMEKIND_UNSPECIFIED_STR) == 0)
        {
        kind = DateTime::Kind::Unspecified;
        return true;
        }
    else if (BeStringUtilities::Stricmp (kindStr, DATETIMEKIND_UTC_STR) == 0)
        {
        kind = DateTime::Kind::Utc;
        return true;
        }
    else if (BeStringUtilities::Stricmp (kindStr, DATETIMEKIND_LOCAL_STR) == 0)
        {
        kind = DateTime::Kind::Local;
        return true;
        }
    else
        {
        //wrong date time kind
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseKind (bool& isKindNull, DateTime::Kind& kind, Utf16CP kindStr)
    {
    if (kindStr == NULL || BeStringUtilities::Utf16Len (kindStr) == 0)
        {
        isKindNull = true;
        return true;
        }

    isKindNull = false;
    if (BeStringUtilities::CompareUtf16WChar (kindStr, DATETIMEKIND_UNSPECIFIED_WSTR) == 0)
        {
        kind = DateTime::Kind::Unspecified;
        return true;
        }
    else if (BeStringUtilities::CompareUtf16WChar (kindStr, DATETIMEKIND_UTC_WSTR) == 0)
        {
        kind = DateTime::Kind::Utc;
        return true;
        }
    else if (BeStringUtilities::CompareUtf16WChar (kindStr, DATETIMEKIND_LOCAL_WSTR) == 0)
        {
        kind = DateTime::Kind::Local;
        return true;
        }
    else
        {
        //wrong date time kind
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseComponent (bool& isComponentNull, DateTime::Component& component, ECValueCR ecValue)
    {
    PRECONDITION (ecValue.IsString (), false);

    if (ecValue.IsNull ())
        {
        isComponentNull = true;
        return true;
        }

    if (ecValue.IsUtf8 ())
        {
        return TryParseComponent (isComponentNull, component, ecValue.GetUtf8CP());
        }
    else
        {
        return TryParseComponent (isComponentNull, component, ecValue.GetUtf16CP ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseComponent (bool& isComponentNull, DateTime::Component& component, Utf8CP componentStr)
    {
    if (Utf8String::IsNullOrEmpty (componentStr))
        {
        isComponentNull = true;
        return true;
        }

    isComponentNull = false;
    if (BeStringUtilities::Stricmp (componentStr, DATETIMECOMPONENT_DATE_STR) == 0)
        {
        component = DateTime::Component::Date;
        return true;
        }
    else if (BeStringUtilities::Stricmp (componentStr, DATETIMECOMPONENT_DATETIME_STR) == 0)
        {
        component = DateTime::Component::DateAndTime;
        return true;
        }
    else
        {
        //wrong date time component
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseComponent (bool& isComponentNull, DateTime::Component& component, Utf16CP componentStr)
    {
    if (componentStr == NULL || BeStringUtilities::Utf16Len (componentStr) == 0)
        {
        isComponentNull = true;
        return true;
        }

    isComponentNull = false;
    if (BeStringUtilities::CompareUtf16WChar (componentStr, DATETIMECOMPONENT_DATE_WSTR) == 0)
        {
        component = DateTime::Component::Date;
        return true;
        }
    else if (BeStringUtilities::CompareUtf16WChar (componentStr, DATETIMECOMPONENT_DATETIME_WSTR) == 0)
        {
        component = DateTime::Component::DateAndTime;
        return true;
        }
    else
        {
        //wrong date time component
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
void DateTimeInfoAccessor::LogPropertyNotFoundError (Utf8CP propertyName)
    {
    LOG.errorv ("Property '%ls' not found in custom attribute class '%s'.", propertyName, DATETIMEINFO_CLASSNAME);
    BeAssert (false);
    }
END_BENTLEY_ECOBJECT_NAMESPACE
