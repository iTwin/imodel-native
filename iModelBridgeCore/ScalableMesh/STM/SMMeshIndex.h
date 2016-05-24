
#pragma once
#include <Mtg/MtgStructs.h>
#include "SMPointIndex.h"
#include "Edits/DifferenceSet.h"
//#include "Edits/ClipUtilities.h"
#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>
#include "Threading/ScalableMeshScheduler.h"
#include "Edits\ClipRegistry.h"
#include "InternalUtilityFunctions.h"
#include <ImagePP/all/h/HRARaster.h>
#include <ImagePP/all/h/HIMMosaic.h>

#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <TerrainModel/Core/DTMIterators.h>
#include "Threading\LightThreadPool.h"
#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

#include "SMMemoryPool.h"

extern bool s_useThreadsInStitching;
extern bool s_useThreadsInMeshing;

USING_NAMESPACE_BENTLEY_SCALABLEMESH

extern ScalableMeshScheduler* s_clipScheduler;
extern std::mutex s_schedulerLock;

template<> struct PoolItem<DifferenceSet>
    {
    typedef HPMIndirectCountLimitedPoolItem<DifferenceSet> Type;
    typedef HPMIndirectCountLimitedPool<DifferenceSet> PoolType;
    };

inline void HPMIndirectCountLimitedPoolItem<DifferenceSet>::RecomputeCount() const
    {
    m_deepCount = 0;
    for (size_t i = 0; i < m_count; i++)
        {
        DifferenceSet* set = m_memory + i;
        m_deepCount += sizeof(set) + set->addedFaces.size()*sizeof(DPoint3d) + set->addedVertices.size() * sizeof(int32_t) +
            set->removedFaces.size() * sizeof(int32_t) + set->removedVertices.size() * sizeof(int32_t);
        }
    }

template<class POINT, class EXTENT> class ISMPointIndexMesher;
template<class POINT, class EXTENT> class ISMMeshIndexFilter;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
enum ClipAction
    {
    ACTION_ADD = 0,
    ACTION_DELETE,
    ACTION_MODIFY
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE

//extern size_t nGraphPins;
//extern size_t nGraphReleases;

template<class POINT, class EXTENT> class SMMeshIndex;

template <class POINT, class EXTENT> class SMMeshIndexNode : public SMPointIndexNode < POINT, EXTENT >
    {
    friend class ISMPointIndexMesher<POINT, EXTENT>;
    friend class SMMeshIndex < POINT, EXTENT > ;
    public:
        SMMeshIndexNode(size_t pi_SplitTreshold,
                        const EXTENT& pi_rExtent,                                                
                        SMMeshIndex<POINT, EXTENT>* meshIndex,
                        ISMPointIndexFilter<POINT, EXTENT>* filter,
                        bool balanced,
                        bool textured,
                        bool propagateDataDown,
                        ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                        ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                        CreatedNodeMap*                      createdNodeMap);

    SMMeshIndexNode(size_t pi_SplitTreshold,
                                     const EXTENT& pi_rExtent,
                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode);

    SMMeshIndexNode(size_t pi_SplitTreshold,
                                     const EXTENT& pi_rExtent,
                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                     bool IsUnsplitSubLevel);

    SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode);

    SMMeshIndexNode(const SMPointIndexNode<POINT, EXTENT>& pi_rNode);

    SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode,
                     const HFCPtr<SMMeshIndexNode>& pi_rpParentNode);

    SMMeshIndexNode(HPMBlockID blockID,
                     HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,                                            
                      SMMeshIndex<POINT, EXTENT>* meshIndex,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool textured,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap*                      createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,                                            
                      SMMeshIndex<POINT, EXTENT>* meshIndex,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool textured,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap* createdNodeMap);

    SMMeshIndexNode(size_t pi_SplitTreshold,
                      const EXTENT& pi_rExtent,                                            
                      SMMeshIndex<POINT, EXTENT>* meshIndex,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap*                      createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,                                          
                     SMMeshIndex<POINT, EXTENT>* meshIndex,
                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                     bool balanced,
                     bool propagateDataDown,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                     CreatedNodeMap* createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,
                     HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,                                          
                     SMMeshIndex<POINT, EXTENT>* meshIndex,
                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                     bool balanced,
                     bool propagateDataDown,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                     CreatedNodeMap*                      createdNodeMap);

    virtual ~SMMeshIndexNode<POINT, EXTENT>();
    
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChildVirtual() const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewChildNode(HPMBlockID blockID);
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false) override;

    virtual bool Discard() override;    
    virtual bool Destroy();


    virtual void Load() const override;

    virtual void Unload() override;

    virtual bool IsGraphLoaded() const;

    void CreateGraph(bool shouldPinGraph = false) const;

    virtual void LoadGraph(bool shouldPinGraph=false) const;


    void ReleaseGraph()
        {
 //       nGraphReleases++;
 //       m_graphVec.UnPin();
        }

    void LockGraph()
        {
        m_graphMutex.lock();
        }
    void UnlockGraph()
        {
        m_graphMutex.unlock();
        }

    void StoreGraph() const;

    void StoreAllGraphs();

   /* virtual MTGGraph* GetGraphPtr()
        {
        if (m_graphVec.size() == 0 || m_graphVec.Discarded()) return NULL;
        else return const_cast<MTGGraph*>(&*m_graphVec.begin());
        }*/

    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> GetGraphPtr(bool loadGraph = true)
        {
        std::lock_guard<std::mutex> lock(m_graphMutex); //don't want to add item twice
        RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> poolMemItemPtr;


        if (!SMMemoryPool::GetInstance()->GetItem<MTGGraph>(poolMemItemPtr, m_graphPoolItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::Graph) && loadGraph)
            {
            //NEEDS_WORK_SM : SharedPtr for GetPtsIndiceStore().get()            
            RefCountedPtr<SMStoredMemoryPoolGenericBlobItem<MTGGraph>> storedMemoryPoolItem(new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(GetBlockID().m_integerID, GetGraphStore().GetPtr(), SMPoolDataTypeDesc::Graph));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
            m_graphPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
            assert(m_graphPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemItemPtr = storedMemoryPoolItem.get();
            }

        return poolMemItemPtr;
        }


    virtual RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> GetDiffSetPtr() const
        {
        RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> poolMemItemPtr;


        if (!SMMemoryPool::GetInstance()->GetItem<DifferenceSet>(poolMemItemPtr, m_diffSetsItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::DiffSet))
            {
            //NEEDS_WORK_SM : SharedPtr for GetPtsIndiceStore().get()            
            RefCountedPtr<SMStoredMemoryPoolGenericVectorItem<DifferenceSet>> storedMemoryPoolItem(new SMStoredMemoryPoolGenericVectorItem<DifferenceSet>(GetBlockID().m_integerID, dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipStore().GetPtr(), SMPoolDataTypeDesc::DiffSet));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
            m_diffSetsItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
            assert(m_diffSetsItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemItemPtr = storedMemoryPoolItem.get();
            }

        return poolMemItemPtr;
        }
    /*void SetGraphDirty()
        {
        //m_graphVec.RecomputeCount();
        m_graphVec.SetDiscarded(false);
        m_graphVec.SetDirty(true);
        }

    bool IsGraphDirty()
        {
        return m_graphVec.IsDirty();
        }

    void PinGraph()
    {
        m_graphVec.Pin();
    }*/
    /**----------------------------------------------------------------------------
    Returns the 2.5d mesher used for meshing the points

    @return Pointer to mesher or NULL if none is set.
    -----------------------------------------------------------------------------*/
    ISMPointIndexMesher<POINT, EXTENT>*
        GetMesher2_5d() const;

    /**----------------------------------------------------------------------------
    Returns the 3d mesher used for meshing the points

    @return Pointer to mesher or NULL if none is set.
    -----------------------------------------------------------------------------*/
    ISMPointIndexMesher<POINT, EXTENT>*
        GetMesher3d() const;


    /**----------------------------------------------------------------------------
    Initiates the meshing of the node.
    -----------------------------------------------------------------------------*/
    virtual void Mesh();

    void                SetClipRegistry(HFCPtr<ClipRegistry>& registry)
        {
        m_clipRegistry = registry;
        }


    void UpdateFromGraph(MTGGraph * graph, bvector<DPoint3d>& pointList);

    void CollectFeatureDefinitionsFromGraph(MTGGraph* graph, size_t maxPtID);

    void SplitNodeBasedOnImageRes();
    void SplitMeshForChildNodes();

    //NEEDS_WORK_SM: refactor all meshIndex recursive calls into something more like a visitor pattern
    //NEEDS_WORK_SM: move clip and raster support to point index

    void                TextureFromRaster(HIMMosaic* sourceRasterP);
    void                TextureFromRasterRecursive(HIMMosaic* sourceRasterP);

    void                  ReadFeatureDefinitions(bvector<bvector<DPoint3d>>& points, bvector<DTMFeatureType> & types);

    size_t                AddFeatureDefinitionSingleNode(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinitionUnconditional(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed);

    //NEEDS_WORK_SM: clean up clipping API (remove extra calls, clarify uses, etc)

    //Synchronously add clip on this node and all descendants, used in generation.
    void                AddClipDefinitionRecursive(bvector<DPoint3d>& points, DRange3d& extent);
    //Synchronously update clips, if necessary, so as to merge them with other clips applied on the node, for this node and all descendants, if applicable. Used in generation.
    void                RefreshMergedClipsRecursive();

    //Checks whether clip should apply to node and update lists accordingly
    void                ClipActionRecursive(ClipAction action,uint64_t clipId, DRange3d& extent, bool setToggledWhenIdIsOn = true);

    ClipRegistry* GetClipRegistry() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipRegistry();
        }

    void  BuildSkirts();

    bool HasClip(uint64_t clipId);

    bool IsClippingUpToDate();

    //If necessary, update clips so as to merge them with other clips on the node.
    void  ComputeMergedClips();
    //Adds a new set of differences matching the desired clip (synchronously).
    //The base mesh is retained and the clip is a non-destructive modification.
    //Caller provides desired ID for the new clip. Returns true if clip was added, false if clip is out of bounds.
    bool  AddClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn = true);

    //Deletes an existing clip or returns false if there is no clip with this ID.
    bool  DeleteClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn=true);

    //Modifies an existing clip or returns false if there is no clip with this ID.
    bool ModifyClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn = true);

    //Requests adding a clip asynchronously. ID is set but diffset is filled later.
    bool  AddClipAsync(uint64_t clipId, bool isVisible);

    //Completes an async clip operation.
    void DoClip(uint64_t clipId, bool isVisible);

    const DifferenceSet& GetClipSet(size_t index) const
        {
        RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> diffset = GetDiffSetPtr();
        return (*diffset.get())[index];
        }


    //Propagation of clips. Requests application of clip operations to descendants, usually asynchronously.
    void PropagateDeleteClipImmediately(uint64_t clipId);

    void PropagateClip(uint64_t clipId,  ClipAction action);

    void PropagateClipToNeighbors(uint64_t clipId, ClipAction action);

    void PropagateClipUpwards(uint64_t clipId, ClipAction action);

    virtual void OnPropagateDataDown() override;
    virtual void OnPushNodeDown() override;

    void PropagateFeaturesToChildren();

    size_t CountAllFeatures();

    /**----------------------------------------------------------------------------
    Initiates the mesh stitching of the node.
    -----------------------------------------------------------------------------*/
    virtual void Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch);

    HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> GetGraphStore() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetGraphStore();
        };

    HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > GetGraphPool() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetGraphPool();
        };
        
    void PushPtsIndices(const int32_t* indices, size_t size);

    void ReplacePtsIndices(const int32_t* indices, size_t size);

    void ClearPtsIndices();                
    
    virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetPtsIndicePtr()
        {
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;
                
        if (!GetMemoryPool()->GetItem<int32_t>(poolMemVectorItemPtr, m_triIndicesPoolItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::TriPtIndices))
            {                  
            //NEEDS_WORK_SM : SharedPtr for GetPtsIndiceStore().get()
            RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(new SMStoredMemoryPoolVectorItem<int32_t>(GetBlockID().m_integerID, GetPtsIndiceStore().GetPtr(), SMPoolDataTypeDesc::TriPtIndices));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
            m_triIndicesPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(m_triIndicesPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemVectorItemPtr = storedMemoryPoolVector.get();            
            }

        return poolMemVectorItemPtr;
        }
        
    SMMemoryPoolPtr GetMemoryPool() const
        {
        return SMMemoryPool::GetInstance();
        //NEEDS_WORK_SM : No one should have a pointer to the memory pool.
        //return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetMemoryPool();
        }

    HFCPtr<SMPointTileStore<int32_t, EXTENT>> GetPtsIndiceStore() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetPtsIndicesStore();
        }

    void         SaveCloudReadyNode(HFCPtr<StreamingPointStoreType> pi_pPointStore,
                                    HFCPtr<StreamingIndiceStoreType> pi_pIndiceStore,
                                    HFCPtr<StreamingUVStoreType> pi_pUVStore,
                                    HFCPtr<StreamingIndiceStoreType> pi_pUVIndiceStore,
                                    HFCPtr<StreamingTextureTileStoreType> pi_pTextureStore) override;

#ifdef INDEX_DUMPING_ACTIVATED
    virtual void         DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                                         bool pi_OnlyLoadedNode) const override;
#endif       
   
    // The byte array starts with three integers specifying the width/heigth in pixels, and the number of channels
    void PushTexture(const Byte* texture, size_t size);              

    virtual RefCountedPtr<SMMemoryPoolBlobItem<Byte>> GetTexturePtr()
        {
        RefCountedPtr<SMMemoryPoolBlobItem<Byte>> poolMemBlobItemPtr;

        if (!IsTextured())
            return poolMemBlobItemPtr;
                             
        if (!GetMemoryPool()->GetItem<Byte>(poolMemBlobItemPtr, m_texturePoolItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::Texture))
            {                              
            RefCountedPtr<SMStoredMemoryPoolBlobItem<Byte>> storedMemoryPoolVector(new SMStoredMemoryPoolBlobItem<Byte>(GetBlockID().m_integerID, GetTextureStore().GetPtr(), SMPoolDataTypeDesc::Texture));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
            m_texturePoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(m_texturePoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemBlobItemPtr = storedMemoryPoolVector.get();            
            }

        return poolMemBlobItemPtr;
        }

    HFCPtr<IScalableMeshDataStore<Byte, float, float>> GetTextureStore() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetTexturesStore();
        }
                    
    void PushUV(const DPoint2d* points, size_t size);
       
    virtual RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> GetUVCoordsPtr()
        {
        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> poolMemVectorItemPtr;

        if (!IsTextured())
            return poolMemVectorItemPtr;
                
        if (!GetMemoryPool()->GetItem<DPoint2d>(poolMemVectorItemPtr, m_uvCoordsPoolItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::UvCoords))
            {                  
            //NEEDS_WORK_SM : SharedPtr for GetPtsIndiceStore().get()
            RefCountedPtr<SMStoredMemoryPoolVectorItem<DPoint2d>> storedMemoryPoolVector(new SMStoredMemoryPoolVectorItem<DPoint2d>(GetBlockID().m_integerID, GetUVStore().GetPtr(), SMPoolDataTypeDesc::UvCoords));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
            m_uvCoordsPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(m_uvCoordsPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemVectorItemPtr = storedMemoryPoolVector.get();            
            }

        return poolMemVectorItemPtr;
        }           

    HFCPtr<SMPointTileStore<DPoint2d, EXTENT>> GetUVStore() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVStore();
        }
            
    void PushUVsIndices(size_t texture_id, const int32_t* uvsIndices, size_t size);
     
    virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetUVsIndicesPtr()
        {
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> poolMemVectorItemPtr;

        if (!IsTextured())
            return poolMemVectorItemPtr;
                
        if (!GetMemoryPool()->GetItem<int32_t>(poolMemVectorItemPtr, m_triUvIndicesPoolItemId, GetBlockID().m_integerID, SMPoolDataTypeDesc::TriUvIndices))
            {                  
            //NEEDS_WORK_SM : SharedPtr for GetPtsIndiceStore().get()
            RefCountedPtr<SMStoredMemoryPoolVectorItem<int32_t>> storedMemoryPoolVector(new SMStoredMemoryPoolVectorItem<int32_t>(GetBlockID().m_integerID, GetUVsIndicesStore().GetPtr(), SMPoolDataTypeDesc::TriUvIndices));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
            m_triUvIndicesPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(m_triUvIndicesPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemVectorItemPtr = storedMemoryPoolVector.get();            
            }

        return poolMemVectorItemPtr;
        }    
        
    HFCPtr<SMPointTileStore<int32_t, EXTENT>> GetUVsIndicesStore() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetUVsIndicesStore();
        }

    /**----------------------------------------------------------------------------
    Indicates if the node need meshing. The fact it is meshed is not
    sufficient not to need meshing, if any sub-node need meshing then
    it also required re-meshing.
    -----------------------------------------------------------------------------*/
    virtual bool NeedsMeshing() const;


#ifdef ACTIVATE_TEXTURE_DUMP
    void                DumpAllNodeTextures()
        {        
        RefCountedPtr<SMMemoryPoolBlobItem<Byte>> texturePtr(GetTexturePtr());
        ScalableMeshTexturePtr smTexturePtr(ScalableMeshTexture::Create(this));

        if (texturePtr == nullptr)
            return;            

        WString fileName = L"file://";
        fileName.append(L"e:\\output\\scmesh\\2015-11-19\\texture_");
        //fileName.append(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\QuebecCityMini\\DumpedTextures\\texture_");
        fileName.append(std::to_wstring(m_nodeHeader.m_level).c_str());
        fileName.append(L"_");
        fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)).c_str());
        fileName.append(L"_");
        fileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)).c_str());
        fileName.append(L".bmp");
        HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
        HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());        
        HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
        smTexturePtr->GetDimension().x,
        smTexturePtr->GetDimension().y,
        pImageDataPixelType,
        smTexturePtr->GetData());
        
        if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
            {
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->DumpAllNodeTextures();
            }
        else if (!IsLeaf())
            {
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                if (m_apSubNodes[indexNodes] != nullptr)
                    {
                    auto mesh = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes]);
                    assert(mesh != nullptr);
                    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->DumpAllNodeTextures();
                    }
                }
            }
        }
#endif

    mutable vector<HPMStoredPooledVector<int32_t>> m_featureDefinitions;
    atomic<size_t> m_nbClips;
    BcDTMPtr m_tileBcDTM;
    std::mutex m_dtmLock;
    mutable SMMemoryPoolItemId m_graphPoolItemId;
    private:

        bool ClipIntersectsBox(uint64_t clipId, EXTENT ext);

        //mutable HPMStoredPooledVector<DifferenceSet> m_differenceSets;
        //mutable HPMStoredPooledVector<MTGGraph> m_graphVec;
        mutable std::mutex m_graphInflateMutex;
        mutable std::mutex m_graphMutex;
        mutable SMMemoryPoolItemId m_triIndicesPoolItemId;        
        mutable SMMemoryPoolItemId m_texturePoolItemId;                        
        mutable SMMemoryPoolItemId m_triUvIndicesPoolItemId;                
        mutable SMMemoryPoolItemId m_uvCoordsPoolItemId;       
        mutable SMMemoryPoolItemId m_diffSetsItemId;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;
       // mutable bool m_isGraphLoaded;                
        HFCPtr<ClipRegistry> m_clipRegistry;
        mutable std::mutex m_headerMutex;
    };


    template<class POINT, class EXTENT> class SMMeshIndex : public SMPointIndex < POINT, EXTENT >
    {
    friend class SMMeshIndexNode < POINT, EXTENT > ;
    public:
        SMMeshIndex(SMMemoryPoolPtr& smMemoryPool,                         
                     HFCPtr<SMPointTileStore<POINT, EXTENT> > ptsStore,                      
                     HFCPtr<SMPointTileStore<int32_t, EXTENT>> ptsIndiceStore,
                     HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                     HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> graphStore,                     
                     HFCPtr<IScalableMeshDataStore<Byte, float, float> > textureStore,                     
                     HFCPtr<SMPointTileStore<DPoint2d, EXTENT> > uvStore,                     
                     HFCPtr<SMPointTileStore<int32_t, EXTENT> > uvsIndicesStore,
                     size_t SplitTreshold, 
                     ISMPointIndexFilter<POINT, EXTENT>* filter, 
                     bool balanced, 
                     bool textured,
                     bool propagatesDataDown, 
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d, 
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d);
        virtual             ~SMMeshIndex<POINT, EXTENT>();


        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher2_5d();

        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher3d();


        virtual void        Mesh();

        virtual void        GetCloudFormatStores(const WString& pi_pOutputDirPath,
                                                 const bool& pi_pCompress,
                                                 HFCPtr<StreamingPointStoreType>& po_pPointStore,
                                                 HFCPtr<StreamingIndiceStoreType>& po_pIndiceStore,
                                                 HFCPtr<StreamingUVStoreType>& po_pUVStore,
                                                 HFCPtr<StreamingIndiceStoreType>& po_pUVIndiceStore,
                                                 HFCPtr<StreamingTextureTileStoreType>& po_pTextureStore) const override;

        virtual void        Stitch(int pi_levelToStitch, bool do2_5dStitchFirst = false);

        void                SetFeatureStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& featureStore);
        void                SetFeaturePool(HFCPtr<HPMCountLimitedPool<int32_t>>& featurePool);
        void                SetClipStore(HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>>& clipStore);
        void                SetClipRegistry(ClipRegistry* registry);

        void                SetPtsIndicesStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& ptsIndicesStore);
        void                SetGraphStore(HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>>& graphStore);
        void                SetGraphPool(HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>>& graphPool);
        void                SetTexturesStore(HFCPtr<IScalableMeshDataStore<Byte, float, float>>& texturesStore);
        void                SetTexturesPool(HFCPtr<HPMCountLimitedPool<Byte>>& texturesPool);
        void                SetUVStore(HFCPtr<SMPointTileStore<DPoint2d, EXTENT>>& uvStore);        
        void                SetUVsIndicesStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& uvsIndicesStore);        

        SMMemoryPoolPtr GetMemoryPool() const { return m_smMemoryPool; }        

        HFCPtr<SMPointTileStore<int32_t, EXTENT>> GetPtsIndicesStore() const { return m_ptsIndicesStore; }
        HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> GetGraphStore() const { return m_graphStore; }
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> GetGraphPool() const { return m_graphPool; }
        HFCPtr<IScalableMeshDataStore<Byte, float, float>> GetTexturesStore() const { return m_texturesStore; }
        HFCPtr<SMPointTileStore<DPoint2d, EXTENT>> GetUVStore() const { return m_uvStore; }        
        HFCPtr<SMPointTileStore<int32_t, EXTENT>> GetUVsIndicesStore() const { return m_uvsIndicesStore; }
        
        HFCPtr<SMPointTileStore<int32_t, EXTENT>> GetFeatureStore() { return m_featureStore; }
        HFCPtr<HPMCountLimitedPool<int32_t>> GetFeaturePool() { return m_featurePool; }

        ClipRegistry* GetClipRegistry()
            {
            return m_clipRegistry.GetPtr();
            }

        HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>>& GetClipStore()
            {
            return m_clipStore;
            }
        HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>> GetClipPool() const { return m_clipPool; }
        void                SetClipPool(HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>>& clipPool);

        //IDTMFile::FeatureType is the same as DTMFeatureType defined in TerrainModel.h.
        void                AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);

        void                AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent);
        void                PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn=true);
        void                RefreshMergedClips();


        void                TextureFromRaster(HIMMosaic* sourceRasterP);
#ifdef ACTIVATE_TEXTURE_DUMP
        void                DumpAllNodeTextures()
            {
            if (m_pRootNode != 0) dynamic_cast<SMMeshIndexNode<POINT,EXTENT>*>(m_pRootNode.GetPtr())->DumpAllNodeTextures();
            }
#endif

        /**----------------------------------------------------------------------------
        Initiates the filtering of the point index, This filtering process includes
        recursively calling global pre-filtering, pre=filtering of nodes, filtering,
        node post-filtering then global post-filtering.

        -----------------------------------------------------------------------------*/
       // virtual void        Filter(int pi_levelToFilter);

        //NEEDS_WORK_SM : Why the same 2 functions in point index?
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(EXTENT extent, bool isRootNode = false);        
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false);

    private:
        
        SMMemoryPoolPtr m_smMemoryPool;
        
        HFCPtr<SMPointTileStore<int32_t, EXTENT> > m_ptsIndicesStore;
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> m_graphPool;
        HFCPtr<IScalableMeshDataStore<MTGGraph, Byte, Byte>> m_graphStore;        
        HFCPtr<IScalableMeshDataStore<Byte, float, float> > m_texturesStore;       
        HFCPtr<SMPointTileStore<DPoint2d, EXTENT> > m_uvStore;        
        HFCPtr<SMPointTileStore<int32_t, EXTENT> > m_uvsIndicesStore;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;
        HFCPtr<SMPointTileStore<int32_t, EXTENT>> m_featureStore;
        HFCPtr<HPMCountLimitedPool<int32_t>> m_featurePool;
        HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>> m_clipStore;
        HFCPtr<ClipRegistry> m_clipRegistry;
        HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>> m_clipPool;

        std::vector<std::future<bool>> m_textureWorkerTasks;

    };

        template <class POINT, class EXTENT> class SMIndexNodeVirtual<POINT, EXTENT, SMMeshIndexNode<POINT, EXTENT>> : public SMMeshIndexNode<POINT, EXTENT>
        {
        public:
            SMIndexNodeVirtual(const SMMeshIndexNode<POINT, EXTENT>* rParentNode) : SMMeshIndexNode<POINT, EXTENT>(rParentNode->GetSplitTreshold(), rParentNode->GetNodeExtent(), const_cast<SMMeshIndexNode<POINT, EXTENT>*>(rParentNode))
            {
            m_nodeHeader.m_contentExtent = rParentNode->m_nodeHeader.m_contentExtent;
            m_nodeHeader.m_nodeExtent = rParentNode->m_nodeHeader.m_nodeExtent;
            m_nodeHeader.m_numberOfSubNodesOnSplit = rParentNode->m_nodeHeader.m_numberOfSubNodesOnSplit;
            m_nodeHeader.m_totalCount = rParentNode->m_nodeHeader.m_totalCount;
            m_nodeHeader.m_IsLeaf = true;
            m_nodeHeader.m_IsBranched = false;
            m_nodeHeader.m_IsUnSplitSubLevel = true;
            m_nodeHeader.m_contentExtentDefined = true;
            }
        virtual bool IsGraphLoaded() const override
            {
            return (dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetParentNodePtr().GetPtr()))->IsGraphLoaded();
            };

        virtual void LoadGraph(bool shouldPinGraph = false) const override
            {
            return dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetParentNodePtr().GetPtr())->LoadGraph();
            };
        virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> GetGraphPtr(bool loadGraph = true) override
            {
            return dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetParentNodePtr().GetPtr())->GetGraphPtr(loadGraph);
            };
        virtual bool IsVirtualNode() const override
            {
            return true;
            }
        
        virtual RefCountedPtr<SMMemoryPoolVectorItem<POINT>> GetPointsPtr(bool loadPts = true) override 
            {
            return GetParentNodePtr()->GetPointsPtr(loadPts);
            }        

        virtual bool Destroy() override
            {
            return false;
            };

        virtual void Load() const override
            {};


        virtual bool Store() override
            {
            return false;
            };

        virtual bool Discard() override
            {
            return false;
            };
        };

template<class POINT, class EXTENT> class ISMPointIndexMesher
    {
    public:
        ISMPointIndexMesher() {};
        virtual ~ISMPointIndexMesher() {};

        /*
        virtual bool        Mesh (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
        size_t numSubNodes) const = 0;
        */

        virtual bool        Init(const SMMeshIndex<POINT, EXTENT>& pointIndex) { return true; }

        virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const = 0;

        virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const = 0;

    };

/*
* See ISMPointIndexFilter for API doc.
*/
template<class POINT, class EXTENT> class ISMMeshIndexFilter : public ISMPointIndexFilter<POINT,EXTENT>
    {
    public:
        ISMMeshIndexFilter() {};
        virtual ~ISMMeshIndexFilter() {};

        virtual bool        IsProgressiveFilter() const = 0;

        virtual bool        ImplementsPreFiltering()
            {
            return false;
            };

        virtual bool        ImplementsPostFiltering()
            {
            return false;
            };

        virtual bool        GlobalPreFilter(SMPointIndex<POINT, EXTENT>& index)
            {
            return true;
            };

        virtual bool        PreFilter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                      vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                      size_t numSubNodes)
            {
            return false;
            };


        virtual bool        Filter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                   size_t numSubNodes) const = 0;
        virtual bool        FilterLeaf(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node) const 
            {
            return false;
            }


        virtual bool        PostFilter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                       vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                       size_t numSubNodes)
            {
            return false;
            };

        virtual bool        GlobalPostFilter(SMPointIndex<POINT, EXTENT>& index)
            {
            return true;
            };

    };

inline bool IsVoidFeature(IDTMFile::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Hole || dtmType == DTMFeatureType::Void || dtmType == DTMFeatureType::BreakVoid ||
        dtmType == DTMFeatureType::DrapeVoid;
    }

//#include "SMMeshIndex.hpp"
