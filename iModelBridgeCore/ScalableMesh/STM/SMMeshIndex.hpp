template <class POINT, class EXTENT> SMMeshIndexNode<POINT,EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
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
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                 m_graphPool(graphPool), m_graphStore(graphStore), m_graphVec(graphPool, graphStore)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_featureStore = nullptr;
    m_featurePool = nullptr;
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                const EXTENT& pi_rExtent,
                const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode)
                : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr())),
                m_graphPool(pi_rpParentNode->GetGraphPool()), m_graphStore(pi_rpParentNode->GetGraphStore()), m_graphVec(m_graphPool, m_graphStore),
                m_featureStore(pi_rpParentNode->m_featureStore), m_featurePool(pi_rpParentNode->m_featurePool)
    {
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,
                 const HFCPtr<SMMeshIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                 bool IsUnsplitSubLevel)
                 : SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, pi_rExtent, dynamic_cast<SMPointIndexNode*>(pi_rpParentNode.GetPtr()), IsUnsplitSubLevel),
                 m_graphPool(pi_rpParentNode->GetGraphPool()), m_graphStore(pi_rpParentNode->GetGraphStore()), m_graphVec(m_graphPool, m_graphStore),
                 m_featureStore(pi_rpParentNode->m_featureStore), m_featurePool(pi_rpParentNode->m_featurePool)
    {
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }


template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode)
    :SMPointIndexNode<POINT, EXTENT>(pi_rNode), m_graphPool(pi_rNode->GetGraphPool()), m_graphStore(pi_rNode->GetGraphStore()), m_graphVec(m_graphPool, m_graphStore),
    m_featureStore(pi_rNode->m_featureStore), m_featurePool(pi_rNode->m_featurePool)
    {
    m_mesher2_5d = pi_rNode.GetMesher2_5d();
    m_mesher3d = pi_rNode.GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMPointIndexNode<POINT, EXTENT>& pi_rNode)
    : SMPointIndexNode<POINT, EXTENT>(pi_rNode)
    {
    m_mesher2_5d = pi_rNode.GetMesher2_5d();
    m_mesher3d = pi_rNode.GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(const SMMeshIndexNode<POINT, EXTENT>& pi_rNode,
                                                                                       const HFCPtr<SMMeshIndexNode>& pi_rpParentNode)
                                                                                       : SMPointIndexNode<POINT, EXTENT>(pi_rNode, pi_rpParentNode),
                                                                                       m_graphPool(pi_rNode->GetGraphPool()), m_graphStore(pi_rNode->GetGraphStore()), m_graphVec(m_graphPool, m_graphStore),
                                                                                       m_featureStore(pi_rpParentNode->m_featureStore), m_featurePool(pi_rpParentNode->m_featurePool)
    {
    m_mesher2_5d = pi_rpParentNode->GetMesher2_5d();
    m_mesher3d = pi_rpParentNode->GetMesher3d();
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
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
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, static_pcast<SMPointIndexNode<POINT, EXTENT>, SMMeshIndexNode<POINT, EXTENT>>(parent), pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                 m_graphPool(graphPool), m_graphStore(graphStore), m_graphVec(graphPool, graphStore),
                 m_featureStore(parent->m_featureStore), m_featurePool(parent->m_featurePool)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                 HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap* createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, pool, store, filter, balanced, propagateDataDown, createdNodeMap),
                 m_graphPool(graphPool), m_graphStore(graphStore), m_graphVec(graphPool, graphStore),
                 m_featureStore(nullptr), m_featurePool(nullptr)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,
                 HFCPtr<HPMCountLimitedPool<POINT>> pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(pi_splitTreshold, pi_rExtent, pool, store, filter, balanced, propagateDataDown, createdNodeMap)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_featureStore = nullptr;
    m_featurePool = nullptr;
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap* createdNodeMap) 
                 : SMPointIndexNode<POINT, EXTENT>(blockID, pool, store, filter, balanced, propagateDataDown, mesher2_5d, mesher3d, createdNodeMap)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    m_featureStore = nullptr;
    m_featurePool = nullptr;
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::SMMeshIndexNode(HPMBlockID blockID,
                 HFCPtr<SMMeshIndexNode<POINT, EXTENT> > parent,
                 HFCPtr<HPMCountLimitedPool<POINT> > pool,
                 HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                 ISMPointIndexFilter<POINT, EXTENT>* filter,
                 bool balanced,
                 bool propagateDataDown,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                 ISMPointIndexMesher<POINT, EXTENT>* mesher3d,
                 CreatedNodeMap*                      createdNodeMap)
                 : SMPointIndexNode<POINT, EXTENT>(blockID, static_pcast<SMPointIndexNode<POINT, EXTENT>, SMMeshIndexNode<POINT, EXTENT>>(parent), pool, store, filter, balanced, propagateDataDown, mesher2_5d, mesher3d, createdNodeMap),
                 m_featureStore(parent->m_featureStore), m_featurePool(parent->m_featurePool)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    m_isGraphLoaded = false;
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_nodeHeader.m_graphID = IDTMFile::GetNullNodeID();
    }

template <class POINT, class EXTENT> SMMeshIndexNode<POINT, EXTENT>::~SMMeshIndexNode()
    {
    if (!IsDestroyed())
        {
        // Unload self ... this should result in discard 
        Unload();

        if (!Discarded() && IsDirty())
            {
            // This is a bit too late to discard as we are during destruction ... virtual overload of Discard is not accessible if inherited
            // Usually inheritance will not exist at this time ...
            Discard();
            }
        else if (m_graphVec.IsDirty())
            {
            if (IsGraphLoaded()) StoreGraph();
            else m_graphVec.SetDirty(false);
            }
        }
    }

template <class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::IsGraphLoaded() const
    {
    return m_isGraphLoaded;
    }

template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Destroy()
    {
    SMPointIndexNode::Destroy();
    //m_graphVec.clear();
    m_graphVec.SetDirty(false);
    m_graphVec.SetDiscarded(true);
    m_isGraphLoaded = false;
    if (m_graphVec.GetBlockID().IsValid())
        m_graphStore->DestroyBlock(m_graphVec.GetBlockID());

    HINVARIANTS;

    return true;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::Clone() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), GetNodeExtent(), GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetGraphPool(), GetGraphStore(), GetFilter(), IsBalanced(), PropagatesDataDown(), GetMesher2_5d(), GetMesher3d(), m_createdNodeMap));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::Clone(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetGraphPool(), GetGraphStore(), GetFilter(), IsBalanced(), PropagatesDataDown(), GetMesher2_5d(), GetMesher3d(), m_createdNodeMap));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChild(const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this), true));
    return pNewNode;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CloneUnsplitChildVirtual() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMIndexNodeVirtual<POINT,EXTENT,SMMeshIndexNode<POINT,EXTENT>>(const_cast<SMMeshIndexNode<POINT, EXTENT>*>(this)));
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewChildNode(HPMBlockID blockID)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(blockID, this, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), GetGraphPool(), GetGraphStore(), m_filter, m_nodeHeader.m_balanced, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap));
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndexNode<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(new SMMeshIndexNode<POINT, EXTENT>(blockID, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), GetGraphPool(), GetGraphStore(), m_filter, m_nodeHeader.m_balanced, !(m_delayedDataPropagation), m_mesher2_5d, m_mesher3d, m_createdNodeMap));
    return pNewNode;
    }


