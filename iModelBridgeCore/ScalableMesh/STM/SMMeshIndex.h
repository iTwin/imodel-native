
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
#include "Stores\SMSQLiteStore.h"

#include <ScalableMesh\IScalableMeshProgressiveQuery.h>

namespace BENTLEY_NAMESPACE_NAME
    {
    namespace ScalableMesh
        {
        class ScalableMeshTexture;
        }
    }


extern bool s_useThreadsInStitching;
extern bool s_useThreadsInMeshing;
extern bool s_useThreadsInTexturing;

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

struct SmCachedDisplayData 
    {
    private : 

        SmCachedDisplayMesh*                m_cachedDisplayMesh;
        SmCachedDisplayTexture*             m_cachedDisplayTexture;
        IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
        size_t                              m_memorySize;
        bvector<uint64_t>                   m_appliedClips; 

    public : 
    
        SmCachedDisplayData(SmCachedDisplayMesh*                 cachedDisplayMesh,
                            SmCachedDisplayTexture*              cachedDisplayTexture,
                            IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, 
                            size_t                               memorySize, 
                            const bvector<uint64_t>&             appliedClips)
            {
            m_cachedDisplayMesh = cachedDisplayMesh;
            m_cachedDisplayTexture = cachedDisplayTexture; 
            m_displayCacheManagerPtr = displayCacheManagerPtr;
            m_memorySize = memorySize;
            m_appliedClips.insert(m_appliedClips.end(), appliedClips.begin(), appliedClips.end());
            }

        virtual ~SmCachedDisplayData()
            {
            if (m_cachedDisplayMesh != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedMesh(m_cachedDisplayMesh); 
                assert(status == SUCCESS);                    
                }

            if (m_cachedDisplayTexture != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedTexture(m_cachedDisplayTexture); 
                assert(status == SUCCESS);                    
                }
            }

        size_t GetMemorySize() const
            {
            return m_memorySize;
            }                

        SmCachedDisplayMesh* GetCachedDisplayMesh() const
            {
            return m_cachedDisplayMesh; 
            }

        SmCachedDisplayTexture* GetCachedDisplayTexture() const
            {
            return m_cachedDisplayTexture;
            }

        const bvector<uint64_t>& GetAppliedClips()
            {
            return m_appliedClips;
            }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

//extern size_t nGraphPins;
//extern size_t nGraphReleases;

inline bool IsLinearFeature(ISMStore::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Breakline || dtmType == DTMFeatureType::SoftBreakline || dtmType == DTMFeatureType::ContourLine || dtmType == DTMFeatureType::GraphicBreak;
    }

inline bool IsClosedFeature(ISMStore::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Hole || dtmType == DTMFeatureType::Island || dtmType == DTMFeatureType::Void || dtmType == DTMFeatureType::BreakVoid ||
        dtmType == DTMFeatureType::Polygon || dtmType == DTMFeatureType::Region || dtmType == DTMFeatureType::Contour || dtmType == DTMFeatureType::Hull ||
        dtmType == DTMFeatureType::DrapeVoid;
    }

template<class POINT, class EXTENT> class SMMeshIndex;

template <class POINT, class EXTENT> class SMMeshIndexNode : public SMPointIndexNode < POINT, EXTENT >
    {
    friend class ISMPointIndexMesher<POINT, EXTENT>;
    friend class SMMeshIndex < POINT, EXTENT > ;
    public:

           
    SMMeshIndexNode(size_t pi_SplitTreshold,
                                     const EXTENT& pi_rExtent,
                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode);
    
    SMMeshIndexNode(size_t pi_SplitTreshold,
                                     const EXTENT& pi_rExtent,
                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                     bool IsUnsplitSubLevel);

    SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode);    

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

    void CreateGraph(bool shouldPinGraph = false) const {}

    virtual void LoadGraph(bool shouldPinGraph = false) const {}


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

    ISMMTGGraphDataStorePtr GetGraphStore() const;
        
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> GetGraphPtr(bool loadGraph = true);       

    virtual RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> GetDiffSetPtr() const;
        
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> GetTileDTM()
        {
        std::lock_guard<std::mutex> lock(m_dtmLock); //don't want to add item twice
        RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> poolMemItemPtr;


        if (!SMMemoryPool::GetInstance()->GetItem<BcDTMPtr>(poolMemItemPtr, m_dtmPoolItemId, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex))
            {
            RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> storedMemoryPoolItem(new SMMemoryPoolGenericBlobItem<BcDTMPtr>(nullptr, 0, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
            m_dtmPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
            assert(m_dtmPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemItemPtr = storedMemoryPoolItem.get();

            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
            IScalableMeshNodePtr nodeP(new ScalableMeshNode<POINT>(nodePtr));
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

    void UpdateNodeFromBcDTM();

#ifdef WIP_MESH_IMPORT
    void  GetMeshParts(bvector<IScalableMeshMeshPtr>& parts, bvector<Utf8String>& metadata, bvector<bvector<uint8_t>>& texData);

    void AppendMeshParts(bvector<bvector<DPoint3d>>& points, bvector<bvector<int32_t>>& indices, bvector<Utf8String>& metadata, bvector<bvector<DPoint2d>>& uvs, bvector<bvector<uint8_t>>& tex, bool shouldCreateGraph);

    size_t             AddMeshDefinitionUnconditional(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent,const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs);
    size_t             AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, bool ExtentFixed, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs);

    void GetMetadata();
    void StoreMetadata();

    void GetMeshParts();
    void StoreMeshParts();
#endif

    //NEEDS_WORK_SM: refactor all meshIndex recursive calls into something more like a visitor pattern
    //NEEDS_WORK_SM: move clip and raster support to point index

    void                TextureFromRaster(HIMMosaic* sourceRasterP, Transform unitTransform = Transform::FromIdentity());
    void                TextureFromRasterRecursive(HIMMosaic* sourceRasterP, Transform unitTransform = Transform::FromIdentity());

    void                  ReadFeatureDefinitions(bvector<bvector<DPoint3d>>& points, bvector<DTMFeatureType> & types);

    size_t                AddFeatureDefinitionSingleNode(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinitionUnconditional(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed);

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

    const DifferenceSet GetClipSet(size_t index) const
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

#ifdef WIP_MESH_IMPORT
    void PropagateMeshToChildren();
#endif

    size_t CountAllFeatures();

    /**----------------------------------------------------------------------------
    Initiates the mesh stitching of the node.
    -----------------------------------------------------------------------------*/
    virtual void Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch);
            
    void PushPtsIndices(const int32_t* indices, size_t size);

    void ReplacePtsIndices(const int32_t* indices, size_t size);

    void ClearPtsIndices();                
    
    virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetPtsIndicePtr();
        
    virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetLinearFeaturesPtr();
        
    void GetFeatureDefinitions(bvector < bvector<int32_t>>& featureDefs, const int32_t* serializedFeatureDefs, size_t size)
        {
        size_t i = 0;
        bool inFeatureDef = false;
        bvector<int32_t> currentFeature;
        size_t firstIdxForCurrentFeature = 0;
        while (i < size)
            {
            if (!inFeatureDef)
                {
                currentFeature.resize(serializedFeatureDefs[i]);
                inFeatureDef = true;
                firstIdxForCurrentFeature = i + 1;
                }
            else
                {
                currentFeature[i - firstIdxForCurrentFeature] = serializedFeatureDefs[i];
                if (i + 1 - firstIdxForCurrentFeature >= currentFeature.size())
                    {
                    inFeatureDef = false;
                    featureDefs.push_back(currentFeature);
                    currentFeature.clear();
                    }
                }
            i++;
            }
        }

    bool SaveFeatureDefinitions(int32_t* serializedFeatureDefs, size_t size, const bvector < bvector<int32_t>>& featureDefs)
        {
        size_t i = 0;
        for (auto& vec : featureDefs)
            {
            if (i > size) return false;
            serializedFeatureDefs[i++] = (int32_t)vec.size();
            for (auto& pt : vec)
                {
                if (i > size) return false;
                serializedFeatureDefs[i++] = pt;
                }
            }
        return true;
        }

    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> AddDisplayData(SmCachedDisplayData* smCachedDisplayData)
        {                        
        assert(smCachedDisplayData != 0);        

        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> customGenericBlobItemPtr(new SMMemoryPoolGenericBlobItem<SmCachedDisplayData>(smCachedDisplayData, smCachedDisplayData->GetMemorySize(), GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex));
        SMMemoryPoolItemBasePtr memPoolItemPtr(customGenericBlobItemPtr.get());
        m_displayDataPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        assert(m_displayDataPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);                                            
        return customGenericBlobItemPtr;
        }    
    
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> GetDisplayData()
        {        
        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> cachedDisplayDataItemPtr;
                
        GetMemoryPool()->GetItem<SmCachedDisplayData>(cachedDisplayDataItemPtr, m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
            
        return cachedDisplayDataItemPtr;
        }    

    virtual void RemoveDisplayData()
        {                                
        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;        
        }    
        
    SMMemoryPoolPtr GetMemoryPool() const
        {
        return SMMemoryPool::GetInstance();
        }               

    void         SaveMeshToCloud(DataSourceAccount *dataSourceAccount,
                                 ISMDataStoreTypePtr<EXTENT>&    pi_pDataStore, 
                                 ISMDataStoreTypePtr<EXTENT>&    pi_pStreamingDataStore);


    virtual void LoadTreeNode(size_t& nLoaded, int level, bool headersOnly) override; 

#ifdef INDEX_DUMPING_ACTIVATED
    virtual void         DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                                         bool pi_OnlyLoadedNode) const override;
#endif       
   
    // The byte array starts with three integers specifying the width/heigth in pixels, and the number of channels
    void PushTexture(const Byte* texture, size_t size);              

    virtual RefCountedPtr<SMMemoryPoolBlobItem<Byte>> GetTexturePtr();
                             
    void PushUV(const DPoint2d* points, size_t size);
            
    virtual RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> GetUVCoordsPtr()
        {
        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> poolMemVectorItemPtr;

        if (!IsTextured())
            return poolMemVectorItemPtr;
                            
        if (!SMMemoryPool::GetInstance()->GetItem<DPoint2d>(poolMemVectorItemPtr, m_uvCoordsPoolItemId, GetBlockID().m_integerID, SMStoreDataType::UvCoords, (uint64_t)m_SMIndex))
            {                              
            ISMUVCoordsDataStorePtr nodeDataStore;
            bool result = m_SMIndex->GetDataStore()->GetNodeDataStore(nodeDataStore, &m_nodeHeader);
            assert(result == true);        

            RefCountedPtr<SMStoredMemoryPoolVectorItem<DPoint2d>> storedMemoryPoolVector(new SMStoredMemoryPoolVectorItem<DPoint2d>(GetBlockID().m_integerID, nodeDataStore, SMStoreDataType::UvCoords, (uint64_t)m_SMIndex));
            SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolVector.get());
            m_uvCoordsPoolItemId = SMMemoryPool::GetInstance()->AddItem(memPoolItemPtr);
            assert(m_uvCoordsPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            poolMemVectorItemPtr = storedMemoryPoolVector.get();            
            }    

        return poolMemVectorItemPtr;
        }           
                
    void PushUVsIndices(size_t texture_id, const int32_t* uvsIndices, size_t size);
     
    virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetUVsIndicesPtr();
                  
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


    atomic<size_t> m_nbClips;

    std::mutex m_dtmLock;
    mutable SMMemoryPoolItemId m_graphPoolItemId;
    private:

        bool ClipIntersectsBox(uint64_t clipId, EXTENT ext);

        mutable std::mutex m_graphInflateMutex;
        mutable std::mutex m_graphMutex;
        mutable SMMemoryPoolItemId m_triIndicesPoolItemId;        
        mutable SMMemoryPoolItemId m_texturePoolItemId;                        
        mutable SMMemoryPoolItemId m_triUvIndicesPoolItemId;                
        mutable SMMemoryPoolItemId m_uvCoordsPoolItemId;
        mutable SMMemoryPoolItemId m_diffSetsItemId;
        mutable SMMemoryPoolItemId m_displayDataPoolItemId;  
        mutable SMMemoryPoolItemId m_featurePoolItemId;
        mutable SMMemoryPoolItemId m_dtmPoolItemId;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;

#ifdef WIP_MESH_IMPORT
        bvector<int> m_meshParts;
        bvector<Utf8String> m_meshMetadata;
        bool m_existingMesh;
#endif 

             
        HFCPtr<ClipRegistry> m_clipRegistry;
        mutable std::mutex m_headerMutex;
    };


    template<class POINT, class EXTENT> class SMMeshIndex : public SMPointIndex < POINT, EXTENT >
    {
    friend class SMMeshIndexNode < POINT, EXTENT > ;
    public:
        SMMeshIndex(ISMDataStoreTypePtr<EXTENT>& smDataStore,
                    SMMemoryPoolPtr& smMemoryPool,
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
                        
        virtual void        GetCloudFormatStore(DataSourceAccount *dataSourceAccount,
                                                const WString& pi_pOutputDirPath,
                                                const bool& pi_pCompress,
                                                ISMDataStoreTypePtr<EXTENT>&     po_pDataStore) const;

        StatusInt           SaveMeshToCloud(DataSourceAccount *dataSourceAccount, const WString& pi_pOutputDirPath, const bool& pi_pCompress);

        virtual void        Stitch(int pi_levelToStitch, bool do2_5dStitchFirst = false);
        
        void                SetClipStore(HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>>& clipStore);
        void                SetClipRegistry(ClipRegistry* registry);        

        SMMemoryPoolPtr GetMemoryPool() const { return m_smMemoryPool; }                                

        ClipRegistry* GetClipRegistry()
            {
            return m_clipRegistry.GetPtr();
            }        
        

        //ISMStore::FeatureType is the same as DTMFeatureType defined in TerrainModel.h.
        void                AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
#ifdef WIP_MESH_IMPORT
        void                AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata="");

        void                AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs);
#endif


        void                AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent);
        void                PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn=true);
        void                RefreshMergedClips();


        void                TextureFromRaster(HIMMosaic* sourceRasterP, Transform unitTransform = Transform::FromIdentity());
#ifdef ACTIVATE_TEXTURE_DUMP
        void                DumpAllNodeTextures()
            {
            if (m_pRootNode != 0) dynamic_cast<SMMeshIndexNode<POINT,EXTENT>*>(m_pRootNode.GetPtr())->DumpAllNodeTextures();
            }
#endif


        //NEEDS_WORK_SM : Why the same 2 functions in point index?
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(EXTENT extent, bool isRootNode = false);        
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false);

    private:
        
        SMMemoryPoolPtr             m_smMemoryPool;
        ISMDataStoreTypePtr<EXTENT> m_smDataStore;
                
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;                
        HFCPtr<ClipRegistry> m_clipRegistry;


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



        virtual bool        Init(const SMMeshIndex<POINT, EXTENT>& pointIndex) { return true; }

        virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const = 0;

        virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const = 0;

        virtual void        AddClip(bvector<DPoint3d>& clip) {};

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

inline bool IsVoidFeature(ISMStore::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Hole || dtmType == DTMFeatureType::Void || dtmType == DTMFeatureType::BreakVoid ||
        dtmType == DTMFeatureType::DrapeVoid;
    }


void MergeTextures(bvector<uint8_t>& outTex, DPoint2d& uvBotLeft, DPoint2d&  uvTopRight, const ScalableMeshTexture* textureP, const uint8_t* texData, size_t texSize);

void RemapAllUVs(bvector<DPoint2d>& inoutUvs, DPoint2d uvBotLeft, DPoint2d uvTopRight);

void ComputeTexPart(bvector<uint8_t>&texPart, DPoint2d* uvPart, size_t nUvs, bvector<uint8_t>& texDataUnified);