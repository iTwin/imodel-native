#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <ImagePP/all/h/HFCException.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include "ScalableMeshProgress.h"

#include "Stores/SMStreamingDataStore.h"
#include "ScalableMesh/IScalableMeshPublisher.h"
#ifndef VANCOUVER_API
#include "TilePublisher/TilePublisher.h"
#endif

#include "ScalableMeshQuery.h"

#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <Mtg/MtgStructs.h>
//#include <Geom/bsp/bspbound.fdf>
#include "ScalableMesh/ScalableMeshGraph.h"
#include <string>
#include <queue>
#include "ScalableMeshMesher.h"
#include <ctime>
#include <fstream>
#include <codecvt>
#include "Edits/ClipUtilities.h"
#include "vuPolygonClassifier.h"
#include "LogUtils.h"
#include "Edits/Skirts.h"
#include <map>
#include <json/json.h>


#include "StreamTextureProvider.h"

#include "ScalableMeshQuadTreeQueries.h"
#if !defined(HVERIFYCONTRACT)
#   define THIS_HINVARIANTS
#else
#   define THIS_HINVARIANTS  this->ValidateInvariants()
#endif

USING_NAMESPACE_BENTLEY_SCALABLEMESH
#define SM_OUTPUT_MESHES_GRAPH 0
#define SM_TRACE_BUILD_SKIRTS 0

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Init()
    {
    m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    this->m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_displayMeshVideoPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    
    m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    this->m_nodeHeader.m_graphID = ISMStore::GetNullNodeID();
    this->m_nodeHeader.m_ptsIndiceID.resize(1);
    this->m_nodeHeader.m_ptsIndiceID[0] = ISMStore::GetNullNodeID();

    this->m_nodeHeader.m_uvsIndicesID.resize(1);
    this->m_nodeHeader.m_uvsIndicesID[0] = ISMStore::GetNullNodeID();

    this->m_nodeHeader.m_uvID = ISMStore::GetNullNodeID();

    this->m_nodeHeader.m_textureID = ISMStore::GetNullNodeID();
    this->m_nodeHeader.m_ptsIndiceID[0] = this->GetBlockID();

    this->m_updateClipTimestamp = dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->m_nodeInstanciationClipTimestamp;
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(uint64_t nodeID,
                                                                                     size_t pi_SplitTreshold,
                                                                                     const EXTENT& pi_rExtent,
                                                                                     SMMeshIndex<POINT, EXTENT>* meshIndex,
                                                                                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                                                                                     bool balanced,
                                                                                     bool textured,
                                                                                     bool propagateDataDown,
                                                                                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                                                                                     CreatedNodeMap*                      createdNodeMap)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(nodeID, pi_SplitTreshold, pi_rExtent, filter, balanced, propagateDataDown, createdNodeMap)
    {
    this->m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;

//#ifdef WIP_MESH_IMPORT        
    m_existingMesh = false;
//#endif

    Init();

    m_nbClips = 0;

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
    this->m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
             
//#ifdef WIP_MESH_IMPORT        
    m_existingMesh = false;
//#endif

    Init();

    m_nbClips = 0;

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(uint64_t nodeId,
                                                                                     size_t pi_SplitTreshold,
                                                                                     const EXTENT& pi_rExtent,
                                                                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(nodeId,pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(pi_rpParentNode.GetPtr()))
    {
    this->m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();

//#ifdef WIP_MESH_IMPORT         
    m_existingMesh = false;
//#endif

    m_nbClips = 0;

    Init();

    }


template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                const EXTENT& pi_rExtent,
                const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode)
                : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode<POINT,EXTENT>*>(pi_rpParentNode.GetPtr()))
    {
    this->m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
     
//#ifdef WIP_MESH_IMPORT         
    m_existingMesh = false;
//#endif

    m_nbClips = 0;

    Init(); 

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                                                                                     const EXTENT& pi_rExtent,
                                                                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                                                                     bool IsUnsplitSubLevel)
                                                                                     : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode<POINT,EXTENT>*>(pi_rpParentNode.GetPtr()), IsUnsplitSubLevel)                                                                              
                                                                                     
    {
    this->m_SMIndex = pi_rpParentNode->m_SMIndex;
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();

     
//#ifdef WIP_MESH_IMPORT         
    m_existingMesh = false;
//#endif
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
    this->m_SMIndex = meshIndex;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
     
//#ifdef WIP_MESH_IMPORT                
    m_existingMesh = false;
//#endif
    m_nbClips = 0;

    Init();

    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::~SMMeshIndexNode()
    {
    if (!this->IsDestroyed())
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
    SMPointIndexNode<POINT,EXTENT>::Destroy();

    if (this->GetBlockID().IsValid())
        {                
        GetMemoryPool()->RemoveItem(m_triIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::TriPtIndices, (uint64_t)this->m_SMIndex);
        ISMInt32DataStorePtr int32DataStore;
        bool result = dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->GetDataStore()->GetNodeDataStore(int32DataStore, &this->m_nodeHeader, SMStoreDataType::TriPtIndices);
        assert(result == true && int32DataStore.IsValid());      
        int32DataStore->DestroyBlock(this->GetBlockID());
        m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(this->m_texturePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Texture, (uint64_t)this->m_SMIndex);
        ISMTextureDataStorePtr nodeTextureStore;
        result = dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->GetDataStore()->GetNodeDataStore(nodeTextureStore, &this->m_nodeHeader);
        assert(result == true && nodeTextureStore.IsValid());  
        nodeTextureStore->DestroyBlock(this->GetBlockID());
        this->m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_triUvIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::TriUvIndices, (uint64_t)this->m_SMIndex);
        result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(int32DataStore, &this->m_nodeHeader, SMStoreDataType::TriUvIndices);
        assert(result == true && int32DataStore.IsValid());      
        int32DataStore->DestroyBlock(this->GetBlockID());
        m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_uvCoordsPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::UvCoords, (uint64_t)this->m_SMIndex);
        ISMUVCoordsDataStorePtr nodeUVCoordsStore;
        result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeUVCoordsStore, &this->m_nodeHeader);
        assert(result == true && nodeUVCoordsStore.IsValid());      
        nodeUVCoordsStore->DestroyBlock(this->GetBlockID());
        m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)this->m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

            {
            std::lock_guard<std::mutex> lock(m_displayMeshLock);
            GetMemoryPool()->RemoveItem(m_displayMeshPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)this->m_SMIndex);
            m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

            SMMemoryPool::GetInstanceVideo()->RemoveItem(m_displayMeshVideoPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)this->m_SMIndex);
            m_displayMeshVideoPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }

        GetMemoryPool()->RemoveItem(m_graphPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex);
        ISMMTGGraphDataStorePtr nodeGraphStore;
        result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeGraphStore, &this->m_nodeHeader);
        assert(result == true && nodeGraphStore.IsValid());  
        nodeGraphStore->DestroyBlock(this->GetBlockID());
        m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        
        if (m_nbClips > 0)
            {
            GetMemoryPool()->RemoveItem(m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex);
            ISDiffSetDataStorePtr nodeDiffsetStore;
            result = this->m_SMIndex->GetDataStore()->GetSisterNodeDataStore(nodeDiffsetStore, &this->m_nodeHeader, false);
            if (nodeDiffsetStore.IsValid()) nodeDiffsetStore->DestroyBlock(this->GetBlockID());
            m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }

        GetMemoryPool()->RemoveItem(m_featurePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t)this->m_SMIndex);
        ISMInt32DataStorePtr nodeFeatureStore;
        result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeFeatureStore, &this->m_nodeHeader, SMStoreDataType::LinearFeature);
        assert(result == true && nodeFeatureStore.IsValid());         
        if (nodeFeatureStore.IsValid()) nodeFeatureStore->DestroyBlock(this->GetBlockID());
        m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
       
        GetMemoryPool()->RemoveItem(m_dtmPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)this->m_SMIndex);
        m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        RemoveMultiTextureData();
        }

    THIS_HINVARIANTS;

    return true;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneChild(uint64_t nodeId, const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(nodeId, this->GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    pNewNode->SetDirty(true);
    return pNewNode;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(this->GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    pNewNode->SetDirty(true);
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(this->GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this), true));
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
    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, this, dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex), this->m_filter, this->m_needsBalancing, false, !(this->m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, this->m_createdNodeMap);
    node->m_clipRegistry = m_clipRegistry;
    node->m_loadNeighbors = this->m_loadNeighbors;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(node);
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode)
    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT>> parent;

    auto node = new SMMeshIndexNode<POINT, EXTENT>(blockID, parent, dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex), this->m_filter, this->m_needsBalancing, false, !(this->m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, this->m_createdNodeMap);

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
    THIS_HINVARIANTS;
    bool returnValue = true;
    if (!m_remainingUnappliedEdits.empty())
        {
        UpdateData();
        }
    if (!this->m_destroyed)
        {
             
        RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
        if (diffsetPtr.IsValid())
        {
            uint64_t myTs = LastClippingStateUpdateTimestamp();
            if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
            {
                uint64_t childTs = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->LastClippingStateUpdateTimestamp();
                if (childTs < myTs)
                {
                    bset<uint64_t> collectedClips;
                    CollectClipIds(collectedClips);
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->SyncWithClipSets(collectedClips);
                }
            }
            else if (this->m_nodeHeader.m_numberOfSubNodesOnSplit > 1 && this->m_apSubNodes[0] != nullptr)
            {
                for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                    if (this->m_apSubNodes[indexNodes] != nullptr)
                    {
                        uint64_t childTs = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->LastClippingStateUpdateTimestamp();
                        if (childTs < myTs)
                        {
                            bset<uint64_t> collectedClips;
                            CollectClipIds(collectedClips);
                            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->SyncWithClipSets(collectedClips);
                        }
                    }
                }
            }
        }

        RemoveNonDisplayPoolData();
       
        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t) this->m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

            {
            std::lock_guard<std::mutex> lock(m_displayMeshLock);
            GetMemoryPool()->RemoveItem(m_displayMeshPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t) this->m_SMIndex);
            m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

            SMMemoryPool::GetInstanceVideo()->RemoveItem(m_displayMeshVideoPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t) this->m_SMIndex);
            m_displayMeshVideoPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }

        if (this->m_texturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
            {
            GetMemoryPool()->RemoveItem(this->m_texturePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::DisplayTexture, (uint64_t) this->m_SMIndex);
            this->m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }




        GetMemoryPool()->RemoveItem(m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t) this->m_SMIndex);
        m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_featurePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t) this->m_SMIndex);
        m_featurePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        GetMemoryPool()->RemoveItem(m_dtmPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t) this->m_SMIndex);
        m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

        RemoveMultiTextureData();
        }

    __super::Discard();

    THIS_HINVARIANTS;

    return returnValue;
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Load() const
    {
    std::lock_guard<std::mutex> lock(m_headerMutex);
    if (this->IsLoaded()) return;
    SMPointIndexNode<POINT, EXTENT>::Load();
        
    //assert(m_triIndicesPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    //assert(this->m_texturePoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    //assert(m_triUvIndicesPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    //assert(m_uvCoordsPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    //assert(m_displayDataPoolItemId == SMMemoryPool::s_UndefinedPoolItemId);
    }

extern std::mutex s_createdNodeMutex;

template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Publish3DTile(ISMDataStoreTypePtr<EXTENT>& pi_pDataStore, TransformCR transform, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const GeoCoordinates::BaseGCSCPtr sourceGCS, const GeoCoordinates::BaseGCSCPtr destinationGCS, IScalableMeshProgressPtr progress, bool outputTexture)
    {
    assert(pi_pDataStore != nullptr);

    static double startTime = clock();
    static std::atomic<uint64_t> loadDataTime = {0};
    static std::atomic<uint64_t> convertTime = {0};
    static std::atomic<uint64_t> storeTime = {0};
    static std::atomic<uint64_t> nbProcessedNodes = {0};
    static std::atomic<uint64_t> nbNodes = {0};

    if (progress != nullptr && progress->IsCanceled()) return false;

    if (!this->IsLoaded())
        Load();

    if (this->m_nodeHeader.m_level == 0)
        {
        startTime = clock();
        loadDataTime = 0;
        convertTime = 0;
        storeTime = 0;
        nbProcessedNodes = 0;
        nbNodes = 0;
        }

    ++nbNodes;

    static const uint64_t nbThreads = std::max((uint64_t)1, (uint64_t)(std::thread::hardware_concurrency() - 2));
    static const uint64_t maxQueueSize = /*std::max((uint64_t)this->m_SMIndex->m_totalNumNodes, (uint64_t)*/30000;//);

    typedef SMNodeDistributor<HFCPtr<SMMeshIndexNode<POINT, EXTENT>>> Distribution_Type;
    static typename Distribution_Type::Ptr distributor = nullptr; 

    if (this->m_nodeHeader.m_level == 0)
        {
        startTime = clock();
        bvector<DRange3d> ranges;
        bool allClipsAreMasks = true;

        if (clips.IsValid())
            {
            for (ClipPrimitivePtr const& primitive : *clips)
                {
                DRange3d        thisRange;
                if (primitive->GetRange(thisRange, nullptr, primitive->IsMask()))
                    {
                    ranges.push_back(thisRange);
                    }
                if (!primitive->IsMask())
                    allClipsAreMasks = false;
                }
            }

        //(*clips)[0]->GetRange(range, nullptr, true);
        auto nodeDataSaver = [pi_pDataStore, ranges, allClipsAreMasks, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture](HFCPtr<SMMeshIndexNode<POINT, EXTENT>>& node)
            {
            if (progress != nullptr  && progress->IsCanceled()) return;

            bool hasMSClips = false;
            for (auto const& range : ranges)
                {
                if (node->m_nodeHeader.m_contentExtentDefined && range.IntersectsWith(node->m_nodeHeader.m_contentExtent))
                    hasMSClips = true;
                }

            if (hasMSClips || allClipsAreMasks)
                {

                // Gather all data in one place
                double t = clock();
                node->GetPointsPtr();
                node->GetPtsIndicePtr();

                if (outputTexture)
                    {
                    node->GetUVCoordsPtr();
                    node->GetUVsIndicesPtr();
                    node->GetTextureCompressedPtr();
                    }

                loadDataTime += clock() - t;

                // Convert data
                t = clock();

                auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(node.GetPtr()));
                IScalableMeshNodePtr nodeP(new ScalableMeshNode<POINT>(nodePtr));
                bvector<Byte> cesiumData;
#if defined(NEED_SAVE_AS_IN_IMPORT_DLL) && !defined(DGNDB06_API)
                IScalableMeshPublisherPtr cesiumPublisher = IScalableMeshPublisher::Create(SMPublishType::CESIUM);
                cesiumPublisher->Publish(nodeP, (hasMSClips ? clips : nullptr), coverageID, isClipBoundary, sourceGCS, destinationGCS, cesiumData, outputTexture);
#else
                assert(!"Not yet implemented on this code base");
#endif


                convertTime += clock() - t;

                // Store data
                t = clock();
                ISMTileMeshDataStorePtr tileStore;
                bool result = pi_pDataStore->GetNodeDataStore(tileStore, &node->m_nodeHeader);
                assert(result == true); // problem getting the tile mesh data store

                if (!cesiumData.empty())
                    tileStore->StoreBlock(&cesiumData, cesiumData.size(), node->GetBlockID());
                storeTime += clock() - t;
                //// Store header
                //pi_pDataStore->StoreNodeHeader(&node->m_nodeHeader, node->GetBlockID());

                //{
                //std::lock_guard<mutex> clk(s_consoleMutex);
                //std::wcout << "[" << std::this_thread::get_id() << "] Done publishing --> " << node->m_nodeId << "    addr: "<< node <<"   ref count: " << node->GetRefCount() << std::endl;
                //}
                }

            node = nullptr;

            if (progress != nullptr)
                {
                // Report progress
                static_cast<ScalableMeshProgress*>(progress.get())->SetCurrentIteration(++nbProcessedNodes);
                }
            else
                {
                ++nbProcessedNodes;
                }
            };
        distributor = new Distribution_Type(nodeDataSaver, [](HFCPtr<SMMeshIndexNode<POINT, EXTENT>>& node) {return true; }, nbThreads, maxQueueSize);
        }

    static auto loadChildExtentHelper = [](SMPointIndexNode<POINT, EXTENT>* parent, SMPointIndexNode<POINT, EXTENT>* child) ->void
        {
        // parent header needs child extent for the Cesium format
        if (child->m_nodeHeader.m_nodeCount > 0 && child->GetBlockID().IsValid())
            {
            auto childExtent = child->m_nodeHeader.m_contentExtentDefined && !child->m_nodeHeader.m_contentExtent.IsNull() ? child->GetContentExtent() : child->GetNodeExtent();
            parent->m_nodeHeader.m_childrenExtents[child->GetBlockID().m_integerID] = childExtent;
            }
        };

    //static auto disconnectChildHelper = [](SMPointIndexNode<POINT, EXTENT>* child) -> void
    //    {
    //    child->SetParentNodePtr(0);
    //
    //    s_createdNodeMutex.lock();
    //
    //    CreatedNodeMap::iterator nodeIter(child->m_createdNodeMap->find(child->GetBlockID().m_integerID));
    //
    //    if (nodeIter != child->m_createdNodeMap->end())
    //        {
    //        child->m_createdNodeMap->erase(nodeIter);
    //        }
    //
    //    s_createdNodeMutex.unlock();
    //    child = NULL;
    //    };

    if (this->m_pSubNodeNoSplit != nullptr)
        {
        assert(this->GetNumberOfSubNodesOnSplit() == 1 || this->GetNumberOfSubNodesOnSplit() == 4);
        if (!static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->Publish3DTile(pi_pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture)) return false;
        loadChildExtentHelper(this, this->m_pSubNodeNoSplit.GetPtr());
        //disconnectChildHelper(this->m_pSubNodeNoSplit.GetPtr());
        //this->m_pSubNodeNoSplit = nullptr;
        }
    else
        {
        assert(this->GetNumberOfSubNodesOnSplit() > 1 || this->IsLeaf());
        if (!this->IsLeaf())
            {
            for (size_t indexNode = 0; indexNode < this->GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                assert(this->m_apSubNodes[indexNode] != nullptr); // A sub node will be skipped
                if (this->m_apSubNodes[indexNode] != nullptr)
                    {
                    if (!static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->Publish3DTile(pi_pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, progress, outputTexture)) return false;
                    loadChildExtentHelper(this, this->m_apSubNodes[indexNode].GetPtr());
                    //disconnectChildHelper(this->m_apSubNodes[indexNode].GetPtr());
                    //this->m_apSubNodes[indexNode] = nullptr;
                    }
                }
            }
        }

    if (this->m_nodeHeader.m_nodeCount > 0)
        {
        distributor->AddWorkItem(this/*, false*/);
        }

    if (this->m_nodeHeader.m_level == 0)
        {
        //distributor->Go();
#ifdef PRINT_PUBLISH_3DTILES_INFO
        std::cout << "\nTime to process tree: " << (clock() - startTime) / CLOCKS_PER_SEC << std::endl;
#endif
        while (progress != nullptr && !distributor->empty())
            {
            progress->UpdateListeners();
            if (progress->IsCanceled()) break;
            }
        distributor = nullptr; // join queue threads
#ifdef PRINT_PUBLISH_3DTILES_INFO
        std::cout << "\nTime to load data: " << (double)loadDataTime / CLOCKS_PER_SEC / nbThreads << std::endl;
        std::cout << "Time to convert data: " << (double)convertTime / CLOCKS_PER_SEC / nbThreads << std::endl;
        std::cout << "Time to store data: " << (double)storeTime / CLOCKS_PER_SEC / nbThreads << std::endl;
        auto tTime = (double)(clock() - startTime) / CLOCKS_PER_SEC;
        std::cout << "Total time: " << tTime << std::endl;
        std::cout << "Number processed nodes: " << nbProcessedNodes << std::endl;
        std::cout << "Total number of nodes: " << nbNodes << std::endl;
        std::cout << "Convert speed (nodes/sec): " << nbProcessedNodes / tTime << std::endl;
#endif
        }
    return true;
    }

    template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ChangeGeometricError(ISMDataStoreTypePtr<EXTENT>&    pi_pDataStore, const double& newGeometricErrorValue)
        {
        assert(pi_pDataStore != nullptr);

        if (!this->IsLoaded())
            Load();

        this->m_nodeHeader.m_geometryResolution = newGeometricErrorValue;
#ifndef VANCOUVER_API
        static const uint64_t nbThreads = std::max((uint64_t)1, (uint64_t)(std::thread::hardware_concurrency() - 2));
        static const uint64_t maxQueueSize = /*std::max((uint64_t)this->m_SMIndex->m_totalNumNodes, (uint64_t)*/30000;//);
        typedef SMNodeDistributor<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> Distribution_Type;
        static Distribution_Type* distributor = new Distribution_Type([&pi_pDataStore](HFCPtr<SMPointIndexNode<POINT, EXTENT>> node)
            {
            auto loadChildExtentHelper = [](HFCPtr<SMPointIndexNode<POINT, EXTENT>> parent, HFCPtr<SMPointIndexNode<POINT, EXTENT>> child) ->void
                {
                if (!child->IsLoaded())
                    child->Load();

                // parent header needs child extent for the Cesium format
                if (child->m_nodeHeader.m_nodeCount > 0 && child->GetBlockID().IsValid())
                    {
                    auto childExtent = child->m_nodeHeader.m_contentExtentDefined && !child->m_nodeHeader.m_contentExtent.IsNull() ? child->GetContentExtent() : child->GetNodeExtent();
                    parent->m_nodeHeader.m_childrenExtents[child->GetBlockID().m_integerID] = childExtent;
                    }
                };

            if (node->m_pSubNodeNoSplit != nullptr)
                {
                loadChildExtentHelper(node, node->m_pSubNodeNoSplit);
                }
            else
                {
                for (size_t indexNode = 0; indexNode < node->GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    if (node->m_apSubNodes[indexNode] != nullptr)
                        {
                        loadChildExtentHelper(node, node->m_apSubNodes[indexNode]);
                        }
                    }
                }

            // Store header
            pi_pDataStore->StoreNodeHeader(&node->m_nodeHeader, node->GetBlockID());

            //node->Unload();
            }, [](HFCPtr<SMPointIndexNode<POINT, EXTENT>> node) {return true; }, nbThreads, maxQueueSize);

        distributor->AddWorkItem(this);
#else
        assert(false && "Make this compile on Vancouver!");
#endif

        if (this->m_pSubNodeNoSplit != nullptr)
            {
            static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->ChangeGeometricError(pi_pDataStore, newGeometricErrorValue * 0.5);
            //m_pSubNodeNoSplit->Unload();        
            }
        else
            {
            for (size_t indexNode = 0; indexNode < this->GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (this->m_apSubNodes[indexNode] != nullptr)
                    {
                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->ChangeGeometricError(pi_pDataStore, newGeometricErrorValue * 0.5);
                    //m_apSubNodes[indexNode]->Unload();
                    }
                }
            }

        if (this->m_nodeHeader.m_level == 0)
            {
#ifndef VANCOUVER_API
            delete distributor;
#else
#endif
            }
        }
    
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadIndexNodes(size_t& nLoaded, int level, bool headersOnly)
    {

    static std::atomic<uint64_t> loadHeaderTime = {0};
    static std::atomic<uint64_t> loadDataTime = {0};
    double t = 0;
    if (this->m_nodeHeader.m_level == 0)
        {
        t = clock();
        }

    if (!this->IsLoaded())
        {
        uint64_t loadTime = clock();
        Load();
        loadHeaderTime += clock() - loadTime;
        }

    nLoaded++;



    //auto loadNodeHelper = [&loadHeaderTime](SMPointIndexNode<POINT, EXTENT>* node, size_t threadId) ->void
    //    {
    //    //node->LoadTreeNode(nLoaded, level, headersOnly);
    //    if (!node->IsLoaded())
    //        {
    //        uint64_t loadTime = clock();
    //        node->Load();
    //        loadHeaderTime += clock() - loadTime;
    //        }
    //    SetThreadAvailableAsync(threadId);
    //    };


    typedef SMNodeDistributor<HFCPtr<SMMeshIndexNode<POINT, EXTENT>>> Distribution_Type;
    static typename Distribution_Type::Ptr distributor(new Distribution_Type([](HFCPtr<SMMeshIndexNode<POINT, EXTENT>>& node)
        {
            uint64_t loadTime = clock();
            node->GetPointsPtr();
            node->GetPtsIndicePtr();
            if (node->m_nodeHeader.m_isTextured)
                {
                node->GetUVCoordsPtr();
                node->GetUVsIndicesPtr();
                node->GetTexturePtr();
                }
            loadDataTime += clock() - loadTime;
        }, std::thread::hardware_concurrency(), 5000));

    if (!this->m_nodeHeader.m_IsLeaf)
        {
        if (this->m_pSubNodeNoSplit != NULL)
            {
            //s_distributor->AddWorkItem(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit)));
            //RunOnNextAvailableThread(std::bind(loadNodeHelper, this->m_pSubNodeNoSplit, std::placeholders::_1));
            this->m_pSubNodeNoSplit->LoadIndexNodes(nLoaded, level, headersOnly);
            }
        else
            {
            //for (size_t indexNodes = 0; indexNodes < this->GetNumberOfSubNodesOnSplit(); indexNodes++)
            //    {
            //    //s_distributor->AddWorkItem(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNodes])));
            //    RunOnNextAvailableThread(std::bind(loadNodeHelper, this->m_apSubNodes[indexNodes], std::placeholders::_1));
            //    }
            for (size_t indexNodes = 0; indexNodes < this->GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                this->m_apSubNodes[indexNodes]->LoadIndexNodes(nLoaded, level, headersOnly);
                }
            }
        }

    if (headersOnly || (level != 0 && this->GetLevel() + 1 > level)) return;

    if (this->GetNbPoints() > 0)
        {
        distributor->AddWorkItem(this/*, false*/);
        }

    if (this->m_nodeHeader.m_level == 0)
        {
        std::cout << "Time to process tree: " << (clock() - t) / CLOCKS_PER_SEC << std::endl;
        //WaitForThreadStop();
        distributor = nullptr;
        std::cout << "Time to load headers: " << (double)loadHeaderTime / CLOCKS_PER_SEC / LightThreadPool::GetInstance()->m_nbThreads << std::endl;
        std::cout << "Time to load data: " << (double)loadDataTime / CLOCKS_PER_SEC / LightThreadPool::GetInstance()->m_nbThreads << std::endl;
        std::cout << "Total time: " << (double)(clock() - t) / CLOCKS_PER_SEC << std::endl;
        }


    }

#ifdef INDEX_DUMPING_ACTIVATED
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                             bool pi_OnlyLoadedNode) const
    {
    if ((pi_OnlyLoadedNode == true) && (this->IsLoaded() == false))
        return;

    if (!this->IsLoaded())
        Load();

    char   TempBuffer[3000];
    int    NbChars;
    size_t NbWrittenChars;
    int64_t nodeId;

    if (this->GetBlockID().IsValid())
        {
        nodeId = this->GetBlockID().m_integerID;
        }
    else
        {
        nodeId = ISMStore::GetNullNodeID();
        }

    NbChars = sprintf(TempBuffer, "<ChildNode NodeId=\"%lli\" TotalPoints=\"%lli\" SplitDepth=\"%zi\" ArePoints3d=\"%i\">", nodeId, this->GetCount(), this->GetSplitDepth(), this->m_nodeHeader.m_arePoints3d ? 1 : 0);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    //Extent
    NbChars = sprintf(TempBuffer,
                      "<NodeExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></NodeExtent>\n",
                      ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (this->m_nodeHeader.m_contentExtentDefined)
        {
        NbChars = sprintf(TempBuffer,
                          "<ContentExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></ContentExtent>\n",
                          ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }


    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfPoints>%u</NbOfPoints>\n", this->GetNbObjects());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Cumulative Number Of Points
    NbChars = sprintf(TempBuffer, "<CumulNbOfPoints>%llu</CumulNbOfPoints>\n", this->GetCount());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfIndexes>%zu</NbOfIndexes>\n", this->m_nodeHeader.m_nbFaceIndexes);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Level
    NbChars = sprintf(TempBuffer, "<Level>%zi</Level>\n", this->m_nodeHeader.m_level);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // SplitTreshold
    NbChars = sprintf(TempBuffer, "<SplitTreshold>%zi</SplitTreshold>", this->m_nodeHeader.m_SplitTreshold);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // Balanced
    if (this->m_nodeHeader.m_balanced)
        NbChars = sprintf(TempBuffer, "<Balanced>true</Balanced>");
    else
        NbChars = sprintf(TempBuffer, "<Balanced>false</Balanced>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    //View Dependent Metrics
    /*
    NbChars = sprintf(TempBuffer,
    "<ViewDependentMetrics>%.3f</ViewDependentMetrics>",
    this->m_nodeHeader.m_ViewDependentMetrics[0]);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);
    */


    // Neighbor Node    
    NbChars = sprintf(TempBuffer, "<NeighborNode> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        for (size_t neighborInd = 0; neighborInd < this->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
            {
            NbChars = sprintf(TempBuffer, "P %zi I %zi Id %lli ", neighborPosInd, neighborInd, this->m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd].m_integerID);

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        }

    NbChars = sprintf(TempBuffer, "</NeighborNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //GraphID
    NbChars = sprintf(TempBuffer, "<GraphID>%llu</GraphID>\n", this->m_nodeHeader.m_graphID.m_integerID);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t i = 0; i < this->m_nodeHeader.m_ptsIndiceID.size(); ++i)
        {
        NbChars = sprintf(TempBuffer, "<IndiceID>%llu</IndiceID>\n", this->m_nodeHeader.m_ptsIndiceID[i].m_integerID);

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }

    
    // Neighbor Stitching    
    NbChars = sprintf(TempBuffer, "<NeighborStitching> ");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        if (this->m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] == true)
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


    if (!this->m_nodeHeader.m_IsLeaf)
        {
        if (this->m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
            }
        else
            {
            for (size_t indexNode = 0; indexNode < this->GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
                }
            }
        }

    NbChars = sprintf(TempBuffer, "</ChildNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

        
    }

#endif


//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::RemoveWithin(ClipVectorCP boundariesToRemoveWithin)
    {
    DRange3d range;
    boundariesToRemoveWithin->GetRange(range, nullptr);
    if (this->m_nodeHeader.m_contentExtentDefined && !range.IntersectsWith(this->m_nodeHeader.m_contentExtent)) return;
    if (this->m_nodeHeader.m_nodeCount < 3) return;
    /*
    bset<int32_t> removedPts;*/
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndicePtr = GetPtsIndicePtr();
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr = this->GetPointsPtr();
    /*
    for (size_t i = 0; i < ptsPtr->size(); ++i)
        {
        if (boundariesToRemoveWithin->PointInside((*ptsPtr)[i], 1e-8))
            removedPts.insert((int)i);
        }*/
    bvector<DPoint3d> clearedPts;
    bvector<int32_t> clearedIndices;
    bmap<int32_t, int32_t> oldToNewIndices;

    bvector<int32_t> newUvsIndices;
    bvector<DPoint2d> newUvs;
   /* RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvsIndicePtr = GetUVsIndicesPtr();

    for (size_t i = 0; i < ptsIndicePtr->size(); i += 3)
        {
        if (removedPts.count((*ptsIndicePtr)[i]-1) == 0 && removedPts.count((*ptsIndicePtr)[i + 1]-1) == 0 && removedPts.count((*ptsIndicePtr)[i + 2]-1) == 0)
            {
            for (size_t j = 0; j < 3; ++j)
                {
                if (oldToNewIndices.count((*ptsIndicePtr)[i + j]) == 0)
                    {
                    oldToNewIndices[(*ptsIndicePtr)[i + j]] = (int)clearedPts.size()+1;
                    clearedPts.push_back((*ptsPtr)[(*ptsIndicePtr)[i + j] - 1]);
                    }
                clearedIndices.push_back(oldToNewIndices[(*ptsIndicePtr)[i + j]]);

                if (uvsIndicePtr.IsValid() && uvsIndicePtr->size() > 0)
                    newUvsIndices.push_back((*uvsIndicePtr)[i + j]);
                }
            }
        }*/

    bvector<bvector<PolyfaceHeaderPtr>> polyfaces;
    auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
        new ScalableMeshNode<POINT>(nodePtr)
#else
        ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
        );
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
    flags->SetLoadTexture(true);
    IScalableMeshMeshPtr meshP = nodeP->GetMesh(flags);
    bvector<bool> isMask;
    bvector<size_t> polyfaceIndices;
    if (meshP.get() != nullptr)
        GetRegionsFromClipVector3D(polyfaces, polyfaceIndices, boundariesToRemoveWithin, meshP->GetPolyfaceQuery(), isMask);

    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    if (polyfaces[0][0]->GetPointCount() > 0)
        {
        DifferenceSet clipped = DifferenceSet::FromPolyfaceSet(polyfaces[0], mapOfPoints, 1);
        clearedPts = clipped.addedVertices;
        clearedIndices = clipped.addedFaces;
        newUvsIndices = clipped.addedUvIndices;
        newUvs = clipped.addedUvs;
        }
    ptsPtr->clear();
    ptsIndicePtr->clear();
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvsIndicePtr = GetUVsIndicesPtr();
    if (uvsIndicePtr.IsValid())
        uvsIndicePtr->clear();
    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvsPts = GetUVCoordsPtr();
    if (uvsPts.IsValid())
        uvsPts->clear();
    if (!clearedPts.empty())
        {
        ptsPtr->push_back(&clearedPts[0], clearedPts.size());
        if (!clearedIndices.empty())
            ptsIndicePtr->push_back(&clearedIndices[0], clearedIndices.size());
        if (!newUvsIndices.empty())
            uvsIndicePtr->push_back(&newUvsIndices[0], newUvsIndices.size());
        if (!newUvs.empty())
            uvsPts->push_back(&newUvs[0], newUvs.size());
        }

    //mark data not up to date
    this->SetDirty(true);
    }


//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::UpdateData()
    {
    if (!m_remainingUnappliedEdits.empty())
        {
        for (auto& edit : m_remainingUnappliedEdits)
            {
            if (edit->m_opType == EditOperation::Op::REMOVE)
                RemoveWithin(edit->m_toRemoveVector.get());
            }
        m_remainingUnappliedEdits.clear();
        }

    this->GetDataStore()->StoreNodeHeader(&this->m_nodeHeader, this->GetBlockID()); //we need to do that now since we set it not dirty afterward
    //mark data up to date
    this->SetDirty(false);
    }


//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 08/16
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::AddEdit(RefCountedPtr<EditOperation>& editDef)
    {
    m_remainingUnappliedEdits.push_back(editDef);
    this->SetDirty(true);
    }

//=======================================================================================
// @bsimethod                                                  Mathieu.St-Pierre 08/18
//=======================================================================================
template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsExistingMesh() const
    {
    return m_existingMesh;
    }

//=======================================================================================
// @bsimethod                                                  Elenie.Godzaridis 03/15
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Unload() 
    {
    if (this->IsLoaded() && this->m_nodeHeader.m_isTextured) // we must do this before Unload since the texID is stored in the header.
    {
        auto tex = GetSingleDisplayTexture();
        if (tex.IsValid()) const_cast<SmCachedDisplayTextureData*>(tex->GetData())->RemoveConsumer(this);
    }

    SMPointIndexNode<POINT, EXTENT>::Unload();
    }


template <class POINT, class EXTENT> ISMMTGGraphDataStorePtr SMMeshIndexNode<POINT, EXTENT>::GetGraphStore() const
    {
    ISMMTGGraphDataStorePtr nodeDataStore;
    bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader);
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
    this->m_nodeHeader.m_uvID = this->GetBlockID();
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushTexture(const Byte* texture, size_t size)
    {
    assert(!GetTexturePtr().IsValid());

    ISMTextureDataStorePtr nodeDataStore;
    bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader);
    assert(result == true);  

    RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(
#ifndef VANCOUVER_API
        new SMStoredMemoryPoolBlobItem<Byte>(this->GetBlockID().m_integerID, nodeDataStore, texture, size, SMStoreDataType::Texture, (uint64_t)this->m_SMIndex)
#else
        SMStoredMemoryPoolBlobItem<Byte>::CreateItem(this->GetBlockID().m_integerID, nodeDataStore, texture, size, SMStoreDataType::Texture, (uint64_t)this->m_SMIndex)
#endif
        );
    SMMemoryPoolItemBasePtr poolItem(storedMemoryPoolVector.get());
    this->m_texturePoolItemId = GetMemoryPool()->AddItem(poolItem);
    assert(this->m_texturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId);  
    this->m_nodeHeader.m_isTextured = true;
    this->m_nodeHeader.m_textureID = this->GetBlockID();
    this->m_nodeHeader.m_nbTextures = 1;
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::PushUVsIndices(size_t texture_id, const int32_t* uvsIndices, size_t size)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvPtsIndicePtr = GetUVsIndicesPtr();    
    bool result = uvPtsIndicePtr->push_back(uvsIndices, size);
    assert(result);
    //assert(this->m_nodeHeader.m_uvsIndicesID.size() == 0);
    this->m_nodeHeader.m_uvsIndicesID.push_back(this->GetBlockID());
    }
   
//=======================================================================================
// @bsimethod                                                 Elenie.Godzaridis 09/17
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::RemoveNonDisplayPoolData()
{
    SMPointIndexNode<POINT, EXTENT>::RemoveNonDisplayPoolData();
    GetMemoryPool()->RemoveItem(m_triIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::TriPtIndices, (uint64_t)this->m_SMIndex);
    GetMemoryPool()->RemoveItem(m_triIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Cesium3DTiles, (uint64_t)this->m_SMIndex);
    m_triIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    GetMemoryPool()->RemoveItem(this->m_texturePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Texture, (uint64_t)this->m_SMIndex);
    GetMemoryPool()->RemoveItem(this->m_texturePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Cesium3DTiles, (uint64_t)this->m_SMIndex);
    this->m_texturePoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    GetMemoryPool()->RemoveItem(m_triUvIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::TriUvIndices, (uint64_t)this->m_SMIndex);
    GetMemoryPool()->RemoveItem(m_triUvIndicesPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Cesium3DTiles, (uint64_t)this->m_SMIndex);
    m_triUvIndicesPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    GetMemoryPool()->RemoveItem(m_uvCoordsPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::UvCoords, (uint64_t)this->m_SMIndex);
    GetMemoryPool()->RemoveItem(m_uvCoordsPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Cesium3DTiles, (uint64_t)this->m_SMIndex);
    m_uvCoordsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;

    GetMemoryPool()->RemoveItem(m_graphPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex);
    GetMemoryPool()->RemoveItem(m_graphPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Cesium3DTiles, (uint64_t)this->m_SMIndex);
    m_graphPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
}

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::RemoveMultiTextureData() 
    {

    for (auto textureId : m_textureIds)
        {                    
        SMMemoryPoolItemId displayTexturePoolItemId = ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->GetPoolIdForTexture(textureId);

        if (displayTexturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
            {
            ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->RemovePoolIdForTexture(textureId);
            GetMemoryPool()->RemoveItem(displayTexturePoolItemId, textureId, SMStoreDataType::DisplayTexture, (uint64_t)this->m_SMIndex);
            }
               
        displayTexturePoolItemId = ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->GetPoolIdForTextureData(textureId);

        if (displayTexturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
            {
            ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->RemovePoolIdForTextureData(textureId);
            GetMemoryPool()->RemoveItem(displayTexturePoolItemId, textureId, SMStoreDataType::Texture, (uint64_t)this->m_SMIndex);            
            }                                                
        }

    m_textureIds.clear();

    for (auto textureVideoId : m_textureVideoIds)
        {
        SMMemoryPoolItemId displayTextureVideoPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->GetPoolIdForTextureVideo(textureVideoId);

        if (displayTextureVideoPoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
            {
            ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->RemovePoolIdForTextureVideo(textureVideoId);
            SMMemoryPool::GetInstanceVideo()->RemoveItem(displayTextureVideoPoolItemId, textureVideoId, SMStoreDataType::DisplayTexture, (uint64_t)this->m_SMIndex);
            }
        }

    m_textureVideoIds.clear();        
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> ISMPointIndexMesher<POINT, EXTENT>* SMMeshIndexNode<POINT, EXTENT>::GetMesher2_5d() const
    {
    if (!this->IsLoaded())
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
    if (!this->IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_mesher3d);
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Mesh(bvector<uint64_t>* pNodesToMesh)
    {
    if (!this->IsLoaded())
        Load();


    THIS_HINVARIANTS;

    //deal with cancelling generation while it is still in progress
    if (this->m_SMIndex->m_progress->IsCanceled()) return;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (this->HasRealChildren())
        {
        if (this->IsParentOfARealUnsplitNode())
            {
#ifdef __HMR_DEBUG
            if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                this->m_parentOfAnUnspliteableNode = true;
#endif                        

            if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->NeedsMeshing())
                static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->Mesh(pNodesToMesh);
            }
        else
            {
            for (size_t indexNode = 0; indexNode < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
#ifdef __HMR_DEBUG
                if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                    this->m_parentOfAnUnspliteableNode = true;
#endif
                if (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->NeedsMeshing())
                    {
                     static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->Mesh(pNodesToMesh);
                    }
                }

            }

        }
    else
        {
        assert(this->NeedsMeshing() == true);

        //Juste accumulate the node ids to mesh, don't mesh anything.
        if (pNodesToMesh != nullptr)
            {
            pNodesToMesh->push_back(this->GetBlockID().m_integerID);
            return;
            }
        

        this->m_SMIndex->m_nMeshedNodes++;
        float progressForLevel = (float)this->m_SMIndex->m_nMeshedNodes / this->m_SMIndex->m_countsOfNodesAtLevel[this->m_nodeHeader.m_level];

        if (this->m_SMIndex->m_progress != nullptr)
        {
            this->m_SMIndex->m_progress->Progress() = progressForLevel;
            this->m_SMIndex->m_progress->UpdateListeners();
        }

        //assert(this->m_nodeHeader.m_balanced == true);
        if (s_useThreadsInMeshing)
            {
            if (!this->m_SMIndex->m_progress->IsCanceled())
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
            if (this->m_nodeHeader.m_arePoints3d)
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

    if (this->m_nodeHeader.m_level == 0 && s_useThreadsInMeshing)
        WaitForThreadStop(this->m_SMIndex->m_progress.get());
    // Now filtering can be performed using the sub-nodes filtered data. This data
    // accessed using the HPMPooledVector interface the Node is a descendant of.
    // Do not hesitate to increase the HPMPooledVector interface if required.
    // The result of the filtering must be added to the Node itself in the
    // HPMVectorPool<POINT> descendant class using the push_back interface.
    // If refiltering is required then clear() must be called beforehand.
    // The member this->m_nodeHeader.m_filtered should be set to true after the filtering process
    // All members that must be serialized in the file must be added in the
    // this->m_nodeHeader fields/struct and these will automatically be serialized in the
    // store. Note that changing this structure automatically
    // renders invalid any previous file.

    this->ValidateInvariants();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch)
    {   
    if (!this->IsLoaded())
        Load();

    THIS_HINVARIANTS;

    if (this->m_SMIndex->m_progress->IsCanceled()) return;
//    size_t nodeInd;

    if (pi_levelToStitch == -1 || this->m_nodeHeader.m_level == pi_levelToStitch && this->GetNbObjects() > 0)
        {
            if (nodesToStitch != 0)
                {
                nodesToStitch->push_back(this);
                }
            else
                {

                bool isStitched;

                if (this->AreAllNeighbor2_5d() && !this->m_nodeHeader.m_arePoints3d)
                    {
                    isStitched = m_mesher2_5d->Stitch(this);
                    }
                else
                    {
                    isStitched = m_mesher3d->Stitch(this);
                    }

                this->m_SMIndex->m_nStitchedNodes++;
                float progressForStep = (float)(this->m_SMIndex->m_nStitchedNodes) / this->m_SMIndex->m_countsOfNodesTotal * 2 / 3 + (float)(this->m_SMIndex->m_nFilteredNodes) / this->m_SMIndex->m_countsOfNodesTotal * 1 / 3;

                if (this->m_SMIndex->m_progress != nullptr) this->m_SMIndex->m_progress->Progress() = progressForStep;
                if (isStitched)
                    this->SetDirty(true);

                }
            }
       // }

        if (pi_levelToStitch == -1 || (int)this->m_nodeHeader.m_level < pi_levelToStitch)
            {
            if (!this->m_nodeHeader.m_IsLeaf)
                {
                if (this->m_pSubNodeNoSplit != NULL)
                    {
#ifdef __HMR_DEBUG
                    if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->m_parentOfAnUnspliteableNode))
                        this->m_parentOfAnUnspliteableNode = true;
#endif

                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->Stitch(pi_levelToStitch, nodesToStitch);

                    }
                else
                    {
                    for (size_t indexNode = 0; indexNode < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
#ifdef __HMR_DEBUG
                        if ((static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->m_unspliteable) || (static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->m_parentOfAnUnspliteableNode))
                            this->m_parentOfAnUnspliteableNode = true;
#endif                
                        if (this->m_nodeHeader.m_level == 0 && nodesToStitch == 0 && pi_levelToStitch > 1 && s_useThreadsInStitching)
                            {
                            if (!this->m_SMIndex->m_progress->IsCanceled())
                                RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>* node, int pi_levelToStitch, size_t threadId) ->void
                                {
                                node->Stitch(pi_levelToStitch, 0);
                                SetThreadAvailableAsync(threadId);
                                }, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode])), pi_levelToStitch, std::placeholders::_1));
                            }
                        else static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->Stitch(pi_levelToStitch, nodesToStitch);
                        }
                    }
                }
            }
        //don't return until all threads are done
        if (this->m_nodeHeader.m_level == 0 && nodesToStitch == 0 && s_useThreadsInStitching)
            WaitForThreadStop(this->m_SMIndex->m_progress.get());
       /* if (this->m_nodeHeader.m_level == 0 && pi_levelToStitch == 0)
            {
            this->m_nodeHeader.m_totalCountDefined = false;
            }*/
        // Now filtering can be performed using the sub-nodes filtered data. This data
        // accessed using the HPMPooledVector interface the Node is a descendant of.
        // Do not hesitate to increase the HPMPooledVector interface if required.
        // The result of the filtering must be added to the Node itself in the
        // HPMVectorPool<POINT> descendant class using the push_back interface.
        // If refiltering is required then clear() must be called beforehand.
        // The member this->m_nodeHeader.m_filtered should be set to true after the filtering process
        // All members that must be serialized in the file must be added in the
        // this->m_nodeHeader fields/struct and these will automatically be serialized in the
        // store. Note that changing this structure automatically
        // renders invalid any previous file.

        this->ValidateInvariants();
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
    if (IsClosedFeature(type) || IsClosedPolygon(origPoints))
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
                if (points.size() == 0) extent = DRange3d::From(origPoints[pt]);

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
        if (faceIds.size() != 0 && faceIds[(&idx - &newIndices[0]) / 3] != currentTexId)
            {
            if (currentTexId != -1)
                {
                assert(currentTexId < 10000);
                newTexIds.push_back(currentTexId);
                newParts.push_back((int)indicesClipped.size());
                }
            currentTexId = faceIds[(&idx - &newIndices[0]) / 3];
            assert(currentTexId < 10000);
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
    assert(currentTexId < 10000);
    if (currentTexId != -1)
        newTexIds.push_back(currentTexId);

    newParts.push_back((int)indicesClipped.size());
    inOutUvs = tempUvs;
    parts = newParts;
    texIds = newTexIds;
   // SimplifyMesh(indicesClipped, pointsClipped, inOutUvs);
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::ReadFeatureDefinitions(bvector<bvector<DPoint3d>>& points, bvector<DTMFeatureType> & types, bool shouldIgnoreOpenFeatures)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    bvector<bvector<int32_t>> defs;
    if (linearFeaturesPtr->size() > 0) GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
    for (size_t i = 0; i < defs.size(); ++i)
        {
        bvector<DPoint3d> feature;
        if (shouldIgnoreOpenFeatures && !IsClosedFeature(defs[i][0])) continue;
        for (size_t j = 1; j < defs[i].size(); ++j)
            {
            if (defs[i][j] < this->GetPointsPtr()->size()) feature.push_back(this->GetPointsPtr()->operator[](defs[i][j]));
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
    //DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
     //                                   ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));
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

//#ifdef WIP_MESH_IMPORT

template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddMeshDefinitionUnconditional(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs, bool isMesh3d)
    {
    if (!this->IsLoaded())
        Load();
    if (this->m_DelayedSplitRequested)
        this->SplitNode(this->GetDefaultSplitPosition());

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
        assert(texId.back() < 10000);
        }

    for (const Json::Value& id : val["parts"])
        {
        parts.push_back(id.asInt());
        }

    //Push some default part if none has been specified.
    if (parts.size() == 0 && nIndices > 0)
        {
        parts.push_back(0);
        }

    ClipMeshDefinition(this->m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, indicesClipped, outUvs, pts, nPts, indices, nIndices, extent,texId, parts);

    val["texId"] = Json::arrayValue;
    val["parts"] = Json::arrayValue;

    for(auto& id: texId){
        val["texId"].append(Json::Value(id));
        assert(id< 10000);
        
        }
    for(auto& id: parts)
        {
        val["parts"].append(id);
        }
    Utf8String metadataStr(Json::FastWriter().write(val));
    if (!this->m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) return 0;

#ifdef WIP_MESH_IMPORT
    if (m_mesher2_5d != &s_ExistingMeshMesher || m_mesher3d != &s_ExistingMeshMesher) m_mesher2_5d = m_mesher3d = &s_ExistingMeshMesher;
#endif
    
    m_existingMesh = true;
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
    
    if (this->m_nodeHeader.m_arePoints3d) this->SetNumberOfSubNodesOnSplit(8);
    else this->SetNumberOfSubNodesOnSplit(4);
    
    if (!this->HasRealChildren() && (pointsPtr->size() + pointsClipped.size() >= this->m_nodeHeader.m_SplitTreshold) /*&& this->m_nodeHeader.m_level <= 10*/)
        {
        // There are too much objects ... need to split current node
        this->SplitNode(this->GetDefaultSplitPosition());
        }
    else if (this->m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() >= this->m_nodeHeader.m_SplitTreshold))
        {
        this->PropagateDataDownImmediately(false);
        }
    if (pointsClipped.size() == 0) return false;


    this->m_nodeHeader.m_totalCount += pointsClipped.size();
    EXTENT featureExtent = ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    if (!this->m_nodeHeader.m_contentExtentDefined)
        {
        this->m_nodeHeader.m_contentExtent = featureExtent;
        this->m_nodeHeader.m_contentExtentDefined = true;
        }
    else
        {
        this->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(this->m_nodeHeader.m_contentExtent, featureExtent);
        }

    size_t added = 0;

    if (!this->HasRealChildren() || (this->m_delayedDataPropagation && ((pointsPtr->size() + pointsClipped.size() < this->m_nodeHeader.m_SplitTreshold)) /*||  this->m_nodeHeader.m_level > 10*/))
        {
        vector<int32_t> indexes;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));

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

                if (texId.size() > currentPart)
                    {
                    int newId = texId[currentPart];
                    assert(newId < 10000);
                    subMetadata["texId"].append(Json::Value(texId[currentPart]));
                    }

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

        if (texId.size() > currentPart)
            {
            val["texId"].append(Json::Value(texId[currentPart]));
            }

        metadataStr = Json::FastWriter().write(val);
        m_meshMetadata.push_back(metadataStr);

        assert(uvs == nullptr);

#ifdef WIP_MESH_IMPORT       
        if (uvs != nullptr)
            {
            this->m_nodeHeader.m_isTextured = true;
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
#endif
        }
    else
        {
        if (this->IsParentOfARealUnsplitNode())
            added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->AddMeshDefinitionUnconditional(&pointsClipped[0], pointsClipped.size(), &indicesClipped[0], indicesClipped.size(), extentClipped, metadataStr.c_str(), texData,texSize,outUvs.size() > 0 ? outUvs.data() : 0, isMesh3d);
        else
            {
            for (size_t indexNode = 0; indexNode < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNode])->AddMeshDefinition(&pointsClipped[0], pointsClipped.size(), &indicesClipped[0], indicesClipped.size(), extentClipped, true, metadataStr.c_str(), texData,texSize,outUvs.size() > 0 ? outUvs.data() : 0, isMesh3d);
                }
            }
        }

    this->SetDirty(true);
    return added;
    }

