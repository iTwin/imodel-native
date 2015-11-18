/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    rapidjson::GenericStringBuffer<UTF8<>> buffer;
    rapidjson::Writer<rapidjson::GenericStringBuffer<UTF8<>>> writer(buffer);
    source.Accept(writer);

    auto sourceStr = buffer.GetString();

    target.Parse<0>(sourceStr);
    BeAssert(!target.HasParseError());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonUtil::ToStyledString(RapidJsonValueCR value)
    {
    rapidjson::GenericStringBuffer<UTF8<>> buffer;
    rapidjson::PrettyWriter<rapidjson::GenericStringBuffer<UTF8<>>> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
    }