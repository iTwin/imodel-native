#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>

#include <ImagePP/all/h/HFCException.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <ScalableMesh\IScalableMeshQuery.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>

#include "Stores\SMStreamingDataStore.h"

//#include <eigen\Eigen\Dense>
//#include <PCLWrapper\IDefines.h>
//#include <PCLWrapper\INormalCalculator.h>
#include "ScalableMeshQuery.h"
//#include "MeshingFunctions.h"
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <Mtg/MtgStructs.h>
#include <Geom/bsp/bspbound.fdf>
#include "ScalableMesh\ScalableMeshGraph.h"
#include <string>
#include <queue>
#include "ScalableMeshMesher.h"
#include <ctime>
#include <fstream>
#include "Edits/ClipUtilities.h"
#include "vuPolygonClassifier.h"
#include "LogUtils.h"
#include "Edits\Skirts.h"
#include <map>
#include <json/json.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH
#define SM_OUTPUT_MESHES_GRAPH 0

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Init()
    {
    m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    m_nodeHeader.m_graphID = ISMStore::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID.resize(1);
    m_nodeHeader.m_ptsIndiceID[0] = ISMStore::GetNullNodeID();

    m_nodeHeader.m_uvsIndicesID.resize(1);
    m_nodeHeader.m_uvsIndicesID[0] = ISMStore::GetNullNodeID();

    m_nodeHeader.m_uvID = ISMStore::GetNullNodeID();

    m_nodeHeader.m_textureID = ISMStore::GetNullNodeID();
    m_nodeHeader.m_ptsIndiceID[0] = GetBlockID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT,EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,                                  
                 SMMeshIndex<POINT, EXTENT>* meshIndex,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool textured,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(meshIndex->GetNextNodeId(), pi_SplitTreshold, pi_rExtent, filter, balanced, propagateDataDown, createdNodeMap)
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
             
#ifdef WIP_MESH_IMPORT        
    m_existingMesh = false;
#endif

    Init();

    m_nbClips = 0;

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                const EXTENT& pi_rExtent,
                const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode)
                : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr()))
    {
    m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
     
#ifdef WIP_MESH_IMPORT         
    m_existingMesh = false;
#endif

    m_nbClips = 0;

    Init(); 

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                                                                                     const EXTENT& pi_rExtent,
                                                                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                                                                     bool IsUnsplitSubLevel)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr()), IsUnsplitSubLevel)                                                                              
                                                                                     
    {
    m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();

     
#ifdef WIP_MESH_IMPORT         
    m_existingMesh = false;
#endif
    m_nbClips = 0;

    Init();

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                                                                                     HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,                 
                                                                                     SMMeshIndex<POINT, EXTENT>* meshIndex,
                                                                                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                                                                                     bool balanced,
                                                                                     bool textured,
                                                                                     bool propagateDataDown,
                                                                                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                                                                                     CreatedNodeMap*                      createdNodeMap)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(blockID, static_pcast<SMPointIndexNode<POINT, EXTENT>, SMMeshIndexNode<POINT, EXTENT>>(parent), filter, balanced, propagateDataDown, createdNodeMap)
                 
    {
    m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
     
#ifdef WIP_MESH_IMPORT                
    m_existingMesh = false;
#endif
    m_nbClips = 0;

    Init();

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::~SMMeshIndexNode()
    {
    if (!IsDestroyed())
        {
        // Unload self ... this should result in discard 
        Unload();
        Discard();
        }
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsGraphLoaded() const
    {
    return true;// m_isGraphLoaded;
    }

template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Destroy()
    {
    SMPointIndexNode::Destroy();

    if (GetBlockID().IsValid())
        {                
        GetMemoryPool()->RemoveItem(m_triIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriPtIndices, (uint64_t)m_SMIndex);
        ISMInt32DataStorePtr int32DataStore;
        bool result = dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetDataStore()->GetNodeDataStore(int32DataStore, &m_nodeHeader, SMStoreDataType::TriPtIndices);
        assert(result == true && int32DataStore.IsValid());      
        int32DataStore->DestroyBlock(GetBlockID());                        
        m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_texturePoolItemId, GetBlockID().m_integerID, SMStoreDataType::Texture, (uint64_t)m_SMIndex);
        ISMTextureDataStorePtr nodeTextureStore;
        result = dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetDataStore()->GetNodeDataStore(nodeTextureStore, &m_nodeHeader);
        assert(result == true && nodeTextureStore.IsValid());  
        nodeTextureStore->DestroyBlock(GetBlockID());                        
        m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_triUvIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriUvIndices, (uint64_t)m_SMIndex);                
        result = m_SMIndex->GetDataStore()->GetNodeDataStore(int32DataStore, &m_nodeHeader, SMStoreDataType::TriUvIndices);
        assert(result == true && int32DataStore.IsValid());      
        int32DataStore->DestroyBlock(GetBlockID());                                                                
        m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_uvCoordsPoolItemId, GetBlockID().m_integerID, SMStoreDataType::UvCoords, (uint64_t)m_SMIndex);
        ISMUVCoordsDataStorePtr nodeUVCoordsStore;
        result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeUVCoordsStore, &m_nodeHeader);
        assert(result == true && nodeUVCoordsStore.IsValid());      
        nodeUVCoordsStore->DestroyBlock(GetBlockID());                                                                        
        m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_displayMeshPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);
        m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_graphPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)m_SMIndex);
        ISMMTGGraphDataStorePtr nodeGraphStore;
        result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeGraphStore, &m_nodeHeader);
        assert(result == true && nodeGraphStore.IsValid());  
        nodeGraphStore->DestroyBlock(GetBlockID());
        m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        
        GetMemoryPool()->RemoveItem(m_diffSetsItemId, GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)m_SMIndex);
        ISDiffSetDataStorePtr nodeDiffsetStore;
        result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDiffsetStore, &m_nodeHeader);                                
        if (nodeDiffsetStore.IsValid()) nodeDiffsetStore->DestroyBlock(GetBlockID());
        m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_featurePoolItemId, GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t)m_SMIndex);
        ISMInt32DataStorePtr nodeFeatureStore;
        result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeFeatureStore, &m_nodeHeader, SMStoreDataType::LinearFeature);
        assert(result == true && nodeFeatureStore.IsValid());         
        if (nodeFeatureStore.IsValid()) nodeFeatureStore->DestroyBlock(GetBlockID());
        m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
       
        GetMemoryPool()->RemoveItem(m_dtmPoolItemId, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex);
        m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        }
                
    HINVARIANTS;

    return true;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    pNewNode->SetDirty(true);
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this), true));
    pNewNode->SetDirty(true);
    return pNewNode;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChildVirtual() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMIndexNodeVirtual<POINT,EXTENT,SMMeshIndexNode<POINT,EXTENT>>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    pNewNode->SetDirty(true);
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewChildNode(HPMBlockID blockID)
    {
    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, this, dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), m_filter, m_needsBalancing, false, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap);
    node->m_clipRegistry = m_clipRegistry;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(node);
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode)
    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT>> parent;

    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, parent, dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex), m_filter, m_needsBalancing, false, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap);

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(node);

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }
    
    return pNewNode;
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Discard()
    {
    HINVARIANTS;
    bool returnValue = true;
    
    if (!m_destroyed)
        {
             
        
        GetMemoryPool()->RemoveItem(m_triIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriPtIndices, (uint64_t)m_SMIndex);
        m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
                                        
        GetMemoryPool()->RemoveItem(m_texturePoolItemId, GetBlockID().m_integerID, SMStoreDataType::Texture, (uint64_t)m_SMIndex);
        m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_triUvIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriUvIndices, (uint64_t)m_SMIndex);
        m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_uvCoordsPoolItemId, GetBlockID().m_integerID, SMStoreDataType::UvCoords, (uint64_t)m_SMIndex);
        m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_graphPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)m_SMIndex);
        m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
       
        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_displayMeshPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);
        m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        
        GetMemoryPool()->RemoveItem(m_diffSetsItemId, GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)m_SMIndex);
        m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_featurePoolItemId, GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t)m_SMIndex);
        m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_dtmPoolItemId, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex);
        m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        }

    __super::Discard();

    HINVARIANTS;

    return returnValue;
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Load() const
    {
    std::lock_guard<std::mutex> lock(m_headerMutex);
    if (IsLoaded()) return;
    SMPointIndexNode<POINT, EXTENT>::Load();

    
    assert(m_triIndicesPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    assert(m_texturePoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    assert(m_triUvIndicesPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    assert(m_uvCoordsPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    assert(m_displayDataPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::SaveMeshToCloud(ISMDataStoreTypePtr<EXTENT>&    pi_pDataStore)
    {
    assert(pi_pDataStore != nullptr);

    if (!IsLoaded())
        Load();
    //auto* node = this;
    RunOnNextAvailableThread(std::bind([pi_pDataStore](SMMeshIndexNode<POINT, EXTENT>* node, size_t threadId) ->void
        {
#ifndef VANCOUVER_API
        // Save indices
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> indicePtr(node->GetPtsIndicePtr());

        if (indicePtr.IsValid() && indicePtr->size() > 0)
            {
            ISMInt32DataStorePtr faceIndDataStore;
            bool result = pi_pDataStore->GetNodeDataStore(faceIndDataStore, &node->m_nodeHeader, SMStoreDataType::TriPtIndices);
            assert(result == true); // problem getting the indice data store for streaming
            faceIndDataStore->StoreBlock(const_cast<int*>(&(*indicePtr)[0]), indicePtr->size(), node->GetBlockID());
            }

        if (node->m_nodeHeader.m_isTextured)
            {
            // Save UVs
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoordsPtr(node->GetUVCoordsPtr());

            if (uvCoordsPtr.IsValid() && uvCoordsPtr->size() > 0)
                {
                ISMUVCoordsDataStorePtr uvCoordDataStore;
                bool result = pi_pDataStore->GetNodeDataStore(uvCoordDataStore, &node->m_nodeHeader);
                assert(result == true); // problem getting the uv data store for streaming
                uvCoordDataStore->StoreBlock(const_cast<DPoint2d*>(&(*uvCoordsPtr)[0]), uvCoordsPtr->size(), node->GetBlockID());
                }

            // Save UVIndices
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(node->GetUVsIndicesPtr());

            if (uvIndicePtr.IsValid() && uvIndicePtr->size() > 0)
                {
                ISMInt32DataStorePtr uvIndiceDataStore;
                bool result = pi_pDataStore->GetNodeDataStore(uvIndiceDataStore, &node->m_nodeHeader, SMStoreDataType::TriUvIndices);
                assert(result == true); // problem getting the uvIndice data store for streaming
                uvIndiceDataStore->StoreBlock(const_cast<int*>(&(*uvIndicePtr)[0]), uvIndicePtr->size(), node->GetBlockID());
                }

            // Save texture
            ISMTextureDataStorePtr textureDataStore;
            bool result = node->m_SMIndex->GetDataStore()->GetNodeDataStore(textureDataStore, &node->m_nodeHeader);
            assert(result == true && textureDataStore.IsValid() && !textureDataStore.IsNull());
            auto countTextureData = textureDataStore->GetBlockDataCount(node->GetBlockID());
            if (countTextureData > 0)
                {
                bvector<uint8_t> textureData(countTextureData);
                size_t newCount = textureDataStore->LoadCompressedBlock(textureData, countTextureData, node->GetBlockID());
                ISMTextureDataStorePtr cloudTextureDataStore;
                bool result = pi_pDataStore->GetNodeDataStore(cloudTextureDataStore, &node->m_nodeHeader);
                assert(result == true && cloudTextureDataStore.IsValid() && !cloudTextureDataStore.IsNull());
                cloudTextureDataStore->StoreCompressedBlock(textureData.data(), newCount, node->GetBlockID());
                node->m_nodeHeader.m_blockSizes.push_back(SMIndexNodeHeader<EXTENT>::BlockSize{ newCount, 5 });
                }
            }
        // Save header and points (specific order must be kept to allow to fetch blob sizes for streaming performance)
        ISMDataStoreTypePtr<EXTENT> pDataStore(pi_pDataStore.get());
        node->SavePointDataToCloud(pDataStore);
#else
        assert(false && "Make this compile on Vancouver!");
#endif

        SetThreadAvailableAsync(threadId);
        }, this, std::placeholders::_1));

    if (m_pSubNodeNoSplit != nullptr)
        {
        static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->SaveMeshToCloud(pi_pDataStore);
        }
    else
        {
        for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
            {
            if (m_apSubNodes[indexNode] != nullptr)
                {
                static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->SaveMeshToCloud(pi_pDataStore);
                }
            }
        }
    if (m_nodeHeader.m_level == 0)
        WaitForThreadStop();
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadTreeNode(size_t& nLoaded, int level, bool headersOnly)
    {
    if (!IsLoaded())
        Load();

    nLoaded++;
    RunOnNextAvailableThread(std::bind([headersOnly](SMMeshIndexNode<POINT, EXTENT>* node, size_t threadId) ->void
        {
        if (!headersOnly)
            {
            if (node->GetNbPoints() > 0)
                {
                // Points
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

                // Indices
                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> indicePtr(node->GetPtsIndicePtr());

                if (node->m_nodeHeader.m_isTextured)
                    {
                    // UVs
                    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoordsPtr(node->GetUVCoordsPtr());

                    // UVIndices
                    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(node->GetUVsIndicesPtr());

                    // Texture
                    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> texturePtr(node->GetTexturePtr());
                    }
                }
            }
        SetThreadAvailableAsync(threadId);
        }, this, std::placeholders::_1));

    if (level != 0 && this->GetLevel() + 1 > level) return;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->LoadTreeNode(nLoaded, level, headersOnly);
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->LoadTreeNode(nLoaded, level, headersOnly);
                }

            }
        }
    if (m_nodeHeader.m_level == 0)
        WaitForThreadStop();

    }

#ifdef INDEX_DUMPING_ACTIVATED
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                             bool pi_OnlyLoadedNode) const
    {
    if ((pi_OnlyLoadedNode == true) && (IsLoaded() == false))
        return;

    if (!IsLoaded())
        Load();

    char   TempBuffer[3000];
    int    NbChars;
    size_t NbWrittenChars;
    __int64 nodeId;

    if (GetBlockID().IsValid())
        {
        nodeId = GetBlockID().m_integerID;
        }
    else
        {
        nodeId = ISMStore::GetNullNodeID();
        }

    NbChars = sprintf(TempBuffer, "<ChildNode NodeId=\"%lli\" TotalPoints=\"%lli\" SplitDepth=\"%zi\" ArePoints3d=\"%i\">", nodeId, GetCount(), GetSplitDepth(), m_nodeHeader.m_arePoints3d ? 1 : 0);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    //Extent
    NbChars = sprintf(TempBuffer,
                      "<NodeExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></NodeExtent>\n",
                      ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (m_nodeHeader.m_contentExtentDefined)
        {
        NbChars = sprintf(TempBuffer,
                          "<ContentExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></ContentExtent>\n",
                          ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent));

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }


    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfPoints>%u</NbOfPoints>\n", GetNbObjects());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Cumulative Number Of Points
    NbChars = sprintf(TempBuffer, "<CumulNbOfPoints>%llu</CumulNbOfPoints>\n", GetCount());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfIndexes>%zu</NbOfIndexes>\n", m_nodeHeader.m_nbFaceIndexes);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Level
    NbChars = sprintf(TempBuffer, "<Level>%zi</Level>\n", m_nodeHeader.m_level);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // SplitTreshold
    NbChars = sprintf(TempBuffer, "<SplitTreshold>%zi</SplitTreshold>", m_nodeHeader.m_SplitTreshold);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // Balanced
    if (m_nodeHeader.m_balanced)
        NbChars = sprintf(TempBuffer, "<Balanced>true</Balanced>");
    else
        NbChars = sprintf(TempBuffer, "<Balanced>false</Balanced>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    //View Dependent Metrics
    /*
    NbChars = sprintf(TempBuffer,
    "<ViewDependentMetrics>%.3f</ViewDependentMetrics>",
    m_nodeHeader.m_ViewDependentMetrics[0]);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);
    */


    // Neighbor Node    
    NbChars = sprintf(TempBuffer, "<NeighborNode> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        for (size_t neighborInd = 0; neighborInd < m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
            {
            NbChars = sprintf(TempBuffer, "P %zi I %zi Id %lli ", neighborPosInd, neighborInd, m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd].m_integerID);

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        }

    NbChars = sprintf(TempBuffer, "</NeighborNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //GraphID
    NbChars = sprintf(TempBuffer, "<GraphID>%llu</GraphID>\n", m_nodeHeader.m_graphID.m_integerID);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t i = 0; i < m_nodeHeader.m_ptsIndiceID.size(); ++i)
        {
        NbChars = sprintf(TempBuffer, "<IndiceID>%llu</IndiceID>\n", m_nodeHeader.m_ptsIndiceID[i].m_integerID);

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }

    
    // Neighbor Stitching    
    NbChars = sprintf(TempBuffer, "<NeighborStitching> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        if (m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] == true)
            {
            NbChars = sprintf(TempBuffer, "1 ");

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        else
            {
            NbChars = sprintf(TempBuffer, "0 ");

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        }

    NbChars = sprintf(TempBuffer, "</NeighborStitching>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
            }
        else
            {
            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
                }
            }
        }

    NbChars = sprintf(TempBuffer, "</ChildNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

        
    }

#endif
//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Unload() 
    {
    SMPointIndexNode<POINT, EXTENT>::Unload();
    }


template <class POINT, class EXTENT> ISMMTGGraphDataStorePtr SMMeshIndexNode<POINT, EXTENT>::GetGraphStore() const
    {
    ISMMTGGraphDataStorePtr nodeDataStore;
    bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
    assert(result == true);  

    return nodeDataStore;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushPtsIndices(const int32_t* indices, size_t size)
    {    
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndicePtr = GetPtsIndicePtr();
    bool result = ptsIndicePtr->push_back(indices, size);
    assert(result);
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ReplacePtsIndices(const int32_t* indices, size_t size)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndicePtr = GetPtsIndicePtr();
    ptsIndicePtr->clear();
    bool result = ptsIndicePtr->push_back(indices, size);
    assert(result);
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ClearPtsIndices()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndicePtr = GetPtsIndicePtr();
    ptsIndicePtr->clear();    
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushUV( const DPoint2d* uv, size_t size) 
    {
  /*  for (size_t i = 0; i < size; ++i)
        {
        if (uv[i].x < -0.00000001 || uv[i].x > 1.00001 || uv[i].y < -0.000001 || uv[i].y > 1.00001 || std::isnan(uv[i].x) || std::isnan(uv[i].y))
            std::cout << "wrong uv " << std::endl;
        }*/
    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoordsPtr = GetUVCoordsPtr();    
    bool result = uvCoordsPtr->push_back(uv, size);
    assert(result);    
    m_nodeHeader.m_uvID = GetBlockID();    
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushTexture(const Byte* texture, size_t size)
    {
    assert(!GetTexturePtr().IsValid());

    ISMTextureDataStorePtr nodeDataStore;
    bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
    assert(result == true);  

    RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
#ifndef VANCOUVER_API
        new SMStoredMemoryPoolBlobItem<Byte>(GetBlockID().m_integerID, nodeDataStore, texture, size, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
#else
        SMStoredMemoryPoolBlobItem<Byte>::CreateItem(GetBlockID().m_integerID, nodeDataStore, texture, size, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
#endif
        );
    SMMemoryPoolItemBasePtr poolItem(storedMemoryPoolVector.get());
    m_texturePoolItemId = GetMemoryPool()->AddItem(poolItem);
    assert(m_texturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId);  
    m_nodeHeader.m_isTextured = true;
    m_nodeHeader.m_textureID = GetBlockID();
    m_nodeHeader.m_nbTextures = 1;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushUVsIndices(size_t texture_id, const int32_t* uvsIndices, size_t size)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvPtsIndicePtr = GetUVsIndicesPtr();    
    bool result = uvPtsIndicePtr->push_back(uvsIndices, size);
    assert(result);
    //assert(m_nodeHeader.m_uvsIndicesID.size() == 0);
    m_nodeHeader.m_uvsIndicesID.push_back(GetBlockID());
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndexNode<POINT, EXTENT>::GetMesher2_5d() const
    {
    if (!IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_mesher2_5d);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndexNode<POINT, EXTENT>::GetMesher3d() const
    {
    if (!IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_mesher3d);
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Mesh()
    {
    if (!IsLoaded())
        Load();


    HINVARIANTS;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (HasRealChildren())
        {
        if (IsParentOfARealUnsplitNode())
            {
#ifdef __HMR_DEBUG
            if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                this->m_parentOfAnUnspliteableNode = true;
#endif                        

            if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsMeshing())
                static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Mesh();
            }
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
#ifdef __HMR_DEBUG
                if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                    this->m_parentOfAnUnspliteableNode = true;
#endif
                if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsMeshing())
                    {
                    /*if (s_useThreadsInMeshing && m_nodeHeader.m_level == 0 && !m_nodeHeader.m_arePoints3d)
                        {
                        RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node,size_t threadId) ->void
                            {
                            node->Mesh();
                            SetThreadAvailableAsync(threadId);
                            }, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode])), std::placeholders::_1));
                        }
                    else*/ static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Mesh();
                    }
                }

            }

        }
    else
        {
        assert(this->NeedsMeshing() == true);
        //assert(this->m_nodeHeader.m_balanced == true);
        if (s_useThreadsInMeshing)
            {
            RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, size_t threadId) ->void
                {
                bool isMeshed;
                if (node->m_nodeHeader.m_arePoints3d)
                    {
                    isMeshed = node->m_mesher3d->Mesh(node);
                    }
                else
                    {
                    isMeshed = node->m_mesher2_5d->Mesh(node);
                    }

                if (isMeshed)
                    {
                    node->SetDirty(true);
                    }
                SetThreadAvailableAsync(threadId);
                }, this, std::placeholders::_1));
            }
        else
            {
            bool isMeshed;
            if (m_nodeHeader.m_arePoints3d)
                {
                isMeshed = this->m_mesher3d->Mesh(this);
                }
            else
                {
                isMeshed = this->m_mesher2_5d->Mesh(this);
                }

            if (isMeshed)
                {
                this->SetDirty(true);
                }
            }
        }

    if (m_nodeHeader.m_level == 0 && s_useThreadsInMeshing)
        WaitForThreadStop();
    // Now filtering can be performed using the sub-nodes filtered data. This data
    // accessed using the HPMPooledVector interface the Node is a descendant of.
    // Do not hesitate to increase the HPMPooledVector interface if required.
    // The result of the filtering must be added to the Node itself in the
    // HPMVectorPool<POINT> descendant class using the push_back interface.
    // If refiltering is required then clear() must be called beforehand.
    // The member m_nodeHeader.m_filtered should be set to true after the filtering process
    // All members that must be serialized in the file must be added in the
    // m_nodeHeader fields/struct and these will automatically be serialized in the
    // store. Note that changing this structure automatically
    // renders invalid any previous file.

    ValidateInvariants();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch)
    {
    if (!IsLoaded())
        Load();

    HINVARIANTS;

//    size_t nodeInd;

    if (pi_levelToStitch == -1 || this->m_nodeHeader.m_level == pi_levelToStitch && this->GetNbObjects() > 0)
        {
            if (nodesToStitch != 0)
                {
                nodesToStitch->push_back(this);
                }
            else
                {
#if 0
                bool wait = true;
                while (wait)
                    {
                    for (size_t t = 0; t < 8; ++t)
                        {
                        bool expected = false;
                        if (s_areThreadsBusy[t].compare_exchange_weak(expected, true))
                            {
                            wait = false;
                            size_t threadId = t;
                            std::atomic<bool>* areThreadsBusy = s_areThreadsBusy;
                            std::thread* threadP = s_threads;
                            s_threads[t] = std::thread([threadId, this, areThreadsBusy, threadP] ()
                                {
                                bool isStitched;

                                if (this->AreAllNeighbor2_5d() && !this->m_nodeHeader.m_arePoints3d)
                                    {
                                    isStitched = this->m_mesher2_5d->Stitch(this);
                                    }
                                else
                                    {
                                    isStitched = this->m_mesher3d->Stitch(this);
                                    }

                                if (isStitched)
                                    this->SetDirty(true);
                                SetThreadAvailableAsync(threadId);
                                /*  std::thread t = std::thread([areThreadsBusy, threadId, threadP] ()
                                      {
                                      threadP[threadId].join();
                                      bool expected = true;
                                      areThreadsBusy[threadId].compare_exchange_strong(expected, false);
                                      assert(expected);
                                      });
                                      t.detach();*/
                                });
                            break;
                            }
                        }
                    }
#elif 0
                RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, size_t threadId)
                    {
                    bool isStitched;

                    if (node->AreAllNeighbor2_5d() && !node->m_nodeHeader.m_arePoints3d)
                        {
                        isStitched = node->m_mesher2_5d->Stitch(node);
                        }
                    else
                        {
                        isStitched = node->m_mesher3d->Stitch(node);
                        }

                    if (isStitched)
                        node->SetDirty(true);
                    SetThreadAvailableAsync(threadId);
                    },node);
#else
                bool isStitched;

                if (AreAllNeighbor2_5d() && !this->m_nodeHeader.m_arePoints3d)
                    {
                    isStitched = m_mesher2_5d->Stitch(this);
                    }
                else
                    {
                    isStitched = m_mesher3d->Stitch(this);
                    }

                if (isStitched)
                    SetDirty(true);
#endif
                }
            }
       // }

        if (pi_levelToStitch == -1 || (int)this->m_nodeHeader.m_level < pi_levelToStitch)
            {
            if (!m_nodeHeader.m_IsLeaf)
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
#ifdef __HMR_DEBUG
                    if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                        this->m_parentOfAnUnspliteableNode = true;
#endif

                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Stitch(pi_levelToStitch, nodesToStitch);

                    }
                else
                    {
                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
#ifdef __HMR_DEBUG
                        if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                            this->m_parentOfAnUnspliteableNode = true;
#endif                
                        if (this->m_nodeHeader.m_level == 0 && nodesToStitch == 0 && pi_levelToStitch > 1 && s_useThreadsInStitching)
                            {
                            RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, int pi_levelToStitch, size_t threadId) ->void
                                {
                                node->Stitch(pi_levelToStitch, 0);
                                SetThreadAvailableAsync(threadId);
                                }, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode])), pi_levelToStitch, std::placeholders::_1));
                            }
                        else static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Stitch(pi_levelToStitch, nodesToStitch);
                        }
                    }
                }
            }
        //don't return until all threads are done
        if (m_nodeHeader.m_level == 0 && nodesToStitch == 0 && s_useThreadsInStitching)
            WaitForThreadStop();
       /* if (m_nodeHeader.m_level == 0 && pi_levelToStitch == 0)
            {
            m_nodeHeader.m_totalCountDefined = false;
            }*/
        // Now filtering can be performed using the sub-nodes filtered data. This data
        // accessed using the HPMPooledVector interface the Node is a descendant of.
        // Do not hesitate to increase the HPMPooledVector interface if required.
        // The result of the filtering must be added to the Node itself in the
        // HPMVectorPool<POINT> descendant class using the push_back interface.
        // If refiltering is required then clear() must be called beforehand.
        // The member m_nodeHeader.m_filtered should be set to true after the filtering process
        // All members that must be serialized in the file must be added in the
        // m_nodeHeader fields/struct and these will automatically be serialized in the
        // store. Note that changing this structure automatically
        // renders invalid any previous file.

        ValidateInvariants();
    }