template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, bool ExtentFixed, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs, bool isMesh3d)
    {
    if (s_inEditing)
        {
        InvalidateFilteringMeshing();
        }

    this->m_nodeHeader.m_arePoints3d |= isMesh3d;
    
    if (this->m_DelayedSplitRequested)
        this->SplitNode(this->GetDefaultSplitPosition());

    if (!ExtentFixed && this->GetParentNode() == NULL && this->m_nodeHeader.m_IsLeaf)
        {
        this->m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(this->GetNodeExtent(), ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));

        if (ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) &&
            ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent)));
            }
        else
            if (ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) &&
                ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetYMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent)));
                ExtentOp<EXTENT>::SetZMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent)));
                }
            else
                if (ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) &&
                    ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent))
                    {
                    ExtentOp<EXTENT>::SetXMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent)));
                    ExtentOp<EXTENT>::SetYMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent)));
                    }

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

        if (nPts + pointsPtr->size() >= this->m_nodeHeader.m_SplitTreshold)
            {
            return AddMeshDefinition(pts, nPts, indices, nIndices, extent, true, metadata, texData, texSize, uvs, isMesh3d);
            }
        else
            {
            return AddMeshDefinitionUnconditional(pts, nPts, indices, nIndices, extent, metadata, texData, texSize, uvs, isMesh3d);
            }
        }
    else
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));
        if (extent.IntersectsWith(nodeRange))
            {
            return AddMeshDefinitionUnconditional(pts, nPts, indices, nIndices, extent, metadata, texData, texSize, uvs, isMesh3d);
            }
        }
    return 0;
    }

//#endif

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 10/14
//=======================================================================================
template<class POINT, class EXTENT>
bool SMMeshIndexNode<POINT, EXTENT>::InvalidateFilteringMeshing(bool becauseDataRemoved)
    {

    // In theory the two next variable invalidations are not required as
    // but just in case the HGFIndexNode            
    if (this->m_nodeHeader.m_filtered)
        {

            //Remove the sub-resolution data.
            //            setNbPointsUsedForMeshIndex(0);
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  indicesPtr = GetPtsIndicePtr();

            indicesPtr->clear();
            this->m_nodeHeader.m_nbFaceIndexes = 0;

            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvPtr = GetUVCoordsPtr();
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIPtr = GetUVsIndicesPtr();
            if (uvPtr.IsValid())
                uvPtr->clear();
            if (uvIPtr.IsValid())
                uvIPtr->clear();


        this->m_nodeHeader.m_filtered = false;
        }

    this->InvalidateStitching();

    if (becauseDataRemoved)
        {
        this->m_nodeHeader.m_contentExtentDefined = false;
        }

    this->m_nodeHeader.m_nbFaceIndexes = 0;
    this->m_nodeHeader.m_balanced = false;
    this->m_wasBalanced = false;
    this->SetDirty(true);


    return true;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
    template<class POINT, class EXTENT> size_t SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinitionUnconditional(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
        {
        if (!this->IsLoaded())
            Load();
        if (this->m_DelayedSplitRequested)
            this->SplitNode(this->GetDefaultSplitPosition());
        
        DRange3d extentClipped;
        bvector<DPoint3d> pointsClipped;
        size_t n = 0;
        bool noIntersect = true;
        DRange2d extent2d= DRange2d::From(this->m_nodeHeader.m_nodeExtent);
        for (auto&pt : points)
            {
            //Don't attempt to clip feature if it is entirely on or outside the box
            if (this->m_nodeHeader.m_nodeExtent.IsContainedXY(pt) && fabs(pt.x - this->m_nodeHeader.m_nodeExtent.low.x) > 1e-5 && 
                fabs(pt.x -  this->m_nodeHeader.m_nodeExtent.high.x) > 1e-5 && 
                fabs(pt.y - this->m_nodeHeader.m_nodeExtent.low.y) > 1e-5 &&
                fabs(pt.y - this->m_nodeHeader.m_nodeExtent.high.y) > 1e-5
                ) ++n;
            if (noIntersect && &pt - &points[0] != 0)
                {
                DRange3d edgeRange = DRange3d::From(pt, points[&pt - &points[0] - 1]);
                if (this->m_nodeHeader.m_nodeExtent.IntersectsWith(edgeRange))
                    {
                    double par1, par2;
                    DPoint2d ptIntersect1, ptIntersect2;
                    DPoint2d start = DPoint2d::From(points[&pt - &points[0] - 1]);
                    DPoint3d direction;
                    direction.DifferenceOf(pt, points[&pt - &points[0] - 1]);
                    DPoint2d dir2d = DPoint2d::From(direction);
                    if (extent2d.IntersectRay(par1, par2, ptIntersect1, ptIntersect2,start ,dir2d ) && ((par1 > 1e-5 && par1 < 1 + 1e-5) ||( par2 > 1e-5  && par2 < 1+1e-5)))
                    noIntersect = false;
                    }
                }
            }

        if (noIntersect && n < 2) return 0;
        ClipFeatureDefinition(type, this->m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, points, extent);
        if (!this->m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) return 0;

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

        if (!this->HasRealChildren() && pointsPtr->size() == 0) this->m_nodeHeader.m_arePoints3d = false;
        if (!this->m_nodeHeader.m_arePoints3d) this->SetNumberOfSubNodesOnSplit(4);        

        if (!this->HasRealChildren() && (pointsPtr->size() + pointsClipped.size() >= this->m_nodeHeader.m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            this->SplitNode(this->GetDefaultSplitPosition());
            }
        else if (this->m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() >= this->m_nodeHeader.m_SplitTreshold))
            {
            this->PropagateDataDownImmediately(false);
            }
        if (pointsClipped.size() == 0) return false;


        this->m_nodeHeader.m_totalCount += pointsClipped.size();
        EXTENT featureExtent = ExtentOp<EXTENT>::Create(extentClipped.low.x, extentClipped.low.y, extentClipped.low.z, extentClipped.high.x, extentClipped.high.y, extentClipped.high.z);
        if (!this->m_nodeHeader.m_contentExtentDefined)
            {
            this->m_nodeHeader.m_contentExtent = featureExtent;
            this->m_nodeHeader.m_contentExtentDefined = true;
            }
        else
            {
            this->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(this->m_nodeHeader.m_contentExtent, featureExtent);
            }

        size_t added = 0;
        
        if (!this->HasRealChildren() || (this->m_delayedDataPropagation && (pointsPtr->size() + pointsClipped.size() < this->m_nodeHeader.m_SplitTreshold)))
            {
            ++s_featuresAddedToTree;
            vector<int32_t> indexes;
            DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                                ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));
            
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
           */ RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
            linearFeaturesPtr->push_back((int)indexes.size()+1);
            linearFeaturesPtr->push_back((int32_t)type);
            linearFeaturesPtr->push_back(&indexes[0], indexes.size());
            }
        else
            {
            if (this->IsParentOfARealUnsplitNode())
                added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional(type, pointsClipped, extentClipped);
            else
                {
                    for (size_t indexNode = 0; indexNode < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
                        added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNode])->AddFeatureDefinition(type, pointsClipped, extentClipped, true);
                        }
                }
            }
     
        this->SetDirty(true);
        return added;
        }

    //=======================================================================================
    // @bsimethod                                                   Elenie.Godzaridis 08/15
    //=======================================================================================
    template<class POINT, class EXTENT>  size_t  SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed)
        {
        if (!this->IsLoaded())
            Load();
        assert(points.size()>0);
        if (s_inEditing)
            {
            InvalidateFilteringMeshing();
            this->m_delayedDataPropagation = false; 
            }
        if (this->m_DelayedSplitRequested)
            this->SplitNode(this->GetDefaultSplitPosition());

        if (!ExtentFixed && this->GetParentNode() == NULL && this->m_nodeHeader.m_IsLeaf)
            {
            this->m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(this->GetNodeExtent(), ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));

            if (ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) &&
                ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetXMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent)));
                ExtentOp<EXTENT>::SetZMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent)));
                }
            else
                if (ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) &&
                    ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent))
                    {
                    ExtentOp<EXTENT>::SetYMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent)));
                    ExtentOp<EXTENT>::SetZMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent)));
                    }
                else
                    if (ExtentOp<EXTENT>::GetWidth(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent) &&
                        ExtentOp<EXTENT>::GetHeight(this->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent))
                        {
                        ExtentOp<EXTENT>::SetXMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent)));
                        ExtentOp<EXTENT>::SetYMax(this->m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(this->m_nodeHeader.m_nodeExtent)));
                        }

            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

            if (points.size() + pointsPtr->size() >= this->m_nodeHeader.m_SplitTreshold)
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
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));
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

    size_t nFeatures = this->IsLeaf() ? linearFeaturesPtr->size() : 0;
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->CountAllFeatures();
        }
    else if (!this->IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            assert(this->m_apSubNodes[indexNodes] != nullptr);
            nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->CountAllFeatures();
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
    PropagateMeshToChildren();
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPropagateDataDown()
    {
    PropagateFeaturesToChildren();
//#ifdef WIP_MESH_IMPORT
    PropagateMeshToChildren();
//#endif
    }
   

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateMeshToChildren()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  indicesPtr = GetPtsIndicePtr();
    if (indicesPtr->size() == 0) return;

#ifndef WIP_MESH_IMPORT
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> subNodePtsIndicesPtr(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->GetPtsIndicePtr());
        subNodePtsIndicesPtr->reserve(indicesPtr->size());
        subNodePtsIndicesPtr->push_back(&(*indicesPtr)[0], indicesPtr->size());        
        indicesPtr->clear();

        RefCountedPtr<SMMemoryPoolBlobItem<uint8_t>>  texPtr = GetTexturePtr();

        if (texPtr.IsValid() && texPtr->GetSize() > 0)
            {            
            //RefCountedPtr<SMMemoryPoolVectorItem<uint8_t>> subNodeTexPtr(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->GetTexturePtr());
            static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->PushTexture(texPtr->GetData(), texPtr->GetSize());
            }

        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvPtr = GetUVCoordsPtr();

        if (uvPtr.IsValid() && uvPtr->size() > 0)
            {
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> subNodeUVPtr(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->GetUVCoordsPtr());
            subNodeUVPtr->reserve(uvPtr->size());
            subNodeUVPtr->push_back(&(*uvPtr)[0], uvPtr->size());
            uvPtr->clear();
            }
        
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicesPtr = GetUVsIndicesPtr();        

        if (uvIndicesPtr.IsValid() && uvIndicesPtr->size() > 0)
            {
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> subNodeUvIndicesPtr(static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->GetUVsIndicesPtr());
            subNodeUvIndicesPtr->reserve(uvIndicesPtr->size());
            subNodeUvIndicesPtr->push_back(&(*uvIndicesPtr)[0], uvIndicesPtr->size());
            uvIndicesPtr->clear();
            }

        static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_pSubNodeNoSplit))->m_existingMesh = m_existingMesh;
        }
