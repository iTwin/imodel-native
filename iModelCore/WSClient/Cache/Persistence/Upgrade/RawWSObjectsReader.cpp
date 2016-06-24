/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/RawWSObjectsReader.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "RawWSObjectsReader.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

static rapidjson::Value s_emptyArrayJsonObject(rapidjson::Type::kArrayType);

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsResponse RawWSObjectsReader::CreateWSObjectsResponse(const bvector<RawInstance>& instances, Utf8StringCR eTag)
    {
    return WSObjectsResponse
        (
        std::shared_ptr<RawWSObjectsReader>(new RawWSObjectsReader(instances)),
        HttpStringBody::Create("{}"),
        HttpStatus::OK,
        eTag,
        nullptr
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RawWSObjectsReader::RawWSObjectsReader(const bvector<RawInstance>& instances) :
m_instances(instances)
    {
    for (auto& instance : m_instances)
        {
        m_instancesToETags[instance.json.get()] = &instance.eTag;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RawWSObjectsReader::Instances RawWSObjectsReader::ReadInstances(std::shared_ptr<const rapidjson::Value> data)
    {
    return Instances(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RawWSObjectsReader::HasReadErrors() const
    {
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType RawWSObjectsReader::GetInstanceCount() const
    {
    return static_cast<rapidjson::SizeType> (m_instances.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RawWSObjectsReader::Instance RawWSObjectsReader::GetInstance(rapidjson::SizeType index) const
    {
    auto& rawInstance = m_instances[index];
    return Instance(shared_from_this(), rawInstance.objectId, rawInstance.json.get(), rawInstance.json.get(), &s_emptyArrayJsonObject);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RawWSObjectsReader::Instance RawWSObjectsReader::GetRelatedInstance(const rapidjson::Value* relatedInstance) const
    {
    return Instance(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RawWSObjectsReader::RelationshipInstance RawWSObjectsReader::GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const
    {
    return RelationshipInstance(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RawWSObjectsReader::GetInstanceETag(const rapidjson::Value* instance) const
    {
    auto it = m_instancesToETags.find(instance);
    if (it == m_instancesToETags.end())
        {
        return nullptr;
        }
    return *it->second;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType RawWSObjectsReader::GetRelationshipInstanceCount(const rapidjson::Value* relationshipInstances) const
    {
    return 0;
    }
