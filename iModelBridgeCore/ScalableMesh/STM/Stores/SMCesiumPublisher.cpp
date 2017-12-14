#include "ScalableMeshPCH.h"
#include "..\ImagePPHeaders.h"
#include "SMCesiumPublisher.h"
#include "TilePublisher\MeshTile.h"
#include "TilePublisher\TilePublisher.h"
#include "..\ScalableMeshQuery.h"
#include "SMStreamingDataStore.h"
#include "..\ScalableMesh.h"

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

void SMCesiumPublisher::_ExtractPublishNodeHeader(IScalableMeshNodePtr nodePtr, Json::Value& smJsonHeader)
    {
    auto smNodeHeader = dynamic_cast<ScalableMeshNode<DPoint3d>*>(nodePtr.get())->GetNodePtr()->m_nodeHeader;
    SMStreamingStore<DRange3d>::SerializeHeaderToJSON(&smNodeHeader, smNodeHeader.m_id, smJsonHeader/*["node"]*/);
    }

void SMCesiumPublisher::_ExtractPublishMasterHeader(IScalableMeshPtr smPtr, Json::Value& smJsonMasterHeader)
    {
    if (smPtr == nullptr)
        {
        BeAssert(!"Trying to extract the master header of an invalid ScalableMesh");
        return;
        }

    auto const& index = static_cast<ScalableMesh<DPoint3d>&>(*smPtr).GetMainIndexP();
    smJsonMasterHeader["Balanced"] = index->IsBalanced();
    smJsonMasterHeader["SplitTreshold"] = index->GetSplitTreshold();
    smJsonMasterHeader["Depth"] = index->GetDepth();
    smJsonMasterHeader["MeshDataDepth"] = index->GetTerrainDepth();
    smJsonMasterHeader["IsTerrain"] = index->IsTerrain();
    smJsonMasterHeader["DataResolution"] = index->GetResolution();
    smJsonMasterHeader["IsTextured"] = (uint32_t)index->IsTextured();
    }