#else //Prototype version for the design mesh handling by ScalableMesh for Hololens prototype. 

    if (indicesPtr->size() == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
    bvector<IScalableMeshMeshPtr> allMeshes;
    bvector<Utf8String> metadata;
    bvector<bvector<uint8_t>> texData;
    GetMeshParts(allMeshes, metadata, texData);
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < allMeshes.size(); ++i)
            {
            if (!allMeshes[i].IsValid() || allMeshes[i]->GetNbFaces() == 0) continue;
            DRange3d extent = DRange3d::From(allMeshes[i]->EditPoints(), (int)allMeshes[i]->GetNbPoints());
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->AddMeshDefinitionUnconditional(allMeshes[i]->EditPoints(), allMeshes[i]->GetNbPoints(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCP(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCount(), extent, metadata[i].c_str(),texData[i].size() > 0 ? texData[i].data() : 0,texData[i].size(),allMeshes[i]->GetPolyfaceQuery()->GetParamCP(), isMesh3d);
            }
        }
    else if (!this->IsLeaf())
        {
            for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                for (size_t i = 0; i < allMeshes.size(); ++i)
                    {
                    if (!allMeshes[i].IsValid() || allMeshes[i]->GetNbFaces() == 0) continue;
                    DRange3d extent = DRange3d::From(allMeshes[i]->EditPoints(), (int)allMeshes[i]->GetNbPoints());
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->AddMeshDefinition(allMeshes[i]->EditPoints(), allMeshes[i]->GetNbPoints(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCP(), allMeshes[i]->GetPolyfaceQuery()->GetPointIndexCount(), extent, true, metadata[i].c_str(),texData[i].size() > 0 ? texData[i].data() : 0,texData[i].size(),allMeshes[i]->GetPolyfaceQuery()->GetParamCP(), isMesh3d);
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
        this->m_nodeHeader.m_isTextured = 0;
        }
    if(uvPtr.IsValid() && uvPtr->size() > 0) uvPtr->clear();
    if(uvIPtr.IsValid() && uvIPtr->size() > 0) uvIPtr->clear();
    this->m_nodeHeader.m_isTextured = false;
    this->SetDirty(true);
    m_meshParts.clear();
    m_meshMetadata.clear();    
#endif
    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateFeaturesToChildren()
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = GetLinearFeaturesPtr();
    DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
    if (linearFeaturesPtr->size() == 0) return;
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
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
        for (auto& pair : this->m_nodeHeader.m_3dPointsDescBins)
            {
            if (pair.m_startIndex > index) --pair.m_startIndex;
            }
        }

    pointsPtr->erase(indices);

    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional((ISMStore::FeatureType)defs[i][0], featurePoints[i], extents[i]);
        }
    else if (!this->IsLeaf())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            {
            size_t added = 0;
            if (featurePoints[i].size() <= 1) continue;
            for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                added += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->AddFeatureDefinition((ISMStore::FeatureType)defs[i][0], featurePoints[i], extents[i], true);

            assert(added >= featurePoints[i].size() - sentinels[i]);
            }
        }

    linearFeaturesPtr->clear();

    }

template<class POINT, class EXTENT>  bool  SMMeshIndexNode<POINT, EXTENT>::SyncWithClipSets(const bset<uint64_t>& clips, Transform tr)
{
    bvector<uint64_t> clipList;
    bvector<DRange3d> clipRanges;
    bvector<bool> withSkirts;

    for (uint64_t clip : clips)
    {
        bool hasSkirt = this->GetClipRegistry()->HasSkirt(clip);

        bvector<DPoint3d> polyPts;
        GetClipRegistry()->GetClip(clip, polyPts);
        DRange3d extPts = DRange3d::From(polyPts);
        clipList.push_back(clip);
        withSkirts.push_back(hasSkirt);
        clipRanges.push_back(extPts);
    }

    return this->SyncWithClipSets(clipList, withSkirts, clipRanges, tr);
}
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/18
//=======================================================================================
template<class POINT, class EXTENT>  bool  SMMeshIndexNode<POINT, EXTENT>::SyncWithClipSets(const bset<uint64_t>& clips, const IScalableMesh* meshP)
{
   return SyncWithClipSets(clips, meshP->GetReprojectionTransform());
}

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/18
//=======================================================================================
template<class POINT, class EXTENT>  void  SMMeshIndexNode<POINT, EXTENT>::CollectClipIds(bset<uint64_t>& clipIds) const
{
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffset = GetDiffSetPtr();

    if (diffset == nullptr)
        {
        return;
        }

    for (const auto& diff : *diffset)
        if (diff.clientID != -1 && diff.clientID != 0)
            clipIds.insert(diff.clientID);
}
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/18
//=======================================================================================
template<class POINT, class EXTENT>  bool  SMMeshIndexNode<POINT, EXTENT>::SyncWithClipSets(const bvector<uint64_t>& clipIds, const bvector<bool>& hasSkirts, const bvector<DRange3d>& clipExtents, Transform tr)
{
    if (!this->IsLoaded()) this->Load();

    bool hasSynced = false;
    if (/*size() == 0 || this->m_nodeHeader.m_nbFaceIndexes < 3*/this->m_nodeHeader.m_totalCount == 0) return false;
    for (size_t i =0; i < clipIds.size(); ++i)
    {
        uint64_t clipId = clipIds[i];
        const DRange3d& extent = clipExtents[i];

        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));
        if (!this->m_SMIndex->IsFromCesium() && !extent.IntersectsWith(nodeRange, 2) && !HasClip(clipId)) continue;

        if (this->m_SMIndex->IsFromCesium()) //there we consider all three dimensions due to not being able to rely on XY orientation
        {
            if (!extent.IntersectsWith(nodeRange) && !HasClip(clipId)) continue;
        }
        if (ModifyClip(clipId, false, hasSkirts[i], tr))
            hasSynced = true;
    }
    return hasSynced;
}

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ClipActionRecursive(ClipAction action, uint64_t clipId, DRange3d& extent, size_t& nOfNodesTouched, bool setToggledWhenIdIsOn, Transform tr)
    {
    if (!this->IsLoaded()) Load();
    if (this->m_nodeHeader.m_totalCountDefined ? this->m_nodeHeader.m_totalCount == 0 : this->GetNbPoints() == 0 || this->m_nodeHeader.m_nbFaceIndexes < 3) return;
   /* DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));*/
    
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));
    if (!this->m_SMIndex->IsFromCesium() && !extent.IntersectsWith(nodeRange, 2)) return;

    if (this->m_SMIndex->IsFromCesium()) //there we consider all three dimensions due to not being able to rely on XY orientation
        {
        if (!extent.IntersectsWith(nodeRange)) return;
        }
    bool clipApplied = true;
    uint64_t myTs = LastClippingStateUpdateTimestamp();
    switch (action)
        {
        case ClipAction::ACTION_ADD:
            clipApplied = AddClip(clipId, false, setToggledWhenIdIsOn, tr);
            break;
        case ClipAction::ACTION_MODIFY:
            clipApplied = ModifyClip(clipId, false, setToggledWhenIdIsOn, tr);
            break;
        case ClipAction::ACTION_DELETE:
            clipApplied = DeleteClip(clipId, false, setToggledWhenIdIsOn);
            break;
        }
    if (!clipApplied) return;
    else
        nOfNodesTouched++;

    if (nOfNodesTouched >= 5000 && action != ClipAction::ACTION_DELETE)
        return;
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        uint64_t childTs = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->LastClippingStateUpdateTimestamp();
        if (childTs < myTs)
        {
            bset<uint64_t> collectedClips;
            CollectClipIds(collectedClips);
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->SyncWithClipSets(collectedClips, tr);
        }
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->ClipActionRecursive(action, clipId, extent, nOfNodesTouched, setToggledWhenIdIsOn, tr);
        }
    else if (this->m_nodeHeader.m_numberOfSubNodesOnSplit > 1 && this->m_apSubNodes[0] != nullptr)
        {
        for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (this->m_apSubNodes[indexNodes] != nullptr)
                {
                uint64_t childTs = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->LastClippingStateUpdateTimestamp();
                if (childTs < myTs)
                {
                    bset<uint64_t> collectedClips;
                    CollectClipIds(collectedClips);
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->SyncWithClipSets(collectedClips, tr);
                }
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->ClipActionRecursive(action, clipId, extent, nOfNodesTouched, setToggledWhenIdIsOn, tr);
                }
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
        bvector<DPoint3d> flippedPts(points.size());
        
        for (size_t pt = 0; pt < points.size(); ++pt) flippedPts[pt] = points[points.size() - 1 - pt];

        id = GetClipRegistry()->AddClip(flippedPts.data(), flippedPts.size()) + 1;
        }
    else id = GetClipRegistry()->AddClip(&points[0], points.size()) + 1;
    bool wasClipAdded = AddClip(id, false);
    if (!wasClipAdded) return;
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->AddClipDefinitionRecursive(points,extent);
        }
    else if (!this->IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(this->m_apSubNodes[indexNodes] != nullptr)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->AddClipDefinitionRecursive(points,extent);
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/16
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::PropagateFullMeshDown(size_t depth)
    {
    if (!this->IsLeaf() && this->m_nodeHeader.m_nodeCount == this->m_nodeHeader.m_totalCount)
        {
        this->SplitMeshForChildNodes();
        for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if (this->m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->PropagateFullMeshDown(depth);
            }
        }

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMMeshIndexNode<POINT, EXTENT>::SplitMeshForChildNodes()
    {
    DRange3d contentRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));
    IScalableMeshMeshPtr meshPtr = nullptr;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

    for (auto& nodeP : this->m_apSubNodes)
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

        meshPtr = IScalableMeshMesh::Create(pointsPtr->size(), &pts[0], ptIndices->size(), &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
        ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
        vector<int32_t> childIndices;
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeP->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeP->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeP->m_nodeHeader.m_nodeExtent));

        
        ClipMeshToNodeRange<POINT, EXTENT>(childIndices, nodePts, pts, nodeP->m_nodeHeader.m_contentExtent, nodeRange, meshP);
        if (childIndices.size() == 0) continue;
        
        DRange3d childContentRange;
        if (contentRange.low.z == contentRange.high.z)
        {
            contentRange.low.z = nodeRange.low.z;
            contentRange.high.z = nodeRange.high.z;
        }

        childContentRange.IntersectionOf(contentRange, nodeRange);
        nodeP->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::Create(childContentRange.low.x, childContentRange.low.y, childContentRange.low.z, childContentRange.high.x, childContentRange.high.y, childContentRange.high.z);
        nodeP->m_nodeHeader.m_contentExtentDefined = true;

        bvector<int32_t> indices(childIndices.size());
        if (childIndices.size() > 0)
        {
            memcpy(&indices[0], &childIndices[0], childIndices.size() * sizeof(int32_t));
            bvector<DPoint2d> uvs;
            SimplifyMesh(indices, pts, uvs);

            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(nodeP)->PushPtsIndices(&indices[0], indices.size());
            auto nodePointsPtr = nodeP->GetPointsPtr();
            nodePointsPtr->push_back(&pts[0], pts.size());
            nodeP->m_nodeHeader.m_totalCount = nodePointsPtr->size();
        }
        else
        {
        auto nodePointsPtr = nodeP->GetPointsPtr();
            nodePointsPtr->push_back(&nodePts[0], nodePts.size());
            nodeP->m_nodeHeader.m_totalCount = nodePointsPtr->size();
        }
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
            int feaId2 = idxOrder[currentId].second;
            if (usedFeatures.count(feaId2) != 0) break;
            usedFeatures.insert(feaId2);
            checkIds.insert(currentId);
            if (idxOrder[currentId].first == 1) currentFeature.insert(currentFeature.end(), featureDefs[feaId2].begin() + 1, featureDefs[feaId2].end()-1);
            else currentFeature.insert(currentFeature.end(), featureDefs[feaId2].rbegin()+1, featureDefs[feaId2].rend() - 1);

            if (currentFeature.back() == currentFeature[1]) break;
            if (iterations % 2 != 0) ++currentId;
            else
                {
                size_t id = 0;
                for (id = currentId + 1; id < idxOrder.size(); ++id)
                    if (idxOrder[id].second == feaId2)
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
                f.open((Utf8String("e:\\output\\scmesh\\2016-05-05\\feature_") + Utf8String(std::to_string(this->GetBlockID().m_integerID).c_str()) + Utf8String(std::to_string(featureDefs.size()).c_str())).c_str(), std::ios_base::trunc);
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

    //SortDefinitionsBasedOnNodeBounds(featureDefs, this->m_nodeHeader.m_nodeExtent, &this->operator[](0), this->size());
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

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr = this->GetPointsPtr();

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
        new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(this->GetBlockID().m_integerID, graphStore, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex)
#else
    SMStoredMemoryPoolGenericBlobItem<MTGGraph>::CreateItem(this->GetBlockID().m_integerID, graphStore, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex)
#endif
        );
    SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());

    MTGGraph* graphP = new MTGGraph(*graph);
    storedMemoryPoolItem->SetData(graphP);
    storedMemoryPoolItem->SetDirty();
    SMMemoryPool::GetInstance()->ReplaceItem(memPoolItemPtr, m_graphPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex);


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
    HPRECONDITION(this->IsLeaf());
    POINT splitPosition = this->GetDefaultSplitPosition();
    
    if (this->m_nodeHeader.m_arePoints3d)
        this->SetNumberOfSubNodesOnSplit(8);            
    else
        this->SetNumberOfSubNodesOnSplit(4);                 
    
    if (this->m_nodeHeader.m_numberOfSubNodesOnSplit == 4)
        { 
        if (!this->m_SMIndex->m_precomputedCountNodes)
        {
            this->m_SMIndex->m_countsOfNodesTotal += 4;
        }
        if (this->m_SMIndex->m_countsOfNodesAtLevel.size() < this->m_nodeHeader.m_level + 2)this->m_SMIndex->m_countsOfNodesAtLevel.resize(this->m_nodeHeader.m_level + 2);
        this->m_SMIndex->m_countsOfNodesAtLevel[this->m_nodeHeader.m_level + 1] += 4;
        this->m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 4;

        }
    else
        {
        HPRECONDITION(ExtentOp<EXTENT>::GetThickness(this->GetNodeExtent()) > 0.0);

        if (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent)))
            {
            // Values would be virtually equal ... we will not split
            HDEBUGCODE(this->m_unspliteable = true;)
                return;
            }
        if (!this->m_SMIndex->m_precomputedCountNodes)
        {
            this->m_SMIndex->m_countsOfNodesTotal += 8;
        }
        if (this->m_SMIndex->m_countsOfNodesAtLevel.size() < this->m_nodeHeader.m_level + 1)this->m_SMIndex->m_countsOfNodesAtLevel.resize(this->m_nodeHeader.m_level + 1);
        this->m_SMIndex->m_countsOfNodesAtLevel[this->m_nodeHeader.m_level + 1] += 8;
        this->m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        this->m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        this->m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        this->m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        this->m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));

        this->m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent)));
        s_nCreatedNodes += 8;
        }    

    // Indicate node is not a leaf anymore
    this->m_nodeHeader.m_IsLeaf = false;
    this->m_nodeHeader.m_IsBranched = true;

    //Doesn't work with multithread optimization, deactivated since currently not needed after the generation
    static bool s_applyNeighbor = false;

    if (s_applyNeighbor)
        this->SetupNeighborNodesAfterSplit();

    for (size_t i = 0; i < this->m_nodeHeader.m_numberOfSubNodesOnSplit;++i)
        {        
        this->AdviseSubNodeIDChanged(this->m_apSubNodes[i]);
        }    
    
    for (auto& node : this->m_apSubNodes) 
        this->AdviseSubNodeIDChanged(node);


   this->SplitMeshForChildNodes();
    this->SetDirty(true);
    }


template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ImportTreeFrom(IScalableMeshNodePtr& sourceNode, bool shouldCopyData, bool use2d)
    {
    if (shouldCopyData) this->m_nodeHeader.m_contentExtent = sourceNode->GetContentExtent();
    this->m_nodeHeader.m_nodeExtent = sourceNode->GetNodeExtent();
    if (use2d)
        {
        this->m_nodeHeader.m_nodeExtent.low.z = DBL_MIN;
        this->m_nodeHeader.m_nodeExtent.high.z = DBL_MAX;
        }
    
    if (shouldCopyData)
        {
        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        auto meshP = sourceNode->GetMesh(flags);
        this->m_nodeHeader.m_isTextured = sourceNode->IsTextured();

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

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
        pointsPtr->push_back(&nodePts[0], nodePts.size());
        if (!uv.empty()) PushUV(&uv[0], uv.size());

        vector<int32_t> indicesLine;


        nIndicesCount += ptsIndices.size();
        PushPtsIndices(&ptsIndices[0], ptsIndices.size());
        indicesLine.insert(indicesLine.end(), ptsIndices.begin(), ptsIndices.end());

        if (!uvIndices.empty()) PushUVsIndices(0, &uvIndices[0], uvIndices.size());
        this->m_nodeHeader.m_nbFaceIndexes = indicesLine.size();
        this->m_nodeHeader.m_nbUvIndexes = uv.size();
        this->IncreaseTotalCount(this->GetNbPoints());
        }
    auto sourceChildren = sourceNode->GetChildrenNodes();
    this->m_nodeHeader.m_IsLeaf = sourceChildren.empty();
    this->m_nodeHeader.m_IsBranched = sourceChildren.size() > 1;
    this->m_nodeHeader.m_numberOfSubNodesOnSplit = use2d ? 4 : sourceChildren.size();
    if (use2d) this->m_nodeHeader.m_arePoints3d = false;
    this->m_apSubNodes.resize(this->m_nodeHeader.m_numberOfSubNodesOnSplit);
    if (!this->m_nodeHeader.m_IsBranched) this->m_apSubNodes.resize(1);
    if (!this->m_nodeHeader.m_IsLeaf)
        {
        int j = 0;
        for (size_t i = 0; i < this->m_apSubNodes.size() && i < sourceChildren.size(); ++i)
            {
            if (!sourceChildren[i].IsValid()) continue;
            if (use2d && sourceChildren[i]->GetNodeExtent().low.z > sourceNode->GetNodeExtent().low.z) continue;
            this->m_apSubNodes[j] = CloneChild(sourceChildren[i]->GetNodeExtent());
            dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->m_apSubNodes[j].GetPtr())->ImportTreeFrom(sourceChildren[i], shouldCopyData, use2d);
            ++j;
            }
        this->m_nodeHeader.m_apSubNodeID.resize(this->m_apSubNodes.size());

        for (auto& node : this->m_apSubNodes) this->AdviseSubNodeIDChanged(node);
        }
    this->SetDirty(true);

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
    en->SetMaxTriangles(dtm->GetBcDTM()->GetTrianglesCount());
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
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> existingPts(this->GetPointsPtr());

#if SM_TRACE_FEATURE_DEFINITIONS
    bool hasPtsToTrack = false;
/*    DPoint3d pts[3] = { DPoint3d::From(427283.84, 4501839.24, 0),
    DPoint3d::From(428610.55, 4504064.41,0),
        DPoint3d::From(435130.82, 450594.72,0)};

    for (size_t i = 0; i < 3; ++i)
        if (this->m_nodeHeader.m_nodeExtent.IsContained(pts[i],2)) hasPtsToTrack = true;*/

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
    GetMemoryPool()->RemoveItem(m_dtmPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)this->m_SMIndex);
    m_dtmPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    }

#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::GetMeshParts(bvector<IScalableMeshMeshPtr>& parts, bvector<Utf8String>& metadata, bvector<bvector<uint8_t>>& texData)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> existingPts(this->GetPointsPtr());
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
    /*bvector<int> newMeshParts;
    bvector<Utf8String> newMeshMetadata;
    int64_t currentTexId = -1;
    int64_t currentElementId = -1;
    if (m_meshParts.size() > 0)
        {
        for (size_t i = 0; i < m_meshParts.size(); i += 2)
            {
            auto metadataString = Utf8String(m_meshMetadata[i/2]);
            Json::Value val;
            Json::Reader reader;
            reader.parse(metadataString, val);
            bvector<int> parts;
            bvector<int64_t> texId;
            for (const Json::Value& id : val["texId"])
                {
                texId.push_back(id.asInt64());
                }
            if (!texId.empty())
                {
                if(currentTexId == texId[0] && val["elementId"].asInt64() == currentElementId)
                    {
                    newMeshParts.back() = m_meshParts[i+1];
                    continue;
                    }
                }
            currentTexId = texId[0];
            currentElementId = val["elementId"].asInt64();
            newMeshParts.push_back(m_meshParts[i]);
            newMeshParts.push_back(m_meshParts[i+1]);
            newMeshMetadata.push_back(m_meshMetadata[i/2]);

            }
        }
    m_meshParts = newMeshParts;
    m_meshMetadata = newMeshMetadata;*/
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
        StoreMetadata();
        StoreMeshParts();
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
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
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
            int newId = texId.back();
            newId = newId;
            }
        ClipMeshDefinition(this->m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, indicesClipped, outUvs, &points[i][0], points[i].size(), &indices[i][0], indices[i].size(), extent);
        if (!this->m_nodeHeader.m_nodeExtent.IntersectsWith(extentClipped)) continue;
        if(sum+pointsClipped.size() > 65000)
            {
            pointsClipped.resize(0);
            indicesClipped.resize(0);
            outUvs.resize(0);
            }

        if(outUvs.size() > 0) this->m_nodeHeader.m_isTextured = true;
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
    bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader, SMStoreDataType::MeshParts);
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
    this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader, SMStoreDataType::MeshParts);
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
    bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader, SMStoreDataType::Metadata);
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
    this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader, SMStoreDataType::Metadata);

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
    
    if (GetClipRegistry() == nullptr || !GetClipRegistry()->IsClipDefinitionFileExist())
        return poolMemItemPtr;

   //if (this->m_SMIndex->IsTerrain() == false) 
    //   return poolMemItemPtr;

    if (!SMMemoryPool::GetInstance()->GetItem<DifferenceSet>(poolMemItemPtr, m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex))
        {   
        ISDiffSetDataStorePtr nodeDataStore;
        bool result = this->m_SMIndex->GetDataStore()->GetSisterNodeDataStore(nodeDataStore, &this->m_nodeHeader, true);
        assert(result == true);
        
        RefCountedPtr<SMStoredMemoryPoolGenericVectorItem<DifferenceSet>> storedMemoryPoolItem(
       #ifndef VANCOUVER_API
        new SMStoredMemoryPoolGenericVectorItem<DifferenceSet>(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex)
#else
        SMStoredMemoryPoolGenericVectorItem<DifferenceSet>::CreateItem(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex)
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
 

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> SMMeshIndexNode<POINT, EXTENT>::GetTileDTM()
    {
    std::lock_guard<std::mutex> lock(m_dtmLock); //don't want to add item twice
    RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> poolMemItemPtr;


    if (!SMMemoryPool::GetInstance()->GetItem<BcDTMPtr>(poolMemItemPtr, m_dtmPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)this->m_SMIndex))
        {
        RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> storedMemoryPoolItem(
#ifndef VANCOUVER_API   
            new SMMemoryPoolGenericBlobItem<BcDTMPtr>(nullptr, 0, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)this->m_SMIndex)
#else
            SMMemoryPoolGenericBlobItem<BcDTMPtr>::CreateItem(nullptr, 0, this->GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)this->m_SMIndex)
#endif
        );
        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
        m_dtmPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(m_dtmPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
        poolMemItemPtr = storedMemoryPoolItem.get();

        IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
            new ScalableMeshNode<POINT>(nodePtr)
#else
            ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
        );
        auto meshP = nodeP->GetMesh(flags);
        if (meshP != nullptr)
            {
            auto ptrP = storedMemoryPoolItem->EditData();
            if (ptrP == nullptr) storedMemoryPoolItem->SetData(new BcDTMPtr(BcDTM::Create()));
            meshP->GetAsBcDTM(*storedMemoryPoolItem->EditData());
            }
        }

    return poolMemItemPtr;
    }



template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> SMMeshIndexNode<POINT, EXTENT>::GetGraphPtr(bool loadGraph)
    {
    std::lock_guard<std::mutex> lock(m_graphMutex); //don't want to add item twice
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> poolMemItemPtr;

    if (!SMMemoryPool::GetInstance()->GetItem<MTGGraph>(poolMemItemPtr, m_graphPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex) && loadGraph)
        {                  
        ISMMTGGraphDataStorePtr nodeDataStore;
        bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader);
        assert(result == true);  

        RefCountedPtr<SMStoredMemoryPoolGenericBlobItem<MTGGraph>> storedMemoryPoolItem(
       #ifndef VANCOUVER_API
        new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex)
        #else
        SMStoredMemoryPoolGenericBlobItem<MTGGraph>::CreateItem(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::Graph, (uint64_t)this->m_SMIndex)
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

    if (!GetMemoryPool()->template GetItem<int32_t>(poolMemVectorItemPtr, m_featurePoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::LinearFeature, (uint64_t)this->m_SMIndex))
        {
        ISMInt32DataStorePtr nodeDataStore;
        bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &this->m_nodeHeader, SMStoreDataType::LinearFeature);
        assert(result == true);         

        RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(
    #ifndef VANCOUVER_API
        new SMStoredMemoryPoolVectorItem<int32_t>(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::LinearFeature, (uint64_t)this->m_SMIndex)
        #else
        SMStoredMemoryPoolVectorItem<int32_t>::CreateItem(this->GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::LinearFeature, (uint64_t)this->m_SMIndex)
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

    if (!this->m_SMIndex->IsFromCesium())
        {
        poolMemVectorItemPtr = this->template GetMemoryPoolItem<ISMInt32DataStorePtr, int32_t, SMMemoryPoolVectorItem<int32_t>, SMStoredMemoryPoolVectorItem<int32_t>>(m_triIndicesPoolItemId, SMStoreDataType::TriPtIndices, this->GetBlockID());
        }
    else
        {
        SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr = this->template  GetMemoryPoolMultiItem<ISMCesium3DTilesDataStorePtr, Cesium3DTilesBase, SMMemoryPoolMultiItemsBase, SMStoredMemoryPoolMultiItems<Cesium3DTilesBase>>(this->m_pointsPoolItemId, SMStoreDataType::Cesium3DTiles, this->GetBlockID()).get();
        // In the cesium format, indices are packaged with the points
        m_triIndicesPoolItemId = this->m_pointsPoolItemId;
        bool result = poolMemMultiItemsPtr->GetItem<int32_t>(poolMemVectorItemPtr, SMStoreDataType::TriPtIndices);
        assert(result == true);
        }

    return poolMemVectorItemPtr;

#if 0 
    //Sample code of using store capabilities to load the point and triangle pt indices atomically
    /*
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;        
    SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr;
                    
    if (!SMMemoryPool::GetInstance()->GetItem(poolMemMultiItemsPtr, this->m_pointsPoolItemId, this->GetBlockID().m_integerID, SMStoreDataType::PointAndTriPtIndices, (uint64_t)this->m_SMIndex))
        {                         
        ISMPointTriPtIndDataStorePtr pointTriIndDataStore;        
        bool result = this->m_SMIndex->GetDataStore()->GetNodeDataStore(pointTriIndDataStore, &this->m_nodeHeader);
        assert(result == true);        
    
        RefCountedPtr<SMStoredMemoryPoolMultiItems<PointAndTriPtIndicesBase>> storedMemoryMultiItemPool;
    
        storedMemoryMultiItemPool = new SMStoredMemoryPoolMultiItems<PointAndTriPtIndicesBase>(pointTriIndDataStore, this->GetBlockID().m_integerID, SMStoreDataType::PointAndTriPtIndices, (uint64_t)this->m_SMIndex);

        SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryMultiItemPool.get());
        this->m_pointsPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
        assert(this->m_pointsPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
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

    if (!this->IsTextured())
        return poolMemVectorItemPtr;
            
    if (!this->m_SMIndex->IsFromCesium())
        {
        poolMemVectorItemPtr = this->template GetMemoryPoolItem<ISMInt32DataStorePtr, int32_t, SMMemoryPoolVectorItem<int32_t>, SMStoredMemoryPoolVectorItem<int32_t>>(m_triUvIndicesPoolItemId, SMStoreDataType::TriUvIndices, this->GetBlockID());
        }
    else
        {
        // In the Cesium format, UV indices are the same as mesh indices
        poolMemVectorItemPtr = GetPtsIndicePtr();
        }

    return poolMemVectorItemPtr;          
    }  

//NEEDS_WORK_MST : Should use only the GetTexturePtr() with a texture id passed in parameter instead. 
template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolBlobItem<Byte>> SMMeshIndexNode<POINT, EXTENT>::GetTexturePtr()
    {
    return GetTexturePtr(this->m_nodeHeader.m_textureID.IsValid() && this->m_nodeHeader.m_textureID != ISMStore::GetNullNodeID() && this->m_nodeHeader.m_textureID.m_integerID != -1 ? this->m_nodeHeader.m_textureID.m_integerID : this->GetBlockID().m_integerID);
    }

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolBlobItem<Byte>> SMMeshIndexNode<POINT, EXTENT>::GetTexturePtr(uint64_t texID)
    {
    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> poolMemBlobItemPtr;

    if (!this->IsTextured())
        return poolMemBlobItemPtr;

    if (!this->m_SMIndex->IsFromCesium())
        {
        SMMemoryPoolItemId texPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->GetPoolIdForTextureData(texID);

        poolMemBlobItemPtr = this->template GetMemoryPoolItem<ISMTextureDataStorePtr, Byte, SMMemoryPoolBlobItem<Byte>, SMStoredMemoryPoolBlobItem<Byte>>(texPoolItemId, SMStoreDataType::Texture, HPMBlockID(texID));
        assert(poolMemBlobItemPtr.IsValid());

        m_textureIds.insert(texID);
        ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->TextureManager()->SetPoolIdForTextureData(texID, texPoolItemId);
        }
    else
        {
        SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr = this->template GetMemoryPoolMultiItem<ISMCesium3DTilesDataStorePtr, Cesium3DTilesBase, SMMemoryPoolMultiItemsBase, SMStoredMemoryPoolMultiItems<Cesium3DTilesBase>>(this->m_pointsPoolItemId, SMStoreDataType::Cesium3DTiles, this->GetBlockID()).get();
        // In the cesium format, textures are packaged with the points
        this->m_texturePoolItemId = this->m_pointsPoolItemId;
        bool result = poolMemMultiItemsPtr->GetItem<Byte>(poolMemBlobItemPtr, SMStoreDataType::Texture);
        assert(result == true);
        }

        
    return poolMemBlobItemPtr;
    }

template<class POINT, class EXTENT> RefCountedPtr<SMMemoryPoolBlobItem<Byte>> SMMeshIndexNode<POINT, EXTENT>::GetTextureCompressedPtr()
    {
    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> poolMemBlobItemPtr;

    if (!this->IsTextured())
        return poolMemBlobItemPtr;

    if (!this->m_SMIndex->IsFromCesium())
        {
        auto texID = this->m_nodeHeader.m_textureID.IsValid() && this->m_nodeHeader.m_textureID != ISMStore::GetNullNodeID() && this->m_nodeHeader.m_textureID.m_integerID != -1 ? this->m_nodeHeader.m_textureID : this->GetBlockID();

        poolMemBlobItemPtr = this->template GetMemoryPoolItem<ISMTextureDataStorePtr, Byte, SMMemoryPoolBlobItem<Byte>, SMStoredMemoryPoolBlobItem<Byte>>(this->m_texturePoolItemId, SMStoreDataType::TextureCompressed, texID);
        assert(poolMemBlobItemPtr.IsValid());
        }
    else
        {
        SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr = this->template GetMemoryPoolMultiItem<ISMCesium3DTilesDataStorePtr, Cesium3DTilesBase, SMMemoryPoolMultiItemsBase, SMStoredMemoryPoolMultiItems<Cesium3DTilesBase>>(this->m_pointsPoolItemId, SMStoreDataType::TextureCompressed, this->GetBlockID()).get();
        // In the cesium format, textures are packaged with the points
        this->m_texturePoolItemId = this->m_pointsPoolItemId;
        bool result = poolMemMultiItemsPtr->GetItem<Byte>(poolMemBlobItemPtr, SMStoreDataType::TextureCompressed);
        assert(result == true);
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
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRaster(ITextureProviderPtr sourceRasterP, Transform unitTransform)
    {
    this->m_SMIndex->m_nTexturedNodes++;
    float progressForStep = (float)(this->m_SMIndex->m_nTexturedNodes) / this->m_SMIndex->m_countsOfNodesTotal;

    if (this->m_SMIndex->m_progress != nullptr) this->m_SMIndex->m_progress->Progress() = progressForStep;

    if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;

    if (!this->IsLoaded()) Load();
    DRange2d rasterBox = sourceRasterP->GetTextureExtent();
    //get overlap between node and raster extent
    DRange2d contentExtent = DRange2d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent));

    unitTransform.Multiply(contentExtent.low, contentExtent.low);
    unitTransform.Multiply(contentExtent.high, contentExtent.high);
    if (!rasterBox.IntersectsWith(contentExtent)) return;
    if (this->GetPointsPtr()->size() == 0 || this->m_nodeHeader.m_nbFaceIndexes == 0) return;

    int textureWidthInPixels = 1024, textureHeightInPixels = 1024;

    if (dynamic_cast<StreamTextureProvider*>(sourceRasterP.get()))
        {
        textureWidthInPixels = 256;
        textureHeightInPixels = 256;
        }


    bvector<uint8_t> tex;
    sourceRasterP->GetTextureForArea(tex, textureWidthInPixels, textureHeightInPixels, contentExtent);
    DPoint2d pixSize = sourceRasterP->GetMinPixelSize();


    if (this->IsLeaf() && (contentExtent.XLength() / pixSize.x > textureWidthInPixels || contentExtent.YLength() / pixSize.y > textureHeightInPixels))        
        this->SplitNodeBasedOnImageRes();
        


    if (contentExtent.XLength() / pixSize.x > textureWidthInPixels || contentExtent.YLength() / pixSize.y > textureHeightInPixels)
        {
        this->m_nodeHeader.m_textureResolution = std::max(contentExtent.XLength() / textureWidthInPixels, contentExtent.YLength() / textureHeightInPixels);
        }
    else
        {
        this->m_nodeHeader.m_textureResolution = std::max(pixSize.x, pixSize.y);
        }


    if (this->m_nodeHeader.m_geometricResolution == 0)this->m_nodeHeader.m_geometricResolution = this->m_nodeHeader.m_textureResolution;
        if (!tex.empty())
        PushTexture(tex.data(), tex.size());     

        this->m_nodeHeader.m_isTextured = true;
        this->m_nodeHeader.m_textureID = this->GetBlockID();
        this->m_nodeHeader.m_nbTextures = 1;
    
    //UpdateNodeFromBcDTM();
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());

    if (existingFaces->size() >= 4)
        {
        //compute uv's        
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

        vector<DPoint3d> points(pointsPtr->size());

        for (size_t i = 0; i < pointsPtr->size(); ++i)
            points[i] = DPoint3d::From(PointOp<POINT>::GetX(pointsPtr->operator[](i)), PointOp<POINT>::GetY(pointsPtr->operator[](i)), PointOp<POINT>::GetZ(pointsPtr->operator[](i)));
        vector<int32_t> indicesOfTexturedRegion;
        vector<DPoint2d> uvsOfTexturedRegion(points.size());

        Transform t;
        t.InverseOf(unitTransform);              

        t.Multiply(contentExtent.low, contentExtent.low);
        t.Multiply(contentExtent.high, contentExtent.high);
        for (size_t i = 0; i < existingFaces->size(); i+=3)
            {
            DPoint3d face[3];
            int32_t idx[3] = { (*existingFaces)[i], (*existingFaces)[i + 1], (*existingFaces)[i + 2] };
            DPoint2d uvCoords[3];
            for (size_t j = 0; j < 3; ++j)
                {
                face[j] = points[idx[j] - 1];
                uvCoords[j].x = max(0.0,min((face[j].x - contentExtent.low.x) / (contentExtent.XLength()),1.0));
                uvCoords[j].y = max(0.0, min((face[j].y - contentExtent.low.y) / (contentExtent.YLength()), 1.0));
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
        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords = GetUVCoordsPtr();
        uvCoords->clear();
        PushUV( &uvsOfTexturedRegion[0], uvsOfTexturedRegion.size());
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes = GetUVsIndicesPtr();
        uvIndexes->clear();
        PushUVsIndices(0, &indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());        
        }

    this->SetDirty(true);

    }


//=======================================================================================
// @description Cut a tile to decrease the number of points toward target split threshold
// @bsimethod                                                     Mathieu.St-Pierre 05/18
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::CutTile(uint32_t splitThreshold, Transform unitTransform)
    {
#if 0
        this->m_SMIndex->m_nTexturedNodes++;
        float progressForStep = (float)(this->m_SMIndex->m_nTexturedNodes) / this->m_SMIndex->m_countsOfNodesTotal;

        if (this->m_SMIndex->m_progress != nullptr) this->m_SMIndex->m_progress->Progress() = progressForStep;

        if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;
#endif
        
        if (!this->IsLoaded()) Load();
/*
        DRange2d rasterBox = sourceRasterP->GetTextureExtent();
        //get overlap between node and raster extent
        DRange2d contentExtent = DRange2d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent));

        unitTransform.Multiply(contentExtent.low, contentExtent.low);
        unitTransform.Multiply(contentExtent.high, contentExtent.high);
        if (!rasterBox.IntersectsWith(contentExtent)) return;
        if (this->GetPointsPtr()->size() == 0 || this->m_nodeHeader.m_nbFaceIndexes == 0) return;
*/

        this->m_nodeHeader.m_balanced = false;
        this->m_wasBalanced = false;
        this->m_nodeHeader.m_SplitTreshold = splitThreshold;

      
        if (this->IsLeaf() && (this->GetNbPoints() > splitThreshold))
            {
            this->SplitNodeBasedOnImageRes();

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndicePtr = GetPtsIndicePtr();
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr = this->GetPointsPtr();

            ptsIndicePtr->clear();
            ptsPtr->clear();
            this->m_nodeHeader.m_nbFaceIndexes = 0;
            this->m_nodeHeader.m_nodeCount = 0;

            this->SetDirty(true);
            }
        
/*

        if (contentExtent.XLength() / pixSize.x > textureWidthInPixels || contentExtent.YLength() / pixSize.y > textureHeightInPixels)
        {
            this->m_nodeHeader.m_textureResolution = std::max(contentExtent.XLength() / textureWidthInPixels, contentExtent.YLength() / textureHeightInPixels);
        }
        else
        {
            this->m_nodeHeader.m_textureResolution = std::max(pixSize.x, pixSize.y);
        }
*/


//        if (this->m_nodeHeader.m_geometricResolution == 0)this->m_nodeHeader.m_geometricResolution = this->m_nodeHeader.m_textureResolution;
  


#if 0
        //UpdateNodeFromBcDTM();
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> existingFaces(GetPtsIndicePtr());

        if (existingFaces->size() >= 4)
        {
            //compute uv's        
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

            vector<DPoint3d> points(pointsPtr->size());

            for (size_t i = 0; i < pointsPtr->size(); ++i)
                points[i] = DPoint3d::From(PointOp<POINT>::GetX(pointsPtr->operator[](i)), PointOp<POINT>::GetY(pointsPtr->operator[](i)), PointOp<POINT>::GetZ(pointsPtr->operator[](i)));
            vector<int32_t> indicesOfTexturedRegion;
            vector<DPoint2d> uvsOfTexturedRegion(points.size());

            Transform t;
            t.InverseOf(unitTransform);

            t.Multiply(contentExtent.low, contentExtent.low);
            t.Multiply(contentExtent.high, contentExtent.high);
            for (size_t i = 0; i < existingFaces->size(); i += 3)
            {
                DPoint3d face[3];
                int32_t idx[3] = { (*existingFaces)[i], (*existingFaces)[i + 1], (*existingFaces)[i + 2] };
                DPoint2d uvCoords[3];
                for (size_t j = 0; j < 3; ++j)
                {
                    face[j] = points[idx[j] - 1];
                    uvCoords[j].x = max(0.0, min((face[j].x - contentExtent.low.x) / (contentExtent.XLength()), 1.0));
                    uvCoords[j].y = max(0.0, min((face[j].y - contentExtent.low.y) / (contentExtent.YLength()), 1.0));
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
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords = GetUVCoordsPtr();
            uvCoords->clear();
            PushUV(&uvsOfTexturedRegion[0], uvsOfTexturedRegion.size());
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes = GetUVsIndicesPtr();
            uvIndexes->clear();
            PushUVsIndices(0, &indicesOfTexturedRegion[0], indicesOfTexturedRegion.size());
        }
#endif                

    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================

struct CreationProcessWorkItem;
typedef RefCountedPtr<CreationProcessWorkItem> CreationProcessWorkItemPtr;

struct CreationProcessWorkItem : RefCounted<WorkItem> 
    {
    private: 


        std::function<void()> m_lambda;

    protected:

        CreationProcessWorkItem(std::function<void()>& lambda) :m_lambda(lambda){}
        ~CreationProcessWorkItem() {};
        virtual void    _DoWork() override
            {
            m_lambda();
            }
        
    public: 

        static CreationProcessWorkItemPtr Create(std::function<void()>& workingFnc)
            {
            return new CreationProcessWorkItem(workingFnc);
            }
    };


static bool s_multiThreadCutting = true;
static bool s_multiThreadTexturing = true;

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::TextureFromRasterRecursive(ITextureProviderPtr sourceRasterP, Transform unitTransform)
    {

    if (s_multiThreadTexturing && dynamic_cast<StreamTextureProvider*>(sourceRasterP.get()) != nullptr)
        {        
        assert(((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->m_creationProcessThreadPoolPtr.IsValid());


        function <void()> textureFnc = std::bind([](SMMeshIndexNode<POINT, EXTENT>* node, ITextureProviderPtr sourceRasterP, Transform unitTransform/*, size_t threadId*/) ->void
        {
            node->TextureFromRaster(sourceRasterP, unitTransform);
            if (node->m_SMIndex->m_progress != nullptr && node->m_SMIndex->m_progress->IsCanceled()) return;

            if (node->m_pSubNodeNoSplit != NULL && !node->m_pSubNodeNoSplit->IsVirtualNode())
            {
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_pSubNodeNoSplit)->TextureFromRasterRecursive(sourceRasterP, unitTransform);
            }
            else if (!node->IsLeaf())
            {
                for (size_t indexNodes = 0; indexNodes < node->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                    if (node->m_apSubNodes[indexNodes] != nullptr)
                    {
                        if (node->m_SMIndex->m_progress != nullptr && node->m_SMIndex->m_progress->IsCanceled()) return;
                        auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apSubNodes[indexNodes]);
                        assert(mesh != nullptr);
                        mesh->TextureFromRasterRecursive(sourceRasterP, unitTransform);
                    }
                }
            }
        }, this, sourceRasterP, unitTransform);

        WorkItemPtr texturingWorkItem(CreationProcessWorkItem::Create(textureFnc));
        ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->m_creationProcessThreadPoolPtr->QueueWork(texturingWorkItem);
        }
    else
        {        
        TextureFromRaster(sourceRasterP, unitTransform);
        if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;

        if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->TextureFromRasterRecursive(sourceRasterP, unitTransform);
            }
        else if (!this->IsLeaf())
            {
            for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                if (this->m_apSubNodes[indexNodes] != nullptr)
                    {
                    if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;
                    auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes]);
                    assert(mesh != nullptr);
                    mesh->TextureFromRasterRecursive(sourceRasterP, unitTransform);
                    }
                }   
            }
        }
    }


template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::CutTileRecursive(uint32_t splitThreshold, Transform unitTransform)
    {
    if (s_multiThreadCutting)
        {
        assert(((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->m_creationProcessThreadPoolPtr.IsValid());


        function <void()> cuttingFnc = std::bind([](SMMeshIndexNode<POINT, EXTENT>* node, uint32_t splitThreshold, Transform unitTransform/*, size_t threadId*/) ->void
            {
            node->CutTile(splitThreshold, unitTransform);
            if (node->m_SMIndex->m_progress != nullptr && node->m_SMIndex->m_progress->IsCanceled()) return;

            if (node->m_pSubNodeNoSplit != NULL && !node->m_pSubNodeNoSplit->IsVirtualNode())
                {
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_pSubNodeNoSplit)->CutTileRecursive(splitThreshold, unitTransform);
                }
            else if (!node->IsLeaf())
                {
                for (size_t indexNodes = 0; indexNodes < node->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                    {
                    if (node->m_apSubNodes[indexNodes] != nullptr)
                        {
                        if (node->m_SMIndex->m_progress != nullptr && node->m_SMIndex->m_progress->IsCanceled()) return;
                        auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apSubNodes[indexNodes]);
                        assert(mesh != nullptr);
                        mesh->CutTileRecursive(splitThreshold, unitTransform);
                        }
                    }
                }
            }, this, splitThreshold, unitTransform);

        WorkItemPtr texturingWorkItem(CreationProcessWorkItem::Create(cuttingFnc));
        ((SMMeshIndex<POINT, EXTENT>*)this->m_SMIndex)->m_creationProcessThreadPoolPtr->QueueWork(texturingWorkItem);
        }
    else
        {
        CutTile(splitThreshold, unitTransform);
        if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;

        if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->CutTileRecursive(splitThreshold, unitTransform);
            }
        else if (!this->IsLeaf())
            {
            for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                if (this->m_apSubNodes[indexNodes] != nullptr)
                    {
                    if (this->m_SMIndex->m_progress != nullptr && this->m_SMIndex->m_progress->IsCanceled()) return;
                    auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes]);
                    assert(mesh != nullptr);
                    mesh->CutTileRecursive(splitThreshold, unitTransform);
                    }
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
    if (this->m_pSubNodeNoSplit != NULL && !this->m_pSubNodeNoSplit->IsVirtualNode())
        {
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->RefreshMergedClipsRecursive();
        }
    else if (!this->IsLeaf())
        {
        for (size_t indexNodes = 0; indexNodes < this->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
            {
            if(this->m_apSubNodes[indexNodes] != nullptr)
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[indexNodes])->RefreshMergedClipsRecursive();
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/18
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::HasAnyClip()
{
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    if (!diffsetPtr.IsValid()) return false;
    return true;
}

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 02/16
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::HasClip(uint64_t clipId)
{
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    if (!diffsetPtr.IsValid()) return false;
    bool isUpToDate = false;
    for (const auto& diffSet : *diffsetPtr)
    {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) isUpToDate= true;
    }
    for (const auto& diffSet : *diffsetPtr)
    {
        if (diffSet.clientID == clipId && (!isUpToDate || !diffSet.upToDate || !diffSet.IsEmpty() || diffSet.clientID == (uint64_t)-1)) return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 05/16
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::IsClippingUpToDate()
    {
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();

    if (!diffsetPtr.IsValid()) return false;

    for (const auto& diffSet : *diffsetPtr)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return true;
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 11/18
//=======================================================================================
template<class POINT, class EXTENT>  uint64_t SMMeshIndexNode<POINT, EXTENT>::LastClippingStateUpdateTimestamp() const
    {
    return m_updateClipTimestamp;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::ComputeMergedClips(Transform tr)
    {
    if (dynamic_cast<SMMeshIndex<POINT,EXTENT>*>(this->m_SMIndex)->m_isInsertingClips) return;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();

    if (!diffSetPtr.IsValid())
        return;


    {
    if (diffSetPtr->size() == 0) return;
    for (const auto& diffSet : *diffSetPtr)
        {
        if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
        }
    //std::cout << "Merging clips for " << this->GetBlockID().m_integerID << " we have " << diffSetPtr->size() << "clips" << std::endl;
   
    if (this->m_SMIndex->IsFromCesium() && tr.IsIdentity())
        assert(!"ECEF datasets must define a transform to apply any clipping");

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

    vector<DPoint3d> points(pointsPtr->size());
    
    PtToPtConverter::Transform(&points[0], &(*pointsPtr)[0], points.size());

  /*  DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));*/

    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
                                        ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));

    if (this->m_SMIndex->IsFromCesium())
        {
        bvector<DPoint3d> box(8);
        nodeRange.Get8Corners(box.data());
        tr.Multiply(&box[0], &box[0], (int)box.size());
        nodeRange = DRange3d::From(box);

        tr.Multiply(&points[0], &points[0], (int)points.size());
        }

    bvector<bvector<DPoint3d>> polys;
    bvector<uint64_t> clipIds;
    bvector<DifferenceSet> skirts;
    bvector<bpair<double, int>> metadata;
    //DRange3d extentOfBiggestPoly = DRange3d::NullRange(); 
    bool polyInclusion = false;
    bset<uint64_t> addedPolyIds;
    double minEdgeLength = DBL_MAX;
   // size_t indexOfBiggestPoly = 0;
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(GetPtsIndicePtr());
   // size_t indexOfBiggestPoly = 0;
    bvector<size_t> useVolumeClips;
    bvector<bool> isMask;
    for (const auto& diffSet : *diffSetPtr)
        {
        //uint64_t upperId = (diffSet.clientID >> 32);
        if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && diffSet.toggledForID)
            {
            if (addedPolyIds.count(diffSet.clientID) > 0) continue;
            addedPolyIds.insert(diffSet.clientID);
            clipIds.push_back(diffSet.clientID);
            polys.push_back(bvector<DPoint3d>());
            SMClipGeometryType geom;
            SMNonDestructiveClipType type;
            bool isActive;
            GetClipRegistry()->GetClipWithParameters(diffSet.clientID, polys.back(), geom, type, isActive);
            isMask.push_back(type == SMNonDestructiveClipType::Mask);
            if (geom == SMClipGeometryType::BoundedVolume)
                useVolumeClips.push_back(polys.size() - 1);

            if (this->m_SMIndex->IsFromCesium())
                tr.Multiply(&polys.back()[0], &polys.back()[0], (int)polys.back().size());

            if (type == SMNonDestructiveClipType::Boundary) polyInclusion = true;
            DRange3d polyExtent = DRange3d::From(&polys.back()[0], (int)polys.back().size());


            if (geom != SMClipGeometryType::BoundedVolume && !polyExtent.IntersectsWith(nodeRange, 2))
                {
                polys.resize(polys.size() - 1);
                clipIds.resize(clipIds.size() - 1);
                continue;
                }

            if (s_simplifyOverviewClips && this->m_nodeHeader.m_level <= 1 && polys.back().size() >= this->m_nodeHeader.m_nodeCount)
            {
                if (minEdgeLength == DBL_MAX)
                    minEdgeLength = ComputeMinEdgeLength(&points[0], pointsPtr->size(), &(*ptIndices)[0], ptIndices->size());
                SimplifyPolygonToMinEdge(minEdgeLength / 2.0, polys.back());
            }

            int nOfLoops = 0;
            if (geom == SMClipGeometryType::ComplexPolygon)
                {
                //count loops
                bvector<bvector<DPoint3d>> polyLoops;
                bvector<DPoint3d> currentLoop;
                for (auto& pt : polys.back())
                    {
                    if (pt.IsDisconnect())
                        {
                        nOfLoops++;
                        polyLoops.push_back(currentLoop);
                        currentLoop.clear();
                        }
                    else currentLoop.push_back(pt);
                    }

                if (!currentLoop.empty())
                    {
                    nOfLoops++;
                    polyLoops.push_back(currentLoop);
                    currentLoop.clear();
                    }

                polys.resize(polys.size() - 1);
                clipIds.resize(clipIds.size() - 1);
                for (auto& loop : polyLoops)
                    {
                    clipIds.push_back(diffSet.clientID);
                    polys.push_back(loop);
                    }

                }
            else nOfLoops = 1;

            double importance;
            int nDimensions;
            GetClipRegistry()->GetClipMetadata(diffSet.clientID, importance, nDimensions);
            for (size_t i = 0; i < nOfLoops; ++i)
                metadata.push_back(make_bpair(importance, nDimensions));
            }
        else if (!diffSet.toggledForID)
            {
            skirts.push_back(diffSet);
            }
             
        }

    bvector<ClipVectorPtr> clips(polys.size());
    //Deal with case where one of the clips is a 3d plane set
    if (!useVolumeClips.empty())
        {
        //create a clipvector with infinite z dimensions for the polygons
        for (size_t index = 0; index < polys.size(); index++)
            {
            if (!polys[index].empty())
            {
                auto curvePtr = ICurvePrimitive::CreateLineString(polys[index]);
                CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
                clips[index] = ClipVector::CreateFromCurveVector(*cv, 0.0, 0.1);
            }
            }
        for (size_t index : useVolumeClips)
        {
            SMClipGeometryType geom;
            SMNonDestructiveClipType type;
            bool isActive;
            GetClipRegistry()->GetClipWithParameters(clipIds[index], clips[index], geom, type, isActive);
            isMask[index] = type == SMNonDestructiveClipType::Mask;
        }
        }


    diffSetPtr->clear();
    for(auto& skirt: skirts) diffSetPtr->push_back(skirt);
    m_nbClips = skirts.size();


   
            

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
    

        Clipper clipNode(&points[0], points.size(), (int32_t*)&(*ptIndices)[0], ptIndices->size(), nodeRange, this->m_nodeHeader.m_nodeExtent, uvBuffer, uvIndices);
        
        if(this->m_nodeHeader.m_isTextured && this->m_SMIndex->IsTextured() == SMTextureType::Streaming)
            clipNode.SetTextureDimensions(256,256);

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
        if (useVolumeClips.empty())
        {
            if (!this->m_nodeHeader.m_arePoints3d && dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->IsTerrain() && !polyInclusion && !this->m_SMIndex->IsFromCesium() && dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->m_canUseBcLibClips)
            {
                BcDTMPtr dtm = nodeP->GetBcDTM().get();
                if (dtm.get() != nullptr)
                {
                    BcDTMPtr toClipBcDTM = dtm->Clone();
                    DTMPtr toClipDTM = toClipBcDTM.get();
                    if (this->m_SMIndex->IsFromCesium())
                        toClipBcDTM->Transform(tr);
                    if (this->IsLeaf()) //always clip leaves regardless of width/area criteria
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
                PolyfaceQueryCP polyfaceQuery = meshP->GetPolyfaceQuery();
                PolyfaceHeaderPtr polyHeader = PolyfaceHeader::New();
                if (this->m_SMIndex->IsFromCesium())
                {
                    polyHeader->CopyFrom(*polyfaceQuery);
                    polyHeader->Transform(tr);
                }
                if (meshP.get() != nullptr)
                    hasClip = GetRegionsFromClipPolys3D(polyfaces, polys, this->m_SMIndex->IsFromCesium() ? polyHeader.get() : polyfaceQuery);
            }
        }
        else
        {
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            flags->SetLoadTexture(true);
            IScalableMeshMeshPtr meshP = nodeP->GetMesh(flags);
            PolyfaceQueryCP polyfaceQuery = meshP->GetPolyfaceQuery();
            PolyfaceHeaderPtr polyHeader = PolyfaceHeader::New();
            if (this->m_SMIndex->IsFromCesium())
            {
                polyHeader->CopyFrom(*polyfaceQuery);
                polyHeader->Transform(tr);
            }
            ClipVectorPtr clipComplete = ClipVector::Create();
            bvector<bool> isMaskPrimitive;
            for (auto&clip : clips)
            {
                for (size_t i =0; i < clip->size(); ++i)
                    isMaskPrimitive.push_back(true/*isMask[&clip - clips.data()]*/);
                clipComplete->Append(*clip);
            }
            bvector<size_t> polyfaceIndices;
            if (meshP.get() != nullptr)
                hasClip = GetRegionsFromClipVector3D(polyfaces, polyfaceIndices, clipComplete.get(), this->m_SMIndex->IsFromCesium() ? polyHeader.get() : polyfaceQuery, isMaskPrimitive);
        }

        if (hasClip) 
            {
           
            if (this->m_SMIndex->IsFromCesium())
            {
                Transform inverseTr;
                inverseTr.InverseOf(tr);
                for(auto& polyList: polyfaces) PolyfaceHeader::Transform(polyList, inverseTr);
                inverseTr.Multiply(&points[0], &points[0], (int)points.size());
            }
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

    //std::cout << "Merged clips for " << this->GetBlockID().m_integerID << " we have " << diffSetPtr->size() << "clips" << std::endl;

    }


template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::BuildSkirts()
    {   
    if (dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(this->m_SMIndex)->m_isInsertingClips) return;
    
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffsetPtr = GetDiffSetPtr();
    if (!diffsetPtr.IsValid() || diffsetPtr->size() == 0) return;
    for (const auto& diffSet : *diffsetPtr)
            {
            if (diffSet.clientID == (uint64_t)-1 && diffSet.upToDate) return;
            }

    bool shouldBuildSkirts = false;
    for (const auto& diffSet : *diffsetPtr)
        {
        if (diffSet.clientID < ((uint64_t)-1) && diffSet.clientID != 0 && !diffSet.toggledForID)
            {
            shouldBuildSkirts = true;
            }
        }

    if (!shouldBuildSkirts) return;
    //if (this->m_nodeHeader.m_arePoints3d) return;
        //DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_nodeExtent),
         //                                   ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_nodeExtent));


        auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
        IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
            new ScalableMeshNode<POINT>(nodePtr)
#else
            ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
            );
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());


#if SM_TRACE_BUILD_SKIRTS
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> pointsIndicePtr(GetPtsIndicePtr());

            {
            WString fileName(L"E:\\output\\scmesh\\2016-11-30\\before_skirt_");
            fileName.append(std::to_wstring(this->GetBlockID().m_integerID).c_str());
            fileName.append(L".m");
                LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(fileName, pointsPtr->size(), pointsIndicePtr->size(), &(*pointsPtr)[0], &(*pointsIndicePtr)[0])
            }
#endif

        DTMPtr dtm = this->m_nodeHeader.m_arePoints3d ? Tile3dTM::Create(nodeP).get() : nodeP->GetBcDTM().get();
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
#if SM_TRACE_BUILD_SKIRTS
                for (auto& poly: polyfaces)
                    {
                    if (poly == nullptr) continue;
                    WString fileName(L"E:\\output\\scmesh\\2016-11-30\\skirt_");
                    fileName.append(std::to_wstring(this->GetBlockID().m_integerID).c_str());
                    fileName.append(L"_");
                    fileName.append(std::to_wstring(&poly-&polyfaces[0]).c_str());
                    fileName.append(L".m");
                    bvector<int32_t> idx;
                    for (size_t i = 0; i < poly->GetPointIndexCount(); i+=4)
                        {
                        idx.push_back(poly->GetPointIndexCP()[i] + 1);
                        idx.push_back(poly->GetPointIndexCP()[i+1] + 1);
                        idx.push_back(poly->GetPointIndexCP()[i+2] + 1);
                        }

                    LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(fileName, poly->GetPointCount(), idx.size(), poly->GetPointCP(), &idx[0])
                    }
#endif
                DifferenceSet current = DifferenceSet::FromPolyfaceSet(polyfaces, mapOfPoints, pointsPtr->size() + 1);
                current.clientID = diffSet.clientID;
                current.toggledForID = false;
                //diffSet = current;
                diffsetPtr->Replace(&diffSet - &(*diffsetPtr->begin()), current);
                }
            }   
    }

template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::ClipIntersectsBox(uint64_t clipId, EXTENT ext, Transform tr, bool skirtIntersects)
    {
    DRange3d extRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), 0,
                                       ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), 0);
    bvector<DPoint3d> polyPts;
    SMClipGeometryType geom = SMClipGeometryType::Polygon;
    SMNonDestructiveClipType type;
    bool isActive;
    if(!skirtIntersects)
        GetClipRegistry()->GetClipWithParameters(clipId, polyPts, geom,type, isActive);
    else
    {
        bvector<bvector<DPoint3d>> skirts;
        GetClipRegistry()->GetSkirt(clipId, skirts);
        for (auto&skirt : skirts)
            polyPts.insert(polyPts.end(), skirt.begin(), skirt.end());
    }

    DRange3d transformedExt = ext;
    if (!tr.IsIdentity())
    {
        extRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext),
            ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext));
        bvector<DPoint3d> box(8);
        extRange.Get8Corners(box.data());
        tr.Multiply(&box[0], &box[0], (int)box.size());
        extRange = DRange3d::From(box);
        transformedExt = extRange;
        extRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(extRange), ExtentOp<EXTENT>::GetYMin(extRange), 0,
            ExtentOp<EXTENT>::GetXMax(extRange), ExtentOp<EXTENT>::GetYMax(extRange), 0);
        for (auto& pt : polyPts)
            tr.Multiply(pt, pt);
    }

    if (geom == SMClipGeometryType::BoundedVolume)
    {
        ClipVectorPtr cp;

        GetClipRegistry()->GetClipWithParameters(clipId, cp, geom, type, isActive);
        
        DPoint3d center = DPoint3d::From(transformedExt.low.x + transformedExt.XLength() / 2, transformedExt.low.y + transformedExt.YLength() / 2, transformedExt.low.z + transformedExt.ZLength() / 2);
        double radius = std::max(std::max(transformedExt.XLength(), transformedExt.YLength()), transformedExt.ZLength());

#ifdef VANCOUVER_API
        return cp->SphereInside(center, radius);
#else
        return cp->PointInside(center, radius);
#endif

    }

    DRange3d polyRange;
    size_t n = 0;
    bool noIntersect = true;
    bvector<int> indices;
    bool useX = extRange.XLength() > extRange.YLength();
    DRange1d ext1d = useX ? DRange1d::From(extRange.low.x, extRange.high.x) : DRange1d::From(extRange.low.y, extRange.high.y);
    int sign = -2;
   
    for (auto&pt : polyPts)
        {
        if (extRange.IsContainedXY(pt)) ++n;
        if (n >2) return true;
        if (((useX && ext1d.low > pt.x) || (!useX && ext1d.low > pt.y)) && sign != -1)
        {
            if (abs(sign) <= 1 && sign > -1)
                indices.push_back(&pt - &polyPts[0]);
            sign = -1;

        }
        else if (((useX && (ext1d.high < pt.x)) || (!useX && (ext1d.high < pt.y))) && sign != 1)
        {
            if (abs(sign) <= 1 && sign < 1)
                indices.push_back(&pt - &polyPts[0]);
            sign = 1;
        }
        else if (((useX && ext1d.Contains(pt.x)) || (!useX && ext1d.Contains(pt.y))))
        {
            indices.push_back(&pt - &polyPts[0]);
            sign = 0;
        }
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


    ICurvePrimitivePtr curvePtr;
    if((sign == 0 && indices.size() == 1) || indices.empty())
        curvePtr = ICurvePrimitive::CreateLineString(polyPts);
    else
    {
        if (sign == 0)
            indices.push_back((int)polyPts.size() - 1);
        bvector<DPoint3d> collectedIndices;
        for (auto i: indices)
                collectedIndices.push_back(polyPts[i]);

        curvePtr = ICurvePrimitive::CreateLineString(collectedIndices);
    }
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
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::AddClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn, Transform tr)
    {
    this->m_updateClipTimestamp = std::chrono::system_clock::now().time_since_epoch().count();

    if (this->m_nodeHeader.m_nodeCount == 0 || this->m_nodeHeader.m_nbFaceIndexes < 3) return true;

        {

        DifferenceSet d;
#if DEBUG && SM_TRACE_CLIPS
        std::string s;
        s += " AREA IS" + std::to_string(bsiGeom_getXYPolygonArea(&clipPts[0], (int)clipPts.size()));
#endif
        bool emptyClip = false;
        //if (nodeRange.XLength() <= clipExt.XLength() * 10000 && nodeRange.YLength() <= clipExt.YLength() * 10000)
        if (ClipIntersectsBox(clipId, this->m_nodeHeader.m_contentExtent, tr, !setToggledWhenIdIsOn)) //m_nodeExtent

            {
            bool clipFound = false;

        RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();

        if (!diffSetPtr.IsValid()) return false;

        for (const auto& diffSet : *diffSetPtr) if (diffSet.clientID == clipId && diffSet.toggledForID == setToggledWhenIdIsOn) clipFound = true;
        if (clipFound) return true; //clip already added
           // RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());
            //std::cout << " adding clip " << clipId << " to node " << this->GetBlockID().m_integerID << std::endl;
            d.clientID = clipId;
            d.firstIndex = (int32_t)this->m_nodeHeader.m_nodeCount + 1; //(int32_t)pointsPtr->size() + 1;
            d.toggledForID = setToggledWhenIdIsOn;
            diffSetPtr->push_back(d);
            m_nbClips++;
            const_cast<DifferenceSet&>(*(diffSetPtr->begin() + (diffSetPtr->size() - 1))).upToDate = false;
            for (const auto& other : *diffSetPtr)
                {
                if (other.clientID == ((uint64_t)-1)) const_cast<DifferenceSet&>(other).upToDate = false;
                }

          /*  GetMemoryPool()->RemoveItem(m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex);
            m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;*/
            
            }


        else
            {
            emptyClip = true;
            //std::cout << " discounting clip " << clipId << " for node " << this->GetBlockID().m_integerID << " because test failed " << std::endl;
            }
       // if (d.addedFaces.size() == 0 && d.removedFaces.size() == 0 && d.addedVertices.size() == 0 && d.removedVertices.size() == 0) emptyClip = true;

        //On large datasets doing everything in the main thread is costly, but so would be adding a mutex in AddClip, so not sure what to do with this for now.
       /* if (isVisible && !emptyClip) 
            {
            PropagateClipUpwards(clipId, ClipAction::ACTION_ADD);
            PropagateClipToNeighbors(clipId, ClipAction::ACTION_ADD);
            }
        PropagateClip(clipId, ClipAction::ACTION_ADD);*/
        return !emptyClip;
        }

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::DeleteClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

    if (pointsPtr->size() == 0 || this->m_nodeHeader.m_nbFaceIndexes < 3) return true;
    bool found = false;
    bvector<size_t> indices;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();

    if (!diffSetPtr.IsValid()) return false;

    for (auto it = diffSetPtr->begin(); it != diffSetPtr->end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {
            indices.push_back(it-diffSetPtr->begin());
            m_nbClips--;
            //if (this->m_nodeHeader.m_level < 7) PropagateDeleteClipImmediately(clipId);
            found = true;
            }
        else if (it->clientID == (uint64_t)-1)
            {
            const_cast<DifferenceSet&>(*it).upToDate = false;
            diffSetPtr->SetDirty(true);
            }
        }
    if (found)
        {
        diffSetPtr->erase(indices);
        /* //force commit
         GetMemoryPool()->RemoveItem(m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex);
         m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;*/
        if (m_nbClips == 0)
            {
            ISDiffSetDataStorePtr nodeDiffsetStore;
            bool result = this->m_SMIndex->GetDataStore()->GetSisterNodeDataStore(nodeDiffsetStore, &this->m_nodeHeader, false);
            BeAssert(result == true);
            if (nodeDiffsetStore.IsValid()) nodeDiffsetStore->DestroyBlock(this->GetBlockID());
            }
        }

    return found;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  bool SMMeshIndexNode<POINT, EXTENT>::ModifyClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn, Transform tr)
    {
    //RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(this->GetPointsPtr());

    this->m_updateClipTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
    if (this->m_nodeHeader.m_nodeCount == 0 || this->m_nodeHeader.m_nbFaceIndexes < 3) return true;

    bool found = false;
    RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffSetPtr = GetDiffSetPtr();

    if (!diffSetPtr.IsValid()) return false;

    for (auto it = diffSetPtr->begin(); it != diffSetPtr->end(); ++it)
        {
        if (it->clientID == clipId && it->toggledForID == setToggledWhenIdIsOn)
            {

            //std::cout << " updating clip " << clipId << " to node " << this->GetBlockID().m_integerID << " toggle is "<<setToggledWhenIdIsOn<< std::endl;
            const_cast<DifferenceSet&>(*it) = DifferenceSet();
            const_cast<DifferenceSet&>(*it).clientID = clipId;
            const_cast<DifferenceSet&>(*it).toggledForID = setToggledWhenIdIsOn;
            found = true;
            diffSetPtr->SetDirty(true);
            }

        else if (it->clientID == (uint64_t)-1)
            {
            const_cast<DifferenceSet&>(*it).upToDate = false;
            diffSetPtr->SetDirty(true);
            }

        }
    if (!found)
        {
        found = ClipIntersectsBox(clipId, this->m_nodeHeader.m_contentExtent, tr); //m_nodeExtent
        if (found) AddClip(clipId, isVisible, setToggledWhenIdIsOn, tr);
        }
    else
    {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMin(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMin(this->m_nodeHeader.m_contentExtent),
            ExtentOp<EXTENT>::GetXMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetYMax(this->m_nodeHeader.m_contentExtent), ExtentOp<EXTENT>::GetZMax(this->m_nodeHeader.m_contentExtent));
        bvector<DPoint3d> clipData;
        SMClipGeometryType geom;
        SMNonDestructiveClipType type;
        bool isActive;

        GetClipRegistry()->GetClipWithParameters(clipId, clipData, geom, type, isActive);
        DRange3d clipExtent = DRange3d::From(&clipData[0], (int)clipData.size());

		//Do 2D intersection for unbounded volume like those for road clipping     
        if ((geom != SMClipGeometryType::BoundedVolume && !clipExtent.IntersectsWith(nodeRange, 2)) || 
            (geom == SMClipGeometryType::BoundedVolume && !clipExtent.IntersectsWith(nodeRange)))
            DeleteClip(clipId, isVisible, setToggledWhenIdIsOn);
  /*      //force commit
        GetMemoryPool()->RemoveItem(m_diffSetsItemId, this->GetBlockID().m_integerID, SMStoreDataType::DiffSet, (uint64_t)this->m_SMIndex);
        m_diffSetsItemId = SMMemoryPool::s_UndefinedPoolItemId;*/

    }
    return found;
    }
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateDeleteClipImmediately(uint64_t clipId)
    {
    if (this->HasRealChildren())
        {
        if (this->m_pSubNodeNoSplit != nullptr)
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pSubNodeNoSplit)->DeleteClip(clipId, false);
            }
        else
            {
            for (size_t i = 0; i < this->GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (this->m_apSubNodes[i] != nullptr)
                    {
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_apSubNodes[i])->DeleteClip(clipId, false);
                    }
                }
            }
        }
    auto node = this->GetParentNode();
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
        for (auto& nodeN : this->m_apNeighborNodes[n])
            if (nodeN != nullptr)
                {
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(nodeN)->DeleteClip(clipId, false);
                }
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 09/15
//=======================================================================================
template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateClip(uint64_t clipId, ClipAction action)
    {
    std::vector<SMTask> createdTasks;

    if (this->HasRealChildren())
        {
        //see http://stackoverflow.com/questions/14593995/problems-with-stdfunction for use of std::mem_fn below
        auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*,uint64_t, bool,bool, Transform) >();
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
        if (this->m_pSubNodeNoSplit != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) this->m_pSubNodeNoSplit.GetPtr(), clipId,  false,true,Transform::FromIdentity()),false));
            }
        else
            {
            for (size_t i = 0; i < this->GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (this->m_apSubNodes[i] != nullptr)
                    {
                    createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*) this->m_apSubNodes[i].GetPtr(), clipId, false,true, Transform::FromIdentity()),false));
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
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t,  bool, bool, Transform) >();
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
        for (auto& node : this->m_apNeighborNodes[n])
            if (node != nullptr)
                {
                createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId,  false,true, Transform::FromIdentity()), false));
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
    auto func = std::function < bool(SMMeshIndexNode<POINT, EXTENT>*, uint64_t, bool, bool, Transform) >();
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
    auto node = this->GetParentNode();
    while (node != nullptr && node->GetLevel() > 1)
        {
        node = node->GetParentNode();
        }
    if (node != nullptr)
        {
        createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node.GetPtr(), clipId, false,true, Transform::FromIdentity()), false));
        if (node->GetParentNode() != nullptr)
            {
            createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode().GetPtr(), clipId, false,true, Transform::FromIdentity()), false));

            for (size_t i = 0; i < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); ++i)
                {
                if (node->GetParentNode()->m_apSubNodes[i] != nullptr) createdTasks.push_back(SMTask(std::bind(func, (SMMeshIndexNode<POINT, EXTENT>*)node->GetParentNode()->m_apSubNodes[i].GetPtr(), clipId, false,true, Transform::FromIdentity()), false));
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
        if (!this->IsLoaded())
            Load();
//#ifdef WIP_MESH_IMPORT
        if (m_existingMesh) return true;
//#endif
        bool needsMeshing = false;

        if (!this->HasRealChildren())
            needsMeshing = this->m_nodeHeader.m_nbFaceIndexes == 0;
        else
            {
            if (this->IsParentOfARealUnsplitNode())
                {
                needsMeshing = this->m_nodeHeader.m_nbFaceIndexes == 0 || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*this->m_pSubNodeNoSplit)->NeedsMeshing();
                }
            else
                {
                needsMeshing = this->m_nodeHeader.m_nbFaceIndexes == 0;

                for (size_t indexNode = 0; !needsMeshing && indexNode < this->GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    needsMeshing = (needsMeshing || static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(this->m_apSubNodes[indexNode]))->NeedsMeshing());
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
                                                                             bool PropagatesDataDown,
                                                                             bool needsNeighbors,
                                                                             ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                             ISMPointIndexMesher<POINT, EXTENT>* mesher3d)
                                                                             : SMPointIndex<POINT, EXTENT>(smDataStore, SplitTreshold, filter, balanced, PropagatesDataDown, false), 
                                                                             m_smDataStore(smDataStore),
                                                                             m_smMemoryPool(smMemoryPool)
    {
    m_canUseBcLibClips = true;
    this->m_loadNeighbors = needsNeighbors;
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;    
    m_isInsertingClips = false;

    s_importedFeatures = 0;
    if (this->m_indexHeader.m_rootNodeBlockID.IsValid() && this->m_pRootNode == nullptr)
        {
        this->m_pRootNode = CreateNewNode(this->m_indexHeader.m_rootNodeBlockID);
        }    
    }


template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::~SMMeshIndex()
    {

    if (m_mesher2_5d != NULL)
        delete m_mesher2_5d;

    if (m_mesher3d != NULL)
        delete m_mesher3d;

    try {
        this->Store();
        }
    catch (...)
        {
        assert(!"Cannot store mesh index");
        }

    this->m_createdNodeMap.clear();

    if (this->m_pRootNode != NULL)
        this->m_pRootNode->Unload();

    this->m_pRootNode = NULL;
    }


template <class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(EXTENT extent, bool isRootNode)
    {
    SMMeshIndexNode<POINT, EXTENT> * meshNode = new SMMeshIndexNode<POINT, EXTENT>(this->m_indexHeader.m_SplitTreshold, extent, this, this->m_filter, this->m_needsBalancing, this->IsTextured() != SMTextureType::None, this->PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &this->m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(meshNode);
    pNewNode->m_isGenerating = this->m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }
   

template <class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(uint64_t nodeId, EXTENT extent, bool isRootNode)
    {
    SMMeshIndexNode<POINT, EXTENT> * meshNode = new SMMeshIndexNode<POINT, EXTENT>(nodeId, this->m_indexHeader.m_SplitTreshold, extent, this, this->m_filter, this->m_needsBalancing, this->IsTextured() != SMTextureType::None, this->PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &this->m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(meshNode);
    pNewNode->m_isGenerating = this->m_isGenerating;

    if (isRootNode)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNodePtr;
        pNewNode->SetParentNodePtr(parentNodePtr);
        }

    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID, bool isRootNode, bool useNodeMap)
    {

    if (useNodeMap)
        {
        typename SMPointIndexNode<POINT, EXTENT>::CreatedNodeMap::iterator nodeIter(this->m_createdNodeMap.find(blockID.m_integerID));

        if (nodeIter != this->m_createdNodeMap.end())
            return nodeIter->second;
        }
#ifndef NDEBUG
    else
        {
        typename SMPointIndexNode<POINT, EXTENT>::CreatedNodeMap::iterator nodeIter(this->m_createdNodeMap.find(blockID.m_integerID));
        assert(nodeIter == this->m_createdNodeMap.end());
        }
#endif
    
    HFCPtr<SMMeshIndexNode<POINT, EXTENT>> parent;

    auto meshNode = new SMMeshIndexNode<POINT, EXTENT>(blockID, parent, this, this->m_filter, this->m_needsBalancing, this->IsTextured() != SMTextureType::None, this->PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &this->m_createdNodeMap);
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(meshNode);
    pNewNode->m_isGenerating = this->m_isGenerating;
    pNewNode->m_loadNeighbors = this->m_loadNeighbors;

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

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::CutTiles(uint32_t splitThreshold, Transform unitTransform)
    {
    this->m_indexHeader.m_SplitTreshold = splitThreshold;
    this->m_indexHeaderDirty = true;
    
    if (this->m_pRootNode != NULL)
        {                
        if (s_multiThreadCutting)
            {
            m_creationProcessThreadPoolPtr = WorkerThreadPool::Create(std::thread::hardware_concurrency());
            }

        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->CutTileRecursive(splitThreshold, unitTransform);

        if (s_multiThreadCutting)
            {
            m_creationProcessThreadPoolPtr->Start();
            m_creationProcessThreadPoolPtr->WaitAndStop();
            m_creationProcessThreadPoolPtr = nullptr;
            }                
        }
     
    this->SetupNeighborNodes();

    this->m_indexHeader.m_depth = (size_t)-1;
    this->m_indexHeader.m_depth = this->GetDepth();
    }



template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::TextureFromRaster(ITextureProviderPtr sourceRasterP, Transform unitTransform)
    {
    if (this->m_indexHeader.m_terrainDepth == (size_t)-1)
        {
        this->m_indexHeader.m_terrainDepth = this->m_pRootNode->GetDepth();
        }
    if (sourceRasterP == nullptr) return;

    //compute estimated number of needed texture nodes
    DRange2d rasterBox = sourceRasterP->GetTextureExtent();

    DRange3d ext = this->GetContentExtent();
    //get overlap between node and raster extent
    DRange2d contentExtent = DRange2d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext),
        ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext));

    unitTransform.Multiply(contentExtent.low, contentExtent.low);
    unitTransform.Multiply(contentExtent.high, contentExtent.high);

    DRange2d interExtent;
    if (interExtent.IntersectionOf(rasterBox, contentExtent))
        {
        //we compute an approximation of the number of extra nodes that will be created to obtain a more reliable progress on datasets with more texture
        DPoint2d pixSize = sourceRasterP->GetMinPixelSize();

        float dimensionSize = 1024.0;
        if (dynamic_cast<StreamTextureProvider*>(sourceRasterP.get()))
            {
            dimensionSize = 256.0;
            }

        double neededResolutions = log2(std::max(contentExtent.XLength() / pixSize.x, contentExtent.YLength() / pixSize.y) / dimensionSize);
        int finalDepth = (int)ceil(neededResolutions);
        if (finalDepth > this->m_indexHeader.m_terrainDepth)
            {
            this->m_precomputedCountNodes = true;


            IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
            DRange3d queryExtent = DRange3d::From(DPoint3d::From(interExtent.low.x, interExtent.low.y, ext.low.z), DPoint3d::From(interExtent.high.x, interExtent.high.y, ext.high.z));
            DPoint3d box[8];
            queryExtent.Get8Corners(box);
            ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>(queryExtent, this->GetDepth(), box, params->GetTargetPixelTolerance()));
            std::vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> returnedMeshNodes;
            if (this->Query(meshQueryP, returnedMeshNodes))
                {
                for (auto&node : returnedMeshNodes)
                    {
                    neededResolutions = log2(std::max(node.m_indexNode->m_nodeHeader.m_nodeExtent.XLength() / pixSize.x, node.m_indexNode->m_nodeHeader.m_nodeExtent.YLength() / pixSize.y) / dimensionSize);
                    finalDepth = (int)ceil(neededResolutions);
                    if (finalDepth > 1)
                        this->m_countsOfNodesTotal += pow(4, finalDepth);
                    }
                }
            }
        }

    if (this->m_pRootNode != NULL)
        {
        //With local texture the texturing process is longer in multi-thread (because of the single thread nature of Image++) so keep it in single thread.
        bool isStreamingTexturing = dynamic_cast<StreamTextureProvider*>(sourceRasterP.get()) != nullptr;

        if (s_multiThreadTexturing && isStreamingTexturing)
            {
            m_creationProcessThreadPoolPtr = WorkerThreadPool::Create(std::thread::hardware_concurrency());
            }

        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->TextureFromRasterRecursive(sourceRasterP, unitTransform);

        if (s_multiThreadTexturing && isStreamingTexturing)
            {
            m_creationProcessThreadPoolPtr->Start();
            m_creationProcessThreadPoolPtr->WaitAndStop();
            m_creationProcessThreadPoolPtr = nullptr;
            }
        }    

    this->m_indexHeader.m_depth = (size_t)-1;
    this->m_indexHeader.m_depth = this->GetDepth();
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn, Transform tr)
    {
    size_t nOfNodesTouched = 0;
    if (this->m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->ClipActionRecursive(action, clipId, extent, nOfNodesTouched, setToggledWhenIDIsOn, tr);
    }
   
//#ifdef WIP_MESH_IMPORT
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs, bool isMesh3d)
    {
    if (0 == nPts)
        return;

    // Check if initial node allocated
    if (this->m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (this->m_indexHeader.m_HasMaxExtent)
            this->m_pRootNode = CreateNewNode(this->m_indexHeader.m_MaxExtent);
        else
            this->m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z), true);

        this->m_pRootNode->m_nodeHeader.m_arePoints3d |= isMesh3d;
        }
    size_t nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->AddMeshDefinition(pts, nPts, indices, nIndices, extent, this->m_indexHeader.m_HasMaxExtent, metadata, texData, texSize, uvs, isMesh3d);
    if (0 == nAddedPoints)
        {
        //can't add feature, need to grow extent
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent));
        while (!extent.IsContained(nodeRange))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (this->m_indexHeader.m_HasMaxExtent)
                return;

            // The extent is not contained... we must create a new node
            this->PushRootDown(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
            nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent),
                                       ExtentOp<EXTENT>::GetXMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent));
            }


        // The root node contains the spatial object ... add it
        nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->AddMeshDefinition(pts, nPts, indices, nIndices, extent, this->m_indexHeader.m_HasMaxExtent, metadata, texData, texSize, uvs, isMesh3d);
        assert(nAddedPoints >= nPts);
        }
    }
