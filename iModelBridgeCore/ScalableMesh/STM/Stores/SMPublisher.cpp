#include "ScalableMeshPCH.h"
#include <ScalableMesh\IScalableMeshPublisher.h>
#include "SMCesiumPublisher.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH


void IScalableMeshPublisher::Publish(IScalableMeshNodePtr node, const Transform& transform, bvector<Byte>& outData)
    {
    return _Publish(node, transform, outData);
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
