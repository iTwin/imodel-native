/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECDbHelper.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECDbHelper.h>

#include <ostream>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECDbHelper::LocateSchema(SchemaKeyCR schemaKey, BeFileNameCR schemaDir)
    {
    ECSchemaReadContextPtr ecSchemaContext = ECSchemaReadContext::CreateContext();
    ecSchemaContext->AddSchemaPath(schemaDir);

    SchemaKey schemaKey_ = schemaKey;

    ECSchemaPtr schema = ecSchemaContext->LocateSchema(schemaKey_, SchemaMatchType::Exact);
    BeAssert(!schema.IsNull());

    return schema;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECDbHelper::CopySchema(ECSchemaCR schema, IECSchemaLocater* optionalLocator)
    {
    WString xmlBuffer;
    if (SchemaWriteStatus::Success != schema.WriteToXmlString(xmlBuffer))
        {
        return nullptr;
        }

    ECSchemaReadContextPtr copyContext = ECSchemaReadContext::CreateContext();

    if (nullptr != optionalLocator)
        {
        copyContext->AddSchemaLocater(*optionalLocator);
        }

    ECSchemaPtr schemaCopy;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schemaCopy, xmlBuffer.c_str(), *copyContext))
        {
        return nullptr;
        }
    return schemaCopy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbHelper::ECInstanceIdFromECInstance(IECInstanceCR ecInstance)
    {
    ECInstanceId instanceId;
    BentleyStatus stat = ECInstanceId::FromString(instanceId, ecInstance.GetInstanceId().c_str());
    BeAssert(SUCCESS == stat);
    return instanceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbHelper::ECInstanceIdFromString(Utf8CP ecIdString)
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, ecIdString);
    return instanceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECDbHelper::StringFromECInstanceId(ECInstanceId ecInstanceId)
    {
    return ecInstanceId.ToString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbHelper::ECInstanceIdFromJsonValue(JsonValueCR jsonValue)
    {
    if (jsonValue.isNull() || !jsonValue.isString())
        {
        return ECInstanceId();
        }
    return ECInstanceIdFromString(jsonValue.asCString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbHelper::ECInstanceIdFromJsonInstance(JsonValueCR jsonInstance)
    {
    if (!jsonInstance.isMember("$ECInstanceId"))
        {
        return ECInstanceId();
        }
    return ECInstanceIdFromJsonValue(jsonInstance["$ECInstanceId"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECDbHelper::ECClassKeyFromClass(ECClassCR ecClass)
    {
    return Utf8PrintfString
        (
        "%s.%s",
        Utf8String(ecClass.GetSchema().GetName()).c_str(),
        Utf8String(ecClass.GetName()).c_str()
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbHelper::ParseECClassKey(Utf8StringCR classKey, Utf8StringR schemaNameOut, Utf8StringR classNameOut)
    {
    auto index = classKey.find('.');
    if (Utf8String::npos == index)
        {
        schemaNameOut = "";
        classNameOut = "";
        return;
        }
    schemaNameOut = classKey.substr(0, index);
    classNameOut = classKey.substr(index + 1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbHelper::GetRelationshipClass(ECSchemaCP schema, Utf8CP className)
    {
    ECClassCP relEcClass = schema->GetClassCP(className);
    if (NULL == relEcClass)
        {
        return NULL;
        }
    ECRelationshipClassCP relationshipClass = relEcClass->GetRelationshipClassCP();
    return relationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbHelper::IsInstanceInMultiMap(ECInstanceKeyCR instance, const ECInstanceKeyMultiMap& map)
    {
    auto range = map.equal_range(instance.GetECClassId());
    for (auto it = range.first; it != range.second; it++)
        {
        if (it->second == instance.GetECInstanceId())
            {
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECDbHelper::UtcDateToString(DateTimeCR utcDate)
    {
    if (utcDate.GetInfo().GetKind() != DateTime::Kind::Utc)
        {
        BeAssert(false && "Only Utc should be passed");
        return nullptr;
        }
    return utcDate.ToUtf8String();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbHelper::IsObjectClass(ECClassCR ecClass)
    {
    return (ecClass.IsEntityClass()); // WIP_EC3 - verify this is the correct check
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyMultiMapPair ECDbHelper::ToPair(ECInstanceKeyCR key)
    {
    return ECInstanceKeyMultiMapPair(key.GetECClassId(), key.GetECInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbHelper::Erase(ECInstanceKeyMultiMap& map, ECInstanceKeyCR key)
    {
    auto range = map.equal_range(key.GetECClassId());
    for (auto it = range.first; it != range.second; ++it)
        {
        if (key.GetECInstanceId() == it->second)
            {
            map.erase(it);
            break;
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyMultiMap ECDbHelper::MergeMultiMaps(const ECInstanceKeyMultiMap& map1, const ECInstanceKeyMultiMap& map2)
    {
    ECInstanceKeyMultiMap result;

    bmap<ECClassId, bset<ECInstanceId>> map;

    for (auto start = map1.begin(), end = start; start != map1.end(); start = end)
        {
        end = map1.upper_bound(end->first);

        auto& set = map[start->first];

        for (auto it = start; it != end; it++)
            {
            set.insert(it->second);
            }
        }

    for (auto start = map2.begin(), end = start; start != map2.end(); start = end)
        {
        end = map2.upper_bound(end->first);

        auto& set = map[start->first];

        for (auto it = start; it != end; it++)
            {
            set.insert(it->second);
            }
        }

    for (auto pair : map)
        {
        for (auto instanceId : pair.second)
            {
            result.insert({pair.first, instanceId});
            }
        }

    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECDbHelper::ToECInstanceIdList
(
ECInstanceKeyMultiMap::const_iterator from,
ECInstanceKeyMultiMap::const_iterator to
)
    {
    Utf8String list;

    while (from != to)
        {
        if (!list.empty())
            {
            list += ",";
            }

        list += Utf8PrintfString("%lld", from->second.GetValue());

        from++;
        }

    return list;
    }

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::ostream& operator << (std::ostream &o, ECInstanceId ecInstanceId)
    {
    o << ECDbHelper::StringFromECInstanceId(ecInstanceId);
    return o;
    }

END_BENTLEY_WEBSERVICES_NAMESPACE