template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::Discard() const
    {
    HINVARIANTS;
    bool returnValue = true;

    if (!m_destroyed && !Discarded())
        {
        if (!m_graphVec.Discarded()) StoreGraph();
        else if (m_graphVec.GetBlockID().IsValid())  m_nodeHeader.m_graphID = m_graphVec.GetBlockID();
        returnValue = SMPointIndexNode<POINT, EXTENT>::Discard();
        }
    HINVARIANTS;

    return returnValue;

    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Load() const
    {
    if (IsLoaded()) return;
    SMPointIndexNode<POINT, EXTENT>::Load();
    m_graphVec.SetBlockID(m_nodeHeader.m_graphID);
    }

template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::Unload() const
    {
    if (m_featureDefinitions.size() > 0)
        {
        for (auto& vec : m_featureDefinitions) if(!vec.Discarded()) vec.Discard();
        }
    SMPointIndexNode<POINT, EXTENT>::Unload();
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::CreateGraph() const
    {
    m_graphVec.SetDiscarded(false);
    if (m_graphVec.size() == 0) m_graphVec.push_back(MTGGraph());
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::LoadGraph() const
    {
    if (m_graphVec.GetBlockID().IsValid() && (!IsGraphLoaded() || m_graphVec.Discarded()))
        {
        if (!m_graphVec.Discarded())
            {
            m_graphVec.SetDirty(false);
            m_graphVec.Discard();
            }
        m_graphVec.Inflate();
        m_isGraphLoaded = true;
        }
    }

template <class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::StoreGraph() const
    {
        if(m_graphVec.size() > 0) m_graphVec.Discard();
        m_nodeHeader.m_graphID = m_graphVec.GetBlockID();
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
                    static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Mesh();
                }

            }

        }
    else
        {
        assert(this->NeedsMeshing() == true);
        //assert(this->m_nodeHeader.m_balanced == true);

        bool isMeshed;

        if (m_nodeHeader.m_arePoints3d)
            {
            isMeshed = m_mesher3d->Mesh(this);
            }
        else
            {
            isMeshed = m_mesher2_5d->Mesh(this);
            }

        if (isMeshed)
            {
            SetDirty(true);
            }
        }


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

    size_t nodeInd;

    if (pi_levelToStitch == -1 || this->m_nodeHeader.m_level == pi_levelToStitch && this->GetNbObjects() > 0)
        {

        for (nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
            {
            if ((m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] == false) && (m_apNeighborNodes[nodeInd].size() > 0))
                {
                break;
                }
            }

        if (nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS)
            {
            if (nodesToStitch != 0)
                {
                nodesToStitch->push_back(this);
                }
            else
                {
bool isStitched;

if (AreAllNeighbor2_5d() && !m_nodeHeader.m_arePoints3d)
    {
    isStitched = m_mesher2_5d->Stitch(this);
    }
else
    {
    isStitched = m_mesher3d->Stitch(this);
    }

if (isStitched)
SetDirty(true);
                }
            }
        }

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
                        static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Stitch(pi_levelToStitch, nodesToStitch);
                        }
                    }
                }
            }


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

inline bool IsLinearFeature(IDTMFile::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Breakline || dtmType == DTMFeatureType::SoftBreakline || dtmType == DTMFeatureType::ContourLine || dtmType == DTMFeatureType::GraphicBreak;
    }

inline bool IsClosedFeature(IDTMFile::FeatureType type)
    {
    DTMFeatureType dtmType = (DTMFeatureType)type;
    return dtmType == DTMFeatureType::Hole || dtmType == DTMFeatureType::Island || dtmType == DTMFeatureType::Void || dtmType == DTMFeatureType::BreakVoid ||
        dtmType == DTMFeatureType::Polygon || dtmType == DTMFeatureType::Region || dtmType == DTMFeatureType::Contour || dtmType == DTMFeatureType::Hull;
    }

static size_t s_featuresAddedToTree = 0;

template<class EXTENT> void ClipFeatureDefinition(IDTMFile::FeatureType type, EXTENT clipExtent, bvector<DPoint3d>& points, DRange3d& extent, const bvector<DPoint3d>& origPoints, DRange3d& origExtent)
    {
    if (IsClosedFeature(type) || (origExtent.low.x >= ExtentOp<EXTENT>::GetXMin(clipExtent) && origExtent.low.y >= ExtentOp<EXTENT>::GetYMin(clipExtent) && origExtent.low.z >= ExtentOp<EXTENT>::GetZMin(clipExtent)
        && origExtent.high.x <= ExtentOp<EXTENT>::GetXMax(clipExtent) && origExtent.high.y <= ExtentOp<EXTENT>::GetYMax(clipExtent) && origExtent.high.z <= ExtentOp<EXTENT>::GetZMax(clipExtent)))
        {
        points.insert(points.end(), origPoints.begin(), origPoints.end());
        extent = origExtent;
        return;
        }
    if (IsLinearFeature(type))
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(clipExtent), ExtentOp<EXTENT>::GetYMin(clipExtent), ExtentOp<EXTENT>::GetZMin(clipExtent),
                                            ExtentOp<EXTENT>::GetXMax(clipExtent), ExtentOp<EXTENT>::GetYMax(clipExtent), ExtentOp<EXTENT>::GetZMax(clipExtent));
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


    template<class POINT, class EXTENT> void SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinitionUnconditional(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
        {
        if (!IsLoaded())
            Load();
        if (m_DelayedSplitRequested)
            SplitNode(GetDefaultSplitPosition());
        DRange3d extentClipped;
        bvector<DPoint3d> pointsClipped;
        ClipFeatureDefinition(type, m_nodeHeader.m_nodeExtent, pointsClipped, extentClipped, points, extent);
        if (!HasRealChildren() && this->size() == 0) m_nodeHeader.m_arePoints3d = false;
        if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
        if (!HasRealChildren() && (this->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode(GetDefaultSplitPosition());
            }
        else if (m_delayedDataPropagation && (this->size() + pointsClipped.size() >= m_nodeHeader.m_SplitTreshold))
            {
            PropagateDataDownImmediately(false);
            }

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
        if (!HasRealChildren() || (m_delayedDataPropagation && (this->size() + pointsClipped.size() < m_nodeHeader.m_SplitTreshold)))
            {
            ++s_featuresAddedToTree;
            vector<int32_t> indexes;

            for (auto pt : pointsClipped)
                {
                if (pt.x == DBL_MAX)
                    {
                    indexes.push_back(INT_MAX);
                    continue;
                    }
                POINT pointToInsert = PointOp<POINT>::Create(pt.x, pt.y, pt.z);
                this->push_back(pointToInsert);
                indexes.push_back((int32_t)this->size()-1);
                }
            if (m_featureDefinitions.capacity() < m_featureDefinitions.size() +1) for(auto& def : m_featureDefinitions) if(!def.Discarded()) def.Discard();
            m_featureDefinitions.resize(m_featureDefinitions.size() + 1);
            auto& newFeatureDef = m_featureDefinitions.back();
            newFeatureDef.SetStore(m_featureStore);
            newFeatureDef.SetPool(m_featurePool);
            newFeatureDef.push_back((int32_t)type);
            newFeatureDef.push_back(&indexes[0], indexes.size());
            }
        else
            {
            if (IsParentOfARealUnsplitNode())
                dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional(type, pointsClipped, extentClipped);
            else
                {
                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
                        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNode])->AddFeatureDefinition(type, pointsClipped, extentClipped, true);
                        }
                }
            }

        SetDirty(true);
        }

    template<class POINT, class EXTENT>  bool  SMMeshIndexNode<POINT, EXTENT>::AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent, bool ExtentFixed)
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
            if (points.size() + this->size() >= m_nodeHeader.m_SplitTreshold)
                {
                return AddFeatureDefinition(type, points, extent,true);
                }
            else
                {
                AddFeatureDefinitionUnconditional(type,points,extent);
                return true;
                }
        }
    else
        {
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));
        if (extent.IntersectsWith(nodeRange))
            {
            AddFeatureDefinitionUnconditional(type, points, extent);
            return true;
            }
        }
        return false;

    }

template<class POINT, class EXTENT>  size_t SMMeshIndexNode<POINT, EXTENT>::CountAllFeatures()
    {
    size_t nFeatures = IsLeaf() ? m_featureDefinitions.size() : 0;
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->CountAllFeatures();
        }
    else if (!IsLeaf())
        {
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                nFeatures += dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->CountAllFeatures();
        }
    return nFeatures;
    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPushNodeDown()
    {
    PropagateFeaturesToChildren();
    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::OnPropagateDataDown()
    {
    PropagateFeaturesToChildren();
    }

template<class POINT, class EXTENT>  void SMMeshIndexNode<POINT, EXTENT>::PropagateFeaturesToChildren()
    {
    bvector<bvector<DPoint3d>> featurePoints(m_featureDefinitions.size());
    bvector<DRange3d> extents(m_featureDefinitions.size());
    size_t featureId = 0;
    vector<int32_t> indices;
    DPoint3d SENTINEL_PT = DPoint3d::From(DBL_MAX, DBL_MAX, DBL_MAX);
    if (m_featureDefinitions.size() == 0) return;
    for (auto& feature : m_featureDefinitions)
        {
        --s_featuresAddedToTree;
        for (size_t pt = 1; pt < feature.size(); ++pt)
            {
            if (feature[pt] < INT_MAX)
                {
                POINT featurePt = this->operator[](feature[pt]);
                featurePoints[featureId].push_back(DPoint3d::From(PointOp<POINT>::GetX(featurePt), PointOp<POINT>::GetY(featurePt), PointOp<POINT>::GetZ(featurePt)));
                if (featurePoints[featureId].size() == 1) extents[featureId] = DRange3d::From(featurePoints[featureId].back());
                else extents[featureId].Extend(featurePoints[featureId].back());
                indices.push_back(feature[pt]);
                }
            else
                featurePoints[featureId].push_back(SENTINEL_PT);
            }
        ++featureId;
        }
    for (auto& index : indices)
        {
        this->erase(index);
        for (auto& pair : m_nodeHeader.m_3dPointsDescBins)
            {
            if (pair.m_startIndex > index) --pair.m_startIndex;
            }
        }
    if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pSubNodeNoSplit)->AddFeatureDefinitionUnconditional((IDTMFile::FeatureType)m_featureDefinitions[i][0], featurePoints[i], extents[i]);
        }
    else if (!IsLeaf())
        {
        for (size_t i = 0; i < featurePoints.size(); ++i)
            {
            bool added = false;
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                added = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_apSubNodes[indexNodes])->AddFeatureDefinition((IDTMFile::FeatureType)m_featureDefinitions[i][0], featurePoints[i], extents[i], true) || added;
            assert(added == true);
            }
        }
    for (auto& vec : m_featureDefinitions)
        {
        vec.clear();
        vec.Discard();
        }
    m_featureDefinitions.clear();

    }

    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 10/14
    //=======================================================================================
    template<class POINT, class EXTENT> bool SMMeshIndexNode<POINT, EXTENT>::NeedsMeshing() const
        {
        if (!IsLoaded())
            Load();

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

template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::SMMeshIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool,
                                                                               HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
                                                                               HFCPtr<HPMIndirectCountLimitedPool<MTGGraph> > graphPool,
                                                                               HFCPtr<IHPMPermanentStore<MTGGraph, Byte, Byte>> graphStore,
                                                                               size_t SplitTreshold,
                                                                               ISMPointIndexFilter<POINT, EXTENT>* filter,
                                                                               bool balanced,
                                                                               bool propagatesDataDown,
                                                                               ISMPointIndexMesher<POINT, EXTENT>* mesher2_5d,
                                                                               ISMPointIndexMesher<POINT, EXTENT>* mesher3d)
                                                                               : SMPointIndex<POINT, EXTENT>(pool, store, SplitTreshold, filter, balanced, propagatesDataDown), m_graphPool(graphPool), m_graphStore(graphStore)
    {
    m_mesher2_5d = mesher2_5d;
    m_mesher3d = mesher3d;
    if (0 == graphStore->LoadMasterHeader(NULL, 0)) graphStore->StoreMasterHeader(NULL, 0);
    m_featureStore = nullptr;
    m_featurePool = nullptr;
    s_importedFeatures = 0;

    }