//#endif

template<class POINT, class EXTENT>  int64_t  SMMeshIndex<POINT, EXTENT>::AddTexture(int width, int height, int nOfChannels, const byte* texData, size_t nOfBytes)
    {

    ISMTextureDataStorePtr nodeDataStore;
    SMIndexNodeHeader<EXTENT> nodeHeader;
    bool result = this->GetDataStore()->GetNodeDataStore(nodeDataStore, &nodeHeader);
    assert(result == true);

    size_t texID = this->GetNextTextureId();

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
    if (this->m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (this->m_indexHeader.m_HasMaxExtent)
            this->m_pRootNode = CreateNewNode(this->m_indexHeader.m_MaxExtent); 
        else
        this->m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z), true);
        }
    size_t nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->AddFeatureDefinition(type, points, extent, this->m_indexHeader.m_HasMaxExtent);
    if (0 == nAddedPoints)
        {
        //can't add feature, need to grow extent
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent));
        while (!extent.IsContained(nodeRange))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (!s_inEditing && this->m_indexHeader.m_HasMaxExtent)
                return;

            // The extent is not contained... we must create a new node
            this->PushRootDown(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
            nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(this->m_pRootNode->m_nodeHeader.m_nodeExtent),
                                       ExtentOp<EXTENT>::GetXMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(this->m_pRootNode->m_nodeHeader.m_nodeExtent));
            }


        // The root node contains the spatial object ... add it
        nAddedPoints = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->AddFeatureDefinition(type, points, extent, this->m_indexHeader.m_HasMaxExtent);
        //assert(nAddedPoints >= points.size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent)
    {
    if (this->m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->AddClipDefinitionRecursive(points,extent);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::RefreshMergedClips()
    {
    if (this->m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode)->RefreshMergedClipsRecursive();
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Elenie.Godzaridis   9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<class POINT, class EXTENT>  size_t  SMMeshIndex<POINT, EXTENT>::GetNextTextureId()
    {
    return ++m_texId;
    }


template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::PropagateFullMeshDown()
    {
    size_t depth = this->GetDepth();
    if (this->m_pRootNode != NULL)
        {
        HFCPtr<SMMeshIndexNode<POINT, EXTENT>> node = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode);
        node->PropagateFullMeshDown(depth);
        }
    }

/**----------------------------------------------------------------------------
Mesh
Mesh the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Mesh(bvector<uint64_t>* pNodesToMesh)
    {
    THIS_HINVARIANTS;

    if (pNodesToMesh == nullptr)
        {
        bool result = m_mesher2_5d->Init(*this);
        assert(result == true);

        result = m_mesher3d->Init(*this);
        assert(result == true);
        }


    // Check if root node is present
    if (this->m_pRootNode != NULL)
        {
        HFCPtr<SMMeshIndexNode<POINT, EXTENT>> node = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(this->m_pRootNode);
            node->Mesh(pNodesToMesh);
        }

    THIS_HINVARIANTS;
    }

/**----------------------------------------------------------------------------
This method saves the node for streaming using the grouping strategy.

@param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::SaveGroupedNodeHeaders(SMNodeGroupPtr pi_pGroup, IScalableMeshProgressPtr progress)
    {
    if (!this->IsLoaded())
        Load();
    bool isClipped = false;
    bool isClipBoundary = pi_pGroup->GetStrategy<EXTENT>()->IsClipBoundary();
    auto clipId = pi_pGroup->GetStrategy<EXTENT>()->GetClipID();
    if (clipId != -1)
        {
        this->ComputeMergedClips();
        if (clipId == 0 || this->HasClip(clipId))
            {
            if (!this->GetDiffSetPtr().IsValid()) return false;

            for (const auto& diffSet : *this->GetDiffSetPtr())
                {
                if (diffSet.clientID == clipId && diffSet.IsEmpty())
                    isClipped = true;
                }
            }
        else if (isClipBoundary)
            {
            isClipped = true;
            }
        }
    auto nodeCount = this->m_nodeHeader.m_nodeCount;
    if (isClipped) this->m_nodeHeader.m_nodeCount = 0;

    bool result = SMPointIndexNode<POINT, EXTENT>::SaveGroupedNodeHeaders(pi_pGroup, progress);

    if (isClipped) this->m_nodeHeader.m_nodeCount = nodeCount;

    return result;
    }

/**----------------------------------------------------------------------------
Publish Cesium ready format
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> StatusInt SMMeshIndex<POINT, EXTENT>::Publish3DTiles(const WString& path, TransformCR transform, ClipVectorPtr clips, const uint64_t& coverageID, const GeoCoordinates::BaseGCSCPtr sourceGCS, bool outputTexture)
    {
    if (this->m_progress != nullptr)
        {
        this->m_progress->ProgressStep() = ScalableMeshStep::STEP_GENERATE_3DTILES_HEADERS;
        this->m_progress->ProgressStepIndex() = 1;
        this->m_progress->Progress() = 0.0f;
        }

    bool isClipBoundary = false;
    if (coverageID != -2 && coverageID != -1)
        {
        if (!static_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->GetRootNode().GetPtr())->GetDiffSetPtr().IsValid()) return ERROR;

        for (const auto& diffSet : *static_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->GetRootNode().GetPtr())->GetDiffSetPtr())
            {
            if (diffSet.clientID == coverageID)
                {
                bvector<bvector<DPoint3d>> polys;
                polys.push_back(bvector<DPoint3d>());
                SMClipGeometryType geom;
                SMNonDestructiveClipType type;
                bool isActive;
                GetClipRegistry()->GetClipWithParameters(diffSet.clientID, polys.back(), geom, type, isActive);
                if (type == SMNonDestructiveClipType::Boundary)
                    {
                    isClipBoundary = true;
                    }
                }
            }
        }

    // Create store for 3DTiles output
    typedef typename SMStreamingStore<EXTENT>::SMStreamingSettings StreamingSettingsType;
	typename SMStreamingStore<EXTENT>::SMStreamingSettingsPtr settings = new StreamingSettingsType();
    settings->m_url = Utf8String(path.c_str());
    settings->m_location = StreamingSettingsType::ServerLocation::LOCAL;
    settings->m_dataType = StreamingSettingsType::DataType::CESIUM3DTILES;
    settings->m_commMethod = StreamingSettingsType::CommMethod::CURL;
    settings->m_isPublishing = true;
    ISMDataStoreTypePtr<EXTENT>     pDataStore(
#ifndef VANCOUVER_API
        new SMStreamingStore<EXTENT>(settings, nullptr)
#else
        SMStreamingStore<EXTENT>::Create(settings, nullptr)
#endif
    );

    // Register 3DTiles index to the store
    pDataStore->Register(this->m_smID);

    // Destination coordinates will be ECEF
    static GeoCoordinates::BaseGCSPtr destinationGCS = GeoCoordinates::BaseGCS::CreateGCS(L"ll84");


    SMIndexMasterHeader<EXTENT> oldMasterHeader;
    this->GetDataStore()->LoadMasterHeader(&oldMasterHeader, sizeof(oldMasterHeader));

    // Force multi file, in case the originating dataset is single file (result is intended for multi file anyway)
    oldMasterHeader.m_singleFile = false;

    DataSource::SessionName dataSourceSessionName(settings->GetGUID().c_str());

    SMGroupGlobalParameters::Ptr groupParameters = SMGroupGlobalParameters::Create(SMGroupGlobalParameters::StrategyType::CESIUM, static_cast<SMStreamingStore<EXTENT>*>(pDataStore.get())->GetDataSourceAccount(), dataSourceSessionName);
    SMGroupCache::Ptr groupCache = nullptr;
    this->m_rootNodeGroup = SMNodeGroup::Create(groupParameters, groupCache, path, 0, nullptr);

    this->m_rootNodeGroup->SetMaxGroupDepth(this->GetDepth() % s_max_group_depth + 1);

    auto strategy = this->m_rootNodeGroup->template GetStrategy<EXTENT>();
    strategy->SetOldMasterHeader(oldMasterHeader);
    strategy->SetClipInfo(coverageID, isClipBoundary);
    strategy->SetSourceAndDestinationGCS(sourceGCS, destinationGCS);
    strategy->AddGroup(this->m_rootNodeGroup.get());

    // Saving groups isn't parallelized therefore we run it in a single separate thread so that we can properly update the listener with the progress
    std::thread saveGroupsThread([this, strategy]()
        {
        this->GetRootNode()->SaveGroupedNodeHeaders(this->m_rootNodeGroup, this->m_progress);

        // Handle all open groups 
        strategy->SaveAllOpenGroups(false/*saveRoot*/);
        if (this->m_progress != nullptr) this->m_progress->Progress() = 1.0f;
        });

    while (this->m_progress != nullptr && !this->m_progress->IsCanceled() && this->m_progress->Progress() != 1.0)
        {
        this->m_progress->UpdateListeners();
        }

    saveGroupsThread.join();

    strategy->Clear();


    if (this->m_progress != nullptr && this->m_progress->IsCanceled()) return SUCCESS;

    if (this->m_progress != nullptr)
        {
        this->m_progress->ProgressStep() = ScalableMeshStep::STEP_CONVERT_3DTILES_DATA;
        this->m_progress->ProgressStepIndex() = 2;
        this->m_progress->Progress() = 0.0f;
        }
    
    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->GetRootNode().GetPtr())->Publish3DTile(pDataStore, transform, clips, coverageID, isClipBoundary, sourceGCS, destinationGCS, this->m_progress, outputTexture);

    if (this->m_progress != nullptr) this->m_progress->Progress() = 1.0f;

    return SUCCESS;
    }