static size_t s_featuresAddedToTree = 0;

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class EXTENT> void ClipFeatureDefinition(ISMStore::FeatureType type, EXTENT clipExtent, bvector<DPoint3d>& points, DRange3d& extent, const bvector<DPoint3d>& origPoints, DRange3d& origExtent)
    {

    if (/*IsClosedFeature(type) ||*/ (origExtent.low.x >= ExtentOp<EXTENT>::GetXMin(clipExtent) && origExtent.low.y >= ExtentOp<EXTENT>::GetYMin(clipExtent) && origExtent.low.z >= ExtentOp<EXTENT>::GetZMin(clipExtent)
        && origExtent.high.x <= ExtentOp<EXTENT>::GetXMax(clipExtent) && origExtent.high.y <= ExtentOp<EXTENT>::GetYMax(clipExtent) && origExtent.high.z <= ExtentOp<EXTENT>::GetZMax(clipExtent)))
        {
        points.insert(points.end(), origPoints.begin(), origPoints.end());
        extent = origExtent;
        return;
        }
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(clipExtent), ExtentOp<EXTENT>::GetYMin(clipExtent), ExtentOp<EXTENT>::GetZMin(clipExtent),
                                        ExtentOp<EXTENT>::GetXMax(clipExtent), ExtentOp<EXTENT>::GetYMax(clipExtent), ExtentOp<EXTENT>::GetZMax(clipExtent));
    if (IsClosedFeature(type))
        {
        DPoint3d origins[6];
        DVec3d normals[6];
        nodeRange.Get6Planes(origins, normals);
        DPlane3d planes[6];
        for (size_t i = 0; i < 6; ++i)
            {
            planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);
            }

        points.insert(points.end(), origPoints.begin(), origPoints.end());
        for (auto& plane : planes)
            {
            double sign = 0;
            bool planeCutsPoly = false;
            for (size_t j = 0; j < points.size() && !planeCutsPoly; j++)
                {
                double sideOfPoint = plane.Evaluate(points[j]);
                if (fabs(sideOfPoint) < 1e-6) sideOfPoint = 0;
                if (sign == 0) sign = sideOfPoint;
                else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
                    planeCutsPoly = true;
                }
            if (!planeCutsPoly) continue;                
            bvector<DPoint3d> points2(points.size() + 10);
            int nPlaneClipSize = (int)points2.size();
            int nLoops = 0;
            bsiPolygon_clipToPlane(&points2[0], &nPlaneClipSize, &nLoops, (int)points2.size(), &points[0], (int)points.size(), &plane);
            if (nPlaneClipSize > 0)
                {
                points.clear();
                points2.resize(nPlaneClipSize);
                for (auto& pt : points2)
                    {
                    if (pt.x < DBL_MAX)
                        points.push_back(pt);
                    //else break;
                    }
                }
            }
        extent = DRange3d::From(points);
        return;
        }
    if (IsLinearFeature(type))
        {
        DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
        bool withinExtent = false;
        for (size_t pt = 0; pt < origPoints.size(); ++pt)
            {
            bool isPointInRange = nodeRange.IsContained(origPoints[pt]);
            if (!withinExtent && isPointInRange && pt > 0)
                {
                points.push_back(origPoints[pt - 1]);
                }
            if (isPointInRange)
                {
                if (points.size() == 0) extent = DRange3d::From(origPoints[pt]);
                else extent.Extend(origPoints[pt]);
                points.push_back(origPoints[pt]);
                }
            if (!isPointInRange && withinExtent && pt < origPoints.size())
                {
                points.push_back(origPoints[pt]);
                points.push_back(SENTINEL_PT);
                }
            withinExtent = isPointInRange;
            }
        }
    }

template<class POINT> void SimplifyMesh(bvector<int32_t>& indices, bvector<POINT>& points, bvector<DPoint2d>& uvs)
    {

    std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPts(DPoint3dZYXTolerancedSortComparison(1e-4, 0));
    vector<int32_t> matchedIndices(points.size(), -1);
    vector<int32_t> newIndices(points.size(), -1);
    for (auto& pt : points)
        {
        DPoint3d pt3d = DPoint3d::From(PointOp<POINT>::GetX(pt), PointOp<POINT>::GetY(pt), PointOp<POINT>::GetZ(pt));
        if (mapOfPts.count(pt3d) == 0)
            mapOfPts[pt3d] = &pt - &points[0];
        matchedIndices[&pt - &points[0]] = mapOfPts[pt3d];
        }
    bvector<POINT> newSet;
    bvector<DPoint2d> newUvs;
    newSet.reserve(points.size());

    for (size_t j = 0; j < indices.size(); j += 3)
        {
        for (size_t k = 0; k < 3; ++k)
            {
            auto& idx = indices[j + k];
            if (newIndices[matchedIndices[idx - 1]] == -1)
                {
                newSet.push_back(PointOp<POINT>::Create(points[idx - 1].x, points[idx - 1].y, points[idx - 1].z));
                if(uvs.size() > 0) newUvs.push_back(uvs[idx - 1]);
                newIndices[matchedIndices[idx - 1]] = (int)newSet.size() - 1;
                }
            idx = newIndices[matchedIndices[idx - 1]] + 1;
            }
        if (indices[j] == indices[j + 1] || indices[j] == indices[j + 2] || indices[j + 1] == indices[j + 2])
            {
            indices.erase(indices.begin() + j, indices.begin() + j + 3);
            j -= 3;
            }
        }
    points = newSet;
    uvs = newUvs;
    }

template<class EXTENT> void ClipMeshDefinition(EXTENT clipExtent, bvector<DPoint3d>& pointsClipped, DRange3d& extentClipped, bvector<int32_t>& indicesClipped, bvector<DPoint2d>& inOutUvs, const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent)
    {
    if ((extent.low.x >= ExtentOp<EXTENT>::GetXMin(clipExtent) && extent.low.y >= ExtentOp<EXTENT>::GetYMin(clipExtent) && extent.low.z >= ExtentOp<EXTENT>::GetZMin(clipExtent)
        && extent.high.x <= ExtentOp<EXTENT>::GetXMax(clipExtent) && extent.high.y <= ExtentOp<EXTENT>::GetYMax(clipExtent) && extent.high.z <= ExtentOp<EXTENT>::GetZMax(clipExtent)))
        {
        pointsClipped.insert(pointsClipped.end(), pts, pts + nPts);
        indicesClipped.insert(indicesClipped.end(), indices, indices + nIndices);
        extentClipped = extent;
        return;
        }
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(clipExtent), ExtentOp<EXTENT>::GetYMin(clipExtent), ExtentOp<EXTENT>::GetZMin(clipExtent),
                                        ExtentOp<EXTENT>::GetXMax(clipExtent), ExtentOp<EXTENT>::GetYMax(clipExtent), ExtentOp<EXTENT>::GetZMax(clipExtent));

    IScalableMeshMeshPtr meshPtr = IScalableMeshMesh::Create(nPts, const_cast<DPoint3d*>(pts), nIndices, indices, 0, 0, 0, 0, 0, 0);
    ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
    vector<int32_t> newIndices;
    bvector<DPoint3d> origPoints;
    vector<DPoint3d> meshPts;
    for (size_t i = 0; i < nPts; ++i)
        {
        meshPts.push_back(pts[i]);
        origPoints.push_back(pts[i]);
        }

    
    ClipMeshToNodeRange<DPoint3d, EXTENT>(newIndices, meshPts, origPoints, inOutUvs, extentClipped, nodeRange, meshP);
    if (newIndices.size() == 0) return;
    bvector<int32_t> ptMap(meshPts.size(), -1);
    bvector<DPoint2d> tempUvs;
    for (auto& idx : newIndices)
        {
        assert(idx - 1 < ptMap.size());
        if (ptMap[idx - 1] == -1)
            {
            pointsClipped.push_back(meshPts[idx - 1]);
            extentClipped.Extend(pointsClipped.back());
            if (inOutUvs.size() > 0) tempUvs.push_back(inOutUvs[idx - 1]);
            ptMap[idx - 1] = (int)pointsClipped.size() - 1;
            }
        indicesClipped.push_back(ptMap[idx - 1] + 1);
        }
    inOutUvs = tempUvs;
    }

template<class EXTENT> void ClipMeshDefinition(EXTENT clipExtent, bvector<DPoint3d>& pointsClipped, DRange3d& extentClipped, bvector<int32_t>& indicesClipped, bvector<DPoint2d>& inOutUvs, const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, bvector<int64_t>& texIds, bvector<int>& parts)
    {
    if ((extent.low.x >= ExtentOp<EXTENT>::GetXMin(clipExtent) && extent.low.y >= ExtentOp<EXTENT>::GetYMin(clipExtent) && extent.low.z >= ExtentOp<EXTENT>::GetZMin(clipExtent)
        && extent.high.x <= ExtentOp<EXTENT>::GetXMax(clipExtent) && extent.high.y <= ExtentOp<EXTENT>::GetYMax(clipExtent) && extent.high.z <= ExtentOp<EXTENT>::GetZMax(clipExtent)))
        {
        pointsClipped.insert(pointsClipped.end(), pts, pts+nPts);
        indicesClipped.insert(indicesClipped.end(), indices, indices + nIndices);
        extentClipped = extent;
        return;
        }
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(clipExtent), ExtentOp<EXTENT>::GetYMin(clipExtent), ExtentOp<EXTENT>::GetZMin(clipExtent),
                                        ExtentOp<EXTENT>::GetXMax(clipExtent), ExtentOp<EXTENT>::GetYMax(clipExtent), ExtentOp<EXTENT>::GetZMax(clipExtent));

    IScalableMeshMeshPtr meshPtr = IScalableMeshMesh::Create(nPts, const_cast<DPoint3d*>(pts), nIndices,indices, 0, 0, 0, 0, 0, 0);
    ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
    vector<int32_t> newIndices;
    bvector<DPoint3d> origPoints;
    vector<DPoint3d> meshPts;
    for (size_t i = 0; i < nPts; ++i)
        {
        meshPts.push_back(pts[i]);
        origPoints.push_back(pts[i]);
        }

    bvector<int64_t> faceIds;
    ClipMeshToNodeRange<DPoint3d, EXTENT>(newIndices, meshPts, origPoints, inOutUvs, extentClipped, nodeRange, meshP, parts, texIds, faceIds);
    if (newIndices.size() == 0) return;
    bvector<int32_t> ptMap(meshPts.size(), -1);
    bvector<DPoint2d> tempUvs;
    bvector<int> newParts;
    bvector<int64_t> newTexIds;
    int64_t currentTexId = -1;
    for (auto& idx : newIndices)
        {
        if (faceIds[(&idx - &newIndices[0]) / 3] != currentTexId)
            {
            if (currentTexId != -1)
                {
                newTexIds.push_back(currentTexId);
                newParts.push_back((int)indicesClipped.size());
                }
            currentTexId = faceIds[(&idx - &newIndices[0]) / 3];
            }
        assert(idx - 1 < ptMap.size());
        if (ptMap[idx - 1] == -1)
            {
            pointsClipped.push_back(meshPts[idx - 1]);
            extentClipped.Extend(pointsClipped.back());
            if(inOutUvs.size() > 0) tempUvs.push_back(inOutUvs[idx - 1]);
            ptMap[idx - 1] = (int)pointsClipped.size() - 1;
            }
        indicesClipped.push_back(ptMap[idx - 1] + 1);
        }
    newTexIds.push_back(currentTexId);
    newParts.push_back((int)indicesClipped.size());
    inOutUvs = tempUvs;
    parts = newParts;
    texIds = newTexIds;
   // SimplifyMesh(indicesClipped, pointsClipped, inOutUvs);
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ReadFeatureDefinitions(bvector<bvector<DPoint3d>>& points, bvector<DTMFeatureType> & types)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    bvector<bvector<int32_t>> defs;
    if (linearFeaturesPtr->size() > 0) GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
    for (size_t i = 0; i < defs.size(); ++i)
        {
        bvector<DPoint3d> feature;
        if (!IsClosedFeature(defs[i][0])) continue;
        for (size_t j = 1; j < defs[i].size(); ++j)
            {
            if (defs[i][j] < GetPointsPtr()->size()) feature.push_back(this->GetPointsPtr()->operator[](defs[i][j]));
            }
        if (IsClosedFeature(defs[i][0]) &&feature.size() > 0)
            if (!feature.back().AlmostEqual(feature.front()))
                feature.push_back(feature.front());
        points.push_back(feature);
        types.push_back((DTMFeatureType)defs[i][0]);
        }
    }

template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinitionSingleNode(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
    {
    vector<int32_t> indexes;
    //DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
     //                                   ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
    for (auto pt : points)
        {
        if (pt.x == DBL_MAX)
            {
            indexes.push_back(INT_MAX);
            continue;
            }
        POINT pointToInsert = PointOp<POINT>::Create(pt.x, pt.y, pt.z);
        this->GetPointsPtr()->push_back(pointToInsert);
        indexes.push_back((int32_t)this->GetPointsPtr()->size() - 1);
        }

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    linearFeaturesPtr->push_back((int)indexes.size()+1); //include type flag
    linearFeaturesPtr->push_back((int32_t)type);
    linearFeaturesPtr->push_back(&indexes[0], indexes.size());
    return 0;
    }

#ifdef WIP_MESH_IMPORT
extern ScalableMeshExistingMeshMesher<DPoint3d, DRange3d> s_ExistingMeshMesher;

template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddMeshDefinitionUnconditional(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs)
    {
    if (!IsLoaded())
        Load();
    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());

    DRange3d extentClipped = DRange3d::NullRange();
    bvector<DPoint3d> pointsClipped;
    bvector<int32_t> indicesClipped;
    bvector<DPoint2d> outUvs;
    if(uvs != nullptr)
        {
        outUvs.insert(outUvs.end(), uvs, uvs+nPts);
        }
    auto metadataString = Utf8String(metadata);
    Json::Value val;
    Json::Reader reader;
    reader.parse(metadataString, val);
    bvector<int> parts;
    bvector<int64_t> texId;
    for (const Json::Value& id : val["texId"])
        {
        texId.push_back(id.asInt64());
        }

    for (const Json::Value& id : val["parts"])
        {
        parts.push_back(id.asInt());
        }
    ClipMeshDefinition(m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, indicesClipped, outUvs, pts, nPts, indices, nIndices, extent,texId, parts);

    val["texId"] = Json::arrayValue;
    val["parts"] = Json::arrayValue;

    for(auto& id: texId) val["texId"].append(id);
    for(auto& id: parts) val["parts"].append(id);
    Utf8String metadataStr(Json::FastWriter().write(val));
    if (!m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) return 0;

    if (m_mesher2_5d != &s_ExistingMeshMesher || m_mesher3d != &s_ExistingMeshMesher) m_mesher2_5d = m_mesher3d = &s_ExistingMeshMesher;
    m_existingMesh = true;
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    if (!HasRealChildren()) m_nodeHeader.m_arePoints3d = true;
    if (m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(8);

    if (!HasRealChildren() && (pointsPtr->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold) /*&& m_nodeHeader.m_level <= 10*/)
        {
        // There are too much objects ... need to split current node
        SplitNode(GetDefaultSplitPosition());
        }
    else if (m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
        {
        PropagateDataDownImmediately(false);
        }
    if (pointsClipped.size() == 0) return false;


    m_nodeHeader.m_totalCount += pointsClipped.size();
    EXTENT featureExtent = ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    if (!m_nodeHeader.m_contentExtentDefined)
        {
        m_nodeHeader.m_contentExtent = featureExtent;
        m_nodeHeader.m_contentExtentDefined = true;
        }
    else
        {
        m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, featureExtent);
        }

    size_t added = 0;

    if (!HasRealChildren() || (m_delayedDataPropagation && ((pointsPtr->size() + pointsClipped.size() < m_nodeHeader.m_SplitTreshold)) /*||  m_nodeHeader.m_level > 10*/))
        {
        vector<int32_t> indexes;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

        added += pointsClipped.size();
        size_t offset = pointsPtr->size();
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  indicesPtr = GetPtsIndicePtr();
        m_meshParts.push_back((int)indicesPtr->size());
        size_t currentPart =0;
        pointsPtr->push_back(&pointsClipped[0], pointsClipped.size());
        for (auto& idx : indicesClipped)
            {
            if(&idx-&indicesClipped[0] >= parts[currentPart] ) 
                {
                m_meshParts.push_back((int)indicesPtr->size());
                Json::Value subMetadata = val;
                subMetadata["texId"] = Json::arrayValue;
                subMetadata["parts"] = Json::arrayValue;
                subMetadata["parts"].append((int)indicesPtr->size());
                subMetadata["texId"].append(texId[currentPart]);
                Utf8String subMetadataStr(Json::FastWriter().write(subMetadata));
                m_meshMetadata.push_back(subMetadataStr);
                m_meshParts.push_back((int)indicesPtr->size());
                currentPart++;
                }
            indicesPtr->push_back(idx+(int)offset);
            }

        m_meshParts.push_back((int)indicesPtr->size());
        val["texId"] = Json::arrayValue;
        val["parts"] = Json::arrayValue;
        val["parts"].append((int)indicesPtr->size());
        val["texId"].append(texId[currentPart]);
        metadataStr = Json::FastWriter().write(val);
        m_meshMetadata.push_back(metadataStr);

       
        if (uvs != nullptr)
            {
            m_nodeHeader.m_isTextured = true;
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>>  uvPtr = GetUVCoordsPtr();
            if(offset != 0 && uvPtr->size() < offset)
                {
                bvector<DPoint2d> dummyUvs(offset-uvPtr->size(),DPoint2d::From(0.0,0.0));
                PushUV(&dummyUvs[0], dummyUvs.size());
                }
            PushUV(&outUvs[0], outUvs.size());
            StoreMetadata();
            StoreMeshParts();
            }
        else
            {
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>>  uvPtr = GetUVCoordsPtr();
            if(uvPtr.IsValid())
                {
                bvector<DPoint2d> dummyUvs(pointsClipped.size(),DPoint2d::From(0.0,0.0));
                PushUV(&dummyUvs[0], dummyUvs.size());
                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(GetUVsIndicesPtr());
                uvIndicePtr->clear();
                uvIndicePtr->push_back(&(*indicesPtr)[0], indicesPtr->size());
                }
            }
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(GetUVsIndicesPtr());
        if(uvIndicePtr.IsValid())
            {
            uvIndicePtr->clear();
            uvIndicePtr->push_back(&(*indicesPtr)[0], indicesPtr->size());
            }
        }
    else
        {
        if (IsParentOfARealUnsplitNode())
            added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddMeshDefinitionUnconditional(&pointsClipped[0], pointsClipped.size(), &indicesClipped[0], indicesClipped.size(), extentClipped, metadataStr.c_str(), texData,texSize,outUvs.size() > 0 ? outUvs.data() : 0);
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNode])->AddMeshDefinition(&pointsClipped[0], pointsClipped.size(), &indicesClipped[0], indicesClipped.size(), extentClipped, true, metadataStr.c_str(), texData,texSize,outUvs.size() > 0 ? outUvs.data() : 0);
                }
            }
        }

    SetDirty(true);
    return added;
    }

template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, bool ExtentFixed, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs)
    {
    if (s_inEditing)
        {
        InvalidateFilteringMeshing();
        }
    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());

    if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
        {
        m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));

        if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) &&
            ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            }
        else
            if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) &&
                ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                }
            else
                if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) &&
                    ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent))
                    {
                    ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                    ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                    }

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

        if (nPts + pointsPtr->size() >= m_nodeHeader.m_SplitTreshold)
            {
            return AddMeshDefinition(pts, nPts, indices, nIndices, extent, true, metadata, texData,texSize,uvs);
            }
        else
            {
            return AddMeshDefinitionUnconditional(pts, nPts, indices, nIndices, extent, metadata, texData,texSize,uvs);
            }
        }
    else
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
        if (extent.IntersectsWith(nodeRange))
            {
            return AddMeshDefinitionUnconditional(pts, nPts, indices, nIndices, extent, metadata, texData,texSize,uvs);
            }
        }
    return 0;
    }

#endif

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
    template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinitionUnconditional(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
        {
        if (!IsLoaded())
            Load();
        if (m_DelayedSplitRequested)
            SplitNode(GetDefaultSplitPosition());
        
        DRange3d extentClipped;
        bvector<DPoint3d> pointsClipped;
        ClipFeatureDefinition(type, m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, points, extent);
        if (!m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) return 0;

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

        if (!HasRealChildren() && pointsPtr->size() == 0) m_nodeHeader.m_arePoints3d = false;
        if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);        

        if (!HasRealChildren() && (pointsPtr->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode(GetDefaultSplitPosition());
            }
        else if (m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            PropagateDataDownImmediately(false);
            }
        if (pointsClipped.size() == 0) return false;


        m_nodeHeader.m_totalCount += pointsClipped.size();
        EXTENT featureExtent = ExtentOp<EXTENT>::Create(extentClipped.low.x, extentClipped.low.y, extentClipped.low.z, extentClipped.high.x, extentClipped.high.y, extentClipped.high.z);
        if (!m_nodeHeader.m_contentExtentDefined)
            {
            m_nodeHeader.m_contentExtent = featureExtent;
            m_nodeHeader.m_contentExtentDefined = true;
            }
        else
            {
            m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, featureExtent);
            }

        size_t added = 0;
        
        if (!HasRealChildren() || (m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() < m_nodeHeader.m_SplitTreshold)))
            {
            ++s_featuresAddedToTree;
            vector<int32_t> indexes;
            DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
            
            for (auto pt : pointsClipped)
                {
                if (pt.x == DBL_MAX)
                    {
                    indexes.push_back(INT_MAX);
                    continue;
                    }
                if (nodeRange.IsContained(pt)) ++added;
                POINT pointToInsert = PointOp<POINT>::Create(pt.x, pt.y, pt.z);
                pointsPtr->push_back(pointToInsert);
                indexes.push_back((int32_t)pointsPtr->size()-1);
                }
           /* if (m_featureDefinitions.capacity() < m_featureDefinitions.size() +1) for(auto& def : m_featureDefinitions) if(!def.Discarded()) def.Discard();
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
            linearFeaturesPtr->push_back((int)indexes.size()+1);
            linearFeaturesPtr->push_back((int32_t)type);
            linearFeaturesPtr->push_back(&indexes[0], indexes.size());*/
            }
        else
            {
            if (IsParentOfARealUnsplitNode())
                added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional(type, pointsClipped, extentClipped);
            else
                {
                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
                        added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNode])->AddFeatureDefinition(type, pointsClipped, extentClipped, true);
                        }
                }
            }
     
        SetDirty(true);
        return added;
        }

    //=======================================================================================
    // @bsimethod                                                   Elenie.Godzaridis 08/15
    //=======================================================================================
    template<class POINT, class EXTENT>  size_t  SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed)
        {
        assert(points.size()>0);
        if (s_inEditing)
            {
            InvalidateFilteringMeshing();
            }
        if (m_DelayedSplitRequested)
            SplitNode(GetDefaultSplitPosition());

        if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
            {
            m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));

            if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) &&
                ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
                ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
                }
            else
                if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) &&
                    ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent))
                    {
                    ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                    ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
                    }
                else
                    if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) &&
                        ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent))
                        {
                        ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                        ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
                        }

            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

            if (points.size() + pointsPtr->size() >= m_nodeHeader.m_SplitTreshold)
                {
                return AddFeatureDefinition(type, points, extent,true);
                }
            else
                {
                return AddFeatureDefinitionUnconditional(type,points,extent);              
                }
        }
    else
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
        if (extent.IntersectsWith(nodeRange))
            {
            return AddFeatureDefinitionUnconditional(type, points, extent);
            }
        }
        return 0;

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  size_t SMMeshIndexNode<POINT, EXTENT>::CountAllFeatures()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();

    size_t nFeatures = IsLeaf() ? linearFeaturesPtr->size() : 0;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->CountAllFeatures();
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            assert(m_apSubNodes[indexNodes] != nullptr);
            nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->CountAllFeatures();
            }
        }
    return nFeatures;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPushNodeDown()
    {
    PropagateFeaturesToChildren();
#ifdef WIP_MESH_IMPORT
    PropagateMeshToChildren();
#endif
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPropagateDataDown()
    {
    PropagateFeaturesToChildren();
#ifdef WIP_MESH_IMPORT
    PropagateMeshToChildren();
#endif
    }

#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateMeshToChildren()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  indicesPtr = GetPtsIndicePtr();
    if (indicesPtr->size() == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
    bvector<IScalableMeshMeshPtr> allMeshes;
    bvector<Utf8String> metadata;
    bvector<bvector<uint8_t>> texData;
    GetMeshParts(allMeshes, metadata, texData);
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < allMeshes.size(); ++i)
            {
            if (!allMeshes[i].IsValid() || allMeshes[i]->GetNbFaces() == 0) continue;
            DRange3d extent = DRange3d::From(allMeshes[i]->EditPoints(), (int)allMeshes[i]->GetNbPoints());
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddMeshDefinitionUnconditional(allMeshes[i]->EditPoints(), allMeshes[i]->GetNbPoints(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCP(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCount(), extent, metadata[i].c_str(),texData[i].size() > 0 ? texData[i].data() : 0,texData[i].size(),allMeshes[i]->GetPolyfaceQuery()->GetParamCP());
            }
        }
    else if (!IsLeaf())
        {
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                for (size_t i = 0; i < allMeshes.size(); ++i)
                    {
                    if (!allMeshes[i].IsValid() || allMeshes[i]->GetNbFaces() == 0) continue;
                    DRange3d extent = DRange3d::From(allMeshes[i]->EditPoints(), (int)allMeshes[i]->GetNbPoints());
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddMeshDefinition(allMeshes[i]->EditPoints(), allMeshes[i]->GetNbPoints(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCP(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCount(), extent, true, metadata[i].c_str(),texData[i].size() > 0 ? texData[i].data() : 0,texData[i].size(),allMeshes[i]->GetPolyfaceQuery()->GetParamCP());
                    }
        }
    indicesPtr->clear();
    pointsPtr->clear();
    RefCountedPtr<SMMemoryPoolBlobItem<uint8_t>>  texPtr = GetTexturePtr();
    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvPtr = GetUVCoordsPtr();
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIPtr = GetUVsIndicesPtr();
    if(texPtr.IsValid() && texPtr->GetSize() > 0)
        {
        texPtr->SetData(0,0);
        m_nodeHeader.m_isTextured = 0;
        }
    if(uvPtr.IsValid() && uvPtr->size() > 0) uvPtr->clear();
    if(uvIPtr.IsValid() && uvIPtr->size() > 0) uvIPtr->clear();
    m_nodeHeader.m_isTextured = false;
    SetDirty(true);
    m_meshParts.clear();
    m_meshMetadata.clear();    
    }
#endif

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateFeaturesToChildren()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
    if (linearFeaturesPtr->size() == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
    bvector<bvector<int32_t>> defs;
    if (linearFeaturesPtr->size() > 0) GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
    bvector<bvector<DPoint3d>> featurePoints(defs.size());
    bvector<DRange3d> extents(defs.size());
    size_t featureId = 0;
    bvector<size_t> indices;
    vector<size_t> sentinels(defs.size());
    for (auto& feature : defs)
        {
        --s_featuresAddedToTree;
        for (size_t pt = 1; pt < feature.size(); ++pt)
            {
            if (feature[pt] < INT_MAX)
                {
                POINT featurePt = pointsPtr->operator[](feature[pt]);
                featurePoints[featureId].push_back(DPoint3d::From(PointOp<POINT>::GetX(featurePt), PointOp<POINT>::GetY(featurePt), PointOp<POINT>::GetZ(featurePt)));
                
                if (!nodeRange.IsContained(featurePoints[featureId].back())) ++sentinels[featureId];
                if (featurePoints[featureId].size() == 1) extents[featureId] = DRange3d::From(featurePoints[featureId].back());
                else extents[featureId].Extend(featurePoints[featureId].back());
                indices.push_back(feature[pt]);
                }
            else
                {
                featurePoints[featureId].push_back(SENTINEL_PT);
                ++sentinels[featureId];
                }
            }
        ++featureId;
        }    

    for (auto& index : indices)
        {        
        for (auto& pair : m_nodeHeader.m_3dPointsDescBins)
            {
            if (pair.m_startIndex > index) --pair.m_startIndex;
            }
        }

    pointsPtr->erase(indices);

    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional((ISMStore::FeatureType)defs[i][0], featurePoints[i], extents[i]);
        }
    else if (!IsLeaf())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            {
            size_t added = 0;
            if (featurePoints[i].size() <= 1) continue;
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddFeatureDefinition((ISMStore::FeatureType)defs[i][0], featurePoints[i], extents[i], true);

            assert(added >= featurePoints[i].size() - sentinels[i]);
            }
        }
/*    for (auto& vec : m_featureDefinitions)
        {
        vec.clear();
        vec.Discard();
        }
    m_featureDefinitions.clear();*/
    linearFeaturesPtr->clear();

    }
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ClipActionRecursive(ClipAction action, uint64_t clipId, DRange3d& extent,bool setToggledWhenIdIsOn)
    {
    if (!IsLoaded()) Load();
    if (/*size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3*/m_nodeHeader.m_totalCount == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
    if (!extent.IntersectsWith(nodeRange, 2)) return;
    bool clipApplied = true;
    switch (action)
        {
        case ClipAction::ACTION_ADD:
            clipApplied = AddClip(clipId, false, setToggledWhenIdIsOn);
            break;
        case ClipAction::ACTION_MODIFY:
            clipApplied = ModifyClip(clipId, false, setToggledWhenIdIsOn);
            break;
        case ClipAction::ACTION_DELETE:
            clipApplied = DeleteClip(clipId, false, setToggledWhenIdIsOn);
            break;
        }
    if (!clipApplied) return;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->ClipActionRecursive(action, clipId, extent, setToggledWhenIdIsOn);
        }
    else if (m_nodeHeader.m_numberOfSubNodesOnSplit > 1 && m_apSubNodes[0] != nullptr)
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->ClipActionRecursive(action, clipId, extent, setToggledWhenIdIsOn);
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::AddClipDefinitionRecursive(bvector<DPoint3d>& points, DRange3d& extent)
    {
    uint64_t id = -1;
    if (bsiGeom_getXYPolygonArea(&points[0], (int)points.size()) < 0) //need to flip polygon so it's counterclockwise
        {
        DPoint3d* flippedPts = new DPoint3d[points.size()];
        for (size_t pt = 0; pt < points.size(); ++pt) flippedPts[pt] = points[points.size() - 1 - pt];
        id = GetClipRegistry()->AddClip(flippedPts, points.size()) + 1;
        delete[] flippedPts;
        }
    else id = GetClipRegistry()->AddClip(&points[0], points.size()) + 1;
    bool wasClipAdded = AddClip(id, false);
    if (!wasClipAdded) return;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddClipDefinitionRecursive(points,extent);
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(m_apSubNodes[indexNodes] != nullptr)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddClipDefinitionRecursive(points,extent);
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::SplitMeshForChildNodes()
    {
    DRange3d contentRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent));
    IScalableMeshMeshPtr meshPtr = nullptr;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    for (auto& nodeP : m_apSubNodes)
        {        
        bvector<DPoint3d> pts(pointsPtr->size());
        vector<POINT> nodePts(pointsPtr->size());
        for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
            {
            pts[pointInd].x =pointsPtr->operator[](pointInd).x;
            pts[pointInd].y = pointsPtr->operator[](pointInd).y;
            pts[pointInd].z = pointsPtr->operator[](pointInd).z;
            nodePts[pointInd] = pointsPtr->operator[](pointInd);
            }
        
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(GetPtsIndicePtr());

        if (!ptIndices.IsValid() || ptIndices->size() <= 3)
            continue;

        IScalableMeshMeshPtr meshPtr = IScalableMeshMesh::Create(pointsPtr->size(), &pts[0], ptIndices->size(), &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
        ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
        vector<int32_t> childIndices;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeP->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeP->m_nodeHeader.m_nodeExtent));

        
        ClipMeshToNodeRange<POINT, EXTENT>(childIndices, nodePts, pts, nodeP->m_nodeHeader.m_contentExtent, nodeRange, meshP);
        if (childIndices.size() == 0) continue;
        
        DRange3d childContentRange;
        childContentRange.IntersectionOf(contentRange, nodeRange);
        nodeP->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::Create(childContentRange.low.x, childContentRange.low.y, childContentRange.low.z, childContentRange.high.x, childContentRange.high.y, childContentRange.high.z);
        nodeP->m_nodeHeader.m_contentExtentDefined = true;
        dynamic_pcast<SMMeshIndexNode<POINT,EXTENT>,SMPointIndexNode<POINT,EXTENT>>(nodeP)->PushPtsIndices(&childIndices[0], childIndices.size());                

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(nodeP->GetPointsPtr());
        pointsPtr->push_back(&nodePts[0], nodePts.size());
        nodeP->m_nodeHeader.m_totalCount = pointsPtr->size();
        nodeP->SetDirty(true);
        meshPtr = nullptr;
        }
    }


