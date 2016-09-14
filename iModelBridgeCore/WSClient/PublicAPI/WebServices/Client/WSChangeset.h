/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSChangeset.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        struct Options;

        typedef std::function<BentleyStatus(ObjectIdCR oldId, ObjectIdCR newId)> IdHandler;

    private:
        Format m_format;
        bvector<std::shared_ptr<Instance>> m_instances;
        std::shared_ptr<Options> m_options;

    private:
        void ToSingleInstanceRequestJson(JsonValueR json) const;
        void ToMultipleInstancesRequestJson(JsonValueR json) const;
        void ToOptionsRequestJson(JsonValueR json) const;

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

        //! Check if changeset does not contain any instances
        WSCLIENT_EXPORT bool IsEmpty() const;
        //! Get total instance count (root and related) used in changeset. Excludes relationship instances.
        WSCLIENT_EXPORT size_t GetInstanceCount() const;
        //! Get total relationship count used in changeset.
        WSCLIENT_EXPORT size_t GetRelationshipCount() const;

        //! Request options allows to change default behaviour of WSG request
        //! Current WSChangeset implementation allows these options: Response Content, Refresh Instances, Custom Options.
        //! If request options are not initialised, this method initialise them.
        //! @return request options
        WSCLIENT_EXPORT Options& GetRequestOptions();

        //! If request options are initialised (by calling GetRequestOptions)
        //! but are not needed for this WSChangeset, request options can be removed from request.
        WSCLIENT_EXPORT void RemoveRequestOptions();

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

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSChangeset::Options
    {
    friend struct WSChangeset;

    //! better enum comments
    public:
        enum ResponseContent
            {
            FullInstance, //full instances are returned (default behaviuor).
            Empty, //response body is empty, only HTTP status is returned.
            InstanceId  //only class info and instance id's are returned. All properties are removed from response. 
            };

    private:
        ResponseContent* m_responseContent;
        bool* m_refreshInstances;
        std::map<Utf8String, Utf8String> m_customOptions;
        mutable size_t m_baseSize = 0;

    private:
        static Utf8CP GetResponseContentStr(ResponseContent option);
        size_t CalculateSize() const;
        void ToJson(JsonValueR jsonOut) const;

    public:
        //! Set ResponseContent option.
        WSCLIENT_EXPORT void SetResponseContent(ResponseContent value);

        //! Set RefreshInstances option.
        //! If RefreshInstances is set to true, newly created or modified instances will be returned refreshed. 
        //! If plugin does not support refresh operation WSG core will refresh instances itself. 
        //! If refresh operation fails, instance will have property "refreshed" set to false. 
        WSCLIENT_EXPORT void SetRefreshInstances(bool value);
        
        //! Set custom option.
        //! CustomOptions can be used to pass any user data for ec plugin via extended parameters
        //! Will overwrite values with same name.
        WSCLIENT_EXPORT void SetCustomOption(Utf8StringCR name, Utf8StringCR value);

        //! Remove custom option.
        WSCLIENT_EXPORT void RemoveCustomOption(Utf8StringCR name);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
