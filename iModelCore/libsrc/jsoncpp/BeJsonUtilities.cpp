/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJsonUtilities.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Logging/bentleylogging.h>
#include <inttypes.h>

#define BEJSONCPP_LOGD(...) NativeLogging::LoggingManager::GetLogger(L"BeJsonCpp")->debugv (__VA_ARGS__)
#define BEJSONCPP_LOGE(...) NativeLogging::LoggingManager::GetLogger(L"BeJsonCpp")->errorv (__VA_ARGS__)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeJsonUtilities::HasRequiredMembers(JsonValueCR valueObj, Utf8CP const* requiredMembers)
    {
    if (!valueObj.isObject())
        return false;

    if (NULL != requiredMembers)
        {
        for (Utf8CP const* memberName = requiredMembers; *memberName; memberName++)
            {
            if (!valueObj.isMember(*memberName))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeJsonUtilities::HasOnlyExpectedMembers(JsonValueCR valueObj, Utf8CP const* requiredMembers, Utf8CP const* optionalMembers)
    {
    if (!valueObj.isObject())
        return false;

    Json::Value expectedMembersObj(Json::objectValue);

    if (NULL != requiredMembers)
        {
        for (Utf8CP const* memberName = requiredMembers; *memberName; memberName++)
            {
            if (!valueObj.isMember(*memberName))
                return false;

            expectedMembersObj[*memberName] = true;
            }
        }

    if (NULL != optionalMembers)
        {
        for (Utf8CP const* memberName = optionalMembers; *memberName; memberName++)
            expectedMembersObj[*memberName] = false;
        }

    Json::Value::Members valueMemberNames = valueObj.getMemberNames();

    for (size_t i = 0; i < valueMemberNames.size(); i++)
        {
        // all members in valueObj should be in expectedMembersObj
        if (!expectedMembersObj.isMember(valueMemberNames[i]))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeJsonUtilities::IsValidObject(Utf8CP objectName, JsonValueCR objectValue, Utf8CP const* requiredMembers, Utf8CP const* optionalMembers)
    {
    if (!objectValue.isObject())
        {
        BEJSONCPP_LOGE("JSON ERROR: \"%hs\" value is not of type object", objectName);
        BeAssert(false);
        return false;
        }

#if defined (NDEBUG)
    bool debugRequest = objectValue.isMember("debug");
#else
    bool debugRequest = true;
#endif
    bool isValid = true;

    if (debugRequest)
        {
        Json::Value expectedMembersObj(Json::objectValue);

        if (NULL != requiredMembers)
            {
            for (Utf8CP const* memberName = requiredMembers; *memberName; memberName++)
                {
                if (!objectValue.isMember(*memberName))
                    {
                    BEJSONCPP_LOGE("JSON ERROR: required member \"%hs\" not found on \"%hs\" object", *memberName, objectName);
                    isValid = false;
                    BeAssert(isValid);
                    }

                expectedMembersObj[*memberName] = true;
                }
            }

        if (NULL != optionalMembers)
            {
            for (Utf8CP const* memberName = optionalMembers; *memberName; memberName++)
                expectedMembersObj[*memberName] = false;
            }

        Json::Value::Members objectValueMemberNames = objectValue.getMemberNames();

        for (size_t i = 0; i < objectValueMemberNames.size(); i++)
            {
            // all members in objectValue should be in expectedMembersObj
            if (!expectedMembersObj.isMember(objectValueMemberNames[i]))
                {
                BEJSONCPP_LOGE("JSON ERROR: member \"%hs\" not expected on \"%hs\" object", objectValueMemberNames[i].c_str(), objectName);
                isValid = false;
                BeAssert(isValid);
                }
            }

        if (!isValid)
            {
            BEJSONCPP_LOGE("JSON ERROR: \"%hs\" object is not valid", objectName);

            if (NULL != requiredMembers)
                {
                Utf8CP header = *requiredMembers ? "  === Required Members ===" : "  === No required members ===";
                BEJSONCPP_LOGE(header);

                for (Utf8CP const* memberName = requiredMembers; *memberName; memberName++)
                    BEJSONCPP_LOGE("    %hs", *memberName);
                }

            if (NULL != optionalMembers)
                {
                Utf8CP header = *optionalMembers ? "  === Optional Members ===" : "  === No optional members ===";
                BEJSONCPP_LOGE(header);

                for (Utf8CP const* memberName = optionalMembers; *memberName; memberName++)
                    BEJSONCPP_LOGE("    %hs", *memberName);
                }

            BEJSONCPP_LOGE("  === Actual Members ===");
            for (size_t i = 0; i < objectValueMemberNames.size(); i++)
                BEJSONCPP_LOGE("    %hs", objectValueMemberNames[i].c_str());

            Json::FastWriter writer;
            BEJSONCPP_LOGE("  === Dump of \"%hs\" object ===", objectName);
            BEJSONCPP_LOGE(writer.write(objectValue).c_str());
            }
        else
            {
            Json::FastWriter writer;
            BEJSONCPP_LOGD("\"%hs\" object is valid, dump below...", objectName);
            BEJSONCPP_LOGD(writer.write(objectValue).c_str());
            }

        return isValid;
        }

    // for production builds, default is a less expensive check
    return BeJsonUtilities::HasRequiredMembers(objectValue, requiredMembers);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value BeJsonUtilities::StringValueFromInt64(int64_t lld)
    {
    char buffer[32];
    sprintf(buffer, "%" PRId64, lld);
    return Json::Value(buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t BeJsonUtilities::UInt64FromValue(JsonValueCR value, uint64_t defaultOnError)
    {
    if (value.isNull())
        return defaultOnError;

    if (value.isIntegral())
        return value.asUInt64();

    // elementIds are usually strings in JavaScript because of UInt64 issues
    if (value.isString())
        {
        uint64_t returnValueInt64 = defaultOnError;
        sscanf(value.asCString(), "%" PRIu64, &returnValueInt64);
        return returnValueInt64;
        }

    return defaultOnError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t BeJsonUtilities::Int64FromValue(JsonValueCR value, int64_t defaultOnError)
    {
    if (value.isNull())
        return defaultOnError;

    if (value.isIntegral())
        return value.asInt64();

    // elementIds are usually strings in JavaScript because of UInt64 issues
    if (value.isString())
        {
        int64_t returnValueInt64 = defaultOnError;
        sscanf(value.asCString(), "%" PRId64, &returnValueInt64);
        return returnValueInt64;
        }

    return defaultOnError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vincas.Razma                    07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime BeJsonUtilities::DateTimeFromValue(JsonValueCR value)
    {
    Utf8CP str = CStringFromStringValue(value, nullptr);
    if (Utf8String::IsNullOrEmpty(str))
        {
        return DateTime();
        }

    DateTime dateTime;
    if (SUCCESS != DateTime::FromString(dateTime, str))
        {
        return DateTime();
        }
    return dateTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BeJsonUtilities::CStringFromStringValue(JsonValueCR stringValue, Utf8CP defaultCString)
    {
    if (stringValue.isNull())
        return defaultCString;

    if (stringValue.isString())
        return stringValue.asCString();

    return defaultCString;
    }