extern size_t s_nCreatedNodes;

void CollectNextFeatureEdges(MTGGraph*graphP, MTGNodeId current, int tagValueI, MTGMask visitedMask, bvector<int32_t>& edges)
    {
    bool hasEdges = true;
    do
        {
        int vIndex = -1;
        graphP->TryGetLabel(current, 0, vIndex);
        graphP->SetMaskAt(current, visitedMask);
        edges.push_back(vIndex - 1);
        MTGNodeId next = graphP->FSucc(current);
        bool foundNext = false;
        MTGARRAY_VERTEX_LOOP(edge, graphP, next)
            {
            int tagValue = -1;
            graphP->TryGetLabel(next, 2, tagValue);
            if (tagValueI == tagValue)
                {
                current = edge;
                foundNext = true;
                break;
                }
            }
        MTGARRAY_END_VERTEX_LOOP(edge, graphP, next)
            if (graphP->GetMaskAt(current, visitedMask)) hasEdges = false;
        if (!foundNext) hasEdges = false;
        }
    while (hasEdges);
    }

void SortDefinitionsBasedOnNodeBounds(bvector<bvector<int32_t>>& featureDefs, const DRange3d& extent, const DPoint3d* pts, const size_t nPts)
    {
    bvector<bvector<bpair<int,int>>> featuresBeginOrEndOnEdge(6);
    DPoint3d origins[6];
    DVec3d normals[6];
    extent.Get6Planes(origins, normals);
    DPlane3d planes[6];
    size_t nOfFeaturesToLink = 0;
    for (size_t i = 0; i < 6; ++i)
        planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);

    for (auto& def : featureDefs)
        {
        if (!IsClosedFeature(def[0])) continue;
        bool isOnEdge = false;
        for (size_t i = 0; i < 6; ++i)
            {
            if (fabs(planes[i].Evaluate(pts[def[1]])) < 1e-4)
                {
                featuresBeginOrEndOnEdge[i].push_back(make_bpair(0, (int)(&def - &featureDefs.front())));
                isOnEdge = true;
                }
            else if (fabs(planes[i].Evaluate(pts[def[def.size()-2]])) < 1e-4)
                {
                featuresBeginOrEndOnEdge[i].push_back(make_bpair(1, (int)(&def - &featureDefs.front())));
                isOnEdge = true;
                }
            }
        if (isOnEdge) nOfFeaturesToLink++;
        }
    bvector<bpair<int,int>> idxOrder;
    for (size_t i = 0; i < 6; ++i)
        for (auto& idx : featuresBeginOrEndOnEdge[i])idxOrder.push_back(idx);

    int currentId = 0;
    bvector<bvector<int32_t>> mergedFeatures;
    bvector<int32_t> currentFeature;
    std::set<int> usedFeatures;
    std::set<int> checkIds;
    while (usedFeatures.size() < nOfFeaturesToLink && checkIds.size() < idxOrder.size() && currentId < idxOrder.size())
        {
        if (currentFeature.size() > 1)
            {
            currentFeature.push_back(currentFeature[1]);
            mergedFeatures.push_back(currentFeature);
            currentFeature.clear();
            }
        if (checkIds.count(currentId) != 0 || usedFeatures.count(idxOrder[currentId].second) != 0)
            {
            currentId++;
            continue;
            }
        checkIds.insert(currentId);
        int feaId = idxOrder[currentId].second;
        if (currentFeature.empty()) currentFeature.push_back(featureDefs[feaId][0]);
        if (idxOrder[currentId].first == 1) currentFeature.insert(currentFeature.end(), featureDefs[feaId].begin() + 1, featureDefs[feaId].end()-1);
        else currentFeature.insert(currentFeature.end(), featureDefs[feaId].rbegin()+1, featureDefs[feaId].rend() - 1);
        usedFeatures.insert(feaId);
        ++currentId;
        int iterations = 0;
        while (currentId < idxOrder.size())
            {
            int feaId = idxOrder[currentId].second;
            if (usedFeatures.count(feaId) != 0) break;
            usedFeatures.insert(feaId);
            checkIds.insert(currentId);
            if (idxOrder[currentId].first == 1) currentFeature.insert(currentFeature.end(), featureDefs[feaId].begin() + 1, featureDefs[feaId].end()-1);
            else currentFeature.insert(currentFeature.end(), featureDefs[feaId].rbegin()+1, featureDefs[feaId].rend() - 1);

            if (currentFeature.back() == currentFeature[1]) break;
            if (iterations % 2 != 0) ++currentId;
            else
                {
                size_t id = 0;
                for (id = currentId + 1; id < idxOrder.size(); ++id)
                    if (idxOrder[id].second == feaId)
                        {
                        break;
                        }
                currentId = (int)id + 1;
                }
            }
        currentId = 0;

        }
    if (currentFeature.size() > 1)
        {
        currentFeature.push_back(currentFeature[1]);
        mergedFeatures.push_back(currentFeature);
        currentFeature.clear();
        }
    for (auto it = usedFeatures.begin(); it != usedFeatures.end(); ++it)
        featureDefs[*it].clear();
    for (auto& feature : mergedFeatures) featureDefs.push_back(feature);
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 4/16
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::CollectFeatureDefinitionsFromGraph(MTGGraph* graph, size_t maxPtID)
    {
    // MTGMask visitedMask = graph->GrabMask();
    bvector<bvector<int32_t>> features;
    bvector<bvector<int32_t>> featureDefs;
    /*    bvector<int32_t> currentFeature;
        MTGARRAY_SET_LOOP(edgeID, graph)
        {
        if (!graph->GetMaskAt(edgeID, visitedMask))
        {
        graph->SetMaskAt(edgeID, visitedMask);
        int tagValue = -1;
        graph->TryGetLabel(edgeID, 2, tagValue);
        if (IsClosedFeature(tagValue))
        {
        if (currentFeature.size() > 0)
        {
        features.push_back(currentFeature);
        currentFeature.clear();
        }
        currentFeature.push_back(tagValue);
        MTGNodeId current = edgeID;
        bvector<int32_t> left;
        bvector<int32_t> right;
        CollectNextFeatureEdges(graph, current, tagValue, visitedMask,left);
        CollectNextFeatureEdges(graph, graph->EdgeMate(current), tagValue, visitedMask, right);
        currentFeature.insert(currentFeature.end(), left.rbegin(), left.rend());
        currentFeature.insert(currentFeature.end(), right.begin(), right.end());

        }
        }
        }
        MTGARRAY_END_SET_LOOP(edgeID, graph)
        graph->ClearMask(visitedMask);
        graph->DropMask(visitedMask);
        if (currentFeature.size() > 0)
        features.push_back(currentFeature);*/
    std::vector<int> temp;
    std::map<int, int> componentForPoints;
    ReadFeatureTags(graph, temp, features, componentForPoints);

    std::map<int,bvector<std::set<int32_t>>> ptsMatch;
    for (auto& feature : features)
        {
        int tag = feature[0];
        if (ptsMatch.count(tag) == 0)
            {
            ptsMatch[tag] = bvector<std::set<int32_t>>(maxPtID);
            }
        ptsMatch[tag][feature[1]].insert(feature[2]);
        ptsMatch[tag][feature[2]].insert(feature[1]);
        }
    for (auto& it : ptsMatch)
        {
        int tag = it.first;
        int start = -1;
        for (size_t t = 0; t < ptsMatch[tag].size(); ++t)
            {
            if (ptsMatch[tag][t].size() == 1)
                {
                start = (int)t;
                break;
                }
            }
        while (start != -1)
            {
            bvector<int32_t> list;
            while (start != -1)
                {
                list.push_back(start);
                if (ptsMatch[tag][start].empty()) start = -1;
                else
                    {
                    int next = *(ptsMatch[tag][start].begin());
                    ptsMatch[tag][start].erase(next);
                    ptsMatch[tag][next].erase(start);
                    start = next;
                    }
                }
            if (!list.empty() && list.size() > 1)
                {
                if (IsClosedFeature(tag)) list.push_back(list.front());
                list.insert(list.begin(), tag);
                featureDefs.push_back(list);
               /* std::ofstream f;
                f.open((Utf8String("e:\\output\\scmesh\\2016-05-05\\feature_") + Utf8String(std::to_string(GetBlockID().m_integerID).c_str()) + Utf8String(std::to_string(featureDefs.size()).c_str())).c_str(), std::ios_base::trunc);
                for (auto& i : list)
                    {
                    f << i;
                    if ((&i - &list.front()) > 0) f << " " << operator[](i).x << " " << operator[](i).y << " " << operator[](i).z;
                    f << std::endl;
                    }
                f.close();*/
                }
            for (size_t t = 0; t < ptsMatch[tag].size(); ++t)
                {
                if (ptsMatch[tag][t].size() == 1)
                    {
                    start = (int)t;
                    break;
                    }
                }
            }
        }

    for (auto& definition : featureDefs)
        {
        bvector<int> feature1 = definition;
        for (auto it = featureDefs.begin(); feature1.size() > 0 && it != featureDefs.end(); ++it)
            {
            auto& nextDefinition = *it;
            if (definition != nextDefinition && nextDefinition.size() > 1 && componentForPoints.count(feature1.back()) != 0 && componentForPoints.count(nextDefinition[1]) != 0)
                {
                if (componentForPoints[feature1.back()] = componentForPoints[nextDefinition[1]])
                    {
                    feature1.insert(feature1.end(), nextDefinition.begin() + 1, nextDefinition.end());
                    nextDefinition.clear();
                    it = featureDefs.begin();
                    }
                }
            }
        definition = feature1;
        }

    //SortDefinitionsBasedOnNodeBounds(featureDefs, m_nodeHeader.m_nodeExtent, &this->operator[](0), this->size());
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    bvector<bvector<int32_t>> newDefs;
    if (linearFeaturesPtr->size() > 0) GetFeatureDefinitions(newDefs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
    size_t count = 0;
    for (auto& definition : featureDefs)
        {
        if (definition.size() < 2) continue;
        newDefs.push_back(definition);
        count += 1 + definition.size();     
        }
    linearFeaturesPtr->reserve(count);
    for (size_t i = 0; i < count; ++i) linearFeaturesPtr->push_back(0);
    if (linearFeaturesPtr->size() > 0)  SaveFeatureDefinitions(const_cast<int32_t*>(&*linearFeaturesPtr->begin()), count, newDefs);
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::UpdateFromGraph(MTGGraph * graph, bvector<DPoint3d>& pointList)
    {
    std::vector<int> faceIndices;
    MTGMask visitedMask = graph->GrabMask();
    bvector<DPoint3d> retainedPts;
    bmap<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> ptMap(DPoint3dZYXTolerancedSortComparison(10e-4, 0));
    bvector<int> indices(pointList.size(), -1);

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr = GetPointsPtr();

    MTGARRAY_SET_LOOP(edgeID, graph)
        {
        if (!graph->GetMaskAt(edgeID, visitedMask))
            {
            if ( FastCountNodesAroundFace(graph, edgeID) != 3)
                {
                int vIndex = -1;
                graph->TryGetLabel(edgeID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)pointList.size());
                if (indices[vIndex - 1] == -1)
                    {
                    if (ptMap.count(pointList[vIndex - 1]) == 0)
                        {
                        retainedPts.push_back(pointList[vIndex - 1]);
                        indices[vIndex - 1] = (int)retainedPts.size();
                        ptMap[pointList[vIndex - 1]] = (int)retainedPts.size();
                        }
                    else
                        {
                        indices[vIndex - 1] = ptMap[pointList[vIndex - 1]];
                        }
                    }
                int idx = indices[vIndex - 1];
                graph->TrySetLabel(edgeID, 0, idx);
                graph->SetMaskAt(edgeID, MTG_EXTERIOR_MASK);
                graph->SetMaskAt(graph->EdgeMate(edgeID), MTG_BOUNDARY_MASK);
                continue;
                }
            MTGARRAY_FACE_LOOP(faceID, graph, edgeID)
                {
                int vIndex = -1;
                graph->TryGetLabel(faceID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)pointList.size());
                if (indices[vIndex - 1] == -1)
                    {
                    if (ptMap.count(pointList[vIndex - 1]) == 0)
                        {
                        retainedPts.push_back(pointList[vIndex - 1]);
                        indices[vIndex - 1] = (int)retainedPts.size();
                        ptMap[pointList[vIndex - 1]] = (int)retainedPts.size();
                        }
                    else
                        {
                        indices[vIndex - 1] = ptMap[pointList[vIndex - 1]];
                        }
                    }
                int idx = indices[vIndex - 1];
                    faceIndices.push_back(idx);
                    graph->SetMaskAt(faceID, visitedMask);
                    if (graph->GetMaskAt(faceID, MTG_EXTERIOR_MASK)) graph->ClearMaskAt(faceID, MTG_EXTERIOR_MASK);
                    if (graph->GetMaskAt(faceID, MTG_BOUNDARY_MASK)) graph->ClearMaskAt(faceID, MTG_BOUNDARY_MASK);
                    graph->TrySetLabel(faceID, 0, idx);
                }
            MTGARRAY_END_FACE_LOOP(faceID, graph, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, graph)
        graph->ClearMask(visitedMask);
    graph->DropMask(visitedMask);
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(this->GetGraphPtr());
    ISMMTGGraphDataStorePtr graphStore(this->GetGraphStore());

    RefCountedPtr<SMStoredMemoryPoolGenericBlobItem<MTGGraph>> storedMemoryPoolItem(
#ifndef VANCOUVER_API
        new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(this->GetBlockID().m_integerID, graphStore, SMStoreDataType::Graph, (uint64_t)m_SMIndex)
#else
    SMStoredMemoryPoolGenericBlobItem<MTGGraph>::CreateItem(this->GetBlockID().m_integerID, graphStore, SMStoreDataType::Graph, (uint64_t)m_SMIndex)
#endif
        );
    SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());

    MTGGraph* graphP = new MTGGraph(*graph);
    storedMemoryPoolItem->SetData(graphP);
    storedMemoryPoolItem->SetDirty();
    SMMemoryPool::GetInstance()->ReplaceItem(memPoolItemPtr, m_graphPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex);


    //this->SetGraphDirty();    
    assert(faceIndices.size() % 3 == 0);
    if (faceIndices.size() > 0 && retainedPts.size() > 0)
        {
        pointsPtr->clear();
        for (size_t i = 0; i < retainedPts.size(); ++i)
            pointsPtr->push_back(PointOp<POINT>::Create(retainedPts[i].x, retainedPts[i].y, retainedPts[i].z));

        this->ReplacePtsIndices((int32_t*)&faceIndices[0], faceIndices.size());
        }
    //CollectFeatureDefinitionsFromGraph(graph, retainedPts.size());
#if SM_OUTPUT_MESHES_GRAPH
    WString nameBefore = L"e:\\output\\scmesh\\2015-12-11\\afterfilter_";
    nameBefore.append(std::to_wstring(this->m_nodeHeader.m_level).c_str());
    nameBefore.append(L"_");
    nameBefore.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent)).c_str());
    nameBefore.append(L"_");
    nameBefore.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent)).c_str());
    nameBefore.append(L".m");
    size_t nVertices = retainedPts.size();
    size_t nIndices = this->m_nodeHeader.m_nbFaceIndexes;
    FILE* meshBeforeStitch = _wfopen(nameBefore.c_str(), L"wb");
    fwrite(&nVertices, sizeof(size_t), 1, meshBeforeStitch);
    fwrite(&retainedPts[0], sizeof(DPoint3d), nVertices, meshBeforeStitch);
    fwrite(&nIndices, sizeof(size_t), 1, meshBeforeStitch);

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(pointsPtr->GetPtsIndicePtr());

    fwrite((int32_t*)&(*ptIndices)[0], sizeof(int32_t), nIndices, meshBeforeStitch);
    fclose(meshBeforeStitch);
#endif
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::SplitNodeBasedOnImageRes()
    {
    HPRECONDITION(IsLeaf());
    POINT splitPosition = GetDefaultSplitPosition();
    if (m_nodeHeader.m_numberOfSubNodesOnSplit == 4)
        {

        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 4;

        }
    else
        {
        HPRECONDITION(ExtentOp<EXTENT>::GetThickness(GetNodeExtent()) > 0.0);

        if (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))
            {
            // Values would be virtually equal ... we will not split
            HDEBUGCODE(m_unspliteable = true;)
                return;
            }


        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 8;
        }    

    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;
    for (size_t i = 0; i < m_nodeHeader.m_numberOfSubNodesOnSplit;++i)
        {        
        this->AdviseSubNodeIDChanged(m_apSubNodes[i]);
        }

    SetupNeighborNodesAfterSplit();


    for (auto& node : m_apSubNodes) 
        this->AdviseSubNodeIDChanged(node);


   SplitMeshForChildNodes();
    SetDirty(true);
    }


template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ImportTreeFrom(IScalableMeshNodePtr& sourceNode)
    {
    m_nodeHeader.m_contentExtent = sourceNode->GetContentExtent();
    m_nodeHeader.m_nodeExtent = sourceNode->GetNodeExtent();
    
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
    auto meshP = sourceNode->GetMesh(flags);
    m_nodeHeader.m_isTextured = sourceNode->IsTextured();

    bvector<DPoint3d> vertices(meshP->GetPolyfaceQuery()->GetPointCount());
    if (!vertices.empty()) memcpy(vertices.data(), meshP->GetPolyfaceQuery()->GetPointCP(), vertices.size()* sizeof(DPoint3d));
    bvector<int32_t> ptsIndices(meshP->GetPolyfaceQuery()->GetPointIndexCount());
    if (!ptsIndices.empty()) memcpy(ptsIndices.data(), meshP->GetPolyfaceQuery()->GetPointIndexCP(), ptsIndices.size()* sizeof(int32_t));

    bvector<DPoint2d> uv(meshP->GetPolyfaceQuery()->GetParamCount());
    if (!uv.empty()) memcpy(uv.data(), meshP->GetPolyfaceQuery()->GetParamCP(), uv.size()* sizeof(DPoint2d));
    bvector<int32_t> uvIndices(meshP->GetPolyfaceQuery()->GetPointIndexCount());
    if (!uvIndices.empty()) memcpy(uvIndices.data(), meshP->GetPolyfaceQuery()->GetParamIndexCP(), uvIndices.size()* sizeof(int32_t));

    size_t nIndicesCount = 0;
    vector<POINT> nodePts(vertices.size());

    for (size_t pointInd = 0; pointInd < vertices.size(); pointInd++)
        {
        nodePts[pointInd].x = vertices[pointInd].x;
        nodePts[pointInd].y = vertices[pointInd].y;
        nodePts[pointInd].z = vertices[pointInd].z;
        }

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
    pointsPtr->push_back(&nodePts[0], nodePts.size());
    if(!uv.empty()) PushUV(&uv[0], uv.size());

    vector<int32_t> indicesLine;

   
    nIndicesCount += ptsIndices.size();
    PushPtsIndices(&ptsIndices[0], ptsIndices.size());
    indicesLine.insert(indicesLine.end(), ptsIndices.begin(), ptsIndices.end());

    if(!uvIndices.empty()) PushUVsIndices(0, &uvIndices[0], uvIndices.size());
    m_nodeHeader.m_nbFaceIndexes = indicesLine.size();
    m_nodeHeader.m_nbUvIndexes = uv.size();
    IncreaseTotalCount(GetNbPoints());

    auto sourceChildren = sourceNode->GetChildrenNodes();
    m_apSubNodes.resize(sourceChildren.size());
    for (size_t i = 0; i < m_apSubNodes.size(); ++i)
        {
        m_apSubNodes[i] = CloneChild(sourceChildren[i]->GetNodeExtent());
        dynamic_cast<SMMeshIndexNode<POINT,EXTENT>*>(m_apSubNodes[i].GetPtr())->ImportTreeFrom(sourceChildren[i]);
        }

    SetDirty(true);

    }

    bool TopologyIsDifferent(const int32_t* indicesA, const size_t nIndicesA, const int32_t* indicesB, const size_t nIndicesB);

    //=======================================================================================
    // @description Converts node mesh to a bcdtm object and use that triangle list as the new mesh.
    //              Used for compatibility with civil analysis functions.
    // @bsimethod                                                   Elenie.Godzaridis 05/16
    //=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::UpdateNodeFromBcDTM()
    {
#if SM_TRACE_MESH_STATS
    LOG_SET_PATH("E:\\output\\scmesh\\2016-06-07\\")
    LOG_SET_PATH_W("E:\\output\\scmesh\\2016-06-07\\")
#endif

    auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
        new ScalableMeshNode<POINT>(nodePtr)
#else
        ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
        );
    BcDTMPtr dtm = nodeP->GetBcDTM().get();
    if (dtm == nullptr || dtm->GetTrianglesCount() == 0) return;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtm);
    en->SetMaxTriangles(2000000);
    bvector<DPoint3d> newVertices;
    bvector<int32_t> newIndices;
    std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    for (PolyfaceQueryP pf : *en)
        {
        PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
        vec->CopyFrom(*pf);
        for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*vec); addedFacets->AdvanceToNextFace();)
            {
            DPoint3d face[3];
            int32_t idx[3] = { -1, -1, -1 };
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = addedFacets->GetPointCP()[i];
                idx[i] = mapOfPoints.count(face[i]) != 0 ? mapOfPoints[face[i]] : -1;
                }
            for (size_t i = 0; i < 3; ++i)
                {
                if (idx[i] == -1)
                    {
                    newVertices.push_back(face[i]);
                    idx[i] = (int)newVertices.size();
                    mapOfPoints[face[i]] = idx[i] - 1;
                    }
                else idx[i]++;
                }
            newIndices.push_back(idx[0]);
            newIndices.push_back(idx[1]);
            newIndices.push_back(idx[2]);
            }
        }

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> existingPts(GetPointsPtr());

#if SM_TRACE_FEATURE_DEFINITIONS
    bool hasPtsToTrack = false;
/*    DPoint3d pts[3] = { DPoint3d::From(427283.84, 4501839.24, 0),
    DPoint3d::From(428610.55, 4504064.41,0),
        DPoint3d::From(435130.82, 450594.72,0)};

    for (size_t i = 0; i < 3; ++i)
        if (m_nodeHeader.m_nodeExtent.IsContained(pts[i],2)) hasPtsToTrack = true;*/

    if (existingFaces->size() > 0 && existingPts->size() > 0 && hasPtsToTrack)
        {
            {
            WString nameStitched = LOG_PATH_STR_W + L"postdtmmesh_1st";
            LOGSTRING_NODE_INFO_W(this, nameStitched)
                nameStitched.append(L".m");
            LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameStitched, existingPts->size(), existingFaces->size(), &(*existingPts)[0], &(*existingFaces)[0])
            }

        if (newIndices.size() > 0 && newVertices.size() > 0)
            {
            WString nameStitched = LOG_PATH_STR_W + L"postdtmmesh_2nd";
            LOGSTRING_NODE_INFO_W(this, nameStitched)
                nameStitched.append(L".m");
            LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameStitched, newVertices.size(), newIndices.size(), &newVertices[0], &newIndices[0])
            }
        }
#endif

    if (newIndices.size() > 0 && newVertices.size() > 0)
        {
        existingFaces->clear();
        existingFaces->push_back(&newIndices[0], (int)newIndices.size());
        existingPts->clear();
        existingPts->push_back(&newVertices[0], (int) newVertices.size());
        }
    //m_tileBcDTM = nullptr;
    GetMemoryPool()->RemoveItem(m_dtmPoolItemId, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex);
    m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    }

#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::GetMeshParts(bvector<IScalableMeshMeshPtr>& parts, bvector<Utf8String>& metadata, bvector<bvector<uint8_t>>& texData)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> existingPts(GetPointsPtr());
    RefCountedPtr<SMMemoryPoolBlobItem<uint8_t>> existingTex(GetTexturePtr());
    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> existingUvs(GetUVCoordsPtr());
    if(existingUvs.IsValid() && existingPts->size() != existingUvs->size())
        {
        existingTex->SetData(0,0);
        existingUvs->clear();
        }
    bvector<uint8_t> texDataUnified;
    if(existingTex.IsValid() && existingTex->GetSize() > 0)
        {
        texDataUnified.insert(texDataUnified.end(), existingTex->GetData(), existingTex->GetData()+existingTex->GetSize());
        }
    GetMetadata();
    GetMeshParts();
    if (m_meshParts.size() > 0)
        {
        for (size_t i = 0; i < m_meshParts.size(); i += 2)
            {
            bvector<int> indices((m_meshParts[i + 1] - m_meshParts[i]));
            if(indices.size() == 0) continue;
            memcpy(&indices[0], &(*existingFaces)[m_meshParts[i]], (m_meshParts[i + 1] - m_meshParts[i]) * sizeof(int32_t));
            int idxMin = INT_MAX;
            int idxMax = INT_MIN;
            for (auto& idx : indices)
                {
                if (idx < idxMin) idxMin = idx;
                if (idx > idxMax) idxMax = idx;
                }
            for (auto& idx : indices) idx -= (idxMin - 1);

            bvector<uint8_t> texPart;
            bvector<DVec2d> newUvs;
            if(!existingUvs.IsValid() || existingUvs->GetSize() == 0)
                {
                texData.push_back(texPart);
                }
            else
                {
                newUvs.resize(idxMax - (idxMin - 1));
                DVec2d* uvPart = (DVec2d*)const_cast<DPoint2d*>(&(*existingUvs)[idxMin-1]);
                memcpy(&newUvs[0], uvPart, newUvs.size()*sizeof(DPoint2d));
               /* ComputeTexPart(texPart,&newUvs[0],idxMax - (idxMin - 1), texDataUnified );
                texData.push_back(texPart);*/
                texData.push_back(texPart);
                }
            IScalableMeshMeshPtr meshPtr;
            if(!existingUvs.IsValid() || existingUvs->GetSize() == 0)
                meshPtr = IScalableMeshMesh::Create(idxMax - (idxMin - 1), const_cast<DPoint3d*>(&(*existingPts)[idxMin-1]), indices.size(), &indices[0], 0, 0, 0, 0, 0, 0);
            else
                meshPtr = IScalableMeshMesh::Create(idxMax - (idxMin - 1), const_cast<DPoint3d*>(&(*existingPts)[idxMin-1]), indices.size(), &indices[0], 0, 0, 0,idxMax - (idxMin - 1), &newUvs[0], &indices[0]);
            parts.push_back(meshPtr);
            metadata.push_back(m_meshMetadata[i/2]);
            }
        }
    else
        {
        metadata.push_back(Utf8String());
        IScalableMeshMeshPtr meshPtr = IScalableMeshMesh::Create(existingPts->size(), const_cast<DPoint3d*>(&(*existingPts)[0]), existingFaces->size(), const_cast<int32_t*>(&(*existingFaces)[0]), 0, 0, 0, 0, 0, 0);
        parts.push_back(meshPtr);
        }
    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::AppendMeshParts(bvector<bvector<DPoint3d>>& points, bvector<bvector<int32_t>>& indices, bvector<Utf8String>& metadata,bvector<bvector<DPoint2d>>& uvs, bvector<bvector<uint8_t>>& tex, bool shouldCreateGraph)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> indicesPtr(GetPtsIndicePtr());
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
    pointsPtr->clear();
    indicesPtr->clear();

    if (pointsPtr->size() > 0) 
        std::cout << " TOO MANY POINTS BEFORE ADD :"<< pointsPtr->size()  << std::endl;
    size_t sum =0;
    for (size_t i = 0; i < points.size(); ++i)
        {
        DRange3d extentClipped = DRange3d::NullRange();
        bvector<DPoint3d> pointsClipped;
        bvector<int32_t> indicesClipped;
        DRange3d extent = DRange3d::From(&points[i][0], (int)points[i].size());
        bvector<DPoint2d> outUvs;
        if(uvs[i].size() > 0)
            {
            outUvs.insert(outUvs.end(), uvs[i].begin(), uvs[i].end());
            }
        Json::Value val;
        Json::Reader reader;
        reader.parse(metadata[i], val);
        bvector<int64_t> texId;
        for (const Json::Value& id : val["texId"])
            {
            texId.push_back(id.asInt64());
            }
        ClipMeshDefinition(m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, indicesClipped, outUvs, &points[i][0], points[i].size(), &indices[i][0], indices[i].size(), extent);
        if (!m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) continue;
        if(sum+pointsClipped.size() > 65000)
            {
            pointsClipped.resize(0);
            indicesClipped.resize(0);
            outUvs.resize(0);
            }

        if(outUvs.size() > 0) m_nodeHeader.m_isTextured = true;
        sum += pointsClipped.size();
        m_meshMetadata.push_back(metadata[i]);
        size_t offset = pointsPtr->size();
        m_meshParts.push_back((int)indicesPtr->size());
        pointsPtr->push_back(&pointsClipped[0], pointsClipped.size());
        for (auto& idx : indicesClipped)
            {
            indicesPtr->push_back(idx + (int)offset);
            }        
        m_meshParts.push_back((int)indicesPtr->size());

        if(outUvs.size() > 0)
            {
            
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>>  uvPtr = GetUVCoordsPtr();
            if (offset != 0 && uvPtr->size() < offset)
                {
                bvector<DPoint2d> dummyUvs(offset - uvPtr->size(), DPoint2d::From(0.0, 0.0));
                PushUV(&dummyUvs[0], dummyUvs.size());
                }
            PushUV(&outUvs[0], outUvs.size());

            }
        else
            {
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>>  uvPtr = GetUVCoordsPtr();
            if (uvPtr.IsValid())
                {
                bvector<DPoint2d> dummyUvs(pointsClipped.size(), DPoint2d::From(0.0, 0.0));
                PushUV(&dummyUvs[0], dummyUvs.size());
                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(GetUVsIndicesPtr());
                uvIndicePtr->clear();
                uvIndicePtr->push_back(&(*indicesPtr)[0], indicesPtr->size());
                }
            }
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicePtr(GetUVsIndicesPtr());
        if (uvIndicePtr.IsValid())
            {
            uvIndicePtr->clear();
            uvIndicePtr->push_back(&(*indicesPtr)[0], indicesPtr->size());
            }
        }
    if (sum > 65000) 
        std::cout << " TOO MANY POINTS :"<< sum << std::endl;
    if (pointsPtr->size() > 65000)
        std::cout << " TOO MANY POINTS AFTER ADD :" << pointsPtr->size() << std::endl;

    StoreMetadata();
    StoreMeshParts();
    }


template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::GetMeshParts()
    {
    std::lock_guard<std::mutex> lock(m_headerMutex);
    if(!m_meshParts.empty()) return;
    ISMInt32DataStorePtr nodeDataStore;
    bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::MeshParts);
    assert(result == true);
    HPMBlockID blockId = HPMBlockID(m_nodeId);
    size_t size = nodeDataStore->GetBlockDataCount(blockId);
    m_meshParts.resize(size);
    if(size > 0)
        {
        nodeDataStore->LoadBlock(&m_meshParts[0], size, blockId);
        }
    }
    
    template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreMeshParts()
    {
    if(m_meshParts.empty()) return;
    ISMInt32DataStorePtr nodeDataStore;
    HPMBlockID blockId = HPMBlockID(m_nodeId);
    m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::MeshParts);
    if(m_meshParts.size() > 0)
        {
        nodeDataStore->StoreBlock(&m_meshParts[0], m_meshParts.size(), blockId);
        }
    }


template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::GetMetadata()
    {
    std::lock_guard<std::mutex> lock(m_headerMutex);
    if(!m_meshParts.empty()) return;
    ISMTextureDataStorePtr nodeDataStore;
    bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::Metadata);
    assert(result == true);
    HPMBlockID blockId = HPMBlockID(m_nodeId);
    size_t size = nodeDataStore->GetBlockDataCount(blockId);
    bvector<unsigned char> str;
    str.resize(size);
    if(size > 0)
        {
        nodeDataStore->LoadBlock(&str[0], size, blockId);
        }
    Utf8String s;
    for(auto& c: str)
        {
        if(c == '\0') 
            {
            m_meshMetadata.push_back(s);
            s.clear();
            }
        else
            {
            Utf8String s2(1,c);
            s.append(s2.c_str());
            }
        }
    if(!s.empty()) m_meshMetadata.push_back(s);
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreMetadata()
    {
    if(m_meshParts.empty()) return;
    ISMTextureDataStorePtr nodeDataStore;
    m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::Metadata);

    HPMBlockID blockId = HPMBlockID(m_nodeId);
    bvector<unsigned char> str;
    for(auto& stri: m_meshMetadata)
        {
        str.insert(str.end(), stri.begin(), stri.end());
        str.push_back('\0');
        }
    if(str.size() > 0)nodeDataStore->StoreBlock(&str[0], str.size(), blockId);
    }

    
#endif

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> SMMeshIndexNode<POINT, EXTENT>::GetDiffSetPtr() const
    {       
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> poolMemItemPtr;

   // if (m_SMIndex->IsTerrain() == false) 
   //     return poolMemItemPtr;

    if (!SMMemoryPool::GetInstance()->GetItem<DifferenceSet>(poolMemItemPtr, m_diffSetsItemId, GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)m_SMIndex))
        {   
        ISDiffSetDataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
        assert(result == true);
        
        RefCountedPtr<SMStoredMemoryPoolGenericVectorItem<DifferenceSet>> storedMemoryPoolItem(
       #ifndef VANCOUVER_API
        new SMStoredMemoryPoolGenericVectorItem<DifferenceSet>(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::DiffSet, (uint64_t)m_SMIndex)
#else
        SMStoredMemoryPoolGenericVectorItem<DifferenceSet>::CreateItem(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::DiffSet, (uint64_t)m_SMIndex)
#endif  
  );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
        m_diffSetsItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(m_diffSetsItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemItemPtr = storedMemoryPoolItem.get();
        const_cast<atomic<size_t>&>(m_nbClips) = poolMemItemPtr->size();
        }
    return poolMemItemPtr;
    }

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> SMMeshIndexNode<POINT, EXTENT>::GetGraphPtr(bool loadGraph = true)
    {
    std::lock_guard<std::mutex> lock(m_graphMutex); //don't want to add item twice
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> poolMemItemPtr;

    if (!SMMemoryPool::GetInstance()->GetItem<MTGGraph>(poolMemItemPtr, m_graphPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)m_SMIndex) && loadGraph)
        {                  
        ISMMTGGraphDataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
        assert(result == true);  

        RefCountedPtr<SMStoredMemoryPoolGenericBlobItem<MTGGraph>> storedMemoryPoolItem(
       #ifndef VANCOUVER_API
        new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::Graph, (uint64_t)m_SMIndex)
        #else
        SMStoredMemoryPoolGenericBlobItem<MTGGraph>::CreateItem(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::Graph, (uint64_t)m_SMIndex)
        #endif
        );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
        m_graphPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(m_graphPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemItemPtr = storedMemoryPoolItem.get();
        }

    return poolMemItemPtr;
    }


template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> SMMeshIndexNode<POINT, EXTENT>::GetLinearFeaturesPtr()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;

    if (!GetMemoryPool()->GetItem<int32_t>(poolMemVectorItemPtr, m_featurePoolItemId, GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t)m_SMIndex))
        {
        ISMInt32DataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::LinearFeature);
        assert(result == true);         

        RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(
    #ifndef VANCOUVER_API
        new SMStoredMemoryPoolVectorItem<int32_t>(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::LinearFeature, (uint64_t)m_SMIndex)
        #else
        SMStoredMemoryPoolVectorItem<int32_t>::CreateItem(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::LinearFeature, (uint64_t)m_SMIndex)
        #endif
        );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
        m_featurePoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        assert(m_featurePoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemVectorItemPtr = storedMemoryPoolVector.get();
        }

    return poolMemVectorItemPtr;
    }

template<class POINT, class EXTENT>  RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> SMMeshIndexNode<POINT, EXTENT>::GetPtsIndicePtr()
    {    
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;        

    if (!GetMemoryPool()->GetItem<int32_t>(poolMemVectorItemPtr, m_triIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriPtIndices, (uint64_t)m_SMIndex))
        {                          
        ISMInt32DataStorePtr faceIndDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(faceIndDataStore, &m_nodeHeader, SMStoreDataType::TriPtIndices);
        assert(result == true);      

        RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(
    #ifndef VANCOUVER_API
        new SMStoredMemoryPoolVectorItem<int32_t>(GetBlockID().m_integerID, faceIndDataStore, SMStoreDataType::TriPtIndices, (uint64_t)m_SMIndex)
#else
 SMStoredMemoryPoolVectorItem<int32_t>::CreateItem(GetBlockID().m_integerID, faceIndDataStore, SMStoreDataType::TriPtIndices, (uint64_t)m_SMIndex)
#endif 
 );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
        m_triIndicesPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        assert(m_triIndicesPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemVectorItemPtr = storedMemoryPoolVector.get();            
        }        

    return poolMemVectorItemPtr;

#if 0 
    //Sample code of using store capabilities to load the point and triangle pt indices atomically
    /*
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;        
    SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr;
                    
    if (!SMMemoryPool::GetInstance()->GetItem(poolMemMultiItemsPtr, m_pointsPoolItemId, GetBlockID().m_integerID, SMStoreDataType::PointAndTriPtIndices, (uint64_t)m_SMIndex))
        {                         
        ISMPointTriPtIndDataStorePtr pointTriIndDataStore;        
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(pointTriIndDataStore, &m_nodeHeader);
        assert(result == true);        
    
        RefCountedPtr<SMStoredMemoryPoolMultiItems<PointAndTriPtIndicesBase>> storedMemoryMultiItemPool;
    
        storedMemoryMultiItemPool = new SMStoredMemoryPoolMultiItems<PointAndTriPtIndicesBase>(pointTriIndDataStore, GetBlockID().m_integerID, SMStoreDataType::PointAndTriPtIndices, (uint64_t)m_SMIndex);

        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryMultiItemPool.get());
        m_pointsPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(m_pointsPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemMultiItemsPtr = (SMMemoryPoolMultiItemsBase*)storedMemoryMultiItemPool.get();            
        }
            
    bool result = poolMemMultiItemsPtr->GetItem<int32_t>(poolMemVectorItemPtr, SMStoreDataType::TriPtIndices);
    assert(result == true);
    */
#endif
    }

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> SMMeshIndexNode<POINT, EXTENT>::GetUVsIndicesPtr()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;

    if (!IsTextured())
        return poolMemVectorItemPtr;
            
    if (!GetMemoryPool()->GetItem<int32_t>(poolMemVectorItemPtr, m_triUvIndicesPoolItemId, GetBlockID().m_integerID, SMStoreDataType::TriUvIndices, (uint64_t)m_SMIndex))
        {  
        ISMInt32DataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader, SMStoreDataType::TriUvIndices);
        assert(result == true);        
               
        RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(
       #ifndef VANCOUVER_API
        new SMStoredMemoryPoolVectorItem<int32_t>(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::TriUvIndices, (uint64_t)m_SMIndex)
        #else
        SMStoredMemoryPoolVectorItem<int32_t>::CreateItem(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::TriUvIndices, (uint64_t)m_SMIndex)
        #endif
        );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
        m_triUvIndicesPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(m_triUvIndicesPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemVectorItemPtr = storedMemoryPoolVector.get();            
        }

    return poolMemVectorItemPtr;          
    }  

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolBlobItem<Byte>> SMMeshIndexNode<POINT, EXTENT>::GetTexturePtr()
    {
    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> poolMemBlobItemPtr;

    if (!IsTextured())
        return poolMemBlobItemPtr;

    int64_t texID = m_nodeHeader.m_textureID.IsValid() && m_nodeHeader.m_textureID != ISMStore::GetNullNodeID() && m_nodeHeader.m_textureID.m_integerID != -1 ? m_nodeHeader.m_textureID.m_integerID : GetBlockID().m_integerID;
              
    //NEEDS_WORK_SM : Need to modify the pool to have a thread safe get or add.
    if (!GetMemoryPool()->GetItem<Byte>(poolMemBlobItemPtr, m_texturePoolItemId, GetBlockID().m_integerID, SMStoreDataType::Texture, (uint64_t)m_SMIndex))
        {      
        ISMTextureDataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
        assert(result == true);  

        RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
 #ifndef VANCOUVER_API
            new SMStoredMemoryPoolBlobItem<Byte>(texID, nodeDataStore, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
 #else
            SMStoredMemoryPoolBlobItem<Byte>::CreateItem(texID, nodeDataStore, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
 #endif
        );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
        m_texturePoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        assert(m_texturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemBlobItemPtr = storedMemoryPoolVector.get();            
        }

    return poolMemBlobItemPtr;
    }

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolBlobItem<Byte>> SMMeshIndexNode<POINT, EXTENT>::GetTexturePtr(uint64_t texID)
    {
    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> poolMemBlobItemPtr;

    if (!IsTextured())
        return poolMemBlobItemPtr;

    SMMemoryPoolItemId texPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTextureData(texID);

    //NEEDS_WORK_SM : Need to modify the pool to have a thread safe get or add.
    if (!GetMemoryPool()->GetItem<Byte>(poolMemBlobItemPtr, texPoolItemId, texID, SMStoreDataType::Texture, (uint64_t)m_SMIndex))
        {
        ISMTextureDataStorePtr nodeDataStore;
        bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
        assert(result == true);

        RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
#ifndef VANCOUVER_API
            new SMStoredMemoryPoolBlobItem<Byte>(texID, nodeDataStore, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
#else
            SMStoredMemoryPoolBlobItem<Byte>::CreateItem(texID, nodeDataStore, SMStoreDataType::Texture, (uint64_t)m_SMIndex)
#endif
            );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
        texPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->SetPoolIdForTextureData(texID, texPoolItemId);
        assert(texPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemBlobItemPtr = storedMemoryPoolVector.get();
        }

    return poolMemBlobItemPtr;
    }

//=======================================================================================
// @description Sets texture data for this node based on a raster. If untextured this adds
//              a new texture.
//              See ScalableMeshSourceCreator::ImportRasterSourcesTo for information
//              on how to create a raster from image files to be used by this function.
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRaster(HIMMosaic* sourceRasterP, Transform unitTransform)
    {
    if (!IsLoaded()) Load();
    DRange2d rasterBox = DRange2d::From(sourceRasterP->GetEffectiveShape()->GetExtent().GetXMin(), sourceRasterP->GetEffectiveShape()->GetExtent().GetYMin(),
                                        sourceRasterP->GetEffectiveShape()->GetExtent().GetXMax(), sourceRasterP->GetEffectiveShape()->GetExtent().GetYMax());
    //get overlap between node and raster extent
    DRange2d contentExtent = DRange2d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));

    Transform t;
    t.InverseOf(unitTransform);
    t.Multiply(contentExtent.low, contentExtent.low);
    t.Multiply(contentExtent.high, contentExtent.high);
    if (!rasterBox.IntersectsWith(contentExtent)) return;
    if (GetPointsPtr()->size() == 0 || m_nodeHeader.m_nbFaceIndexes == 0) return;
    



    int textureWidthInPixels = 1024, textureHeightInPixels = 1024;
    double unitsPerPixelX = (contentExtent.high.x - contentExtent.low.x) / textureWidthInPixels;
    double unitsPerPixelY = (contentExtent.high.y - contentExtent.low.y) / textureHeightInPixels;
    contentExtent.low.x -= 5 * unitsPerPixelX;
    contentExtent.low.y -= 5 * unitsPerPixelY;
    contentExtent.high.x += 5 * unitsPerPixelX;
    contentExtent.high.y += 5 * unitsPerPixelY;
    HPRECONDITION((textureWidthInPixels != 0) && (textureHeightInPixels != 0));

    HFCMatrix<3, 3> transfoMatrix;
    transfoMatrix[0][0] = (contentExtent.high.x - contentExtent.low.x) / textureWidthInPixels;
    transfoMatrix[0][1] = 0;
    transfoMatrix[0][2] = contentExtent.low.x;
    transfoMatrix[1][0] = 0;
    transfoMatrix[1][1] = -(contentExtent.high.y - contentExtent.low.y) / textureHeightInPixels;
    transfoMatrix[1][2] = contentExtent.high.y;
    transfoMatrix[2][0] = 0;
    transfoMatrix[2][1] = 0;
    transfoMatrix[2][2] = 1;

    HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

    if (pSimplifiedModel != 0)
        {
        pTransfoModel = pSimplifiedModel;
        }

    HFCPtr<HRABitmap> pTextureBitmap;

    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());

    HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
    byte* pixelBufferP = new byte[textureWidthInPixels * textureHeightInPixels * 3 + 3 * sizeof(int)];
    memcpy(pixelBufferP, &textureWidthInPixels, sizeof(int));
    memcpy(pixelBufferP + sizeof(int), &textureHeightInPixels, sizeof(int));
    int nOfChannels = 3;
    memcpy(pixelBufferP + 2 * sizeof(int), &nOfChannels, sizeof(int));
#ifdef VANCOUVER_API
    pTextureBitmap = new HRABitmap(textureWidthInPixels,
                                   textureHeightInPixels,
                                   pTransfoModel.GetPtr(),
                                   sourceRasterP->GetCoordSys(),
                                   pPixelType,
                                   8,
                                   HRABitmap::UPPER_LEFT_HORIZONTAL,
                                   pCodec);
#else
    pTextureBitmap = HRABitmap::Create(textureWidthInPixels,
                                   textureHeightInPixels,
                                   pTransfoModel.GetPtr(),
                                   sourceRasterP->GetCoordSys(),
                                   pPixelType,
                                   8);
#endif
    HGF2DExtent minExt, maxExt;
    sourceRasterP->GetPixelSizeRange(minExt, maxExt);
    minExt.ChangeCoordSys(pTextureBitmap->GetCoordSys());
    if (/*m_nodeHeader.m_level <= 6 && */IsLeaf() && (contentExtent.XLength() / minExt.GetWidth() > textureWidthInPixels || contentExtent.YLength() / minExt.GetHeight() > textureHeightInPixels) /*&& GetNbPoints() > 0*/)
        SplitNodeBasedOnImageRes();
    byte* pixelBufferPRGBA = new byte[textureWidthInPixels * textureHeightInPixels * 4];
    pTextureBitmap->GetPacket()->SetBuffer(pixelBufferPRGBA, textureWidthInPixels * textureHeightInPixels * 4);
    pTextureBitmap->GetPacket()->SetBufferOwnership(false);

    HRAClearOptions clearOptions;

   //green color when no texture is available
    uint32_t green;

    ((uint8_t*)&green)[0] = 0;
    ((uint8_t*)&green)[1] = 0x77;
    ((uint8_t*)&green)[2] = 0;

    clearOptions.SetRawDataValue(&green);

    pTextureBitmap->Clear(clearOptions);

    HRACopyFromOptions copyFromOptions;

    //Rasterlib set this option on the last tile of a row or a column to avoid black lines.     
    copyFromOptions.SetAlphaBlend(true);

#ifdef VANCOUVER_API
    copyFromOptions.SetGridShapeMode(true);
    pTextureBitmap->CopyFrom(sourceRasterP, copyFromOptions);
#else
    pTextureBitmap->CopyFrom(*sourceRasterP, copyFromOptions);
#endif
#ifdef ACTIVATE_TEXTURE_DUMP
    WString fileName = L"file://";
    fileName.append(L"e:\\output\\scmesh\\2016-4-11\\texture_before_");
    //fileName.append(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\QuebecCityMini\\texture_before_"); 
    fileName.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName.append(L"_");
    fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName.append(L"_");
    fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName.append(L".bmp");
    HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
    HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
    byte* pixelBuffer = new byte[1024*1024*3];
    size_t t = 0;
    for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
        {
        pixelBuffer[t] = *(pixelBufferPRGBA + 3 * sizeof(int) + i);
        pixelBuffer[t + 1] = *(pixelBufferPRGBA + 3 * sizeof(int) + i + 1);
        pixelBuffer[t + 2] = *(pixelBufferPRGBA + 3 * sizeof(int) + i + 2);
        t += 3;
        }
    HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                              1024,
                                              1024,
                                              pImageDataPixelType,
                                              pixelBuffer);
    delete[] pixelBuffer;
