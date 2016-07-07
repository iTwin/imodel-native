/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeJsonCpp/BeJsonUtilities.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/DateTime.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/LocalState.h>
#include <json/json.h>

BEGIN_BENTLEY_NAMESPACE

typedef Json::Value& JsonValueR;
typedef Json::Value const& JsonValueCR;

//=======================================================================================
//! ILocalState wrapper that handles JSON - string conversion.
//  @bsiclass                                           Grigas.Petraitis        12/14
//=======================================================================================
struct IJsonLocalState : ILocalState
    {
    public:
    //! Saves the supplied JSON value as string in the local state
    virtual void SaveJsonValue(Utf8CP nameSpace, Utf8CP key, JsonValueCR value) {SaveValue(nameSpace, key, Json::FastWriter().write(value)); }

    //! Parses the stored serialized JSON value and returns it as JSON
    virtual Json::Value GetJsonValue(Utf8CP nameSpace, Utf8CP key) const 
        {
        Json::Value value;
        if (Json::Reader().parse(_GetValue(nameSpace, key), value, false))
            return value;
        return Json::nullValue;
        }
    };

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall     10/2012
//=======================================================================================
struct BeJsonUtilities
{
public:
    //! Returns true if all required members are present on the specified Json::Value object.
    //! @param[in] valueObj the Json::Value object to check
    //! @param[in] requiredMembers The member names to test. Specified as a C-style array of Utf8CP strings with NULL as last entry.
    static bool HasRequiredMembers(JsonValueCR valueObj, Utf8CP const* requiredMembers);

    //! Returns true if all required members are present on the specified Json::Value object AND the only other members are within
    //! the specified set of optional members. Note that this is a somewhat expensive method, so is intended for debugging JSON messages only.
    //! @param[in] valueObj the Json::Value object to check
    //! @param[in] requiredMembers The required member names to test. Specified as a C-style array of Utf8CP strings with NULL as last entry.
    //! @param[in] optionalMembers The optional member names that may or may not be present. Specified as a C-style array of Utf8CP strings with NULL as last entry.
    static bool HasOnlyExpectedMembers(JsonValueCR valueObj, Utf8CP const* requiredMembers, Utf8CP const* optionalMembers);

    //! Returns a Json::stringValue representation of the specified Int64.
    static Json::Value StringValueFromInt64(int64_t lld);

    //! Returns a UInt64 value from a Json::Value that may be integral or string.
    //! @param[in] value the source Json::Value
    //! @param[in] defaultOnError the UInt64 value to return if it is not possible to convert the source value to UInt64.
    static uint64_t UInt64FromValue(JsonValueCR value, uint64_t defaultOnError = 0);

    //! Returns an Int64 value from a Json::Value that may be integral or string.
    //! @param[in] value the source Json::Value
    //! @param[in] defaultOnError the Int64 value to return if it is not possible to convert the source value to Int64.
    static int64_t Int64FromValue(JsonValueCR value, int64_t defaultOnError = 0);

    //! Parse standard ISO8601 date from string value
    //! @param[in] value containing ISO8601 date string
    //! @return parsed DateTime or invalid DateTime if errors occurred
    static DateTime DateTimeFromValue(JsonValueCR value);

    //! Equivalent to HasRequiredMembers in production builds unless objectValue has a member named "debug".  In this case,
    //! the behavior is similar to HasOnlyExpectedMembers except there is more extensive logging.
    //! @param[in] objectName the name used for logging purposes.  Must not be NULL.
    //! @param[in] objectValue the Json::Value object to check
    //! @param[in] requiredMembers The required member names to test. Specified as a C-style array of Utf8CP strings with NULL as last entry.
    //! @param[in] optionalMembers The optional member names that may or may not be present. Specified as a C-style array of Utf8CP strings with NULL as last entry.
    static bool IsValidObject(Utf8CP objectName, JsonValueCR objectValue, Utf8CP const* requiredMembers, Utf8CP const* optionalMembers);

    //! Equivalent to Json::Value::get + Json::Value::asCString with proper NULL / error handling.  If a null value or non-string value is
    //! encountered, then the specified default string is returned.
    //! @param[in] stringValue the value that is expected to be a Json::stringValue
    //! @param[in] defaultCString the string to return if stringValue is not actually a Json::stringValue or stringValue is null.
    //! @return a pointer to the string within the stringValue or to the defaultCString parameter.  Therefore, attention must be
    //!         paid to the lifetime of stringValue and defaultCString.
    static Utf8CP CStringFromStringValue(JsonValueCR stringValue, Utf8CP defaultCString);
};

END_BENTLEY_NAMESPACE
