/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Compatibility/SchemaChangeWSObjectsReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
class SchemaChangeWSObjectsReader : public WSObjectsReader
    {
    private:
        std::shared_ptr<WSObjectsReader> m_reader;
        Utf8String m_schemaName;
        mutable bmap<const rapidjson::Value*, rapidjson::SizeType> m_indexes;
        rapidjson::Value m_emptyArrayJsonObject = rapidjson::Type::kArrayType;

    public:
        SchemaChangeWSObjectsReader(std::shared_ptr<WSObjectsReader> reader, Utf8String schemaName) : m_reader(reader), m_schemaName(schemaName) {}
        virtual ~SchemaChangeWSObjectsReader() {}

        virtual Instances ReadInstances(std::shared_ptr<const rapidjson::Value> data) override
            {
            m_reader->ReadInstances(data);
            return Instances(shared_from_this());
            }
        virtual bool HasReadErrors() const override
            {
            return m_reader->HasReadErrors();
            }
        virtual rapidjson::SizeType GetInstanceCount() const override
            {
            return m_reader->GetInstanceCount();
            }
        virtual Instance GetInstance(rapidjson::SizeType index) const override
            {
            auto instance = m_reader->GetInstance(index);
            auto properties = &instance.GetProperties();
            auto objectId = instance.GetObjectId();
            objectId.schemaName = m_schemaName;

            m_indexes[properties] = index;

            return Instance(shared_from_this(), objectId, properties, properties, &m_emptyArrayJsonObject);
            }
        virtual Instance GetRelatedInstance(const rapidjson::Value* relatedInstance) const override
            {
            BeAssert(false); // Stub
            return Instance(shared_from_this());
            }
        virtual RelationshipInstance GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const override
            {
            BeAssert(false); // Stub
            return RelationshipInstance(shared_from_this());
            }
        virtual Utf8String GetInstanceETag(const rapidjson::Value* properties) const override
            {
            return m_reader->GetInstance(m_indexes[properties]).GetETag();
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE