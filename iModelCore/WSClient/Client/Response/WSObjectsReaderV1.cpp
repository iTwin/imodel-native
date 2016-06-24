/*--------------------------------------------------------------------------------------+
|
|  $Source: Client/Response/WSObjectsReaderV1.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSObjectsReaderV1.h>

static rapidjson::Value s_nullJsonObject(rapidjson::Type::kNullType);
static rapidjson::Value s_emptyArrayJsonObject(rapidjson::Type::kArrayType);

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReaderV1::WSObjectsReaderV1(Utf8StringCR schemaName) :
m_schemaName(schemaName),
m_className(),
m_instanceCount(0),
m_multipleInstancesFormat(true)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReaderV1::WSObjectsReaderV1(Utf8StringCR schemaName, Utf8StringCR className) :
m_schemaName(schemaName),
m_className(className),
m_instanceCount(1),
m_multipleInstancesFormat(false)
    {
    if (className.empty())
        {
        m_instanceCount = 0;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReaderV1::~WSObjectsReaderV1()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WSObjectsReaderV1> WSObjectsReaderV1::Create(Utf8StringCR schemaName)
    {
    return std::shared_ptr<WSObjectsReaderV1>(new WSObjectsReaderV1(schemaName));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<WSObjectsReaderV1> WSObjectsReaderV1::Create(Utf8StringCR schemaName, Utf8StringCR className)
    {
    return std::shared_ptr<WSObjectsReaderV1>(new WSObjectsReaderV1(schemaName, className));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instances WSObjectsReaderV1::ReadInstances(std::shared_ptr<const rapidjson::Value> data)
    {
    m_data = data;

    if (nullptr == m_data ||
        !m_data->IsObject())
        {
        BeAssert(false && "Bad format");
        m_instanceCount = 0;
        m_data = nullptr;
        return Instances(shared_from_this());
        }

    if (m_multipleInstancesFormat)
        {
        for (rapidjson::Value::ConstMemberIterator member = m_data->MemberBegin(); member != m_data->MemberEnd(); ++member)
            {
            if (!member->value.IsArray() || member->value.IsNull())
                {
                BeAssert(false && "Bad format");
                m_instanceCount = 0;
                m_data = nullptr;
                return Instances(shared_from_this());
                }
            m_instanceCount += member->value.Size();
            }
        }

    return Instances(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsReaderV1::HasReadErrors() const
    {
    return nullptr == m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV1::ReadInstance(const rapidjson::Value* instanceProperties, Utf8String className) const
    {
    BeAssert(!m_schemaName.empty());
    BeAssert(!className.empty());

    if (!instanceProperties->IsObject() || !(*instanceProperties)["$id"].IsString())
        {
        BeAssert(false && "Bad format");
        return Instance(shared_from_this());
        }

    Utf8String remoteId((*instanceProperties)["$id"].GetString());

    return Instance
        (
        shared_from_this(),
        ObjectId(m_schemaName, className, remoteId),
        instanceProperties,
        instanceProperties,
        &s_emptyArrayJsonObject
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReaderV1::GetInstanceCount() const
    {
    return m_instanceCount;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV1::GetInstance(rapidjson::SizeType index) const
    {
    if (index >= m_instanceCount)
        {
        BeAssert(false && "Index out of range");
        return Instance(shared_from_this());
        }

    if (m_multipleInstancesFormat)
        {
        Json::ArrayIndex previousTotal = 0;
        for (rapidjson::Value::ConstMemberIterator member = m_data->MemberBegin(); member != m_data->MemberEnd(); ++member)
            {
            RapidJsonValueCR classInstances = member->value;

            if (previousTotal + classInstances.Size() > index)
                {
                const rapidjson::Value* instanceProperties = &classInstances[index - previousTotal];
                return ReadInstance(instanceProperties, member->name.GetString());
                }

            previousTotal += classInstances.Size();
            }
        }
    else
        {
        return ReadInstance(m_data.get(), m_className);
        }

    return Instance(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::Instance WSObjectsReaderV1::GetRelatedInstance(const rapidjson::Value* relatedInstance) const
    {
    BeAssert(false && "Stub");
    return Instance(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsReader::RelationshipInstance WSObjectsReaderV1::GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const
    {
    BeAssert(false && "Stub");
    return RelationshipInstance(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSObjectsReaderV1::GetInstanceETag(const rapidjson::Value* instance) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::SizeType WSObjectsReaderV1::GetRelationshipInstanceCount(const rapidjson::Value* instance) const
    {
    return 0;
    }
