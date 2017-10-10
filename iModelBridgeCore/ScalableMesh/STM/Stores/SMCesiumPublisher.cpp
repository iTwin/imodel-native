#include "ScalableMeshPCH.h"
#include "SMCesiumPublisher.h"
#include "TilePublisher\MeshTile.h"
#include "TilePublisher\TilePublisher.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, const Transform& tranform, bvector<Byte>& outData)
    {
    size_t siblingIndex = 0;
    TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), tranform/*Transform::FromIdentity()*/, siblingIndex, parent, nullptr, -1, false);
    auto meshes = tileNode->GenerateMeshes();
    if (!meshes.empty())
        {
        TilePublisher publisher(*tileNode, nullptr, nullptr);
        publisher.Publish(*reinterpret_cast<TileMesh*>(&*meshes[0]), outData);
        }
    }

void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData)
    {
    size_t siblingIndex = 0;
    TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), Transform::FromIdentity(), siblingIndex, parent, clips, coverageID, isClipBoundary);
    TilePublisher publisher(*tileNode, sourceGCS, destinationGCS);
    publisher.Publish(outData);
    }
