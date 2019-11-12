/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/JsonDiff.h>
#include <WebServices/Cache/Util/JsonUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace rapidjson;

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonDiff::JsonDiff(int flags) :
m_avoidCopies(flags & Flags::DoNotCopyValues ? true : false),
m_findDeletions(flags & Flags::FindDeletions ? true : false)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonDiff::~JsonDiff()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonDocumentR jsonOut)
    {
    return GetChanges(oldJson, newJson, jsonOut, jsonOut.GetAllocator());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::GetChanges(RapidJsonValueCR oldJsonIn, RapidJsonValueCR newJsonIn, RapidJsonValueR jsonOut, rapidjson::Value::AllocatorType& allocator)
    {
    bool changesFound = false;

    RapidJsonValueCR oldJson = ValidateObject(oldJsonIn);
    RapidJsonValueCR newJson = ValidateObject(newJsonIn);

    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    // Find additions, modifications
    for (auto newMemberItr = newJson.MemberBegin(); newMemberItr != newJson.MemberEnd(); ++newMemberItr)
        {
        // null member added case
        if (newMemberItr->value.IsNull() &&
            !oldJson.HasMember(newMemberItr->name.GetString()))
            {
            changesFound = true;
            AddMember(jsonOut, newMemberItr->name, newMemberItr->value, allocator);
            continue;
            }

        static rapidjson::Value s_nullValue(kNullType);
        CharCP memberName = "";

        RapidJsonValueCR oldValue = HasMemberI(oldJson, newMemberItr->name.GetString(), memberName) ? oldJson[memberName] : s_nullValue;

        if (newMemberItr->value.GetType() == kObjectType)
            {
            bool needsAdding = true;

            Value temp(kObjectType);
            Value* outMember = &temp;

            if (jsonOut.HasMember(newMemberItr->name.GetString()))
                {
                needsAdding = false;
                outMember = &jsonOut[newMemberItr->name.GetString()];
                }

            if (GetChanges(oldValue, newMemberItr->value, *outMember, allocator))
                {
                changesFound = true;

                if (needsAdding)
                    {
                    AddMember(jsonOut, newMemberItr->name, temp, allocator);
                    }
                }
            }
        else if (!JsonUtil::AreValuesEqual(oldValue, newMemberItr->value))
            {
            changesFound = true;
            AddMember(jsonOut, newMemberItr->name, newMemberItr->value, allocator);
            }
        }

    // Find deletions
    if (m_findDeletions)
        {
        CharCP memberName = "";
        for (auto oldMemberItr = oldJson.MemberBegin(); oldMemberItr != oldJson.MemberEnd(); ++oldMemberItr)
            {
            if (!HasMemberI(newJson, oldMemberItr->name.GetString(), memberName))
                {
                changesFound = true;
                AddMember(jsonOut, oldMemberItr->name, Value(kNullType), allocator);
                }
            }
        }

    return changesFound;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::HasMemberI(RapidJsonValueCR json, CharCP searchMember, CharCP& memberOut)
    {
    bool isMemberFound = false;
    for (auto jsonIterator = json.MemberBegin(); jsonIterator != json.MemberEnd(); ++jsonIterator)
        {
        if (0 != BeStringUtilities::Stricmp(searchMember, jsonIterator->name.GetString()))
            continue;

        memberOut = jsonIterator->name.GetString();
        isMemberFound = true;
        break;
        }

    return isMemberFound;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::AddMember(RapidJsonValueR parent, RapidJsonValueR name, RapidJsonValueR value, rapidjson::Value::AllocatorType& allocator)
    {
    parent.RemoveMember(name.GetString());
    parent.AddMember(name, value, allocator);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::AddMember(RapidJsonValueR parent, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator)
    {
    Value nameToAdd(kStringType);
    CopyValues(name, nameToAdd, allocator);

    Value valueToAdd(value.GetType());
    CopyValues(value, valueToAdd, allocator);

    AddMember(parent, nameToAdd, valueToAdd, allocator);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::CopyValues(RapidJsonValueCR source, RapidJsonValueR target, rapidjson::Value::AllocatorType& allocator)
    {
    JsonUtil::CopyValues(source, target, allocator, m_avoidCopies);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR JsonDiff::ValidateObject(RapidJsonValueCR value)
    {
    using namespace rapidjson;

    if (!value.IsObject())
        {
        static const Value empty(kObjectType);
        return empty;
        }

    return value;
    }
