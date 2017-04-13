#include "ScalableMeshPCH.h"
#include "SMCesiumPublisher.h"
#include "TilePublisher\MeshTile.h"
#include "TilePublisher\TilePublisher.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, const Transform& tranform, bvector<Byte>& outData)
    {
    size_t siblingIndex = 0;
    BentleyApi::Dgn::TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), tranform/*Transform::FromIdentity()*/, siblingIndex, parent);
    auto meshes = tileNode->GenerateMeshes();
    if (!meshes.empty())
        {
        // SM_NEEDS_WORK_STREAMING : do we need a specific context?
        //PublisherContextPtr context = new PublisherContext(outDir, (/*L"p_" +*/ std::to_wstring(blockID.m_integerID) + L".b3dm").c_str());
        TilePublisher publisher(nullptr/*context*/);
        publisher.Publish(*reinterpret_cast<TileMesh*>(&*meshes[0]), outData);
        }
    }

void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS, bvector<Byte>& outData)
    {
    size_t siblingIndex = 0;
    BentleyApi::Dgn::TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), Transform::FromIdentity(), siblingIndex, parent);
    auto meshes = tileNode->GenerateMeshes();
    if (!meshes.empty())
        {
        meshes[0]->ReprojectPoints(sourceGCS, destinationGCS);
        // SM_NEEDS_WORK_STREAMING : do we need a specific context?
        //PublisherContextPtr context = new PublisherContext(outDir, (/*L"p_" +*/ std::to_wstring(blockID.m_integerID) + L".b3dm").c_str());
        TilePublisher publisher(nullptr/*context*/);
        publisher.Publish(*reinterpret_cast<TileMesh*>(&*meshes[0]), outData);
        }
    }
