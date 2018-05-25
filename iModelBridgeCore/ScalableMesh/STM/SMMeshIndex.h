
#pragma once
#include <Mtg/MtgStructs.h>
#include "SMPointIndex.h"
#include "Edits/DifferenceSet.h"
//#include "Edits/ClipUtilities.h"

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

#include "Edits\EditOperation.h"

#include "SMMemoryPool.h"
#include "Stores\SMSQLiteStore.h"

#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include "SharedTextureManager.h"
#include "SmCachedDisplayData.h"
#include "ScalableMeshQuery.h"
#include <ScalableMesh\ITextureProvider.h>
#include "ScalableMeshDraping.h"



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

/*template<> struct PoolItem<DifferenceSet>
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
    }*/

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

#include "SmCachedDisplayData.h"

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
        dtmType == DTMFeatureType::TinHull || dtmType == DTMFeatureType::DrapeVoid;
    }

inline bool IsClosedPolygon(const bvector<DPoint3d>& vec)
    {
    return !vec.empty() && (vec.front() == vec.back());
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

    SMMeshIndexNode(uint64_t nodeID,
                    size_t pi_SplitTreshold,
                    const EXTENT& pi_rExtent,
                    const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode);
    
    SMMeshIndexNode(size_t pi_SplitTreshold,
                                     const EXTENT& pi_rExtent,
                                     const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                     bool IsUnsplitSubLevel);

    SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode);    

    SMMeshIndexNode(uint64_t nodeID,
                    size_t pi_SplitTreshold,
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

    void Init();
    
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneChild(uint64_t nodeId, const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChildVirtual() const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewChildNode(HPMBlockID blockID);
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false) override;

    virtual bool Discard() override;    
    virtual bool Destroy();


    virtual void Load() const override;

    virtual void Unload() override;

	virtual void RemoveNonDisplayPoolData() override;

    virtual bool InvalidateFilteringMeshing(bool becauseDataRemoved = false) override;

    virtual bool IsGraphLoaded() const;

    void LockGraph()
        {
        m_graphMutex.lock();
        }

    void UnlockGraph()
        {
        m_graphMutex.unlock();
        }

    BENTLEY_SM_EXPORT ISMMTGGraphDataStorePtr GetGraphStore() const;
        
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> GetGraphPtr(bool loadGraph = true);       

    virtual RefCountedPtr<SMMemoryPoolGenericVectorItem<DifferenceSet>> GetDiffSetPtr() const;
        
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> GetTileDTM()
        {
        std::lock_guard<std::mutex> lock(m_dtmLock); //don't want to add item twice
        RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> poolMemItemPtr;


        if (!SMMemoryPool::GetInstance()->GetItem<BcDTMPtr>(poolMemItemPtr, m_dtmPoolItemId, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex))
            {
            RefCountedPtr<SMMemoryPoolGenericBlobItem<BcDTMPtr>> storedMemoryPoolItem(
#ifndef VANCOUVER_API   
                new SMMemoryPoolGenericBlobItem<BcDTMPtr>(nullptr, 0, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex)
#else
            SMMemoryPoolGenericBlobItem<BcDTMPtr>::CreateItem(nullptr, 0, GetBlockID().m_integerID, SMStoreDataType::BcDTM, (uint64_t)m_SMIndex)
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

    /**----------------------------------------------------------------------------
    Returns the 2.5d mesher used for meshing the points

    @return Pointer to mesher or NULL if none is set.
    -----------------------------------------------------------------------------*/
    BENTLEY_SM_EXPORT  ISMPointIndexMesher<POINT, EXTENT>*
        GetMesher2_5d() const;

    /**----------------------------------------------------------------------------
    Returns the 3d mesher used for meshing the points

    @return Pointer to mesher or NULL if none is set.
    -----------------------------------------------------------------------------*/
    BENTLEY_SM_EXPORT ISMPointIndexMesher<POINT, EXTENT>*
        GetMesher3d() const;


    /**----------------------------------------------------------------------------
    Initiates the meshing of the node.
    -----------------------------------------------------------------------------*/
    virtual void Mesh();

    void                SetClipRegistry(HFCPtr<ClipRegistry>& registry)
        {
        m_clipRegistry = registry;
        }


    BENTLEY_SM_EXPORT void UpdateFromGraph(MTGGraph * graph, bvector<DPoint3d>& pointList);

    void CollectFeatureDefinitionsFromGraph(MTGGraph* graph, size_t maxPtID);

    void SplitNodeBasedOnImageRes();
    void SplitMeshForChildNodes();

    void PropagateFullMeshDown(size_t depth);

    void UpdateNodeFromBcDTM();

    void ImportTreeFrom(IScalableMeshNodePtr& sourceNode, bool shouldCopyData=true, bool use2d = false);

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

    void                TextureFromRaster(ITextureProviderPtr sourceRasterP, Transform unitTransform = Transform::FromIdentity());
    void                TextureFromRasterRecursive(ITextureProviderPtr sourceRasterP, Transform unitTransform = Transform::FromIdentity());

    BENTLEY_SM_EXPORT void                  ReadFeatureDefinitions(bvector<bvector<DPoint3d>>& points, bvector<DTMFeatureType> & types, bool shouldIgnoreOpenFeatures);

    BENTLEY_SM_EXPORT size_t                AddFeatureDefinitionSingleNode(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinitionUnconditional(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    size_t                AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed);

    //NEEDS_WORK_SM: clean up clipping API (remove extra calls, clarify uses, etc)

    //Synchronously add clip on this node and all descendants, used in generation.
    void                AddClipDefinitionRecursive(bvector<DPoint3d>& points, DRange3d& extent);
    //Synchronously update clips, if necessary, so as to merge them with other clips applied on the node, for this node and all descendants, if applicable. Used in generation.
    void                RefreshMergedClipsRecursive();

    //Checks whether clip should apply to node and update lists accordingly
    void                ClipActionRecursive(ClipAction action,uint64_t clipId, DRange3d& extent, bool setToggledWhenIdIsOn = true, Transform tr = Transform::FromIdentity());

    ClipRegistry* GetClipRegistry() const
        {
        return dynamic_cast<SMMeshIndex<POINT, EXTENT>*>(m_SMIndex)->GetClipRegistry();
        }

    void  BuildSkirts();

#if 0
    void CreateSkirtsForMatchingTerrain();

    SMPointIndexNode<POINT, EXTENT>* FindMatchingTerrainNode();

    void FindMatchingTerrainNodes(bvector<IScalableMeshNodePtr>& terrainNodes);
#endif

    bool HasClip(uint64_t clipId);

    bool IsClippingUpToDate();

    //If necessary, update clips so as to merge them with other clips on the node.
    void  ComputeMergedClips(Transform tr = Transform::FromIdentity());
    //Adds a new set of differences matching the desired clip (synchronously).
    //The base mesh is retained and the clip is a non-destructive modification.
    //Caller provides desired ID for the new clip. Returns true if clip was added, false if clip is out of bounds.
    bool  AddClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn = true, Transform tr = Transform::FromIdentity());

    //Deletes an existing clip or returns false if there is no clip with this ID.
    bool  DeleteClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn=true);

    //Modifies an existing clip or returns false if there is no clip with this ID.
    bool ModifyClip(uint64_t clipId, bool isVisible, bool setToggledWhenIdIsOn = true, Transform tr = Transform::FromIdentity());

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

    virtual bool        SaveGroupedNodeHeaders(SMNodeGroupPtr pi_pGroup, IScalableMeshProgressPtr progress) override;

    size_t CountAllFeatures();

    /**----------------------------------------------------------------------------
    Initiates the mesh stitching of the node.
    -----------------------------------------------------------------------------*/
    virtual void Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch);
            
    BENTLEY_SM_EXPORT void PushPtsIndices(const int32_t* indices, size_t size);

    BENTLEY_SM_EXPORT void ReplacePtsIndices(const int32_t* indices, size_t size);

    BENTLEY_SM_EXPORT void ClearPtsIndices();
    
    BENTLEY_SM_EXPORT virtual RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> GetPtsIndicePtr();
        
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
#if 0
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> AddDisplayData(SmCachedDisplayData* smCachedDisplayData)
        {                        
        assert(smCachedDisplayData != 0);        

        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> customGenericBlobItemPtr(
#ifndef VANCOUVER_API            
            new SMMemoryPoolGenericBlobItem<SmCachedDisplayData>(smCachedDisplayData, smCachedDisplayData->GetMemorySize(), GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex)
#else
        SMMemoryPoolGenericBlobItem<SmCachedDisplayData>::CreateItem(smCachedDisplayData, smCachedDisplayData->GetMemorySize(), GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex)
#endif
            );
        SMMemoryPoolItemBasePtr memPoolItemPtr(customGenericBlobItemPtr.get());
        m_displayDataPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
        assert(m_displayDataPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);                                            
        return customGenericBlobItemPtr;
        }    
#endif  

    virtual RefCountedPtr<SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>> AddDisplayMesh(SmCachedDisplayMeshData* smCachedDisplayData, size_t sizeToReserve, bool isVideoMemory=false)
        {
        std::lock_guard<std::mutex> lock(m_displayMeshLock);
        assert(smCachedDisplayData != 0);

        RefCountedPtr<SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>> customGenericBlobItemPtr(
#ifndef VANCOUVER_API            
            new SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>(0, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex)
#else
            SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>::CreateItem(0, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex)
#endif
            );
        SMMemoryPoolItemBasePtr memPoolItemPtr(customGenericBlobItemPtr.get());

        if (isVideoMemory)
            { 
            m_displayMeshVideoPoolItemId = SMMemoryPool::GetInstanceVideo()->AddItem(memPoolItemPtr);
            assert(m_displayMeshVideoPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            }
        else
            {
            m_displayMeshPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(m_displayMeshPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            }                

        customGenericBlobItemPtr->reserve(sizeToReserve);
        customGenericBlobItemPtr->push_back(*smCachedDisplayData);
        
        return customGenericBlobItemPtr;
        }


    virtual SMMemoryPoolItemBasePtr AddDisplayMesh(SMMemoryPoolItemBasePtr itemPtr, bool isVideoMemory = false)
    {
        std::lock_guard<std::mutex> lock(m_displayMeshLock);

        if (isVideoMemory)
            {
            m_displayMeshVideoPoolItemId = SMMemoryPool::GetInstanceVideo()->AddItem(itemPtr);
            assert(m_displayMeshVideoPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            }
        else
            {
            m_displayMeshPoolItemId = GetMemoryPool()->AddItem(itemPtr);
            assert(m_displayMeshPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            }
                   
        return itemPtr;
    }


    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> AddDisplayTexture(SmCachedDisplayTextureData* smCachedDisplayData, uint64_t texID, bool isVideoMemory=false)
        {
        assert(smCachedDisplayData != 0);
    

        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> customGenericBlobItemPtr(
#ifndef VANCOUVER_API            
            new SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>(smCachedDisplayData, smCachedDisplayData->GetMemorySize(), texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex)
#else
            SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>::CreateItem(smCachedDisplayData, smCachedDisplayData->GetMemorySize(), texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex)
#endif
            );
        SMMemoryPoolItemBasePtr memPoolItemPtr(customGenericBlobItemPtr.get());

        if (isVideoMemory)
            {
            auto displayTexVideoPoolItemId = SMMemoryPool::GetInstanceVideo()->AddItem(memPoolItemPtr);
            assert(displayTexVideoPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->SetPoolIdForTextureVideo(texID, displayTexVideoPoolItemId);
            m_textureVideoIds.insert(texID);
            }
        else
            {            
            auto displayTexPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTexture(texID);
            
            if (displayTexPoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
                {
                //If we get here is because a change of Display Cache Manager. Ensure to remove the display texture for the previous display cache manager first.
                GetMemoryPool()->RemoveItem(displayTexPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);                
                }
            
            displayTexPoolItemId = GetMemoryPool()->AddItem(memPoolItemPtr);
            assert(displayTexPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->SetPoolIdForTexture(texID, displayTexPoolItemId);
            m_textureIds.insert(texID);
            }
                               
        smCachedDisplayData->AddConsumer((SMMeshIndexNode<DPoint3d,DRange3d>*)(this));
        return customGenericBlobItemPtr;
        }

    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> AddDisplayTexture(SMMemoryPoolItemBasePtr itemPtr, uint64_t texID, bool isVideoMemory = false)
        {
        if (isVideoMemory)
            {
            auto displayTexVideoPoolItemId = SMMemoryPool::GetInstanceVideo()->AddItem(itemPtr);
            assert(displayTexVideoPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->SetPoolIdForTextureVideo(texID, displayTexVideoPoolItemId);
            m_textureVideoIds.insert(texID);
            }
        else
            {
            auto displayTexPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTexture(texID);

            if (displayTexPoolItemId != SMMemoryPool::s_UndefinedPoolItemId)
                {
                //If we get here is because a change of Display Cache Manager. Ensure to remove the display texture for the previous display cache manager first.
                GetMemoryPool()->RemoveItem(displayTexPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);
                }

            displayTexPoolItemId = GetMemoryPool()->AddItem(itemPtr);
            assert(displayTexPoolItemId != SMMemoryPool::s_UndefinedPoolItemId);
            ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->SetPoolIdForTexture(texID, displayTexPoolItemId);
            m_textureIds.insert(texID);
            }                

        SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>* blobItemP = (dynamic_cast<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>*>(itemPtr.get()));
        SmCachedDisplayTextureData* data = const_cast<SmCachedDisplayTextureData*>(blobItemP->GetData());
        data->AddConsumer((SMMeshIndexNode<DPoint3d, DRange3d>*)(this));
        return blobItemP;
        }

#if 0
    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> GetDisplayData()
        {        
        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> cachedDisplayDataItemPtr;
                
        GetMemoryPool()->GetItem<SmCachedDisplayData>(cachedDisplayDataItemPtr, m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
            
        return cachedDisplayDataItemPtr;
        }  
#endif

    virtual RefCountedPtr<SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>> GetDisplayMeshes(bool isVideoMemory=false)
        {
        std::lock_guard<std::mutex> lock(m_displayMeshLock);
        RefCountedPtr<SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>> cachedDisplayMeshItemPtr;

        if (isVideoMemory)
            {
            if (m_displayMeshVideoPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayMeshItemPtr;
            
            SMMemoryPool::GetInstanceVideo()->GetItem<SmCachedDisplayMeshData>(cachedDisplayMeshItemPtr, m_displayMeshVideoPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);
            }
        else
            {
            if (m_displayMeshPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayMeshItemPtr;

            GetMemoryPool()->GetItem<SmCachedDisplayMeshData>(cachedDisplayMeshItemPtr, m_displayMeshPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);            
            }
        
        return cachedDisplayMeshItemPtr;
        }

    virtual RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> GetDisplayTexture(uint64_t texID, bool isVideoMemory = false)
        {
        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> cachedDisplayDataItemPtr;
        
        if(!isVideoMemory)
            {
            SMMemoryPoolItemId displayTexPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTexture(texID);
            if (displayTexPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayDataItemPtr;

            GetMemoryPool()->GetItem<SmCachedDisplayTextureData>(cachedDisplayDataItemPtr, displayTexPoolItemId, texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);
            }
        else
            {
            SMMemoryPoolItemId displayTexVideoPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTextureVideo(texID);
            if (displayTexVideoPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayDataItemPtr;

            SMMemoryPool::GetInstanceVideo()->GetItem<SmCachedDisplayTextureData>(cachedDisplayDataItemPtr, displayTexVideoPoolItemId, texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);
            }

        return cachedDisplayDataItemPtr;
        }

     RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> GetSingleDisplayTexture(bool isVideoMemory = false)
        {
        uint64_t texID = GetSingleTextureID();
        RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>> cachedDisplayDataItemPtr;
                
        if (!isVideoMemory)
            {
            SMMemoryPoolItemId displayTexPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTexture(texID);
            if (displayTexPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayDataItemPtr;

            GetMemoryPool()->GetItem<SmCachedDisplayTextureData>(cachedDisplayDataItemPtr, displayTexPoolItemId, texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);
            }
        else
            {
            SMMemoryPoolItemId displayTexVideoPoolItemId = ((SMMeshIndex<POINT, EXTENT>*)m_SMIndex)->TextureManager()->GetPoolIdForTextureVideo(texID);
            if (displayTexVideoPoolItemId == SMMemoryPool::s_UndefinedPoolItemId) return cachedDisplayDataItemPtr;

            SMMemoryPool::GetInstanceVideo()->GetItem<SmCachedDisplayTextureData>(cachedDisplayDataItemPtr, displayTexVideoPoolItemId, texID, SMStoreDataType::DisplayTexture, (uint64_t)m_SMIndex);
            }

        return cachedDisplayDataItemPtr;
        }

     uint64_t GetSingleTextureID()
         {
         if (!IsLoaded()) Load();
         return m_nodeHeader.m_textureID.IsValid() && m_nodeHeader.m_textureID.m_integerID != -1 ? m_nodeHeader.m_textureID.m_integerID : GetBlockID().m_integerID;
         }

     bool GetAllDisplayTextures(bvector<RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>>>& textures, bool isInVRAM=false)
         {
#ifdef WIP_MESH_IMPORT
         GetMetadata();
         GetMeshParts();
         if (!m_meshMetadata.empty())
             {
             bvector<uint64_t> textureIDs;
             for(auto& data: m_meshMetadata)
                 {
                 Json::Value val;
                 Json::Reader reader;
                 reader.parse(data, val);
                 if (val["texId"].size() > 0)
                     textureIDs.push_back( (*val["texId"].begin()).asUInt64());
                 }
             for (auto& id : textureIDs)
                 {
                 textures.push_back(GetDisplayTexture(id, isInVRAM));
                 }
             }
         else
             {
#endif
             if (IsTextured())
             {
                 auto tex = GetSingleDisplayTexture(isInVRAM);
                 if (!tex.IsValid()) return false;
                 textures.push_back(tex);
             }
             else return false;
#ifdef WIP_MESH_IMPORT
             }
#endif
         return true;
         }

    virtual void RemoveDisplayMesh(bool isVideoMemory=false)
        {
        //std::lock_guard<std::mutex> lock(m_displayMeshLock);      
    
        if(!isVideoMemory)
            { 
            GetMemoryPool()->RemoveItem(m_displayMeshPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);
            m_displayMeshPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }
        else
            { 
            SMMemoryPool::GetInstanceVideo()->RemoveItem(m_displayMeshVideoPoolItemId, GetBlockID().m_integerID, SMStoreDataType::DisplayMesh, (uint64_t)m_SMIndex);
            m_displayMeshVideoPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
            }        
        }

	virtual void RefreshDisplayMesh()
	{
/*		auto meshesPtr = GetDisplayMeshes();
		if (meshesPtr->size() > 0)
		{
			IScalableMeshDisplayCacheManagerPtr displayCacheManagerPtr;
			bvector<uint64_t>                   appliedClips;
			displayCacheManagerPtr = const_cast<SmCachedDisplayMeshData&>((*meshesPtr)[0]).GetDisplayCacheManager();
			appliedClips = const_cast<SmCachedDisplayMeshData&>((*meshesPtr)[0]).GetAppliedClips();

			RemoveDisplayMesh();

			bset<uint64_t> clipsToApply;
			for (auto& clip : appliedClips) clipsToApply.insert(clip);

			Transform reprojectTransform = Transform::FromIdentity();
			HFCPtr<SMPointIndexNode<POINT, EXTENT>> nodeP = dynamic_cast<SMPointIndexNode<POINT,EXTENT>*>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this));
			ScalableMeshCachedDisplayNode<DPoint3d>* meshNode(ScalableMeshCachedDisplayNode<DPoint3d>::Create(nodeP, reprojectTransform));

			meshNode->LoadMesh(false, clipsToApply, displayCacheManagerPtr, true);
			IScalableMeshCachedDisplayNodePtr displayNodePtr = meshNode;
		}*/
	}


    virtual void RemoveDisplayData()
        {                                
        GetMemoryPool()->RemoveItem(m_displayDataPoolItemId, GetBlockID().m_integerID, SMStoreDataType::Display, (uint64_t)m_SMIndex);
        m_displayDataPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;        
        }   

    void RemoveMultiTextureData();        
            
    SMMemoryPoolPtr GetMemoryPool() const
        {
        return SMMemoryPool::GetInstance();
        }               

    bool         Publish3DTile(ISMDataStoreTypePtr<EXTENT>& pi_pDataStore, TransformCR transform, ClipVectorPtr clips, const uint64_t& coverageID, bool isClipBoundary, const GeoCoordinates::BaseGCSCPtr sourceGCS, const GeoCoordinates::BaseGCSCPtr destinationGCS, IScalableMeshProgressPtr progress, bool outputTexture);

    void         ChangeGeometricError(ISMDataStoreTypePtr<EXTENT>&    pi_pDataStore, const double& newGeometricErrorValue);

    void         SaveMeshToCloud(ISMDataStoreTypePtr<EXTENT>&    pi_pDataStore);

    virtual void LoadIndexNodes(size_t& nLoaded, int level, bool headersOnly) override;

#ifdef INDEX_DUMPING_ACTIVATED
    virtual void         DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                                         bool pi_OnlyLoadedNode) const override;
#endif       
   

    void RemoveWithin(ClipVectorCP boundariesToRemoveWithin);

    void UpdateData();

    void AddEdit(RefCountedPtr<EditOperation>& editDef);


    // The byte array starts with three integers specifying the width/heigth in pixels, and the number of channels
    void PushTexture(const Byte* texture, size_t size);              

    virtual RefCountedPtr<SMMemoryPoolBlobItem<Byte>> GetTexturePtr();

    virtual RefCountedPtr<SMMemoryPoolBlobItem<Byte>> GetTexturePtr(uint64_t texID);

    virtual RefCountedPtr<SMMemoryPoolBlobItem<Byte>> GetTextureCompressedPtr();

    void PushUV(const DPoint2d* points, size_t size);
            
    virtual RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> GetUVCoordsPtr()
        {
        RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> poolMemVectorItemPtr;

        if (!IsTextured())
            return poolMemVectorItemPtr;
                            
        if (!m_SMIndex->IsFromCesium())
            {
            poolMemVectorItemPtr = GetMemoryPoolItem<ISMUVCoordsDataStorePtr, DPoint2d, SMMemoryPoolVectorItem<DPoint2d>, SMStoredMemoryPoolVectorItem<DPoint2d>>(m_uvCoordsPoolItemId, SMStoreDataType::UvCoords, GetBlockID());
            }
        else
            {
            SMMemoryPoolMultiItemsBasePtr poolMemMultiItemsPtr = GetMemoryPoolMultiItem<ISMCesium3DTilesDataStorePtr, Cesium3DTilesBase, SMMemoryPoolMultiItemsBase, SMStoredMemoryPoolMultiItems<Cesium3DTilesBase>>(m_pointsPoolItemId, SMStoreDataType::Cesium3DTiles, GetBlockID()).get();
            bool result = poolMemMultiItemsPtr->GetItem<DPoint2d>(poolMemVectorItemPtr, SMStoreDataType::UvCoords);
            assert(result == true);
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
    std::mutex m_displayMeshLock;
    mutable SMMemoryPoolItemId m_graphPoolItemId;
    mutable SMMemoryPoolItemId m_smMeshPoolItemId;

    private:

		bool ClipIntersectsBox(uint64_t clipId, EXTENT ext, Transform tr = Transform::FromIdentity());

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
        mutable SMMemoryPoolItemId m_displayMeshPoolItemId;
        mutable SMMemoryPoolItemId m_displayMeshVideoPoolItemId;
        mutable bset<uint64_t>     m_textureIds;
        mutable bset<uint64_t>     m_textureVideoIds;
        
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;


             
        HFCPtr<ClipRegistry> m_clipRegistry;
        mutable std::mutex m_headerMutex;


        bvector<RefCountedPtr<EditOperation>> m_remainingUnappliedEdits;

#ifdef WIP_MESH_IMPORT
        public: //NEEDS_WORK: make private
        bvector<int> m_meshParts;
        bvector<Utf8String> m_meshMetadata;
        bool m_existingMesh;
#endif 

    };


    template<class POINT, class EXTENT> class SMMeshIndex : public SMPointIndex < POINT, EXTENT >
    {
    friend class SMMeshIndexNode < POINT, EXTENT > ;
    public:
        BENTLEY_SM_EXPORT SMMeshIndex(ISMDataStoreTypePtr<EXTENT>& smDataStore,
                    SMMemoryPoolPtr& smMemoryPool,
                    size_t SplitTreshold, 
                    ISMPointIndexFilter<POINT, EXTENT>* filter, 
                    bool balanced, 
                    bool textured,
                    bool propagatesDataDown,
                    bool needsNeighbors,
                    ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d, 
                    ISMPointIndexMesher<POINT, EXTENT>* mesher3d);

        BENTLEY_SM_EXPORT virtual             ~SMMeshIndex<POINT, EXTENT>();


        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher2_5d();

        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher3d();


        virtual void        Mesh();
                        
        StatusInt           Publish3DTiles(const WString& path, TransformCR transform, ClipVectorPtr clips, const uint64_t& clipID, const GeoCoordinates::BaseGCSCPtr sourceGCS, bool outputTexture);

        StatusInt           SaveMeshToCloud(const WString& path, const bool& pi_pCompress);

        StatusInt           ChangeGeometricError(const WString& path, const bool& pi_pCompress, const double& newGeometricErrorValue);

        virtual void        Stitch(int pi_levelToStitch, bool do2_5dStitchFirst = false);
        
        void                SetClipStore(HFCPtr<IScalableMeshDataStore<DifferenceSet, Byte, Byte>>& clipStore);
        void                SetClipRegistry(ClipRegistry* registry);       


        SMMemoryPoolPtr GetMemoryPool() const { return m_smMemoryPool; }                                

        ClipRegistry* GetClipRegistry()
            {
            return m_clipRegistry.GetPtr();
            }
        

        //ISMStore::FeatureType is the same as DTMFeatureType defined in TerrainModel.h.
        BENTLEY_SM_EXPORT void                AddFeatureDefinition(ISMStore::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
#ifdef WIP_MESH_IMPORT
        void                AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata="");

        void                AddMeshDefinition(const DPoint3d* pts, size_t nPts, const int32_t* indices, size_t nIndices, DRange3d extent, const char* metadata, const uint8_t* texData, size_t texSize, const DPoint2d* uvs);
#endif

        BENTLEY_SM_EXPORT void PropagateFullMeshDown();


        BENTLEY_SM_EXPORT void                AddClipDefinition(bvector<DPoint3d>& points, DRange3d& extent);
        void                PerformClipAction(ClipAction action, uint64_t clipId, DRange3d& extent, bool setToggledWhenIDIsOn=true, Transform tr = Transform::FromIdentity());
        BENTLEY_SM_EXPORT void                RefreshMergedClips();

        BENTLEY_SM_EXPORT size_t GetNextTextureId();


        BENTLEY_SM_EXPORT void                TextureFromRaster(ITextureProviderPtr sourceRasterP, Transform unitTransform = Transform::FromIdentity());

        int                 RemoveWithin(ClipVectorCP boundariesToRemoveWithin, const bvector<IScalableMeshNodePtr>& priorityNodes);
#ifdef ACTIVATE_TEXTURE_DUMP
        void                DumpAllNodeTextures()
            {
            if (m_pRootNode != 0) dynamic_cast<SMMeshIndexNode<POINT,EXTENT>*>(m_pRootNode.GetPtr())->DumpAllNodeTextures();
            }
#endif

        SharedTextureManager* TextureManager() { return &m_texMgr; }

        SMMeshIndex<POINT, EXTENT>* CloneIndex(ISMDataStoreTypePtr<EXTENT> associatedStore);

        //NEEDS_WORK_SM : Why the same 2 functions in point index?
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(uint64_t nodeId, EXTENT extent, bool isRootNode = false);
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(EXTENT extent, bool isRootNode = false);        
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false);

        int64_t  AddTexture(int width, int height, int nOfChannels, const byte* texData, size_t nOfBytes);

        bool m_isInsertingClips;

		bool m_canUseBcLibClips;

        void SetSMTerrain(SMMeshIndex<POINT, EXTENT>* terrainP);
        SMMeshIndex<POINT, EXTENT>* GetSMTerrain();
#if 0
        void SetSMTerrainMesh(IScalableMesh* terrainP);
        IScalableMesh* GetSMTerrainMesh();
#endif
    private:
        
        SMMemoryPoolPtr             m_smMemoryPool;
        ISMDataStoreTypePtr<EXTENT> m_smDataStore;
                
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;                
        HFCPtr<ClipRegistry> m_clipRegistry;

        size_t m_texId = 0;

        SharedTextureManager m_texMgr;

        
        std::vector<std::future<bool>> m_textureWorkerTasks;
        WorkerThreadPoolPtr            m_texturingThreadPoolPtr;

        bvector < RefCountedPtr<EditOperation> > m_edits;
#if 0
        SMMeshIndex<POINT, EXTENT>* m_smTerrain;
        IScalableMesh* m_smTerrainMesh;
#endif
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

void ComputeAndRemoveIntersections(bvector<bvector<DPoint3d>>& polygons, bvector<uint64_t>& ids);