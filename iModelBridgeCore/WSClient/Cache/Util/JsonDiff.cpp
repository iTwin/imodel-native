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
JsonDiff::JsonDiff(bool copyValues, bool ignoreDeletedProperties) :
m_copyValues(copyValues),
m_ignoreDeletedProperties(ignoreDeletedProperties)
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
BentleyStatus JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonDocumentR jsonOut)
    {
    return GetChanges(oldJson, newJson, jsonOut, jsonOut.GetAllocator());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonValueR jsonOut, rapidjson::Value::AllocatorType& allocator)
    {
    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    // Find additions, modifications
    for (auto newMemberItr = newJson.MemberBegin(); newMemberItr != newJson.MemberEnd(); ++newMemberItr)
        {
        auto& oldValue = oldJson[newMemberItr->name.GetString()];

        if (!ValuesEqual(oldValue, newMemberItr->value, false))
            {
            AddMember(jsonOut, newMemberItr->name, newMemberItr->value, allocator);
            }
        }

    // Find deletions
    if (!m_ignoreDeletedProperties)
        {
        if (std::distance(oldJson.MemberBegin(), oldJson.MemberEnd()) != std::distance(newJson.MemberBegin(), newJson.MemberEnd()))
            {
            for (auto oldMemberItr = oldJson.MemberBegin(); oldMemberItr != oldJson.MemberEnd(); ++oldMemberItr)
                {
                auto& newValue = newJson[oldMemberItr->name.GetString()];
                if (newValue.IsNull())
                    {
                    AddMember(jsonOut, oldMemberItr->name, newValue, allocator);
                    }
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::AddMember(RapidJsonValueR jsonOut, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator)
    {
    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    Value nameToAdd(kStringType);
    CopyValues(name, nameToAdd, allocator);

    Value valueToAdd(value.GetType());

    switch (value.GetType())
        {
        case kObjectType:
            break;

        case kArrayType:
            CopyValues(value, valueToAdd, allocator);
            break;

        case kStringType:
            if (m_copyValues)
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

    jsonOut.RemoveMember(nameToAdd.GetString());
    jsonOut.AddMember(nameToAdd, valueToAdd, allocator);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonDiff::CopyValues(RapidJsonValueCR from, RapidJsonValueR to, rapidjson::Value::AllocatorType& allocator)
    {
    switch (from.GetType())
        {
        case kObjectType:
            for (auto memberFromItr = from.MemberBegin(); memberFromItr != from.MemberEnd(); ++memberFromItr)
                {
                Value nameToAdd(kStringType);
                CopyValues(memberFromItr->name, nameToAdd, allocator);

                Value valueToAdd(memberFromItr->value.GetType());
                CopyValues(memberFromItr->value, valueToAdd, allocator);

                to.AddMember(nameToAdd, valueToAdd, allocator);
                }
            break;

        case kArrayType:
            for (SizeType i = 0; i < from.Size(); i++)
                {
                Value valueToAdd(from[i].GetType());

                CopyValues(from[i], valueToAdd, allocator);

                to.PushBack(valueToAdd, allocator);
                }
            break;

        case kStringType:
            if (m_copyValues)
                {
                to.SetString(from.GetString(), from.GetStringLength(), allocator);
                }
            else
                {
                to.SetString(from.GetString(), from.GetStringLength());
                }
            break;

        case kNumberType:
            to.SetInt(from.GetInt());
            break;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::ValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2, bool deep)
    {
    if (value1.GetType() != value2.GetType())
        {
        return false;
        }

    switch (value1.GetType())
        {
        case kObjectType:
            if (deep)
                {
                return ObjectValuesEqual(value1, value2, true);
                }
            else
                {
                return true;
                }

        case kArrayType:
            return ArrayValuesEqual(value1, value2);

        case kStringType:
            return StringValuesEqual(value1, value2);

        case kNumberType:
            return value1.GetInt() == value2.GetInt();

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
bool JsonDiff::ObjectValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2, bool deep)
    {
    if (std::distance(value1.MemberBegin(), value1.MemberEnd()) != 
        std::distance(value2.MemberBegin(), value2.MemberEnd()))
        {
        return false;
        }

    for (auto member1Itr = value1.MemberBegin(); member1Itr != value1.MemberEnd(); ++member1Itr)
        {
        auto& value2Value = value2[member1Itr->name.GetString()];

        if (!ValuesEqual(member1Itr->value, value2Value, deep))
            {
            return false;
            }
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::ArrayValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    if (value1.Size() != value2.Size())
        {
        return false;
        }

    for (SizeType i = 0; i < value1.Size(); i++)
        {
        if (!ValuesEqual(value1[i], value2[i], true))
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
