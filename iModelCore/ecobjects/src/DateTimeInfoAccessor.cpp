/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DateTimeInfoAccessor.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
WCharCP const DateTimeInfoAccessor::DATETIMEINFO_CLASSNAME = L"DateTimeInfo";
//static
WCharCP const DateTimeInfoAccessor::DATETIMEINFO_KIND_PROPERTYNAME = L"DateTimeKind";
//static
WCharCP const DateTimeInfoAccessor::DATETIMEINFO_COMPONENT_PROPERTYNAME = L"DateTimeComponent";

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryGetFrom (DateTimeInfo& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    PrimitiveECPropertyP dateTimePropertyPrim = dateTimeProperty.GetAsPrimitiveProperty ();
    PRECONDITION (dateTimePropertyPrim != NULL && dateTimePropertyPrim->GetType () == PRIMITIVETYPE_DateTime, false);

    IECInstancePtr caInstance = dateTimeProperty.GetCustomAttribute (DATETIMEINFO_CLASSNAME);
    if (caInstance.IsNull())
        return false;

    //Retrieve DateTimeKind
    ECValue caVal;
    ECObjectsStatus stat = caInstance->GetValue (caVal, DATETIMEINFO_KIND_PROPERTYNAME);
    if (stat != ECOBJECTS_STATUS_Success)
        {
        ECObjectsLogger::Log ()->errorv (L"Property '%ls' not found in custom attribute class '%ls'.", DATETIMEINFO_KIND_PROPERTYNAME, DATETIMEINFO_CLASSNAME);
        BeAssert (false);
        return false;
        }

    bool isKindNull = true;
    DateTime::Kind kind = DateTime::DATETIMEKIND_Unspecified;
    //parsing returns false in error case
    if (!TryParseKind (isKindNull, kind, caVal))
        {
        return false;
        }

    //Retrieve DateTimeComponent
    caVal.Clear ();
    stat = caInstance->GetValue (caVal, DATETIMEINFO_COMPONENT_PROPERTYNAME);
    if (stat != ECOBJECTS_STATUS_Success)
        {
        ECObjectsLogger::Log ()->errorv (L"Property '%ls' not found in custom attribute class '%ls'.", DATETIMEINFO_KIND_PROPERTYNAME, DATETIMEINFO_CLASSNAME);
        BeAssert (false);
        return false;
        }

    bool isComponentNull = true;
    DateTime::Component component = DateTime::DATETIMECOMPONENT_DateTime;
    //parsing returns false in error case
    if (!TryParseComponent (isComponentNull, component, caVal))
        {
        return false;
        }
        
    //if both meta data items are unset, consider this as if the CA wasn't specified
    if (isKindNull && isComponentNull)
        {
        return false;
        }

    dateTimeInfo = DateTimeInfo (isKindNull, kind, isComponentNull, component);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseKind (bool& isKindNull, DateTime::Kind& kind, ECValueCR ecValue)
    {
    if (ecValue.IsNull ())
        {
        isKindNull = true;
        return true;
        }

    WCharCP kindStr = ecValue.GetString ();
    if (WString::IsNullOrEmpty (kindStr))
        {
        isKindNull = true;
        return true;
        }

    isKindNull = false;
    if (BeStringUtilities::Wcsicmp (kindStr, L"Unspecified") == 0)
        {
        kind = DateTime::DATETIMEKIND_Unspecified;
        return true;
        }
    else if (BeStringUtilities::Wcsicmp (kindStr, L"Utc") == 0)
        {
        kind = DateTime::DATETIMEKIND_Utc;
        return true;
        }
    else if (BeStringUtilities::Wcsicmp (kindStr, L"Local") == 0)
        {
        kind = DateTime::DATETIMEKIND_Local;
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
    if (ecValue.IsNull ())
        {
        isComponentNull = true;
        return true;
        }

    WCharCP componentStr = ecValue.GetString ();
    if (WString::IsNullOrEmpty (componentStr))
        {
        isComponentNull = true;
        return true;
        }

    isComponentNull = false;
    if (BeStringUtilities::Wcsicmp (componentStr, L"Date") == 0)
        {
        component = DateTime::DATETIMECOMPONENT_Date;
        return true;
        }
    else if (BeStringUtilities::Wcsicmp (componentStr, L"DateTime") == 0)
        {
        component = DateTime::DATETIMECOMPONENT_DateTime;
        return true;
        }
    else
        {
        //wrong date time component
        return false;
        }
    }

END_BENTLEY_ECOBJECT_NAMESPACE
