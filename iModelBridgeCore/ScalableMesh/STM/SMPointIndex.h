//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMPointIndex.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HFCPtr.h>

#include <ImagePP/all/h/HGF3DExtent.h>
//#include <ImagePP/all/h/IDTMTypes.h>
//#include <ImagePP/all/h/ISMStore.h>

#include <ImagePP/all/h/HPMPooledVector.h>

#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HVE2DSegment.h>
//#include "HGFSpatialIndex.h"
#include "SMMemoryPool.h"
#include <ImagePP/all/h/HVEShape.h>

#include <ScalableMesh\IScalableMeshQuery.h>

#include "Stores\SMSQLiteStore.h"
#include "SMNodeGroup.h"

class DataSourceAccount;

USING_NAMESPACE_BENTLEY_SCALABLEMESH


#define MAX_NUM_SUBNODES 8
#define MAX_NUM_NEIGHBORNODE_POSITIONS 26
namespace BENTLEY_NAMESPACE_NAME
    {
    namespace ScalableMesh
        {
        class ScalableMeshMesh;

        extern std::atomic<uint64_t> s_nextNodeID;

        }
    }
//NEEDS_WORK_SM : Temp global variable probably only for debug purpose, not sure we want to know if we are in editing.
extern bool s_inEditing; 
extern bool s_useThreadsInFiltering;

// Predeclaration of the Point Index Filter interface. This interface is defined lower in this same file.
template<class POINT, class EXTENT> class ISMPointIndexFilter; 
template<class POINT, class EXTENT> class ISMPointIndexQuery;
template<class POINT, class EXTENT> class SMPointIndex;


/** -----------------------------------------------------------------------------

    This class implements a point index node.

    -----------------------------------------------------------------------------
*/

template<bool, class T> class OverrideSizeTypeTrait : public T
    {};
//This makes sure size() is only overriden on the HPMPooledVector
template<class T> class OverrideSizeTypeTrait<true, T> : public T
    {
    private:
    
    
    public:

    void setNbPointsUsedForMeshIndex(size_t nbPointsUsedForMeshIndex) const
        {
        m_nbPointsUsedForMeshIndex = nbPointsUsedForMeshIndex;
        }
    };


namespace BENTLEY_NAMESPACE_NAME
    {
    namespace ScalableMesh
        {        
        template <class POINT, class EXTENT> class NodeQueryProcessor;
        class ScalableMeshProgressiveQueryEngine;      
        }
    }


struct IStopQuery 
    {    
                           
    public:
        
        virtual bool DoStop() const = 0;    
    };


struct IDisplayCacheNodeManager
    {    
                           
    public:
        
        virtual bool IsNodeCached(HPMBlockID blockId) const = 0;
    };


template <class POINT, class EXTENT> class ProducedNodeContainer;

template <class POINT, class EXTENT> class SMMeshIndex;    
template <class POINT, class EXTENT> class SMMeshIndexNode;    

template <class POINT, class EXTENT> class SMPointIndexNode : public HFCShareableObject<SMPointIndexNode<POINT, EXTENT>>
    {
    friend class ISMPointIndexFilter<POINT, EXTENT>;
    friend class SMPointIndex<POINT, EXTENT>;    
    friend class SMMeshIndex<POINT, EXTENT>;    
    friend class SMMeshIndexNode<POINT, EXTENT>;    
    friend class BENTLEY_NAMESPACE_NAME::ScalableMesh::NodeQueryProcessor<POINT, EXTENT>;
    friend class BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshProgressiveQueryEngine;

    //typedef OverrideSizeTypeTrait<std::is_base_of<HPMPooledVector<POINT>, HPMStoredPooledVector<POINT>>::value, HPMStoredPooledVector<POINT>> CONTAINER;
    typedef HPMStoredPooledVector<POINT> CONTAINER;

public:
    
    class QueriedNode 
        {
        public :

            QueriedNode(HFCPtr<SMPointIndexNode<POINT, EXTENT>> indexNode, 
                        bool                                    isOverview = false)
                {
                m_indexNode = indexNode;
                m_isOverview = isOverview;
                }

            bool                                    m_isOverview;
            HFCPtr<SMPointIndexNode<POINT, EXTENT>> m_indexNode;
        };

    typedef std::map<__int64, HFCPtr<SMPointIndexNode<POINT, EXTENT>>> CreatedNodeMap;

   
    SMPointIndexNode(uint64_t nodeInd, 
                      size_t pi_SplitTreshold,
                      const EXTENT& pi_rExtent,                                            
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown,                                            
                      CreatedNodeMap*                      createdNodeMap);
  
    SMPointIndexNode<POINT, EXTENT>(size_t pi_SplitTreshold,
                                      const EXTENT& pi_rExtent,
                                      const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode);
    
    SMPointIndexNode<POINT, EXTENT>(size_t pi_SplitTreshold,
                                      const EXTENT& pi_rExtent,
                                      const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode,
                                      bool IsUnsplitSubLevel);
        
   
    SMPointIndexNode(HPMBlockID blockID,
                     HFCPtr<SMPointIndexNode<POINT, EXTENT> > parent,                                          
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown, 
                      CreatedNodeMap*                      createdNodeMap);
 
    SMPointIndexNode(HPMBlockID blockID,                      
                      ISMPointIndexFilter<POINT, EXTENT>* filter,
                      bool balanced,
                      bool propagateDataDown, 
                      CreatedNodeMap* createdNodeMap);

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    virtual ~SMPointIndexNode<POINT, EXTENT>();

    virtual bool IsVirtualNode() const
        {
        return false;
        }
        
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChild(const EXTENT& newNodeExtent) const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CloneUnsplitChildVirtual() const;
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewChildNode(HPMBlockID blockID);
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false);

        
            bool IsDirty() const;

    virtual void SetDirty(bool dirty); 
                
    /**----------------------------------------------------------------------------
    Sets the parent node ..

    @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    virtual void SetParentNode(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode);

    /**----------------------------------------------------------------------------
    Returns the next available node ID

    @return next available node ID
    -----------------------------------------------------------------------------*/
    uint64_t GetNextID() const;

    virtual RefCountedPtr<SMMemoryPoolVectorItem<POINT>> GetPointsPtr(bool loadPts = true);
            
    /**----------------------------------------------------------------------------
    Returns the parent node

    @return reference to the parent node
    -----------------------------------------------------------------------------*/
    const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& GetParentNode() const;

    //NEEDS_WORK_SM : In the original STM code sometime SMPointIndexNode access directly to the pointer without loading 
    //the parent node. 
    const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& GetParentNodePtr() const;

    bool  IsParentSet() const { return m_isParentNodeSet; }
    
    /**----------------------------------------------------------------------------
    Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
    size_t GetSplitTreshold() const;

    /**----------------------------------------------------------------------------
    Returns the number of subnodes on split. This number can be 4 or 8

    -----------------------------------------------------------------------------*/
    size_t GetNumberOfSubNodesOnSplit() const;

    /**----------------------------------------------------------------------------
    Invalidate the stitching that could have been done in a node.
    -----------------------------------------------------------------------------*/
    bool InvalidateStitching();

    /**----------------------------------------------------------------------------
    Invalidate the filtering and meshing that could have been done in a node.
    -----------------------------------------------------------------------------*/
    bool InvalidateFilteringMeshing(bool becauseDataRemoved = false);

    /**----------------------------------------------------------------------------
    Invalidate the filtering in all filtered ancestor nodes.
    -----------------------------------------------------------------------------*/
    bool InvalidateParentFiltering();

    /**----------------------------------------------------------------------------
    Indicates if the node requires a balanced index
    -----------------------------------------------------------------------------*/
    bool IsBalanced() const;

    bool IsTextured() const;

    /*
    Recursively set nodes as balanced or not.
    */
    void  SetBalanced(bool balanced);

    /**----------------------------------------------------------------------------
    Changes an unbalanced index to a balanced index. If the index is already balanced
    nothing will occur otherwise the balancing will immediately be performed.
    -----------------------------------------------------------------------------*/
    void Balance(size_t depth);

    /**----------------------------------------------------------------------------
    Pull up sub node.
    -----------------------------------------------------------------------------*/
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > PullSubNodeNoSplitUp(size_t levelToPullTo);

    /**----------------------------------------------------------------------------
    Rebalance part of the index that hasn't been modified but might not be at the
    same level as the modified part. This function should be called only during
    modification/editing of the index.
    -----------------------------------------------------------------------------*/
    void ReBalance(size_t depth);

    /**----------------------------------------------------------------------------
    Unsplit empty node after data removal
    -----------------------------------------------------------------------------*/
    bool UnsplitEmptyNode();

    /**----------------------------------------------------------------------------
    Changes a balanced index to an unbalanced index. This function only sets the
    balance field of the node and subnodes to false. The topology of the index is not
    modified.
    -----------------------------------------------------------------------------*/
    void Unbalance();

    /**----------------------------------------------------------------------------
    Indicates if the node propagates data toward nodes upon addition or not.
    -----------------------------------------------------------------------------*/
    bool PropagatesDataDown() const;

    /**----------------------------------------------------------------------------
    Changes an Propagate Data Down setting. Changing this value will not provoke a
    propagation down of the data currnetly located in the parent nodes but deactivating
    this propagation will prevent data to immediately propagate data. Newly added
    data will remain in upper parent nodes till required.
    -----------------------------------------------------------------------------*/
    void SetPropagateDataDown(bool propagate);


    /**----------------------------------------------------------------------------
    Move down the node extent of this node and all sub-node to ensure that 2.5D
    nodes have no neighbor node below them.
    -----------------------------------------------------------------------------*/
    virtual void        MoveDownNodes(double moveDownDistance);

    /**----------------------------------------------------------------------------
    Returns the depth level of a node in the index
    -----------------------------------------------------------------------------*/
    size_t GetLevel() const;

    /**----------------------------------------------------------------------------
    Decreases the level by the number of levels passed in parameter ... this decrease will propagate to all subnodes.
    -----------------------------------------------------------------------------*/
    virtual void DecreaseLevel(size_t numberOfLevels);

    /**----------------------------------------------------------------------------
    Increases the level by one ... this increase will propagate to all subnodes.
    -----------------------------------------------------------------------------*/
    virtual void IncreaseLevel();

    /**----------------------------------------------------------------------------
    Increases the total count for this and all ancestor nodes.
    -----------------------------------------------------------------------------*/
    virtual void IncreaseTotalCount(size_t nOfPoints);

    /**----------------------------------------------------------------------------
    Returns the highest depth level of a node sub-index
    -----------------------------------------------------------------------------*/
    size_t GetDepth() const;

    void GetAllNeighborNodes(vector<SMPointIndexNode*>& nodes) const;

    /**----------------------------------------------------------------------------
    Indicates if node is leaf

    @return true if node is a leaf
    -----------------------------------------------------------------------------*/
    bool IsLeaf() const;

    /**----------------------------------------------------------------------------
    Returns node extent

    @return reference to extent of node
    -----------------------------------------------------------------------------*/
    const EXTENT& GetNodeExtent() const;

    /**----------------------------------------------------------------------------
    Returns node content extent

    @return reference to extent of content of node
    -----------------------------------------------------------------------------*/
    const EXTENT& GetContentExtent() const;

    /**----------------------------------------------------------------------------
    Returns the number of objects in node and sub-nodes added

    @return Total number of objects in node and sub-nodes
    -----------------------------------------------------------------------------*/
    uint64_t GetCount() const;

    /**----------------------------------------------------------------------------
    Returns the number of levels for which there is splits leafward. This number
    can be used to identify the densest subnode of a node. The split level
    is the deepest level below the node for which there is still splitting.

    @return Split depth
    -----------------------------------------------------------------------------*/
    size_t GetSplitDepth() const;

    /**----------------------------------------------------------------------------
    Returns the sub-node no split of the index.

    @return Sub-node no split
    -----------------------------------------------------------------------------*/
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > GetSubNodeNoSplit() const;

    /**----------------------------------------------------------------------------
    Indicates if the node or one of its sub-nodes contains data.

    @return true if node and sub-nodes are empty
    -----------------------------------------------------------------------------*/
    bool IsEmpty() const;
        
    /**----------------------------------------------------------------------------
     Get the data store
    -----------------------------------------------------------------------------*/
    ISMDataStoreTypePtr<EXTENT> GetDataStore()
        {
        return m_SMIndex->GetDataStore();
        }

    /**----------------------------------------------------------------------------
     Stores the present node on store (Discard) and stores all sub-nodes prior to this
    -----------------------------------------------------------------------------*/
    virtual bool Store();

    /**----------------------------------------------------------------------------
     Stores point node header and points to the store. It does no provoque storage
     of sub-nodes.
     Overloads HPMStoredPooledVector::Discard()
     In addition advises parent node in case of block ID change.
    -----------------------------------------------------------------------------*/
    virtual bool Discard();

    /**----------------------------------------------------------------------------
     Adds pointnode if it is contained in the extent
     or if the extent of the node can be extended.

     @param Point to add to in the index

     @param ExtentFixed IN indicates if the extent can be increased to include given
             point. To be meaningfull, the node must be the first and only node of the tree
             which means it is a leaf and has no parent, otherwise this parameter
             value is ignored.

     @return True if the point could be added and false otherwise.
    -----------------------------------------------------------------------------*/
    virtual bool AddConditional (const POINT pi_rpSpatialObject, bool ExtentFixed, bool isPoint3d);

    /**----------------------------------------------------------------------------
     Adds a list of points in node if it can. The method will
     add the points that fit into the node starting at the first point till a point does
     not fit. The method will not continue into the list to add points that would
     have been included in the node but will stop at the first occurence that does not fit into
     the node.

     @param Pointer to point array to add in the index

     @param startSpatialIndex IN the first index in the array of points
               to add (Previous objects are assumed to have already been added)

     @param countPoints IN The number of points in array

     @param ExtentFixed IN indicates if the extent can be increased to include given
             point. To be meaningfull, the node must be the first and only node of the tree
             which means it is a leaf and has no parent, otherwise this parameter
             value is ignored.

     @return The index of the last added point. If the returned value
             is countPoints then all points of the array have been added.

    -----------------------------------------------------------------------------*/
    virtual size_t AddArrayConditional (const POINT* pointsArray, size_t startPointIndex, size_t countPoints, bool ExtentFixed, bool are3dPoints, bool isRegularGrid=false);

    /**----------------------------------------------------------------------------
     Adds point in node.
     The point must be included (contained) in node extent

     @param Point to add
    -----------------------------------------------------------------------------*/
    virtual bool Add(const POINT pi_rpSpatialObject, bool isPoint3d);
                  
    /**----------------------------------------------------------------------------
     Clears all points that are included in given shape.

     @param pi_shapeToClear IN The shape that defines the area to clear.

     @returns The number of points removed
    -----------------------------------------------------------------------------*/
    virtual size_t Clear(HFCPtr<HVEShape> pi_shapeToClear, double pi_minZ, double pi_maxZ);    

    /**----------------------------------------------------------------------------
    Remove all points in the giving region.

    @param pi_extentToClear IN The extent that defines the region to clear.

    @returns The number of points removed
    ----------------------------------------------------------------------------*/
    virtual size_t RemovePoints(const EXTENT& pi_extentToClear);    

    /**----------------------------------------------------------------------------
     This method provoques the propagation of data down immediately.
     It overloads the base class method to indicate that filtering needs to be performed.
    -----------------------------------------------------------------------------*/
    virtual void PropagateDataDownImmediately(bool propagateRecursively = true);


    virtual void OnPropagateDataDown() {};

    virtual void OnPushNodeDown() {};


    /**----------------------------------------------------------------------------
     Returns the filter used for filtering the points when promoting them to upper levels.

     @return Pointer to filter or NULL if none is set.
    -----------------------------------------------------------------------------*/
    ISMPointIndexFilter<POINT, EXTENT>*
    GetFilter() const;


    /**----------------------------------------------------------------------------
     Initiates a pre-filtering of the node. This is called prior to calling the
     filtering process. The pre-filtering is recursive and will call sub-nodes
     pre-filtering.
    -----------------------------------------------------------------------------*/
    virtual bool PreFilter();

    /**----------------------------------------------------------------------------
     Initiates the filtering of the node.
     The filtering is recursive and will call sub-nodes filtering.
    -----------------------------------------------------------------------------*/
    virtual void Filter(int pi_levelToFilter = -1);    

    /**----------------------------------------------------------------------------
     Initiates a post-filtering of the node. This is called after calling the
     filtering process. The post-filtering is recursive and will call sub-nodes
     post-filtering.
    -----------------------------------------------------------------------------*/
    virtual bool PostFilter();
            
    /**----------------------------------------------------------------------------
     Loads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    virtual void Load() const; 

    /**----------------------------------------------------------------------------
    Unloads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    virtual void Unload();

    /**----------------------------------------------------------------------------
    This method indicates if the node is loaded or not.
    -----------------------------------------------------------------------------*/
    bool IsLoaded() const;

    virtual bool Destroy();


    virtual void LoadTreeNode(size_t& nLoaded, int level, bool headersOnly);

    uint32_t       GetNbObjects() const;

    uint64_t      GetNbObjectsAtLevel(size_t pi_depthLevel) const;

    void SetGenerating(bool isGenerating)
        {
        m_isGenerating = isGenerating;
        if (IsLoaded() && !m_nodeHeader.m_IsLeaf)
            {
            if(!m_nodeHeader.m_IsBranched)
                {
                if (m_pSubNodeNoSplit != nullptr) m_pSubNodeNoSplit->SetGenerating(isGenerating);
                }
            else
                {
                for (size_t node = 0; node < m_nodeHeader.m_numberOfSubNodesOnSplit; ++node)
                    if (m_apSubNodes[node] != nullptr) m_apSubNodes[node]->SetGenerating(isGenerating);
                }
            }
        }

    virtual void         AddOpenGroup(const size_t&, SMNodeGroup* pi_pNodeGroup) const;

    virtual void         SaveAllOpenGroups() const;

    void                 SavePointsToCloud(ISMDataStoreTypePtr<EXTENT>& pi_pDataStore);
    virtual void         SaveGroupedNodeHeaders(SMNodeGroup* pi_pGroup, SMNodeGroupMasterHeader* pi_pGroupsHeader);