//#ifdef ACTIVATE_TEXTURE_DUMP
    auto codec = new HCDCodecIJG(1024, 1024, 8 * 4);
    codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
    codec->SetQuality(100);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec2 = codec;
    size_t compressedBufferSize = pCodec2->GetSubsetMaxCompressedSize();
    byte* pCompressedPixelBuffer = new byte[compressedBufferSize];
    size_t nCompressed = pCodec2->CompressSubset(pixelBufferP + 3 * sizeof(int), 1024 * 1024 * 4 * sizeof(Byte), pCompressedPixelBuffer, compressedBufferSize * sizeof(Byte));
    byte * pUncompressedPixelBuffer = new byte[1024*1024*4];
    pCodec2->DecompressSubset(pCompressedPixelBuffer, compressedBufferSize* sizeof(Byte), pUncompressedPixelBuffer, 1024 * 1024 * 4 * sizeof(Byte));
    /*WString fileName2;
    fileName2.append(L"e:\\output\\scmesh\\2015-11-19\\texture_compressed_");
    fileName2.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L".bin");
    FILE* binCompressed = _wfopen(fileName2.c_str(), L"wb");
    fwrite(pCompressedPixelBuffer,sizeof(byte),nCompressed, binCompressed);
    fclose(binCompressed);*/
    std::string myS = " BUFFER SIZE "+std::to_string(compressedBufferSize) +" DATA SIZE "+ std::to_string(nCompressed);
    delete[] pCompressedPixelBuffer;
    WString fileName2 = L"file://";
    fileName2.append(L"e:\\output\\scmesh\\2015-11-19\\texture_before_compressed_");
    //fileName2.append(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\QuebecCityMini\\texture_before_compressed_");
    fileName2.append(std::to_wstring(m_nodeHeader.m_level).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L"_");
    fileName2.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
    fileName2.append(L".bmp");
    HFCPtr<HFCURL> fileUrl2(HFCURL::Instanciate(fileName2));
    pixelBuffer = new byte[1024 * 1024 * 3];
    t = 0;
    for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
        {
        pixelBuffer[t] = *(pUncompressedPixelBuffer + i);
        pixelBuffer[t + 1] = *(pUncompressedPixelBuffer + i + 1);
        pixelBuffer[t + 2] = *(pUncompressedPixelBuffer + i + 2);
        t += 3;
        }
    delete[] pUncompressedPixelBuffer;
    HRFBmpCreator::CreateBmpFileFromImageData(fileUrl2,
                                              1024,
                                              1024,
                                              pImageDataPixelType,
                                              pixelBuffer);
    delete[] pixelBuffer;
#endif
    Byte *pPixel = pixelBufferP + 3 * sizeof(int);
	for (size_t i = 0; i < textureWidthInPixels*textureHeightInPixels; ++i)
        {
        *pPixel++ = pixelBufferPRGBA[i * 4];
        *pPixel++ = pixelBufferPRGBA[i * 4 + 1];
        *pPixel++ = pixelBufferPRGBA[i * 4 + 2];
        }
    PushTexture(pixelBufferP, 3 * sizeof(int) + textureWidthInPixels * textureHeightInPixels * 3);     
    
    UpdateNodeFromBcDTM();
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());

    if (existingFaces->size() >= 4)
        {
        //compute uv's        
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

        vector<DPoint3d> points(pointsPtr->size());

        for (size_t i = 0; i < pointsPtr->size(); ++i)
            points[i] = DPoint3d::From(PointOp<POINT>::GetX(pointsPtr->operator[](i)), PointOp<POINT>::GetY(pointsPtr->operator[](i)), PointOp<POINT>::GetZ(pointsPtr->operator[](i)));
        vector<int32_t> indicesOfTexturedRegion;
        vector<DPoint2d> uvsOfTexturedRegion(points.size());
        unitTransform.Multiply(contentExtent.low, contentExtent.low);
        unitTransform.Multiply(contentExtent.high, contentExtent.high);
        for (size_t i = 0; i < existingFaces->size(); i+=3)
            {
            DPoint3d face[3];
            int32_t idx[3] = { (*existingFaces)[i], (*existingFaces)[i + 1], (*existingFaces)[i + 2] };
            DPoint2d uvCoords[3];
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = points[idx[i] - 1];
                uvCoords[i].x = max(0.0,min((face[i].x - contentExtent.low.x) / (contentExtent.XLength()),1.0));
                uvCoords[i].y = max(0.0, min((face[i].y - contentExtent.low.y) / (contentExtent.YLength()), 1.0));
                }
            indicesOfTexturedRegion.push_back(idx[0]);
            indicesOfTexturedRegion.push_back(idx[1]);
            indicesOfTexturedRegion.push_back(idx[2]);
            uvsOfTexturedRegion[idx[0] - 1] = uvCoords[0];
            uvsOfTexturedRegion[idx[1] - 1] = uvCoords[1];
            uvsOfTexturedRegion[idx[2] - 1] = uvCoords[2];
            }

        ClearPtsIndices();    
        PushPtsIndices(&indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());
        PushUV(/*texId + 1,*/ &uvsOfTexturedRegion[0], uvsOfTexturedRegion.size());
        PushUVsIndices(0, &indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());        
        }

    SetDirty(true);

    delete[] pixelBufferP;
    delete[] pixelBufferPRGBA;
    pTextureBitmap = 0;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
    template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRasterRecursive(HIMMosaic* sourceRasterP, Transform unitTransform)
    {

        TextureFromRaster(sourceRasterP, unitTransform);

    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->TextureFromRasterRecursive(sourceRasterP, unitTransform);
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (m_apSubNodes[indexNodes] != nullptr)
                {
                auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes]);
                assert(mesh != nullptr);
                mesh->TextureFromRasterRecursive(sourceRasterP, unitTransform);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::RefreshMergedClipsRecursive()
    {
    BuildSkirts();
    ComputeMergedClips();
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->RefreshMergedClipsRecursive();
        }
    else if (!IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->RefreshMergedClipsRecursive();
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::HasClip(uint64_t clipId)
    {
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    if (!diffsetPtr.IsValid()) return false;
    for (const auto& diffSet : *diffsetPtr)
        {
        if (diffSet.clientID == clipId && (!diffSet.upToDate || !diffSet.IsEmpty() || diffSet.clientID == (uint64_t)-1)) return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 05/16
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::IsClippingUpToDate()
    {
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    for (const auto& diffSet : *diffsetPtr)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ComputeMergedClips()
    {
    if (dynamic_cast<SMMeshIndex<POINT,EXTENT>*>(m_SMIndex)->m_isInsertingClips) return;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();

    if (!diffSetPtr.IsValid())
        return;


    {
    if (diffSetPtr->size() == 0) return;
    for (const auto& diffSet : *diffSetPtr)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
        }
    //std::cout << "Merging clips for " << GetBlockID().m_integerID << " we have " << diffSetPtr->size() << "clips" << std::endl;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    vector<DPoint3d> points(pointsPtr->size());
    
    PtToPtConverter::Transform(&points[0], &(*pointsPtr)[0], points.size());

    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));


    bvector<bvector<DPoint3d>> polys;
    bvector<uint64_t> clipIds;
    bvector<DifferenceSet> skirts;
    bvector<bpair<double, int>> metadata;
    for (const auto& diffSet : *diffSetPtr)
        {
        //uint64_t upperId = (diffSet.clientID >> 32);
        if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && diffSet.toggledForID)
            {
            clipIds.push_back(diffSet.clientID);
            polys.push_back(bvector<DPoint3d>());
            GetClipRegistry()->GetClip(diffSet.clientID, polys.back());
            DRange3d polyExtent = DRange3d::From(&polys.back()[0], (int)polys.back().size());
            if (!polyExtent.IntersectsWith(nodeRange, 2))
                {
                polys.resize(polys.size() - 1);
                clipIds.resize(clipIds.size() - 1);
                continue;
                }
            double importance;
            int nDimensions;
            GetClipRegistry()->GetClipMetadata(diffSet.clientID, importance, nDimensions);
            metadata.push_back(make_bpair(importance, nDimensions));
            }
        else if (!diffSet.toggledForID)
            {
            skirts.push_back(diffSet);
            }
             
        }
    diffSetPtr->clear();
    for(auto& skirt: skirts) diffSetPtr->push_back(skirt);
    m_nbClips = skirts.size();
    
            
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(GetPtsIndicePtr());

    if (ptIndices->size() == 0)
        {
        DifferenceSet current;
        current.clientID = 0;
        diffSetPtr->push_back(current);
        const_cast<DifferenceSet&>(*(diffSetPtr->begin() + (diffSetPtr->size() - 1))).upToDate = true;
        ++m_nbClips;        
        }
    else
        {
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes = GetUVsIndicesPtr();
        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords = GetUVCoordsPtr();

        const int32_t* uvIndices = uvIndexes.IsValid() ? &(*uvIndexes)[0] : 0;
        const DPoint2d* uvBuffer= uvCoords.IsValid() ? &(*uvCoords)[0] : 0;        
    

        Clipper clipNode(&points[0], points.size(), (int32_t*)&(*ptIndices)[0], ptIndices->size(), nodeRange, uvBuffer, uvIndices);
        bvector<bvector<PolyfaceHeaderPtr>> polyfaces;
        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
            new ScalableMeshNode<POINT>(nodePtr)
#else
            ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
            );

        bool hasClip = false;
        if (!m_nodeHeader.m_arePoints3d)
            {
            BcDTMPtr dtm = nodeP->GetBcDTM().get();
            if (dtm.get() != nullptr)
                {
                BcDTMPtr toClipBcDTM = dtm->Clone();
                DTMPtr toClipDTM = toClipBcDTM.get();
                if (IsLeaf()) //always clip leaves regardless of width/area criteria
                    {
                    for (auto& mdata : metadata)
                        mdata.second = 0;
                    }
                if (toClipBcDTM->GetTinHandle() != nullptr) hasClip = clipNode.GetRegionsFromClipPolys(polyfaces, polys, metadata, toClipDTM);
                }
            }
        else
            {
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            flags->SetLoadTexture(true);
            IScalableMeshMeshPtr meshP = nodeP->GetMesh(flags);
            if (meshP.get() != nullptr)
                hasClip = GetRegionsFromClipPolys3D(polyfaces, polys, meshP->GetPolyfaceQuery());
            }

        if (hasClip) 
            {
            bvector<bvector<PolyfaceHeaderPtr>> skirts;
            //BuildSkirtMeshesForPolygonSet(skirts, polyfaces, polys, nodeRange);
            map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));

            for (size_t i = 0; i < points.size(); ++i)
                mapOfPoints[points[i]] = (int)i;
            for (auto& polyface : polyfaces)
                {
                DifferenceSet current = DifferenceSet::FromPolyfaceSet(polyface, mapOfPoints, points.size() + 1);
                for (auto& poly : polyface) poly = nullptr;
                if (&polyface - &polyfaces[0] == 0) current.clientID = 0;
                else current.clientID = clipIds[(&polyface - &polyfaces[0]) - 1];
                diffSetPtr->push_back(current);
                const_cast<DifferenceSet&>(*((diffSetPtr->begin() + (diffSetPtr->size() - 1)))).upToDate = true;
                ++m_nbClips;
                } 
            }
        }
    }

    DifferenceSet allClips;
    allClips.clientID = (uint64_t)-1;
    bool added = false;
    for (const auto& diffSet : *diffSetPtr)
        if (diffSet.clientID == (uint64_t)-1) { const_cast<DifferenceSet&>(diffSet) = allClips; const_cast<DifferenceSet&>(diffSet).upToDate = true; added = true; }
    if (!added)
        {
        diffSetPtr->push_back(allClips);
        const_cast<DifferenceSet&>(*(diffSetPtr->begin() + (diffSetPtr->size() - 1))).upToDate = true;
        m_nbClips++;
        }
    assert(m_nbClips > 0 || diffSetPtr->size() == 0);
    //std::cout << "Merged clips for " << GetBlockID().m_integerID << " we have " << diffSetPtr->size() << "clips" << std::endl;

    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::BuildSkirts()
    {
    if (dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->m_isInsertingClips) return;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    if (!diffsetPtr.IsValid() || diffsetPtr->size() == 0) return;
    for (const auto& diffSet : *diffsetPtr)
            {
            if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
            }

        //DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
         //                                   ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));


        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
            new ScalableMeshNode<POINT>(nodePtr)
#else
            ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
            );
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

        auto dtm = nodeP->GetBcDTM();
        if (dtm.get() == nullptr) return;
            SkirtBuilder builder(dtm);
            map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
            for (size_t i = 0; i < pointsPtr->size(); ++i)
                mapOfPoints[pointsPtr->operator[](i)] = (int)i;

            for (const auto& diffSet : *diffsetPtr)
            {
            if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && !diffSet.toggledForID)
                {
                bvector<bvector<DPoint3d>> skirts;
                GetClipRegistry()->GetSkirt(diffSet.clientID, skirts);
                bvector<PolyfaceHeaderPtr> polyfaces;
                builder.BuildSkirtMesh(polyfaces, skirts);
                DifferenceSet current = DifferenceSet::FromPolyfaceSet(polyfaces, mapOfPoints, pointsPtr->size() + 1);
                current.clientID = diffSet.clientID;
                current.toggledForID = false;
                //diffSet = current;
                diffsetPtr->Replace(&diffSet - &(*diffsetPtr->begin()), current);
                }
            }
    }

