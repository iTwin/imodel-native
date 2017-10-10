#include "ScalableMeshPCH.h"
#include <ScalableMesh\IScalableMeshPublisher.h>
#include "SMCesiumPublisher.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH


void IScalableMeshPublisher::Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData)
    {
    return _Publish(node, transform, outData);
    }

void IScalableMeshPublisher::Publish(IScalableMeshNodePtr node, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData)
    {
    return _Publish(node, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, outData);
    }

IScalableMeshPublisherPtr IScalableMeshPublisher::Create(const SMPublishType& type)
    {
    switch (type)
        {
        case SMPublishType::CESIUM:
            {
            return new SMCesiumPublisher();
            }
        default:
            {
            return nullptr;
            }
        }
    }