#ifdef INDEX_DUMPING_ACTIVATED
    virtual void         DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
                                  bool pi_OnlyLoadedNode) const;
#endif

#ifdef __HMR_DEBUG    
    void         ValidateIs3dDataNodeState(const vector<DRange3d>& source2_5dRanges, const vector<DRange3d>& source3dRanges) const;
#endif


    /**----------------------------------------------------------------------------
    This method indicates that a split node just occured. For a balanced index
    the maximum level depth of each branch must be equal. The method will propagate
    this increase in depth toward sub-nodes and parent node except if it is the
    calling node. The method will of course result in an increase of the depth by
    split or unsplit sub-node generation.

    @param initiator Pointer to node that calls this function. This node will not be called
    during the message propagation thus preventing infinite loops.

    @param quadTreeDepth IN the final depth of the index. This value is used for
    balanced quadtrees
    -----------------------------------------------------------------------------*/
    virtual void PropagateSplitNode(HFCPtr<SMPointIndexNode<POINT, EXTENT> > initiator, size_t octTreeDepth);

    /**----------------------------------------------------------------------------
    This method pushes the root down, by creating a single sub-node and pushing
    all spatials down to this subnode. The process is repeated till the desired provided
    target level. This method is only called for balanced quadtrees

    @param targetLevel IN The level depth desired.
    -----------------------------------------------------------------------------*/
    virtual void PushNodeDown(size_t targetLevel);


    /**----------------------------------------------------------------------------
    This method pushes the root down, by creating a single sub-node that points to this
    sub-node for all its data and metadata. The process is repeated till the desired provided
    target level. This method is only called for balanced trees.
    The resulting subnodes have a unique neighbor list, but otherwise refer to their parent
    for all data.

    @param targetLevel IN The level depth desired.
    -----------------------------------------------------------------------------*/
    virtual void PushNodeDownVirtual(size_t targetLevel);


    void SetZRange(const double zMin, const double zMax);
    /**----------------------------------------------------------------------------
    This method is called by a non-split subnode to its parent. The purpose
    is to advise that a split should have been performed, yet such a split cannot be performed
    at an unsplit node level. The non-split node calls his parent that if this parent is
    also unsplit will propagate the event to its parent, up until the initial unsplit node
    is reached.
    This initial unsplit node will remember that it must split and will split at the earliest
    possible moment. Typically, this call occurs during batch addition of spatial objects
    into the node. The addition causes the split treshold to be reached but the split
    is delayed till the control can be given back to the initial unsplit node.

    This method is only used for unsplit nodes thus will only be called for
    balaced quadtrees
    -----------------------------------------------------------------------------*/
    virtual void AdviseDelayedSplitRequested() const;

    /**----------------------------------------------------------------------------
    This method indicates if the node is destroyed. Typically a destroyed node
    is a node that used to be an unsplit sub-node that got orphaned
    following a subsequent split. The destroyed state reminds this node
    that it must not attempt to store itself upon destruction.
    -----------------------------------------------------------------------------*/
    virtual bool IsDestroyed() const;

    /**----------------------------------------------------------------------------
    This method advises that a neighbor node has been changed.
    -----------------------------------------------------------------------------*/
    virtual void AdviseNeighborNodeChange(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& oldNeighborNode, HFCPtr<SMPointIndexNode<POINT, EXTENT> >& newNeighborNode);

    POINT GetDefaultSplitPosition();

    /**----------------------------------------------------------------------------
    This method causes a split of the node to occur. This method can only be
    called on a leaf node or the parent of an unsplit subnode.
    The optional parameter indicates if the split event must be propagated
    to the whole tree for index balancing purposes.
    -----------------------------------------------------------------------------*/
    virtual void SplitNode(POINT splitPosition, bool propagateSplit = true);

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > AddChild(EXTENT newExtent);
    
    void SortSubNodes();

    /*
    * Re-split a branched node (used to get different splits e.g. 8/4 children).
    */
    virtual void ReSplitNode(POINT splitPosition, size_t newNumberOfChildNodesOnSplit);

    void SetNumberOfSubNodesOnSplit(size_t numberOfSubNodesOnSplit);


    bool HasRealChildren() const;

    bool IsParentOfAVirtualNode() const;

    bool IsParentOfARealUnsplitNode() const;

    HPMBlockID         GetBlockID() const;

    size_t             GetNbPoints() const
        {
        if (!IsLoaded())
            Load();

        //NEEDS_WORK_SM : Try do create something cleaner when doing storage factoring 
        //(i.e. : having count only in header automatically modified when storedpoolvector is modified).
        return m_nodeHeader.m_nodeCount;       
        }

    void LockPts()
        {
        m_ptsLock.lock();        
        }

    void UnlockPts()
        {        
        m_ptsLock.unlock();
        }

    /**----------------------------------------------------------------------------
    Get the neighbor reciprocal position.
    -----------------------------------------------------------------------------*/
    bool AreAllNeighbor2_5d() const;

    /**----------------------------------------------------------------------------
    Get the neighbor reciprocal position.
    -----------------------------------------------------------------------------*/
    size_t GetReciprocalNeighborPos(size_t neighborPos);

    /**----------------------------------------------------------------------------
    Setup the neighbor nodes of newly creating sub-nodes after a split.
    -----------------------------------------------------------------------------*/
    void SetupNeighborNodesAfterSplit();

    /**----------------------------------------------------------------------------
    Setup a neighbor relation between two nodes.
    -----------------------------------------------------------------------------*/
    void SetNeighborRelationAfterSplit(size_t subNodeInd, size_t parentNeighborInd);

    /**----------------------------------------------------------------------------
    Setup a neighbor relation between two nodes.
    -----------------------------------------------------------------------------*/
    void SetupNeighborNodesAfterPushDown();

    /**----------------------------------------------------------------------------
    Setup a neighbor relation between two nodes. 
    -----------------------------------------------------------------------------*/
    void SetNeighborRelationAfterPushDown(size_t          neighborNodeInd,
                                          vector<size_t>& neighborSubNodeIndexes,
                                          size_t          neighborSubNodeNeighborInd);

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > FindNode(EXTENT ext, size_t level) const;    

    void ValidateNeighborsOfChildren();
    bool NeighborsFillParentExtent();
    bool AllSiblingsAreNeighbors();
    bool AllNeighborsAreAtSameLevel();

#ifndef NDEBUG
    void ValidateNeighbors();

    //Replace pi_node by this.
    void ValidateDualNeighborship(HFCPtr<SMPointIndexNode<POINT, EXTENT> > pi_node);

    //Replace pi_node by this.
    void ValidateNeighborsAfterSplit(HFCPtr<SMPointIndexNode<POINT, EXTENT> > pi_node);

    void ValidateNeighborsAfterPushDown(HFCPtr<SMPointIndexNode<POINT, EXTENT> > pi_node);
#endif
    
    // Did tried to make this not public but it is accessed by HGFSpatialIndex which has some unknown and undefinable template arguments by the node so
    // it cannot be made friend either.
    vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>         m_apSubNodes;

    //Neighbor node at the same level as this node
    vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> m_apNeighborNodes[MAX_NUM_NEIGHBORNODE_POSITIONS];

    bool                              m_isGenerating;
    mutable SMIndexNodeHeader<EXTENT> m_nodeHeader;         // The node header. Contains permanent control data.
    mutable bool m_wasBalanced;
    bool m_needsBalancing;
    bool m_isGrid;
    //NEEDS_WORK_SM : Need to be set even if SMMeshIndexNode are not used (point cloud)
    SMPointIndex<POINT, EXTENT>* m_SMIndex;

    virtual void                ValidateInvariants() const
        {
        ValidateInvariantsSoft();

#ifdef __HMR_DEBUG
        // We only check invariants if the node is loaded ...
        if (IsLoaded() && !m_destroyed)
            {

            // The following verifications  require that subnodes or parent nodes be loaded
            if (HasRealChildren())
                {
                if ((m_pSubNodeNoSplit != NULL) && (!m_pSubNodeNoSplit->IsLoaded()))
                    return;

                if (m_pSubNodeNoSplit == NULL)
                    {
                    for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                        {
                        if (m_apSubNodes[indexNodes] != NULL && !m_apSubNodes[indexNodes]->IsLoaded())
                            return;
                        }
                    }
                }

            if (GetParentNodePtr() != NULL)
                {
                if (!GetParentNodePtr()->IsLoaded())
                    return;

                }

            if (m_nodeHeader.m_totalCountDefined)
                {
                // The total number of points in a leaf node must be the node content itself        
                //NEEDS_WORK_SM:Reactivate this
                //HASSERT((!m_nodeHeader.m_IsLeaf)|| (m_nodeHeader.m_totalCount == this->size()) || IsVirtualNode());

                // If the node is not leaf and is unsplit then total count must be equal to self size + unsplit child                
                //NEEDS_WORK_SM - depends on filtering. 
                HASSERT(m_nodeHeader.m_IsLeaf || (m_pSubNodeNoSplit == 0) || true /*|| (m_nodeHeader.m_totalCount == this->size() + m_pSubNodeNoSplit->m_nodeHeader.m_totalCount) || ((this->GetFilter()->IsProgressiveFilter() == false) && (m_nodeHeader.m_totalCount == m_pSubNodeNoSplit->m_nodeHeader.m_totalCount))*/);

                // If the node is not leaf and is unsplit then total count must be equal to self size + unsplit child
                if (!m_nodeHeader.m_IsLeaf && m_pSubNodeNoSplit == 0)
                    {
                    //MST TBD : Need to be reactivated
                    //uint64_t effectiveTotalCount = this->GetFilter()->IsProgressiveFilter() ? this->size() : 0;
                    /*
                    uint64_t effectiveTotalCount = 0;

                    for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                    {
                    effectiveTotalCount += m_apSubNodes[indexNodes]->m_nodeHeader.m_totalCount;
                    }
                    HASSERT(m_nodeHeader.m_totalCount == effectiveTotalCount);
                    */
                    }
                }


            // Either case the node is an unsplit sublevel which implies it must have a parent and this parent may node be a leaf and the subnode no split must
            // point to this node. OR the node is not an unsplit sublevel which implies that either it has no parent or the parent is not a leaf and does not
            // point to an unsplit node.
            HASSERT((m_nodeHeader.m_IsUnSplitSubLevel && GetParentNodePtr() != NULL && !GetParentNodePtr()->m_nodeHeader.m_IsLeaf && (GetParentNodePtr()->m_pSubNodeNoSplit == this)) ||
                    (!m_nodeHeader.m_IsUnSplitSubLevel && ((GetParentNodePtr() == NULL) || (!GetParentNodePtr()->m_nodeHeader.m_IsLeaf && (GetParentNodePtr()->m_pSubNodeNoSplit == NULL)))));

            // Balancing must be homogenous throughout the index
            //NEEDS_WORK_SM: During partial update, possibly not.
            if (HasRealChildren())
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
                    //HASSERT(m_pSubNodeNoSplit->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                    }
                else
                    {
                    for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                        {
                       // HASSERT(m_apSubNodes[indexNodes] == NULL || m_apSubNodes[indexNodes]->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                        }
                    }
                }
            if (GetParentNodePtr() != NULL)
                {
                //NEEDS_WORK_SM : During balancing it is posisble that the parent node is balanced but some 
                //of its children has not been balanced yet.
                //HASSERT (GetParentNodePtr()->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                }



            }

#endif
        };

    virtual void               ValidateInvariantsSoft() const
        {            
#ifdef __HMR_DEBUG
        // We only check invariants if the node is loaded ...
        if (IsLoaded())
            {

            // If the node is a leaf, then subnodes must not exist                
            HASSERT(!m_nodeHeader.m_IsLeaf || m_apSubNodes.size() < 8 || ((m_pSubNodeNoSplit == 0) && (m_apSubNodes[0] == 0) && (m_apSubNodes[1] == 0) && (m_apSubNodes[2] == 0) && (m_apSubNodes[3] == 0) && (m_apSubNodes[4] == 0) && (m_apSubNodes[5] == 0) && (m_apSubNodes[6] == 0) && (m_apSubNodes[7] == 0)));

            // If the node is not a leaf, then subnodes must exist
            //HASSERT(m_nodeHeader.m_IsLeaf || m_apSubNodes.size() < 8 || (m_pSubNodeNoSplit != 0) || ((m_apSubNodes[0] != 0) && (m_apSubNodes[1] != 0) && (m_apSubNodes[2] != 0) && (m_apSubNodes[3] != 0) && (m_apSubNodes[4] != 0) && (m_apSubNodes[5] != 0) && (m_apSubNodes[6] != 0) && (m_apSubNodes[7] != 0)));


            HASSERT((m_pSubNodeNoSplit == 0) || (!m_pSubNodeNoSplit->IsLoaded()) || (this->m_nodeHeader.m_level + 1 == m_pSubNodeNoSplit->m_nodeHeader.m_level));

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                HASSERT((m_apSubNodes[indexNodes] == 0) || (!m_apSubNodes[indexNodes]->IsLoaded()) || (this->m_nodeHeader.m_level + 1 == m_apSubNodes[indexNodes]->m_nodeHeader.m_level));
                }

            // If the node is a leaf, then split treshold must not be attained
            // This condition cannot be true anymore as split can be delayed.
            // HASSERT(!m_nodeHeader.m_IsLeaf || this->size() < m_nodeHeader.m_SplitTreshold);


            // The content extent must fit completely into the node extent at all times (if defined).
            if (m_nodeHeader.m_contentExtentDefined)
                {
                //NEEDS_WORK_SM : Deactivated for now since the stitching triangles of the mesh might intersect the tile boundary. 
                /*
                HASSERT(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                */
                }

            // Make sure that the subnodes fallows some rules
            if (!m_nodeHeader.m_IsLeaf)
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
                    if (m_pSubNodeNoSplit->IsLoaded())
                        {
                        //HASSERT(m_nodeHeader.m_balanced == m_pSubNodeNoSplit->m_nodeHeader.m_balanced);

                        HASSERT(ExtentOp<EXTENT>::GetXMin(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetXMax(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetYMin(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetYMax(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                        }

                    }
                else
                    {
                    for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                        {
                        if (m_apSubNodes[indexNodes] != NULL && m_apSubNodes[indexNodes]->IsLoaded())
                            {
                            //HASSERT(m_nodeHeader.m_balanced == m_apSubNodes[indexNodes]->m_nodeHeader.m_balanced);
                            HASSERT(ExtentOp<EXTENT>::GetXMin(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetXMax(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetYMin(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetYMax(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                            }

                        }
                    }
                }

            // If the count of points is zero then the memory must be cleared.
            }

#endif
        }

  

protected:

    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<SMPointIndexNode<POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes);

    /**----------------------------------------------------------------------------
     Sets the neighbor nodes.

     @param pi_apNeighborNodes - Array of neighbor nodes. 
    -----------------------------------------------------------------------------*/
    virtual void SetNeighborNodes();

    /**----------------------------------------------------------------------------
     Performs the pre-query process upon node and sub-nodes

     @param queryObject IN The query object to apply pre-query with.

     @return true if the process should continue and false otherwise.

    -----------------------------------------------------------------------------*/
    bool              PreQuery (ISMPointIndexQuery<POINT, EXTENT>* queryObject);

    /**----------------------------------------------------------------------------
     Performs the query process upon node and sub-nodes

     @param queryObject IN The query object to apply query with.

     @param points IN OUT The list of ponts to which the query object may add
       points as part of the result set.

     @return true if the process should dig down into sub-nodes and false otherwise.
     TBD Something wrong with query return value.
    -----------------------------------------------------------------------------*/    
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, HPMMemoryManagedVector<POINT>& points);
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh);
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, ProducedNodeContainer<POINT, EXTENT>& foundNodes, IStopQuery* stopQueryP);
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, vector<QueriedNode>& meshNodes);
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, vector<QueriedNode>& meshNodes, bool* pIsComplete, bool isProgressiveDisplay = false, bool hasOverview = false, StopQueryCallbackFP stopQueryCallbackFP = 0);        
    bool              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, HFCPtr<SMPointIndexNode<POINT, EXTENT>>& resultNode);        

    bool              QueryVisibleNode (ISMPointIndexQuery<POINT, EXTENT>* queryObject, size_t maxLevel, ProducedNodeContainer<POINT, EXTENT>& overviewNodes, ProducedNodeContainer<POINT, EXTENT>& foundNodes, ProducedNodeContainer<POINT, EXTENT>& nodesToSearch, IStopQuery* stopQueryP);   
    bool              QueryOverview (ISMPointIndexQuery<POINT, EXTENT>* queryObject, size_t maxLevel, ProducedNodeContainer<POINT, EXTENT>& overviewNodes, ProducedNodeContainer<POINT, EXTENT>& foundNodes, ProducedNodeContainer<POINT, EXTENT>& nodesToSearch, IStopQuery* stopQueryP);       


    /**----------------------------------------------------------------------------
     Performs the post-query process upon node and sub-nodes

     @param queryObject IN The query object to apply post-query with.

     @return true if the process should continue and false otherwise.
    -----------------------------------------------------------------------------*/
    bool              PostQuery (ISMPointIndexQuery<POINT, EXTENT>* queryObject);

    void              AddNumberObjectsInAncestors(EXTENT& pi_rNodeExtent,
                                                  uint32_t& pio_rNbPoints) const;
    

    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    bool AddArrayUnconditional(const POINT* pointsArray, size_t countPoints, bool are3dPoints, bool isRegularGrid=false);

    /**----------------------------------------------------------------------------
     This method advises that one of the subnode storage block ID has changed
     as a result of reallocation, destruction or initial store.

     @param p_subNode - Pointer to sub node for which the ID changed
    -----------------------------------------------------------------------------*/
    virtual void AdviseSubNodeIDChanged(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& p_subNode);

    /**----------------------------------------------------------------------------
     This method advises that one of the neighbor storage block ID has changed
     as a result of reallocation, destruction or initial store.

     @param p_neighborNode - Pointer to neighbor node for which the ID changed
    -----------------------------------------------------------------------------*/
    virtual void AdviseNeighborNodeIDChanged(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& p_neighborNode);

    /**----------------------------------------------------------------------------
     This method advises the children nodes that the storage block ID of their parent has changed
     as a result of reallocation, destruction or initial store.     
    -----------------------------------------------------------------------------*/
    virtual void AdviseParentNodeIDChanged();    

    /**----------------------------------------------------------------------------
     Indicates if the node is filtered

     @return true if the node is filtered and false otherwise.
    -----------------------------------------------------------------------------*/
    virtual bool IsFiltered() const;


    /**----------------------------------------------------------------------------
     Indicates if the node need filtering. The fact it is filtered is not
     sufficient not to need filtering, if any sub-node need filtering then
     it also required re-filtering.
    -----------------------------------------------------------------------------*/
    virtual bool NeedsFiltering() const;

    /**----------------------------------------------------------------------------
     Saves node header and point data in files that can be used for streaming
     point data from a cloud server.
    -----------------------------------------------------------------------------*/
    void SavePointDataToCloud(ISMDataStoreTypePtr<EXTENT>& pi_pDataStreamingStore);

    ISMPointIndexFilter<POINT, EXTENT>* m_filter;

        
    CreatedNodeMap* m_createdNodeMap;     

    virtual void SetNodeExtent(const EXTENT& extent);    

    void               SetParentNodePtr(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& parentNodePtr);    

    void               Track3dPoints(size_t countPoints);
    
    size_t         m_numSubNodes;

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > m_pSubNodeNoSplit;  // Pointer to non-split sub-node. If NULL then node is either a leaf or has split sub-nodes

    //NEEDS_WORK_SM : Try removing all those mutable.
    mutable bool m_loaded;
    mutable bool m_destroyed;


    // The following members are currently used for invariant validation purposes
    // Indicates when a node cannot be split. The most likely reason is that the coordinates extremes are very close
    // to the maximum number of digits.
    HDEBUGCODE(mutable bool m_unspliteable;)
        // The following member indicates that one of its sub-node or sub-node of sub-node is unspliteable
        // Since in the case of an unspliteable node then number of points can exceed the number of
        // allowed treshold points, after filtering the parent can likewise exceed this number
        // This member is set upon filtering only. If the node has never been filtered then it is meaningless.
        HDEBUGCODE(mutable bool m_parentOfAnUnspliteableNode;)

        //NEEDS_WORK_SM : Cached here but should eventually be in the header        
        //NEEDS_WORK_SM : Only different if progressive filtering, which is not supported anymore.
        mutable int32_t   m_NbObjects;     // The total number of object in and over(in parent nodes (within tile extent)) this node.

    mutable bool m_DelayedSplitRequested;    // Control variable. Indicates a split is requested for the initial parent of un-split sub-nodes.

    bool m_delayedDataPropagation;


    private:

        static ISMStore::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<ISMStore::NodeID>(blockID.m_integerID);
            }

        static ISMStore::NodeID ConvertChildID(const HPMBlockID& childID)
            {
            return static_cast<ISMStore::NodeID>(childID.m_integerID);
            }

        static ISMStore::NodeID ConvertNeighborID(const HPMBlockID& neighborID)
            {
            return static_cast<ISMStore::NodeID>(neighborID.m_integerID);
            }

        //Should be accessed using GetParentNode.        
        bool m_isParentNodeSet;
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > m_pParentNode;      // Parent node      
        SMMemoryPoolItemId m_pointsPoolItemId;
        uint64_t           m_nodeId; 
        bool               m_isDirty;
        std::mutex         m_ptsLock;
        
        static std::map<size_t, SMNodeGroup*> s_OpenGroups;
        static int s_GroupID;    
    };


template <class POINT, class EXTENT, class NODE> class SMIndexNodeVirtual : public NODE
    {
    //static_assert(std::is_base_of<SMPointIndexNode<POINT,EXTENT>, NODE>::value, "Virtual node can only be used with point index nodes");
    public:
        SMIndexNodeVirtual(const NODE* rParentNode) : NODE(rParentNode->GetSplitTreshold(), rParentNode->GetNodeExtent(), const_cast<NODE*>(rParentNode))
            {
            m_nodeHeader.m_contentExtent = rParentNode->m_nodeHeader.m_contentExtent;
            m_nodeHeader.m_nodeExtent = rParentNode->m_nodeHeader.m_nodeExtent;
            m_nodeHeader.m_numberOfSubNodesOnSplit = rParentNode->m_nodeHeader.m_numberOfSubNodesOnSplit;
            m_nodeHeader.m_totalCount = rParentNode->m_nodeHeader.m_totalCount;
            m_nodeHeader.m_IsLeaf = true;
            m_nodeHeader.m_IsBranched = false;
            }
        virtual bool IsVirtualNode() const override
            {
            volatile bool a = 1;
            a = a;
            return true;
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


    template<class POINT, class EXTENT> class SMPointIndex : public HFCShareableObject<SMPointIndex<POINT, EXTENT>>
    {

public:

    static map<void*, int> s_allNodes;
    static int s_lastNodeIdx;
    // Primary methods
    /**----------------------------------------------------------------------------------------------
     Constructor for this class. The split threshold is used to indicate the maximum
     amount of spatial objects to be indexed by an index node after which the node is
     split.

        @param pool IN The memory manager pool to be used. This pool is used to limit the
        number of points in memory at any given time.

        @param store IN The store to which points are stored. This store is used in conjunction with
            pool management system.

        @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                                   node after which the node may be split.

    -------------------------------------------------------------------------------------------------*/
    SMPointIndex(ISMDataStoreTypePtr<EXTENT>& newDataStore, size_t SplitTreshold, ISMPointIndexFilter<POINT, EXTENT>* filter, bool balanced, bool propagatesDataDown, bool shouldCreateRoot = true);
    /**----------------------------------------------------------------------------
     Destructor
     If the index has unstored nodes then those will be stored.
    -----------------------------------------------------------------------------*/
    virtual             ~SMPointIndex();

    /**----------------------------------------------------------------------------
     Returns the meory pool
    -----------------------------------------------------------------------------*/
    HFCPtr<HPMCountLimitedPool<POINT> >
    GetPool() const;
    
    /**----------------------------------------------------------------------------
     Returns the data store
    -----------------------------------------------------------------------------*/
    ISMDataStoreTypePtr<EXTENT> GetDataStore();

    /**----------------------------------------------------------------------------
     Returns the next node id available
    -----------------------------------------------------------------------------*/
    uint64_t GetNextNodeId();

    /**----------------------------------------------------------------------------
     Forces an immmediate store (to minimize the chances of corruption
    -----------------------------------------------------------------------------*/
    virtual bool        Store();
    
    /**----------------------------------------------------------------------------
     Returns the filter used for filtering the points when promoting them to upper levels.

     @return Pointer to filter or NULL if none is set.
    -----------------------------------------------------------------------------*/
    ISMPointIndexFilter<POINT, EXTENT>*
    GetFilter() ;


    /**----------------------------------------------------------------------------
     Push the data in leaf and balance the octree
    -----------------------------------------------------------------------------*/
    //NEEDS_WORK_SM : Maybe should be virtual.
    void BalanceDown(size_t depthBeforePartialUpdate, bool keepUnbalanced = false);

    /**----------------------------------------------------------------------------
     Remove useless root
    -----------------------------------------------------------------------------*/
    bool BalanceRoot();

    /*
    Recursively set nodes as balanced or not.
    */
    void SetBalanced(bool balanced);

    /**----------------------------------------------------------------------------
    If maximum depth termined only by no splitted subnode then pull them up. 
    -----------------------------------------------------------------------------*/
    void PullSubNodeNoSplitUp();

    /**----------------------------------------------------------------------------
     Unsplit empty node after data removal
    -----------------------------------------------------------------------------*/
    bool UnsplitEmptyNode();

    
    /**----------------------------------------------------------------------------
     Initiates the filtering of the point index, This filtering process includes
     recursively calling global pre-filtering, pre=filtering of nodes, filtering,
     node post-filtering then global post-filtering.

    -----------------------------------------------------------------------------*/
    virtual void        Filter(int pi_levelToFilter);


    /**----------------------------------------------------------------------------
     Initiates a query upon the point index, This query process includes
     recursively calling global pre-query, pre-query of nodes, query proper,
     node post-query then global post-filtering.

    -----------------------------------------------------------------------------*/    
    size_t              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                               HPMMemoryManagedVector<POINT>& resultPoints);
    size_t              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                               BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* resultMesh);
    size_t              Query(ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT>>& resultNode);   
    size_t              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                               vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);           

    //NEEDS_WORK_SM : Could be removed once new display is activated. 
    size_t              Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                               vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes, 
                               bool* pIsComplete, 
                               bool isProgressiveDisplay, 
                               bool hasOverview = false, 
                               StopQueryCallbackFP stopQueryCallbackFP = 0);           
   
    uint64_t             GetNbObjectsAtLevel(size_t pi_depthLevel);

    /*
    This method is used as a factory to create the right type of node.
    */
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(EXTENT extent, bool isRootNode = false);
    virtual HFCPtr<SMPointIndexNode<POINT, EXTENT> > CreateNewNode(HPMBlockID blockID, bool isRootNode = false);
    
    bool                AddArray (const POINT* pointsArray, size_t countOfPoints, bool arePoints3d, bool regularGrid=false);
    bool                Clear(HFCPtr<HVEShape> pi_shapeToClear);    
    bool                RemovePoints(const EXTENT& pi_extentToClear);    

    StatusInt           SaveGroupedNodeHeaders(DataSourceAccount *dataSourceAccount, const WString& pi_pOutputDirectoryName, const short& pi_pGroupMode, bool pi_pCompress = true);
    StatusInt           SavePointsToCloud(DataSourceAccount *dataSourceAccount, const WString& pi_pOutputDirectoryName, bool pi_pCompress = true);
    StatusInt           SaveMasterHeaderToCloud(DataSourceAccount *dataSourceAccount);

#ifdef INDEX_DUMPING_ACTIVATED    
    virtual void                DumpOctTree(char* pi_pOutputXMLFileName, bool pi_OnlyLoadedNode) const;
#endif

#ifdef __HMR_DEBUG
    void         ValidateIs3dDataStates(const vector<DRange3d>& source2_5dRanges, const vector<DRange3d>& source3dRanges) const;            
#endif

#ifdef SCALABLE_MESH_ATP
    unsigned __int64    m_nbInputPoints;
#endif    

    void SetNextID(const uint64_t& id);
    uint64_t GetNextID() const;


    /**----------------------------------------------------------------------------
    Indicates if the data is propagated toward the leaves immediately or if it is
    delayed till the most appropariate moment (re-filtering for example)

    @return true if the data is immediately propagated towards the leaves.

    -----------------------------------------------------------------------------*/
    bool                PropagatesDataDown() const;

    /**----------------------------------------------------------------------------
    Changes an Propagate Data Down setting. Changing this value will not provoke a
    propagation down of the data currently located in the parent nodes but deactivating
    this propagation will prevent data to immediately propagate data. Newly added
    data will remain in upper parent nodes till required.
    -----------------------------------------------------------------------------*/
    void SetPropagateDataDown(bool propagate);

    /**----------------------------------------------------------------------------
    This method provoques the propagation of data down immediately.
    Note that even if data propagation is off this function can be called. It will
    not change the PropagatesDataDown() value.

    This emthod may result in an increase of the depth of the index as propagating
    data down may result in new node splitting as a result of split treshold being
    attained.
    -----------------------------------------------------------------------------*/
    virtual void PropagateDataDownImmediately();

    /**----------------------------------------------------------------------------
    Indicates if the node is part of a balanced index

    @return true if the index is balanced and false otherwise.

    -----------------------------------------------------------------------------*/
    bool                IsBalanced() const;

    bool                IsTextured() const;

    bool IsSingleFile() const;
    void SetSingleFile(bool singleFile);

    void SetIsTerrain(bool isTerrain)
        {
        m_indexHeader.m_isTerrain = isTerrain;
        m_indexHeaderDirty = true;
        }

    bool IsTerrain() const
        {
        return m_indexHeader.m_isTerrain;
        }


    /**----------------------------------------------------------------------------
    Changes an unbalanced index to a balanced index. If the index is already balanced
    nothing will occur otherwise the balancing will immediately be performed.
    -----------------------------------------------------------------------------*/
    void                Balance();

    /**----------------------------------------------------------------------------
    Returns the number of subnodes on split. This number can be 4 or 8

    -----------------------------------------------------------------------------*/
    size_t              GetNumberOfSubNodesOnSplit() const;

    /**----------------------------------------------------------------------------
    Returns the highest depth level of the index

    @return The highest depth level

    -----------------------------------------------------------------------------*/
    size_t              GetDepth() const;

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > FindNode(EXTENT ext, size_t level) const;

    size_t              GetTerrainDepth() const;

    /**----------------------------------------------------------------------------
    Returns the root node of the index

    @return The root node

    -----------------------------------------------------------------------------*/
    HFCPtr<SMPointIndexNode<POINT, EXTENT> >       GetRootNode() const;    

    /**----------------------------------------------------------------------------
    Create the root node of the index

    @return The root node

   -----------------------------------------------------------------------------*/        
    HFCPtr<SMPointIndexNode<POINT, EXTENT> >       CreateRootNode();       
      
    /**----------------------------------------------------------------------------
    Gets the effective limiting outter extent. This extent is the node extent
    of the root node.

    @return Returns the extent of spatial index.
    -----------------------------------------------------------------------------*/
    EXTENT              GetIndexExtent() const;
    /**----------------------------------------------------------------------------
    Gets the effective limiting outter extent of the content.

    @return Returns the extent of the content of the spatial index.
    -----------------------------------------------------------------------------*/
    EXTENT              GetContentExtent() const;

    /**----------------------------------------------------------------------------
    Returns the total number of objects in index

    @return The number of objects

    -----------------------------------------------------------------------------*/
    uint64_t              GetCount() const;

    /**----------------------------------------------------------------------------
    Indicates if the index is empty

    @return true if the index is empty and false otherwise

    -----------------------------------------------------------------------------*/
    bool                IsEmpty() const;

    /**----------------------------------------------------------------------------
    Returns the split treshold value for index. This treshold value is the
    maximum number of items idexed by a single node before splitting occurs.

    @return The treshold value.

    -----------------------------------------------------------------------------*/
    size_t              GetSplitTreshold() const;

    void LoadTree (size_t& nLoaded, int level, bool headersOnly);
    void SetGenerating(bool isGenerating)
        {
        m_isGenerating = isGenerating;
        if (m_pRootNode != nullptr) m_pRootNode->SetGenerating(isGenerating);
        }

#ifndef NDEBUG
    void                ValidateNeighbors();
#endif
protected:

    SMPointIndex(const SMPointIndex&   pi_rSpatialObject);

    SMPointIndex&
    operator=(const SMPointIndex& pi_rObj);

    virtual void                ValidateInvariants() const
        {

#ifdef __HMR_DEBUG

        //            if (m_pRootNode != NULL)
        //                HASSERT(m_pRootNode->GetDepth() == m_pRootNode->GetSplitDepth());
        // Notice that even if we have strong aggregation we do not check invariants of root node
#endif
        };
       
    ISMDataStoreTypePtr<EXTENT> m_dataStore;
    std::atomic<uint64_t>       m_nextNodeID;

    ISMPointIndexFilter<POINT, EXTENT>* m_filter;    
    typename SMPointIndexNode<POINT, EXTENT>::CreatedNodeMap m_createdNodeMap;

    virtual void        PushRootDown(const EXTENT& pi_rObjectExtent);    

    bool                HasMaxExtent() const;
    EXTENT              GetMaxExtent() const;


    HFCPtr<SMPointIndexNode<POINT, EXTENT>>                    m_pRootNode;

    SMIndexMasterHeader<EXTENT> m_indexHeader;

    bool                    m_indexHeaderDirty;

    bool                    m_isGenerating;

    bool                    m_needsBalancing;

    bool                    m_propagatesDataDown;    
    };






/*======================================================================================================
** The ISMPointIndexFilter provides a mechanism to implement a generic filter for the point index
**
** An object that implements this interface is provided to the point index and will be invoqued at need
** by this index or when the filtering process is started manually. The index will then traverse
** the index content providing the filter with individual nodes and sub-nodes.
**
** The interface defines many methods that will be called at different stages of the filter process.
**
** The first one called is the GlobalPreFilter() method. This method is provided with the
** index upon which the filter is to be performed.
** At this stage, the filter can simply clear any remaining settings from previous filtering that may
** have been based upon assumptions concerning the index that may have changed after previous filtering.
** The index is provided and can be used in any required analysis.
**
** The ImplementsPreFiltering() and ImplementsPostFiltering() indicate if it is necessary to call PreFilter
** and PostFilter() methods for the index. Calling PreFilter() or PostFilter() should not result in
** adverse effects even if they are not implemented.
**
** The second stage of the query is to call the PreFilter() given pre-filtering is implemented (ImplementsPreFiltering)
** method upon ALL nodes part of the index.
** This stage can be used for example in the analysis of the node content in order to determine
** parameters required during the filtering stage proper. Note that the PreFilter will operate upon  parent nodes
** of a node only if true is returned for the sub-node. It is then possible to stop the pre filter process
** by returning false once all pre-filtering analysis required has been completed either for a branch or for the
** whole tree. Note that if any of the subnpodes returns true then the parent node will be prefiltered.
**
** The next process is the Filter itself. At this stage the Filter method is called iteratively for all
** nodes part of the index. It is at this stage and the current SMPointIndexFilter implementation
** that parent nodes or sub-nodes are repopulated according to the filtering applied. If the Filter function returns false, then
** subnodes of the node being filtered will not be filtered, thus stopping the Filter traversal for this branch
** of the index.
**
** Exactly the same way as the PreFilter, a PostFilter is called for every node of the index. The PostFilter
** can be used to clean any changes of states that may have been brought during the PreFilter. For example
** a table of point oriented data may have been created during the Filter process that now needs to be
** cleaned. Again returning false will stop the traversal process for the branch
**
** Finally a GlobalPostFilter method is called. It can  be used to put the index pre-filter state if it
** has been changed.
**
** NOTE: It is not recommendable to modify any state of the nodes or the index during the process
** other than reordering point distribution between nodes and sub-nodes,
** but for practical purposes, a limited amount of states can be modified by the filter object.
** For this reason the filter process will be executed as a single step upon the index, forbidding during the
** process any other threads to operate upon the index or nodes. For this same reason, the PostFilter process
** and the GlobalPostFilter process will always complete regardless an error or exception is thrown.
**====================================================================================================*/

template<class POINT, class EXTENT> class ISMPointIndexFilter
    {
public:
    ISMPointIndexFilter() {};
    virtual ~ISMPointIndexFilter() {};
    
    /*======================================================================================================
    ** This property indicates if the filtering process is progressive or not. Since the main purpose of
    ** filtering is to spread spatially index points conceptually located at the leaf towards parent nodes
    ** to provide importance to these points, the filter may either copy the leaf points or move points
    ** to parent nodes. If points are moved then the filter is progressive and if they are copied then the
    ** filter is not progressive. This state is very important to understand the structure of the point
    ** distribution and essential when Query process will be performed.
    **====================================================================================================*/
    virtual bool        IsProgressiveFilter() const 
        {
        return false;
        }

    /*======================================================================================================
    ** The ImplementsPreFiltering() indicates if it is necessary to call PreFilter() for the index nodes.
    ** The defaults PreFilter() does nothing and as a result the default ImplementsPreFiltering() returns false.
    **====================================================================================================*/
    virtual bool        ImplementsPreFiltering () {
        return false;
        };

    /*======================================================================================================
    ** The ImplementsPostFiltering() indicates if it is necessary to call PostFilter() for the index nodes.
    ** The defaults PostFilter() does nothing and as a result the default ImplementsPostFiltering() returns false.
    **====================================================================================================*/
    virtual bool        ImplementsPostFiltering () {
        return false;
        };

    /*======================================================================================================
    ** The GlobalPreFilter method is called before the pre-filter and filter process ... It provides a way
    ** for the filter object to allocate ressources or do whatever is requesred prior to starting the
    ** process. Global PreFilter is not usually required.
    ** Since most Filter objects do not need a global pre-filter performed a default implementation is provided.
    **
    ** @return true if the global pre-filter must continue and false to cancel it all together. If false neither
    ** pre-filter, filter or post-filter will be called, but the global post-filter will still be called
    **====================================================================================================*/
    virtual bool        GlobalPreFilter (SMPointIndex<POINT, EXTENT>& index) {
        return true;
        };


    /*======================================================================================================
    ** The PreFilter method is called before the filter process ... It provides a way for the filter object
    ** to determine before the filtering proper starts, conditions to apply to the filter processing.
    ** Since not all filter objects need a pre-filter performed a default implementation is provided.
    **
    ** @return true if the pre-filter must continue and false otherwise. Returning false does to stop
    ** the filter process, it only stops the pre-filter of digging deeper into the point index tree structure.
    **====================================================================================================*/
    virtual bool        PreFilter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                   size_t numSubNodes) {
        return false;
        };


    /*======================================================================================================
    ** This method re-orders or cleans the node point datasets. Both node and sub-nodes point set may and
    ** will usually be modified. The process gathers points from all sub-nodes, makes a selection
    ** of points that should be promoted to the parent level then redistributes points in the subnodes.
    **====================================================================================================*/
    virtual bool        Filter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                               std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                size_t numSubNodes) const = 0;

    /*======================================================================================================
    ** The filter leaf process is called for leaf nodes only. It is customary for the real filtering
    ** process to occur when filtering is applied to the parent node. The filter leaf will thus
    ** usually not perform any filtering process yet may be useful for other purposes such as pre-calculations
    ** The method will return true if any processing has been applied to the leaf node and false otherwise.
    **====================================================================================================*/
    virtual bool        FilterLeaf(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node) const
        {
        // Nothing to be done    

        return false;
        }

    /*======================================================================================================
    ** The PostFilter method is called after the filter process ... It provides a way for the filter object
    ** to clean ressources filter or format result set.
    ** Since not all filter objects need a post-filter performed a default implementation is provided.
    **
    ** @return true if the post-filter must continue and false otherwise. Even if false is returned the
    ** global post-filter process will be called.
    **====================================================================================================*/
    virtual bool        PostFilter(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) {
        return false;
        };

    /*======================================================================================================
    ** The GlobalPostFilter method is called after the filter process and the Post-Filter... It provides a way
    ** for the filter object to de-allocate ressources or do whatever is requesred after application of filtering
    ** Global post filtering is not usually required.
    ** Since most filter objects do not need a global post-filtering performed a default implementation is provided.
    **
    ** @return true if the global post-filter was successful and false otherwise. This value may be returned as indication
    ** of the global success of the filter
    **====================================================================================================*/
    virtual bool        GlobalPostFilter (SMPointIndex<POINT, EXTENT>& index) {
        return true;
        };

    };





/*======================================================================================================
** The ISMPointIndexQuery provides a mechanism to implement a generic query for the point index
**
** An object that implements this interface is provided to the point index query function that will call
** as needed the query object, providing this object with nodes and subnodes iteratively through
** the index traversal.
**
** The interface defines many methods that will be called at different stages of the query process.
**
** The first one called is the GlobalPreQuery() method. This method is provided with the
** index upon which the query is to be performed and the target list of result points.
** At this stage, the query can simply clear any remaining settings from previous query that may
** have been based upon assumptions concerning the index that may have changed after previous query.
** The index is provided and can be used in any required analysis.
**
** The second stage of the query is to call the PreQuery() method upon ALL nodes part of the index.
** This stage can be used for example in the analysis of the node content in order to determine
** parameters required during the query stage proper. Note that the PreQuery will operate upon sub-nodes
** of a node only if true is returned for the parent node. It is then possible to stop the pre query process
** by returning false once all prequery analysis required has been completed either for a branch or for the
** whole tree.
**
** The next process is the Query itself. At this stage the Query method is called iteratively for all
** nodes part of the index. It is at this stage and the current SMPointIndexQuery implementation
** that points are gathered and copied into the result set. If the Query function returns false, then
** subnodes of the node being queired will not be queried, thus stopping the Query traversal for this branch
** of the index.
**
** Exactly the same way as the PreQuery, a PostQuery is called for every node of the index. The PostQuery
** can be used to clean any changes of states that may have been brought during the PreQuery. For example
** a table of point oriented data may have been created during the Query process that now needs to be
** cleaned. Again returning false will stop the traversal process for the branch
**
** Finally a GlobalPostQuery method is called. This method can typically be used to analyse and modify
** the result point set. It can also be used to put the index pre-query state if it has been changed.
**
** NOTE: It is not recommendable to modify any state of the nodes or the index during the process
** but for practical purposes, a limited amount of state can be modified by the query object.
** For this reason the query process will be executed as a single step upon tyhe index, forbidding during the
** process any other threads to operate upon the index or nodes. For this same reason, the PostQuery process
** and the GlobalPostQuery process will always complete regardless an error or exception is thrown.
**====================================================================================================*/

template<class POINT, class EXTENT> class ISMPointIndexQuery
    {
public:
    ISMPointIndexQuery() {};
    virtual ~ISMPointIndexQuery() {};

    /*======================================================================================================
    ** The GlobalPreQuery method is called before the pre-query and query process ... It provides a way
    ** for the query object to allocate ressources or do whatever is requesred prior to starting the
    ** process. Global preQuery is not usually required.
    ** Since most query objects do not need a gloabl pre-query performed a default implementation is provided.
    **
    ** @return true if the query must continue and false to cancle it all together. If false neither
    ** pre-query, query or post-query will be called, but the global post-query will still be callsed
    **====================================================================================================*/   
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points) {
        return true;
        };

    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* resultMesh)
        {
        return true;
        };

    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,                    
                                        vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes) {
        return true;
        };

    



    /*======================================================================================================
    ** The PreQuery method is called before the query process ... It provides a way for the query object
    ** to determine before the query proper starts, conditions to apply to the query processing.
    ** Since not all query objects need a pre-query performed a default implementation is provided.
    **
    ** @return true if the pre-query must continue and false otherwise. Returning false does to stop
    ** the query process, it only stops the pre-query of digging deeper into the point index tree structure.
    **====================================================================================================*/
    virtual bool        PreQuery(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                 HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes) {
        return false;
        };


    /*======================================================================================================
    ** This method gathers all points from the node that satisfy the query. The
    ** subnodes are provided for determination of the points that must be gathered only.
    ** The subnodes will be queried after this call UNLESS false is returned, thus
    ** indicating that query conditions have been reached for this branch of the index
    ** and digging down any further is unnecessary. The query process will not be altogether cancelled
    ** to completely cancel the query process, it is possible for all subsequent calls to this Query method
    ** to immediately return false. The hierarchical and recursive nature of the index insures that the query
    ** process will complete promptly in just a few calls.
    **
    ** Even if the query is stopped, the post-query process will be called
    **====================================================================================================*/    
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& points) = 0;
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* meshPtr)
        {
        assert(!"Not implemented"); return 0;
        };
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes) {assert(!"Not implemented"); return 0;};    

    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               ProducedNodeContainer<POINT, EXTENT>& foundNodes) {assert(!"Not implemented"); return 0;};    
    
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode)
          {
          assert(!"Not implemented"); return 0;
          };


    /*======================================================================================================
    ** The PostQuery method is called after the query process ... It provides a way for the query object
    ** to clean ressources filter or format result set.
    ** Since not all query objects need a post-query performed a default implementation is provided.
    **
    ** @return true if the post-query must continue and false otherwise. Even if false is returned the
    ** global post-query process will be called.
    **====================================================================================================*/
    virtual bool        PostQuery(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes) {
        return false;
        };

    /*======================================================================================================
    ** The GlobalPostQuery method is called after the query process and the Post-Query... It provides a way
    ** for the query object to allocate ressources or do whatever is requesred prior to starting the
    ** process. Global preQuery is not usually required.
    ** Since most query objects do not need a global post-query performed a default implementation is provided.
    **
    ** @return true if the query was successful and false otherwise. This value may be returned as indication
    ** of the global success of the query
    **====================================================================================================*/    
    virtual bool        GlobalPostQuery (SMPointIndex<POINT, EXTENT>& index,
                                         HPMMemoryManagedVector<POINT>& points) {
        return true;
        }

    virtual bool        IsVisible(const EXTENT& pi_rExtent) {
        HASSERT(!"Should be pure virtual");
        return false;
        }

    virtual void GetQueryNodeOrder(vector<size_t>&                            queryNodeOrder, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> >  node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> >  subNodes[],
                                   size_t numSubNodes)
        {
        for (size_t ind = 0; ind < numSubNodes; ind++)
            {
            queryNodeOrder.push_back(ind);
            }
        }



#ifdef ACTIVATE_NODE_QUERY_TRACING
    /*======================================================================================================
    **  Those methods are called by the application and the index to activate the node query tracing mechanism.
    **  The default is to have no node query tracing mechanism.
    **====================================================================================================*/
    virtual void        SetTracingXMLFileName(string& pi_rTracingXMLFileName) {};
    virtual void        OpenTracingXMLFile() {};
    virtual void        CloseTracingXMLFile() {};
#endif

    };



/*======================================================================================================
** The ISMPointIndexSpatialLimitWrapQuery provides a mechanism to implement a generic query for the point index
** with application of an optional spatial 2-Dimensional limit.
** It behaves exactly like a normal query except that any and all nodes that do not overlap the limit
** shape are not traversed and the query process returns false for such non-overlapping nodes, effectively
** preventing digging down into its sub-nodes. All other nodes are pushed to the wrapped query
**
** In addition to this the present implementation provides a mechansim for optionaly filtering out
** points that are not part of the limit specified. These services can be ignored.
**
** The default implementations of PreQuery, Query and PostQuery will automatically check the
**
**====================================================================================================*/

template<class POINT, class EXTENT> class ISMPointIndexSpatialLimitWrapQuery : public ISMPointIndexQuery<POINT, EXTENT>
    {
private:
    HFCPtr<HVE2DShape> m_limit;
    ISMPointIndexQuery* m_wrappedQuery;

    bool IsNodeDisjointFromLimit(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node)
        {
        // Create a rectangle from extent
        EXTENT nodeExtent = node->GetNodeExtent();
        HVE2DRectangle myNodeShape(ExtentOp<EXTENT>::GetXMin(nodeExtent),
                                   ExtentOp<EXTENT>::GetYMin(nodeExtent),
                                   ExtentOp<EXTENT>::GetXMax(nodeExtent),
                                   ExtentOp<EXTENT>::GetYMax(nodeExtent),
                                   m_limit->GetCoordSys());
        return ((myNodeShape.CalculateSpatialPositionOf(*m_limit) == HVE2DShape::S_OUT) &&
                (m_limit->CalculateSpatialPositionOf(myNodeShape) == HVE2DShape::S_OUT));
        }



public:
    ISMPointIndexSpatialLimitWrapQuery(HFCPtr<HVE2DShape> pi_Limit, ISMPointIndexQuery* pi_Query)
        {
        m_limit = pi_Limit;
        m_wrappedQuery = pi_Query;
        }
    virtual ~ISMPointIndexSpatialLimitWrapQuery()
        {
        delete m_wrappedQuery;
        }


    /*======================================================================================================
    ** @return Returns the wrapped query
    **====================================================================================================*/
    ISMPointIndexQuery* GetWrappedQuery() const {
        return m_wrappedQuery;
        }

    /*======================================================================================================
    ** @return Returns the limit
    **====================================================================================================*/
    HFCPtr<HVE2DShape> GetLimit() const {
        return m_limit;
        }

    /*======================================================================================================
    ** The PreQuery method is called before the query process ... It provides a way for the query object
    ** to determine before the query proper starts, conditions to apply to the query processing.
    ** Since not all query objects need a pre-query performed a default implementation is provided.
    **
    ** @return true if the pre-query must continue and false otherwise. Returning false does to stop
    ** the query process, it only stops the pre-query of digging deeper into the point index tree structure.
    **====================================================================================================*/
    virtual bool        PreQuery(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                 HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes)
        {
        // Check if node is within limit
        if (IsNodeDisjointFromLimit (node))
            {
            // Shapes are disjoint
            return false;
            }

        // Called wrapped query
        return m_wrappedQuery->PreQuery(node, subNodes, numSubNodes);
        }

    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& points)
        {
        // Check if node is within limit
        if (IsNodeDisjointFromLimit (node))
            {
            // Shapes are disjoint
            return false;
            }
        
        HPMMemoryManagedVector<POINT> tempList(&s_queryMemoryManager);    

        // Called wrapped query
        bool status =  m_wrappedQuery->Query(node, subNodes, numSubNodes, tempList);

        HPMMemoryManagedVector<POINT>::iterator itrPts;
        for (itrPts = tempList.begin() ; itrPts != tempList.end() ; itrPts++)
            {
            if (m_limit->IsPointIn(HGF2DLocation(PointOp<POINT>::GetX(*itrPts), PointOp<POINT>::GetY(*itrPts), m_limit->GetCoordSys())))
                {
                if (points.size() == points.capacity())
                    points.reserve(points.size() + (points.size()/10) + 1);
                points.push_back(*itrPts);
                }
            }

        return status;

        }



    /*======================================================================================================
    ** The PostQuery method is called after the query process ... It provides a way for the query object
    ** to clean ressources filter or format result set.
    ** Since not all query objects need a post-query performed a default implementation is provided.
    **
    ** @return true if the post-query must continue and false otherwise. Even if false is returned the
    ** global post-query process will be called.
    **====================================================================================================*/
    virtual bool        PostQuery(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes)
        {
        // Check if node is within limit
        if (IsNodeDisjointFromLimit (node))
            {
            // Shapes are disjoint
            return false;
            }

        // Called wrapped query
        return m_wrappedQuery->PostQuery(node, subNodes, numSubNodes);
        }



    virtual bool        IsVisible(const EXTENT& pi_rExtent) {
        HASSERT(!"Should be pure virtual");
        return false;
        }



#ifdef ACTIVATE_NODE_QUERY_TRACING
    /*======================================================================================================
    **  Those methods are called by the application and the index to activate the node query tracing mechanism.
    **  The default is to have no node query tracing mechanism.
    **====================================================================================================*/
    virtual void        SetTracingXMLFileName(string& pi_rTracingXMLFileName) {};
    virtual void        OpenTracingXMLFile() {};
    virtual void        CloseTracingXMLFile() {};
#endif

    };



/*======================================================================================================
** HGFLevelPointIndexQuery
**
** This query fetches points within a specified area (extent) at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level.
**====================================================================================================*/
template<class POINT, class EXTENT> class HGFLevelPointIndexQuery: public ISMPointIndexQuery<POINT, EXTENT>
    {

private:
    size_t m_expectedLevel;


protected:
    size_t m_requestedLevel;
    EXTENT m_extent;
        bool   m_returnAllPtsForLowestLevel;
        size_t m_maxNumberOfPoints;

public:


        HGFLevelPointIndexQuery(const EXTENT extent, /*double meterToNPCMatrix[][4],*/ size_t level, bool returnAllPtsForLowestLevel = true, 
                                size_t maxNumberOfPoints = std::numeric_limits<size_t>::max()) 
        {
                                m_extent = extent;
                                m_requestedLevel = level;                                
            m_returnAllPtsForLowestLevel = returnAllPtsForLowestLevel;   
            m_maxNumberOfPoints = maxNumberOfPoints;
        }

                            virtual ~HGFLevelPointIndexQuery() {}
   
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        // Make sure the index is balanced ... we stop if it is not
        if (!index.IsBalanced())
            return false;

        // Obtain the deepest level of index and make sure requested index is
        size_t fullIndexDepth = index.GetDepth();

        // If requested level is deeper than actual index depth we limit to index depth
        if (fullIndexDepth < m_requestedLevel)
            return false;

        return true;
        }   

    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& resultPoints)
        {       
            // Before to continue, make sure we don't already have the maximum of points allow   
            if (m_maxNumberOfPoints < resultPoints.size())
                return false;
            
        // Before we make sure requested level is appropriate
        if (m_requestedLevel < 0)
            m_requestedLevel = 0;


        if (ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent) && node->GetLevel() <= m_requestedLevel)

            {
            // If this is the appropriate level or it is a higher level and progressive is set.
            if (node->IsLeaf() ||
                m_requestedLevel == node->GetLevel() ||
                (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel()))
                {
                // Copy content
                POINT nodeExtentOrigin = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMin(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMin(node->GetNodeExtent()), ExtentOp<EXTENT>::GetZMin(node->GetNodeExtent()));
                POINT nodeExtentCorner = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMax(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMax(node->GetNodeExtent()), ExtentOp<EXTENT>::GetZMax(node->GetNodeExtent()));

                // If the whole node is located within extent ... copy it all
                if ((ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentOrigin) && 
                     ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentCorner)) || 
                        ((node->GetLevel() == 0) && m_returnAllPtsForLowestLevel && //Always return all the points in the lowest level. 
                      (node->GetFilter()->IsProgressiveFilter() == true)))  
                    {                     
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                    for (size_t currentIndex = 0 ; currentIndex < ptsPtr->size(); currentIndex++)
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(ptsPtr->operator[](currentIndex));
                        }
                    }
                else
                    {
                    // Search in present list of objects for current node
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                    for (size_t currentIndex = 0 ; currentIndex < ptsPtr->size(); currentIndex++)
                        {
                        // Check if point is in extent of object                        
                        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, ptsPtr->operator[](currentIndex)))
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(ptsPtr->operator[](currentIndex));
                            }
                        }
                    }
                }                
                
                // Verify if we have the maximum of point allow
                if (m_maxNumberOfPoints < resultPoints.size())
                    return false;
                    
            return true;
            }
        else
            {
            // No need to dig deeper in subnodes ... either extents do not overlap or
            // level reached
            return false;
            }
        }

    };

/*======================================================================================================
** HGFLevelPointIndexByShapeQuery 
**
** This query fetches points within a specified area (shape) at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level.
**====================================================================================================*/
template<class POINT, class EXTENT> class HGFLevelPointIndexByShapeQuery: public ISMPointIndexQuery<POINT, EXTENT>
{

private:
    size_t m_expectedLevel;

protected: 
    size_t m_requestedLevel;
    HFCPtr<HVE2DShape> m_areaShape;
    bool m_returnAllPtsForLowestLevel;
    size_t m_maxNumberOfPoints;
    
public:


    HGFLevelPointIndexByShapeQuery(HFCPtr<HVE2DShape> areaShape, /*double meterToNPCMatrix[][4],*/ size_t level, bool returnAllPtsForLowestLevel = true, 
                                   size_t maxNumberOfPoints = std::numeric_limits<size_t>::max()) 
        {
        m_areaShape = areaShape;
        m_requestedLevel = level; 
        m_returnAllPtsForLowestLevel = returnAllPtsForLowestLevel; 
        m_maxNumberOfPoints = maxNumberOfPoints;                             
        }

    virtual ~HGFLevelPointIndexByShapeQuery() {}
    
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
        HPMMemoryManagedVector<POINT>& points) 
        {
        // Make sure the index is balanced ... we stop if it is not
        if (!index.IsBalanced())
            return false;

        // Obtain the deepest level of index and make sure requested index is
        size_t fullIndexDepth = index.GetDepth();

        // If requested level is deeper than actual index depth we limit to index depth            
        if (fullIndexDepth < m_requestedLevel)
            return false;

        return true;
        }


    // The Query process gathers points up to level depth  
    virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
        size_t numSubNodes,
        HPMMemoryManagedVector<POINT>& resultPoints)
        {   
        // Before to continue, make sure we don't already have the maximum of points allow   
        if (m_maxNumberOfPoints < resultPoints.size())
            return false;
                         
        // Before we make sure requested level is appropriate
        if (m_requestedLevel < 0)
            m_requestedLevel = 0;

        HFCPtr<HVE2DShape> pNodeShape(new HVE2DRectangle(ExtentOp<EXTENT>::GetXMin(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMin(node->GetNodeExtent()), 
                                                         ExtentOp<EXTENT>::GetXMax(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMax(node->GetNodeExtent()), m_areaShape->GetCoordSys()));

      

        HVE2DShape::SpatialPosition finalePosition = HVE2DShape::S_OUT;
        
        HVE2DShape::SpatialPosition nodePosition = m_areaShape->CalculateSpatialPositionOf(*pNodeShape);
        
        if (nodePosition != HVE2DShape::S_OUT)
            finalePosition = nodePosition;
        else 
            {
            // We need to do this test because if the shape is all inside the node, we need to return S_PARTIALY_IN
            HVE2DShape::SpatialPosition shapePosition = pNodeShape->CalculateSpatialPositionOf(*m_areaShape);

            if (shapePosition == HVE2DShape::S_IN)
                finalePosition = HVE2DShape::S_PARTIALY_IN;            
            }
            
        if (finalePosition != HVE2DShape::S_OUT && node->GetLevel() <= m_requestedLevel)
            {            
            // If this is the appropriate level or it is a higher level and progressive is set.
            if (node->IsLeaf() || 
                m_requestedLevel == node->GetLevel() || 
                (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel()))
                {
                // If the whole node is located within extent ... copy it all
                if ((finalePosition == HVE2DShape::S_IN) || 
                    ((node->GetLevel() == 0) && m_returnAllPtsForLowestLevel && //Always return all the points in the lowest level. 
                    (node->GetFilter()->IsProgressiveFilter() == true)))  
                    {
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                    for (size_t currentIndex = 0 ; currentIndex < ptsPtr->size(); currentIndex++)
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(ptsPtr->operator[](currentIndex));
                        }
                    }
                else
                    {                    
                    // Search in present list of objects for current node
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                    for (size_t currentIndex = 0 ; currentIndex < ptsPtr->size(); currentIndex++)
                        {
                        // Check if point is in extent of object
                        if (m_areaShape->IsPointIn(HGF2DLocation(ptsPtr->operator[](currentIndex).x, ptsPtr->operator[](currentIndex).y, m_areaShape->GetCoordSys())))
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(ptsPtr->operator[](currentIndex));
                            }
                        }
                    }  
                }                

            // Verify if we have the maximum of point allow
            if (m_maxNumberOfPoints < resultPoints.size())
                return false;
                
            return true;
            }
        else
            {
            // No need to dig deeper in subnodes ... either extents do not overlap or 
            // level reached
            return false;
            }
        }
};
    
/*======================================================================================================
** HGFAutoLevelPointIndexQuery
**
** This query fetches points within a specified area at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level. The level is automatically detemined upon a simple criteria. The extent of nodes at
** target level must be smaller than the requested area. A level bias can be added to take an even deeper
** level than the level that satisfy the spatial condition.
**====================================================================================================*/
template<class POINT, class EXTENT> class HGFAutoLevelPointIndexQuery: public HGFLevelPointIndexQuery<POINT, EXTENT>
    {
private:
    size_t m_levelBias;
    bool   m_levelSet;

public:

    HGFAutoLevelPointIndexQuery(const EXTENT* exent, size_t levelBias = 0)
        : HGFLevelPointIndexQuery (extent, 0)
        {
        m_levelBias = levelBias;
        m_levelSet = false;
        }
    virtual ~HGFAutoLevelPointIndexQuery();

    // The global prequery simply cleans pre-query specific structure    
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        m_levelSet = false;

        return true;
        };



    // The pre-query will determine the depth level to perform the query upon
    virtual bool        PreQuery (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes)
        {
        if (ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            {
            if (IsLeaf())
                m_requestedLevel = (int)GetLevel();

            if (!m_levelSet)
                {
                if (ExtentOp<EXTENT>::GetWidth(node->GetNodeExtent()) < ExtentOp<EXTENT>::GetWidth(m_extent))
                    {

                    // current level satisfactory ... go ahead ... set it
                    m_requestedLevel = (int)GetLevel() + m_levelBias;
                    m_levelSet = true;
                    }
                }
            }

        // Request to continue till level is set
        return (!m_levelSet);
        }

    };


/*======================================================================================================
** HGFNeighborNodesPointIndexQuery
**
** This object uses the query traversal mechansim to gather a list of neighbor nodes. The points
** are not needed nor copied. The query object requires a node upon construction and will return
** all an any nodes that are contiguous to it that are part of the index.
** IF the index is balanced in form then nodes from the same level are returned only. If the index
** is not balanced then only leaf nodes will be gathered.
**====================================================================================================*/
template<class POINT, class EXTENT> class HGFNeighborNodesPointIndexQuery: public ISMPointIndexQuery<POINT, EXTENT>
    {
private:
    SMPointIndex<POINT, EXTENT>*  m_index;
    HFCPtr<SMPointIndexNode<POINT, EXTENT> >  m_node;
    list <HFCPtr<SMPointIndexNode<POINT, EXTENT> > > m_neighborNodes;
public:

    HGFNeighborNodesPointIndexQuery(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node)
        {
        m_node = node;
        m_index = NULL;
        }
    virtual ~HGFNeighborNodesPointIndexQuery()
        {
        m_index = NULL;
        m_node = NULL;
        }

    const list<HFCPtr<SMPointIndexNode<POINT, EXTENT> > >&
    GetResultNeighborList() const
        {
        return m_neighborNodes;
        }

    // The global prequery simply cleans pre-query specific structure   
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        // Save the index
        m_index = &index;
        m_neighborNodes.clear();
        return true;
        };

    // The Query process gathers points up to level depth   
    virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& resultPoints)
        {
        // We only search branches for which the exents do not overlap (but are simply contiguous)
        if (ExtentOp<EXTENT>::InnerOverlap(m_node->GetNodeExtent(), node->GetNodeExtent()))
            return true;

        // The nodes must at least be contiguous otherwise no need to continue
        if (!ExtentOp<EXTENT>::OutterOverlap(m_node->GetNodeExtent(), node->GetNodeExtent()))
            return false;

        // We only look at nodes that are leafes OR than are on the same level as node if balanced
        // If the level is deeper ... we stop branch otherwise we dig deeper in branch
        if (m_index->IsBalanced() && node->GetLevel() > m_node->GetLevel())
            return false;
        if (m_index->IsBalanced() && node->GetLevel() < m_node->GetLevel())
            return true;
        if (!m_index->IsBalanced() && !node->IsLeaf())
            return true;

        // If we get there then we have a neighbor node
        m_neighborNodes.push_back(node);

        return false;
        }  

    // The global post query releases the index pointer
    virtual bool        GlobalPostQuery (SMPointIndex<POINT, EXTENT>& index,
                                         HPMMemoryManagedVector<POINT>& points)
        {
        // Save the index
        list<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>::iterator nodeIter(m_neighborNodes.begin());
        list<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>::iterator nodeIterEnd(m_neighborNodes.end());

        EXTENT inflatedNodeExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMin(m_node->GetNodeExtent()) - ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetXMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetWidth(m_node->GetNodeExtent()) * 0.05,
                                                             ExtentOp<EXTENT>::GetYMax(m_node->GetNodeExtent()) + ExtentOp<EXTENT>::GetHeight(m_node->GetNodeExtent()) * 0.05);

        while (nodeIter != nodeIterEnd)
            {
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr((*nodeIter)->GetPointsPtr());

            for (uint32_t PtInd = 0; PtInd < ptsPtr->size(); PtInd++)
                {
                if (ExtentOp<EXTENT>::InnerOverlap(ptsPtr->GetNodeExtent(), inflatedNodeExtent) == false)
                    continue;

                POINT point = ptsPtr->operator[](PtInd);

                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(point, inflatedNodeExtent))
                    {
                    if (points.size() == points.capacity())
                        points.reserve(points.size() + (points.size()/10) + 1);
                    points.push_back(point);
                    }
                }

            nodeIter++;
            }

        m_index = NULL;
        return true;
        };
    };




/*======================================================================================================
** HGFViewDependentPointIndexQuery
**
** This point index query will gather points according to view parameters. At construction the
** view matrix is provided. It will access individual nodes view parameters if they are present
**
** In addition to gathering points, the query object will also gather the extents of the nodes
** that satisfy the query requirements.
**
**====================================================================================================*/
template<class POINT, class EXTENT> class HGFViewDependentPointIndexQuery: public ISMPointIndexQuery<POINT, EXTENT>
    {
private:

    list<HVE2DSegment> m_listOfTileBreaklines;

protected :

    EXTENT m_extent;
    bool   m_gatherTileBreaklines;    
    double m_viewportRotMatrix[3][3];
    double m_rootToViewMatrix[4][4];

    void AddBreaklinesForExtent(const EXTENT&       pi_rExtent)
        {
        HFCPtr<HGF2DCoordSys> pDummyCoordSys = new HGF2DCoordSys();

        HGF2DLocation LLPoint(ExtentOp<EXTENT>::GetXMin(pi_rExtent), ExtentOp<EXTENT>::GetYMin(pi_rExtent), pDummyCoordSys);
        HGF2DLocation LRPoint(ExtentOp<EXTENT>::GetXMax(pi_rExtent), ExtentOp<EXTENT>::GetYMin(pi_rExtent), pDummyCoordSys);
        HGF2DLocation ULPoint(ExtentOp<EXTENT>::GetXMin(pi_rExtent), ExtentOp<EXTENT>::GetYMax(pi_rExtent), pDummyCoordSys);
        HGF2DLocation URPoint(ExtentOp<EXTENT>::GetXMax(pi_rExtent), ExtentOp<EXTENT>::GetYMax(pi_rExtent), pDummyCoordSys);

        m_listOfTileBreaklines.push_back(HVE2DSegment(LLPoint, LRPoint));
        m_listOfTileBreaklines.push_back(HVE2DSegment(LLPoint, ULPoint));
        m_listOfTileBreaklines.push_back(HVE2DSegment(LRPoint, URPoint));
        m_listOfTileBreaklines.push_back(HVE2DSegment(ULPoint, URPoint));
        }

public:
    HGFViewDependentPointIndexQuery(const EXTENT& extent,
                                    const double rootToViewMatrix[][4],
                                    const double viewportRotMatrix[][3],
                                    bool gatherTileBreaklines)
        {
        m_extent = extent;        
        m_gatherTileBreaklines = gatherTileBreaklines;

        for (size_t i = 0 ; i < 3 ; i++)
            for (size_t j = 0 ; j < 3 ; j++)
                m_viewportRotMatrix[i][j] = viewportRotMatrix[i][j];

        for (size_t i = 0 ; i < 4 ; i++)
            for (size_t j = 0 ; j < 4 ; j++)
                m_rootToViewMatrix[i][j] = rootToViewMatrix[i][j];
        };

    virtual ~HGFViewDependentPointIndexQuery() {};

    // New interface ...

    list<HVE2DSegment>& GetListOfTileBreaklines()
        {
        return m_listOfTileBreaklines;
        }

    // The global prequery simply cleans query specific structure (the list of tile breaklines)    
    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        HPMMemoryManagedVector<POINT>& points)
        {
        m_listOfTileBreaklines.clear();        
        return true;
        };

    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh)
        {
        m_listOfTileBreaklines.clear();        
        return true;
        };

    virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                        vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
        {
        m_listOfTileBreaklines.clear();        
        return true;
        };
 

    virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                               HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                               size_t numSubNodes,
                               HPMMemoryManagedVector<POINT>& resultPoints)
        {

        // Check if extent overlap
        if (!ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        finalNode = !IsCorrectForCurrentView(node, node->GetNodeExtent(), m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (finalNode == false)
            {
            if (node->GetFilter()->IsProgressiveFilter())
                {
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                for (size_t currentIndex = 0 ; currentIndex < ptsPtr->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if ((node->GetFilter()->IsProgressiveFilter() && (node->GetLevel() == 0)) ||
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, (ptsPtr->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(ptsPtr->operator[](currentIndex));
                        }
                    }
                }
            }
        else
            {
            if ((node->GetParentNode() != 0) &&
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPtsPtr(parentNode ->GetPointsPtr());

                for (size_t currentIndex = 0 ; currentIndex < parentPtsPtr->size(); currentIndex++)
                    {
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, parentPtsPtr->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(parentPtsPtr->operator[](currentIndex));
                        }
                    }
                }
            }

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

        if (finalNode && m_gatherTileBreaklines && ptsPtr->size() > 0)
            {
            AddBreaklinesForExtent(node->GetNodeExtent());
            }

        return (!finalNode);
        }


    virtual bool IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                         const EXTENT& pi_visibleExtent,                                         
                                         double        pi_RootToViewMatrix[][4]) const = 0;
    };


/** -----------------------------------------------------------------------------

    This class implements a default filter for spatial index of points. It takes
    one every four points out of every vectors.
    -----------------------------------------------------------------------------
*/
template<class POINT, class EXTENT> class SMPointIndexDefaultFilter : public ISMPointIndexFilter<POINT, EXTENT>
    {

public:

    // Primary methods
    SMPointIndexDefaultFilter() {};
    virtual             ~SMPointIndexDefaultFilter() {};

    // IHGFPointFilter implementation
    virtual bool        Filter (HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
                                HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                size_t numSubNodes) const
        {
        size_t totalNumPoints = 0;
        for (size_t indexNode = 1; indexNode < numSubNodes; indexNode++)
            {
            if (subNodes[indexNode] != NULL)
                {
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(subNodes[indexNode]->GetPointsPtr());
                totalNumPoints += ptsPtr->size();
                }
            }


        parentNode->reserve (totalSubNodes / numSubNodes + 1);
        for (size_t indexNode = 1; indexNode < numSubNodes; indexNode++)
            {
            if (subNodes[indexNode] != NULL)
                {
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(subNodes[indexNode]->GetPointsPtr());
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPtsPtr(parentNode->GetPointsPtr());

                parentPtsPtr->push_back (&(*ptsPtr[0]), ptsPtr->size());                
                }
            }

        return true;
        }




    virtual bool        FilterLeaf (HFCPtr<SMPointIndexNode<POINT, EXTENT> >  outputNode) const
        {
        return true;
        }

    virtual bool        IsProgressiveFilter() const {
        return false;
        };
    };

/*======================================================================================================
** SMLeafPointIndexQuery
**
** This query fetches leaf points within a specified area (extent). It is possible to limit the number
** of returned points or use a query returning nodes for memory management.
**====================================================================================================*/
template<class POINT, class EXTENT> class SMLeafPointIndexQuery : public ISMPointIndexQuery<POINT, EXTENT>
    {

    private:


    protected:
        EXTENT m_extent;
        bool   m_returnAllPtsForLowestLevel;
        size_t m_maxNumberOfPoints;

    public:


        SMLeafPointIndexQuery(const EXTENT extent, /*double meterToNPCMatrix[][4],*/ bool returnAllPtsForLowestLevel = true,
                                size_t maxNumberOfPoints = std::numeric_limits<size_t>::max())
            {
            m_extent = extent;
            m_returnAllPtsForLowestLevel = returnAllPtsForLowestLevel;
            m_maxNumberOfPoints = maxNumberOfPoints;
            }

        virtual ~SMLeafPointIndexQuery() {}
      
        virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes,
                                  HPMMemoryManagedVector<POINT>& resultPoints)
            {
            // Before to continue, make sure we don't already have the maximum of points allow   
            if (m_maxNumberOfPoints < resultPoints.size())
                return false;


            if (ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))

                {
                // If this is the appropriate level or it is a higher level and progressive is set.
                if (node->IsLeaf())
                    {
                    // Copy content
                    POINT nodeExtentOrigin = PointOp<POINT>::Create(ExtentOp<EXTENT>::GetXMin(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMin(node->GetNodeExtent()), ExtentOp<EXTENT>::GetZMin(node->GetNodeExtent()));
                    POINT nodeExtentCorner = PointOp<POINT>::Create(ExtentOp<EXTENT>::GetXMax(node->GetNodeExtent()), ExtentOp<EXTENT>::GetYMax(node->GetNodeExtent()), ExtentOp<EXTENT>::GetZMax(node->GetNodeExtent()));

                    // If the whole node is located within extent ... copy it all
                    if ((ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentOrigin) &&
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentCorner)) ||
                        (m_returnAllPtsForLowestLevel //Always return all the points in the lowest level. 
                        ))
                        {
                        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                        for (size_t currentIndex = 0; currentIndex < ptsPtr->size(); currentIndex++)
                            {
                            // The point falls inside extent of object .. we add a reference to the list
                            resultPoints.push_back(ptsPtr->operator[](currentIndex));
                            }
                        }
                    else
                        {
                        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> ptsPtr(node->GetPointsPtr());

                        for (size_t currentIndex = 0; currentIndex < ptsPtr->size(); currentIndex++)
                            {
                            // Check if point is in extent of object
                            if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, ptsPtr->operator[](currentIndex)))
                                {
                                // The point falls inside extent of object .. we add a reference to the list
                                resultPoints.push_back(ptsPtr->operator[](currentIndex));
                                }
                            }
                        }
                    }

                // Verify if we have the maximum of point allow
                if (m_maxNumberOfPoints < resultPoints.size())
                    return false;

                return true;
                }
            else
                {
                // No need to dig deeper in subnodes ... either extents do not overlap or
                // level reached
                return false;
                }
            }

        virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes,
                                  vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
            {

                if (ExtentOp<EXTENT>::Overlap(node->GetNodeExtent(), m_extent))

                    {
                    // If this is the appropriate level or it is a higher level and progressive is set.
                    if (node->IsLeaf())
                        {
                            // The point falls inside extent of object .. we add a reference to the list
                        meshNodes.push_back(node);
                        }

                    return true;
                    }
                else
                    {
                    // No need to dig deeper in subnodes ... either extents do not overlap or
                    // level reached
                    return false;
                    }
            };
    };


template <class POINT, class EXTENT> class ProducedNodeContainer
    {
     private : 
             
         bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> m_producedNodes;
         std::mutex                                       m_producedNodesMutex;          
         bool                                             m_threadSafe;         

     public :

         ProducedNodeContainer(bool threadSafe = false)
             {
             m_threadSafe = threadSafe;
             }

         void SetThreadSafe(bool threadSafe)
             {
             m_threadSafe = threadSafe;
             }

         void AddNode(HFCPtr<SMPointIndexNode<POINT, EXTENT>>& nodePtr)
             {
             while (m_threadSafe && !m_producedNodesMutex.try_lock());             

#ifndef NDEBUG
             for (auto& node : m_producedNodes)
                 {
                 assert(node.GetPtr() != nodePtr.GetPtr());
                 assert(!(node->GetBlockID() == nodePtr->GetBlockID()));
                 }             
#endif

             m_producedNodes.push_back(nodePtr);      

             if (m_threadSafe)
                m_producedNodesMutex.unlock();
             }

         bvector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>>& GetNodes()
             {
             assert(m_threadSafe == false);
             return m_producedNodes;
             }         

         bool ConsumeNode(HFCPtr<SMPointIndexNode<POINT, EXTENT>>& consumedNodePtr)
             {  
             assert(consumedNodePtr == 0);

             while (m_threadSafe && !m_producedNodesMutex.try_lock());             

             if (m_producedNodes.size() > 0)
                {
                consumedNodePtr = m_producedNodes.back();
                m_producedNodes.pop_back();                
                }

             if (m_threadSafe)
                m_producedNodesMutex.unlock();

             return consumedNodePtr != 0;
             }

         bool WaitConsumption()
             {
             if (StopProducing())
                 return false;

             while (m_threadSafe && !m_producedNodesMutex.try_lock());             

             bool waitConsumption = m_producedNodes.size() > 0;

             if (m_threadSafe)
                m_producedNodesMutex.unlock();

             return waitConsumption;
             }

         virtual bool StopProducing() const
            {
            return false;
            }
     };

//#include "SMPointIndex.hpp"