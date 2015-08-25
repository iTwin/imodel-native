
#pragma once
#include <Mtg/MtgStructs.h>
#include "SMPointIndex.h"
#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>


template<class POINT, class EXTENT> class ISMPointIndexMesher;
template<class POINT, class EXTENT> class ISMMeshIndexFilter;


template<class POINT, class EXTENT> class SMMeshIndex;

template <class POINT, class EXTENT> class SMMeshIndexNode : public SMPointIndexNode < POINT, EXTENT >
    {
    friend class ISMPointIndexMesher<POINT, EXTENT>;
    friend class SMMeshIndex < POINT, EXTENT > ;
    public:
    SMMeshIndexNode(size_t pi_SplitTreshold,
                        const EXTENT& pi_rExtent,
                        HFCPtr<HPMCountLimitedPool<POINT>> pool,
                        HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                        HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                        ISMPointIndexFilter<POINT, EXTENT>* filter,
                        bool balanced,
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
                      HFCPtr<HPMCountLimitedPool<POINT> > pool,
                      HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                      HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                      HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap*                      createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,
                      HFCPtr<HPMCountLimitedPool<POINT> > pool,
                      HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                      HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                      HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap* createdNodeMap);

    SMMeshIndexNode(size_t pi_SplitTreshold,
                      const EXTENT& pi_rExtent,
                      HFCPtr<HPMCountLimitedPool<POINT>> pool,
                      HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                      ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                      CreatedNodeMap*                      createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,
                     HFCPtr<HPMCountLimitedPool<POINT> > pool,
                     HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                     bool balanced,
                     bool propagateDataDown,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                     CreatedNodeMap* createdNodeMap);

    SMMeshIndexNode(HPMBlockID blockID,
                     HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,
                     HFCPtr<HPMCountLimitedPool<POINT> > pool,
                     HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                     ISMPointIndexFilter<POINT, EXTENT>* filter,
                     bool balanced,
                     bool propagateDataDown,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                     CreatedNodeMap*                      createdNodeMap);

    virtual ~SMMeshIndexNode<POINT, EXTENT>();

    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > Clone() const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > Clone(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChildVirtual() const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewChildNode(HPMBlockID blockID);
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID);

    virtual bool Discard() const;
    virtual bool Destroy();


    virtual void Load() const;

    virtual void Unload() const override;

    virtual bool IsGraphLoaded() const;

    void CreateGraph() const;

    virtual void LoadGraph() const;

    void StoreGraph() const;

    virtual MTGGraph* GetGraphPtr()
        {
        if (m_graphVec.size() == 0 || m_graphVec.Discarded()) return NULL;
        else return const_cast<MTGGraph*>(&*m_graphVec.begin());
        }

    void SetGraphDirty()
        {
        m_graphVec.SetDiscarded(false);
        m_graphVec.SetDirty(true);
        }

    bool IsGraphDirty()
        {
        return m_graphVec.IsDirty();
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

    void                SetFeatureStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& featureStore)
        {
        m_featureStore = featureStore;
        }

    void                SetFeaturePool(HFCPtr<HPMCountLimitedPool<int32_t>>& featurePool)
        {
        m_featurePool = featurePool;
        }

    void                AddFeatureDefinitionUnconditional(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);
    bool                AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed);

    virtual void OnPropagateDataDown() override;
    virtual void OnPushNodeDown() override;

    void PropagateFeaturesToChildren();

    size_t CountAllFeatures();

    /**----------------------------------------------------------------------------
    Initiates the mesh stitching of the node.
    -----------------------------------------------------------------------------*/
    virtual void Stitch(int pi_levelToStitch, vector<SMMeshIndexNode<POINT, EXTENT>*>* nodesToStitch);

    HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> GetGraphStore() const
        {
        return m_graphStore;
        };

    HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > GetGraphPool() const
        {
        return m_graphPool;
        };

    void SetGraphStore(HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>>& store)
        {
        m_graphStore = store;
        m_graphVec.SetStore(store);
        };

    void SetGraphPool(HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>>& pool)
        {
        m_graphPool = pool;
        m_graphVec.SetPool(&*pool);
        };

    /**----------------------------------------------------------------------------
    Indicates if the node need meshing. The fact it is meshed is not
    sufficient not to need meshing, if any sub-node need meshing then
    it also required re-meshing.
    -----------------------------------------------------------------------------*/
    virtual bool NeedsMeshing() const;

    mutable vector<HPMStoredPooledVector<int32_t>> m_featureDefinitions;
    private:
        HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> m_graphStore;
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > m_graphPool;
        mutable HPMStoredPooledVector<MTGGraph> m_graphVec;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;
        mutable bool m_isGraphLoaded;
        HFCPtr<SMPointTileStore<int32_t, EXTENT>> m_featureStore;
        HFCPtr<HPMCountLimitedPool<int32_t>> m_featurePool;
    };


    template<class POINT, class EXTENT> class SMMeshIndex : public SMPointIndex < POINT, EXTENT >
    {
    public:
        SMMeshIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool, 
                     HFCPtr<SMPointTileStore<POINT, EXTENT> > store, 
                     HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                     HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                     size_t SplitTreshold, 
                     ISMPointIndexFilter<POINT, EXTENT>* filter, 
                     bool balanced, 
                     bool propagatesDataDown, 
                     ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d, 
                     ISMPointIndexMesher<POINT, EXTENT>* mesher3d);
        virtual             ~SMMeshIndex<POINT, EXTENT>();


        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher2_5d();

        ISMPointIndexMesher<POINT, EXTENT>*
            GetMesher3d();


        virtual void        Mesh();

        virtual void        Stitch(int pi_levelToStitch, bool do2_5dStitchFirst = false);

        void                SetFeatureStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& featureStore);
        void                SetFeaturePool(HFCPtr<HPMCountLimitedPool<int32_t>>& featurePool);

        //IDTMFile::FeatureType is the same as DTMFeatureType defined in TerrainModel.h.
        void                AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent);


        /**----------------------------------------------------------------------------
        Initiates the filtering of the point index, This filtering process includes
        recursively calling global pre-filtering, pre=filtering of nodes, filtering,
        node post-filtering then global post-filtering.

        -----------------------------------------------------------------------------*/
       // virtual void        Filter(int pi_levelToFilter);

        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(EXTENT extent);
        virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID);

    private:
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > m_graphPool;
        HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> m_graphStore;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher2_5d;
        ISMPointIndexMesher<POINT, EXTENT>* m_mesher3d;
        HFCPtr<SMPointTileStore<int32_t, EXTENT>> m_featureStore;
        HFCPtr<HPMCountLimitedPool<int32_t>> m_featurePool;
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

        virtual void LoadGraph() const override
            {
            return dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetParentNodePtr().GetPtr())->LoadGraph();
            };
        virtual MTGGraph* GetGraphPtr() override
            {
            return dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(GetParentNodePtr().GetPtr())->GetGraphPtr();
            };
        virtual bool IsVirtualNode() const override
            {
            volatile bool a = 1;
            a = a;
            return true;
            }

        virtual void SetDirty(bool dirty) const override
            {
            HPMCountLimitedPoolItem<POINT>::SetDirty(false);
            }

        virtual size_t size() const override
            {
            return GetParentNodePtr()->size();
            };

        virtual const POINT& operator[](size_t index) const override
            {
            return GetParentNodePtr()->operator[](index);
            };

        virtual bool Destroy() override
            {
            return false;
            };

        virtual void Load() const override
            {};


        virtual bool Store() const override
            {
            return false;
            };

        virtual bool Discard() const override
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

        virtual bool        ComputeObjectRelevance(HFCPtr<SMPointIndexNode<POINT, EXTENT> > outputNode) = 0;


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
                                   size_t numSubNodes,
                                   double viewParameters[]) const = 0;
        virtual bool        FilterLeaf(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                       double viewParameters[]) const 
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


#include "SMMeshIndex.hpp"
