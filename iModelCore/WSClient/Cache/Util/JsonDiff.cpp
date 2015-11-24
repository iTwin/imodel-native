/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonDiff.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/JsonDiff.h>
#include "JsonUtil.h"

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

        auto& oldValue = oldJson[newMemberItr->name.GetString()];

        if (newMemberItr->value.GetType() == kObjectType)
            {
            bool needsAdding = false;
            auto& newParent = jsonOut[newMemberItr->name.GetString()];
            if (newParent.IsNull())
                {
                needsAdding = true;
                Value newPar(kObjectType);
                newParent = newPar;
                }

            if (GetChanges(oldValue, newMemberItr->value, newParent, allocator))
                {
                changesFound = true;

                if (needsAdding)
                    {
                    AddMember(jsonOut, newMemberItr->name, newParent, allocator);
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
    if (m_findDeletions &&
        std::distance(oldJson.MemberBegin(), oldJson.MemberEnd()) != std::distance(newJson.MemberBegin(), newJson.MemberEnd()))
        {
        for (auto oldMemberItr = oldJson.MemberBegin(); oldMemberItr != oldJson.MemberEnd(); ++oldMemberItr)
            {
            auto& newValue = newJson[oldMemberItr->name.GetString()];
            if (newValue.IsNull())
                {
                changesFound = true;
                AddMember(jsonOut, oldMemberItr->name, newValue, allocator);
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
        BeAssert(false && "Expecting object value");
        static const Value empty(kObjectType);
        return empty;
        }

    return value;
    }
