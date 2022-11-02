/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECObjectsStatus DateTimeInfoAccessor::GetFrom(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty)
    {
    PrimitiveArrayECPropertyCP arrayDateTimeProp = nullptr;
    PRECONDITION((dateTimeProperty.GetIsPrimitive() && dateTimeProperty.GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_DateTime) ||
        ((arrayDateTimeProp = dateTimeProperty.GetAsPrimitiveArrayProperty()) != nullptr && arrayDateTimeProp->GetKind() == ARRAYKIND_Primitive && arrayDateTimeProp->GetPrimitiveElementType() == PRIMITIVETYPE_DateTime), ECObjectsStatus::DataTypeNotSupported);

    IECInstancePtr caInstance = nullptr;

    {
    //Deal with old and new standard schemas. First try CA from new schema, then from old.
    caInstance = dateTimeProperty.GetCustomAttribute("CoreCustomAttributes", "DateTimeInfo");
    if (caInstance == nullptr)
        caInstance = dateTimeProperty.GetCustomAttribute("Bentley_Standard_CustomAttributes", "DateTimeInfo");
    }

    if (caInstance == nullptr)
        {
        //no CA found -> return without modifying the out parameter
        return ECObjectsStatus::Success;
        }

    //Retrieve DateTimeKind
    ECValue caVal;
    ECObjectsStatus stat = caInstance->GetValue(caVal, "DateTimeKind");
    if (stat != ECObjectsStatus::Success)
        {
        LogPropertyNotFoundError("DateTimeKind");
        return ECObjectsStatus::PropertyNotFound;
        }

    Nullable<DateTime::Kind> kind;
    //parsing returns false in error case
    if (!TryParseKind(kind, caVal))
        return ECObjectsStatus::ParseError;

    //Retrieve DateTimeComponent
    caVal.Clear();
    stat = caInstance->GetValue(caVal, "DateTimeComponent");
    if (stat != ECObjectsStatus::Success)
        {
        LogPropertyNotFoundError("DateTimeComponent");
        return ECObjectsStatus::PropertyNotFound;
        }

    Nullable<DateTime::Component> component;
    //parsing returns false in error case
    if (!TryParseComponent(component, caVal))
        return ECObjectsStatus::ParseError;

    if (component != nullptr)
        {
        if (component != DateTime::Component::DateAndTime && kind != nullptr)
            {
            LOG.errorv("Invalid 'DateTimeInfo' custom attribute on ECProperty '%s.%s'. DateTimeKind must remain unset unless DateTimeComponent is set to 'DateAndTime'.", dateTimeProperty.GetClass().GetFullName(), dateTimeProperty.GetName().c_str());
            return ECObjectsStatus::ParseError;
            }

        if (component == DateTime::Component::Date)
            {
            dateTimeInfo = DateTime::Info::CreateForDate();
            return ECObjectsStatus::Success;
            }

        if (component == DateTime::Component::TimeOfDay)
            {
            dateTimeInfo = DateTime::Info::CreateForTimeOfDay();
            return ECObjectsStatus::Success;
            }

        BeAssert(component == DateTime::Component::DateAndTime);
        }

    if (kind != nullptr)
        dateTimeInfo = DateTime::Info::CreateForDateTime(kind.Value());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseKind(Nullable<DateTime::Kind>& kind, ECValueCR ecValue)
    {
    kind = nullptr;

    if (ecValue.IsNull())
        return true;

    /* In case DateTimeKind property is switched to an int enum
    if (ecValue.IsInteger())
        {
        switch (ecValue.GetInteger())
            {
                case 0:
                    kind = DateTime::Kind::Unspecified;
                    return true;
                case 1:
                    kind = DateTime::Kind::Local;
                    return true;
                case 2:
                    kind = DateTime::Kind::Utc;
                    return true;
                default:
                    //wrong enum value
                    return false;
            }
        }
    */
    if (!ecValue.IsString())
        return false;

    if (ecValue.IsUtf8())
        {
        Utf8CP kindStr = ecValue.GetUtf8CP();
        if (Utf8String::IsNullOrEmpty(kindStr))
            return true;

        if (BeStringUtilities::StricmpAscii(kindStr, "Unspecified") == 0)
            {
            kind = DateTime::Kind::Unspecified;
            return true;
            }

        if (BeStringUtilities::StricmpAscii(kindStr, "Utc") == 0)
            {
            kind = DateTime::Kind::Utc;
            return true;
            }

        if (BeStringUtilities::StricmpAscii(kindStr, "Local") == 0)
            {
            kind = DateTime::Kind::Local;
            return true;
            }

        return false;
        }

    Utf16CP kindStr = ecValue.GetUtf16CP();
    if (kindStr == nullptr || BeStringUtilities::Utf16Len(kindStr) == 0)
        return true;

    if (BeStringUtilities::CompareUtf16WChar(kindStr, L"Unspecified") == 0)
        {
        kind = DateTime::Kind::Unspecified;
        return true;
        }

    if (BeStringUtilities::CompareUtf16WChar(kindStr, L"Utc") == 0)
        {
        kind = DateTime::Kind::Utc;
        return true;
        }

    if (BeStringUtilities::CompareUtf16WChar(kindStr, L"Local") == 0)
        {
        kind = DateTime::Kind::Local;
        return true;
        }

    return false;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DateTimeInfoAccessor::TryParseComponent(Nullable<DateTime::Component>& component, ECValueCR ecValue)
    {
    component = nullptr;
    if (ecValue.IsNull())
        return true;

    /* In case DateTimeComponent property is switched to an int enum
    if (ecValue.IsInteger())
        {
        switch (ecValue.GetInteger())
            {
                case 0:
                    component = DateTime::Component::DateAndTime;
                    return true;
                case 1:
                    component = DateTime::Component::Date;
                    return true;
                default:
                    //wrong enum value
                    return false;
            }
        }
    */
    if (!ecValue.IsString())
        return false;

    if (ecValue.IsUtf8())
        {
        Utf8CP componentStr = ecValue.GetUtf8CP();
        if (Utf8String::IsNullOrEmpty(componentStr))
            return true;

        if (BeStringUtilities::StricmpAscii(componentStr, "DateTime") == 0)
            {
            component = DateTime::Component::DateAndTime;
            return true;
            }

        if (BeStringUtilities::StricmpAscii(componentStr, "Date") == 0)
            {
            component = DateTime::Component::Date;
            return true;
            }

        if (BeStringUtilities::StricmpAscii(componentStr, "TimeOfDay") == 0)
            {
            component = DateTime::Component::TimeOfDay;
            return true;
            }

        return false;
        }

    Utf16CP componentStr = ecValue.GetUtf16CP();
    if (componentStr == nullptr || BeStringUtilities::Utf16Len(componentStr) == 0)
        return true;

    if (BeStringUtilities::CompareUtf16WChar(componentStr, L"DateTime") == 0)
        {
        component = DateTime::Component::DateAndTime;
        return true;
        }

    if (BeStringUtilities::CompareUtf16WChar(componentStr, L"Date") == 0)
        {
        component = DateTime::Component::Date;
        return true;
        }

    if (BeStringUtilities::CompareUtf16WChar(componentStr, L"TimeOfDay") == 0)
        {
        component = DateTime::Component::TimeOfDay;
        return true;
        }

    return false;
    }



//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void DateTimeInfoAccessor::LogPropertyNotFoundError(Utf8CP propertyName)
    {
    LOG.errorv("Property '%s' not found in custom attribute class 'DateTimeInfo'.", propertyName);
    BeAssert(false);
    }
END_BENTLEY_ECOBJECT_NAMESPACE
