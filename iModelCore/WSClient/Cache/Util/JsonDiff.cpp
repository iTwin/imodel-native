/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonDiff.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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

        RapidJsonValueCR oldValue = oldJson[newMemberItr->name.GetString()];

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
        for (auto oldMemberItr = oldJson.MemberBegin(); oldMemberItr != oldJson.MemberEnd(); ++oldMemberItr)
            {
            if (!newJson.HasMember(oldMemberItr->name.GetString()))
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
