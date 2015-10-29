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
bool JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonDocumentR jsonOut)
    {
    return GetChanges(oldJson, newJson, jsonOut, jsonOut.GetAllocator());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                
+---------------+---------------+---------------+---------------+---------------+------*/
 bool JsonDiff::GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonValueR jsonOut, rapidjson::Value::AllocatorType& allocator)
    {
    bool changesFound = false;

    if (jsonOut.IsNull())
        {
        jsonOut.SetObject();
        }

    // Find additions, modifications
    for (auto newMemberItr = newJson.MemberBegin(); newMemberItr != newJson.MemberEnd(); ++newMemberItr)
        {
        // null member added case
        if (newMemberItr->value.IsNull())
            {
            if (!oldJson.HasMember(newMemberItr->name.GetString()))
                {
                changesFound = true;
                AddMember(jsonOut, newMemberItr->name, newMemberItr->value, allocator);
                continue;
                }
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
        else if (!ValuesEqual(oldValue, newMemberItr->value))
            {
            changesFound = true;
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
                    changesFound = true;
                    AddMember(jsonOut, oldMemberItr->name, newValue, allocator);
                    }
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
    switch (source.GetType())
        {
        case kObjectType:
            for (auto memberSourceItr = source.MemberBegin(); memberSourceItr != source.MemberEnd(); ++memberSourceItr)
                {
                Value nameToAdd(kStringType);
                CopyValues(memberSourceItr->name, nameToAdd, allocator);

                Value valueToAdd(memberSourceItr->value.GetType());
                CopyValues(memberSourceItr->value, valueToAdd, allocator);

                target.AddMember(nameToAdd, valueToAdd, allocator);
                }
            break;

        case kArrayType:
            for (SizeType i = 0; i < source.Size(); i++)
                {
                Value valueToAdd(source[i].GetType());

                CopyValues(source[i], valueToAdd, allocator);

                target.PushBack(valueToAdd, allocator);
                }
            break;

        case kStringType:
            if (m_copyValues)
                {
                target.SetString(source.GetString(), source.GetStringLength(), allocator);
                }
            else
                {
                target.SetString(source.GetString(), source.GetStringLength());
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
                target.SetDouble(source.GetInt());
                break;
                }
            if (source.IsInt64())
                {
                target.SetInt64(source.GetInt());
                break;
                }
            if (source.IsUint())
                {
                target.SetUint(source.GetInt());
                break;
                }
            if (source.IsUint64())
                {
                target.SetUint64(source.GetInt());
                break;
                }
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDiff::ValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    if (value1.GetType() != value2.GetType())
        {
        return false;
        }

    switch (value1.GetType())
        {
        case kObjectType:
            return ObjectValuesEqual(value1, value2);

        case kArrayType:
            return ArrayValuesEqual(value1, value2);

        case kStringType:
            return StringValuesEqual(value1, value2);

        case kNumberType:
            {
            if (value1.IsInt())
                return value1.GetInt() == value2.GetInt();
            if (value1.IsDouble())
                return value1.GetDouble() == value2.GetDouble();
            if (value1.IsInt64())
                return value1.GetInt64() == value2.GetInt64();
            if (value1.IsUint())
                return value1.GetUint() == value2.GetUint();
            if (value1.IsUint64())
                return value1.GetUint64() == value2.GetUint64();
            }
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
bool JsonDiff::ObjectValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2)
    {
    if (std::distance(value1.MemberBegin(), value1.MemberEnd()) != 
        std::distance(value2.MemberBegin(), value2.MemberEnd()))
        {
        return false;
        }

    for (auto member1Itr = value1.MemberBegin(); member1Itr != value1.MemberEnd(); ++member1Itr)
        {
        auto& value2Value = value2[member1Itr->name.GetString()];

        if (!ValuesEqual(member1Itr->value, value2Value))
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
        if (!ValuesEqual(value1[i], value2[i]))
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
