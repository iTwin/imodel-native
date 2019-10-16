#include "ScalableMeshPCH.h"
#include "../ImagePPHeaders.h"
#include "SMCesiumPublisher.h"
#include "TilePublisher/MeshTile.h"
#include "TilePublisher/TilePublisher.h"
#include "../ScalableMeshQuery.h"
#include "SMStreamingDataStore.h"
#include "../ScalableMesh.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, const Transform& tranform, bvector<Byte>& outData, bool outputTexture)
    {
    size_t siblingIndex = 0;
    TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), tranform/*Transform::FromIdentity()*/, siblingIndex, parent, nullptr, -1, false, outputTexture);
    auto meshes = tileNode->GenerateMeshes();
    if (!meshes.empty())
        {
        TilePublisher publisher(*tileNode, nullptr, nullptr);
        publisher.Publish(*reinterpret_cast<TileMesh*>(&*meshes[0]), outData);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SMCesiumPublisher::_Publish(IScalableMeshNodePtr nodePtr, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const Transform& transform, bvector<Byte>& outData, bool outputTexture)
    {
    size_t siblingIndex = 0;
    TileNodeP parent = nullptr;
    TileNodePtr tileNode = new ScalableMeshTileNode(nodePtr, nodePtr->GetNodeExtent(), transform, siblingIndex, parent, clips, coverageID, isClipBoundary, outputTexture);
    TilePublisher publisher(*tileNode, nullptr/*sourceGCS*/, nullptr/*destinationGCS*/);
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

    auto& sm = static_cast<ScalableMesh<DPoint3d>&>(*smPtr);
    auto const& index = sm.GetMainIndexP();
    smJsonMasterHeader["Balanced"] = index->IsBalanced();
    smJsonMasterHeader["SplitTreshold"] = Json::Value(index->GetSplitTreshold());
    smJsonMasterHeader["Depth"] = Json::Value(index->GetDepth());
    smJsonMasterHeader["MeshDataDepth"] = Json::Value(index->GetTerrainDepth());
    smJsonMasterHeader["IsTerrain"] = index->IsTerrain();
    smJsonMasterHeader["DataResolution"] = index->GetResolution();
    smJsonMasterHeader["IsTextured"] = (uint32_t)index->IsTextured();

    WString wktString;
    sm.GetDbFile()->GetWkt(wktString);
    if (!wktString.empty())
        smJsonMasterHeader["GCS"] = Utf8String(wktString.c_str());
    }
