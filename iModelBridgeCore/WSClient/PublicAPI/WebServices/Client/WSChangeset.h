/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSChangeset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/ObjectId.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC

typedef std::shared_ptr<Json::Value> JsonValuePtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSChangeset
    {
    public:
        enum Format
            {
            SingeInstance,
            MultipleInstances
            };

        enum ChangeState
            {
            Existing,
            Created,
            Modified,
            Deleted
            };

        struct Instance;
        struct Relationship;

        typedef std::function<BentleyStatus(ObjectIdCR oldId, ObjectIdCR newId)> IdHandler;

    private:
        Format m_format;
        bvector<std::shared_ptr<Instance>> m_instances;

    private:
        void ToSingleInstanceRequestJson(JsonValueR json) const;
        void ToMultipleInstancesRequestJson(JsonValueR json) const;

        BentleyStatus ExtractNewIdsFromSingleInstanceResponse(RapidJsonValueCR response, const IdHandler& handler) const;
        BentleyStatus ExtractNewIdsFromMultipleInstancesResponse(RapidJsonValueCR response, const IdHandler& handler) const;

    public:
        //! Create changeset. 
        //! Specify Format::SingeInstance for one instance with related instances changeset format.
        //! Specify Format::MultipleInstance for multiple instances with telated instances changeset format.
        WSCLIENT_EXPORT WSChangeset(Format format = Format::MultipleInstances);

        //! Add new instance. 
        //! @return added instance. Will assert if adding second instance with Format::SingeInstance and return undefined result.
        WSCLIENT_EXPORT Instance& AddInstance(ObjectId instanceId, ChangeState state, JsonValuePtr properties);

        //! Find instance in changeset by its id. If multiple instnaces with same id exist, first one will be returned.
        //! Null will be returned if instnace was not found.
        WSCLIENT_EXPORT Instance* FindInstance(ObjectIdCR id) const;

        //! Remove added instance, will preserve existing pointers.
        //! @return true if removed, false if not found
        WSCLIENT_EXPORT bool RemoveInstance(Instance& instance);

        //! Get total instance count (root and related) used in changeset. Excludes relationship instances.
        WSCLIENT_EXPORT size_t GetInstanceCount() const;
        //! Get total relationship count used in changeset.
        WSCLIENT_EXPORT size_t GetRelationshipCount() const;
        //! Get total size of serialized changeset in bytes
        WSCLIENT_EXPORT size_t CalculateSize() const;

        //! Serialize changeset to JSON string
        WSCLIENT_EXPORT Utf8String ToRequestString() const;

        //! Serialize changeset to JSON
        WSCLIENT_EXPORT void ToRequestJson(JsonValueR json) const;

        //! Extract ids for created instances from response JSON
        WSCLIENT_EXPORT BentleyStatus ExtractNewIdsFromResponse(RapidJsonValueCR response, const IdHandler& handler) const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSChangeset::Instance
    {
    friend struct WSChangeset;

    private:
        ObjectId m_id;
        ChangeState m_state;
        std::shared_ptr<Json::Value> m_properties;
        mutable size_t m_baseSize = 0;
        bvector<std::shared_ptr<Relationship>> m_relationships;

    private:
        size_t CountRelatedInstances() const;
        size_t CalculateSize() const;
        void ToJson(JsonValueR instanceJsonOut) const;

        BentleyStatus ExtractNewIdsFromInstanceAfterChange(RapidJsonValueCR instanceAfterChange, const IdHandler& handler) const;

        static void FillBase(JsonValueR instanceJsonOut, ObjectIdCR id, ChangeState state, JsonValuePtr properties);
        static size_t CalculateBaseSize(ObjectIdCR id, ChangeState state, JsonValuePtr properties, size_t& propertiesSizeInOut);
        static size_t CalculateFieldSize(Utf8CP fieldName, Utf8CP fieldValue);
        static size_t CalculateDirectionFieldSize(ECRelatedInstanceDirection direction);
        static Utf8CP GetChangeStateStr(ChangeState state);
        static Utf8CP GetDirectionStr(ECRelatedInstanceDirection direction);
        static ObjectId GetObjectIdFromInstance(RapidJsonValueCR instance);

    public:
        //! Add related instance
        //! @return added related instance
        WSCLIENT_EXPORT Instance& AddRelatedInstance
            (
                ObjectId relId,
                ChangeState relState,
                ECRelatedInstanceDirection relDirection,
                ObjectId instanceId,
                ChangeState state,
                JsonValuePtr properties
                );
            //! Find related instance. Returns null if no matches found
        WSCLIENT_EXPORT Instance* FindRelatedInstance(ObjectIdCR id) const;
        //! Remove related instance. Returns false if nothing to remove
        WSCLIENT_EXPORT bool RemoveRelatedInstance(Instance& instance);

        WSCLIENT_EXPORT ObjectIdCR GetId() const;
        WSCLIENT_EXPORT ChangeState GetState() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSChangeset::Relationship
    {
    friend struct WSChangeset;
    friend struct WSChangeset::Instance;

    private:
        ObjectId m_id;
        ChangeState m_state;
        ECRelatedInstanceDirection m_direction;
        mutable size_t m_baseSize = 0;
        Instance m_instance;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