template <class POINT, class EXTENT> SMMeshIndex<POINT, EXTENT>::~SMMeshIndex()
    {
    if (m_mesher2_5d != NULL)
        delete m_mesher2_5d;

    if (m_mesher3d != NULL)
        delete m_mesher3d;

    m_graphStore->Close();
    }


template <class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(EXTENT extent)
    {
    SMMeshIndexNode<POINT, EXTENT> * meshNode = new SMMeshIndexNode<POINT, EXTENT>(GetSplitTreshold(), extent, m_pool, m_store, m_graphPool, m_graphStore, m_filter, IsBalanced(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d,&m_createdNodeMap);
    meshNode->m_featureStore = m_featureStore;
    meshNode->m_featurePool = m_featurePool;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = dynamic_cast<SMPointIndexNode<POINT, EXTENT>*>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMMeshIndex<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID)
    {
    auto meshNode = new SMMeshIndexNode<POINT, EXTENT>(blockID, m_pool, m_store, m_graphPool, m_graphStore, m_filter, IsBalanced(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
    meshNode->m_featurePool = m_featurePool;
    meshNode->m_featureStore = m_featureStore;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = static_cast<SMPointIndexNode<POINT, EXTENT> *>(meshNode);
    pNewNode->m_isGenerating = m_isGenerating;
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


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::AddFeatureDefinition(IDTMFile::FeatureType type, bvector<DPoint3d>& points, DRange3d& extent)
    {
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == NULL)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    ++s_importedFeatures;
    if (0 == points.size())
        return;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent); // new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), m_indexHeader.m_MaxExtent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
        else
        m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z));
        }
    if (!dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent))
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
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->AddFeatureDefinition(type, points, extent, m_indexHeader.m_HasMaxExtent);
        }
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


template<class POINT, class EXTENT>  void  SMMeshIndex<POINT, EXTENT>::SetFeatureStore(HFCPtr<SMPointTileStore<int32_t, EXTENT>>& featureStore)
    {
    m_featureStore = featureStore;
    if (m_pRootNode != NULL)   dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->SetFeatureStore(m_featureStore);
    }

template<class POINT, class EXTENT>  void SMMeshIndex<POINT, EXTENT>::SetFeaturePool(HFCPtr<HPMCountLimitedPool<int32_t>>& featurePool)
    {
    m_featurePool = featurePool;
    if (m_pRootNode != NULL)  dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode)->SetFeaturePool(m_featurePool);
    }
