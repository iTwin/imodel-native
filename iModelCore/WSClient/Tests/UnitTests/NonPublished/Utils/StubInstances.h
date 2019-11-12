/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

// Class for mocking server returned instances to use in cache tests
struct StubInstances
    {
    public:
        struct StubRelationshipInstances;
        struct StubRelationshipInstance;
        struct StubInstance;

    private:
        std::vector<StubInstance> m_instances;

    public:
        Json::Value ConvertStubInstanceToJson(const StubInstance& instance) const;
        Json::Value ConvertStubRelationshipInstanceToJson(const StubRelationshipInstance& instance) const;

    public:
        //! Clear any added instances
        void Clear();
        //! Add instance with optional properties and eTag
        StubRelationshipInstances Add
            (
            ObjectIdCR objectId,
            std::map<Utf8String,Json::Value> properties = std::map<Utf8String, Json::Value>(),
            Utf8StringCR eTag = ""
            );
        //! Add instance with properties and eTag
        StubRelationshipInstances Add
            (
            ObjectIdCR objectId,
            Json::Value const* properties,
            Utf8StringCR eTag = ""
            );
        //! Create server objects result to be used in caching
        WSObjectsResponse ToWSObjectsResponse(Utf8StringCR eTag = "", Utf8StringCR skipToken = "") const;

        WSObjectsResponse ToWSObjectsResponseNotModified(Utf8StringCR eTag = "", Utf8StringCR skipToken = "") const;
        //! Create success WSObjectsResult
        WSObjectsResult ToWSObjectsResult(Utf8StringCR eTag = "", Utf8StringCR skipToken = "") const;
        //! Create success WSChangesetResult
        WSChangesetResult ToWSChangesetResult() const;
        //! Create WSG 2.0 server response body JSON string
        Utf8String ToJsonWebApiV2() const;
        //! Create server changeset response body JSON string
        Utf8String ToChangesetResponseJson() const;
        //! Create server object creation result
        WSCreateObjectResult ToWSCreateObjectResult(Utf8StringCR fileETag = "") const;
    };

struct StubInstances::StubRelationshipInstances
    {
    private:
        bvector<std::shared_ptr<StubRelationshipInstance>>& m_relationshipInstances;

    public:
        StubRelationshipInstances(bvector<std::shared_ptr<StubRelationshipInstance>>& relationshipInstances);

        StubRelationshipInstances AddRelated
            (
            ObjectIdCR relationshipObjectId,
            ObjectIdCR relatedObjectId,
            std::map<Utf8String, Json::Value> relatedProperties = std::map<Utf8String, Json::Value>(),
            ECRelatedInstanceDirection direction = ECRelatedInstanceDirection::Forward,
            std::map<Utf8String, Json::Value> relationshipProperties = std::map<Utf8String, Json::Value>()
            );
    };

struct StubInstances::StubInstance
    {
    ObjectId objectId;
    Utf8String eTag;
    std::map<Utf8String, Json::Value> properties;
    bvector<std::shared_ptr<StubRelationshipInstance>> relationshipInstances;
    };

struct StubInstances::StubRelationshipInstance
    {
    ObjectId objectId;
    std::map<Utf8String, Json::Value> properties;
    BentleyApi::ECN::ECRelatedInstanceDirection direction;
    StubInstance relatedInstance;
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