/**----------------------------------------------------------------------------
Publish Cesium ready format
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> StatusInt SMMeshIndex<POINT, EXTENT>::ChangeGeometricError(const WString& path, const bool& pi_pCompress, const double& newGeometricErrorValue)
    {
#ifndef VANCOUVER_API
    ISMDataStoreTypePtr<EXTENT>     pDataStore = new SMStreamingStore<EXTENT>(path, pi_pCompress, false, false, L"data", SMStreamingStore<EXTENT>::FormatType::Cesium3DTiles);

    //this->SaveMasterHeaderToCloud(pDataStore);
    // NEEDS_WORK_SM : publish Cesium 3D tiles tileset

    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->GetRootNode().GetPtr())->ChangeGeometricError(pDataStore, newGeometricErrorValue);

    Json::Value         rootJson;

    rootJson["refine"] = "replace";
    rootJson["geometricError"] = 1.E+06; // What should this value be?
    TilePublisher::WriteBoundingVolume(rootJson, this->GetRootNode()->GetNodeExtent());

    rootJson["content"]["url"] = Utf8String((BeFileName(path) + L"quebeccity.json").c_str());


    return SUCCESS;

#else
    assert(!"Not implemented");
    return ERROR;
#endif
    }

/**----------------------------------------------------------------------------
Save cloud ready format
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> StatusInt SMMeshIndex<POINT, EXTENT>::SaveMeshToCloud(const WString& path, const bool& pi_pCompress)
    {
    assert(false && "Please correct topaz build");
    //NEEDS_WORK_STREAMING: can't use new IRefCounted on vancouver
  #if 0
    ISMDataStoreTypePtr<EXTENT>     pDataStore = new SMStreamingStore<EXTENT>(path, SMStreamingStore<EXTENT>::FormatType::Binary, pi_pCompress);

    this->SaveMasterHeaderToCloud(pDataStore);

    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(this->GetRootNode().GetPtr())->SaveMeshToCloud(pDataStore);
#endif
    return SUCCESS;
    }

template<class POINT, class EXTENT>  int     SMMeshIndex<POINT, EXTENT>::RemoveWithin(ClipVectorCP boundariesToRemoveWithin, const bvector<IScalableMeshNodePtr>& priorityNodes)
    {
    for (auto& node : priorityNodes)
        {
        HFCPtr<SMPointIndexNode<POINT,EXTENT>> nodeP = dynamic_cast<ScalableMeshNode<POINT>*>(node.get())->GetNodePtr();
        ((SMMeshIndexNode<POINT,EXTENT>*)nodeP.GetPtr())->RemoveWithin(boundariesToRemoveWithin);
        }

    DRange3d range;
    boundariesToRemoveWithin->GetRange(range, nullptr);
    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>(
        range, this->GetDepth(), boundariesToRemoveWithin, true));

    vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> returnedMeshNodes;

    if (this->Query(meshQueryP, returnedMeshNodes))
        {
        RefCountedPtr<EditOperation> editDef = EditOperation::Create(EditOperation::Op::REMOVE, boundariesToRemoveWithin);
        for (auto& node : returnedMeshNodes)
            {
            ((SMMeshIndexNode<POINT,EXTENT>*)(&*node.m_indexNode))->AddEdit(editDef);
            }
        m_edits.push_back(editDef);
        }

    return SMStatus::S_SUCCESS;
    }

/**----------------------------------------------------------------------------
Stitch
Stitch the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMMeshIndex<POINT, EXTENT>::Stitch(int pi_levelToStitch, bool do2_5dStitchFirst, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitchInfo)
    {
    THIS_HINVARIANTS;

    // Check if root node is present
    if (this->m_pRootNode != NULL)
        {

        try
            {
            //Just want the node to stitch without doing anything.
            if (nodesToStitchInfo != nullptr)
                { 
                ((SMMeshIndexNode<POINT, EXTENT>*)&*this->m_pRootNode)->Stitch(pi_levelToStitch, nodesToStitchInfo);
                }               
            else
            if (do2_5dStitchFirst)
                {
                //Done for level stitching.
                assert(pi_levelToStitch != -1);
                vector<SMMeshIndexNode<POINT, EXTENT>*> nodesToStitch;
                ((SMMeshIndexNode<POINT, EXTENT>*)&*this->m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);

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
                ((SMMeshIndexNode<POINT, EXTENT>*)&*this->m_pRootNode)->Stitch(pi_levelToStitch, &nodesToStitch);
                if (nodesToStitch.size() == 0) return;
                if (nodesToStitch.size() <= 72)
                    {
                    s_useThreadsInStitching = false;
                    ((SMMeshIndexNode<POINT, EXTENT>*)&*this->m_pRootNode)->Stitch(pi_levelToStitch, 0);
                    s_useThreadsInStitching = true;
                    return;
                    }
                set<SMMeshIndexNode<POINT, EXTENT>*> stitchedNodes;
                std::recursive_mutex stitchedMutex;
                for (auto& node : nodesToStitch)
                    {
                    LightThreadPool::GetInstance()->m_nodeMap[(void*)node] = std::make_shared<std::atomic<unsigned int>>();
                    *LightThreadPool::GetInstance()->m_nodeMap[(void*)node] = -1;
                    }
                for (size_t i =0; i < LightThreadPool::GetInstance()->m_nbThreads; ++i)
                    RunOnNextAvailableThread(std::bind([] (SMMeshIndexNode<POINT, EXTENT>** vec, set<SMMeshIndexNode<POINT,EXTENT>*>* stitchedNodes, std::recursive_mutex* stitchedMutex, size_t nNodes, size_t threadId) ->void
                    {
                    vector<SMMeshIndexNode<POINT, EXTENT>*> myNodes;
                    size_t firstIdx = nNodes / LightThreadPool::GetInstance()->m_nbThreads * threadId;
                    myNodes.push_back(vec[firstIdx]);
                    while (myNodes.size() > 0)
                        {
                        //grab node
                        SMMeshIndexNode<POINT, EXTENT>* current = myNodes[0];
                        vector<SMPointIndexNode<POINT, EXTENT>*> neighbors;
                        current->GetAllNeighborNodes(neighbors);
                        neighbors.push_back(current);
                        bool reservedNode = false;

                        reservedNode = TryReserveNodes(LightThreadPool::GetInstance()->m_nodeMap, (void**)&neighbors[0], neighbors.size(),(unsigned int)threadId);

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
                              if(LightThreadPool::GetInstance()->m_nodeMap.count(current) == 0)
                            LightThreadPool::GetInstance()->m_nodeMap[current] = std::make_shared<std::atomic<unsigned int>>();
                            LightThreadPool::GetInstance()->m_nodeMap[current]->compare_exchange_strong(id, val);
                            }
                        myNodes.erase(myNodes.begin());
                        if (myNodes.size() == 0 && firstIdx + 1 < nNodes / LightThreadPool::GetInstance()->m_nbThreads * (threadId+1))
                            {
                            firstIdx += 1;
                            myNodes.push_back(vec[firstIdx]);
                            }
                        }
                    SetThreadAvailableAsync(threadId);
                    }, &nodesToStitch[0], &stitchedNodes, &stitchedMutex, nodesToStitch.size(), std::placeholders::_1));

                WaitForThreadStop(this->m_progress.get());
                LightThreadPool::GetInstance()->m_nodeMap.clear();
                for (auto& node : nodesToStitch)
                    {
                    if (stitchedNodes.count(node) == 0)
                        node->Stitch((int)node->m_nodeHeader.m_level, 0);
                    }
                }
            else
                {
                ((SMMeshIndexNode<POINT, EXTENT>*)&*this->m_pRootNode)->Stitch(pi_levelToStitch, 0);
                }
            }
        catch (...)
            {
            throw;
            }
        }

    THIS_HINVARIANTS;
    }

template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetClipRegistry(ClipRegistry* clipRegistry)
    {
    m_clipRegistry = clipRegistry;
    }


template<class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>* SMMeshIndex<POINT, EXTENT>::CloneIndex(ISMDataStoreTypePtr<EXTENT> associatedStore)
    {
    SMMeshIndex<POINT, EXTENT>* index = new SMMeshIndex<POINT, EXTENT>(associatedStore, m_smMemoryPool, this->m_indexHeader.m_SplitTreshold, this->m_filter->Clone(),
                                                                       this->m_indexHeader.m_balanced, this->m_indexHeader.m_textured != SMTextureType::None,
                                                                       this->m_propagatesDataDown, this->m_loadNeighbors, m_mesher2_5d, m_mesher3d);
    auto node = this->GetRootNode();
    if (node == nullptr) return index;
    auto rootClone = index->GetRootNode();
    if(rootClone == nullptr) index->CreateRootNode(node->GetBlockID().m_integerID);
    auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(const_cast<SMPointIndexNode<POINT, EXTENT>*>(node.GetPtr()));
    IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
        new ScalableMeshNode<POINT>(nodePtr)
#else
        ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
        );
    dynamic_cast<SMMeshIndexNode<POINT,EXTENT>*>(rootClone.GetPtr())->ImportTreeFrom(nodeP, false, true);
    return index;
    }

#if 0
template<class POINT, class EXTENT> void  SMMeshIndex<POINT, EXTENT>::SetSMTerrain(SMMeshIndex<POINT, EXTENT>* terrainP)
    {
    m_smTerrain = terrainP;
    }
template<class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>*  SMMeshIndex<POINT, EXTENT>::GetSMTerrain()
    {
    return m_smTerrain;
    }


template<class POINT, class EXTENT> void  SMMeshIndex<POINT, EXTENT>::SetSMTerrainMesh(IScalableMesh* terrainP)
    {
    m_smTerrainMesh = terrainP;
    }

template<class POINT, class EXTENT> IScalableMesh*  SMMeshIndex<POINT, EXTENT>::GetSMTerrainMesh()
    {
    return m_smTerrainMesh;

    }
#endif