template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::ClipIntersectsBox(uint64_t clipId, EXTENT ext)
    {
    DRange3d extRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), 0,
                                       ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), 0);
    bvector<DPoint3d> polyPts;
    GetClipRegistry()->GetClip(clipId, polyPts);

    DRange3d polyRange;
    size_t n = 0;
    bool noIntersect = true;
    for (auto&pt : polyPts)
        {
        if (extRange.IsContainedXY(pt)) ++n;
        pt.z = 0;
        polyRange.Extend(pt);
        if (noIntersect && &pt - &polyPts[0] != 0)
            {
            DRange3d edgeRange = DRange3d::From(pt, polyPts[&pt - &polyPts[0] - 1]);
            if (extRange.IntersectsWith(edgeRange))
                {
                noIntersect = false;
                }
            }
        }
    if (n >2) return true;

    ICurvePrimitivePtr curvePtr(ICurvePrimitive::CreateLineString(polyPts));
    CurveVectorPtr curveVectorPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr));

    extRange.low.z = extRange.high.z = 0;
    DPoint3d box[5] = { extRange.low, DPoint3d::From(extRange.low.x, extRange.high.y, 0), extRange.high, DPoint3d::From(extRange.high.x, extRange.low.y, 0), extRange.low };
    bool allInsidePolygon = true;
    for (size_t i = 0; i < 4 && allInsidePolygon; ++i)
        {
        auto classif =curveVectorPtr->PointInOnOutXY(box[i]);
        if (classif == CurveVector::InOutClassification::INOUT_Out) allInsidePolygon = false;
        }
    if (allInsidePolygon) return true;
    if (noIntersect) return false;

    extRange.low.z = -100;
    extRange.high.z = 100;
    bvector<DPoint3d> clippedPts;
    DRange3d clippedExt;
    ClipFeatureDefinition((uint32_t)DTMFeatureType::Polygon, extRange, clippedPts, clippedExt, polyPts, polyRange);

    ICurvePrimitivePtr boxCurvePtr(ICurvePrimitive::CreateLineString(box,5));
    CurveVectorPtr boxCurveVecPtr(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, boxCurvePtr));
    if (clippedExt.IsNull() || clippedPts.empty()) return false;
    if (!clippedExt.IsEqual(polyRange))
        {
        curvePtr = ICurvePrimitive::CreateLineString(clippedPts);
        curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
        }
    const CurveVectorPtr intersection = CurveVector::AreaIntersection(*curveVectorPtr, *boxCurveVecPtr);

    return (!intersection.IsNull());
    }
    

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
    template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::AddClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {

    if (m_nodeHeader.m_nodeCount == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return true;

        {

        DifferenceSet d;
#if DEBUG && SM_TRACE_CLIPS
        std::string s;
        s += " AREA IS" + std::to_string(bsiGeom_getXYPolygonArea(&clipPts[0], (int)clipPts.size()));
#endif
        bool emptyClip = false;
        //if (nodeRange.XLength() <= clipExt.XLength() * 10000 && nodeRange.YLength() <= clipExt.YLength() * 10000)
        if (ClipIntersectsBox(clipId, m_nodeHeader.m_nodeExtent))

            {
                    bool clipFound = false;
        RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();
        for (const auto& diffSet : *diffSetPtr) if (diffSet.clientID == clipId && diffSet.toggledForID == setToggledWhenIdIsOn) clipFound = true;
        if (clipFound) return true; //clip already added
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
            //std::cout << " adding clip " << clipId << " to node " << GetBlockID().m_integerID << std::endl;
            d.clientID = clipId;
            d.firstIndex = (int32_t)pointsPtr->size() + 1;
            d.toggledForID = setToggledWhenIdIsOn;
            diffSetPtr->push_back(d);
            m_nbClips++;
            const_cast<DifferenceSet&>(*(diffSetPtr->begin() + (diffSetPtr->size() - 1))).upToDate = false;
            for (const auto& other : *diffSetPtr)
                {
                if (other.clientID == ((uint64_t)-1)) const_cast<DifferenceSet&>(other).upToDate = false;
                }
            
            }


        else
            {
            emptyClip = true;
            //std::cout << " discounting clip " << clipId << " for node " << GetBlockID().m_integerID << " because test failed " << std::endl;
            }
       // if (d.addedFaces.size() == 0 && d.removedFaces.size() == 0 && d.addedVertices.size() == 0 && d.removedVertices.size() == 0) emptyClip = true;

        if (isVisible && !emptyClip)
            {
            PropagateClipUpwards(clipId, ClipAction::ACTION_ADD);
            PropagateClipToNeighbors(clipId, ClipAction::ACTION_ADD);
            }
        PropagateClip(clipId, ClipAction::ACTION_ADD);
        return !emptyClip;
        }

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::DeleteClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    if (pointsPtr->size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return true;
    bool found = false;
    bvector<size_t> indices;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();
    for (auto it = diffSetPtr->begin(); it != diffSetPtr->end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {
            indices.push_back(it-diffSetPtr->begin());
            m_nbClips--;
            //if (m_nodeHeader.m_level < 7) PropagateDeleteClipImmediately(clipId);
            found = true;
            }
        else if (it->clientID == (uint64_t)-1)
            {
            const_cast<DifferenceSet&>(*it).upToDate = false;
            }
        }
    if(found) diffSetPtr->erase(indices);
    return found;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::ModifyClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    if (pointsPtr->size() == 0 || m_nodeHeader.m_nbFaceIndexes < 3) return true;

    bool found = false;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();
    for (auto it = diffSetPtr->begin(); it != diffSetPtr->end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {

            //std::cout << " updating clip " << clipId << " to node " << GetBlockID().m_integerID << " toggle is "<<setToggledWhenIdIsOn<< std::endl;
            const_cast<DifferenceSet&>(*it) = DifferenceSet();
            const_cast<DifferenceSet&>(*it).clientID = clipId;
            const_cast<DifferenceSet&>(*it).toggledForID = setToggledWhenIdIsOn;
            found = true;

            }

        else if (it->clientID == (uint64_t)-1)
            {
            const_cast<DifferenceSet&>(*it).upToDate = false;
            }

        }
    if (!found)
        {
        found = ClipIntersectsBox(clipId, m_nodeHeader.m_nodeExtent);
        if (found) AddClip(clipId, isVisible, setToggledWhenIdIsOn);
        }
    return found;
    }
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateDeleteClipImmediately(uint64_t clipId)
    {
    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != nullptr)
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->DeleteClip(clipId, false);
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (m_apSubNodes[i] != nullptr)
                    {
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[i])->DeleteClip(clipId, false);
                    }
                }
            }
        }
    auto node = GetParentNode();
    while (node != nullptr && node->GetLevel() > 1)
        {
        node = node->GetParentNode();
        }
    if (node != nullptr)
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node)->DeleteClip(clipId, false);
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->GetParentNode())->DeleteClip(clipId, false);
        for (size_t i = 0; i < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); ++i)
            {
            if (node->GetParentNode()->m_apSubNodes[i] != nullptr) dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->GetParentNode()->m_apSubNodes[i])->DeleteClip(clipId, false);
            }
        }

    for (size_t n = 0; n < MAX_NUM_NEIGHBORNODE_POSITIONS; ++n)
        {
        for (auto& node : m_apNeighborNodes[n])
            if (node != nullptr)
                {
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node)->DeleteClip(clipId, false);
                }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClip(uint64_t clipId, ClipAction action)
    {
    std::vector<SMTask> createdTasks;

    if (HasRealChildren())
        {
        //see http://stackoverflow.com/questions/14593995/problems-with-stdfunction for use of std::mem_fn below
        auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*,uint64_t, bool,bool) >();
        switch (action)
            {
            case ACTION_ADD:
                func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
                break;
            case ACTION_MODIFY:
                func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
                break;
            default:
                break;
            }
        if (m_pSubNodeNoSplit != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) m_pSubNodeNoSplit.GetPtr(), clipId,  false,true),false));
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (m_apSubNodes[i] != nullptr)
                    {
                    createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) m_apSubNodes[i].GetPtr(), clipId, false,true),false));
                    }
                }
            }
        }

    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task);

    s_schedulerLock.unlock();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClipToNeighbors(uint64_t clipId,  ClipAction action)
    {
    std::vector<SMTask> createdTasks;
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t,  bool, bool) >();
    switch (action)
        {
        case ACTION_ADD:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
            break;
        case ACTION_MODIFY:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
            break;

        default:
            break;
        }

    for (size_t n = 0; n < MAX_NUM_NEIGHBORNODE_POSITIONS; ++n)
        {
        for (auto& node : m_apNeighborNodes[n])
            if (node != nullptr)
                {
                createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId,  false,true), false));
                }
        }

    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task);

    s_schedulerLock.unlock();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClipUpwards(uint64_t clipId, ClipAction action)
    {
    std::vector<SMTask> createdTasks;
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t, bool, bool) >();
    switch (action)
        {
        case ACTION_ADD:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::AddClip);
            break;
        case ACTION_MODIFY:
            func = std::mem_fn(&SMMeshIndexNode<POINT, EXTENT>::ModifyClip);
            break;
        default:
            break;
        }
    auto node = GetParentNode();
    while (node != nullptr && node->GetLevel() > 1)
        {
        node = node->GetParentNode();
        }
    if (node != nullptr)
        {
        createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId, false,true), false));
        if (node->GetParentNode() != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode().GetPtr(), clipId, false,true), false));

            for (size_t i = 0; i < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (node->GetParentNode()->m_apSubNodes[i] != nullptr) createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode()->m_apSubNodes[i].GetPtr(), clipId, false,true), false));
                }
            }
        }
    s_schedulerLock.lock();
    if (s_clipScheduler == nullptr) s_clipScheduler = new ScalableMeshScheduler();

    for (auto& task : createdTasks) s_clipScheduler->ScheduleTask(task, ScalableMeshScheduler::PRIORITY_HIGH);

    s_schedulerLock.unlock();
    }

    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 10/14
    //=======================================================================================
    template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::NeedsMeshing() const
        {
        if (!IsLoaded())
            Load();
#ifdef WIP_MESH_IMPORT
        if (m_existingMesh) return true;
#endif
        bool needsMeshing = false;

        if (!HasRealChildren())
            needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0;
        else
            {
            if (IsParentOfARealUnsplitNode())
                {
                needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0 || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsMeshing();
                }
            else
                {
                needsMeshing = m_nodeHeader.m_nbFaceIndexes == 0;

                for (size_t indexNode = 0; !needsMeshing && indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    needsMeshing = (needsMeshing || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsMeshing());
                    }
                }
            }

        return needsMeshing;

        }
    static size_t s_importedFeatures;

template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::SMMeshIndex(ISMDataStoreTypePtr<EXTENT>& smDataStore,
                                                                             SMMemoryPoolPtr& smMemoryPool,                                                                             
                                                                             size_t SplitTreshold,
                                                                             ISMPointIndexFilter<POINT, EXTENT>* filter,
                                                                             bool balanced,
                                                                             bool textured,
                                                                             bool propagatesDataDown,
                                                                             ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                             ISMPointIndexMesher<POINT, EXTENT>* mesher3d)
                                                                             : SMPointIndex<POINT, EXTENT>(smDataStore, SplitTreshold, filter, balanced, propagatesDataDown, false), 
                                                                             m_smDataStore(smDataStore),
                                                                             m_smMemoryPool(smMemoryPool)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;    
    m_isInsertingClips = false;

    s_importedFeatures = 0;
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }    
    }


template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::~SMMeshIndex()
    {
    if (m_mesher2_5d != NULL)
        delete m_mesher2_5d;

    if (m_mesher3d != NULL)
        delete m_mesher3d;
    Store();
    if (m_pRootNode != NULL)
        m_pRootNode->Unload();

    m_pRootNode = NULL;
    m_createdNodeMap.clear();
    }


template <class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(EXTENT extent, bool isRootNode)
    {
    SMMeshIndexNode<POINT, EXTENT> * meshNode = new SMMeshIndexNode<POINT, EXTENT>(m_indexHeader.m_SplitTreshold, extent, this, m_filter, m_needsBalancing, IsTextured(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode)
    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT>> parent;

    auto meshNode = new SMMeshIndexNode<POINT, EXTENT>(blockID, parent, this, m_filter, m_needsBalancing, IsTextured(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }


template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndex<POINT, EXTENT>::GetMesher2_5d()
    {
    return(m_mesher2_5d);
    }

template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndex<POINT, EXTENT>::GetMesher3d()
    {
    return(m_mesher3d);
    }


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::TextureFromRaster(HIMMosaic* sourceRasterP, Transform unitTransform)
    {
    if (m_indexHeader.m_terrainDepth == (size_t)-1)
        {
        m_indexHeader.m_terrainDepth = m_pRootNode->GetDepth();
        }
    if (sourceRasterP == nullptr || sourceRasterP->GetEffectiveShape() == nullptr || sourceRasterP->GetEffectiveShape()->IsEmpty()) return;
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->TextureFromRasterRecursive(sourceRasterP, unitTransform);
   // WaitForThreadStop();
    for (auto& task : m_textureWorkerTasks) task.get();
    m_indexHeader.m_depth = (size_t)-1;
    m_indexHeader.m_depth = GetDepth();
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn)
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->ClipActionRecursive(action, clipId, extent, setToggledWhenIDIsOn);
    }

#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs)
    {
    if (0 == nPts)
        return;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent);
        else
            m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z), true);
        }
    size_t nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddMeshDefinition(pts, nPts, indices, nIndices, extent, m_indexHeader.m_HasMaxExtent, metadata,texData,texSize,uvs);
    if (0 == nAddedPoints)
        {
        //can't add feature, need to grow extent
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
        while (!extent.IsContained(nodeRange))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return;

            // The extent is not contained... we must create a new node
            PushRootDown(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
            nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                       ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
            }


        // The root node contains the spatial object ... add it
        nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddMeshDefinition(pts, nPts, indices, nIndices, extent, m_indexHeader.m_HasMaxExtent, metadata, texData,texSize,uvs);
        assert(nAddedPoints >= nPts);
        }
    }
#endif

template<class POINT, class EXTENT>  int64_t  SMMeshIndex<POINT, EXTENT>::AddTexture(int width, int height, int nOfChannels, const byte* texData, size_t nOfBytes)
    {

    ISMTextureDataStorePtr nodeDataStore;
    SMIndexNodeHeader<EXTENT> nodeHeader;
    bool result = GetDataStore()->GetNodeDataStore(nodeDataStore, &nodeHeader);
    assert(result == true);

    size_t texID = GetNextTextureId();

    size_t size = nOfBytes + 3 * sizeof(int);
    bvector<uint8_t> texture(size);
    memcpy(texture.data(), &width, sizeof(int));
    memcpy(texture.data() + sizeof(int), &height, sizeof(int));
    memcpy(texture.data() + 2 * sizeof(int), &nOfChannels, sizeof(int));
    memcpy(texture.data() + 3 * sizeof(int), texData, nOfBytes);
    RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
#ifndef VANCOUVER_API
        new SMStoredMemoryPoolBlobItem<Byte>(texID, nodeDataStore, texture.data(), size, SMStoreDataType::Texture, (uint64_t)(this))
#else
        SMStoredMemoryPoolBlobItem<Byte>::CreateItem(texID, nodeDataStore, texture.data(), size, SMStoreDataType::Texture, (uint64_t)(this))
#endif
        );
    SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
    auto id = GetMemoryPool()->AddItem(memPoolItemPtr);
    GetMemoryPool()->RemoveItem(id, texID, SMStoreDataType::Texture, (uint64_t)this);
    return (int64_t)texID;
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
    {
    ++s_importedFeatures;
    if (0 == points.size())
        return;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent); 
        else
        m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z), true);
        }
    size_t nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent);
    if (0 == nAddedPoints)
        {
        //can't add feature, need to grow extent
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
        while (!extent.IsContained(nodeRange))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return;

            // The extent is not contained... we must create a new node
            PushRootDown(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
            nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent),
                                       ExtentOp<EXTENT>::GetXMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_pRootNode->m_nodeHeader.m_nodeExtent));
            }


        // The root node contains the spatial object ... add it
        nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent);
        assert(nAddedPoints >= points.size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent)
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddClipDefinitionRecursive(points,extent);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::RefreshMergedClips()
    {
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->RefreshMergedClipsRecursive();
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  size_t  SMMeshIndex<POINT, EXTENT>::GetNextTextureId()
    {
    return ++m_texId;
    }


/**----------------------------------------------------------------------------
Mesh
Mesh the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Mesh()
    {
    HINVARIANTS;

    bool result = m_mesher2_5d->Init(*this);
    assert(result == true);

    result = m_mesher3d->Init(*this);
    assert(result == true);

    // Check if root node is present
    if (m_pRootNode != NULL)
        {
        HFCPtr<SMMeshIndexNode<POINT, EXTENT>> node = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode);
            node->Mesh();
        }

    HINVARIANTS;
    }

/**----------------------------------------------------------------------------
Save cloud ready format
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> StatusInt SMMeshIndex<POINT, EXTENT>::SaveMeshToCloud(DataSourceManager *dataSourceManager, const WString& path, const bool& pi_pCompress)
    {
    ISMDataStoreTypePtr<EXTENT>     pDataStore = new SMStreamingStore<EXTENT>(*dataSourceManager, path, pi_pCompress);

    this->SaveMasterHeaderToCloud(pDataStore);

    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetRootNode().GetPtr())->SaveMeshToCloud(pDataStore);

    return SUCCESS;
    }

/**----------------------------------------------------------------------------
Stitch
Stitch the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Stitch(int pi_levelToStitch, bool do2_5dStitchFirst)
    {
    HINVARIANTS;

    // Check if root node is present
    if (m_pRootNode != NULL)
        {

        try
            {

            if (do2_5dStitchFirst)
                {
                //Done for level stitching.
                assert(pi_levelToStitch != -1);
                vector<SMMeshIndexNode<POINT, EXTENT>*> nodesToStitch;
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);

                auto nodeIter(nodesToStitch.begin());

                //Do 2.5d stitch first
                while (nodeIter != nodesToStitch.end())
                    {
                    if (!(*nodeIter)->m_nodeHeader.m_arePoints3d && (*nodeIter)->AreAllNeighbor2_5d())
                        {
                        if (m_mesher2_5d->Stitch(*nodeIter))
                            {
                            (*nodeIter)->SetDirty(true);
                            }

                        nodeIter = nodesToStitch.erase(nodeIter);
                        }
                    else
                        {
                        nodeIter++;
                        }
                    }

                //Do 3d stitch last
                nodeIter = nodesToStitch.begin();
                auto nodeIterEnd = nodesToStitch.end();

                while (nodeIter != nodeIterEnd)
                    {
                    assert((*nodeIter)->m_nodeHeader.m_arePoints3d || !(*nodeIter)->AreAllNeighbor2_5d());

                    if (m_mesher3d->Stitch(*nodeIter))
                        {
                        (*nodeIter)->SetDirty(true);
                        }

                    nodeIter++;
                    }
                }
            else if (s_useThreadsInStitching)
                {
                vector<SMMeshIndexNode<POINT, EXTENT>*> nodesToStitch;
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);
                if (nodesToStitch.size() == 0) return;
                if (nodesToStitch.size() <= 72)
                    {
                    s_useThreadsInStitching = false;
                    ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, 0);
                    s_useThreadsInStitching = true;
                    return;
                    }
                set<SMMeshIndexNode<POINT, EXTENT>*> stitchedNodes;
                std::recursive_mutex stitchedMutex;
                for (auto& node : nodesToStitch) s_nodeMap.insert(std::make_pair((void*)node, (unsigned int)-1));
                for (size_t i =0; i < 8; ++i)
                    RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>** vec, set<SMMeshIndexNode<POINT,EXTENT>*>* stitchedNodes, std::recursive_mutex* stitchedMutex, size_t nNodes, size_t threadId) ->void
                    {
                    vector<SMMeshIndexNode<POINT, EXTENT>*> myNodes;
                    size_t firstIdx = nNodes / 8 * threadId;
                    myNodes.push_back(vec[firstIdx]);
                    while (myNodes.size() > 0)
                        {
                        //grab node
                        SMMeshIndexNode<POINT, EXTENT>* current = myNodes[0];
                        vector<SMPointIndexNode<POINT, EXTENT>*> neighbors;
                        current->GetAllNeighborNodes(neighbors);
                        neighbors.push_back(current);
                        bool reservedNode = false;

                        reservedNode = TryReserveNodes(s_nodeMap, (void**)&neighbors[0], neighbors.size(),(unsigned int)threadId);

                        if (reservedNode == true)
                            {
                            bool needsStitching = false;
                            stitchedMutex->lock();
                            if (stitchedNodes->count(current) != 0) needsStitching = false;
                            else
                                {
                                stitchedNodes->insert(current);
                                needsStitching = true;
                                }
                            for (auto& neighbor : neighbors)
                                if (std::find(myNodes.begin(), myNodes.end(), neighbor) == myNodes.end() && stitchedNodes->count((SMMeshIndexNode<POINT,EXTENT>*)neighbor) == 0) myNodes.push_back((SMMeshIndexNode<POINT, EXTENT>*)neighbor);
                            stitchedMutex->unlock();
                            if(needsStitching) current->Stitch((int)current->m_nodeHeader.m_level, 0);
                            unsigned int val = (unsigned int)-1;
                            unsigned int id = (unsigned int)threadId;
                            s_nodeMap[current].compare_exchange_strong(id, val);
                            }
                        myNodes.erase(myNodes.begin());
                        if (myNodes.size() == 0 && firstIdx + 1 < nNodes / 8 * (threadId+1))
                            {
                            firstIdx += 1;
                            myNodes.push_back(vec[firstIdx]);
                            }
                        }
                    SetThreadAvailableAsync(threadId);
                    }, &nodesToStitch[0], &stitchedNodes, &stitchedMutex, nodesToStitch.size(), std::placeholders::_1));

                WaitForThreadStop();
                s_nodeMap.clear();
                for (auto& node : nodesToStitch)
                    {
                    if (stitchedNodes.count(node) == 0)
                        node->Stitch((int)node->m_nodeHeader.m_level, 0);
                    }
                }
            else
                {
                ((SMMeshIndexNode<POINT, EXTENT>*)&*m_pRootNode)->Stitch(pi_levelToStitch, 0);
                }
            }
        catch (...)
            {
            throw;
            }
        }

    HINVARIANTS;
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetClipRegistry(ClipRegistry* clipRegistry)
    {
    m_clipRegistry = clipRegistry;
    }
