/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/WebServices/Response/WSObjectsReader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <rapidjson/BeRapidJson.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <WebServices/Common.h>
#include <WebServices/ObjectId.h>

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
        WS_EXPORT WSObjectsReader ();

    public:
        WS_EXPORT virtual ~WSObjectsReader ();

        WS_EXPORT virtual Instances ReadInstances (std::shared_ptr<const rapidjson::Value> data) = 0;

        virtual bool HasReadErrors () const = 0;
        virtual rapidjson::SizeType GetInstanceCount () const = 0;
        virtual Instance GetInstance (rapidjson::SizeType index) const = 0;
        virtual Instance GetRelatedInstance (const rapidjson::Value* relatedInstance) const = 0;
        virtual RelationshipInstance GetRelationshipInstance (const rapidjson::Value* relationshipInstance) const = 0;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE WSObjectsReader::Instances
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;

    public:
        WS_EXPORT Instances (std::shared_ptr<const WSObjectsReader> reader);

        WS_EXPORT bool IsValid () const;
        WS_EXPORT bool IsEmpty () const;
        WS_EXPORT virtual InstanceIterator begin () const;
        WS_EXPORT virtual InstanceIterator end () const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::Instance
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        ObjectId m_objectId;
        const rapidjson::Value* m_instanceProperties;
        const rapidjson::Value* m_relationshipInstances;

    public:
        WS_EXPORT explicit Instance (std::shared_ptr<const WSObjectsReader> adapter);
        WS_EXPORT Instance
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            ObjectId objectId,
            const rapidjson::Value* instanceProperties,
            const rapidjson::Value* relationshipInstances
            );

        WS_EXPORT bool IsValid () const;
        WS_EXPORT ObjectIdCR GetObjectId () const;
        WS_EXPORT RapidJsonValueCR GetProperties () const;
        WS_EXPORT RelationshipInstances GetRelationshipInstances () const;
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
        WS_EXPORT RelationshipInstances 
            (
            std::shared_ptr<const WSObjectsReader> adapter, 
            const rapidjson::Value* relationshipInstances
            );

        WS_EXPORT bool IsValid () const;
        WS_EXPORT bool IsEmpty () const;
        WS_EXPORT RelationshipInstanceIterator begin () const;
        WS_EXPORT RelationshipInstanceIterator end () const;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReader::RelationshipInstance
    {
    private:
        std::shared_ptr<const WSObjectsReader> m_reader;
        ObjectId m_objectId;
        const rapidjson::Value* m_relationshipInstanceProperties;
        const rapidjson::Value* m_relatedInstance;
        BentleyApi::ECN::ECRelatedInstanceDirection m_direction;

    public:
        WS_EXPORT explicit RelationshipInstance (std::shared_ptr<const WSObjectsReader> adapter);
        WS_EXPORT RelationshipInstance
            (
            std::shared_ptr<const WSObjectsReader> adapter,
            ObjectId objectId,
            const rapidjson::Value* relationshipInstanceProperties,
            const rapidjson::Value* relatedInstance,
            BentleyApi::ECN::ECRelatedInstanceDirection direction
            );

        WS_EXPORT bool IsValid () const;
        WS_EXPORT ObjectIdCR GetObjectId () const;
        WS_EXPORT RapidJsonValueCR GetProperties () const;
        WS_EXPORT BentleyApi::ECN::ECRelatedInstanceDirection GetDirection () const;
        WS_EXPORT Instance GetRelatedInstance () const;
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
        WS_EXPORT RelationshipInstanceIterator 
            (
            std::shared_ptr<const WSObjectsReader> adapter, 
            rapidjson::Value::ConstValueIterator it
            );

        WS_EXPORT RelationshipInstance operator*() const;
        WS_EXPORT bool operator==(const RelationshipInstanceIterator& rhs) const;
        WS_EXPORT bool operator!=(const RelationshipInstanceIterator& rhs) const;
        WS_EXPORT void operator++();
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
        WS_EXPORT InstanceIterator (std::shared_ptr<const WSObjectsReader> adapter, rapidjson::SizeType index);

        WS_EXPORT Instance operator*() const;
        WS_EXPORT bool operator==(const InstanceIterator& rhs) const;
        WS_EXPORT bool operator!=(const InstanceIterator& rhs) const;
        WS_EXPORT void operator++();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
