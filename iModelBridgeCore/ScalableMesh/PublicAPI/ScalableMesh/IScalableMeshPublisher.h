/*--------------------------------------------------------------------------------------+
|
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/IScalableMesh.h>
#include <Bentley/RefCounted.h>
#include <json/json.h>

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshPublisher;
typedef RefCountedPtr<IScalableMeshPublisher> IScalableMeshPublisherPtr;

struct IScalableMeshPublishParams;
typedef RefCountedPtr<IScalableMeshPublishParams> IScalableMeshPublishParamsPtr;


enum class SMPublishType
    {
    THREESM,
    CESIUM
    };

struct IScalableMeshPublishParams: virtual public RefCountedBase
    {
    BENTLEY_SM_IMPORT_EXPORT static IScalableMeshPublishParamsPtr Create(const SMPublishType& type);
    };

struct IScalableMeshPublisher: virtual public RefCountedBase
    {

    /*__PUBLISH_SECTION_END__*/
    private:

        virtual StatusInt _Publish(IScalableMeshPublishParamsPtr params) = 0;
        
        virtual void _Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData, bool outputTexture) = 0;

        virtual void _Publish(IScalableMeshNodePtr nodePtr, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const Transform& transform, bvector<Byte>& outData, bool outputTexture) = 0;

        virtual void _ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smHeader) = 0;

        virtual void _ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smMasterHeader) = 0;

    /*__PUBLISH_SECTION_START__*/
    public:

        BENTLEY_SM_IMPORT_EXPORT StatusInt Publish(IScalableMeshPublishParamsPtr params);

        BENTLEY_SM_IMPORT_EXPORT void Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData, bool outputTexture);

        BENTLEY_SM_IMPORT_EXPORT void Publish(IScalableMeshNodePtr nodePtr, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData, bool outputTexture);

        BENTLEY_SM_IMPORT_EXPORT void Publish(IScalableMeshNodePtr nodePtr, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const Transform& transform, bvector<Byte>& outData, bool outputTexture);

        BENTLEY_SM_IMPORT_EXPORT void ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smHeader);

        BENTLEY_SM_IMPORT_EXPORT void ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smMasterHeader);

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshPublisherPtr Create(const SMPublishType& type);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE