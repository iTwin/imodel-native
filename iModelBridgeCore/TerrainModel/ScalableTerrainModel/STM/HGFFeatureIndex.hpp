//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFFeatureIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/all/h/HPMPooledVector.h>
#include "HGFPointTileStore.h"


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGFFeatureIndexNode Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

/** -----------------------------------------------------------------------------

    This class implements a spatial index default node. It comforms to the
    IHGFSpatialIndexNode pseudo interface defined above. This default
    node maintains in memory all of the data.

    -----------------------------------------------------------------------------
*/
//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode(size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store,
        bool balanced)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, balanced, true, false, false),
    m_discarded(false),
    m_dirty(true)
    {
    m_TotalPointsCount = -1;
    m_storeBlockID = HPMBlockID();

    SetStore (store);

    for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& pi_rpParentNode)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode),
    m_discarded(false),
    m_dirty(true)
    {
    m_TotalPointsCount = -1;
    m_storeBlockID = HPMBlockID();
    SetStore (static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(pi_rpParentNode->GetStore())));
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }

    ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& pi_rpParentNode,
        bool IsUnsplitSubLevel)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode, true),
    m_discarded(false),
    m_dirty(true)
    {
    m_TotalPointsCount = -1;
    m_storeBlockID = HPMBlockID();

    SetStore (static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(pi_rpParentNode->GetStore())));
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode (const HGFFeatureIndexNode<FEATURE, POINT, EXTENT>& pi_rNode)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(&*pi_rNode),
    m_discarded(false),
    m_dirty(pi_rNode->m_dirty)
    {
    m_TotalPointsCount = -1;
    SetStore (static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(pi_rNode->GetStore())));
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode(const HGFFeatureIndexNode& pi_rNode,
        const HFCPtr<HGFFeatureIndexNode>& pi_rpParentNode)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(&*pi_rNode, &*pi_rpParentNode),
    m_discarded(false),
    m_dirty(pi_rNode->m_dirty)
    {
    m_TotalPointsCount = -1;
    m_storeBlockID = HPMBlockID();
    SetStore (static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(pi_rNode->GetStore())));
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::HGFFeatureIndexNode(HPMBlockID blockID,
        HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > parent,
        HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store)
    :   HGFIndexNode<FEATURE, POINT, EXTENT, vector<FEATURE>, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >, HGFFeatureNodeHeader<EXTENT> >(false),
        m_dirty(false)
    {
    HPRECONDITION (blockID.IsValid());
    m_TotalPointsCount = -1;
    m_loaded = false;
    m_discarded = true;
    SetStore (store);
    m_storeBlockID = blockID;
    m_pParentNode = parent;


    ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::~HGFFeatureIndexNode()
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
    	}
    }


/**----------------------------------------------------------------------------
 @bsimethod                                         AlainRobert
-----------------------------------------------------------------------------*/
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Clone () const
    {
    HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pNewNode = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(GetSplitTreshold(), GetNodeExtent(), dynamic_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>* >(&*(GetStore())), IsBalanced());
    return pNewNode;
    }
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Clone (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pNewNode = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, dynamic_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>* >(&*(GetStore())), IsBalanced());
    return pNewNode;
    }
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::CloneChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pNewNode = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(this));
    return pNewNode;
    }
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::CloneUnsplitChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pNewNode = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(this), true);
    return pNewNode;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HPMBlockID HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::GetBlockID() const
    {
    return m_storeBlockID;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> void HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Load() const
    {
    HPRECONDITION (!IsLoaded());

    if (0 == static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*m_store)->LoadHeader (&m_nodeHeader, m_storeBlockID))
        {
        // Something went wrong
        throw HFCUnknownException();
        }

    HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* UNCONSTTHIS =  const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* >(this);

    UNCONSTTHIS->Inflate();

    // If there are sub-nodes we must create them
    if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
        {
        if (!UNCONSTTHIS->m_nodeHeader.m_IsBranched)
            UNCONSTTHIS->m_pSubNodeNoSplit = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT> (UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID, UNCONSTTHIS, static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)));
        else
            {

            // ATTENTION! DO NOT CALL GetNumberOfSubNodesOnSplit() FUNCTION AS IT WILL CALL Load() RESULTING
            // INTO AN INFINITE LOOP
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                UNCONSTTHIS->m_apSubNodes[indexNode] = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode], UNCONSTTHIS, static_cast<HGFFeatureTileStore<FEATURE, POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)));
                }
            }
        }

    m_loaded = true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> void HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Unload() const
	{
    if (IsLoaded())
    	{
        HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* UNCONSTTHIS =  const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* >(this);

        // If there are sub-nodes we must create them
        if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
        	{
            if (UNCONSTTHIS->m_pSubNodeNoSplit != NULL)
            	{
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = false;

                UNCONSTTHIS->m_pSubNodeNoSplit->Unload();
            	}
            else
            	{
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = true;
            	

                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                	{
                    UNCONSTTHIS->m_apSubNodes[indexNode]->Unload();
                	}
            	}
        	}
        
        if (!Discarded() && IsDirty())
        	{
            Discard();
        	}


        // If there are sub-nodes we must create them
        if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
        	{
            if (UNCONSTTHIS->m_pSubNodeNoSplit != NULL)
            	{
                UNCONSTTHIS->m_pSubNodeNoSplit->m_pParentNode = NULL;
                UNCONSTTHIS->m_pSubNodeNoSplit = NULL;
            	}
            else
            	{
                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                	{
                    UNCONSTTHIS->m_apSubNodes[indexNode]->m_pParentNode = NULL;
                    UNCONSTTHIS->m_apSubNodes[indexNode] = NULL;
                	}
            	}
        	}

        m_loaded = false;
    	}

	}


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Destroy()
    {
    HGFIndexNode::Destroy();
    if (GetBlockID().IsValid())
        m_store->DestroyBlock(GetBlockID());

    return true;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::GetStore() const
    {
    return m_store;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::SetStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore)
    {
    // Change the store
    m_store = newStore;

    // Sabotage block id ... this should be sufficient
    m_storeBlockID = HPMBlockID();

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::ChangeStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore)
    {
    // Call node
    if (!IsLeaf())
        {
        if (static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit) != NULL)
            static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->ChangeStore(newStore);

        for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
            {
            if (m_apSubNodes[indexNode] != NULL)
                static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ChangeStore(newStore);
            }
        }

    // Call ancester
    // First load into memory
    if (m_storeBlockID.IsValid() && m_discarded)
        Inflate();

    // Change the store
    m_store = newStore;

    // Sabotage block id ... this should be sufficient
    m_storeBlockID = HPMBlockID();

    SetDirty(true);
    Discard();

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Store()
    {
    HINVARIANTS;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Store();
            HASSERT (m_nodeHeader.m_SubNodeNoSplitID.IsValid());
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Store();
                HASSERT (m_nodeHeader.m_apSubNodeID[indexNode].IsValid());
                }
            }

        }

    if (!Discarded())
        {
        return Discard();
        }
    else
        {
        if (IsDirty())
            {
            Inflate();
            Discard();
            }
        }

    return true;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::IsDirty() const
    {
    return m_dirty;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> void HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
    {
    if (!IsLoaded())
        Load();

    // Invalidate all cached counts
    m_NbObjects = -1;
    m_TotalPointsCount = -1;
    m_nodeHeader.m_totalCountDefined = false;

    m_dirty = dirty;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Discarded() const
    {
    return m_discarded;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Discard() const
    {
    HINVARIANTS;
    HASSERT (!Discarded());

    bool returnValue = true;

    if (!m_destroyed)
        {
        if (!IsLoaded())
            Load();

        // Save the current blockID
        HPMBlockID currentBlockId = GetBlockID();

        if (IsDirty() == true)
        {
        FEATURE* arrayOfFeatures = new FEATURE[this->size()];

        for (size_t indexFeature = 0 ; indexFeature < this->size() ; indexFeature++)
            arrayOfFeatures[indexFeature] = this->operator[](indexFeature);

        m_storeBlockID = m_store->StoreBlock(arrayOfFeatures, this->size(), m_storeBlockID);

        delete [] arrayOfFeatures;
        }

        m_discarded = true;
        SetDirty (false);

        if (m_storeBlockID.IsValid())
            m_store->StoreHeader(&m_nodeHeader, GetBlockID()); // TODO: Why do we store the header when not dirty??

        if (returnValue && (currentBlockId != GetBlockID()))
            {
            HASSERT(GetBlockID().IsValid());

            // Block ID changed (tile reallocated on store ... advise parent)
            if (GetParentNode() != NULL)
                GetParentNode()->AdviseSubNodeIDChanged(const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(this));
            }
        }

    return returnValue;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Inflate()  const
    {
    HPRECONDITION(Discarded());

    HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* UNCONSTTHIS =  const_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>* >(this);

    size_t Count = m_store->GetBlockDataCount (m_storeBlockID);

    if (Count != 0)
        {
        size_t countToAllocate = Count + 10;

//        this->reserve (countToAllocate);

        FEATURE* arrayOfFeatures = new FEATURE[countToAllocate];

        m_store->LoadBlock (arrayOfFeatures, Count+1, m_storeBlockID);

        for (size_t indexFeature = 0 ; indexFeature < Count ; indexFeature++)
            UNCONSTTHIS->push_back(arrayOfFeatures[indexFeature]);


        delete [] arrayOfFeatures;
        }
    m_discarded = false;
    return true;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> void HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::AdviseSubNodeIDChanged(const HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >& p_subNode)
    {
    if (!IsLoaded())
        Load();

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (p_subNode == m_pSubNodeNoSplit)
            m_nodeHeader.m_SubNodeNoSplitID = p_subNode->GetBlockID();

        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (p_subNode == m_apSubNodes[indexNode])
                    {
                    m_nodeHeader.m_apSubNodeID[indexNode] = p_subNode->GetBlockID();
                    break;
                    }
                }
            }
        SetDirty(true);
        }
    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> void HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::SetSubNodes(HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes)
    {
    HINVARIANTS;

    HGFIndexNode::SetSubNodes(pi_apSubNodes, numSubNodes);


    size_t indexNode;
    for (indexNode = 0 ; indexNode < numSubNodes; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = m_apSubNodes[indexNode]->GetBlockID();
        }
//    SetDirty(true);
    }








/**----------------------------------------------------------------------------
 Performs the pre-query process upon node and sub-nodes

-----------------------------------------------------------------------------*/
// template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::PreQuery (IHGFFeatureIndexQuery<FEATURE, POINT, EXTENT>* queryObject)
// {
//     HINVARIANTS;
//     if (!IsLoaded())
//         Load();
//
//     bool digDown = true;
//
//     if (!IsLeaf ())
//     {
//         if (m_pSubNodeNoSplit != NULL)
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//             pSubNodes[0] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
//             digDown = queryObject->PreQuery (this, pSubNodes, 1);
//
//             if (digDown)
//                 static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PreQuery (queryObject);
//         }
//         else
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>> subNodes[8];
//             for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
//                 subNodes[indexNodes] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
//             digDown = queryObject->PreQuery (this, subNodes, GetNumberOfSubNodesOnSplit());
//
//             if (digDown)
//             {
//                 for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
//                 {
//                     static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PreQuery(queryObject);
//                 }
//             }
//
//         }
//     }
//     else
//     {
//         HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//         queryObject->PreQuery (this, pSubNodes, 0);
//     }
//
//     return digDown;
// }
// /**----------------------------------------------------------------------------
//  Performs the pre-query process upon node and sub-nodes
//
// -----------------------------------------------------------------------------*/
// template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::Query (IHGFFeatureIndexQuery<FEATURE, POINT, EXTENT>* queryObject, list<POINT>& resultFeatures)
// {
//     HINVARIANTS;
//     if (!IsLoaded())
//         Load();
//
//     bool digDown = true;
//
//     if (!IsLeaf ())
//     {
//         if (m_pSubNodeNoSplit != NULL)
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//             pSubNodes[0] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
//             digDown = queryObject->Query (this, pSubNodes, 1, resultFeatures);
//
//             if (digDown)
//                 static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultFeatures);
//         }
//         else
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>> subNodes[8];
//             for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
//                 subNodes[indexNodes] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
//
//             digDown = queryObject->Query (this, subNodes, GetNumberOfSubNodesOnSplit(), resultFeatures);
//
//             if (digDown)
//             {
//                 for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
//                 {
//                     static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->Query(queryObject, resultFeatures);
//                 }
//             }
//
//         }
//     }
//     else
//     {
//         HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//         queryObject->Query (this, pSubNodes, 0, resultFeatures);
//     }
//
//
//     return digDown;
// }
// /**----------------------------------------------------------------------------
//  Performs the post-query process upon node and sub-nodes
//
// -----------------------------------------------------------------------------*/
// template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::PostQuery (IHGFFeatureIndexQuery<FEATURE, POINT, EXTENT>* queryObject)
// {
//     HINVARIANTS;
//     if (!IsLoaded())
//         Load();
//
//     bool digDown = true;
//
//     if (!IsLeaf ())
//     {
//         if (m_pSubNodeNoSplit != NULL)
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//             pSubNodes[0] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
//             digDown = queryObject->PostQuery (this, pSubNodes, 1);
//
//             if (digDown)
//                 static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PostQuery (queryObject);
//         }
//         else
//         {
//             HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>> subNodes[8];
//             for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
//                 subNodes[indexNodes] = static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
//             digDown = queryObject->PostQuery (this, subNodes, GetNumberOfSubNodesOnSplit());
//
//             if (digDown)
//             {
//                 for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
//                 {
//                     static_cast<HGFFeatureIndexNode<FEATURE, POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PostQuery(queryObject);
//                 }
//             }
//
//         }
//     }
//     else
//     {
//         HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> > pSubNodes[1];
//         queryObject->PostQuery (this, pSubNodes, 0);
//     }
//     return digDown;
// }
//

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> size_t HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
        list<FEATURE>& pio_rListOfObjects) const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // Check if coordinate falls inside node extent
#if (0)

    if (ExtentOp<EXTENT>::Overlap(GetNodeExtent(), pi_rExtent))
#else
    if ((ExtentOp<EXTENT>::GetXMin(GetNodeExtent()) < ExtentOp<EXTENT>::GetXMax(pi_rExtent)) &&
        (ExtentOp<EXTENT>::GetXMax(GetNodeExtent()) > ExtentOp<EXTENT>::GetXMin(pi_rExtent)) &&
        (ExtentOp<EXTENT>::GetYMin(GetNodeExtent()) < ExtentOp<EXTENT>::GetYMax(pi_rExtent)) &&
        (ExtentOp<EXTENT>::GetYMax(GetNodeExtent()) > ExtentOp<EXTENT>::GetYMin(pi_rExtent)))
#endif
        {
        // The feature is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (!IsLeaf())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                m_pSubNodeNoSplit->GetIn (pi_rExtent, pio_rListOfObjects);
                }
            else
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                for (size_t indexNodes = 0 ;  indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    m_apSubNodes[indexNodes]->GetIn(pi_rExtent, pio_rListOfObjects);
                    }

                }
            }

        // Search in present list of objects for current node
        for (size_t currentIndex = 0 ; currentIndex < this->size(); currentIndex++)
            {
            // Check if feature is in extent of object
#if (0)
            if (SpatialOp<FEATURE, POINT, EXTENT>::IsSpatialInExtent2D (this->operator[](currentIndex), pi_rExtent))
#else
            EXTENT spatialExtent = SpatialOp<FEATURE, POINT, EXTENT>::GetExtent(this->operator[](currentIndex));
            if ((ExtentOp<EXTENT>::GetXMin(spatialExtent) < ExtentOp<EXTENT>::GetXMax(pi_rExtent)) &&
                (ExtentOp<EXTENT>::GetXMax(spatialExtent) > ExtentOp<EXTENT>::GetXMin(pi_rExtent)) &&
                (ExtentOp<EXTENT>::GetYMin(spatialExtent) < ExtentOp<EXTENT>::GetYMax(pi_rExtent)) &&
                (ExtentOp<EXTENT>::GetYMax(spatialExtent) > ExtentOp<EXTENT>::GetYMin(pi_rExtent)))
#endif
                {
                // The feature falls inside extent of object .. we add a reference to the list
                pio_rListOfObjects.push_back(this->operator[](currentIndex));
                }
            }
        }


    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> size_t HGFFeatureIndexNode<FEATURE, POINT, EXTENT>::GetPointCount() const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_TotalPointsCount == -1)
        {
        size_t count = 0;

        size_t featureCount = this->size();
        for (size_t featureIndex = 0 ; featureIndex < featureCount; featureIndex++)
            {
            count += SpatialOp<FEATURE, POINT, EXTENT>::GetPointCount(this->operator[](featureIndex));
            }

        if (!IsLeaf())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                count += m_pSubNodeNoSplit->GetPointCount();
                }
            else
                {
                for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit(); ++ i)
                    {
                    count += m_apSubNodes[i]->GetPointCount();
                    }
                }
            }
        m_TotalPointsCount = (uint32_t)count;
        }

    HINVARIANTS;

    return m_TotalPointsCount;
    }



//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGFFeatureIndex Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndex<FEATURE, POINT, EXTENT>::HGFFeatureIndex(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > store, size_t pi_SplitTreshold, bool balanced)
    : HGFSpatialIndex <FEATURE, POINT, EXTENT, HGFFeatureIndexNode<FEATURE, POINT, EXTENT>, HGFFeatureIndexHeader<EXTENT> >(pi_SplitTreshold, balanced, true, new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(pi_SplitTreshold, ExtentOp<EXTENT>::Create(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), store, balanced)),
      m_store (store)
    {
    // If a store is provided ...
    if (store != NULL)
        {
        // Try to load master header
        if (0 != store->LoadMasterHeader(&m_indexHeader, sizeof(m_indexHeader)))
            {
            // File allready contains a DTM ... load it
            // First reset pool pointer and store pointer that got wiped during load
            m_store = store;
            // (Split threshold, max extent and so on are ignored (we retain stored value)
            // In this particular case, the root node is created immediately

            if (m_indexHeader.m_rootNodeBlockID.IsValid())
                m_pRootNode = new HGFFeatureIndexNode<FEATURE, POINT, EXTENT>(m_indexHeader.m_rootNodeBlockID, HFCPtr<HGFFeatureIndexNode<FEATURE, POINT, EXTENT> >(NULL), store);

            // Master header just loaded.
            m_indexHeaderDirty = false;

            }
        else
            {
            // No master header ... force writting it (to prime the store)
            store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader));

            // Master header just stored
            m_indexHeaderDirty = false;
            }

        }
    HINVARIANTS;
    }




/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
// template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndex<FEATURE, POINT, EXTENT>::HGFFeatureIndex(const HGFFeatureIndex& pi_rObj)
// : HGFFeatureIndex<FEATURE, POINT, EXTENT, HGFFeatureIndexNode> (pi_rObj)
// {
//     HINVARIANTS;
//      if (pi_rObj != this)
//      {
//      m_LastNode = 0;
//      m_indexHeader.m_SplitTreshold = pi_rObj.GetSplitTreshold();
//      m_indexHeader.m_MaxExtent = pi_rObjm_indexHeader..m_MaxExtent;
//      m_indexHeader.m_HasMaxExtent = pi_rObjm_indexHeader..m_HaxMaxExtent;
//         m_indexHeader.m_rootNodeBlockID = 0;
//
//              m_pool = pi_rObj->GetPool();
//         m_store = pi_rObj->GetStore();
//      }
// }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HGFFeatureIndex<FEATURE, POINT, EXTENT>::~HGFFeatureIndex()
    {
    // Store root node
    if (m_pRootNode != NULL)
        {
        m_pRootNode->Store();
        HPMBlockID newBlockID = m_pRootNode->GetBlockID();
        if (m_indexHeader.m_rootNodeBlockID != newBlockID)
        {
            m_indexHeader.m_rootNodeBlockID = newBlockID;
            m_indexHeaderDirty = true;
        }
        m_pRootNode->Unload();
        m_pRootNode = NULL; // This should provoque storage of all nodes
        }

    if (m_indexHeaderDirty)
    {
        m_store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader));
        m_indexHeaderDirty = false;
    }

    // Close store
    m_store->Close();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> bool HGFFeatureIndex<FEATURE, POINT, EXTENT>::ChangeStore(HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > newStore)
    {
    // Set the store
    m_store = newStore;

    m_pRootNode->ChangeStore (newStore);

    // Store master header
    m_store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader));


    return true;
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> HFCPtr<HGFFeatureTileStore<FEATURE, POINT, EXTENT> > HGFFeatureIndex<FEATURE, POINT, EXTENT>::GetStore() const
    {
    HINVARIANTS;
    return m_store;
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> size_t HGFFeatureIndex<FEATURE, POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
        list<FEATURE>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    if (0 != m_pRootNode)
        m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 03/11
//=======================================================================================
template<class FEATURE, class POINT, class EXTENT> size_t HGFFeatureIndex<FEATURE, POINT, EXTENT>::GetPointCount() const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetPointCount();
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
// template<class FEATURE, class POINT, class EXTENT> size_t HGFFeatureIndex<FEATURE, POINT, EXTENT>::Query (IHGFFeatureIndexQuery<FEATURE, POINT, EXTENT>* queryObject,
//                                                                                 list<POINT>& resultFeatures)
// {
//     // Save the number of objects currently in list
//     size_t InitialNumberOfObjects = resultFeatures.size();
//
//     if (m_pRootNode != NULL)
//     {
//         // Call global pre query
//         if (queryObject->GlobalPreQuery (*this, resultFeatures))
//         {
//             try
//             {
//                 try
//                 {
//                     m_pRootNode->PreQuery (queryObject);
//
//                     m_pRootNode->Query (queryObject, resultFeatures);
//                 }
//                 catch (...)
//                 {
//                     m_pRootNode->PostQuery (queryObject);
//                     throw;
//                 }
//
//                 m_pRootNode->PostQuery (queryObject);
//             }
//             catch (...)
//             {
//                 queryObject->GlobalPostQuery (*this, resultFeatures);
//                 throw;
//             }
//
//             // Call global post query
//             queryObject->GlobalPostQuery (*this, resultFeatures);
//         }
//     }
//
//     // Return number of newly found objects
//     return(resultFeatures.size()- InitialNumberOfObjects);
// }

