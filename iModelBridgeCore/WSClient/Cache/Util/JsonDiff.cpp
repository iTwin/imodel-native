/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonDiff.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/JsonDiff.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace rapidjson;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                      
+---------------+---------------+---------------+---------------+---------------+------*/
JsonDiff::JsonDiff()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                        
+---------------+---------------+---------------+---------------+---------------+------*/
JsonDiff::~JsonDiff()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonDocumentR jsonOut, bool copyValues)
    {
    return GetChanges(oldJson, newJson, jsonOut, jsonOut.GetAllocator(), copyValues);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonValueR jsonOut, rapidjson::Value::AllocatorType& allocator, bool copyValues)
    {
    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    // Find additions, modifications
    for (auto newMemberItr = newJson.MemberBegin(); newMemberItr != newJson.MemberEnd(); ++newMemberItr)
        {
        auto& oldValue = oldJson[newMemberItr->name.GetString()];

        if (!ValuesShallowEqual(oldValue, newMemberItr->value))
            {
            AddMember(jsonOut, newMemberItr->name, newMemberItr->value, allocator, copyValues);
            }
        }

    // Find deletions
    if (std::distance(oldJson.MemberBegin(), oldJson.MemberEnd()) != std::distance(newJson.MemberBegin(), newJson.MemberEnd()))
        {
        for (auto oldMemberItr = oldJson.MemberBegin(); oldMemberItr != oldJson.MemberEnd(); ++oldMemberItr)
            {
            auto& newValue = newJson[oldMemberItr->name.GetString()];
            if (newValue.IsNull())
                {
                AddMember(jsonOut, oldMemberItr->name, newValue, allocator, copyValues);
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::AddMember(RapidJsonValueR jsonOut, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator, bool copyValues)
    {
    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    Value nameToAdd(kStringType);

    if (copyValues)
        {
        nameToAdd.SetString(name.GetString(), name.GetStringLength(), allocator);
        }
    else
        {
        nameToAdd.SetString(name.GetString(), name.GetStringLength());
        }

    Value valueToAdd(value.GetType());

    switch (value.GetType())
        {
        case kObjectType:
            break;

        case kArrayType:
            //value.Accept(valueToAdd);
            break;

        case kStringType:
            if (copyValues)
                {
                valueToAdd.SetString(value.GetString(), value.GetStringLength(), allocator);
                }
            else
                {
                valueToAdd.SetString(value.GetString(), value.GetStringLength());
                }
            break;

        case kNumberType:
            valueToAdd.SetInt(value.GetInt());
            break;
        }

    jsonOut.AddMember(nameToAdd, valueToAdd, allocator);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::ValuesShallowEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    if (value1.GetType() != value2.GetType())
        {
        return false;
        }

    switch (value1.GetType())
        {
        case kObjectType:
            return true;

        case kArrayType:
            return ArrayValuesEqual(value1, value2);

        case kStringType:
            return StringValuesEqual(value1, value2);

        case kNumberType:
            return value1.GetInt() == value2.GetInt();
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::ArrayValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    // BLI: WIP

    if (value1.Size() != value2.Size())
        {
        return false;
        }

    for (SizeType i = 0; i <= value1.Size(); i++)
        {
        if (!ValuesShallowEqual(value1[i], value2[i]))
            {
            return false;
            }         
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::StringValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    const SizeType len1 = value1.GetStringLength();
    const SizeType len2 = value2.GetStringLength();
    if (len1 != len2)
        {
        return false;
        }

    const UTF8<>::Ch* const str1 = value1.GetString();
    const UTF8<>::Ch* const str2 = value2.GetString();
    if (str1 == str2)
        {
        return true;
        } // fast path for constant string

    return (std::memcmp(str1, str2, sizeof(UTF8<>::Ch)* len1) == 0);
    }