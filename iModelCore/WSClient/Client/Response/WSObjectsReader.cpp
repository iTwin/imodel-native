/*--------------------------------------------------------------------------------------+
|
|  $Source: Client/Response/WSObjectsReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSObjectsReader.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::WSObjectsReader() :
m_data(nullptr)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::~WSObjectsReader()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instances::Instances(std::shared_ptr<const WSObjectsReader> reader) :
m_reader(reader)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::Instances::IsValid() const
    {
    return !m_reader->HasReadErrors();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::Instances::IsEmpty() const
    {
    return begin() == end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReader::Instances::Size() const
    {
    return m_reader->GetInstanceCount();
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::InstanceIterator WSObjectsReader::Instances::begin() const
    {
    return InstanceIterator(m_reader, 0);
    }

/*--------------------------------------------------------------------------------------+
*  @bsimethod                                                   Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::InstanceIterator WSObjectsReader::Instances::end() const
    {
    return InstanceIterator(m_reader, m_reader->GetInstanceCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance::Instance
(
std::shared_ptr<const WSObjectsReader> adapter
) :
m_reader(adapter),
m_objectId(),
m_instance(nullptr),
m_instanceProperties(nullptr),
m_relationshipInstances(nullptr)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance::Instance
(
std::shared_ptr<const WSObjectsReader> adapter,
ObjectId objectId,
const rapidjson::Value* instance,
const rapidjson::Value* instanceProperties,
const rapidjson::Value* relationshipInstances
) :
m_reader(adapter),
m_objectId(std::move(objectId)),
m_instance(instance),
m_instanceProperties(instanceProperties),
m_relationshipInstances(relationshipInstances)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::Instance::IsValid() const
    {
    return nullptr != m_instanceProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR WSObjectsReader::Instance::GetObjectId() const
    {
    return m_objectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSObjectsReader::Instance::GetETag() const
    {
    return m_reader->GetInstanceETag(m_instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const rapidjson::Value& WSObjectsReader::Instance::GetProperties() const
    {
    return *m_instanceProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstances WSObjectsReader::Instance::GetRelationshipInstances() const
    {
    return RelationshipInstances(m_reader, m_relationshipInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstance::RelationshipInstance
(
std::shared_ptr<const WSObjectsReader> adapter
) :
m_reader(adapter),
m_objectId(),
m_instance(nullptr),
m_relationshipInstanceProperties(nullptr),
m_relatedInstance(nullptr),
m_direction(BentleyApi::ECN::ECRelatedInstanceDirection::Forward)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstance::RelationshipInstance
(
std::shared_ptr<const WSObjectsReader> adapter,
ObjectId objectId,
const rapidjson::Value* instance,
const rapidjson::Value* relationshipInstanceProperties,
const rapidjson::Value* relatedInstance,
BentleyApi::ECN::ECRelatedInstanceDirection direction
) :
m_reader(adapter),
m_objectId(std::move(objectId)),
m_instance(instance),
m_relationshipInstanceProperties(relationshipInstanceProperties),
m_relatedInstance(relatedInstance),
m_direction(direction)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::RelationshipInstance::IsValid() const
    {
    return nullptr != m_relationshipInstanceProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR WSObjectsReader::RelationshipInstance::GetObjectId() const
    {
    return m_objectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::ECN::ECRelatedInstanceDirection WSObjectsReader::RelationshipInstance::GetDirection() const
    {
    return m_direction;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSObjectsReader::RelationshipInstance::GetETag() const
    {
    return m_reader->GetInstanceETag(m_instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR WSObjectsReader::RelationshipInstance::GetProperties() const
    {
    return *m_relationshipInstanceProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReader::RelationshipInstance::GetRelatedInstance() const
    {
    return m_reader->GetRelatedInstance(m_relatedInstance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstances::RelationshipInstances
(
std::shared_ptr<const WSObjectsReader> adapter,
const rapidjson::Value* relationshipInstances
) :
m_reader(adapter),
m_relationshipInstances(relationshipInstances)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::RelationshipInstances::IsValid() const
    {
    return nullptr != m_relationshipInstances;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::RelationshipInstances::IsEmpty() const
    {
    return begin() == end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReader::RelationshipInstances::Size() const
    {
    return m_reader->GetRelationshipInstanceCount(m_relationshipInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstanceIterator WSObjectsReader::RelationshipInstances::begin() const
    {
    return RelationshipInstanceIterator(m_reader, m_relationshipInstances ? m_relationshipInstances->Begin() : 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstanceIterator WSObjectsReader::RelationshipInstances::end() const
    {
    return RelationshipInstanceIterator(m_reader, m_relationshipInstances ? m_relationshipInstances->End() : 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstanceIterator::RelationshipInstanceIterator
(
std::shared_ptr<const WSObjectsReader> adapter,
rapidjson::Value::ConstValueIterator it
) :
m_reader(adapter),
m_it(it)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstance WSObjectsReader::RelationshipInstanceIterator::operator*() const
    {
    return m_reader->GetRelationshipInstance(m_it);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::RelationshipInstanceIterator::operator==(const RelationshipInstanceIterator& rhs) const
    {
    return m_it == rhs.m_it;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::RelationshipInstanceIterator::operator!=(const RelationshipInstanceIterator& rhs) const
    {
    return m_it != rhs.m_it;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSObjectsReader::RelationshipInstanceIterator::operator++()
    {
    ++m_it;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::InstanceIterator::InstanceIterator
(
std::shared_ptr<const WSObjectsReader> adapter,
rapidjson::SizeType index
) :
m_reader(adapter),
m_index(index)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReader::InstanceIterator::operator*() const
    {
    return m_reader->GetInstance(m_index);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::InstanceIterator::operator==(const InstanceIterator& rhs) const
    {
    return m_index == rhs.m_index && m_reader == rhs.m_reader;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReader::InstanceIterator::operator!=(const InstanceIterator& rhs) const
    {
    return m_index != rhs.m_index || m_reader != rhs.m_reader;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSObjectsReader::InstanceIterator::operator++()
    {
    ++m_index;
    }
