/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshQuery.h>
#include <Bentley/RefCounted.h>

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshPublisher;
typedef RefCountedPtr<IScalableMeshPublisher> IScalableMeshPublisherPtr;

enum class SMPublishType
    {
    CESIUM
    };

struct IScalableMeshPublisher abstract : virtual public RefCountedBase
    {

    /*__PUBLISH_SECTION_END__*/
    private:
        
        virtual void _Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData) = 0;

        virtual void _Publish(IScalableMeshNodePtr nodePtr, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData) = 0;

    /*__PUBLISH_SECTION_START__*/
    public:

        BENTLEY_SM_EXPORT void Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData);

        BENTLEY_SM_EXPORT void Publish(IScalableMeshNodePtr nodePtr, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData);
        
        BENTLEY_SM_EXPORT static IScalableMeshPublisherPtr Create(const SMPublishType& type);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE