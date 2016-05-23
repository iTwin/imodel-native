/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/WebServices/Client/Response/WSObjectsReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <rapidjson/BeRapidJson.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/ObjectId.h>

#include <ECObjects/ECObjectsAPI.h>

#include <memory>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef const struct WSObjectsReader& WSObjectsReaderCR;
typedef struct WSObjectsReader& WSObjectsReaderR;

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE WSObjectsReader : public std::enable_shared_from_this<WSObjectsReader>
    {
    public:
        struct Instance;
        struct Instances;
        struct InstanceIterator;
        struct RelationshipInstance;
        struct RelationshipInstances;
        struct RelationshipInstanceIterator;

    protected:
        std::shared_ptr<const rapidjson::Value> m_data;

    protected:
        WSCLIENT_EXPORT WSObjectsReader();

    public:
        WSCLIENT_EXPORT virtual ~WSObjectsReader();

        WSCLIENT_EXPORT virtual Instances ReadInstances(std::shared_ptr<const rapidjson::Value> data) = 0;

        virtual bool HasReadErrors() const = 0;
        virtual rapidjson::SizeType GetInstanceCount() const = 0;

        virtual Instance GetInstance(rapidjson::SizeType index) const = 0;
        virtual Instance GetRelatedInstance(const rapidjson::Value* relatedInstance) const = 0;
        virtual RelationshipInstance GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const = 0;

        virtual Utf8String GetInstanceETag(const rapidjson::Value* instance) const = 0;
        virtual rapidjson::SizeType GetRelationshipInstanceCount(const rapidjson::Value* instance) const = 0;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE WSObjectsReader::Instances
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;

    public:
        WSCLIENT_EXPORT Instances(std::shared_ptr<const WSObjectsReader> reader);

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT bool IsEmpty() const;
        WSCLIENT_EXPORT rapidjson::SizeType Size() const;
        WSCLIENT_EXPORT virtual InstanceIterator begin() const;
        WSCLIENT_EXPORT virtual InstanceIterator end() const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::Instance
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        ObjectId m_objectId;
        const rapidjson::Value* m_instance;
        const rapidjson::Value* m_instanceProperties;
        const rapidjson::Value* m_relationshipInstances;

    public:
        WSCLIENT_EXPORT explicit Instance(std::shared_ptr<const WSObjectsReader> adapter);
        WSCLIENT_EXPORT Instance
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            ObjectId objectId,
            const rapidjson::Value* instance,
            const rapidjson::Value* instanceProperties,
            const rapidjson::Value* relationshipInstances
            );

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT ObjectIdCR GetObjectId() const;
        WSCLIENT_EXPORT Utf8String GetETag() const;
        WSCLIENT_EXPORT RapidJsonValueCR GetProperties() const;
        WSCLIENT_EXPORT RelationshipInstances GetRelationshipInstances() const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::RelationshipInstances
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        const rapidjson::Value* m_relationshipInstances;

    public:
        WSCLIENT_EXPORT RelationshipInstances
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            const rapidjson::Value* relationshipInstances
            );

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT bool IsEmpty() const;
        WSCLIENT_EXPORT rapidjson::SizeType Size() const;
        WSCLIENT_EXPORT RelationshipInstanceIterator begin() const;
        WSCLIENT_EXPORT RelationshipInstanceIterator end() const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::RelationshipInstance
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        ObjectId m_objectId;
        const rapidjson::Value* m_instance;
        const rapidjson::Value* m_relationshipInstanceProperties;
        const rapidjson::Value* m_relatedInstance;
        BentleyApi::ECN::ECRelatedInstanceDirection m_direction;

    public:
        WSCLIENT_EXPORT explicit RelationshipInstance(std::shared_ptr<const WSObjectsReader> adapter);
        WSCLIENT_EXPORT RelationshipInstance
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            ObjectId objectId,
            const rapidjson::Value* instance,
            const rapidjson::Value* relationshipInstanceProperties,
            const rapidjson::Value* relatedInstance,
            BentleyApi::ECN::ECRelatedInstanceDirection direction
            );

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT ObjectIdCR GetObjectId() const;
        WSCLIENT_EXPORT Utf8String GetETag() const;
        WSCLIENT_EXPORT RapidJsonValueCR GetProperties() const;
        WSCLIENT_EXPORT BentleyApi::ECN::ECRelatedInstanceDirection GetDirection() const;
        WSCLIENT_EXPORT Instance GetRelatedInstance() const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::RelationshipInstanceIterator
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        rapidjson::Value::ConstValueIterator m_it;

    public:
        WSCLIENT_EXPORT RelationshipInstanceIterator
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            rapidjson::Value::ConstValueIterator it
            );

        WSCLIENT_EXPORT RelationshipInstance operator*() const;
        WSCLIENT_EXPORT bool operator==(const RelationshipInstanceIterator& rhs) const;
        WSCLIENT_EXPORT bool operator!=(const RelationshipInstanceIterator& rhs) const;
        WSCLIENT_EXPORT void operator++();
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::InstanceIterator
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        rapidjson::SizeType m_index;

    public:
        WSCLIENT_EXPORT InstanceIterator(std::shared_ptr<const WSObjectsReader> adapter, rapidjson::SizeType index);

        WSCLIENT_EXPORT Instance operator*() const;
        WSCLIENT_EXPORT bool operator==(const InstanceIterator& rhs) const;
        WSCLIENT_EXPORT bool operator!=(const InstanceIterator& rhs) const;
        WSCLIENT_EXPORT void operator++();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
