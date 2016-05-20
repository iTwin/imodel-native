/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "JsonUtil.h"
#include <rapidjson/prettywriter.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace rapidjson;

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::RemoveECMembers(JsonValueR instanceJson)
    {
    for (auto& memberName : instanceJson.getMemberNames())
        {
        if (!memberName.empty() && '$' == memberName[0])
            {
            instanceJson.removeMember(memberName);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::RemoveECMembers(RapidJsonValueR instanceJson)
    {
    bvector<Utf8CP> membersToRemove;
    for (auto it = instanceJson.MemberBegin(); it != instanceJson.MemberEnd(); ++it)
        {
        if (0 != it->name.GetStringLength() && '$' == it->name.GetString()[0])
            {
            membersToRemove.push_back(it->name.GetString());
            }
        }
    for (auto& memberName : membersToRemove)
        {
        instanceJson.RemoveMember(memberName);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::ToRapidJson(JsonValueCR source, RapidJsonDocumentR target)
    {
    auto sourceStr = Json::FastWriter::ToString(source);
    target.Parse<0>(sourceStr.c_str());
    BeAssert(!target.HasParseError());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::ToJsonValue(RapidJsonValueCR source, JsonValueR target)
    {
    rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
    rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);
    source.Accept(writer);

    auto sourceStr = buffer.GetString();

    target = Json::nullValue;
    bool success = Json::Reader::Parse(sourceStr, target);
    BeAssert(success);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::DeepCopy(RapidJsonValueCR source, RapidJsonDocumentR target)
    {
    CopyValues(source, target, target.GetAllocator());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonUtil::CopyValues(RapidJsonValueCR source, RapidJsonValueR target, rapidjson::Value::AllocatorType& allocator, bool reuseStrings)
    {
    switch (source.GetType())
        {
        case kObjectType:
            {
            bool targetIsEmpty = false;
            if (kObjectType != target.GetType())
                {
                targetIsEmpty = true;
                target.SetObject();
                }

            for (auto memberSourceItr = source.MemberBegin(); memberSourceItr != source.MemberEnd(); ++memberSourceItr)
                {
                Value nameToAdd(kStringType);
                CopyValues(memberSourceItr->name, nameToAdd, allocator);

                Value valueToAdd(memberSourceItr->value.GetType());
                CopyValues(memberSourceItr->value, valueToAdd, allocator);

                if (!targetIsEmpty)
                    {
                    target.RemoveMember(nameToAdd.GetString());
                    }

                target.AddMember(nameToAdd, valueToAdd, allocator);
                }
            }
            break;

        case kArrayType:
            target.SetArray();
            for (SizeType i = 0; i < source.Size(); i++)
                {
                Value valueToAdd(source[i].GetType());

                CopyValues(source[i], valueToAdd, allocator);

                target.PushBack(valueToAdd, allocator);
                }
            break;

        case kStringType:
            if (reuseStrings)
                {
                target.SetString(source.GetString(), source.GetStringLength());
                }
            else
                {
                target.SetString(source.GetString(), source.GetStringLength(), allocator);
                }
            break;

        case kNumberType:
            {
            if (source.IsInt())
                {
                target.SetInt(source.GetInt());
                break;
                }
            if (source.IsDouble())
                {
                target.SetDouble(source.GetDouble());
                break;
                }
            if (source.IsInt64())
                {
                target.SetInt64(source.GetInt64());
                break;
                }
            if (source.IsUint())
                {
                target.SetUint(source.GetUint());
                break;
                }
            if (source.IsUint64())
                {
                target.SetUint64(source.GetUint64());
                break;
                }
            BeAssert(false);
            }
        case kTrueType:
            target.SetBool(true);
            break;
        case kFalseType:
            target.SetBool(false);
            break;
        case kNullType:
            target.SetNull();
            break;
        default:
            BeAssert(false);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonUtil::ToStyledString(RapidJsonValueCR value)
    {
    if (value.IsNull())
        {
        return "null";
        }
    rapidjson::GenericStringBuffer<UTF8<>> buffer;
    rapidjson::PrettyWriter<rapidjson::GenericStringBuffer<UTF8<>>> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonUtil::AreValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b)
    {
    if (a.GetType() != b.GetType())
        {
        return false;
        }

    switch (a.GetType())
        {
        case kObjectType:
            return ObjectValuesEqual(a, b);

        case kArrayType:
            return ArrayValuesEqual(a, b);

        case kStringType:
            return StringValuesEqual(a, b);

        case kNumberType:
            // TODO: update rapidjson lib to newer version that supports native comparison
            BeAssert(
                (!a.IsInt64() && !a.IsUint64() || a.IsInt() || a.IsUint()) &&
                (!b.IsInt64() && !b.IsUint64() || b.IsInt() || b.IsUint()) &&
                "64 bit integer comparison not supported. 64 bit integers should be saved as string in JSON");

            // Convert values to double and compare
            return a.GetDouble() == b.GetDouble();
        case kTrueType:
        case kFalseType:
        case kNullType:
            return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonUtil::ObjectValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b)
    {
    if (std::distance(a.MemberBegin(), a.MemberEnd()) !=
        std::distance(b.MemberBegin(), b.MemberEnd()))
        {
        return false;
        }

    for (auto member1Itr = a.MemberBegin(); member1Itr != a.MemberEnd(); ++member1Itr)
        {
        if (!b.HasMember(member1Itr->name.GetString()))
            return false;

        auto& bValue = b[member1Itr->name.GetString()];
        if (!AreValuesEqual(member1Itr->value, bValue))
            return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonUtil::ArrayValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b)
    {
    if (a.Size() != b.Size())
        {
        return false;
        }

    for (SizeType i = 0; i < a.Size(); i++)
        {
        if (!AreValuesEqual(a[i], b[i]))
            {
            return false;
            }
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonUtil::StringValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b)
    {
    const SizeType len1 = a.GetStringLength();
    const SizeType len2 = b.GetStringLength();
    if (len1 != len2)
        {
        return false;
        }

    const UTF8<>::Ch* const str1 = a.GetString();
    const UTF8<>::Ch* const str2 = b.GetString();
    if (str1 == str2)
        {
        return true;
        } // fast path for constant string

    return (std::memcmp(str1, str2, sizeof(UTF8<>::Ch)* len1) == 0);
    }
